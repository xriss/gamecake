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

static int lgob_parse_launch(lua_State* L)
{
	luaL_checktype(L, 1, LUA_TSTRING);
	
	GError* errors = NULL;
	GstElement* ptr = gst_parse_launch(lua_tostring(L, 1), &errors);
	
	if(!ptr)
	{
		lua_pushboolean(L, FALSE);
		lua_pushstring(L, errors->message);
		g_error_free(errors);
		return 2;
	}
	 
	object_new(L, ptr, "Pipeline");
	
	return 1;
}
