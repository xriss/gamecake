/* Lua PolarSSL: RFC 1521 base64 encoding/decoding */

#include "polarssl/base64.h"


typedef int (*f_base64_t) (unsigned char *dst, size_t *dlen,
                           const unsigned char *src, size_t slen);


/*
 * Arguments: src_ludata, src_length (number), [dst_ludata, dst_length (number)]
 * Returns: [number]
 */
static int
lbase64_oper (lua_State *L, f_base64_t func)
{
  const unsigned char *src = lua_touserdata(L, 1);
  const size_t slen = lua_tointeger(L, 2);
  unsigned char *dst = lua_touserdata(L, 3);
  size_t dlen = lua_tointeger(L, 4);
  int res;

  res = func(dst, &dlen, src, slen);
  if (!res) {
    lua_pushinteger(L, dlen);
    return 1;
  }
  return lssl_seterror(L, res);
}

static int
lbase64_encode (lua_State *L)
{
  return lbase64_oper(L, base64_encode);
}

static int
lbase64_decode (lua_State *L)
{
  return lbase64_oper(L, base64_decode);
}


#define BASE64_METHODS \
  {"base64_encode",	lbase64_encode}, \
  {"base64_decode",	lbase64_decode}
