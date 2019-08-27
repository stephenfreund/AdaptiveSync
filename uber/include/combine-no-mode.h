/*
 * uberlock.h
 *
 *  Created on: Jan 10, 2017
 *      Author: freund
 */

#ifndef COMBINE_H_
#define COMBINE_H_

#include <pthread.h>
#include "ppoint.h"
#include "patch.h"
#include "probe.h"
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
    uberprobe_t *probe;							\
									\
    void (*patch)(int);							\
									\
  } T##_group_t;							\
									\
									\
  int T##_group_init(T##_group_t *g);					\
  int T##_group_init_special(T##_group_t *g, void (*patch)(int));	\
									\
  int T##_group_get_mode(T##_group_t* o);				\
  int T##_group_set_mode(T##_group_t* o, int mode, int at_safe_point);	\
									\
  int T##_group_destroy(T##_group_t *g);				\
									\
  const char *T##_group_name_for_mode(T##_group_t *g, int mode, char *name); \
									\
  T##_group_t *T##_group_default();					\
									\
									\
  int T##_lock_init(T##_lock_t* o, T##_group_t *g);			\
  int T##_lock_destroy(T##_lock_t* o);					\
  int T##_lock_rdlock(T##_lock_t* o);					\
  int T##_lock_wrlock(T##_lock_t* o);					\
  int T##_lock_unlock(T##_lock_t* o);					\
									\
  CPP_END

#ifndef __APPLE__
#define DEFINE_COMBINED_GROUP(G, T, L1, S1, init1, destroy1, wr1, rd1, un1, L2, S2, init2, destroy2, wr2, rd2, un2) \
									\
  ubergroup_t T ## __group_ ## G;					\
									\
  EXTERN3(rdlock_top_ ## G, rdlock_mutex_ ## G, rdlock_rw_ ## G);	\
  EXTERN3(wrlock_top_ ## G, wrlock_mutex_ ## G, wrlock_rw_ ## G);	\
  EXTERN3(unlock_top_ ## G, unlock_mutex_ ## G, unlock_rw_ ## G); 	\
									\
  __attribute__((optimize("align-functions=1024")))			\
  int T ## _lock_unlock_ ## G(T ## _lock_t *t) {			\
    JMP(unlock_top_ ## G, unlock_mutex_ ## G, unlock_rw_ ## G);		\
    LABEL(unlock_mutex_ ## G);						\
    un1(&t->l1);							\
    return 0;								\
    LABEL(unlock_rw_ ## G);						\
    un2(&t->l2);							\
    return 0;								\
  }									\
									\
  int T ## _lock_rdlock_ ## G(T ## _lock_t *t) {			\
    JMP(rdlock_top_ ## G, rdlock_mutex_ ## G, rdlock_rw_ ## G);		\
    LABEL(rdlock_mutex_ ## G);						\
    rd1(&t->l1);							\
    return 0;								\
    LABEL(rdlock_rw_ ## G);						\
    rd2(&t->l2);							\
    return 0;								\
  }									\
									\
									\
									\
  __attribute__((noinline))						\
  int T ## _lock_wrlock_ ## G(T ## _lock_t *t) {			\
    JMP(wrlock_top_ ## G, wrlock_mutex_ ## G, wrlock_rw_ ## G);		\
    LABEL(wrlock_mutex_ ## G);						\
    wr1(&t->l1);							\
    return 0;								\
    LABEL(wrlock_rw_ ## G);						\
    wr2(&t->l2);							\
    return 0;								\
  }									\
									\
									\
									\
  static void T ## __patch_ ## G(int mode) {				\
    static int init = 0;						\
    if (init == 0) {							\
      set_permissions(&rdlock_top_ ## G, 1);				\
      set_permissions(&wrlock_top_ ## G, 1);				\
      set_permissions(&unlock_top_ ## G, 1);				\
      init = 1;								\
    }									\
									\
    PATCH(mode, &rdlock_top_ ## G, &rdlock_mutex_ ## G, &rdlock_rw_ ## G); \
    PATCH(mode, &wrlock_top_ ## G, &wrlock_mutex_ ## G, &wrlock_rw_ ## G); \
    PATCH(mode, &unlock_top_ ## G, &unlock_mutex_ ## G, &unlock_rw_ ## G); \
  }									\

#define NUM_PATCHES 3

#define COMBINED_GROUP(G, T) T ## __group_ ## G

#define INIT_COMBINED_GROUP(T)						\
  int ubergroup_init_special(&T ## __group_ ## G, T ## __patch_ ## G);
#else
#define DEFINE_COMBINED_GROUP(G, T, L1, S1, init1, destroy1, wr1, rd1, un1, L2, S2, init2, destroy2, wr2, rd2, un2) \
									\
  ubergroup_t T ## __group_ ## G;					\
									\
  int T ## _lock_rdlock_ ## G(T ## _lock_t *t) { return T ## _lock_rdlock(t); } \
  int T ## _lock_wrlock_ ## G(T ## _lock_t *t) { return T ## _lock_wrlock(t); } \
  int T ## _lock_unlock_ ## G(T ## _lock_t *t) { return T ## _lock_unlock(t); } \
									\

#define COMBINED_GROUP(G, T) T ## __group_ ## G


#define INIT_COMBINED_GROUP(T)					\
  int ubergroup_init_special(&T ## __group_ ## G, NULL);
#endif




#ifdef __cplusplus
#define EXTERN_COMBINED_GROUP(G, T)			\
  extern "C" {						\
    extern ubergroup_t T ## __group_ ## G;		\
    int T ## _lock_rdlock_ ## G(T ## _lock_t *t);	\
    int T ## _lock_wrlock_ ## G(T ## _lock_t *t);	\
    int T ## _lock_unlock_ ## G(T ## _lock_t *t);	\
  } 
#else
#define EXTERN_COMBINED_GROUP(G, T)		\
  extern ubergroup_t T ## __group_ ## G;	\
  int T ## _lock_rdlock_ ## G(T ## _lock_t *t);	\
  int T ## _lock_wrlock_ ## G(T ## _lock_t *t);	\
  int T ## _lock_unlock_ ## G(T ## _lock_t *t);    
#endif



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
  static int T##_group_init_id(T##_group_t *g, int id, void (*patch)(int)) { \
    g->locks = NULL;							\
    g->id = id;								\
    g->mode = MUTEX_MODE;						\
    g->patch = patch;							\
    if (g->patch != NULL) {						\
      g->patch(MUTEX_MODE);						\
    }									\
    pthread_mutex_init(&g->mutex, NULL);				\
    return 0;								\
  }									\
									\
									\
  int T##_group_init(T##_group_t *g) {					\
    return T##_group_init_special(g, NULL);				\
  }									\
									\
  int T##_group_init_special(T##_group_t *g, void (*patch)(int)) {	\
									\
    static int group_id_counter = 1;					\
    return T##_group_init_id(g, group_id_counter++, patch);		\
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
  int T##_group_set_mode(T##_group_t* g, int mode, int at_safe_point) {	\
    struct timeval t1, t2;						\
    double elapsedTime;							\
    gettimeofday(&t1, NULL);						\
    pthread_mutex_lock(&(g->mutex));					\
    if (g->patch != NULL) {						\
      assert(at_safe_point);						\
      g->patch(mode);							\
      g->mode = mode;							\
    } else {								\
      if (!at_safe_point) {						\
	for (T##_lock_t *l = g->locks; l != NULL; l = l->next) {	\
	  printf("0\n");						\
	  wr1(&l->l1);							\
	  printf("1\n");						\
	  wr2(&l->l2);							\
	  printf("2\n");						\
	}								\
      }									\
      g->patch(mode);							\
      g->mode = mode;							\
      if (!at_safe_point) {						\
	for (T##_lock_t *l = g->locks; l != NULL; l = l->next) {	\
	  printf("2\n");						\
	  un1(&l->l1);							\
	  printf("1\n");						\
	  un2(&l->l2);							\
	  printf("0\n");						\
	}								\
      }									\
    }									\
    pthread_mutex_unlock(&(g->mutex));					\
    gettimeofday(&t2, NULL);						\
    elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;			\
    elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;			\
    printf("MODE %g\n", elapsedTime);					\
    return 0;								\
  }									\
									\
  const char *T##_group_name_for_mode(T##_group_t *g, int mode, char *name) { \
    return strcpy(name, mode == 0 ? S1 : S2);				\
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
  DEFINE_COMBINED_GROUP(DEFAULT,T, L1, S1, init1, destroy1, wr1, rd1, un1, L2, S2, init2, destroy2, wr2, rd2, un2); \
									\
  T##_group_t *T##_group_default() {					\
    static int initialized = 0;						\
    if (!initialized) {							\
      T##_group_init_id(&T##_group_default_group, 0, T ## __patch_DEFAULT); \
      char *mode_env = getenv("UBER_DEFAULT_MODE");			\
      int mode = (mode_env == NULL) ? 0 : atoi(mode_env);		\
      T##_group_set_mode(&T##_group_default_group, mode, 1);		\
      initialized = 1;							\
    }									\
    return &T##_group_default_group;					\
  }									\
									\
									\
  CPP_END



#endif /* UBERLOCK_H_ */
