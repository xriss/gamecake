/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss Daniels 2003 http://www.XIXs.com
//
/*+-----------------------------------------------------------------------------------------------------------------+*/





struct mmap_tile
{
	struct mmap_tile *master;	// pointer to master tile or 0 if we are a master tile


	s32		x;					// location oftile in base image
	s32		y;
	s32		w;					// width and height of tile
	s32		h;

	s32		hx;					// x,y handle, local coords so relative to x,y 
	s32		hy;

	struct metamap		*base;				// where this tile comes from

};



struct metamap
{

	s32					numof_tiles;	// number of tiles allocated
	struct mmap_tile		*tiles;		// pointer to memory chunk that contains numof_tiles
	s32		tw,th;						// width,height in tiles, or 0,0 if not a cut up tile map used for 2d indexing


	ILuint image;			// cached DevIL image ID and other cached Devil info follows

	s32		w,h;			// width,height in pixels
	s32		sizeof_pix;		// byte size of a pixel

	s32		format,type;	// image type info
	s32		span;			// amount to add to data to move down a line
	u8		*data;			// image data pointer

};


bool metamap_char_compare(mmap_tile *a , mmap_tile *b);
void metamap_char_shrink(mmap_tile *a );


bool mmap_setup( metamap *mmap );
bool mmap_clean( metamap *mmap );

void mmap_update_cache( metamap *mmap );

bool mmap_cutup( metamap *mmap , s32 w, s32 h);
bool mmap_merge( metamap *mmap );

bool mmap_keymap( metamap *a , metamap *b );
bool mmap_layout( metamap *a , metamap *b , s32 border);

bool mmap_shrink( metamap *mmap );


bool mmap_save_XTX( metamap *mmap_layout , metamap *mmap_master ,const char *filename);

