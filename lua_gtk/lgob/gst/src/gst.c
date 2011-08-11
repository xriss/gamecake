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

static int priv_timescale = LGOBGST_TIMESCALE;

static int lgob_version(lua_State* L)
{
	char rv[31], cv[31];
	guint v[4];

	gst_version(&v[0], &v[1], &v[2], &v[3]);
	snprintf(rv, 30, "%d.%d.%d.%d", v[0], v[1], v[2], v[3]);
	snprintf(cv, 30, "%d.%d.%d.%d", GST_VERSION_MAJOR, GST_VERSION_MINOR, GST_VERSION_MICRO, GST_VERSION_NANO);

	lua_pushstring(L, rv);
	lua_pushstring(L, cv);

	return 2;
}

static void object_new(lua_State* L, gpointer ptr, gchar* name)
{
	lua_pushliteral(L, "lgobObjectNew");
	lua_rawget(L, LUA_REGISTRYINDEX);
	lua_pushlightuserdata(L, ptr);
	lua_pushboolean(L, FALSE);
	lua_call(L, 2, 1);
	
	/* Now configure the metatable */
	if(lua_isuserdata(L, -1))
	{
		lua_getmetatable(L, -1);
		
		/* Set the index */
		lua_pushliteral(L, "__index");
		lua_pushliteral(L, "gst");
		lua_rawget(L, LUA_GLOBALSINDEX);
		lua_pushstring(L, name);
		lua_rawget(L, -2);
		lua_replace(L, -2);
		lua_rawset(L, -3);
		lua_pop(L, 1);
	}
}

static int lgob_get_timescale(lua_State* L)
{
	lua_pushinteger(L, priv_timescale);
	return 1;
}

static int lgob_set_timescale(lua_State* L)
{
	luaL_checktype(L, 1, LUA_TNUMBER);
	priv_timescale = lua_tointeger(L, 1);
	return 0;
}
