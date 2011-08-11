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

static int lgob_bin_add(lua_State* L)
{
	luaL_checktype(L, 1, LUA_TUSERDATA);
	Object* obj = lua_touserdata(L, 1);
	int top = lua_gettop(L), i;
	gboolean res = TRUE;
	
	for(i = 2; i <= top; ++i)
	{
		luaL_checktype(L, i, LUA_TUSERDATA);
		Object* child = lua_touserdata(L, i);
		res = res & gst_bin_add(GST_BIN(obj->pointer), GST_ELEMENT(child->pointer));
	}
	
	lua_pushboolean(L, res);
	
	return 1;
}

static int lgob_bin_remove(lua_State* L)
{
	luaL_checktype(L, 1, LUA_TUSERDATA);
	Object* obj = lua_touserdata(L, 1);
	int top = lua_gettop(L), i;
	gboolean res = TRUE;
	
	for(i = 2; i <= top; ++i)
	{
		luaL_checktype(L, i, LUA_TUSERDATA);
		Object* child = lua_touserdata(L, i);
		res = res & gst_bin_remove(GST_BIN(obj->pointer), GST_ELEMENT(child->pointer));
	}
	
	lua_pushboolean(L, res);
	
	return 1;
}
