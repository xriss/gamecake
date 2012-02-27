
#include "lua.h"
#include "lauxlib.h"

#include <GLES/gl.h>



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
		case GL_VENDOR:
		case GL_RENDERER:
		case GL_VERSION:
		case GL_EXTENSIONS:

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
// enabler
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_gles_Enable (lua_State *l)
{
	glEnable(		(int)lua_tonumber(l,1)	);
	return 0;
}
static int lua_gles_Disable (lua_State *l)
{
	glDisable(		(int)lua_tonumber(l,1)	);
	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// Clear
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_gles_ClearColor (lua_State *l)
{
	glClearColor(	(float)lua_tonumber(l,1)	,
					(float)lua_tonumber(l,2)	,
					(float)lua_tonumber(l,3)	,
					(float)lua_tonumber(l,4)	);
	return 0;
}

static int lua_gles_ClearDepth (lua_State *l)
{
	glClearDepthf(	(float)lua_tonumber(l,1)	);
	return 0;
}

static int lua_gles_Clear (lua_State *l)
{
	glClear(		(int)lua_tonumber(l,1)	);
	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// Matrix
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_gles_MatrixMode (lua_State *l)
{
	glMatrixMode(	(int)lua_tonumber(l,1)	);
	return 0;
}

static int lua_gles_LoadMatrix (lua_State *l)
{
int i;
float ff[16];
	for(i=0;i<16;i++)
	{
		lua_pushnumber(l,i+1);
		lua_gettable(l,1);
		ff[i]=(float)lua_tonumber(l,-1);
		lua_pop(l,1);
	}

	glLoadMatrixf(ff);
	return 0;
}

static int lua_gles_LoadIdentity (lua_State *l)
{
	glLoadIdentity();
	return 0;
}

static int lua_gles_Translate (lua_State *l)
{
	glTranslatef(	(float)lua_tonumber(l,1)	,
					(float)lua_tonumber(l,2)	,
					(float)lua_tonumber(l,3)	);
	return 0;
}
static int lua_gles_Rotate (lua_State *l)
{
	glRotatef(		(float)lua_tonumber(l,1)	,
					(float)lua_tonumber(l,2)	,
					(float)lua_tonumber(l,3)	,
					(float)lua_tonumber(l,4)	);
	return 0;
}
static int lua_gles_Scale (lua_State *l)
{
	glScalef(		(float)lua_tonumber(l,1)	,
					(float)lua_tonumber(l,2)	,
					(float)lua_tonumber(l,3)	);
	return 0;
}


static int lua_gles_PushMatrix (lua_State *l)
{
	glPushMatrix();
	return 0;
}

static int lua_gles_PopMatrix (lua_State *l)
{
	glPopMatrix();
	return 0;
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

		{"Enable",				lua_gles_Enable},
		{"Disable",				lua_gles_Disable},

		{"ClearColor",			lua_gles_ClearColor},
		{"ClearDepth",			lua_gles_ClearDepth},
		{"Clear",				lua_gles_Clear},

		{"MatrixMode",			lua_gles_MatrixMode},
		{"LoadMatrix",			lua_gles_LoadMatrix},
		{"LoadIdentity",		lua_gles_LoadIdentity},
		{"Translate",			lua_gles_Translate},
		{"Rotate",				lua_gles_Rotate},
		{"Scale",				lua_gles_Scale},
		{"PushMatrix",			lua_gles_PushMatrix},
		{"PopMatrix",			lua_gles_PopMatrix},
		
		{0,0}
	};

	lua_newtable(l);
	luaL_openlib(l, NULL, lib, 0);
	
	return 1;
}

