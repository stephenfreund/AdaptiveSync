/*
 * Copyright (c) 2016 Intel Corporation
 * Author: Andi Kleen
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that: (1) source code distributions
 * retain the above copyright notice and this paragraph in its entirety, (2)
 * distributions including binary code include the above copyright notice and
 * this paragraph in its entirety in the documentation or other materials
 * provided with the distribution
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* Simple non-adaptive, non backoff RTM lock. */
#include "spin.h"

#define pause() asm volatile("pause" ::: "memory")

#define RETRY_CON 3
#define RETRY_CAP 1
#define RETRY_OTHER 3

static inline int lock_is_free(int *lock)
{
	return *lock == 1;
}

int spinlock_init(spinlock_t *lock, void *dummy) {
     *lock = 1;
     return 0;
}

int spinlock_destroy(spinlock_t *lock) {
     return 0;
}

int spinlock_lock(spinlock_t *lock)
{
	while (__sync_sub_and_fetch(lock, 1) < 0) {
		do
			pause();
		while (!lock_is_free(lock));
	}
     return 0;
}

int spinlock_unlock(spinlock_t *lock)
{
	*lock = 1;
     return 0;
}

