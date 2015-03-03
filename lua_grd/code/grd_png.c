/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss@XIXs.com 2013
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
#include "all.h"


#define abort_(x) { err=x; goto bogus; }


static void read_func(png_structp ptr, png_bytep buff, png_size_t count)
{
struct grd_io_info *inf=(struct grd_io_info *)png_get_io_ptr(ptr);

	if(inf == NULL)
	{
		return;   // error
	}

	if( ( inf->pos + count ) > inf->data_len )
	{
		return;   // error
	}

	memcpy(buff,inf->data+inf->pos,count);
	
	inf->pos+=count;
} 

static void write_func(png_structp ptr, png_bytep buff, png_size_t count)
{
	struct grd_io_info *inf=(struct grd_io_info *)png_get_io_ptr(ptr);
	
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

	if(!inf->data) { png_error(ptr, "Write Error"); }
	
	memcpy(inf->data + inf->data_len, buff, count);
	inf->data_len=new_len;
}

static void flush_func(png_structp ptr)
{
	struct grd_io_info *inf=(struct grd_io_info *)png_get_io_ptr(ptr);
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// allocate a grd and read a png file into it
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void grd_png_load(struct grd * g, struct grd_io_info * inf )
{
	const char *err=0;
	int x, y, z;
	int i;
	
	int width, height,depth;
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
	
	png_textp text_ptr;
	int num_text;

    png_uint_32 next_frame_width;
    png_uint_32 next_frame_height;
    png_uint_32 next_frame_x_offset;
    png_uint_32 next_frame_y_offset;
    png_uint_16 next_frame_delay_num;
    png_uint_16 next_frame_delay_den;
    png_byte next_frame_dispose_op;
    png_byte next_frame_blend_op;
    
	int grdfmt;

	g->err="unknown error";

//	char header[8];	// 8 is the maximum size that can be checked

	/* open file and test for it being a png */
	FILE *fp=0;
	

	/* initialize stuff */
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	if (!png_ptr)
		abort_("png alloc read fail");

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
		abort_("png alloc info fail");

	if (setjmp(png_jmpbuf(png_ptr)))
		abort_("png init io fail");

	if(inf->file_name) // reading from a file
	{
		fp = fopen(inf->file_name, "rb");
		if (!fp) { abort_("png open fail"); }
		png_init_io(png_ptr, fp);
	}
	else
	{
		png_set_read_fn(png_ptr, inf, read_func);
	}
	

	png_read_info(png_ptr, info_ptr);

	width = png_get_image_width(png_ptr,info_ptr);
	height = png_get_image_height(png_ptr,info_ptr);
	color_type = png_get_color_type(png_ptr,info_ptr);
	bit_depth = png_get_bit_depth(png_ptr,info_ptr);

	depth=1;
	if(png_get_valid(png_ptr, info_ptr, PNG_INFO_acTL))
	{
		depth=png_get_num_frames(png_ptr, info_ptr);
	}
	
// choose grdfmt
	grdfmt=GRD_FMT_U8_RGBA;
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
		png_set_expand_gray_1_2_4_to_8(png_ptr);
	}
	if ( (color_type == PNG_COLOR_TYPE_GRAY_ALPHA ) || ( color_type == PNG_COLOR_TYPE_GRAY ) )
	{
		png_set_gray_to_rgb(png_ptr);
//		png_set_swap_alpha(png_ptr);
	}
	if (color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_RGB_ALPHA)
	{
//		png_set_swap_alpha(png_ptr);
//		png_set_bgr(png_ptr);png_set_gray_1_2_4_to_8
	}
	if (color_type == PNG_COLOR_TYPE_RGB)
	{
		png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);
	}


	number_of_passes = png_set_interlace_handling(png_ptr);
	png_read_update_info(png_ptr, info_ptr);

	if(!grd_realloc(g,grdfmt,width,height,depth))
		abort_("grd realloc fail");

	row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * height);
	if (!row_pointers)
		abort_("png alloc rows fail");


	/* read file */
	if (setjmp(png_jmpbuf(png_ptr)))
		abort_("png read fail");
		
	if(png_get_valid(png_ptr, info_ptr, PNG_INFO_acTL))
	{
		for(z=0;z<depth;z++)
		{
			png_read_frame_head(png_ptr, info_ptr);

			if(png_get_valid(png_ptr, info_ptr, PNG_INFO_fcTL))
			{
				png_get_next_frame_fcTL(png_ptr, info_ptr,
					&next_frame_width, &next_frame_height,
					&next_frame_x_offset, &next_frame_y_offset,
					&next_frame_delay_num, &next_frame_delay_den,
					&next_frame_dispose_op, &next_frame_blend_op);

				for (y=0; y<next_frame_height; y++)
				{
					row_pointers[y] = grdinfo_get_data( g->bmap,next_frame_x_offset , next_frame_y_offset+y , z );
				}
				png_read_image(png_ptr, row_pointers);
			}
			else
			{
				for (y=0; y<height; y++) { row_pointers[y] = grdinfo_get_data(g->bmap,0,y,z); }
				png_read_image(png_ptr, row_pointers);
			}
		}
	}
	else
	{
		for (y=0; y<height; y++) { row_pointers[y] = grdinfo_get_data(g->bmap,0,y,0); }
		png_read_image(png_ptr, row_pointers);
	}
	
	
	num_palette=0;num_trans=0;
	png_get_PLTE(png_ptr, info_ptr, &palptr , &num_palette);
	png_get_tRNS(png_ptr, info_ptr, &trans, &num_trans, &trans_values);   
	
    for(x=0;x<num_palette;x++)
    {
	u32 c;
		c=0xff000000|
		    ((palptr[x].red  &0xff)    )|
		    ((palptr[x].green&0xff)<< 8)|
		    ((palptr[x].blue &0xff)<<16);
		((u32*)g->cmap->data)[x]=c;
	}
    for(x=0;x<num_trans;x++)
    {
		((u8*)g->cmap->data)[(4*x)+3]=trans[x];
	}
    
	if (png_get_text(png_ptr, info_ptr, &text_ptr, &num_text) > 0)
	{
		for (i=0; i<num_text; i++)
		{
			if( strcmp(text_ptr[i].key,"JSON")==0 )
			{
				g->data_sizeof=strlen(text_ptr[i].text)+1;
				g->data=calloc(g->data_sizeof,1);
				if(!g->data) { abort_("text json alloc fail"); }
				strncpy(g->data,text_ptr[i].text,g->data_sizeof);
			}
//		   printf("text[%s]=%s\n",text_ptr[i].key, text_ptr[i].text);
		}
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
// read a jpg into a grd from a file
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void grd_png_load_file(struct grd * g, const char* file_name, u32 *tags)
{
	struct grd_io_info inf[1]={0};
	
	inf->file_name=file_name;
	inf->tags=tags;
	
	grd_png_load(g,inf);	
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// read a jpg into a grd from data
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void grd_png_load_data(struct grd *g, const unsigned char* data, int data_len, u32 *tags)
{
	struct grd_io_info inf[1]={0};
	
	inf->file_name=0;
	inf->data=(u8*)data;
	inf->pos=0;
	inf->data_len=data_len;
	inf->tags=tags;
	
	grd_png_load(g,inf);
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// save a grd as a png file
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void grd_png_save(struct grd *g , struct grd_io_info *inf )
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
	
    png_textp text_ptr;
	int num_text;

	png_text  text[1];
	
	u32 *tag_JSON=grd_tags_find(inf->tags,GRD_TAG_DEF('J','S','O','N'));

	/* need to create file ? */
	FILE *fp = 0;
	
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
		
			palptr[x].blue =(c>>16)&0xff;
			palptr[x].green=(c>> 8)&0xff;
			palptr[x].red  =(c    )&0xff;
			  tptr[x]      =(c>>24)&0xff;
			  
			if(tptr[x]!=255) { max_trans=x; } // skip some trans values?
		}
	
		png_set_PLTE(png_ptr, info_ptr, palptr, 256);
		if(max_trans>=0) // do we have any trans values?
		{
			png_set_tRNS(png_ptr, info_ptr, tptr, max_trans+1, 0);
		}
	}
	else
	if( (g->bmap->fmt==GRD_FMT_U8_RGBA) || (g->bmap->fmt==GRD_FMT_U8_RGBA_PREMULT) )
	{
//		png_set_swap_alpha(png_ptr);
//		png_set_bgr(png_ptr);
	}
	else
	{
		abort_("unsupported png save format");
	}




	if (setjmp(png_jmpbuf(png_ptr)))
		abort_("png init io fail");

	if(inf->file_name)
	{
		fp=fopen(inf->file_name, "wb");
		if (!fp)
			abort_("png file open fail");
		png_init_io(png_ptr, fp);
	}
	else
	{
		png_set_write_fn(png_ptr, inf, write_func, flush_func);
	}


	/* write header */
	if (setjmp(png_jmpbuf(png_ptr)))
		abort_("png write header fail");

	png_set_IHDR(png_ptr, info_ptr, width, height,
	             bit_depth, color_type, PNG_INTERLACE_NONE,
	             PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

// add a json chunk containing extra info?
	if(tag_JSON)
	{
		if(*((char **)(tag_JSON+2)))
		{
			text[0].compression = PNG_TEXT_COMPRESSION_zTXt; // PNG_TEXT_COMPRESSION_NONE;
			text[0].key = "JSON";
			text[0].text = *((char **)(tag_JSON+2));
			png_set_text(png_ptr, info_ptr, text, 1);
		}
	}

	png_write_info(png_ptr, info_ptr);


	row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * height);
	if (!row_pointers)
		abort_("png alloc rows fail");

	for (y=0; y<height; y++)
		row_pointers[y] = grdinfo_get_data(g->bmap,0,y,0); //(png_byte*) malloc(info_ptr->rowbytes);



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

void grd_png_save_file(struct grd *g , const char* file_name , u32 *tags)
{
	struct grd_io_info inf[1]={0};
	
	inf->file_name=file_name;
	inf->tags=tags;
	
	grd_png_save(g,inf);
}

// inf and inf->data must be freed by caller
/*
 * struct grd_io_info * grd_png_save_data(struct grd *g )
{
	struct grd_io_info *inf;
	inf=(struct grd_io_info *)calloc(sizeof(struct grd_io_info),1);
	if(inf)
	{
		inf->file_name=0;

		inf->data=0;
		inf->pos=0;
		inf->data_len=0;
		inf->data_len_max=0;
		
		grd_png_save(g,inf);
	}
	return inf;
}
*/
