/* kdf.c
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

#include <wolfssl/wolfcrypt/wc_port.h>
#include <wolfssl/wolfcrypt/error-crypt.h>
#include <wolfssl/wolfcrypt/logging.h>

#ifndef NO_KDF

#if defined(HAVE_FIPS) && \
    defined(HAVE_FIPS_VERSION) && (HAVE_FIPS_VERSION >= 5)

    /* set NO_WRAPPERS before headers, use direct internal f()s not wrappers */
    #define FIPS_NO_WRAPPERS

    #ifdef USE_WINDOWS_API
        #pragma code_seg(".fipsA$m")
        #pragma const_seg(".fipsB$m")
    #endif
#endif


#ifdef NO_INLINE
    #include <wolfssl/wolfcrypt/misc.h>
#else
    #define WOLFSSL_MISC_INCLUDED
    #include <wolfcrypt/src/misc.c>
#endif

#include <wolfssl/wolfcrypt/hmac.h>
#include <wolfssl/wolfcrypt/kdf.h>


#ifdef WOLFSSL_HAVE_PRF

#ifdef WOLFSSL_SHA512
    #define P_HASH_MAX_SIZE WC_SHA512_DIGEST_SIZE
#elif defined(WOLFSSL_SHA384)
    #define P_HASH_MAX_SIZE WC_SHA384_DIGEST_SIZE
#else
    #define P_HASH_MAX_SIZE WC_SHA256_DIGEST_SIZE
#endif

/* Pseudo Random Function for MD5, SHA-1, SHA-256, SHA-384, or SHA-512 */
int wc_PRF(byte* result, word32 resLen, const byte* secret,
                  word32 secLen, const byte* seed, word32 seedLen, int hash,
                  void* heap, int devId)
{
    word32 len = P_HASH_MAX_SIZE;
    word32 times;
    word32 lastLen;
    word32 lastTime;
    word32 i;
    word32 idx = 0;
    int    ret = 0;
#ifdef WOLFSSL_SMALL_STACK
    byte*  previous;
    byte*  current;
    Hmac*  hmac;
#else
    byte   previous[P_HASH_MAX_SIZE];  /* max size */
    byte   current[P_HASH_MAX_SIZE];   /* max size */
    Hmac   hmac[1];
#endif

#ifdef WOLFSSL_SMALL_STACK
    previous = (byte*)XMALLOC(P_HASH_MAX_SIZE, heap, DYNAMIC_TYPE_DIGEST);
    current  = (byte*)XMALLOC(P_HASH_MAX_SIZE, heap, DYNAMIC_TYPE_DIGEST);
    hmac     = (Hmac*)XMALLOC(sizeof(Hmac),    heap, DYNAMIC_TYPE_HMAC);

    if (previous == NULL || current == NULL || hmac == NULL) {
        if (previous) XFREE(previous, heap, DYNAMIC_TYPE_DIGEST);
        if (current)  XFREE(current,  heap, DYNAMIC_TYPE_DIGEST);
        if (hmac)     XFREE(hmac,     heap, DYNAMIC_TYPE_HMAC);

        return MEMORY_E;
    }
#endif

    switch (hash) {
    #ifndef NO_MD5
        case md5_mac:
            hash = WC_MD5;
            len  = WC_MD5_DIGEST_SIZE;
        break;
    #endif

    #ifndef NO_SHA256
        case sha256_mac:
            hash = WC_SHA256;
            len  = WC_SHA256_DIGEST_SIZE;
        break;
    #endif

    #ifdef WOLFSSL_SHA384
        case sha384_mac:
            hash = WC_SHA384;
            len  = WC_SHA384_DIGEST_SIZE;
        break;
    #endif

    #ifdef WOLFSSL_SHA512
        case sha512_mac:
            hash = WC_SHA512;
            len  = WC_SHA512_DIGEST_SIZE;
        break;
    #endif

    #ifndef NO_SHA
        case sha_mac:
            hash = WC_SHA;
            len  = WC_SHA_DIGEST_SIZE;
        break;
    #endif
        default:
        #ifdef WOLFSSL_SMALL_STACK
            if (previous) XFREE(previous, heap, DYNAMIC_TYPE_DIGEST);
            if (current)  XFREE(current,  heap, DYNAMIC_TYPE_DIGEST);
            if (hmac)     XFREE(hmac,     heap, DYNAMIC_TYPE_HMAC);
        #endif
            return HASH_TYPE_E;
    }

    times   = resLen / len;
    lastLen = resLen % len;

    if (lastLen)
        times += 1;

    lastTime = times - 1;

    ret = wc_HmacInit(hmac, heap, devId);
    if (ret == 0) {
        ret = wc_HmacSetKey(hmac, hash, secret, secLen);
        if (ret == 0)
            ret = wc_HmacUpdate(hmac, seed, seedLen); /* A0 = seed */
        if (ret == 0)
            ret = wc_HmacFinal(hmac, previous);       /* A1 */
        if (ret == 0) {
            for (i = 0; i < times; i++) {
                ret = wc_HmacUpdate(hmac, previous, len);
                if (ret != 0)
                    break;
                ret = wc_HmacUpdate(hmac, seed, seedLen);
                if (ret != 0)
                    break;
                ret = wc_HmacFinal(hmac, current);
                if (ret != 0)
                    break;

                if ((i == lastTime) && lastLen)
                    XMEMCPY(&result[idx], current,
                                             min(lastLen, P_HASH_MAX_SIZE));
                else {
                    XMEMCPY(&result[idx], current, len);
                    idx += len;
                    ret = wc_HmacUpdate(hmac, previous, len);
                    if (ret != 0)
                        break;
                    ret = wc_HmacFinal(hmac, previous);
                    if (ret != 0)
                        break;
                }
            }
        }
        wc_HmacFree(hmac);
    }

    ForceZero(previous,  P_HASH_MAX_SIZE);
    ForceZero(current,   P_HASH_MAX_SIZE);
    ForceZero(hmac,      sizeof(Hmac));

#ifdef WOLFSSL_SMALL_STACK
    XFREE(previous, heap, DYNAMIC_TYPE_DIGEST);
    XFREE(current,  heap, DYNAMIC_TYPE_DIGEST);
    XFREE(hmac,     heap, DYNAMIC_TYPE_HMAC);
#endif

    return ret;
}
#undef P_HASH_MAX_SIZE

/* compute PRF (pseudo random function) using SHA1 and MD5 for TLSv1 */
int wc_PRF_TLSv1(byte* digest, word32 digLen, const byte* secret,
           word32 secLen, const byte* label, word32 labLen,
           const byte* seed, word32 seedLen, void* heap, int devId)
{
    int    ret  = 0;
    word32 half = (secLen + 1) / 2;

#ifdef WOLFSSL_SMALL_STACK
    byte* md5_half;
    byte* sha_half;
    byte* md5_result;
    byte* sha_result;
#else
    byte  md5_half[MAX_PRF_HALF];     /* half is real size */
    byte  sha_half[MAX_PRF_HALF];     /* half is real size */
    byte  md5_result[MAX_PRF_DIG];    /* digLen is real size */
    byte  sha_result[MAX_PRF_DIG];    /* digLen is real size */
#endif
#if defined(WOLFSSL_ASYNC_CRYPT) && !defined(WC_ASYNC_NO_HASH)
    WC_DECLARE_VAR(labelSeed, byte, MAX_PRF_LABSEED, heap);
    if (labelSeed == NULL)
        return MEMORY_E;
#else
    byte labelSeed[MAX_PRF_LABSEED];
#endif

    if (half > MAX_PRF_HALF ||
        labLen + seedLen > MAX_PRF_LABSEED ||
        digLen > MAX_PRF_DIG)
    {
    #if defined(WOLFSSL_ASYNC_CRYPT) && !defined(WC_ASYNC_NO_HASH)
        WC_FREE_VAR(labelSeed, heap);
    #endif
        return BUFFER_E;
    }

#ifdef WOLFSSL_SMALL_STACK
    md5_half   = (byte*)XMALLOC(MAX_PRF_HALF,    heap, DYNAMIC_TYPE_DIGEST);
    sha_half   = (byte*)XMALLOC(MAX_PRF_HALF,    heap, DYNAMIC_TYPE_DIGEST);
    md5_result = (byte*)XMALLOC(MAX_PRF_DIG,     heap, DYNAMIC_TYPE_DIGEST);
    sha_result = (byte*)XMALLOC(MAX_PRF_DIG,     heap, DYNAMIC_TYPE_DIGEST);

    if (md5_half == NULL || sha_half == NULL || md5_result == NULL ||
                                                           sha_result == NULL) {
        if (md5_half)   XFREE(md5_half,   heap, DYNAMIC_TYPE_DIGEST);
        if (sha_half)   XFREE(sha_half,   heap, DYNAMIC_TYPE_DIGEST);
        if (md5_result) XFREE(md5_result, heap, DYNAMIC_TYPE_DIGEST);
        if (sha_result) XFREE(sha_result, heap, DYNAMIC_TYPE_DIGEST);
    #if defined(WOLFSSL_ASYNC_CRYPT) && !defined(WC_ASYNC_NO_HASH)
        WC_FREE_VAR(labelSeed, heap);
    #endif

        return MEMORY_E;
    }
#endif

    XMEMSET(md5_result, 0, digLen);
    XMEMSET(sha_result, 0, digLen);

    XMEMCPY(md5_half, secret, half);
    XMEMCPY(sha_half, secret + half - secLen % 2, half);

    XMEMCPY(labelSeed, label, labLen);
    XMEMCPY(labelSeed + labLen, seed, seedLen);

    if ((ret = wc_PRF(md5_result, digLen, md5_half, half, labelSeed,
                                labLen + seedLen, md5_mac, heap, devId)) == 0) {
        if ((ret = wc_PRF(sha_result, digLen, sha_half, half, labelSeed,
                                labLen + seedLen, sha_mac, heap, devId)) == 0) {
            /* calculate XOR for TLSv1 PRF */
            XMEMCPY(digest, md5_result, digLen);
            xorbuf(digest, sha_result, digLen);
        }
    }

#ifdef WOLFSSL_SMALL_STACK
    XFREE(md5_half,   heap, DYNAMIC_TYPE_DIGEST);
    XFREE(sha_half,   heap, DYNAMIC_TYPE_DIGEST);
    XFREE(md5_result, heap, DYNAMIC_TYPE_DIGEST);
    XFREE(sha_result, heap, DYNAMIC_TYPE_DIGEST);
#endif

#if defined(WOLFSSL_ASYNC_CRYPT) && !defined(WC_ASYNC_NO_HASH)
    WC_FREE_VAR(labelSeed, heap);
#endif

    return ret;
}

/* Wrapper for TLS 1.2 and TLSv1 cases to calculate PRF */
/* In TLS 1.2 case call straight thru to wc_PRF */
int wc_PRF_TLS(byte* digest, word32 digLen, const byte* secret, word32 secLen,
            const byte* label, word32 labLen, const byte* seed, word32 seedLen,
            int useAtLeastSha256, int hash_type, void* heap, int devId)
{
    int ret = 0;

    if (useAtLeastSha256) {
    #if defined(WOLFSSL_ASYNC_CRYPT) && !defined(WC_ASYNC_NO_HASH)
        WC_DECLARE_VAR(labelSeed, byte, MAX_PRF_LABSEED, heap);
        if (labelSeed == NULL)
            return MEMORY_E;
    #else
        byte labelSeed[MAX_PRF_LABSEED];
    #endif

        if (labLen + seedLen > MAX_PRF_LABSEED)
            return BUFFER_E;

        XMEMCPY(labelSeed, label, labLen);
        XMEMCPY(labelSeed + labLen, seed, seedLen);

        /* If a cipher suite wants an algorithm better than sha256, it
         * should use better. */
        if (hash_type < sha256_mac || hash_type == blake2b_mac)
            hash_type = sha256_mac;
        /* compute PRF for MD5, SHA-1, SHA-256, or SHA-384 for TLSv1.2 PRF */
        ret = wc_PRF(digest, digLen, secret, secLen, labelSeed,
                     labLen + seedLen, hash_type, heap, devId);

    #if defined(WOLFSSL_ASYNC_CRYPT) && !defined(WC_ASYNC_NO_HASH)
        WC_FREE_VAR(labelSeed, heap);
    #endif
    }
    else {
#ifndef NO_OLD_TLS
        /* compute TLSv1 PRF (pseudo random function using HMAC) */
        ret = wc_PRF_TLSv1(digest, digLen, secret, secLen, label, labLen, seed,
                          seedLen, heap, devId);
#else
        ret = BAD_FUNC_ARG;
#endif
    }


    return ret;
}
#endif /* WOLFSSL_HAVE_PRF */


#if defined(HAVE_HKDF)

    /* Extract data using HMAC, salt and input.
     * RFC 5869 - HMAC-based Extract-and-Expand Key Derivation Function (HKDF)
     *
     * prk      The generated pseudorandom key.
     * salt     The salt.
     * saltLen  The length of the salt.
     * ikm      The input keying material.
     * ikmLen   The length of the input keying material.
     * digest   The type of digest to use.
     * returns 0 on success, otherwise failure.
     */
    int wc_Tls13_HKDF_Extract(byte* prk, const byte* salt, int saltLen,
                                 byte* ikm, int ikmLen, int digest)
    {
        int ret;
        int len = 0;

        switch (digest) {
            #ifndef NO_SHA256
            case WC_SHA256:
                len = WC_SHA256_DIGEST_SIZE;
                break;
            #endif

            #ifdef WOLFSSL_SHA384
            case WC_SHA384:
                len = WC_SHA384_DIGEST_SIZE;
                break;
            #endif

            #ifdef WOLFSSL_TLS13_SHA512
            case WC_SHA512:
                len = WC_SHA512_DIGEST_SIZE;
                break;
            #endif
            default:
                return BAD_FUNC_ARG;
        }

        /* When length is 0 then use zeroed data of digest length. */
        if (ikmLen == 0) {
            ikmLen = len;
            XMEMSET(ikm, 0, len);
        }

#ifdef WOLFSSL_DEBUG_TLS
        WOLFSSL_MSG("  Salt");
        WOLFSSL_BUFFER(salt, saltLen);
        WOLFSSL_MSG("  IKM");
        WOLFSSL_BUFFER(ikm, ikmLen);
#endif

        ret = wc_HKDF_Extract(digest, salt, saltLen, ikm, ikmLen, prk);

#ifdef WOLFSSL_DEBUG_TLS
        WOLFSSL_MSG("  PRK");
        WOLFSSL_BUFFER(prk, len);
#endif

        return ret;
    }

    /* Expand data using HMAC, salt and label and info.
     * TLS v1.3 defines this function.
     *
     * okm          The generated pseudorandom key - output key material.
     * okmLen       The length of generated pseudorandom key -
     *              output key material.
     * prk          The salt - pseudo-random key.
     * prkLen       The length of the salt - pseudo-random key.
     * protocol     The TLS protocol label.
     * protocolLen  The length of the TLS protocol label.
     * info         The information to expand.
     * infoLen      The length of the information.
     * digest       The type of digest to use.
     * returns 0 on success, otherwise failure.
     */
    int wc_Tls13_HKDF_Expand_Label(byte* okm, word32 okmLen,
                                 const byte* prk, word32 prkLen,
                                 const byte* protocol, word32 protocolLen,
                                 const byte* label, word32 labelLen,
                                 const byte* info, word32 infoLen,
                                 int digest)
    {
        int    ret = 0;
        int    idx = 0;
        byte   data[MAX_TLS13_HKDF_LABEL_SZ];

        /* Output length. */
        data[idx++] = (byte)(okmLen >> 8);
        data[idx++] = (byte)okmLen;
        /* Length of protocol | label. */
        data[idx++] = (byte)(protocolLen + labelLen);
        /* Protocol */
        XMEMCPY(&data[idx], protocol, protocolLen);
        idx += protocolLen;
        /* Label */
        XMEMCPY(&data[idx], label, labelLen);
        idx += labelLen;
        /* Length of hash of messages */
        data[idx++] = (byte)infoLen;
        /* Hash of messages */
        XMEMCPY(&data[idx], info, infoLen);
        idx += infoLen;

#ifdef WOLFSSL_DEBUG_TLS
        WOLFSSL_MSG("  PRK");
        WOLFSSL_BUFFER(prk, prkLen);
        WOLFSSL_MSG("  Info");
        WOLFSSL_BUFFER(data, idx);
#endif

        ret = wc_HKDF_Expand(digest, prk, prkLen, data, idx, okm, okmLen);

#ifdef WOLFSSL_DEBUG_TLS
        WOLFSSL_MSG("  OKM");
        WOLFSSL_BUFFER(okm, okmLen);
#endif

        ForceZero(data, idx);

        return ret;
    }

#endif /* HAVE_HKDF */


#ifdef WOLFSSL_WOLFSSH

/* hash union */
typedef union {
#ifndef NO_MD5
    wc_Md5 md5;
#endif
#ifndef NO_SHA
    wc_Sha sha;
#endif
#ifdef WOLFSSL_SHA224
    wc_Sha224 sha224;
#endif
#ifndef NO_SHA256
    wc_Sha256 sha256;
#endif
#ifdef WOLFSSL_SHA384
    wc_Sha384 sha384;
#endif
#ifdef WOLFSSL_SHA512
    wc_Sha512 sha512;
#endif
#ifdef WOLFSSL_SHA3
    wc_Sha3 sha3;
#endif
} _hash;

static
int _HashInit(byte hashId, _hash* hash)
{
    int ret = BAD_FUNC_ARG;

    switch (hashId) {
    #ifndef NO_SHA
        case WC_SHA:
            ret = wc_InitSha(&hash->sha);
            break;
    #endif /* !NO_SHA */

    #ifndef NO_SHA256
        case WC_SHA256:
            ret = wc_InitSha256(&hash->sha256);
            break;
    #endif /* !NO_SHA256 */

    #ifdef WOLFSSL_SHA384
        case WC_SHA384:
            ret = wc_InitSha384(&hash->sha384);
            break;
    #endif /* WOLFSSL_SHA384 */
    #ifdef WOLFSSL_SHA512
        case WC_SHA512:
            ret = wc_InitSha512(&hash->sha512);
            break;
    #endif /* WOLFSSL_SHA512 */
    }

    return ret;
}

static
int _HashUpdate(byte hashId, _hash* hash,
        const byte* data, word32 dataSz)
{
    int ret = BAD_FUNC_ARG;

    switch (hashId) {
    #ifndef NO_SHA
        case WC_SHA:
            ret = wc_ShaUpdate(&hash->sha, data, dataSz);
            break;
    #endif /* !NO_SHA */

    #ifndef NO_SHA256
        case WC_SHA256:
            ret = wc_Sha256Update(&hash->sha256, data, dataSz);
            break;
    #endif /* !NO_SHA256 */

    #ifdef WOLFSSL_SHA384
        case WC_SHA384:
            ret = wc_Sha384Update(&hash->sha384, data, dataSz);
            break;
    #endif /* WOLFSSL_SHA384 */
    #ifdef WOLFSSL_SHA512
        case WC_SHA512:
            ret = wc_Sha512Update(&hash->sha512, data, dataSz);
            break;
    #endif /* WOLFSSL_SHA512 */
    }

    return ret;
}

static
int _HashFinal(byte hashId, _hash* hash, byte* digest)
{
    int ret = BAD_FUNC_ARG;

    switch (hashId) {
    #ifndef NO_SHA
        case WC_SHA:
            ret = wc_ShaFinal(&hash->sha, digest);
            break;
    #endif /* !NO_SHA */

    #ifndef NO_SHA256
        case WC_SHA256:
            ret = wc_Sha256Final(&hash->sha256, digest);
            break;
    #endif /* !NO_SHA256 */

    #ifdef WOLFSSL_SHA384
        case WC_SHA384:
            ret = wc_Sha384Final(&hash->sha384, digest);
            break;
    #endif /* WOLFSSL_SHA384 */
    #ifdef WOLFSSL_SHA512
        case WC_SHA512:
            ret = wc_Sha512Final(&hash->sha512, digest);
            break;
    #endif /* WOLFSSL_SHA512 */
    }

    return ret;
}

static
void _HashFree(byte hashId, _hash* hash)
{
    switch (hashId) {
    #ifndef NO_SHA
        case WC_SHA:
            wc_ShaFree(&hash->sha);
            break;
    #endif /* !NO_SHA */

    #ifndef NO_SHA256
        case WC_SHA256:
            wc_Sha256Free(&hash->sha256);
            break;
    #endif /* !NO_SHA256 */

    #ifdef WOLFSSL_SHA384
        case WC_SHA384:
            wc_Sha384Free(&hash->sha384);
            break;
    #endif /* WOLFSSL_SHA384 */
    #ifdef WOLFSSL_SHA512
        case WC_SHA512:
            wc_Sha512Free(&hash->sha512);
            break;
    #endif /* WOLFSSL_SHA512 */
    }
}


#define LENGTH_SZ 4

int wc_SSH_KDF(byte hashId, byte keyId, byte* key, word32 keySz,
        const byte* k, word32 kSz, const byte* h, word32 hSz,
        const byte* sessionId, word32 sessionIdSz)
{
    word32 blocks, remainder;
    _hash hash;
    enum wc_HashType enmhashId = (enum wc_HashType)hashId;
    byte kPad = 0;
    byte pad = 0;
    byte kSzFlat[LENGTH_SZ];
    int digestSz;
    int ret;

    if (key == NULL || keySz == 0 ||
        k == NULL || kSz == 0 ||
        h == NULL || hSz == 0 ||
        sessionId == NULL || sessionIdSz == 0) {

        return BAD_FUNC_ARG;
    }

    digestSz = wc_HmacSizeByType(enmhashId);
    if (digestSz < 0) {
        return BAD_FUNC_ARG;
    }

    if (k[0] & 0x80) kPad = 1;
    c32toa(kSz + kPad, kSzFlat);

    blocks = keySz / digestSz;
    remainder = keySz % digestSz;

    ret = _HashInit(enmhashId, &hash);
    if (ret == 0)
        ret = _HashUpdate(enmhashId, &hash, kSzFlat, LENGTH_SZ);
    if (ret == 0 && kPad)
        ret = _HashUpdate(enmhashId, &hash, &pad, 1);
    if (ret == 0)
        ret = _HashUpdate(enmhashId, &hash, k, kSz);
    if (ret == 0)
        ret = _HashUpdate(enmhashId, &hash, h, hSz);
    if (ret == 0)
        ret = _HashUpdate(enmhashId, &hash, &keyId, sizeof(keyId));
    if (ret == 0)
        ret = _HashUpdate(enmhashId, &hash, sessionId, sessionIdSz);

    if (ret == 0) {
        if (blocks == 0) {
            if (remainder > 0) {
                byte lastBlock[WC_MAX_DIGEST_SIZE];
                ret = _HashFinal(enmhashId, &hash, lastBlock);
                if (ret == 0)
                    XMEMCPY(key, lastBlock, remainder);
            }
        }
        else {
            word32 runningKeySz, curBlock;

            runningKeySz = digestSz;
            ret = _HashFinal(enmhashId, &hash, key);

            for (curBlock = 1; curBlock < blocks; curBlock++) {
                ret = _HashInit(enmhashId, &hash);
                if (ret != 0) break;
                ret = _HashUpdate(enmhashId, &hash, kSzFlat, LENGTH_SZ);
                if (ret != 0) break;
                if (kPad)
                    ret = _HashUpdate(enmhashId, &hash, &pad, 1);
                if (ret != 0) break;
                ret = _HashUpdate(enmhashId, &hash, k, kSz);
                if (ret != 0) break;
                ret = _HashUpdate(enmhashId, &hash, h, hSz);
                if (ret != 0) break;
                ret = _HashUpdate(enmhashId, &hash, key, runningKeySz);
                if (ret != 0) break;
                ret = _HashFinal(enmhashId, &hash, key + runningKeySz);
                if (ret != 0) break;
                runningKeySz += digestSz;
            }

            if (remainder > 0) {
                byte lastBlock[WC_MAX_DIGEST_SIZE];
                if (ret == 0)
                    ret = _HashInit(enmhashId, &hash);
                if (ret == 0)
                    ret = _HashUpdate(enmhashId, &hash, kSzFlat, LENGTH_SZ);
                if (ret == 0 && kPad)
                    ret = _HashUpdate(enmhashId, &hash, &pad, 1);
                if (ret == 0)
                    ret = _HashUpdate(enmhashId, &hash, k, kSz);
                if (ret == 0)
                    ret = _HashUpdate(enmhashId, &hash, h, hSz);
                if (ret == 0)
                    ret = _HashUpdate(enmhashId, &hash, key, runningKeySz);
                if (ret == 0)
                    ret = _HashFinal(enmhashId, &hash, lastBlock);
                if (ret == 0)
                    XMEMCPY(key + runningKeySz, lastBlock, remainder);
            }
        }
    }

    _HashFree(enmhashId, &hash);

    return ret;
}

#endif /* WOLFSSL_WOLFSSH */

#endif /* NO_KDF */
