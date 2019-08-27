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
#include "uberpool.h"
#include "uberqueue.h"
#include "uknob.h"
#include "ppoint.h"
#include "util.h"

#define N 1000

typedef struct {
  int count;
} data_t;

typedef struct {
  int delay;
} stage_t;

int stage_begin(void *dummy) {
  return 0;
}

size_t dummy;

int stage(void *thread_data, void *dummy, void *value, void *produce_data, produce_f produce) {
  data_t *load = (data_t *)value;;
  stage_t *stage = (stage_t*)dummy;
  size_t start = util_get_time();
  size_t end_time = start + stage->delay * 1000 * 1000 + ((double)rand()) / RAND_MAX * N - N/2;
  while (util_get_time() < end_time) {
    dummy++;
  }
  load->count++;
  produce(produce_data, load);
  return 0;
}

int stage_end(void *dummy) {
  return 0;
}

ppoint_t exit_counter = PPOINT_INIT;
ppoint_t start_counter = PPOINT_INIT;
uberprobe_t probe;


void * consume(void *dummy) {
  void *value;
  uberpool_t *pool = (uberpool_t *)dummy;
  while (uberpool_retrieve(pool, &value) != -1) {
    ppoint_tick(&exit_counter);
    free(value);
  }
  return 0;
}


#define FIRST 100
#define MID 2
#define LAST 2

#define SCALE  2
#define STAGES 4

int main (int argc, char *argv[]) {
  
  //  stage_t s0 = { 1 };
  stage_t s[] = { { 8 }, { 6 }, { 11 }, { 2 } };
  
  uberdesc_t descs[] = {
    { stage, stage_begin, stage_end, &(s[0]) },
    { stage, stage_begin, stage_end, &(s[1]) },
    { stage, stage_begin, stage_end, &(s[2]) },
    { stage, stage_begin, stage_end, &(s[3]) },
  };
  
throughput_probe_init(&probe, &exit_counter);
  uberpool_t *pool = uberpool_create(SCALE * STAGES, NULL, descs, 4, NULL);
  
  pthread_t pthread;
  pthread_create(&pthread,
                 NULL,
                 consume,
                 pool);
  
  knob_t *knob = make_thread_knob(pool);
  adapter_t *adapter = deterministic_adapter(knob, &probe);
  async_control_t *control = async_control(adapter, 1000, 500);
  async_control_start(control);
  
  int duration = 0;
  for (int i = 0; i < STAGES; i++) {
    duration += s[i].delay;
  }
  
  for (int i = 0; i < 10000000; i++) {
    usleep(duration * 1000 / 40);
    data_t *data = (data_t *)malloc(sizeof(data_t));
    assert(data != NULL);
    data->count = 0;
    uberpool_submit(pool, data);
    ppoint_tick(&start_counter);
  }
  
  uberpool_join(pool, NULL);
  pthread_join(pthread, NULL);
  async_control_stop(control);
  delete_async_control(control);
  delete_adapter(adapter);
  
  uberpool_destroy(pool);
  uberprobe_destroy(&probe);

  return 0;
}
