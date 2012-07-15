
#include <stdlib.h>

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
//TODO	glClearDepthf(	(float)luaL_checknumber(l,1)	);
	return 0;
}

static int lua_gles_Clear (lua_State *l)
{
	glClear(		(int)luaL_checknumber(l,1)	);
	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// Matrix (gles1)
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
#if defined(LUA_GLES_GLES1) || defined(LUA_GLES_GL)

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
	glGenTextures(1,&id);
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
	glDeleteTextures(1,&id);
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
					(void*)lua_touserdata(l,9)	);
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
					(void*)lua_touserdata(l,9)	);
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

#if defined(LUA_GLES_GLES1) || defined(LUA_GLES_GL)

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
#if defined(LUA_GLES_GLES1) || defined(LUA_GLES_GL)

static int lua_gles_ColorPointer (lua_State *l)
{
	glColorPointer(		(int)luaL_checknumber(l,1)		,
						(int)luaL_checknumber(l,2)		,
						(int)luaL_checknumber(l,3)		,
						((char*)lua_touserdata(l,5))+((int)luaL_checknumber(l,4))	);
	return 0;
}

static int lua_gles_TexCoordPointer (lua_State *l)
{
	glTexCoordPointer(	(int)luaL_checknumber(l,1)		,
						(int)luaL_checknumber(l,2)		,
						(int)luaL_checknumber(l,3)		,
						((char*)lua_touserdata(l,5))+((int)luaL_checknumber(l,4))	);
	return 0;
}

static int lua_gles_NormalPointer (lua_State *l)
{
	glNormalPointer(	(int)luaL_checknumber(l,1)		,
						(int)luaL_checknumber(l,2)		,
						((char*)lua_touserdata(l,4))+((int)luaL_checknumber(l,3))	);
	return 0;
}

static int lua_gles_VertexPointer (lua_State *l)
{
	glVertexPointer(	(int)luaL_checknumber(l,1)		,
						(int)luaL_checknumber(l,2)		,
						(int)luaL_checknumber(l,3)		,
						((char*)lua_touserdata(l,5))+((int)luaL_checknumber(l,4))	);
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
						(void*)lua_touserdata(l,4)	);
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
	glGenBuffers(1,&id);
	lua_pushnumber(l,(double)id);
	return 1;
}
static int lua_gles_DeleteBuffer (lua_State *l)
{
int id=luaL_checknumber(l,1);
	glDeleteBuffers(1,&id);
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
						((char*)lua_touserdata(l,3)),
						(int)luaL_checknumber(l,4));
	return 0;
}
static int lua_gles_BufferSubData (lua_State *l)
{
	glBufferSubData((int)luaL_checknumber(l,1),(int)luaL_checknumber(l,2),(int)luaL_checknumber(l,3),
						((char*)lua_touserdata(l,4))+((int)luaL_checknumber(l,5)) );
	return 0;
}



/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// Stubs2
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
#if defined(LUA_GLES_GLES2) || defined(LUA_GLES_GL)

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
unsigned int pointer;
	index=luaL_checknumber(l,1);
	size=luaL_checknumber(l,2);
	type=luaL_checknumber(l,3);
	normalized=luaL_checknumber(l,4);
	stride=luaL_checknumber(l,5);
	pointer=(unsigned int)luaL_checknumber(l,6); // probably just an offset
	if(lua_isuserdata(l,7))
	{
		pointer+=(unsigned int)lua_touserdata(l,7); // but possibly it might be a real pointer
	}

	glVertexAttribPointer(index,size,type,normalized,stride,(void *)pointer);
	return 0;
}

static int lua_gles_VertexAttrib1f (lua_State *l)
{
int i;
float ff[1];
	i=(int)luaL_checknumber(l,1);
	ff[0]=(float)luaL_checknumber(l,2);
	glVertexAttrib1fv(i,ff);
	return 0;
}
static int lua_gles_VertexAttrib2f (lua_State *l)
{
int i;
float ff[2];
	i=(int)luaL_checknumber(l,1);
	ff[0]=(float)luaL_checknumber(l,2);
	ff[1]=(float)luaL_checknumber(l,3);
	glVertexAttrib2fv(i,ff);
	return 0;
}
static int lua_gles_VertexAttrib3f (lua_State *l)
{
int i;
float ff[3];
	i=(int)luaL_checknumber(l,1);
	ff[0]=(float)luaL_checknumber(l,2);
	ff[1]=(float)luaL_checknumber(l,3);
	ff[2]=(float)luaL_checknumber(l,4);
	glVertexAttrib3fv(i,ff);
	return 0;
}
static int lua_gles_VertexAttrib4f (lua_State *l)
{
int i;
float ff[4];
	i=(int)luaL_checknumber(l,1);
	ff[0]=(float)luaL_checknumber(l,2);
	ff[1]=(float)luaL_checknumber(l,3);
	ff[2]=(float)luaL_checknumber(l,4);
	ff[3]=(float)luaL_checknumber(l,5);
	glVertexAttrib4fv(i,ff);
	return 0;
}

static int lua_gles_Uniform1f (lua_State *l)
{
int i;
float ff[1];
	i=(int)luaL_checknumber(l,1);
	ff[0]=(float)luaL_checknumber(l,2);
	glUniform1fv(i,1,ff);
	return 0;
}
static int lua_gles_Uniform2f (lua_State *l)
{
int i;
float ff[2];
	i=(int)luaL_checknumber(l,1);
	ff[0]=(float)luaL_checknumber(l,2);
	ff[1]=(float)luaL_checknumber(l,3);
	glUniform2fv(i,1,ff);
	return 0;
}
static int lua_gles_Uniform3f (lua_State *l)
{
int i;
float ff[3];
	i=(int)luaL_checknumber(l,1);
	ff[0]=(float)luaL_checknumber(l,2);
	ff[1]=(float)luaL_checknumber(l,3);
	ff[2]=(float)luaL_checknumber(l,4);
	glUniform3fv(i,1,ff);
	return 0;
}
static int lua_gles_Uniform4f (lua_State *l)
{
int i;
float ff[4];
	i=(int)luaL_checknumber(l,1);
	ff[0]=(float)luaL_checknumber(l,2);
	ff[1]=(float)luaL_checknumber(l,3);
	ff[2]=(float)luaL_checknumber(l,4);
	ff[3]=(float)luaL_checknumber(l,5);
//	glUniform4f(i,ff[0],ff[1],ff[2],ff[3]); // this function seems totally fucked??? Gonna asume its a driver bug...
	glUniform4fv(i,1,ff); // so we use this one
	return 0;
}

static int lua_gles_Uniform1i (lua_State *l)
{
int i;
int ii[1];
	i=(int)luaL_checknumber(l,1);
	ii[0]=(int)luaL_checknumber(l,2);
	glUniform1iv(i,1,ii);
	return 0;
}
static int lua_gles_Uniform2i (lua_State *l)
{
int i;
int ii[2];
	i=(int)luaL_checknumber(l,1);
	ii[0]=(int)luaL_checknumber(l,2);
	ii[1]=(int)luaL_checknumber(l,3);
	glUniform2iv(i,1,ii);
	return 0;
}
static int lua_gles_Uniform3i (lua_State *l)
{
int i;
int ii[3];
	i=(int)luaL_checknumber(l,1);
	ii[0]=(int)luaL_checknumber(l,2);
	ii[1]=(int)luaL_checknumber(l,3);
	ii[2]=(int)luaL_checknumber(l,4);
	glUniform3iv(i,1,ii);
	return 0;
}
static int lua_gles_Uniform4i (lua_State *l)
{
int i;
int ii[4];
	i=(int)luaL_checknumber(l,1);
	ii[0]=(int)luaL_checknumber(l,2);
	ii[1]=(int)luaL_checknumber(l,3);
	ii[2]=(int)luaL_checknumber(l,4);
	ii[3]=(int)luaL_checknumber(l,5);
	glUniform4iv(i,1,ii);
	return 0;
}

static int lua_gles_UniformMatrix2f (lua_State *l)
{
int i;
float ff[4];
	for(i=0;i<4;i++)
	{
		lua_pushnumber(l,(double)(i+1));
		lua_gettable(l,2);
		ff[i]=(float)lua_tonumber(l,-1);
		lua_pop(l,1);
	}	
	glUniformMatrix2fv((int)luaL_checknumber(l,1), 1, 0, ff );
	return 0;
}
static int lua_gles_UniformMatrix3f (lua_State *l)
{
int i;
float ff[9];
	for(i=0;i<9;i++)
	{
		lua_pushnumber(l,(double)(i+1));
		lua_gettable(l,2);
		ff[i]=(float)lua_tonumber(l,-1);
		lua_pop(l,1);
	}	
	glUniformMatrix3fv((int)luaL_checknumber(l,1), 1, 0, ff );
	return 0;
}
static int lua_gles_UniformMatrix4f (lua_State *l)
{
int i;
float ff[16];
	for(i=0;i<16;i++)
	{
		lua_pushnumber(l,(double)(i+1));
		lua_gettable(l,2);
		ff[i]=(float)lua_tonumber(l,-1);
		lua_pop(l,1);
	}	
	glUniformMatrix4fv((int)luaL_checknumber(l,1), 1, 0, ff );
	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// more stubs
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_gles_BindAttribLocation (lua_State *l)
{
	glBindAttribLocation(0,0,0);
	return 0;
}


static int lua_gles_GetActiveAttrib (lua_State *l)
{
	glGetActiveAttrib(0,0,0,0,0,0,0);
	return 0;
}
static int lua_gles_GetActiveUniform (lua_State *l)
{
	glGetActiveUniform(0,0,0,0,0,0,0);
	return 0;
}

static int lua_gles_GetBufferParameter (lua_State *l)
{
	glGetBufferParameteriv(0,0,0);
	return 0;
}

static int lua_gles_GetShaderPrecisionFormat (lua_State *l)
{
//TODO	glGetShaderPrecisionFormat(0,0,0,0);
	return 0;
}

static int lua_gles_GetShaderSource (lua_State *l)
{
	glGetShaderSource(0,0,0,0);
	return 0;
}


static int lua_gles_GetUniform (lua_State *l)
{
	glGetUniformfv(0,0,0);
	glGetUniformiv(0,0,0);
	return 0;
}


static int lua_gles_GetVertexAttrib (lua_State *l)
{
	glGetVertexAttribfv(0,0,0);
	glGetVertexAttribiv(0,0,0);
	return 0;
}

static int lua_gles_GetVertexAttribPointer (lua_State *l)
{
	glGetVertexAttribPointerv(0,0,0);
	return 0;
}

static int lua_gles_ReleaseShaderCompiler (lua_State *l)
{
//TODO	glReleaseShaderCompiler();
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
		{"TexSubImage2D",		lua_gles_TexSubImage2D},
		{"TexParameter",		lua_gles_TexParameter},

		{"BlendFunc",			lua_gles_BlendFunc},		
		{"Viewport",			lua_gles_Viewport},
		
		{"DrawArrays",			lua_gles_DrawArrays},
		{"DrawElements",		lua_gles_DrawElements},

		{"GenBuffer",				lua_gles_GenBuffer},
		{"BindBuffer",				lua_gles_BindBuffer},
		{"DeleteBuffer",			lua_gles_DeleteBuffer},
		{"BufferData",				lua_gles_BufferData},
		{"BufferSubData",			lua_gles_BufferSubData},

// fixed pipeline
#if defined(LUA_GLES_GLES1) || defined(LUA_GLES_GL)

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
#if defined(LUA_GLES_GLES2) || defined(LUA_GLES_GL)

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

//		{"UniformMatrix2fv",		lua_gles_UniformMatrix2fv},
//		{"UniformMatrix3fv",		lua_gles_UniformMatrix3fv},
//		{"UniformMatrix4fv",		lua_gles_UniformMatrix4fv},

		{"BindAttribLocation",		lua_gles_BindAttribLocation},
		{"GetActiveAttrib",			lua_gles_GetActiveAttrib},
		{"GetActiveUniform",		lua_gles_GetActiveUniform},
		{"GetBufferParameter",		lua_gles_GetBufferParameter},
		{"GetShaderPrecisionFormat",lua_gles_GetShaderPrecisionFormat},
		{"GetShaderSource",			lua_gles_GetShaderSource},
		{"GetUniform",				lua_gles_GetUniform},
		{"GetVertexAttrib",			lua_gles_GetVertexAttrib},
		{"GetVertexAttribPointer",	lua_gles_GetVertexAttribPointer},
		{"ReleaseShaderCompiler",	lua_gles_ReleaseShaderCompiler},

#endif

		{0,0}
	};

	lua_newtable(l);
	luaL_openlib(l, NULL, lib, 0);

#if defined(LUA_GLES_GLES2) || defined(LUA_GLES_GL)
	lua_pushboolean(l,1);
	lua_setfield(l,-2,"programmable_pipeline_available");
#endif

#if defined(LUA_GLES_GLES1) || defined(LUA_GLES_GL)
	lua_pushboolean(l,1);
	lua_setfield(l,-2,"fixed_pipeline_available");
#endif
	
	return 1;
}

