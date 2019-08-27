/*
 * uberlock.h
 *
 *  Created on: Jan 10, 2017
 *      Author: freund
 */

#ifndef UBERGROUP_GENERIC_H_
#define UBERGROUP_GENERIC_H_

#include <pthread.h>
#include "ppoint.h"
#include <assert.h>

#include <pthread.h>
#include "uberlock.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"
#include "ppoint.h"
#include <string.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <unistd.h>
#include "uknob.h"

#ifdef __cplusplus
#define CPP_START extern "C" {
#define CPP_END }
#else
#define CPP_START
#define CPP_END 
#endif

#define DECLARE_COMBINED_LOCK(T, L1, L2)				\
  CPP_START								\
									\
  typedef struct T##_group_a T##_group_t;				\
  typedef struct T##_lock_a T##_lock_t;					\
									\
  typedef struct T##_lock_a {						\
    L1 l1    __attribute__((aligned(64)));				\
    L2 l2    __attribute__((aligned(64)));				\
    T##_group_t *group;							\
    struct T##_lock_a *prev, *next;					\
  } T##_lock_t;								\
									\
									\
  typedef struct T##_group_a {						\
    pthread_mutex_t mutex;						\
    T##_lock_t *locks;							\
    int id;								\
    int mode;								\
									\
  } T##_group_t;							\
									\
									\
  int T##_group_init(T##_group_t *g);					\
  int T##_group_get_mode(T##_group_t* o);				\
  int T##_group_set_mode(T##_group_t* o, int mode);			\
  int T##_group_destroy(T##_group_t *g);				\
  knob_t * T##_group_knob(T##_group_t *g);          \
									\
  char *T##_group_name_for_mode(T##_group_t *g, int mode); \
									\
  T##_group_t *T##_group_default();					\
									\
  int T##_lock_init(T##_lock_t* o, T##_group_t *g);			\
  int T##_lock_destroy(T##_lock_t* o);					\
  int T##_lock_rdlock(T##_lock_t* o);					\
  int T##_lock_wrlock(T##_lock_t* o);					\
  int T##_lock_unlock(T##_lock_t* o);					\
									\
  CPP_END


#define DEFINE_COMBINED_LOCK(T, L1, S1, init1, destroy1, wr1, rd1, un1, L2, S2, init2, destroy2, wr2, rd2, un2) \
  DECLARE_COMBINED_LOCK(T,L1,L2)					\
  CPP_START								\
  int T##_lock_init(T##_lock_t* o, T##_group_t *group) {		\
    o->group = group;							\
    o->prev = NULL;							\
									\
    int err = init1(&(o->l1), NULL);					\
    if (err != 0) return err;						\
    {									\
									\
      err = init2(&(o->l2), NULL);					\
      if (err != 0) return err;						\
    }									\
									\
    pthread_mutex_lock(&(o->group->mutex));				\
									\
    o->next = o->group->locks;						\
    if (o->next != NULL) {						\
      o->next->prev = o;						\
    } else {								\
      o->next = NULL;							\
    }									\
    o->group->locks = o;						\
    pthread_mutex_unlock(&(o->group->mutex));				\
    return 0;								\
  }									\
									\
  int T##_lock_destroy(T##_lock_t* o) {					\
    pthread_mutex_lock(&(o->group->mutex));				\
    if (o->prev != NULL) {						\
      o->prev->next = o->next;						\
    } else {								\
      o->group->locks = o->next;					\
    }									\
    if (o->next != NULL) {						\
      o->next->prev = o->prev;						\
    }									\
    pthread_mutex_unlock(&(o->group->mutex));				\
									\
    int result = destroy1(&(o->l1));					\
    if (result != 0) return result;					\
    result = destroy2(&(o->l2));					\
    return result;							\
  }									\
									\
  int T##_lock_rdlock(T##_lock_t* o) {					\
    T##_group_t *g = o->group;						\
  top:									\
    if (g->mode == 0) {							\
      rd1(&o->l1);							\
      if (g->mode != 0) {						\
	un1(&o->l1);							\
	goto top;							\
      }									\
    } else {								\
      rd2(&o->l2);							\
      if (g->mode == 0) {						\
	un2(&o->l2);							\
	goto top;							\
      }									\
    }									\
    return 0;								\
  }									\
									\
  int T##_lock_wrlock(T##_lock_t* o)  {					\
    T##_group_t *g = o->group;						\
  top:									\
    if (g->mode == 0) {							\
      wr1(&o->l1);							\
      if (g->mode != 0) {						\
	un1(&o->l1);							\
	goto top;							\
      }									\
    } else {								\
      wr2(&o->l2);							\
      if (g->mode == 0) {						\
	un2(&o->l2);							\
	goto top;							\
      }									\
    }									\
    return 0;								\
  }									\
									\
  int T##_lock_unlock(T##_lock_t* o) {					\
    if (o->group->mode == 0) {						\
      un1(&o->l1);							\
    } else {								\
      un2(&o->l2);							\
    }									\
    return 0;								\
  }									\
									\
  static T##_group_t T##_group_default_group __attribute__ ((__aligned__(64)));	\
									\
  static int T##_group_init_id(T##_group_t *g, int id) {		\
    g->locks = NULL;							\
    g->id = id;								\
    g->mode = MUTEX_MODE;						\
    pthread_mutex_init(&g->mutex, NULL);					\
    return 0;								\
  }									\
									\
									\
  int T##_group_init(T##_group_t *g) {					\
    static int group_id_counter = 1;					\
    return T##_group_init_id(g, group_id_counter++);			\
  }									\
									\
									\
									\
  int T##_group_destroy(T##_group_t *g) {				\
    return pthread_mutex_destroy(&(g->mutex));				\
  }									\
									\
  int T##_group_get_mode(T##_group_t* g) {				\
    pthread_mutex_lock(&(g->mutex));					\
    int x = g->mode;							\
    pthread_mutex_unlock(&(g->mutex));					\
    return x;								\
  }									\
									\
  int T##_group_set_mode(T##_group_t* g, int mode) {			\
    struct timeval t1, t2;						\
    double elapsedTime;							\
    gettimeofday(&t1, NULL);						\
    pthread_mutex_lock(&(g->mutex));					\
    /* This can Deadlock if client acquires  multiple locks ... */	\
    for (T##_lock_t *l = g->locks; l != NULL; l = l->next) {		\
      wr1(&l->l1);							\
      wr2(&l->l2);							\
    }									\
    g->mode = mode;							\
    for (T##_lock_t *l = g->locks; l != NULL; l = l->next) {		\
      un1(&l->l1);							\
      un2(&l->l2);							\
    }									\
    pthread_mutex_unlock(&(g->mutex));					\
    gettimeofday(&t2, NULL);						\
    elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;			\
    elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;			\
    /* printf("MODE %g\n", elapsedTime);			*/		\
    return 0;								\
  }									\
									\
  char *T##_group_name_for_mode(T##_group_t *g, int mode) { \
    return mode == 0 ? S1 : S2;        \
  }									\
									\
									\
  int T##_lock_tryrdlock(T##_lock_t* o) {				\
    return 0;								\
  }									\
									\
  int T##_lock_trywrlock(T##_lock_t* o) {				\
    return 0;								\
  }									\
									\
  T##_group_t *T##_group_default() {						\
    static int initialized = 0;						\
    if (!initialized) {							\
      T##_group_init_id(&T##_group_default_group, 0);			\
      char *mode_env = getenv("UBER_DEFAULT_MODE");			\
      int mode = (mode_env == NULL) ? 0 : atoi(mode_env);		\
      T##_group_set_mode(&T##_group_default_group, mode);		\
      initialized = 1;							\
    }									\
    return &T##_group_default_group;					\
  }									\
									\
  knob_t * T##_group_knob(T##_group_t *g) {          \
    return make_knob(#T, g, 0, 1,\
                    (int (*)(void*))T##_group_get_mode, \
                    (void (*)(void*,int))T##_group_set_mode,\
                    (char *(*)(void*,int))T##_group_name_for_mode, NULL, NULL); \
  }\
  CPP_END



#endif
