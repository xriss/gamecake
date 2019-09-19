/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss@XIXs.com 2013
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
#include "all.h"

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// better image resize code, static include for use in grd_scale
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_STATIC
#include "stb_image_resize.h"



void inline static grd_rgb2hsv(int r, int g, int b,
	unsigned char *h, unsigned char *s, unsigned char *v)
{
	int n , x , d , t;
 
	if(r<g){n=r;}else{n=g;}if(b<n){n=b;} // n==min of r,g,b
	if(r>g){x=r;}else{x=g;}if(b>x){x=b;} // x==max of r,g,b
	d=x-n;

	*v=x;

	if(x==0) { *s=0; *h=0; return; } // no color
	
	*s=(unsigned char)(d*255/x);
	
	if(d==0)	{ d=1; } // fudge
	
	if(r==x)	{ t=(      ((g-b)*10923/d))/256; }	else
	if(g==x)	{ t=(21846+((b-r)*10923/d))/256; }	else
				{ t=(43691+((r-g)*10923/d))/256; }
	if(t<0) { t+=256; } // handle wrap 
	*h=(unsigned char)t;
}

void inline static grd_hsv2rgb(int h, int s, int v,
	unsigned char *r, unsigned char *g, unsigned char *b)
{
	int i , f , p , q , t ;
	
	if(s==0) { *r=v; *g=v; *b=v; return; } // grey
	i=(256*h)/10923;
	f=(256*h)%10923;
	
	p=v*(255-s)/255;
	q=v*(255-s*f/10923)/255;
	t=v*(255-s*(10923-f)/10923)/255;
	
	switch(i)
	{
		case  0: *r=v; *g=t; *b=p; return;
		case  1: *r=q; *g=v; *b=p; return;
		case  2: *r=p; *g=v; *b=t; return;
		case  3: *r=p; *g=q; *b=v; return;
		case  4: *r=t; *g=p; *b=v; return;
		default: *r=v; *g=p; *b=q; return;
	}
 }



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
		case GRD_FMT_U8_LUMINANCE_ALPHA:
		case GRD_FMT_U8_LUMINANCE_ALPHA_PREMULT:
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
	if( data[0]==0xFF && data[1]==0xD8 )
	{
		return GRD_FMT_HINT_JPG;
	}
//	else
//	if( data[0]==0x42 && data[1]==0x4D )
//	{
//		return GRD_FMT_HINT_BMP;
//	}
	
	return 0; // unknown
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// load an image and fill out a bmp information structure if a pointer is provided
// returns 0 on error
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
struct grd * grd_load_file( const char *filename , int fmt , u32 *tags)
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
			int fn=fread(data,1,16,fp);
			fclose(fp);
			fmt=grd_fileheader_to_format(data);
		}
	}

	if(g)
	{
		switch(fmt)
		{
			default:
			case GRD_FMT_HINT_PNG: grd_png_load_file(g,filename,tags); break;
			case GRD_FMT_HINT_JPG: grd_jpg_load_file(g,filename,tags); break;
			case GRD_FMT_HINT_GIF: grd_gif_load_file(g,filename,tags); break;
		}
	}

	return g;
}
struct grd * grd_load_data( const unsigned char *data , int len,  int fmt , u32 *tags)
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
			case GRD_FMT_HINT_PNG: grd_png_load_data(g,data,len,tags); break;
			case GRD_FMT_HINT_JPG: grd_jpg_load_data(g,data,len,tags); break;
			case GRD_FMT_HINT_GIF: grd_gif_load_data(g,data,len,tags); break;
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
struct grd * grd_save_file( struct grd *g , const char *filename , int fmt , u32 *tags)
{
	if(g)
	{
		switch(fmt)
		{
			default:
			case GRD_FMT_HINT_PNG: grd_png_save_file(g,filename,tags); break;
			case GRD_FMT_HINT_JPG: grd_jpg_save_file(g,filename,tags); break;
			case GRD_FMT_HINT_GIF: grd_gif_save_file(g,filename,tags); break;
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
			case GRD_FMT_HINT_JPG: grd_jpg_save(g,filedata); break;
			case GRD_FMT_HINT_GIF: grd_gif_save(g,filedata); break;
		}
	}
	
	return g->err ? 0 : g ;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// duplicate the image and return new duplicate
// now works ok with grd_clip structures which have strange spans
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
struct grd * grd_duplicate( struct grd *g )
{
int x,y,z,lx,ly,lz,ps;

struct grd * g2=grd_create( g->bmap->fmt , g->bmap->w, g->bmap->h, g->bmap->d );

	if(g2)
	{
		if(g->bmap->data && g2->bmap->data)
		{
			ps=grd_sizeof_pixel(g->bmap->fmt); // size of pixel	
			lx=g->bmap->w; // cache size
			ly=g->bmap->h;
			lz=g->bmap->d;

			for( z=0 ; z<lz  ; z++ ) // deal with strange spans
			{
				for( y=0 ; y<ly ; y++ ) // deal with strange spans
				{
					memcpy( grdinfo_get_data(g2->bmap,0,y,z) , grdinfo_get_data(g->bmap,0,y,z) , lx*ps );
				}
			}
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
		c=((pa[1]*a))>>16; pb[1]=(u8)(c>255?255:c);
		c=((pa[2]*a))>>16; pb[2]=(u8)(c>255?255:c);
		c=((pa[3]*a))>>16; pb[3]=(u8)(c>255?255:c);
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
		c=((pa[0]*a))>>16; pb[0]=(u8)(c>255?255:c);
		c=((pa[1]*a))>>16; pb[1]=(u8)(c>255?255:c);
		c=((pa[2]*a))>>16; pb[2]=(u8)(c>255?255:c);
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

void grd_convert_8008_8888( struct grd *ga , struct grd *gb )
{
int x,y,z; u8 *pa,*pb;
	for(z=0;z<ga->bmap->d;z++) { for(y=0;y<ga->bmap->h;y++) {
	pa=grdinfo_get_data(ga->bmap,0,y,z); pb=grdinfo_get_data(gb->bmap,0,y,z);
	for(x=0;x<ga->bmap->w;x++) {
		u32 b=(u32)pa[0];
		u32 a=(u32)pa[1];
		*((u32*)pb)=(a<<24) | (b<<16) | (b<<8) | (b) ;
		pa+=2; pb+=4;
	}}}
}
void grd_convert_8888_8008( struct grd *ga , struct grd *gb )
{
int x,y,z; u8 *pa,*pb;
	for(z=0;z<ga->bmap->d;z++) { for(y=0;y<ga->bmap->h;y++) {
	pa=grdinfo_get_data(ga->bmap,0,y,z); pb=grdinfo_get_data(gb->bmap,0,y,z);
	for(x=0;x<ga->bmap->w;x++) {
		u32 b=*((u32*)pa);
		pb[0]=(u8)( ( (((b)&0xff)*54) + (((b>>8)&0xff)*182) + (((b>>16)&0xff)*20) )>>8 );
		pb[1]=(u8)( ((b>>24)&0xff) );
		pa+=4; pb+=2;
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
		return grd_duplicate(ga);
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

				case GRD_FMT_U8_LUMINANCE_ALPHA:
					gb=grd_create(fmt,ga->bmap->w,ga->bmap->h,ga->bmap->d); if(!gb) { return 0; }
					grd_convert_8888_8008(ga,gb);
				break;

				case GRD_FMT_U8_INDEXED:
					gb=grd_duplicate_quant(ga,256,4);
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

				case GRD_FMT_U8_LUMINANCE_ALPHA_PREMULT:
					gb=grd_create(fmt,ga->bmap->w,ga->bmap->h,ga->bmap->d); if(!gb) { return 0; }
					grd_convert_8888_8008(ga,gb);
				break;

				case GRD_FMT_U8_INDEXED_PREMULT:
					gb=grd_duplicate_quant(ga,256,4);
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

		case GRD_FMT_U8_LUMINANCE_ALPHA:
			switch(fmt)
			{
				case GRD_FMT_U8_RGBA :
					gb=grd_create(fmt,ga->bmap->w,ga->bmap->h,ga->bmap->d); if(!gb) { return 0; }
					grd_convert_8008_8888(ga,gb);
				break;
			}
		break;

		case GRD_FMT_U8_LUMINANCE_ALPHA_PREMULT:
			switch(fmt)
			{
				case GRD_FMT_U8_RGBA_PREMULT :
					gb=grd_create(fmt,ga->bmap->w,ga->bmap->h,ga->bmap->d); if(!gb) { return 0; }
					grd_convert_8008_8888(ga,gb);
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
// New swankyquant code.
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
#define SWANKYQUANT_C
#define SWANKYQUANT_STATIC
#include "swankyquant.h"

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// do a 32bit quantise that includes alpha (png8 suports this) max colors of 256
// result will be an indexed image with a funky palette
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int grd_quant(struct grd *g , s32 num_colors , s32 dither )
{
	struct grd *gb=grd_duplicate_quant(g , num_colors , dither );
	
	if( gb == 0 ) { return 0; }
	if( gb != g )
	{
		grd_insert(g,gb);
		grd_free(gb);
	}
	return 1;
}
struct grd * grd_duplicate_quant(struct grd *g , s32 num_colors , s32 dither )
{
struct grd *gb;
struct grd *gc;
int i;
u8 *optr;
u32 *ptr;
u32 c;
int siz=g->bmap->w*g->bmap->h*g->bmap->d;
int w,h;

// need to force input format here...

	gb=grd_create(GRD_FMT_U8_INDEXED | (g->bmap->fmt&GRD_FMT_PREMULT) ,g->bmap->w,g->bmap->h,g->bmap->d);
	if(!gb) { return 0; }

	if(num_colors>256) { num_colors=256; } // sanity
	if(num_colors<2  ) { num_colors=2; }
	
	w=g->bmap->w;
	h=g->bmap->h;
	while( w*h > 256*256 ) { w=w/2; h=h/2; } // maximum number of pixels we want to process (speed hack)


	if( (w!=g->bmap->w) || (h!=g->bmap->h) || g->bmap->d!=1 ) // scale down first so we can deal with less pixels
	{
//printf("%dx%d -> %dx%d\n",g->bmap->w,g->bmap->h,w,h);

		gc=grd_create(g->bmap->fmt,w,h,1);
		if(!gc) { grd_free(gb); return 0; } // failure to allocate

		// scale down
		stbir_resize_uint8(	grdinfo_get_data(g->bmap,0,0,0),	g->bmap->w,		g->bmap->h,		g->bmap->yscan,
							grdinfo_get_data(gc->bmap,0,0,0),	gc->bmap->w,	gc->bmap->h,	gc->bmap->yscan,
							grd_sizeof_pixel(gc->bmap->fmt) );

		swanky_quant( gc->bmap->data, w*h, num_colors, gb->bmap->data, gb->cmap->data , 6 ); // use smaller image to build palette
		
		swanky_quant_remap( g->bmap->data, siz, num_colors, gb->bmap->data, gb->cmap->data , g->bmap->w , dither ); // remap full image

		grd_free(gc); // remember to free
	}
	else
	{

//printf("%dx%d == %dx%d\n",g->bmap->w,g->bmap->h,w,h);

		swanky_quant( g->bmap->data, siz, num_colors, gb->bmap->data, gb->cmap->data , 6 );
		swanky_quant_remap( g->bmap->data, siz, num_colors, gb->bmap->data, gb->cmap->data , g->bmap->w , dither );
	}
	
	gb->cmap->w=num_colors; // keep track of the number of used colors


	return gb;
}



/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// apply a hsv adjust to the image, image must be u8 rgba or u8 indexed
// h is -360 to +360
// s is -1 to +1
// v is -1 to +1
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int grd_adjust_hsv( struct grd *g , f32 fh, f32 fs, f32 fv)
{
int x,y,z; u8 *p;

unsigned char bh,bs,bv;
int ah,as,av;
int h,s,v;

	if( ! ( (g->bmap->fmt==GRD_FMT_U8_RGBA) || (g->bmap->fmt==GRD_FMT_U8_RGBA_PREMULT) ) )
	{
		g->err="bad adjust hsv format"; // complain
		return 0;
	}

	ah=(int)(fh*256.0f/360.0f); // squish to a byte
	as=(int)(fs*255.0f);
	av=(int)(fv*255.0f);

	for(z=0;z<g->bmap->d;z++) { for(y=0;y<g->bmap->h;y++) {
	p=grdinfo_get_data(g->bmap,0,y,z);
	for(x=0;x<g->bmap->w;x++) {
		grd_rgb2hsv(p[0],p[1],p[2],&bh,&bs,&bv);
		grd_hsv2rgb(
			(unsigned char)( (bh+ah)&0xff ),
			(unsigned char)( as<0 ? bs*(255+as)/255 : 255-(((255-bs)*(255-as))/255) ),
			(unsigned char)( av<0 ? bv*(255+av)/255 : 255-(((255-bv)*(255-av))/255) ),
			p+0,p+1,p+2);
		p+=4;
	}}}
	
	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// apply a rgb adjust to the image, image must be u8 rgba or u8 indexed
// r is -1 to +1
// g is -1 to +1
// b is -1 to +1
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int grd_adjust_rgb( struct grd *g , f32 fr, f32 fg, f32 fb)
{
int x,y,z; u8 *p;

int ar,ag,ab;

	if( ! ( (g->bmap->fmt==GRD_FMT_U8_RGBA) || (g->bmap->fmt==GRD_FMT_U8_RGBA_PREMULT) ) )
	{
		g->err="bad adjust rgb format"; // complain
		return 0;
	}

	ar=(int)(fr*256.0f); // squish to byte range so we can use integer math
	ag=(int)(fg*255.0f);
	ab=(int)(fb*255.0f);
	
	ar=ar<-255 ? -255 : ar; ar=ar>255 ? 255 : ar; // clamp
	ag=ag<-255 ? -255 : ag; ag=ag>255 ? 255 : ag;
	ab=ab<-255 ? -255 : ab; ab=ab>255 ? 255 : ab;

	for(z=0;z<g->bmap->d;z++) { for(y=0;y<g->bmap->h;y++) {
	p=grdinfo_get_data(g->bmap,0,y,z);
	for(x=0;x<g->bmap->w;x++) {
		p[0]=(unsigned char)( ar<0 ? (int)p[0]*(255+ar)/255 : 255-(((255-(int)p[0])*(255-ar))/255) );
		p[1]=(unsigned char)( ag<0 ? (int)p[1]*(255+ag)/255 : 255-(((255-(int)p[1])*(255-ag))/255) );
		p[2]=(unsigned char)( ab<0 ? (int)p[2]*(255+ab)/255 : 255-(((255-(int)p[2])*(255-ab))/255) );
		p+=4;
	}}}
	
	return 1;
}


/*#grd_adjust_contrast

perform a contrast adjust

con is a contrast scale where 0 is no change and +1 
gives full contrast while -1 removes all contrast.

*/
int grd_adjust_contrast( struct grd *g , int sub, f32 con)
{
int x,y,z; u8 *p;
int i;
int pos;
int c;
int s;

	pos=255-sub; // the other half of the range

	if( ! ( (g->bmap->fmt==GRD_FMT_U8_RGBA) || (g->bmap->fmt==GRD_FMT_U8_RGBA_PREMULT) ) )
	{
		g->err="bad adjust contrast format"; // complain
		return 0;
	}

	s=(int)(1024.0f*( (259.0f * (con*255.0f + 255.0f)) / (255.0f * (259.0f - con*255.0f)) ));
	
	for(z=0;z<g->bmap->d;z++) { for(y=0;y<g->bmap->h;y++) {
	p=grdinfo_get_data(g->bmap,0,y,z);
	for(x=0;x<g->bmap->w;x++,p+=4) {
		for(i=0;i<=3;i++) // RGB
		{
			c=((int)p[i]); // convert to int
			c=sub + (((c-sub)*s)>>10) ;
			if     ( c<  0 ) { p[i]=0;     } // write out with clamp
			else if( c>255 ) { p[i]=255;   }
			else             { p[i]=(u8)c; }
		}
	}}}
	
	return 1;
}


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
// a height map (greyscale) to normal (rgb) conversion
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int grd_sobelnormal(struct grd *g )
{
	struct grd *gb=grd_duplicate_sobelnormal(g );
	
	if( gb == 0 ) { return 0; }
	grd_insert(g,gb);
	grd_free(gb);
	return 1;
}
struct grd * grd_duplicate_sobelnormal(struct grd *g )
{
struct grd *r;
int i;
u8 *optr;
u32 *ptr;
u32 c;
s32 w,h,d;
s32 x,y,z;
f32 nx,ny,nz,nd;

	if( g->bmap->fmt!=GRD_FMT_U8_LUMINANCE) { return 0; }
	if( g->bmap->w<1 || g->bmap->h<1 || g->bmap->d<1 ) { return 0; }

	r=grd_create(GRD_FMT_U8_RGB ,g->bmap->w,g->bmap->h,g->bmap->d);
	if(!r) { return 0; }

	w=g->bmap->w;
	h=g->bmap->h;
	d=g->bmap->d;

	for(z=0;z<d;z++)
	{
		for(y=0;y<h;y++)
		{
			optr=(u8*)grdinfo_get_data(r->bmap,0,y,z);
			for(x=0;x<w;x++)
			{
				// sample gradient
				nx =	grd_sample(g,(x  +1)%w,(y  +1)%h,z,1,1,1)  +
						grd_sample(g,(x  +1)%w, y     ,z,1,1,1)*2+
						grd_sample(g,(x  +1)%w,(y+h-1)%h,z,1,1,1)  ;

				nx-=	grd_sample(g,(x+w-1)%w,(y  +1)%h,z,1,1,1)  +
						grd_sample(g,(x+w-1)%w, y     ,z,1,1,1)*2+
						grd_sample(g,(x+w-1)%w,(y+h-1)%h,z,1,1,1)  ;

				ny =	grd_sample(g,(x  +1)%w,(y  +1)%h,z,1,1,1)  +
						grd_sample(g, x       ,(y  +1)%h,z,1,1,1)*2+
						grd_sample(g,(x+w-1)%w,(y  +1)%h,z,1,1,1)  ;

				ny-=	grd_sample(g,(x  +1)%w,(y+h-1)%h,z,1,1,1)  +
						grd_sample(g, x       ,(y+h-1)%h,z,1,1,1)*2+
						grd_sample(g,(x+w-1)%w,(y+h-1)%h,z,1,1,1)  ;
				
				// fix range to within +1 and -1
				nx/=255.0f*4.0f;
				ny/=255.0f*4.0f;
				nx=-nx;
				
				// add the outward z but try not to overwhelm the x and y
				nz=1.0f - sqrtf(nx*nx+ny*ny);
				if(nz<0.0f) { nz=0.0f; }
				
				// normalize? as we may still be a little bit off
				nd=sqrtf(nx*nx+ny*ny+nz*nz);
				if(nd<=0.0f) { nd=1.0f; }
				nx/=nd;
				ny/=nd;
				nz/=nd;
				
				// output rgb (should be safe to skip the clamping)
				*optr++=(u8)(((nx+1.0f)*0.5f)*255.0f); // r
				*optr++=(u8)(((ny+1.0f)*0.5f)*255.0f); // g
				*optr++=(u8)(((nz+1.0f)*0.5f)*255.0f); // b

			}
		}
	}
	
	return r;
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
int suc;

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
	

	// need our own code for index images which just picks the closest pixel
	if( ( g->bmap->fmt==GRD_FMT_U8_INDEXED ) ||
		( g->bmap->fmt==GRD_FMT_U8_INDEXED_PREMULT ) )
	{
		memcpy(gb->cmap->data,g->cmap->data, 4 * 256 ); // copy palette colors

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
				p8=(u8*)grdinfo_get_data(gb->bmap,0,y,z);
				for(x=0,fx=0.0f;x<w;x++,fx+=fw)
				{
					*p8++=(u8)grd_sample(g, fx,fy,fz, sw,sh,sd );
				}
			}
		}

	}
	else // use stb_image_resize code
	{
		fd=( (f32)gi->d / (f32)d );
		sd=(s32)ceilf(fd);
		
		for(z=0,fz=0.0f;z<d;z++,fz+=fd) // simple z scale, just pick the closest z
		{
			if(!
				stbir_resize_uint8(	grdinfo_get_data(gi,0,0,(s32)fz),	gi->w,			gi->h,			gi->yscan,
									grdinfo_get_data(gb->bmap,0,0,z),	gb->bmap->w,	gb->bmap->h,	gb->bmap->yscan,
									grd_sizeof_pixel(gi->fmt) )
			) { return 0; } // TODO:  should pass in the premult alpha flags
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
// Perform an exclusive or of two images into a third, these must all be the same size and format.
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int grd_xor(struct grd *gd,struct grd *ga)
{
	if( ! ( (gd->bmap->fmt==ga->bmap->fmt) ) )
	{
		gd->err="bad format";
		return 0;
	}
	if( ! ( (gd->bmap->w==ga->bmap->w) ) )
	{
		gd->err="bad width";
		return 0;
	}
	if( ! ( (gd->bmap->h==ga->bmap->h) ) )
	{
		gd->err="bad height";
		return 0;
	}
	if( ! ( (gd->bmap->d==ga->bmap->d) ) )
	{
		gd->err="bad depth";
		return 0;
	}
	if( ! ( (gd->cmap->w==ga->cmap->w) ) )
	{
		gd->err="bad cmap";
		return 0;
	}

s32 x,y,z;
s32 w,h,d;
u8  *p1d,*p1a;
u16 *p2d,*p2a;
u32 *p4d,*p4a;

	w=gd->bmap->w;
	h=gd->bmap->h;
	d=gd->bmap->d;
	
	if( grd_sizeof_pixel(gd->bmap->fmt)==4 )
	{
		for( z=0 ; z<d ; z++ )
		{
			for( y=0 ; y<h ; y++ )
			{
				p4d=(u32*)grdinfo_get_data( gd->bmap, 0 , y , z );
				p4a=(u32*)grdinfo_get_data( ga->bmap, 0 , y , z );
				for( x=0 ; x<w ; x++ )
				{
					(*p4d++) ^= (*p4a++);
				}
			}
		}
	}
	else
	if( grd_sizeof_pixel(gd->bmap->fmt)==3 )
	{
		for( z=0 ; z<d ; z++ )
		{
			for( y=0 ; y<h ; y++ )
			{
				p1d=grdinfo_get_data( gd->bmap, 0 , y , z );
				p1a=grdinfo_get_data( ga->bmap, 0 , y , z );
				for( x=0 ; x<w ; x++ )
				{
					(*p1d++) ^= (*p1a++);
					(*p1d++) ^= (*p1a++);
					(*p1d++) ^= (*p1a++);
				}
			}
		}
	}
	else
	if( grd_sizeof_pixel(gd->bmap->fmt)==2 )
	{
		for( z=0 ; z<d ; z++ )
		{
			for( y=0 ; y<h ; y++ )
			{
				p2d=(u16*)grdinfo_get_data( gd->bmap, 0 , y , z );
				p2a=(u16*)grdinfo_get_data( ga->bmap, 0 , y , z );
				for( x=0 ; x<w ; x++ )
				{
					(*p2d++) ^= (*p2a++);
				}
			}
		}
	}
	else
	if( grd_sizeof_pixel(gd->bmap->fmt)==1 )
	{
		for( z=0 ; z<d ; z++ )
		{
			for( y=0 ; y<h ; y++ )
			{
				p1d=grdinfo_get_data( gd->bmap, 0 , y , z );
				p1a=grdinfo_get_data( ga->bmap, 0 , y , z );
				for( x=0 ; x<w ; x++ )
				{
					(*p1d++) ^= (*p1a++);
				}
			}
		}
	}
	if(gd->cmap->data && gd->cmap->w) // and xor palette data?
	{
		p4d=(u32*)grdinfo_get_data( gd->cmap, 0 , 0 , 0 );
		p4a=(u32*)grdinfo_get_data( ga->cmap, 0 , 0 , 0 );
		for( x=0 ; x<gd->cmap->w ; x++ )
		{
			(*p4d++) ^= (*p4a++);
		}
	}
	return 1;
}
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// shrink the given image clip,
// just adjust the position and size such that only the non transparent or index 0 portion is within the area
// return true if done, we may have shrunk to nothing
//
// this function is not intending to be overly fast
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int grd_shrink(struct grd *g,struct grd_area *gc )
{
struct grd_info *gi=g->bmap;
s32 x,y,z;
s32 w,h,d;
u8 *p;
u8 a;
int mode=grd_sizeof_pixel(gi->fmt);
int i;

	if( ! ( ((gi->fmt&~GRD_FMT_PREMULT)==GRD_FMT_U8_RGBA) || ((gi->fmt&~GRD_FMT_PREMULT)==GRD_FMT_U8_INDEXED) ) )
	{
		g->err="bad shrink format"; // complain
		return 0;
	}
	
/*
	if( gc->d != 1 ) // the code does not shrink depth
	{
		g->err="bad shrink depth"; // complain
		return 0;
	}
*/

// push in
	for( z=gc->z ; z<gc->z+gc->d ; z++ )
	{
		a=0;
		for( y=gc->y ; y<gc->y+gc->h ; y++ )
		{
			if(a!=0) { break; }
			p=grdinfo_get_data( gi, gc->x , y , z );
			for( x=gc->x ; x<gc->x+gc->w ; x++ )
			{
				if(a!=0) { break; }
				for(i=0;i<mode;i++)
				{
					a=p[i]; if(a!=0) { break; }
				}
				p+=mode;
			}
		}
		if(a==0) // empty frame
		{
			gc->z++;
			gc->d--;
		}
		else
		{
			break;
		}
	}
	if(gc->d<=0) { gc->x=0; gc->y=0; gc->z=0; gc->w=0; gc->h=0; gc->d=0; return 1; }

// push out
	for( z=gc->z+gc->d-1 ; z>=gc->z ; z-- )
	{
		a=0;
		for( y=gc->y ; y<gc->y+gc->h ; y++ )
		{
			if(a!=0) { break; }
			p=grdinfo_get_data( gi, gc->x , y , z );
			for( x=gc->x ; x<gc->x+gc->w ; x++ )
			{
				if(a!=0) { break; }
				for(i=0;i<mode;i++)
				{
					a=p[i]; if(a!=0) { break; }
				}
				p+=mode;
			}
		}
		if(a==0) // empty frame
		{
			gc->d--;
		}
		else
		{
			break;
		}
	}
	if(gc->d<=0) { gc->x=0; gc->y=0; gc->z=0; gc->w=0; gc->h=0; gc->d=0; return 1; }

// push down
	for( y=gc->y ; y<gc->y+gc->h ; y++ )
	{
		a=0;
		for( z=gc->z ; z<gc->z+gc->d ; z++ )
		{
			if(a!=0) { break; }
			p=grdinfo_get_data( gi, gc->x , y , z );
			for( x=gc->x ; x<gc->x+gc->w ; x++ )
			{
				if(a!=0) { break; }
				for(i=0;i<mode;i++)
				{
					a=p[i]; if(a!=0) { break; }
				}
				p+=mode;
			}
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
		a=0;
		for( z=gc->z ; z<gc->z+gc->d ; z++ )
		{
			if(a!=0) { break; }
			p=grdinfo_get_data( gi, gc->x , y , z );
			for( x=gc->x ; x<gc->x+gc->w ; x++ )
			{
				if(a!=0) { break; }
				for(i=0;i<mode;i++)
				{
					a=p[i]; if(a!=0) { break; }
				}
				p+=mode;
			}
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
		a=0;
		for( z=gc->z ; z<gc->z+gc->d ; z++ )
		{
			if(a!=0) { break; }
			p=grdinfo_get_data( gi, x , gc->y , z );
			for( y=gc->y ; y<gc->y+gc->h ; y++ )
			{
				if(a!=0) { break; }
				for(i=0;i<mode;i++)
				{
					a=p[i]; if(a!=0) { break; }
				}
				p+=gi->yscan;
			}
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
		a=0;
		for( z=gc->z ; z<gc->z+gc->d ; z++ )
		{
			if(a!=0) { break; }
			p=grdinfo_get_data( gi, x , gc->y , z );
			for( y=gc->y ; y<gc->y+gc->h ; y++ )
			{
				if(a!=0) { break; }
				for(i=0;i<mode;i++)
				{
					a=p[i]; if(a!=0) { break; }
				}
				p+=gi->yscan;
			}
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
			case 3:
				for(x=0;x<ga->bmap->w;x++) {
					*((u8*)pa++)=(u8)((val    )&0xff); // assume little endian
					*((u8*)pa++)=(u8)((val>>8 )&0xff);
					*((u8*)pa++)=(u8)((val>>16)&0xff);
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


/*+-----------------------------------------------------------------------------------------------------------------+*/

static inline int grd_remap_color_distance(int ar,int ag,int ab,int aa,int br,int bg,int bb,int ba)
{
	return (int)( (ar-br)*(ar-br) + (ag-bg)*(ag-bg) + (ab-bb)*(ab-bb) + (aa-ba)*(aa-ba) );
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// remap the pixels in ga into gb, using the palette that is *already* there
// ga must be rgba
// gb must be indexed and of the same size as ga
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int grd_remap(struct grd *ga, struct grd *gb, int colors, int dither)
{
int x,y,z;
u8 *pa,*pb;

unsigned char *palette;
unsigned char *pp;	// pointer to input palette

int i,j;
int step;
int best1_idx;
int best2_idx;
int best_dither;
int best_distance;
int best1_distance;
int best2_distance;
int distance;

int cr,cg,cb,ca;
int c1r,c1g,c1b,c1a;
int c2r,c2g,c2b,c2a;

int x8,y8;

const int pattern[64]={
	22,38,26,42,23,39,27,43,
	54, 6,58,10,55, 7,59,11,
	30,46,18,34,31,47,19,35,
	62,14,50, 2,63,15,51, 3,
	24,40,28,44,21,37,25,41,
	56, 8,60,12,53, 5,57, 9,
	32,48,20,36,29,45,17,33,
	64,16,52, 4,61,13,49, 1,
};

	if( ! ( (ga->bmap->fmt==GRD_FMT_U8_RGBA) || (ga->bmap->fmt==GRD_FMT_U8_RGBA_PREMULT) ) )
	{
		ga->err="bad remap format"; // complain
		return 0;
	}
	if( ! ( (gb->bmap->fmt==GRD_FMT_U8_INDEXED) || (gb->bmap->fmt==GRD_FMT_U8_INDEXED_PREMULT) ) )
	{
		gb->err="bad remap format"; // complain
		return 0;
	}

	step=64; // default to no dither
	switch(dither)
	{
		case 1: step=32; break;
		case 2: step=16; break;
		case 3: step=8;  break;
		case 4: step=4;  break;
		case 5: step=2;  break;
		case 6: step=1;  break;
	}

	palette=gb->cmap->data;
	if( colors < 2 ) { colors=gb->cmap->w; } // read palette size 
	else { gb->cmap->w=colors; } // write palette size
	
	for(z=0;z<ga->bmap->d;z++) {
		for(y=0;y<ga->bmap->h;y++) {
			y8=y%8;
			pa=grdinfo_get_data(ga->bmap,0,y,z);
			pb=grdinfo_get_data(gb->bmap,0,y,z);

			for(x=0;x<ga->bmap->w;x++,pa+=4) {
				x8=x%8;
				best1_idx=0;
				best1_distance=0x7ffffff;
				best2_idx=0;
				best2_distance=0x7ffffff;
				for( i=0 , pp=palette ; i<colors ; i++ , pp+=4 ) // search for the two best colors
				{
					distance=grd_remap_color_distance(pp[0],pp[1],pp[2],pp[3],pa[0],pa[1],pa[2],pa[3]);
					if(distance<best1_distance)
					{
						best2_distance=best1_distance;
						best2_idx=best1_idx;
						best1_distance=distance; // push the previous best to the 2nd best
						best1_idx=i;
					}
					else
					if(distance<best2_distance) // check for second best
					{
						best2_distance=distance;
						best2_idx=i;
					}
				}
				
				
				if(step==64) // no dither
				{
					*pb++=best1_idx; // write out
				}
				else
				{
					if(best2_idx < best1_idx) { i=best1_idx; best1_idx=best2_idx; best2_idx=i; } // maintain order
					
					c1r=palette[ best1_idx*4 + 0 ];
					c1g=palette[ best1_idx*4 + 1 ];
					c1b=palette[ best1_idx*4 + 2 ];
					c1a=palette[ best1_idx*4 + 3 ];

					c2r=palette[ best2_idx*4 + 0 ];
					c2g=palette[ best2_idx*4 + 1 ];
					c2b=palette[ best2_idx*4 + 2 ];
					c2a=palette[ best2_idx*4 + 3 ];

					best_dither=0;
					best_distance=0x7ffffff;
					for( i=0 ; i<=64 ; i+=step ) // check each dither option
					{
						j=64-i;
						
						cr = ( 32 + c1r*i + c2r*j ) / 64 ;
						cg = ( 32 + c1g*i + c2g*j ) / 64 ;
						cb = ( 32 + c1b*i + c2b*j ) / 64 ;
						ca = ( 32 + c1a*i + c2a*j ) / 64 ;
						
						distance=grd_remap_color_distance(cr,cg,cb,ca,pa[0],pa[1],pa[2],pa[3]);
						if(distance<best_distance)
						{
							best_distance=distance;
							best_dither=i;
						}
					}
					
					if( pattern[ x8 + y8*8 ] <= best_dither )
					{
						*pb++=best1_idx; // write out
					}
					else
					{
						*pb++=best2_idx; // write out
					}
					
				}		
			}
		}
	}
	
	return 1;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// tags are a dumb and extendable stream of extra data.
//
// u32 size of chunk (will be roundup to nearest 4 bytes)
// u32 id of chunk
// ... any data in this chunk but be careful with pointer sizes, 8bytes should be safe, probably.
//
// this is null terminated so ends at the first 0 size.
// 
// find the first tag of the given ID and return a pointer to it
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
u32* grd_tags_find(u32 *tags,u32 id)
{
	if(!tags){ return 0;}
	
	u32 *td=tags;
	while( td[0] != 0 ) // null terminated
	{
		if(td[1]==id)
		{
			return td;
		}
		td=td+((td[0]+3)>>2); // next tag round size up to 4 bytes
	}
	
	return 0;
}

/*------------------------------------------------------------------------------

Sort the colors in a cmap, will need to be an indexed image for this to 
work and we will then remap any image data associated with it.

*/
int grd_sort_cmap( struct grd *ga )
{
int x,y,z; u8 *p;

int i,b,bi,d;

u8 avail[256]; // available indexes
u8 order[256]; // the new palette order
u8 remap[256]; // remap table

u8 cc[256*4]; // temp cmap store

int avail_max;
int order_max;

int idx,dist;
int best_idx,best_dist,last_idx;

int colors;

u8 *cmap;

int thinking;

	if( ! ( (ga->bmap->fmt==GRD_FMT_U8_INDEXED) || (ga->bmap->fmt==GRD_FMT_U8_INDEXED_PREMULT) ) )
	{
		ga->err="bad sort cmap format"; // complain
		return 0; // bad
	}
	colors=ga->cmap->w;
	cmap=ga->cmap->data;
	

	best_idx=0;
	best_dist=0x7fffffff;

	idx=0;thinking=colors+1;
	while(thinking)
	{
		// fill in available colors
		order_max=0;
		avail_max=colors;
		for(i=0;i<colors;i++) { avail[i]=i; }
		avail[idx]=avail[--avail_max];
		last_idx=idx;
		order[order_max++]=idx;
		dist=0;
		
		while(avail_max>0)
		{
			b=0x7fffffff;
			bi=0;
			for(i=0;i<avail_max;i++)
			{
				d=grd_remap_color_distance(
					cmap[last_idx*4+0]	,
					cmap[last_idx*4+1]	,
					cmap[last_idx*4+2]	,
					cmap[last_idx*4+3]	,
					cmap[avail[i]*4+0]	,
					cmap[avail[i]*4+1]	,
					cmap[avail[i]*4+2]	,
					cmap[avail[i]*4+3]	);
				if(d<b)
				{
					b=d;
					bi=i;
				}
			}

			dist+=b;
			last_idx=avail[bi];
			order[order_max++]=last_idx;
			avail[bi]=avail[--avail_max];
		}
		
		if(dist<best_dist)
		{
			best_dist=dist;
			best_idx=idx;
		}
		
		idx=idx+1;
		thinking=thinking-1;
		if(thinking==1) // rethink the winner on this special last pass
		{
			idx=best_idx;
		}
	}
	
// when we get to here order will be the best one we could find

	for(i=0;i<256;i++) { remap[i]=0; } // empty and then
	for(i=0;i<order_max;i++) {
		remap[ order[i] ]=i;	// build remap table
		cc[ i*4 + 0 ]=cmap[ order[i]*4 + 0 ]; // re order pallete
		cc[ i*4 + 1 ]=cmap[ order[i]*4 + 1 ];
		cc[ i*4 + 2 ]=cmap[ order[i]*4 + 2 ];
		cc[ i*4 + 3 ]=cmap[ order[i]*4 + 3 ];
	}
	for(i=0;i<order_max;i++) {
		cmap[ i*4 + 0 ]=cc[ i*4 + 0 ]; // replace colors
		cmap[ i*4 + 1 ]=cc[ i*4 + 1 ];
		cmap[ i*4 + 2 ]=cc[ i*4 + 2 ];
		cmap[ i*4 + 3 ]=cc[ i*4 + 3 ];
	}

	for(z=0;z<ga->bmap->d;z++) { for(y=0;y<ga->bmap->h;y++) {
	p=grdinfo_get_data(ga->bmap,0,y,z);
	for(x=0;x<ga->bmap->w;x++,p++) {
		p[0]=remap[ p[0] ]; // perform remap of any pixels we have
	}}}


	return 1; // OK
}


/*------------------------------------------------------------------------------

Slide image along x,y,z , wrapping at the edges so the bitmap never loses any detail.
 

*/
int grd_slide( struct grd *ga , int dx , int dy , int dz )
{
int x,y,z,lx,ly,lz;
u8 *tp,*bp;
struct grd *gt;
int ps=grd_sizeof_pixel(ga->bmap->fmt); // size of pixel
	
	lx=ga->bmap->w; // cache size
	ly=ga->bmap->h;
	lz=ga->bmap->d;

	dx=((dx%lx)+lx)%lx; // remove negativity
	dy=((dy%ly)+ly)%ly;
	dz=((dz%lz)+lz)%lz;

	gt=grd_duplicate(ga);
	if(!gt) { ga->err="out of memory"; return 0; }
	
	for( z=0 ; z<lz  ; z++ )
	{
		for( y=0 ; y<ly ; y++ )
		{
			tp=grdinfo_get_data(gt->bmap,0,y,z);
			bp=grdinfo_get_data(ga->bmap,0,(y+dy)%ly,(z+dz)%lz);
			memcpy( bp+(dx*ps) , tp                , (lx-dx)*ps );
			if(dx>0)
			{
				memcpy( bp         , tp + ((lx-dx)*ps) ,     dx *ps );
			}
		}
	}

	grd_free(gt);
	return 1; // OK
}



/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// Create an alpha mask in gb using a floodfill at a specific xy point.
// ga must be indexed (this code is for swanky paint)
// gb must be GRD_FMT_U8_ALPHA and the same size as ga
// x,y is the pixel location of the start of the flood fill (topleft is 0,0)
// threshold is how close we have to be to the starting color, 0 means exact color only.
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int grd_fillmask(struct grd *ga, struct grd *gb, int seedx, int seedy, int threshold)
{
	u8 *pa;
	u8 *pb;
	int x,y;
	int xh,yh;
	int ya,yb;
	
	int flooded;
	int color;

	int ca,cb,cl;
	
	

	if( (ga->bmap->w!=gb->bmap->w) || (ga->bmap->h!=gb->bmap->h) )
	{
		gb->err="bad fillmask size"; // complain
		return 0;
	}

	if( ! ( (ga->bmap->fmt==GRD_FMT_U8_INDEXED) || (ga->bmap->fmt==GRD_FMT_U8_INDEXED_PREMULT) ) )
	{
		ga->err="bad fillmask format"; // complain
		return 0;
	}

	if( ! ( (gb->bmap->xscan==1) ) )
	{
		gb->err="bad fillmask format"; // complain
		return 0;
	}
	
	grd_clear(gb,0x00); // clear output to 0
	

	xh=ga->bmap->w; // cache the max x,y 
	yh=ga->bmap->h;
	
	ya=ga->bmap->yscan; // cache y scan
	yb=gb->bmap->yscan;

	*( grdinfo_get_data(gb->bmap,seedx,seedy,0) )=0xff; // seed the start pixel with 0xff

	color=*( grdinfo_get_data(ga->bmap,seedx,seedy,0) ); // the starting color
	


	flooded=1;
	while(flooded) // repeat until a pass finds no more pixels
	{
		flooded=0;

		for(y=0;y<yh;y++)
		{
			pa=grdinfo_get_data(ga->bmap,0,y,0);
			pb=grdinfo_get_data(gb->bmap,0,y,0);
			cl=*pb; // last
			pa++;pb++;
			for(x=1;x<xh;x++) // left to right
			{
				ca=*pa;
				cb=*pb;
				
				if( (cl==0xff)&&(cb==0x00) ) // test for flood
				{
					if(ca==color)
					{
						flooded=1;
						cb=0xff;
						*pb=cb;
					}
				}
				
				cl=cb; // last 
				pa++;pb++;
			}

			pa=grdinfo_get_data(ga->bmap,xh-1,y,0);
			pb=grdinfo_get_data(gb->bmap,xh-1,y,0);
			cl=*pb; // last
			pa--;pb--;
			for(x=xh-2;x>=0;x--) // right to left
			{
				ca=*pa;
				cb=*pb;

				if( (cl==0xff)&&(cb==0x00) ) // test for flood
				{
					if(ca==color)
					{
						flooded=1;
						cb=0xff;
						*pb=cb;
					}
				}

				cl=cb; // last
				pa--;pb--;
			}
		}

		for(x=0;x<xh;x++)
		{
			pa=grdinfo_get_data(ga->bmap,x,0,0);
			pb=grdinfo_get_data(gb->bmap,x,0,0);
			cl=*pb; // last
			pa+=ya;pb+=yb;
			for(y=1;y<yh;y++) // top to bottom
			{
				ca=*pa;
				cb=*pb;
				
				if( (cl==0xff)&&(cb==0x00) ) // test for flood
				{
					if(ca==color)
					{
						flooded=1;
						cb=0xff;
						*pb=cb;
					}
				}
				
				cl=cb; // last 
				pa+=ya;pb+=yb;
			}
			
			pa=grdinfo_get_data(ga->bmap,x,yh-1,0);
			pb=grdinfo_get_data(gb->bmap,x,yh-1,0);
			cl=*pb; // last
			pa-=ya;pb-=yb;
			for(y=yh-2;y>=0;y--) // bottom to top
			{
				ca=*pa;
				cb=*pb;

				if( (cl==0xff)&&(cb==0x00) ) // test for flood
				{
					if(ca==color)
					{
						flooded=1;
						cb=0xff;
						*pb=cb;
					}
				}

				cl=cb; // last
				pa-=ya;pb-=yb;
			}
		}

	}

	return 1; // OK
}
