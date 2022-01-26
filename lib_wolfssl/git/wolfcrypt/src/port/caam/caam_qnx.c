/* caam_qnx.c
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

#ifdef HAVE_CONFIG_H
    #include <config.h>
#endif

#include <wolfssl/wolfcrypt/settings.h>

#if defined(__QNX__) || defined(__QNXNTO__)

#include <wolfssl/wolfcrypt/port/caam/caam_driver.h>
#include <wolfssl/version.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <sys/neutrino.h>
#include <sys/resmgr.h>
#include <devctl.h>

/* virtual address for accessing CAAM addresses */
uintptr_t virtual_base = 0;

/* keep track of which ID memory belongs to so it can be free'd up */
#define MAX_PART 7
pthread_mutex_t sm_mutex;
CAAM_ADDRESS sm_ownerId[MAX_PART];

/* variables for I/O of resource manager */
resmgr_connect_funcs_t connect_funcs;
resmgr_io_funcs_t      io_funcs;
dispatch_t             *dpp;
resmgr_attr_t          rattr;
dispatch_context_t     *ctp;
iofunc_attr_t          ioattr;

int io_devctl (resmgr_context_t *ctp, io_devctl_t *msg, iofunc_ocb_t *ocb);
int io_open (resmgr_context_t *ctp, io_open_t  *msg, RESMGR_HANDLE_T *handle,
        void *extra);
int io_read (resmgr_context_t *ctp, io_read_t  *msg, RESMGR_OCB_T *ocb);
int io_write(resmgr_context_t *ctp, io_write_t *msg, RESMGR_OCB_T *ocb);
int io_close_ocb(resmgr_context_t *ctp, void *reserved, RESMGR_OCB_T *ocb);


/* read the contents at offset from BASE address */
unsigned int CAAM_READ(unsigned int ofst) {
    return in32(virtual_base + ofst);
}


/* takes in offset from BASE address */
void CAAM_WRITE(unsigned int ofst, unsigned int in)
{
    out32(virtual_base + ofst, in);
}


/* Sets the base address to use for read/write
 * returns 0 on success
 */
int CAAM_SET_BASEADDR()
{
    /* address range for CAAM is CAAM_BASE plus 0x10000 */
    virtual_base = mmap_device_io(0x00010000, CAAM_BASE);
    if (virtual_base == (uintptr_t)MAP_FAILED) {
        WOLFSSL_MSG("Unable to map virtual memory");
        return -1;
    }
    return 0;
}


/* cleans up having set the base address */
void CAAM_UNSET_BASEADDR()
{
    munmap_device_io(virtual_base, 0x00010000);
}

/* convert a virtual address to a physical address
 * returns the physical address on success
 */
CAAM_ADDRESS CAAM_ADR_TO_PHYSICAL(void* in, int inSz)
{
    off64_t ofst = 0;
    int ret, count = 0;;

    if (in == NULL)
        return 0;

    if (inSz == 0)
        inSz = 1;

    do {
        ret = mem_offset64(in, NOFD, inSz, &ofst, NULL);
        if (ret != 0) {
            WOLFSSL_MSG("posix offset failed");
        #if defined(WOLFSSL_CAAM_DEBUG) || defined(WOLFSSL_CAAM_PRINT)
            perror("posix offset failed : ");
        #endif
        }
        msync(in, inSz, MS_INVALIDATE);
        count++;
    } while (ret != 0 && ret == -1 && count < 5);

    return (CAAM_ADDRESS)ofst;
}


/* convert a physical address to virtual address
 * returns the virtual address on success
 */
CAAM_ADDRESS CAAM_ADR_TO_VIRTUAL(CAAM_ADDRESS in, int len)
{
    void* ret;
    ret = mmap_device_memory(NULL, len, PROT_READ | PROT_WRITE | PROT_NOCACHE,
            0, in);
    return (CAAM_ADDRESS)ret;
}


/* map a virtual address to a created coherent physical address
 * returns the mapped address on success
 */
void* CAAM_ADR_MAP(unsigned int in, int inSz, unsigned char copy)
{
    int  sz;
    void *vaddr;

    sz = inSz;
    if (inSz == 0) {
        sz = 1;
    }

    vaddr = mmap(NULL, sz, PROT_READ | PROT_WRITE | PROT_NOCACHE,
                    MAP_PHYS | MAP_SHARED | MAP_ANON, NOFD, 0);
    if (vaddr == MAP_FAILED) {
        WOLFSSL_MSG("Failed to map memory");
    #if defined(WOLFSSL_CAAM_DEBUG) || defined(WOLFSSL_CAAM_PRINT)
        perror("Failed to map memory : ");
    #endif
    }
    else {
        if (copy && in != 0 && inSz > 0) {
            memcpy((unsigned char*)vaddr, (unsigned char*)in, inSz);
        }

        if (msync(vaddr, sz, MS_SYNC) != 0) {
            WOLFSSL_MSG("Failed to sync memory after copy");
        }
    }
    return vaddr;
}


/* un map address, should be called when done with a mapped address */
void CAAM_ADR_UNMAP(void* vaddr, unsigned int out, int outSz,
        unsigned char copy)
{
    int sz;

    sz = outSz;
    if (outSz == 0)
        sz = 1;

    if (msync(vaddr, sz, MS_SYNC) != 0) {
    #if defined(WOLFSSL_CAAM_DEBUG) || defined(WOLFSSL_CAAM_PRINT)
        perror("Failed to sync output");
    #endif
        /* even though the address was not synced up still try to copy and
           unmap it */
    }

    if (copy && out != 0 && outSz > 0) {
        memcpy((unsigned char*)out, (unsigned char*)vaddr, outSz);
    }
    munmap(vaddr, sz);
}


/* synchronize virtual buffer with physical
 * return 0 on success */
int CAAM_ADR_SYNC(void* vaddr, int sz)
{
    if (msync(vaddr, sz, MS_SYNC) != 0) {
    #if defined(WOLFSSL_CAAM_DEBUG) || defined(WOLFSSL_CAAM_PRINT)
        perror("address sync failed");
    #endif
        return -1;
    }
    return 0;
}


/* macros for QNX devctl commands */
#define WC_TRNG_CMD __DIOTF(_DCMD_ALL, CAAM_ENTROPY, iov_t)
#define WC_CAAM_GET_PART __DIOTF(_DCMD_ALL, CAAM_GET_PART, iov_t)
#define WC_CAAM_FREE_PART __DIOT(_DCMD_ALL, CAAM_FREE_PART, iov_t)
#define WC_CAAM_FIND_PART __DIOTF(_DCMD_ALL, CAAM_FIND_PART, iov_t)
#define WC_CAAM_READ_PART __DIOTF(_DCMD_ALL, CAAM_READ_PART, iov_t)
#define WC_CAAM_WRITE_PART __DIOT(_DCMD_ALL, CAAM_WRITE_PART, iov_t)

#define WC_CAAM_ECDSA_KEYPAIR __DIOTF(_DCMD_ALL, CAAM_ECDSA_KEYPAIR, iov_t)
#define WC_CAAM_ECDSA_VERIFY __DIOT(_DCMD_ALL, CAAM_ECDSA_VERIFY, iov_t)
#define WC_CAAM_ECDSA_SIGN __DIOTF(_DCMD_ALL, CAAM_ECDSA_SIGN, iov_t)
#define WC_CAAM_ECDSA_ECDH __DIOTF(_DCMD_ALL, CAAM_ECDSA_ECDH, iov_t)

#define WC_CAAM_BLOB_ENCAP __DIOTF(_DCMD_ALL, CAAM_BLOB_ENCAP, iov_t)
#define WC_CAAM_BLOB_DECAP __DIOTF(_DCMD_ALL, CAAM_BLOB_DECAP, iov_t)

#define WC_CAAM_CMAC __DIOTF(_DCMD_ALL, CAAM_CMAC, iov_t)

#define WC_CAAM_FIFO_S __DIOTF(_DCMD_ALL, CAAM_FIFO_S, iov_t)


/* partAddr is virtual address
 * partSz   size expected to access
 *
 * returns 0 on ok
 */
static int sanityCheckPartitionAddress(CAAM_ADDRESS partAddr, int partSz)
{
    if (partAddr < CAAM_PAGE || partAddr > CAAM_PAGE * MAX_PART ||
            partSz > 4096) {
        WOLFSSL_MSG("error in physical address range");
        return -1;
    }
    return 0;
}


/* return 0 on success */
static int getArgs(unsigned int args[4], resmgr_context_t *ctp, io_devctl_t *msg,
        unsigned int *idx, unsigned int maxIdx)
{
    int expectedSz;
    iov_t in_iov;

    expectedSz = sizeof(unsigned int) * 4;
    if (*idx + expectedSz > maxIdx) {
        WOLFSSL_MSG("not enough for arguments");
        return -1;
    }

    SETIOV(&in_iov, args, expectedSz);
    if (resmgr_msgreadv(ctp, &in_iov, 1, *idx) != expectedSz) {
        WOLFSSL_MSG("unexpected msg size read");
        return -1;
    }
    *idx += expectedSz;
    return 0;
}


/* helper function to setup and run CMAC operation
 * returns EOK on success
 */
static int doCMAC(resmgr_context_t *ctp, io_devctl_t *msg, unsigned int args[4],
        unsigned int idx)
{
    DESCSTRUCT  desc;
    CAAM_BUFFER tmp[3];
    iov_t in_iovs[3], out_iov;

    int msgSz = 0, ret, numBuf, keySz;
    unsigned char ctx[32];    /* running CMAC context is a constant 32 bytes */
    unsigned char keybuf[32 + BLACK_KEY_MAC_SZ];/*max AES key size is 32 + MAC*/
    unsigned char *buf = NULL;

    numBuf = 2; /* start with 2 (key + ctx) for case with no msg input */
    keySz  = args[1];
    if (args[2] == 1) { /* is it a black key? */
        keySz = keySz + BLACK_KEY_MAC_SZ;
    }
    SETIOV(&in_iovs[0], keybuf, keySz);
    SETIOV(&in_iovs[1], ctx, sizeof(ctx));
    msgSz = args[3];
    if (msgSz < 0) {
        WOLFSSL_MSG("CMAC msg size was a negative value");
        return EBADMSG;
    }

    if (msgSz > 0) {
        buf = (unsigned char*)CAAM_ADR_MAP(0, msgSz, 0);
        if (buf == NULL) {
            return ECANCELED;
        }
        SETIOV(&in_iovs[2], buf, msgSz);
        numBuf = numBuf + 1; /* increase buffer size by one when adding msg */
    }

    ret = resmgr_msgreadv(ctp, in_iovs, numBuf, idx);
    if (ret < (msgSz + keySz + sizeof(ctx))) {
        /* sanity check that enough data was sent */
        if (buf != NULL)
            CAAM_ADR_UNMAP(buf, 0, msgSz, 0);
        return EOVERFLOW;
    }

    tmp[0].TheAddress = (CAAM_ADDRESS)keybuf;
    tmp[0].Length = args[1];

    tmp[1].TheAddress = (CAAM_ADDRESS)ctx;
    tmp[1].Length = sizeof(ctx);

    if (msgSz > 0) {
        tmp[2].TheAddress = (CAAM_ADDRESS)buf;
        tmp[2].Length     = msgSz;
    }
    caamDescInit(&desc, CAAM_CMAC, args, tmp, numBuf);
    ret = caamAesCmac(&desc, numBuf, args);
    if (msgSz > 0) {
        if (buf != NULL)
            CAAM_ADR_UNMAP(buf, 0, msgSz, 0);
    }

    if (ret != Success) {
        return EBADMSG;
    }
    SETIOV(&out_iov, ctx, sizeof(ctx));

    /* extra sanity check that out buffer is large enough */
    if (sizeof(ctx) > msg->o.nbytes) {
        return EOVERFLOW;
    }
    ret = resmgr_msgwritev(ctp, &out_iov, 1, sizeof(msg->o));
    if (ret < 0) {
        return ECANCELED;
    }

    return EOK;
}


/* helper function to setup and run TRNG operation
 * returns EOK on success
 */
static int doTRNG(resmgr_context_t *ctp, io_devctl_t *msg, unsigned int args[4],
        unsigned int idx)
{
    int length, ret;
    unsigned char *buf;
    iov_t out_iov;

    length = args[0];

    /* sanity check that length out is not over the edge */
    if (length > msg->o.nbytes) {
        WOLFSSL_MSG("Length too large for TRNG out size available");
        return EOVERFLOW;
    }

    if (length > 0) {
        buf = (unsigned char*)CAAM_ADR_MAP(0, length, 0);
        if (buf == NULL) {
            return ECANCELED;
        }

        ret = caamTRNG(buf, length);
        if (ret == CAAM_WAITING) {
            /* waiting for more entropy */
            CAAM_ADR_UNMAP(buf, 0, length, 0);
            return EAGAIN;
        }

        SETIOV(&out_iov, buf, length);
        ret = resmgr_msgwritev(ctp, &out_iov, 1, sizeof(msg->o));
        CAAM_ADR_UNMAP(buf, 0, length, 0);
        if (ret < 0) {
            return ECANCELED;
        }
    }
    return EOK;
}


/* helper function to setup and run BLOB operation
 * returns EOK on success
 */
static int doBLOB(resmgr_context_t *ctp, io_devctl_t *msg, unsigned int args[4],
        unsigned int idx)
{
    int WC_CAAM_BLOB_SZ = 48; /* extra blob size from manual */
    int dir, ret, inSz, outSz;
    DESCSTRUCT  desc;
    CAAM_BUFFER tmp[3];
    iov_t in_iovs[2], out_iov;

    unsigned char *inBuf, *outBuf;
    unsigned char keymod[BLACK_BLOB_KEYMOD_SZ];
    /* 16 is max size for keymod (8 with red blobs and 16 with black) */

    if (msg->i.dcmd == WC_CAAM_BLOB_ENCAP) {
        dir = CAAM_BLOB_ENCAP;
    }
    else {
        dir = CAAM_BLOB_DECAP;
    }

    inSz = args[2];
    if (inSz < 0) {
        return EBADMSG;
    }

    if (args[0] == 1 && dir == CAAM_BLOB_ENCAP) {
        inSz = inSz + BLACK_KEY_MAC_SZ;
    }

    SETIOV(&in_iovs[0], keymod, args[3]);
    if ((inSz + args[3]) > (ctp->size - idx)) {
        return EOVERFLOW;
    }

    inBuf = (unsigned char*)CAAM_ADR_MAP(0, inSz, 0);
    if (inBuf == NULL) {
        return ECANCELED;
    }
    SETIOV(&in_iovs[1], inBuf, inSz);
    ret = resmgr_msgreadv(ctp, in_iovs, 2, idx);
    if (ret < inSz + args[3]) {
        return EBADMSG;
    }

    /* key mod */
    tmp[0].TheAddress = (CAAM_ADDRESS)keymod;
    tmp[0].Length = args[3];

    /* input */
    tmp[1].TheAddress = (CAAM_ADDRESS)inBuf;
    tmp[1].Length = args[2];

    /* output */
    outSz = args[2];
    if (msg->i.dcmd == WC_CAAM_BLOB_ENCAP) {
        outSz = outSz + WC_CAAM_BLOB_SZ;
    }
    else {
        outSz = outSz - WC_CAAM_BLOB_SZ;
    }
    if (outSz < 0) {
        return EBADMSG;
    }

    if (args[0] == 1 && dir == CAAM_BLOB_DECAP) {
        outBuf = (unsigned char*)CAAM_ADR_MAP(0, outSz + BLACK_KEY_MAC_SZ, 0);
    }
    else {
        outBuf = (unsigned char*)CAAM_ADR_MAP(0, outSz, 0);
    }
    if (outBuf == NULL) {
        CAAM_ADR_UNMAP(inBuf, 0, inSz, 0);
        return ECANCELED;
    }
    tmp[2].TheAddress = (CAAM_ADDRESS)outBuf;
    tmp[2].Length     = outSz;

    caamDescInit(&desc, dir, args, tmp, 3);
    ret = caamBlob(&desc);
    CAAM_ADR_UNMAP(inBuf, 0, inSz, 0);

    /* adjust outSz for MAC tag at the end of black key */
    if (args[0] == 1 && dir == CAAM_BLOB_DECAP) {
        outSz = outSz + BLACK_KEY_MAC_SZ;
    }

    if (ret != Success) {
        CAAM_ADR_UNMAP(outBuf, 0, outSz, 0);
        return ECANCELED;
    }

    CAAM_ADR_SYNC(outBuf, outSz);
    SETIOV(&out_iov, outBuf, outSz);
    if (outSz > msg->o.nbytes) {
        CAAM_ADR_UNMAP(outBuf, 0, outSz, 0);
        return EOVERFLOW;
    }
    ret = resmgr_msgwritev(ctp, &out_iov, 1, sizeof(msg->o));
    CAAM_ADR_UNMAP(outBuf, 0, outSz, 0);
    if (ret < 0) {
        return ECANCELED;
    }
    return EOK;
}


/* helper function to setup and make ECC key
 * returns EOK on success
 */
static int doECDSA_KEYPAIR(resmgr_context_t *ctp, io_devctl_t *msg, unsigned int args[4],
        unsigned int idx, iofunc_ocb_t *ocb)
{
    int ret;
    DESCSTRUCT desc;
    CAAM_BUFFER tmp[2];
    iov_t in_iovs[2], out_iovs[3];

    SETIOV(&in_iovs[0], &tmp[0], sizeof(CAAM_BUFFER));
    SETIOV(&in_iovs[1], &tmp[1], sizeof(CAAM_BUFFER));
    ret = resmgr_msgreadv(ctp, in_iovs, 2, idx);

    caamDescInit(&desc, CAAM_ECDSA_KEYPAIR, args, tmp, 2);
    ret = caamECDSAMake(&desc, tmp, args);
    if (ret != Success) {
        return ECANCELED;
    }

    SETIOV(&out_iovs[0], &tmp[0], sizeof(CAAM_BUFFER));
    SETIOV(&out_iovs[1], &tmp[1], sizeof(CAAM_BUFFER));
    SETIOV(&out_iovs[2], args, sizeof(int) * 4);
    ret = resmgr_msgwritev(ctp, &out_iovs[0], 3, sizeof(msg->o));
    if (ret < 0) {
        return ECANCELED;
    }

    /* claim ownership of a secure memory location */
    if (pthread_mutex_lock(&sm_mutex) != EOK) {
        return ECANCELED;
    }
    else {
        sm_ownerId[args[2]] = (CAAM_ADDRESS)ocb;
        pthread_mutex_unlock(&sm_mutex);
    }

    return EOK;
}


/* helper function to setup and do ECC verify
 * returns EOK on success
 */
static int doECDSA_VERIFY(resmgr_context_t *ctp, io_devctl_t *msg,
        unsigned int args[4], unsigned int idx)
{
    DESCSTRUCT  desc;
    CAAM_BUFFER tmp[5];
    iov_t in_iovs[4];
    int ret, pubSz;

    unsigned char *hash, *pubkey = NULL, *r, *s;
    CAAM_ADDRESS securePub;

    if (args[0] == 1) {
        pubSz = sizeof(CAAM_ADDRESS);

        SETIOV(&in_iovs[0], &securePub, sizeof(CAAM_ADDRESS));
    }
    else {
        pubSz  = args[3]*2;
        pubkey = (unsigned char*)CAAM_ADR_MAP(0, pubSz, 0);
        if (pubkey == NULL) {
            return ECANCELED;
        }

        SETIOV(&in_iovs[0], pubkey, args[3]*2);
    }

    hash = (unsigned char*)CAAM_ADR_MAP(0, args[2], 0);
    if (hash == NULL) {
        if (pubkey != NULL)
            CAAM_ADR_UNMAP(pubkey, 0, pubSz, 0);
        return ECANCELED;
    }
    SETIOV(&in_iovs[1], hash, args[2]);

    r = (unsigned char*)CAAM_ADR_MAP(0, args[3], 0);
    if (r == NULL) {
        if (pubkey != NULL)
            CAAM_ADR_UNMAP(pubkey, 0, pubSz, 0);
        CAAM_ADR_UNMAP(hash, 0, args[2], 0);
        return ECANCELED;
    }
    SETIOV(&in_iovs[2], r, args[3]);

    s = (unsigned char*)CAAM_ADR_MAP(0, args[3], 0);
    if (s == NULL) {
        if (pubkey != NULL)
            CAAM_ADR_UNMAP(pubkey, 0, pubSz, 0);
        CAAM_ADR_UNMAP(hash, 0, args[2], 0);
        CAAM_ADR_UNMAP(r, 0, args[3], 0);
        return ECANCELED;
    }
    SETIOV(&in_iovs[3], s, args[3]);

    ret = resmgr_msgreadv(ctp, in_iovs, 4, idx);
    if (((args[3] * 2) + args[2] + pubSz) > ret) {
        if (pubkey != NULL)
            CAAM_ADR_UNMAP(pubkey, 0, pubSz, 0);
        CAAM_ADR_UNMAP(hash, 0, args[2], 0);
        CAAM_ADR_UNMAP(r, 0, args[3], 0);
        CAAM_ADR_UNMAP(s, 0, args[3], 0);
        return EOVERFLOW;
    }

    /* setup CAAM buffers to pass to driver */
    if (args[0] == 1) {
        tmp[0].TheAddress = securePub;
    }
    else {
        tmp[0].TheAddress = (CAAM_ADDRESS)pubkey;
    }
    tmp[0].Length = args[3]*2;

    tmp[1].TheAddress = (CAAM_ADDRESS)hash;
    tmp[1].Length = args[2];

    tmp[2].TheAddress = (CAAM_ADDRESS)r;
    tmp[2].Length = args[3];

    tmp[3].TheAddress = (CAAM_ADDRESS)s;
    tmp[3].Length = args[3];

    if (pubkey != NULL)
        CAAM_ADR_SYNC(pubkey, pubSz);
    CAAM_ADR_SYNC(hash, args[2]);
    CAAM_ADR_SYNC(r, args[3]);
    CAAM_ADR_SYNC(s, args[3]);
    caamDescInit(&desc, CAAM_ECDSA_VERIFY, args, tmp, 4);
    ret = caamECDSAVerify(&desc, tmp, 4, args);

    /* free all buffers before inspecting the return value */
    CAAM_ADR_UNMAP(hash, 0, args[2], 0);
    CAAM_ADR_UNMAP(r, 0, args[3], 0);
    CAAM_ADR_UNMAP(s, 0, args[3], 0);
    if (pubkey != NULL)
        CAAM_ADR_UNMAP(pubkey, 0, pubSz, 0);

    if (ret != Success) {
        return EBADMSG;
    }
    return EOK;
}


/* helper function to setup and do ECC sign
 * returns EOK on success
 */
static int doECDSA_SIGN(resmgr_context_t *ctp, io_devctl_t *msg,
        unsigned int args[4], unsigned int idx)
{
    int ret, keySz;
    DESCSTRUCT  desc;
    CAAM_BUFFER tmp[4];

    unsigned char *key = NULL, *hash, *r, *s;
    CAAM_ADDRESS blackKey;

    iov_t in_iovs[2], out_iovs[2];

    if (args[0] == 1) {
        keySz = sizeof(CAAM_ADDRESS);
        SETIOV(&in_iovs[0], &blackKey, sizeof(CAAM_ADDRESS));
    }
    else {
        keySz = args[3];
        key   = (unsigned char*)CAAM_ADR_MAP(0, keySz, 0);
        if (key == NULL) {
            return ECANCELED;
        }
        SETIOV(&in_iovs[0], key, keySz);
    }

    hash = (unsigned char*)CAAM_ADR_MAP(0, args[2], 0);
    if (hash == NULL) {
        if (key != NULL)
            CAAM_ADR_UNMAP(key, 0, keySz, 0);
        return ECANCELED;
    }
    SETIOV(&in_iovs[1], hash, args[2]);
    ret = resmgr_msgreadv(ctp, in_iovs, 2, idx);
    if ((keySz + args[2]) > ret) {
        CAAM_ADR_UNMAP(hash, 0, args[2], 0);
        if (key != NULL)
            CAAM_ADR_UNMAP(key, 0, keySz, 0);
        return EOVERFLOW;
    }


    /* setup CAAM buffers to pass to driver */
    if (args[0] == 1) {
        tmp[0].TheAddress = blackKey;
    }
    else {
        tmp[0].TheAddress = (CAAM_ADDRESS)key;
    }
    tmp[0].Length = args[3];

    tmp[1].TheAddress = (CAAM_ADDRESS)hash;
    tmp[1].Length = args[2];

    r = (unsigned char*)CAAM_ADR_MAP(0, args[3], 0);
    if (r == NULL) {
        CAAM_ADR_UNMAP(hash, 0, args[2], 0);
        if (key != NULL)
            CAAM_ADR_UNMAP(key, 0, keySz, 0);
        return ECANCELED;
    }
    tmp[2].TheAddress = (CAAM_ADDRESS)r;
    tmp[2].Length = args[3];

    s = (unsigned char*)CAAM_ADR_MAP(0, args[3], 0);
    if (s == NULL) {
        CAAM_ADR_UNMAP(r, 0, args[3], 0);
        CAAM_ADR_UNMAP(hash, 0, args[2], 0);
        if (key != NULL)
            CAAM_ADR_UNMAP(key, 0, keySz, 0);
        return ECANCELED;
    }
    tmp[3].TheAddress = (CAAM_ADDRESS)s;
    tmp[3].Length = args[3];

    caamDescInit(&desc, CAAM_ECDSA_SIGN, args, tmp, 4);
    ret = caamECDSASign(&desc, 4, args);
    CAAM_ADR_UNMAP(hash, 0, args[2], 0);
    if (key != NULL)
        CAAM_ADR_UNMAP(key, 0, keySz, 0);
    if (ret != Success) {
        CAAM_ADR_UNMAP(s, 0, args[3], 0);
        CAAM_ADR_UNMAP(r, 0, args[3], 0);
        return EBADMSG;
    }

    if ((args[3] * 2) > msg->o.nbytes) {
        CAAM_ADR_UNMAP(s, 0, args[3], 0);
        CAAM_ADR_UNMAP(r, 0, args[3], 0);
        return EOVERFLOW;
    }

    CAAM_ADR_SYNC(r, args[3]);
    CAAM_ADR_SYNC(s, args[3]);
    SETIOV(&out_iovs[0], r, args[3]);
    SETIOV(&out_iovs[1], s, args[3]);

    ret = resmgr_msgwritev(ctp, &out_iovs[0], 2, sizeof(msg->o));
    CAAM_ADR_UNMAP(s, 0, args[3], 0);
    CAAM_ADR_UNMAP(r, 0, args[3], 0);
    if (ret < 0) {
        return ECANCELED;
    }
    return EOK;
}


/* helper function to setup and get an ECC shared secret
 * returns EOK on success
 */
static int doECDSA_ECDH(resmgr_context_t *ctp, io_devctl_t *msg,
        unsigned int args[4], unsigned int idx)
{
    int ret;
    DESCSTRUCT desc;
    CAAM_BUFFER tmp[3];
    int expectedSz = 0;
    iov_t in_iovs[2], out_iov;

    unsigned char *pubkey = NULL, *key = NULL, *shared;
    CAAM_ADDRESS securePub, blackKey;

    /* when using memory in secure partition just send the address */
    if (args[1] == 1) {
        SETIOV(&in_iovs[0], &securePub, sizeof(CAAM_ADDRESS));
        expectedSz += sizeof(CAAM_ADDRESS);
    }
    else {
        pubkey = (unsigned char*)CAAM_ADR_MAP(0, args[3]*2, 0);
        if (pubkey == NULL) {
            return ECANCELED;
        }

        SETIOV(&in_iovs[0], pubkey, args[3]*2);
        expectedSz += args[3]*2;
    }

    if (args[0] == 1) {
        SETIOV(&in_iovs[1], &blackKey, sizeof(CAAM_ADDRESS));
        expectedSz += sizeof(CAAM_ADDRESS);
    }
    else {
        key = (unsigned char*)CAAM_ADR_MAP(0, args[3], 0);
        if (key == NULL) {
            if (pubkey != NULL)
                CAAM_ADR_UNMAP(pubkey, 0, args[3]*2, 0);
            return ECANCELED;
        }

        SETIOV(&in_iovs[1], key, args[3]);
        expectedSz += args[3];
    }

    ret = resmgr_msgreadv(ctp, in_iovs, 2, idx);
    if (expectedSz > ret) {
        if (pubkey != NULL)
            CAAM_ADR_UNMAP(pubkey, 0, args[3]*2, 0);
        if (key != NULL)
            CAAM_ADR_UNMAP(key, 0, args[3], 0);
        return ECANCELED;
    }

    /* setup CAAM buffers to pass to driver */
    if (args[1] == 1) {
        tmp[0].TheAddress = securePub;
    }
    else {
        tmp[0].TheAddress = (CAAM_ADDRESS)pubkey;
    }
    tmp[0].Length = args[3]*2;

    if (args[0] == 1) {
        tmp[1].TheAddress = blackKey;
    }
    else {
        tmp[1].TheAddress = (CAAM_ADDRESS)key;
    }
    tmp[1].Length = args[3];

    shared = (unsigned char*)CAAM_ADR_MAP(0, args[3], 0);
    if (shared == NULL) {
        if (pubkey != NULL)
            CAAM_ADR_UNMAP(pubkey, 0, args[3]*2, 0);
        if (key != NULL)
            CAAM_ADR_UNMAP(key, 0, args[3], 0);
        return ECANCELED;
    }

    tmp[2].TheAddress = (CAAM_ADDRESS)shared;
    tmp[2].Length = args[3];
    caamDescInit(&desc, CAAM_ECDSA_ECDH, args, tmp, 3);
    ret = caamECDSA_ECDH(&desc, 3, args);
    if (pubkey != NULL)
        CAAM_ADR_UNMAP(pubkey, 0, args[3]*2, 0);
    if (key != NULL)
        CAAM_ADR_UNMAP(key, 0, args[3], 0);

    if (ret != Success) {
        CAAM_ADR_UNMAP(shared, 0, args[3], 0);
        return EBADMSG;
    }

    if (args[3] > msg->o.nbytes) {
        CAAM_ADR_UNMAP(shared, 0, args[3], 0);
        return EOVERFLOW;
    }
    CAAM_ADR_SYNC(shared, args[3]);
    SETIOV(&out_iov, shared, args[3]);
    resmgr_msgwritev(ctp, &out_iov, 1, sizeof(msg->o));
    CAAM_ADR_UNMAP(shared, 0, args[3], 0);
    return EOK;
}


/* helper function to setup and cover data
 * returns EOK on success
 */
static int doFIFO_S(resmgr_context_t *ctp, io_devctl_t *msg,
        unsigned int args[4], unsigned int idx)
{
    int ret;
    DESCSTRUCT desc;
    CAAM_BUFFER tmp[2];
    iov_t in_iov, out_iov;
    unsigned char *inBuf, *outBuf;

    inBuf = (unsigned char*)CAAM_ADR_MAP(0, args[1], 0);
    if (inBuf == NULL) {
        return ECANCELED;
    }

    SETIOV(&in_iov, inBuf, args[1]);
    ret = resmgr_msgreadv(ctp, &in_iov, 1, idx);
    if (ret < args[1]) {
        return EBADMSG;
    }

    outBuf = (unsigned char*)CAAM_ADR_MAP(0, args[1] + BLACK_KEY_MAC_SZ, 0);
    if (outBuf == NULL) {
        CAAM_ADR_UNMAP(inBuf, 0, args[1], 0);
        return ECANCELED;
    }

    tmp[0].TheAddress = (CAAM_ADDRESS)inBuf;
    tmp[0].Length     = args[1];
    tmp[1].TheAddress = (CAAM_ADDRESS)outBuf;
    tmp[1].Length     = args[1]; /* tmp1 actually needs an additional 16 bytes
                                  * for MAC */

    caamDescInit(&desc, CAAM_FIFO_S, args, tmp, 2);
    ret = caamKeyCover(&desc, 2, args);
    CAAM_ADR_UNMAP(inBuf, 0, args[1], 0);
    if (ret != Success) {
        CAAM_ADR_UNMAP(outBuf, 0, args[1] + BLACK_KEY_MAC_SZ, 0);
        return EBADMSG;
    }

    if (args[1] + BLACK_KEY_MAC_SZ > msg->o.nbytes) {
        CAAM_ADR_UNMAP(outBuf, 0, args[1] + BLACK_KEY_MAC_SZ, 0);
        WOLFSSL_MSG("would cause output buffer overflow");
        return EOVERFLOW;
    }

    SETIOV(&out_iov, outBuf, args[1] + BLACK_KEY_MAC_SZ);
    resmgr_msgwritev(ctp, &out_iov, 1, sizeof(msg->o));
    CAAM_ADR_UNMAP(outBuf, 0, args[1] + BLACK_KEY_MAC_SZ, 0);
    return EOK;
}


/* helper function to get partition
 * returns EOK on success
 */
static int doGET_PART(resmgr_context_t *ctp, io_devctl_t *msg,
        unsigned int args[4], unsigned int idx, iofunc_ocb_t *ocb)
{
    int partNumber;
    int partSz;
    CAAM_ADDRESS partAddr;
    iov_t out_iov;

    partNumber = args[0];
    partSz     = args[1];

    partAddr = caamGetPartition(partNumber, partSz, 0);
    if (partAddr == 0) {
        return EBADMSG;
    }

    SETIOV(&out_iov, &partAddr, sizeof(CAAM_ADDRESS));
    resmgr_msgwritev(ctp, &out_iov, 1, sizeof(msg->o));

    if (pthread_mutex_lock(&sm_mutex) != EOK) {
        return ECANCELED;
    }
    else {
        sm_ownerId[partNumber] = (CAAM_ADDRESS)ocb;
        pthread_mutex_unlock(&sm_mutex);
    }
    return EOK;
}


/* helper function to write to a partition
 * returns EOK on success
 */
static int doWRITE_PART(resmgr_context_t *ctp, io_devctl_t *msg,
        unsigned int args[4], unsigned int idx)
{
    int partSz, ret;
    CAAM_ADDRESS partAddr;
    CAAM_ADDRESS vaddr;
    unsigned char *buf;
    iov_t in_iov;

    /* get arguments */
    partAddr = args[0];
    partSz   = args[1];

    buf = (unsigned char*)CAAM_ADR_MAP(0, partSz, 0);
    if (buf == NULL) {
        return ECANCELED;
    }

    SETIOV(&in_iov, buf, partSz);
    ret = resmgr_msgreadv(ctp, &in_iov, 1, idx);
    if (ret != partSz) {
        CAAM_ADR_UNMAP(buf, 0, partSz, 0);
        return EBADMSG;
    }

    /* sanity check on address and length */
    if (sanityCheckPartitionAddress(partAddr, partSz) != 0) {
        CAAM_ADR_UNMAP(buf, 0, partSz, 0);
        return EBADMSG;
    }

    vaddr = CAAM_ADR_TO_VIRTUAL(partAddr, partSz);
    if (vaddr == 0) {
        CAAM_ADR_UNMAP(buf, 0, partSz, 0);
        return ECANCELED;
    }

    CAAM_ADR_UNMAP(buf, vaddr, partSz, 1);
    CAAM_ADR_UNMAP((void*)vaddr, 0, partSz, 0);
    return EOK;
}


/* helper function to read a partition
 * returns EOK on success
 */
static int doREAD_PART(resmgr_context_t *ctp, io_devctl_t *msg,
        unsigned int args[4], unsigned int idx)
{
    int partSz;
    CAAM_ADDRESS partAddr;
    CAAM_ADDRESS vaddr;
    unsigned char *buf;
    iov_t out_iov;

    /* get arguments */
    partAddr = args[0];
    partSz   = args[1];

    if (partSz > msg->o.nbytes) {
        WOLFSSL_MSG("not enough space to store read bytes");
        return EOVERFLOW;
    }

    /* sanity check on address and length */
    if (sanityCheckPartitionAddress(partAddr, partSz) != 0) {
        return EBADMSG;
    }

    buf = (unsigned char*)CAAM_ADR_MAP(0, partSz, 0);
    if (buf == NULL) {
        return ECANCELED;
    }

    vaddr = CAAM_ADR_TO_VIRTUAL(partAddr, partSz);
    if (vaddr == 0) {
        CAAM_ADR_UNMAP(buf, 0, partSz, 0);
        return ECANCELED;
    }

    memcpy(buf, (unsigned char*)vaddr, partSz);
    SETIOV(&out_iov, buf, partSz);
    resmgr_msgwritev(ctp, &out_iov, 1, sizeof(msg->o));
    CAAM_ADR_UNMAP(buf,   0, partSz, 0);
    CAAM_ADR_UNMAP((void*)vaddr, 0, partSz, 0);
    return EOK;
}


int io_devctl (resmgr_context_t *ctp, io_devctl_t *msg, iofunc_ocb_t *ocb)
{
    int ret = EBADMSG;
    unsigned int idx = sizeof(msg->i);
    unsigned int args[4];
    iov_t out_iov;

    /* check if at least got the msg header */
    if( ctp->size < sizeof(msg->i) ) {
        return EBADMSG;
    }

    if ((ret = iofunc_devctl_default (ctp, msg, ocb)) != _RESMGR_DEFAULT) {
        return ret;
    }

    /* check callers access rights for read/write */
    if ((ret = iofunc_devctl_verify(ctp, msg, ocb,
                    _IO_DEVCTL_VERIFY_OCB_READ | _IO_DEVCTL_VERIFY_OCB_WRITE))
                    != EOK) {
        WOLFSSL_MSG("issue verify devctl");
        return ret;
    }
    /* _IO_DEVCTL_VERIFY_PRIV      : restrict to root */
    /* _IO_DEVCTL_VERIFY_ACC_ISUID : restrict to owner of device */
    /* _IO_DEVCTL_VERIFY_ACC_ISGID : restrict to group permissions */

    if (getArgs(args, ctp, msg, &idx, ctp->size) != 0) {
        WOLFSSL_MSG("issue reading arguments");
        return EBADMSG;
    }

    switch (msg->i.dcmd) {
        case WC_CAAM_CMAC:
            ret = doCMAC(ctp, msg, args, idx);
            break;

        case WC_TRNG_CMD:
            ret = doTRNG(ctp, msg, args, idx);
            break;

        case WC_CAAM_BLOB_ENCAP:
        case WC_CAAM_BLOB_DECAP:
            ret = doBLOB(ctp, msg, args, idx);
            break;

        case WC_CAAM_ECDSA_KEYPAIR:
            ret = doECDSA_KEYPAIR(ctp, msg, args, idx, ocb);
            break;

        case WC_CAAM_ECDSA_VERIFY:
            ret = doECDSA_VERIFY(ctp, msg, args, idx);
            break;

        case WC_CAAM_ECDSA_SIGN:
            ret = doECDSA_SIGN(ctp, msg, args, idx);
            break;

        case WC_CAAM_ECDSA_ECDH:
            ret = doECDSA_ECDH(ctp, msg, args, idx);
            break;

        case WC_CAAM_FIFO_S:
            ret = doFIFO_S(ctp, msg, args, idx);
            break;

        case WC_CAAM_GET_PART:
            ret = doGET_PART(ctp, msg, args, idx, ocb);
            break;

        case WC_CAAM_FREE_PART:
            caamFreePart(args[0]);

            if (pthread_mutex_lock(&sm_mutex) != EOK) {
                ret = ECANCELED;
            }
            else {
                sm_ownerId[args[0]] = 0;
                pthread_mutex_unlock(&sm_mutex);
                ret = EOK;
            }
            break;

        case WC_CAAM_FIND_PART:
            ret = caamFindUnusedPartition();
            if (ret < 0) {
                /* none found, try again later */
                return EAGAIN;
            }
            SETIOV(&out_iov, &ret, sizeof(ret));
            resmgr_msgwritev(ctp, &out_iov, 1, sizeof(msg->o));
            ret = EOK;
            break;

        case WC_CAAM_WRITE_PART:
            ret = doWRITE_PART(ctp, msg, args, idx);
            break;

        case WC_CAAM_READ_PART:
            ret = doREAD_PART(ctp, msg, args, idx);
            break;

        default:
            WOLFSSL_MSG("unknown option");
            return (ENOSYS);
    }

    return ret;
}


int io_open(resmgr_context_t *ctp, io_open_t *msg, RESMGR_HANDLE_T *handle,
        void *extra)
{
    WOLFSSL_MSG("starting up");
    return (iofunc_open_default (ctp, msg, handle, extra));
}


int io_close_ocb(resmgr_context_t *ctp, void *reserved, RESMGR_OCB_T *ocb)
{
    int i;

    WOLFSSL_MSG("shutting down");

    /* free up any dangling owned memory */
    if (pthread_mutex_lock(&sm_mutex) != EOK) {
        return ECANCELED;
    }
    else {
        for (i = 0; i < MAX_PART; i++) {
            if (sm_ownerId[i] == (CAAM_ADDRESS)ocb) {
                sm_ownerId[i] = 0;
            #if defined(WOLFSSL_CAAM_DEBUG) || defined(WOLFSSL_CAAM_PRINT)
                printf("found dangiling partition at index %d\n", i);
            #endif
                caamFreePart(i);
            }
        }
        pthread_mutex_unlock(&sm_mutex);
    }
    return iofunc_close_ocb_default(ctp, reserved, ocb);
}


#if 0
static int getSupported(char* in)
{
        //printf("CAAM Status [0x%8.8x]   = 0x%8.8x\n",
        //    CAAM_STATUS, WC_CAAM_READ(CAAM_STATUS));
        printf("CAAM Version MS Register [0x%8.8x]  = 0x%8.8x\n",
            CAAM_VERSION_MS, CAAM_READ(CAAM_VERSION_MS));
        printf("CAAM Version LS Register [0x%8.8x]  = 0x%8.8x\n",
            CAAM_VERSION_LS, CAAM_READ(CAAM_VERSION_LS));
        printf("CAAM Support MS Register [0x%8.8x] = 0x%8.8x\n",
            CAMM_SUPPORT_MS, CAAM_READ(CAMM_SUPPORT_MS));
        printf("CAAM Support LS [0x%8.8x] = 0x%8.8x\n",
            CAMM_SUPPORT_LS, CAAM_READ(CAMM_SUPPORT_LS));

    return strlen(in)+1;
}
#endif

char cannedResponse[] = {
        "wolfCrypt QNX CAAM driver version "
        LIBWOLFSSL_VERSION_STRING
        "\nSupports:\n"
        "\tAES-CMAC\n"
        "\tECC (sign, verify, ecdh, keygen)\n"
        "\tBlobs (black and red)\n"
};

/* read is only used to get banner info of the driver */
int io_read (resmgr_context_t *ctp, io_read_t *msg, RESMGR_OCB_T *ocb)
{
    int status;

    /* check callers access rights */
    if ((status = iofunc_read_verify(ctp, msg, ocb, NULL)) != EOK) {
        return (status);
    }

    /* only support read not pread */
    if ((msg->i.xtype & _IO_XTYPE_MASK) != _IO_XTYPE_NONE) {
        return (ENOSYS);
    }

    if (ocb->offset == 0) { /* just fill up what can */
        int sz = min(msg->i.nbytes, sizeof(cannedResponse));
        MsgReply(ctp->rcvid, sz, cannedResponse, sz);
        ocb->offset += sz;
    }
    else {
        MsgReply(ctp->rcvid, EOK, NULL, 0);
    }

    return (_RESMGR_NOREPLY);
}


int io_write(resmgr_context_t *ctp, io_write_t *msg, RESMGR_OCB_T *ocb)
{
    (void)ctp;
    (void)msg;
    (void)ocb;

    /* write is not supported */
    return (ENOSYS);
}


int main(int argc, char *argv[])
{
    int name;
    int i;

    pthread_mutex_init(&sm_mutex, NULL);
    for (i = 0; i < MAX_PART; i++) {
        sm_ownerId[i] = 0;
    }

    if (InitCAAM() != 0) {
        WOLFSSL_MSG("unable to start up caam driver!");
        exit(1);
    }

    dpp = dispatch_create();
    if (dpp == NULL) {
        exit (1);
    }
    memset(&rattr, 0, sizeof(rattr));
    iofunc_func_init(_RESMGR_CONNECT_NFUNCS, &connect_funcs,
            _RESMGR_IO_NFUNCS, &io_funcs);

    connect_funcs.open = io_open;
    io_funcs.close_ocb = io_close_ocb;
    io_funcs.read      = io_read;
    io_funcs.write     = io_write;
    io_funcs.devctl    = io_devctl;

    iofunc_attr_init (&ioattr, S_IFCHR | 0666, NULL, NULL);
    name = resmgr_attach(dpp, &rattr, "/dev/wolfCrypt",
            _FTYPE_ANY, 0, &connect_funcs, &io_funcs, &ioattr);
    if (name == -1) {
        exit (1);
    }

    ctp = dispatch_context_alloc(dpp);
    while (1) {
        ctp = dispatch_block(ctp);
        if (ctp == NULL) {
            CleanupCAAM();
            exit (1);
        }
        dispatch_handler(ctp);
    }

    pthread_mutex_destroy(&sm_mutex);
    CleanupCAAM();
    return 0;
}

#endif /* __QNX__ || __QNXNTO__ */
