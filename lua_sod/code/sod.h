/*
-- Copyright (C) 2012 Kriss Blank < Kriss@XIXs.com >
-- This file is distributed under the terms of the MIT license.
-- http://en.wikipedia.org/wiki/MIT_License
-- Please ping me if you use it for anything cool...
*/


//
// format types, these are all signed values 
//
#define	SOD_FMT_NONE								0
#define	SOD_FMT_MONO8								0x1100
#define	SOD_FMT_MONO16								0x1101
#define SOD_FMT_STEREO8								0x1102
#define SOD_FMT_STEREO16							0x1103

typedef struct
{
	s32	fmt;			// format of data
	
	s32	sample_size;	// size of each sample in bytes, probably 2 (16bits)
	s32	samples;		// size of data in samples
	s32 chanels;		// number of chanels, probably 1 or maybe 2
	
	u8 *data;			// pointer to sample data
	s32 data_sizeof;	// size of data in bytes probably ( sample_size * samples * chanels )
	
	s32 freq;			// sugested default frequency this sample should be played back at
		
	const char *err;	// can be used to pass error strings back to other code

} sod;


sod * sod_alloc();
sod * sod_alloc_data(sod *sd,s32 fmt,s32 samples);

void sod_free(sod *sd);
void sod_free_data(sod *sd);;

void sod_set_fmt( sod * sd, s32 fmt );

sod * sod_load_file( sod * sd, const char *filename , const char *opts );

sod * sod_load_data( sod * sd, const unsigned char *data , int len, const char *opts );


