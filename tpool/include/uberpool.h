
#ifndef UBERPOOL_H
#define UBERPOOL_H

#include <pthread.h>
#include "uknob.h"

typedef enum {
  POOL_STATE_READY,
  POOL_STATE_RUNNING,
  POOL_STATE_ENDING,
  POOL_STATE_DONE,
  POOL_STATE_ERROR
} pool_state_t;

typedef int    (*produce_f)(void *produce_data, void *value);
typedef int    (*uberpool_control_f)(void *desc_data);
typedef int    (*uberpool_routine_f)(void *thread_data, void *desc_data, void *value, void *produce_data, produce_f produce);

typedef struct uberdesc_s {
  uberpool_routine_f routine;
  uberpool_control_f begin;
  uberpool_control_f end;
  void *data;
} uberdesc_t;

typedef struct uberpool uberpool_t;

uberpool_t *uberpool_create(int nthreads, void **thread_data, uberdesc_t *descs, int ndescs, pthread_attr_t *attr);

int items_in_queues(uberpool_t *pool);

size_t uberpool_submit(uberpool_t *pool, void *value);
size_t uberpool_retrieve(uberpool_t *pool, void **value);

void uberpool_destroy(uberpool_t *pool);

// result can be NULL, or must be big enough for pool->nthreads return values
int uberpool_join(uberpool_t *pool, void **result);

int uberpool_cancel(uberpool_t *pool);

knob_t *make_thread_knob(uberpool_t *pool);

#endif
