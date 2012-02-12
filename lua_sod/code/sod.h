/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss Daniels 2003 http://www.XIXs.com
//
/*+-----------------------------------------------------------------------------------------------------------------+*/


//
// format types, these are all signed values 
//
#define	SOD_FMT_NONE								0
#define	SOD_FMT_MONO8								0x1100
#define	SOD_FMT_MONO16								0x1101
#define SOD_FMT_STEREO8								0x1102
#define SOD_FMT_STEREO16							0x1103

struct sod
{
	s32	fmt;			// format of data
	
	s32	sample_size;	// size of each sample in bytes, probably 2 (16bits)
	s32	samples;		// size of data in samples
	s32 chanels;		// number of chanels, probably 1 or maybe 2
	
	u8 *data;			// pointer to sample data
	s32 data_sizeof;	// size of data in bytes probably ( sample_size * samples * chanels )
	
	s32 freq;			// sugested default frequency this sample should be played back at
		
	const char *err;	// can be used to pass error strings back to other code
};
