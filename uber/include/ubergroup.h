#if 0
#include "uberlock.h"
#include "patch.h"


#ifndef UBERGROUP_HH_
#define UBERGROUP_HH_

#ifdef __APPLE__
 #define DEFINE_GROUP(T) ubergroup_t __group_ ## T;
 #define NUM_PATCHES 7
 #define GROUP(T) __group_ ## T

 #define INIT_GROUP(T, probe, exp_freq, exp_time)  \
   int ubergroup_init(&__group_ ## T);

 #ifdef __cplusplus
 #define EXTERN_GROUP(T)\
   extern "C" { \
    extern ubergroup_t __group_ ## T;
   }
 #else
 #define EXTERN_GROUP(T) \
   extern ubergroup_t __group_ ## T;
 #endif

 #define uber_wrlock_special(T, t)  uberlock_wrlock(t)
 #define uber_rdlock_special(T, t)  uberlock_rdlock(t)
 #define uber_unlock_special(T, t)  uberlock_unlock(t)

#else

#define DEFINE_GROUP(T) \
\
ubergroup_t __group_ ## T; \
\
EXTERN3(rdlock_top_ ## T, rdlock_mutex_ ## T, rdlock_rw_ ## T);  \
EXTERN3(rdlock_mutex_held_ ## T, rdlock_end_ ## T, rdlock_mutex_failed_ ## T); \
EXTERN3(rdlock_rw_held_ ## T, rdlock_rw_failed_ ## T, rdlock_end_ ## T); \
EXTERN3(wrlock_top_ ## T, wrlock_mutex_ ## T, wrlock_rw_ ## T);  \
EXTERN3(wrlock_mutex_held_ ## T, wrlock_end_ ## T, wrlock_mutex_failed_ ## T); \
EXTERN3(wrlock_rw_held_ ## T, wrlock_rw_failed_ ## T, wrlock_end_ ## T);    \
EXTERN3(unlock_top_ ## T, unlock_mutex_ ## T, unlock_rw_ ## T);   \
\
__attribute__((optimize("align-functions=1024")))\
int uberlock_unlock_ ## T(uberlock_t *t) { \
 top:              \
 JMP(unlock_top_ ## T, unlock_mutex_ ## T, unlock_rw_ ## T);  \
 LABEL(unlock_mutex_ ## T);      \
   pthread_mutex_unlock(&t->mutex);      \
   goto end;            \
   LABEL(unlock_rw_ ## T);        \
   pthread_rwlock_unlock(&t->rw);      \
   goto end;            \
end:             \
return 0;            \
}              \
\
int uberlock_rdlock_ ## T(uberlock_t *t) { \
top:            \
 JMP(rdlock_top_ ## T, rdlock_mutex_ ## T, rdlock_rw_ ## T);  \
 LABEL(rdlock_mutex_ ## T);      \
   pthread_mutex_lock(&t->mutex);      \
   JMP(rdlock_mutex_held_ ## T, rdlock_end_ ## T, rdlock_mutex_failed_ ## T);  \
   LABEL(rdlock_mutex_failed_ ## T);      \
     pthread_mutex_unlock(&t->mutex);      \
     goto top;            \
 LABEL(rdlock_rw_ ## T);        \
   pthread_rwlock_rdlock(&t->rw);      \
   JMP(rdlock_rw_held_ ## T, rdlock_rw_failed_ ## T, rdlock_end_ ## T);  \
   LABEL(rdlock_rw_failed_ ## T);      \
     pthread_rwlock_unlock(&t->rw);      \
     goto top;            \
 LABEL(rdlock_end_ ## T);        \
 return 0;            \
}              \
              \
\
\
int uberlock_wrlock_ ## T(uberlock_t *t) { \
top:            \
  JMP(wrlock_top_ ## T, wrlock_mutex_ ## T, wrlock_rw_ ## T);  \
 LABEL(wrlock_mutex_ ## T);      \
   pthread_mutex_lock(&t->mutex);      \
   JMP(wrlock_mutex_held_ ## T, wrlock_end_ ## T, wrlock_mutex_failed_ ## T);  \
   LABEL(wrlock_mutex_failed_ ## T);      \
     pthread_mutex_unlock(&t->mutex);      \
     goto top;            \
 LABEL(wrlock_rw_ ## T);        \
   pthread_rwlock_wrlock(&t->rw);      \
   JMP(wrlock_rw_held_ ## T, wrlock_rw_failed_ ## T, wrlock_end_ ## T); \
   LABEL(wrlock_rw_failed_ ## T);      \
     pthread_rwlock_unlock(&t->rw);      \
     goto top;            \
 LABEL(wrlock_end_ ## T);        \
 return 0;            \
}              \
\
\
\
static void __patch_ ## T(int mode) {    \
 static int init = 0;      \
   if (init == 0) {         \
   set_permissions(&rdlock_top_ ## T, 1); \
   set_permissions(&rdlock_mutex_held_ ## T, 1); \
   set_permissions(&rdlock_rw_held_ ## T, 1);\
   set_permissions(&wrlock_top_ ## T, 1); \
   set_permissions(&wrlock_mutex_held_ ## T, 1); \
   set_permissions(&wrlock_rw_held_ ## T, 1); \
   set_permissions(&unlock_top_ ## T, 1);    \
   init = 1;                \
 }                  \
                     \
 PATCH(mode, &rdlock_top_ ## T, &rdlock_mutex_ ## T, &rdlock_rw_ ## T); \
 PATCH(mode, &rdlock_mutex_held_ ## T, &rdlock_end_ ## T, &rdlock_mutex_failed_ ## T); \
 PATCH(mode, &rdlock_rw_held_ ## T, &rdlock_rw_failed_ ## T, &rdlock_end_ ## T);    \
 PATCH(mode, &wrlock_top_ ## T, &wrlock_mutex_ ## T, &wrlock_rw_ ## T);  \
 PATCH(mode, &wrlock_mutex_held_ ## T, &wrlock_end_ ## T, &wrlock_mutex_failed_ ## T); \
 PATCH(mode, &wrlock_rw_held_ ## T, &wrlock_rw_failed_ ## T, &wrlock_end_ ## T); \
 PATCH(mode, &unlock_top_ ## T, &unlock_mutex_ ## T, &unlock_rw_ ## T);    \
}               \


#define NUM_PATCHES 7

#define GROUP(T) __group_ ## T

#define INIT_GROUP(T)  \
 int ubergroup_init_special(&__group_ ## T, __patch_ ## T);


#ifdef __cplusplus
#define EXTERN_GROUP(T)\
 extern "C" { \
  extern ubergroup_t __group_ ## T;                                   \
  int uberlock_rdlock_ ## T(uberlock_t *t);\
  int uberlock_wrlock_ ## T(uberlock_t *t);   \
  int uberlock_unlock_ ## T(uberlock_t *t);   \
 }
#else
#define EXTERN_GROUP(T) \
 extern ubergroup_t __group_ ## T;                                    \
 int uberlock_rdlock_ ## T(uberlock_t *t);\
 int uberlock_wrlock_ ## T(uberlock_t *t);    \
 int uberlock_unlock_ ## T(uberlock_t *t);
#endif


#define uber_wrlock_special(T, t)  uberlock_wrlock_ ## T(t)
#define uber_rdlock_special(T, t)  uberlock_rdlock_ ## T(t)
#define uber_unlock_special(T, t)  uberlock_unlock_ ## T(t)

#endif
#endif

#endif

