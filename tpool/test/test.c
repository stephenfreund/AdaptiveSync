/* AUTORIGHTS
Copyright (C) 2007 Princeton University
      
This file is part of Ferret Toolkit.

Ferret Toolkit is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software Foundation,
Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <pthread.h>
#include "tpool.h"
#include "queue.h"

typedef struct { 
  int count; 
} data_t;

typedef struct {
  int delay;
  queue_t *in;
  queue_t *out;
} stage_t;


/* the whole path to the file */
void *first(void *dummy) {
  data_t *data = (data_t *)malloc(sizeof(data_t));
  queue_t *out = (queue_t *)dummy;
  assert(data != NULL);
  data->count = 0;
  
  enqueue(out, data);
  
  queue_signal_terminate(out);
  
  return NULL;
}

void *stage(void *dummy) {
  data_t *load;
  stage_t *stage = (stage_t*)dummy;
  while (dequeue(stage->in, (void **)&load) >= 0) {
    assert(load != NULL);
    usleep(stage->delay * 1000);
    load->count++;
    enqueue(stage->out, load);
  }
  
  queue_signal_terminate(stage->out);
  return NULL;
}

void *last(void *dummy) {
  queue_t *in = (queue_t *)dummy;
  data_t *load;
  
  while (dequeue(in, (void **)&load) >= 0) {
    assert(load != NULL);
    printf("%d\n", load->count);
    free(load);
  }
  return NULL;
}

#define FIRST 100
#define MID 2
#define LAST 2
#define SIZE 50

static tdesc_t* make_desc(int n, const pthread_attr_t *attr, tpool_start_routine_f start_routine, void *arg) {
  tdesc_t *desc = (tdesc_t *)calloc(n, sizeof(tdesc_t));
  for (int i = 0; i < n; i++) {
    desc[i].attr = attr;
    desc[i].start_routine = start_routine;
    desc[i].arg = arg;
  }
  return desc;
}


int main (int argc, char *argv[]) {

  queue_t q1;
  queue_t q2;
  queue_t q3;

  queue_init(&q1, SIZE, FIRST);
  queue_init(&q2, SIZE, MID);
  queue_init(&q3, SIZE, LAST);

  tdesc_t *t_first = make_desc(FIRST, NULL, first, &q1);
  stage_t s1 = { 100, &q1, &q2 };
  tdesc_t *t_seg1 = make_desc(MID, NULL, stage, &s1);
  stage_t s2 = { 200, &q2, &q3 };
  tdesc_t *t_seg2 = make_desc(MID, NULL, stage, &s2);
  tdesc_t *t_last = make_desc(LAST, NULL, last, &q3);
  
  tpool_t *p_first = tpool_create(t_first, FIRST);
  tpool_t *p_mid1  = tpool_create(t_seg1, MID);
  tpool_t *p_mid2  = tpool_create(t_seg2, MID);
  tpool_t *p_last  = tpool_create(t_last, LAST);
  
  tpool_join(p_last, NULL);
  tpool_join(p_mid2, NULL);
  tpool_join(p_mid1, NULL);
  tpool_join(p_first, NULL);
  
  tpool_destroy(p_first);
  tpool_destroy(p_mid1);
  tpool_destroy(p_mid2);
  tpool_destroy(p_last);
  
  free(t_first);
  free(t_seg1);
  free(t_seg2);
  free(t_last);
  
  queue_destroy(&q1);
  queue_destroy(&q2);
  queue_destroy(&q3);
  return 0;
}
