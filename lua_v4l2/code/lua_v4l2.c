/*
-- Copyright (C) 2012 Kriss Blank < Kriss@XIXs.com >
-- This file is distributed under the terms of the MIT license.
-- http://en.wikipedia.org/wiki/MIT_License
-- Please ping me if you use it for anything cool...
*/

#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"


#include "code/lua_v4l2.h"

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// convert a userdata to a string
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_v4l2_test (lua_State *l)
{

	lua_pushstring(l,"test OK");
	return 1;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// open library.
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
LUALIB_API int luaopen_wetgenes_v4l2_core (lua_State *l)
{
	const luaL_reg lib[] =
	{
		{"test",			lua_v4l2_test},

		{0,0}
	};
		
	lua_newtable(l);
	luaL_openlib(l, NULL, lib, 0);
	return 1;
}

