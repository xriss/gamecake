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

/* Lua headers */
#include <lua.h>
#include <lauxlib.h>

/* C headers */
#include <stdlib.h>
#include <string.h>

#define TRUE 1
#define FALSE 0

/* lgob */
#include <lgob/common/types.h>

/* Cairo */
#include <cairo.h>
#include <cairo-pdf.h>
#include <cairo-svg.h>
#include <cairo-ps.h>

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


/**
 * Creates a double array with the contens of a table.
 * The caller is the owner of the array.
 * 
 * @param L lua_State to use
 * @param tpos Position of the table in the stack
 * @param size Size of the table
 * @return Newly allocated double array
 */ 
static double* tableToDoubleArray(lua_State* L, int tpos, int size)
{
	int i;
	double* array = malloc(sizeof(double) * size);
	
	for(i = 1; i <= size; ++i)
	{
		lua_rawgeti(L, tpos, i);
		array[i - 1] = lua_tonumber(L, -1);
	}
	
	return array;	
}

static int _wrap_cairo_version(lua_State* L)
{	
	lua_pushstring(L, cairo_version_string());
	lua_pushstring(L, CAIRO_VERSION_STRING);
	
	return 2;
}

static void priv_struct_new(lua_State* L, void* ptr, cairo_bool_t need_unref, const char* mtname)
{
	if(ptr)
	{
		static size_t tam = sizeof(Object);
		Object* context = lua_newuserdata(L, tam);
		context->pointer = ptr;
		context->need_unref = need_unref;
		luaL_getmetatable(L, mtname);
		lua_setmetatable(L, -2);
	}
}

static int priv_context_push(lua_State* L)
{
	priv_struct_new(L, lua_touserdata(L, 1), lua_toboolean(L, 2), "cairoContextMT");
	return 1;
}

static int priv_matrix_push(lua_State* L)
{
	priv_struct_new(L, lua_touserdata(L, 1), lua_toboolean(L, 2), "cairoMatrixMT");
	return 1;
}

static const struct luaL_reg cairo [] =
{
	{"version", _wrap_cairo_version},
	{NULL, NULL}
};

static void _wrap_cairo_init(lua_State* L)
{
	luaL_register(L, "cairo", cairo);
	luaL_loadstring(L, "require('lgob.common')"); lua_call(L, 0, 0);
	
	/* Create contexts from other modules */
	lua_pushcfunction(L, priv_context_push);
	lua_setfield(L, LUA_REGISTRYINDEX, "cairoContextPush");
    
    /* Create matrix from other modules */
	lua_pushcfunction(L, priv_matrix_push);
	lua_setfield(L, LUA_REGISTRYINDEX, "cairoMatrixPush");
}

static void _wrap_cairo_ret(lua_State* L)
{
}
