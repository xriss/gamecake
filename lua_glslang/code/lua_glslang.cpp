/*

(C) Kriss@XIXs.com 2017 and released under the https://opensource.org/licenses/MIT license.

*/
#include "all.h"




/*

Test...

*/
static int lua_glslang_test(lua_State *l)
{

//
// Read a file's data into a string, and compile it using the old interface ShCompile,
// for non-linkable results.
//

	const char* fileName;
	ShHandle compiler;
	TBuiltInResource Resources;

    int ret = 0;
    char* shaderString;

    EShMessages messages = EShMsgDefault;
//    SetMessageOptions(messages);

	ret = ShCompile(compiler, &shaderString, 1, nullptr, EShOptNone, &Resources, 0, 110 , false, messages);

	return 0;
}


/*

Open the lua library.

*/
extern "C" int luaopen_glslang_core (lua_State *l)
{
	const luaL_reg lib[] =
	{
		{"test",	lua_glslang_test},

		{0,0}
	};

	const luaL_reg meta[] =
	{
//		{"__gc",			lua_grd_destroy},

		{0,0}
	};

//	luaL_newmetatable(l, lua_grd_ptr_name);
//	luaL_openlib(l, NULL, meta, 0);
//	lua_pop(l,1);

	lua_newtable(l);
	luaL_openlib(l, NULL, lib, 0);
	return 1;
}

