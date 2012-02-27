
#include "lua.h"
#include "lauxlib.h"

#include <GL/gl.h>


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// turn a def into a property type and size
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void lua_gles_get_prop_info (int def, char *v, int *num)
{
	switch(def)
	{
//		case GL_:
// int[1]
			*num=1; *v='i';
		break;
// float[1]
			*num=1; *v='f';
		break;
// float[3]
			*num=3; *v='f';
		break;
// float[6]
			*num=6; *v='f';
		break;
// const char *
			*num=1; *v='s';
		break;
	}
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// get error number
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_gles_GetError (lua_State *l)
{
	lua_pushnumber(l,glGetError());
	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// get base properties
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_gles_Get (lua_State *l)
{	
int def;

// default of a single integer
char v='i';
int num=1;

int iv[16];
float fv[16];

int i;

	def=luaL_checknumber(l,1);
	
	lua_gles_get_prop_info(def, &v, &num);
	
	switch(v)
	{
		case 'i':
			glGetIntegerv(def,iv);
			for(i=0;i<num;i++)
			{
				lua_pushnumber(l,iv[i]);
			}
		break;
		case 'f':
			glGetFloatv(def,fv);
			for(i=0;i<num;i++)
			{
				lua_pushnumber(l,fv[i]);
			}
		break;
		case 's':
			for(i=0;i<num;i++) // this should only ever be 1
			{
				lua_pushstring(l, glGetString(def) );
			}
		break;
	}
	
	return num;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// open library.
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
LUALIB_API int luaopen_gles_core(lua_State *l)
{
	const luaL_reg lib[] =
	{
		{"Get",					lua_gles_Get},
		{"GetError",			lua_gles_GetError},
		
		{0,0}
	};

	lua_newtable(l);
	luaL_openlib(l, NULL, lib, 0);
	
	return 1;
}

