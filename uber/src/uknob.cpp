//
//  uknob.cpp
//  Combine-Test
//
//  Created by Stephen Freund on 7/18/17.
//  Copyright © 2017 Stephen Freund. All rights reserved.
//

#include <stdio.h>
#include "uknob.h"
#include <assert.h>
#include <pthread.h>
#include "util.h"
#include <random>
#include <string.h>
#include <algorithm>
#include <sys/time.h>
#include <map>

using namespace std;

#define DECL extern "C"

static void *run_it(void *x) {
  ((AsyncControl *)x)->run();
  return NULL;
}

DECL void AsyncControl::start() {
  if (exp_freq > 0) {
    adapting = true;
    pthread_create(&thread, NULL, run_it, this);
  }
};

DECL void AsyncControl::stop() {
  if (exp_freq > 0) {
    assert(adapting);
    adapting = false;
    pthread_cancel(thread);
    pthread_join(thread, NULL);
  }
};




DECL void AdaptControl::print_experiment(int orig_mode, double orig_value,
                                         int new_mode,  double new_value) {
  if (orig_mode <= new_mode) {
    printf("%d,%g  # measurement\n",orig_mode, orig_value);
    printf("%d,%g  # measurement\n",new_mode, new_value);
    printf("#%s %4s/%4s: %-8.2g         %-8.4g   %-8.4g\n", adapter->get_name().c_str(),
           adapter->mode_name(orig_mode).c_str(), adapter->mode_name(new_mode).c_str(),
           orig_value / new_value, orig_value, new_value);
  } else {
    print_experiment(new_mode, new_value, orig_mode, orig_value);
  }
}

// Default is pick best, provide enough samples
DECL void Adapter::complete_experiment(int orig_mode, double orig_value,
                                       int new_mode,  double new_value) {
  lock();
  
  if (uberprobe_less_than(probe, new_value, orig_value)) { //}  || orig_value / orig_interval * 1000 * 1000 < 100) {
    printf("%d, %d # choice (preferred/other)\n", orig_mode, new_mode);
    knob->set_mode(orig_mode);
  } else {
    printf("%d, %d # choice (preferred/other)\n", new_mode, orig_mode);
  }
  int mode = knob->get_mode();
  int index = mode - knob->get_lo();
  assert (0<= index && index < knob->get_num_modes());
  wins[mode - knob->get_lo()]++;
  unlock();
  LOG(("Mode is now: %d", mode));
}

DECL int Adapter::preferred_mode() {
  lock();
  int pref = 0;
  for (int i = 1; i < num_modes; i++) {
    if (wins[i] > wins[pref]) {
      pref = i;
    }
  }
  unlock();
  return pref + knob->get_lo();
}

DECL void adapter_mode_name(adapter_t *adapter, int mode, char buf[64]) {
  strncpy(buf, adapter->mode_name(mode).c_str(), 64);
}

DECL int adapter_preferred_mode(adapter_t *adapter) {
  return adapter->preferred_mode();
}

DECL void delete_adapter(adapter_t *adapter) {
  delete adapter;
}




DECL void AsyncControl::run() {
  Knob *knob = adapter->get_knob();
  uberprobe_t *probe = adapter->get_probe();
  
  LOG(("Launched control"));
  util_wait_ms(INITAL_DELAY);
  while (1) {
    LOG(("Starting Experiment for knob %s", "cow"));
    knob->experiment_starting();
    probe->start(probe);
    double orig_interval = util_wait(exp_time * 1000 * 1000);
    double orig_probe_value = probe->stop(probe, orig_interval);
    knob->experiment_ending();
    
    adapter->lock();
    int orig_value = knob->get_mode();
    int new_value = adapter->gen_mode(orig_value);
    knob->set_mode(new_value);
    adapter->unlock();
    
    util_wait_ms(exp_time); // to stabilize behavior...
    
    knob->experiment_starting();
    probe->start(probe);
    double new_interval = util_wait(exp_time * 1000 * 1000);
    double new_probe_value = probe->stop(probe, new_interval);
    knob->experiment_ending();
    
    print_experiment(orig_value, orig_probe_value,
                     new_value, new_probe_value);
    
    adapter->complete_experiment(orig_value, orig_probe_value,
                                 new_value, new_probe_value);
    util_wait_ms(exp_freq);
  }
}

static double elapsed_time(struct timeval *old, struct timeval *now) {
 double elapsed_time = (now->tv_sec - old->tv_sec) * 1000.0;
 elapsed_time += (now->tv_usec - old->tv_usec) / 1000.0;
 return elapsed_time;
}

DECL void SyncControl::safe_point() {
  struct timeval t2;
  gettimeofday(&t2, NULL);
  double elapsed = elapsed_time(&interval_start, &t2);
  if (elapsed < interval_length)
    return;
  Knob *knob = adapter->get_knob();
  uberprobe_t *probe = adapter->get_probe();

  switch (status)
  {
  case WAITING:
  {
    interval_start = t2;
    status = CUR_MEASURE;
    probe->start(probe);
    knob->experiment_starting();
    interval_length = exp_time;
    break;
  }
  case CUR_MEASURE:
  {
    orig_interval = elapsed;
    orig_probe_value = probe->stop(probe, orig_interval);
    knob->experiment_ending();
    orig_value = knob->get_mode();
    new_value = adapter->gen_mode(orig_value);
    knob->set_mode(new_value);
    interval_start = t2;
    status = PAUSE;
    break;
  }
  case PAUSE:
  {
    interval_start = t2;
    status = OTHER_MEASURE;
    probe->start(probe);
    knob->experiment_starting();
    interval_length = exp_time;
    break;
  }
  case OTHER_MEASURE:
  {
    double new_interval = elapsed;
    double new_probe_value = probe->stop(probe, new_interval);
    knob->experiment_ending();
    print_experiment(orig_value, orig_probe_value,
                     new_value, new_probe_value);

    adapter->complete_experiment(orig_value, orig_probe_value,
                                 new_value, new_probe_value);
    interval_start = t2;
    status = WAITING;
    interval_length = exp_freq;
    break;
  }
}
}

DECL void delete_sync_control(sync_control_t *control) { delete control; }
DECL void delete_async_control(async_control_t *control) { delete control; }

DECL sync_control_t *sync_control(adapter_t *adapter, size_t exp_time, size_t exp_freq) {
  return new SyncControl(adapter, exp_time, exp_freq);
}

DECL void sync_control_safe_point(sync_control_t *control) { control->safe_point(); }

DECL async_control_t *async_control(adapter_t *adapter, size_t exp_time, size_t exp_freq) {
  return new AsyncControl(adapter, exp_time, exp_freq);
}

DECL void async_control_start(async_control_t *control) { control->start(); }
DECL void async_control_stop(async_control_t *control) { control->stop(); }

DECL void async_stop(async_control_t *control) { control->stop(); }

DECL knob_t *make_knob(char * name,
                       void *arg,
                       int lo, int hi,
                       int (*a_get_mode)(void *arg),
                       void (*a_set_mode)(void *arg, int mode),
                       char *(*a_mode_name)(void *arg, int mode),
                       void (*a_experiment_starting)(void *arg),
                       void (*a_experiment_ending)(void *arg)) {
  return new Knob(name, arg, lo, hi, a_get_mode, a_set_mode, a_mode_name, a_experiment_starting, a_experiment_ending);
}


DECL void delete_knob(knob_t *knob) {
  delete knob;
}

class BinaryAdapter : public Adapter {
public:
  BinaryAdapter(Knob *knob, uberprobe_t *probe):
  Adapter(knob, probe) {
    assert(knob->get_num_modes() == 2);
  }
  int gen_mode(int cur_mode) { return cur_mode == knob->get_lo() ? knob->get_hi() : knob->get_lo(); }
};

DECL adapter_t *binary_adapter(knob_t *knob, uberprobe_t *probe) {
  return new BinaryAdapter(knob, probe);
}

class RandomAdapter : public Adapter {
protected:
  std::default_random_engine generator;
  std::uniform_int_distribution<int> distribution;
public:
  RandomAdapter(Knob *knob, uberprobe_t *probe):
  Adapter(knob, probe), generator(), distribution(knob->get_lo(), knob->get_hi()-1) { }
  
  virtual ~RandomAdapter() {
  }
  
  
  int gen_mode(int cur_mode) {
    int n = distribution(generator);
    if (n >= cur_mode) n++;
    return n;
  }
  
};

DECL adapter_t *random_adapter(knob_t *knob, uberprobe_t *probe) {
  return new RandomAdapter(knob, probe);
}


class DeterministcAdapter : public Adapter {
protected:
  double x;
public:
  DeterministcAdapter(Knob *knob, uberprobe_t *probe):
  Adapter(knob, probe) {
    x = 55;
  }
  
  virtual ~DeterministcAdapter() {
  }
  
  
  int gen_mode(int cur_mode) {
    x = x + 1;// 0.25;
    return int(x) % knob->get_num_modes() + knob->get_lo();
  }
  
};


DECL adapter_t *deterministic_adapter(knob_t *knob, uberprobe_t *probe) {
  return new DeterministcAdapter(knob, probe);
}


class BinarySearchAdapter : public Adapter {
protected:
  double temp = 1000;
  double cooling_rate = 0.1;
  std::default_random_engine generator;
  std::uniform_int_distribution<int> distribution;
  std::uniform_real_distribution<double> real_dist;

public:
  BinarySearchAdapter(Knob *knob, uberprobe_t *probe):
  Adapter(knob, probe), generator(), distribution(knob->get_lo(), knob->get_hi()-1), real_dist(0,1) { }
  
  virtual ~BinarySearchAdapter() {
  }

  virtual void complete_experiment(int orig_mode, double orig_value,
                                   int new_mode,  double new_value) {
    lock();
    double accept = acceptanceProbability(orig_value, new_value);
    printf("orig_value=%g  new_value=%g  new_better=%d, accept_prob=%g\n", orig_value, new_value, uberprobe_less_than(probe, orig_value, new_value), accept);
    if (accept < real_dist(generator)) {
      knob->set_mode(orig_mode);
    }
    int mode = knob->get_mode();
    int index = mode - knob->get_lo();
    assert (0<= index && index < knob->get_num_modes());
    wins[mode - knob->get_lo()]++;
    unlock();
    LOG(("Mode is now: %d", mode));
  }
  
  double acceptanceProbability(double orig_value, double new_value) {
    // If the new solution is better, accept it
    if (uberprobe_less_than(probe, orig_value, new_value)) {
      return 1.0;
    }
    // If the new solution is worse, calculate an acceptance probability
    if (orig_value > new_value) {
      return exp((new_value - orig_value) / temp);
    } else {
      return exp((orig_value - new_value) / temp);
    }
  }

  
  int neighbor(int cur_mode) {
    int n = distribution(generator);
    if (n >= cur_mode) n++;
    return n;
  }
  
  int gen_mode(int cur_mode) {
    temp *= (1 - cooling_rate);
    return neighbor(cur_mode);
  }
  
};


DECL adapter_t *binary_search_adapter(knob_t *knob, uberprobe_t *probe) {
  return new BinarySearchAdapter(knob, probe);
}


class LastNAdapter : public RandomAdapter {
protected:
  std::map<int,int> count;
  std::map<int,double> average;
  
public:
  LastNAdapter(Knob *knob, uberprobe_t *probe):
  RandomAdapter(knob, probe) {
  }
  
  virtual ~LastNAdapter() {
  }
  
  virtual void complete_experiment(int orig_mode, double orig_value,
                                   int new_mode,  double new_value) {
    lock();
    if (count[orig_mode] < 3) count[orig_mode]++;
    if (count[new_mode] < 3) count[new_mode]++;
    average[orig_mode] = (average[orig_mode] * (count[orig_mode] - 1) + orig_value) / count[orig_mode];
    average[new_mode] = (average[new_mode] * (count[new_mode] - 1) + new_value) / count[new_mode];
    unlock();
    RandomAdapter::complete_experiment(orig_mode, average[orig_mode], new_mode, average[new_mode]);
  }
  
};


DECL adapter_t *lastn_adapter(knob_t *knob, uberprobe_t *probe) {
  return new LastNAdapter(knob, probe);
}


DECL int default_exp_freq(void) {
  char *freq_env = getenv("UBER_DEFAULT_FREQ");   
  int freq = (freq_env == NULL) ? 0 : atoi(freq_env);  
  return freq;
}

DECL int default_exp_time(void) {
  char *time_env = getenv("UBER_DEFAULT_TIME");    
  int time = (time_env == NULL) ? 10 : atoi(time_env);    
  return time;   
}