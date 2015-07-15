/* Lua PolarSSL: Hash implementations */

#include "polarssl/md2.h"
#include "polarssl/md4.h"
#include "polarssl/md5.h"
#include "polarssl/sha1.h"
#include "polarssl/sha2.h"
#include "polarssl/sha4.h"


#define HASH_TYPENAME		"polarssl.hash"

#define HASH_BUFSIZE		64  /* max hash size */

typedef struct {
  union {
#ifdef POLARSSL_MD2_C
    md2_context md2;
#endif
#ifdef POLARSSL_MD4_C
    md4_context md4;
#endif
#ifdef POLARSSL_MD5_C
    md5_context md5;
#endif
#ifdef POLARSSL_SHA1_C
    sha1_context sha1;
#endif
#ifdef POLARSSL_SHA2_C
    sha2_context sha2;
#endif
#ifdef POLARSSL_SHA4_C
    sha4_context sha4;
#endif
  } eng;
  short h_idx;  /* index in hash engines union */
  short h_size;  /* output length */
} hash_context;

typedef void (*f_hash_t) (const unsigned char *in, size_t ilen, unsigned char *buf);
typedef int (*f_hash_file_t) (const char *path, unsigned char *buf);
typedef void (*f_hash_hmac_t) (const unsigned char *key, size_t klen,
                               const unsigned char *src, size_t slen,
                               unsigned char *buf);

typedef void (*f_hash_starts_t) (void *ctx);
typedef void (*f_hash_update_t) (void *ctx, const unsigned char *in, size_t ilen);
typedef void (*f_hash_finish_t) (void *ctx, unsigned char *buf);

typedef void (*f_hash_hmac_starts_t) (void *ctx, const unsigned char *key, size_t klen);
typedef void (*f_hash_hmac_update_t) (void *ctx, const unsigned char *in, size_t ilen);
typedef void (*f_hash_hmac_finish_t) (void *ctx, unsigned char *buf);
typedef void (*f_hash_hmac_reset_t) (void *ctx);


static void
lhash_push (lua_State *L, hash_context *ctx, unsigned char *p, const int raw)
{
  unsigned char buf[2 * HASH_BUFSIZE];
  int h_size = ctx->h_size;

  if (!raw) {
    int i;
    for (i = 0; i < h_size; ++i, ++p) {
      unsigned char c = (*p >> 4);
      buf[i * 2] = c + (c > 9 ? 'a' - 10 : '0');
      c = (*p & 0x0F);
      buf[i * 2 + 1] = c + (c > 9 ? 'a' - 10 : '0');
    }
    h_size *= 2;
  }
  lua_pushlstring(L, (char *) (raw ? p : buf), h_size);
}

/*
 * Arguments: ..., hash_type (string)
 */
static void
lhash_totype (lua_State *L, int idx, hash_context *ctx)
{
  static const char *const hash_names[] = {
#ifdef POLARSSL_MD2_C
    "MD2",
#endif
#ifdef POLARSSL_MD4_C
    "MD4",
#endif
#ifdef POLARSSL_MD5_C
    "MD5",
#endif
#ifdef POLARSSL_SHA1_C
    "SHA1",
#endif
#ifdef POLARSSL_SHA2_C
    "SHA224", "SHA256",
#endif
#ifdef POLARSSL_SHA4_C
    "SHA384", "SHA512",
#endif
    NULL
  };
  static const int hash_sizes[] = {
#ifdef POLARSSL_MD2_C
    16,
#endif
#ifdef POLARSSL_MD4_C
    16,
#endif
#ifdef POLARSSL_MD5_C
    16,
#endif
#ifdef POLARSSL_SHA1_C
    20,
#endif
#ifdef POLARSSL_SHA2_C
    28, 32,
#endif
#ifdef POLARSSL_SHA4_C
    48, 64
#endif
  };

  ctx->h_idx = luaL_checkoption(L, idx, NULL, hash_names);
  ctx->h_size = hash_sizes[ctx->h_idx];
}

#ifdef POLARSSL_SHA2_C

static void
sha224 (const unsigned char *input, size_t ilen, unsigned char *buf)
{
  sha2(input, ilen, buf, 1);
}

static void
sha256 (const unsigned char *input, size_t ilen, unsigned char *buf)
{
  sha2(input, ilen, buf, 0);
}

#endif

#ifdef POLARSSL_SHA4_C

static void
sha384 (const unsigned char *input, size_t ilen, unsigned char *buf)
{
  sha4(input, ilen, buf, 1);
}

static void
sha512 (const unsigned char *input, size_t ilen, unsigned char *buf)
{
  sha4(input, ilen, buf, 0);
}

#endif

/*
 * Arguments: hash_type (string), data (string), raw (boolean)
 * Returns: string
 */
static int
lhash_data (lua_State *L)
{
  static const f_hash_t hash_funcs[] = {
#ifdef POLARSSL_MD2_C
    md2,
#endif
#ifdef POLARSSL_MD4_C
    md4,
#endif
#ifdef POLARSSL_MD5_C
    md5,
#endif
#ifdef POLARSSL_SHA1_C
    sha1,
#endif
#ifdef POLARSSL_SHA2_C
    sha224, sha256,
#endif
#ifdef POLARSSL_SHA4_C
    sha384, sha512
#endif
  };

  hash_context ctx;
  size_t slen;
  const unsigned char *src = (const unsigned char *) luaL_checklstring(L, 2, &slen);
  const int raw = lua_toboolean(L, 3);
  unsigned char buf[HASH_BUFSIZE];

  lhash_totype(L, 1, &ctx);

  hash_funcs[ctx.h_idx](src, slen, buf);
  lhash_push(L, &ctx, buf, raw);
  return 1;
}

#ifdef POLARSSL_SHA2_C

static int
sha224_file (const char *path, unsigned char *buf)
{
  return sha2_file(path, buf, 1);
}

static int
sha256_file (const char *path, unsigned char *buf)
{
  return sha2_file(path, buf, 0);
}

#endif

#ifdef POLARSSL_SHA4_C

static int
sha384_file (const char *path, unsigned char *buf)
{
  return sha4_file(path, buf, 1);
}

static int
sha512_file (const char *path, unsigned char *buf)
{
  return sha4_file(path, buf, 0);
}

#endif

/*
 * Arguments: hash_type (string), path (string), raw (boolean)
 * Returns: [string]
 */
static int
lhash_file (lua_State *L)
{
  static const f_hash_file_t hash_file_funcs[] = {
#ifdef POLARSSL_MD2_C
    md2_file,
#endif
#ifdef POLARSSL_MD4_C
    md4_file,
#endif
#ifdef POLARSSL_MD5_C
    md5_file,
#endif
#ifdef POLARSSL_SHA1_C
    sha1_file,
#endif
#ifdef POLARSSL_SHA2_C
    sha224_file, sha256_file,
#endif
#ifdef POLARSSL_SHA4_C
    sha384_file, sha512_file
#endif
  };

  hash_context ctx;
  const char *path = luaL_checkstring(L, 2);
  const int raw = lua_toboolean(L, 3);
  unsigned char buf[HASH_BUFSIZE];
  int res;

  lhash_totype(L, 1, &ctx);

  res = hash_file_funcs[ctx.h_idx](path, buf);
  if (!res) {
    lhash_push(L, &ctx, buf, raw);
    return 1;
  }
  return lssl_seterror(L, -res);
}

#ifdef POLARSSL_SHA2_C

static void
sha224_hmac (const unsigned char *key, size_t klen,
             const unsigned char *src, size_t slen,
             unsigned char *buf)
{
  sha2_hmac(key, klen, src, slen, buf, 1);
}

static void
sha256_hmac (const unsigned char *key, size_t klen,
             const unsigned char *src, size_t slen,
             unsigned char *buf)
{
  sha2_hmac(key, klen, src, slen, buf, 0);
}

#endif

#ifdef POLARSSL_SHA4_C

static void
sha384_hmac (const unsigned char *key, size_t klen,
             const unsigned char *src, size_t slen,
             unsigned char *buf)
{
  sha4_hmac(key, klen, src, slen, buf, 1);
}

static void
sha512_hmac (const unsigned char *key, size_t klen,
             const unsigned char *src, size_t slen,
             unsigned char *buf)
{
  sha4_hmac(key, klen, src, slen, buf, 0);
}

#endif

/*
 * Arguments: hash_type (string), key (string), data (string), raw (boolean)
 * Returns: string
 */
static int
lhash_hmac (lua_State *L)
{
  static const f_hash_hmac_t hash_hmac_funcs[] = {
#ifdef POLARSSL_MD2_C
    md2_hmac,
#endif
#ifdef POLARSSL_MD4_C
    md4_hmac,
#endif
#ifdef POLARSSL_MD5_C
    md5_hmac,
#endif
#ifdef POLARSSL_SHA1_C
    sha1_hmac,
#endif
#ifdef POLARSSL_SHA2_C
    sha224_hmac, sha256_hmac,
#endif
#ifdef POLARSSL_SHA4_C
    sha384_hmac, sha512_hmac
#endif
  };

  hash_context ctx;
  size_t klen, slen;
  const unsigned char *key = (const unsigned char *) luaL_checklstring(L, 2, &klen);
  const unsigned char *src = (const unsigned char *) luaL_checklstring(L, 3, &slen);
  const int raw = lua_toboolean(L, 4);
  unsigned char buf[HASH_BUFSIZE];

  lhash_totype(L, 1, &ctx);

  hash_hmac_funcs[ctx.h_idx](key, klen, src, slen, buf);
  lhash_push(L, &ctx, buf, raw);
  return 1;
}


/*
 * Arguments: hash_type (string)
 * Returns: hash_udata
 */
static int
lhash_new (lua_State *L)
{
  hash_context *ctx = lua_newuserdata(L, sizeof(hash_context));

  luaL_getmetatable(L, HASH_TYPENAME);
  lua_setmetatable(L, -2);

  memset(ctx, 0, sizeof(hash_context));
  lhash_totype(L, 1, ctx);
  return 1;
}

#ifdef POLARSSL_SHA2_C

static void
sha224_starts (void *ctx)
{
  sha2_starts(ctx, 1);
}

static void
sha256_starts (void *ctx)
{
  sha2_starts(ctx, 0);
}

#endif

#ifdef POLARSSL_SHA4_C

static void
sha384_starts (void *ctx)
{
  sha4_starts(ctx, 1);
}

static void
sha512_starts (void *ctx)
{
  sha4_starts(ctx, 0);
}

#endif

/*
 * Arguments: hash_udata
 * Returns: hash_udata
 */
static int
lhash_starts (lua_State *L)
{
  static const f_hash_starts_t hash_starts_funcs[] = {
#ifdef POLARSSL_MD2_C
    (f_hash_starts_t) md2_starts,
#endif
#ifdef POLARSSL_MD4_C
    (f_hash_starts_t) md4_starts,
#endif
#ifdef POLARSSL_MD5_C
    (f_hash_starts_t) md5_starts,
#endif
#ifdef POLARSSL_SHA1_C
    (f_hash_starts_t) sha1_starts,
#endif
#ifdef POLARSSL_SHA2_C
    (f_hash_starts_t) sha224_starts,
    (f_hash_starts_t) sha256_starts,
#endif
#ifdef POLARSSL_SHA4_C
    (f_hash_starts_t) sha384_starts,
    (f_hash_starts_t) sha512_starts
#endif
  };

  hash_context *ctx = checkudata(L, 1, HASH_TYPENAME);

  return lssl_seterror(L,
   (hash_starts_funcs[ctx->h_idx](ctx), 0));
}

/*
 * Arguments: hash_udata, data (string)
 * Returns: hash_udata
 */
static int
lhash_update (lua_State *L)
{
  static const f_hash_update_t hash_update_funcs[] = {
#ifdef POLARSSL_MD2_C
    (f_hash_update_t) md2_update,
#endif
#ifdef POLARSSL_MD4_C
    (f_hash_update_t) md4_update,
#endif
#ifdef POLARSSL_MD5_C
    (f_hash_update_t) md5_update,
#endif
#ifdef POLARSSL_SHA1_C
    (f_hash_update_t) sha1_update,
#endif
#ifdef POLARSSL_SHA2_C
    (f_hash_update_t) sha2_update,
    (f_hash_update_t) sha2_update,
#endif
#ifdef POLARSSL_SHA4_C
    (f_hash_update_t) sha4_update,
    (f_hash_update_t) sha4_update
#endif
  };

  hash_context *ctx = checkudata(L, 1, HASH_TYPENAME);
  size_t slen;
  const unsigned char *src = (const unsigned char *) luaL_checklstring(L, 2, &slen);

  return lssl_seterror(L,
   (hash_update_funcs[ctx->h_idx](ctx, src, slen), 0));
}

/*
 * Arguments: hash_udata, raw (boolean)
 * Returns: string
 */
static int
lhash_finish (lua_State *L)
{
  static const f_hash_finish_t hash_finish_funcs[] = {
#ifdef POLARSSL_MD2_C
    (f_hash_finish_t) md2_finish,
#endif
#ifdef POLARSSL_MD4_C
    (f_hash_finish_t) md4_finish,
#endif
#ifdef POLARSSL_MD5_C
    (f_hash_finish_t) md5_finish,
#endif
#ifdef POLARSSL_SHA1_C
    (f_hash_finish_t) sha1_finish,
#endif
#ifdef POLARSSL_SHA2_C
    (f_hash_finish_t) sha2_finish,
    (f_hash_finish_t) sha2_finish,
#endif
#ifdef POLARSSL_SHA4_C
    (f_hash_finish_t) sha4_finish,
    (f_hash_finish_t) sha4_finish
#endif
  };

  hash_context *ctx = checkudata(L, 1, HASH_TYPENAME);
  const int raw = lua_toboolean(L, 2);
  unsigned char buf[HASH_BUFSIZE];

  hash_finish_funcs[ctx->h_idx](ctx, buf);
  lhash_push(L, ctx, buf, raw);
  return 1;
}

#ifdef POLARSSL_SHA2_C

static void
sha224_hmac_starts (void *ctx, const unsigned char *key, size_t klen)
{
  sha2_hmac_starts(ctx, key, klen, 1);
}

static void
sha256_hmac_starts (void *ctx, const unsigned char *key, size_t klen)
{
  sha2_hmac_starts(ctx, key, klen, 0);
}

#endif

#ifdef POLARSSL_SHA4_C

static void
sha384_hmac_starts (void *ctx, const unsigned char *key, size_t klen)
{
  sha4_hmac_starts(ctx, key, klen, 1);
}

static void
sha512_hmac_starts (void *ctx, const unsigned char *key, size_t klen)
{
  sha4_hmac_starts(ctx, key, klen, 0);
}

#endif

/*
 * Arguments: hash_udata, key (string)
 * Returns: hash_udata
 */
static int
lhash_hmac_starts (lua_State *L)
{
  static const f_hash_hmac_starts_t hash_hmac_starts_funcs[] = {
#ifdef POLARSSL_MD2_C
    (f_hash_hmac_starts_t) md2_hmac_starts,
#endif
#ifdef POLARSSL_MD4_C
    (f_hash_hmac_starts_t) md4_hmac_starts,
#endif
#ifdef POLARSSL_MD5_C
    (f_hash_hmac_starts_t) md5_hmac_starts,
#endif
#ifdef POLARSSL_SHA1_C
    (f_hash_hmac_starts_t) sha1_hmac_starts,
#endif
#ifdef POLARSSL_SHA2_C
    (f_hash_hmac_starts_t) sha224_hmac_starts,
    (f_hash_hmac_starts_t) sha256_hmac_starts,
#endif
#ifdef POLARSSL_SHA4_C
    (f_hash_hmac_starts_t) sha384_hmac_starts,
    (f_hash_hmac_starts_t) sha512_hmac_starts
#endif
  };

  hash_context *ctx = checkudata(L, 1, HASH_TYPENAME);
  size_t klen;
  const unsigned char *key = (const unsigned char *) luaL_checklstring(L, 2, &klen);

  return lssl_seterror(L,
   (hash_hmac_starts_funcs[ctx->h_idx](ctx, key, klen), 0));
}

/*
 * Arguments: data (string)
 * Returns: hash_udata
 */
static int
lhash_hmac_update (lua_State *L)
{
  static const f_hash_hmac_update_t hash_hmac_update_funcs[] = {
#ifdef POLARSSL_MD2_C
    (f_hash_hmac_update_t) md2_hmac_update,
#endif
#ifdef POLARSSL_MD4_C
    (f_hash_hmac_update_t) md4_hmac_update,
#endif
#ifdef POLARSSL_MD5_C
    (f_hash_hmac_update_t) md5_hmac_update,
#endif
#ifdef POLARSSL_SHA1_C
    (f_hash_hmac_update_t) sha1_hmac_update,
#endif
#ifdef POLARSSL_SHA2_C
    (f_hash_hmac_update_t) sha2_hmac_update,
    (f_hash_hmac_update_t) sha2_hmac_update,
#endif
#ifdef POLARSSL_SHA4_C
    (f_hash_hmac_update_t) sha4_hmac_update,
    (f_hash_hmac_update_t) sha4_hmac_update
#endif
  };

  hash_context *ctx = checkudata(L, 1, HASH_TYPENAME);
  size_t slen;
  const unsigned char *src = (const unsigned char *) luaL_checklstring(L, 2, &slen);

  return lssl_seterror(L,
   (hash_hmac_update_funcs[ctx->h_idx](ctx, src, slen), 0));
}

/*
 * Arguments: hash_udata, raw (boolean)
 * Returns: string
 */
static int
lhash_hmac_finish (lua_State *L)
{
  static const f_hash_hmac_finish_t hash_hmac_finish_funcs[] = {
#ifdef POLARSSL_MD2_C
    (f_hash_hmac_finish_t) md2_hmac_finish,
#endif
#ifdef POLARSSL_MD4_C
    (f_hash_hmac_finish_t) md4_hmac_finish,
#endif
#ifdef POLARSSL_MD5_C
    (f_hash_hmac_finish_t) md5_hmac_finish,
#endif
#ifdef POLARSSL_SHA1_C
    (f_hash_hmac_finish_t) sha1_hmac_finish,
#endif
#ifdef POLARSSL_SHA2_C
    (f_hash_hmac_finish_t) sha2_hmac_finish,
    (f_hash_hmac_finish_t) sha2_hmac_finish,
#endif
#ifdef POLARSSL_SHA4_C
    (f_hash_hmac_finish_t) sha4_hmac_finish,
    (f_hash_hmac_finish_t) sha4_hmac_finish
#endif
  };

  hash_context *ctx = checkudata(L, 1, HASH_TYPENAME);
  const int raw = lua_toboolean(L, 2);
  unsigned char buf[HASH_BUFSIZE];

  hash_hmac_finish_funcs[ctx->h_idx](ctx, buf);
  lhash_push(L, ctx, buf, raw);
  return 1;
}

/*
 * Arguments: hash_udata
 * Returns: hash_udata
 */
static int
lhash_hmac_reset (lua_State *L)
{
  static const f_hash_hmac_reset_t hash_hmac_reset_funcs[] = {
#ifdef POLARSSL_MD2_C
    (f_hash_hmac_reset_t) md2_hmac_reset,
#endif
#ifdef POLARSSL_MD4_C
    (f_hash_hmac_reset_t) md4_hmac_reset,
#endif
#ifdef POLARSSL_MD5_C
    (f_hash_hmac_reset_t) md5_hmac_reset,
#endif
#ifdef POLARSSL_SHA1_C
    (f_hash_hmac_reset_t) sha1_hmac_reset,
#endif
#ifdef POLARSSL_SHA2_C
    (f_hash_hmac_reset_t) sha2_hmac_reset,
    (f_hash_hmac_reset_t) sha2_hmac_reset,
#endif
#ifdef POLARSSL_SHA4_C
    (f_hash_hmac_reset_t) sha4_hmac_reset,
    (f_hash_hmac_reset_t) sha4_hmac_reset
#endif
  };

  hash_context *ctx = checkudata(L, 1, HASH_TYPENAME);

  return lssl_seterror(L,
   (hash_hmac_reset_funcs[ctx->h_idx](ctx), 0));
}

/*
 * Arguments: hash_udata
 * Returns: string
 */
static int
lhash_tostring (lua_State *L)
{
  hash_context *ctx = checkudata(L, 1, HASH_TYPENAME);

  lua_pushfstring(L, HASH_TYPENAME " (%p)", ctx);
  return 1;
}


#define HASH_METHODS \
  {"hash_data",		lhash_data}, \
  {"hash_file",		lhash_file}, \
  {"hash_hmac",		lhash_hmac}, \
  {"hash",		lhash_new}

static luaL_Reg lhash_meth[] = {
  {"starts",		lhash_starts},
  {"update",		lhash_update},
  {"finish",		lhash_finish},
  {"hmac_starts",	lhash_hmac_starts},
  {"hmac_update",	lhash_hmac_update},
  {"hmac_finish",	lhash_hmac_finish},
  {"hmac_reset",	lhash_hmac_reset},
  {"__tostring",	lhash_tostring},
  {NULL, NULL}
};
