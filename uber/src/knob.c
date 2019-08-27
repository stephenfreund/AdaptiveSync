//#include "knob.h"
//
//#include <stdlib.h>
//#include <stdio.h>
//#include <string.h>
//#include <assert.h>
//#include <pthread.h>
//#include <float.h>
//#include <sys/time.h>
//#include <unistd.h>
//
//#include "probe.h"
//#include "ppoint.h"
//#include "util.h"
//
//
//static void print_experiment(int_knob_t *knob,
//                             int orig_mode, double orig_value, double orig_interval,
//                             int new_mode,  double new_value,  double new_interval) {
//  char orig_name[MAX_NAME_SIZE];
//  char new_name[MAX_NAME_SIZE];
//  if (orig_mode < new_mode) {
//    printf("#%s %4s/%4s: %-8.2g         %-8.2g   %-8.2g\n", knob->name,
//           knob->value_name(knob->arg, orig_mode, orig_name), knob->value_name(knob->arg, new_mode, new_name), orig_value / new_value, orig_value, new_value);
//  } else {
//    printf("#%s %4s/%4s: %-8.2g         %-8.2g   %-8.2g\n", knob->name,
//           knob->value_name(knob->arg, new_mode, new_name), knob->value_name(knob->arg, orig_mode, orig_name), new_value / orig_value, new_value, orig_value);
//  }
//}
//
//char *knob_default_name(void *dummy, int mode, char *name) {
//  snprintf(name, MAX_NAME_SIZE, "%d", mode);
//  return name;
//}
//
//char *knob_value_name(int_knob_t *knob, int value, char *name) {
//  return knob->value_name(knob, value, name);
//}
//
//int knob_preferred_mode(int_knob_t *knob) {
//  int pref = 0;
//  for (int i = 1; i < knob->num_values; i++) {
//    if (knob->wins[i] > knob->wins[pref]) {
//      pref = i;
//    }
//  }
//  return pref;
//}
//
//static void complete_experiment(int_knob_t *knob, int at_safe_point,
//                                int orig_mode, size_t orig_value, double orig_interval,
//                                int new_mode,  size_t new_value,  double new_interval) {
//  pthread_mutex_lock(&knob->lock);
//  print_experiment(knob,
//                   orig_mode, orig_value, orig_interval,
//                   new_mode,  new_value,  new_interval);
//  if (orig_value / orig_interval > new_value / new_interval
//      || orig_value / orig_interval * 1000 * 1000 < 100) {
//    knob->set_value(knob->arg, orig_mode, at_safe_point);
//    knob->wins[orig_mode]++;
//  } else {
//    knob->wins[new_mode]++;
//  }
//  pthread_mutex_unlock(&knob->lock);
//  LOG(("Mode is now: %d", knob->get_value(knob->arg)));
//}
//
//static void *run(int_knob_t *knob) {
//  LOG(("Launched adapter"));
//  util_wait_ms(INITAL_DELAY);
//  while (1) {
//    LOG(("Starting Experiment for knob %s", knob->name));
//    uberprobe_t* probe = knob->probe;
//    
//    size_t start = probe->start(probe->probe);
//    double orig_interval = util_wait_ms(knob->exp_time) * 1000; // micros
//    size_t orig_probe_value = probe->stop(probe->probe, start);
//    
//    pthread_mutex_lock(&knob->lock);
//    int orig_value = knob->get_value(knob->arg);
//    int new_value = knob->exp_value(knob->arg, orig_value);
//    knob->set_value(knob->arg, new_value, 0);
//    pthread_mutex_unlock(&knob->lock);
//
//    util_wait_ms(knob->exp_time); // to stabilize behavior...
//
//    start = probe->start(probe->probe); 
//    double new_interval = util_wait_ms(knob->exp_time) * 1000; // micros
//    size_t new_probe_value = probe->stop(probe->probe, start);
//    
//    complete_experiment(knob, 0, orig_value, orig_probe_value, orig_interval, new_value, new_probe_value, new_interval);
//    util_wait_ms(knob->exp_freq);
//  }
//  return NULL;
//}
//
//static int flip(int_knob_t *knob, int x) {
//  return 1 - x;
//}
//
//static double elapsed_time(struct timeval *old, struct timeval *now) {
//  double elapsed_time = (now->tv_sec - old->tv_sec) * 1000.0;
//  elapsed_time += (now->tv_usec - old->tv_usec) / 1000.0;
//  return elapsed_time;
//}
//
//void knob_safe_point(int_knob_t *knob) {
//  struct timeval t2;
//  gettimeofday(&t2, NULL);
//  double elapsed = elapsed_time(&knob->interval_start, &t2);
//  if (elapsed < knob->interval_length) return;
//  switch (knob->status) {
//  case WAITING: {
//    knob->interval_start = t2;
//    knob->status = CUR_MEASURE;
//    knob->probe_start = knob->probe->start(knob->probe);
//    knob->interval_length = knob->exp_time;
//    break;
//  }
//  case CUR_MEASURE: {
//    knob->orig_interval = elapsed;
//    knob->orig_probe_value = knob->probe->stop(knob->probe, knob->probe_start);
//    knob->orig_value = knob->get_value(knob->arg);
//    knob->new_value = knob->exp_value(knob, knob->orig_value);
//    knob->set_value(knob->arg, knob->new_value, 1);
//    knob->interval_start = t2;
//    knob->status = PAUSE;
//    break;
//  }
//  case PAUSE: {
//    knob->interval_start = t2;
//    knob->status = OTHER_MEASURE;
//    knob->probe_start = knob->probe->start(knob->probe);
//    knob->interval_length = knob->exp_time;
//    break;
//  }
//  case OTHER_MEASURE: {
//    double new_interval = elapsed;
//    double new_probe_value = knob->probe->stop(knob->probe, knob->probe_start);
//    complete_experiment(knob, 1, new_interval, new_probe_value, knob->new_value, knob->orig_interval, knob->orig_probe_value, knob->orig_value);
//    knob->interval_start = t2;
//    knob->status = WAITING;
//    knob->interval_length = knob->exp_freq;
//    break;
//  }
//  }
//}
//
//
//void knob_binary_init(int_knob_t *knob, char *name, size_t exp_time, size_t exp_freq, int num_values, uberprobe_t *probe,
//                      void *arg, void (*set_value)(void *arg, int value, int at_safe_point), int (*get_value)(void *arg),
//                      char *(*value_name)(void *arg, int value, char *name)) {
//  assert(num_values == 2);
//  pthread_mutex_init(&knob->lock, NULL);
//  strncpy(knob->name, name, 63);
//  knob->exp_time = exp_time;
//  knob->exp_freq = exp_freq;
//  knob->num_values = num_values;
//  knob->probe = probe;
//  knob->arg = arg;
//  knob->set_value = set_value;
//  knob->get_value = get_value;
//  knob->value_name = value_name;
//  knob->adapting = 0;
//  
//  knob->status = WAITING;
//  gettimeofday(&(knob->interval_start), NULL);
//  knob->interval_length = INITAL_DELAY;
//
//  
//  int n = sizeof(int)*knob->num_values;
//  knob->wins = (int *)malloc(n);
//  memset(knob->wins, 0, n);
//  knob->exp_value = flip;
//}
//
//void knob_start(int_knob_t *knob) {
//  if (knob->exp_freq > 0) {
//    knob->adapting = 1;
//    pthread_create(&(knob->thread), NULL, (void *(*)(void *)) run, knob);
//  }
//}
//
//void knob_stop(int_knob_t *knob) {
//  if (knob->exp_freq > 0) {
//    assert(knob->adapting == 1);
//    pthread_mutex_lock(&knob->lock);
//    pthread_cancel((knob->thread));
//    pthread_join((knob->thread), NULL);
//    pthread_mutex_unlock(&knob->lock);
//    knob->adapting = 0;
//  }
//}
//
//void knob_destroy(int_knob_t *knob) {
//  assert(knob->adapting == 0);
//  pthread_mutex_destroy(&knob->lock);
//  free(knob->wins);
//}

