/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss@XIXs.com 2013
//
/*+-----------------------------------------------------------------------------------------------------------------+*/


//
// current version information for read/write
//
#define U32_ID4_XTX0  U32_ID4('X','T','X','0')
#define XTX0_VERSION 19


#define XTX0_RELOCATE(target,type,base)	if(target) { target=(type*) ( ((u32)(base)) + ((u32)(target)) ); }


// the twiddle function converts into little endian if this code is run on a big endian system


struct XTX0_header
{
	u32 id;
	u32 version;
	u32 filesize; // not including this header

	void twiddle(void)
	{
	}
};


struct XTX0_area
{
	s16 x,y;		// position in pixels
	s16 w,h;		// size in piels
	s16 hx,hy;		// handle offset pixels, add to draw pos to find correct draw position

	void twiddle(void)
	{
	}
};


struct XTX0_info
{
	s32 numof_areas;
	s32 numof_maps;
	s32 numof_kerns;
	s32 numof_pages;		// for multipage textures, the base filename of this is used and number appended
							// if this is 0 then only one page so no need to append anything just find and load associated bitmap

	XTX0_area		*areas;

	u16				*maps;		// if nonzero, used to index into area, 

	u8				*kerns;		// extra font kerning info, 2d table numof_areas*numof_areas in size

	void twiddle(void)
	{
	}

//
// Relocate the file offsets into pointers
//
	void relocate(void)
	{

		XTX0_RELOCATE(	areas		,	XTX0_area		,	this	)
		XTX0_RELOCATE(	maps		,	u16				,	this	)
		XTX0_RELOCATE(	kerns		,	u8				,	this	)

	}
};
