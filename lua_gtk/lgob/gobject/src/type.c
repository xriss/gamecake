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

int lgob_type_from_name(lua_State* L)
{
	luaL_checktype(L, 1, LUA_TSTRING);
	GType type = g_type_from_name(lua_tostring(L, 1));
	lua_pushinteger(L, type);
	
	return 1;
}

int lgob_type_from_instance(lua_State* L)
{
	luaL_checktype(L, 1, LUA_TUSERDATA);
	Object* obj = lua_touserdata(L, 1);
	GType type = G_TYPE_FROM_INSTANCE(obj->pointer);
	lua_pushinteger(L, type);
	
	return 1;
}

int lgob_type_name_from_type(lua_State* L)
{
	luaL_checktype(L, 1, LUA_TNUMBER);
	const gchar* name = g_type_name(lua_tointeger(L, 1));
	lua_pushstring(L, name);
	
	return 1;
}

int lgob_type_parent(lua_State* L)
{
	luaL_checktype(L, 1, LUA_TNUMBER);
	GType parent = g_type_parent(lua_tointeger(L, 1));
	lua_pushinteger(L, parent);
	
	return 1;
}

int lgob_type_depth(lua_State* L)
{
	luaL_checktype(L, 1, LUA_TNUMBER);
	guint depth = g_type_depth(lua_tointeger(L, 1));
	lua_pushinteger(L, depth);
	
	return 1;
}

int lgob_type_is_a(lua_State* L)
{
	luaL_checktype(L, 1, LUA_TNUMBER);
	luaL_checktype(L, 2, LUA_TNUMBER);
	gboolean res = g_type_is_a(lua_tointeger(L, 1), lua_tointeger(L, 2));
	lua_pushboolean(L, res);
	
	return 1;
}
