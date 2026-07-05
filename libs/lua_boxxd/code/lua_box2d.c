/*

 Copyright (C) 2026 Kriss Blank < Kriss@XIXs.com >
 This file is distributed under the terms of the MIT license.
 http://en.wikipedia.org/wiki/MIT_License

*/
#include "all.h"


/*

we can use either this string as a string identifier
or its address as a light userdata identifier, both unique

*/
const char *lua_box2d_space_meta_name="box2d_space*ptr";
const char *lua_box2d_body_meta_name ="box2d_body*ptr";
const char *lua_box2d_shape_meta_name="box2d_shape*ptr";
const char *lua_box2d_constraint_meta_name="box2d_constraint*ptr";


/*+---------------------------------------------------------------------

open library.

*/
LUALIB_API int luaopen_box2d_core (lua_State *l)
{
	const luaL_Reg lib[] =
	{
		{0,0}
	};


	lua_newtable(l);
	luaL_openlib(l, NULL, lib, 0);
	return 1;
}

