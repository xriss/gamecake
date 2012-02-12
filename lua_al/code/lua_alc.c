
#include "lua.h"
#include "lauxlib.h"

#include <AL/al.h>
#include <AL/alc.h>

//
// we can use either these strings as a string identifier
// or the address as a light userdata identifier, both unique
//
const char *lua_alc_device_meta_name="alc*device*ptr";
const char *lua_alc_context_meta_name="alc*context*ptr";




/*
alcCreateContext
alcMakeContextCurrent
alcProcessContext
alcSuspendContext
alcDestroyContext
alcGetCurrentContext
alcGetContextsDevice
alcOpenDevice
alcCloseDevice
alcGetError
alcIsExtensionPresent
alcGetProcAddress
alcGetEnumValue
alcGetString
alcGetIntegerv
alcCaptureOpenDevice
alcCaptureCloseDevice
alcCaptureStart
alcCaptureStop
alcCaptureSamples
*/

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// simple test...
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_alc_test(lua_State *l)
{
int format=AL_FORMAT_MONO16;
short data[512];
int size=512*2;
int freq=1024*10;
int i;

for(i=0;i<512;i++)
{
	data[i]=(short)random();
}

	
	
int buffer;
	
//ALCdevice* device = alcOpenDevice(NULL);
//ALCcontext* context = alcCreateContext(device, NULL);
//alcMakeContextCurrent(context);

alListener3f(AL_POSITION, 0, 0, 0);
alListener3f(AL_VELOCITY, 0, 0, 0);
alListener3f(AL_ORIENTATION, 0, 0, -1);

ALuint source;
alGenSources(1, &source);

alSourcef(source, AL_PITCH, 1);
alSourcef(source, AL_GAIN, 1);
alSource3f(source, AL_POSITION, 0, 0, 0);
alSource3f(source, AL_VELOCITY, 0, 0, 0);
alSourcei(source, AL_LOOPING, AL_FALSE);

alGenBuffers(1, &buffer);

alBufferData(buffer,format,data,size,freq);


alSourcei(source, AL_BUFFER, buffer);
alSourcei(source, AL_LOOPING,AL_TRUE);

alSourcePlay(source);
fgetc(stdin);


alDeleteSources(1, &source);
alDeleteBuffers(1, &buffer);
//alcDestroyContext(context);
//alcCloseDevice(device);



	
  return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// open device
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_alc_OpenDevice(lua_State *l)
{
	ALCdevice **device;

// create a device userdata pointer pointer
	device = (ALCdevice**)lua_newuserdata(l, sizeof(ALCdevice**));	
	luaL_getmetatable(l, lua_alc_device_meta_name);
	lua_setmetatable(l, -2);
	
//start pointer at 0
	(*device)=0;

//open the actual device
	(*device)=alcOpenDevice(NULL);

//return the userdata	
	return 1;
}
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// get **device and error if it is not the right udata
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
ALCdevice ** lua_alc_get_device_ptr (lua_State *l,int idx)
{
ALCdevice **device;
	device = (ALCdevice**)luaL_checkudata(l, idx , lua_alc_device_meta_name);
	return device;
}
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// get *device and error if it is 0
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
ALCdevice * lua_alc_check_device (lua_State *l,int idx)
{	
ALCdevice **device;
	device = lua_alc_get_device_ptr (l, idx);
	if(!*device)
	{
		luaL_error(l,"alc device is null");
	}
	return *device;
}
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// __GC for device ptr (may be null)
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_alc_CloseDevice (lua_State *l)
{	
ALCdevice **device;

	device = lua_alc_get_device_ptr(l, 1 );
	
	if(*device)
	{
		alcCloseDevice(*device);
		(*device)=0;
	}
	
	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// open device
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_alc_CreateContext(lua_State *l)
{
	ALCdevice *device;
	ALCcontext **context;

// must pass in a device
	device = lua_alc_check_device(l, 1);

// create a context userdata pointer pointer
	context = (ALCcontext**)lua_newuserdata(l, sizeof(ALCcontext**));	
	luaL_getmetatable(l, lua_alc_context_meta_name);
	lua_setmetatable(l, -2);
	
//start pointer at 0
	(*context)=0;

//open the actual context
	(*context)=alcCreateContext(device,NULL);

//return the userdata	
	return 1;
}
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// get **context and error if it is not the right udata
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
ALCcontext ** lua_alc_get_context_ptr (lua_State *l,int idx)
{
ALCcontext **context;
	context = (ALCcontext**)luaL_checkudata(l, idx , lua_alc_context_meta_name);
	return context;
}
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// get *context and error if it is 0
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
ALCcontext * lua_alc_check_context (lua_State *l,int idx)
{	
ALCcontext **context;
	context = lua_alc_get_context_ptr (l,idx);
	if(!*context)
	{
		luaL_error(l,"alc device is null");
	}
	return *context;
}
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// __GC for context ptr (may be null)
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_alc_DestroyContext (lua_State *l)
{	
ALCcontext **context;

	context = lua_alc_get_context_ptr(l, 1 );
	
	if(*context)
	{
		alcDestroyContext(*context);
		(*context)=0;
	}
	
	return 0;
}
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// Select the current context
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_alc_MakeContextCurrent (lua_State *l)
{	
ALCcontext *context;

	context = lua_alc_check_context(l, 1);
	
	alcMakeContextCurrent(context);
	
	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// open library.
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
LUALIB_API int luaopen_alc_core(lua_State *l)
{
	const luaL_reg lib[] =
	{
		{"OpenDevice",			lua_alc_OpenDevice},
		{"CloseDevice",			lua_alc_CloseDevice},

		{"CreateContext",		lua_alc_CreateContext},
		{"DestroyContext",		lua_alc_DestroyContext},
		
		{"MakeContextCurrent",	lua_alc_MakeContextCurrent},

		{"test",				lua_alc_test},
		{0,0}
	};
	const luaL_reg meta_device[] =
	{
		{"__gc",			lua_alc_CloseDevice},
		{0,0}
	};
	const luaL_reg meta_context[] =
	{
		{"__gc",			lua_alc_DestroyContext},
		{0,0}
	};

	luaL_newmetatable(l, lua_alc_device_meta_name);
	luaL_openlib(l, NULL, meta_device, 0);
	lua_pop(l,1);
		
	luaL_newmetatable(l, lua_alc_context_meta_name);
	luaL_openlib(l, NULL, meta_context, 0);
	lua_pop(l,1);

	lua_newtable(l);
	luaL_openlib(l, NULL, lib, 0);
	return 1;
}

