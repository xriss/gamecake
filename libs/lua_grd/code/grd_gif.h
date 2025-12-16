/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss@XIXs.com 2013
//
/*+-----------------------------------------------------------------------------------------------------------------+*/


#include "gif_lib.h"

struct grd_io_gif
{
	int i;
	int z;
	struct grd_io_info inf[1];
	GifFileType *gif;
	SavedImage img;
	GifColorType colors[256];
	ExtensionBlock ext[4];
	unsigned char control[4];
	unsigned char wank[3];
	char *err;
};

void grd_gif_save_stream_open( struct grd_io_gif *sgif,struct grd * g);
void grd_gif_save_stream_write(struct grd_io_gif *sgif,struct grd * g);
void grd_gif_save_stream_close(struct grd_io_gif *sgif);


void grd_gif_load_file(struct grd *g, const char* file_name, u32 *tags);
void grd_gif_load_data(struct grd *g, const unsigned char* data, int data_len, u32 *tags);

void grd_gif_save(struct grd * g, struct grd_io_info * inf );
void grd_gif_save_file(struct grd *g , const char* file_name , u32 *tags);
