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

/* C */
#include <string.h>

/* lgob */
#include <lgob/common/types.h>

/* glib */
#include <glib.h>

/* GObject */
#include <glib-object.h>
#include <gobject/gvaluecollector.h>

#include "types.h"

#define _LIB_FREE g_free
#define luaopen_lgob_glib luaopen_lgob_gobject

/* signatures */
static gboolean priv_object_new(lua_State* L, GObject* pointer, gboolean constructor);
static gboolean priv_generic_struct_new(lua_State* L, gpointer pointer, gboolean need_unref);
static gboolean priv_boxed_new(lua_State* L, GType type, gpointer pointer);
static gboolean priv_boxed_new_no_copy(lua_State* L, GType type, gpointer pointer);
static gboolean priv_object_new_no_ref(lua_State* L, GObject* pointer);
static void priv_callback_free(gpointer user_data, GClosure* closure);
static void* priv_callback_handle(Data* data, ...);
static int lgob_struct_gc(lua_State* L);
static int lgob_struct_to_string(lua_State* L);

#include "list.c"
#include "type.c"
#include "object.c"
#include "boxed.c"

static void priv_register(lua_State* L, const char* name, lua_CFunction func)
{
	lua_pushstring(L, name);
	lua_pushcfunction(L, func);
	lua_rawset(L, LUA_REGISTRYINDEX);
};

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

static void priv_struct_new(lua_State* L, void* ptr, gboolean need_unref, const char* mtname)
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

static const struct luaL_reg _global[];

/* GObject */

static const struct luaL_reg gobject [] =
{
	{NULL, NULL}
};

static const struct luaL_reg object [] =
{	
	{"set", lgob_object_set},
	{"get", lgob_object_get},
	{"connect", lgob_object_connect},
	{"disconnect", lgob_object_disconnect},
	{"block", lgob_object_block},
	{"unblock", lgob_object_unblock},
	{"ref", lgob_object_ref},
	{"unref", lgob_object_unref},
	{"notify", lgob_object_notify},
	{"thaw_notify", lgob_object_thaw_notify},
	{"freeze_notify", lgob_object_freeze_notify},
	{"cast", lgob_object_cast},
	{NULL, NULL}
};

static const struct luaL_reg type [] =
{
	{"from_name", lgob_type_from_name},
	{"from_instance", lgob_type_from_instance},
	{"name_from_type", lgob_type_name_from_type},
	{"parent", lgob_type_parent},
	{"depth", lgob_type_depth},
	{"is_a", lgob_type_is_a},
	{NULL, NULL}
};

static const struct luaL_reg _glib [] =
{
	{NULL, NULL}
};

static void priv_log_handler(const gchar *log_domain, GLogLevelFlags log_level,
	const gchar *message, lua_State* L)
{
	/* Show the line of the Lua program */
	luaL_where(L, 1);
	if(lua_objlen(L, -1) == 0) luaL_where(L, 1);
	if(lua_objlen(L, -1) == 0) lua_pushstring(L, "? ");
	
	/* Build the message */
	lua_pushstring(L, log_domain);
	lua_pushstring(L, " - ");
	lua_pushstring(L, message);
	lua_pushstring(L, "\n");
	
	/* Log it */
	lua_concat(L, 5);
	g_printerr("%s", lua_tostring(L, -1));
	lua_pop(L, 1);
}

static void priv_handle_log(lua_State* L, const char* domain)
{
	g_log_set_handler(domain, G_LOG_LEVEL_WARNING | G_LOG_LEVEL_CRITICAL | G_LOG_FLAG_RECURSION, (GLogFunc)priv_log_handler, L);
}

static int index_closure(lua_State* L)
{	
	/* Organize the stack to be: 1 - udata, 2 - property, 3 - value (if present) */
	size_t len;
	const char* p = lua_tolstring(L, lua_upvalueindex(1), &len);
	
	#ifdef IDEBUG
	g_debug("Calling index_closure for key %s!", p);
	#endif
	
	lua_pushvalue(L, 2);
	
	if(p[1] == 'e' && p[2] == 't' && len >= 5)
	{
		lua_pushstring(L, &p[4]);
		lua_replace(L, 2);
		
		if(p[0] == 'g') /* getter */
		{
			lgob_object_get(L);
			return 1;
		}
		else
			if(p[0] == 's') /* setter */
			{
				lgob_object_set(L);
				return 0;
			}
	}
	
	luaL_error(L, "attempt to call method '%s' (a nil value)", p);
	return 0;
}

static int __index(lua_State* L)
{
	lua_pushvalue(L, 2);
	lua_pushcclosure(L, index_closure, 1);
	
	return 1;
}

static void _wrap_glib_init(lua_State* L)
{
    /* Initialization */
	g_type_init();
	priv_handle_log(L, "GLib");
	priv_handle_log(L, "GThread");
	priv_handle_log(L, "GLib-GObject");
	luaL_register(L, "glib", _glib);
	luaL_register(L, NULL, _global);
	luaL_loadstring(L, "require('lgob.common')"); lua_call(L, 0, 0);
}

static void _wrap_glib_ret(lua_State* L)
{
	/* Register the classes */
	luaL_register(L, "gobject", gobject);
	int top = lua_gettop(L);
    
	lua_pushliteral(L, "lgobObject");
	luaL_register(L, "gobject.Object", object);
	
	/* Set the object metatable */
	lua_newtable(L);
	lua_pushliteral(L, "__index");
	lua_pushcfunction(L, __index);
	lua_rawset(L, -3);
    
	lua_setmetatable(L, -2);
	
	lua_rawset(L, LUA_REGISTRYINDEX);
	luaL_register(L, "gobject.Type", type);
    
	/* Export some internal functions to other bindings */
	priv_register(L, "lgobObjectNew", lgob_object_new); 
	priv_register(L, "lgobStructNew", lgob_struct_new); 
	priv_register(L, "lgobBoxedNew", lgob_boxed_new); 
	priv_register(L, "lgobGc", lgob_object_gc); 
	priv_register(L, "lgobValuePush", lgob_value_push); 
	priv_register(L, "lgobValueSet", lgob_value_set); 
	priv_register(L, "lgobStructGc", lgob_struct_gc); 
	priv_register(L, "lgobStructTostring", lgob_struct_to_string); 
	priv_register(L, "lgobTostring", lgob_object_to_string); 
	priv_register(L, "lgobEq", lgob_object_eq);
	priv_register(L, "lgobClosureNew", lgob_closure_new);
	priv_register(L, "lgobGstrv2table", lgob_gstrv2table);
	priv_register(L, "lgobTable2gstrv", lgob_table2gstrv);
	priv_register(L, "lgobGSList2table", lgob_gslist2table);
	priv_register(L, "lgobGList2table", lgob_glist2table);
	
	/* Create a table to store class name exceptions */
	lua_newtable(L);
	lua_setfield(L, LUA_REGISTRYINDEX, "lgobSpecial");
	
	/* Create a table to store prefix */
	lua_newtable(L);
	lua_pushliteral(L, "G"); lua_pushliteral(L, "lgob"); lua_rawset(L, -3);
	lua_setfield(L, LUA_REGISTRYINDEX, "lgobPrefix");
	
	/* gobject table must be in the top */
	lua_settop(L, top);
    
}
