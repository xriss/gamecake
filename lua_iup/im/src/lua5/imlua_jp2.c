/** \file
 * \brief jp2 format Lua 5 Binding
 *
 * See Copyright Notice in cd.h
 */

#include <stdlib.h>
#include <stdio.h>

#include "im_format_jp2.h"

#include <lua.h>
#include <lauxlib.h>


static int imlua_FormatRegisterJP2(lua_State *L)
{
  (void)L;
  imFormatRegisterJP2();
  return 0;
}

static const struct luaL_reg imlib[] = {
  {"FormatRegisterJP2", imlua_FormatRegisterJP2},
  {NULL, NULL},
};


static int imlua_jp2_open (lua_State *L)
{
  imFormatRegisterJP2();
  luaL_register(L, "im", imlib);   /* leave "im" table at the top of the stack */
  return 1;
}

int luaopen_imlua_jp2(lua_State* L)
{
  return imlua_jp2_open(L);
}
