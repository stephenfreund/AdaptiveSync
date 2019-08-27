/*
 * ppoint.h
 *
 *  Created on: Jan 10, 2017
 *      Author: freund
 */

#ifndef PPOINT_H_
#define PPOINT_H_

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  size_t count  __attribute__((aligned(64)));
  int on  __attribute__((aligned(64)));
} ppoint_t;

#define PPOINT_INIT  { 0, 0 }

inline static void ppoint_tick_noguard(ppoint_t *p) {
  __atomic_add_fetch(&((p)->count), 1, __ATOMIC_RELAXED);
}

inline static void ppoint_tick(ppoint_t *p) {
  if (__atomic_load_n(&((p)->on), __ATOMIC_RELAXED) != 0) {
    __atomic_add_fetch(&((p)->count), 1, __ATOMIC_RELAXED);
  }
}

inline static void ppoint_add(ppoint_t *p, size_t v) {
  if (__atomic_load_n(&((p)->on), __ATOMIC_RELAXED) != 0) {
    __atomic_add_fetch(&((p)->count), v, __ATOMIC_RELAXED);
  }
}

inline static size_t ppoint_start(ppoint_t *p) {
  __atomic_add_fetch(&((p)->on), 1, __ATOMIC_RELAXED);
  return __atomic_load_n(&((p)->count), __ATOMIC_RELAXED);
}

inline static size_t ppoint_stop(ppoint_t *p, size_t start) {
  size_t result = __atomic_load_n(&((p)->count), __ATOMIC_RELAXED);
  /* Don't reset anymore...
  if (__atomic_add_fetch(&((p)->on), -1, __ATOMIC_RELAXED) == 0) {
    __atomic_store_n(&((p)->count), 0, __ATOMIC_RELAXED);
  } 
  */ 
  return result - start;
}
//
//inline static void probe_for_ppoint(uberprobe_t *probe, ppoint_t *ppoint) {
//  probe->arg = ppoint;
//  probe->start = (size_t (*)(void *)) ppoint_start;
//  probe->stop = (size_t (*)(void *, size_t)) ppoint_stop;
//}

void ppoint_init(ppoint_t *p);
  ppoint_t *default_ppoint(void);


#ifdef __cplusplus
}
#endif

#endif /* PPOINT_H_ */
