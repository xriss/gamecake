/** \file
 * \brief wmv format Lua 5 Binding
 *
 * See Copyright Notice in cd.h
 */

#include <stdlib.h>
#include <stdio.h>

#include "im_format_wmv.h"

#include <lua.h>
#include <lauxlib.h>


static int imlua_FormatRegisterWMV(lua_State *L)
{
  (void)L;
  imFormatRegisterWMV();
  return 0;
}

static const struct luaL_reg imlib[] = {
  {"FormatRegisterWMV", imlua_FormatRegisterWMV},
  {NULL, NULL},
};


static int imlua_wmv_open (lua_State *L)
{
  imFormatRegisterWMV();
  luaL_register(L, "im", imlib);   /* leave "im" table at the top of the stack */
  return 1;
}

int luaopen_imlua_wmv(lua_State* L)
{
  return imlua_wmv_open(L);
}
