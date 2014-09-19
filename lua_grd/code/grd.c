/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss@XIXs.com 2013
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
#include "all.h"

//
// GRD Image handling layer, load/save/manipulate
//

//
// this code makes an assumption of little endian, needs some if defs and slightly different conversion code
// to run on bigendian hardware, we will leave that for later...
//


void grdinfo_reset(struct grd_info *gi)
{
	gi->fmt=GRD_FMT_NONE;
	gi->flags=0;

	gi->w=0;
	gi->h=0;
	gi->d=0;

	gi->xscan=0;
	gi->yscan=0;
	gi->zscan=0;

	gi->data=0;
}

void grdinfo_set(  struct grd_info *gi , struct grd_info *ga )
{
	gi->fmt=ga->fmt;
	gi->flags=ga->flags;

	gi->w=ga->w;
	gi->h=ga->h;
	gi->d=ga->d;

	gi->xscan=ga->xscan;
	gi->yscan=ga->yscan;
	gi->zscan=ga->zscan;

	gi->data=ga->data;
}
	
u8 * grdinfo_get_data( struct grd_info *gi , s32 x, s32 y, s32 z)
{
	return gi->data+(z*gi->zscan)+(y*gi->yscan)+(x*gi->xscan);
}

/*
u8 * grdinfo_get_data( struct grd_info *ga  , f32 x, f32 y, f32 z)
{
	return gi->data+(((s32)(gi->z))*gi->zscan)+(((s32)(gi->y))*gi->yscan)+(((s32)(gi->x))*gi->xscan);
}
*/




/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// how big is each pixel for the given format, returns 0 if not known
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int grd_sizeof_pixel(int id)
{
	switch(id)
	{
		case GRD_FMT_U8_ARGB:
		case GRD_FMT_U8_ARGB_PREMULT:
		case GRD_FMT_U8_RGBA:
		case GRD_FMT_U8_RGBA_PREMULT:
			return 4;
		case GRD_FMT_U8_RGB:
			return 3;
		case GRD_FMT_U16_RGBA_5551:
		case GRD_FMT_U16_RGBA_5551_PREMULT:
		case GRD_FMT_U16_RGBA_5650:
		case GRD_FMT_U16_RGBA_5650_PREMULT:
		case GRD_FMT_U16_RGBA_4444:
		case GRD_FMT_U16_RGBA_4444_PREMULT:
			return 2;
		case GRD_FMT_U8_INDEXED:
		case GRD_FMT_U8_INDEXED_PREMULT:
		case GRD_FMT_U8_LUMINANCE:
		case GRD_FMT_U8_ALPHA:
			return 1;
	}
	return 0;
}	
									
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// allocate some data in a grd_info
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void * grd_info_alloc(struct grd_info *gi,  s32 fmt , s32 w, s32 h, s32 d )
{
int ps=grd_sizeof_pixel(fmt);

	grd_info_free(gi); // this resets everything and frees any data it finds
	
	if(ps==0) { goto bogus; } // bad format
	
	gi->fmt=fmt;
	
	if(w*h*d>0) // must all be 1 or more
	{
		gi->data=(u8*)calloc( w*h*d , ps );
		
		if(gi->data)
		{
			gi->w=w;
			gi->h=h;
			gi->d=d;
			gi->xscan=ps;
			gi->yscan=gi->xscan*w;
			gi->zscan=gi->yscan*h;
		}
		else { goto bogus; }
	}
	
	return gi;
bogus:
	grd_info_free(gi);
	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// free any data in a grd_info and reset it
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void grd_info_free(struct grd_info *gi)
{
	if(gi->data)
	{
		if(!(gi->flags&GRD_FLAGS_BORROWED)) // do not free if memory was only borrowed
		{
			free(gi->data);
		}
	}
	grdinfo_reset(gi);
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// reallocate an image, any image data will be lost
// returns 0 on error but does not free the grd you passed in
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
struct grd * grd_realloc( struct grd *g, s32 fmt , s32 w, s32 h, s32 d )
{
u8 *bp;
int i;
	if(g)
	{
		if(!grd_info_alloc(g->bmap, fmt , w, h, d )) { goto bogus; }
		if( (fmt&(~GRD_FMT_PREMULT))==GRD_FMT_U8_INDEXED) // do we need a palette?
		{
			if(!grd_info_alloc(g->cmap, GRD_FMT_U8_RGBA , 256, 1, 1 )) { goto bogus; }
		}
		else
		if(fmt==GRD_FMT_U8_LUMINANCE) // do we need a fake palette?
		{
			if(!grd_info_alloc(g->cmap, GRD_FMT_U8_RGBA , 256, 1, 1 )) { goto bogus; }
			bp=grdinfo_get_data(g->cmap,0,0,0);
			for(i=0;i<256;i++)
			{
				bp[3]=255;
				bp[0]=i;
				bp[1]=i;
				bp[2]=i;
				bp+=4;
			}
		}
		else
		{
			grd_info_free(g->cmap);
		}
	}

	return g;
bogus:
	return 0;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// creat an image and return, pass in w==0 for just a structure allocation
// returns 0 on error
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
struct grd * grd_create( s32 fmt , s32 w, s32 h, s32 d )
{
struct grd *g=0;

	g=(struct grd *)calloc(sizeof(struct grd),1);
	
	if(g)
	{
		if(!grd_realloc(g,fmt , w, h, d)){ goto bogus; }
	}

	return g;
bogus:
	grd_free(g);
	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// free an image
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void grd_free( struct grd *g )
{
	if(g)
	{
		grd_info_free(g->cmap);
		grd_info_free(g->bmap);
		if(g->data)
		{
			free(g->data);
		}
		free(g);
	}
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// given a 16 byte file header, work out what the file format probably is
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int grd_fileheader_to_format( const unsigned char data[16] )
{
	if( data[1]=='P' && data[2]=='N' && data[3]=='G' )
	{
		return GRD_FMT_HINT_PNG;
	}
	else
	if( data[0]=='G' && data[1]=='I' && data[2]=='F' )
	{
		return GRD_FMT_HINT_GIF;
	}
	else
	if( data[6]=='J' && data[7]=='F' && data[8]=='I' && data[9]=='F' )
	{
		return GRD_FMT_HINT_JPG;
	}
	
	return 0; // unknown
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// load an image and fill out a bmp information structure if a pointer is provided
// returns 0 on error
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
struct grd * grd_load_file( const char *filename , int fmt )
{
FILE *fp;
unsigned char data[16];
struct grd *g=0;

	g=(struct grd *)calloc(sizeof(struct grd),1);
	
	if(fmt==0) // read some bytes and pick a format
	{
		fp=fopen(filename,"rb");
		if(fp)
		{
			fread(data,1,16,fp);
			fclose(fp);
			fmt=grd_fileheader_to_format(data);
		}
	}

	if(g)
	{
		switch(fmt)
		{
			default:
			case GRD_FMT_HINT_PNG: grd_png_load_file(g,filename); break;
			case GRD_FMT_HINT_JPG: grd_jpg_load_file(g,filename); break;
			case GRD_FMT_HINT_GIF: grd_gif_load_file(g,filename); break;
		}
	}

	return g;
}
struct grd * grd_load_data( const unsigned char *data , int len,  int fmt )
{
struct grd *g=0;

	g=(struct grd *)calloc(sizeof(struct grd),1);
	
	if(fmt==0)
	{
		fmt=grd_fileheader_to_format(data);
	}
	
	if(g)
	{
		switch(fmt)
		{
			default:
			case GRD_FMT_HINT_PNG: grd_png_load_data(g,data,len); break;
			case GRD_FMT_HINT_JPG: grd_jpg_load_data(g,data,len); break;
			case GRD_FMT_HINT_GIF: grd_gif_load_data(g,data,len); break;
		}
	}

	return g;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// save an image
// opts is optional to force a type of output
// otherwise the filename extension is just used to guess
// returns 0 on error
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
struct grd * grd_save_file( struct grd *g , const char *filename , int fmt )
{
	if(g)
	{
		switch(fmt)
		{
			default:
			case GRD_FMT_HINT_PNG: grd_png_save_file(g,filename); break;
			case GRD_FMT_HINT_JPG: grd_jpg_save_file(g,filename); break;
			case GRD_FMT_HINT_GIF: grd_gif_save_file(g,filename); break;
		}
	}
	
	return g->err ? 0 : g ;
}

struct grd * grd_save_data( struct grd *g , struct grd_io_info *filedata , int fmt )
{
	if(g)
	{
		switch(fmt)
		{
			default:
			case GRD_FMT_HINT_PNG: grd_png_save(g,filedata); break;
//			case GRD_FMT_HINT_JPG: grd_jpg_save(g,filedata); break;
			case GRD_FMT_HINT_GIF: grd_gif_save(g,filedata); break;
		}
	}
	
	return g->err ? 0 : g ;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// duplicate the image and return new duplicate
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
struct grd * grd_duplicate( struct grd *g )
{
struct grd * g2=grd_create( g->bmap->fmt , g->bmap->w, g->bmap->h, g->bmap->d );

	if(g2)
	{
		if(g->bmap->data && g2->bmap->data)
		{
			int ps=grd_sizeof_pixel(g->bmap->fmt);

			memcpy(g2->bmap->data,g->bmap->data, ps * g->bmap->w * g->bmap->h * g->bmap->d );
		}
		if(g->cmap->data && g2->cmap->data)
		{

			memcpy(g2->cmap->data,g->cmap->data, 4 * 256 );
		}
	}

	return g2;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// swap bitmap data from a new grd (gb) with old grd (ga)
//
// after this you should free the old gb structure and all its associated infos
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
struct grd * grd_insert( struct grd *ga ,  struct grd *gb )
{
	ga->err=gb->err;
	
	grd_info_free(ga->cmap);
	grd_info_free(ga->bmap);
	
	grdinfo_set(ga->cmap,gb->cmap);
	grdinfo_set(ga->bmap,gb->bmap);

	grdinfo_reset(gb->cmap);
	grdinfo_reset(gb->bmap);
	
	return ga;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// convert to given format
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int grd_convert( struct grd *g , s32 fmt )
{
	struct grd *gb=grd_duplicate_convert( g , fmt );
	if( gb == 0 )
	{
		return 0; // error
	}
	if( gb != g ) // if these pointers are the same then there was no need to convert
	{
		grd_insert(g,gb); // we swap thw datas around
		grd_free(gb);
	}
	return 1;
}

// many many bit twiddles follow
void grd_convert_8888_rotate_left( struct grd *ga , struct grd *gb )
{
int x,y,z; u8 *pa,*pb;
	for(z=0;z<ga->bmap->d;z++) { for(y=0;y<ga->bmap->h;y++) {
	pa=grdinfo_get_data(ga->bmap,0,y,z); pb=grdinfo_get_data(gb->bmap,0,y,z);
	for(x=0;x<ga->bmap->w;x++) {
		pb[0]=pa[1];
		pb[1]=pa[2];
		pb[2]=pa[3];
		pb[3]=pa[0];
		pa+=4; pb+=4;
	}}}
}
void grd_convert_8888_rotate_right( struct grd *ga , struct grd *gb )
{
int x,y,z; u8 *pa,*pb;
	for(z=0;z<ga->bmap->d;z++) { for(y=0;y<ga->bmap->h;y++) {
	pa=grdinfo_get_data(ga->bmap,0,y,z); pb=grdinfo_get_data(gb->bmap,0,y,z);
	for(x=0;x<ga->bmap->w;x++) {
		pb[0]=pa[3];
		pb[1]=pa[0];
		pb[2]=pa[1];
		pb[3]=pa[2];
		pa+=4; pb+=4;
	}}}
}
void grd_convert_8888_multiply_a0( struct grd *ga , struct grd *gb )
{
int x,y,z; u8 *pa,*pb;
	for(z=0;z<ga->bmap->d;z++) { for(y=0;y<ga->bmap->h;y++) {
	pa=grdinfo_get_data(ga->bmap,0,y,z); pb=grdinfo_get_data(gb->bmap,0,y,z);
	for(x=0;x<ga->bmap->w;x++) {
		u32 a=pa[0];
		pb[0]=(u8)a;
		pb[1]=(u8)((pa[1]*a)/255);
		pb[2]=(u8)((pa[2]*a)/255);
		pb[3]=(u8)((pa[3]*a)/255);
		pa+=4; pb+=4;
	}}}
}
void grd_convert_8888_multiply_a3( struct grd *ga , struct grd *gb )
{
int x,y,z; u8 *pa,*pb;
	for(z=0;z<ga->bmap->d;z++) { for(y=0;y<ga->bmap->h;y++) {
	pa=grdinfo_get_data(ga->bmap,0,y,z); pb=grdinfo_get_data(gb->bmap,0,y,z);
	for(x=0;x<ga->bmap->w;x++) {
		u32 a=pa[3];
		pb[3]=(u8)a;
		pb[2]=(u8)((pa[2]*a)/255);
		pb[1]=(u8)((pa[1]*a)/255);
		pb[0]=(u8)((pa[0]*a)/255);
		pa+=4; pb+=4;
	}}}
}
void grd_convert_8888_divide_a0( struct grd *ga , struct grd *gb )
{
int x,y,z; u8 *pa,*pb;
	for(z=0;z<ga->bmap->d;z++) { for(y=0;y<ga->bmap->h;y++) {
	pa=grdinfo_get_data(ga->bmap,0,y,z); pb=grdinfo_get_data(gb->bmap,0,y,z);
	for(x=0;x<ga->bmap->w;x++) {
		u32 c;
		u32 a=pa[0]; if(a>0) { a=(255<<16)/a; }
		c=((pa[1]*a))>>16; pb[1]=(u8)(c>255?255:0);
		c=((pa[2]*a))>>16; pb[2]=(u8)(c>255?255:0);
		c=((pa[3]*a))>>16; pb[3]=(u8)(c>255?255:0);
		pb[0]=pa[0];
		pa+=4; pb+=4;
	}}}
}
void grd_convert_8888_divide_a3( struct grd *ga , struct grd *gb )
{
int x,y,z; u8 *pa,*pb;
	for(z=0;z<ga->bmap->d;z++) { for(y=0;y<ga->bmap->h;y++) {
	pa=grdinfo_get_data(ga->bmap,0,y,z); pb=grdinfo_get_data(gb->bmap,0,y,z);
	for(x=0;x<ga->bmap->w;x++) {
		u32 c;
		u32 a=pa[3]; if(a>0) { a=(255<<16)/a; }
		c=((pa[0]*a))>>16; pb[0]=(u8)(c>255?255:0);
		c=((pa[1]*a))>>16; pb[1]=(u8)(c>255?255:0);
		c=((pa[2]*a))>>16; pb[2]=(u8)(c>255?255:0);
		pb[3]=pa[3];
		pa+=4; pb+=4;
	}}}
}
void grd_convert_8888_8880( struct grd *ga , struct grd *gb )
{
int x,y,z; u8 *pa,*pb;
	for(z=0;z<ga->bmap->d;z++) { for(y=0;y<ga->bmap->h;y++) {
	pa=grdinfo_get_data(ga->bmap,0,y,z); pb=grdinfo_get_data(gb->bmap,0,y,z);
	for(x=0;x<ga->bmap->w;x++) {
		pb[0]=pa[0];
		pb[1]=pa[1];
		pb[2]=pa[2];
		pa+=4; pb+=3;
	}}}
}
void grd_convert_8888_0888( struct grd *ga , struct grd *gb )
{
int x,y,z; u8 *pa,*pb;
	for(z=0;z<ga->bmap->d;z++) { for(y=0;y<ga->bmap->h;y++) {
	pa=grdinfo_get_data(ga->bmap,0,y,z); pb=grdinfo_get_data(gb->bmap,0,y,z);
	for(x=0;x<ga->bmap->w;x++) {
		pb[0]=pa[1];
		pb[1]=pa[2];
		pb[2]=pa[3];
		pa+=4; pb+=3;
	}}}
}
void grd_convert_8880_8888( struct grd *ga , struct grd *gb )
{
int x,y,z; u8 *pa,*pb;
	for(z=0;z<ga->bmap->d;z++) { for(y=0;y<ga->bmap->h;y++) {
	pa=grdinfo_get_data(ga->bmap,0,y,z); pb=grdinfo_get_data(gb->bmap,0,y,z);
	for(x=0;x<ga->bmap->w;x++) {
		pb[0]=pa[0];
		pb[1]=pa[1];
		pb[2]=pa[2];
		pb[3]=255;
		pa+=3; pb+=4;
	}}}
}
void grd_convert_0888_8888( struct grd *ga , struct grd *gb )
{
int x,y,z; u8 *pa,*pb;
	for(z=0;z<ga->bmap->d;z++) { for(y=0;y<ga->bmap->h;y++) {
	pa=grdinfo_get_data(ga->bmap,0,y,z); pb=grdinfo_get_data(gb->bmap,0,y,z);
	for(x=0;x<ga->bmap->w;x++) {
		pb[0]=255;
		pb[1]=pa[0];
		pb[2]=pa[1];
		pb[3]=pa[2];
		pa+=3; pb+=4;
	}}}
}
void grd_convert_8888_5650( struct grd *ga , struct grd *gb )
{
int x,y,z; u8 *pa,*pb;
	for(z=0;z<ga->bmap->d;z++) { for(y=0;y<ga->bmap->h;y++) {
	pa=grdinfo_get_data(ga->bmap,0,y,z); pb=grdinfo_get_data(gb->bmap,0,y,z);
	for(x=0;x<ga->bmap->w;x++) {
		u32 c=*((u32*)(pa));
		*((u16*)(pb))=(u16)(
				((c>>16)&0xf800) |
				((c>>13)&0x07e0) |
				((c>>11)&0x001f)
			);
		pa+=4; pb+=2;
	}}}
}
void grd_convert_8888_0565( struct grd *ga , struct grd *gb )
{
int x,y,z; u8 *pa,*pb;
	for(z=0;z<ga->bmap->d;z++) { for(y=0;y<ga->bmap->h;y++) {
	pa=grdinfo_get_data(ga->bmap,0,y,z); pb=grdinfo_get_data(gb->bmap,0,y,z);
	for(x=0;x<ga->bmap->w;x++) {
		u32 c=*((u32*)(pa));
		*((u16*)(pb))=(u16)(
				((c>>8)&0xf800) |
				((c>>5)&0x07e0) |
				((c>>3)&0x001f)
			);
		pa+=4; pb+=2;
	}}}
}
void grd_convert_0565_8888( struct grd *ga , struct grd *gb )
{
int x,y,z; u8 *pa,*pb;
	for(z=0;z<ga->bmap->d;z++) { for(y=0;y<ga->bmap->h;y++) {
	pa=grdinfo_get_data(ga->bmap,0,y,z); pb=grdinfo_get_data(gb->bmap,0,y,z);
	for(x=0;x<ga->bmap->w;x++) {
		u32 c=(u32)*((u16*)(pa));
		*((u32*)(pb))=(u32)( 0xff000000 |
				((c<<8)&0x00f80000) | ((c<<3)&0x00070000) |
				((c<<5)&0x0000fc00) | ((c>>1)&0x00000300) |
				((c<<3)&0x000000f8) | ((c>>2)&0x00000007)
			);
		pa+=2; pb+=4;
	}}}
}
void grd_convert_5650_8888( struct grd *ga , struct grd *gb )
{
int x,y,z; u8 *pa,*pb;
	for(z=0;z<ga->bmap->d;z++) { for(y=0;y<ga->bmap->h;y++) {
	pa=grdinfo_get_data(ga->bmap,0,y,z); pb=grdinfo_get_data(gb->bmap,0,y,z);
	for(x=0;x<ga->bmap->w;x++) {
		u32 c=(u32)*((u16*)(pa));
		*((u32*)(pb))=(u32)( 0x000000ff |
				((c<<16)&0xf8000000) | ((c<<11)&0x07000000) |
				((c<<13)&0x00fc0000) | ((c<< 6)&0x00030000) |
				((c<<11)&0x0000f800) | ((c<< 5)&0x00000700)
			);
		pa+=2; pb+=4;
	}}}
}
void grd_convert_8888_5551( struct grd *ga , struct grd *gb )
{
int x,y,z; u8 *pa,*pb;
	for(z=0;z<ga->bmap->d;z++) { for(y=0;y<ga->bmap->h;y++) {
	pa=grdinfo_get_data(ga->bmap,0,y,z); pb=grdinfo_get_data(gb->bmap,0,y,z);
	for(x=0;x<ga->bmap->w;x++) {
		u32 c=*((u32*)(pa));
		*((u16*)(pb))=(u16)(
				((c<< 8)&0xf800) |
				((c>> 5)&0x07c0) |
				((c>>18)&0x003e) |
				((c>>31)&0x0001)
			);
		pa+=4; pb+=2;
	}}}
}
void grd_convert_5551_8888( struct grd *ga , struct grd *gb )
{
int x,y,z; u8 *pa,*pb;
	for(z=0;z<ga->bmap->d;z++) { for(y=0;y<ga->bmap->h;y++) {
	pa=grdinfo_get_data(ga->bmap,0,y,z); pb=grdinfo_get_data(gb->bmap,0,y,z);
	for(x=0;x<ga->bmap->w;x++) {
		u32 c=(u32)*((u16*)(pa));
		*((u32*)(pb))=(u32)( ( (c&0x0001) ? 0xff000000 : 0 ) |
				((c>> 8)&0x000000f8) | ((c>>13)&0x00000007) |
				((c<< 5)&0x0000f800) | ((c    )&0x00000700) |
				((c<<18)&0x00f80000) | ((c<<13)&0x00070000)
			);
		pa+=2; pb+=4;
	}}}
}
void grd_convert_8888_4444( struct grd *ga , struct grd *gb )
{
int x,y,z; u8 *pa,*pb;
	for(z=0;z<ga->bmap->d;z++) { for(y=0;y<ga->bmap->h;y++) {
	pa=grdinfo_get_data(ga->bmap,0,y,z); pb=grdinfo_get_data(gb->bmap,0,y,z);
	for(x=0;x<ga->bmap->w;x++) {
		u32 c=*((u32*)(pa));
		*((u16*)(pb))=(u16)(
				((c>>16)&0xf000) |
				((c>>12)&0x0f00) |
				((c>> 8)&0x00f0) |
				((c>> 4)&0x000f)
			);
		pa+=4; pb+=2;
	}}}
}
void grd_convert_4444_8888( struct grd *ga , struct grd *gb )
{
int x,y,z; u8 *pa,*pb;
	for(z=0;z<ga->bmap->d;z++) { for(y=0;y<ga->bmap->h;y++) {
	pa=grdinfo_get_data(ga->bmap,0,y,z); pb=grdinfo_get_data(gb->bmap,0,y,z);
	for(x=0;x<ga->bmap->w;x++) {
		u32 c=(u32)*((u16*)(pa));
		*((u32*)(pb))=(u32)(
				((c<<16)&0xf0000000) | ((c<<12)&0x0f000000) |
				((c<<12)&0x00f00000) | ((c<< 8)&0x000f0000) |
				((c<< 8)&0x0000f000) | ((c<< 4)&0x00000f00) |
				((c<< 4)&0x000000f0) | ((c    )&0x0000000f) 
			);
		pa+=2; pb+=4;
	}}}
}
void grd_convert_indexed_8888( struct grd *ga , struct grd *gb )
{
int x,y,z; u8 *pa,*pb;
	for(z=0;z<ga->bmap->d;z++) { for(y=0;y<ga->bmap->h;y++) {
	pa=grdinfo_get_data(ga->bmap,0,y,z); pb=grdinfo_get_data(gb->bmap,0,y,z);
	for(x=0;x<ga->bmap->w;x++) {
		*((u32*)pb)=*((u32*)grdinfo_get_data(ga->cmap,*pa,0,0));
		pa+=1; pb+=4;
	}}}
}

void grd_convert_grey_8888( struct grd *ga , struct grd *gb )
{
int x,y,z; u8 *pa,*pb;
	for(z=0;z<ga->bmap->d;z++) { for(y=0;y<ga->bmap->h;y++) {
	pa=grdinfo_get_data(ga->bmap,0,y,z); pb=grdinfo_get_data(gb->bmap,0,y,z);
	for(x=0;x<ga->bmap->w;x++) {
		u32 b=(u32)*pa;
		*((u32*)pb)=0xff000000 | (b<<16) | (b<<8) | (b) ;
		pa+=1; pb+=4;
	}}}
}
void grd_convert_8888_grey( struct grd *ga , struct grd *gb )
{
int x,y,z; u8 *pa,*pb;
	for(z=0;z<ga->bmap->d;z++) { for(y=0;y<ga->bmap->h;y++) {
	pa=grdinfo_get_data(ga->bmap,0,y,z); pb=grdinfo_get_data(gb->bmap,0,y,z);
	for(x=0;x<ga->bmap->w;x++) {
		u32 b=*((u32*)pa);
		*pb=(u8)( ( (((b)&0xff)*54) + (((b>>8)&0xff)*182) + (((b>>16)&0xff)*20) )>>8 );
		pa+=4; pb+=1;
	}}}
}

void grd_convert_0008_8888( struct grd *ga , struct grd *gb )
{
int x,y,z; u8 *pa,*pb;
	for(z=0;z<ga->bmap->d;z++) { for(y=0;y<ga->bmap->h;y++) {
	pa=grdinfo_get_data(ga->bmap,0,y,z); pb=grdinfo_get_data(gb->bmap,0,y,z);
	for(x=0;x<ga->bmap->w;x++) {
		u32 b=(u32)*pa;
		*((u32*)pb)=0x00ffffff | (b<<24) ;
		pa+=1; pb+=4;
	}}}
}
void grd_convert_8888_0008( struct grd *ga , struct grd *gb )
{
int x,y,z; u8 *pa,*pb;
	for(z=0;z<ga->bmap->d;z++) { for(y=0;y<ga->bmap->h;y++) {
	pa=grdinfo_get_data(ga->bmap,0,y,z); pb=grdinfo_get_data(gb->bmap,0,y,z);
	for(x=0;x<ga->bmap->w;x++) {
		u32 b=*((u32*)pa);
		*pb=(u8)( ((b>>24)&0xff) );
		pa+=4; pb+=1;
	}}}
}


struct grd * grd_duplicate_convert( struct grd *ga , s32 fmt )
{
struct grd *gb=0;
int x,y,z;
u8 *pa;
u8 *pb;
u8 *pc;

	ga->err=0;

	if(ga->bmap->fmt==fmt) // nothing to change
	{
		return ga;
	}
	
	switch( ga->bmap->fmt )
	{
		case GRD_FMT_U8_ARGB :
			switch(fmt)
			{
				case GRD_FMT_U8_RGBA :
					gb=grd_create(fmt,ga->bmap->w,ga->bmap->h,ga->bmap->d); if(!gb) { return 0; }
					grd_convert_8888_rotate_left(ga,gb);
				break;
			}
		break;
		
		case GRD_FMT_U8_ARGB_PREMULT :
			switch(fmt)
			{
				case GRD_FMT_U8_RGBA_PREMULT :
					gb=grd_create(fmt,ga->bmap->w,ga->bmap->h,ga->bmap->d); if(!gb) { return 0; }
					grd_convert_8888_rotate_left(ga,gb);
				break;
			}
		break;

		case GRD_FMT_U8_RGBA :
			switch(fmt)
			{
				case GRD_FMT_U8_RGBA_PREMULT :
					gb=grd_create(fmt,ga->bmap->w,ga->bmap->h,ga->bmap->d); if(!gb) { return 0; }
					grd_convert_8888_multiply_a3(ga,gb);
				break;
				
				case GRD_FMT_U8_ARGB :
					gb=grd_create(fmt,ga->bmap->w,ga->bmap->h,ga->bmap->d); if(!gb) { return 0; }
					grd_convert_8888_rotate_right(ga,gb);
				break;

				case GRD_FMT_U8_RGB :
					gb=grd_create(fmt,ga->bmap->w,ga->bmap->h,ga->bmap->d); if(!gb) { return 0; }
					grd_convert_8888_8880(ga,gb);
				break;

				case GRD_FMT_U16_RGBA_5551 :
					gb=grd_create(fmt,ga->bmap->w,ga->bmap->h,ga->bmap->d); if(!gb) { return 0; }
					grd_convert_8888_5551(ga,gb);
				break;

				case GRD_FMT_U16_RGBA_4444 :
					gb=grd_create(fmt,ga->bmap->w,ga->bmap->h,ga->bmap->d); if(!gb) { return 0; }
					grd_convert_8888_4444(ga,gb);
				break;

				case GRD_FMT_U16_RGBA_5650 :
					gb=grd_create(fmt,ga->bmap->w,ga->bmap->h,ga->bmap->d); if(!gb) { return 0; }
					grd_convert_8888_5650(ga,gb);
				break;

				case GRD_FMT_U8_LUMINANCE:
					gb=grd_create(fmt,ga->bmap->w,ga->bmap->h,ga->bmap->d); if(!gb) { return 0; }
					grd_convert_8888_grey(ga,gb);
				break;
				
				case GRD_FMT_U8_ALPHA:
					gb=grd_create(fmt,ga->bmap->w,ga->bmap->h,ga->bmap->d); if(!gb) { return 0; }
					grd_convert_8888_0008(ga,gb);
				break;

				case GRD_FMT_U8_INDEXED:
					gb=grd_duplicate_quant(ga,256);
				break;
			}
		break;

		case GRD_FMT_U8_RGBA_PREMULT :
			switch(fmt)
			{
				case GRD_FMT_U8_RGB : // premult flag just drops off
					gb=grd_create(fmt,ga->bmap->w,ga->bmap->h,ga->bmap->d); if(!gb) { return 0; }
					grd_convert_8888_8880(ga,gb);
				break;

				case GRD_FMT_U8_RGBA :
					gb=grd_create(fmt,ga->bmap->w,ga->bmap->h,ga->bmap->d); if(!gb) { return 0; }
					grd_convert_8888_divide_a3(ga,gb);
				break;

				case GRD_FMT_U8_ARGB_PREMULT :
					gb=grd_create(fmt,ga->bmap->w,ga->bmap->h,ga->bmap->d); if(!gb) { return 0; }
					grd_convert_8888_rotate_right(ga,gb);
				break;

				case GRD_FMT_U16_RGBA_5551_PREMULT :
					gb=grd_create(fmt,ga->bmap->w,ga->bmap->h,ga->bmap->d); if(!gb) { return 0; }
					grd_convert_8888_5551(ga,gb);
				break;

				case GRD_FMT_U16_RGBA_4444_PREMULT :
					gb=grd_create(fmt,ga->bmap->w,ga->bmap->h,ga->bmap->d); if(!gb) { return 0; }
					grd_convert_8888_4444(ga,gb);
				break;

				case GRD_FMT_U16_RGBA_5650_PREMULT :
					gb=grd_create(fmt,ga->bmap->w,ga->bmap->h,ga->bmap->d); if(!gb) { return 0; }
					grd_convert_8888_5650(ga,gb);
				break;

				case GRD_FMT_U8_INDEXED_PREMULT:
					gb=grd_duplicate_quant(ga,256);
				break;
			}
		break;
		
		case GRD_FMT_U8_RGB :
			switch(fmt)
			{
				case GRD_FMT_U8_RGBA :
					gb=grd_create(fmt,ga->bmap->w,ga->bmap->h,ga->bmap->d); if(!gb) { return 0; }
					grd_convert_8880_8888(ga,gb);
				break;
			}
		break;

		case GRD_FMT_U16_RGBA_5551 :
			switch(fmt)
			{
				case GRD_FMT_U8_RGBA :
					gb=grd_create(fmt,ga->bmap->w,ga->bmap->h,ga->bmap->d); if(!gb) { return 0; }
					grd_convert_5551_8888(ga,gb);
				break;
			}
		break;

		case GRD_FMT_U16_RGBA_5551_PREMULT :
			switch(fmt)
			{
				case GRD_FMT_U8_RGBA_PREMULT :
					gb=grd_create(fmt,ga->bmap->w,ga->bmap->h,ga->bmap->d); if(!gb) { return 0; }
					grd_convert_5551_8888(ga,gb);
				break;
			}
		break;
		 
		case GRD_FMT_U16_RGBA_4444 :
			switch(fmt)
			{
				case GRD_FMT_U8_RGBA :
					gb=grd_create(fmt,ga->bmap->w,ga->bmap->h,ga->bmap->d); if(!gb) { return 0; }
					grd_convert_4444_8888(ga,gb);
				break;
			}
		break;

		case GRD_FMT_U16_RGBA_4444_PREMULT :
			switch(fmt)
			{
				case GRD_FMT_U8_RGBA_PREMULT :
					gb=grd_create(fmt,ga->bmap->w,ga->bmap->h,ga->bmap->d); if(!gb) { return 0; }
					grd_convert_4444_8888(ga,gb);
				break;
			}
		break;
		 
		case GRD_FMT_U16_RGBA_5650 :
			switch(fmt)
			{
				case GRD_FMT_U8_RGBA :
					gb=grd_create(fmt,ga->bmap->w,ga->bmap->h,ga->bmap->d); if(!gb) { return 0; }
					grd_convert_5650_8888(ga,gb);
				break;
			}
		break;

		case GRD_FMT_U16_RGBA_5650_PREMULT :
			switch(fmt)
			{
				case GRD_FMT_U8_RGBA_PREMULT :
					gb=grd_create(fmt,ga->bmap->w,ga->bmap->h,ga->bmap->d); if(!gb) { return 0; }
					grd_convert_5650_8888(ga,gb);
				break;
			}
		break;

		case GRD_FMT_U8_LUMINANCE:
			switch(fmt)
			{
				case GRD_FMT_U8_RGBA :
					gb=grd_create(fmt,ga->bmap->w,ga->bmap->h,ga->bmap->d); if(!gb) { return 0; }
					grd_convert_grey_8888(ga,gb);
				break;
			}
		break;
		case GRD_FMT_U8_ALPHA:
			switch(fmt)
			{
				case GRD_FMT_U8_RGBA :
					gb=grd_create(fmt,ga->bmap->w,ga->bmap->h,ga->bmap->d); if(!gb) { return 0; }
					grd_convert_0008_8888(ga,gb);
				break;
			}
		break;
		case GRD_FMT_U8_INDEXED:
			switch(fmt)
			{
				case GRD_FMT_U8_RGBA :
					gb=grd_create(fmt,ga->bmap->w,ga->bmap->h,ga->bmap->d); if(!gb) { return 0; }
					grd_convert_indexed_8888(ga,gb);
				break;
			}
		break;

		case GRD_FMT_U8_INDEXED_PREMULT:
			switch(fmt)
			{
				case GRD_FMT_U8_RGBA_PREMULT :
					gb=grd_create(fmt,ga->bmap->w,ga->bmap->h,ga->bmap->d); if(!gb) { return 0; }
					grd_convert_indexed_8888(ga,gb);
				break;
			}
		break;
		 
	}

// if nothing above matched then try a multi step convert, first to RGBA, then mayb add/remove premult and then to fmt
	if( (!gb) && (ga->bmap->fmt!=GRD_FMT_U8_RGBA) && (ga->bmap->fmt!=GRD_FMT_U8_RGBA_PREMULT) )
	{

		gb=grd_duplicate_convert(ga ,GRD_FMT_U8_RGBA | (ga->bmap->fmt&GRD_FMT_PREMULT) ); // this one forces a new bitmap, keep the premult fflag


		if(gb)
		{
			if( (gb->bmap->fmt&GRD_FMT_PREMULT) != (fmt&GRD_FMT_PREMULT) ) // also need to add or remove premult
			{
				if( ! grd_convert(gb , GRD_FMT_U8_RGBA | (fmt&GRD_FMT_PREMULT) ) )
				{
					grd_free(gb); // fail, so cleanup
					gb=0;
				}
			}
			if(gb)
			{
				if( ! grd_convert(gb ,fmt) )  // try to convert that new one in place,
				{
					grd_free(gb); // fail, so cleanup
					gb=0;
				}
			}
		}
	}
	
	return gb; // success or fail
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// do a 32bit quantise that includes alpha (png8 suports this) max colors of 256
// result will be an indexed image with a funky palette
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int grd_quant(struct grd *g , s32 num_colors )
{
	struct grd *gb=grd_duplicate_quant(g , num_colors );
	
	if( gb == 0 ) { return 0; }
	if( gb != g )
	{
		grd_insert(g,gb);
		grd_free(gb);
	}
	return 1;
}
struct grd * grd_duplicate_quant(struct grd *g , s32 num_colors )
{
struct grd *gb;
int i;
u8 *optr;
u32 *ptr;
u32 c;
int siz=g->bmap->w*g->bmap->h*g->bmap->d;

	gb=grd_create(GRD_FMT_U8_INDEXED | (g->bmap->fmt&GRD_FMT_PREMULT) ,g->bmap->w,g->bmap->h,g->bmap->d);
	if(!gb) { return 0; }
	
	neuquant32_initnet( g->bmap->data , siz*4 , num_colors , 1.0/*1.0/2.2*/ );
	neuquant32_learn( 1 ); // 1 is the best quality, 30 is worst
	neuquant32_inxbuild();
	neuquant32_getcolormap(gb->cmap->data);
	
	
	optr=(u8*)gb->bmap->data;
	ptr=(u32*)g->bmap->data;
	for( i=0 ; i<siz ; i++ )
	{
		c=*ptr++;
		*optr++=neuquant32_inxsearch( (c>>24)&0xff , (c)&0xff , (c>>8)&0xff , (c>>16)&0xff );
	}
	
	return gb;
}



/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// apply a contrast adjust : subtract base then apply scale and clip value.
// so 0.5 halves the range 2.0 doubles it
// and base chooses the center of the scale probably 128
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
/*
bool grd_conscale( struct grd *g , f32 base, f32 scale)
{
	return false;
}
*/



/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// sample a block of pixels from an image and return the color of this sample
// handling out of bounds problems
// this gives us a simple but slow way to scale images
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
u32 grd_sample( struct grd *ga , s32 x, s32 y, s32 z , s32 w, s32 h, s32 d)
{
struct grd_info *gi=ga->bmap;
s32 r=0;
s32 g=0;
s32 b=0;
s32 a=0;
s32 c=0;

s32 xx,yy,zz;

u32 *ptr;
u32 abgr;

u8 *p8;

// clamp the input values
	if(x<0) {x=0;}	if(x>=gi->w) {x=gi->w-1;}
	if(y<0) {y=0;}	if(y>=gi->h) {y=gi->h-1;}
	if(z<0) {z=0;}	if(z>=gi->d) {z=gi->d-1;}

	if(w<0) {w=0;}	if(x+w>gi->w) {w=gi->w-x;}
	if(h<0) {h=0;}	if(y+h>gi->h) {h=gi->h-y;}
	if(d<0) {d=0;}	if(z+d>gi->d) {d=gi->d-z;}

	if	( // any U8 32bit format should be fine
			( gi->fmt==GRD_FMT_U8_ARGB ) ||
			( gi->fmt==GRD_FMT_U8_ARGB_PREMULT ) ||
			( gi->fmt==GRD_FMT_U8_RGBA ) ||
			( gi->fmt==GRD_FMT_U8_RGBA_PREMULT )
		)
	{
		for(zz=z;zz<z+d;zz++)
		{
			for(yy=y;yy<y+h;yy++)
			{
				ptr=(u32*)grdinfo_get_data(gi,x,yy,zz);
				for(xx=x;xx<x+w;xx++)
				{
					abgr=*ptr++;
					a+=(abgr>>24)&0xff;
					b+=(abgr>>16)&0xff;
					g+=(abgr>> 8)&0xff;
					r+=(abgr    )&0xff;
					c++;
				}
			}
		}
		
		
		if(c>0)
		{
			abgr=(((a/c)&0xff)<<24)|(((b/c)&0xff)<<16)|(((g/c)&0xff)<<8)|(((r/c)&0xff));
		}
		else
		{
			abgr=0;
		}

		return abgr;
	}

	if	( // any U8 32bit format should be fine
			( gi->fmt==GRD_FMT_U8_LUMINANCE ) ||
			( gi->fmt==GRD_FMT_U8_ALPHA ) 
		)
	{
		for(zz=z;zz<z+d;zz++)
		{
			for(yy=y;yy<y+h;yy++)
			{
				p8=(u8*)grdinfo_get_data(gi,x,yy,zz);
				for(xx=x;xx<x+w;xx++)
				{
					a+=*p8++;
					c++;
				}
			}
		}
		
		
		if(c>0)
		{
			a=((a/c)&0xff);
		}
		else
		{
			a=0;
		}

		return a;
	}
	
	if	( // just a single byte pickup for indexed images ( no real way to merge )
			( gi->fmt==GRD_FMT_U8_INDEXED ) ||
			( gi->fmt==GRD_FMT_U8_INDEXED_PREMULT )
		)
	{
		p8=(u8*)grdinfo_get_data(gi,x,y,z);
		return (u32)*p8;
	}
	
	return 0;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// change the size of the image but keep the image data the same, rest of image is filled in with black
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int grd_resize( struct grd *g , s32 w, s32 h, s32 d)
{
struct grd_info *gi=g->bmap;
struct grd *gb;

s32 s;
s32 ww,hh,dd;
s32 x,y,z;
u8 *ptr;
u8 *pts;

/*
 * 	if( // any U8 32bit format should be fine
		( g->bmap->fmt!=GRD_FMT_U8_ARGB ) &&
		( g->bmap->fmt!=GRD_FMT_U8_ARGB_PREMULT ) &&
		( g->bmap->fmt!=GRD_FMT_U8_RGBA ) &&
		( g->bmap->fmt!=GRD_FMT_U8_RGBA_PREMULT )

	) { return 0; } // must be this format
*/

	if( gi->w<1 || gi->h<1 || gi->d<1 ) { return 0; }
	if( w<1 || h<1 || d<1 ) { return 0; }
	
	gb=grd_create(g->bmap->fmt,w,h,d);
	if(!gb) { return 0; }

	ww=gi->w;
	hh=gi->h;
	dd=gi->d;
	
	for(z=0;z<d;z++)
	{
		for(y=0;y<h;y++)
		{
			ptr=(u8*)grdinfo_get_data(gb->bmap,0,y,z);
			if(y<hh && z<dd && y<h && z<d) // scan line must exist both sides
			{
				pts=(u8*)grdinfo_get_data(gi,0,y,z);
				if(ww<w) { s=ww*grd_sizeof_pixel(gb->bmap->fmt); } else { s=w*grd_sizeof_pixel(gb->bmap->fmt); }
				memcpy(ptr,pts,s);
			}
//			else
//			{
//				pts=(u8*)0;
//			}
//			if(pts)
//			{
//			}
//			for(x=0;x<w;x++)
//			{
//				if(pts && x<ww) { *ptr++=*pts++; }
//				else { *ptr++=0; }
//			}
		}
	}

	if(gb->cmap->data)
	{
		memcpy(gb->cmap->data,g->cmap->data,gb->cmap->w*grd_sizeof_pixel(gb->cmap->fmt));
	}
	
	grd_insert(g,gb);
	grd_free(gb);

	return 1;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// scale image
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int grd_scale( struct grd *g , s32 w, s32 h, s32 d)
{
struct grd_info *gi=g->bmap;
struct grd *gb;

f32 fw=0;
f32 fh=0;
f32 fd=0;

s32 sw=0;
s32 sh=0;
s32 sd=0;

f32 fx,fy,fz;
s32 x,y,z;
u32 *ptr;
u8 *p8;

	if( // any U8 32bit format should be fine
		( g->bmap->fmt!=GRD_FMT_U8_LUMINANCE ) &&
		( g->bmap->fmt!=GRD_FMT_U8_ALPHA ) &&
		( g->bmap->fmt!=GRD_FMT_U8_INDEXED ) &&
		( g->bmap->fmt!=GRD_FMT_U8_INDEXED_PREMULT ) &&
		( g->bmap->fmt!=GRD_FMT_U8_ARGB ) &&
		( g->bmap->fmt!=GRD_FMT_U8_ARGB_PREMULT ) &&
		( g->bmap->fmt!=GRD_FMT_U8_RGBA ) &&
		( g->bmap->fmt!=GRD_FMT_U8_RGBA_PREMULT )

	) { return 0; } // must be one of these formats

	if( gi->w<1 || gi->h<1 || gi->d<1 ) { return 0; }
	if( w<1 || h<1 || d<1 ) { return 0; }
	
	gb=grd_create(g->bmap->fmt,w,h,d);
	if(!gb) { return 0; }

	fw=( (f32)gi->w / (f32)w );
	sw=(s32)ceilf(fw);

	fh=( (f32)gi->h / (f32)h );
	sh=(s32)ceilf(fh);

	fd=( (f32)gi->d / (f32)d );
	sd=(s32)ceilf(fd);
	
	for(z=0,fz=0.0f;z<d;z++,fz+=fd)
	{
		for(y=0,fy=0.0f;y<h;y++,fy+=fh)
		{
			if( ( g->bmap->fmt==GRD_FMT_U8_INDEXED ) ||
				( g->bmap->fmt==GRD_FMT_U8_INDEXED_PREMULT ) ||
				( g->bmap->fmt==GRD_FMT_U8_LUMINANCE ) ||
				( g->bmap->fmt==GRD_FMT_U8_ALPHA ) )
			{
				p8=(u8*)grdinfo_get_data(gb->bmap,0,y,z);
				for(x=0,fx=0.0f;x<w;x++,fx+=fw)
				{
					*p8++=(u8)grd_sample(g, fx,fy,fz, sw,sh,sd );
				}
			}
			else
			{
				ptr=(u32*)grdinfo_get_data(gb->bmap,0,y,z);
				for(x=0,fx=0.0f;x<w;x++,fx+=fw)
				{
					*ptr++=grd_sample(g, fx,fy,fz, sw,sh,sd ); // this is a terrible way to scale...
				}
			}
		}
	}
	
	grd_insert(g,gb);
	grd_free(gb);

	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// rotate the grd clockwise, write into output grd (probably different aspect)
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
/*
void grd_rotate_clockwise( struct grd *g )
{
struct grd_info *gi=g->bmap;
s32 x,y,z,i;
s32 pw;
u8 *p1;
u8 *p2;
u8 b;
	pw=grd_sizeof_pixel(gi->fmt);
	for(z=0;z<gi->d;z++)
	{
		for(y=0;y<((gi->h+1)/2);y++)
		{
			for(x=y;x<gi->w-y);x++)
			{
				p1=(u8*)grdinfo_get_data(gi,x,y,z);
				p2=(u8*)grdinfo_get_data(gi,y,x,z);
				for(i=0;i<pw;i++) { b=p1[i]; p1[i]=p2[i]; p2[i]=b; }
				p1+=pw;
				p2-=pw;
			}		
		}
	}
}
*/

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// flip left to right
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void grd_flipx( struct grd *g )
{
struct grd_info *gi=g->bmap;
s32 x,y,z,i;
s32 pw;
u8 *p1;
u8 *p2;
u8 b;
	pw=grd_sizeof_pixel(gi->fmt);
	for(z=0;z<gi->d;z++)
	{
		for(y=0;y<(gi->h);y++)
		{
			p1=(u8*)grdinfo_get_data(gi,0,y,z);
			p2=(u8*)grdinfo_get_data(gi,gi->w-1,y,z);
			for(x=0;x<(gi->w/2);x++)
			{
				for(i=0;i<pw;i++) { b=p1[i]; p1[i]=p2[i]; p2[i]=b; }
				p1+=pw;
				p2-=pw;
			}		
		}
	}
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// flip top to bottom, ogl is often upside down so this is a useful function?
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void grd_flipy( struct grd *g )
{
struct grd_info *gi=g->bmap;
s32 x,y,z;
u8 *p1;
u8 *p2;
u8 b;
	for(z=0;z<gi->d;z++)
	{
		for(y=0;y<(gi->h/2);y++) // only need half to flip
		{
			p1=(u8*)grdinfo_get_data(gi,0,y,z);
			p2=(u8*)grdinfo_get_data(gi,0,(gi->h-1)-y,z);
			for(x=0;x<gi->yscan;x++) // yscan is a full line (use abs?) this may break, fixit
			{
				b=*p1;
				*p1++=*p2;
				*p2++=b;
			}
		}
	}
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// copy gb into ga, but only copy the pointers
//
// then adjust the size and address so it points to a single layer of the image
// as described by z
//
// this new structure should not be freed as it is mearly a copy of pointers
// also you should not free the original while using this new one
// ie it is designed as tempory use for effects that should only effect a portion of another grd
//
// if gb==ga then it is not copied just adjusted
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int grd_layer( struct grd *ga , struct grd *gb , s32 z)
{
	gb->err=0;
	
	if(z<0)             { gb->err="layer out of bounds"; }
	if((z)>gb->bmap->d) { gb->err="layer out of bounds"; }

	ga->err=gb->err; // put error in both
	if(ga->err) { return 0; }

	grdinfo_set(ga->cmap,gb->cmap);
	grdinfo_set(ga->bmap,gb->bmap);
	
	ga->bmap->d=1;
	ga->bmap->data+=(ga->bmap->zscan*z);

	ga->cmap->flags|=GRD_FLAGS_BORROWED; // flag this memory as do not free
	ga->bmap->flags|=GRD_FLAGS_BORROWED; // flag this memory as do not free

	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// copy gb into ga, but only copy the pointers
//
// then adjust the size and address so it points to a smaller portion of the image
// as described by x,y : w,h
//
// we also always select layer 0
//
// this new structure should not be freed as it is mearly a copy of pointers
// also you should not free the original while using this new one
//
// ie it is designed as tempory use for effects that should only effect a portion of another grd
//
// if gb==ga then it is not copied just adjusted
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int grd_clip( struct grd *ga , struct grd *gb , s32 x, s32 y, s32 z, s32 w, s32 h, s32 d)
{
	gb->err=0;
	
	if(x<0)               { gb->err="clip x out of bounds"; }
	if((x+w)>gb->bmap->w) { gb->err="clip x out of bounds"; }
	if(y<0)               { gb->err="clip y out of bounds"; }
	if((y+h)>gb->bmap->h) { gb->err="clip y out of bounds"; }
	if(z<0)               { gb->err="clip z out of bounds"; }
	if((z+d)>gb->bmap->d) { gb->err="clip z out of bounds"; }
	
	ga->err=gb->err; // put error in both
	if(ga->err) { return 0; }
	
	grdinfo_set(ga->cmap,gb->cmap);
	grdinfo_set(ga->bmap,gb->bmap);
	
	ga->bmap->w=w;
	ga->bmap->h=h;
	ga->bmap->d=d;
	ga->bmap->data+=(ga->bmap->xscan*x)+(ga->bmap->yscan*y)+(ga->bmap->zscan*z);

	ga->cmap->flags|=GRD_FLAGS_BORROWED; // flag this memory as do not free
	ga->bmap->flags|=GRD_FLAGS_BORROWED; // flag this memory as do not free

	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// blit gb onto ga at x,y (topleft is 0,0)
//
// you can use a fake gb to choose a portion of a larger image built using grd_clip and grd_layer
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int grd_blit( struct grd *ga , struct grd *gb , s32 x, s32 y)
{
struct grd_info *ba=ga->bmap;
struct grd_info *bb=gb->bmap;

u32 *pa,*pb;
u16 *p16a,*p16b;
u32 a,b;

s32 i,j;
s32 w=bb->w;
s32 h=bb->h;

	ga->err=0;

	if(x<0)               { ga->err="blit x out of bounds"; }
	if((x+w)>ga->bmap->w) { ga->err="blit x out of bounds"; }
	if(y<0)               { ga->err="blit y out of bounds"; }
	if((y+h)>ga->bmap->h) { ga->err="blit y out of bounds"; }

	gb->err=ga->err; // put error in both
	if(ga->err) { return 0; }

	if( (ba->fmt==GRD_FMT_U8_RGBA) && (bb->fmt==GRD_FMT_U8_RGBA_PREMULT) )
	{
		u32 ialpha;
		u32 ban;
		for(i=0;i<h;i++)
		{
			pa=(u32*)grdinfo_get_data(ba,x,y+i,0);
			pb=(u32*)grdinfo_get_data(bb,0,i,0);
			for(j=0;j<w;j++)
			{
				b=*(pb++);
				ban=(b>>24)&0xff;
				switch(ban)
				{
					case 0xff:
						*(pa++)=b;
					break;
					case 0x00:
						pa++;
					break;
					default:
						a=*(pa);
						ialpha=(0x100-((b>>24)&0xff));
						*(pa++)=
						 ( ( ( ((a&0x00ff0000) * ialpha )>>8) + (b&0x00ff0000) ) & 0x00ff0000 ) |
						 ( ( ( ((a&0x0000ff00) * ialpha )>>8) + (b&0x0000ff00) ) & 0x0000ff00 ) |
						 ( ( ( ((a&0x000000ff) * ialpha )>>8) + (b&0x000000ff) ) & 0x000000ff ) |
						 ((a+b)&0xff000000); // real alpha
					break;
				}
			}
		}

	}
	else
	if( (ba->fmt==GRD_FMT_U16_RGBA_5650) && (bb->fmt==GRD_FMT_U16_RGBA_4444_PREMULT) )
	{
		u32 ialpha;
		for(i=0;i<h;i++)
		{
			p16a=(u16*)grdinfo_get_data(ba,x,y+i,0);
			p16b=(u16*)grdinfo_get_data(bb,0,i,0);
			for(j=0;j<w;j++)
			{
				b=*(p16b++);
				switch(b&0x000f)
				{
					case 0x000f:
						*(p16a++)=(b&0xf000) | ((b&0x0f00)>>1) | ((b&0x00f0)>>3) ;
					break;
					case 0x0000:
						p16a++;
					break;
					default:
						ialpha=(0x0010 - (b&0x000f) );
						a=*(p16a);
						*(p16a++)=
						 ( ( ( ( (a&0xf800) * ialpha ) >> 4 ) +  (b&0xf000)     ) &0xf800 ) |
						 ( ( ( ( (a&0x07e0) * ialpha ) >> 4 ) + ((b&0x0f00)>>1) ) &0x07e0 ) |
						 ( ( ( ( (a&0x001f) * ialpha ) >> 4 ) + ((b&0x00f0)>>3) ) &0x001f ) ;
					break;
				}
			}
		}
	}
	else
	{
		ga->err="bad blit format"; // complain rather than auto convert
		gb->err=ga->err;
		return 0;
	}
	
	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// paint gb onto ga at x,y (topleft is 0,0) using the given (dpaint) style mode
//
// you can use a fake gb to choose a portion of a larger image built using grd_clip and grd_layer
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int grd_paint( struct grd *ga , struct grd *gb , s32 x, s32 y, s32 mode, u32 trans, u32 color)
{
struct grd_info *ba=ga->bmap;
struct grd_info *bb=gb->bmap;
struct grd_info *cb=gb->cmap;

u8 *pa,*pb;
u32 a,b;

s32 i,j;
s32 w=bb->w;
s32 h=bb->h;

//printf("%d,%d\n",w,h);

	ga->err=0;

	if(x<0)               { ga->err="paint x out of bounds"; }
	if((x+w)>ga->bmap->w) { ga->err="paint x out of bounds"; }
	if(y<0)               { ga->err="paint y out of bounds"; }
	if((y+h)>ga->bmap->h) { ga->err="paint y out of bounds"; }

	gb->err=ga->err; // put error in both
	if(ga->err) { return 0; }

	if( ((ba->fmt&~GRD_FMT_PREMULT)!=GRD_FMT_U8_INDEXED) || ((bb->fmt&~GRD_FMT_PREMULT)!=GRD_FMT_U8_INDEXED) )
	{
		ga->err="bad paint format"; // complain rather than auto convert
		gb->err=ga->err;
		return 0;
	}

	for(i=0;i<h;i++)
	{
		pa=(u8*)grdinfo_get_data(ba,x,y+i,0);
		pb=(u8*)grdinfo_get_data(bb,0,i,0);
		switch(mode)
		{
			case GRD_PAINT_MODE_ALPHA:
				for(j=0;j<w;j++)
				{
					b=*(pb++);
					if(cb->data[3+b*4]>=128) // simple alpha check against rgba
					{
						*(pa++)=b;
					}
					else
					{
						pa++;
					}
				}
			break;
			case GRD_PAINT_MODE_TRANS:
				for(j=0;j<w;j++)
				{
					b=*(pb++);
					if(b!=trans)
					{
						*(pa++)=b;
					}
					else
					{
						pa++;
					}
				}
			break;
			case GRD_PAINT_MODE_COLOR:
				for(j=0;j<w;j++)
				{
					b=*(pb++);
					if(b!=trans)
					{
						*(pa++)=color;
					}
					else
					{
						pa++;
					}
				}
			break;
			case GRD_PAINT_MODE_COPY:
				for(j=0;j<w;j++)
				{
					*(pa++)=*(pb++);
				}
			break;
			
			case GRD_PAINT_MODE_XOR:
				for(j=0;j<w;j++)
				{
					*(pa)=*(pb++) ^ *(pa);
					pa++;
				}
			break;
		}
	}
	
	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// shrink the given image clip,
// just adjust the posiion and size such that only the non transparent potion are within the area
// return true if done, we may have shrunk to nothing
//
// this function is not intending to be overly fast
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int grd_shrink(struct grd *g,struct grd_area *gc )
{
struct grd_info *gi=g->bmap;
s32 x,y;
s32 w,h;
u8 *p;
u32 a;

	if( ! ( (gi->fmt==GRD_FMT_U8_RGBA) || (gi->fmt==GRD_FMT_U8_RGBA_PREMULT) ) )
	{
		g->err="bad shrink format"; // complain
		return 0;
	}
	
	if( gc->d != 1 ) // the code does not shrink depth
	{
		g->err="bad shrink depth"; // complain
		return 0;
	}




// push down
	for( y=gc->y ; y<gc->y+gc->h ; y++ )
	{
		for( x=gc->x ; x<gc->x+gc->w ; x++ )
		{
			p=grdinfo_get_data( gi, x , y , gc->z );
			a=p[3];
			if(a!=0) { break; }
		}
		if(a==0) // empty line
		{
			gc->y++;
			gc->h--;
		}
		else
		{
			break;
		}
	}
	if(gc->h<=0) { gc->x=0; gc->y=0; gc->z=0; gc->w=0; gc->h=0; gc->d=0; return 1; }

// push up
	for( y=gc->y+gc->h-1 ; y>=gc->y ; y-- )
	{
		for( x=gc->x ; x<gc->x+gc->w ; x++ )
		{
			p=grdinfo_get_data( gi, x , y , gc->z );
			a=p[3];
			if(a!=0) { break; }
		}
		if(a==0) // empty line
		{
			gc->h--;
		}
		else
		{
			break;
		}
	}
	if(gc->h<=0) { gc->x=0; gc->y=0; gc->z=0; gc->w=0; gc->h=0; gc->d=0; return 1; }

// push right
	for( x=gc->x ; x<gc->x+gc->w ; x++ )
	{
		for( y=gc->y ; y<gc->y+gc->h ; y++ )
		{
			p=grdinfo_get_data( gi, x , y , gc->z );
			a=p[3];
			if(a!=0) { break; }
		}
		if(a==0) // empty line
		{
			gc->x++;
			gc->w--;
		}
		else
		{
			break;
		}
	}
	if(gc->w<=0) { gc->x=0; gc->y=0; gc->z=0; gc->w=0; gc->h=0; gc->d=0; return 1; }

// push left
	for( x=gc->x+gc->w-1 ; x>=gc->x ; x-- )
	{
		for( y=gc->y ; y<gc->y+gc->h ; y++ )
		{
			p=grdinfo_get_data( gi, x , y , gc->z );
			a=p[3];
			if(a!=0) { break; }
		}
		if(a==0) // empty line
		{
			gc->w--;
		}
		else
		{
			break;
		}
	}
	if(gc->w<=0) { gc->x=0; gc->y=0; gc->z=0; gc->w=0; gc->h=0; gc->d=0; return 1; }


	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// set the bmap data to the given value (u32) (u16) or (u8)
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void grd_clear(struct grd *ga, u32 val )
{
int x,y,z; u8 *pa,*pb;
	for(z=0;z<ga->bmap->d;z++) { for(y=0;y<ga->bmap->h;y++) {
	pa=grdinfo_get_data(ga->bmap,0,y,z);
		switch(grd_sizeof_pixel(ga->bmap->fmt))
		{
			case 1:
				for(x=0;x<ga->bmap->w;x++) {
					*((u8*)pa)=(u8)val;
					pa+=1;
				}
			break;
			case 2:
				for(x=0;x<ga->bmap->w;x++) {
					*((u16*)pa)=(u16)val;
					pa+=2;
				}
			break;
			case 4:
				for(x=0;x<ga->bmap->w;x++) {
					*((u32*)pa)=(u32)val;
					pa+=4;
				}
			break;
		}
	}}
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// copy the bmap (and cmap) *data* from gb into ga
// size and fmt *must* match
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void grd_copy_data(struct grd *ga, struct grd *gb )
{
int x,y,z; u8 *pa,*pb;
int s;
	for(z=0;z<ga->bmap->d;z++) { for(y=0;y<ga->bmap->h;y++) {
		pa=grdinfo_get_data(ga->bmap,0,y,z);
		pb=grdinfo_get_data(gb->bmap,0,y,z);
		memcpy(pa,pb,grd_sizeof_pixel(ga->bmap->fmt)*ga->bmap->w);
	}}
	if(ga->cmap->data && ga->cmap->w)
	{
		memcpy(ga->cmap->data,gb->cmap->data,grd_sizeof_pixel(ga->cmap->fmt)*ga->cmap->w);
	}
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// copy the bmap (and cmap) *data* from gb into ga
// size and fmt *must* match
// However we only copy one Z layer, from and too,
// useful for handling layers as animation frames and moving them around
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void grd_copy_data_layer(struct grd *ga, struct grd *gb , int za , int zb)
{
int x,y,z; u8 *pa,*pb;
int s;
	for(y=0;y<ga->bmap->h;y++) {
		pa=grdinfo_get_data(ga->bmap,0,y,za);
		pb=grdinfo_get_data(gb->bmap,0,y,zb);
		memcpy(pa,pb,grd_sizeof_pixel(ga->bmap->fmt)*ga->bmap->w);
	}
	if(ga->cmap->data && ga->cmap->w)
	{
		memcpy(ga->cmap->data,gb->cmap->data,grd_sizeof_pixel(ga->cmap->fmt)*ga->cmap->w);
	}
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// reduce the number of colours in a palleted image to the given value and perform simple remapping.
// 
// if cw or ch are 0 then they will be set to the images width or height.
// reduction is then performed on character areas of that size, eg each 8x8 area is limited to num colors
// this can be used to simulate spectrum attribute "clash"
// sub is the size of sub pallete groups, eg 16 in nes mode or 8 in spectrum mode, EG bright simulation in spectrum mode
// requires all colors in a attr block to be from the bright palette or the dark palette no mixing so this forces that
// grouping
// bak if non negative can be a global background colour that is always allowed
// this allows c64 multicolor mode simulation
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int grd_attr_redux(struct grd *g, int cw, int ch, int num, int sub,int bak)
{
int x,y,z,cx,cy;
u8 *p;
int i,j,t,tsub;
int look[3][256]; // [0] counts [1] order [2] remap

int d,dd;
int best_i=0;
int best_d=0;
u32 c1,c2;

	if(cw==0) { cw=g->bmap->w; }
	if(ch==0) { ch=g->bmap->h; }

	for(z=0;z<g->bmap->d;z++) {
		for(cy=0;cy<g->bmap->h;cy+=ch) {
			for(cx=0;cx<g->bmap->w;cx+=cw) {
		
				// clear
				for(j=0;j<3;j++){
					for(i=0;i<256;i++){look[j][i]=0;}
				}

				// loop char
				for( y=0 ; (y<ch)&&(cy+y<g->bmap->h) ; y++ ){
					p=grdinfo_get_data(g->bmap,cx,cy+y,z);
					for( x=0 ; (x<cw)&&(cx+x<g->bmap->w) ; x++ ) {
						look[0][*(p++)]++; // add one for this colours count
					}
				}
	
				// fill
				for(i=0;i<256;i++) { look[1][i]=i; }
				// very simple sort
				for(i=1;i<256;i++)
				{
					t=look[1][i]; // pickup
					for(j=i;1;j--)
					{
						if	(
								(j>0) && // last case
								(t!=bak) && // sort background to bottom
								(
									( look[0][ t ] > look[0][ look[1][j-1] ] ) || // compare using lookup counts
									look[1][j-1]==bak // sort background to bottom
								)
							)
						{
							look[1][j]=look[1][j-1]; // shift down
						}
						else
						{
							look[1][j]=t; // place
							break;
						}
					}
				}
				

				// force the top colors to be in the same subgroup palette
				if((sub>0)&&(sub<256))
				{
					if(sub<num) { num=sub; } // can only pick this many colors anyhow
					tsub=look[1][0]/sub;
					for(i=1;i<256;i++)
					{
						t=look[1][i]; // pickup
						for(j=i;1;j--)
						{
							if	(
									(j>0) && // last case
									(t!=bak) && // sort background to bottom
									(
										(tsub != (look[1][j-1]/sub) ) || // sort bad subgroup colors to bottom
										look[1][j-1]==bak // sort background to bottom
									)
								)
							{
								look[1][j]=look[1][j-1]; // shift down
							}
							else
							{
								look[1][j]=t; // place
								break;
							}
						}
					}
				}

				// build remap array
				for(i=0;i<256;i++)
				{
					if( (i<num) || (i==bak) )// use as is
					{
						best_i=look[1][i];
					}
					else
					{
						c1=*((u32*)grdinfo_get_data(g->cmap,look[1][i],0,0));
						best_i=look[1][0];
						best_d=0x7fffffff;
						if(bak>=0) // start by selecting bak
						{
							c2=*((u32*)grdinfo_get_data(g->cmap,bak,0,0));
							dd=0;
							d=((int) (c1&0x000000ff)     )-((int) (c2&0x000000ff)     ); if(d<0){d=-d;} dd+=d;
							d=((int)((c1&0x0000ff00)>>8) )-((int)((c2&0x0000ff00)>>8) ); if(d<0){d=-d;} dd+=d;
							d=((int)((c1&0x00ff0000)>>16))-((int)((c2&0x00ff0000)>>16)); if(d<0){d=-d;} dd+=d*2;
							d=((int)((c1&0xff000000)>>24))-((int)((c2&0xff000000)>>24)); if(d<0){d=-d;} dd+=d;
							best_i=bak;
							best_d=dd;
						}
						for(j=0;j<num;j++)
						{
							c2=*((u32*)grdinfo_get_data(g->cmap,look[1][j],0,0));
							dd=0;
							d=((int) (c1&0x000000ff)     )-((int) (c2&0x000000ff)     ); if(d<0){d=-d;} dd+=d;
							d=((int)((c1&0x0000ff00)>>8) )-((int)((c2&0x0000ff00)>>8) ); if(d<0){d=-d;} dd+=d;
							d=((int)((c1&0x00ff0000)>>16))-((int)((c2&0x00ff0000)>>16)); if(d<0){d=-d;} dd+=d*2;
							d=((int)((c1&0xff000000)>>24))-((int)((c2&0xff000000)>>24)); if(d<0){d=-d;} dd+=d;
							if(dd<best_d)
							{
								best_i=look[1][j];
								best_d=dd;
							}
						}
					}
					look[2][ look[1][i] ]=best_i;
				}

				// remap
				for( y=0 ; (y<ch)&&(cy+y<g->bmap->h) ; y++ ){
					p=grdinfo_get_data(g->bmap,cx,cy+y,z);
					for( x=0 ; (x<cw)&&(cx+x<g->bmap->w) ; x++ ) {
						*p=look[2][*p];
						p++;
					}
				}

			}
		}
	}

	return 1;
}
