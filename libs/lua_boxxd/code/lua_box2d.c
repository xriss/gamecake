/*

 Copyright (C) 2026 Kriss Blank < Kriss@XIXs.com >
 This file is distributed under the terms of the MIT license.
 http://en.wikipedia.org/wiki/MIT_License

*/
#include "all.h"
#include "box2d/box2d.h"

/*

we can use either this string as a string identifier
or its address as a light userdata identifier, both unique

*/
const char *lua_b2_space_meta_name="box2d_space*ptr";
const char *lua_b2_body_meta_name ="box2d_body*ptr";
const char *lua_b2_shape_meta_name="box2d_shape*ptr";
const char *lua_b2_constraint_meta_name="box2d_constraint*ptr";


/*

Get library version as 3 numbers

*/
static int lua_b2_version (lua_State *l)
{
b2Version v=b2GetVersion();

	lua_pushnumber(l,v.major);
	lua_pushnumber(l,v.minor);
	lua_pushnumber(l,v.revision);

	return 3;
}

/*

open library.

*/
LUALIB_API int luaopen_box2d_core (lua_State *l)
{
	const luaL_Reg lib[] =
	{
		{"version",					lua_b2_version},
		{0,0}
	};


	lua_newtable(l);
	luaL_openlib(l, NULL, lib, 0);
	return 1;
}

