
#include "lua.h"

extern void lua_preloadlibs(lua_State *L)
{
    lua_getglobal(L, "package");
    lua_getfield(L, -1, "preload");
    lua_pop(L, 2);
}

