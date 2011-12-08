
#include "lua.h"
extern void wetgenes_cache_preloader(lua_State *L);
extern int luaopen_zlib(lua_State *L);
extern int luaopen_freetype(lua_State *L);
extern int luaopen_bit(lua_State *L);
extern int luaopen_luagl(lua_State *L);

extern void lua_preloadlibs(lua_State *L)
{
    lua_getglobal(L, "package");
    lua_getfield(L, -1, "preload");
    lua_pushliteral(L, "zlib");
    lua_pushcfunction(L, luaopen_zlib);
    lua_settable(L, -3);
    lua_pushliteral(L, "freetype");
    lua_pushcfunction(L, luaopen_freetype);
    lua_settable(L, -3);
    lua_pushliteral(L, "bit");
    lua_pushcfunction(L, luaopen_bit);
    lua_settable(L, -3);
    lua_pushliteral(L, "gl");
    lua_pushcfunction(L, luaopen_luagl);
    lua_settable(L, -3);
    lua_pop(L, 2);
	wetgenes_cache_preloader(L); // include embeded strings loader
}

