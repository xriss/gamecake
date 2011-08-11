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

static int lgob_gstrv2table(lua_State* L)
{
	gsize length = G_MAXSIZE;
	
	if(lua_gettop(L) == 2)
		length = lua_tointeger(L, 2);
	
	gchar* value = NULL;
	GStrv strv = lua_touserdata(L, 1);
	int i;

	lua_newtable(L);

	for(i = 0; i < length; ++i)
	{
		value = strv[i];

		#ifdef IDEBUG
		g_debug("Iterating: i == %i and value == %s", i, value);
		#endif

		if(value == NULL) break;
		
		/* Set it */
		lua_pushstring(L, value);
		lua_rawseti(L, -2, i + 1);
	}
	
	#ifdef IDEBUG
	g_debug("Returning a GStrv with %i elements!", i);
	#endif
	
	return 1;
}

static int lgob_table2gstrv(lua_State* L)
{
	int i = 0;
	int table_size = lua_objlen(L, 1);

	/* Allocate the array, for table_size elements + 1 NULL */
	GStrv array = g_malloc(sizeof(gchar*) * (table_size + 1));

	/*
	 * Transport the values to a array
	 */
	for (;;)
	{
		lua_rawgeti(L, 1, ++i);

		#ifdef IDEBUG
		g_debug("Iterating: i == %i, type == %s", i, lua_typename(L, lua_type(L, -1)));
		#endif

		if(lua_isnil(L, -1)) break;

		gchar* tmp = g_strdup(lua_tostring(L, -1));
		array[i - 1] = tmp;
      	lua_pop(L, 1);
    }
    lua_pop(L, 1);

    /* Set the guard */
    array[i - 1] = NULL;

	lua_pushlightuserdata(L, array);
	return 1;
}

static int lgob_gslist2table(lua_State* L)
{
	int i = 0;
	GSList* list = lua_touserdata(L, 1);
	Type type = lua_tointeger(L, 2); 

	lua_newtable(L);

	while(list)
	{
		switch(type)
		{
			case TYPE_STRING: lua_pushstring(L, (const gchar*)list->data); break;
			case TYPE_DOUBLE: lua_pushnumber(L, *(double*)list->data); break;
			case TYPE_OBJECT: priv_object_new(L, list->data, FALSE); break;
			case TYPE_CUSTOM: /* Passed special handler, that must be on the top */
			{
				lua_pushvalue(L, -2); /* Copy the function, to reuse it */
				lua_pushlightuserdata(L, list->data);				
				lua_call(L, 1, 1); /* This leaves the desired value on the top */
				break;
			}
			default: priv_generic_struct_new(L, list->data, FALSE); break;
		}

		lua_rawseti(L, -2, ++i);
		list = list->next;
	}

	#ifdef IDEBUG
	g_debug("Returning a table with %i elements!", i);
	#endif
	
	return 1;
}

static int lgob_glist2table(lua_State* L)
{
	int i = 0;
	GList* list = lua_touserdata(L, 1);
	Type type = lua_tointeger(L, 2);

	lua_newtable(L);

	while(list)
	{
		switch(type)
		{
			case TYPE_STRING: lua_pushstring(L, (const gchar*)list->data); break;
			case TYPE_DOUBLE: lua_pushnumber(L, *(double*)list->data); break;
			case TYPE_OBJECT: priv_object_new(L, list->data, FALSE); break;
			case TYPE_CUSTOM: /* Passed special handler, that must be on the top */
			{
				lua_pushvalue(L, -2); /* Copy the function, to reuse it */
				lua_pushlightuserdata(L, list->data);				
				lua_call(L, 1, 1); /* This leaves the desired value on the top */
				break;
			}
			
			default: priv_generic_struct_new(L, list->data, FALSE); break;
		}		

		lua_rawseti(L, -2, ++i);
		list = list->next;
	}

	#ifdef IDEBUG
	g_debug("Returning a table with %i elements!", i);
	#endif
	
	return 1;
}
