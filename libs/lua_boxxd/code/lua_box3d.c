/*

 Copyright (C) 2026 Kriss Blank < Kriss@XIXs.com >
 This file is distributed under the terms of the MIT license.
 http://en.wikipedia.org/wiki/MIT_License

*/
#include "all.h"
#include "box3d/box3d.h"

/*

we can use either this string as a string identifier
or its address as a light userdata identifier, both unique

*/
const char *lua_b3_space_meta_name="box3d_space*ptr";
const char *lua_b3_body_meta_name ="box3d_body*ptr";
const char *lua_b3_shape_meta_name="box3d_shape*ptr";
const char *lua_b3_constraint_meta_name="box3d_constraint*ptr";


/*

Get library version as 3 numbers

*/
static int lua_b3_version (lua_State *l)
{
b3Version v=b3GetVersion();

	lua_pushnumber(l,v.major);
	lua_pushnumber(l,v.minor);
	lua_pushnumber(l,v.revision);

	return 3;
}

/*

open library.

*/
LUALIB_API int luaopen_box3d_core (lua_State *l)
{
	const luaL_Reg lib[] =
	{
		{"version",					lua_b3_version},
		{0,0}
	};


	lua_newtable(l);
	luaL_openlib(l, NULL, lib, 0);
	return 1;
}

