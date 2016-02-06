/* Lua PolarSSL: SSL/TLS functions */

#include "polarssl/ssl.h"
#include "polarssl/entropy.h"
#include "polarssl/ctr_drbg.h"
#include "polarssl/certs.h"


#define SSL_TYPENAME	"polarssl.ssl"

/*
 * Computing a "safe" DH-1024 prime can take a very
 * long time, so a precomputed value is provided below.
 * You may run dh_genprime to generate a new value.
 */
static const char * const default_dhm_P =
  "E4004C1F94182000103D883A448B3F80" \
  "2CE4B44A83301270002C20D0321CFD00" \
  "11CCEF784C26A400F43DFB901BCA7538" \
  "F2C6B176001CF5A0FD16D2C48B1D0C1C" \
  "F6AC8E1DA6BCC3B4E1F96B0564965300" \
  "FFA1D0B601EB2800F489AA512C4B248C" \
  "01F76949A60BB7F00A40B1EAB64BDD48" \
  "E8A700D60B7F1200FA8E77B0A979DABF";

static const char * const default_dhm_G = "4";

/* SSL Context environ. table reserved indexes */
enum {
  LSSL_CIPHERSUITES = 1,  /* custom ciphersuites */
  LSSL_CA_CERT,  /* own trusted CA chain */
  LSSL_CA_CRL,  /* trusted CA CRLs */
  LSSL_OWN_CERT,  /* own X.509 certificate */
  LSSL_PEER_CN,  /* expected peer CN */
  LSSL_RNG,  /* random number generator callback */
  LSSL_PRNG,  /* context for the RNG function */
  LSSL_DBG,  /* debug callback */
  LSSL_PDBG,  /* context for the debug function */
  LSSL_BIO_RECV,  /* BIO read callback */
  LSSL_BIO_PRECV,  /* context for reading operations */
  LSSL_BIO_SEND,  /* BIO write callback */
  LSSL_BIO_PSEND,  /* context for writing operations */
  LSSL_SESS_GET,  /* (server) session get callback */
  LSSL_SESS_SET,  /* (server) session set callback */
  LSSL_SESS_CUR,  /* current session */
  LSSL_SNI,  /* ServerName TLS extension callback */
  LSSL_PSNI,  /* context for the ServerName TLS extension function */
  LSSL_ENV_MAX
};

typedef struct {
  ssl_context ssl;
  ssl_session ssn;
  rsa_context rsa_key;

  ctr_drbg_context ctr_drbg;

  lua_State *L;

  FILE *dbg_file;
  int dbg_level;

  size_t bio_len;
  unsigned char *bio_buf;
} lssl_context;

typedef int (*f_rng_t) (void *ud, unsigned char *buf, size_t len);
typedef void (*f_dbg_t) (void *ud, int level, const char *str);
typedef int (*f_recv_t) (void *ud, unsigned char *buf, size_t len);
typedef int (*f_send_t) (void *ud, const unsigned char *buf, size_t len);
typedef int (*f_get_cache_t) (void *ud, ssl_session *ssn);
typedef int (*f_set_cache_t) (void *ud, const ssl_session *ssn);
typedef int (*f_sni_t) (void *ud, ssl_context *ctx,
                        const unsigned char *buf, size_t len);


static void *g_EntropyKey;


/*
 * Returns: ssl_udata
 */
static int
lssl_new (lua_State *L)
{
  lua_newuserdata(L, sizeof(lssl_context));

  luaL_getmetatable(L, SSL_TYPENAME);
  lua_setmetatable(L, -2);

  lua_newtable(L);  /* environ. */
  lua_setfenv(L, -2);
  return 1;
}

static int
lssl_bio_cb (lssl_context *ctx, unsigned char *buf, size_t n)
{
  if (ctx->bio_len == 0) {
    ctx->bio_len = n;
    ctx->bio_buf = buf;
  }
  else if (ctx->bio_buf == NULL) {
    n = ctx->bio_len;
    ctx->bio_len = 0;
    return n;
  }
  return -1;
}

/*
 * Arguments: ssl_udata
 * Returns: [buffer (ludata), length (number)]
 */
static int
lssl_bio_begin (lua_State *L)
{
  lssl_context *ctx = checkudata(L, 1, SSL_TYPENAME);

  if (ctx->bio_buf == NULL)
    lua_settop(L, 0);
  else {
    lua_pushlightuserdata(L, ctx->bio_buf);
    lua_pushinteger(L, ctx->bio_len);
  }
  return 2;
}

/*
 * Arguments: ssl_udata, length (number)
 */
static int
lssl_bio_end (lua_State *L)
{
  lssl_context *ctx = checkudata(L, 1, SSL_TYPENAME);
  const size_t n = lua_tointeger(L, 2);

  if (ctx->bio_buf != NULL) {
    ctx->bio_buf = NULL;
    ctx->bio_len = n;
  }
  return 0;
}

/*
 * Arguments: ssl_udata
 * Returns: ssl_udata
 */
static int
lssl_init (lua_State *L)
{
  lssl_context *ctx = checkudata(L, 1, SSL_TYPENAME);
  ssl_context *ssl = &ctx->ssl;
  int res;

  memset(ctx, 0, sizeof(lssl_context));

  res = ssl_init(ssl);
  if (!res) {
    entropy_context *entropy;

    lua_rawgetp(L, LUA_REGISTRYINDEX, &g_EntropyKey);
    entropy = lua_touserdata(L, -1);
    lua_pop(L, 1);
    if (!entropy) {
      entropy = lua_newuserdata(L, sizeof(entropy_context));
      lua_rawsetp(L, LUA_REGISTRYINDEX, &g_EntropyKey);
      entropy_init(entropy);
    }
    rsa_init(&ctx->rsa_key, RSA_PKCS_V15, 0);
    res = ctr_drbg_init(&ctx->ctr_drbg, entropy_func, entropy, NULL, 0);
    if (!res) {
      ssl_set_rng(ssl, ctr_drbg_random, &ctx->ctr_drbg);
      ssl_set_bio(ssl, (f_recv_t) lssl_bio_cb, ctx,
       (f_send_t) lssl_bio_cb, ctx);
      ssl_set_session(ssl, &ctx->ssn);
      ssl_set_dh_param(ssl, default_dhm_P, default_dhm_G);
    }
  }
  return lssl_seterror(L, res);
}

/*
 * Arguments: ssl_udata
 */
static int
lssl_close (lua_State *L)
{
  lssl_context *ctx = checkudata(L, 1, SSL_TYPENAME);
  int i;

  lua_getfenv(L, 1);
  for (i = 1; i < LSSL_ENV_MAX; ++i) {
    lua_pushnil(L);
    lua_rawseti(L, -2, i);
  }

  rsa_free(&ctx->rsa_key);
  ssl_free(&ctx->ssl);

  memset(ctx, 0, sizeof(lssl_context));
  return 0;
}

/*
 * Arguments: ssl_udata
 * Returns: ssl_udata
 */
static int
lssl_session_reset (lua_State *L)
{
  lssl_context *ctx = checkudata(L, 1, SSL_TYPENAME);

  return lssl_seterror(L,
   ssl_session_reset(&ctx->ssl));
}

/*
 * Arguments: ssl_udata, [upper_bound (number)
 *	| buffer (ludata), buffer_length (number)]
 * Returns: number
 */
static int
lssl_random (lua_State *L)
{
  lssl_context *ctx = checkudata(L, 1, SSL_TYPENAME);
  const int is_udata = lua_isuserdata(L, 2);
  const unsigned int ub = is_udata ? 0 : lua_tointeger(L, 2);
  unsigned int num;
  unsigned char *buf = is_udata ? lua_touserdata(L, 2) : &num;
  const int len = is_udata ? luaL_checkinteger(L, 3) : (int) sizeof(num);
  const int res = ctx->ssl.f_rng(&ctx->ssl.p_rng, buf, len);

  if (!res) {
    lua_pushinteger(L, is_udata ? 1
     : (ub ? num % ub : num));
    return 1;
  }
  return lssl_seterror(L, res);
}

/*
 * Arguments: ssl_udata, endpoint (string: "client", "server")
 * Returns: ssl_udata
 */
static int
lssl_set_endpoint (lua_State *L)
{
  ssl_context *ssl = checkudata(L, 1, SSL_TYPENAME);
  const char *s = luaL_checkstring(L, 2);
  const int endpoint = (*s == 's') ? SSL_IS_SERVER : SSL_IS_CLIENT;

  return lssl_seterror(L,
   (ssl_set_endpoint(ssl, endpoint), 0));
}

/*
 * Arguments: ssl_udata, authmode (string: "none", "optional", "required")
 * Returns: ssl_udata
 */
static int
lssl_set_authmode (lua_State *L)
{
  ssl_context *ssl = checkudata(L, 1, SSL_TYPENAME);
  const char *s = luaL_checkstring(L, 2);
  const int authmode = (*s == 'n') ? SSL_VERIFY_NONE
   : (*s == 'o' ? SSL_VERIFY_OPTIONAL : SSL_VERIFY_REQUIRED);

  return lssl_seterror(L,
   (ssl_set_authmode(ssl, authmode), 0));
}

/*
 * Arguments: ssl_udata, ..., environ. (table)
 */
static int
lssl_rng_cb (lssl_context *ctx, unsigned char *buf, size_t len)
{
  lua_State *L = ctx->L;
  int res;

  lua_rawgeti(L, -1, LSSL_RNG);  /* function */
  lua_rawgeti(L, -2, LSSL_PRNG);  /* rng_context */
  lua_pushlightuserdata(L, buf);
  lua_pushinteger(L, len);
  lua_call(L, 1, 3);
  res = lua_tointeger(L, -1);
  lua_pop(L, 1);
  return res;
}

/*
 * Arguments: ssl_udata, rng_callback (function), rng_context (any)
 * Returns: ssl_udata
 */
static int
lssl_set_rng (lua_State *L)
{
  lssl_context *ctx = checkudata(L, 1, SSL_TYPENAME);
  const int is_nil = lua_isnoneornil(L, 2);

  lua_settop(L, 3);
  lua_getfenv(L, 1);
  lua_pushvalue(L, 2);
  lua_rawseti(L, -2, LSSL_RNG);
  lua_pushvalue(L, 3);
  lua_rawseti(L, -2, LSSL_PRNG);

  return lssl_seterror(L, (ssl_set_rng(&ctx->ssl,
   (is_nil ? ctr_drbg_random : (f_rng_t) lssl_rng_cb),
   (is_nil ? (void *) &ctx->ctr_drbg : ctx)), 0));
}

/*
 * Arguments: ssl_udata, ..., environ. (table)
 */
static void
lssl_dbg_cb (lssl_context *ctx, int level, const char *str)
{
  lua_State *L = ctx->L;

  if (level >= ctx->dbg_level) return;

  if (ctx->dbg_file != NULL) {
    fputs(str, ctx->dbg_file);
    fflush(ctx->dbg_file);
    return;
  }

  lua_rawgeti(L, -1, LSSL_DBG);  /* function */
  lua_rawgeti(L, -2, LSSL_PDBG);  /* debug_context */
  lua_pushinteger(L, level);  /* level */
  lua_pushstring(L, str);  /* text */
  lua_call(L, 3, 0);
}

/*
 * Arguments: ssl_udata, file_udata | callback (function),
 *	debug_context (any)
 * Returns: ssl_udata
 */
static int
lssl_set_dbg (lua_State *L)
{
  lssl_context *ctx = checkudata(L, 1, SSL_TYPENAME);
  const int is_nil = lua_isnoneornil(L, 2);

  ctx->dbg_file = lua_isuserdata(L, 2)
   ? lua_unboxpointer(L, 2, LUA_FILEHANDLE) : NULL;

  lua_settop(L, 3);
  lua_getfenv(L, 1);
  lua_pushvalue(L, 2);
  lua_rawseti(L, -2, LSSL_DBG);
  lua_pushvalue(L, 3);
  lua_rawseti(L, -2, LSSL_PDBG);

  return lssl_seterror(L, (ssl_set_dbg(&ctx->ssl,
   (is_nil ? NULL : (f_dbg_t) lssl_dbg_cb), ctx), 0));
}

/*
 * Arguments: ssl_udata, [debug_level (number)]
 * Returns: debug_level (number)
 */
static int
lssl_dbg_level (lua_State *L)
{
  lssl_context *ctx = checkudata(L, 1, SSL_TYPENAME);
  const int nargs = lua_gettop(L);

  lua_pushinteger(L, ctx->dbg_level);
  if (nargs > 1) {
    ctx->dbg_level = lua_tointeger(L, 2);
  }
  return 1;
}

/*
 * Arguments: ssl_udata, ..., environ. (table)
 */
static int
lssl_bio_recv_cb (lssl_context *ctx, unsigned char *buf, size_t n)
{
  lua_State *L = ctx->L;
  const char *s;

  lua_rawgeti(L, -1, LSSL_BIO_RECV);  /* function */
  lua_rawgeti(L, -2, LSSL_BIO_PRECV);  /* recv_context */
  lua_pushinteger(L, n);  /* number of bytes */
  lua_call(L, 2, 1);
  s = lua_tolstring(L, -1, &n);  /* data */
  if (s)
    memcpy(buf, s, n);
  else
    n = -1;
  lua_pop(L, 1);
  return n;
}

/*
 * Arguments: ssl_udata, ..., environ. (table)
 */
static int
lssl_bio_send_cb (lssl_context *ctx, const unsigned char *buf, size_t n)
{
  lua_State *L = ctx->L;
  int res;

  lua_rawgeti(L, -1, LSSL_BIO_SEND);  /* function */
  lua_rawgeti(L, -2, LSSL_BIO_PSEND);  /* send_context */
  lua_pushlstring(L, (const char *) buf, n);  /* data */
  lua_call(L, 2, 2);
  res = lua_isnil(L, -2) ? -1
   : lua_tointeger(L, -1);  /* number of bytes */
  lua_pop(L, 2);
  return res;
}

/*
 * Arguments: ssl_udata,
 *	recv_callback (function), recv_context (any),
 *	send_callback (function), send_context (any)
 * Returns: ssl_udata
 */
static int
lssl_set_bio (lua_State *L)
{
  lssl_context *ctx = checkudata(L, 1, SSL_TYPENAME);
  const int is_nil = lua_isnoneornil(L, 2);

  lua_settop(L, 5);
  lua_getfenv(L, 1);
  lua_pushvalue(L, 2);
  lua_rawseti(L, -2, LSSL_BIO_RECV);
  lua_pushvalue(L, 3);
  lua_rawseti(L, -2, LSSL_BIO_PRECV);
  lua_pushvalue(L, 4);
  lua_rawseti(L, -2, LSSL_BIO_SEND);
  lua_pushvalue(L, 5);
  lua_rawseti(L, -2, LSSL_BIO_PSEND);

  return lssl_seterror(L, (ssl_set_bio(&ctx->ssl,
   (is_nil ? (f_recv_t) lssl_bio_cb : (f_recv_t) lssl_bio_recv_cb), ctx,
   (is_nil ? (f_send_t) lssl_bio_cb : (f_send_t) lssl_bio_send_cb), ctx), 0));
}

/*
 * Arguments: ssl_udata, ..., environ. (table)
 */
static int
lssl_session_get_cb (lssl_context *ctx, ssl_session *ssn)
{
  lua_State *L = ctx->L;
  ssl_session *ssn_cache;

  lua_rawgeti(L, -1, LSSL_SESS_GET);  /* function */
  lua_pushvalue(L, 1);  /* ssl_udata */
  lsession_pushid(L, ssn);
  lua_call(L, 2, 1);
  ssn_cache = lua_isuserdata(L, -1)
   ? checkudata(L, -1, SESSION_TYPENAME) : NULL;
  lua_rawseti(L, -2, LSSL_SESS_CUR);  /* [session_udata] */
  if (ssn_cache) {
    memcpy(ssn->master, ssn_cache->master, sizeof(ssn_cache->master));
    return 0;
  }
  return 1;
}

/*
 * Arguments: ssl_udata, ..., environ. (table)
 */
static int
lssl_session_set_cb (lssl_context *ctx, const ssl_session *ssn)
{
  lua_State *L = ctx->L;
  ssl_session *ssn_cache;

  lua_rawgeti(L, -1, LSSL_SESS_SET);  /* function */
  lua_pushvalue(L, 1);  /* ssl_udata */
  lsession_pushid(L, ssn);
  lua_call(L, 2, 1);
  ssn_cache = lua_isuserdata(L, -1)
   ? checkudata(L, -1, SESSION_TYPENAME) : NULL;
  lua_rawseti(L, -2, LSSL_SESS_CUR);  /* [session_udata] */
  if (ssn_cache) {
    *ssn_cache = *ssn;
    ssn_cache->peer_cert = NULL;
    return 0;
  }
  return 1;
}

/*
 * Arguments: ssl_udata, get_session_callback (function),
 *	set_session_callback (function)
 * Returns: ssl_udata
 */
static int
lssl_set_session_cache (lua_State *L)
{
  lssl_context *ctx = checkudata(L, 1, SSL_TYPENAME);
  const int is_nil = lua_isnoneornil(L, 2);

  lua_settop(L, 3);
  lua_getfenv(L, 1);
  lua_pushvalue(L, 2);
  lua_rawseti(L, -2, LSSL_SESS_GET);
  lua_pushvalue(L, 3);
  lua_rawseti(L, -2, LSSL_SESS_SET);

  return lssl_seterror(L, (ssl_set_session_cache(&ctx->ssl,
   (is_nil ? NULL : (f_get_cache_t) lssl_session_get_cb), ctx,
   (is_nil ? NULL : (f_set_cache_t) lssl_session_set_cb), ctx), 0));
}

/*
 * Arguments: ssl_udata, ..., environ. (table)
 */
static int
lssl_sni_cb (lssl_context *ctx, ssl_context *ssl,
             const unsigned char *buf, size_t len)
{
  lua_State *L = ctx->L;
  int res;

  (void) ssl;

  lua_rawgeti(L, -1, LSSL_RNG);  /* function */
  lua_rawgeti(L, -2, LSSL_PRNG);  /* sni_context */
  lua_pushlstring(L, (const char *) buf, len);  /* hostname */
  lua_call(L, 1, 2);
  res = lua_tointeger(L, -1);
  lua_pop(L, 1);
  return res;
}

/*
 * Arguments: ssl_udata, sni_callback (function), sni_context (any)
 * Returns: ssl_udata
 */
static int
lssl_set_sni (lua_State *L)
{
  lssl_context *ctx = checkudata(L, 1, SSL_TYPENAME);
  const int is_nil = lua_isnoneornil(L, 2);

  lua_settop(L, 3);
  lua_getfenv(L, 1);
  lua_pushvalue(L, 2);
  lua_rawseti(L, -2, LSSL_SNI);
  lua_pushvalue(L, 3);
  lua_rawseti(L, -2, LSSL_PSNI);

  return lssl_seterror(L, (ssl_set_sni(&ctx->ssl,
   (is_nil ? NULL : (f_sni_t) lssl_sni_cb), ctx), 0));
}

/*
 * Arguments: ssl_udata, [ciphersuite_names (table: 1..n => name)]
 * Returns: ssl_udata
 */
static int
lssl_set_ciphersuites (lua_State *L)
{
  lssl_context *ctx = checkudata(L, 1, SSL_TYPENAME);
  const int *ciphersuites;

  if (lua_istable(L, 2)) {
    const int n = lua_rawlen(L, 2);
    int *cp = lua_newuserdata(L, (n + 1) * sizeof(int));
    int i;

    ciphersuites = cp;

    for (i = 1; i <= n; ++i) {
      lua_rawgeti(L, 2, i);
      *cp++ = ssl_get_ciphersuite_id(luaL_checkstring(L, -1));
      lua_pop(L, 1);
    }
    *cp = 0;
  } else {
    ciphersuites = ssl_default_ciphersuites;
    lua_pushnil(L);
  }

  lua_getfenv(L, 1);
  lua_pushvalue(L, -2);
  lua_rawseti(L, -2, LSSL_CIPHERSUITES);

  return lssl_seterror(L,
   (ssl_set_ciphersuites(&ctx->ssl, ciphersuites), 0));
}

/*
 * Arguments: ssl_udata, peer_cn (string)
 * Returns: ssl_udata
 */
static int
lssl_set_peer_cn (lua_State *L)
{
  ssl_context *ssl = checkudata(L, 1, SSL_TYPENAME);
  const char *peer_cn = lua_tostring(L, 2);

  lua_settop(L, 2);
  lua_getfenv(L, 1);
  lua_pushvalue(L, 2);
  lua_rawseti(L, -2, LSSL_PEER_CN);

  ssl->peer_cn = peer_cn;
  return lssl_seterror(L, 0);
}

/*
 * Arguments: ssl_udata, x509_cert_udata, [chain_offset (number)]
 * Returns: ssl_udata
 */
static int
lssl_set_ca_cert (lua_State *L)
{
  ssl_context *ssl = checkudata(L, 1, SSL_TYPENAME);
  x509_cert *crt = checkudata(L, 2, X509_CERT_TYPENAME);
  int off = lua_tointeger(L, 3);

  lua_settop(L, 2);
  lua_getfenv(L, 1);
  lua_pushvalue(L, 2);
  lua_rawseti(L, -2, LSSL_CA_CERT);

  while (off--) {
    crt = crt->next;
    if (!crt)
      return lssl_seterror(L, 1);
  }
  ssl->ca_chain = crt;
  return lssl_seterror(L, 0);
}

/*
 * Arguments: ssl_udata, x509_crl_udata, [chain_offset (number)]
 * Returns: ssl_udata
 */
static int
lssl_set_ca_crl (lua_State *L)
{
  ssl_context *ssl = checkudata(L, 1, SSL_TYPENAME);
  x509_crl *crl = checkudata(L, 2, X509_CRL_TYPENAME);
  int off = lua_tointeger(L, 3);

  lua_settop(L, 2);
  lua_getfenv(L, 1);
  lua_pushvalue(L, 2);
  lua_rawseti(L, -2, LSSL_CA_CRL);

  while (off--) {
    crl = crl->next;
    if (!crl)
      return lssl_seterror(L, 1);
  }
  ssl->ca_crl = crl;
  return lssl_seterror(L, 0);
}

/*
 * Arguments: ssl_udata, x509_cert_udata, [chain_offset (number)]
 * Returns: ssl_udata
 */
static int
lssl_set_own_cert (lua_State *L)
{
  ssl_context *ssl = checkudata(L, 1, SSL_TYPENAME);
  x509_cert *crt = checkudata(L, 2, X509_CERT_TYPENAME);
  int off = lua_tointeger(L, 3);

  lua_settop(L, 2);
  lua_getfenv(L, 1);
  lua_pushvalue(L, 2);
  lua_rawseti(L, -2, LSSL_OWN_CERT);

  while (off--) {
    crt = crt->next;
    if (!crt)
      return lssl_seterror(L, 1);
  }
  ssl->own_cert = crt;
  return lssl_seterror(L, 0);
}

static int
lssl_set_rsa (lua_State *L, lssl_context *ctx, rsa_context *rsa_key, int res)
{
  if (!res) {
    rsa_free(&ctx->rsa_key);
    ctx->rsa_key = *rsa_key;
    ctx->ssl.rsa_key = &ctx->rsa_key;
  }
  return lssl_seterror(L, res);
}

/*
 * Arguments: ssl_udata, key (string), [password (string)]
 * Returns: ssl_udata
 */
static int
lssl_set_rsa_key (lua_State *L)
{
  lssl_context *ctx = checkudata(L, 1, SSL_TYPENAME);
  size_t keylen, pwdlen;
  const unsigned char *key = (const unsigned char *) luaL_checklstring(L, 2, &keylen);
  const unsigned char *pwd = (const unsigned char *) lua_tolstring(L, 3, &pwdlen);
  rsa_context rsa_key;

  rsa_init(&rsa_key, RSA_PKCS_V15, 0);

  return lssl_set_rsa(L, ctx, &rsa_key,
   x509parse_key(&rsa_key, key, keylen, pwd, pwdlen));
}

/*
 * Arguments: ssl_udata, path (string), [password (string)]
 * Returns: ssl_udata
 */
static int
lssl_set_rsa_keyfile (lua_State *L)
{
  lssl_context *ctx = checkudata(L, 1, SSL_TYPENAME);
  const char *path = luaL_checkstring(L, 2);
  const char *pwd = lua_tostring(L, 3);
  rsa_context rsa_key;

  rsa_init(&rsa_key, RSA_PKCS_V15, 0);

  return lssl_set_rsa(L, ctx, &rsa_key,
   x509parse_keyfile(&rsa_key, path, pwd));
}

/*
 * Arguments: ssl_udata
 * Returns: ssl_udata
 */
static int
lssl_set_rsa_keytest (lua_State *L)
{
  lssl_context *ctx = checkudata(L, 1, SSL_TYPENAME);
  const char *key = (ctx->ssl.endpoint == SSL_IS_SERVER)
   ? test_srv_key : test_cli_key;
  rsa_context rsa_key;

  rsa_init(&rsa_key, RSA_PKCS_V15, 0);

  return lssl_set_rsa(L, ctx, &rsa_key,
   x509parse_key(&rsa_key, (const unsigned char *) key, strlen(key), NULL, 0));
}

/*
 * Arguments: ssl_udata, dhm_P (string), dhm_G (string)
 * Returns: ssl_udata
 */
static int
lssl_set_dh_param (lua_State *L)
{
  ssl_context *ssl = checkudata(L, 1, SSL_TYPENAME);
  const char *dhm_P = lua_tostring(L, 2);
  const char *dhm_G = lua_tostring(L, 3);

  return lssl_seterror(L, ssl_set_dh_param(ssl,
   dhm_P ? dhm_P : default_dhm_P,
   dhm_G ? dhm_G : default_dhm_G));
}

/*
 * Arguments: ssl_udata, hostname (string)
 * Returns: ssl_udata
 */
static int
lssl_set_hostname (lua_State *L)
{
  ssl_context *ssl = checkudata(L, 1, SSL_TYPENAME);
  const char *hostname = luaL_checkstring(L, 2);

  return lssl_seterror(L, ssl_set_hostname(ssl, hostname));
}

/*
 * Arguments: ssl_udata, major (number), minor (number)
 * Returns: ssl_udata
 */
static int
lssl_set_max_version (lua_State *L)
{
  ssl_context *ssl = checkudata(L, 1, SSL_TYPENAME);
  const int major = luaL_checkint(L, 2);
  const int minor = luaL_checkint(L, 3);

  return lssl_seterror(L,
   (ssl_set_max_version(ssl, major, minor), 0));
}

/*
 * Arguments: ssl_udata, major (number), minor (number)
 * Returns: ssl_udata
 */
static int
lssl_set_min_version (lua_State *L)
{
  ssl_context *ssl = checkudata(L, 1, SSL_TYPENAME);
  const int major = luaL_checkint(L, 2);
  const int minor = luaL_checkint(L, 3);

  return lssl_seterror(L,
   (ssl_set_min_version(ssl, major, minor), 0));
}

/*
 * Arguments: ssl_udata, renegotiation (boolean)
 * Returns: ssl_udata
 */
static int
lssl_set_renegotiation (lua_State *L)
{
  ssl_context *ssl = checkudata(L, 1, SSL_TYPENAME);
  const int renegotiation = lua_toboolean(L, 2)
   ? SSL_RENEGOTIATION_ENABLED : SSL_RENEGOTIATION_DISABLED;

  return lssl_seterror(L,
   (ssl_set_renegotiation(ssl, renegotiation), 0));
}

/*
 * Arguments: ssl_udata, [legacy (string: "no", "allow", "break")]
 * Returns: ssl_udata
 */
static int
lssl_legacy_renegotiation (lua_State *L)
{
  ssl_context *ssl = checkudata(L, 1, SSL_TYPENAME);
  const char *s = lua_tostring(L, 2);
  const int renegotiation = (s && *s == 'a') ? SSL_LEGACY_ALLOW_RENEGOTIATION
   : (s && *s == 'b' ? SSL_LEGACY_BREAK_HANDSHAKE
   : SSL_LEGACY_NO_RENEGOTIATION);

  return lssl_seterror(L,
   (ssl_legacy_renegotiation(ssl, renegotiation), 0));
}

/*
 * Arguments: ssl_udata
 * Returns: number
 */
static int
lssl_get_bytes_avail (lua_State *L)
{
  ssl_context *ssl = checkudata(L, 1, SSL_TYPENAME);

  lua_pushinteger(L, ssl_get_bytes_avail(ssl));
  return 1;
}

/*
 * Arguments: ssl_udata
 * Returns: expired (boolean), revoked (boolean),
 *	cn_mismatch (boolean), not_trusted (boolean)
 */
static int
lssl_get_verify_result (lua_State *L)
{
  lssl_context *ctx = checkudata(L, 1, SSL_TYPENAME);
  const int res = ssl_get_verify_result(&ctx->ssl);

  lua_pushboolean(L, res & BADCERT_EXPIRED);
  lua_pushboolean(L, res & BADCERT_REVOKED);
  lua_pushboolean(L, res & BADCERT_CN_MISMATCH);
  lua_pushboolean(L, res & BADCERT_NOT_TRUSTED);
  return 4;
}

/*
 * Arguments: ssl_udata
 * Returns: cipher_name (string)
 */
static int
lssl_get_ciphersuite (lua_State *L)
{
  lssl_context *ctx = checkudata(L, 1, SSL_TYPENAME);

  lua_pushstring(L, ssl_get_ciphersuite(&ctx->ssl));
  return 1;
}

/*
 * Arguments: ssl_udata
 * Returns: version_name (string)
 */
static int
lssl_get_version (lua_State *L)
{
  lssl_context *ctx = checkudata(L, 1, SSL_TYPENAME);

  lua_pushstring(L, ssl_get_version(&ctx->ssl));
  return 1;
}

/*
 * Arguments: ssl_udata, x509_cert_udata
 * Returns: [ssl_udata]
 */
static int
lssl_get_peer_cert (lua_State *L)
{
  ssl_context *ssl = checkudata(L, 1, SSL_TYPENAME);
  x509_cert *crt = checkudata(L, 2, X509_CERT_TYPENAME);
  const x509_cert *peer_cert = ssl_get_peer_cert(ssl);

  if (!peer_cert) return 0;

  *crt = *peer_cert;
  return lssl_seterror(L, 0);
}

/*
 * Arguments: ssl_udata
 * Returns: ssl_udata
 */
static int
lssl_handshake (lua_State *L)
{
  lssl_context *ctx = checkudata(L, 1, SSL_TYPENAME);

  lua_getfenv(L, 1);

  ctx->L = L;
  return lssl_seterror(L, ssl_handshake(&ctx->ssl));
}

/*
 * Arguments: ssl_udata
 * Returns: ssl_udata
 */
static int
lssl_handshake_step (lua_State *L)
{
  lssl_context *ctx = checkudata(L, 1, SSL_TYPENAME);

  lua_getfenv(L, 1);

  ctx->L = L;
  return lssl_seterror(L, ssl_handshake_step(&ctx->ssl));
}

/*
 * Arguments: ssl_udata
 * Returns: ssl_udata
 */
static int
lssl_renegotiate (lua_State *L)
{
  lssl_context *ctx = checkudata(L, 1, SSL_TYPENAME);

  lua_getfenv(L, 1);

  ctx->L = L;
  return lssl_seterror(L, ssl_renegotiate(&ctx->ssl));
}

/*
 * Arguments: ssl_udata, [buffer (ludata), length (number)]
 * Returns: number | string
 */
static int
lssl_read (lua_State *L)
{
  lssl_context *ctx = checkudata(L, 1, SSL_TYPENAME);
  const int is_udata = lua_isuserdata(L, 2);
  unsigned char buffer[4096];
  unsigned char *buf = is_udata ? lua_touserdata(L, 2) : buffer;
  const size_t len = is_udata ? (size_t) lua_tointeger(L, 3) : sizeof(buffer);
  int res;

  lua_settop(L, 1);
  lua_getfenv(L, 1);

  ctx->L = L;
  res = ssl_read(&ctx->ssl, buf, len);
  if (res >= 0) {
    if (is_udata)
      lua_pushinteger(L, res);
    else
      lua_pushlstring(L, (char *) buf, res);
    return 1;
  }
  return lssl_seterror(L, res);
}

/*
 * Arguments: ssl_udata, string | {buffer (ludata), length (number)}
 * Returns: [success/partial (boolean), count (number)]
 */
static int
lssl_write (lua_State *L)
{
  lssl_context *ctx = checkudata(L, 1, SSL_TYPENAME);
  const int is_udata = lua_isuserdata(L, 2);
  size_t len = is_udata ? lua_tointeger(L, 3) : 0;
  const unsigned char *buf = is_udata ? lua_touserdata(L, 2)
   : lua_tolstring(L, 2, &len);
  int res;

  lua_settop(L, 2);
  lua_getfenv(L, 1);

  ctx->L = L;
  res = ssl_write(&ctx->ssl, buf, len);
  if (res >= 0) {
    lua_pushboolean(L, (res == (int) len));
    lua_pushinteger(L, res);
    return 2;
  }
  return lssl_seterror(L, res);
}

/*
 * Arguments: ssl_udata, alert_message(string), [is_fatal (boolean)]
 * Returns: ssl_udata
 */
static int
lssl_send_alert_message (lua_State *L)
{
  static const char *const alert_names[] = {
    "UNEXPECTED-MESSAGE",
    "BAD-RECORD-MAC",
    "DECRYPTION-FAILED",
    "RECORD-OVERFLOW",
    "DECOMPRESSION-FAILURE",
    "HANDSHAKE-FAILURE",
    "NO-CERT",
    "BAD-CERT",
    "UNSUPPORTED-CERT",
    "CERT-REVOKED",
    "CERT-EXPIRED",
    "CERT-UNKNOWN",
    "ILLEGAL-PARAMETER",
    "UNKNOWN-CA",
    "ACCESS-DENIED",
    "DECODE-ERROR",
    "DECRYPT-ERROR",
    "EXPORT-RESTRICTION",
    "PROTOCOL-VERSION",
    "INSUFFICIENT-SECURITY",
    "INTERNAL-ERROR",
    "USER-CANCELED",
    "NO-RENEGOTIATION",
    "UNSUPPORTED-EXT",
    "UNRECOGNIZED-NAME",
    NULL
  };
  static const int alert_values[] = {
    SSL_ALERT_MSG_UNEXPECTED_MESSAGE,
    SSL_ALERT_MSG_BAD_RECORD_MAC,
    SSL_ALERT_MSG_DECRYPTION_FAILED,
    SSL_ALERT_MSG_RECORD_OVERFLOW,
    SSL_ALERT_MSG_DECOMPRESSION_FAILURE,
    SSL_ALERT_MSG_HANDSHAKE_FAILURE,
    SSL_ALERT_MSG_NO_CERT,
    SSL_ALERT_MSG_BAD_CERT,
    SSL_ALERT_MSG_UNSUPPORTED_CERT,
    SSL_ALERT_MSG_CERT_REVOKED,
    SSL_ALERT_MSG_CERT_EXPIRED,
    SSL_ALERT_MSG_CERT_UNKNOWN,
    SSL_ALERT_MSG_ILLEGAL_PARAMETER,
    SSL_ALERT_MSG_UNKNOWN_CA,
    SSL_ALERT_MSG_ACCESS_DENIED,
    SSL_ALERT_MSG_DECODE_ERROR,
    SSL_ALERT_MSG_DECRYPT_ERROR,
    SSL_ALERT_MSG_EXPORT_RESTRICTION,
    SSL_ALERT_MSG_PROTOCOL_VERSION,
    SSL_ALERT_MSG_INSUFFICIENT_SECURITY,
    SSL_ALERT_MSG_INTERNAL_ERROR,
    SSL_ALERT_MSG_USER_CANCELED,
    SSL_ALERT_MSG_NO_RENEGOTIATION,
    SSL_ALERT_MSG_UNSUPPORTED_EXT,
    SSL_ALERT_MSG_UNRECOGNIZED_NAME
  };

  lssl_context *ctx = checkudata(L, 1, SSL_TYPENAME);
  const int alert_idx = luaL_checkoption(L, 2, NULL, alert_names);
  const unsigned char message = alert_values[alert_idx];
  const unsigned char level = lua_toboolean(L, 3)
   ? SSL_ALERT_LEVEL_FATAL : SSL_ALERT_LEVEL_WARNING;

  lua_settop(L, 1);
  lua_getfenv(L, 1);

  ctx->L = L;
  return lssl_seterror(L,
   ssl_send_alert_message(&ctx->ssl, level, message));
}

/*
 * Arguments: ssl_udata
 * Returns: ssl_udata
 */
static int
lssl_close_notify (lua_State *L)
{
  lssl_context *ctx = checkudata(L, 1, SSL_TYPENAME);

  lua_getfenv(L, 1);

  ctx->L = L;
  return lssl_seterror(L, ssl_close_notify(&ctx->ssl));
}

/*
 * Arguments: ssl_udata
 * Returns: string
 */
static int
lssl_tostring (lua_State *L)
{
  lssl_context *ctx = checkudata(L, 1, SSL_TYPENAME);

  lua_pushfstring(L, SSL_TYPENAME " (%p)", ctx);
  return 1;
}


#define SSL_METHODS \
  {"ssl",			lssl_new}

static luaL_Reg lssl_meth[] = {
  {"init",			lssl_init},
  {"close",			lssl_close},
  {"session_reset",		lssl_session_reset},
  {"bio_begin",			lssl_bio_begin},
  {"bio_end",			lssl_bio_end},
  {"random",			lssl_random},
  {"set_endpoint",		lssl_set_endpoint},
  {"set_authmode",		lssl_set_authmode},
  {"set_rng",			lssl_set_rng},
  {"set_dbg",			lssl_set_dbg},
  {"dbg_level",			lssl_dbg_level},
  {"set_bio",			lssl_set_bio},
  {"set_session_cache",		lssl_set_session_cache},
  {"set_sni",			lssl_set_sni},
  {"set_ciphersuites",		lssl_set_ciphersuites},
  {"set_peer_cn",		lssl_set_peer_cn},
  {"set_ca_cert",		lssl_set_ca_cert},
  {"set_ca_crl",		lssl_set_ca_crl},
  {"set_own_cert",		lssl_set_own_cert},
  {"set_rsa_key",		lssl_set_rsa_key},
  {"set_rsa_keyfile",		lssl_set_rsa_keyfile},
  {"set_rsa_keytest",		lssl_set_rsa_keytest},
  {"set_dh_param",		lssl_set_dh_param},
  {"set_hostname",		lssl_set_hostname},
  {"set_max_version",		lssl_set_max_version},
  {"set_min_version",		lssl_set_min_version},
  {"set_renegotiation",		lssl_set_renegotiation},
  {"legacy_renegotiation",	lssl_legacy_renegotiation},
  {"get_bytes_avail",		lssl_get_bytes_avail},
  {"get_verify_result",		lssl_get_verify_result},
  {"get_ciphersuite",		lssl_get_ciphersuite},
  {"get_version",		lssl_get_version},
  {"get_peer_cert",		lssl_get_peer_cert},
  {"handshake",			lssl_handshake},
  {"handshake_step",		lssl_handshake_step},
  {"renegotiate",		lssl_renegotiate},
  {"read",			lssl_read},
  {"write",			lssl_write},
  {"send_alert_message",	lssl_send_alert_message},
  {"close_notify",		lssl_close_notify},
  {"__tostring",		lssl_tostring},
  {"__gc",			lssl_close},
  {NULL, NULL}
};
