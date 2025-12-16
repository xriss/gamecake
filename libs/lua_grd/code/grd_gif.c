/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss@XIXs.com 2013
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
#include "all.h"


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// file or memory
//
/*+-----------------------------------------------------------------------------------------------------------------+*/

static int grd_gif_read(GifFileType *gif,char *buff,int count)
{
	struct grd_io_info *inf=(struct grd_io_info *)(gif->UserData);
	
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

static int grd_gif_write(GifFileType *gif,char *buff,int count)
{
	struct grd_io_info *inf=(struct grd_io_info *)(gif->UserData);
	
	size_t new_len = inf->data_len + count;
	
	if(!inf->data)
	{
		inf->data_len_max=1024;
		while(new_len > inf->data_len_max ) { inf->data_len_max*=2; }
		inf->data = malloc(inf->data_len_max);
	}
	if(new_len > inf->data_len_max )
	{
		while(new_len > inf->data_len_max ) { inf->data_len_max*=2; }
		inf->data = realloc(inf->data, inf->data_len_max);
	}

	if(!inf->data) { return 0; }
	
	memcpy(inf->data + inf->data_len, buff, count);
	inf->data_len=new_len;

	return count;
}

void grd_gif_inf_clean(struct grd_io_info *inf)
{
	// free any allocated data
 	if(inf->data)
	{
		free(inf->data);
		inf->data=0;
		inf->data_len_max=0;
		inf->data_len=0;
	}
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// read a gif, into this grd, allocating the correct size (animations just go in the Z depth)
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static void grd_gif_load(struct grd * g, struct grd_io_info * inf )
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
		g->cmap->w=cmp->ColorCount;
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
		if (DGifCloseFile(gif,0) == GIF_ERROR)
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
void grd_gif_load_file(struct grd * g, const char* file_name, u32 *tags)
{
	struct grd_io_info inf[1]={0};
	
	inf->file_name=file_name;
	inf->data=0;
	inf->pos=0;
	inf->data_len=0;
	inf->tags=tags;
	
	grd_gif_load(g,inf);	
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// read a gif into a grd from data
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void grd_gif_load_data(struct grd *g, const unsigned char* data, int data_len, u32 *tags)
{
	struct grd_io_info inf[1]={0};
	
	inf->file_name=0;
	inf->data=(u8*)data;
	inf->pos=0;
	inf->data_len=data_len;
	inf->tags=tags;
	
	grd_gif_load(g,inf);
}



/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// save a gif into a file (Z layers are animation frames)
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void grd_gif_save_file(struct grd * g, const char* file_name, u32 *tags)
{
	struct grd_io_info inf[1]={0};
	inf->file_name=file_name;
	grd_gif_save(g,inf);
	return;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// save a gif into a file (Z layers are animation frames)
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void grd_gif_save(struct grd * g, struct grd_io_info * inf )
{
	int i;
	struct grd_io_gif sgif[1]={0};
	memcpy(sgif->inf,inf,sizeof(sgif->inf));
	
	grd_gif_save_stream_open(sgif,g);
	for (i = 0; i < g->bmap->d; i++)
	{
		sgif->z=i;
		grd_gif_save_stream_write(sgif,g);
	}
	grd_gif_save_stream_close(sgif);

	memcpy(inf,sgif->inf,sizeof(sgif->inf)); // may contain return values so copy it back
	return;

bogus:

	grd_gif_inf_clean(inf);
	return;
}

/*{
int ErrorCode;
GifFileType *gif = (GifFileType *)NULL;
SavedImage img;
GifColorType colors[256];
int i;
unsigned char *p;

ExtensionBlock ext[4];
unsigned char control[4];
unsigned char wank[3];

	u32 *tag_SPED=grd_tags_find(inf->tags,GRD_TAG_DEF('S','P','E','D'));
	u32 speed=80;
	if(tag_SPED) { speed=*((u32*)(tag_SPED+2)); } // get speed in 1/1000 seconds


	ext[0].ByteCount=11;
	ext[0].Bytes=(GifByteType *)"NETSCAPE2.0";
	ext[0].Function=APPLICATION_EXT_FUNC_CODE;

	ext[1].ByteCount=3;
	ext[1].Bytes=wank;
	ext[1].Function=CONTINUE_EXT_FUNC_CODE;

	ext[2].ByteCount=4;
	ext[2].Bytes=(GifByteType*)control;
	ext[2].Function=GRAPHICS_EXT_FUNC_CODE;

	wank[0]=0x01;
	wank[1]=0x00; // loop 4 ever
	wank[2]=0x00;

	control[0]=0x08; // flags: 
	control[1]=((speed/10)    )&0xff; // speed_lo: speed of animation in 100ths of a second, so 8 is 12.5fps (80ms)
	control[2]=((speed/10)>>16)&0xff; // speed_hi: which is near the classic "Shooting on twos" speed of 12fps
	control[3]=0; // alpha: find first fully alpha color to use as transparent 
	for(i=0;i<256;i++)
	{
		if(g->cmap->data[3+i*4]==0)
		{
			control[3]=i;
			control[0]|=0x01; // set flag to use a transparent color
			break;
		}
	}
// Actually I think gifs are broken if you use any index other than 0 for the transparency...
// it breaks the dispose method, whatever you choose...

//unsigned char *data=0;

	if(inf->file_name)
	{
		if((gif = EGifOpenFileName( inf->file_name , 0, &ErrorCode)) == NULL)
		{
			g->err=GifErrorString(ErrorCode);
			goto bogus;
		}
	}
	else
	{
		if((gif = EGifOpen( (void *)inf , (OutputFunc) grd_gif_write , &ErrorCode)) == NULL)
		{
			g->err=GifErrorString(ErrorCode);
			goto bogus;
		}
	}
//    EGifSetGifVersion(gif,1); // must be set for transparency to work?

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
	gif->SBackGroundColor = control[3];
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
			img.ExtensionBlocks=ext+2;
		}
		
		(void) GifMakeSavedImage(gif,&img);
	}

	if (EGifSpew(gif) == GIF_ERROR)
	{
		g->err=GifErrorString(ErrorCode);
		goto bogus;
	}
	
	return;

bogus:

	// free any allocated data
 	if(inf->data)
	{
		free(inf->data);
		inf->data=0;
		inf->data_len_max=0;
		inf->data_len=0;
	}
	return;
}*/



void grd_gif_save_stream_open(struct grd_io_gif *sgif,struct grd * g)
{
int ErrorCode;
int i;
unsigned char *p;

u32 *tag_SPED=grd_tags_find(sgif->inf->tags,GRD_TAG_DEF('S','P','E','D'));
u32 speed=80;
if(tag_SPED) { speed=*((u32*)(tag_SPED+2)); } // get speed in 1/1000 seconds
	
	sgif->ext[0].ByteCount=11;
	sgif->ext[0].Bytes=(GifByteType *)"NETSCAPE2.0";
	sgif->ext[0].Function=APPLICATION_EXT_FUNC_CODE;

	sgif->ext[1].ByteCount=3;
	sgif->ext[1].Bytes=sgif->wank;
	sgif->ext[1].Function=CONTINUE_EXT_FUNC_CODE;

	sgif->ext[2].ByteCount=4;
	sgif->ext[2].Bytes=(GifByteType*)sgif->control;
	sgif->ext[2].Function=GRAPHICS_EXT_FUNC_CODE;

	sgif->wank[0]=0x01;
	sgif->wank[1]=0x00; // loop 4 ever
	sgif->wank[2]=0x00;

	sgif->control[0]=0x08; // flags: 
	sgif->control[1]=((speed/10)    )&0xff; // speed_lo: speed of animation in 100ths of a second, so 8 is 12.5fps (80ms)
	sgif->control[2]=((speed/10)>>16)&0xff; // speed_hi: which is near the classic "Shooting on twos" speed of 12fps
	sgif->control[3]=0; // alpha: find first fully alpha color to use as transparent 
	for(i=0;i<g->cmap->w;i++)
	{
		if(g->cmap->data[3+i*4]==0)
		{
			sgif->control[3]=i;
			sgif->control[0]|=0x01; // set flag to use a transparent color
			break;
		}
	}
// Actually I think gifs are broken if you use any index other than 0 for the transparency...
// it breaks the dispose method, whatever you choose...

//unsigned char *data=0;

	if(sgif->inf->file_name)
	{
		if((sgif->gif = EGifOpenFileName( sgif->inf->file_name , 0, &ErrorCode)) == NULL)
		{
			g->err=GifErrorString(ErrorCode);
			goto bogus;
		}
	}
	else
	{
		if((sgif->gif = EGifOpen( (void *)sgif->inf , (OutputFunc) grd_gif_write , &ErrorCode)) == NULL)
		{
			g->err=GifErrorString(ErrorCode);
			goto bogus;
		}
	}
//    EGifSetGifVersion(gif,1); // must be set for transparency to work?

	sgif->gif->SWidth = g->bmap->w;
	sgif->gif->SHeight = g->bmap->h;
	
	p=grdinfo_get_data(g->cmap,0,0,0);
	for(i=0;i<g->cmap->w;i++)
	{
		sgif->colors[i].Red=p[0];
		sgif->colors[i].Green=p[1];
		sgif->colors[i].Blue=p[2];
		p=p+4;
	}
	
	sgif->gif->SColorResolution = 8;
	sgif->gif->SBackGroundColor = sgif->control[3];
	sgif->gif->SColorMap = GifMakeMapObject(g->cmap->w,sgif->colors);

	sgif->i=0;

	return;
bogus:
	grd_gif_inf_clean(sgif->inf);
}

void grd_gif_save_stream_write(struct grd_io_gif *sgif,struct grd * g)
{
int i;
unsigned char *p;

	sgif->img.ImageDesc.Left=0;
	sgif->img.ImageDesc.Top=0;
	sgif->img.ImageDesc.Width=g->bmap->w;
	sgif->img.ImageDesc.Height=g->bmap->h;
	sgif->img.ImageDesc.Interlace=0;

	p=grdinfo_get_data(g->cmap,0,0,0);
	for(i=0;i<g->cmap->w;i++)
	{
		sgif->colors[i].Red=p[0];
		sgif->colors[i].Green=p[1];
		sgif->colors[i].Blue=p[2];
		p=p+4;
	}
	sgif->img.ImageDesc.ColorMap=GifMakeMapObject(g->cmap->w,sgif->colors);

	
	sgif->img.RasterBits=grdinfo_get_data(g->bmap,0,0,sgif->z);
	
	sgif->img.ExtensionBlockCount=0;
	sgif->img.ExtensionBlocks=0;
	
	if(sgif->i==0)
	{
		sgif->img.ExtensionBlockCount=3;
		sgif->img.ExtensionBlocks=sgif->ext;
	}
	else
	{
		sgif->img.ExtensionBlockCount=1;
		sgif->img.ExtensionBlocks=sgif->ext+2;
	}
	
	(void) GifMakeSavedImage(sgif->gif,&sgif->img);

	sgif->i++;

}

void grd_gif_save_stream_close(struct grd_io_gif *sgif)
{

	if (EGifSpew(sgif->gif) == GIF_ERROR)
	{
		sgif->err="write fail";
		goto bogus;
	}
	
	return;
bogus:
	grd_gif_inf_clean(sgif->inf);
}
