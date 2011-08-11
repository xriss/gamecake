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

static int lgob_element_set_state(lua_State* L)
{
	luaL_checktype(L, 1, LUA_TUSERDATA);
	luaL_checktype(L, 2, LUA_TNUMBER);
	
	Object* obj = lua_touserdata(L, 1);
	GstStateChangeReturn res = gst_element_set_state(GST_ELEMENT(obj->pointer), (GstState)lua_tointeger(L, 2));
	lua_pushnumber(L, res);
	
	return 1;
}

static int lgob_element_get_state(lua_State* L)
{
    luaL_checktype(L, 1, LUA_TUSERDATA);
    luaL_checktype(L, 2, LUA_TNUMBER);
    
	Object* obj = lua_touserdata(L, 1);
	GstState state , pending ;
	GstStateChangeReturn res = gst_element_get_state(GST_ELEMENT(obj->pointer), &state , &pending , (GstClockTime)lua_tointeger(L, 2) );
	
	lua_pushnumber(L, res);
	lua_pushnumber(L, state);
	lua_pushnumber(L, pending);
	
	return 3;
}

static int lgob_element_query_positon(lua_State* L)
{
	luaL_checktype(L, 1, LUA_TUSERDATA);
	luaL_checktype(L, 2, LUA_TNUMBER);
	Object* obj = lua_touserdata(L, 1);
	
	gint64 pos;
	GstFormat format = lua_tointeger(L, 2);
	gboolean res = gst_element_query_position(GST_ELEMENT(obj->pointer), &format, &pos);
	pos /= LGOBGST_TIMESCALE;
	
	lua_pushboolean(L, res);
	lua_pushnumber(L, format);
	lua_pushnumber(L, pos);
	
	return 3;
}

static int lgob_element_query_duration(lua_State* L)
{
	luaL_checktype(L, 1, LUA_TUSERDATA);
	luaL_checktype(L, 2, LUA_TNUMBER);
	Object* obj = lua_touserdata(L, 1);
	
	gint64 pos;
	GstFormat format = lua_tointeger(L, 2);
	gboolean res = gst_element_query_duration(GST_ELEMENT(obj->pointer), &format, &pos);
	pos /= LGOBGST_TIMESCALE;
	
	lua_pushboolean(L, res);
	lua_pushnumber(L, format);
	lua_pushnumber(L, pos);
	
	return 3;
}

static int lgob_element_seek_simple(lua_State* L)
{
	luaL_checktype(L, 1, LUA_TUSERDATA);
	luaL_checktype(L, 2, LUA_TNUMBER);
	luaL_checktype(L, 3, LUA_TNUMBER);
	
	Object* obj = lua_touserdata(L, 1);
	gint64 pos = lua_tonumber(L, 3) * LGOBGST_TIMESCALE;
	
	gboolean res = gst_element_seek_simple(GST_ELEMENT(obj->pointer), (GstFormat)lua_tointeger(L, 2),
		GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT, pos);
		
	lua_pushboolean(L, res);
	return 1;
}

static int lgob_element_seek(lua_State* L)
{
    luaL_checktype(L, 1, LUA_TUSERDATA);
    luaL_checktype(L, 2, LUA_TNUMBER);
    luaL_checktype(L, 3, LUA_TNUMBER);
    luaL_checktype(L, 4, LUA_TNUMBER);
    luaL_checktype(L, 5, LUA_TNUMBER);
    luaL_checktype(L, 6, LUA_TNUMBER);
    luaL_checktype(L, 7, LUA_TNUMBER);
    luaL_checktype(L, 8, LUA_TNUMBER);
    
    Object* obj = lua_touserdata(L, 1);
    gint64 cur = lua_tointeger(L, 6) * LGOBGST_TIMESCALE;
    
    gboolean res = gst_element_seek(obj->pointer, lua_tonumber(L, 2),
        lua_tointeger(L, 3), lua_tointeger(L, 4), lua_tointeger(L, 5),
        cur, lua_tointeger(L, 7), lua_tointeger(L, 8));
        
    lua_pushboolean(L, res);
    return 1;
}

static int lgob_element_link(lua_State* L)
{
	luaL_checktype(L, 1, LUA_TUSERDATA);
	
	gboolean res = TRUE;
	int i, top = lua_gettop(L);
	Object* obj1 = lua_touserdata(L, 1);
	Object* obj2;
	
	for(i = 2; i <=top; ++i)
	{
		luaL_checktype(L, i, LUA_TUSERDATA);
		obj2 = lua_touserdata(L, i);
		res = res & gst_element_link(GST_ELEMENT(obj1->pointer), obj2->pointer);
		obj1 = obj2;
	}
	
	lua_pushboolean(L, res);
	return 1;
	
}

static int lgob_element_unlink(lua_State* L)
{
	luaL_checktype(L, 1, LUA_TUSERDATA);
	
	int i, top = lua_gettop(L);
	Object* obj1 = lua_touserdata(L, 1);
	Object* obj2;
	
	for(i = 2; i <=top; ++i)
	{
		luaL_checktype(L, i, LUA_TUSERDATA);
		obj2 = lua_touserdata(L, i);
		gst_element_unlink(GST_ELEMENT(obj1->pointer), obj2->pointer);
		obj1 = obj2;
	}
	
	return 0;
}

static int lgob_element_get_static_pad(lua_State* L)
{
	luaL_checktype(L, 1, LUA_TUSERDATA);
	luaL_checktype(L, 2, LUA_TSTRING);
	
	Object* obj = lua_touserdata(L, 1);
	GstPad* ptr = gst_element_get_static_pad(GST_ELEMENT(obj->pointer), lua_tostring(L, 2));
	object_new(L, ptr, "Pad");
	
	return 1;
}
