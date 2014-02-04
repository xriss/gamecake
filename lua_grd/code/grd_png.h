/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss@XIXs.com 2013
//
/*+-----------------------------------------------------------------------------------------------------------------+*/

void grd_png_load_file(struct grd *g, const char* file_name);
void grd_png_load_data(struct grd *g, const unsigned char* data, int data_len);

void grd_png_save(struct grd *g , struct grd_io_info *inf );
void grd_png_save_file(struct grd *g , const char* file_name );
struct grd_io_info * grd_png_save_data(struct grd *g );
