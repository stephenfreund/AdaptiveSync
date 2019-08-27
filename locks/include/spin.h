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

#ifndef SPIN_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef int spinlock_t;

extern int spinlock_init(spinlock_t *, void *dummy);
extern int spinlock_destroy(spinlock_t *);
extern int spinlock_lock(spinlock_t *);
extern int spinlock_unlock(spinlock_t *);

extern int spinlock_init_rtm(spinlock_t *, void *dummy);
extern int spinlock_destroy_rtm(spinlock_t *);
extern int spinlock_lock_rtm(spinlock_t *);
extern int spinlock_unlock_rtm(spinlock_t *);

#ifdef __cplusplus
}
#endif

#endif
