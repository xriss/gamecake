/*
-- Copyright (C) 2012 Kriss Blank < Kriss@XIXs.com >
-- This file is distributed under the terms of the MIT license.
-- http://en.wikipedia.org/wiki/MIT_License
-- Please ping me if you use it for anything cool...
*/

#include "all.h"

//
// SOD sound object data handling layer, load/save/manipulate
//



/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// allocate the structure, and then the data, seperated because we expect to reuse the struct with
// diferent data
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
sod * sod_alloc()
{
	return (sod *) calloc( sizeof(sod) , 1 );
}
sod * sod_alloc_data(sod *sd,s32 fmt,s32 samples)
{
	sod_free_data(sd); // free old data if we have any
	
	sod_set_fmt(sd,fmt); // this fills in fmt, sample_size and chanels
	sd->samples=samples;
	sd->data_sizeof=( sd->sample_size * sd->samples * sd->chanels );
	sd->data=(u8*)calloc(sd->data_sizeof,1);
	if(!sd->data) { return 0; } //fail
	return sd; //success
}
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// free the structure and or the data
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void sod_free(sod *sd)
{
	if(sd)
	{
		sod_free_data(sd);
		free(sd);
	}
}
void sod_free_data(sod *sd)
{
	if(sd->data)
	{
		free(sd->data);
		sd->data=0;
	}
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// set fmt and chanels and sample_size
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void sod_set_fmt( sod * sd, s32 fmt )
{
	sd->fmt=fmt;
	
	switch(fmt)
	{
		case SOD_FMT_STEREO8:
		case SOD_FMT_STEREO16:
			sd->chanels=2;
		default:
			sd->chanels=1;
		break;
	}

	switch(fmt)
	{
		case SOD_FMT_MONO8:
		case SOD_FMT_STEREO8:
			sd->sample_size=1;
		default:
			sd->sample_size=2;
		break;
	}
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// load an image into a sod
// returns 0 on error
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
sod * sod_load_file( sod * sd, const char *filename , const char *opts )
{
	sod_wav_load_file(sd,filename);

	if(sd->err) { return 0; }
	return sd;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// load an image into a sod
// returns 0 on error
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
sod * sod_load_data( sod * sd, const unsigned char *data , int len, const char *opts )
{
	sod_wav_load_data(sd,data,len);

	if(sd->err) { return 0; }
	return sd;
}


#if 0

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss@XIXs.com 2013
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
#include "all.h"


struct sfx SFX[1]={{0}};


#if !defined( WET_DISABLE_OPENAL )


#if defined _MSC_VER
	#pragma pack (push,1) 							/* Turn off alignment */
#elif defined __GNUC__
	#define PADOFF_VAR __attribute__((packed))
#endif

#ifndef PADOFF_VAR
	#define PADOFF_VAR
#endif

typedef struct                                  /* WAV File-header */
{
  ALubyte  Id[4]			PADOFF_VAR;
  ALsizei  Size				PADOFF_VAR;
  ALubyte  Type[4]			PADOFF_VAR;
} WAVFileHdr_Struct;

typedef struct                                  /* WAV Fmt-header */
{
  ALushort Format			PADOFF_VAR;
  ALushort Channels			PADOFF_VAR;
  ALuint   SamplesPerSec	PADOFF_VAR;
  ALuint   BytesPerSec		PADOFF_VAR;
  ALushort BlockAlign		PADOFF_VAR;
  ALushort BitsPerSample	PADOFF_VAR;
} WAVFmtHdr_Struct;

typedef struct									/* WAV FmtEx-header */
{
  ALushort Size				PADOFF_VAR;
  ALushort SamplesPerBlock	PADOFF_VAR;
} WAVFmtExHdr_Struct;

typedef struct                                  /* WAV Smpl-header */
{
  ALuint   Manufacturer		PADOFF_VAR;
  ALuint   Product			PADOFF_VAR;
  ALuint   SamplePeriod		PADOFF_VAR;
  ALuint   Note				PADOFF_VAR;
  ALuint   FineTune			PADOFF_VAR;
  ALuint   SMPTEFormat		PADOFF_VAR;
  ALuint   SMPTEOffest		PADOFF_VAR;
  ALuint   Loops			PADOFF_VAR;
  ALuint   SamplerData		PADOFF_VAR;
  struct
  {
    ALuint Identifier		PADOFF_VAR;
    ALuint Type				PADOFF_VAR;
    ALuint Start			PADOFF_VAR;
    ALuint End				PADOFF_VAR;
    ALuint Fraction			PADOFF_VAR;
    ALuint Count			PADOFF_VAR;
  }      Loop[1]			PADOFF_VAR;
} WAVSmplHdr_Struct;

typedef struct                                  /* WAV Chunk-header */
{
  ALubyte  Id[4]			PADOFF_VAR;
  ALuint   Size				PADOFF_VAR;
} WAVChunkHdr_Struct;


#ifdef PADOFF_VAR			    				/* Default alignment */
	#undef PADOFF_VAR
#endif

#if defined _MSC_VER
	#pragma pack (pop)
#endif


void alutLoadWAVFile(ALbyte *file,ALenum *format,ALvoid **data,ALsizei *size,ALsizei *freq, ALboolean *loop)
{
	WAVChunkHdr_Struct ChunkHdr;
	WAVFmtExHdr_Struct FmtExHdr;
	WAVFileHdr_Struct FileHdr;
	WAVSmplHdr_Struct SmplHdr;
	WAVFmtHdr_Struct FmtHdr;
	FILE *Stream;
	
	*format=AL_FORMAT_MONO16;
	*data=NULL;
	*size=0;
	*freq=22050;
	*loop=AL_FALSE;
	if (file)
	{
		Stream=fopen(file,"rb");
		if (Stream)
		{
			fread(&FileHdr,1,sizeof(WAVFileHdr_Struct),Stream);
			FileHdr.Size=((FileHdr.Size+1)&~1)-4;
			while ((FileHdr.Size!=0)&&(fread(&ChunkHdr,1,sizeof(WAVChunkHdr_Struct),Stream)))
			{
				if (!memcmp(ChunkHdr.Id,"fmt ",4))
				{
					fread(&FmtHdr,1,sizeof(WAVFmtHdr_Struct),Stream);
					if (FmtHdr.Format==0x0001)
					{
						*format=(FmtHdr.Channels==1?
								(FmtHdr.BitsPerSample==8?AL_FORMAT_MONO8:AL_FORMAT_MONO16):
								(FmtHdr.BitsPerSample==8?AL_FORMAT_STEREO8:AL_FORMAT_STEREO16));
						*freq=FmtHdr.SamplesPerSec;
						fseek(Stream,ChunkHdr.Size-sizeof(WAVFmtHdr_Struct),SEEK_CUR);
					} 
					else
					{
						fread(&FmtExHdr,1,sizeof(WAVFmtExHdr_Struct),Stream);
						fseek(Stream,ChunkHdr.Size-sizeof(WAVFmtHdr_Struct)-sizeof(WAVFmtExHdr_Struct),SEEK_CUR);
					}
				}
				else if (!memcmp(ChunkHdr.Id,"data",4))
				{
					if (FmtHdr.Format==0x0001)
					{
						*size=ChunkHdr.Size;
						*data=malloc(ChunkHdr.Size+31);
						if (*data) fread(*data,FmtHdr.BlockAlign,ChunkHdr.Size/FmtHdr.BlockAlign,Stream);
						memset(((char *)*data)+ChunkHdr.Size,0,31);
					}
					else if (FmtHdr.Format==0x0011)
					{
						//IMA ADPCM
					}
					else if (FmtHdr.Format==0x0055)
					{
						//MP3 WAVE
					}
				}
				else if (!memcmp(ChunkHdr.Id,"smpl",4))
				{
					fread(&SmplHdr,1,sizeof(WAVSmplHdr_Struct),Stream);
					*loop = (SmplHdr.Loops ? AL_TRUE : AL_FALSE);
					fseek(Stream,ChunkHdr.Size-sizeof(WAVSmplHdr_Struct),SEEK_CUR);
				}
				else fseek(Stream,ChunkHdr.Size,SEEK_CUR);
				fseek(Stream,ChunkHdr.Size&1,SEEK_CUR);
				FileHdr.Size-=(((ChunkHdr.Size+1)&~1)+8);
			}
			fclose(Stream);
		}
		
	}
}

void alutLoadWAVMemory(ALbyte *memory,ALenum *format,ALvoid **data,ALsizei *size,ALsizei *freq,ALboolean *loop)
{
	WAVChunkHdr_Struct ChunkHdr;
	WAVFmtExHdr_Struct FmtExHdr;
	WAVFileHdr_Struct FileHdr;
	WAVSmplHdr_Struct SmplHdr;
	WAVFmtHdr_Struct FmtHdr;
	ALbyte *Stream;
	
	*format=AL_FORMAT_MONO16;
	*data=NULL;
	*size=0;
	*freq=22050;
	*loop=AL_FALSE;
	if (memory)
	{
		Stream=memory;
		if (Stream)
		{
			memcpy(&FileHdr,Stream,sizeof(WAVFileHdr_Struct));
			Stream+=sizeof(WAVFileHdr_Struct);
			FileHdr.Size=((FileHdr.Size+1)&~1)-4;
			while ((FileHdr.Size!=0)&&(memcpy(&ChunkHdr,Stream,sizeof(WAVChunkHdr_Struct))))
			{
				Stream+=sizeof(WAVChunkHdr_Struct);
				if (!memcmp(ChunkHdr.Id,"fmt ",4))
				{
					memcpy(&FmtHdr,Stream,sizeof(WAVFmtHdr_Struct));
					if (FmtHdr.Format==0x0001)
					{
						*format=(FmtHdr.Channels==1?
								(FmtHdr.BitsPerSample==8?AL_FORMAT_MONO8:AL_FORMAT_MONO16):
								(FmtHdr.BitsPerSample==8?AL_FORMAT_STEREO8:AL_FORMAT_STEREO16));
						*freq=FmtHdr.SamplesPerSec;
						Stream+=ChunkHdr.Size;
					} 
					else
					{
						memcpy(&FmtExHdr,Stream,sizeof(WAVFmtExHdr_Struct));
						Stream+=ChunkHdr.Size;
					}
				}
				else if (!memcmp(ChunkHdr.Id,"data",4))
				{
					if (FmtHdr.Format==0x0001)
					{
						*size=ChunkHdr.Size;
						*data=malloc(ChunkHdr.Size+31);
						if (*data) memcpy(*data,Stream,ChunkHdr.Size);
						memset(((char *)*data)+ChunkHdr.Size,0,31);
						Stream+=ChunkHdr.Size;
					}
					else if (FmtHdr.Format==0x0011)
					{
						//IMA ADPCM
					}
					else if (FmtHdr.Format==0x0055)
					{
						//MP3 WAVE
					}
				}
				else if (!memcmp(ChunkHdr.Id,"smpl",4))
				{
					memcpy(&SmplHdr,Stream,sizeof(WAVSmplHdr_Struct));
					*loop = (SmplHdr.Loops ? AL_TRUE : AL_FALSE);
					Stream+=ChunkHdr.Size;
				}
				else Stream+=ChunkHdr.Size;
				Stream+=ChunkHdr.Size&1;
				FileHdr.Size-=(((ChunkHdr.Size+1)&~1)+8);
			}
		}
	}
}



void alutUnloadWAV(ALenum format,ALvoid *data,ALsizei size,ALsizei freq)
{
	if (data)
		free(data);
}




size_t ovCB_read(void *buf,unsigned int a,unsigned int b,void * fp) {
   return fread(buf,a,b,(FILE *)fp);
}

int ovCB_close(void * fp) {
   return fclose((FILE *)fp);
}

int ovCB_seek(void *fp,__int64 a,int b) {
   return fseek((FILE *)fp,(long)a,b);
}

long ovCB_tell(void *fp) {
   return ftell((FILE *)fp);
}


ov_callbacks ovCB={ovCB_read,ovCB_seek,ovCB_close,ovCB_tell};




/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// Setup junk
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
bool sfx_stream::setup(void)
{
bool check;

	DMEM_ZERO(this);

	// Generate Buffers
	OALCHECK( check , alGenBuffers(NUMOF_STREAM_BUFFERS,(ALuint*)Buffers) ); if(!check) { goto bogus; }

	// Generate sources
	OALCHECK( check , alGenSources(1,(ALuint*)Sources) ); if(!check) { goto bogus; }

	return(true);
bogus:
	clean();
	return(false);
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// Clean junk
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void sfx_stream::clean(void)
{
#define RELEASE_THIS(ptr) if(ptr) { ptr->Release(); ptr=0; }

bool check;

    ov_clear(&oggStream);

	OALCHECK( check , alDeleteSources(1,(ALuint*)Sources) );

	OALCHECK( check , alDeleteBuffers(NUMOF_STREAM_BUFFERS,(ALuint*)Buffers) );


#undef  RELEASE_THIS
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// update
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void sfx_stream::update(void)
{
bool check;
ALuint buff;
ALint processed;

	if(!oggFile) return;

	if( added < NUMOF_STREAM_BUFFERS ) // initial buffer adds
	{
		processed=NUMOF_STREAM_BUFFERS-added;

		buff=Buffers[added++];
	}
	else
	{
	    OALCHECK( check , alGetSourcei(Sources[0], AL_BUFFERS_PROCESSED, &processed) );

		if( processed )
		{
	        OALCHECK( check , alSourceUnqueueBuffers(Sources[0], 1, &buff) );
		}
	}

// stream file data into buffer

	if(processed)
	{
		char pcm[SIZEOF_STREAM_BUFFERS];
		int  size = 0;
		int  section;
		int  result;

		while(size < SIZEOF_STREAM_BUFFERS)
		{
			result = ov_read(&oggStream, pcm + size, SIZEOF_STREAM_BUFFERS - size, 0, 2, 1, &section);
    
			if(result > 0)
				size += result;
			else
				if(result < 0)
					return; // error
				else // EOF
				{
					ov_pcm_seek(&oggStream,0);
				}
		}
    
		if(size == 0) return; // error
        
		OALCHECK( check , alBufferData(buff, format, pcm, size, vorbisInfo->rate) );

        OALCHECK( check , alSourceQueueBuffers(Sources[0], 1, &buff) );

	}

	{
    ALenum state;
    
	    OALCHECK( check , alGetSourcei(Sources[0], AL_SOURCE_STATE, &state) );
    
		if(state != AL_PLAYING) // sound has stalled
		{
			if(processed<=1) // start again when we have filled up the buffers
			{
			    OALCHECK( check , alSourcePlay(Sources[0]) );
			}
		}
	}

}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// play a file
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void sfx_stream::play(const char *path)
{
bool check;

	if( ( !OPT->sfx_enabled ) || ( OPT->sfx_background_volume==0.0f ) ) return; // don't play

	if(path) // play a file
	{
		int result;

		if(oggFile) // stop old file
		{
			OALCHECK( check , alSourceStop(Sources[0]) );
			ov_clear(&oggStream);
			oggFile=0;
		}
    
		if(!(oggFile = fopen(path, "rb")))
		{
			DBG_Error("Failed to open file \"%s\"\n",path);
			return;
		}

		if( ( result = ov_open_callbacks(oggFile,&oggStream,NULL,0,ovCB) ) < 0)
		{
			fclose(oggFile);
			oggFile=0;
        
			DBG_Error("Failed to open ogg \"%s\"\n",path);
			return;
		}

		vorbisInfo = ov_info(&oggStream, -1);
		vorbisComment = ov_comment(&oggStream, -1);

		if(vorbisInfo->channels == 1)
			format = AL_FORMAT_MONO16;
		else
			format = AL_FORMAT_STEREO16;
		
		OALCHECK( check , alSource3f(Sources[0], AL_POSITION,        0.0, 0.0, 0.0) );
		OALCHECK( check , alSource3f(Sources[0], AL_VELOCITY,        0.0, 0.0, 0.0) );
		OALCHECK( check , alSource3f(Sources[0], AL_DIRECTION,       0.0, 0.0, 0.0) );
		OALCHECK( check , alSourcef (Sources[0], AL_ROLLOFF_FACTOR,  0.0          ) );
		OALCHECK( check , alSourcei (Sources[0], AL_SOURCE_RELATIVE, AL_TRUE      ) );

		OALCHECK( check , alSourcef (Sources[0],AL_GAIN,OPT->sfx_background_volume) );

//	    alSourcePlay(Sources[0]);
	}
	else
	{
		OALCHECK( check , alSourceStop(Sources[0]) );
	    ov_clear(&oggStream);
		oggFile=0;
	}


}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// Setup junk
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
bool sfx::setup(void)
{
bool check;
s32 i;

	DMEM_ZERO(this);

#if defined(WET_DISABLE_OPENAL)

	return false;

#endif

	if( !OPT->sfx_enabled ) return true; // don't setup

	numof_Sources=OPT->sfx_voices;

	if(strlen(OPT->sfx_device_name))
	{
		if( ! ( Device = alcOpenDevice((const unsigned char *)OPT->sfx_device_name) ) )
		{
			DBG_Error("SFX.OpenAL subsystem failed to initialize %s device,\n",OPT->sfx_device_name);

			if( ! ( Device = alcOpenDevice(0) ) )
			{
				DBG_Error("SFX.OpenAL subsystem failed to initialize DEFAULT device,\n");
				goto bogus;
			}
		}
	}
	else
	if( ! ( Device = alcOpenDevice(0) ) )
	{
		DBG_Error("SFX.OpenAL subsystem failed to initialize device,\n");
		goto bogus;
	}

	if( ! ( Context = alcCreateContext(Device,0) ) )
	{
		DBG_Error("SFX.OpenAL subsystem failed to initialize context,\n");
		goto bogus;
	}

	alcMakeContextCurrent(Context);

	if(alcGetError(Device) != ALC_NO_ERROR)
	{
		DBG_Error("SFX.OpenAL subsystem failed to set current context,\n");
		goto bogus;
	}

/*
	if(!alIsExtensionPresent((ALubyte*)"AL_EXT_vorbis"))
	{
		DBG_Error("SFX.OpenAL subsystem is missing OGG extension,\n");
		goto bogus;
	}
*/

// Clear Error Codes
	alGetError();
	alcGetError(Device);

	ListenerReset();

	// Generate Buffers
	OALCHECK( check , alGenBuffers(NUMOF(Buffers),(ALuint*)Buffers) ); if(!check) { goto bogus; }

	// Generate sources, adjust numof_Sources if we couldnt get that many, only fail if we didnt get at least 1
	for(i=0;i<numof_Sources;i++)
	{
		OALCHECK( check , alGenSources(1,(ALuint*)Sources+i) );
		
		if(!check)
		{
			numof_Sources=i;

			if(i==0) goto bogus;
		}
	}


	if( ! streams->setup() ) { goto bogus; }

//	if( ! Load(SFXID(Test),"data/sfx/test.ogg") ) { goto bogus; }

//	if( ! Load(SFXID(Roll01),PATH_DATADIR "amaze/sfx/roll01.wav") ) { goto bogus; }

	available=true;

	return(true);
bogus:
	clean();
	return(false);
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// Reset junk
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
bool sfx::reset(void)
{
	clean();
	return setup();
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// Clean junk
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void sfx::clean(void)
{
#define RELEASE_THIS(ptr) if(ptr) { ptr->Release(); ptr=0; }

	streams->clean();

	alDeleteSources(numof_Sources,(ALuint*)Sources);
	alDeleteBuffers(NUMOF(Buffers),(ALuint*)Buffers);

	//Get active context
	Context=alcGetCurrentContext();
	//Get device for active context
	Device=alcGetContextsDevice(Context);
	//Disable context
	alcMakeContextCurrent(NULL);
	//Release context(s)
	alcDestroyContext(Context);
	//Close device
	alcCloseDevice(Device);

#undef  RELEASE_THIS
}




/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// Call often to maintain streaming sounds :)
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void sfx::update(void)
{
	streams->update();
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// Reset Listener attributes
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void sfx::ListenerReset(void)
{

	listener_pos[0]=0.0f;
	listener_pos[1]=0.0f;
	listener_pos[2]=0.0f;

	listener_vel[0]=0.0f;
	listener_vel[1]=0.0f;
	listener_vel[2]=0.0f;

	listener_ori[0]=0.0f;
	listener_ori[1]=0.0f;
	listener_ori[2]=-1.0f;
	listener_ori[3]=0.0f;
	listener_ori[4]=1.0f;
	listener_ori[5]=0.0f;

	ListenerUpdate();

}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// Set Listener attributes
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void sfx::ListenerUpdate(void)
{
bool check;

	if(!available) return; // sound disabled/not setup

	// Position ...
	OALCHECK( check , alListenerfv(AL_POSITION,listener_pos) );

	// Velocity ...
	OALCHECK( check , alListenerfv(AL_VELOCITY,listener_vel) );

	// Orientation ...
	OALCHECK( check , alListenerfv(AL_ORIENTATION,listener_ori) );

}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// load a sound file into a previously allocated buffer, some basic noises are preallocated
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
bool sfx::Load(s32 buffer,const char *path)
{
bool check;
s32 len;
u8 *buff;
FILE *f;

	if(!available) return true; // sound disabled/not setup

	buff=0;
	f=0;

	len=strlen(path);

	if( (path[len-3]=='o') && (path[len-2]=='g') && (path[len-1]=='g') ) // simple file type test
	{
		goto bogus; // ogg not suported yet
	}
	else
	{
	ALsizei size,freq;
	ALenum	format;
	ALvoid	*data;
	ALboolean loop;

		// Load wave3.wav
		OALCHECK( check , alutLoadWAVFile((char*)path,&format,&data,&size,&freq,&loop) ) ; if(!check) { goto bogus; }

		// Copy wave3.wav data into Buffer
		OALCHECK( check , alBufferData(buffer,format,data,size,freq) ) ; if(!check) { goto bogus; }

		// Unload wave3.wav
		OALCHECK( check , alutUnloadWAV(format,data,size,freq) ) ; if(!check) { goto bogus; }

	}

	return true;

bogus:

	DBG_Error("SFX.OpenAL failed to load \"%s\" file,\n",path);

	return false;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// allocate a sound buffer
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
s32 sfx::Alloc(void)
{
s32 ret;
bool check;
static s32 random=1;

	if(!available) return random++; // sound disabled/not setup


	OALCHECK( check , alGenBuffers(1,(ALuint*)&ret) );
	if(!check) // give it a second chance?
	{
		OALCHECK( check , alGenBuffers(1,(ALuint*)&ret) );
		if(!check) { goto bogus; }
	}

	return ret;
bogus:
	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// free a sound buffer
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void sfx::Free(s32 buffer)
{
//bool check;
s32 error;

	if(!available) return; // sound disabled/not setup

// this seems to produce a spurious error if the buffer has ever been used, ignore for now
//	OALCHECK( check , alDeleteBuffers(1,(ALuint*)&buffer) );

	error=alGetError();

	alDeleteBuffers(1,(ALuint*)&buffer);

	error=alGetError();
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// Play a sound
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void sfx::Play(s32 buffer,f32 gain,f32 pitch, v3 *pos, v3 *dir)
{
bool check;

	if(!available) return; // sound disabled/not setup

	if( ( !OPT->sfx_enabled ) || ( OPT->sfx_foreground_volume==0.0f ) ) return; // don't play

	if(source_idx>=numof_Sources)
	{
		source_idx=0;
	}

//	OALCHECK( check , alSourceStop(Sources[source_idx]) );

	OALCHECK( check , alSourcei(Sources[source_idx],AL_BUFFER,buffer) );

	OALCHECK( check , alSourcef(Sources[source_idx],AL_GAIN,gain*OPT->sfx_foreground_volume) );

	OALCHECK( check , alSourcef(Sources[source_idx],AL_PITCH,pitch) );

	if(pos)
	{
		OALCHECK( check , alSourcei( Sources[source_idx],AL_SOURCE_RELATIVE, AL_FALSE ) );
//		OALCHECK( check , alSourcei( Sources[source_idx],AL_SOURCE_ABSOLUTE, AL_TRUE ) );
		OALCHECK( check , alSource3f(Sources[source_idx],AL_POSITION,pos->x,pos->y,pos->z) );
		if(dir)
		{
			OALCHECK( check , alSource3f(Sources[source_idx],AL_DIRECTION,dir->x,dir->y,dir->z) );
		}
		else
		{
			OALCHECK( check , alSource3f(Sources[source_idx],AL_DIRECTION,0,0,0) );
		}
	}
	else
	{
		OALCHECK( check , alSourcei( Sources[source_idx],AL_SOURCE_RELATIVE, AL_TRUE ) );
		OALCHECK( check , alSource3f(Sources[source_idx],AL_POSITION,0,0,0) );
		OALCHECK( check , alSource3f(Sources[source_idx],AL_DIRECTION,0,0,0) );
	}


	OALCHECK( check , alSourcePlay(Sources[source_idx]) );

	source_idx++;
	if(source_idx>=numof_Sources)
	{
		source_idx=0;
	}
}
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// stop/remove all sources
//
// gets us to a point where it should be ok to free buffers that where previously atached
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void sfx::StopAll(void)
{
s32 i;
s32 error;

	for(i=0;i<numof_Sources;i++)
	{
		alSourceStop(Sources[i]);

		error=alGetError();
	}
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// report d3d errors
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
bool oalcheck(const char *name,const char *f,s32 l, const char *t)
{
s32 error;

	if ((error = alGetError()) != AL_NO_ERROR)
	{
		DBG_Error("\n%s(%d) : %s ****OPENALCHECK**** %s($%x)==%s\n",f,l,t,alGetString(error),error,name);
		return false;
	}

	return true;
}


#endif



#endif
