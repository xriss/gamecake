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

static int lgob_pad_link(lua_State* L)
{
	luaL_checktype(L, 1, LUA_TUSERDATA);
	luaL_checktype(L, 2, LUA_TUSERDATA);
	
	Object* obj1 = lua_touserdata(L, 1);
	Object* obj2 = lua_touserdata(L, 2);
	GstPadLinkReturn res = gst_pad_link(GST_PAD(obj1->pointer), GST_PAD(obj2->pointer));
	
	lua_pushnumber(L, res);
	return 1;
}

static int lgob_pad_unlink(lua_State* L)
{
	luaL_checktype(L, 1, LUA_TUSERDATA);
	luaL_checktype(L, 2, LUA_TUSERDATA);
	
	Object* obj1 = lua_touserdata(L, 1);
	Object* obj2 = lua_touserdata(L, 2);
	gboolean res = gst_pad_unlink(GST_PAD(obj1->pointer), GST_PAD(obj2->pointer));
	
	lua_pushboolean(L, res);
	return 1;
}
