/* caam_qnx.h
 *
 * Copyright (C) 2006-2021 wolfSSL Inc.
 *
 * This file is part of wolfSSL.
 *
 * wolfSSL is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * wolfSSL is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1335, USA
 */


/* This file is porting low level QNX interactions, such as reading register */

#ifndef CAAM_QNX_H
#define CAAM_QNX_H

#ifdef WOLFSSL_CAAM_PRINT
#include <stdio.h>
#define WOLFSSL_MSG(in) printf("%s\n", (in))
#else
#define WOLFSSL_MSG(in)
#endif

#include <sys/mman.h>
#include <hw/inout.h>
#include <sys/iofunc.h>
#include <sys/neutrino.h>
#include <pthread.h>

#define CAAM_MUTEX pthread_mutex_t
#define CAAM_INIT_MUTEX(x) pthread_mutex_init((x), NULL)
#define CAAM_FREE_MUTEX(x) pthread_mutex_destroy((x))
#define CAAM_LOCK_MUTEX(x) pthread_mutex_lock((x))
#define CAAM_UNLOCK_MUTEX(x) pthread_mutex_unlock((x))

#define Error int
#define Value int
#define Boolean int
#define CAAM_ADDRESS unsigned int
#define Success 1
#define Failure 0
#define INTERRUPT_Panic()
#define MemoryMapMayNotBeEmpty -1
#define CAAM_WAITING -2
#define NoActivityReady -1
#define MemoryOperationNotPerformed -1

#ifndef WOLFSSL_CAAM_BUFFER
#define WOLFSSL_CAAM_BUFFER
    typedef struct CAAM_BUFFER {
        int BufferType;
        CAAM_ADDRESS TheAddress;
        int Length;
    } CAAM_BUFFER;
#endif

/* check kernel and yield to same priority threads waiting */
#define CAAM_CPU_CHILL() sched_yield()

/* IMX6UL */
#define CAAM_BASE 0x02140000
#define CAAM_PAGE 0x00100000

#endif /* CAAM_QNX_H */
