/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss@XIXs.com 2013
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
#include "all.h"



#define CHECKIL(ret,callhook)\
		callhook;\
		if((ret=ilGetError()) != IL_NO_ERROR)\
		{\
			DBG_Error("Failed to " #callhook " : %s \n",iluErrorString(ilerror));\
			goto bogus;\
		}\



static bool grd_il_quantise255alpha(u32 image_id );



/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// creat an image and return
// returns 0 on error
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
s32 grd_create( s32 fmt , s32 w, s32 h, s32 d )
{
s32 ilid[1]={0}; // array of image ids

s32 ilerror;


	CHECKIL(ilerror, ilGenImages(NUMOF(ilid),(u32*)ilid) )

	CHECKIL(ilerror, ilBindImage(ilid[0]) )

	if( (fmt==GRD_FMT_U8_INDEXED)  )
	{
		CHECKIL(ilerror, ilTexImage(w,h,d,1,IL_COLOUR_INDEX,IL_UNSIGNED_BYTE,0) )
	}
	else
	if( (fmt==GRD_FMT_U8_LUMINANCE)  )
	{
		CHECKIL(ilerror, ilTexImage(w,h,d,1,IL_LUMINANCE,IL_UNSIGNED_BYTE,0) )
	}
	else
	{
		CHECKIL(ilerror, ilTexImage(w,h,d,4,IL_BGRA,IL_UNSIGNED_BYTE,0) )
	}

//retard check
//	CHECKIL(ilerror, ilSetInteger(IL_ORIGIN_MODE,IL_ORIGIN_UPPER_LEFT) )

	
	return ilid[0];
	
bogus:

	ilDeleteImages(NUMOF(ilid),(u32*)ilid);

	for( ilerror=ilGetError() ; ilerror!=IL_NO_ERROR ; )
	{
		DBG_Error("ILERROR %s\n",iluErrorString(ilerror));
	}

	return(0);
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// load an image and fill out a bmp information structure if a pointer is provided
// returns 0 on error
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
s32 grd_load( const char *filename , s32 fmt , grd_info *grd )
{
s32 ilid[1]={0}; // array of image ids

s32 ilerror;


	CHECKIL(ilerror, ilGenImages(NUMOF(ilid),(u32*)ilid) )

	CHECKIL(ilerror, ilBindImage(ilid[0]) )

	CHECKIL(ilerror, ilLoadImage((char*const)filename) )

	if( (fmt==GRD_FMT_U8_BGRA) )
	{
		CHECKIL(ilerror, ilConvertImage(IL_BGRA,IL_UNSIGNED_BYTE) )
	}
	else
	if( (fmt==GRD_FMT_U8_LUMINANCE)  )
	{
		CHECKIL(ilerror, ilConvertImage(IL_LUMINANCE,IL_UNSIGNED_BYTE) )
	}
	else
	{
	}
//retard check
//	CHECKIL(ilerror, ilSetInteger(IL_ORIGIN_MODE,IL_ORIGIN_UPPER_LEFT) )

	if(grd)
	{
		grd_getinfo(ilid[0],grd);
	}

	return ilid[0];

bogus:

	ilDeleteImages(NUMOF(ilid),(u32*)ilid);

	for( ilerror=ilGetError() ; ilerror!=IL_NO_ERROR ; )
	{
		DBG_Error("ILERROR %s\n",iluErrorString(ilerror));
	}

	if(grd)
	{
		grd->reset();
	}

	return(0);
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// save an image
// returns 0 on error
//
/*+-----------------------------------------------------------------------------------------------------------------+*/

bool grd_save( s32 ID , const char *filename )
{
s32 ilerror;


	CHECKIL(ilerror, ilBindImage(ID) )

	ilSetInteger( IL_PNG_ALPHA_INDEX , 0 );

	CHECKIL(ilerror, ilSaveImage((char*const)filename) )

	ilSetInteger( IL_PNG_ALPHA_INDEX , -1 );

	return true ;

bogus:

	for( ilerror=ilGetError() ; ilerror!=IL_NO_ERROR ; )
	{
		DBG_Error("ILERROR %s\n",iluErrorString(ilerror));
	}

	return false ;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// get info from a previously loaded image ( the id returned from grd_load )
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
bool grd_getinfo( s32 ID , grd_info *grd )
{
s32 ilerror;
s32 type;

	grd->reset();

	CHECKIL(ilerror, ilBindImage(ID ) );

	CHECKIL(ilerror, type=ilGetInteger(IL_IMAGE_FORMAT) )

	if(type==IL_COLOR_INDEX)
	{
		grd->fmt=GRD_FMT_U8_INDEXED;
		grd->xscan=1;
	}
	else
	if(type==IL_LUMINANCE)
	{
		grd->fmt=GRD_FMT_U8_LUMINANCE;
		grd->xscan=1;
	}
	else
	if(type==IL_BGRA)
	{
		grd->fmt=GRD_FMT_U8_BGRA;
		grd->xscan=4;
	}
	else // convert the bugger to something we do understand
	{
		CHECKIL(ilerror, ilConvertImage(IL_BGRA,IL_UNSIGNED_BYTE) )

		grd->fmt=GRD_FMT_U8_BGRA;
		grd->xscan=4;
	}

	CHECKIL(ilerror, grd->w=ilGetInteger(IL_IMAGE_WIDTH) )
	CHECKIL(ilerror, grd->h=ilGetInteger(IL_IMAGE_HEIGHT) )
	grd->d=1;

	grd->yscan=grd->w*grd->xscan;
	grd->zscan=grd->h*grd->yscan;

	CHECKIL(ilerror, grd->data=ilGetData() )

	return true;

bogus:

	grd->reset();

	return false;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// get palette info from a previously loaded image ( the id returned from grd_load )
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
bool grd_getpalinfo( s32 ID , grd_info *grd )
{
s32 ilerror;
s32 type;

	grd->reset();

	CHECKIL(ilerror, ilBindImage(ID ) );

	CHECKIL(ilerror, type=ilGetInteger(IL_IMAGE_FORMAT) )

	if(type==IL_COLOR_INDEX)
	{
		grd->fmt=GRD_FMT_U8_RGB;
		grd->xscan=1;

		CHECKIL(ilerror, grd->w=ilGetInteger(IL_PALETTE_NUM_COLS)*3 )
		grd->h=1;
		grd->d=1;

		grd->yscan=grd->w*grd->xscan;
		grd->zscan=grd->h*grd->yscan;

		CHECKIL(ilerror, grd->data=ilGetPalette() )

		return true;
	}
	else
	{
		grd->reset();
	}



bogus:

	grd->reset();

	return false;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// duplicate the image and return new id of duplicate
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
s32 grd_duplicate( s32 ID )
{
s32 ilid[1];

s32 ilerror;

	CHECKIL(ilerror, ilGenImages(NUMOF(ilid),(u32*)ilid) )

	CHECKIL(ilerror, ilBindImage(ilid[0]) );
	CHECKIL(ilerror, ilCopyImage(ID) );


	return ilid[0];

bogus:

	ilDeleteImages(NUMOF(ilid),(u32*)ilid);

	for( ilerror=ilGetError() ; ilerror!=IL_NO_ERROR ; )
	{
		DBG_Error("ILERROR %s\n",iluErrorString(ilerror));
	}

	return 0;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// convert to given format
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
bool grd_convert( s32 ID , s32 fmt )
{
s32 ilerror;


	CHECKIL(ilerror, ilBindImage(ID) );

	if(fmt==GRD_FMT_U8_BGRA)
	{
		CHECKIL(ilerror, ilConvertImage(IL_BGRA,IL_UNSIGNED_BYTE) )
	}
	else
	if(fmt==GRD_FMT_U8_INDEXED)
	{
		grd_il_quantise255alpha(ID);
	}
	else
	{
		DBG_Error("Missing code, please write me.\n");
	}


	return true;

bogus:

	for( ilerror=ilGetError() ; ilerror!=IL_NO_ERROR ; )
	{
		DBG_Error("ILERROR %s\n",iluErrorString(ilerror));
	}

	return false;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// free a previously loaded image ( the id returned from grd_load )
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void grd_free( s32 ID )
{
s32 a[1];
	a[0]=ID;
	ilDeleteImages(1,(u32*)a);
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// do quantise to 255 colors then shift up one so we can put an trans color at index 0
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static bool grd_il_quantise255alpha(u32 image_id )
{
s32 ilerror;

s32 ilid[1]={0}; // array of image ids

u32 oldimage; // tempory store
u32 newimage; // in out


// create new and dupe to it

	CHECKIL(ilerror, ilGenImages(NUMOF(ilid),(u32*)ilid) )
	ilBindImage(ilid[0]);
	ilCopyImage(image_id);


// set from -> to ( output is input...)

	newimage=image_id;
	oldimage=ilid[0];




//
// image is argb, use the alpha to set the alphed bits to black on a threshhold, these bits will later become transparent
//

	ilBindImage(newimage);

	if ((ilerror=ilGetError()) == IL_NO_ERROR) // all loaded up
	{
	s32 width;
	s32 height;
	s32 x;
	s32 y;
	u32  *data;
	u32	color;

		width=ilGetInteger(IL_IMAGE_WIDTH);
		height=ilGetInteger(IL_IMAGE_HEIGHT);

		data=(u32*)ilGetData();

		for( y=0 ; y<height ; y++ )
		{	
			for( x=0 ; x<width ; x++ )
			{
				color=*data;

				if( (color&0xff000000) < 0xf8000000 ) // min alpha before transparent
				{
					color=color&0xff000000;
				}

				*data=color;

				data++;
			}
		}
	}


// add alpha idx

	ilSetInteger(IL_QUANTIZATION_MODE,IL_WU_QUANT);
	ilSetInteger(IL_MAX_QUANT_INDEXS,255);

	ilConvertImage(IL_COLOUR_INDEX ,IL_BYTE );


	if ((ilerror=ilGetError()) == IL_NO_ERROR) // all converted up
	{
	s32 width;
	s32 height;
	s32 x;
	s32 y;
	u32 *data;
	u8  *dataidx;
	u32	color;

		width=ilGetInteger(IL_IMAGE_WIDTH);
		height=ilGetInteger(IL_IMAGE_HEIGHT);

		ilBindImage(oldimage);
		data=(u32*)ilGetData();

		ilBindImage(newimage);
		dataidx=(u8*)ilGetData();

		for( y=0 ; y<height ; y++ )
		{	
			for( x=0 ; x<width ; x++ )
			{
				color=*data;

				if( (color&0xff000000) < 0xf8000000 ) // min alpha before transparent
				{
					*dataidx=0;
				}
				else
				{
					*dataidx+=1;
				}

				data++;
				dataidx++;
			}
		}
	}
//	goto test;

// fixup pal, it will have space for 256 entries but only be using the first 255

	if ((ilerror=ilGetError()) == IL_NO_ERROR) // all converted up
	{
	s32 i;
	u8  *data;

		ilBindImage(newimage);

		data=ilGetPalette();

		for(i=255;i>0;i--)
		{
			data[i*3 + 0]=data[(i-1)*3 + 0];
			data[i*3 + 1]=data[(i-1)*3 + 1];
			data[i*3 + 2]=data[(i-1)*3 + 2];
		}

		data[0]=128;
		data[1]=128;
		data[2]=128;

// BORK: ???? Was causing buffer overflows and killing DevIL
//		ilSetInteger( IL_PALETTE_NUM_COLS , 256 );

	}

//test:
	ilBindImage(newimage);

	ilSetInteger(IL_MAX_QUANT_INDEXS,256);

	ilDeleteImages(1,(u32*)ilid);

	return true;

bogus:

	ilDeleteImages(1,(u32*)ilid);

	for( ilerror=ilGetError() ; ilerror!=IL_NO_ERROR ; )
	{
		DBG_Error("ILERROR %s\n",iluErrorString(ilerror));
	}

	return false;
}



/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// apply a contrast adjust adjust to +-128 then scale then clip, so 0.5 halves the range 2.0 doubles it
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
bool grd_conscale( s32 ID , f32 base, f32 scale)
{
grd_info grd[1];

u8 *p;
s32 i,x,y,z;
f32 f;

	grd_getinfo(ID,grd);

	if(grd->fmt!=GRD_FMT_U8_BGRA) { return false; }

	for(z=0;z<grd->d;z++)
	{
		for(y=0;y<grd->h;y++)
		{
			p=grd->get_data(0,y,z);

			for(x=0;x<grd->w;x++)
			{
				for(i=0;i<3;i++) // scale RGB not A
				{
					f=base+((((f32)p[i])-base)*scale);
					if(f<0) { f=0; }
					if(f>255) { f=255; }
					p[i]=(u8)f;
				}
				p+=grd->xscan;
			}
		}
	}


	return true;

}


static bool scaleargb( s32 fromID , s32 toID , s32 w, s32 h );

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// scale image
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
bool grd_scale( s32 ID , s32 w, s32 h, s32 d)
{
grd_info grd[1];
s32 n;

	grd_getinfo(ID,grd);
	


	if( (w<grd->w) && (h<grd->h) && (d==grd->d) && (d==1) ) // do special case better quality reduce
	{
		n=grd_duplicate(ID);
		grd_convert(n,GRD_FMT_U8_BGRA);
		scaleargb(n,ID,w,h);
		grd_free(n);
	}
	else
	{
		iluImageParameter(ILU_FILTER,ILU_BILINEAR);
		iluScale(w,h,d);
	}

	return true;

}




/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// do my image scale argb format only
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static bool scaleargb( s32 fromID , s32 toID , s32 w, s32 h )
{
s32 orig_width;
s32 orig_height;

u32 *fromdata;
u32 *todata;

f32 a,r,g,b,t;

s32 xi,yi,xo,yo;

s32 ximin,ximax;
s32 yimin,yimax;

u32 *fp;
u32 *tp;

u32 c;

	ilBindImage(fromID);

	orig_width=ilGetInteger(IL_IMAGE_WIDTH);
	orig_height=ilGetInteger(IL_IMAGE_HEIGHT);
	fromdata=(u32*)ilGetData();

	ilBindImage(toID);
	if(!ilTexImage(w,h,1,4,IL_BGRA,IL_UNSIGNED_BYTE,0))
	{
		DBG_Warn("Failed to create output image.\n");
		goto bogus;
	}
	todata=(u32*)ilGetData();

	for(yo=0;yo<h;yo++)
	{
		tp=todata+yo*w;
		for(xo=0;xo<w;xo++)
		{
			a=r=g=b=t=0;

			ximin=xo*orig_width/w;
			ximax=(xo+1)*orig_width/w;

			yimin=yo*orig_height/h;
			yimax=(yo+1)*orig_height/h;

			if(ximax>orig_width) ximax=orig_width;
			if(yimax>orig_height) yimax=orig_height;

			for( yi=yimin ; yi<yimax ; yi++ )
			{
				fp=fromdata+(yi*orig_width)+ximin;
				for( xi=ximin ; xi<ximax ; xi++ )
				{
					c=*fp;

					a+=(f32)((c>>24)&0xff);
					r+=(f32)((c>>16)&0xff);
					g+=(f32)((c>>8) &0xff);
					b+=(f32)((c>>0) &0xff);
					t+=1.0f;

				fp++;
				}
			}

			if(t>0.0f)
			{
				a=a/t;
				r=r/t;
				g=g/t;
				b=b/t;
			}

			if(a>255.0f) a=255.0f;
			if(r>255.0f) r=255.0f;
			if(g>255.0f) g=255.0f;
			if(b>255.0f) b=255.0f;

			c= (((u32)(a))<<24) | (((u32)(r))<<16) | (((u32)(g))<<8) | (((u32)(b))<<0) ;

			*tp=c;


		tp++;
		}
	}


	return true;
bogus:
	return false;
}
