#ifndef COMPAT_H
#define COMPAT_H

#include "lua.h"
#include "lauxlib.h"

#if LUA_VERSION_NUM==501
void luaL_setfuncs_socket (lua_State *L, const luaL_Reg *l, int nup);
#define luaL_setfuncs luaL_setfuncs_socket
#endif

#endif
