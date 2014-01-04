#define LUA_CORE
#include "lua.h"
#include "lauxlib.h"
#include "compat.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <stdlib.h>
#endif

LUA_API const wchar_t *lua_tolwstring (lua_State *L, int index, size_t *length) {
  switch (lua_type(L, index)) {
  case LUA_TSTRING: {
    const char *utf8;
    size_t size8, size16;
    wchar_t *utf16;
    utf8 = lua_tolstring(L, index, &size8);
#ifdef _WIN32
    size16 = MultiByteToWideChar(CP_UTF8, 0, utf8, size8, 0, 0);
#else
    size16 = mbstowcs(NULL, utf8, size8);
#endif
    if (size16 > INT_MAX)
      size16 = INT_MAX;
    utf16 = (wchar_t*)lua_newuserdata(L, (size16+1) * sizeof(wchar_t));
#ifdef _WIN32
    MultiByteToWideChar(CP_UTF8, 0, utf8, size8, utf16, (int)size16);
#else
    size16 = mbstowcs(utf16, utf8, size8);
#endif
    utf16[size16] = 0;
    lua_replace(L, index);
    if (length)
      *length = size16;  /* remove trailing 0 */
    return utf16;
  }
  case LUA_TUSERDATA:
    if (length)
      *length = lua_objlen(L, index) / sizeof(wchar_t) - 1;  /* remove trailing 0 */
    return (const wchar_t*)lua_touserdata(L, index);
  default:
    return 0;
  }
}

LUA_API const wchar_t *lua_towstring (lua_State *L, int index) {
  return lua_tolwstring(L, index, 0);
}

LUA_API const wchar_t *luaL_checkwstring (lua_State *L, int index) {
  if (lua_type(L, index)!=LUA_TUSERDATA)
    luaL_checkstring(L, index);
  return lua_towstring(L, index);
}

LUA_API const wchar_t *luaL_optwstring (lua_State *L, int narg, const wchar_t *def) {
  if (lua_isnoneornil(L, narg))
    return def;
  else
    return luaL_checkwstring(L, narg);
}

LUA_API const wchar_t *luaL_checklwstring (lua_State *L, int index, size_t *length) {
  luaL_checkstring(L, index);
  return lua_tolwstring(L, index, length);
}

LUA_API void lua_pushwstring (lua_State *L, const wchar_t *value) {
  if (value == NULL)
    lua_pushnil(L);
  else
  {
#ifdef _WIN32
    size_t size8;
    char *utf8;
    size8 = WideCharToMultiByte(CP_UTF8, 0, value, -1, 0, 0, 0, 0);
    if (size8 > INT_MAX)
      size8 = INT_MAX;
    utf8 = (char*)lua_newuserdata(L, size8);
    WideCharToMultiByte(CP_UTF8, 0, value, -1, utf8, (int)size8, 0, 0);
    lua_pushlstring(L, utf8, size8 - 1);  /* remove the terminating NULL, Lua adds another one */
    lua_remove(L, -2);  /* remove the utf-8 userdata */
#else
    size_t size8;
    size8 = wcstombs(NULL, value, 0);
    if (size8==(size_t)-1)
      luaL_error(L, "could not convert wstring to string");
    if (size8 > INT_MAX)
      size8 = INT_MAX;
    else {
      char *utf8;
      utf8 = (char*)lua_newuserdata(L, size8);
      size8 = wcstombs(utf8, value, size8);
      lua_pushlstring(L, utf8, size8 - 1);  /* remove the terminating NULL, Lua adds another one */
      lua_remove(L, -2);  /* remove the utf-8 userdata */
    }
#endif
  }
}

LUA_API void lua_pushlwstring (lua_State *L, const wchar_t *value, size_t isize) {
  size_t osize;
  char *utf8;
#ifdef _WIN32
  if (isize > INT_MAX)
    isize = INT_MAX;
  osize = WideCharToMultiByte(CP_UTF8, 0, value, (int)isize, 0, 0, 0, 0);
  if (osize > INT_MAX)
    osize = INT_MAX;
  utf8 = (char*)lua_newuserdata(L, (int)osize);
  WideCharToMultiByte(CP_UTF8, 0, value, (int)isize, utf8, (int)osize, 0, 0);
#else
  size_t i;
  /* wcstombs is not capable of converting embedded zeros, so we do it
   * manually */
  osize = 0;
  for (i=0; i<isize; ++i)
    osize += wctomb(NULL, value[i]);
  utf8 = (char*)lua_newuserdata(L, (int)osize);
  for (i=0; i<isize; ++i)
    utf8 += wctomb(utf8, value[i]);
#endif
  lua_pushlstring(L, utf8, osize);
  lua_remove(L, -2);  /* remove the utf-8 userdata */
}

LUA_API int luaL_loadwfile (lua_State *L, const wchar_t *filename) {
  int result;
  int top;
  top = lua_gettop(L);
  lua_pushwstring(L, filename);
  result = luaL_loadfile(L, lua_tostring(L, -1));
  lua_remove(L, top+1);
  return result;
}

/* vi: ts=2 sts=2 sw=2 et
*/
