/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss Daniels 2003 http://www.XIXs.com
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
#include "all.h"



#define abort_(x) { err=x; goto bogus; }




/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// allocate a grd and read a png file into it
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void grd_png_load_file(struct grd * g, const char* file_name)
{
	
	const char *err=0;
	int x, y;

	int width, height;
	png_byte color_type;
	png_byte bit_depth;

	png_structp png_ptr=0;
	png_infop info_ptr=0;
	int number_of_passes;
	png_bytep * row_pointers=0;
	int num_palette;
	png_color *palptr;
	int num_trans;
	u8 *trans;
	png_color_16 *trans_values;
	
	int grdfmt;

	char header[8];	// 8 is the maximum size that can be checked

	/* open file and test for it being a png */
	FILE *fp = fopen(file_name, "rb");
	if (!fp)
		abort_("png open fail");
	fread(header, 1, 8, fp);
	if (png_sig_cmp((png_byte *)header, 0, 8))
		abort_("png unrecognised header");


	/* initialize stuff */
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	if (!png_ptr)
		abort_("png alloc read fail");

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
		abort_("png alloc info fail");

	if (setjmp(png_jmpbuf(png_ptr)))
		abort_("png init io fail");

	png_init_io(png_ptr, fp);
	png_set_sig_bytes(png_ptr, 8);

	png_read_info(png_ptr, info_ptr);

	width = info_ptr->width;
	height = info_ptr->height;
	color_type = info_ptr->color_type;
	bit_depth = info_ptr->bit_depth;

// choose grdfmt
	grdfmt=GRD_FMT_U8_BGRA;
	if (color_type == PNG_COLOR_TYPE_PALETTE)
	{
		grdfmt=GRD_FMT_U8_INDEXED;
	}

	if (color_type == PNG_COLOR_TYPE_GRAY )
	{
		grdfmt=GRD_FMT_U8_LUMINANCE;
	}


// cleanup what we are expecting a little
	if (bit_depth == 16)
	{
		png_set_strip_16(png_ptr);
	}
	if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
	{
		png_set_gray_1_2_4_to_8(png_ptr);
	}
	if (color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_RGB_ALPHA)
	{
		png_set_swap_alpha(png_ptr);
//		png_set_bgr(png_ptr);
	}
	if (color_type == PNG_COLOR_TYPE_RGB)
	{
		png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
	}



	number_of_passes = png_set_interlace_handling(png_ptr);
	png_read_update_info(png_ptr, info_ptr);


	/* read file */
	if (setjmp(png_jmpbuf(png_ptr)))
		abort_("png read fail");

	if(!grd_realloc(g,grdfmt,width,height,1))
		abort_("grd realloc fail");

	row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * height);
	if (!row_pointers)
		abort_("png alloc rows fail");

	for (y=0; y<height; y++)
		row_pointers[y] = g->bmap->get_data(0,y,0); //(png_byte*) malloc(info_ptr->rowbytes);

	png_read_image(png_ptr, row_pointers);
	
	num_palette=0;num_trans=0;
	png_get_PLTE(png_ptr, info_ptr, &palptr , &num_palette);
	png_get_tRNS(png_ptr, info_ptr, &trans, &num_trans, &trans_values);   
	
    for(x=0;x<num_palette;x++)
    {
	u32 c;
		c=0x000000ff|
		    ((palptr[x].red  &0xff)<< 8)|
		    ((palptr[x].green&0xff)<<16)|
		    ((palptr[x].blue &0xff)<<24);
		((u32*)g->cmap->data)[x]=c;
	}
    for(x=0;x<num_trans;x++)
    {
		((u8*)g->cmap->data)[(4*x)+0]=trans[x];
	}
    
bogus:

	if( png_ptr )
	{
		png_destroy_read_struct(&png_ptr,&info_ptr,0);
	}
	if(row_pointers)
	{
		free(row_pointers); // temporary
	}
	if (fp)
	{
		fclose(fp);
	}
	if(err) {g->err=err;} else {g->err=0; }
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// save a grd as a png file
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void grd_png_save_file(struct grd *g , const char* file_name )
{
	const char *err=0;

	int x, y;

	int width, height;
	png_byte color_type;
	png_byte bit_depth;

	png_structp png_ptr=0;
	png_infop info_ptr=0;
	int number_of_passes;
	png_bytep * row_pointers;


	/* create file */
	FILE *fp = fopen(file_name, "wb");
	if (!fp)
		abort_("png file open fail");


	/* initialize stuff */
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	if (!png_ptr)
		abort_("png alloc write fail");

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
		abort_("png alloc info fail");


	width=g->bmap->w;
	height=g->bmap->h;
	bit_depth=8;
	color_type = PNG_COLOR_TYPE_RGB_ALPHA;
//	png_set_filler(png_ptr, 0, PNG_FILLER_AFTER);
//	png_set_bgr(png_ptr);

	if(g->bmap->fmt==GRD_FMT_U8_INDEXED)
	{
   	png_color palptr[256];
   	u8 tptr[256];
   	int max_trans=-1;
   	
		color_type = PNG_COLOR_TYPE_PALETTE;
		

		for(x=0;x<256;x++)
		{
		u32 c=((u32*)g->cmap->data)[x];
		
			palptr[x].blue =(c>>24)&0xff;
			palptr[x].green=(c>>16)&0xff;
			palptr[x].red  =(c>> 8)&0xff;
			  tptr[x]      =(c    )&0xff;
			  
			if(tptr[x]!=255) { max_trans=x; } // skip some trans values?
		}
	
		png_set_PLTE(png_ptr, info_ptr, palptr, 256);
		if(max_trans>=0) // do we have any trans values?
		{
			png_set_tRNS(png_ptr, info_ptr, tptr, max_trans+1, 0);
		}
	}
	else
	if(g->bmap->fmt==GRD_FMT_U8_BGRA)	
	{
		png_set_swap_alpha(png_ptr);
//		png_set_bgr(png_ptr);
	}
	else
	{
		abort_("unsupported png save format");
	}




	if (setjmp(png_jmpbuf(png_ptr)))
		abort_("png init io fail");

	png_init_io(png_ptr, fp);


	/* write header */
	if (setjmp(png_jmpbuf(png_ptr)))
		abort_("png write header fail");

	png_set_IHDR(png_ptr, info_ptr, width, height,
	             bit_depth, color_type, PNG_INTERLACE_NONE,
	             PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	png_write_info(png_ptr, info_ptr);


	row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * height);
	if (!row_pointers)
		abort_("png alloc rows fail");

	for (y=0; y<height; y++)
		row_pointers[y] = g->bmap->get_data(0,y,0); //(png_byte*) malloc(info_ptr->rowbytes);



	/* write bytes */
	if (setjmp(png_jmpbuf(png_ptr)))
		abort_("png write fail");

	png_write_image(png_ptr, row_pointers);


	/* end write */
	if (setjmp(png_jmpbuf(png_ptr)))
		abort_("png write end fail");

	png_write_end(png_ptr, NULL);


bogus:

	if( png_ptr )
	{
		png_destroy_write_struct(&png_ptr,&info_ptr);
	}
	if(row_pointers)
	{
		free(row_pointers); // temporary
	}
	if (fp)
	{
		fclose(fp);
	}
	if(err) {g->err=err;} else {g->err=0; }
}

