/*
 * util.c
 *
 *  Created on: Jan 10, 2017
 *      Author: freund
 */

#include "util.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>
#include <pthread.h>

#ifndef _GNU_SOURCE
	#define _GNU_SOURCE
#endif

#include <unistd.h>
#include <sys/syscall.h>

#if defined(__APPLE__)
#include <mach/mach_time.h>
#else
#include <time.h>
#endif


size_t util_get_time() {
#if defined(__APPLE__)
  return mach_absolute_time();
#else
  struct timespec ts;
  if (clock_gettime(CLOCK_REALTIME, &ts) != 0) {
    perror("get_time():");
    abort();
  }
  return ts.tv_nsec + ts.tv_sec * 1000 * 1000 * 1000;
#endif
}

// returns nanos
size_t util_wait(size_t ns) {
  if(ns == 0) return 0;

  struct timespec ts;
  ts.tv_nsec = ns % (1000 * 1000 * 1000);
  ts.tv_sec = (ns - ts.tv_nsec) / (1000 * 1000 * 1000);

  size_t start_time = util_get_time();
  while(nanosleep(&ts, &ts) != 0) {}

  return util_get_time() - start_time;
}

// returns millis
size_t util_wait_ms(size_t ms) {
  return util_wait(ms * 1000 * 1000) / 1000 / 1000;
}

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void util_log(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    pthread_mutex_lock(&mutex);
    fprintf(stderr, "[");
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "]\n");
    pthread_mutex_unlock(&mutex);
    fflush(stdout);
}

