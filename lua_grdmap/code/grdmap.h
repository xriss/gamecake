/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss Daniels 2003 http://www.XIXs.com
//
/*+-----------------------------------------------------------------------------------------------------------------+*/


struct grdmap_tile
{
	struct grdmap_tile *master;	// pointer to master tile or 0 if we are a master tile

	s32		id; // a unique ID of this tile (an index into base->tiles)
	
	s32		x;					// location oftile in base image
	s32		y;
	s32		w;					// width and height of tile
	s32		h;

	s32		hx;					// x,y handle, local coords so relative to x,y 
	s32		hy;

	struct grdmap		*base;				// where this tile comes from
};

struct grdmap
{

	s32					numof_tiles;	// number of tiles allocated
	struct grdmap_tile		 *tiles;	// pointer to memory chunk that contains numof_tiles
	
	s32		tw,th;						// width,height of grd in tiles
	s32		pw,ph;						// width,height of each tile in pixels

	struct grd *g;	// pointer to grd data
	
	const char *err; // set to an error string on error
};


struct grdmap * grdmap_alloc();
void            grdmap_free(struct grdmap *gm);

bool grdmap_setup(struct grdmap *gm,struct grd *g,s32 pw,s32 ph);
bool grdmap_clean(struct grdmap *gm);

bool grdmap_cutup(struct grdmap *gm,s32 pw,s32 ph);


bool grdmap_merge( grdmap *g );

bool grdmap_keymap( grdmap *a , grdmap *b );

bool grdmap_shrink( grdmap *grdmap );

void grdmap_tile_shrink(grdmap_tile *a );

bool grdmap_tile_compare(grdmap_tile *a , grdmap_tile *b);


