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

static int priv_boxed_gc(lua_State* L)
{
	#ifdef IDEBUG
	g_debug("Garbage collecting a Boxed!");
	#endif

	Boxed* b = lua_touserdata(L, 1);
	g_boxed_free(b->type, b->pointer);

	return 0;
}

static int priv_boxed_tostring(lua_State* L)
{
	Boxed* b = lua_touserdata(L, 1);
	char name[51];
	snprintf(name, 50, "Boxed(%s): %p", g_type_name(b->type), b->pointer);
	lua_pushstring(L, name);

	return 1;
}

static int priv_boxed_eq(lua_State* L)
{
	Boxed* b1 = lua_touserdata(L, 1);
	Boxed* b2 = lua_touserdata(L, 2);
	lua_pushboolean(L, b1->pointer == b2->pointer);

	return 1;
}

static int lgob_boxed_new(lua_State* L)
{
	priv_boxed_new(L, lua_tointeger(L, 1), lua_touserdata(L, 2));
	return 1;
}

static gboolean priv_boxed_new(lua_State* L, GType type, gpointer pointer)
{
	#ifdef IDEBUG
	g_debug("Creating a new Boxed(%s)!", g_type_name(type));
	#endif

	if(pointer)
	{
		Boxed* b = lua_newuserdata(L, sizeof(Boxed));
		b->pointer = g_boxed_copy(type, pointer);
		b->type = type;
		int top = lua_gettop(L);

		/* Set the metatable */
		if(luaL_newmetatable(L, "lgobBoxed"))
		{
			/* A new metatable */
			#ifdef IDEBUG
			g_debug("Creating a new metatable!");
			#endif

			/* __gc */
			lua_pushliteral(L, "__gc");
			lua_pushcfunction(L, priv_boxed_gc);
			lua_rawset(L, -3);

			/* __tostring */
			lua_pushliteral(L, "__tostring");
			lua_pushcfunction(L, priv_boxed_tostring);
			lua_rawset(L, -3);

			/* __eq */
			lua_pushliteral(L, "__eq");
			lua_pushcfunction(L, priv_boxed_eq);
			lua_rawset(L, -3);
		}
		/* Just for debug, the set metatable block is generic */
		#ifdef IDEBUG
		else
		{
			g_debug("Using an already created metatable!");
		}
		#endif

		/* Ensure that the new userdata is at the top */
		lua_settop(L, top);

		luaL_getmetatable(L, "lgobBoxed");
		lua_setmetatable(L, -2);

		return TRUE;
	}
	else
	{
		#ifdef IDEBUG
		g_debug("\tInvalid pointer!");
		#endif

		lua_pushnil(L);

		return FALSE;
	}
}

static gboolean priv_boxed_new_no_copy(lua_State* L, GType type, gpointer pointer)
{
	#ifdef IDEBUG
	g_debug("Creating a new Boxed(%s) (without copying)!", g_type_name(type));
	#endif

	if(pointer)
	{
		Boxed* b = lua_newuserdata(L, sizeof(Boxed));
		b->pointer = pointer;
		b->type = type;
		int top = lua_gettop(L);

		/* Set the metatable */
		if(luaL_newmetatable(L, "lgobBoxedNoCopy"))
		{
			/* A new metatable */
			#ifdef IDEBUG
			g_debug("Creating a new metatable!");
			#endif

			/* __tostring */
			lua_pushliteral(L, "__tostring");
			lua_pushcfunction(L, priv_boxed_tostring);
			lua_rawset(L, -3);

			/* __eq */
			lua_pushliteral(L, "__eq");
			lua_pushcfunction(L, priv_boxed_eq);
			lua_rawset(L, -3);
		}
		/* Just for debug, the set metatable block is generic */
		#ifdef IDEBUG
		else
		{
			g_debug("Using an already created metatable!");
		}
		#endif

		/* Ensure that the new userdata is at the top */
		lua_settop(L, top);

		luaL_getmetatable(L, "lgobBoxedNoCopy");
		lua_setmetatable(L, -2);

		return TRUE;
	}
	else
	{
		#ifdef IDEBUG
		g_debug("\tInvalid pointer!");
		#endif

		lua_pushnil(L);

		return FALSE;
	}
}
