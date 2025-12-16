/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss@XIXs.com 2013
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
#include "all.h"



#define abort_(x) { err=x; goto bogus; }


struct my_error_mgr {
  struct jpeg_error_mgr pub;	/* "public" fields */
  jmp_buf setjmp_buffer;	/* for return to caller */
  const char *errstr;
  char buffer[JMSG_LENGTH_MAX];
};
typedef struct my_error_mgr * my_error_ptr;
METHODDEF(void)
my_error_exit (j_common_ptr cinfo)
{
  my_error_ptr myerr = (my_error_ptr) cinfo->err;
  (*cinfo->err->output_message) (cinfo);
  longjmp(myerr->setjmp_buffer, 1);
}
METHODDEF(void)
my_output_message (j_common_ptr cinfo)
{
  my_error_ptr myerr = (my_error_ptr) cinfo->err;
  (*cinfo->err->format_message) (cinfo, myerr->buffer);
   myerr->errstr=myerr->buffer;
}






/* Read JPEG image from a memory segment */
static void init_source (j_decompress_ptr cinfo) {}
static boolean fill_input_buffer (j_decompress_ptr cinfo)
{
    ERREXIT(cinfo, JERR_INPUT_EMPTY);
return TRUE;
}
static void skip_input_data (j_decompress_ptr cinfo, long num_bytes)
{
    struct jpeg_source_mgr* src = (struct jpeg_source_mgr*) cinfo->src;

    if (num_bytes > 0) {
        src->next_input_byte += (size_t) num_bytes;
        src->bytes_in_buffer -= (size_t) num_bytes;
    }
}
static void term_source (j_decompress_ptr cinfo) {}
static void jpeg_mymem_src (j_decompress_ptr cinfo, void* buffer, long nbytes)
{
    struct jpeg_source_mgr* src;

    if (cinfo->src == NULL) {   /* first time for this JPEG object? */
        cinfo->src = (struct jpeg_source_mgr *)
            (*cinfo->mem->alloc_small) ((j_common_ptr) cinfo, JPOOL_PERMANENT,
            sizeof(struct jpeg_source_mgr));
    }

    src = (struct jpeg_source_mgr*) cinfo->src;
    src->init_source = init_source;
    src->fill_input_buffer = fill_input_buffer;
    src->skip_input_data = skip_input_data;
    src->resync_to_restart = jpeg_resync_to_restart; /* use default method */
    src->term_source = term_source;
    src->bytes_in_buffer = nbytes;
    src->next_input_byte = (JOCTET*)buffer;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// allocate a grd and read a png file into it
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static void grd_jpg_load(struct grd * g, struct grd_io_info * inf )
{
	const char *err=0;

	int grdfmt;
	int width, height;
	int y;

	unsigned char *bb,*bi,*bo; // buffer ptrs
	
	JSAMPARRAY buffer;		/* Output row buffer */
	int row_stride;		/* physical row width in output buffer */

	struct jpeg_decompress_struct cinfo;
	FILE *fp=0;

	struct my_error_mgr jerr;
	jerr.errstr=0;
	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = my_error_exit;
	jerr.pub.output_message = my_output_message;
	if(setjmp(jerr.setjmp_buffer)) { goto bogus; }

	jpeg_create_decompress(&cinfo);
		
	/* open file and test for it being a jpeg */

	
	if(inf->file_name) // reading from a file
	{
		fp = fopen(inf->file_name, "rb");
		if (!fp) { abort_("jpg open fail"); }
		jpeg_stdio_src(&cinfo, fp);
	}
	else // reading from memory
	{
		jpeg_mymem_src(&cinfo, (void*)inf->data ,inf->data_len );
	}
		
	jpeg_read_header(&cinfo, TRUE);
	
// choose grdfmt
	width=cinfo.image_width;
	height=cinfo.image_height;
	grdfmt=GRD_FMT_U8_RGBA;
	
	if(!grd_realloc(g,grdfmt,width,height,1))
		abort_("grd realloc fail");
	
	jpeg_start_decompress(&cinfo);
	
	row_stride = cinfo.output_width * cinfo.output_components;
	
// libjpeg will GC this memory, grab a line at a time
	buffer = (*cinfo.mem->alloc_sarray) ((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);
	y=0;	
	while (cinfo.output_scanline < cinfo.output_height)
	{
		jpeg_read_scanlines(&cinfo, buffer, 1);
		
		bo=grdinfo_get_data(g->bmap,0,y,0);
		bb=buffer[0];
		
		if(cinfo.num_components==1) // handle greyscale jpegs
		{
			for( bi=bb ; bi<bb+(width*1) ; bi+=1 , bo+=4 )
			{
				bo[0]=bi[0];
				bo[1]=bi[0];
				bo[2]=bi[0];
				bo[3]=255;
			}
		}
		else
		{
			for( bi=bb ; bi<bb+(width*3) ; bi+=3 , bo+=4 )
			{
				bo[0]=bi[0];
				bo[1]=bi[1];
				bo[2]=bi[2];
				bo[3]=255;
			}
		}
		
		y++;
	}
 
	jpeg_finish_decompress(&cinfo);

bogus:
	if(jerr.errstr) { err=jerr.errstr; } // internal jpeg error
	jpeg_destroy_decompress(&cinfo);
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
void grd_jpg_load_file(struct grd * g, const char* file_name, u32 *tags)
{
	struct grd_io_info inf[1]={0};
	
	inf->file_name=file_name;
	inf->data=0;
	inf->pos=0;
	inf->data_len=0;
	inf->tags=tags;
	
	grd_jpg_load(g,inf);	
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// read a jpg into a grd from data
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void grd_jpg_load_data(struct grd * g, const unsigned char* data, int data_len, u32 *tags)
{
	struct grd_io_info inf[1]={0};
	
	inf->file_name=0;
	inf->data=(u8*)data;
	inf->pos=0;
	inf->data_len=data_len;
	inf->tags=tags;
	
	grd_jpg_load(g,inf);	
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// save a grd as a jpg file
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void grd_jpg_save_file(struct grd *g , const char* file_name , u32 *tags)
{
	struct grd *rgb;
	int quality=85;
	const char *err=0;

	int image_width=g->bmap->w;
	int image_height=g->bmap->h;
	int y;


	rgb=grd_duplicate_convert(g,GRD_FMT_U8_RGB); // tempory data?


//	g->err="jpg save disabled";

	struct jpeg_compress_struct cinfo;
	FILE * outfile;		/* target file */
	JSAMPROW row_pointer[1];	/* pointer to JSAMPLE row[s] */

	struct my_error_mgr jerr;
	jerr.errstr=0;
	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = my_error_exit;
	jerr.pub.output_message = my_output_message;
	if(setjmp(jerr.setjmp_buffer)) { goto bogus; }

	jpeg_create_compress(&cinfo);

	if ((outfile = fopen(file_name, "wb")) == NULL)	{ abort_("jpg open fail"); }
	jpeg_stdio_dest(&cinfo, outfile);

	cinfo.image_width = image_width; 	/* image width and height, in pixels */
	cinfo.image_height = image_height;
	cinfo.input_components = 3;		/* # of color components per pixel */
	cinfo.in_color_space = JCS_RGB; 	/* colorspace of input image */
	jpeg_set_defaults(&cinfo);
	jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);

	jpeg_start_compress(&cinfo, TRUE);
	y=0;
	while (cinfo.next_scanline < cinfo.image_height)
	{
		row_pointer[0] = grdinfo_get_data(rgb->bmap,0,y++,0);
		(void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
	}
	jpeg_finish_compress(&cinfo);

bogus:
	if(jerr.errstr) { err=jerr.errstr; } // internal jpeg error
	jpeg_destroy_compress(&cinfo);
	if(outfile) { fclose(outfile); }
	if(err) {g->err=err;} else {g->err=0; }
	if(rgb!=g) { grd_free(rgb); } // destroy tmp bgr grid
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// save a grd as a jpg file
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void grd_jpg_save(struct grd *g , struct grd_io_info *inf )
{
	const char *err=0;
	g->err=0;
	if(inf->file_name)
	{
		grd_jpg_save_file(g,inf->file_name,inf->tags);
	}
	else
	{
		abort_("grd jpg save data fail");
	}
bogus:
	if(err) {g->err=err;}
}


