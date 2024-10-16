/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss@XIXs.com 2013
//
/*+-----------------------------------------------------------------------------------------------------------------+*/


//
// Only have popular/usefull formats as basic types that can be used internaly
//
enum GRD_FMT
{
	GRD_FMT_NONE=0,

// basic formats, most manipulations will only work on GRD_FMT_ARGB_U8
// data can be convereted too or from other formats but only during load/save its never going to be worked on like that

	GRD_FMT_F32_ARGB,				// f32[4] per pixel, forced F32 ARGB

	GRD_FMT_U8_BGRA,				// u8[4]  per pixel, forced U8 BGRA (thinking little endian makes this the default) 
	GRD_FMT_U8_INDEXED,				// u8[1]  per pixel, forced U8 Indexed input
	GRD_FMT_U8_LUMINANCE,			// u8[1]  per pixel, forced U8 gray scale


// these are slightly less basic, intended to be converted to, on read/write but not manipulated
// some of them fit multiple pixels into 1 byte for instance.
// generally duplicate the image, convert to one of these then dump or save it

	GRD_FMT_U16_ARGB_1555,			// u16[1] per pixel, 1 bit alpha , 5 bits red , 5 bits green , 5 bits blue

	GRD_FMT_U8_RGB,					// u8[3]  per pixel, probably normal palette information

// more formats, not to be used when mucking about with data
// these are hints for textures rather than specific formats and don't guarantee any number of bits
// in fact the texture may even use a simple lossy compressed format if enabled
// basically it is none of your concern, if you intend to do anything with the dat convert it to one of the
// above basic formats

	GRD_FMT_HINT_NO_ALPHA,			// just RGB , probably u32 or u16(565)
	GRD_FMT_HINT_ALPHA_1BIT,		// and RGB  , probably u32 or u16(1555)
	GRD_FMT_HINT_ALPHA,				// and RGB  , probably u32 or u16(4444)
	GRD_FMT_HINT_ONLY_ALPHA,		// no RGB   , probably u8


	GRD_FMT_MAX
};
#define GRD_FMT_GOTALPHA(x) (x!=GRD_FMT_NO_ALPHA)
#define GRD_FMT_SIZEOFPIXEL(x) ((x==GRD_FMT_U8_INDEXED)?1:4)






// information about a bitmap held in memory (or even a palette)
// by using scan values we can describe a section of a larger bitmap in this structure
// or even upside down bitmaps with negative values provided we don't make any assumptions about scan values

struct grd_info
{
	s32	fmt;	// format of data

	s32	w,h,d;			// width and height and depth of image

	s32	xscan;			// add this to data to move across the image probably (sizeof(pixel))
	s32	yscan;			// add this to data to move down the image probably (w*sizeof(pixel))
	s32	zscan;			// add this to data to move into the image probably (w*sizeof(pixel)*h)

	u8 *data;			// pointer to image data

	inline void reset(void)
	{
		fmt=GRD_FMT_NONE;

		w=0;
		h=0;
		d=0;

		xscan=0;
		yscan=0;
		zscan=0;

		data=0;
	}

	inline u8 * get_data(s32 x, s32 y, s32 z)
	{
		return data+(z*zscan)+(y*yscan)+(x*xscan);
	}

	inline u8 * get_data(f32 x, f32 y, f32 z)
	{
		return data+(((s32)(z))*zscan)+(((s32)(y))*yscan)+(((s32)(x))*xscan);
	}

};



s32 grd_load_smart( const char *filename , s32 fmt , grd_info *grd );

