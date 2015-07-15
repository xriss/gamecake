/* Lua PolarSSL: X.509 Certificate */

#include "polarssl/certs.h"
#include "polarssl/x509.h"


#define X509_CERT_TYPENAME	"polarssl.x509_cert"


/*
 * Returns: x509_cert_udata
 */
static int
lx509_cert_new (lua_State *L)
{
  x509_cert *crt = lua_newuserdata(L, sizeof(x509_cert));

  memset(crt, 0, sizeof(x509_cert));
  luaL_getmetatable(L, X509_CERT_TYPENAME);
  lua_setmetatable(L, -2);
  return 1;
}

/*
 * Arguments: x509_cert_udata, [dispose (boolean)]
 */
static int
lx509_cert_close (lua_State *L)
{
  x509_cert *crt = checkudata(L, 1, X509_CERT_TYPENAME);

  if (lua_toboolean(L, 2))
    memset(crt, 0, sizeof(x509_cert));
  else
    x509_free(crt);
  return 0;
}

/*
 * Arguments: x509_cert_udata, type (string: "server", "ca", "client")
 * Returns: [x509_cert_udata]
 */
static int
lx509_cert_parse_test (lua_State *L)
{
  x509_cert *crt = checkudata(L, 1, X509_CERT_TYPENAME);
  const char *type = luaL_checkstring(L, 2);
  const char *data = (*type == 's') ? test_srv_crt
   : (type[1] == 'a' ? test_ca_crt : test_cli_crt);

  return lssl_seterror(L, x509parse_crt(crt,
   (const unsigned char *) data, strlen(data)));
}

/*
 * Arguments: x509_cert_udata, nblimbs (number)
 * Returns: [x509_cert_udata]
 */
static int
lx509_cert_parse_data (lua_State *L)
{
  x509_cert *crt = checkudata(L, 1, X509_CERT_TYPENAME);
  size_t len;
  const unsigned char *data = (const unsigned char *) luaL_checklstring(L, 2, &len);

  return lssl_seterror(L, x509parse_crt(crt, data, len));
}

/*
 * Arguments: x509_cert_udata, path (string)
 * Returns: [x509_cert_udata]
 */
static int
lx509_cert_parse_file (lua_State *L)
{
  x509_cert *crt = checkudata(L, 1, X509_CERT_TYPENAME);
  const char *path = luaL_checkstring(L, 2);

  return lssl_seterror(L, x509parse_crtfile(crt, path));
}

/*
 * Arguments: x509_cert_udata, [prefix (string)]
 * Returns: [string]
 */
static int
lx509_cert_info (lua_State *L)
{
  x509_cert *crt = checkudata(L, 1, X509_CERT_TYPENAME);
  const char *prefix = luaL_optstring(L, 2, "");
  char buf[4096];
  const int len = x509parse_cert_info(buf, sizeof(buf), prefix, crt);

  lua_pushlstring(L, buf, (len != -1) ? len : 0);
  return 1;
}

/*
 * Arguments: x509_cert_udata
 * Returns: string
 */
static int
lx509_cert_tostring (lua_State *L)
{
  x509_cert *crt = checkudata(L, 1, X509_CERT_TYPENAME);

  lua_pushfstring(L, X509_CERT_TYPENAME " (%p)", crt);
  return 1;
}


#define X509_CERT_METHODS \
  {"x509_cert",	lx509_cert_new}

static luaL_Reg lx509_cert_meth[] = {
  {"close",		lx509_cert_close},
  {"parse_test",	lx509_cert_parse_test},
  {"parse_data",	lx509_cert_parse_data},
  {"parse_file",	lx509_cert_parse_file},
  {"info",		lx509_cert_info},
  {"__tostring",	lx509_cert_tostring},
  {"__gc",		lx509_cert_close},
  {NULL, NULL}
};
