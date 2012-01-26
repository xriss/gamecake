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
void grd_jpg_load_file(struct grd * g, const char* file_name)
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
	FILE *fp = fopen(file_name, "rb");
	if (!fp)
		abort_("jpg open fail");

	
	jpeg_stdio_src(&cinfo, fp);
		
	jpeg_read_header(&cinfo, TRUE);
	
// choose grdfmt
	width=cinfo.image_width;
	height=cinfo.image_height;
	grdfmt=GRD_FMT_U8_BGRA;
	
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
// save a grd as a png file
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void grd_jpg_save_file(struct grd *g , const char* file_name )
{
	g->err="jpg save disabled";
}

