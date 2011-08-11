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

static int lgob_message_get_type(lua_State* L)
{
	luaL_checktype(L, 1, LUA_TUSERDATA);
	Object* obj = lua_touserdata(L, 1);
	
	lua_pushinteger(L, GST_MESSAGE_TYPE(obj->pointer));
	
	return 1;
}

static int lgob_message_get_name(lua_State* L)
{
	luaL_checktype(L, 1, LUA_TUSERDATA);
	Object* obj = lua_touserdata(L, 1);
	
	const GstStructure* st = gst_message_get_structure(GST_MESSAGE(obj->pointer));
	
	if(st)
		lua_pushstring(L, gst_structure_get_name(st));
	else
		lua_pushnil(L); /* For consistency */
	
	return 1;
}

static int lgob_message_get_source(lua_State* L)
{
	luaL_checktype(L, 1, LUA_TUSERDATA);
	Object* obj = lua_touserdata(L, 1);
	
	object_new(L, GST_MESSAGE_SRC(obj->pointer), "Object");
	return 1;
}

static void priv_tag_foreach(const GstTagList* list, const gchar* tag, gpointer ud)
{
	lua_State* L = ud;

	/* Get the GValue */
	const GValue* val = gst_tag_list_get_value_index(list, tag, 0);
	
	/* Convert it */
	lua_getfield(L, LUA_REGISTRYINDEX, "lgobValuePush");
	lua_pushlightuserdata(L, (void*)val);
	lua_call(L, 1, 1);
	
	/* Set the table field. We know that the table is at index 2 */
	lua_setfield(L, 2, tag);
}

static int lgob_message_parse_tag(lua_State* L)
{
	luaL_checktype(L, 1, LUA_TUSERDATA);
	luaL_checktype(L, 2, LUA_TTABLE);
	
	Object* obj = lua_touserdata(L, 1);
	GstTagList* list;
	gst_message_parse_tag(GST_MESSAGE(obj->pointer), &list);
	gst_tag_list_foreach(list, priv_tag_foreach, L);
	gst_tag_list_free(list);
	
	return 0;
}
