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
    
    Copyright (C) 2008 - 2010 Lucas Hermann Negri
*/

/* Lua */
#include <lua.h>
#include <lauxlib.h>

/* C */
#include <stdlib.h>

/* GStreamer */
#include <gst/gst.h>
#include <lgob/common/types.h>
#include <lgob/gobject/types.h>

#ifndef WITHOUT_XOVERLAY
    #include <gst/interfaces/xoverlay.h>
#endif

#define LGOBGST_TIMESCALE 1000000

#include "gst.c"
#include "bin.c"
#include "bus.c"
#include "element.c"
#include "elementfactory.c"
#include "message.c"
#include "pad.c"
#include "parse.c"
#include "pipeline.c"
#include "xoverlay.c"

/* All classes have a metatable */
static void register_class(lua_State* L, const char* name, const char* base,
	const luaL_reg* methods)
{
	/* Register the methods */
	luaL_register(L, name, methods);
	
	/* Create a metatable */
	lua_newtable(L); /* table */
	
	if(base)
	{
		lua_pushliteral(L, "gst");			/* Get the lgob table */
		lua_rawget(L, LUA_GLOBALSINDEX);
		lua_pushliteral(L, "__index");		/* Set the key */
		lua_pushstring(L, base);
		lua_rawget(L, -3);					/* Push the value */
		lua_rawset(L, -4);					/* Set the metatable */
		lua_pop(L, 1);						/* Pop the remaining value */
	}
	
	/* Set the metatable */
	lua_setmetatable(L, -2);
}

static const struct luaL_reg gst [] =
{
	{"version", lgob_version},
	{"get_timescale", lgob_get_timescale},
	{"set_timescale", lgob_set_timescale},
	{NULL, NULL}
};

static const struct luaL_reg element [] =
{
	{"set_state", lgob_element_set_state},
	{"get_state", lgob_element_get_state},
	{"query_duration", lgob_element_query_duration},
	{"query_position", lgob_element_query_positon},
	{"seek_simple", lgob_element_seek_simple},
	{"seek", lgob_element_seek},
	{"link", lgob_element_link},
	{"unlink", lgob_element_unlink},
	{"get_static_pad", lgob_element_get_static_pad},
	{NULL, NULL}
};

static const struct luaL_reg pipeline [] =
{
	{"new", lgob_pipeline_new},
	{"get_bus", lgob_pipeline_get_bus},
	{NULL, NULL}
};

static const struct luaL_reg elementfactory [] =
{
	{"make", lgob_element_factory_make},
	{NULL, NULL}
};

static const struct luaL_reg bus [] =
{
	{"add_signal_watch", lgob_bus_add_signal_watch},
	{"enable_sync_message_emission", lgob_bus_enable_sync_message_emission},
	{NULL, NULL}
};

static const struct luaL_reg bin [] =
{
	{"add", lgob_bin_add},
	{"remove", lgob_bin_remove},
	{NULL, NULL}
};

static const struct luaL_reg pad [] =
{
	{"link", lgob_pad_link},
	{"unlink", lgob_pad_unlink},
	{NULL, NULL}
};

static const struct luaL_reg message [] =
{
	{"get_name", lgob_message_get_name},
	{"get_type", lgob_message_get_type},
	{"get_source", lgob_message_get_source},
	{"parse_tag", lgob_message_parse_tag},
	{NULL, NULL}
};

static const struct luaL_reg parse [] =
{
	{"launch", lgob_parse_launch},
	{NULL, NULL}
};

#ifndef WITHOUT_XOVERLAY
static const struct luaL_reg xoverlay [] =
{
	{"set_xwindow_id", lgob_x_overlay_set_xwindow_id},
	{NULL, NULL}
};
#endif

static void priv_init_gst(lua_State* L)
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

	gst_init(&argc, &argv);
	g_free(argv);
}

int luaopen_lgob_gst(lua_State *L)
{
	priv_init_gst(L);
	
	luaL_register(L, "gst", gst);
	luaL_loadstring(L, "require('lgob.gobject')"); lua_call(L, 0, 0);
	luaL_loadstring(L, "gst.Object = gobject.Object"); lua_call(L, 0, 0);
	
	/* enum GstState */
	lua_pushliteral(L, "STATE_VOID_PENDING"); lua_pushnumber(L, GST_STATE_VOID_PENDING); lua_rawset(L, -3);
	lua_pushliteral(L, "STATE_NULL"); lua_pushnumber(L, GST_STATE_NULL); lua_rawset(L, -3);
	lua_pushliteral(L, "STATE_READY"); lua_pushnumber(L, GST_STATE_READY); lua_rawset(L, -3);
	lua_pushliteral(L, "STATE_PAUSED"); lua_pushnumber(L, GST_STATE_PAUSED); lua_rawset(L, -3);
	lua_pushliteral(L, "STATE_PLAYING"); lua_pushnumber(L, GST_STATE_PLAYING); lua_rawset(L, -3);
	
	/* enum GstStateChangeReturn */
	lua_pushliteral(L, "STATE_CHANGE_FAILURE"); lua_pushnumber(L, GST_STATE_CHANGE_FAILURE); lua_rawset(L, -3);
	lua_pushliteral(L, "STATE_CHANGE_SUCCESS"); lua_pushnumber(L, GST_STATE_CHANGE_SUCCESS); lua_rawset(L, -3);
	lua_pushliteral(L, "STATE_CHANGE_ASYNC"); lua_pushnumber(L, GST_STATE_CHANGE_ASYNC); lua_rawset(L, -3);
	lua_pushliteral(L, "STATE_CHANGE_NO_PREROLL"); lua_pushnumber(L, GST_STATE_CHANGE_NO_PREROLL); lua_rawset(L, -3);
	
	/* enum GstFormat */
	lua_pushliteral(L, "FORMAT_UNDEFINED"); lua_pushnumber(L, GST_FORMAT_UNDEFINED); lua_rawset(L, -3);
	lua_pushliteral(L, "FORMAT_DEFAULT"); lua_pushnumber(L, GST_FORMAT_DEFAULT); lua_rawset(L, -3);
	lua_pushliteral(L, "FORMAT_BYTES"); lua_pushnumber(L, GST_FORMAT_BYTES); lua_rawset(L, -3);
	lua_pushliteral(L, "FORMAT_TIME"); lua_pushnumber(L, GST_FORMAT_TIME); lua_rawset(L, -3);
	lua_pushliteral(L, "FORMAT_BUFFERS"); lua_pushnumber(L, GST_FORMAT_BUFFERS); lua_rawset(L, -3);
	lua_pushliteral(L, "FORMAT_PERCENT"); lua_pushnumber(L, GST_FORMAT_PERCENT); lua_rawset(L, -3);
	
	/* enum GstMessageType */
	lua_pushliteral(L, "MESSAGE_UNKNOWN"); lua_pushnumber(L, GST_MESSAGE_UNKNOWN); lua_rawset(L, -3);
	lua_pushliteral(L, "MESSAGE_EOS"); lua_pushnumber(L, GST_MESSAGE_EOS); lua_rawset(L, -3);
	lua_pushliteral(L, "MESSAGE_ERROR"); lua_pushnumber(L, GST_MESSAGE_ERROR); lua_rawset(L, -3);
	lua_pushliteral(L, "MESSAGE_WARNING"); lua_pushnumber(L, GST_MESSAGE_WARNING); lua_rawset(L, -3);
	lua_pushliteral(L, "MESSAGE_INFO"); lua_pushnumber(L, GST_MESSAGE_INFO); lua_rawset(L, -3);
	lua_pushliteral(L, "MESSAGE_TAG"); lua_pushnumber(L, GST_MESSAGE_TAG); lua_rawset(L, -3);
	lua_pushliteral(L, "MESSAGE_BUFFERING"); lua_pushnumber(L, GST_MESSAGE_BUFFERING); lua_rawset(L, -3);
	lua_pushliteral(L, "MESSAGE_STATE_CHANGED"); lua_pushnumber(L, GST_MESSAGE_STATE_CHANGED); lua_rawset(L, -3);
	lua_pushliteral(L, "MESSAGE_STATE_DIRTY"); lua_pushnumber(L, GST_MESSAGE_STATE_DIRTY); lua_rawset(L, -3);
	lua_pushliteral(L, "MESSAGE_STEP_DONE"); lua_pushnumber(L, GST_MESSAGE_STEP_DONE); lua_rawset(L, -3);
	lua_pushliteral(L, "MESSAGE_CLOCK_PROVIDE"); lua_pushnumber(L, GST_MESSAGE_CLOCK_PROVIDE); lua_rawset(L, -3);
	lua_pushliteral(L, "MESSAGE_CLOCK_LOST"); lua_pushnumber(L, GST_MESSAGE_CLOCK_LOST); lua_rawset(L, -3);
	lua_pushliteral(L, "MESSAGE_NEW_CLOCK"); lua_pushnumber(L, GST_MESSAGE_NEW_CLOCK); lua_rawset(L, -3);
	lua_pushliteral(L, "MESSAGE_STRUCTURE_CHANGE"); lua_pushnumber(L, GST_MESSAGE_STRUCTURE_CHANGE); lua_rawset(L, -3);
	lua_pushliteral(L, "MESSAGE_STREAM_STATUS"); lua_pushnumber(L, GST_MESSAGE_STREAM_STATUS); lua_rawset(L, -3);
	lua_pushliteral(L, "MESSAGE_APPLICATION"); lua_pushnumber(L, GST_MESSAGE_APPLICATION); lua_rawset(L, -3);
	lua_pushliteral(L, "MESSAGE_ELEMENT"); lua_pushnumber(L, GST_MESSAGE_ELEMENT); lua_rawset(L, -3);
	lua_pushliteral(L, "MESSAGE_SEGMENT_START"); lua_pushnumber(L, GST_MESSAGE_SEGMENT_START); lua_rawset(L, -3);
	lua_pushliteral(L, "MESSAGE_SEGMENT_DONE"); lua_pushnumber(L, GST_MESSAGE_SEGMENT_DONE); lua_rawset(L, -3);
	lua_pushliteral(L, "MESSAGE_DURATION"); lua_pushnumber(L, GST_MESSAGE_DURATION); lua_rawset(L, -3);
	lua_pushliteral(L, "MESSAGE_LATENCY"); lua_pushnumber(L, GST_MESSAGE_LATENCY); lua_rawset(L, -3);
	lua_pushliteral(L, "MESSAGE_ASYNC_START"); lua_pushnumber(L, GST_MESSAGE_ASYNC_START); lua_rawset(L, -3);
	lua_pushliteral(L, "MESSAGE_ASYNC_DONE"); lua_pushnumber(L, GST_MESSAGE_ASYNC_DONE); lua_rawset(L, -3);
	lua_pushliteral(L, "MESSAGE_ANY"); lua_pushnumber(L, GST_MESSAGE_ANY); lua_rawset(L, -3);
	
	/* enum GstPadLinkReturn */
	lua_pushliteral(L, "PAD_LINK_OK"); lua_pushnumber(L, GST_PAD_LINK_OK); lua_rawset(L, -3);
	lua_pushliteral(L, "PAD_LINK_WRONG_HIERARCHY"); lua_pushnumber(L, GST_PAD_LINK_WRONG_HIERARCHY); lua_rawset(L, -3);
	lua_pushliteral(L, "PAD_LINK_WAS_LINKED"); lua_pushnumber(L, GST_PAD_LINK_WAS_LINKED); lua_rawset(L, -3);
	lua_pushliteral(L, "PAD_LINK_WRONG_DIRECTION"); lua_pushnumber(L, GST_PAD_LINK_WRONG_DIRECTION); lua_rawset(L, -3);
	lua_pushliteral(L, "PAD_LINK_NOFORMAT"); lua_pushnumber(L, GST_PAD_LINK_NOFORMAT); lua_rawset(L, -3);
	lua_pushliteral(L, "PAD_LINK_NOSCHED"); lua_pushnumber(L, GST_PAD_LINK_NOSCHED); lua_rawset(L, -3);
	lua_pushliteral(L, "PAD_LINK_REFUSED"); lua_pushnumber(L, GST_PAD_LINK_REFUSED); lua_rawset(L, -3);
    
    /* enum GstSeekFlags */
    lua_pushliteral(L, "SEEK_FLAG_NONE"); lua_pushnumber(L, GST_SEEK_FLAG_NONE); lua_rawset(L, -3);
    lua_pushliteral(L, "SEEK_FLAG_FLUSH"); lua_pushnumber(L, GST_SEEK_FLAG_FLUSH); lua_rawset(L, -3);
    lua_pushliteral(L, "SEEK_FLAG_ACCURATE"); lua_pushnumber(L, GST_SEEK_FLAG_ACCURATE); lua_rawset(L, -3);
    lua_pushliteral(L, "SEEK_FLAG_KEY_UNIT"); lua_pushnumber(L, GST_SEEK_FLAG_KEY_UNIT); lua_rawset(L, -3);
    lua_pushliteral(L, "SEEK_FLAG_SEGMENT"); lua_pushnumber(L, GST_SEEK_FLAG_SEGMENT); lua_rawset(L, -3);
    lua_pushliteral(L, "SEEK_FLAG_SKIP");   lua_pushnumber(L, GST_SEEK_FLAG_SKIP); lua_rawset(L, -3);
	
	/* Register the classes */
	register_class(L, "gst.Bus", "Object", bus);
	register_class(L, "gst.Pad", "Object", pad);
	register_class(L, "gst.ElementFactory", "Object", elementfactory);
	register_class(L, "gst.Element", "Object", element);
		register_class(L, "gst.Bin", "Element", bin);
			register_class(L, "gst.Pipeline", "Bin", pipeline);

	/* Interfaces and non-objects */
	register_class(L, "gst.Parse", NULL, parse);
	register_class(L, "gst.Message", NULL, message);
	
#ifndef WITHOUT_XOVERLAY
	register_class(L, "gst.XOverlay", NULL, xoverlay);
	#endif
	
	/* Own the Gst prefix */
	lua_getfield(L, LUA_REGISTRYINDEX, "lgobPrefix");
	lua_pushliteral(L, "Gst"); lua_pushliteral(L, "gst"); lua_rawset(L, -3);
	
	lua_pop(L, 1);
	/* Push the main table to the top */
	lua_getfield(L, LUA_GLOBALSINDEX, "gst");
	
	luaL_loadstring(L, "glib.handle_log('Gst')"); lua_call(L, 0, 0);
	
	return 1;
}
