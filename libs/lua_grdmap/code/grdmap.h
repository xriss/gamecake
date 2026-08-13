/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss@XIXs.com 2013
//
/*+-----------------------------------------------------------------------------------------------------------------+*/


struct grdmap_tile
{
	struct grdmap_tile *master;	// pointer to master tile or 0 if we are a master tile

	int32_t		id; // a unique ID of this tile (an index into base->tiles)
	
	int32_t		x;					// location oftile in base image
	int32_t		y;
	int32_t		w;					// width and height of tile
	int32_t		h;

	int32_t		hx;					// x,y handle, local coords so relative to x,y 
	int32_t		hy;

	struct grdmap		*base;				// where this tile comes from
};

struct grdmap
{

	int32_t					numof_tiles;	// number of tiles allocated
	struct grdmap_tile		 *tiles;	// pointer to memory chunk that contains numof_tiles
	
	int32_t		tw,th;						// width,height of grd in tiles
	int32_t		pw,ph;						// width,height of each tile in pixels

	struct grd *g;	// pointer to grd data
	
	const char *err; // set to an error string on error
};


struct grdmap * grdmap_alloc();
void            grdmap_free(struct grdmap *gm);

//int grdmap_setup(struct grdmap *gm,struct grd *g,int32_t pw,int32_t ph);
int grdmap_setup(struct grdmap *gm,struct grd *g);
int grdmap_clean(struct grdmap *gm);

int grdmap_cutup(struct grdmap *gm,int32_t pw,int32_t ph);


int grdmap_merge( struct grdmap *g );

int grdmap_keymap( struct grdmap *a , struct grdmap *b );

int grdmap_shrink( struct grdmap *grdmap );

void grdmap_tile_shrink(struct grdmap_tile *a );

int grdmap_tile_compare(struct grdmap_tile *a , struct grdmap_tile *b);


