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

/* Model of the main library file */

/* Lua */
#include <lua.h>
#include <lauxlib.h>

/* GTK+ */
#include <gtk/gtk.h>
#include <lobj/types.h>

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
	lua_pushliteral(L, "lobjObjectNew");
	lua_rawget(L, LUA_REGISTRYINDEX);
	lua_pushlightuserdata(L, ptr);
	lua_pushboolean(L, constructor);
	lua_call(L, 2, 1);
}

static void struct_new(lua_State* L, gpointer ptr, gboolean need_unref)
{
	lua_pushliteral(L, "lobjStructNew");
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
		obj->need_unref = TRUE; /* not used - only for consistency */
		lua_getfield(L, LUA_REGISTRYINDEX, mt);
		lua_setmetatable(L, -2);
	}
	else
		lua_pushnil(L);
}

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

static const struct luaL_reg small [] =
{
	{NULL, NULL}
};

static const struct luaL_reg _global[];

static void priv_init_gtk(lua_State* L)
{
	int argc = 1;
	char** argv = g_malloc(argc * sizeof(char*));

	lua_pushliteral(L, "arg");
	lua_rawget(L, LUA_GLOBALSINDEX);

	if(!lua_istable(L, -1))
		argv[0] = "small";
	else
	{
		lua_pushnumber(L, 0);
		lua_rawget(L, -2);
		argv[0] = (char*)luaL_optstring(L, -1, "small");
	}

	gtk_init(&argc, &argv);
	g_free(argv);
}

static void _wrap_small_init(lua_State* L)
{
	priv_init_gtk(L);
	
	luaL_register(L, "small", small);
	luaL_register(L, NULL, _global);
	
	(void)luaL_dostring(L, "require('lobj')");
	(void)luaL_dostring(L, "small.Object = lobj.Object");
	
	lua_getfield(L, LUA_REGISTRYINDEX, "lobjPrefix");
	lua_pushliteral(L, "Gtk"); lua_pushliteral(L, "small"); lua_rawset(L, -3);
	lua_pushliteral(L, "Gdk"); lua_pushliteral(L, "small"); lua_rawset(L, -3);
	lua_pop(L, 1);
}

static void _wrap_small_ret(lua_State* L)
{
}
