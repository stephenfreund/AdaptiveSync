//
//  uadapter.h
//  SCORE
//
//  Created by Stephen Freund on 7/18/17.
//  Copyright © 2017 Stephen Freund. All rights reserved.
//

#ifndef uadapter_h
#define uadapter_h

#include <stdio.h>
#include "probe.h"
#include <pthread.h>

#ifdef __cplusplus
#include <mutex>
#include <thread>
#include <string>
#define DECL extern "C"
//using namespace std;

#define MAX_NAME_SIZE 10
#define INITAL_DELAY 100

DECL typedef enum { CUR_MEASURE, PAUSE, OTHER_MEASURE, WAITING } adapter_status_t;

DECL struct Knob {
protected:
    std::string name;
    int lo, hi;
    void *arg;
    int (*a_get_mode)(void *arg);
    void (*a_set_mode)(void *arg, int mode);
    char *(*a_mode_name)(void *arg, int mode);
    void (*a_experiment_starting)(void *arg);
    void (*a_experiment_ending)(void *arg);
    
public:
    Knob(std::string name,
         void *arg,
         int lo, int hi,
         int (*a_get_mode)(void *arg),
         void (*a_set_mode)(void *arg, int mode),
         char *(*a_mode_name)(void *arg, int mode),
         void (*a_experiment_starting)(void *arg),
         void (*a_experiment_ending)(void *arg)): name(name), lo(lo), hi(hi), arg(arg), a_get_mode(a_get_mode), a_set_mode(a_set_mode),
    a_mode_name(a_mode_name),
    a_experiment_starting(a_experiment_starting), a_experiment_ending(a_experiment_ending) { }
    
    std::string get_name() { return name; }
    int get_mode() { return a_get_mode(arg); }
    void set_mode(int mode) { a_set_mode(arg, mode); }
  int get_lo() { return lo; }
  int get_hi() { return hi; }
  int get_num_modes() { return hi - lo + 1; }
    
    std::string mode_name(int mode) {
        return a_mode_name != NULL ? a_mode_name(arg, mode) : std::to_string(mode);
    }
    void experiment_starting() { if (a_experiment_starting != NULL) a_experiment_starting(arg); }
    void experiment_ending() { if (a_experiment_ending != NULL) a_experiment_ending(arg); }
};


DECL struct Adapter {
protected:
    Knob *knob;
    std::mutex mutex;

    int num_modes;
    uberprobe_t *probe;
    int *wins;
    
public:
    Adapter(Knob *knob, uberprobe_t *probe): knob(knob), mutex(), num_modes(knob->get_num_modes()), probe(probe) {
        wins = new int[num_modes];
    }
    virtual ~Adapter() {
        delete wins;
    }
    int preferred_mode();
    std::string get_name() { return knob->get_name(); }
    Knob *get_knob() { return knob; }
    uberprobe_t *get_probe() { return probe; }
    std::string mode_name(int mode) { return knob->mode_name(mode); }
    std::string current_mode_name() { return knob->mode_name(knob->get_mode()); }
    
    void lock() { mutex.lock(); }
    void unlock() { mutex.unlock(); }

    virtual int gen_mode(int cur_mode) = 0;
    
    virtual void complete_experiment(int orig_mode, double orig_value,
                                     int new_mode,  double new_value);
    
};

DECL struct AdaptControl {
protected:
    Adapter *adapter;
    size_t exp_time;
    size_t exp_freq;
    
public:
    AdaptControl(Adapter *adapter, size_t exp_time, size_t exp_freq):
    adapter(adapter), exp_time(exp_time), exp_freq(exp_freq) {    }
    virtual ~AdaptControl() { }
    void start();
    void stop();
    void run();
    void print_experiment(int orig_mode, double orig_value, int new_mode,  double new_value);

};

DECL struct AsyncControl : public AdaptControl {
protected:
    bool adapting;
    pthread_t thread;
public:
    AsyncControl(Adapter *adapter, size_t exp_time, size_t exp_freq):
    AdaptControl(adapter, exp_time, exp_freq) {
        adapting = false;
    }
    virtual ~AsyncControl() {
    }
    void start();
    void stop();
    void run();
};


DECL struct SyncControl : public AdaptControl {
protected:
    adapter_status_t status;
    size_t orig_probe_value;
    double orig_interval;
    int orig_value;
    int new_value;
    size_t probe_start;
    struct timeval interval_start;
    double interval_length;
    
public:
    SyncControl(Adapter *adapter, size_t exp_time, size_t exp_freq):
    AdaptControl(adapter, exp_time, exp_freq) {  }
    virtual ~SyncControl() { }
    virtual void safe_point();
    
};


typedef struct Knob knob_t;
typedef struct Adapter adapter_t;
typedef struct SyncControl sync_control_t;
typedef struct AsyncControl async_control_t;

#else

#define DECL

typedef struct Knob knob_t;
typedef struct Adapter adapter_t;
typedef struct SyncControl sync_control_t;
typedef struct AsyncControl async_control_t;

#endif

DECL knob_t *make_knob(char * name,
                       void *arg,
                       int lo, int hi,
                       int (*a_get_mode)(void *arg),
                       void (*a_set_mode)(void *arg, int mode),
                       char *(*a_mode_name)(void *arg, int mode),
                       void (*a_experiment_starting)(void *arg),
                       void (*a_experiment_ending)(void *arg));
DECL void delete_knob(knob_t *knob);

DECL adapter_t *binary_adapter(knob_t *knob, uberprobe_t *probe);
DECL adapter_t *random_adapter(knob_t *knob, uberprobe_t *probe);
DECL adapter_t *deterministic_adapter(knob_t *knob, uberprobe_t *probe);
DECL adapter_t *binary_search_adapter(knob_t *knob, uberprobe_t *probe);
DECL adapter_t *lastn_adapter(knob_t *knob, uberprobe_t *probe);


DECL void adapter_mode_name(adapter_t *adapter, int mdoe, char buf[64]);
DECL int adapter_preferred_mode(adapter_t *adapter);
DECL void delete_adapter(adapter_t *adapter);

DECL sync_control_t *sync_control(adapter_t *adapter, size_t exp_time, size_t exp_freq);
DECL async_control_t *async_control(adapter_t *adapter, size_t exp_time, size_t exp_freq);
DECL async_control_t *async_binary_control(adapter_t *adapter, size_t exp_time, size_t exp_freq);
DECL void sync_control_safe_point(sync_control_t *control);
DECL void async_control_start(async_control_t *control);
DECL void async_control_stop(async_control_t *control);
DECL void delete_sync_control(sync_control_t *control);
DECL void delete_async_control(async_control_t *control);

DECL int default_exp_freq(void);
DECL int default_exp_time(void);


#endif /* ucontrol_h */
