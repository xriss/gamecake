/*
-- Copyright (C) 2013 Kriss Blank < Kriss@XIXs.com >
-- This file is distributed under the terms of the MIT license.
-- http://en.wikipedia.org/wiki/MIT_License
-- Please ping me if you use it for anything cool...
*/
#include "all.h"

#include "hidapi.h"

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// HID library.
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
LUALIB_API int luaopen_wetgenes_hid_core (lua_State *l)
{
	const luaL_reg lib[] =
	{
//		{"test",			lua_hid_test},

		{"init",			lua_hid_init},
		{"exit",			lua_hid_exit},
	
		{0,0}
	};

	lua_newtable(l);
	luaL_openlib(l, NULL, lib, 0);
	return 1;
}

