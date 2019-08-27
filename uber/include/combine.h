///*
// * uberlock.h
// *
// *  Created on: Jan 10, 2017
// *      Author: freund
// */
//
//#ifndef COMBINE_H_
//#define COMBINE_H_
//
//#include <bits/pthreadtypes.h>
//#include <pthread.h>
//#include "ppoint.h"
//#include "patch.h"
//#include "probe.h"
//
//#include <pthread.h>
//#include "uberlock.h"
//#include "ubergroup.h"
//
//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//#include "util.h"
//#include "ppoint.h"
//#include <string.h>
//#include <sys/mman.h>
//#include <unistd.h>
//
//#ifdef __cplusplus
//   #define CPP_START extern "C" {
//   #define CPP_END }
//#else
//   #define CPP_START
//   #define CPP_END 
//#endif
//
//#define DECLARE_COMBINED_LOCK(T, L1, L2)    \
//  CPP_START \
//    \
//typedef struct T##_group_a T##_group_t;    \
//typedef struct T##_lock_a T##_lock_t;    \
//    \
//typedef struct T##_lock_a {    \
//  L1 l1    __attribute__((aligned(64)));        \
//  L2 l2    __attribute__((aligned(64)));        \
//  int mode __attribute__((aligned(64)));      \
//  T##_group_t *group;    \
//  struct T##_lock_a *prev, *next;    \
//} T##_lock_t;    \
//    \
//    \
//typedef struct T##_group_a {    \
//  pthread_mutex_t mutex;    \
//  T##_lock_t *locks;    \
//  int id;    \
//  int mode;    \
//      \
//  uberprobe_t *probe;    \
//      \
//  pthread_t adapter;    \
//  size_t exp_freq; /* 0 for no adapt */    \
//  size_t exp_time;    \
//  size_t initial_delay;  \
//    \
//  int experiments;    \
//  int results[2];    \
//      \
//  void (*patch)(int);    \
//      \
//} T##_group_t;    \
//    \
//    \
//int T##_group_init(T##_group_t *g, uberprobe_t *probe, size_t exp_freq, size_t exp_time);    \
//int T##_group_init_special(T##_group_t *g, uberprobe_t *probe, size_t exp_freq,    \
//         size_t exp_time, void (*patch)(int));    \
//    \
//size_t T##_group_get_exp_freq(T##_group_t *group);    \
//void T##_group_set_exp_freq(T##_group_t *group, size_t exp_freq);    \
//    \
//size_t T##_group_get_exp_time(T##_group_t *group);    \
//void T##_group_set_exp_time(T##_group_t *group, size_t exp_time);    \
//    \
//int T##_group_get_mode(T##_group_t* o);    \
//int T##_group_set_mode(T##_group_t* o, int mode);    \
//    \
//size_t T##_group_get_initial_adapt_delay(T##_group_t* o);    \
//void T##_group_set_initial_adapt_delay(T##_group_t* o, size_t init_delay);    \
//    \
//int T##_group_destroy(T##_group_t *g);    \
//    \
//int T##_group_get_wins(T##_group_t *g, int mode);    \
//int T##_group_get_preferred_mode(T##_group_t *g);    \
//const char *T##_group_name_for_mode(int mode);    \
//      \
//T##_group_t *T##_group_default();    \
//    \
//    \
//int T##_lock_init(T##_lock_t* o, T##_group_t *g);    \
//int T##_lock_destroy(T##_lock_t* o);    \
//int T##_lock_setmode(T##_lock_t* o, int mode);    \
//int T##_lock_rdlock(T##_lock_t* o);    \
//int T##_lock_wrlock(T##_lock_t* o);    \
//int T##_lock_unlock(T##_lock_t* o);    \
//  CPP_END\
//
//#define DEFINE_COMBINED_GROUP(G, T, L1, S1, init1, destroy1, wr1, rd1, un1, L2, S2, init2, destroy2, wr2, rd2, un2) \
//\
//ubergroup_t T ## __group_ ## G; \
//\
//EXTERN3(rdlock_top_ ## G, rdlock_mutex_ ## G, rdlock_rw_ ## G);  \
//EXTERN3(rdlock_mutex_held_ ## G, rdlock_end_ ## G, rdlock_mutex_failed_ ## G); \
//EXTERN3(rdlock_rw_held_ ## G, rdlock_rw_failed_ ## G, rdlock_end_ ## G); \
//EXTERN3(wrlock_top_ ## G, wrlock_mutex_ ## G, wrlock_rw_ ## G);  \
//EXTERN3(wrlock_mutex_held_ ## G, wrlock_end_ ## G, wrlock_mutex_failed_ ## G); \
//EXTERN3(wrlock_rw_held_ ## G, wrlock_rw_failed_ ## G, wrlock_end_ ## G);    \
//EXTERN3(unlock_top_ ## G, unlock_mutex_ ## G, unlock_rw_ ## G);   \
//\
// __attribute__((optimize("align-functions=1024")))\
//int T ## _lock_unlock_ ## G(T ## _lock_t *t) { \
//  top:              \
//  JMP(unlock_top_ ## G, unlock_mutex_ ## G, unlock_rw_ ## G);  \
//  LABEL(unlock_mutex_ ## G);      \
//    un1(&t->l1);      \
//    goto end;            \
//    LABEL(unlock_rw_ ## G);        \
//    un2(&t->l2);      \
//    goto end;            \
// end:             \
// return 0;            \
//}              \
//\
//int T ## _lock_rdlock_ ## G(T ## _lock_t *t) { \
// top:            \
//  JMP(rdlock_top_ ## G, rdlock_mutex_ ## G, rdlock_rw_ ## G);  \
//  LABEL(rdlock_mutex_ ## G);      \
//    rd1(&t->l1);      \
//    JMP(rdlock_mutex_held_ ## G, rdlock_end_ ## G, rdlock_mutex_failed_ ## G);  \
//    LABEL(rdlock_mutex_failed_ ## G);      \
//      un1(&t->l1);      \
//      goto top;            \
//  LABEL(rdlock_rw_ ## G);        \
//    rd2(&t->l2);      \
//    JMP(rdlock_rw_held_ ## G, rdlock_rw_failed_ ## G, rdlock_end_ ## G);  \
//    LABEL(rdlock_rw_failed_ ## G);      \
//      un2(&t->l2);      \
//      goto top;            \
//  LABEL(rdlock_end_ ## G);        \
//  return 0;            \
//}              \
//               \
//\
//\
// int T ## _lock_wrlock_ ## G(T ## _lock_t *t) { \
// top:            \
//   JMP(wrlock_top_ ## G, wrlock_mutex_ ## G, wrlock_rw_ ## G);  \
//  LABEL(wrlock_mutex_ ## G);      \
//    wr1(&t->l1);      \
//    JMP(wrlock_mutex_held_ ## G, wrlock_end_ ## G, wrlock_mutex_failed_ ## G);  \
//    LABEL(wrlock_mutex_failed_ ## G);      \
//      un1(&t->l1);      \
//      goto top;            \
//  LABEL(wrlock_rw_ ## G);        \
//    wr2(&t->l2);      \
//    JMP(wrlock_rw_held_ ## G, wrlock_rw_failed_ ## G, wrlock_end_ ## G); \
//    LABEL(wrlock_rw_failed_ ## G);      \
//      un2(&t->l2);      \
//      goto top;            \
//  LABEL(wrlock_end_ ## G);        \
//  return 0;            \
//}              \
//\
//\
//\
//static void T ## __patch_ ## G(int mode) {    \
//  static int init = 0;      \
//    if (init == 0) {         \
//    set_permissions(&rdlock_top_ ## G, 1); \
//    set_permissions(&rdlock_mutex_held_ ## G, 1); \
//    set_permissions(&rdlock_rw_held_ ## G, 1);\
//    set_permissions(&wrlock_top_ ## G, 1); \
//    set_permissions(&wrlock_mutex_held_ ## G, 1); \
//    set_permissions(&wrlock_rw_held_ ## G, 1); \
//    set_permissions(&unlock_top_ ## G, 1);    \
//    init = 1;                \
//  }                  \
//                      \
//  PATCH(mode, &rdlock_top_ ## G, &rdlock_mutex_ ## G, &rdlock_rw_ ## G); \
//  PATCH(mode, &rdlock_mutex_held_ ## G, &rdlock_end_ ## G, &rdlock_mutex_failed_ ## G); \
//  PATCH(mode, &rdlock_rw_held_ ## G, &rdlock_rw_failed_ ## G, &rdlock_end_ ## G);    \
//  PATCH(mode, &wrlock_top_ ## G, &wrlock_mutex_ ## G, &wrlock_rw_ ## G);  \
//  PATCH(mode, &wrlock_mutex_held_ ## G, &wrlock_end_ ## G, &wrlock_mutex_failed_ ## G); \
//  PATCH(mode, &wrlock_rw_held_ ## G, &wrlock_rw_failed_ ## G, &wrlock_end_ ## G); \
//  PATCH(mode, &unlock_top_ ## G, &unlock_mutex_ ## G, &unlock_rw_ ## G);    \
//}               \
//
//#define NUM_PATCHES 7
//
//#define COMBINED_GROUP(G, T) T ## __group_ ## G
//
//#define INIT_COMBINED_GROUP(T, probe, exp_freq, exp_time)  \
//  int ubergroup_init_special(&T ## __group_ ## G, probe, exp_freq, exp_time, T ## __patch_ ## G);
//
//#ifdef __cplusplus
//#define EXTERN_COMBINED_GROUP(G, T)      \
//  extern "C" { \
//   extern ubergroup_t T ## __group_ ## G;                                   \
//   int T ## _lock_rdlock_ ## G(T ## _lock_t *t);\
//   int T ## _lock_wrlock_ ## G(T ## _lock_t *t);   \
//   int T ## _lock_unlock_ ## G(T ## _lock_t *t);   \
//  } 
//#else
//#define EXTERN_COMBINED_GROUP(G, T)                 \
//  extern ubergroup_t T ## __group_ ## G;                                    \
//  int T ## _lock_rdlock_ ## G(T ## _lock_t *t);\
//  int T ## _lock_wrlock_ ## G(T ## _lock_t *t);    \
//  int T ## _lock_unlock_ ## G(T ## _lock_t *t);    
//#endif
//
//
//
//#define DEFINE_COMBINED_LOCK(T, L1, S1, init1, destroy1, wr1, rd1, un1, L2, S2, init2, destroy2, wr2, rd2, un2) \
//  DECLARE_COMBINED_LOCK(T,L1,L2) \
//  CPP_START \
//int T##_lock_init(T##_lock_t* o, T##_group_t *group) {    \
//  o->group = group;    \
//  o->prev = NULL;    \
//      \
//  int err = init1(&(o->l1), NULL);    \
//  if (err != 0) return err;    \
//  {    \
//    \
//    err = init2(&(o->l2), NULL);    \
//    if (err != 0) return err;    \
//  }    \
//      \
//  pthread_mutex_lock(&(o->group->mutex));    \
//      \
//  o->mode = group->mode;    \
//      \
//  o->next = o->group->locks;    \
//  if (o->next != NULL) {    \
//    o->next->prev = o;    \
//  } else {    \
//    o->next = NULL;    \
//  }    \
//  o->group->locks = o;    \
//  pthread_mutex_unlock(&(o->group->mutex));    \
//  return 0;    \
//}    \
//    \
//int T##_lock_destroy(T##_lock_t* o) {    \
//  pthread_mutex_lock(&(o->group->mutex));    \
//  if (o->prev != NULL) {    \
//    o->prev->next = o->next;    \
//  } else {    \
//    o->group->locks = o->next;    \
//  }    \
//  if (o->next != NULL) {    \
//    o->next->prev = o->prev;    \
//  }    \
//  pthread_mutex_unlock(&(o->group->mutex));    \
//      \
//  int result = destroy1(&(o->l1));    \
//  if (result != 0) return result;    \
//  result = destroy2(&(o->l2));    \
//  return result;    \
//}    \
//    \
// /* call only when lock on group is held. */    \
//int T##_lock_setmode(T##_lock_t* o, int mode) {    \
//    wr1(&o->l1);    \
//    wr2(&o->l2);    \
//    o->mode = mode;    \
//    un1(&o->l1);    \
//    un2(&o->l2);    \
//  return 0;    \
//}    \
//    \
//int T##_lock_rdlock(T##_lock_t* o) {    \
// top:    \
//  if (o->mode == 0) {    \
//    rd1(&o->l1);    \
//    if (o->mode != 0) {    \
//      un1(&o->l1);    \
//      goto top;    \
//    }    \
//  } else {    \
//    rd2(&o->l2);    \
//    if (o->mode == 0) {    \
//      un2(&o->l2);    \
//      goto top;    \
//    }    \
//  }    \
//  return 0;    \
//}    \
//    \
//int T##_lock_wrlock(T##_lock_t* o)  {    \
// top:    \
//  if (o->mode == 0) {    \
//    wr1(&o->l1);    \
//    if (o->mode != 0) {    \
//      un1(&o->l1);    \
//      goto top;    \
//    }    \
//  } else {    \
//    wr2(&o->l2);    \
//    if (o->mode == 0) {    \
//      un2(&o->l2);    \
//      goto top;    \
//    }    \
//  }    \
//  return 0;    \
//}    \
//    \
//int T##_lock_unlock(T##_lock_t* o) {    \
//  if (o->mode == 0) {    \
//    un1(&o->l1);    \
//  } else {    \
//    un2(&o->l2);    \
//  }    \
//  return 0;    \
//}    \
//    \
//    \
//static void T##_group_probe_for_ppoint(uberprobe_t *probe, ppoint_t *ppoint);    \
//    \
//static void T##_group_print_experiment(int mode, double orig, double other) {    \
//  if (mode == 0) {    \
//    printf("# %s/%s: %-8.2g         %5s:%-8.2g   %5s:%-8.2g\n",    \
//     S1, S2, orig / other, T##_group_name_for_mode(mode), orig, T##_group_name_for_mode((mode + 1) % 2), \
//     other);    \
//  } else {    \
//    printf("# %s/%s: %-8.2g         %5s:%-8.2g   %5s:%-8.2g\n",    \
//     S1, S2, other / orig, T##_group_name_for_mode((mode + 1) % 2), other, T##_group_name_for_mode(mode), \
//     orig);    \
//  }    \
//}    \
//    \
//int T##_group_get_wins(T##_group_t *group, int mode) {    \
//  pthread_mutex_lock(&group->mutex);    \
//  int result = group->results[mode];    \
//  pthread_mutex_unlock(&group->mutex);    \
//  return result;    \
//}    \
//    \
//int T##_group_get_preferred_mode(T##_group_t *group) {    \
//  int pref = 0;    \
//  pthread_mutex_lock(&group->mutex);    \
//  if (group->results[1] > group->results[0]) {    \
//    pref = 1;    \
//  }    \
//  pthread_mutex_unlock(&group->mutex);    \
//  return pref;    \
//}    \
//    \
//const char *T##_group_name_for_mode(int mode) {    \
//  static const char * mode_name[] = { S1, S2 };    \
//  return mode_name[mode];    \
//}    \
//    \
//void *T##_group_adapt(T##_group_t *group) {    \
//  LOG(("Launched adapter"));    \
//  util_wait_ms(group->initial_delay);    \
//  while (1) {    \
//    struct timeval t1, t2; \
//    gettimeofday(&t1, NULL); \
//\
//    LOG(("Starting Experiment for Group %d", group->id));    \
//    uberprobe_t* probe = group->probe;    \
//    size_t exp_time = T##_group_get_exp_time(group);    \
//    size_t exp_freq = T##_group_get_exp_freq(group);    \
//    \
//    size_t start = probe->start(probe->probe);    \
//    double originalInterval = util_wait_ms(exp_time);    \
//    size_t originalValue = probe->stop(probe->probe, start);    \
//    \
//    int mode = T##_group_get_mode(group);    \
//    \
//    LOG(("Results: %d -> %g:%d: %g", T##_group_get_mode(group), originalValue, originalInterval, originalValue/originalInterval*1000));    \
//    \
//    printf("Start\n"); \
//    T##_group_set_mode(group, mode == MUTEX_MODE ? RW_MODE : MUTEX_MODE);    \
//    printf("Stop\n"); \
//    \
//    util_wait_ms(exp_time);    \
//    \
//    start = probe->start(probe->probe);    \
//    double switchedInterval = util_wait_ms(exp_time);    \
//    size_t switchedValue = probe->stop(probe->probe, start);    \
//        \
//    T##_group_print_experiment(mode,    \
//         originalValue / originalInterval * 1000,    \
//         switchedValue / switchedInterval * 1000);    \
//    \
//    if (originalValue / originalInterval    \
//  > switchedValue / switchedInterval    \
//  ||    \
//  originalValue / originalInterval * 1000 < 5) {    \
//      printf("Start\n");            \
//      T##_group_set_mode(group, mode);    \
//      printf("Stop\n");            \
//\
//      double elapsedTime; \
//      gettimeofday(&t2, NULL); \
//      elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;      \
//      elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;   \
//      printf("EXP: %g ms\n", elapsedTime);\
//    } \
//    LOG(("Mode is now: %d", T##_group_get_mode(group)));  \
//    mode = T##_group_get_mode(group);    \
//    group->results[mode]++;    \
//    printf("Mode is now: %d\n", mode);    \
//        \
//    util_wait_ms(exp_freq);    \
//  }    \
//  return NULL;    \
//}    \
//    \
//static uberprobe_t T##_group_default_probe __attribute__ ((__aligned__(64)));    \
//static T##_group_t T##_group_default_group __attribute__ ((__aligned__(64)));    \
//    \
//static int T##_group_init_id(T##_group_t *g, int id, uberprobe_t *probe,    \
//           size_t exp_freq, size_t exp_time, void (*patch)(int)) {    \
//  g->locks = NULL;    \
//  g->id = id;    \
//  g->probe = probe;    \
//  g->exp_freq = 0;    \
//  g->exp_time = exp_time;    \
//  g->initial_delay = 100;    \
//  g->mode = MUTEX_MODE;    \
//  g->patch = patch;    \
//  if (g->patch != NULL) {    \
//    g->patch(MUTEX_MODE);    \
//  }    \
//  pthread_mutex_init(&g->mutex, NULL);    \
//  T##_group_set_exp_freq(g, exp_freq);    \
//  return 0;    \
//}    \
//    \
//static void T##_group_probe_for_ppoint(uberprobe_t *probe, ppoint_t *ppoint) {    \
//  probe->probe = ppoint;    \
//  probe->start = (size_t (*)(void *)) ppoint_start;    \
//  probe->stop = (size_t (*)(void *, size_t)) ppoint_stop;    \
//}    \
//    \
//int T##_group_init(T##_group_t *g, uberprobe_t *probe, size_t exp_freq,    \
//       size_t exp_time) {    \
//  return T##_group_init_special(g, probe, exp_freq, exp_time, NULL);    \
//}    \
//    \
//int T##_group_init_special(T##_group_t *g, uberprobe_t *probe, size_t exp_freq,    \
//         size_t exp_time, void (*patch)(int)) {    \
//    \
//  static int group_id_counter = 1;    \
//  return T##_group_init_id(g, group_id_counter++, probe, exp_freq, exp_time, patch);    \
//}    \
//    \
//size_t T##_group_get_exp_freq(T##_group_t *group) {    \
//  pthread_mutex_lock(&group->mutex);    \
//  size_t result = group->exp_freq;    \
//  pthread_mutex_unlock(&group->mutex);    \
//  return result;    \
//}    \
//    \
//void T##_group_set_exp_freq(T##_group_t *group, size_t exp_freq) {    \
//  pthread_mutex_lock(&group->mutex);    \
//  if (group->exp_freq == 0 && exp_freq > 0) {    \
//    pthread_create(&(group->adapter), NULL,    \
//       (void *(*)(void *)) T##_group_adapt, group);    \
//  } else if (group->exp_freq > 0 && exp_freq == 0) {    \
//    pthread_cancel(group->adapter);    \
//    pthread_join(group->adapter, NULL);    \
//  }    \
//  group->exp_freq = exp_freq;    \
//  pthread_mutex_unlock(&group->mutex);    \
//}    \
//    \
//size_t T##_group_get_exp_time(T##_group_t *group) {    \
//  pthread_mutex_lock(&group->mutex);    \
//  size_t result = group->exp_time;    \
//  pthread_mutex_unlock(&group->mutex);    \
//  return result;    \
//}    \
//    \
//void T##_group_set_exp_time(T##_group_t *group, size_t exp_time) {    \
//  pthread_mutex_lock(&group->mutex);    \
//  group->exp_time = exp_time;    \
//  pthread_mutex_unlock(&group->mutex);    \
//}    \
//\
//size_t T##_group_get_initial_adapt_delay(T##_group_t* group) {  \
//  pthread_mutex_lock(&group->mutex);    \
//  size_t result = group->initial_delay;    \
//  pthread_mutex_unlock(&group->mutex);    \
//  return result; \
//}    \
//\
//void T##_group_set_initial_adapt_delay(T##_group_t* group, size_t init_delay) { \
//  pthread_mutex_lock(&group->mutex);    \
//  group->initial_delay = init_delay;    \
//  pthread_mutex_unlock(&group->mutex);    \
//}    \
//\
//int T##_group_destroy(T##_group_t *g) {    \
//  T##_group_set_exp_freq(g, 0);    \
//  return pthread_mutex_destroy(&(g->mutex));    \
//}    \
//    \
//int T##_group_get_mode(T##_group_t* g) {    \
//  pthread_mutex_lock(&(g->mutex));    \
//  int x = g->mode;    \
//  pthread_mutex_unlock(&(g->mutex));    \
//  return x;    \
//}    \
//    \
//int T##_group_set_mode(T##_group_t* g, int mode) {    \
//  /* return 0; */            \
// struct timeval t1, t2;\
// double elapsedTime; \
// gettimeofday(&t1, NULL); \
//  pthread_mutex_lock(&(g->mutex));    \
//  if (g->patch == NULL) {    \
//    for (T##_lock_t *l = g->locks; l != NULL; l = l->next) {    \
//      T##_lock_setmode(l, mode);            \
//    }    \
//    g->mode = mode;    \
//  } else {    \
//    for (T##_lock_t *l = g->locks; l != NULL; l = l->next) {    \
//        wr1(&l->l1);              \
//  wr2(&l->l2);              \
//    }    \
//    g->patch(mode);    \
//    g->mode = mode;    \
//    for (T##_lock_t *l = g->locks; l != NULL; l = l->next) {    \
//      l->mode = mode;    \
//        un1(&l->l1);    \
//  un2(&l->l2);     \
//    }    \
//  }     \
//  pthread_mutex_unlock(&(g->mutex));    \
//  gettimeofday(&t2, NULL);\
//  elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;      \
//  elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;   \
//  printf("MODE %g\n", elapsedTime); \
//  return 0;    \
//}    \
//    \
//    \
//int T##_lock_tryrdlock(T##_lock_t* o) {    \
//  return 0;    \
//}    \
//    \
//int T##_lock_trywrlock(T##_lock_t* o) {    \
//  return 0;    \
//}    \
//    \
//DEFINE_COMBINED_GROUP(DEFAULT,T, L1, S1, init1, destroy1, wr1, rd1, un1, L2, S2, init2, destroy2, wr2, rd2, un2);\
//    \
//T##_group_t *T##_group_default() {    \
//  static int initialized = 0;    \
//  if (!initialized) {    \
//    T##_group_probe_for_ppoint(&T##_group_default_probe, default_ppoint());    \
//    char *freq_env = getenv("UBER_DEFAULT_FREQ");    \
//    int freq = (freq_env == NULL) ? 0 : atoi(freq_env);    \
//    char *time_env = getenv("UBER_DEFAULT_TIME");    \
//    int time = (time_env == NULL) ? 10 : atoi(time_env);    \
//    T##_group_init_id(&T##_group_default_group, 0, &T##_group_default_probe, freq, time, NULL); /*&T ## __patch_DEFAULT); */ \
//    initialized = 1;    \
//  }    \
//  return &T##_group_default_group;    \
//}    \
//    \
//  CPP_END\
//
//
//
//#endif /* UBERLOCK_H_ */
//

