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
// read a gif into a grd from a file
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void grd_gif_save_file(struct grd * g, const char* file_name)
{
}


