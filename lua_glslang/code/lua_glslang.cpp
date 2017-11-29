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
	ShHandle compilers[2];
	ShHandle linker;
	TBuiltInResource Resources;

	int ret = 0;
	const char* shaderString;

	shaderString=lua_tostring(l,1);

	ShInitialize();

	compilers[0]=ShConstructCompiler(EShLangVertex,0);
	compilers[1]=ShConstructCompiler(EShLangFragment,0);
	linker=ShConstructLinker(EShExVertexFragment,0);


	ret = ShCompile(compilers[0], &shaderString, 1, nullptr, EShOptNone, &Resources, 0, 100 , true, EShMsgDefault);
	lua_pushstring(l,ShGetInfoLog(compilers[0]));

	ret = ShCompile(compilers[1], &shaderString, 1, nullptr, EShOptNone, &Resources, 0, 100 , true, EShMsgDefault);
	lua_pushstring(l,ShGetInfoLog(compilers[1]));

	ret = ShLinkExt(linker, compilers, 2);
	lua_pushstring(l,ShGetInfoLog(linker));

	ShDestruct(compilers[0]);
	ShDestruct(compilers[1]);
	ShDestruct(linker);
	
	ShFinalize();

	return 3;
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

