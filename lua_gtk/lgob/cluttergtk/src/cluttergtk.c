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

/* clutter */
#include <clutter-gtk/clutter-gtk.h>

/* C */
#include <string.h>

#include <lgob/common/types.h>
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

/*
static void boxed_new(lua_State* L, GType type, gpointer ptr)
{
	lua_pushliteral(L, "lgobBoxedNew");
	lua_rawget(L, LUA_REGISTRYINDEX);
	lua_pushinteger(L, type);
	lua_pushlightuserdata(L, ptr);
	lua_call(L, 2, 1);
}


static void struct_new(lua_State* L, gpointer ptr, gboolean need_unref)
{
	lua_pushliteral(L, "lgobStructNew");
	lua_rawget(L, LUA_REGISTRYINDEX);
	lua_pushlightuserdata(L, ptr);
	lua_pushboolean(L, need_unref);
	lua_call(L, 2, 1);
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
}*/

static void copy_interface(lua_State* L, const char* iface, const char* target, const luaL_Reg* fnames)
{
	lua_getfield(L, -1, iface);
	lua_getfield(L, -2, target);
	
	for( ; fnames->name; ++fnames)
	{
		lua_getfield(L, -2, fnames->name);
		lua_setfield(L, -2, fnames->name);
	}
	
	lua_pop(L, 2);
}

static const struct luaL_reg cluttergtk [] =
{
	{NULL, NULL}
};

static const struct luaL_reg _global[];

static void _wrap_cluttergtk_init(lua_State* L)
{
	luaL_register(L, "cluttergtk", cluttergtk);
	luaL_register(L, NULL, _global);
	
	luaL_loadstring(L, "require('lgob.gtk')"); lua_call(L, 0, 0);
	luaL_loadstring(L, "require('lgob.clutter')"); lua_call(L, 0, 0);
	luaL_loadstring(L, "cluttergtk.Widget = gtk.Widget"); lua_call(L, 0, 0);
	luaL_loadstring(L, "cluttergtk.Container = gtk.Container"); lua_call(L, 0, 0);
	luaL_loadstring(L, "glib.handle_log('GtkClutter')"); lua_call(L, 0, 0);
}


static void _wrap_cluttergtk_ret(lua_State* L)
{
	/* Fix some names */ 
	lua_getfield(L, LUA_REGISTRYINDEX, "lgobSpecial");
	lua_pushliteral(L, "GtkClutterEmbed"); lua_getfield(L, -3, "Embed"); lua_rawset(L, -3);
	lua_pop(L, 1);
}

