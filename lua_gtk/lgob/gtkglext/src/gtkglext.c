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
    
    Copyright (C) 2009 - 2011 Lucas Hermann Negri
*/

/* Lua */
#include <lua.h>
#include <lauxlib.h>

/* GTK+ */
#include <gtk/gtk.h>
#include <gtk/gtkgl.h>

/* C */
#include <string.h>

/* lgob */
#include <lgob/common/types.h>
#include <lgob/gobject/types.h>

static void object_new(lua_State* L, gpointer ptr, gboolean constructor)
{
	lua_pushliteral(L, "lgobObjectNew");
	lua_rawget(L, LUA_REGISTRYINDEX);
	lua_pushlightuserdata(L, ptr);
	
#if GLIB_MINOR_VERSION >= 10
	lua_pushboolean(L, constructor);
#else
	g_object_ref(ptr);

	if( GTK_IS_OBJECT(ptr) && GTK_OBJECT_FLOATING(ptr) )
	{
		lua_pushboolean(L, FALSE);
		gtk_object_sink(ptr);
	}
	else
	{
		lua_pushboolean(L, constructor);
	}
#endif
	lua_call(L, 2, 1);
}

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

static int lglext_query_version(lua_State* L)
{
	int major, minor;
	gdk_gl_query_version(&major, &minor);

	lua_pushinteger(L, major);
	lua_pushinteger(L, minor);

	return 2;
}

/* GdkGLConfig */

static int lglext_config_new_by_mode(lua_State* L)
{
	luaL_checktype(L, 1, LUA_TNUMBER);
	GdkGLConfig* ptr = gdk_gl_config_new_by_mode(lua_tointeger(L, 1));
	object_new(L, ptr, TRUE);

	return 1;
}

static int lglext_config_new_by_mode_for_screen(lua_State* L)
{
	luaL_checktype(L, 1, LUA_TUSERDATA);
	luaL_checktype(L, 2, LUA_TNUMBER);
	Object* obj = lua_touserdata(L, 1);

	GdkGLConfig* ptr = gdk_gl_config_new_by_mode_for_screen(
		GDK_SCREEN(obj->pointer), lua_tointeger(L, 2));
	object_new(L, ptr, TRUE);

	return 1;
}

/* GdkGLDrawable */

static int lglext_drawable_gl_begin(lua_State* L)
{
	luaL_checktype(L, 1, LUA_TUSERDATA);
	luaL_checktype(L, 2, LUA_TUSERDATA);

	Object* obj1 = lua_touserdata(L, 1);
	Object* obj2 = lua_touserdata(L, 2);
	gboolean res = gdk_gl_drawable_gl_begin(GDK_GL_DRAWABLE(obj1->pointer),
		GDK_GL_CONTEXT(obj2->pointer));

	lua_pushboolean(L, res);

	return 1;
}

static int lglext_drawable_gl_end(lua_State* L)
{
	luaL_checktype(L, 1, LUA_TUSERDATA);

	Object* obj = lua_touserdata(L, 1);
	gdk_gl_drawable_gl_end(GDK_GL_DRAWABLE(obj->pointer));

	return 0;
}

static int lglext_drawable_swap_buffers(lua_State* L)
{
	luaL_checktype(L, 1, LUA_TUSERDATA);

	Object* obj = lua_touserdata(L, 1);
	gdk_gl_drawable_swap_buffers(GDK_GL_DRAWABLE(obj->pointer));

	return 0;
}

/* Widget extensions */

static int lglext_widget_set_gl_capability(lua_State* L)
{
	luaL_checktype(L, 1, LUA_TUSERDATA);
	luaL_checktype(L, 2, LUA_TUSERDATA);
	luaL_checktype(L, 3, LUA_TBOOLEAN);
	luaL_checktype(L, 4, LUA_TNUMBER);

	Object* obj1 = lua_touserdata(L, 1);
	Object* obj2 = lua_touserdata(L, 2);

	gboolean res = gtk_widget_set_gl_capability(GTK_WIDGET(obj1->pointer),
		GDK_GL_CONFIG(obj2->pointer), NULL, lua_toboolean(L, 3),
		lua_tointeger(L, 4));

	lua_pushboolean(L, res);

	return 1;
}

static int lglext_widget_is_gl_capable(lua_State* L)
{
	luaL_checktype(L, 1, LUA_TUSERDATA);
	Object* obj = lua_touserdata(L, 1);

	gboolean res = gtk_widget_is_gl_capable(GTK_WIDGET(obj->pointer));
	lua_pushboolean(L, res);

	return 1;
}

static int lglext_widget_get_gl_config(lua_State* L)
{
	luaL_checktype(L, 1, LUA_TUSERDATA);
	Object* obj = lua_touserdata(L, 1);

	GdkGLConfig* ptr = gtk_widget_get_gl_config(GTK_WIDGET(obj->pointer));
	object_new(L, ptr, FALSE);

	return 1;
}

static int lglext_widget_get_gl_context(lua_State* L)
{
	luaL_checktype(L, 1, LUA_TUSERDATA);
	Object* obj = lua_touserdata(L, 1);

	GdkGLContext* ptr = gtk_widget_get_gl_context(GTK_WIDGET(obj->pointer));
	object_new(L, ptr, FALSE);

	return 1;
}

static int lglext_widget_get_gl_drawable(lua_State* L)
{
	luaL_checktype(L, 1, LUA_TUSERDATA);
	Object* obj = lua_touserdata(L, 1);

	GdkGLDrawable* ptr = gtk_widget_get_gl_drawable(GTK_WIDGET(obj->pointer));
	object_new(L, ptr, FALSE);

	return 1;
}

static const struct luaL_reg gtkglext [] =
{
	{"query_version", lglext_query_version},
	{NULL, NULL}
};

static const struct luaL_reg config [] =
{
	{"new_by_mode", lglext_config_new_by_mode},
	{"new_by_mode_for_screen", lglext_config_new_by_mode_for_screen},
	{NULL, NULL}
};

static const struct luaL_reg widget [] =
{
	{"get_gl_config", lglext_widget_get_gl_config},
	{"get_gl_context", lglext_widget_get_gl_context},
	{"get_gl_drawable", lglext_widget_get_gl_drawable},
	{"is_gl_capable", lglext_widget_is_gl_capable},
	{"set_gl_capability", lglext_widget_set_gl_capability},
	{NULL, NULL}
};

static const struct luaL_reg drawable [] =
{
	{"gl_begin", lglext_drawable_gl_begin},
	{"gl_end", lglext_drawable_gl_end},
	{"swap_buffers", lglext_drawable_swap_buffers},
	{NULL, NULL}
};

int luaopen_lgob_gtkglext(lua_State *L)
{
    luaL_loadstring(L, "require('lgob.gtk')"); lua_call(L, 0, 0);
	luaL_loadstring(L, "glib.handle_log('GtkGLExt')"); lua_call(L, 0, 0);
    
    gtk_gl_init(NULL, NULL);
	luaL_register(L, "gtkglext", gtkglext);
	luaL_loadstring(L, "gtkglext.Object = gobject.Object"); lua_call(L, 0, 0);
    
	/* Register the classes */
	register_class(L, "Config", "Object", config);
	register_class(L, "Widget", "Object", widget);
	register_class(L, "Drawable", NULL, drawable);
	
	/* Register special class table */
	lua_getfield(L, LUA_REGISTRYINDEX, "lgobSpecial");
	lua_pushliteral(L, "GdkGLWindowImplX11"); lua_getfield(L, -3, "Drawable"); lua_rawset(L, -3);
	lua_pushliteral(L, "GdkGLWindowImplWin32"); lua_getfield(L, -3, "Drawable"); lua_rawset(L, -3);
	lua_pop(L, 1);
	
	/* GdkGLConfigMode */
	lua_pushliteral(L, "MODE_RGB"); lua_pushinteger(L, GDK_GL_MODE_RGB); lua_rawset(L, -3);
	lua_pushliteral(L, "MODE_RGBA"); lua_pushinteger(L, GDK_GL_MODE_RGBA); lua_rawset(L, -3);
	lua_pushliteral(L, "MODE_SINGLE"); lua_pushinteger(L, GDK_GL_MODE_SINGLE); lua_rawset(L, -3);
	lua_pushliteral(L, "MODE_INDEX"); lua_pushinteger(L, GDK_GL_MODE_INDEX); lua_rawset(L, -3);
	lua_pushliteral(L, "MODE_DOUBLE"); lua_pushinteger(L, GDK_GL_MODE_DOUBLE); lua_rawset(L, -3);
	lua_pushliteral(L, "MODE_STEREO"); lua_pushinteger(L, GDK_GL_MODE_STEREO); lua_rawset(L, -3);
	lua_pushliteral(L, "MODE_ALPHA"); lua_pushinteger(L, GDK_GL_MODE_ALPHA); lua_rawset(L, -3);
	lua_pushliteral(L, "MODE_DEPTH"); lua_pushinteger(L, GDK_GL_MODE_DEPTH); lua_rawset(L, -3);
	lua_pushliteral(L, "MODE_STENCIL"); lua_pushinteger(L, GDK_GL_MODE_STENCIL); lua_rawset(L, -3);
	lua_pushliteral(L, "MODE_ACCUM"); lua_pushinteger(L, GDK_GL_MODE_ACCUM); lua_rawset(L, -3);
	
	return 1;
}
