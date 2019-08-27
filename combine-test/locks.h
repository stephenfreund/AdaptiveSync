
#ifndef LOCKS_H_
#define LOCKS_H_

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
#include <mutex>
#include "ubergroup.h"
#include "spin.h"
#include "ubergroup-generic.h"
#include "ubergroup-specialized.h"
#include <cstdlib>
#include "uknob.h"
#include "flags.h"

using namespace std;

#define aligned_alloc(A,N) \
malloc( (N + (A-1)) & ~(A-1) )

#define ALLOC(X) (X*)aligned_alloc(1024, 1024*(((sizeof(X)) + 1024)/1024));
#define FREE(X)  free(X);

class Mutex {
protected:
    pthread_mutex_t *lock;
public:
    Mutex() {
        lock = ALLOC(pthread_mutex_t);
        pthread_mutex_init(lock, NULL);
    }
    ~Mutex() {
        pthread_mutex_destroy(lock);
        FREE(lock);
    }
    inline void wrlock() {
        pthread_mutex_lock(lock);
    }
    inline void rdlock() {
        pthread_mutex_lock(lock);
    }
    inline void wr_unlock() {
        pthread_mutex_unlock(lock);
    }
    inline void rd_unlock() {
        pthread_mutex_unlock(lock);
    }
    string name() {
        return "Mutex";
    }
    string mode() {
        return "Mutex";
    }
    string preferred_mode() {
        return mode();
    }
};

class RWLock {
public:
    pthread_rwlock_t *lock;
public:
    RWLock() {
        lock = ALLOC(pthread_rwlock_t);
        pthread_rwlock_init(lock, NULL);
    }
    ~RWLock() {
        pthread_rwlock_destroy(lock);
        FREE(lock);
    }
    inline void wrlock() {
        pthread_rwlock_wrlock(lock);
    }
    inline void rdlock() {
        pthread_rwlock_rdlock(lock);
    }
    inline void wr_unlock() {
        pthread_rwlock_unlock(lock);
    }
    inline void rd_unlock() {
        pthread_rwlock_unlock(lock);
    }
    string name() {
        return "RW";
    }
    string mode() {
        return "RW";
    }
    string preferred_mode() {
        return mode();
    }
};


class SpinLock {
public:
    spinlock_t *lock;
public:
    SpinLock() {
        lock = ALLOC(spinlock_t);
        spinlock_init(lock, NULL);
    }
    ~SpinLock() {
        spinlock_destroy(lock);
        FREE(lock);
    }
    inline void wrlock() {
        spinlock_lock(lock);
    }
    inline void rdlock() {
        spinlock_lock(lock);
    }
    inline void wr_unlock() {
        spinlock_unlock(lock);
    }
    inline void rd_unlock() {
        spinlock_unlock(lock);
    }
    string name() {
        return "Spin";
    }
    string mode() {
        return "Spin";
    }
    string preferred_mode() {
        return mode();
    }
};


class SpinRTMLock {
public:
    spinlock_t *lock;
public:
    SpinRTMLock() {
        lock = ALLOC(spinlock_t);
        spinlock_init_rtm(lock, NULL);
    }
    ~SpinRTMLock() {
        spinlock_destroy_rtm(lock);
        FREE(lock);
    }
    inline void wrlock() {
        spinlock_lock_rtm(lock);
    }
    inline void rdlock() {
        spinlock_lock_rtm(lock);
    }
    inline void wr_unlock() {
        spinlock_unlock_rtm(lock);
    }
    inline void rd_unlock() {
        spinlock_unlock_rtm(lock);
    }
    string name() {
        return "SpinRTM";
    }
    string mode() {
        return "Spin";
    }
    string preferred_mode() {
        return mode();
    }
};


DECLARE_COMBINED_LOCK(mutex_rw, pthread_mutex_t, pthread_rwlock_t);


class MutexRWLock {
protected:
    mutex_rw_lock_t *lock;
    mutex_rw_group_t *group;
    knob_t *knob;
    uberprobe_t probe;
    adapter_t *adapter;
    async_control_t *controller;
    
public:
    MutexRWLock() : group(mutex_rw_group_default()) {
        lock = ALLOC(mutex_rw_lock_t);
        mutex_rw_lock_init(lock, group);
        throughput_probe_init(&probe, default_ppoint());
        knob = mutex_rw_group_knob(group);
        
        adapter = binary_adapter(knob, &probe);
        controller = async_control(adapter, FLAGS_expTime, FLAGS_expFreq);
        async_control_start(controller);
    }
    ~MutexRWLock() {
        async_control_stop(controller);
        delete_async_control(controller);
        delete_adapter(adapter);
        delete_knob(knob);
        mutex_rw_lock_destroy(lock);
        FREE(lock);
    }
    inline void wrlock() {
        mutex_rw_lock_wrlock(lock);
    }
    inline void rdlock() {
        mutex_rw_lock_rdlock(lock);
    }
    inline void wr_unlock() {
        mutex_rw_lock_unlock(lock);
    }
    inline void rd_unlock() {
        mutex_rw_lock_unlock(lock);
    }
    string name() {
        return "mutex_rw_";
    }
    
    string mode() {
        int mode = mutex_rw_group_get_mode(group);
        return string(mutex_rw_group_name_for_mode(group, mode));
    }
    
    
    string preferred_mode() {
        int pref = adapter_preferred_mode(adapter);
        return string(mutex_rw_group_name_for_mode(group, pref));
    }
    
};


DECLARE_COMBINED_LOCK(mutex_spin, pthread_mutex_t, spinlock_t);

class MutexSpinLock {
protected:
    mutex_spin_lock_t *lock;
    mutex_spin_group_t *group;
    knob_t *knob;
    uberprobe_t probe;
    adapter_t *adapter;
    async_control_t *controller;

public:
    MutexSpinLock() : group(mutex_spin_group_default()) {
        lock = ALLOC(mutex_spin_lock_t);
        mutex_spin_lock_init(lock, group);
        throughput_probe_init(&probe, default_ppoint());
        knob = mutex_spin_group_knob(group);
        
        adapter = binary_adapter(knob, &probe);
        controller = async_control(adapter, FLAGS_expTime, FLAGS_expFreq);
        async_control_start(controller);
    }
    
    ~MutexSpinLock() {
        async_control_stop(controller);
        delete_async_control(controller);
        delete_adapter(adapter);
        mutex_spin_lock_destroy(lock);
        FREE(lock);
    }
    inline void wrlock() {
        mutex_spin_lock_wrlock(lock);
    }
    inline void rdlock() {
        mutex_spin_lock_rdlock(lock);
    }
    inline void wr_unlock() {
        mutex_spin_lock_unlock(lock);
    }
    inline void rd_unlock() {
        mutex_spin_lock_unlock(lock);
    }
    string name() {
        return "mutex_spin_";
    }
    
    string mode() {
        int mode = mutex_spin_group_get_mode(group);
        return string(mutex_spin_group_name_for_mode(group, mode));
    }
    
    
    string preferred_mode() {
        int pref = adapter_preferred_mode(adapter);
        return string(mutex_spin_group_name_for_mode(group, pref));
    }
    
};

#ifndef __APPLE__

DECLARE_SPECIALIZED_LOCK(mutex_rw_1, pthread_mutex_t, pthread_rwlock_t);
DECLARE_SPECIALIZED_LOCK(mutex_spin_1, pthread_mutex_t, spinlock_t);

class MutexRWLockDEFAULT {
protected:
    mutex_rw_1_lock_t *lock;
    knob_t *knob;
    uberprobe_t probe;
    adapter_t *adapter;
    async_control_t *controller;
    
public:
    MutexRWLockDEFAULT() {
        lock = ALLOC(mutex_rw_1_lock_t);
        mutex_rw_1_init(lock);

        throughput_probe_init(&probe, default_ppoint());
        knob = mutex_rw_1_group_knob(&mutex_rw_1_group);
        
        adapter = binary_adapter(knob, &probe);
        controller = async_control(adapter, FLAGS_expTime, FLAGS_expFreq);
        async_control_start(controller);

	/*
        throughput_probe_init(&probe, default_ppoint());
        knob_binary_init(&knob, (char*)"LockMode", FLAGS_expTime, FLAGS_expFreq, 2, &probe, &mutex_rw_1_group,
                         (void (*)(void *, int, int))mutex_rw_1_group_set_mode,
                         (int (*)(void *))mutex_rw_1_group_get_mode,
                         (char *(*)(void *, int, char*))mutex_rw_1_group_name_for_mode);
			 knob_start(&knob); */
    }
    
    ~MutexRWLockDEFAULT() {
        async_control_stop(controller);
        delete_async_control(controller);
        delete_adapter(adapter);
        FREE(lock);
    }
    inline void wrlock() {
        mutex_rw_1_wrlock(lock);
    }
    inline void rdlock() {
        mutex_rw_1_rdlock(lock);
    }
    inline void wr_unlock() {
        mutex_rw_1_unlock(lock);
    }
    inline void rd_unlock() {
        mutex_rw_1_unlock(lock);
    }
    string name() {
        return "mutex_rw_1_";
    }
    
    string mode() {
        int mode = mutex_rw_1_group_get_mode(&mutex_rw_1_group);
        return string(mutex_rw_1_group_name_for_mode(&mutex_rw_1_group, mode));
    }
    
    
    string preferred_mode() {
        int pref = adapter_preferred_mode(adapter);
        return string(mutex_rw_1_group_name_for_mode(&mutex_rw_1_group, pref));
    }
    
};



class MutexSpinLockDEFAULT {
protected:
    mutex_spin_1_lock_t *lock;
    knob_t *knob;
    uberprobe_t probe;
    adapter_t *adapter;
    async_control_t *controller;
    
public:
    MutexSpinLockDEFAULT() {
        lock = ALLOC(mutex_spin_1_lock_t);
        mutex_spin_1_init(lock);

        throughput_probe_init(&probe, default_ppoint());
        knob = mutex_spin_1_group_knob(&mutex_spin_1_group);
        
        adapter = binary_adapter(knob, &probe);
        controller = async_control(adapter, FLAGS_expTime, FLAGS_expFreq);
        async_control_start(controller);
    }
    
    ~MutexSpinLockDEFAULT() {
        async_control_stop(controller);
        delete_async_control(controller);
        delete_adapter(adapter);
        FREE(lock);
    }
    inline void wrlock() {
        mutex_spin_1_wrlock(lock);
    }
    inline void rdlock() {
        mutex_spin_1_rdlock(lock);
    }
    inline void wr_unlock() {
        mutex_spin_1_unlock(lock);
    }
    inline void rd_unlock() {
        mutex_spin_1_unlock(lock);
    }
    string name() {
        return "mutex_spin_1_";
    }

    
    string mode() {
        int mode = mutex_spin_1_group_get_mode(&mutex_spin_1_group);
        return string(mutex_spin_1_group_name_for_mode(&mutex_spin_1_group, mode));
    }
    
    
    string preferred_mode() {
        int pref = adapter_preferred_mode(adapter);
        return string(mutex_spin_1_group_name_for_mode(&mutex_spin_1_group, pref));
    }
    
};

#endif

#endif
