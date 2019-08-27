/*
 * uberlock.h
 *
 *  Created on: Jan 10, 2017
 *      Author: freund
 */

#ifndef UBERLOCK_H_
#define UBERLOCK_H_

#ifdef __APPLE__
  #include <sys/_pthread/_pthread_types.h>
#else
  #include <bits/pthreadtypes.h>
#endif
#include <pthread.h>
#include "ppoint.h"
#include "patch.h"
#include "probe.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MUTEX_MODE 0
#define RW_MODE    1

typedef struct ubergroup_a ubergroup_t;
typedef struct uberlock_a uberlock_t;

typedef struct uberlock_a {
  pthread_rwlock_t rw     __attribute__((__aligned__(64)));
  pthread_mutex_t  mutex  __attribute__((__aligned__(64)));
  int mode                __attribute__((__aligned__(64)));
  ubergroup_t *group;
  struct uberlock_a *prev, *next;
} uberlock_t;


typedef struct ubergroup_a {
  pthread_mutex_t mutex;
  uberlock_t *locks;
  int id;
  int mode;
  
  void (*patch)(int);
  
} ubergroup_t;


int ubergroup_init(ubergroup_t *g);
int ubergroup_init_special(ubergroup_t *g, void (*patch)(void));

int ubergroup_get_mode(ubergroup_t* o);
int ubergroup_set_mode(ubergroup_t* o, int mode);

int ubergroup_destroy(ubergroup_t *g);

const char *ubergroup_name_for_mode(void *dummy, int mode, char *name);
  
ubergroup_t *ubergroup_default(void);


int uberlock_init(uberlock_t* o, ubergroup_t *g);
int uberlock_destroy(uberlock_t* o);
int uberlock_setmode(uberlock_t* o, int mode);
int uberlock_rdlock(uberlock_t* o);
int uberlock_wrlock(uberlock_t* o);
int uberlock_unlock(uberlock_t* o);


#ifdef __cplusplus
}
#endif


#endif /* UBERLOCK_H_ */

