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

/* Lua */
#include <lua.h>
#include <lauxlib.h>

/* Poppler */
#include <poppler.h>

/* C */
#include <string.h>

/* lgob */
#include <lgob/common/types.h>
#include <lgob/gobject/types.h>

#define _LIB_FREE g_free

static void register_class(lua_State* L, const char* name, const char* base, const luaL_Reg* reg)
{
	lua_pushstring(L, name);
	lua_newtable(L);
	luaL_register(L, NULL, reg);

	if(base)
	{
		lua_newtable(L);
		lua_pushliteral(L, "__index");
		lua_pushstring(L, base);
		lua_rawget(L, -6);
		lua_rawset(L, -3);
		lua_setmetatable(L, -2);
	}
	
	lua_rawset(L, -3); 
}

static void object_new(lua_State* L, gpointer ptr, gboolean constructor)
{
	lua_pushliteral(L, "lgobObjectNew");
	lua_rawget(L, LUA_REGISTRYINDEX);
	lua_pushlightuserdata(L, ptr);
	lua_pushboolean(L, constructor); 
	lua_call(L, 2, 1);
}

static const struct luaL_reg poppler [] =
{
	{NULL, NULL}
};

static const struct luaL_reg _global[];

static void _wrap_poppler_init(lua_State* L)
{	
	luaL_register(L, "poppler", poppler);
	luaL_register(L, NULL, _global);
	
	luaL_loadstring(L, "require('lgob.gobject')"); lua_call(L, 0, 0);
	luaL_loadstring(L, "poppler.Object = gobject.Object"); lua_call(L, 0, 0);
	
	lua_getfield(L, LUA_REGISTRYINDEX, "lgobPrefix");
	lua_pushliteral(L, "Poppler"); lua_pushliteral(L, "poppler"); lua_rawset(L, -3);
	lua_pop(L, 1);
	
	luaL_loadstring(L, "glib.handle_log('Poppler')"); lua_call(L, 0, 0);
}

static void _wrap_poppler_ret(lua_State* L)
{
	/* nothing */
}
