/*
-- Copyright (C) 2012 Kriss Blank < Kriss@XIXs.com >
-- This file is distributed under the terms of the MIT license.
-- http://en.wikipedia.org/wiki/MIT_License
-- Please ping me if you use it for anything cool...
*/

#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"


#include <errno.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
//#include <opencv2/core/core.hpp>
//#include <opencv2/highgui/highgui.hpp>


#include "code/lua_v4l2.h"

#include "wet_types.h"
#include "grd.h"
#include "lua_grd.h"

/*
 *  V4L2 video capture example
 *
 *  This program can be used and distributed without restrictions.
 *
 *      This program is provided with the V4L2 API
 * see http://linuxtv.org/docs.php for more information
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

//#include <getopt.h>             /* getopt_long() */

#include <fcntl.h>              /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <linux/videodev2.h>


#define CLEAR(x) memset(&(x), 0, sizeof(x))


//
// we can use either this string as a string identifier
// or its address as a light userdata identifier, both unique
//
const char *lua_v4l2_ptr_name="v4l2*ptr";


// the data pointer we are using
typedef struct lua_v4l2_struct part_ptr ;



#ifndef V4L2_PIX_FMT_Y10B
#define V4L2_PIX_FMT_Y10B     v4l2_fourcc('Y', '1', '0', 'B')
#endif





//
// some 'slow' YUYV convert codes
//

static __inline int32_t clamp0(int32_t v) {
  return ((-(v) >> 31) & (v));
}

static __inline int32_t clamp255(int32_t v) {
  return (((255 - (v)) >> 31) | (v)) & 255;
}

static __inline uint32_t Clamp(int32_t val) {
  int v = clamp0(val);
  return (uint32_t)(clamp255(v));
}

static __inline uint32_t Abs(int32_t v) {
  int m = v >> 31;
  return (v + m) ^ m;
}



// BT.601 YUV to RGB reference
//  R = (Y - 16) * 1.164              - V * -1.596
//  G = (Y - 16) * 1.164 - U *  0.391 - V *  0.813
//  B = (Y - 16) * 1.164 - U * -2.018

// Y contribution to R,G,B.  Scale and bias.
// TODO(fbarchard): Consider moving constants into a common header.
#define YG 18997 /* round(1.164 * 64 * 256 * 256 / 257) */
#define YGB -1160 /* 1.164 * 64 * -16 + 64 / 2 */

// U and V contributions to R,G,B.
#define UB -128 /* max(-128, round(-2.018 * 64)) */
#define UG 25 /* round(0.391 * 64) */
#define VG 52 /* round(0.813 * 64) */
#define VR -102 /* round(-1.596 * 64) */

// Bias values to subtract 16 from Y and 128 from U and V.
#define BB (UB * 128            + YGB)
#define BG (UG * 128 + VG * 128 + YGB)
#define BR            (VR * 128 + YGB)

// C reference code that mimics the YUV assembly.
static __inline void YuvPixel(uint8_t y, uint8_t u, uint8_t v,
                              uint8_t* b, uint8_t* g, uint8_t* r) {
  uint32_t y1 = (uint32_t)(y * 0x0101 * YG) >> 16;
  *b = Clamp((int32_t)(-(         u * UB) + y1 + BB) >> 6);
  *g = Clamp((int32_t)(-(v * VG + u * UG) + y1 + BG) >> 6);
  *r = Clamp((int32_t)(-(v * VR         ) + y1 + BR) >> 6);
}

// C reference code that mimics the YUV assembly.
static __inline void YPixel(uint8_t y, uint8_t* b, uint8_t* g, uint8_t* r) {
  uint32_t y1 = (uint32_t)(y * 0x0101 * YG) >> 16;
  *b = Clamp((int32_t)(y1 + YGB) >> 6);
  *g = Clamp((int32_t)(y1 + YGB) >> 6);
  *r = Clamp((int32_t)(y1 + YGB) >> 6);
}


// 10bits in 24bits out, do 40bits at a time (5bytes in -> 12bytes out)
// so width *must* be multiple of 4
static __inline void Y10B_to_RGB(uint8_t* p, uint8_t* rgb) {

	uint32_t y = (*p++)<<8;

	y |= (*p++);

	rgb[ 0] = ((y   )&0xc0);
	rgb[ 1] = ((y>>8)&0xff);
	rgb[ 2] = 0x00;

	y = (y<<8) | (*p++);

	rgb[ 3] = ((y<<2)&0xc0);
	rgb[ 4] = ((y>>6)&0xff);
	rgb[ 5] = 0x00;

	y = (y<<8) | (*p++);

	rgb[ 6] = ((y<<4)&0xc0);
	rgb[ 7] = ((y>>4)&0xff);
	rgb[ 8] = 0x00;

	y = (y<<8) | (*p++);

	rgb[ 9] = ((y<<6)&0xc0);
	rgb[10] = ((y>>2)&0xff);
	rgb[11] = 0x00;
}


#undef YG
#undef YGB
#undef UB
#undef UG
#undef VG
#undef VR
#undef BB
#undef BG
#undef BR

// ioctrl with auto retry
static int xioctl(int fh, int request, void *arg)
{
	int r;
	do { r = ioctl(fh, request, arg); }	while(-1 == r && EINTR == errno);
	return r;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// check that a userdata at the given index is a v4l2 object
// return the part_ptr if it does, otherwise return 0
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
part_ptr *lua_v4l2_get_ptr (lua_State *l, int idx,int check)
{
part_ptr *p=0;

	p = ((part_ptr *)luaL_checkudata(l, idx , lua_v4l2_ptr_name));

	if(check)
	{
		if (-1==p->fd)
		{
			luaL_error(l, "bad v4l2 file" );
		}
	}
	
	return p;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// open file
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_v4l2_open(lua_State *l)
{
part_ptr *p;
struct stat st;
const char *dev_name="/dev/video0";

	if(lua_isstring(l,1))
	{
		dev_name=lua_tostring(l,1);
	}


	if (-1 == stat(dev_name, &st))
	{
		lua_pushnil(l);
		lua_pushfstring(l, "Cannot identify '%s': %d, %s",dev_name, errno, strerror(errno));
		return 2;
	}

	if (!S_ISCHR(st.st_mode))
	{
		lua_pushnil(l);
		lua_pushfstring(l, "%s is no device", dev_name);
		return 2;
	}

	p = (part_ptr *)lua_newuserdata(l, sizeof(part_ptr));
	p->fd=-1;
	p->buffer_data=0;
	luaL_getmetatable(l, lua_v4l2_ptr_name);
	lua_setmetatable(l, -2);

	p->fd=open(dev_name, O_RDWR | O_NONBLOCK, 0);
	if(-1 == p->fd)
	{
		lua_pop(l,1); // remove return value
		lua_pushnil(l);
		lua_pushfstring(l, "Cannot open '%s': %d, %s",dev_name, errno, strerror(errno));
		return 2;
	}
	
	return 1; // success
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// close file
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_v4l2_close (lua_State *l)
{
part_ptr *p=lua_v4l2_get_ptr(l,1,0);

	if(p->fd!=-1)
	{
		close(p->fd);
//printf("file %d closed\n",p->fd);
		p->fd=-1;
	}

	if(p->buffer_data)
	{
		free(p->buffer_data);
		p->buffer_data=0;
	}

	return 0;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// return possible capture formats and sizes in nested tables
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_v4l2_capture_list (lua_State *l)
{
part_ptr *p=lua_v4l2_get_ptr(l,1,1);

	struct v4l2_fmtdesc fmt;
	struct v4l2_frmsizeenum frmsize;
	struct v4l2_frmivalenum frmival;

	lua_newtable(l);
	fmt.index = 0;
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	while (ioctl(p->fd, VIDIOC_ENUM_FMT, &fmt) >= 0)
	{
		lua_newtable(l);
		lua_pushliteral(l,"name");		lua_pushstring(l,(const char *)fmt.description); lua_rawset(l,-3);
		lua_pushliteral(l,"flags");		lua_pushnumber(l,fmt.flags); lua_rawset(l,-3);
		lua_pushliteral(l,"type");		lua_pushnumber(l,fmt.type); lua_rawset(l,-3);
		lua_pushliteral(l,"format");	lua_pushfstring(l,"%c%c%c%c",
											(fmt.pixelformat>>0)&0xff,
											(fmt.pixelformat>>8)&0xff,
											(fmt.pixelformat>>16)&0xff,
											(fmt.pixelformat>>24)&0xff); lua_rawset(l,-3);

		lua_pushliteral(l,"sizes");
		lua_newtable(l);
		frmsize.pixel_format = fmt.pixelformat;
		frmsize.index = 0;
		while (ioctl(p->fd, VIDIOC_ENUM_FRAMESIZES, &frmsize) >= 0)
		{
			lua_newtable(l);

			lua_pushliteral(l,"type"); lua_pushnumber(l,frmsize.type); lua_rawset(l,-3);
			if(frmsize.type == V4L2_FRMSIZE_TYPE_STEPWISE)
			{
				lua_pushliteral(l,"min_width");		lua_pushnumber(l,frmsize.stepwise.min_width);	lua_rawset(l,-3);
				lua_pushliteral(l,"min_height");	lua_pushnumber(l,frmsize.stepwise.min_height);	lua_rawset(l,-3);
				lua_pushliteral(l,"max_width");		lua_pushnumber(l,frmsize.stepwise.max_width);	lua_rawset(l,-3);
				lua_pushliteral(l,"max_height");	lua_pushnumber(l,frmsize.stepwise.max_height);	lua_rawset(l,-3);
				lua_pushliteral(l,"step_width");	lua_pushnumber(l,frmsize.stepwise.step_width);	lua_rawset(l,-3);
				lua_pushliteral(l,"step_height");	lua_pushnumber(l,frmsize.stepwise.step_height);	lua_rawset(l,-3);
			}
			else
			if (frmsize.type == V4L2_FRMSIZE_TYPE_DISCRETE)
			{
				lua_pushliteral(l,"width");		lua_pushnumber(l,frmsize.discrete.width);	lua_rawset(l,-3);
				lua_pushliteral(l,"height");	lua_pushnumber(l,frmsize.discrete.height);	lua_rawset(l,-3);

				frmival.index = 0;
				frmival.pixel_format = fmt.pixelformat;
				frmival.width = frmsize.discrete.width;
				frmival.height = frmsize.discrete.height;
				
				lua_pushliteral(l,"intervals");
				lua_newtable(l);
				while (ioctl(p->fd, VIDIOC_ENUM_FRAMEINTERVALS, &frmival) >= 0)
				{
					lua_newtable(l);

					if (frmival.type == V4L2_FRMIVAL_TYPE_DISCRETE)
					{
						lua_pushliteral(l,"rate");
						lua_pushnumber(l,	((double)frmival.discrete.numerator) / 
											((double)frmival.discrete.denominator) );	lua_rawset(l,-3);
					}
					else
					if(frmival.type == V4L2_FRMIVAL_TYPE_STEPWISE)
					{
						lua_pushliteral(l,"min_rate");
						lua_pushnumber(l,	((double)frmival.stepwise.min.numerator) / 
											((double)frmival.stepwise.min.denominator) );	lua_rawset(l,-3);
						lua_pushliteral(l,"max_rate");
						lua_pushnumber(l,	((double)frmival.stepwise.max.numerator) / 
											((double)frmival.stepwise.max.denominator) );	lua_rawset(l,-3);
						lua_pushliteral(l,"step_rate");
						lua_pushnumber(l,	((double)frmival.stepwise.step.numerator) /
											((double)frmival.stepwise.step.denominator) );	lua_rawset(l,-3);
					}
					frmival.index++;
					lua_rawseti(l,-2,frmival.index);
				}
				lua_rawset(l,-3);
			}
			frmsize.index++;
			lua_rawseti(l,-2,frmsize.index);
		}
		lua_rawset(l,-3);
		fmt.index++;	
		lua_rawseti(l,-2,fmt.index);
	}
	return 1;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// initialise capture mode and start capture
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_v4l2_capture_start (lua_State *l)
{
part_ptr *p=lua_v4l2_get_ptr(l,1,1);

struct v4l2_capability cap;
struct v4l2_cropcap cropcap;
struct v4l2_crop crop;
struct v4l2_format fmt;
struct v4l2_requestbuffers req;
enum v4l2_buf_type type;

unsigned int min;
int i;
const char *s;

	p->width=480;
	p->height=480;
	p->format=V4L2_PIX_FMT_YUYV;
	p->buffer_count=4;

	if(lua_istable(l,2))
	{
		lua_getfield(l,2,"width");
		if(lua_isnumber(l,-1)) { p->width=(int)lua_tonumber(l,-1); }
		lua_pop(l,1);

		lua_getfield(l,2,"height");
		if(lua_isnumber(l,-1)) { p->height=(int)lua_tonumber(l,-1); }
		lua_pop(l,1);

		lua_getfield(l,2,"format");
		if(lua_isnumber(l,-1)) { p->format=(int)lua_tonumber(l,-1); }
		if(lua_isstring(l,-1))
		{
			s=lua_tostring(l,-1);
			p->format=v4l2_fourcc(s[0],s[1],s[2],s[3]);
		}
		lua_pop(l,1);

		lua_getfield(l,2,"buffer_count");
		if(lua_isnumber(l,-1)) { p->buffer_count=(int)lua_tonumber(l,-1); }
		lua_pop(l,1);
	}

	if (-1 == xioctl(p->fd, VIDIOC_QUERYCAP, &cap))
	{
		luaL_error(l, "VIDIOC_QUERYCAP" );
	}

	if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
	{
		luaL_error(l, "not a capture device" );
	}

	if (!(cap.capabilities & V4L2_CAP_STREAMING))
	{
		luaL_error(l, "device does not support streaming" );
	}

	CLEAR(cropcap);
	cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if (0 == xioctl(p->fd, VIDIOC_CROPCAP, &cropcap))
	{
		crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		crop.c = cropcap.defrect; /* reset to default */

		if (-1 == xioctl(p->fd, VIDIOC_S_CROP, &crop))
		{
			switch (errno)
			{
				case EINVAL:
					/* Cropping not supported. */
				break;
				default:
					/* Errors ignored. */
				break;
			}
		}
	}
	else
	{
		/* Errors ignored. */
	}

	CLEAR(fmt);
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width       = p->width; 	//replace
	fmt.fmt.pix.height      = p->height; 	//replace
	fmt.fmt.pix.pixelformat = p->format; 	//replace
	fmt.fmt.pix.field       = V4L2_FIELD_ANY;

//printf(" %d %d %d \n",fmt.fmt.pix.width , fmt.fmt.pix.height , fmt.fmt.pix.pixelformat);

	if(-1 == xioctl(p->fd, VIDIOC_S_FMT, &fmt))
	{
		luaL_error(l, "invalid format" );
	}

// may have a different format out this end?
	p->width=fmt.fmt.pix.width;
	p->height=fmt.fmt.pix.height;
	p->format=fmt.fmt.pix.pixelformat;

	p->buffer_size=fmt.fmt.pix.sizeimage;

	p->buffer_size=(p->buffer_size+15)&0xfffffff0; // round size of each buffer up

	CLEAR(req);

	req.count  = p->buffer_count;
	req.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_USERPTR;

	if (-1 == xioctl(p->fd, VIDIOC_REQBUFS, &req))
	{
		if (EINVAL == errno)
		{
			luaL_error(l, "userptr not supported" );
		}
		else
		{
			luaL_error(l, "VIDIOC_REQBUFS" );
		}
	}

	p->buffer_data = malloc( p->buffer_count * p->buffer_size );
	
	if( ! p->buffer_data )
	{
		luaL_error(l, "buffer alloc failed" );
	}

// tell device where our buffers are
	for (i = 0; i < p->buffer_count; ++i)
	{
		struct v4l2_buffer buf;
		CLEAR(buf);
		buf.index = i;
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_USERPTR;
		buf.m.userptr = (unsigned long) (p->buffer_data + (i*p->buffer_size)) ;
		buf.length = p->buffer_size;

		if (-1 == xioctl(p->fd, VIDIOC_QBUF, &buf))
		{
			luaL_error(l, "VIDIOC_QBUF" );
		}
	}

// and start capture
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (-1 == xioctl(p->fd, VIDIOC_STREAMON, &type))
	{
		luaL_error(l, "VIDIOC_STREAMON" );
	}

	return 0; // no return but an error will have been raised on failure
}




/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// stop capture
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_v4l2_capture_stop (lua_State *l)
{
part_ptr *p=lua_v4l2_get_ptr(l,1,1);

	enum v4l2_buf_type type;
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (-1 == xioctl(p->fd, VIDIOC_STREAMOFF, &type))
	{
		luaL_error(l, "VIDIOC_STREAMOFF" );
	}
	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// stop capture
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_v4l2_info (lua_State *l)
{
part_ptr *p=lua_v4l2_get_ptr(l,1,1);

char format[5];

	if(lua_istable(l,2)) // reuse
	{
		lua_pushvalue(l,2);
	}
	else
	{
		lua_newtable(l);
	}

	lua_pushliteral(l,"buffer_size");		lua_pushnumber(l,p->buffer_size);		lua_rawset(l,-3);
	lua_pushliteral(l,"buffer_count");		lua_pushnumber(l,p->buffer_count);		lua_rawset(l,-3);

	lua_pushliteral(l,"width");		lua_pushnumber(l,p->width);		lua_rawset(l,-3);
	lua_pushliteral(l,"height");	lua_pushnumber(l,p->height);	lua_rawset(l,-3);
	
	format[0]=(p->format>>0)&0xff;
	format[1]=(p->format>>8)&0xff;
	format[2]=(p->format>>16)&0xff;
	format[3]=(p->format>>24)&0xff;
	format[4]=0;
	lua_pushliteral(l,"format");	lua_pushstring(l,format);	lua_rawset(l,-3);

	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// read GRD
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_v4l2_capture_read_grd (lua_State *l)
{
part_ptr *p=lua_v4l2_get_ptr(l,1,1);

struct v4l2_buffer buf;
int i;

uint8_t *pp=0;
uint8_t *bp=0;
uint8_t *cp=0;
int size;
struct grd **gg=0;

	if(lua_isuserdata(l,2))
	{
		gg=lua_grd_get_ptr(l,2); // might have one passed in
	}

	CLEAR(buf);
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_USERPTR;

	if (-1 == xioctl(p->fd, VIDIOC_DQBUF, &buf))
	{
		switch(errno)
		{
			case EAGAIN:
			return 0;

			case EIO:
			default:
				luaL_error(l,"VIDIOC_DQBUF");
		}
	}

	pp=(uint8_t*)buf.m.userptr;
	size=buf.bytesused;

	if(gg) // use passed in
	{
		if	(
				((*gg)->bmap->fmt == GRD_FMT_U8_RGB)	&&
				((*gg)->bmap->w   == p->width) 			&&
				((*gg)->bmap->h   == p->height)
			)
		{
			// we can reuse
		}
		else
		{
			gg=0; // re-allocate
		}
	}
	
	if(!gg) // allocate
	{
		gg=lua_grd_create_ptr(l);
		*gg=grd_create(GRD_FMT_U8_RGB,p->width,p->height,1);
	}
	if(*gg)
	{
		switch(p->format)
		{
			case V4L2_PIX_FMT_YUYV:

				for( cp=(uint8_t*)pp , bp=(*gg)->bmap->data ; bp<(*gg)->bmap->data+(*gg)->bmap->zscan ; cp=cp+4 , bp=bp+6 )
				{
					YuvPixel(cp[0], cp[1], cp[3], bp+2, bp+1, bp+0);
					YuvPixel(cp[2], cp[1], cp[3], bp+5, bp+4, bp+3);
				}

			break;
			case V4L2_PIX_FMT_UYVY:

				for( cp=(uint8_t*)pp , bp=(*gg)->bmap->data ; bp<(*gg)->bmap->data+(*gg)->bmap->zscan ; cp=cp+4 , bp=bp+6 )
				{
					YuvPixel(cp[1], cp[0], cp[2], bp+2, bp+1, bp+0);
					YuvPixel(cp[3], cp[0], cp[2], bp+5, bp+4, bp+3);
				}

			break;

			case V4L2_PIX_FMT_Y10B:

				for( cp=(uint8_t*)pp , bp=(*gg)->bmap->data ; bp<(*gg)->bmap->data+(*gg)->bmap->zscan ; cp=cp+5 , bp=bp+12 )
				{
					Y10B_to_RGB(cp,bp);
				}

			break;
		}
	}

	if (-1 == xioctl(p->fd, VIDIOC_QBUF, &buf))
	{
			luaL_error(l,"VIDIOC_QBUF");
	}

	return 1;
}



/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// open library.
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
LUALIB_API int luaopen_wetgenes_v4l2_core (lua_State *l)
{
	const luaL_Reg lib[] =
	{
		{"open",			lua_v4l2_open},
		{"close",			lua_v4l2_close},
		{"info",			lua_v4l2_info},

		{"capture_list",		lua_v4l2_capture_list},
		{"capture_start",		lua_v4l2_capture_start},
		{"capture_read_grd",	lua_v4l2_capture_read_grd},
		{"capture_stop",		lua_v4l2_capture_stop},

		{0,0}
	};
		
	const luaL_Reg meta[] =
	{
		{"__gc",			lua_v4l2_close},

		{0,0}
	};

	luaL_newmetatable(l, lua_v4l2_ptr_name);
	luaL_openlib(l, NULL, meta, 0);
	lua_pop(l,1);

	lua_newtable(l);
	luaL_openlib(l, NULL, lib, 0);
	return 1;
}







