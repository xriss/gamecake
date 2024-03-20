
#include <stdlib.h>
#include <stdint.h>

#include "lua.h"
#include "lauxlib.h"

#include "../../lua_tardis/code/lua_tardis.h"

// we suport various GL versions
#include INCLUDE_GLES_GL


// link with lua/hacks.c plz
extern unsigned char * lua_toluserdata (lua_State *L, int idx, size_t *len);

// 512 vec4 is 128 mat4
#define MAX_UNIFORM_V4_LEN 512

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// turn a string or userdata at given idx into a ptr, returns 0 if not possible
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static const unsigned char* lua_gles_topointer (lua_State *l,int idx,int *plen)
{
const unsigned char *ptr=0;
size_t len=0x7fffffff; // fake max length if we have no idea what it is (light userdata)

	if(lua_isnumber(l,idx))
	{
		ptr=(const unsigned char*)((intptr_t)lua_tonumber(l,idx));
	}
	else
	if(lua_isstring(l,idx))
	{
		ptr=(const unsigned char*)lua_tolstring(l,idx,&len);
	}
	else
	if(lua_islightuserdata(l,idx))
	{
		ptr=(const unsigned char*)lua_touserdata(l,idx);
	}
	else
	if(lua_isuserdata(l,idx)) // real user data only because we first checked for light
	{
		ptr=(const unsigned char*)lua_toluserdata(l,idx,&len);
	}
	
	if(plen) { *plen=len; } //write out len if it was asked for

	return ptr;
}
	
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
//			*num=1; *v='i';
//		break;
// float[1]
//			*num=1; *v='f';
//		break;
// float[3]
//			*num=3; *v='f';
//		break;
// float[6]
//			*num=6; *v='f';
//		break;

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
	lua_pushnumber(l,(double)glGetError());
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
				lua_pushnumber(l,(double)iv[i]);
			}
		break;
		case 'f':
			glGetFloatv(def,fv);
			for(i=0;i<num;i++)
			{
				lua_pushnumber(l,(double)fv[i]);
			}
		break;
		case 's':
			if( lua_isnumber(l,2) ) // requesting string of given index
			{
				lua_pushstring(l, (const char *)glGetStringi(def , (int)luaL_checknumber(l,2) ) );
			}
			else
			{
				lua_pushstring(l, (const char *)glGetString(def) );
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
	glEnable(		(int)luaL_checknumber(l,1)	);
	return 0;
}
static int lua_gles_Disable (lua_State *l)
{
	glDisable(		(int)luaL_checknumber(l,1)	);
	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// Clear
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_gles_ClearColor (lua_State *l)
{
	glClearColor(	(float)luaL_checknumber(l,1)	,
					(float)luaL_checknumber(l,2)	,
					(float)luaL_checknumber(l,3)	,
					(float)luaL_checknumber(l,4)	);
	return 0;
}

static int lua_gles_ClearDepth (lua_State *l)
{
	glClearDepthf(	(float)luaL_checknumber(l,1)	);
	return 0;
}

static int lua_gles_Clear (lua_State *l)
{
	glClear(		(int)luaL_checknumber(l,1)	);
	return 0;
}

static int lua_gles_Finish (lua_State *l)
{
	glFinish();
	return 0;
}
static int lua_gles_Flush (lua_State *l)
{
	glFlush();
	return 0;
}

static int lua_gles_DepthFunc (lua_State *l)
{
	glDepthFunc( (int)luaL_checknumber(l,1) );
	return 0;
}

static int lua_gles_DepthMask (lua_State *l)
{

	glDepthMask( (int)luaL_checknumber(l,1) );
	return 0;
}
static int lua_gles_DepthRange (lua_State *l)
{

#if defined(LUA_GLES_GL)
	glDepthRange( luaL_checknumber(l,1) , luaL_checknumber(l,2)  );
#else
	glDepthRangef( (float)luaL_checknumber(l,1) , (float)luaL_checknumber(l,2)  );
#endif

	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// Matrix (gles1)
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
#if defined(LUA_GLES_GLES1) //|| defined(LUA_GLES_GL)

static int lua_gles_MatrixMode (lua_State *l)
{
	glMatrixMode(	(int)luaL_checknumber(l,1)	);
	return 0;
}

static int lua_gles_LoadMatrix (lua_State *l)
{
int i;
float ff[16];
	for(i=0;i<16;i++)
	{
		lua_pushnumber(l,(double)(i+1));
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
		lua_pushnumber(l,(double)(i+1));
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

	glFrustum(		luaL_checknumber(l,1)	,
					luaL_checknumber(l,2)	,
					luaL_checknumber(l,3)	,
					luaL_checknumber(l,4)	,
					luaL_checknumber(l,5)	,
					luaL_checknumber(l,6)	);

#else

	glFrustumf(		(float)luaL_checknumber(l,1)	,
					(float)luaL_checknumber(l,2)	,
					(float)luaL_checknumber(l,3)	,
					(float)luaL_checknumber(l,4)	,
					(float)luaL_checknumber(l,5)	,
					(float)luaL_checknumber(l,6)	);
					
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
	glTranslatef(	(float)luaL_checknumber(l,1)	,
					(float)luaL_checknumber(l,2)	,
					(float)luaL_checknumber(l,3)	);
	return 0;
}
static int lua_gles_Rotate (lua_State *l)
{
	glRotatef(		(float)luaL_checknumber(l,1)	,
					(float)luaL_checknumber(l,2)	,
					(float)luaL_checknumber(l,3)	,
					(float)luaL_checknumber(l,4)	);
	return 0;
}
static int lua_gles_Scale (lua_State *l)
{
	glScalef(		(float)luaL_checknumber(l,1)	,
					(float)luaL_checknumber(l,2)	,
					(float)luaL_checknumber(l,3)	);
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
	glGenTextures(1,(unsigned int *)&id);
	lua_pushnumber(l,(double)id);
	return 1;
}

static int lua_gles_BindTexture (lua_State *l)
{
	glBindTexture((int)luaL_checknumber(l,1),(int)luaL_checknumber(l,2));
	return 0;
}

static int lua_gles_DeleteTexture (lua_State *l)
{
int id=(int)luaL_checknumber(l,1);
	glDeleteTextures(1,(const unsigned int*)&id);
	return 0;
}

static int lua_gles_TexImage2D (lua_State *l)
{		
	glTexImage2D(	(int)luaL_checknumber(l,1)		,
					(int)luaL_checknumber(l,2)		,
					(int)luaL_checknumber(l,3)		,
					(int)luaL_checknumber(l,4)		,
					(int)luaL_checknumber(l,5)		,
					(int)luaL_checknumber(l,6)		,
					(int)luaL_checknumber(l,7)		,
					(int)luaL_checknumber(l,8)		,
					(void*)lua_gles_topointer(l,9,0));
	return 0;
}

static int lua_gles_TexSubImage2D (lua_State *l)
{	
	glTexSubImage2D((int)luaL_checknumber(l,1)		,
					(int)luaL_checknumber(l,2)		,
					(int)luaL_checknumber(l,3)		,
					(int)luaL_checknumber(l,4)		,
					(int)luaL_checknumber(l,5)		,
					(int)luaL_checknumber(l,6)		,
					(int)luaL_checknumber(l,7)		,
					(int)luaL_checknumber(l,8)		,
					(void*)lua_gles_topointer(l,9,0));
	return 0;
}

static int lua_gles_TexParameter (lua_State *l)
{
	glTexParameteri(	(int)luaL_checknumber(l,1)		,
						(int)luaL_checknumber(l,2)		,
						(int)luaL_checknumber(l,3)		);
	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// how to draw
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_gles_BlendFunc (lua_State *l)
{
	glBlendFunc(	(int)luaL_checknumber(l,1)		,
					(int)luaL_checknumber(l,2)		);
	return 0;
}

static int lua_gles_Viewport (lua_State *l)
{
	glViewport(			(int)luaL_checknumber(l,1)	,
						(int)luaL_checknumber(l,2)	,
						(int)luaL_checknumber(l,3)	,
						(int)luaL_checknumber(l,4)	);
	return 0;
}

#if defined(LUA_GLES_GLES1) //|| defined(LUA_GLES_GL)

static int lua_gles_Color (lua_State *l)
{
	glColor4f(		(float)luaL_checknumber(l,1)	,
					(float)luaL_checknumber(l,2)	,
					(float)luaL_checknumber(l,3)	,
					(float)luaL_checknumber(l,4)	);
	return 0;
}

static int lua_gles_ShadeModel (lua_State *l)
{
	glShadeModel(	(int)luaL_checknumber(l,1)		);
	return 0;
}

static int lua_gles_EnableClientState (lua_State *l)
{
	glEnableClientState(	(int)luaL_checknumber(l,1)	);
	return 0;
}

static int lua_gles_DisableClientState (lua_State *l)
{
	glDisableClientState(	(int)luaL_checknumber(l,1)	);
	return 0;
}

#endif


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// what to draw (gles1) no userdata means touserdata returns 0 , which is correct for vbo use
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
#if defined(LUA_GLES_GLES1) //|| defined(LUA_GLES_GL)

static int lua_gles_ColorPointer (lua_State *l)
{
	glColorPointer(		(int)luaL_checknumber(l,1)		,
						(int)luaL_checknumber(l,2)		,
						(int)luaL_checknumber(l,3)		,
						lua_gles_topointer(l,4,0)		);
	return 0;
}

static int lua_gles_TexCoordPointer (lua_State *l)
{
	glTexCoordPointer(	(int)luaL_checknumber(l,1)		,
						(int)luaL_checknumber(l,2)		,
						(int)luaL_checknumber(l,3)		,
						lua_gles_topointer(l,4,0)		);
	return 0;
}

static int lua_gles_NormalPointer (lua_State *l)
{
	glNormalPointer(	(int)luaL_checknumber(l,1)		,
						(int)luaL_checknumber(l,2)		,
						lua_gles_topointer(l,3,0)		);
	return 0;
}

static int lua_gles_VertexPointer (lua_State *l)
{
	glVertexPointer(	(int)luaL_checknumber(l,1)		,
						(int)luaL_checknumber(l,2)		,
						(int)luaL_checknumber(l,3)		,
						lua_gles_topointer(l,4,0)		);
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
	glDrawArrays(		(int)luaL_checknumber(l,1)		,
						(int)luaL_checknumber(l,2)		,
						(int)luaL_checknumber(l,3)		);
	return 0;
}

static int lua_gles_DrawElements (lua_State *l)
{
	glDrawElements(		(int)luaL_checknumber(l,1)		,
						(int)luaL_checknumber(l,2)		,
						(int)luaL_checknumber(l,3)		,
						(void*)lua_gles_topointer(l,4,0)	);
	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// Buffer functions
//
/*+-----------------------------------------------------------------------------------------------------------------+*/

static int lua_gles_GenBuffer (lua_State *l)
{
int id;
	glGenBuffers(1,(unsigned int*)&id);
	lua_pushnumber(l,(double)id);
	return 1;
}
static int lua_gles_DeleteBuffer (lua_State *l)
{
int id=luaL_checknumber(l,1);
	glDeleteBuffers(1,(const unsigned int*)&id);
	return 0;
}
static int lua_gles_BindBuffer (lua_State *l)
{
	glBindBuffer((int)luaL_checknumber(l,1),(int)luaL_checknumber(l,2));
	return 0;
}
static int lua_gles_BufferData (lua_State *l)
{
	glBufferData((int)luaL_checknumber(l,1),(int)luaL_checknumber(l,2),
						((char*)lua_gles_topointer(l,3,0)),
						(int)luaL_checknumber(l,4));
	return 0;
}
static int lua_gles_BufferSubData (lua_State *l)
{
	glBufferSubData((int)luaL_checknumber(l,1),(int)luaL_checknumber(l,2),(int)luaL_checknumber(l,3),
						lua_gles_topointer(l,4,0) );
	return 0;
}



/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// Stubs2
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
#if defined(LUA_GLES_GLES2) || defined(LUA_GLES_GLES3) || defined(LUA_GLES_GL)

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// Shaders
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_gles_CreateShader (lua_State *l)
{
int i,r;
	i=luaL_checknumber(l,1);
	r=glCreateShader(i);
	lua_pushnumber(l,(double)r);
	return 1;
}
static int lua_gles_DeleteShader (lua_State *l)
{
int i;
	i=luaL_checknumber(l,1);
	glDeleteShader(i);
	return 0;
}
static int lua_gles_ShaderSource (lua_State *l)
{
int i;
const char *s;
const char *ss[1];

	i=luaL_checknumber(l,1);
	s=luaL_checkstring(l,2);
	ss[0]=s;
	
	glShaderSource(i,1,ss,0);
	return 0;
}
static int lua_gles_CompileShader (lua_State *l)
{
int i;
	i=luaL_checknumber(l,1);
	glCompileShader(i);
	return 0;
}
static int lua_gles_GetShader (lua_State *l)
{
	int i,v;
	int ii[16]; // use a safe size
	
	i=luaL_checknumber(l,1);
	v=luaL_checknumber(l,2);

	glGetShaderiv(i,v,ii);
	lua_pushnumber(l,(double)ii[0]);
	
	return 1;
}
static int lua_gles_GetShaderInfoLog (lua_State *l)
{
#if !defined(GL_INFO_LOG_LENGTH)
	return 0;
#else
int i;
int size=0;
int newsize=0;
char *p=0;
	i=luaL_checknumber(l,1);

	glGetShaderiv(i, GL_INFO_LOG_LENGTH, &size);
	if(size==0) { size=16384; } // pick a fuckoff buffer size when driver is retarted
	p=malloc(size);
	if(p==0) { lua_pushfstring(l,"malloc failed (%d)",size); lua_error(l); return 0; }
	
	glGetShaderInfoLog(i,size,&newsize,p);
	lua_pushstring(l,p);
	free(p);
	
	return 1;
#endif
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// Programs
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_gles_CreateProgram (lua_State *l)
{
int i=0;
	i=glCreateProgram();
	lua_pushnumber(l,(double)i);
	return 1;
}
static int lua_gles_DeleteProgram (lua_State *l)
{
int i=0;
	i=luaL_checknumber(l,1);
	glDeleteProgram(i);
	return 0;
}

static int lua_gles_AttachShader (lua_State *l)
{
int p,s;
	p=luaL_checknumber(l,1);
	s=luaL_checknumber(l,2);
	glAttachShader(p,s);
	return 0;
}

static int lua_gles_LinkProgram (lua_State *l)
{
int i=0;
	i=luaL_checknumber(l,1);
	glLinkProgram(i);
	return 0;
}

static int lua_gles_GetProgramInfoLog (lua_State *l)
{
#if !defined(GL_INFO_LOG_LENGTH)
	return 0;
#else
int i;
int size=0;
char *p=0;
	i=luaL_checknumber(l,1);

	glGetProgramiv(i, GL_INFO_LOG_LENGTH, &size);
	if(size==0) { return 0; }
	p=malloc(size);
	if(p==0) { lua_pushfstring(l,"malloc failed (%d)",size); lua_error(l); return 0; }
	
	glGetProgramInfoLog(i,size,0,p);
	lua_pushstring(l,p);
	free(p);
	
	return 1;
#endif
}

static int lua_gles_GetProgram (lua_State *l)
{
int i,v;
int ii[16]; // use a safe size
	
	i=luaL_checknumber(l,1);
	v=luaL_checknumber(l,2);

	glGetProgramiv(i,v,ii);
	lua_pushnumber(l,(double)ii[0]);
	
	return 1;
}
static int lua_gles_UseProgram (lua_State *l)
{
int i;
	i=luaL_checknumber(l,1);
	glUseProgram(i);
	return 0;
}
static int lua_gles_GetAttribLocation (lua_State *l)
{
int i=0;
int p=0;
const char *s=0;
	p=luaL_checknumber(l,1);
	s=luaL_checkstring(l,2);
	i=glGetAttribLocation(p,s);
	lua_pushnumber(l,(double)i);
	return 1;
}
static int lua_gles_GetUniformLocation (lua_State *l)
{
int i=0;
int p=0;
const char *s=0;
	p=luaL_checknumber(l,1);
	s=luaL_checkstring(l,2);
	i=glGetUniformLocation(p,s);
	lua_pushnumber(l,(double)i);
	return 1;
}

static int lua_gles_EnableVertexAttribArray (lua_State *l)
{
int i;
	i=luaL_checknumber(l,1);
	glEnableVertexAttribArray(i);
	return 0;
}
static int lua_gles_DisableVertexAttribArray (lua_State *l)
{
int i;
	i=luaL_checknumber(l,1);
	glDisableVertexAttribArray(i);
	return 0;
}

static int lua_gles_ValidateProgram (lua_State *l)
{
int i;
	i=luaL_checknumber(l,1);
	glValidateProgram(i);
	return 0;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// data for programs
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_gles_VertexAttribPointer (lua_State *l)
{
int index;
int size;
int type;
int normalized;
int stride;
const void *pointer;
	index=luaL_checknumber(l,1);
	size=luaL_checknumber(l,2);
	type=luaL_checknumber(l,3);
	normalized=luaL_checknumber(l,4);
	stride=luaL_checknumber(l,5);
	pointer=lua_gles_topointer(l,6,0);

	glVertexAttribPointer(index,size,type,normalized,stride,pointer);
	return 0;
}

static int lua_gles_VertexAttrib1f (lua_State *l)
{
int i;
float *fp;
float ff[1];
	i=(int)luaL_checknumber(l,1);
	fp=(float*)lua_tardis_uda(l,2);
	if(!fp)
	{
		fp=ff;
		ff[0]=(float)luaL_checknumber(l,2);
	}
	glVertexAttrib1fv(i,fp);
	return 0;
}
static int lua_gles_VertexAttrib2f (lua_State *l)
{
int i;
float *fp;
float ff[2];
	i=(int)luaL_checknumber(l,1);
	fp=(float*)lua_tardis_uda(l,2);
	if(!fp)
	{
		fp=ff;
		ff[0]=(float)luaL_checknumber(l,2);
		ff[1]=(float)luaL_checknumber(l,3);
	}
	glVertexAttrib2fv(i,fp);
	return 0;
}
static int lua_gles_VertexAttrib3f (lua_State *l)
{
int i;
float *fp;
float ff[3];
	i=(int)luaL_checknumber(l,1);
	fp=(float*)lua_tardis_uda(l,2);
	if(!fp)
	{
		fp=ff;
		ff[0]=(float)luaL_checknumber(l,2);
		ff[1]=(float)luaL_checknumber(l,3);
		ff[2]=(float)luaL_checknumber(l,4);
	}
	glVertexAttrib3fv(i,fp);
	return 0;
}
static int lua_gles_VertexAttrib4f (lua_State *l)
{
int i;
float *fp;
float ff[4];
	i=(int)luaL_checknumber(l,1);
	fp=(float*)lua_tardis_uda(l,2);
	if(!fp)
	{
		fp=ff;
		ff[0]=(float)luaL_checknumber(l,2);
		ff[1]=(float)luaL_checknumber(l,3);
		ff[2]=(float)luaL_checknumber(l,4);
		ff[3]=(float)luaL_checknumber(l,5);
	}
	glVertexAttrib4fv(i,fp);
	return 0;
}



static int lua_gles_Uniform1i (lua_State *l)
{
int idx;
int *ip;
int ii[MAX_UNIFORM_V4_LEN*1];
int len=1;
int i;
	idx=(int)luaL_checknumber(l,1);
	ip=(int*)lua_tardis_uda(l,2);
	if(ip)
	{
		len=lua_tardis_uda_count(l,2)/1;
	}
	else
	if( lua_istable(l,2))
	{
		ip=ii;
		len=lua_objlen(l,2)/1;
		if(len>MAX_UNIFORM_V4_LEN) {len=MAX_UNIFORM_V4_LEN;}
		for(i=0;i<len*1;i++)
		{
			lua_rawgeti(l,2,i+1); ii[i]=(int)luaL_checknumber(l,-1); lua_pop(l,1);
		}
	}
	else
	{
		ip=ii;
		for(i=0;i<len*1;i++)
		{
			ii[i]=(int)luaL_checknumber(l,i+2);
		}
	}
	glUniform1iv(idx,len,ip);
	return 0;
}
static int lua_gles_Uniform2i (lua_State *l)
{
int idx;
int *ip;
int ii[MAX_UNIFORM_V4_LEN*2];
int len=1;
int i;
	idx=(int)luaL_checknumber(l,1);
	ip=(int*)lua_tardis_uda(l,2);
	if(ip)
	{
		len=lua_tardis_uda_count(l,2)/2;
	}
	else
	if( lua_istable(l,2))
	{
		ip=ii;
		len=lua_objlen(l,2)/2;
		if(len>MAX_UNIFORM_V4_LEN) {len=MAX_UNIFORM_V4_LEN;}
		for(i=0;i<len*2;i++)
		{
			lua_rawgeti(l,2,i+1); ii[i]=(int)luaL_checknumber(l,-1); lua_pop(l,1);
		}
	}
	else
	{
		ip=ii;
		for(i=0;i<len*2;i++)
		{
			ii[i]=(int)luaL_checknumber(l,i+2);
		}
	}
	glUniform2iv(idx,len,ip);
	return 0;
}
static int lua_gles_Uniform3i (lua_State *l)
{
int idx;
int *ip;
int ii[MAX_UNIFORM_V4_LEN*3];
int len=1;
int i;
	idx=(int)luaL_checknumber(l,1);
	ip=(int*)lua_tardis_uda(l,2);
	if(ip)
	{
		len=lua_tardis_uda_count(l,2)/3;
	}
	else
	if( lua_istable(l,2))
	{
		ip=ii;
		len=lua_objlen(l,2)/3;
		if(len>MAX_UNIFORM_V4_LEN) {len=MAX_UNIFORM_V4_LEN;}
		for(i=0;i<len*3;i++)
		{
			lua_rawgeti(l,2,i+1); ii[i]=(int)luaL_checknumber(l,-1); lua_pop(l,1);
		}
	}
	else
	{
		ip=ii;
		for(i=0;i<len*3;i++)
		{
			ii[i]=(int)luaL_checknumber(l,i+2);
		}
	}
	glUniform3iv(idx,len,ip);
	return 0;
}
static int lua_gles_Uniform4i (lua_State *l)
{
int idx;
int *ip;
int ii[MAX_UNIFORM_V4_LEN*4];
int len=1;
int i;
	idx=(int)luaL_checknumber(l,1);
	ip=(int*)lua_tardis_uda(l,2);
	if(ip)
	{
		len=lua_tardis_uda_count(l,2)/4;
	}
	else
	if( lua_istable(l,2))
	{
		ip=ii;
		len=lua_objlen(l,2)/4;
		if(len>MAX_UNIFORM_V4_LEN) {len=MAX_UNIFORM_V4_LEN;}
		for(i=0;i<len*4;i++)
		{
			lua_rawgeti(l,2,i+1); ii[i]=(int)luaL_checknumber(l,-1); lua_pop(l,1);
		}
	}
	else
	{
		ip=ii;
		for(i=0;i<len*4;i++)
		{
			ii[i]=(int)luaL_checknumber(l,i+2);
		}
	}
	glUniform4iv(idx,len,ip);
	return 0;
}


static int lua_gles_Uniform1f (lua_State *l)
{
int idx;
float *fp;
float ff[MAX_UNIFORM_V4_LEN*1];
int len=1;
int i;
	idx=(int)luaL_checknumber(l,1);
	fp=(float*)lua_tardis_uda(l,2);
	if(fp)
	{
		len=lua_tardis_uda_count(l,2)/1;
	}
	else
	if( lua_istable(l,2))
	{
		fp=ff;
		len=lua_objlen(l,2)/1;
		if(len>MAX_UNIFORM_V4_LEN) {len=MAX_UNIFORM_V4_LEN;}
		for(i=0;i<len*1;i++)
		{
			lua_rawgeti(l,2,i+1); ff[i]=(float)luaL_checknumber(l,-1); lua_pop(l,1);
		}
	}
	else
	{
		fp=ff;
		for(i=0;i<len*1;i++)
		{
			ff[i]=(float)luaL_checknumber(l,i+2);
		}
	}
	glUniform1fv(idx,len,fp);
	return 0;
}
static int lua_gles_Uniform2f (lua_State *l)
{
int idx;
float *fp;
float ff[MAX_UNIFORM_V4_LEN*2];
int len=1;
int i;
	idx=(int)luaL_checknumber(l,1);
	fp=(float*)lua_tardis_uda(l,2);
	if(fp)
	{
		len=lua_tardis_uda_count(l,2)/2;
	}
	else
	if( lua_istable(l,2))
	{
		fp=ff;
		len=lua_objlen(l,2)/2;
		if(len>MAX_UNIFORM_V4_LEN) {len=MAX_UNIFORM_V4_LEN;}
		for(i=0;i<len*2;i++)
		{
			lua_rawgeti(l,2,i+1); ff[i]=(float)luaL_checknumber(l,-1); lua_pop(l,1);
		}
	}
	else
	{
		fp=ff;
		for(i=0;i<len*2;i++)
		{
			ff[i]=(float)luaL_checknumber(l,i+2);
		}
	}
	glUniform2fv(idx,len,fp);
	return 0;
}
static int lua_gles_Uniform3f (lua_State *l)
{
int idx;
float *fp;
float ff[MAX_UNIFORM_V4_LEN*3];
int len=1;
int i;
	idx=(int)luaL_checknumber(l,1);
	fp=(float*)lua_tardis_uda(l,2);
	if(fp)
	{
		len=lua_tardis_uda_count(l,2)/3;
	}
	else
	if( lua_istable(l,2))
	{
		fp=ff;
		len=lua_objlen(l,2)/3;
		if(len>MAX_UNIFORM_V4_LEN) {len=MAX_UNIFORM_V4_LEN;}
		for(i=0;i<len*3;i++)
		{
			lua_rawgeti(l,2,i+1); ff[i]=(float)luaL_checknumber(l,-1); lua_pop(l,1);
		}
	}
	else
	{
		fp=ff;
		for(i=0;i<len*3;i++)
		{
			ff[i]=(float)luaL_checknumber(l,i+2);
		}
	}
	glUniform3fv(idx,len,fp);
	return 0;
}
static int lua_gles_Uniform4f (lua_State *l)
{
int idx;
float *fp;
float ff[MAX_UNIFORM_V4_LEN*4];
int len=1;
int i;
	idx=(int)luaL_checknumber(l,1);
	
	fp=(float*)lua_tardis_uda(l,2);
	if(fp)
	{
		len=lua_tardis_uda_count(l,2)/4;
	}
	else
	if( lua_istable(l,2))
	{
		fp=ff;
		len=lua_objlen(l,2)/4;
		if(len>MAX_UNIFORM_V4_LEN) {len=MAX_UNIFORM_V4_LEN;}
		for(i=0;i<len*4;i++)
		{
			lua_rawgeti(l,2,i+1); ff[i]=(float)luaL_checknumber(l,-1); lua_pop(l,1);
		}
	}
	else
	{
		fp=ff;
		for(i=0;i<len*4;i++)
		{
			ff[i]=(float)luaL_checknumber(l,i+2);
		}
	}
	glUniform4fv(idx,len,fp);
	return 0;
}


static int lua_gles_UniformMatrix2f (lua_State *l)
{
int idx;
float *fp;
float ff[MAX_UNIFORM_V4_LEN*4];
int len=1;
int i;
	idx=(int)luaL_checknumber(l,1);
	
	fp=(float*)lua_tardis_uda(l,2);
	if(fp)
	{
		len=lua_tardis_uda_count(l,2)/4;
	}
	else
	if( lua_istable(l,2))
	{
		fp=ff;
		len=lua_objlen(l,2)/4;
		if(len>MAX_UNIFORM_V4_LEN) {len=MAX_UNIFORM_V4_LEN;}
		for(i=0;i<len*4;i++)
		{
			lua_rawgeti(l,2,i+1); ff[i]=(float)luaL_checknumber(l,-1); lua_pop(l,1);
		}
	}
	else
	{
		fp=ff;
		for(i=0;i<len*4;i++)
		{
			ff[i]=(float)luaL_checknumber(l,i+2);
		}
	}
	glUniformMatrix2fv(idx, len, 0, fp );
	return 0;
}
static int lua_gles_UniformMatrix3f (lua_State *l)
{
int idx;
float *fp;
float ff[(MAX_UNIFORM_V4_LEN/4)*9];
int len=1;
int i;
	idx=(int)luaL_checknumber(l,1);
	
	fp=(float*)lua_tardis_uda(l,2);
	if(fp)
	{
		len=lua_tardis_uda_count(l,2)/9;
	}
	else
	if( lua_istable(l,2))
	{
		fp=ff;
		len=lua_objlen(l,2)/9;
		if(len>(MAX_UNIFORM_V4_LEN/4)) {len=(MAX_UNIFORM_V4_LEN/4);}
		for(i=0;i<len*9;i++)
		{
			lua_rawgeti(l,2,i+1); ff[i]=(float)luaL_checknumber(l,-1); lua_pop(l,1);
		}
	}
	else
	{
		fp=ff;
		for(i=0;i<len*9;i++)
		{
			ff[i]=(float)luaL_checknumber(l,i+2);
		}
	}
	glUniformMatrix3fv(idx, len, 0, fp );
	return 0;
}
static int lua_gles_UniformMatrix4f (lua_State *l)
{
int idx;
float *fp;
float ff[(MAX_UNIFORM_V4_LEN/4)*16];
int len=1;
int i;
	idx=(int)luaL_checknumber(l,1);
	
	fp=(float*)lua_tardis_uda(l,2);
	if(fp)
	{
		len=lua_tardis_uda_count(l,2)/16;
	}
	else
	if( lua_istable(l,2))
	{
		fp=ff;
		len=lua_objlen(l,2)/16;
		if(len>(MAX_UNIFORM_V4_LEN/4)) {len=(MAX_UNIFORM_V4_LEN/4);}
		for(i=0;i<len*16;i++)
		{
			lua_rawgeti(l,2,i+1); ff[i]=(float)luaL_checknumber(l,-1); lua_pop(l,1);
		}
	}
	else
	{
		fp=ff;
		for(i=0;i<len*16;i++)
		{
			ff[i]=(float)luaL_checknumber(l,i+2);
		}
	}
	glUniformMatrix4fv(idx, len, 0, fp );
	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// stubs
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_gles_BindAttribLocation (lua_State *l)
{
	glBindAttribLocation(	(int)luaL_checknumber(l,1)	,
							(int)luaL_checknumber(l,2)	,
							luaL_checkstring(l,3) 		);
	return 0;
}


static int lua_gles_GetActiveAttrib (lua_State *l)
{
char name[256];
int size=0;
int type=0;
	name[0]=0; // safety
	glGetActiveAttrib(	(int)luaL_checknumber(l,1)	,
						(int)luaL_checknumber(l,2)	,
						256,
						0,
						&size,
						(unsigned int*)&type,
						name);
	lua_pushstring(l,name);
	lua_pushnumber(l,(double)type);
	lua_pushnumber(l,(double)size);
	return 3;
}
static int lua_gles_GetActiveUniform (lua_State *l)
{
char name[256];
int size=0;
int type=0;
	name[0]=0; // safety
	glGetActiveUniform(	(int)luaL_checknumber(l,1)	,
						(int)luaL_checknumber(l,2)	,
						256,
						0,
						&size,
						(unsigned int*)&type,
						name);
	lua_pushstring(l,name);
	lua_pushnumber(l,(double)type);
	lua_pushnumber(l,(double)size);
	return 3;
}

static int lua_gles_GetBufferParameter (lua_State *l)
{
int ret=0;
	glGetBufferParameteriv(		(int)luaL_checknumber(l,1)	,
								(int)luaL_checknumber(l,2)	,
								&ret						);
	lua_pushnumber(l,(double)ret);
	return 1;
}

static int lua_gles_GetShaderPrecisionFormat (lua_State *l)
{
int ret[3]={0,0,0};
	glGetShaderPrecisionFormat(		(int)luaL_checknumber(l,1)	,
									(int)luaL_checknumber(l,2)	,
									ret							,
									ret+2						);
	lua_pushnumber(l,(double)ret[0]);
	lua_pushnumber(l,(double)ret[1]);
	lua_pushnumber(l,(double)ret[2]);
	return 3;
}

static int lua_gles_GetShaderSource (lua_State *l)
{
int i;
int size=0;
int newsize=0;
char *p=0;
	i=luaL_checknumber(l,1);

	glGetShaderiv(i, GL_SHADER_SOURCE_LENGTH, &size);
	if(size==0) { size=16384; } // pick a fuckoff buffer size when driver is retarted
	p=malloc(size);
	if(p==0) { lua_pushfstring(l,"malloc failed (%d)",size); lua_error(l); return 0; }
	
	glGetShaderSource(i,size,&newsize,p);
	lua_pushstring(l,p);
	free(p);
	
	return 1;
}


static int lua_gles_GetUniformf (lua_State *l)
{
int i;
float ret[16];
int size=16;
	if(lua_isnumber(l,3)) { size=luaL_checknumber(l,3); }
	if(size>16) { size=16; }
	glGetUniformfv(luaL_checknumber(l,1),luaL_checknumber(l,2),ret);
	for(i=0;i<size;i++)
	{
		lua_pushnumber(l,(double)ret[i]);
	}
	return size;
}
static int lua_gles_GetUniformi (lua_State *l)
{
int i;
int ret[16];
int size=16;
	if(lua_isnumber(l,3)) { size=luaL_checknumber(l,3); }
	if(size>16) { size=16; }
	glGetUniformiv(luaL_checknumber(l,1),luaL_checknumber(l,2),ret);
	for(i=0;i<size;i++)
	{
		lua_pushnumber(l,(double)ret[i]);
	}
	return size;
}

static int lua_gles_GetVertexAttrib (lua_State *l)
{
int idx;
int def;

// default of a single integer
char v='i';
int num=1;

int iv[16];
float fv[16];

int i;

	idx=luaL_checknumber(l,1);
	def=luaL_checknumber(l,2);
	
	if(def==GL_CURRENT_VERTEX_ATTRIB) // everything else is just an int
	{
		num=4; v='f';
	}

	lua_gles_get_prop_info(def, &v, &num);
	
	switch(v)
	{
		case 'i':
			glGetVertexAttribiv(idx,def,iv);
			for(i=0;i<num;i++)
			{
				lua_pushnumber(l,(double)iv[i]);
			}
		break;
		case 'f':
			glGetVertexAttribfv(idx,def,fv);
			for(i=0;i<num;i++)
			{
				lua_pushnumber(l,(double)fv[i]);
			}
		break;
	}
	
	return num;
}

static int lua_gles_GetVertexAttribPointer (lua_State *l)
{
void *ret=0;
	glGetVertexAttribPointerv(		(int)luaL_checknumber(l,1)	,
									(int)luaL_checknumber(l,2)	,
									&ret						);
	lua_pushnumber(l,(double)((intptr_t)ret) );
	return 1;
}

static int lua_gles_ReleaseShaderCompiler (lua_State *l)
{
	glReleaseShaderCompiler();
	return 0;
}





/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// function stubs so we at least have them all typed into this file as a reminder to wire them up
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_gles_ActiveTexture (lua_State *l)
{
	glActiveTexture(	(int)luaL_checknumber(l,1)	);
	return 0;
}
static int lua_gles_BindFramebuffer (lua_State *l)
{
	glBindFramebuffer(	(int)luaL_checknumber(l,1)	,
						(int)luaL_checknumber(l,2)	);
	return 0;
}
static int lua_gles_BindRenderbuffer (lua_State *l)
{
	glBindRenderbuffer(	(int)luaL_checknumber(l,1)	,
						(int)luaL_checknumber(l,2)	);
	return 0;
}
static int lua_gles_BlendColor (lua_State *l)
{
	glBlendColor(		(float)luaL_checknumber(l,1)	,
						(float)luaL_checknumber(l,2)	,
						(float)luaL_checknumber(l,3)	,
						(float)luaL_checknumber(l,4)	);
	return 0;
}
static int lua_gles_BlendEquation (lua_State *l)
{
	glBlendEquation(	(int)luaL_checknumber(l,1)	);
	return 0;
}
static int lua_gles_BlendEquationSeparate (lua_State *l)
{
	glBlendEquationSeparate(	(int)luaL_checknumber(l,1)	,
								(int)luaL_checknumber(l,2)	);
	return 0;
}
static int lua_gles_BlendFuncSeparate (lua_State *l)
{
	glBlendFuncSeparate(		(int)luaL_checknumber(l,1)	,
								(int)luaL_checknumber(l,2)	,
								(int)luaL_checknumber(l,3)	,
								(int)luaL_checknumber(l,4)	);
	return 0;
}
static int lua_gles_CheckFramebufferStatus (lua_State *l)
{
	int n=glCheckFramebufferStatus(		(int)luaL_checknumber(l,1)	);
	lua_pushnumber(l,n);
	return 1;
}
static int lua_gles_ClearStencil (lua_State *l)
{
	glClearStencil(		(int)luaL_checknumber(l,1)	);
	return 0;
}
static int lua_gles_ColorMask (lua_State *l)
{
	glColorMask(		(int)luaL_checknumber(l,1)	,
						(int)luaL_checknumber(l,2)	,
						(int)luaL_checknumber(l,3)	,
						(int)luaL_checknumber(l,4)	);
	return 0;
}
static int lua_gles_CompressedTexImage2D (lua_State *l)
{
	int len=0;
	void *ptr=(void*)lua_gles_topointer(l,7,&len);
	
	glCompressedTexImage2D(		(int)luaL_checknumber(l,1)		,
								(int)luaL_checknumber(l,2)		,
								(int)luaL_checknumber(l,3)		,
								(int)luaL_checknumber(l,4)		,
								(int)luaL_checknumber(l,5)		,
								(int)luaL_checknumber(l,6)		,
								(int)len						,
								(void*)ptr);
	return 0;
}
static int lua_gles_CompressedTexSubImage2D (lua_State *l)
{
	int len=0;
	void *ptr=(void*)lua_gles_topointer(l,8,&len);

	glCompressedTexSubImage2D(	(int)luaL_checknumber(l,1)		,
								(int)luaL_checknumber(l,2)		,
								(int)luaL_checknumber(l,3)		,
								(int)luaL_checknumber(l,4)		,
								(int)luaL_checknumber(l,5)		,
								(int)luaL_checknumber(l,6)		,
								(int)luaL_checknumber(l,7)		,
								(int)len						,
								(void*)ptr						);
	return 0;
}
static int lua_gles_CopyTexImage2D (lua_State *l)
{
	glCopyTexImage2D(	(int)luaL_checknumber(l,1)		,
						(int)luaL_checknumber(l,2)		,
						(int)luaL_checknumber(l,3)		,
						(int)luaL_checknumber(l,4)		,
						(int)luaL_checknumber(l,5)		,
						(int)luaL_checknumber(l,6)		,
						(int)luaL_checknumber(l,7)		,
						(int)luaL_checknumber(l,8)		);
	return 0;
}
static int lua_gles_CopyTexSubImage2D (lua_State *l)
{
	glCopyTexSubImage2D(	(int)luaL_checknumber(l,1)		,
							(int)luaL_checknumber(l,2)		,
							(int)luaL_checknumber(l,3)		,
							(int)luaL_checknumber(l,4)		,
							(int)luaL_checknumber(l,5)		,
							(int)luaL_checknumber(l,6)		,
							(int)luaL_checknumber(l,7)		,
							(int)luaL_checknumber(l,8)		);
	return 0;
}
static int lua_gles_CullFace (lua_State *l)
{
	glCullFace(	(int)luaL_checknumber(l,1)		);
	return 0;
}
static int lua_gles_DeleteFramebuffer (lua_State *l)
{
int id=luaL_checknumber(l,1);
	glDeleteFramebuffers(1,(const unsigned int*)&id);
	return 0;
}
static int lua_gles_DeleteRenderbuffer (lua_State *l)
{
unsigned int id=luaL_checknumber(l,1);
	glDeleteRenderbuffers(1,(const unsigned int*)&id);
	return 0;
}
static int lua_gles_DetachShader (lua_State *l)
{
	glDetachShader(		(int)luaL_checknumber(l,1)		,
						(int)luaL_checknumber(l,2)		);
	return 0;
}
static int lua_gles_FramebufferRenderbuffer (lua_State *l)
{
	glFramebufferRenderbuffer(	(int)luaL_checknumber(l,1)		,
								(int)luaL_checknumber(l,2)		,
								(int)luaL_checknumber(l,3)		,
								(int)luaL_checknumber(l,4)		);
	return 0;
}
static int lua_gles_FramebufferTexture2D (lua_State *l)
{
	glFramebufferTexture2D(		(int)luaL_checknumber(l,1)		,
								(int)luaL_checknumber(l,2)		,
								(int)luaL_checknumber(l,3)		,
								(int)luaL_checknumber(l,4)		,
								(int)luaL_checknumber(l,5)		);
	return 0;
}
static int lua_gles_FrontFace (lua_State *l)
{
	glFrontFace(	(int)luaL_checknumber(l,1)		);
	return 0;
}
static int lua_gles_GenFramebuffer (lua_State *l)
{
unsigned int id=0;
	glGenFramebuffers(1,&id);
	lua_pushnumber(l,(double)id);
	return 1;
}
static int lua_gles_GenRenderbuffer (lua_State *l)
{
unsigned int id=0;
	glGenRenderbuffers(1,&id);
	lua_pushnumber(l,(double)id);
	return 1;
}
static int lua_gles_GenerateMipmap (lua_State *l)
{
	glGenerateMipmap(	(int)luaL_checknumber(l,1)		);
	return 0;
}
static int lua_gles_GetAttachedShaders (lua_State *l)
{
int i;
int count=0;
int ret[16];

	glGetAttachedShaders( (int)luaL_checknumber(l,1)	, 16 , (int *)&count, (unsigned int *)ret);
	for(i=0;i<count;i++)
	{
		lua_pushnumber(l,(double)ret[i]);
	}
	return count;
}
static int lua_gles_GetFramebufferAttachmentParameter (lua_State *l)
{
int ret=0;
	glGetFramebufferAttachmentParameteriv(	(int)luaL_checknumber(l,1)	,
											(int)luaL_checknumber(l,2)	,
											(int)luaL_checknumber(l,3)	,
											&ret						);
	lua_pushnumber(l,(double)ret);
	return 1;
}
static int lua_gles_GetRenderbufferParameter (lua_State *l)
{
int ret=0;
	glGetRenderbufferParameteriv(	(int)luaL_checknumber(l,1)	,
									(int)luaL_checknumber(l,2)	,
									&ret						);
	lua_pushnumber(l,(double)ret);
	return 1;
}
static int lua_gles_GetTexParameter (lua_State *l)
{
int ret=0;
	glGetTexParameteriv(	(int)luaL_checknumber(l,1)	,
							(int)luaL_checknumber(l,2)	,
							&ret						);
	lua_pushnumber(l,(double)ret);
	return 1;
}
static int lua_gles_Hint (lua_State *l)
{
	glHint(	(int)luaL_checknumber(l,1)	,
			(int)luaL_checknumber(l,2)	);
	return 0;
}
static int lua_gles_IsBuffer (lua_State *l)
{
	if( glIsBuffer( (int)luaL_checknumber(l,1) ) == GL_TRUE )
	{ lua_pushboolean(l,1); } else
	{ lua_pushboolean(l,0); }
	return 1;
}
static int lua_gles_IsEnabled (lua_State *l)
{
	if( glIsEnabled( (int)luaL_checknumber(l,1) ) == GL_TRUE )
	{ lua_pushboolean(l,1); } else
	{ lua_pushboolean(l,0); }
	return 1;
}
static int lua_gles_IsFramebuffer (lua_State *l)
{
	if( glIsFramebuffer( (int)luaL_checknumber(l,1) ) == GL_TRUE )
	{ lua_pushboolean(l,1); } else
	{ lua_pushboolean(l,0); }
	return 1;
}
static int lua_gles_IsProgram (lua_State *l)
{
	if( glIsProgram( (int)luaL_checknumber(l,1) ) == GL_TRUE )
	{ lua_pushboolean(l,1); } else
	{ lua_pushboolean(l,0); }
	return 1;
}
static int lua_gles_IsRenderbuffer (lua_State *l)
{
	if( glIsRenderbuffer( (int)luaL_checknumber(l,1) ) == GL_TRUE )
	{ lua_pushboolean(l,1); } else
	{ lua_pushboolean(l,0); }
	return 1;
}
static int lua_gles_IsShader (lua_State *l)
{
	if( glIsShader( (int)luaL_checknumber(l,1) ) == GL_TRUE )
	{ lua_pushboolean(l,1); } else
	{ lua_pushboolean(l,0); }
	return 1;
}
static int lua_gles_IsTexture (lua_State *l)
{
	if( glIsTexture( (int)luaL_checknumber(l,1) ) == GL_TRUE )
	{ lua_pushboolean(l,1); } else
	{ lua_pushboolean(l,0); }
	return 1;
}
static int lua_gles_LineWidth (lua_State *l)
{
	glLineWidth(	(float)luaL_checknumber(l,1)	);
	return 0;
}
static int lua_gles_PixelStore (lua_State *l)
{
	glPixelStorei(		(int)luaL_checknumber(l,1)	,
						(int)luaL_checknumber(l,2)	);
	return 0;
}
static int lua_gles_PolygonOffset (lua_State *l)
{
	glPolygonOffset(	(float)luaL_checknumber(l,1)	,
						(float)luaL_checknumber(l,2)	);
	return 0;
}
static int lua_gles_ReadPixels (lua_State *l)
{
	glReadPixels(		(int)luaL_checknumber(l,1)		,
						(int)luaL_checknumber(l,2)		,
						(int)luaL_checknumber(l,3)		,
						(int)luaL_checknumber(l,4)		,
						(int)luaL_checknumber(l,5)		,
						(int)luaL_checknumber(l,6)		,
						(void*)lua_touserdata(l,7)		); // must be userdata to be able to write to it
	return 0;
}
static int lua_gles_RenderbufferStorage (lua_State *l)
{
	glRenderbufferStorage(		(int)luaL_checknumber(l,1)		,
								(int)luaL_checknumber(l,2)		,
								(int)luaL_checknumber(l,3)		,
								(int)luaL_checknumber(l,4)		);
	return 0;
}
static int lua_gles_SampleCoverage (lua_State *l)
{
	glSampleCoverage(		(float)luaL_checknumber(l,1)		,
							(int)luaL_checknumber(l,2)			);
	return 0;
}
static int lua_gles_Scissor (lua_State *l)
{
	glScissor(		(int)luaL_checknumber(l,1)		,
					(int)luaL_checknumber(l,2)		,
					(int)luaL_checknumber(l,3)		,
					(int)luaL_checknumber(l,4)		);
	return 0;
}
static int lua_gles_ShaderBinary (lua_State *l)
{
// only one shader at a time?
// can change to a table as first param if we need more?

	int a=luaL_checknumber(l,1);
	int len=0;
	void *ptr=(void*)lua_gles_topointer(l,3,&len);
	
	glShaderBinary(		1								,
						(const unsigned int *)&a								,
						(int)luaL_checknumber(l,2)		,
						(void*)ptr						,
						(int)len						);
	return 0;
}
static int lua_gles_StencilFunc (lua_State *l)
{
	glStencilFunc(		(int)luaL_checknumber(l,1)		,
						(int)luaL_checknumber(l,2)		,
						(int)luaL_checknumber(l,3)		);
	return 0;
}
static int lua_gles_StencilFuncSeparate (lua_State *l)
{
	glStencilFuncSeparate(	(int)luaL_checknumber(l,1)		,
							(int)luaL_checknumber(l,2)		,
							(int)luaL_checknumber(l,4)		,
							(int)luaL_checknumber(l,3)		);
	return 0;
}
static int lua_gles_StencilOp (lua_State *l)
{
	glStencilOp(		(int)luaL_checknumber(l,1)		,
						(int)luaL_checknumber(l,2)		,
						(int)luaL_checknumber(l,3)		);
	return 0;
}
static int lua_gles_StencilOpSeparate (lua_State *l)
{
	glStencilOpSeparate(	(int)luaL_checknumber(l,1)		,
							(int)luaL_checknumber(l,2)		,
							(int)luaL_checknumber(l,4)		,
							(int)luaL_checknumber(l,3)		);
	return 0;
}


#endif


#if defined(LUA_GLES_GLES3) || defined(LUA_GLES_GL)

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// GLES3 functions
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_gles_VertexAttribIPointer (lua_State *l)
{
int index;
int size;
int type;
int normalized;
int stride;
const void *pointer;
	index=luaL_checknumber(l,1);
	size=luaL_checknumber(l,2);
	type=luaL_checknumber(l,3);
	stride=luaL_checknumber(l,4);
	pointer=lua_gles_topointer(l,5,0);

	glVertexAttribIPointer(index,size,type,stride,pointer);
	return 0;
}


static int lua_gles_GenVertexArray (lua_State *l)
{
int id;
	glGenVertexArrays(1,(unsigned int *)&id);
	lua_pushnumber(l,(double)id);
	return 1;
}

static int lua_gles_BindVertexArray (lua_State *l)
{
	glBindVertexArray((int)luaL_checknumber(l,1));
	return 0;
}

static int lua_gles_DeleteVertexArray (lua_State *l)
{
int id=(int)luaL_checknumber(l,1);
	glDeleteVertexArrays(1,(const unsigned int*)&id);
	return 0;
}



static int lua_gles_BlitFramebuffer (lua_State *l)
{
	int srcX0  =(int)luaL_checknumber(l,1);
 	int srcY0  =(int)luaL_checknumber(l,2);
 	int srcX1  =(int)luaL_checknumber(l,3);
 	int srcY1  =(int)luaL_checknumber(l,4);
 	int dstX0  =(int)luaL_checknumber(l,5);
 	int dstY0  =(int)luaL_checknumber(l,6);
 	int dstX1  =(int)luaL_checknumber(l,7);
 	int dstY1  =(int)luaL_checknumber(l,8);
 	int mask   =(int)luaL_checknumber(l,9);
 	int filter =(int)luaL_checknumber(l,10);

	glBlitFramebuffer(
		srcX0,srcY0,srcX1,srcY1,
		dstX0,dstY0,dstX1,dstY1,
		mask,filter
	);

	return 0;
}

#endif

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// open library.
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
LUALIB_API int luaopen_gles_core(lua_State *l)
{
	
	const luaL_Reg lib[] =
	{
		{"Get",					lua_gles_Get},
		{"GetError",			lua_gles_GetError},

		{"Enable",				lua_gles_Enable},
		{"Disable",				lua_gles_Disable},

		{"ClearColor",			lua_gles_ClearColor},
		{"ClearDepth",			lua_gles_ClearDepth},
		{"Clear",				lua_gles_Clear},
		{"Finish",				lua_gles_Finish},
		{"Flush",				lua_gles_Flush},

		{"GenTexture",			lua_gles_GenTexture},
		{"BindTexture",			lua_gles_BindTexture},
		{"DeleteTexture",		lua_gles_DeleteTexture},
		{"TexImage2D",			lua_gles_TexImage2D},
		{"TexSubImage2D",		lua_gles_TexSubImage2D},
		{"TexParameter",		lua_gles_TexParameter},

		{"BlendFunc",			lua_gles_BlendFunc},		
		{"Viewport",			lua_gles_Viewport},
		
		{"DrawArrays",			lua_gles_DrawArrays},
		{"DrawElements",		lua_gles_DrawElements},

		{"GenBuffer",			lua_gles_GenBuffer},
		{"BindBuffer",			lua_gles_BindBuffer},
		{"DeleteBuffer",		lua_gles_DeleteBuffer},
		{"BufferData",			lua_gles_BufferData},
		{"BufferSubData",		lua_gles_BufferSubData},

		{"DepthMask",			lua_gles_DepthMask},
		{"DepthRange",			lua_gles_DepthRange},
		{"DepthFunc",			lua_gles_DepthFunc},

// fixed pipeline
#if defined(LUA_GLES_GLES1) //|| defined(LUA_GLES_GL)

		{"Color",				lua_gles_Color},
		{"EnableClientState",	lua_gles_EnableClientState},
		{"DisableClientState",	lua_gles_DisableClientState},
		{"ShadeModel",			lua_gles_ShadeModel},		

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

// programable pipeline
#if defined(LUA_GLES_GLES3) || defined(LUA_GLES_GLES2) || defined(LUA_GLES_GL)

		{"CreateShader",			lua_gles_CreateShader},
		{"DeleteShader",			lua_gles_DeleteShader},
		{"ShaderSource",			lua_gles_ShaderSource},
		{"CompileShader",			lua_gles_CompileShader},
		{"GetShader",				lua_gles_GetShader},
		{"GetShaderInfoLog",		lua_gles_GetShaderInfoLog},
		{"AttachShader",			lua_gles_AttachShader},

		{"CreateProgram",			lua_gles_CreateProgram},
		{"DeleteProgram",			lua_gles_DeleteProgram},
		{"LinkProgram",				lua_gles_LinkProgram},
		{"GetProgram",				lua_gles_GetProgram},
		{"UseProgram",				lua_gles_UseProgram},
		{"ValidateProgram",			lua_gles_ValidateProgram},
		{"GetProgramInfoLog",		lua_gles_GetProgramInfoLog},
		{"GetAttribLocation",		lua_gles_GetAttribLocation},
		{"GetUniformLocation",		lua_gles_GetUniformLocation},

		{"EnableVertexAttribArray",	lua_gles_EnableVertexAttribArray},
		{"DisableVertexAttribArray",lua_gles_DisableVertexAttribArray},

		{"VertexAttribPointer",		lua_gles_VertexAttribPointer},
		
		{"VertexAttrib1f",			lua_gles_VertexAttrib1f},
		{"VertexAttrib2f",			lua_gles_VertexAttrib2f},
		{"VertexAttrib3f",			lua_gles_VertexAttrib3f},
		{"VertexAttrib4f",			lua_gles_VertexAttrib4f},

		{"Uniform1f",				lua_gles_Uniform1f},
		{"Uniform2f",				lua_gles_Uniform2f},
		{"Uniform3f",				lua_gles_Uniform3f},
		{"Uniform4f",				lua_gles_Uniform4f},
		{"Uniform1i",				lua_gles_Uniform1i},
		{"Uniform2i",				lua_gles_Uniform2i},
		{"Uniform3i",				lua_gles_Uniform3i},
		{"Uniform4i",				lua_gles_Uniform4i},

		{"UniformMatrix2f",			lua_gles_UniformMatrix2f},
		{"UniformMatrix3f",			lua_gles_UniformMatrix3f},
		{"UniformMatrix4f",			lua_gles_UniformMatrix4f},

		{"BindAttribLocation",		lua_gles_BindAttribLocation},
		{"GetActiveAttrib",			lua_gles_GetActiveAttrib},
		{"GetActiveUniform",		lua_gles_GetActiveUniform},
		{"GetBufferParameter",		lua_gles_GetBufferParameter},
		{"GetShaderPrecisionFormat",lua_gles_GetShaderPrecisionFormat},
		{"GetShaderSource",			lua_gles_GetShaderSource},
		{"GetUniformf",				lua_gles_GetUniformf},
		{"GetUniformi",				lua_gles_GetUniformi},
		{"GetVertexAttrib",			lua_gles_GetVertexAttrib},
		{"GetVertexAttribPointer",	lua_gles_GetVertexAttribPointer},
		{"ReleaseShaderCompiler",	lua_gles_ReleaseShaderCompiler},

//functions below may or may not be in gles1

		{"ActiveTexture",						lua_gles_ActiveTexture},

		{"BindFramebuffer",						lua_gles_BindFramebuffer},
		{"BindRenderbuffer",					lua_gles_BindRenderbuffer},
		{"BlendColor",							lua_gles_BlendColor},
		{"BlendEquation",						lua_gles_BlendEquation},
		{"BlendEquationSeparate",				lua_gles_BlendEquationSeparate},
		{"BlendFuncSeparate",					lua_gles_BlendFuncSeparate},

		{"CheckFramebufferStatus",				lua_gles_CheckFramebufferStatus},
		{"ClearStencil",						lua_gles_ClearStencil},
		{"ColorMask",							lua_gles_ColorMask},
		{"CompressedTexImage2D",				lua_gles_CompressedTexImage2D},
		{"CompressedTexSubImage2D",				lua_gles_CompressedTexSubImage2D},
		{"CopyTexImage2D",						lua_gles_CopyTexImage2D},
		{"CopyTexSubImage2D",					lua_gles_CopyTexSubImage2D},
		{"CullFace",							lua_gles_CullFace},


		{"DeleteFramebuffer",					lua_gles_DeleteFramebuffer},
		{"DeleteRenderbuffer",					lua_gles_DeleteRenderbuffer},
		{"DetachShader",						lua_gles_DetachShader},

		{"FramebufferRenderbuffer",				lua_gles_FramebufferRenderbuffer},
		{"FramebufferTexture2D",				lua_gles_FramebufferTexture2D},
		{"FrontFace",							lua_gles_FrontFace},

		{"GenFramebuffer",						lua_gles_GenFramebuffer},
		{"GenRenderbuffer",						lua_gles_GenRenderbuffer},
		{"GenerateMipmap",						lua_gles_GenerateMipmap},
		{"GetAttachedShaders",					lua_gles_GetAttachedShaders},
		{"GetFramebufferAttachmentParameter",	lua_gles_GetFramebufferAttachmentParameter},
		{"GetRenderbufferParameter",			lua_gles_GetRenderbufferParameter},
		{"GetTexParameter",						lua_gles_GetTexParameter},

		{"Hint",								lua_gles_Hint},

		{"IsBuffer",							lua_gles_IsBuffer},
		{"IsEnabled",							lua_gles_IsEnabled},
		{"IsFramebuffer",						lua_gles_IsFramebuffer},
		{"IsProgram",							lua_gles_IsProgram},
		{"IsRenderbuffer",						lua_gles_IsRenderbuffer},
		{"IsShader",							lua_gles_IsShader},
		{"IsTexture",							lua_gles_IsTexture},

		{"LineWidth",							lua_gles_LineWidth},

		{"PixelStore",							lua_gles_PixelStore},
		{"PolygonOffset",						lua_gles_PolygonOffset},

		{"ReadPixels",							lua_gles_ReadPixels},
		{"RenderbufferStorage",					lua_gles_RenderbufferStorage},

		{"SampleCoverage",						lua_gles_SampleCoverage},
		{"Scissor",								lua_gles_Scissor},
		{"ShaderBinary",						lua_gles_ShaderBinary},
		{"StencilFunc",							lua_gles_StencilFunc},
		{"StencilFuncSeparate",					lua_gles_StencilFuncSeparate},
		{"StencilOp",							lua_gles_StencilOp},
		{"StencilOpSeparate",					lua_gles_StencilOpSeparate},
#endif

// GLES3 ( should add all of them, at the moment is is cherry picked as needed )
#if defined(LUA_GLES_GLES3) || defined(LUA_GLES_GL)

		{"VertexAttribIPointer",	lua_gles_VertexAttribIPointer},

		{"GenVertexArray",			lua_gles_GenVertexArray},
		{"BindVertexArray",			lua_gles_BindVertexArray},
		{"DeleteVertexArray",		lua_gles_DeleteVertexArray},

		{"BlitFramebuffer",			lua_gles_BlitFramebuffer},

#endif

		{0,0}
	};

	lua_newtable(l);
	luaL_openlib(l, NULL, lib, 0);

#if defined(LUA_GLES_GLES3)
	lua_pushboolean(l,1);
	lua_setfield(l,-2,"GLES3");
#endif

#if defined(LUA_GLES_GLES2)
	lua_pushboolean(l,1);
	lua_setfield(l,-2,"GLES2");
#endif

#if defined(LUA_GLES_GLES3) || defined(LUA_GLES_GLES2) || defined(LUA_GLES_GL)
	lua_pushboolean(l,1);
	lua_setfield(l,-2,"programmable_pipeline_available");
#endif

#if defined(LUA_GLES_GLES1) //|| defined(LUA_GLES_GL)
	lua_pushboolean(l,1);
	lua_setfield(l,-2,"fixed_pipeline_available");
#endif

#if defined(__gl3w_h_)
	gl3wInit();
#endif

	return 1;
}

