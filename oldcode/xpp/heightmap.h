/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss Daniels 2003 http://www.XIXs.com
//
/*+-----------------------------------------------------------------------------------------------------------------+*/


struct hmap_tile
{
	f32	height;

	f32 x,y;	// intended slope used for basic building of height
};



struct heightmap
{

	s32					numof_tiles;	// number of tiles allocated
	struct hmap_tile		*tiles;		// pointer to memory chunk that contains numof_tiles
	s32		tw,th;						// width,height in tiles, or 0,0 if not a cut up tilesmap uesd for 2d indexing
};


heightmap * hmap_alloc( void );
void hmap_free( heightmap *hmap );


bool hmap_setup( heightmap *hmap );
bool hmap_clean( heightmap *hmap );

bool hmap_build( heightmap *hmap , s32 width , s32 height );
bool hmap_build_from_mmap( heightmap *hmap , metamap *mmap , f32 steepness );


bool hmap_draw(heightmap *hmap);
