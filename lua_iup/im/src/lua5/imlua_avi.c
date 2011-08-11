/** \file
 * \brief AVI format Lua 5 Binding
 *
 * See Copyright Notice in cd.h
 */

#include <stdlib.h>
#include <stdio.h>

#include "im_format_avi.h"

#include <lua.h>
#include <lauxlib.h>


static int imlua_FormatRegisterAVI(lua_State *L)
{
  (void)L;
  imFormatRegisterAVI();
  return 0;
}

static const struct luaL_reg imlib[] = {
  {"FormatRegisterAVI", imlua_FormatRegisterAVI},
  {NULL, NULL},
};


static int imlua_avi_open (lua_State *L)
{
  imFormatRegisterAVI();
  luaL_register(L, "im", imlib);   /* leave "im" table at the top of the stack */
  return 1;
}

int luaopen_imlua_avi(lua_State* L)
{
  return imlua_avi_open(L);
}
