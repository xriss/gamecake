/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss Daniels 2003 http://www.XIXs.com
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
#include "all.h"


#include "../_src/DevIL/include/IL/devil_internal_exports.h"
extern "C" ILimage *iCurImage;


/*----------------------------------------------------------------------------------------------------------------------------*/
//
// str def lookups
//
/*----------------------------------------------------------------------------------------------------------------------------*/

#define ILSTRDEF(s) {#s,s},


struct strnum
{
	const char *str;
	u32 num;
};

const strnum IL_STR_ENUM_DEFS[]=
{
ILSTRDEF(IL_COLOUR_INDEX)//     0x1900
ILSTRDEF(IL_COLOR_INDEX)//      0x1900
ILSTRDEF(IL_RGB)//              0x1907
ILSTRDEF(IL_RGBA)//             0x1908
ILSTRDEF(IL_BGR)//              0x80E0
ILSTRDEF(IL_BGRA)//             0x80E1
ILSTRDEF(IL_LUMINANCE)//        0x1909
ILSTRDEF(IL_LUMINANCE_ALPHA)//  0x190A


ILSTRDEF(IL_BYTE)//           0x1400
ILSTRDEF(IL_UNSIGNED_BYTE)//  0x1401
ILSTRDEF(IL_SHORT)//          0x1402
ILSTRDEF(IL_UNSIGNED_SHORT)// 0x1403
ILSTRDEF(IL_INT)//            0x1404
ILSTRDEF(IL_UNSIGNED_INT)//   0x1405
ILSTRDEF(IL_FLOAT)//          0x1406
ILSTRDEF(IL_DOUBLE)//         0x140A


ILSTRDEF(IL_VENDOR)//   0x1F00
ILSTRDEF(IL_LOAD_EXT)// 0x1F01
ILSTRDEF(IL_SAVE_EXT)// 0x1F02




// Palette types
ILSTRDEF(IL_PAL_NONE)//   0x0400
ILSTRDEF(IL_PAL_RGB24)//  0x0401
ILSTRDEF(IL_PAL_RGB32)//  0x0402
ILSTRDEF(IL_PAL_RGBA32)// 0x0403
ILSTRDEF(IL_PAL_BGR24)//  0x0404
ILSTRDEF(IL_PAL_BGR32)//  0x0405
ILSTRDEF(IL_PAL_BGRA32)// 0x0406


// Image types
ILSTRDEF(IL_TYPE_UNKNOWN)// 0x0000
ILSTRDEF(IL_BMP)//          0x0420
ILSTRDEF(IL_CUT)//          0x0421
ILSTRDEF(IL_DOOM)//         0x0422
ILSTRDEF(IL_DOOM_FLAT)//    0x0423
ILSTRDEF(IL_ICO)//          0x0424
ILSTRDEF(IL_JPG)//          0x0425
ILSTRDEF(IL_JFIF)//         0x0425
ILSTRDEF(IL_LBM)//          0x0426
ILSTRDEF(IL_PCD)//          0x0427
ILSTRDEF(IL_PCX)//          0x0428
ILSTRDEF(IL_PIC)//          0x0429
ILSTRDEF(IL_PNG)//          0x042A
ILSTRDEF(IL_PNM)//          0x042B
ILSTRDEF(IL_SGI)//          0x042C
ILSTRDEF(IL_TGA)//          0x042D
ILSTRDEF(IL_TIF)//          0x042E
ILSTRDEF(IL_CHEAD)//        0x042F
ILSTRDEF(IL_RAW)//          0x0430
ILSTRDEF(IL_MDL)//          0x0431
ILSTRDEF(IL_WAL)//          0x0432
ILSTRDEF(IL_LIF)//          0x0434
ILSTRDEF(IL_MNG)//          0x0435
ILSTRDEF(IL_JNG)//          0x0435
ILSTRDEF(IL_GIF)//          0x0436
ILSTRDEF(IL_DDS)//          0x0437
ILSTRDEF(IL_DCX)//          0x0438
ILSTRDEF(IL_PSD)//          0x0439
ILSTRDEF(IL_EXIF)//         0x043A
ILSTRDEF(IL_PSP)//          0x043B
ILSTRDEF(IL_PIX)//          0x043C
ILSTRDEF(IL_PXR)//          0x043D
ILSTRDEF(IL_XPM)//          0x043E
ILSTRDEF(IL_HDR)//          0x043F

ILSTRDEF(IL_JASC_PAL)//     0x0475


// Error Types
ILSTRDEF(IL_NO_ERROR)//             0x0000
ILSTRDEF(IL_INVALID_ENUM)//         0x0501
ILSTRDEF(IL_OUT_OF_MEMORY)//        0x0502
ILSTRDEF(IL_FORMAT_NOT_SUPPORTED)// 0x0503
ILSTRDEF(IL_INTERNAL_ERROR)//       0x0504
ILSTRDEF(IL_INVALID_VALUE)//        0x0505
ILSTRDEF(IL_ILLEGAL_OPERATION)//    0x0506
ILSTRDEF(IL_ILLEGAL_FILE_VALUE)//   0x0507
ILSTRDEF(IL_INVALID_FILE_HEADER)//  0x0508
ILSTRDEF(IL_INVALID_PARAM)//        0x0509
ILSTRDEF(IL_COULD_NOT_OPEN_FILE)//  0x050A
ILSTRDEF(IL_INVALID_EXTENSION)//    0x050B
ILSTRDEF(IL_FILE_ALREADY_EXISTS)//  0x050C
ILSTRDEF(IL_OUT_FORMAT_SAME)//      0x050D
ILSTRDEF(IL_STACK_OVERFLOW)//       0x050E
ILSTRDEF(IL_STACK_UNDERFLOW)//      0x050F
ILSTRDEF(IL_INVALID_CONVERSION)//   0x0510
ILSTRDEF(IL_BAD_DIMENSIONS)//       0x0511
ILSTRDEF(IL_FILE_READ_ERROR)//      0x0512  // 05/12/2002: Addition by Sam.
ILSTRDEF(IL_FILE_WRITE_ERROR)//     0x0512

ILSTRDEF(IL_LIB_GIF_ERROR)//  0x05E1
ILSTRDEF(IL_LIB_JPEG_ERROR)// 0x05E2
ILSTRDEF(IL_LIB_PNG_ERROR)//  0x05E3
ILSTRDEF(IL_LIB_TIFF_ERROR)// 0x05E4
ILSTRDEF(IL_LIB_MNG_ERROR)//  0x05E5
ILSTRDEF(IL_UNKNOWN_ERROR)//  0x05FF


// Origin Definitions
ILSTRDEF(IL_ORIGIN_SET)//        0x0600
ILSTRDEF(IL_ORIGIN_LOWER_LEFT)// 0x0601
ILSTRDEF(IL_ORIGIN_UPPER_LEFT)// 0x0602
ILSTRDEF(IL_ORIGIN_MODE)//       0x0603


// Format and Type Mode Definitions
ILSTRDEF(IL_FORMAT_SET)//  0x0610
ILSTRDEF(IL_FORMAT_MODE)// 0x0611
ILSTRDEF(IL_TYPE_SET)//    0x0612
ILSTRDEF(IL_TYPE_MODE)//   0x0613


// File definitions
ILSTRDEF(IL_FILE_OVERWRITE)// 0x0620
ILSTRDEF(IL_FILE_MODE)//      0x0621


// Palette definitions
ILSTRDEF(IL_CONV_PAL)// 0x0630


// Load fail definitions
ILSTRDEF(IL_DEFAULT_ON_FAIL)// 0x0632


// Key colour definitions
ILSTRDEF(IL_USE_KEY_COLOUR)// 0x0635
ILSTRDEF(IL_USE_KEY_COLOR)//  0x0635


// Interlace definitions
ILSTRDEF(IL_SAVE_INTERLACED)// 0x0639
ILSTRDEF(IL_INTERLACE_MODE)//  0x063A


// Quantization definitions
ILSTRDEF(IL_QUANTIZATION_MODE)// 0x0640
ILSTRDEF(IL_WU_QUANT)//          0x0641
ILSTRDEF(IL_NEU_QUANT)//         0x0642
ILSTRDEF(IL_NEU_QUANT_SAMPLE)//  0x0643
ILSTRDEF(IL_MAX_QUANT_INDEXS)//  0x0644 //XIX : ILint : Maximum number of colors to reduce to, default of 256. and has a range of 2-256


// Hints
ILSTRDEF(IL_FASTEST)//          0x0660
ILSTRDEF(IL_LESS_MEM)//         0x0661
ILSTRDEF(IL_DONT_CARE)//        0x0662
ILSTRDEF(IL_MEM_SPEED_HINT)//   0x0665
ILSTRDEF(IL_USE_COMPRESSION)//  0x0666
ILSTRDEF(IL_NO_COMPRESSION)//   0x0667
ILSTRDEF(IL_COMPRESSION_HINT)// 0x0668


// Subimage types
ILSTRDEF(IL_SUB_NEXT)//   0x0680
ILSTRDEF(IL_SUB_MIPMAP)// 0x0681
ILSTRDEF(IL_SUB_LAYER)//  0x0682


// Compression definitions
ILSTRDEF(IL_COMPRESS_MODE)// 0x0700
ILSTRDEF(IL_COMPRESS_NONE)// 0x0701
ILSTRDEF(IL_COMPRESS_RLE)//  0x0702
ILSTRDEF(IL_COMPRESS_LZO)//  0x0703
ILSTRDEF(IL_COMPRESS_ZLIB)// 0x0704


// File format-specific values
ILSTRDEF(IL_TGA_CREATE_STAMP)//        0x0710
ILSTRDEF(IL_JPG_QUALITY)//             0x0711
ILSTRDEF(IL_PNG_INTERLACE)//           0x0712
ILSTRDEF(IL_TGA_RLE)//                 0x0713
ILSTRDEF(IL_BMP_RLE)//                 0x0714
ILSTRDEF(IL_SGI_RLE)//                 0x0715
ILSTRDEF(IL_TGA_ID_STRING)//           0x0717
ILSTRDEF(IL_TGA_AUTHNAME_STRING)//     0x0718
ILSTRDEF(IL_TGA_AUTHCOMMENT_STRING)//  0x0719
ILSTRDEF(IL_PNG_AUTHNAME_STRING)//     0x071A
ILSTRDEF(IL_PNG_TITLE_STRING)//        0x071B
ILSTRDEF(IL_PNG_DESCRIPTION_STRING)//  0x071C
ILSTRDEF(IL_TIF_DESCRIPTION_STRING)//  0x071D
ILSTRDEF(IL_TIF_HOSTCOMPUTER_STRING)// 0x071E
ILSTRDEF(IL_TIF_DOCUMENTNAME_STRING)// 0x071F
ILSTRDEF(IL_TIF_AUTHNAME_STRING)//     0x0720
ILSTRDEF(IL_JPG_SAVE_FORMAT)//         0x0721
ILSTRDEF(IL_CHEAD_HEADER_STRING)//     0x0722
ILSTRDEF(IL_PCD_PICNUM)//              0x0723

ILSTRDEF(IL_PNG_ALPHA_INDEX)// 0x0724 //XIX : ILint : the color in the pallete at this index value (0-255) is considered transparent, -1 for no trasparent color

// DXTC definitions
ILSTRDEF(IL_DXTC_FORMAT)//      0x0705
ILSTRDEF(IL_DXT1)//             0x0706
ILSTRDEF(IL_DXT2)//             0x0707
ILSTRDEF(IL_DXT3)//             0x0708
ILSTRDEF(IL_DXT4)//             0x0709
ILSTRDEF(IL_DXT5)//             0x070A
ILSTRDEF(IL_DXT_NO_COMP)//      0x070B
ILSTRDEF(IL_KEEP_DXTC_DATA)//   0x070C
ILSTRDEF(IL_DXTC_DATA_FORMAT)// 0x070D
ILSTRDEF(IL_3DC)//              0x070E



// Values
ILSTRDEF(IL_VERSION_NUM)//           0x0DE2
ILSTRDEF(IL_IMAGE_WIDTH)//           0x0DE4
ILSTRDEF(IL_IMAGE_HEIGHT)//          0x0DE5
ILSTRDEF(IL_IMAGE_DEPTH)//           0x0DE6
ILSTRDEF(IL_IMAGE_SIZE_OF_DATA)//    0x0DE7
ILSTRDEF(IL_IMAGE_BPP)//             0x0DE8
ILSTRDEF(IL_IMAGE_BYTES_PER_PIXEL)// 0x0DE8
ILSTRDEF(IL_IMAGE_BPP)//             0x0DE8
ILSTRDEF(IL_IMAGE_BITS_PER_PIXEL)//  0x0DE9
ILSTRDEF(IL_IMAGE_FORMAT)//          0x0DEA
ILSTRDEF(IL_IMAGE_TYPE)//            0x0DEB
ILSTRDEF(IL_PALETTE_TYPE)//          0x0DEC
ILSTRDEF(IL_PALETTE_SIZE)//          0x0DED
ILSTRDEF(IL_PALETTE_BPP)//           0x0DEE
ILSTRDEF(IL_PALETTE_NUM_COLS)//      0x0DEF
ILSTRDEF(IL_PALETTE_BASE_TYPE)//     0x0DF0
ILSTRDEF(IL_NUM_IMAGES)//            0x0DF1
ILSTRDEF(IL_NUM_MIPMAPS)//           0x0DF2
ILSTRDEF(IL_NUM_LAYERS)//            0x0DF3
ILSTRDEF(IL_ACTIVE_IMAGE)//          0x0DF4
ILSTRDEF(IL_ACTIVE_MIPMAP)//         0x0DF5
ILSTRDEF(IL_ACTIVE_LAYER)//          0x0DF6
ILSTRDEF(IL_CUR_IMAGE)//             0x0DF7
ILSTRDEF(IL_IMAGE_DURATION)//        0x0DF8
ILSTRDEF(IL_IMAGE_PLANESIZE)//       0x0DF9
ILSTRDEF(IL_IMAGE_BPC)//             0x0DFA
ILSTRDEF(IL_IMAGE_OFFX)//            0x0DFB
ILSTRDEF(IL_IMAGE_OFFY)//            0x0DFC
ILSTRDEF(IL_IMAGE_CUBEFLAGS)//       0x0DFD
ILSTRDEF(IL_IMAGE_ORIGIN)//          0x0DFE
ILSTRDEF(IL_IMAGE_CHANNELS)//        0x0DFF

{0,0}
};



//
// IL-specific #define's
//

const strnum IL_STR_OTHER_DEFS[]=
{

//ILSTRDEF(IL_VERSION_1_6_7)// 1
ILSTRDEF(IL_VERSION)//       167


// Attribute Bits
ILSTRDEF(IL_ORIGIN_BIT)//          0x00000001
ILSTRDEF(IL_FILE_BIT)//            0x00000002
ILSTRDEF(IL_PAL_BIT)//             0x00000004
ILSTRDEF(IL_FORMAT_BIT)//          0x00000008
ILSTRDEF(IL_TYPE_BIT)//            0x00000010
ILSTRDEF(IL_COMPRESS_BIT)//        0x00000020
ILSTRDEF(IL_LOADFAIL_BIT)//        0x00000040
ILSTRDEF(IL_FORMAT_SPECIFIC_BIT)// 0x00000080
ILSTRDEF(IL_ALL_ATTRIB_BITS)//     0x000FFFFF

// Cube map definitions
ILSTRDEF(IL_CUBEMAP_POSITIVEX)// 0x00000400
ILSTRDEF(IL_CUBEMAP_NEGATIVEX)// 0x00000800
ILSTRDEF(IL_CUBEMAP_POSITIVEY)// 0x00001000
ILSTRDEF(IL_CUBEMAP_NEGATIVEY)// 0x00002000
ILSTRDEF(IL_CUBEMAP_POSITIVEZ)// 0x00004000
ILSTRDEF(IL_CUBEMAP_NEGATIVEZ)// 0x00008000

{0,0}
};



/*----------------------------------------------------------------------------------------------------------------------------*/
//
// base type handling
//
/*----------------------------------------------------------------------------------------------------------------------------*/

//
// check for DevIL errors
//
int lua_devil_error_check (lua_State *l)
{
ILenum err;

	err=ilGetError();

	if(err!=IL_NO_ERROR)
	{
		luaL_error(l, "DevIL ERROR %s",iluErrorString(err));
	}

	return 0;
}

//
// check meta table is valid and get data
//
devilhandle *lua_devil_to_imagep (lua_State *L, int index)
{
devilhandle *p;
	
	p = (devilhandle *)luaL_checkudata(L, index, DEVILHANDLE);

	if (p == NULL)
	{
		luaL_argerror(L, index, "bad " DEVILHANDLE);
	}

	return p;
}

//
// is image open or closed?
//
static int lua_devil_image_type (lua_State *L)
{
devilhandle *p;
		
	p = (devilhandle *)luaL_checkudata(L, 1, DEVILHANDLE);

	if (p == NULL)
	{
		lua_pushnil(L);
	}
	else if (p->image == NULL)
	{
		lua_pushliteral(L, "closed image");
	}
	else
	{
		lua_pushliteral(L, "image");
	}

	return 1;
}

//
// check image is allocated and get data
//
devilhandle *lua_devil_to_image (lua_State *l, int index)
{
devilhandle *p;

	p=lua_devil_to_imagep(l, index);

	if (p->image == NULL)
	{
		luaL_error(l, "attempt to use a closed image");
	}

	return p;
}


/*----------------------------------------------------------------------------------------------------------------------------*/
//
// string to enum type functions, tables live in lua so memory for the string is already allocated and wont
// need to be GCd also you can see what the available options are by dumping the tables
//
// used to erorr check function calls and get the required value
//
/*----------------------------------------------------------------------------------------------------------------------------*/

static u32 lua_devil_to_enum (lua_State *l, int index)
{
u32 ret;

	lua_pushstring(l, LUA_DEVILLIBNAME);
	lua_gettable(l, LUA_GLOBALSINDEX);

	lua_pushstring(l,"ENUM");
	lua_gettable(l, -2);
	
	lua_pushvalue(l,index);
	lua_gettable(l, -2);

	if( lua_type(l,-1) != LUA_TNUMBER )
	{
		luaL_error(l, "invalid DevIL ENUM string \"%s\"",lua_tostring(l,index));
	}

	ret=(u32)lua_tonumber(l, -1);

	lua_pop(l,3); // remove junk from stack

	return ret;
}


static u32 lua_devil_to_defs (lua_State *l, int index)
{
u32 ret;

	lua_pushstring(l, LUA_DEVILLIBNAME);
	lua_gettable(l, LUA_GLOBALSINDEX);

	lua_pushstring(l,"DEFS");
	lua_gettable(l, -2);
	
	lua_pushvalue(l,index);
	lua_gettable(l, -2);

	if( lua_type(l,-1) != LUA_TNUMBER )
	{
		luaL_error(l, "invalid DevIL DEFS string \"%s\"",lua_tostring(l,index));
	}

	ret=(u32)lua_tonumber(l, -1);

	lua_pop(l,3); // remove junk from stack

	return ret;
}

/*----------------------------------------------------------------------------------------------------------------------------*/
//
// global settings hint/get/set stuff
//
/*----------------------------------------------------------------------------------------------------------------------------*/

static int lua_devil_hint (lua_State *l)
{
ILenum target;
ILenum mode;

	target=lua_devil_to_enum(l,1);
	mode=lua_devil_to_enum(l,2);

	ilHint(target,mode);
	lua_devil_error_check(l);

	return 0;
}

static int lua_devil_get (lua_State *l)
{
ILenum mode;
ILint num;

	mode=lua_devil_to_enum(l,1);

	num=ilGetInteger(mode);
	lua_devil_error_check(l);

	lua_pushnumber(l,num);

	return 1;
}

static int lua_devil_image_get (lua_State *l)
{
devilhandle *p;
ILenum mode;
ILint num;

	p=lua_devil_to_imagep(l,1);
	lua_devil_bind_imagep(p);
	lua_devil_error_check(l);

	mode=lua_devil_to_enum(l,2);

	num=ilGetInteger(mode);
	lua_devil_error_check(l);

	lua_pushnumber(l,num);

	return 1;
}

static int lua_devil_set (lua_State *l)
{
ILenum mode;
ILint num;

	mode=lua_devil_to_enum(l,1);
	num=(ILint)lua_tonumber(l,2);

	ilSetInteger(mode,num);
	lua_devil_error_check(l);

	return 0;
}

static int lua_devil_image_set (lua_State *l)
{
devilhandle *p;
ILenum mode;
ILint num;

	p=lua_devil_to_imagep(l,1);
	lua_devil_bind_imagep(p);
	lua_devil_error_check(l);

	mode=lua_devil_to_enum(l,2);
	num=(ILint)lua_tonumber(l,3);

	ilSetInteger(mode,num);
	lua_devil_error_check(l);

	return 0;
}

static int lua_devil_getstr (lua_State *l)
{
ILenum mode;
const char *str;

	mode=lua_devil_to_enum(l,1);

	str=ilGetString(mode);
	lua_devil_error_check(l);

	lua_pushstring(l,str);

	return 1;
}

static int lua_devil_image_getstr (lua_State *l)
{
devilhandle *p;
ILenum mode;
const char *str;

	p=lua_devil_to_imagep(l,1);
	lua_devil_bind_imagep(p);
	lua_devil_error_check(l);

	mode=lua_devil_to_enum(l,2);

	str=ilGetString(mode);
	lua_devil_error_check(l);

	lua_pushstring(l,str);

	return 1;
}

static int lua_devil_setstr (lua_State *l)
{
ILenum mode;
const char *str;

	mode=lua_devil_to_enum(l,1);
	str=lua_tostring(l,2);

	ilSetString(mode,str);
	lua_devil_error_check(l);

	return 0;
}

static int lua_devil_image_setstr (lua_State *l)
{
devilhandle *p;
ILenum mode;
const char *str;

	p=lua_devil_to_imagep(l,1);
	lua_devil_bind_imagep(p);
	lua_devil_error_check(l);

	mode=lua_devil_to_enum(l,2);
	str=lua_tostring(l,3);

	ilSetString(mode,str);
	lua_devil_error_check(l);

	return 0;
}

static int lua_devil_push (lua_State *l)
{
	ilPushAttrib(IL_ALL_ATTRIB_BITS);
	lua_devil_error_check(l);

	return 0;
}

static int lua_devil_pop (lua_State *l)
{
	ilPopAttrib();
	lua_devil_error_check(l);

	return 0;
}

/*----------------------------------------------------------------------------------------------------------------------------*/
//
// alloc/free/bind
//
/*----------------------------------------------------------------------------------------------------------------------------*/


static int lua_devil_alloc_imagep (lua_State *l)
{
devilhandle *p;
	
	p = (devilhandle *)lua_newuserdata(l, sizeof(devilhandle));
	
	
	p->image=0;
	p->frame=0;
	p->layer=0;
	p->mipmap=0;


	luaL_getmetatable(l, DEVILHANDLE);
	lua_setmetatable(l, -2);

	return 1;
}

static int lua_devil_alloc_image (lua_State *l)
{
devilhandle *p;

	lua_devil_alloc_imagep(l);

	p=lua_devil_to_imagep(l,lua_gettop(l));

	ilGenImages(1,&(p->image));
	lua_devil_error_check(l);

	if(!p->image)
	{
		luaL_error(l, "DevIL ilGenImages failed");
	}

	return 1;
}

static int lua_devil_free_image (lua_State *l)
{
devilhandle *p;
	p=lua_devil_to_imagep(l,1);

	if(p->image)
	{
		ilDeleteImages(1,&(p->image));
		lua_devil_error_check(l);
		p->image=0;
	}

	return 0;
}


void lua_devil_bind_imagep (devilhandle *p)
{
	ilBindImage(p->image);

	if(p->frame)
	{
		ilActiveImage(p->frame);
	}

	if(p->layer)
	{
		ilActiveLayer(p->layer);
	}

	if(p->mipmap)
	{
		ilActiveMipmap(p->mipmap);
	}
}

static int lua_devil_clone_image (lua_State *l)
{
devilhandle *p;
devilhandle *p2;
	p=lua_devil_to_imagep(l,1);

	lua_devil_alloc_imagep(l);

	p2=lua_devil_to_imagep(l,lua_gettop(l));

	lua_devil_bind_imagep(p);
	lua_devil_error_check(l);

	p2->image=ilCloneCurImage();
	lua_devil_error_check(l);

	return 1;
}

static int lua_devil_copyto_image (lua_State *l)
{
devilhandle *p;
devilhandle *p2;
	p=lua_devil_to_imagep(l,1);
	p2=lua_devil_to_imagep(l,2);


	lua_devil_bind_imagep(p);
	lua_devil_error_check(l);

	ilCopyImage(p2->image);
	lua_devil_error_check(l);

	return 0;
}

static int lua_devil_copyto_pal (lua_State *l)
{
devilhandle *p;
devilhandle *p2;
	p=lua_devil_to_imagep(l,1);
	p2=lua_devil_to_imagep(l,2);

u8 *op;
u8 *np;
s32 siz;
//s32 i;

ILpal *pp;

	lua_devil_bind_imagep(p2);
	lua_devil_error_check(l);

	op=ilGetPalette();
	lua_devil_error_check(l);

	pp=&iCurImage->Pal;


	lua_devil_bind_imagep(p);
	lua_devil_error_check(l);

	np=ilGetPalette();
	lua_devil_error_check(l);


	siz=ilGetInteger(IL_PALETTE_NUM_COLS)*3;
	lua_devil_error_check(l);


	ilSetPal(pp);


/*
	for(i=0;i<siz;i++)
	{
		np[i]=op[i];
	}
*/

	return 0;
}

/*----------------------------------------------------------------------------------------------------------------------------*/
//
// load/save
//
/*----------------------------------------------------------------------------------------------------------------------------*/
static int lua_devil_load (lua_State *l)
{
devilhandle *p;
const char *str;

	p=lua_devil_to_image(l,1);
	str=lua_tostring(l,2);

	lua_devil_bind_imagep(p);
	lua_devil_error_check(l);
	
	ilLoadImage((char*const)str);
	lua_devil_error_check(l);


	return 0;
}

static int lua_devil_save (lua_State *l)
{
devilhandle *p;
const char *str;

	p=lua_devil_to_image(l,1);
	str=lua_tostring(l,2);

	lua_devil_bind_imagep(p);
	lua_devil_error_check(l);

	ilSaveImage((char*const)str);
	lua_devil_error_check(l);

	return 0;
}

static int lua_devil_save_pal (lua_State *l)
{
devilhandle *p;
const char *str;

	p=lua_devil_to_image(l,1);
	str=lua_tostring(l,2);

	lua_devil_bind_imagep(p);
	lua_devil_error_check(l);

	ilSavePal((char*const)str);
	lua_devil_error_check(l);

	return 0;
}

/*----------------------------------------------------------------------------------------------------------------------------*/
//
// transform an image
//
/*----------------------------------------------------------------------------------------------------------------------------*/
static int lua_devil_convert_image (lua_State *l)
{
devilhandle *p;
ILenum format;
ILenum type;


	p=lua_devil_to_image(l,1);
	format=lua_devil_to_enum(l,2);
	type=lua_devil_to_enum(l,3);

	lua_devil_bind_imagep(p);
	lua_devil_error_check(l);

	ilConvertImage(format,type);
	lua_devil_error_check(l);

	return 0;
}

static int lua_devil_convert_pal (lua_State *l)
{
devilhandle *p;
ILenum format;


	p=lua_devil_to_image(l,1);
	format=lua_devil_to_enum(l,2);

	lua_devil_bind_imagep(p);
	lua_devil_error_check(l);

	ilConvertPal(format);
	lua_devil_error_check(l);

	return 0;
}

/*----------------------------------------------------------------------------------------------------------------------------*/
//
// frame/layer/mipmap select
//
/*----------------------------------------------------------------------------------------------------------------------------*/
static int lua_devil_pick_frame (lua_State *l)
{
devilhandle *p;

	p=lua_devil_to_image(l,1);
	p->frame=(ILuint)lua_tonumber(l,2);

	return 0;
}

static int lua_devil_pick_layer (lua_State *l)
{
devilhandle *p;

	p=lua_devil_to_image(l,1);
	p->layer=(ILuint)lua_tonumber(l,2);

	return 0;
}

static int lua_devil_pick_mipmap (lua_State *l)
{
devilhandle *p;

	p=lua_devil_to_image(l,1);
	p->mipmap=(ILuint)lua_tonumber(l,2);

	return 0;
}

/*----------------------------------------------------------------------------------------------------------------------------*/
//
// apply pal/profile
//
/*----------------------------------------------------------------------------------------------------------------------------*/
static int lua_devil_apply_pal (lua_State *l)
{
devilhandle *p;
const char *s1;

	p=lua_devil_to_image(l,1);
	s1=lua_tostring(l,2);


	lua_devil_bind_imagep(p);
	lua_devil_error_check(l);

	ilApplyPal((char*const)s1);
	lua_devil_error_check(l);

	return 0;
}

static int lua_devil_apply_profile (lua_State *l)
{
devilhandle *p;
const char *s1;
const char *s2;

	p=lua_devil_to_image(l,1);
	s1=lua_tostring(l,2);
	s2=lua_tostring(l,3);


	lua_devil_bind_imagep(p);
	lua_devil_error_check(l);

	ilApplyProfile((char*const)s1,(char*const)s2);
	lua_devil_error_check(l);

	return 0;
}

/*----------------------------------------------------------------------------------------------------------------------------*/
//
// create
//
/*----------------------------------------------------------------------------------------------------------------------------*/
static int lua_devil_create (lua_State *l)
{
devilhandle *p;

s32 w,h,d,b;
ILenum format;
ILenum type;

	p=lua_devil_to_image(l,1);

	lua_devil_bind_imagep(p);
	lua_devil_error_check(l);

	w=(s32)luaL_checknumber(l,2);
	h=(s32)luaL_checknumber(l,3);
	d=(s32)luaL_checknumber(l,4);
	b=(s32)luaL_checknumber(l,5);
	format=lua_devil_to_enum(l,6);
	type=lua_devil_to_enum(l,7);
	ilTexImage(w,h,d,b,format,type,0);
	lua_devil_error_check(l);


	ilClearImage();
	lua_devil_error_check(l);

	return 0;
}

/*----------------------------------------------------------------------------------------------------------------------------*/
//
// clear
//
/*----------------------------------------------------------------------------------------------------------------------------*/
static int lua_devil_clear (lua_State *l)
{
devilhandle *p;

	p=lua_devil_to_image(l,1);

	lua_devil_bind_imagep(p);
	lua_devil_error_check(l);

	ilClearImage();
	lua_devil_error_check(l);

	return 0;
}


/*----------------------------------------------------------------------------------------------------------------------------*/
//
// library defs
//
/*----------------------------------------------------------------------------------------------------------------------------*/

static const luaL_reg lua_devil_lib[] =
{
	
	{	"alloc",		lua_devil_alloc_image	},
	{	"free",			lua_devil_free_image	},
	{	"set_hint",		lua_devil_hint	},
	{	"get",			lua_devil_get	},
	{	"set",			lua_devil_set	},
	{	"get_string",	lua_devil_getstr	},
	{	"set_string",	lua_devil_setstr	},
	{	"push",			lua_devil_push	},
	{	"pop",			lua_devil_pop	},
	
	{NULL, NULL}
};


// suport for one type of object, the image

static const luaL_reg lua_devil_meta[] = {
	{"load",			lua_devil_load},
	{"save",			lua_devil_save},
	{"save_pal",		lua_devil_save_pal},
	{"free",			lua_devil_free_image},

	{	"get",			lua_devil_image_get	},
	{	"set",			lua_devil_image_set	},
	{	"get_string",	lua_devil_image_getstr	},
	{	"set_string",	lua_devil_image_setstr	},

	{"clone",			lua_devil_clone_image},
	{"copy",			lua_devil_copyto_image},

	{"copy_pal",		lua_devil_copyto_pal},


	{"create",			lua_devil_create},
	{"clear",			lua_devil_clear},

	{"select_frame",	lua_devil_pick_frame},
	{"select_layer",	lua_devil_pick_layer},
	{"select_mipmap",	lua_devil_pick_mipmap},
	
	{"convert",			lua_devil_convert_image},
	{"convert_pal",		lua_devil_convert_pal},

	{"apply_pal",		lua_devil_apply_pal},
	{"apply_profile",	lua_devil_apply_profile},

	
	{"__gc",			lua_devil_free_image},

	//  {"__tostring", io_tostring},
	{NULL, NULL}
};


static void lua_devil_meta_create (lua_State *l)
{
	luaL_newmetatable(l, DEVILHANDLE);  /* create new metatable */

	lua_pushliteral(l, "__index");
	lua_pushvalue(l, -2);  /* push metatable */
	lua_rawset(l, -3);  /* metatable.__index = metatable */
	luaL_openlib(l, NULL, lua_devil_meta, 0);
}


/*----------------------------------------------------------------------------------------------------------------------------*/
//
// open library.
//
/*----------------------------------------------------------------------------------------------------------------------------*/

int luaopen_devil (lua_State *l)
{
const strnum *p;

	lua_devil_meta_create(l);

	lua_pushstring(l, LUA_DEVILLIBNAME );
	lua_newtable(l);
	luaL_openlib(l, 0, lua_devil_lib, 0);
	lua_rawset(l, LUA_GLOBALSINDEX);


// set sub ENUM table up with all enum string tags

	lua_pushstring(l, LUA_DEVILLIBNAME);
	lua_gettable(l, LUA_GLOBALSINDEX);

	lua_pushstring(l,"ENUM");
	lua_newtable(l);

	for( p=IL_STR_ENUM_DEFS ; p->str ; p++ )
	{
		lua_pushstring(l,  p->str );
		lua_pushnumber(l,  p->num );
		lua_rawset(l,-3);
	}

	lua_rawset(l,-3);

// set sub DEFS table up with all other string tags

	lua_pushstring(l,"DEFS");
	lua_newtable(l);

	for( p=IL_STR_OTHER_DEFS ; p->str ; p++ )
	{
		lua_pushstring(l,  p->str );
		lua_pushnumber(l,  p->num );
		lua_rawset(l,-3);
	}

	lua_rawset(l,-3);

	lua_remove(l,-1);



	return 1;
}


