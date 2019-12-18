
#include "lua.h"
#include "lauxlib.h"

#include <AL/al.h>
#include <AL/alc.h>

#if defined(EMCC)
// emscripten does not have all the damn functions...
#define alGetFloatv(a,b)
#define alGetListeneriv alGetListeneri
#define alGetListenerfv alGetListenerf
#define alGetBufferiv alGetBufferi
#define alGetSourceiv alGetSourcei
#define alSourceiv(a,b,c) alSourcei(a,b,*c)
#define alGetIntegerv(a,b) /* (*b=alGetInteger(a)) */
#define alListeneriv(a,b) /* alListeneri(a,*b) */
#define alBufferfv(a,b,c) /* alBufferf(a,b,*c) */
#define alBufferiv(a,b,c) /* alBufferi(a,b,*c) */
#define alGetBufferfv(a,b,c) /* alGetBufferf(a,b,*c) */
#endif

/*
alEnable
alDisable
alIsEnabled
alGetString
alGetBooleanv
alGetIntegerv
alGetFloatv
alGetDoublev
alGetBoolean
alGetInteger
alGetFloat
alGetDouble
alGetError
alIsExtensionPresent
alGetProcAddress
alGetEnumValue
alListenerf
alListener3f
alListenerfv
alListeneri
alListener3i
alListeneriv
alGetListenerf
alGetListener3f
alGetListenerfv
alGetListeneri
alGetListener3i
alGetListeneriv
alGenSources
alDeleteSources
alIsSource
alSourcef
alSource3f
alSourcefv
alSourcei
alSource3i
alSourceiv
alGetSourcef
alGetSource3f
alGetSourcefv
alGetSourcei
alGetSource3i
alGetSourceiv
alSourcePlayv
alSourceStopv
alSourceRewindv
alSourcePausev
alSourcePlay
alSourceStop
alSourceRewind
alSourcePause
alSourceQueueBuffers
alSourceUnqueueBuffers
alGenBuffers
alDeleteBuffers
alIsBuffer
alBufferf
alBuffer3f
alBufferfv
alBufferi
alBuffer3i
alBufferiv
alGetBufferf
alGetBuffer3f
alGetBufferfv
alGetBufferi
alGetBuffer3i
alGetBufferiv
alDopplerFactor
alDopplerVelocity
alSpeedOfSound
alDistanceModel
*/


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// generate one buffer
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_al_GenBuffer (lua_State *l)
{	
int buff;
	alGenBuffers(1,(unsigned int *)&buff);
	lua_pushnumber(l,buff);
	return 1;
}
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// delete one buffer
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_al_DeleteBuffer (lua_State *l)
{	
int buff;
	buff=luaL_checknumber(l,1);
	alDeleteBuffers(1,(unsigned int *)&buff);
	return 0;
}
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// Load some data into a buffer
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_al_BufferData (lua_State *l)
{
int buff,fmt,freq;
size_t size;
void *data;

	buff=luaL_checknumber(l,1);
	fmt=luaL_checknumber(l,2);
	if(lua_isstring(l,3))
	{
		data=(void*)luaL_checklstring(l,3,&size);
	}
	else
	if(lua_isuserdata(l,3))
	{
		data=(void*)lua_touserdata(l,3);
	}
	else
	if(lua_islightuserdata(l,3))
	{
		data=(void*)lua_touserdata(l,3);
	}
	size=(size_t)luaL_checknumber(l,4);
	freq=luaL_checknumber(l,5);

	alBufferData(buff,fmt,data,(int)size,freq);
	
	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// generate one source
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_al_GenSource (lua_State *l)
{	
int src;
	alGenSources(1,(unsigned int *)&src);
	lua_pushnumber(l,src);
	return 1;
}
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// delete one source
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_al_DeleteSource (lua_State *l)
{	
int src;
	src=luaL_checknumber(l,1);
	alDeleteSources(1,(unsigned int *)&src);
	return 0;
}
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// Play pause rewind or stop a source
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_al_SourcePlay (lua_State *l)
{
int src=luaL_checknumber(l,1);
	alSourcePlay(src);
	return 0;
}
static int lua_al_SourcePause (lua_State *l)
{
int src=luaL_checknumber(l,1);
	alSourcePause(src);
	return 0;
}
static int lua_al_SourceRewind (lua_State *l)
{
int src=luaL_checknumber(l,1);
	alSourceRewind(src);
	return 0;
}
static int lua_al_SourceStop (lua_State *l)
{
int src=luaL_checknumber(l,1);
	alSourceStop(src);
	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// for streaming, normally we just feed in one buffer at a time anyway so might as well force
// multiple calls for multiple buffers, we can fake the buffers array in lua and this is not
// going to be a performance issue, probably :)
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_al_SourceQueueBuffer (lua_State *l)
{
int src=luaL_checknumber(l,1);
int buf=luaL_checknumber(l,2);
	alSourceQueueBuffers(src,1,(unsigned int *)&buf);
	return 0;
}
static int lua_al_SourceUnqueueBuffer (lua_State *l)
{
int src=luaL_checknumber(l,1);
int buf=luaL_checknumber(l,2);
	alSourceUnqueueBuffers(src,1,(unsigned int *)&buf);
	lua_pushnumber(l,buf);
	return 1;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// turn a def into a property type and size
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void lua_al_get_prop_info (int def, char *v, int *num)
{
	switch(def)
	{
// int[1]
		case AL_SOURCE_RELATIVE:
		case AL_SOURCE_TYPE:
		case AL_LOOPING:
		case AL_BUFFER:
		case AL_SOURCE_STATE:
		case AL_BUFFERS_QUEUED:
		case AL_BUFFERS_PROCESSED:
		case AL_SAMPLE_OFFSET:
		case AL_BYTE_OFFSET:
		case AL_FREQUENCY:
		case AL_BITS:
		case AL_CHANNELS:
		case AL_SIZE:
		case AL_DISTANCE_MODEL:
			*num=1; *v='i';
		break;
// float[1]
		case AL_GAIN:
		case AL_PITCH:
		case AL_MAX_DISTANCE:
		case AL_ROLLOFF_FACTOR:
		case AL_REFERENCE_DISTANCE:
		case AL_MIN_GAIN:
		case AL_MAX_GAIN:
		case AL_CONE_OUTER_GAIN:
		case AL_CONE_INNER_ANGLE:
		case AL_CONE_OUTER_ANGLE:
		case AL_SEC_OFFSET:
		case AL_DOPPLER_FACTOR:
		case AL_SPEED_OF_SOUND:
			*num=1; *v='f';
		break;
// float[3]
		case AL_POSITION:
		case AL_VELOCITY:
		case AL_DIRECTION:
			*num=3; *v='f';
		break;
// float[6]
		case AL_ORIENTATION:
			*num=6; *v='f';
		break;
// const char *
		case AL_VENDOR:
		case AL_RENDERER:
		case AL_EXTENSIONS:
			*num=1; *v='s';
		break;
	}
}



/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// get or set listener properties
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_al_Listener (lua_State *l,int get)
{	
int def;

// default of a single integer
char v='i';
int num=1;

int iv[16];
float fv[16];

int i;

	def=luaL_checknumber(l,1);
	
	lua_al_get_prop_info(def, &v, &num);
	
	switch(v)
	{
		case 'i':
			if(get)
			{
				alGetListeneriv(def,iv);
				for(i=0;i<num;i++)
				{
					lua_pushnumber(l,iv[i]);
				}
			}
			else
			{
				for(i=0;i<num;i++)
				{
					iv[i]=(int)luaL_checknumber(l,2+i);
				}
				alListeneriv(def,iv);
			}
		break;
		case 'f':
			if(get)
			{
				alGetListenerfv(def,fv);
				for(i=0;i<num;i++)
				{
					lua_pushnumber(l,fv[i]);
				}
			}
			else
			{
				for(i=0;i<num;i++)
				{
					fv[i]=(float)luaL_checknumber(l,2+i);
				}
				alListenerfv(def,fv);
			}
		break;
	}
	
	if(get)	{ return num; }
	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// get or set Source properties
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_al_Source (lua_State *l,int get)
{
int src;
int def;

// default of a single integer
char v='i';
int num=1;

int iv[16];
float fv[16];

int i;

	src=luaL_checknumber(l,1);
	def=luaL_checknumber(l,2);
	
	lua_al_get_prop_info(def, &v, &num);
	
	switch(v)
	{
		case 'i':
			if(get)
			{
				alGetSourceiv(src,def,iv);
				for(i=0;i<num;i++)
				{
					lua_pushnumber(l,iv[i]);
				}
			}
			else
			{
				for(i=0;i<num;i++)
				{
					iv[i]=(int)luaL_checknumber(l,3+i);
				}
				alSourceiv(src,def,iv);
			}
		break;
		case 'f':
			if(get)
			{
				alGetSourcefv(src,def,fv);
				for(i=0;i<num;i++)
				{
					lua_pushnumber(l,fv[i]);
				}
			}
			else
			{
				for(i=0;i<num;i++)
				{
					fv[i]=(float)luaL_checknumber(l,3+i);
				}
				alSourcefv(src,def,fv);
			}
		break;
	}
	
	if(get)	{ return num; }
	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// get or set Buffer properties
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_al_Buffer (lua_State *l,int get)
{	
int buff;
int def;

// default of a single integer
char v='i';
int num=1;

int iv[16];
float fv[16];

int i;

	buff=luaL_checknumber(l,1);
	def=luaL_checknumber(l,2);
	
	lua_al_get_prop_info(def, &v, &num);
	
	switch(v)
	{
		case 'i':
			if(get)
			{
				alGetBufferiv(buff,def,iv);
				for(i=0;i<num;i++)
				{
					lua_pushnumber(l,iv[i]);
				}
			}
			else
			{
				for(i=0;i<num;i++)
				{
					iv[i]=(int)luaL_checknumber(l,2+i);
				}
				alBufferiv(buff,def,iv);
			}
		break;
		case 'f':
			if(get)
			{
				alGetBufferfv(buff,def,fv);
				for(i=0;i<num;i++)
				{
					lua_pushnumber(l,fv[i]);
				}
			}
			else
			{
				for(i=0;i<num;i++)
				{
					fv[i]=(float)luaL_checknumber(l,2+i);
				}
				alBufferfv(buff,def,fv);
			}
		break;
	}
	
	if(get)	{ return num; }
	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// get error number
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_al_GetError (lua_State *l)
{
	lua_pushnumber(l,alGetError());
	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// get base properties
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_al_Get (lua_State *l)
{	
int def;

// default of a single integer
char v='i';
int num=1;

int iv[16];
float fv[16];

int i;

	def=luaL_checknumber(l,1);
	
	lua_al_get_prop_info(def, &v, &num);
	
	switch(v)
	{
		case 'i':
			alGetIntegerv(def,iv);
			for(i=0;i<num;i++)
			{
				lua_pushnumber(l,iv[i]);
			}
		break;
		case 'f':
			alGetFloatv(def,fv);
			for(i=0;i<num;i++)
			{
				lua_pushnumber(l,fv[i]);
			}
		break;
		case 's':
			for(i=0;i<num;i++) // this should only ever be 1
			{
				lua_pushstring(l, alGetString(def) );
			}
		break;
	}
	
	return num;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// Property stubs
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_al_GetListener (lua_State *l)
{	
	return lua_al_Listener(l,1);
}
static int lua_al_SetListener (lua_State *l)
{	
	return lua_al_Listener(l,0);
}
static int lua_al_GetSource (lua_State *l)
{	
	return lua_al_Source(l,1);
}
static int lua_al_SetSource (lua_State *l)
{	
	return lua_al_Source(l,0);
}
static int lua_al_GetBuffer (lua_State *l)
{	
	return lua_al_Buffer(l,1);
}
static int lua_al_SetBuffer (lua_State *l)
{	
	return lua_al_Buffer(l,0);
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// open library.
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
LUALIB_API int luaopen_al_core(lua_State *l)
{
	const luaL_Reg lib[] =
	{
		{"Get",					lua_al_Get},
		{"GetError",			lua_al_GetError},
		
		{"Listener",			lua_al_SetListener},
		{"GetListener",			lua_al_GetListener},

		{"GenSource",			lua_al_GenSource},
		{"DeleteSource",		lua_al_DeleteSource},
		{"Source",				lua_al_SetSource},
		{"GetSource",			lua_al_GetSource},
		{"SourcePlay",			lua_al_SourcePlay},
		{"SourcePause",			lua_al_SourcePause},
		{"SourceRewind",		lua_al_SourceRewind},
		{"SourceStop",			lua_al_SourceStop},
		{"SourceQueueBuffer",	lua_al_SourceQueueBuffer},
		{"SourceUnqueueBuffer",	lua_al_SourceUnqueueBuffer},

		{"GenBuffer",			lua_al_GenBuffer},
		{"DeleteBuffer",		lua_al_DeleteBuffer},
		{"Buffer",				lua_al_SetBuffer},
		{"GetBuffer",			lua_al_GetBuffer},
		{"BufferData",			lua_al_BufferData},
		{0,0}
	};

	lua_newtable(l);
	luaL_openlib(l, NULL, lib, 0);
	
	return 1;
}

