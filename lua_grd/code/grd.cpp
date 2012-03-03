/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss Daniels 2003 http://www.XIXs.com
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

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// allocate some data in a grd_info
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void * grd_info_alloc(struct grd_info *gi,  s32 fmt , s32 w, s32 h, s32 d )
{
int ps=GRD_FMT_SIZEOFPIXEL(fmt);

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
		free(gi->data);
	}
	gi->reset();
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
		if(fmt==GRD_FMT_U8_INDEXED) // do we need a palette?
		{
			if(!grd_info_alloc(g->cmap, GRD_FMT_U8_ARGB , 256, 1, 1 )) { goto bogus; }
		}
		else
		if(fmt==GRD_FMT_U8_LUMINANCE) // do we need a palette?
		{
			if(!grd_info_alloc(g->cmap, GRD_FMT_U8_ARGB , 256, 1, 1 )) { goto bogus; }
			bp=g->cmap->get_data(0,0,0);
			for(i=0;i<256;i++)
			{
				bp[0]=255;
				bp[1]=i;
				bp[2]=i;
				bp[3]=i;
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
	
	if(g && w)
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
// load an image and fill out a bmp information structure if a pointer is provided
// returns 0 on error
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
struct grd * grd_load_file( const char *filename , int fmt )
{
struct grd *g=0;

	g=(struct grd *)calloc(sizeof(struct grd),1);

	if(g)
	{
		switch(fmt)
		{
			default:
			case GRD_FMT_HINT_PNG: grd_png_load_file(g,filename); break;
			case GRD_FMT_HINT_JPG: grd_jpg_load_file(g,filename); break;
		}
	}

	return g;
}
struct grd * grd_load_data( const unsigned char *data , int len,  int fmt )
{
struct grd *g=0;

	g=(struct grd *)calloc(sizeof(struct grd),1);

	if(g)
	{
		switch(fmt)
		{
			default:
			case GRD_FMT_HINT_PNG: grd_png_load_data(g,data,len); break;
			case GRD_FMT_HINT_JPG: grd_jpg_load_data(g,data,len); break;
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
			int ps=GRD_FMT_SIZEOFPIXEL(g->bmap->fmt);

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
	
	ga->cmap->set(gb->cmap);
	ga->bmap->set(gb->bmap);

	gb->cmap->reset();
	gb->bmap->reset();
	
	return ga;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// convert to given format
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
bool grd_convert( struct grd *g , s32 fmt )
{
	struct grd *gb=grd_duplicate_convert( g , fmt );
	if( gb == 0 )
	{
		return false; // error
	}
	if( gb != g ) // if these pointers are the same then there was no need to convert
	{
		grd_insert(g,gb); // we swap thw datas around
		grd_free(gb);
	}
	return true;
}
struct grd * grd_duplicate_convert( struct grd *g , s32 fmt )
{
struct grd *gb;
int x,y,z;
u8 *pa;
u8 *pb;
u8 *pc;

	g->err=0;

	if(g->bmap->fmt==fmt) // nothing to change
	{
		return g;
	}

	if(g->bmap->fmt==GRD_FMT_U8_ARGB) // convert from ARGB
	{
		switch(fmt)
		{
			case GRD_FMT_HINT_ALPHA:
			case GRD_FMT_U8_ARGB:
				return g; // no change needed
			break;
			
			case GRD_FMT_U8_ARGB_PREMULT:
				gb=grd_create(GRD_FMT_U8_ARGB_PREMULT,g->bmap->w,g->bmap->h,g->bmap->d);
				if(!gb) { return 0; }
				for(z=0;z<g->bmap->d;z++)
				{
					for(y=0;y<g->bmap->h;y++)
					{
						pa=g->bmap->get_data(0,y,z);
						pb=gb->bmap->get_data(0,y,z);
						for(x=0;x<g->bmap->w;x++)
						{
							u32 a=pa[0];
							pb[0]=a;
							pb[1]=(u8)((pa[1]*a)>>8);
							pb[2]=(u8)((pa[2]*a)>>8);
							pb[3]=(u8)((pa[3]*a)>>8);
							pa+=4;
							pb+=4;
						}
					}
				}
				return gb;
			break;

			case GRD_FMT_U8_INDEXED:
				return grd_duplicate_quant(g,256);
			break;
			
			case GRD_FMT_HINT_ALPHA_1BIT:
			case GRD_FMT_U16_ARGB_1555:
				gb=grd_create(GRD_FMT_U16_ARGB_1555,g->bmap->w,g->bmap->h,g->bmap->d);
				if(!gb) { return false; }
				
				for(z=0;z<g->bmap->d;z++)
				{
					for(y=0;y<g->bmap->h;y++)
					{
						pa=g->bmap->get_data(0,y,z);
						pb=gb->bmap->get_data(0,y,z);
						for(x=0;x<g->bmap->w;x++)
						{
							u16 d;
							
							d=(  (pa[0]>=128) ? 0x8000 : 0x0000 ) |
								((pa[1]>>3)<<10) |
								((pa[2]>>3)<<5) |
								 (pa[3]>>3) ;
								
							*((u16 *)(pb))=d;
							
							pa+=4;
							pb+=2;							
						}
					}
				}
				
				return gb ;
			break;

			case GRD_FMT_U16_RGBA_4444:
				gb=grd_create(GRD_FMT_U16_RGBA_4444,g->bmap->w,g->bmap->h,g->bmap->d);
				if(!gb) { return false; }
				
				for(z=0;z<g->bmap->d;z++)
				{
					for(y=0;y<g->bmap->h;y++)
					{
						pa=g->bmap->get_data(0,y,z);
						pb=gb->bmap->get_data(0,y,z);
						for(x=0;x<g->bmap->w;x++)
						{
							u32 d;
							
							d=	((((u32)pa[1])<<8)&0xf000) |
								((((u32)pa[2])<<4)&0x0f00) |
								((((u32)pa[3])   )&0x00f0) |
								((((u32)pa[0])>>4)&0x000f) ;
								
							*((u16 *)(pb))=(u16)d;
							
							pa+=4;
							pb+=2;							
						}
					}
				}
				
				return gb ;
			break;

			case GRD_FMT_U16_RGBA_4444_PREMULT:
				gb=grd_create(GRD_FMT_U16_RGBA_4444_PREMULT,g->bmap->w,g->bmap->h,g->bmap->d);
				if(!gb) { return false; }
				
				for(z=0;z<g->bmap->d;z++)
				{
					for(y=0;y<g->bmap->h;y++)
					{
						pa=g->bmap->get_data(0,y,z);
						pb=gb->bmap->get_data(0,y,z);
						for(x=0;x<g->bmap->w;x++)
						{
							u32 d;
							u32 a=pa[0];
							
							d=	(((((u32)pa[1])*a)   )&0xf000) |
								(((((u32)pa[2])*a)>>4)&0x0f00) |
								(((((u32)pa[3])*a)>>8)&0x00f0) |
								((((u32)pa[0])>>4)    &0x000f) ;
								
							*((u16 *)(pb))=(u16)d;
							
							pa+=4;
							pb+=2;							
						}
					}
				}
				
				return gb ;			break;
			break;
			
			case GRD_FMT_U16_RGB_565:
				gb=grd_create(GRD_FMT_U16_RGB_565,g->bmap->w,g->bmap->h,g->bmap->d);
				if(!gb) { return false; }
				
				for(z=0;z<g->bmap->d;z++)
				{
					for(y=0;y<g->bmap->h;y++)
					{
						pa=g->bmap->get_data(0,y,z);
						pb=gb->bmap->get_data(0,y,z);
						for(x=0;x<g->bmap->w;x++)
						{
							u16 d;
							
							d=	((pa[1]>>3)<<11) |
								((pa[2]>>2)<<5) |
								 (pa[3]>>3) ;
								
							*((u16 *)(pb))=d;
							
							pa+=4;
							pb+=2;							
						}
					}
				}
				
				return gb ;
			break;

						
			case GRD_FMT_U8_RGB:
			
			default:
				return 0;
			break;	
		}
	}
	else
	if(fmt==GRD_FMT_U8_ARGB) // convert to BGRA
	{
		switch(g->bmap->fmt)
		{
			case GRD_FMT_U8_ARGB:
				return g; // no change needed
			break;
			
			case GRD_FMT_U8_LUMINANCE:
			case GRD_FMT_U8_INDEXED:
				gb=grd_create(GRD_FMT_U8_ARGB,g->bmap->w,g->bmap->h,g->bmap->d);
				if(!gb) { return false; }
				
				for(z=0;z<g->bmap->d;z++)
				{
					for(y=0;y<g->bmap->h;y++)
					{
						pa=g->bmap->get_data(0,y,z);
						pb=gb->bmap->get_data(0,y,z);
						for(x=0;x<g->bmap->w;x++)
						{
							pc=g->cmap->get_data(*pa,0,0);
							pb[0]=pc[0];
							pb[1]=pc[1];
							pb[2]=pc[2];
							pb[3]=pc[3];
							pa+=1;
							pb+=4;
						}
					}
				}
				
				return gb ;
			break;
						
			case GRD_FMT_U16_ARGB_1555:
				gb=grd_create(GRD_FMT_U8_ARGB,g->bmap->w,g->bmap->h,g->bmap->d);
				if(!gb) { return false; }
				
				for(z=0;z<g->bmap->d;z++)
				{
					for(y=0;y<g->bmap->h;y++)
					{
						pa=g->bmap->get_data(0,y,z);
						pb=gb->bmap->get_data(0,y,z);
						for(x=0;x<g->bmap->w;x++)
						{
							u16 d;
							d=*((u16*)(pa));
							pb[0]=(d>=0x8000)?255:0;
							pb[1]=(((d>>10)&0x1f)<<3) | (((d>>10)&0x1f)>>2);
							pb[2]=(((d>> 5)&0x1f)<<3) | (((d>> 5)&0x1f)>>2);
							pb[3]=(((d    )&0x1f)<<3) | (((d    )&0x1f)>>2);
							
							pa+=2;
							pb+=4;
						}
					}
				}
				
				return gb ;
			break;
			
			case GRD_FMT_U8_RGB:
			
			default:
				return 0;
			break;	
		}
	}
	else // convert source then try again
	{
		gb=grd_duplicate_convert(g ,GRD_FMT_U8_ARGB); // this one always makes a new bitmap
		if(gb==0) { return 0; }
		if( ! grd_convert(gb ,GRD_FMT_U8_ARGB) ) { grd_free(gb); return 0; }
		if( ! grd_convert(gb ,fmt) ) { grd_free(gb); return 0; }
		return gb;
	}
	
	return 0; // default fail
}



/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// do a 32bit quantise that includes alpha (png8 suports this) max colors of 256
// result will be an indexed image with a funky palette
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
bool grd_quant(struct grd *g , s32 num_colors )
{
	struct grd *gb=grd_duplicate_quant(g , num_colors );
	
	if( gb == 0 ) { return false; }
	if( gb != g )
	{
		grd_insert(g,gb);
		grd_free(gb);
	}
	return true;
}
struct grd * grd_duplicate_quant(struct grd *g , s32 num_colors )
{
struct grd *gb;
int i;
u8 *optr;
u32 *ptr;
u32 c;
int siz=g->bmap->w*g->bmap->h*g->bmap->d;

	gb=grd_create(GRD_FMT_U8_INDEXED,g->bmap->w,g->bmap->h,g->bmap->d);
	if(!gb) { return false; }
	
	neuquant32_initnet( g->bmap->data , siz*4 , num_colors , 1.0/*1.0/2.2*/ );
	neuquant32_learn( 1 ); // 1 is the best quality, 30 is worst
	neuquant32_inxbuild();
	neuquant32_getcolormap(gb->cmap->data);
	
	
	optr=(u8*)gb->bmap->data;
	ptr=(u32*)g->bmap->data;
	for( i=0 ; i<siz ; i++ )
	{
		c=*ptr++;
		*optr++=neuquant32_inxsearch( (c)&0xff , (c>>8)&0xff , (c>>16)&0xff , (c>>24)&0xff );
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
u32 bgra;

// clamp the input values
	if(x<0) {x=0;}	if(x>=gi->w) {x=gi->w-1;}
	if(y<0) {y=0;}	if(y>=gi->h) {y=gi->h-1;}
	if(z<0) {z=0;}	if(z>=gi->d) {z=gi->d-1;}

	if(w<0) {w=0;}	if(x+w>gi->w) {w=gi->w-x;}
	if(h<0) {h=0;}	if(y+h>gi->h) {h=gi->h-y;}
	if(d<0) {d=0;}	if(z+d>gi->d) {d=gi->d-z;}

	for(zz=z;zz<z+d;zz++)
	{
		for(yy=y;yy<y+h;yy++)
		{
			ptr=(u32*)gi->get_data(x,yy,zz);
			for(xx=x;xx<x+w;xx++)
			{
				bgra=*ptr++;
				a+=(bgra>>24)&0xff;
				r+=(bgra>>16)&0xff;
				g+=(bgra>> 8)&0xff;
				b+=(bgra    )&0xff;
				c++;
			}
		}
	}
	
	
	if(c>0)
	{
		bgra=(((a/c)&0xff)<<24)|(((r/c)&0xff)<<16)|(((g/c)&0xff)<<8)|(((b/c)&0xff));
	}
	else
	{
		bgra=0;
	}

	return bgra;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// scale image
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
bool grd_scale( struct grd *g , s32 w, s32 h, s32 d)
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

	if( g->bmap->fmt!=GRD_FMT_U8_ARGB ) { return false; } // must be this format

	if( gi->w<1 || gi->h<1 || gi->d<1 ) { return false; }
	if( w<1 || h<1 || d<1 ) { return false; }
	
	gb=grd_create(g->bmap->fmt,w,h,d);
	if(!gb) { return false; }

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
			ptr=(u32*)gb->bmap->get_data(0,y,z);
			for(x=0,fx=0.0f;x<w;x++,fx+=fw)
			{
				*ptr++=grd_sample(g, (s32)fx,(s32)fy,(s32)fz, sw,sh,sd );
			}
		}
	}
	
	grd_insert(g,gb);
	grd_free(gb);

	return true;
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
			p1=(u8*)gi->get_data(0,y,z);
			p2=(u8*)gi->get_data(0,(gi->h-1)-y,z);
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
bool grd_layer( struct grd *ga , struct grd *gb , s32 z)
{
	gb->err=0;
	
	if(z<0)             { gb->err="layer out of bounds"; }
	if((z)>gb->bmap->d) { gb->err="layer out of bounds"; }

	ga->err=gb->err; // put error in both
	if(ga->err) { return false; }

	if(ga!=gb)
	{
		ga->cmap->set(gb->cmap);
		ga->bmap->set(gb->bmap);
	}
	
	ga->bmap->d=1;
	ga->bmap->data+=(ga->bmap->zscan*z);

	return true;
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
bool grd_clip( struct grd *ga , struct grd *gb , s32 x, s32 y, s32 w, s32 h)
{
	gb->err=0;
	
	if(x<0)               { gb->err="clip x out of bounds"; }
	if((x+w)>gb->bmap->w) { gb->err="clip x out of bounds"; }
	if(y<0)               { gb->err="clip y out of bounds"; }
	if((y+h)>gb->bmap->h) { gb->err="clip y out of bounds"; }
	
	ga->err=gb->err; // put error in both
	if(ga->err) { return false; }
	
	if(ga!=gb)
	{
		ga->cmap->set(gb->cmap);
		ga->bmap->set(gb->bmap);
	}
	
	ga->bmap->w=w;
	ga->bmap->h=h;
	ga->bmap->d=1;
	ga->bmap->data+=(ga->bmap->xscan*x)+(ga->bmap->yscan*y);

	return true;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// blit gb onto ga at x,y (topleft is 0,0)
//
// you can use a fake gb to choose a portion of a larger image built using grd_clip and grd_layer
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
bool grd_blit( struct grd *ga , struct grd *gb , s32 x, s32 y)
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
	if(ga->err) { return false; }

	if( (ba->fmt==GRD_FMT_U8_ARGB) && (bb->fmt==GRD_FMT_U8_ARGB_PREMULT) )
	{
		u32 ialpha;
		for(i=0;i<h;i++)
		{
			pa=(u32*)ba->get_data(x,y+i,0);
			pb=(u32*)bb->get_data(0,i,0);
			for(j=0;j<w;j++)
			{
				b=*(pb++);
				switch(b&0xff)
				{
					case 0xff:
						*(pa++)=b|0xff;
					break;
					case 0x00:
						pa++;
					break;
					default:
						ialpha=(0x100 - (b&0xff) );
						a=*(pa);
						*(pa++)=
						 ( ( ( ((a>>8)&0x00ff0000) * ialpha ) + (b&0xff000000) ) & 0xff000000 ) |
						 ( ( ( ((a>>8)&0x0000ff00) * ialpha ) + (b&0x00ff0000) ) & 0x00ff0000 ) |
						 ( ( ( ((a>>8)&0x000000ff) * ialpha ) + (b&0x0000ff00) ) & 0x0000ff00 ) |
						 (0xff); // full alpha
					break;
				}
			}
		}

	}
	else
	if( (ba->fmt==GRD_FMT_U16_RGB_565) && (bb->fmt==GRD_FMT_U16_RGBA_4444_PREMULT) )
	{
		u32 ialpha;
		for(i=0;i<h;i++)
		{
			p16a=(u16*)ba->get_data(x,y+i,0);
			p16b=(u16*)bb->get_data(0,i,0);
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
		return false;
	}
	
	return true;
}


