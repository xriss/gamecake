
#include "lua.h"
extern int luaopen_zip(lua_State *L);
extern int luaopen_zlib(lua_State *L);
extern int luaopen_freetype(lua_State *L);
extern int luaopen_bit(lua_State *L);
extern int luaopen_box2d_core(lua_State *L);
extern int luaopen_luagl(lua_State *L);
extern int luaopen_grd(lua_State *L);
extern int luaopen_lash(lua_State *L);
extern int luaopen_lfs(lua_State *L);
extern int luaopen_socket_core(lua_State *L);
extern int luaopen_mime_core(lua_State *L);
extern int luaopen_fenestra_core(lua_State *L);

extern void lua_preloadlibs(lua_State *L)
{
    lua_getglobal(L, "package");
    lua_getfield(L, -1, "preload");
    lua_pushliteral(L, "zip");
    lua_pushcfunction(L, luaopen_zip);
    lua_settable(L, -3);
    lua_pushliteral(L, "zlib");
    lua_pushcfunction(L, luaopen_zlib);
    lua_settable(L, -3);
    lua_pushliteral(L, "freetype");
    lua_pushcfunction(L, luaopen_freetype);
    lua_settable(L, -3);
    lua_pushliteral(L, "bit");
    lua_pushcfunction(L, luaopen_bit);
    lua_settable(L, -3);
    lua_pushliteral(L, "box2d.core");
    lua_pushcfunction(L, luaopen_box2d_core);
    lua_settable(L, -3);
    lua_pushliteral(L, "gl");
    lua_pushcfunction(L, luaopen_luagl);
    lua_settable(L, -3);
    lua_pushliteral(L, "grd");
    lua_pushcfunction(L, luaopen_grd);
    lua_settable(L, -3);
    lua_pushliteral(L, "lash");
    lua_pushcfunction(L, luaopen_lash);
    lua_settable(L, -3);
    lua_pushliteral(L, "lfs");
    lua_pushcfunction(L, luaopen_lfs);
    lua_settable(L, -3);
    lua_pushliteral(L, "socket.core");
    lua_pushcfunction(L, luaopen_socket_core);
    lua_settable(L, -3);
    lua_pushliteral(L, "mime.core");
    lua_pushcfunction(L, luaopen_mime_core);
    lua_settable(L, -3);
    lua_pushliteral(L, "fenestra.core");
    lua_pushcfunction(L, luaopen_fenestra_core);
    lua_settable(L, -3);
    lua_pop(L, 2);
}

