/* Lua PolarSSL: X.509 CRL */

#include "polarssl/x509.h"


#define X509_CRL_TYPENAME	"polarssl.x509_crl"


/*
 * Returns: x509_crl_udata
 */
static int
lx509_crl_new (lua_State *L)
{
  x509_crl *crl = lua_newuserdata(L, sizeof(x509_crl));

  memset(crl, 0, sizeof(x509_crl));
  luaL_getmetatable(L, X509_CRL_TYPENAME);
  lua_setmetatable(L, -2);
  return 1;
}

/*
 * Arguments: x509_cert_udata
 */
static int
lx509_crl_close (lua_State *L)
{
  x509_crl *crl = checkudata(L, 1, X509_CRL_TYPENAME);

  x509_crl_free(crl);
  return 0;
}

/*
 * Arguments: x509_crl_udata, nblimbs (number)
 * Returns: [x509_crl_udata]
 */
static int
lx509_crl_parse_data (lua_State *L)
{
  x509_crl *crl = checkudata(L, 1, X509_CRL_TYPENAME);
  size_t len;
  const unsigned char *s = (const unsigned char *) luaL_checklstring(L, 2, &len);

  return lssl_seterror(L, x509parse_crl(crl, s, len));
}

/*
 * Arguments: x509_crl_udata, path (string)
 * Returns: [x509_crl_udata]
 */
static int
lx509_crl_parse_file (lua_State *L)
{
  x509_crl *crl = checkudata(L, 1, X509_CRL_TYPENAME);
  const char *path = luaL_checkstring(L, 2);

  return lssl_seterror(L, x509parse_crlfile(crl, path));
}

/*
 * Arguments: x509_crl_udata, [prefix (string)]
 * Returns: [string]
 */
static int
lx509_crl_info (lua_State *L)
{
  x509_crl *crl = checkudata(L, 1, X509_CRL_TYPENAME);
  const char *prefix = luaL_optstring(L, 2, "");
  char buf[4096];
  const int len = x509parse_crl_info(buf, sizeof(buf), prefix, crl);

  lua_pushlstring(L, buf, (len != -1) ? len : 0);
  return 1;
}

/*
 * Arguments: x509_crl_udata
 * Returns: string
 */
static int
lx509_crl_tostring (lua_State *L)
{
  x509_crl *crl = checkudata(L, 1, X509_CRL_TYPENAME);

  lua_pushfstring(L, X509_CRL_TYPENAME " (%p)", crl);
  return 1;
}


#define X509_CRL_METHODS \
  {"x509_crl",		lx509_crl_new}

static luaL_Reg lx509_crl_meth[] = {
  {"close",		lx509_crl_close},
  {"parse_data",	lx509_crl_parse_data},
  {"parse_file",	lx509_crl_parse_file},
  {"info",		lx509_crl_info},
  {"__tostring",	lx509_crl_tostring},
  {"__gc",		lx509_crl_close},
  {NULL, NULL}
};
