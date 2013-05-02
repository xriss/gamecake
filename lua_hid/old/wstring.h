#ifndef WSTRING_H
#define WSTRING_H

#include <lua.h>
#include <lauxlib.h>


#if !LUA_HAS_WSTRING || FORCE_OWN_WSTRING

const wchar_t *(lwin32__lua_towstring) (lua_State *L, int index);
const wchar_t *(lwin32__lua_tolwstring) (lua_State *L, int index, size_t *length);
void (lwin32__lua_pushwstring) (lua_State *L, const wchar_t *value);
void (lwin32__lua_pushlwstring) (lua_State *L, const wchar_t *value, size_t size);

const wchar_t *(lwin32__luaL_optwstring) (lua_State *L, int index, const wchar_t *def);
const wchar_t *(lwin32__luaL_checkwstring) (lua_State *L, int index);
const wchar_t *(lwin32__luaL_checklwstring) (lua_State *L, int index, size_t *length);
int	(lwin32__luaL_loadwfile) (lua_State *L, const wchar_t *filename);

# if FORCE_OWN_WSTRING
#  undef lua_towstring
#  undef lua_tolwstring
#  undef lua_pushwstring
#  undef lua_pushlwstring

#  undef luaL_optwstring
#  undef luaL_checkwstring
#  undef luaL_checklwstring
#  undef luaL_loadwfile

#  undef luaL_dowfile
# endif

#define lua_towstring lwin32__lua_towstring
#define lua_tolwstring lwin32__lua_tolwstring
#define lua_pushwstring lwin32__lua_pushwstring
#define lua_pushlwstring lwin32__lua_pushlwstring

#define luaL_optwstring lwin32__luaL_optwstring
#define luaL_checkwstring lwin32__luaL_checkwstring
#define luaL_checklwstring lwin32__luaL_checklwstring
#define luaL_loadwfile lwin32__luaL_loadwfile

#define luaL_dowfile(L, fn)	\
	(lwin32__luaL_loadwfile(L, fn) || lua_pcall(L, 0, LUA_MULTRET, 0))

#endif


#endif
