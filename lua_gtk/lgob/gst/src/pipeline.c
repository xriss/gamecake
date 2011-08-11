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

static int lgob_pipeline_new(lua_State* L)
{
	luaL_checktype(L, 1, LUA_TSTRING);
	GstElement* ptr = gst_pipeline_new(lua_tostring(L, 1));
	
	object_new(L, ptr, "Pipeline");
	return 1;
}

static int lgob_pipeline_get_bus(lua_State* L)
{
	luaL_checktype(L, 1, LUA_TUSERDATA);
	Object* obj = lua_touserdata(L, 1);
	
	GstBus* ptr = gst_pipeline_get_bus(GST_PIPELINE(obj->pointer));
	object_new(L, ptr, "Bus");
	
	return 1; 
	
}
