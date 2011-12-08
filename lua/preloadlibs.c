
#include "lua.h"
extern void wetgenes_cache_preloader(lua_State *L);

extern void lua_preloadlibs(lua_State *L)
{
    lua_getglobal(L, "package");
    lua_getfield(L, -1, "preload");
    lua_pop(L, 2);
	wetgenes_cache_preloader(L); // include embeded strings loader
}

