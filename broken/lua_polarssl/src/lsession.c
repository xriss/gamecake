/* Lua PolarSSL: Session */

#include "polarssl/ssl.h"


#define SESSION_TYPENAME	"polarssl.session"


/*
 * Returns: session_udata
 */
static int
lsession_new (lua_State *L)
{
  ssl_session *ssn = lua_newuserdata(L, sizeof(ssl_session));

  memset(ssn, 0, sizeof(ssl_session));
  luaL_getmetatable(L, SESSION_TYPENAME);
  lua_setmetatable(L, -2);
  return 1;
}

/*
 * Arguments: session_udata
 */
static int
lsession_close (lua_State *L)
{
  ssl_session *ssn = checkudata(L, 1, SESSION_TYPENAME);

  memset(ssn, 0, sizeof(ssl_session));
  return 0;
}

/*
 * Returns: session_id (string)
 */
static void
lsession_pushid (lua_State *L, const ssl_session *ssn)
{
  const int len = ssn->length;
  unsigned char buf[2 + sizeof(ssn->id)];

  buf[0] = (unsigned char) ssn->ciphersuite;
  buf[1] = (unsigned char) ssn->compression;
  memcpy(&buf[2], ssn->id, len);
  lua_pushlstring(L, (char *) buf, 2 + len);
}

/*
 * Arguments: session_udata
 * Returns: session_id (string)
 */
static int
lsession_id (lua_State *L)
{
  ssl_session *ssn = checkudata(L, 1, SESSION_TYPENAME);

  lsession_pushid(L, ssn);
  return 1;
}

/*
 * Arguments: session_udata
 * Returns: string
 */
static int
lsession_tostring (lua_State *L)
{
  ssl_session *ssn = checkudata(L, 1, SESSION_TYPENAME);

  lua_pushfstring(L, SESSION_TYPENAME " (%p)", ssn);
  return 1;
}


#define SESSION_METHODS \
  {"session",		lsession_new}

static luaL_Reg lsession_meth[] = {
  {"close",		lsession_close},
  {"id",		lsession_id},
  {"__tostring",	lsession_tostring},
  {"__gc",		lsession_close},
  {NULL, NULL}
};
