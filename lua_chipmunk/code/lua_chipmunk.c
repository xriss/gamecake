/*
-- Copyright (C) 2016 Kriss Blank < Kriss@XIXs.com >
-- This file is distributed under the terms of the MIT license.
-- http://en.wikipedia.org/wiki/MIT_License
-- Please ping me if you use it for anything cool...
*/
#include "all.h"

//
// we can use either this string as a string identifier
// or its address as a light userdata identifier, both unique
//
const char *lua_chipmunk_meta_name="chipmunk*ptr";



/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// open library.
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_chipmunk_test (lua_State *l)
{	
	return 0;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// open library.
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
LUALIB_API int luaopen_wetgenes_sod_core (lua_State *l)
{
	const luaL_reg lib[] =
	{
		{"test",			lua_chipmunk_test,
		
		{0,0}
	};

/*
	const luaL_reg meta_chipmunk[] =
	{
		{"__gc",			lua_chipmunk_destroy},
		{0,0}
	};


	luaL_newmetatable(l, lua_chipmunk_meta_name);
	luaL_openlib(l, NULL, meta_chipmunk, 0);
	lua_pop(l,1);
*/
		
	lua_newtable(l);
	luaL_openlib(l, NULL, lib, 0);
	return 1;
}

