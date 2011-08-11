/** \file
 * \brief Context Plus Lua 5 Binding
 *
 * See Copyright Notice in cd.h
 */

#include <stdlib.h>
#include <stdio.h>

#include "cd.h"

#include <lua.h>
#include <lauxlib.h>


static int cdlua5_initcontextplus(lua_State *L)
{
  (void)L;
  cdInitContextPlus();
  return 0;
}

static const struct luaL_reg cdlib[] = {
  {"InitContextPlus", cdlua5_initcontextplus},
  {NULL, NULL},
};


static int cdluacontextplus_open (lua_State *L)
{
  cdInitContextPlus();
  luaL_register(L, "cd", cdlib);   /* leave "cd" table at the top of the stack */
  return 1;
}

int luaopen_cdluacontextplus(lua_State* L)
{
  return cdluacontextplus_open(L);
}
