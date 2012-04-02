
#include "lua.h"
#include "lauxlib.h"

#include INCLUDE_GLES_GL


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
// Matrix (gles1)
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
#if defined(LUA_GLES_GLES) || defined(LUA_GLES_GL)

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

static int lua_gles_MultMatrix (lua_State *l)
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

	glMultMatrixf(ff);
	return 0;
}

static int lua_gles_Frustum (lua_State *l)
{
#if defined(LUA_GLES_GL)

	glFrustum(		lua_tonumber(l,1)	,
					lua_tonumber(l,2)	,
					lua_tonumber(l,3)	,
					lua_tonumber(l,4)	,
					lua_tonumber(l,5)	,
					lua_tonumber(l,6)	);

#else

	glFrustumf(		(float)lua_tonumber(l,1)	,
					(float)lua_tonumber(l,2)	,
					(float)lua_tonumber(l,3)	,
					(float)lua_tonumber(l,4)	,
					(float)lua_tonumber(l,5)	,
					(float)lua_tonumber(l,6)	);
					
#endif

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
#endif

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// textures
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_gles_GenTexture (lua_State *l)
{
int id;
	glGenTextures(1,&id);
	lua_pushnumber(l,id);
	return 1;
}

static int lua_gles_BindTexture (lua_State *l)
{
	glBindTexture((int)lua_tonumber(l,1),(int)lua_tonumber(l,2));
	return 0;
}

static int lua_gles_DeleteTexture (lua_State *l)
{
int id=(int)lua_tonumber(l,1);
	glDeleteTextures(1,&id);
	return 0;
}

static int lua_gles_TexImage2D (lua_State *l)
{
	glTexImage2D(	(int)lua_tonumber(l,1)		,
					(int)lua_tonumber(l,2)		,
					(int)lua_tonumber(l,3)		,
					(int)lua_tonumber(l,4)		,
					(int)lua_tonumber(l,5)		,
					(int)lua_tonumber(l,6)		,
					(int)lua_tonumber(l,7)		,
					(int)lua_tonumber(l,8)		,
					(void*)lua_touserdata(l,9)	);
	return 0;
}

static int lua_gles_TexParameter (lua_State *l)
{
	glTexParameteri(	(int)lua_tonumber(l,1)		,
						(int)lua_tonumber(l,2)		,
						(int)lua_tonumber(l,3)		);
	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// how to draw
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_gles_BlendFunc (lua_State *l)
{
	glBlendFunc(	(int)lua_tonumber(l,1)		,
					(int)lua_tonumber(l,2)		);
	return 0;
}


#if defined(LUA_GLES_GLES) || defined(LUA_GLES_GL)

static int lua_gles_Color (lua_State *l)
{
	glColor4f(		(float)lua_tonumber(l,1)	,
					(float)lua_tonumber(l,2)	,
					(float)lua_tonumber(l,3)	,
					(float)lua_tonumber(l,4)	);
	return 0;
}

static int lua_gles_ShadeModel (lua_State *l)
{
	glShadeModel(	(int)lua_tonumber(l,1)		);
	return 0;
}

static int lua_gles_EnableClientState (lua_State *l)
{
	glEnableClientState(	(int)lua_tonumber(l,1)	);
	return 0;
}

static int lua_gles_DisableClientState (lua_State *l)
{
	glDisableClientState(	(int)lua_tonumber(l,1)	);
	return 0;
}

#endif

static int lua_gles_Viewport (lua_State *l)
{
	glViewport(			(int)lua_tonumber(l,1)	,
						(int)lua_tonumber(l,2)	,
						(int)lua_tonumber(l,3)	,
						(int)lua_tonumber(l,4)	);
	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// what to draw (gles1)
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
#if defined(LUA_GLES_GLES) || defined(LUA_GLES_GL)

static int lua_gles_ColorPointer (lua_State *l)
{
	glColorPointer(		(int)lua_tonumber(l,1)		,
						(int)lua_tonumber(l,2)		,
						(int)lua_tonumber(l,3)		,
						((char*)lua_touserdata(l,4))+((int)lua_tonumber(l,5))	);
	return 0;
}

static int lua_gles_TexCoordPointer (lua_State *l)
{
	glTexCoordPointer(	(int)lua_tonumber(l,1)		,
						(int)lua_tonumber(l,2)		,
						(int)lua_tonumber(l,3)		,
						((char*)lua_touserdata(l,4))+((int)lua_tonumber(l,5))	);
	return 0;
}

static int lua_gles_NormalPointer (lua_State *l)
{
	glNormalPointer(	(int)lua_tonumber(l,1)		,
						(int)lua_tonumber(l,2)		,
						((char*)lua_touserdata(l,3))+((int)lua_tonumber(l,4))	);
	return 0;
}

static int lua_gles_VertexPointer (lua_State *l)
{
	glVertexPointer(	(int)lua_tonumber(l,1)		,
						(int)lua_tonumber(l,2)		,
						(int)lua_tonumber(l,3)		,
						((char*)lua_touserdata(l,4))+((int)lua_tonumber(l,5))	);
	return 0;
}
#endif

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// when to draw
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_gles_DrawArrays (lua_State *l)
{
	glDrawArrays(		(int)lua_tonumber(l,1)		,
						(int)lua_tonumber(l,2)		,
						(int)lua_tonumber(l,3)		);
	return 0;
}

static int lua_gles_DrawElements (lua_State *l)
{
	glDrawElements(		(int)lua_tonumber(l,1)		,
						(int)lua_tonumber(l,2)		,
						(int)lua_tonumber(l,3)		,
						(void*)lua_touserdata(l,4)	);
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

		{"GenTexture",			lua_gles_GenTexture},
		{"BindTexture",			lua_gles_BindTexture},
		{"DeleteTexture",		lua_gles_DeleteTexture},
		{"TexImage2D",			lua_gles_TexImage2D},
		{"TexParameter",		lua_gles_TexParameter},

#if defined(LUA_GLES_GLES) || defined(LUA_GLES_GL)
		{"Color",				lua_gles_Color},
		{"EnableClientState",	lua_gles_EnableClientState},
		{"DisableClientState",	lua_gles_DisableClientState},
		{"ShadeModel",			lua_gles_ShadeModel},		
#endif

		{"BlendFunc",			lua_gles_BlendFunc},		
		{"Viewport",			lua_gles_Viewport},

		
		{"DrawArrays",			lua_gles_DrawArrays},
		{"DrawElements",		lua_gles_DrawElements},

#if defined(LUA_GLES_GLES) || defined(LUA_GLES_GL)
		{"MatrixMode",			lua_gles_MatrixMode},
		{"LoadMatrix",			lua_gles_LoadMatrix},
		{"MultMatrix",			lua_gles_MultMatrix},
		{"LoadIdentity",		lua_gles_LoadIdentity},
		{"Translate",			lua_gles_Translate},
		{"Rotate",				lua_gles_Rotate},
		{"Scale",				lua_gles_Scale},
		{"PushMatrix",			lua_gles_PushMatrix},
		{"PopMatrix",			lua_gles_PopMatrix},
		{"Frustum",				lua_gles_Frustum},
		
		{"ColorPointer",		lua_gles_ColorPointer},
		{"TexCoordPointer",		lua_gles_TexCoordPointer},
		{"NormalPointer",		lua_gles_NormalPointer},
		{"VertexPointer",		lua_gles_VertexPointer},
#endif
		
		{0,0}
	};

	lua_newtable(l);
	luaL_openlib(l, NULL, lib, 0);
	
	return 1;
}

