/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss@XIXs.com 2013
//
/*+-----------------------------------------------------------------------------------------------------------------+*/


// memory has been borrowed from another GRD and should not be freed
#define	GRD_FLAGS_BORROWED								0x0001


//
// We shall stop fighting the force of open GL and accept RGBA as the damn color order
//
// Old code will break after this change (its all ARGB order) so keep stuff upto date mkay :)
// all palettes have been switched to this new order as well
//

#define	GRD_FMT_NONE								0x0000

// A is the same as in ARGB but ( RGB=RGB*A )
// Notice that this is more of a flag ontop of other formats
#define	GRD_FMT_PREMULT								0x0100

// basic formats, most internal manipulations will only work on GRD_FMT_U8_RGBA
// in fact many will convert to this as an intermediate step.
// also you may need to convert to RGBA or RGB or INDEXED before saving and from after loading
// I'm trying to avoid diferent byte order to keep it simple, so RGBA **memory** order default

// bit swizzzzzzzzled for gls prefered (and only suported) order
// u32[1] or u8[4] RGBA per pixel, so thats a U32-ABGR (in little endian)
#define	GRD_FMT_U8_RGBA								0x0001
#define	GRD_FMT_U8_RGBA_PREMULT						0x0101

// u32[1] or u8[4] ARGB per pixel, so thats a U32-BGRA (in little endian)
// This format is not used anywhere internally anymore.
#define	GRD_FMT_U8_ARGB								0x0002
#define	GRD_FMT_U8_ARGB_PREMULT						0x0102


// u8[3]  per pixel RGB, no alpha
#define	GRD_FMT_U8_RGB								0x0011


// u8[1]  per pixel, palette indexed, palette contains alpha (png suports this)
#define	GRD_FMT_U8_INDEXED							0x0021
#define	GRD_FMT_U8_INDEXED_PREMULT					0x0121

// u8[1]  per pixel, black to white, gray scale
#define	GRD_FMT_U8_LUMINANCE						0x0022

// u8[1]  per pixel, alpha chanel only, assume white for rgb values
#define	GRD_FMT_U8_ALPHA							0x0023


// u16[1] per pixel
#define	GRD_FMT_U16_RGBA_5551						0x0031	
#define	GRD_FMT_U16_RGBA_5551_PREMULT				0x0131

// u16[1] per pixel
#define	GRD_FMT_U16_RGBA_4444						0x0032
#define	GRD_FMT_U16_RGBA_4444_PREMULT				0x0132

// u16[1] an output display 16bit format for gles
#define	GRD_FMT_U16_RGBA_5650						0x0033
#define	GRD_FMT_U16_RGBA_5650_PREMULT				0x0133

// u8[2]  luminance and alpha
#define	GRD_FMT_U8_LUMINANCE_ALPHA					0x0034
#define	GRD_FMT_U8_LUMINANCE_ALPHA_PREMULT			0x0134



// I think it makes sense to keep all floating point values as premultiplied alpha?
// a 1.0 in here is the same as a 255 in U8 format
// none of these are used, so lets comment them all out for now
// f16[4] per pixel
//#define	GRD_FMT_F16_RGBA_PREMULT					0x0141
// f32[4] per pixel
//#define	GRD_FMT_F32_RGBA_PREMULT					0x0151
// f64[4] per pixel
//#define	GRD_FMT_F64_RGBA_PREMULT					0x0161


// the following are hints for internal code

// just RGB , probably u32 or u16(565)
#define	GRD_FMT_HINT_NO_ALPHA						0x0201

// and RGB  , probably u32 or u16(1555)
#define	GRD_FMT_HINT_ALPHA_1BIT						0x0202

// and RGB  , probably u32 or u16(4444)
#define	GRD_FMT_HINT_ALPHA							0x0203

// no RGB   , probably u8
#define	GRD_FMT_HINT_ONLY_ALPHA						0x0204



// we want to save or load as a png									
#define	GRD_FMT_HINT_PNG							0x0401

// we want to save or load as a jpg									
#define	GRD_FMT_HINT_JPG							0x0402

// we want to save or load as a gif									
#define	GRD_FMT_HINT_GIF							0x0403


// Paint a copy skiping transparent color
#define	GRD_PAINT_MODE_TRANS						0x0801
// Paint a single color
#define	GRD_PAINT_MODE_COLOR						0x0802
// Paint a copy
#define	GRD_PAINT_MODE_COPY							0x0803
// Paint using XOR
#define	GRD_PAINT_MODE_XOR							0x0804
// Paint using ALPHA from palette (skip if < 0x80)
#define	GRD_PAINT_MODE_ALPHA						0x0805


// little endian
#define	GRD_TAG_DEF(a,b,c,d) (((((u32)d)<<24)+(((u32)c)<<16)+(((u32)b)<<8)+((u32)a)))


// information about a bitmap held in memory (or even a palette)
// by using scan values we can describe a section of a larger bitmap in this structure
// or even upside down bitmaps with negative values provided we don't make any assumptions about scan values

struct grd_info
{
	s16	fmt;			// format of data
	s16	flags;			// flags of data
	
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

// everything you need to know to load/save an image from/to file or memory
struct grd_io_info
{
	const char * file_name; // 0 if not a file load/save
	u8 * data;
	int data_len;
	int data_len_max; // data may be stored into a bigger buffer (to reduce realocs while writing)
	int pos;
	int fmt;
	u32 *tags;
};


// just a simple  area
struct grd_area
{
	s32	x,y,z;			// the top left corner and its
	s32	w,h,d;			// width and height and depth
};


void * grd_info_alloc(struct grd_info *gi,  s32 fmt , s32 w, s32 h, s32 d );
void grd_info_free(struct grd_info *gi);

void grd_copy_data(struct grd *ga, struct grd *gb );
void grd_copy_data_layer(struct grd *ga, struct grd *gb , int za , int zb );

struct grd * grd_realloc( struct grd *g, s32 fmt , s32 w, s32 h, s32 d );

struct grd * grd_create( s32 fmt , s32 w, s32 h, s32 d );

void grd_free( struct grd *g );

struct grd * grd_load_file( const char *filename , int fmt , u32 *tags );
struct grd * grd_load_data( const unsigned char *data , int len , int fmt , u32 *tags );

struct grd * grd_save_file( struct grd *g, const char *filename , int fmt , u32 *tags );
struct grd * grd_save_data( struct grd *g, struct grd_io_info *filedata , int fmt );


struct grd * grd_duplicate( struct grd *g );
struct grd * grd_duplicate_convert( struct grd *g , s32 fmt );
struct grd * grd_duplicate_quant(struct grd *g , s32 num_colors , s32 dither );
struct grd * grd_duplicate_sobelnormal(struct grd *g );

int grd_sobelnormal( struct grd *g );
int grd_convert( struct grd *g , s32 fmt );
int grd_quant(struct grd *g , s32 num_colors , s32 dither );
int grd_attr_redux(struct grd *g, int cw, int ch, int num, int sub, int bak);

int grd_slide( struct grd *ga , int dx , int dy , int dz );

void grd_flipx( struct grd *g );
void grd_flipy( struct grd *g );

//int grd_conscale( struct grd *g , f32 base, f32 scale);

int grd_resize( struct grd *g , s32 w, s32 h, s32 d);
int grd_scale( struct grd *g , s32 w, s32 h, s32 d);


int grd_layer( struct grd *ga , struct grd *gb , s32 z);

int grd_clip( struct grd *ga , struct grd *gb ,  s32 x, s32 y, s32 z, s32 w, s32 h, s32 d);

int grd_blit( struct grd *ga , struct grd *gb , s32 x, s32 y);
int grd_paint( struct grd *ga , struct grd *gb , s32 x, s32 y, s32 mode, u32 trans, u32 color);

int grd_xor(struct grd *gd,struct grd *ga);
int grd_shrink(struct grd *ga,struct grd_area *gc );

void grd_clear( struct grd *g , u32 val);

u32* grd_tags_find(u32 *tags,u32 id);

int grd_remap(struct grd *ga, struct grd *gb, int colors, int dither);

int grd_adjust_hsv( struct grd *g , f32 fh, f32 fs, f32 fv);
int grd_adjust_rgb( struct grd *g , f32 fr, f32 fg, f32 fb);
int grd_adjust_contrast( struct grd *g , int sub, f32 con);


int grd_sort_cmap( struct grd *g );

int grd_fillmask(struct grd *ga, struct grd *gb, int seedx, int seedy, int threshold);

