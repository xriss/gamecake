/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss Daniels 2003 http://www.XIXs.com
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
#include "all.h"

//
// GRD Image handling layer, load/save/manipulate
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
			if(!grd_info_alloc(g->pall, GRD_FMT_U8_BGRA , 256, 1, 1 )) { goto bogus; }
		}
		else
		if(fmt==GRD_FMT_U8_LUMINANCE) // do we need a palette?
		{
			if(!grd_info_alloc(g->pall, GRD_FMT_U8_BGRA , 256, 1, 1 )) { goto bogus; }
			bp=g->pall->get_data(0,0,0);
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
			grd_info_free(g->pall);
		}
	}

	return g;
bogus:
	return 0;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// creat an image and return
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
		grd_info_free(g->pall);
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
struct grd * grd_load( const char *filename , s32 fmt , const char *opts )
{
struct grd *g=0;

	g=(struct grd *)calloc(sizeof(struct grd),1);

	if(g)
	{
		if(opts)
		{
			if(strncmp(opts,"jpg",3)==0)
			{
				grd_jpg_load_file(g,filename);
			}
			else
			{
				grd_png_load_file(g,filename);
			}
		}
		else
		{
			grd_png_load_file(g,filename);
		}
		if(!g->err)
		{
			if(!grd_convert(g,fmt))
			{
				g->err="failed to convert to requested format";
			}
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

bool grd_save( struct grd *g , const char *filename , const char *opts )
{
		if(opts)
		{
			if(strncmp(opts,"jpg",3)==0)
			{
				grd_jpg_save_file(g,filename);
			}
			else
			{
				grd_png_save_file(g,filename);
			}
		}
		else
		{
			grd_png_save_file(g,filename);
		}

	return g->err?false:true ;
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
		if(g->pall->data && g2->pall->data)
		{

			memcpy(g2->pall->data,g->pall->data, 4 * 256 );
		}
	}

	return g2;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// swap bitmap data from a new grd (gb) with old grd (ga)
// then free all the data that is now only in gb
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
struct grd * grd_insert( struct grd *ga ,  struct grd *gb )
{
	ga->err=gb->err;
	
	grd_info_free(ga->pall);
	grd_info_free(ga->bmap);
	
	ga->pall->set(gb->pall);
	ga->bmap->set(gb->bmap);

	gb->pall->reset();
	gb->bmap->reset();
	grd_free(gb);
	
	return ga;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// convert to given format
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
bool grd_convert( struct grd *g , s32 fmt )
{
struct grd *gb;
int x,y,z;
u8 *pa;
u8 *pb;
u8 *pc;

	if(g->bmap->fmt==fmt) // nothing to do
	{
		return true;
	}

	if(g->bmap->fmt==GRD_FMT_U8_BGRA) // convert from BGRA
	{
		switch(fmt)
		{
			case GRD_FMT_HINT_ALPHA:
			case GRD_FMT_U8_BGRA:
				return true; // no change needed
			break;
			
			case GRD_FMT_U8_INDEXED:
				return grd_quant(g,256);
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
							
							d=( (pa[3]>=128) ? 0x8000 : 0x0000 ) |
								(pa[0]>>3) | 
								((pa[1]>>3)<<5) |
								((pa[2]>>3)<<10) ;
								
							*((u16 *)(pb))=d;
							
							pa+=4;
							pb+=2;							
						}
					}
				}
				
				grd_insert(g,gb);
			break;
						
			case GRD_FMT_HINT_ONLY_ALPHA:
			case GRD_FMT_U8_LUMINANCE:
			
			case GRD_FMT_F32_ARGB:
			
			case GRD_FMT_F64_ARGB:
			
			case GRD_FMT_HINT_NO_ALPHA:
			case GRD_FMT_U8_RGB:
			
			default:
				return false;
			break;	
		}
	}
	else
	if(fmt==GRD_FMT_U8_BGRA) // convert to BGRA
	{
		switch(g->bmap->fmt)
		{
			case GRD_FMT_U8_BGRA:
				return true; // no change needed
			break;
			
			case GRD_FMT_U8_LUMINANCE:
			case GRD_FMT_U8_INDEXED:
				gb=grd_create(GRD_FMT_U8_BGRA,g->bmap->w,g->bmap->h,g->bmap->d);
				if(!gb) { return false; }
				
				for(z=0;z<g->bmap->d;z++)
				{
					for(y=0;y<g->bmap->h;y++)
					{
						pa=g->bmap->get_data(0,y,z);
						pb=gb->bmap->get_data(0,y,z);
						for(x=0;x<g->bmap->w;x++)
						{
							pc=g->pall->get_data(*pa,0,0);
							pb[0]=pc[0];
							pb[1]=pc[1];
							pb[2]=pc[2];
							pb[3]=pc[3];
							pa+=1;
							pb+=4;
						}
					}
				}
				
				grd_insert(g,gb);
			break;
						
			case GRD_FMT_U16_ARGB_1555:
				gb=grd_create(GRD_FMT_U8_BGRA,g->bmap->w,g->bmap->h,g->bmap->d);
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
							pb[3]=(d>=0x8000)?255:0;
							pb[2]=(((d>>10)&0x1f)<<3) | (((d>>10)&0x1f)>>2);
							pb[1]=(((d>> 5)&0x1f)<<3) | (((d>> 5)&0x1f)>>2);
							pb[0]=(((d    )&0x1f)<<3) | (((d>>  )&0x1f)>>2);
							
							pa+=2;
							pb+=4;
						}
					}
				}
				
				grd_insert(g,gb);
			break;
			
			case GRD_FMT_F32_ARGB:
			
			case GRD_FMT_F64_ARGB:
			
			case GRD_FMT_U8_RGB:
			
			default:
				return false;
			break;	
		}
	}
	else // convert source to BGRA first then try again
	{
		if(! grd_convert(g ,GRD_FMT_U8_BGRA) ) { return false; }
		return grd_convert(g ,fmt);
	}
	
	return true;
}



/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// do a 32bit quantise that includes alpha (png8 suports this) max colors of 256
// result will be an indexed image with a funky palette
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
bool grd_quant(struct grd *g , s32 num_colors )
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
	neuquant32_getcolormap(gb->pall->data);
	
	
	optr=(u8*)gb->bmap->data;
	ptr=(u32*)g->bmap->data;
	for( i=0 ; i<siz ; i++ )
	{
		c=*ptr++;
		*optr++=neuquant32_inxsearch( (c>>24)&0xff , (c>>16)&0xff , (c>>8)&0xff , (c)&0xff  );
	}
	
	grd_insert(g,gb);
	return true;
}



/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// apply a contrast adjust adjust to +-128 then scale then clip, so 0.5 halves the range 2.0 doubles it
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
bool grd_conscale( struct grd *g , f32 base, f32 scale)
{


	return false;

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


