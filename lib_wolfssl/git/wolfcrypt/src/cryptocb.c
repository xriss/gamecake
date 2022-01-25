/* cryptocb.c
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

/* This framework provides a central place for crypto hardware integration
   using the devId scheme. If not supported return `CRYPTOCB_UNAVAILABLE`. */

#ifdef HAVE_CONFIG_H
    #include <config.h>
#endif

#include <wolfssl/wolfcrypt/settings.h>

#ifdef WOLF_CRYPTO_CB

#include <wolfssl/wolfcrypt/cryptocb.h>
#include <wolfssl/wolfcrypt/error-crypt.h>
#include <wolfssl/wolfcrypt/logging.h>


/* TODO: Consider linked list with mutex */
#ifndef MAX_CRYPTO_DEVID_CALLBACKS
#define MAX_CRYPTO_DEVID_CALLBACKS 8
#endif

typedef struct CryptoCb {
    int devId;
    CryptoDevCallbackFunc cb;
    void* ctx;
} CryptoCb;
static WOLFSSL_GLOBAL CryptoCb gCryptoDev[MAX_CRYPTO_DEVID_CALLBACKS];

static CryptoCb* wc_CryptoCb_FindDevice(int devId)
{
    int i;
    for (i=0; i<MAX_CRYPTO_DEVID_CALLBACKS; i++) {
        if (gCryptoDev[i].devId == devId)
            return &gCryptoDev[i];
    }
    return NULL;
}
static CryptoCb* wc_CryptoCb_FindDeviceByIndex(int startIdx)
{
    int i;
    for (i=startIdx; i<MAX_CRYPTO_DEVID_CALLBACKS; i++) {
        if (gCryptoDev[i].devId != INVALID_DEVID)
            return &gCryptoDev[i];
    }
    return NULL;
}

static WC_INLINE int wc_CryptoCb_TranslateErrorCode(int ret)
{
    if (ret == NOT_COMPILED_IN) {
        /* backwards compatibility for older NOT_COMPILED_IN syntax */
        ret = CRYPTOCB_UNAVAILABLE;
    }
    return ret;
}

void wc_CryptoCb_Init(void)
{
    int i;
    for (i=0; i<MAX_CRYPTO_DEVID_CALLBACKS; i++) {
        gCryptoDev[i].devId = INVALID_DEVID;
    }
}

int wc_CryptoCb_GetDevIdAtIndex(int startIdx)
{
    int devId = INVALID_DEVID;
    CryptoCb* dev = wc_CryptoCb_FindDeviceByIndex(startIdx);
    if (dev) {
        devId = dev->devId;
    }
    return devId;
}

int wc_CryptoCb_RegisterDevice(int devId, CryptoDevCallbackFunc cb, void* ctx)
{
    /* find existing or new */
    CryptoCb* dev = wc_CryptoCb_FindDevice(devId);
    if (dev == NULL)
        dev = wc_CryptoCb_FindDevice(INVALID_DEVID);

    if (dev == NULL)
        return BUFFER_E; /* out of devices */

    dev->devId = devId;
    dev->cb = cb;
    dev->ctx = ctx;

    return 0;
}

void wc_CryptoCb_UnRegisterDevice(int devId)
{
    CryptoCb* dev = wc_CryptoCb_FindDevice(devId);
    if (dev) {
        XMEMSET(dev, 0, sizeof(*dev));
        dev->devId = INVALID_DEVID;
    }
}

#ifndef NO_RSA
int wc_CryptoCb_Rsa(const byte* in, word32 inLen, byte* out,
    word32* outLen, int type, RsaKey* key, WC_RNG* rng)
{
    int ret = CRYPTOCB_UNAVAILABLE;
    CryptoCb* dev;

    if (key == NULL)
        return ret;

    /* locate registered callback */
    dev = wc_CryptoCb_FindDevice(key->devId);
    if (dev && dev->cb) {
        wc_CryptoInfo cryptoInfo;
        XMEMSET(&cryptoInfo, 0, sizeof(cryptoInfo));
        cryptoInfo.algo_type = WC_ALGO_TYPE_PK;
        cryptoInfo.pk.type = WC_PK_TYPE_RSA;
        cryptoInfo.pk.rsa.in = in;
        cryptoInfo.pk.rsa.inLen = inLen;
        cryptoInfo.pk.rsa.out = out;
        cryptoInfo.pk.rsa.outLen = outLen;
        cryptoInfo.pk.rsa.type = type;
        cryptoInfo.pk.rsa.key = key;
        cryptoInfo.pk.rsa.rng = rng;

        ret = dev->cb(dev->devId, &cryptoInfo, dev->ctx);
    }

    return wc_CryptoCb_TranslateErrorCode(ret);
}

#ifdef WOLFSSL_KEY_GEN
int wc_CryptoCb_MakeRsaKey(RsaKey* key, int size, long e, WC_RNG* rng)
{
    int ret = CRYPTOCB_UNAVAILABLE;
    CryptoCb* dev;

    if (key == NULL)
        return ret;

    /* locate registered callback */
    dev = wc_CryptoCb_FindDevice(key->devId);
    if (dev && dev->cb) {
        wc_CryptoInfo cryptoInfo;
        XMEMSET(&cryptoInfo, 0, sizeof(cryptoInfo));
        cryptoInfo.algo_type = WC_ALGO_TYPE_PK;
        cryptoInfo.pk.type = WC_PK_TYPE_RSA_KEYGEN;
        cryptoInfo.pk.rsakg.key = key;
        cryptoInfo.pk.rsakg.size = size;
        cryptoInfo.pk.rsakg.e = e;
        cryptoInfo.pk.rsakg.rng = rng;

        ret = dev->cb(dev->devId, &cryptoInfo, dev->ctx);
    }

    return wc_CryptoCb_TranslateErrorCode(ret);
}
#endif

int wc_CryptoCb_RsaCheckPrivKey(RsaKey* key, const byte* pubKey,
    word32 pubKeySz)
{
    int ret = CRYPTOCB_UNAVAILABLE;
    CryptoCb* dev;

    if (key == NULL)
        return ret;

    /* locate registered callback */
    dev = wc_CryptoCb_FindDevice(key->devId);
    if (dev && dev->cb) {
        wc_CryptoInfo cryptoInfo;
        XMEMSET(&cryptoInfo, 0, sizeof(cryptoInfo));
        cryptoInfo.algo_type = WC_ALGO_TYPE_PK;
        cryptoInfo.pk.type = WC_PK_TYPE_RSA_CHECK_PRIV_KEY;
        cryptoInfo.pk.rsa_check.key = key;
        cryptoInfo.pk.rsa_check.pubKey = pubKey;
        cryptoInfo.pk.rsa_check.pubKeySz = pubKeySz;

        ret = dev->cb(dev->devId, &cryptoInfo, dev->ctx);
    }

    return wc_CryptoCb_TranslateErrorCode(ret);
}
#endif /* !NO_RSA */

#ifdef HAVE_ECC
int wc_CryptoCb_MakeEccKey(WC_RNG* rng, int keySize, ecc_key* key, int curveId)
{
    int ret = CRYPTOCB_UNAVAILABLE;
    CryptoCb* dev;

    if (key == NULL)
        return ret;

    /* locate registered callback */
    dev = wc_CryptoCb_FindDevice(key->devId);
    if (dev && dev->cb) {
        wc_CryptoInfo cryptoInfo;
        XMEMSET(&cryptoInfo, 0, sizeof(cryptoInfo));
        cryptoInfo.algo_type = WC_ALGO_TYPE_PK;
        cryptoInfo.pk.type = WC_PK_TYPE_EC_KEYGEN;
        cryptoInfo.pk.eckg.rng = rng;
        cryptoInfo.pk.eckg.size = keySize;
        cryptoInfo.pk.eckg.key = key;
        cryptoInfo.pk.eckg.curveId = curveId;

        ret = dev->cb(dev->devId, &cryptoInfo, dev->ctx);
    }

    return wc_CryptoCb_TranslateErrorCode(ret);
}

int wc_CryptoCb_Ecdh(ecc_key* private_key, ecc_key* public_key,
    byte* out, word32* outlen)
{
    int ret = CRYPTOCB_UNAVAILABLE;
    CryptoCb* dev;

    if (private_key == NULL)
        return ret;

    /* locate registered callback */
    dev = wc_CryptoCb_FindDevice(private_key->devId);
    if (dev && dev->cb) {
        wc_CryptoInfo cryptoInfo;
        XMEMSET(&cryptoInfo, 0, sizeof(cryptoInfo));
        cryptoInfo.algo_type = WC_ALGO_TYPE_PK;
        cryptoInfo.pk.type = WC_PK_TYPE_ECDH;
        cryptoInfo.pk.ecdh.private_key = private_key;
        cryptoInfo.pk.ecdh.public_key = public_key;
        cryptoInfo.pk.ecdh.out = out;
        cryptoInfo.pk.ecdh.outlen = outlen;

        ret = dev->cb(dev->devId, &cryptoInfo, dev->ctx);
    }

    return wc_CryptoCb_TranslateErrorCode(ret);
}

int wc_CryptoCb_EccSign(const byte* in, word32 inlen, byte* out,
    word32 *outlen, WC_RNG* rng, ecc_key* key)
{
    int ret = CRYPTOCB_UNAVAILABLE;
    CryptoCb* dev;

    if (key == NULL)
        return ret;

    /* locate registered callback */
    dev = wc_CryptoCb_FindDevice(key->devId);
    if (dev && dev->cb) {
        wc_CryptoInfo cryptoInfo;
        XMEMSET(&cryptoInfo, 0, sizeof(cryptoInfo));
        cryptoInfo.algo_type = WC_ALGO_TYPE_PK;
        cryptoInfo.pk.type = WC_PK_TYPE_ECDSA_SIGN;
        cryptoInfo.pk.eccsign.in = in;
        cryptoInfo.pk.eccsign.inlen = inlen;
        cryptoInfo.pk.eccsign.out = out;
        cryptoInfo.pk.eccsign.outlen = outlen;
        cryptoInfo.pk.eccsign.rng = rng;
        cryptoInfo.pk.eccsign.key = key;

        ret = dev->cb(dev->devId, &cryptoInfo, dev->ctx);
    }

    return wc_CryptoCb_TranslateErrorCode(ret);
}

int wc_CryptoCb_EccVerify(const byte* sig, word32 siglen,
    const byte* hash, word32 hashlen, int* res, ecc_key* key)
{
    int ret = CRYPTOCB_UNAVAILABLE;
    CryptoCb* dev;

    if (key == NULL)
        return ret;

    /* locate registered callback */
    dev = wc_CryptoCb_FindDevice(key->devId);
    if (dev && dev->cb) {
        wc_CryptoInfo cryptoInfo;
        XMEMSET(&cryptoInfo, 0, sizeof(cryptoInfo));
        cryptoInfo.algo_type = WC_ALGO_TYPE_PK;
        cryptoInfo.pk.type = WC_PK_TYPE_ECDSA_VERIFY;
        cryptoInfo.pk.eccverify.sig = sig;
        cryptoInfo.pk.eccverify.siglen = siglen;
        cryptoInfo.pk.eccverify.hash = hash;
        cryptoInfo.pk.eccverify.hashlen = hashlen;
        cryptoInfo.pk.eccverify.res = res;
        cryptoInfo.pk.eccverify.key = key;

        ret = dev->cb(dev->devId, &cryptoInfo, dev->ctx);
    }

    return wc_CryptoCb_TranslateErrorCode(ret);
}

int wc_CryptoCb_EccCheckPrivKey(ecc_key* key, const byte* pubKey,
    word32 pubKeySz)
{
    int ret = CRYPTOCB_UNAVAILABLE;
    CryptoCb* dev;

    if (key == NULL)
        return ret;

    /* locate registered callback */
    dev = wc_CryptoCb_FindDevice(key->devId);
    if (dev && dev->cb) {
        wc_CryptoInfo cryptoInfo;
        XMEMSET(&cryptoInfo, 0, sizeof(cryptoInfo));
        cryptoInfo.algo_type = WC_ALGO_TYPE_PK;
        cryptoInfo.pk.type = WC_PK_TYPE_EC_CHECK_PRIV_KEY;
        cryptoInfo.pk.ecc_check.key = key;
        cryptoInfo.pk.ecc_check.pubKey = pubKey;
        cryptoInfo.pk.ecc_check.pubKeySz = pubKeySz;

        ret = dev->cb(dev->devId, &cryptoInfo, dev->ctx);
    }

    return wc_CryptoCb_TranslateErrorCode(ret);
}
#endif /* HAVE_ECC */

#ifdef HAVE_CURVE25519
int wc_CryptoCb_Curve25519Gen(WC_RNG* rng, int keySize,
    curve25519_key* key)
{
    int ret = CRYPTOCB_UNAVAILABLE;
    CryptoCb* dev;

    if (key == NULL)
        return ret;

    /* locate registered callback */
    dev = wc_CryptoCb_FindDevice(key->devId);
    if (dev && dev->cb) {
        wc_CryptoInfo cryptoInfo;
        XMEMSET(&cryptoInfo, 0, sizeof(cryptoInfo));
        cryptoInfo.algo_type = WC_ALGO_TYPE_PK;
        cryptoInfo.pk.type = WC_PK_TYPE_CURVE25519_KEYGEN;
        cryptoInfo.pk.curve25519kg.rng = rng;
        cryptoInfo.pk.curve25519kg.size = keySize;
        cryptoInfo.pk.curve25519kg.key = key;

        ret = dev->cb(dev->devId, &cryptoInfo, dev->ctx);
    }

    return wc_CryptoCb_TranslateErrorCode(ret);
}

int wc_CryptoCb_Curve25519(curve25519_key* private_key,
    curve25519_key* public_key, byte* out, word32* outlen, int endian)
{
    int ret = CRYPTOCB_UNAVAILABLE;
    CryptoCb* dev;

    if (private_key == NULL)
        return ret;

    /* locate registered callback */
    dev = wc_CryptoCb_FindDevice(private_key->devId);
    if (dev && dev->cb) {
        wc_CryptoInfo cryptoInfo;
        XMEMSET(&cryptoInfo, 0, sizeof(cryptoInfo));
        cryptoInfo.algo_type = WC_ALGO_TYPE_PK;
        cryptoInfo.pk.type = WC_PK_TYPE_CURVE25519;
        cryptoInfo.pk.curve25519.private_key = private_key;
        cryptoInfo.pk.curve25519.public_key = public_key;
        cryptoInfo.pk.curve25519.out = out;
        cryptoInfo.pk.curve25519.outlen = outlen;
        cryptoInfo.pk.curve25519.endian = endian;

        ret = dev->cb(dev->devId, &cryptoInfo, dev->ctx);
    }

    return wc_CryptoCb_TranslateErrorCode(ret);
}
#endif /* HAVE_CURVE25519 */

#ifdef HAVE_ED25519
int wc_CryptoCb_Ed25519Gen(WC_RNG* rng, int keySize,
    ed25519_key* key)
{
    int ret = CRYPTOCB_UNAVAILABLE;
    CryptoCb* dev;

    if (key == NULL)
        return ret;

    /* locate registered callback */
    dev = wc_CryptoCb_FindDevice(key->devId);
    if (dev && dev->cb) {
        wc_CryptoInfo cryptoInfo;
        XMEMSET(&cryptoInfo, 0, sizeof(cryptoInfo));
        cryptoInfo.algo_type = WC_ALGO_TYPE_PK;
        cryptoInfo.pk.type = WC_PK_TYPE_ED25519_KEYGEN;
        cryptoInfo.pk.ed25519kg.rng = rng;
        cryptoInfo.pk.ed25519kg.size = keySize;
        cryptoInfo.pk.ed25519kg.key = key;

        ret = dev->cb(dev->devId, &cryptoInfo, dev->ctx);
    }

    return wc_CryptoCb_TranslateErrorCode(ret);
}

int wc_CryptoCb_Ed25519Sign(const byte* in, word32 inLen, byte* out,
                            word32 *outLen, ed25519_key* key, byte type,
                            const byte* context, byte contextLen)
{
    int ret = CRYPTOCB_UNAVAILABLE;
    CryptoCb* dev;

    if (key == NULL)
        return ret;

    /* locate registered callback */
    dev = wc_CryptoCb_FindDevice(key->devId);
    if (dev && dev->cb) {
        wc_CryptoInfo cryptoInfo;
        XMEMSET(&cryptoInfo, 0, sizeof(cryptoInfo));
        cryptoInfo.algo_type = WC_ALGO_TYPE_PK;
        cryptoInfo.pk.type = WC_PK_TYPE_ED25519_SIGN;
        cryptoInfo.pk.ed25519sign.in = in;
        cryptoInfo.pk.ed25519sign.inLen = inLen;
        cryptoInfo.pk.ed25519sign.out = out;
        cryptoInfo.pk.ed25519sign.outLen = outLen;
        cryptoInfo.pk.ed25519sign.key = key;
        cryptoInfo.pk.ed25519sign.type = type;
        cryptoInfo.pk.ed25519sign.context = context;
        cryptoInfo.pk.ed25519sign.contextLen = contextLen;

        ret = dev->cb(dev->devId, &cryptoInfo, dev->ctx);
    }

    return wc_CryptoCb_TranslateErrorCode(ret);
}

int wc_CryptoCb_Ed25519Verify(const byte* sig, word32 sigLen,
    const byte* msg, word32 msgLen, int* res, ed25519_key* key, byte type,
    const byte* context, byte contextLen)
{
    int ret = CRYPTOCB_UNAVAILABLE;
    CryptoCb* dev;

    if (key == NULL)
        return ret;

    /* locate registered callback */
    dev = wc_CryptoCb_FindDevice(key->devId);
    if (dev && dev->cb) {
        wc_CryptoInfo cryptoInfo;
        XMEMSET(&cryptoInfo, 0, sizeof(cryptoInfo));
        cryptoInfo.algo_type = WC_ALGO_TYPE_PK;
        cryptoInfo.pk.type = WC_PK_TYPE_ED25519_VERIFY;
        cryptoInfo.pk.ed25519verify.sig = sig;
        cryptoInfo.pk.ed25519verify.sigLen = sigLen;
        cryptoInfo.pk.ed25519verify.msg = msg;
        cryptoInfo.pk.ed25519verify.msgLen = msgLen;
        cryptoInfo.pk.ed25519verify.res = res;
        cryptoInfo.pk.ed25519verify.key = key;
        cryptoInfo.pk.ed25519verify.type = type;
        cryptoInfo.pk.ed25519verify.context = context;
        cryptoInfo.pk.ed25519verify.contextLen = contextLen;

        ret = dev->cb(dev->devId, &cryptoInfo, dev->ctx);
    }

    return wc_CryptoCb_TranslateErrorCode(ret);
}
#endif /* HAVE_ED25519 */

#ifndef NO_AES
#ifdef HAVE_AESGCM
int wc_CryptoCb_AesGcmEncrypt(Aes* aes, byte* out,
                               const byte* in, word32 sz,
                               const byte* iv, word32 ivSz,
                               byte* authTag, word32 authTagSz,
                               const byte* authIn, word32 authInSz)
{
    int ret = CRYPTOCB_UNAVAILABLE;
    CryptoCb* dev;

    /* locate registered callback */
    if (aes) {
        dev = wc_CryptoCb_FindDevice(aes->devId);
    }
    else {
        /* locate first callback and try using it */
        dev = wc_CryptoCb_FindDeviceByIndex(0);
    }

    if (dev && dev->cb) {
        wc_CryptoInfo cryptoInfo;
        XMEMSET(&cryptoInfo, 0, sizeof(cryptoInfo));
        cryptoInfo.algo_type = WC_ALGO_TYPE_CIPHER;
        cryptoInfo.cipher.type = WC_CIPHER_AES_GCM;
        cryptoInfo.cipher.enc = 1;
        cryptoInfo.cipher.aesgcm_enc.aes       = aes;
        cryptoInfo.cipher.aesgcm_enc.out       = out;
        cryptoInfo.cipher.aesgcm_enc.in        = in;
        cryptoInfo.cipher.aesgcm_enc.sz        = sz;
        cryptoInfo.cipher.aesgcm_enc.iv        = iv;
        cryptoInfo.cipher.aesgcm_enc.ivSz      = ivSz;
        cryptoInfo.cipher.aesgcm_enc.authTag   = authTag;
        cryptoInfo.cipher.aesgcm_enc.authTagSz = authTagSz;
        cryptoInfo.cipher.aesgcm_enc.authIn    = authIn;
        cryptoInfo.cipher.aesgcm_enc.authInSz  = authInSz;

        ret = dev->cb(dev->devId, &cryptoInfo, dev->ctx);
    }

    return wc_CryptoCb_TranslateErrorCode(ret);
}

int wc_CryptoCb_AesGcmDecrypt(Aes* aes, byte* out,
                               const byte* in, word32 sz,
                               const byte* iv, word32 ivSz,
                               const byte* authTag, word32 authTagSz,
                               const byte* authIn, word32 authInSz)
{
    int ret = CRYPTOCB_UNAVAILABLE;
    CryptoCb* dev;

    /* locate registered callback */
    if (aes) {
        dev = wc_CryptoCb_FindDevice(aes->devId);
    }
    else {
        /* locate first callback and try using it */
        dev = wc_CryptoCb_FindDeviceByIndex(0);
    }

    if (dev && dev->cb) {
        wc_CryptoInfo cryptoInfo;
        XMEMSET(&cryptoInfo, 0, sizeof(cryptoInfo));
        cryptoInfo.algo_type = WC_ALGO_TYPE_CIPHER;
        cryptoInfo.cipher.type = WC_CIPHER_AES_GCM;
        cryptoInfo.cipher.enc = 0;
        cryptoInfo.cipher.aesgcm_dec.aes       = aes;
        cryptoInfo.cipher.aesgcm_dec.out       = out;
        cryptoInfo.cipher.aesgcm_dec.in        = in;
        cryptoInfo.cipher.aesgcm_dec.sz        = sz;
        cryptoInfo.cipher.aesgcm_dec.iv        = iv;
        cryptoInfo.cipher.aesgcm_dec.ivSz      = ivSz;
        cryptoInfo.cipher.aesgcm_dec.authTag   = authTag;
        cryptoInfo.cipher.aesgcm_dec.authTagSz = authTagSz;
        cryptoInfo.cipher.aesgcm_dec.authIn    = authIn;
        cryptoInfo.cipher.aesgcm_dec.authInSz  = authInSz;

        ret = dev->cb(dev->devId, &cryptoInfo, dev->ctx);
    }

    return wc_CryptoCb_TranslateErrorCode(ret);
}
#endif /* HAVE_AESGCM */

#ifdef HAVE_AESCCM
int wc_CryptoCb_AesCcmEncrypt(Aes* aes, byte* out,
                               const byte* in, word32 sz,
                               const byte* nonce, word32 nonceSz,
                               byte* authTag, word32 authTagSz,
                               const byte* authIn, word32 authInSz)
{
    int ret = CRYPTOCB_UNAVAILABLE;
    CryptoCb* dev;

    /* locate registered callback */
    if (aes) {
        dev = wc_CryptoCb_FindDevice(aes->devId);
    }
    else {
        /* locate first callback and try using it */
        dev = wc_CryptoCb_FindDeviceByIndex(0);
    }

    if (dev && dev->cb) {
        wc_CryptoInfo cryptoInfo;
        XMEMSET(&cryptoInfo, 0, sizeof(cryptoInfo));
        cryptoInfo.algo_type = WC_ALGO_TYPE_CIPHER;
        cryptoInfo.cipher.type = WC_CIPHER_AES_CCM;
        cryptoInfo.cipher.enc = 1;
        cryptoInfo.cipher.aesccm_enc.aes       = aes;
        cryptoInfo.cipher.aesccm_enc.out       = out;
        cryptoInfo.cipher.aesccm_enc.in        = in;
        cryptoInfo.cipher.aesccm_enc.sz        = sz;
        cryptoInfo.cipher.aesccm_enc.nonce     = nonce;
        cryptoInfo.cipher.aesccm_enc.nonceSz   = nonceSz;
        cryptoInfo.cipher.aesccm_enc.authTag   = authTag;
        cryptoInfo.cipher.aesccm_enc.authTagSz = authTagSz;
        cryptoInfo.cipher.aesccm_enc.authIn    = authIn;
        cryptoInfo.cipher.aesccm_enc.authInSz  = authInSz;

        ret = dev->cb(dev->devId, &cryptoInfo, dev->ctx);
    }

    return wc_CryptoCb_TranslateErrorCode(ret);
}

int wc_CryptoCb_AesCcmDecrypt(Aes* aes, byte* out,
                               const byte* in, word32 sz,
                               const byte* nonce, word32 nonceSz,
                               const byte* authTag, word32 authTagSz,
                               const byte* authIn, word32 authInSz)
{
    int ret = CRYPTOCB_UNAVAILABLE;
    CryptoCb* dev;

    /* locate registered callback */
    if (aes) {
        dev = wc_CryptoCb_FindDevice(aes->devId);
    }
    else {
        /* locate first callback and try using it */
        dev = wc_CryptoCb_FindDeviceByIndex(0);
    }

    if (dev && dev->cb) {
        wc_CryptoInfo cryptoInfo;
        XMEMSET(&cryptoInfo, 0, sizeof(cryptoInfo));
        cryptoInfo.algo_type = WC_ALGO_TYPE_CIPHER;
        cryptoInfo.cipher.type = WC_CIPHER_AES_CCM;
        cryptoInfo.cipher.enc = 0;
        cryptoInfo.cipher.aesccm_dec.aes       = aes;
        cryptoInfo.cipher.aesccm_dec.out       = out;
        cryptoInfo.cipher.aesccm_dec.in        = in;
        cryptoInfo.cipher.aesccm_dec.sz        = sz;
        cryptoInfo.cipher.aesccm_enc.nonce     = nonce;
        cryptoInfo.cipher.aesccm_enc.nonceSz   = nonceSz;
        cryptoInfo.cipher.aesccm_dec.authTag   = authTag;
        cryptoInfo.cipher.aesccm_dec.authTagSz = authTagSz;
        cryptoInfo.cipher.aesccm_dec.authIn    = authIn;
        cryptoInfo.cipher.aesccm_dec.authInSz  = authInSz;

        ret = dev->cb(dev->devId, &cryptoInfo, dev->ctx);
    }

    return wc_CryptoCb_TranslateErrorCode(ret);
}
#endif /* HAVE_AESCCM */

#ifdef HAVE_AES_CBC
int wc_CryptoCb_AesCbcEncrypt(Aes* aes, byte* out,
                               const byte* in, word32 sz)
{
    int ret = CRYPTOCB_UNAVAILABLE;
    CryptoCb* dev;

    /* locate registered callback */
    if (aes) {
        dev = wc_CryptoCb_FindDevice(aes->devId);
    }
    else {
        /* locate first callback and try using it */
        dev = wc_CryptoCb_FindDeviceByIndex(0);
    }

    if (dev && dev->cb) {
        wc_CryptoInfo cryptoInfo;
        XMEMSET(&cryptoInfo, 0, sizeof(cryptoInfo));
        cryptoInfo.algo_type = WC_ALGO_TYPE_CIPHER;
        cryptoInfo.cipher.type = WC_CIPHER_AES_CBC;
        cryptoInfo.cipher.enc = 1;
        cryptoInfo.cipher.aescbc.aes = aes;
        cryptoInfo.cipher.aescbc.out = out;
        cryptoInfo.cipher.aescbc.in = in;
        cryptoInfo.cipher.aescbc.sz = sz;

        ret = dev->cb(dev->devId, &cryptoInfo, dev->ctx);
    }

    return wc_CryptoCb_TranslateErrorCode(ret);
}

int wc_CryptoCb_AesCbcDecrypt(Aes* aes, byte* out,
                               const byte* in, word32 sz)
{
    int ret = CRYPTOCB_UNAVAILABLE;
    CryptoCb* dev;

    /* locate registered callback */
    if (aes) {
        dev = wc_CryptoCb_FindDevice(aes->devId);
    }
    else {
        /* locate first callback and try using it */
        dev = wc_CryptoCb_FindDeviceByIndex(0);
    }

    if (dev && dev->cb) {
        wc_CryptoInfo cryptoInfo;
        XMEMSET(&cryptoInfo, 0, sizeof(cryptoInfo));
        cryptoInfo.algo_type = WC_ALGO_TYPE_CIPHER;
        cryptoInfo.cipher.type = WC_CIPHER_AES_CBC;
        cryptoInfo.cipher.enc = 0;
        cryptoInfo.cipher.aescbc.aes = aes;
        cryptoInfo.cipher.aescbc.out = out;
        cryptoInfo.cipher.aescbc.in = in;
        cryptoInfo.cipher.aescbc.sz = sz;

        ret = dev->cb(dev->devId, &cryptoInfo, dev->ctx);
    }

    return wc_CryptoCb_TranslateErrorCode(ret);
}
#endif /* HAVE_AES_CBC */
#endif /* !NO_AES */

#ifndef NO_DES3
int wc_CryptoCb_Des3Encrypt(Des3* des3, byte* out,
                               const byte* in, word32 sz)
{
    int ret = CRYPTOCB_UNAVAILABLE;
    CryptoCb* dev;

    /* locate registered callback */
    if (des3) {
        dev = wc_CryptoCb_FindDevice(des3->devId);
    }
    else {
        /* locate first callback and try using it */
        dev = wc_CryptoCb_FindDeviceByIndex(0);
    }

    if (dev && dev->cb) {
        wc_CryptoInfo cryptoInfo;
        XMEMSET(&cryptoInfo, 0, sizeof(cryptoInfo));
        cryptoInfo.algo_type = WC_ALGO_TYPE_CIPHER;
        cryptoInfo.cipher.type = WC_CIPHER_DES3;
        cryptoInfo.cipher.enc = 1;
        cryptoInfo.cipher.des3.des = des3;
        cryptoInfo.cipher.des3.out = out;
        cryptoInfo.cipher.des3.in = in;
        cryptoInfo.cipher.des3.sz = sz;

        ret = dev->cb(dev->devId, &cryptoInfo, dev->ctx);
    }

    return wc_CryptoCb_TranslateErrorCode(ret);
}

int wc_CryptoCb_Des3Decrypt(Des3* des3, byte* out,
                               const byte* in, word32 sz)
{
    int ret = CRYPTOCB_UNAVAILABLE;
    CryptoCb* dev;

    /* locate registered callback */
    if (des3) {
        dev = wc_CryptoCb_FindDevice(des3->devId);
    }
    else {
        /* locate first callback and try using it */
        dev = wc_CryptoCb_FindDeviceByIndex(0);
    }

    if (dev && dev->cb) {
        wc_CryptoInfo cryptoInfo;
        XMEMSET(&cryptoInfo, 0, sizeof(cryptoInfo));
        cryptoInfo.algo_type = WC_ALGO_TYPE_CIPHER;
        cryptoInfo.cipher.type = WC_CIPHER_DES3;
        cryptoInfo.cipher.enc = 0;
        cryptoInfo.cipher.des3.des = des3;
        cryptoInfo.cipher.des3.out = out;
        cryptoInfo.cipher.des3.in = in;
        cryptoInfo.cipher.des3.sz = sz;

        ret = dev->cb(dev->devId, &cryptoInfo, dev->ctx);
    }

    return wc_CryptoCb_TranslateErrorCode(ret);
}
#endif /* !NO_DES3 */

#ifndef NO_SHA
int wc_CryptoCb_ShaHash(wc_Sha* sha, const byte* in,
    word32 inSz, byte* digest)
{
    int ret = CRYPTOCB_UNAVAILABLE;
    CryptoCb* dev;

    /* locate registered callback */
    if (sha) {
        dev = wc_CryptoCb_FindDevice(sha->devId);
    }
    else {
        /* locate first callback and try using it */
        dev = wc_CryptoCb_FindDeviceByIndex(0);
    }

    if (dev && dev->cb) {
        wc_CryptoInfo cryptoInfo;
        XMEMSET(&cryptoInfo, 0, sizeof(cryptoInfo));
        cryptoInfo.algo_type = WC_ALGO_TYPE_HASH;
        cryptoInfo.hash.type = WC_HASH_TYPE_SHA;
        cryptoInfo.hash.sha1 = sha;
        cryptoInfo.hash.in = in;
        cryptoInfo.hash.inSz = inSz;
        cryptoInfo.hash.digest = digest;

        ret = dev->cb(dev->devId, &cryptoInfo, dev->ctx);
    }

    return wc_CryptoCb_TranslateErrorCode(ret);
}
#endif /* !NO_SHA */

#ifndef NO_SHA256
int wc_CryptoCb_Sha256Hash(wc_Sha256* sha256, const byte* in,
    word32 inSz, byte* digest)
{
    int ret = CRYPTOCB_UNAVAILABLE;
    CryptoCb* dev;

    /* locate registered callback */
    if (sha256) {
        dev = wc_CryptoCb_FindDevice(sha256->devId);
    }
    else {
        /* locate first callback and try using it */
        dev = wc_CryptoCb_FindDeviceByIndex(0);
    }

    if (dev && dev->cb) {
        wc_CryptoInfo cryptoInfo;
        XMEMSET(&cryptoInfo, 0, sizeof(cryptoInfo));
        cryptoInfo.algo_type = WC_ALGO_TYPE_HASH;
        cryptoInfo.hash.type = WC_HASH_TYPE_SHA256;
        cryptoInfo.hash.sha256 = sha256;
        cryptoInfo.hash.in = in;
        cryptoInfo.hash.inSz = inSz;
        cryptoInfo.hash.digest = digest;

        ret = dev->cb(dev->devId, &cryptoInfo, dev->ctx);
    }

    return wc_CryptoCb_TranslateErrorCode(ret);
}
#endif /* !NO_SHA256 */

#ifdef WOLFSSL_SHA384
int wc_CryptoCb_Sha384Hash(wc_Sha384* sha384, const byte* in,
    word32 inSz, byte* digest)
{
    int ret = CRYPTOCB_UNAVAILABLE;
    CryptoCb* dev;

    /* locate registered callback */
    #ifndef NO_SHA2_CRYPTO_CB
    if (sha384) {
        dev = wc_CryptoCb_FindDevice(sha384->devId);
    }
    else
    #endif
    {
        /* locate first callback and try using it */
        dev = wc_CryptoCb_FindDeviceByIndex(0);
    }

    if (dev && dev->cb) {
        wc_CryptoInfo cryptoInfo;
        XMEMSET(&cryptoInfo, 0, sizeof(cryptoInfo));
        cryptoInfo.algo_type = WC_ALGO_TYPE_HASH;
        cryptoInfo.hash.type = WC_HASH_TYPE_SHA384;
        cryptoInfo.hash.sha384 = sha384;
        cryptoInfo.hash.in = in;
        cryptoInfo.hash.inSz = inSz;
        cryptoInfo.hash.digest = digest;

        ret = dev->cb(dev->devId, &cryptoInfo, dev->ctx);
    }

    return wc_CryptoCb_TranslateErrorCode(ret);
}
#endif /* WOLFSSL_SHA384 */

#ifdef WOLFSSL_SHA512
int wc_CryptoCb_Sha512Hash(wc_Sha512* sha512, const byte* in,
    word32 inSz, byte* digest)
{
    int ret = CRYPTOCB_UNAVAILABLE;
    CryptoCb* dev;

    /* locate registered callback */
    #ifndef NO_SHA2_CRYPTO_CB
    if (sha512) {
        dev = wc_CryptoCb_FindDevice(sha512->devId);
    }
    else
    #endif
    {
        /* locate first callback and try using it */
        dev = wc_CryptoCb_FindDeviceByIndex(0);
    }

    if (dev && dev->cb) {
        wc_CryptoInfo cryptoInfo;
        XMEMSET(&cryptoInfo, 0, sizeof(cryptoInfo));
        cryptoInfo.algo_type = WC_ALGO_TYPE_HASH;
        cryptoInfo.hash.type = WC_HASH_TYPE_SHA512;
        cryptoInfo.hash.sha512 = sha512;
        cryptoInfo.hash.in = in;
        cryptoInfo.hash.inSz = inSz;
        cryptoInfo.hash.digest = digest;

        ret = dev->cb(dev->devId, &cryptoInfo, dev->ctx);
    }

    return wc_CryptoCb_TranslateErrorCode(ret);
}
#endif /* WOLFSSL_SHA512 */

#ifndef NO_HMAC
int wc_CryptoCb_Hmac(Hmac* hmac, int macType, const byte* in, word32 inSz,
    byte* digest)
{
    int ret = CRYPTOCB_UNAVAILABLE;
    CryptoCb* dev;

    if (hmac == NULL)
        return ret;

    /* locate registered callback */
    dev = wc_CryptoCb_FindDevice(hmac->devId);
    if (dev && dev->cb) {
        wc_CryptoInfo cryptoInfo;
        XMEMSET(&cryptoInfo, 0, sizeof(cryptoInfo));
        cryptoInfo.algo_type = WC_ALGO_TYPE_HMAC;
        cryptoInfo.hmac.macType = macType;
        cryptoInfo.hmac.in = in;
        cryptoInfo.hmac.inSz = inSz;
        cryptoInfo.hmac.digest = digest;
        cryptoInfo.hmac.hmac = hmac;

        ret = dev->cb(dev->devId, &cryptoInfo, dev->ctx);
    }

    return wc_CryptoCb_TranslateErrorCode(ret);
}
#endif /* !NO_HMAC */

#ifndef WC_NO_RNG
int wc_CryptoCb_RandomBlock(WC_RNG* rng, byte* out, word32 sz)
{
    int ret = CRYPTOCB_UNAVAILABLE;
    CryptoCb* dev;

    /* locate registered callback */
    if (rng) {
        dev = wc_CryptoCb_FindDevice(rng->devId);
    }
    else {
        /* locate first callback and try using it */
        dev = wc_CryptoCb_FindDeviceByIndex(0);
    }

    if (dev && dev->cb) {
        wc_CryptoInfo cryptoInfo;
        XMEMSET(&cryptoInfo, 0, sizeof(cryptoInfo));
        cryptoInfo.algo_type = WC_ALGO_TYPE_RNG;
        cryptoInfo.rng.rng = rng;
        cryptoInfo.rng.out = out;
        cryptoInfo.rng.sz = sz;

        ret = dev->cb(dev->devId, &cryptoInfo, dev->ctx);
    }

    return wc_CryptoCb_TranslateErrorCode(ret);
}

int wc_CryptoCb_RandomSeed(OS_Seed* os, byte* seed, word32 sz)
{
    int ret = CRYPTOCB_UNAVAILABLE;
    CryptoCb* dev;

    /* locate registered callback */
    dev = wc_CryptoCb_FindDevice(os->devId);
    if (dev && dev->cb) {
        wc_CryptoInfo cryptoInfo;
        XMEMSET(&cryptoInfo, 0, sizeof(cryptoInfo));
        cryptoInfo.algo_type = WC_ALGO_TYPE_SEED;
        cryptoInfo.seed.os = os;
        cryptoInfo.seed.seed = seed;
        cryptoInfo.seed.sz = sz;

        ret = dev->cb(dev->devId, &cryptoInfo, dev->ctx);
    }

    return wc_CryptoCb_TranslateErrorCode(ret);
}
#endif /* !WC_NO_RNG */
#ifdef WOLFSSL_CMAC
int wc_CryptoCb_Cmac(Cmac* cmac, const byte* key, word32 keySz,
        const byte* in, word32 inSz, byte* out, word32* outSz, int type,
        void* ctx)
{
    int ret = CRYPTOCB_UNAVAILABLE;
    CryptoCb* dev;

    /* locate registered callback */
    if (cmac) {
        dev = wc_CryptoCb_FindDevice(cmac->devId);
    }
    else {
        /* locate first callback and try using it */
        dev = wc_CryptoCb_FindDeviceByIndex(0);
    }

    if (dev && dev->cb) {
        wc_CryptoInfo cryptoInfo;
        XMEMSET(&cryptoInfo, 0, sizeof(cryptoInfo));
        cryptoInfo.algo_type = WC_ALGO_TYPE_CMAC;

        cryptoInfo.cmac.cmac  = cmac;
        cryptoInfo.cmac.ctx   = ctx;
        cryptoInfo.cmac.key   = key;
        cryptoInfo.cmac.in    = in;
        cryptoInfo.cmac.out   = out;
        cryptoInfo.cmac.outSz = outSz;
        cryptoInfo.cmac.keySz = keySz;
        cryptoInfo.cmac.inSz  = inSz;
        cryptoInfo.cmac.type  = type;

        ret = dev->cb(dev->devId, &cryptoInfo, dev->ctx);
    }

    return wc_CryptoCb_TranslateErrorCode(ret);
}
#endif

#endif /* WOLF_CRYPTO_CB */
