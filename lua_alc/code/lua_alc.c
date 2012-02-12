
#include "lua.h"
#include "lauxlib.h"

#include <AL/al.h>
#include <AL/alc.h>


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
	
ALCdevice* device = alcOpenDevice(NULL);
ALCcontext* context = alcCreateContext(device, NULL);
alcMakeContextCurrent(context);

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
alcDestroyContext(context);
alcCloseDevice(device);



	
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
		{"test",			lua_alc_test},

		{0,0}
	};

	lua_newtable(l);
	luaL_openlib(l, NULL, lib, 0);
	
	return 1;
}

