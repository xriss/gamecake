
#include "lua.h"
extern int luaopen_bit(lua_State *L);

extern void lua_preloadlibs(lua_State *L)
{
    lua_getglobal(L, "package");
    lua_getfield(L, -1, "preload");
    lua_pushliteral(L, "bit");
    lua_pushcfunction(L, luaopen_bit);
    lua_settable(L, -3);
    lua_pop(L, 2);
}

