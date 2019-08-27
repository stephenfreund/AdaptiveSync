
#include "ubergroup-generic.h"
#include "ubergroup-specialized.h"
#include "spin.h"
#include "pthread.h"

DEFINE_COMBINED_LOCK(mutex_rw,
                     pthread_mutex_t, "Mutex", pthread_mutex_init, pthread_mutex_destroy, pthread_mutex_lock, pthread_mutex_lock, pthread_mutex_unlock,
                     pthread_rwlock_t, "RW", pthread_rwlock_init, pthread_rwlock_destroy, pthread_rwlock_wrlock, pthread_rwlock_rdlock, pthread_rwlock_unlock);

DEFINE_COMBINED_LOCK(mutex_spin,
                     pthread_mutex_t, "Mutex", pthread_mutex_init, pthread_mutex_destroy, pthread_mutex_lock, pthread_mutex_lock, pthread_mutex_unlock,
                     spinlock_t, "Spin", spinlock_init, spinlock_destroy, spinlock_lock, spinlock_lock, spinlock_unlock);

#ifndef __APPLE__

DEFINE_SPECIALIZED_LOCK(mutex_rw_1,
                        pthread_mutex_t, "Mutex", pthread_mutex_init, pthread_mutex_destroy, pthread_mutex_lock, pthread_mutex_lock, pthread_mutex_unlock,
                        pthread_rwlock_t, "RW", pthread_rwlock_init, pthread_rwlock_destroy, pthread_rwlock_wrlock, pthread_rwlock_rdlock, pthread_rwlock_unlock);

DEFINE_SPECIALIZED_LOCK(mutex_spin_1,
                        pthread_mutex_t, "Mutex", pthread_mutex_init, pthread_mutex_destroy, pthread_mutex_lock, pthread_mutex_lock, pthread_mutex_unlock,
                        spinlock_t, "Spin", spinlock_init, spinlock_destroy, spinlock_lock, spinlock_lock, spinlock_unlock);

#endif

