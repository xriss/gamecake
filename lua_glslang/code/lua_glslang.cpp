/*

(C) Kriss@XIXs.com 2017 and released under the https://opensource.org/licenses/MIT license.

*/
#include "all.h"


/*

	lua_glslang_lint_gles2(lua)

	inputs
		vertex code string
		fragment code string

	return
		vertex error string
		fragment error string

Compile a vertex shader and a fragment shader for GLES2, return nil,nil 
for no errors or an error string for either phase if something went 
wrong.

*/
static int lua_glslang_lint_gles2(lua_State *l)
{
	ShHandle compilers[2];
	TBuiltInResource Resources;

	int count=0;
	int ret;
	const char *s;

	const char *vstr=lua_tostring(l,1);
	const char *fstr=lua_tostring(l,2);

	ShInitialize();

	compilers[0]=ShConstructCompiler(EShLangVertex,0);
	compilers[1]=ShConstructCompiler(EShLangFragment,0);

	if(vstr)
	{
		ret = ShCompile(compilers[0], &vstr, 1, nullptr, EShOptNone, &Resources, 0, 100 , true, EShMsgDefault);
		s=ShGetInfoLog(compilers[0]);
		if(ret)	{	lua_pushnil(l);			}
		else	{	lua_pushstring(l,s);	}
	}
	else	{	lua_pushnil(l);			}

	if(fstr)
	{
		ret = ShCompile(compilers[1], &fstr, 1, nullptr, EShOptNone, &Resources, 0, 100 , true, EShMsgDefault);
		s=ShGetInfoLog(compilers[1]);
		if(ret)	{	lua_pushnil(l);			}
		else	{	lua_pushstring(l,s);	}
	}
	else	{	lua_pushnil(l);			}

	ShDestruct(compilers[0]);
	ShDestruct(compilers[1]);
	
	ShFinalize();

	return 2;
}


/*

Open the lua library.

*/
extern "C" int luaopen_glslang_core (lua_State *l)
{
	const luaL_reg lib[] =
	{
		{"lint_gles2",	lua_glslang_lint_gles2},

		{0,0}
	};

	lua_newtable(l);
	luaL_openlib(l, NULL, lib, 0);
	return 1;
}
