/*
 LuaZip - Reading files inside zip files.
 http://www.keplerproject.org/luazip/

 Author: Danilo Tuler
 Copyright (c) 2003-2007 Kepler Project

 $Id: luazip.h,v 1.5 2007-06-18 18:47:05 carregal Exp $
*/

#ifndef luazip_h
#define luazip_h

#include "lua.h"

#ifndef LUAZIP_API
#define LUAZIP_API	LUA_API
#endif

#define LUA_ZIPLIBNAME	"zip"
LUAZIP_API int luaopen_zip (lua_State *L);

#endif
