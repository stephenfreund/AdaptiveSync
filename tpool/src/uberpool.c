/* AUTORIGHTS
 Copyright (C) 2007 Princeton University
 
 This file is part of Ferret Toolkit.
 Ferret Toolkit is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2, or (at your option)
 any later version.
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software Foundation,
 Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
/*
 * tpool.c: A thread pool
 *
 * Functions to start, manage and terminate a thread pool
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include <float.h>

#include "uberpool.h"
#include "uberqueue.h"
#include "probe.h"
#include "ppoint.h"
#include "pthread_barrier.h"
#include "util.h"
#include "uknob.h"

#define max(a,b)				\
({ __typeof__ (a) _a = (a);			\
__typeof__ (b) _b = (b);			\
_a > _b ? _a : _b; })

#define SIZE 100
#define STAGE_MAX 10

typedef struct idesc {
  int id;
  int threads;
  uberdesc_t *desc;
  
  uberqueue_t *in;
  uberqueue_t *out;
  
  // for experiments
  ppoint_t blocked_time;
  ppoint_t count;
  size_t start_blocked;
  size_t start_count;
  
  double work_fraction;
  
} idesc_t;


typedef struct uberthread {
  void *data;
  idesc_t *role;
  uberpool_t *pool;
  int id;
} uberthread_t;


struct uberpool {
  pthread_mutex_t lock;
  pthread_cond_t block;
  
  pthread_barrier_t begin_barrier;
  
  int max_threads;
  int nthreads;
  pthread_t *threads;
  uberthread_t *controls;
  
  int ndescs;
  idesc_t *descs;
  
  pool_state_t state;
  
  uberprobe_t *probe;
  pthread_t adapter;
  
  // for experiments
  size_t start_count;
  size_t start_time;
  
  ppoint_t arrival_count;
  
} ;

static void *dispatch(void *arg);

static int indexOfLargest(double d[], int n) {
  int large = 0;
  for (int i = 1; i < n; i++) {
    if (d[i] > d[large]) {
      large = i;
    }
  }
  return large;
}

static void start_experiment(uberpool_t *pool) {
  pthread_mutex_lock(&pool->lock);
  pool->start_count = ppoint_start(&pool->arrival_count);
  pool->start_time = util_get_time();
  
  for (int i = 0; i < pool->ndescs; i++) {
    idesc_t *idesc = &pool->descs[i];
    idesc->start_blocked = ppoint_start(&idesc->blocked_time);
    idesc->start_count = ppoint_start(&idesc->count);
  }
  pthread_mutex_unlock(&pool->lock);
}


#define LOG2(X)  // printf X

static void end_experiment(uberpool_t *pool) {
  double d_total = 0;
  double w[STAGE_MAX];
  double D[STAGE_MAX];
  double d[STAGE_MAX];
  
  pthread_mutex_lock(&pool->lock);
  int ndescs = pool->ndescs;
  
  size_t arrivals = ppoint_stop(&pool->arrival_count, pool->start_count);
  size_t interval = util_get_time() - pool->start_time;
  
  double lambda_0 = ((double)arrivals) / interval;
  
  LOG2(("%8s %8s %8s %8s %8s %8s %8s %8s\n", "i", "n_i", "c_i", "p_i", "b_i", "w_i", "D_i", "d[i]"));
  for (int i = 0; i < ndescs; i++) {
    idesc_t *stage = &pool->descs[i];
    int    n_i = __atomic_load_n(&stage->threads, __ATOMIC_RELAXED);
    size_t b_i = ppoint_stop(&stage->blocked_time, stage->start_blocked);
    size_t c_i = ppoint_stop(&stage->count, stage->start_count);
    size_t p_i  = uberqueue_size(stage->in);
    size_t tmp = (max(b_i + 1, n_i * interval) - b_i);
    w[i] =  ((c_i == 0) ? DBL_MAX : ((double)c_i) / tmp)  * 1000 * 1000 * 1000;
    if (i == 0) {
      D[i] = (interval * lambda_0 + p_i);
    } else {
      D[i] = (D[i-1] + p_i);
    }
    d[i] = D[i] / w[i];
    d_total += d[i];
    if (n_i > 0) {
      LOG2(("%8d %8d %8zu %8zu %8zu %8.4g %8.3g %8.3g \n", i, n_i, c_i, p_i, b_i / n_i / 1000 / 1000, w[i], D[i], d[i]));
    } else {
      LOG2(("%8d %8d %8zu %8zu %8s %8.4g %8.3g %8.3g \n", i, n_i, c_i, p_i, "--", w[i], D[i], d[i]));
    }
  }
  LOG2(("\n"));
  
  LOG2(("Work Fractions: "));
  for (int i = 0; i < ndescs; i++) {
    idesc_t *stage = &pool->descs[i];
    stage->work_fraction = d[i] / d_total;
    LOG2((" %8.3g", stage->work_fraction));
  }
  
  LOG2(("\n"));
  fflush(NULL);
  
  pthread_mutex_unlock(&pool->lock);
}

static void allocate_threads(uberpool_t *pool) {
  double work_fraction[STAGE_MAX];
  
  int ndescs = pool->ndescs;
  int n = 0;
  while (n < ndescs) {
    pool->controls[n].role = &pool->descs[n];
    work_fraction[n] = pool->descs[n].work_fraction - 1 / pool->nthreads;
    n++;
    assert(n <= pool->max_threads);
  }
  
  for (int i = 0; i < ndescs; i++) {
    int count = work_fraction[i] * (pool->nthreads - ndescs);
    for (int j = 0; j < count; j++) {
      pool->controls[n++].role = &pool->descs[i];
      assert(n <= pool->max_threads);
    }
  }
  
  while (n < pool->nthreads) {
    int x = indexOfLargest(work_fraction, ndescs);
    pool->controls[n++].role = &pool->descs[x];
    assert(n <= pool->max_threads);
    work_fraction[x] = 0;
  }
  
  while (n < pool->max_threads) {
    pool->controls[n++].role = NULL;
    assert(n <= pool->max_threads);
  }
  
  for(int i = 0; i < pool->max_threads; i++) {
    LOG(("%d", pool->controls[i].role == NULL ? 9 : pool->controls[i].role->id));
  }
  LOG(("\n"));
  
  // printf("# BALANCE for %d threads: ", pool->nthreads);
  for (int i = 0; i < ndescs+1; i++) {
    int n = 0;
    for (int j = 0; j < pool->max_threads; j++) {
      idesc_t *role = pool->controls[j].role;
      if ((role != NULL && role->id == i) || (role == NULL && i == ndescs)) {
        n++;
      }
    }
    printf("%d,", n);
    if (i != ndescs) uberqueue_abort(pool->descs[i].in);
  }
  printf(" # balance \n");
  fflush(NULL);
}

// adjust by ndescs since modes must start at 0 and we can't go lower than ndescs
static int uberpool_get_nthreads(uberpool_t *pool) {
  pthread_mutex_lock(&pool->lock);
  int n = pool->nthreads;
  pthread_mutex_unlock(&pool->lock);
  return n;
}

static int uberpool_set_nthreads(uberpool_t *pool, int n) {
  pthread_mutex_lock(&pool->lock);
  pool->nthreads = n;
  allocate_threads(pool);
  pthread_cond_broadcast(&pool->block);
  pthread_mutex_unlock(&pool->lock);
  return n;
}



knob_t *make_thread_knob(uberpool_t *pool) {
  return make_knob("Thread Pool", pool,
                   pool->ndescs, pool->max_threads,
                   (int (*)(void*))uberpool_get_nthreads,
                   (void (*)(void*,int))uberpool_set_nthreads,
                   NULL,
                   NULL,
                   NULL);
}


void *uberpool_adapt(uberpool_t *pool) {
  LOG(("Launched adapter"));
  util_wait_ms(100);
  while (1) {
    start_experiment(pool);
    util_wait(500 * 1000 * 1000);
    end_experiment(pool);
    pthread_mutex_lock(&pool->lock);
    if (pool->state != POOL_STATE_RUNNING) {
      pthread_mutex_unlock(&pool->lock);
      break;
    }
    allocate_threads(pool);
    pthread_mutex_unlock(&pool->lock);
  }
  return NULL;
}


int items_in_queues(uberpool_t *pool) {
  int result = 0;
  for (int i = 0; i < pool->ndescs; i++) {
    idesc_t *stage = &pool->descs[i];
    result += uberqueue_size(stage->in);
  }
  return result;
}

uberpool_t *uberpool_create(int nthreads, void **thread_data, uberdesc_t *descs, int ndescs, pthread_attr_t *attr) {
  int i;
  uberpool_t *pool;
  int rv;
  
  /* Create data structure */
  pool = (uberpool_t *)malloc(sizeof(uberpool_t));
  if(pool == NULL) {
    return NULL;
  }
  
  ppoint_init(&pool->arrival_count);
  
  pool -> descs = (idesc_t *)malloc(sizeof(idesc_t) * ndescs);
  
  if (pool -> descs == NULL) {
    free(pool);
    return NULL;
  }

  assert(nthreads >= ndescs);
  pool -> nthreads = nthreads;

  for (int i = 0; i < ndescs; i++) {
    idesc_t *idesc = &pool -> descs[i];
    idesc->desc = &descs[i];
    idesc->threads = 0;
    idesc->id = i;
    ppoint_init(&idesc->blocked_time);
    ppoint_init(&idesc->count);
    idesc->work_fraction = ((double)ndescs) / nthreads;
  }
  
  for (int i = 0; i < ndescs; i++) {
    idesc_t *idesc = &pool -> descs[i];
    if (i == 0) {
      idesc->in = (uberqueue_t *)malloc(sizeof(uberqueue_t));
      uberqueue_init(idesc->in, SIZE);
      uberqueue_add_producer(idesc->in);
    } else  {
      idesc->in = pool->descs[i - 1].out;
    }
    idesc->out = (uberqueue_t *)malloc(sizeof(uberqueue_t));
    uberqueue_init(idesc->out, SIZE);
  }
  
  pool->ndescs = ndescs;
  
  pool->threads = (pthread_t *)malloc(sizeof(pthread_t) * nthreads);
  if(pool->threads == NULL) {
    free(pool->descs);
    free(pool);
    return NULL;
  }
  
  pool->controls = (uberthread_t *)malloc(sizeof(uberthread_t) * nthreads);
  if(pool->controls == NULL) {
    free(pool->descs);
    free(pool->threads);
    free(pool);
    return NULL;
  }

  for (int i = 0; i < nthreads; i++) {
    pool -> controls[i].data = (thread_data == NULL ? NULL : thread_data[i]);
  }
  
  pthread_mutex_init(&pool->lock, NULL);
  pthread_cond_init(&pool->block, NULL);
  pthread_barrier_init(&pool->begin_barrier, NULL, nthreads);
  
  /* Create thread and initialize data structures */
  for(i=0; i<ndescs; i++) {
    pool->controls[i].pool = pool;
    pool->controls[i].role = &pool->descs[i];
    pool->controls[i].id = i;
    
    rv = pthread_create(&(pool->threads[i]),
                        attr,
                        dispatch,
                        &(pool->controls[i]));
    assert (rv == 0);
  }
  
  for (int thread = ndescs; thread < nthreads; thread++) {
    int desc = (thread - ndescs) % ndescs;
      pool->controls[thread].pool = pool;
      pool->controls[thread].role = &pool->descs[desc];
      pool->controls[thread].id = thread;
      rv = pthread_create(&(pool->threads[thread]),
                          attr,
                          dispatch,
                          &(pool->controls[thread]));
      assert (rv == 0);
  }
  
  pool->max_threads = pool->nthreads;
  pool->state = POOL_STATE_RUNNING;
  
  pthread_create(&(pool->adapter), NULL,
                 (void *(*)(void *)) uberpool_adapt, pool);
  
  return pool;
}

static int produce(idesc_t *role, void *value) {
  size_t t = uberqueue_enqueue(role->out, value);
  if (t != -1) {
    ppoint_add(&role->blocked_time, t);
  }
  return 0;
}


size_t uberpool_submit(uberpool_t *pool, void *value) {
  ppoint_tick(&pool->arrival_count);
  return uberqueue_enqueue(pool->descs[0].in, value);
}

size_t uberpool_retrieve(uberpool_t *pool, void **value) {
  return uberqueue_dequeue(pool->descs[pool->ndescs-1].out, value);
}


static void *dispatch(void *arg) {
  uberthread_t *info = (uberthread_t *)arg;
  idesc_t *role = info->role;
  uberdesc_t *desc = role->desc;
  uberpool_t *pool = info->pool;
  
  __atomic_add_fetch(&role->threads, 1, __ATOMIC_RELAXED);
  if (role->out != NULL) uberqueue_add_producer(role->out);
  if (desc->begin != NULL) {
    desc->begin(desc->data);
  }
  
  pthread_barrier_wait(&pool->begin_barrier);
  
outer:
  while (1) {
    void *load;
    int result;
    if (role -> in == NULL) {
      load = NULL;
      result = 0;
    } else {
      result = uberqueue_dequeue_can_abort(role->in, (void **)&load);
    }
    
    if (result == -1) {
      // shutting down
      break;
    } else if (result == -2) {
      // time to switch roles
    } else {
      // got valid value
      ppoint_tick(&role->count);
      ppoint_add(&role->blocked_time, result);
      assert (desc->routine(info->data, desc->data, load, role, (produce_f)produce) != -1);
    }
    
    if (role != info->role) {
      if (desc->end != NULL) {
        desc->end(desc->data);
      }
      if (role->out != NULL) uberqueue_remove_producer(role->out);
      __atomic_add_fetch(&role->threads, -1, __ATOMIC_RELAXED);
      
      if (info->role == NULL) {
        pthread_mutex_lock(&pool->lock);
        while (info->role == NULL) {
          pthread_cond_wait(&pool->block, &pool->lock);
          //                    pthread_mutex_unlock(&pool->lock);
          if (pool->state != POOL_STATE_RUNNING) {
            pthread_mutex_unlock(&pool->lock);
            return NULL;
          }
        }
        pthread_mutex_unlock(&pool->lock);
      }
      
      role = info->role;
      desc = role->desc;
      __atomic_add_fetch(&role->threads, 1, __ATOMIC_RELAXED);
      if (desc->begin != NULL) {
        desc->begin(desc->data);
      }
      if (role->out != NULL) uberqueue_add_producer(role->out);
    }
  }
  
  if (desc->end != NULL) {
    desc->end(desc->data);
  }
  if (role->out != NULL) uberqueue_remove_producer(role->out);
  __atomic_add_fetch(&role->threads, -1, __ATOMIC_RELAXED);
  
  return NULL;
}


void uberpool_destroy(uberpool_t *pool) {
  assert(pool!=NULL);
  pthread_mutex_lock(&pool->lock);
  assert(pool->state==POOL_STATE_DONE);
  
  free(pool->controls);
  free(pool->descs);
  free(pool->threads);
  
  pthread_mutex_unlock(&pool->lock);
  pthread_mutex_destroy(&pool->lock);
  pthread_cond_destroy(&pool->block);
  free(pool);
  
}

// result must be large enough to hold pool->nthreads 
// results for the n threads
int uberpool_join(uberpool_t *pool, void *result[]) {
  int i;
  int rv;
  
  assert(pool!=NULL);
  pthread_mutex_lock(&pool->lock);
  assert(pool->state==POOL_STATE_RUNNING);
  pool->state = POOL_STATE_ENDING;
  pthread_cond_broadcast(&pool->block);
  pthread_mutex_unlock(&pool->lock);
  
  uberqueue_remove_producer(pool->descs[0].in);
  
  /* Join threads */
  for(i=0; i<pool->nthreads; i++) {
    rv = pthread_join(pool->threads[i], result == NULL ? NULL : &result[i]);
    if(rv != 0) {
      pool->state = POOL_STATE_ERROR;
      return -1;
    }
  }
  
  pool->state = POOL_STATE_DONE;
  
  pthread_cancel(pool->adapter);
  assert(pthread_join(pool->adapter, NULL) == 0);
  
  return 0;
}

int uberpool_cancel(uberpool_t *pool) {
  int i;
  int rv;
  
  assert(pool!=NULL);
  pthread_mutex_lock(&pool->lock);
  assert(pool->state==POOL_STATE_RUNNING);
  
  rv = 0;
  for(i=0; i<pool->nthreads; i++) {
    rv += pthread_cancel(pool->threads[i]);
  }
  
  if(rv != 0) {
    pool->state = POOL_STATE_ERROR;
    return -1;
  }
  
  //  pool->state = POOL_STATE_READY;
  pthread_mutex_unlock(&pool->lock);
  return 0;
}





#if 0

static void rebalance(uberpool_t *pool, size_t interval, size_t arrivals, int threads_to_use) {
  double d_total = 0;
  double w[STAGE_MAX];
  double D[STAGE_MAX];
  double d[STAGE_MAX];
  
  
  pthread_mutex_lock(&pool->lock);
  int ndescs = pool->ndescs;
  
  assert(ndescs <= threads_to_use && threads_to_use <= pool->nthreads);
  
  double lambda_0 = ((double)arrivals) / interval;
  
  LOG(("%8s %8s %8s %8s %8s %8s %8s %8s\n", "i", "n_i", "c_i", "p_i", "b_i", "w_i", "D_i", "d[i]"));
  for (int i = 0; i < ndescs; i++) {
    idesc_t *stage = &pool->descs[i];
    int    n_i = __atomic_load_n(&stage->threads, __ATOMIC_RELAXED);
    size_t b_i = ppoint_stop(&stage->blocked_time, stage->start_blocked);
    size_t c_i = ppoint_stop(&stage->count, stage->start_count);
    size_t p_i  = uberqueue_size(stage->in);
    size_t tmp = (max(b_i + 1, n_i * interval) - b_i);
    w[i] =  ((c_i == 0) ? DBL_MAX : ((double)c_i) / tmp)  * 1000 * 1000 * 1000;
    if (i == 0) {
      D[i] = (interval * lambda_0 + p_i);
    } else {
      D[i] = (D[i-1] + p_i);
    }
    d[i] = D[i] / w[i];
    d_total += d[i];
    if (n_i > 0) {
      LOG(("%8d %8d %8zu %8zu %8zu %8.4g %8.3g %8.3g \n", i, n_i, c_i, p_i, b_i / n_i / 1000 / 1000, w[i], D[i], d[i]));
    } else {
      LOG(("%8d %8d %8zu %8zu %8s %8.4g %8.3g %8.3g \n", i, n_i, c_i, p_i, "--", w[i], D[i], d[i]));
    }
  }
  
  int n = 0;
  while (n < ndescs) {
    d[n] -= 1;
    pool->controls[n].role = &pool->descs[n];
    n++;
  }
  d_total -= ndescs;
  
  int threads_to_allocate = pool->nthreads - ndescs;
  for (int i = 0; i < ndescs; i++) {
    d[i] = threads_to_allocate * d[i] / d_total;
    //    LOG(("%d %g\n", i, d[i]);
    int count = (int)d[i];
    d[i] = d[i] - count;
    for (int j = 0; j < count; j++) {
      pool->controls[n++].role = &pool->descs[i];
      
    }
  }
  
  while (n < pool->nthreads) {
    int x = indexOfLargest(d, ndescs);
    pool->controls[n++].role = &pool->descs[x];
    d[x] = 0;
  }
  
  while (n < pool->max_threads) {
    pool->controls[n++].role = NULL;
  }
  
  LOG2(("BALANCE"));
  for (int i = 0; i < ndescs; i++) {
    int n = 0;
    for (int j = 0; j < pool->nthreads; j++) {
      idesc_t *role = pool->controls[j].role;
      if (role != NULL && role->id == i) {
        n++;
      }
    }
    LOG2(("\t%d", n));
    uberqueue_abort(pool->descs[i].in);
  }
  LOG2(("\n"));
  fflush(NULL);
  
  pthread_mutex_unlock(&pool->lock);
}

#endif
