/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss@XIXs.com 2013
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



#if 0

#define MAX_METAMAP_LAYERS	6


struct datachunk
{
	s32 totalsize;		// size of data allocated to this structure
	s32 size;			// size of data used ( has a maximum of totalsize-sizeof(this) )

	u8	data[4];
};


struct metamap_tile
{
	s8		flip_x;
	s8		flip_y;
	s8		mapID;			// id of map to get tile from,
	s8		layaID;			// id of laya to get tile from, for flat tiles.
	s16		x;				// location to get tile from,
	s16		y;				// if id and location point at this tile, then the tile is not referenced

	s32		index;			// the index to be used for this tile, set on export
	s32		pal;			// the pallete

	struct metamap_tile *master;	// pointer to master tile or 0 if we are a master tile
};


struct metamap_layer
{
	s32		charsetID;

	bool	ignore_chars;	// dont output chars from this layer

	path_bits	fname[1];

	ILuint ImgId[1];

	u8	*data;
};


struct metamap
{

// input

	s32		ID;				// ID of this map, its index into the map table you are using

	u8		index_mask;		// the index bits we care about, for 256/metatiles use 0xff for 16color use 0x0f

	bool	canflip;

	s32		tile_width;		// size of tile to cut the metamap into
	s32		tile_height;	// size of tiles to cut the metamap into

	bool	ignore_tiles;	// do not output tiles from this map...

	path_bits	fname[1];


	s32		maptype;
	s32		mapindex;

// data inmput/output

	s32						num_layers;
	struct metamap_layer		layers[MAX_METAMAP_LAYERS];

// output

	s32		depth;		// layers

	s32		width;		// in pixels
	s32		height;		// in pixels

	s32		width_in_tiles;
	s32		height_in_tiles;

	s32		width_in_8tiles;
	s32		height_in_8tiles;

	struct metamap_tile		*map;		// x,y array of tiles
	struct metamap_tile		*map8;		// x,y array of 8x8 tiles

	datachunk *indexmap;			// output tile index data chunk map

// stats

	s32 stats_8tiles_master;
	s32 stats_8tiles_slave;
	s32 stats_8tiles_slave_noflip;
	s32 stats_8tiles_slave_xflip;
	s32 stats_8tiles_slave_yflip;
	s32 stats_8tiles_slave_xyflip;

	s32 stats_tiles_master;
	s32 stats_tiles_slave;
	s32 stats_tiles_slave_noflip;
	s32 stats_tiles_slave_xflip;
	s32 stats_tiles_slave_yflip;
	s32 stats_tiles_slave_xyflip;
};



int DO_MMap(void);
int DO_RaceMap(void);

#endif
