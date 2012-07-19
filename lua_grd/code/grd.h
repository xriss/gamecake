/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss Daniels 2003 http://www.XIXs.com
//
/*+-----------------------------------------------------------------------------------------------------------------+*/


//
// Only have popular/usefull formats as basic types that can be used internaly
//
// data is always in ARGB order in memory but these are little endian, hence the BGRA (u32) default 
//
// these are all signed values and fit in 16bits
//

#define	GRD_FMT_NONE								0x0000

// basic formats, most internal manipulations will only work on GRD_FMT_U8_ARGB
// in fact many will convert to this as an intermediate step.
// also you may need to convert to ARGB or RGB or INDEXED before saving and from after loading
// I'm trying to avoid diferent byte order to keep it simple, so ARGB **memory** order default

// u32[1] or u8[4] ARGB per pixel, so thats a U32-BGRA (in little endian)
#define	GRD_FMT_U8_ARGB								0x0001
	
// A is the same as in ARGB but ( RGB=RGB*A )
#define	GRD_FMT_U8_ARGB_PREMULT						0x0002

// bit swizzzzzzzzled for gles prefered (and only suported) order
#define	GRD_FMT_U8_RGBA								0x0003
#define	GRD_FMT_U8_RGBA_PREMULT						0x0004

// u16[1] per pixel, 1 bit alpha , 5 bits red , 5 bits green , 5 bits blue
#define	GRD_FMT_U16_ARGB_1555						0x0021	
// again premult makes more sense
#define	GRD_FMT_U16_ARGB_1555_PREMULT				0x0022

// again, this is bit swizzzzzzzzled for the gles prefered order
// u16[1] per pixel, 4 bits red , 4 bits green , 4 bits blue , 4 bit alpha
#define	GRD_FMT_U16_RGBA_4444						0x0023
// again premult makes more sense
#define	GRD_FMT_U16_RGBA_4444_PREMULT				0x0024

// u16[1] an output display 16bit format for gles
#define	GRD_FMT_U16_RGB_565							0x0025


// I think it makes sense to keep all floating point values as premultiplied alpha?
// a 1.0 in here is the same as a 255 in U8 format

// f16[4] per pixel
#define	GRD_FMT_F16_ARGB_PREMULT					0x0041
// f32[4] per pixel
#define	GRD_FMT_F32_ARGB_PREMULT					0x0062
// f64[4] per pixel
#define	GRD_FMT_F64_ARGB_PREMULT					0x0083

// u8[3]  per pixel, probably just normal palette information
#define	GRD_FMT_U8_RGB								0x00a1

// u8[1]  per pixel, forced U8 Indexed input
#define	GRD_FMT_U8_INDEXED							0x00c1

// u8[1]  per pixel, forced U8 gray scale (treat as indexed)
#define	GRD_FMT_U8_LUMINANCE						0x00e1
#define	GRD_FMT_U8_ALPHA							0x00e2


// more formats, not to be used when mucking about with data
// these are hints for textures rather than specific formats and don't guarantee any number of bits
// in fact the texture may even use a simple lossy compressed format if enabled
// basically it is none of your concern, if you intend to do anything with the dat convert it to one of the
// basic formats

// just RGB , probably u32 or u16(565)
#define	GRD_FMT_HINT_NO_ALPHA						0x0101

// and RGB  , probably u32 or u16(1555)
#define	GRD_FMT_HINT_ALPHA_1BIT						0x0102

// and RGB  , probably u32 or u16(4444)
#define	GRD_FMT_HINT_ALPHA							0x0103

// no RGB   , probably u8
#define	GRD_FMT_HINT_ONLY_ALPHA						0x0104

// we want to save or load as a png									
#define	GRD_FMT_HINT_PNG							0x0105

// we want to save or load as a jpg									
#define	GRD_FMT_HINT_JPG							0x0106
	
// maximum GRD_FMT value		
#define	GRD_FMT_MAX									0x0107

// information about a bitmap held in memory (or even a palette)
// by using scan values we can describe a section of a larger bitmap in this structure
// or even upside down bitmaps with negative values provided we don't make any assumptions about scan values

struct grd_info
{
	s32	fmt;			// format of data
	
	s32	w,h,d;			// width and height and depth of image

	s32	xscan;			// add this to data to move across the image probably (sizeof(pixel))
	s32	yscan;			// add this to data to move down the image probably (w*sizeof(pixel))
	s32	zscan;			// add this to data to move into the image probably (w*sizeof(pixel)*h)

	u8 *data;			// pointer to image data

};

void grdinfo_reset(struct grd_info *gi);
void grdinfo_set(  struct grd_info *gi , struct grd_info *ga );
u8 * grdinfo_get_data( struct grd_info *gi , s32 x, s32 y, s32 z);


//
// We own the data stored here
//
struct grd
{
	
	struct grd_info cmap[1]; // a palette, if we are a paletted file (cmap.data!=0)
	
	struct grd_info bmap[1]; // the bitmap data

// this can be set to an error string if we hit any problems

	const char *err;

// extra allocated memory associated with this grd,
	
	void *data;
	s32   data_sizeof;
	
};

// everything you need to know to load an image from file or memory
struct grd_loader_info
{
	const char * file_name;
	u8 * data;
	int data_len;
	int pos;
	int fmt;
};


// just a simple  area
struct grd_area
{
	s32	x,y,z;			// the top left corner and its
	s32	w,h,d;			// width and height and depth
};


void * grd_info_alloc(struct grd_info *gi,  s32 fmt , s32 w, s32 h, s32 d );
void grd_info_free(struct grd_info *gi);


struct grd * grd_realloc( struct grd *g, s32 fmt , s32 w, s32 h, s32 d );

struct grd * grd_create( s32 fmt , s32 w, s32 h, s32 d );

void grd_free( struct grd *g );

struct grd * grd_load_file( const char *filename , int fmt );
struct grd * grd_load_data( const unsigned char *data , int len , int fmt );

struct grd * grd_save_file( struct grd *g, const char *filename , int fmt );


struct grd * grd_duplicate( struct grd *g );
struct grd * grd_duplicate_convert( struct grd *g , s32 fmt );
struct grd * grd_duplicate_quant(struct grd *g , s32 num_colors );

int grd_convert( struct grd *g , s32 fmt );
int grd_quant(struct grd *g , s32 num_colors );

void grd_flipy( struct grd *g );

//int grd_conscale( struct grd *g , f32 base, f32 scale);

int grd_scale( struct grd *g , s32 w, s32 h, s32 d);


int grd_layer( struct grd *ga , struct grd *gb , s32 z);

int grd_clip( struct grd *ga , struct grd *gb , s32 x, s32 y, s32 w, s32 h);

int grd_blit( struct grd *ga , struct grd *gb , s32 x, s32 y);

int grd_shrink(struct grd *ga,struct grd_area *gc );

