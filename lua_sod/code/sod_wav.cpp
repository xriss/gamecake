/*
-- Copyright (C) 2012 Kriss Blank < Kriss@XIXs.com >
-- This file is distributed under the terms of the MIT license.
-- http://en.wikipedia.org/wiki/MIT_License
-- Please ping me if you use it for anything cool...
*/

#include "all.h"



#if defined _MSC_VER
	#pragma pack (push,1)
	#define PACKBYTES
#elif defined __GNUC__
	#define PACKBYTES __attribute__((packed))
#endif

typedef struct PACKBYTES /* WAV File-header */
{
  u8   Id[4];
  s32  Size;
  u8   Type[4];
} WAVFileHdr_Struct;

typedef struct PACKBYTES /* WAV Fmt-header */
{
	u16	Format;
	u16	Channels;
	u32	SamplesPerSec;
	u32	BytesPerSec;
	u16	BlockAlign;
	u16	BitsPerSample;
} WAVFmtHdr_Struct;

typedef struct PACKBYTES /* WAV FmtEx-header */
{
	u16 Size;
	u16 SamplesPerBlock;
} WAVFmtExHdr_Struct;

typedef struct PACKBYTES /* WAV Smpl-header */
{
	u32	Manufacturer;
	u32	Product;
	u32	SamplePeriod;
	u32	Note;
	u32	FineTune;
	u32	SMPTEFormat;
	u32	SMPTEOffest;
	u32	Loops;
	u32	SamplerData;
	struct PACKBYTES
	{
		u32	Identifier;
		u32	Type;
		u32	Start;
		u32	End;
		u32	Fraction;
		u32	Count;
	}	Loop[1];
} WAVSmplHdr_Struct;

typedef struct PACKBYTES /* WAV Chunk-header */
{
	u8	Id[4];
	u32	Size;
} WAVChunkHdr_Struct;


#ifdef PACKBYTES
	#undef PACKBYTES
#endif

#if defined _MSC_VER
	#pragma pack (pop)
#endif

#define error(x) { err=x; goto bogus; }

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// read a wav file into a sod
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
sod * sod_wav_load_file(sod * sd, const char* file_name)
{
	const char *err=0;

	WAVChunkHdr_Struct ChunkHdr;
	WAVFmtExHdr_Struct FmtExHdr;
	WAVFileHdr_Struct FileHdr;
	WAVSmplHdr_Struct SmplHdr;
	WAVFmtHdr_Struct FmtHdr;
	FILE *fp;
	
	int format=SOD_FMT_MONO16;
	void *data=NULL;
	int size=0;
	int freq=22050;
	int loop=0;

	fp=fopen(file_name,"rb");
	if(!fp) error("failed to open wav file")
	
	fread(&FileHdr,1,sizeof(WAVFileHdr_Struct),fp);
	FileHdr.Size=((FileHdr.Size+1)&~1)-4;
	while ((FileHdr.Size!=0)&&(fread(&ChunkHdr,1,sizeof(WAVChunkHdr_Struct),fp)))
	{
		if (!memcmp(ChunkHdr.Id,"fmt ",4))
		{
			fread(&FmtHdr,1,sizeof(WAVFmtHdr_Struct),fp);
			if (FmtHdr.Format==0x0001)
			{
				format=(FmtHdr.Channels==1?
						(FmtHdr.BitsPerSample==8?SOD_FMT_MONO8:SOD_FMT_MONO16):
						(FmtHdr.BitsPerSample==8?SOD_FMT_STEREO8:SOD_FMT_STEREO16));
				freq=FmtHdr.SamplesPerSec;
				fseek(fp,ChunkHdr.Size-sizeof(WAVFmtHdr_Struct),SEEK_CUR);
			} 
			else
			{
				fread(&FmtExHdr,1,sizeof(WAVFmtExHdr_Struct),fp);
				fseek(fp,ChunkHdr.Size-sizeof(WAVFmtHdr_Struct)-sizeof(WAVFmtExHdr_Struct),SEEK_CUR);
			}
		}
		else if (!memcmp(ChunkHdr.Id,"data",4))
		{
			if (FmtHdr.Format==0x0001)
			{
				if(!sod_alloc_data(sd,format, ChunkHdr.Size/FmtHdr.BlockAlign ))
				{
					error("failed to allocate wav data");
				}
				fread(sd->data,1,sd->data_sizeof,fp);
				
//						size=ChunkHdr.Size;
//						data=calloc(ChunkHdr.Size);
//						if (data)
//						{
//							fread(data,FmtHdr.BlockAlign,ChunkHdr.Size/FmtHdr.BlockAlign,fp);
//							memset(((char *)data)+ChunkHdr.Size,0,31);
//						}
			}
			else if (FmtHdr.Format==0x0011)
			{
				error("ADPCM not suported in wav loader");
			}
			else if (FmtHdr.Format==0x0055)
			{
				error("MP3 not suported in wav loader");
			}
			else
			{
				error("unknown format in wav loader");
			}
		}
		else if (!memcmp(ChunkHdr.Id,"smpl",4))
		{
			fread(&SmplHdr,1,sizeof(WAVSmplHdr_Struct),fp);
			loop = (SmplHdr.Loops ? 1 : 0);
			fseek(fp,ChunkHdr.Size-sizeof(WAVSmplHdr_Struct),SEEK_CUR);
		}
		else fseek(fp,ChunkHdr.Size,SEEK_CUR);
		fseek(fp,ChunkHdr.Size&1,SEEK_CUR);
		FileHdr.Size-=(((ChunkHdr.Size+1)&~1)+8);
	}
	
	fclose(fp);

	return sd;
	
bogus:
	if(fp) 	{ fclose(fp); }
	if(err) { sd->err=err; } else { sd->err=0; }
	return 0;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// save a sod as a wav file
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
sod * grd_wav_save_file(sod *sd , const char* file_name )
{
	sd->err="wav save disabled";
	return 0;
}

