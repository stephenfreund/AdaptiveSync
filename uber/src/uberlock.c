/*
 * uberlock.c
 *
 *  Created on: Jan 12, 2017
 *      Author: freund
 */
#define __USE_GNU
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
#include <unistd.h>


int uberlock_init(uberlock_t* o, ubergroup_t *group) {
  o->group = group;
  o->prev = NULL;
  
  int err = pthread_rwlock_init(&(o->rw), NULL);
  if (err != 0) return err;
  {

    err = pthread_mutex_init(&(o->mutex), NULL);
    if (err != 0) return err;
  }
  
  pthread_mutex_lock(&(o->group->mutex));
  
  o->mode = group->mode;
  
  o->next = o->group->locks;
  if (o->next != NULL) {
    o->next->prev = o;
  } else {
    o->next = NULL;
  }
  o->group->locks = o;
  pthread_mutex_unlock(&(o->group->mutex));
  return 0;
}

int uberlock_destroy(uberlock_t* o) {
  pthread_mutex_lock(&(o->group->mutex));
  if (o->prev != NULL) {
    o->prev->next = o->next;
  } else {
    o->group->locks = o->next;
  }
  if (o->next != NULL) {
    o->next->prev = o->prev;
  }
  pthread_mutex_unlock(&(o->group->mutex));
  
  int result = pthread_rwlock_destroy(&(o->rw));
  if (result != 0) return result;
  result = pthread_mutex_destroy(&(o->mutex));
  return result;
}


// call only when lock on group is held.
int uberlock_setmode(uberlock_t* o, int mode) {
  pthread_mutex_lock(&o->mutex);
  pthread_rwlock_wrlock(&o->rw);
  o->mode = mode;
  pthread_mutex_unlock(&o->mutex);
  pthread_rwlock_unlock(&o->rw);
  return 0;
}

int uberlock_rdlock(uberlock_t* o) {
 top:
  if (o->mode == MUTEX_MODE) {
    pthread_mutex_lock(&o->mutex);
    if (o->mode != MUTEX_MODE) {
      pthread_mutex_unlock(&o->mutex);
      goto top;
    }
  } else {
    pthread_rwlock_rdlock(&o->rw);
    if (o->mode == MUTEX_MODE) {
      pthread_rwlock_unlock(&o->rw);
      goto top;
    }
  }
  return 0;
}

int uberlock_wrlock(uberlock_t* o)  {
 top:
  if (o->mode == MUTEX_MODE) {
    pthread_mutex_lock(&o->mutex);
    if (o->mode != MUTEX_MODE) {
      pthread_mutex_unlock(&o->mutex);
      goto top;
    }
  } else {
    pthread_rwlock_wrlock(&o->rw);
    if (o->mode == MUTEX_MODE) {
      pthread_rwlock_unlock(&o->rw);
      goto top;
    }
  }
  return 0;
}

int uberlock_unlock(uberlock_t* o) {
  if (o->mode == MUTEX_MODE) {
    pthread_mutex_unlock(&o->mutex);
  } else {
    pthread_rwlock_unlock(&o->rw);
  }
  return 0;
}



