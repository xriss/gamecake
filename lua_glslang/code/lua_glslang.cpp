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
	ShHandle linker;
	TBuiltInResource Resources;

	int count=0;
	int ret;
	const char *s;

	const char *vstr=lua_tostring(l,1);
	const char *fstr=lua_tostring(l,2);
	const char *cstr=lua_tostring(l,3);

	compilers[0]=ShConstructCompiler(EShLangVertex,0);
	compilers[1]=ShConstructCompiler(EShLangFragment,0);
	linker=ShConstructLinker(EShExVertexFragment,0);

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

	if(cstr)
	{
		ret = ShLinkExt(linker, compilers, 2);
		s=ShGetInfoLog(linker);					// I think we always get an error here?
		if(ret)	{	lua_pushnil(l);			}
		else	{	lua_pushstring(l,s);	}  // but nothing in the log
	}
	else	{	lua_pushnil(l);			}


	ShDestruct(linker);
	ShDestruct(compilers[1]);
	ShDestruct(compilers[0]);
	
	return 3;
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

	ShInitialize();

	lua_newtable(l);
	luaL_openlib(l, NULL, lib, 0);
	return 1;
}

