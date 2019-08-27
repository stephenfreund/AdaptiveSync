//#ifndef knob_h
//#define knob_h
//
//#include <stdio.h>
//#include "probe.h"
//#include <pthread.h>
//
//#ifdef __cplusplus
//extern "C" {
//#endif
//
//
//#define MAX_NAME_SIZE 10
//
//#define INITAL_DELAY 100
//
//typedef enum { CUR_MEASURE, PAUSE, OTHER_MEASURE, WAITING } knob_status_t;
//
//typedef struct int_knob {
//  char name[64];
//  size_t exp_time;
//  size_t exp_freq;
//  
//  int num_values;
//  uberprobe_t *probe;
//  
//  void *arg;
//  void  (*set_value)(void *arg, int value, int at_safe_point);
//  int   (*get_value)(void *arg);
//  char *(*value_name)(void *arg, int value, char *name);
//  
//  int   (*exp_value)(struct int_knob *knob, int value);
//  int *wins;
//  
//  pthread_mutex_t lock; // ??
//  int adapting;
//  
//  union {
//    pthread_t thread;
//    struct {
//      knob_status_t status;
//      size_t orig_probe_value;
//      double orig_interval;
//      int orig_value;
//      int new_value;
//      size_t probe_start;
//      struct timeval interval_start;
//      double interval_length;
//    };
//  };
//  
//} int_knob_t;
//
//
//
//char *knob_default_name(void *dummy, int value, char *name);
//
//void knob_binary_init(int_knob_t *knob, char *name, size_t exp_time, size_t exp_freq, int num_values,
//                      uberprobe_t *probe,
//                      void *arg, void (*set_value)(void *arg, int value, int at_safe_point),
//                      int (*get_value)(void *arg),
//                      char *(*)(void *arg, int value, char *name));
//
//void knob_destroy(int_knob_t *knob);
//
//void knob_start(int_knob_t *knob);
//void knob_stop(int_knob_t *knob);
//
//void knob_safe_point(int_knob_t *knob);
//
//char *knob_value_name(int_knob_t *knob, int value, char *name);
//int knob_preferred_mode(int_knob_t *knob);
//
//#ifdef __cplusplus
//  }
//#endif
//
//  
//#endif /* knob_h */

