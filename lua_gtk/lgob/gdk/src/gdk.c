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

/* GDK */
#include <gdk/gdk.h>

/* C */
#include <string.h>

#include <lgob/common/types.h>
#define _LIB_FREE g_free

/* Platform specific */
#ifdef G_OS_WIN32
	#include <gdk/gdkwin32.h> /* Go ahead, blame me */
	#define GET_HANDLER(win) ((ptrdiff_t)GDK_WINDOW_HWND(win))
#else
	#include <gdk/gdkx.h>
	#define GET_HANDLER(win) (GDK_DRAWABLE_XID(win))
#endif

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

static void boxed_new(lua_State* L, GType type, gpointer ptr)
{
	lua_pushliteral(L, "lgobBoxedNew");
	lua_rawget(L, LUA_REGISTRYINDEX);
	lua_pushinteger(L, type);
	lua_pushlightuserdata(L, ptr);
	lua_call(L, 2, 1);
}

/*
static void struct_new(lua_State* L, gpointer ptr, gboolean need_unref)
{
	lua_pushliteral(L, "lgobStructNew");
	lua_rawget(L, LUA_REGISTRYINDEX);
	lua_pushlightuserdata(L, ptr);
	lua_pushboolean(L, need_unref);
	lua_call(L, 2, 1);
}*/

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

static const struct luaL_reg gdk [] =
{
	{NULL, NULL}
};

static const struct luaL_reg _global[];

static void priv_init_gdk(lua_State* L)
{
	int argc = 1;
	char** argv = g_malloc(argc * sizeof(char*));

	lua_pushliteral(L, "arg");
	lua_rawget(L, LUA_GLOBALSINDEX);

	if(!lua_istable(L, -1))
		argv[0] = "lgob";
	else
	{
		lua_pushnumber(L, 0);
		lua_rawget(L, -2);
		argv[0] = (char*)luaL_optstring(L, -1, "lgob");
	}

	gdk_init(&argc, &argv);
	g_free(argv);
}

static void _wrap_gdk_init(lua_State* L)
{
	priv_init_gdk(L);
	
	luaL_register(L, "gdk", gdk);
	luaL_register(L, NULL, _global);
	
	luaL_loadstring(L, "require('lgob.gobject')"); lua_call(L, 0, 0);
	luaL_loadstring(L, "gdk.Object = gobject.Object"); lua_call(L, 0, 0);
	
	lua_getfield(L, LUA_REGISTRYINDEX, "lgobPrefix");
	lua_pushliteral(L, "Gdk"); lua_pushliteral(L, "gdk"); lua_rawset(L, -3);
	lua_pop(L, 1);
	
	luaL_loadstring(L, "glib.handle_log('Gdk')"); lua_call(L, 0, 0);
}

static void _wrap_gdk_ret(lua_State* L)
{
	/* Fix some names */
	lua_getfield(L, LUA_REGISTRYINDEX, "lgobSpecial");
	lua_pushliteral(L, "GdkScreenX11"); lua_getfield(L, -3, "Screen"); lua_rawset(L, -3);
	lua_pushliteral(L, "GdkScreenWin32"); lua_getfield(L, -3, "Screen"); lua_rawset(L, -3);
	lua_pushliteral(L, "GdkPixbufGifAnim"); lua_getfield(L, -3, "PixbufAnimation"); lua_rawset(L, -3);
	lua_pushliteral(L, "GdkPixbufAniAnim"); lua_getfield(L, -3, "PixbufAnimation"); lua_rawset(L, -3);
	lua_pushliteral(L, "GdkPixbufGifAnimIter"); lua_getfield(L, -3, "PixbufAnimationIter"); lua_rawset(L, -3);
	lua_pushliteral(L, "GdkPixbufAniAnimIter"); lua_getfield(L, -3, "PixbufAnimationIter"); lua_rawset(L, -3);
	lua_pop(L, 1);
}

static void pixbuf_options(lua_State* L, int idx, char*** k, char*** v)
{
    size_t len = lua_objlen(L, idx);
    
    if(len > 0)
    {
        size_t size     =  sizeof(char*) * (len + 1);
        size_t cur      = 0;
        char** keys     = g_malloc(size);
        char** values   = g_malloc(size);
        
        lua_pushnil(L);
        while( lua_next(L, idx) )
        {
            /* key MUST be a string. Will not be modified. */
            keys  [cur] = (char*)lua_tostring(L, -2);
            values[cur] = (char*)lua_tostring(L, -1);
            lua_pop(L, 1);
            ++cur;
        }
        
        keys[cur] = values[cur] = NULL;
        *k = keys;
        *v = values;
    }
    else
    {
        *k = *v = NULL;
    }
}
