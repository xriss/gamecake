/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss Daniels 2003 http://www.XIXs.com
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
#include "all.h"



#define abort_(x) { err=x; goto bogus; }



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
static void jpeg_mem_src (j_decompress_ptr cinfo, void* buffer, long nbytes)
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

struct jpg_loader_info
{
	const char * file_name;
	u8 * data;
	int data_len;
};

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// allocate a grd and read a png file into it
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void grd_jpg_load(struct grd * g, struct jpg_loader_info * inf )
{
	const char *err=0;

	int grdfmt;
	int width, height;
	int y;

	unsigned char *bb,*bi,*bo; // buffer ptrs
	
	JSAMPARRAY buffer;		/* Output row buffer */
	int row_stride;		/* physical row width in output buffer */

	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	
	cinfo.err = jpeg_std_error(&jerr);

	jpeg_create_decompress(&cinfo);
		
	/* open file and test for it being a jpeg */
	FILE *fp=0;
	
	if(inf->file_name) // reading from a file
	{
		fp = fopen(inf->file_name, "rb");
		if (!fp) { abort_("jpg open fail"); }
		jpeg_stdio_src(&cinfo, fp);
	}
	else // reading from memory
	{
		jpeg_mem_src(&cinfo, inf->data ,inf->data_len );
	}
		
	jpeg_read_header(&cinfo, TRUE);
	
// choose grdfmt
	width=cinfo.image_width;
	height=cinfo.image_height;
	grdfmt=GRD_FMT_U8_ARGB;
	
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
		
		bo=g->bmap->get_data(0,y,0);
		bb=buffer[0];
		for( bi=bb ; bi<bb+(width*3) ; bi+=3 , bo+=4 )
		{
			bo[0]=255;
			bo[1]=bi[0];
			bo[2]=bi[1];
			bo[3]=bi[2];
		}
		
		y++;
	}
 
	jpeg_finish_decompress(&cinfo);

bogus:

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
void grd_jpg_load_file(struct grd * g, const char* file_name)
{
	jpg_loader_info inf[1];
	
	inf->file_name=file_name;
	inf->data=0;
	inf->data_len=0;
	
	grd_jpg_load(g,inf);	
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// read a jpg into a grd from data
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void grd_jpg_load_data(struct grd * g, u8* data, int data_len)
{
	jpg_loader_info inf[1];
	
	inf->file_name=0;
	inf->data=data;
	inf->data_len=data_len;
	
	grd_jpg_load(g,inf);	
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// save a grd as a png file
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void grd_jpg_save_file(struct grd *g , const char* file_name )
{
	g->err="jpg save disabled";
}

