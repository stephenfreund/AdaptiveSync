/*
 * test-exp.c
 *
 *  Created on: Jan 13, 2017
 *      Author: freund
 */

#include <pthread.h>
#include "uberlock.h"
#include "util.h"
#include <random>
#include <time.h>
#include <iostream>
#include "ppoint.h"
#include <vector>
#include <iostream>
#include <chrono>
#include <thread>
#include <atomic>
#include "gflags/gflags.h"
#include "locks.h"
#include "workload.h"
#include "worker.h"
#include "flags.h"
#include <string.h>
#include "ubergroup-generic.h"
#include "ubergroup-specialized.h"
#include "spin.h"

 using namespace std;

void my_sleep(int millis) {
  std::this_thread::sleep_for(std::chrono::milliseconds(millis));
}

WorkLoad *makeWorkLoad() {
	if (FLAGS_load == "Basic") {
		return new BasicWorkLoad();
	} else if (FLAGS_load == "Vector") {
		return new VectorWorkLoad(1024 * 10);
	}
	return 0;
}

template<typename T>
void experiment(T *lock) {

	std::vector<Worker<T>*> workers;
	std::vector<std::thread*> threads;
	std::atomic<int> *flag = new std::atomic<int>();
	double *d;
	d = new double[FLAGS_threads];
	*flag = WARMUP;
	WorkLoad *load = makeWorkLoad();
	for (int i = 0; i < FLAGS_threads; i++) {
	  void *buffer = aligned_alloc(1024, 1024 * (sizeof(Worker<T>)/1024 + 1));
	  workers.push_back(
			    !FLAGS_pp ?
			    new(buffer) Worker<T>(i, lock, load, flag) :
			    new(buffer) PPWorker<T>(i, lock, load, flag,
						    default_ppoint()));
	  thread *t = new thread(&Worker<T>::run, workers[i], &d[i]);
	  threads.push_back(t);

#ifndef __APPLE__
		// Create a cpu_set_t object representing a set of CPUs. Clear it and mark
		// only CPU i as set.
		cpu_set_t cpuset;
		CPU_ZERO(&cpuset);
		CPU_SET(i, &cpuset);
		int rc = pthread_setaffinity_np(t->native_handle(),
						sizeof(cpu_set_t), &cpuset);
		if (rc != 0) {
		  cout << "# Yikes -- didn't set affinity -- " << rc <<  endl;
		}
#endif
    
	}

	if (!FLAGS_quiet)
		cerr << "  Warmup" << endl;
	my_sleep(FLAGS_warmup * 1000);
	*flag = MEASURE;
	if (!FLAGS_quiet)
		cerr << "  Measure" << endl;
	my_sleep(FLAGS_measure * 1000);
	*flag = WRAPUP;
	if (!FLAGS_quiet)
		cerr << "  Wrapup" << endl;
	string mode = lock->preferred_mode();
	my_sleep(FLAGS_wrapup * 1000);
	*flag = DONE;
	if (!FLAGS_quiet)
		cerr << "  Done" << endl;

	double aveThroughput = 0;
	for (int i = 0; i < FLAGS_threads; i++) {
		threads[i]->join();
		aveThroughput += d[i];  // ordered by join
	}
	double throughput = aveThroughput * 1000 * 1000 * 1000;

	for (int i = 0; i < FLAGS_threads; i++) {
		delete workers[i];
		delete threads[i];
	}
	delete[] d;
	delete load;
	delete flag;

	cout << lock->name() << "\t" << mode << "\t" << FLAGS_threads << "\t"
			<< FLAGS_writePercent << "\t" << FLAGS_writeDelay << "\t"
			<< FLAGS_readDelay << "\t" << FLAGS_outsideDelay << "\t"
			<< FLAGS_expFreq << "\t" << FLAGS_expTime << "\t" << throughput
			<< endl;
}

#include <cstddef>

extern int spin_success, spin_fail;

int main(int argc, char *argv[]) {

	gflags::ParseCommandLineFlags(&argc, &argv, true);

	if (FLAGS_expFreq > 0) {
		FLAGS_pp = 1;
	}

	void* buffer = aligned_alloc(1024, 1024);

	if (FLAGS_lock == "Mutex") {
	  Mutex *lock = new(buffer) Mutex();
		experiment(lock);
		delete lock;
	} else if (FLAGS_lock == "RW") {
	  RWLock *lock = new(buffer) RWLock();
		experiment(lock);
		delete lock;
	} else if (FLAGS_lock == "Spin") {
		SpinLock *lock = new(buffer) SpinLock();
		experiment(lock);
		delete lock;
#ifndef __APPLE__
	} else if (FLAGS_lock == "SpinRTM") {
		SpinRTMLock *lock = new(buffer) SpinRTMLock();
		experiment(lock);
		delete lock;
#endif
  } else if (FLAGS_lock == "MRW" || FLAGS_lock == "MRWRW") {
    MutexRWLock *lock = new(buffer) MutexRWLock();
    mutex_rw_group_t *group = mutex_rw_group_default();
    mutex_rw_group_set_mode(group, FLAGS_lock == "MRW" ? MUTEX_MODE : RW_MODE);
    experiment(lock);
    delete lock;
  } else if (FLAGS_lock == "SM" || FLAGS_lock == "SMRW") {
    MutexSpinLock *lock = new(buffer) MutexSpinLock();
    mutex_spin_group_t *group = mutex_spin_group_default();
    mutex_spin_group_set_mode(group, FLAGS_lock == "SM" ? MUTEX_MODE : RW_MODE);
    experiment(lock);
    delete lock;
#ifndef __APPLE__
  } else if (FLAGS_lock == "MRW2" || FLAGS_lock == "MRWRW2") {
      mutex_rw_1_group_t *group = &mutex_rw_1_group;
      MutexRWLockDEFAULT *lock = new(buffer) MutexRWLockDEFAULT();
      mutex_rw_1_group_set_mode(group, FLAGS_lock == "MRW2" ? MUTEX_MODE : RW_MODE);
      experiment(lock);
      delete lock;
	} else if (FLAGS_lock == "SM2" || FLAGS_lock == "SMRW2") {
	  mutex_spin_1_group_t *group = &mutex_spin_1_group;
	  MutexSpinLockDEFAULT *lock = new(buffer) MutexSpinLockDEFAULT();
	  mutex_spin_1_group_set_mode(group, FLAGS_lock == "SM2" ? MUTEX_MODE : RW_MODE);
	  experiment(lock);
	  delete lock;
#endif
	} else {
		cerr << "BAD LOCK " << FLAGS_lock << endl;
		abort();
		return 0;
	}
}

