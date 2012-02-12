
#include "lua.h"
#include "lauxlib.h"

#include <AL/al.h>
#include <AL/alc.h>


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
// simple test...
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_al_test(lua_State *l)
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
LUALIB_API int luaopen_al_core(lua_State *l)
{
	const luaL_reg lib[] =
	{
		{"test",			lua_al_test},

		{0,0}
	};

	lua_newtable(l);
	luaL_openlib(l, NULL, lib, 0);
	
	return 1;
}

