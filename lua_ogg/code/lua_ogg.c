/*
-- Copyright (C) 2012 Kriss Blank < Kriss@XIXs.com >
-- This file is distributed under the terms of the MIT license.
-- http://en.wikipedia.org/wiki/MIT_License
-- Please ping me if you use it for anything cool...
*/

#include "all.h"


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// test
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_ogg_test (lua_State *l)
{
	printf("test ogg");

	return 0;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// open library.
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
LUALIB_API int luaopen_wetgenes_ogg_core (lua_State *l)
{
	const luaL_reg lib[] =
	{
		{"test",			lua_ogg_test},

		{0,0}
	};
		
	lua_newtable(l);
	luaL_openlib(l, NULL, lib, 0);
	return 1;
}

