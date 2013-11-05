/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss@XIXs.com 2013
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
#include "all.h"

#include "gif_lib.h"


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// file or memory
//
/*+-----------------------------------------------------------------------------------------------------------------+*/

static int grd_gif_read(GifFileType *gif,char *buff,int count)
{
	struct grd_loader_info *inf=(struct grd_loader_info *)(gif->UserData);
	
	if(inf == NULL)
	{
		return 0;   // error
	}

	if( ( inf->pos + 0 ) >= inf->data_len )
	{
		return 0;   // error
	}

	if( ( inf->pos + count ) > inf->data_len )
	{
		count=inf->data_len - inf->pos;
	}

	memcpy(buff,inf->data+inf->pos,count);
	inf->pos+=count;
	
	return count;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// read a gif, into this grd, allocating the correct size (animations just go in the Z depth)
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static void grd_gif_load(struct grd * g, struct grd_loader_info * inf )
{
	int x,y,z,j;
    int	i, ErrorCode;
    GifFileType *gif = (GifFileType *)NULL;
	SavedImage *img;
	GifImageDesc *dsc;
	ColorMapObject *cmp;
	
	unsigned char *pw=0;
	unsigned char *pr=0;
	
	if(inf->file_name)
	{
		if((gif = DGifOpenFileName( inf->file_name , &ErrorCode)) == NULL)
		{
			g->err=GifErrorString(ErrorCode);
			goto bogus;
		}
	}
	else
	{
		if((gif = DGifOpen( (void *)inf , (InputFunc) grd_gif_read , &ErrorCode)) == NULL)
		{
			g->err=GifErrorString(ErrorCode);
			goto bogus;
		}
	}
	
    if (DGifSlurp(gif) == GIF_ERROR)
    {
		g->err=GifErrorString(ErrorCode);
		goto bogus;
    }
    
	if(!grd_realloc(g,GRD_FMT_U8_INDEXED,gif->SWidth,gif->SHeight,gif->ImageCount))
    {
		g->err="grd realloc fail";
		goto bogus;
    }

//printf("res %d\n",gif->SColorResolution);

	cmp=gif->SColorMap;
	if(cmp)
	{
		for(j=0;j<cmp->ColorCount;j++)
		{
			g->cmap->data[j*4+0]=cmp->Colors[j].Red;
			g->cmap->data[j*4+1]=cmp->Colors[j].Green;
			g->cmap->data[j*4+2]=cmp->Colors[j].Blue;
			g->cmap->data[j*4+3]=0xff;
		}
	}
	j=gif->SBackGroundColor;
	if( (j>=0) && (j<=255) )
	{
		g->cmap->data[j*4+3]=0x00;
	}

    for (z = 0; z < gif->ImageCount; z++)
    {
		img=gif->SavedImages+z;
		dsc=&img->ImageDesc;
		
		pr=img->RasterBits;

		
//printf("%d,%d,%d,%d\n",dsc->Top,dsc->Height,dsc->Left,dsc->Width);

		for( y=dsc->Top ; y<dsc->Top+dsc->Height ; y++ )
		{
			pw=grdinfo_get_data(g->bmap,dsc->Left,y,z);
			for( x=dsc->Left ; x<dsc->Left+dsc->Width ; x++ )
			{
				*(pw++)=*(pr++);
			}
		}

	}

    
bogus:
	if(gif)
	{
		if (DGifCloseFile(gif) == GIF_ERROR)
		{
			g->err=GifErrorString(ErrorCode);
			return;
		}
	}
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// read a gif into a grd from a file
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void grd_gif_load_file(struct grd * g, const char* file_name)
{
	struct grd_loader_info inf[1];
	
	inf->file_name=file_name;
	inf->data=0;
	inf->pos=0;
	inf->data_len=0;
	
	grd_gif_load(g,inf);	
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// read a gif into a grd from data
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void grd_gif_load_data(struct grd *g, const unsigned char* data, int data_len)
{
	struct grd_loader_info inf[1];
	
	inf->file_name=0;
	inf->data=(u8*)data;
	inf->pos=0;
	inf->data_len=data_len;
	
	grd_gif_load(g,inf);
}



/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// save a gif into a file (Z layers are animation frames)
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void grd_gif_save_file(struct grd * g, const char* file_name)
{
int ErrorCode;
GifFileType *gif = (GifFileType *)NULL;
SavedImage img;
GifColorType colors[256];
int i;
unsigned char *p;

ExtensionBlock ext[3];
GraphicsControlBlock control;
unsigned char wank[3];

	ext[0].ByteCount=sizeof(GraphicsControlBlock);
	ext[0].Bytes=(GifByteType*)&control;
	ext[0].Function=GRAPHICS_EXT_FUNC_CODE;

	ext[1].ByteCount=11;
	ext[1].Bytes=(GifByteType *)"NETSCAPE2.0";
	ext[1].Function=APPLICATION_EXT_FUNC_CODE;

	ext[2].ByteCount=3;
	ext[2].Bytes=wank;
	ext[2].Function=CONTINUE_EXT_FUNC_CODE;
	
	wank[0]=0x01;
	wank[1]=0xff; // loop 4 ever, well for as long as we can
	wank[2]=0xff;


	control.TransparentColor=0; 
	control.DisposalMode=DISPOSE_BACKGROUND;
	control.UserInputFlag=0;
	control.DelayTime=8;
// speed of animation in 100ths of a second 8 is 12.5fps (80ms)
// which is nearest the classic "Shooting on twos" speed of 12fps

//unsigned char *data=0;

    if ((gif = EGifOpenFileName(file_name,0,&ErrorCode)) == NULL)
    {
		g->err=GifErrorString(ErrorCode);
//		goto bogus;
		return;
    }
    
//    data=calloc( g->bmap->w*g->bmap->h , 1 );

    gif->SWidth = g->bmap->w;
    gif->SHeight = g->bmap->h;
    
    p=grdinfo_get_data(g->cmap,0,0,0);
    for(i=0;i<256;i++)
    {
		colors[i].Red=p[0];
		colors[i].Green=p[1];
		colors[i].Blue=p[2];
		p=p+4;
	}
    
    gif->SColorResolution = 8;
    gif->SBackGroundColor = 0;
    gif->SColorMap = GifMakeMapObject(256,colors);

    for (i = 0; i < g->bmap->d; i++)
    {
		img.ImageDesc.Left=0;
		img.ImageDesc.Top=0;
		img.ImageDesc.Width=g->bmap->w;
		img.ImageDesc.Height=g->bmap->h;
		img.ImageDesc.Interlace=0;
		img.ImageDesc.ColorMap=0;
		
		img.RasterBits=grdinfo_get_data(g->bmap,0,0,i);
		
		img.ExtensionBlockCount=0;
		img.ExtensionBlocks=0;
		
		if(i==0)
		{
			img.ExtensionBlockCount=3;
			img.ExtensionBlocks=ext;
		}
		else
		{
			img.ExtensionBlockCount=1;
			img.ExtensionBlocks=ext;
		}
		
		(void) GifMakeSavedImage(gif,&img);
	}

    if (EGifSpew(gif) == GIF_ERROR)
    {
		g->err=GifErrorString(ErrorCode);
	}
	
//bogus:
//	if(data)
//	{
//		free(data);
//	}
}


