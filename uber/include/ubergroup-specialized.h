/*
 * uberlock.h
 *
 *  Created on: Jan 10, 2017
 *      Author: freund
 */

#ifndef UBERGROUP_SPECIALIZED_H_
#define UBERGROUP_SPECIALIZED_H_

#include <pthread.h>
#include <assert.h>

#include <pthread.h>
#include "uberlock.h"
#include "ubergroup.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"
#include "ppoint.h"
#include <string.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <unistd.h>

#ifdef __cplusplus
#define CPP_START extern "C" {
#define CPP_END }
#else
#define CPP_START
#define CPP_END
#endif

#define DECLARE_SPECIALIZED_LOCK(T, L1, L2)				\
  CPP_START								\
									\
  typedef struct T##_group_a T##_group_t;				\
  typedef struct T##_lock_a T##_lock_t;					\
									\
  typedef struct T##_lock_a {						\
    L1 l1    __attribute__((aligned(64)));				\
    L2 l2    __attribute__((aligned(64)));				\
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
  extern T##_group_t T##_group;						\
									\
  int T##_group_get_mode(T##_group_t *group);				\
  int T##_group_set_mode(T##_group_t *group, int mode);			\
  knob_t * T##_group_knob(T##_group_t *g);				\
									\
  const char *T##_group_name_for_mode(T##_group_t *group, int mode);	\
									\
  int T##_init(T##_lock_t* o);						\
  int T##_destroy(T##_lock_t* o);					\
  int T##_rdlock(T##_lock_t* o);					\
  int T##_wrlock(T##_lock_t* o);					\
  int T##_unlock(T##_lock_t* o);					\
									\
  CPP_END


#define DEFINE_SPECIALIZED_LOCK(T, L1, S1, init1, destroy1, wr1, rd1, un1, L2, S2, init2, destroy2, wr2, rd2, un2) \
  DECLARE_SPECIALIZED_LOCK(T, L1, L2)					\
  CPP_START								\
									\
  EXTERN3(T##_rdlock_top, T##_rdlock_mutex, T##_rdlock_rw);		\
  EXTERN3(T##_wrlock_top, T##_wrlock_mutex, T##_wrlock_rw);		\
  EXTERN3(T##_unlock_top, T##_unlock_mutex, T##_unlock_rw);		\
									\
  __attribute__((optimize("align-functions=1024")))			\
  int T##_unlock(T##_lock_t *t) {					\
    JMP(T##_unlock_top, T##_unlock_mutex, T##_unlock_rw);		\
    LABEL(T##_unlock_mutex);						\
    un1(&t->l1);							\
    return 0;								\
    LABEL(T##_unlock_rw);						\
    un2(&t->l2);							\
    return 0;								\
  }									\
									\
  int T##_rdlock(T##_lock_t *t) {					\
    JMP(T##_rdlock_top, T##_rdlock_mutex, T##_rdlock_rw);		\
    LABEL(T##_rdlock_mutex);						\
    rd1(&t->l1);							\
    return 0;								\
    LABEL(T##_rdlock_rw);						\
    rd2(&t->l2);							\
    return 0;								\
  }									\
									\
  __attribute__((noinline))						\
  int T##_wrlock(T##_lock_t *t) {					\
    JMP(T##_wrlock_top, T##_wrlock_mutex, T##_wrlock_rw);		\
    LABEL(T##_wrlock_mutex);						\
    wr1(&t->l1);							\
    return 0;								\
    LABEL(T##_wrlock_rw);						\
    wr2(&t->l2);							\
    return 0;								\
  }									\
									\
  static void T##_patch(int mode) {					\
    static int init = 0;						\
    if (init == 0) {							\
      set_permissions(&T##_rdlock_top, 1);				\
      set_permissions(&T##_wrlock_top, 1);				\
      set_permissions(&T##_unlock_top, 1);				\
      init = 1;								\
    }									\
									\
    PATCH(mode, &T##_rdlock_top, &T##_rdlock_mutex, &T##_rdlock_rw);	\
    PATCH(mode, &T##_wrlock_top, &T##_wrlock_mutex, &T##_wrlock_rw);	\
    PATCH(mode, &T##_unlock_top, &T##_unlock_mutex, &T##_unlock_rw);	\
  }									\
  int T##_init(T##_lock_t* o) {						\
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
    pthread_mutex_lock(&(T##_group.mutex));				\
									\
    o->next = T##_group.locks;						\
    if (o->next != NULL) {						\
      o->next->prev = o;						\
    } else {								\
      o->next = NULL;							\
    }									\
    T##_group.locks = o;						\
    pthread_mutex_unlock(&(T##_group.mutex));				\
    return 0;								\
  }									\
									\
  int T##_destroy(T##_lock_t* o) {					\
    pthread_mutex_lock(&(T##_group.mutex));				\
    if (o->prev != NULL) {						\
      o->prev->next = o->next;						\
    } else {								\
      T##_group.locks = o->next;					\
    }									\
    if (o->next != NULL) {						\
      o->next->prev = o->prev;						\
    }									\
    pthread_mutex_unlock(&(T##_group.mutex));				\
									\
    int result = destroy1(&(o->l1));					\
    if (result != 0) return result;					\
    result = destroy2(&(o->l2));					\
    return result;							\
  }									\
									\
  int T##_group_get_mode(T##_group_t *group) {				\
    assert(&T##_group == group);					\
    pthread_mutex_lock(&(T##_group.mutex));				\
    int x = T##_group.mode;						\
    pthread_mutex_unlock(&(T##_group.mutex));				\
    return x;								\
  }									\
									\
									\
  /* No threads actively operating on locks */				\
  int T##_group_set_mode(T##_group_t *group, int mode) {		\
    struct timeval t1, t2;						\
    double elapsedTime;							\
    gettimeofday(&t1, NULL);						\
    assert(&T##_group == group);					\
    pthread_mutex_lock(&(T##_group.mutex));				\
    T##_patch(mode);							\
    T##_group.mode = mode;						\
    pthread_mutex_unlock(&(T##_group.mutex));				\
    gettimeofday(&t2, NULL);						\
    elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;			\
    elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;			\
    /* printf("MODE %g\n", elapsedTime);			*/		\
    return 0;								\
  }									\
									\
  const char *T##_group_name_for_mode(T##_group_t *group, int mode) {	\
    assert(&T##_group == group);					\
    return mode == 0 ? S1 : S2;						\
  }									\
									\
									\
  int T##_tryrdlock(T##_lock_t* o) {					\
    return 0;								\
  }									\
									\
  int T##_trywrlock(T##_lock_t* o) {					\
    return 0;								\
  }									\
									\
  T##_group_t T##_group __attribute__ ((__aligned__(64))) = {		\
    PTHREAD_MUTEX_INITIALIZER, NULL, 0, 0,  };				\
									\
  knob_t * T##_group_knob(T##_group_t *g) {				\
    return make_knob(#T, \
        g,						\
        0, \
        1, \
		    (int (*)(void*))T##_group_get_mode,		\
        (void (*)(void*,int))T##_group_set_mode,		\
        (char *(*)(void*,int))T##_group_name_for_mode, \
        NULL, \
        NULL); \
  }									\
									\
  CPP_END



#endif
