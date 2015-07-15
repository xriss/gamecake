/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss@XIXs.com 2013
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
#include "all.h"



//
// GRD Image handling layer, load/save/manipulate
//
// currently sandwiched on top of openil but I plan on killing that dependency
//
//






/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// load an image and fill out a bmp information structure if a pointer is provided
// returns 0 on error
//
// this is a smarter version that will merge two .jpg file containing an alpha and an rgb into one image
//
// it is keyed off of filenames provided, if the filename ends in .a.jpg or .b.jpg then it assumed to refer to both files
// *.a.jpg is a greyscale alpha jpeg and *.b.jpg is a normal rgb color jpg , these are merged into a single
// argb image and returned
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
s32 grd_load_smart( const char *filename , s32 fmt , grd_info *grd )
{
char buff[512];
char *tail;

grd_info t_grd[2];
s32 t_bmid[2]={0};

s32 r_bmid=0;
grd_info r_grd[1];

s32 x,y,z;

cu8* aptr;
cu32* bptr;
u32* cptr;

	tail=(char*)filename+strlen(filename)-6;

	if	(
			(tail>filename) // sanity check
			&&
			(
				(strcasecmp(".a.jpg",tail)==0)
				||
				(strcasecmp(".b.jpg",tail)==0)
				||
				(strcasecmp(".a.png",tail)==0)
				||
				(strcasecmp(".b.png",tail)==0)
			)
		)
	{
		strncpy(buff,filename,512); buff[511]=0;
		tail=buff+strlen(buff)-5;

		tail[0]='a'; // the alpha image
		t_bmid[0]=grd_load(buff,GRD_FMT_U8_LUMINANCE,t_grd+0);

		if(t_bmid[0]==0) goto bogus;

		tail[0]='b'; // the main image
		t_bmid[1]=grd_load(buff,GRD_FMT_U8_BGRA,t_grd+1);

		if(t_bmid[1]==0) goto bogus;

// the two images must be the same size

		if( t_grd[0].w != t_grd[1].w ) goto bogus;
		if( t_grd[0].h != t_grd[1].h ) goto bogus;
		if( t_grd[0].d != t_grd[1].d ) goto bogus;

// allocate an output image

		r_bmid=grd_create(GRD_FMT_U8_BGRA,t_grd[0].w,t_grd[0].h,t_grd[0].d);
		grd_getinfo(r_bmid,r_grd);

// merge the alpha into the rgb of the output texture

		for(z=0;z<t_grd[0].d;z++)
		{
			for(y=0;y<t_grd[0].h;y++)
			{
				aptr=(u8*) (t_grd[0].data+y*t_grd[0].yscan+z*t_grd[0].zscan);
				bptr=(u32*)(t_grd[1].data+y*t_grd[1].yscan+z*t_grd[1].zscan);
				cptr=(u32*)(r_grd[0].data+y*r_grd[0].yscan+z*r_grd[0].zscan);

				for(x=0;x<t_grd[0].w;x++)
				{
					*cptr= ((*bptr)&0x00ffffff) + ((*aptr)<<24) ;

					aptr++;
					bptr++;
					cptr++;
				}
			}
		}

		grd_free(t_bmid[0]);
		grd_free(t_bmid[1]);


		if(grd)
		{
			grd_getinfo(r_bmid,grd);
		}
		return r_bmid;

	}
	else
	{
		r_bmid= grd_load(filename,fmt,r_grd);

// if we got an indexed format, need to convert to argb and treat 0 as transparent

		if(r_grd[0].fmt==GRD_FMT_U8_INDEXED)
		{
			t_bmid[0]=grd_duplicate(r_bmid);
			grd_convert(t_bmid[0],GRD_FMT_U8_BGRA);
			grd_getinfo(t_bmid[0],t_grd+0);

			for(z=0;z<t_grd[0].d;z++)
			{
				for(y=0;y<t_grd[0].h;y++)
				{
					aptr=(u8*) (r_grd[0].data+y*r_grd[0].yscan+z*r_grd[0].zscan);
					cptr=(u32*)(t_grd[0].data+y*t_grd[0].yscan+z*t_grd[0].zscan);

					for(x=0;x<t_grd[0].w;x++)
					{
						*cptr= ((*cptr)&0x00ffffff) + ((*aptr)?0xff000000:0x00000000) ;

						aptr++;
						cptr++;
					}
				}
			}

			grd_free(r_bmid);

			if(grd)
			{
				grd_getinfo(t_bmid[0],grd);
			}
			return t_bmid[0];
		}

		if(grd)
		{
			grd_getinfo(r_bmid,grd);
		}
		return r_bmid;
	}

bogus:

	grd_free(r_bmid);

	grd_free(t_bmid[0]);
	grd_free(t_bmid[1]);

	return 0;
}





/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// blit the area of one grd to an area of another
//
// use two grd structures to define the areas, all of grd_a is pasted into grd_b at x,y 
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
//s32 grd_blit_area( grd_info *grd , grd_info *src , s32 x, s32 y)
//{
//}



