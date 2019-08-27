
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
#include "gflags/gflags.h"
#include <mutex>
#include "ubergroup.h"
//#include <shared_mutex>
//#include "uberlock2.h"

#include "combine.h"

EXTERN_GROUP(DEFAULT);

using namespace std;

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
 protected:
  std::atomic_flag flag = ATOMIC_FLAG_INIT;
 private:
  inline void lock() {
    while (flag.test_and_set()) {
#if defined(__i386__) || defined(__x86_64__)
      /*
       * NOTE: "rep nop" works on all Intel architectures and has the same
       * encoding as "pause" on the newer ones.
       */
      __asm__ __volatile__ ("rep nop");
#else
      /* nothing */
#endif
    }
  }

 public:
  SpinLock() {

  }
  ~SpinLock() {
  }
  inline void wrlock() {
    lock();
  }
  inline void rdlock() {
    lock();
  }
  inline void rd_unlock() {
    flag.clear();
  }
  inline void wr_unlock() {
    flag.clear();
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

class UberLock {
 protected:
  uberlock_t *lock;
  ubergroup_t *group;
 public:
 UberLock(ubergroup_t *group) : group(group) {
    lock = ALLOC(uberlock_t);
    uberlock_init(lock, group);
  }
  ~UberLock() {
    uberlock_destroy(lock);
    FREE(lock);
  }
  inline void wrlock() {
    uberlock_wrlock(lock);
  }
  inline void rdlock() {
    uberlock_rdlock(lock);
  }
  inline void wr_unlock() {
    uberlock_unlock(lock);
  }
  inline void rd_unlock() {
    uberlock_unlock(lock);
  }
  string name() {
    return "Uber";
  }

  string mode() {
    int mode = ubergroup_get_mode(group);
    if (mode == MUTEX_MODE) {
      return "Mutex";
    } else {
      return "RW";
    }
  }
 	
  string preferred_mode() {
    int mutex_won = ubergroup_get_wins(group, MUTEX_MODE);
    int rw_won = ubergroup_get_wins(group, RW_MODE);
    int mode;
    if (mutex_won == rw_won) {
      mode = ubergroup_get_mode(group);
    } else {
      mode = (mutex_won > rw_won) ? MUTEX_MODE : RW_MODE;
    }
    if (mode == MUTEX_MODE) {
      return "Mutex";
    } else {
      return "RW";
    }
  }

};


class UberLockDEFAULT {
 protected:
  uberlock_t *lock;
  ubergroup_t *group;
 public:
 UberLockDEFAULT(ubergroup_t *group) : group(group) {
    lock = ALLOC(uberlock_t);
    uberlock_init(lock, group);
  }
  ~UberLockDEFAULT() {
    uberlock_destroy(lock);
    FREE(lock);
  }
  inline void wrlock() {
    uber_wrlock_special(DEFAULT, lock);
  }
  inline void rdlock() {
    uber_rdlock_special(DEFAULT, lock);
  }
  inline void wr_unlock() {
    uber_unlock_special(DEFAULT, lock);
  }
  inline void rd_unlock() {
    uber_unlock_special(DEFAULT, lock);
  }
  string name() {
    return "UberMod";
  }

  string mode() {
    int mode = ubergroup_get_mode(group);
    if (mode == MUTEX_MODE) {
      return "Mutex";
    } else {
      return "RW";
    }
  }
 	
  string preferred_mode() {
    int mutex_won = ubergroup_get_wins(group, MUTEX_MODE);
    int rw_won = ubergroup_get_wins(group, RW_MODE);
    int mode;
    if (mutex_won == rw_won) {
      mode = ubergroup_get_mode(group);
    } else {
      mode = (mutex_won > rw_won) ? MUTEX_MODE : RW_MODE;
    }
    if (mode == MUTEX_MODE) {
      return "Mutex";
    } else {
      return "RW";
    }
  }

};

/*
class UberLock2 {
 protected:
  uberlock2_t *lock;
  ubergroup2_t *group;
 public:
 UberLock2(ubergroup2_t *group) : group(group) {
    lock = ALLOC(uberlock2_t);
    uberlock2_init(lock, group);
  }
  ~UberLock2() {
    uberlock2_destroy(lock);
    FREE(lock);
  }
  inline void wrlock() {
    uberlock2_wrlock(lock);
  }
  inline void rdlock() {
    uberlock2_rdlock(lock);
  }
  inline void wr_unlock() {
    uberlock2_wrunlock(lock);
  }
  inline void rd_unlock() {
    uberlock2_rdunlock(lock);
  }
  string name() {
    return "Uber2";
  }

  string mode() {
    int mode = ubergroup2_get_mode(group);
    if (mode == MUTEX_MODE) {
      return "Mutex";
    } else {
      return "RW";
    }
  }
 

  string preferred_mode() {
    int mutex_won = ubergroup2_get_wins(group, MUTEX_MODE);
    int rw_won = ubergroup2_get_wins(group, RW_MODE);
    int mode;
    if (mutex_won == rw_won) {
      mode = ubergroup2_get_mode(group);
    } else {
      mode = (mutex_won > rw_won) ? MUTEX_MODE : RW_MODE;
    }
    if (mode == MUTEX_MODE) {
      return "Mutex";
    } else {
      return "RW";
    }
  }

};
*/


DEFINE_LOCK(test,
	    pthread_rwlock_t,
	    "RW",
	    pthread_rwlock_init,
	    pthread_rwlock_destroy,
	    pthread_rwlock_wrlock,
	    pthread_rwlock_rdlock,
	    pthread_rwlock_unlock,
	    pthread_mutex_t,
	    "MU",
	    pthread_mutex_init,
	    pthread_mutex_destroy,
	    pthread_mutex_lock,
	    pthread_mutex_lock,
	    pthread_mutex_unlock);


class TestLock {
 protected:
  testlock_t *lock;
  testgroup_t *group;
 public:
 TestLock(testgroup_t *group) : group(group) {
    lock = ALLOC(testlock_t);
    testlock_init(lock, group);
  }
  ~TestLock() {
    testlock_destroy(lock);
    FREE(lock);
  }
  inline void wrlock() {
    testlock_wrlock(lock);
  }
  inline void rdlock() {
    testlock_rdlock(lock);
  }
  inline void wr_unlock() {
    testlock_unlock(lock);
  }
  inline void rd_unlock() {
    testlock_unlock(lock);
  }
  string name() {
    return "Test";
  }

  string mode() {
    return testgroup_name_for_mode(testgroup_get_mode(group));
  }
 	
  string preferred_mode() {
    int mutex_won = testgroup_get_wins(group, MUTEX_MODE);
    int rw_won = testgroup_get_wins(group, RW_MODE);
    int mode;
    if (mutex_won == rw_won) {
      mode = testgroup_get_mode(group);
    } else {
      mode = (mutex_won > rw_won) ? MUTEX_MODE : RW_MODE;
    }
    return testgroup_name_for_mode(mode);   
  }

};



#endif
