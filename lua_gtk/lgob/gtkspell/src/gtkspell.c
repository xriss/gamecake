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

/* GTK+ */
#include <gtk/gtk.h>
#include <gtkspell/gtkspell.h>

/* C */
#include <string.h>

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

static void special_type_new(lua_State* L, const gchar* mt, gpointer ptr)
{
	if(ptr)
	{
		Object* obj = lua_newuserdata(L, sizeof(Object));
		obj->pointer = ptr;
		obj->need_unref = TRUE;
		lua_getfield(L, LUA_REGISTRYINDEX, mt);
		lua_setmetatable(L, -2);
	}
	else
		lua_pushnil(L);
}

static const struct luaL_reg gtkspell [] =
{
	{NULL, NULL}
};

static void _wrap_gtkspell_init(lua_State* L)
{
	luaL_register(L, "gtkspell", gtkspell);
	
	luaL_loadstring(L, "require('lgob.gtk')"); lua_call(L, 0, 0);
	luaL_loadstring(L, "glib.handle_log('GtkSpell')"); lua_call(L, 0, 0);
}

static void _wrap_gtkspell_ret(lua_State* L)
{
	/* nothing */
}
