#include "uberqueue.h"

#include <pthread.h>
#include <stdlib.h>
#include "util.h"
#include <assert.h>

void uberqueue_init(uberqueue_t * que, int size) {
  pthread_mutex_init(&que->mutex, NULL);
  pthread_cond_init(&que->empty, NULL);
  pthread_cond_init(&que->full, NULL);
  que->head = que->tail = 0;
  que->data = (void **)malloc(sizeof(void*) * size);
  que->size = size;
  que->prod_threads = 0;
}

void uberqueue_destroy(uberqueue_t* que) {
  pthread_mutex_destroy(&que->mutex);
  pthread_cond_destroy(&que->empty);
  pthread_cond_destroy(&que->full);
  free(que->data);
  que->data = NULL;
}

void uberqueue_remove_producer(uberqueue_t * que) {
  pthread_mutex_lock(&que->mutex);
  que->prod_threads--;
  pthread_cond_broadcast(&que->empty);
  pthread_mutex_unlock(&que->mutex);
}

void uberqueue_add_producer(uberqueue_t * que) {
  pthread_mutex_lock(&que->mutex);
  que->prod_threads++;
  pthread_cond_broadcast(&que->empty);
  pthread_mutex_unlock(&que->mutex);
}

int uberqueue_dequeue(uberqueue_t * que, void **to_buf) {
  int wait_nanos = 0;
  pthread_mutex_lock(&que->mutex);
  
  // chceck if queue is empty?
  while (que->tail == que->head && (que->prod_threads > 0)) {
    size_t start = util_get_time();
    pthread_cond_wait(&que->empty, &que->mutex);
    size_t stop = util_get_time();
    wait_nanos += (int)(stop - start);
   // assert(wait_nanos >= 0);
  }
  // check if queue has been terminated?
  if (que->tail == que->head && (que->prod_threads == 0)) {
    pthread_cond_broadcast(&que->empty);
    pthread_mutex_unlock(&que->mutex);
    return -1;
  }
  
  *to_buf = que->data[que->tail];
  que->tail ++;
  if (que->tail == que->size)
    que->tail = 0;
  pthread_cond_signal(&que->full);
  pthread_mutex_unlock(&que->mutex);
  return wait_nanos;
}



void uberqueue_abort(struct uberqueue * que) {
  pthread_mutex_lock(&que->mutex);
  pthread_cond_broadcast(&que->empty);
  pthread_mutex_unlock(&que->mutex);
}

int uberqueue_dequeue_can_abort(uberqueue_t * que, void **to_buf) {
  int wait_nanos = 0;
  pthread_mutex_lock(&que->mutex);
  
  // chceck if queue is empty?
  if (que->tail == que->head && (que->prod_threads > 0)) {
    size_t start = util_get_time();
    pthread_cond_wait(&que->empty, &que->mutex);
    size_t stop = util_get_time();
    wait_nanos += (int)(stop - start);
    // assert(wait_nanos >= 0);

    if (que->tail == que->head && que->prod_threads > 0) {
      pthread_cond_broadcast(&que->empty);
      pthread_mutex_unlock(&que->mutex);
      return -2;
    }
  }

  // check if queue has been terminated?
  if (que->tail == que->head && que->prod_threads == 0) {
    pthread_cond_broadcast(&que->empty);
    pthread_mutex_unlock(&que->mutex);
    return -1;
  }
  
  *to_buf = que->data[que->tail];
  que->tail ++;
  if (que->tail == que->size)
    que->tail = 0;
  pthread_cond_signal(&que->full);
  pthread_mutex_unlock(&que->mutex);
  return wait_nanos;
}




int uberqueue_enqueue(uberqueue_t * que, void *from_buf) {
  int wait_nanos = 0;
  
  pthread_mutex_lock(&que->mutex);
  while (que->head == (que->tail-1+que->size)%que->size) {
    size_t start = util_get_time();
    pthread_cond_wait(&que->full, &que->mutex);
    size_t stop = util_get_time();
    wait_nanos += (int)(stop - start);
    assert(wait_nanos >= 0);
  }
  
  que->data[que->head] = from_buf;
  que->head ++;
  if (que->head == que->size)
    que->head = 0;
  
  pthread_cond_signal(&que->empty);
  pthread_mutex_unlock(&que->mutex);
  return wait_nanos;
}

size_t uberqueue_size(uberqueue_t * que) {
  pthread_mutex_lock(&que->mutex);
  int size =  (que->head + que->size - que->tail) % que->size;
  pthread_mutex_unlock(&que->mutex);
  return size;
}
