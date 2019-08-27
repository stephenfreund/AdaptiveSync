#if 0

/*
 * uberlock.c
 *
 *  Created on: Jan 12, 2017
 *      Author: freund
 */

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


const char *ubergroup_name_for_mode(void *dummy, int mode, char *name) {
  static const char * mode_name[] = { "Mutex", "RW" };
  return strcpy(name, mode_name[mode]);
}




static int ubergroup_init_id(ubergroup_t *g, int id, void (*patch)(int)) {
  g->locks = NULL;
  g->id = id;
  g->mode = MUTEX_MODE;
  g->patch = patch;
  if (g->patch != NULL) {
    g->patch(MUTEX_MODE);
  }
  pthread_mutex_init(&g->mutex, NULL);
  return 0;
}

int ubergroup_init(ubergroup_t *g) {
  return ubergroup_init_special(g, NULL);
}

int ubergroup_init_special(ubergroup_t *g, void (*patch)(int)) {
  static int group_id_counter = 1;
  return ubergroup_init_id(g, group_id_counter++, patch);
}

int ubergroup_destroy(ubergroup_t *g) {
  return pthread_mutex_destroy(&(g->mutex));
}

int ubergroup_get_mode(ubergroup_t* g) {
  pthread_mutex_lock(&(g->mutex));
  int x = g->mode;
  pthread_mutex_unlock(&(g->mutex));
  return x;
}

int ubergroup_set_mode(ubergroup_t* g, int mode, int at_safe_point) {
  pthread_mutex_lock(&(g->mutex));
  if (g->patch == NULL) {
    for (uberlock_t *l = g->locks; l != NULL; l = l->next) {
      // l->mode = mode;
      uberlock_setmode(l, mode);
    }
    g->mode = mode;
  } else {
    for (uberlock_t *l = g->locks; l != NULL; l = l->next) {
      pthread_mutex_lock(&l->mutex);
      pthread_rwlock_wrlock(&l->rw);
    }
    g->patch(mode);
    g->mode = mode;
    for (uberlock_t *l = g->locks; l != NULL; l = l->next) {
      l->mode = mode;
      pthread_mutex_unlock(&l->mutex);
      pthread_rwlock_unlock(&l->rw);
    }
  } 
  pthread_mutex_unlock(&(g->mutex));
  return 0;
}


int uberlock_tryrdlock(uberlock_t* o) {
  return 0;
}

int uberlock_trywrlock(uberlock_t* o) {
  return 0;
}

//DEFINE_GROUP(DEFAULT);

//static uberprobe_t default_probe __attribute__ ((__aligned__(64)));

ubergroup_t *ubergroup_default() {
  static ubergroup_t default_group __attribute__ ((__aligned__(64)));
  static int initialized = 0;
  printf("XXX\n");
  if (!initialized) {
#ifdef __APPLE__
    ubergroup_init_id(&default_group, 0, NULL);
#else 
    ubergroup_init_id(&default_group, 0, NULL); // &__patch_DEFAULT);
#endif

    initialized = 1;
  }
  return &default_group;
}

/*
 probe_for_ppoint(&default_probe, default_ppoint());
 char *freq_env = getenv("UBER_DEFAULT_FREQ");
 int freq = (freq_env == NULL) ? 0 : atoi(freq_env);
 char *time_env = getenv("UBER_DEFAULT_TIME");
 int time = (time_env == NULL) ? 10 : atoi(time_env);
 printf("%d\n", freq);
 */

#endif
