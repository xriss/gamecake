/* Lua PolarSSL */

#include "common.h"

#include "polarssl/config.h"
#include "polarssl/error.h"
#include "polarssl/version.h"
#include "polarssl/havege.h"

#ifdef POLARSSL_AES_C
#include "laes.c"
#endif
#ifdef POLARSSL_BASE64_C
#include "lbase64.c"
#endif
#ifdef POLARSSL_CAMELLIA_C
#include "lcamellia.c"
#endif
#ifdef POLARSSL_DES_C
#include "ldes.c"
#endif
#include "lhash.c"
#ifdef POLARSSL_PADLOCK_C
#include "lpadlock.c"
#endif
#include "lsession.c"
#ifdef POLARSSL_TIMING_C
#include "ltiming.c"
#endif
#include "lx509_cert.c"
#include "lx509_crl.c"
#include "lssl.c"
#include "lmpi.c"


/*
 * Returns: self | nil, string
 */
static int
lssl_seterror (lua_State *L, int err)
{
  if (!err) {
    lua_settop(L, 1);
    return 1;
  }
  lua_pushnil(L);
  {
    char buf[256];
    error_strerror(err, buf, sizeof(buf));
    lua_pushstring(L, buf);
  }
  lua_pushvalue(L, -1);
  lua_setglobal(L, LSSL_ERROR_MESSAGE);
  return 2;
}

/*
 * Returns: number (MMNNPP00), string ("x.y.z"), sting ("PolarSSL x.y.z")
 */
static int
lpssl_get_version (lua_State *L)
{
  lua_pushinteger(L, POLARSSL_VERSION_NUMBER);
  lua_pushliteral(L, POLARSSL_VERSION_STRING);
  lua_pushliteral(L, POLARSSL_VERSION_STRING_FULL);
  return 3;
}


static luaL_Reg polarssl_lib[] = {
#ifdef POLARSSL_BASE64_C
  BASE64_METHODS,
#endif
  HASH_METHODS,
  MPI_METHODS,
  SESSION_METHODS,
  SSL_METHODS,
#ifdef POLARSSL_TIMING_C
  TIMING_METHODS,
#endif
  X509_CERT_METHODS,
  X509_CRL_METHODS,
  {"get_version", 	lpssl_get_version},
  {NULL, NULL}
};

static void
createmeta (lua_State *L)
{
  struct meta_s {
    const char *tname;
    luaL_Reg *meth;
  } meta[] = {
    {HASH_TYPENAME,		lhash_meth},
    {MPI_TYPENAME,		lmpi_meth},
    {SESSION_TYPENAME,		lsession_meth},
    {SSL_TYPENAME,		lssl_meth},
    {X509_CERT_TYPENAME,	lx509_cert_meth},
    {X509_CRL_TYPENAME,		lx509_crl_meth},
  };
  int i;

  for (i = 0; i < (int) (sizeof(meta) / sizeof(struct meta_s)); ++i) {
    luaL_newmetatable(L, meta[i].tname);
    lua_pushvalue(L, -1);  /* push metatable */
    lua_setfield(L, -2, "__index");  /* metatable.__index = metatable */
    luaL_setfuncs(L, meta[i].meth, 0);
    lua_pop(L, 1);
  }
}


LUALIB_API int
luaopen_polarssl (lua_State *L)
{
  luaL_register(L, LUA_POLARSSLLIBNAME, polarssl_lib);
  createmeta(L);
  return 1;
}
