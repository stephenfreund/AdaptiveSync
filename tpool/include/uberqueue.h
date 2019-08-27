#ifndef UBERQUEUE_H
#define UBERQUEUE_H

#include <pthread.h>
#include "ppoint.h"

typedef struct uberqueue {
  int head, tail;
  void ** data;
  int size;

  int prod_threads;		// no of producing threads

  pthread_mutex_t mutex;
  pthread_cond_t empty, full;

} uberqueue_t;

void uberqueue_init(uberqueue_t* que, int size);
void uberqueue_destroy(uberqueue_t* que);

void uberqueue_add_producer(uberqueue_t* que);
void uberqueue_remove_producer(uberqueue_t* que);

int uberqueue_dequeue(uberqueue_t* que, void** to_buf);

int uberqueue_dequeue_can_abort(uberqueue_t* que, void** to_buf);
void uberqueue_abort(uberqueue_t* que);

int uberqueue_enqueue(uberqueue_t* que, void* from_buf);

size_t uberqueue_size(uberqueue_t * que);
#endif
