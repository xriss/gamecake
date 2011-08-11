/*
	This file is part of lgob.

	lgob is free software: you can redistribute it and/or modify
	it under the terms of the GNU Lesser General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	lgob is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public License
	along with lgob.  If not, see <http://www.gnu.org/licenses/>.
    
    Copyright (C) 2009 - 2010 Lucas Hermann Negri
*/

#include <lua.h>
#include <lauxlib.h>

#ifndef _WIN32
#include <dlfcn.h>

/* libraries loaded with this loader aren't unloaded at the end of the execution
 * as I don't think that theres need for it */

static int lgob_loadlib(lua_State* L)
{
	const char* path     = luaL_checkstring(L, 1);
	const char* funcname = luaL_checkstring(L, 2);
    
	void* lib = dlopen(path, RTLD_NOW | RTLD_GLOBAL);
	if(!lib) luaL_error(L, "%s", dlerror());
	
	lua_CFunction f = dlsym(lib, funcname);
	if(!f)
	{
		dlclose(lib);
		luaL_error(L, "Couldn't find the init function %s", funcname);
	}
	
	lua_settop(L, 0);
	lua_pushcfunction(L, f);
    
    return 1;
}

int luaopen_lgob__loader(lua_State *L)
{
	lua_pushcfunction(L, lgob_loadlib);
	return 1;
}

#else

int luaopen_lgob__loader(lua_State *L)
{
	lua_pushboolean(L, 0);
	return 1;
}

#endif
