

// this is going to be a bit wobbiliy and redefinable, somehow...
// however its size is always 1x1x1 
typedef struct {
	
	u32		id;
	
} myc_cell;


// every byte in this structure expands to 32k per block
typedef struct {
	
	u16		idx;	// index into parent data so bottom 15 bits are zzzzzyyyyyxxxxx
	u16		flags;
	
	myc_cell	*data; // possible pointer to array of 8x8x8 (512) cells, or 0 if this block is just empty
	
} myc_block;

typedef struct {
	
	s32		px;		// position, within world, of this chunk
	s32		py;
	s32		pz;
	
	myc_block	blocks[32*32*32]; // an array of 32*32*32 (32768) blocks

} myc_chunk;

typedef struct {
	
	s32		hx;		// size (defined in chunks) so 1x1x1 is 32x32x32 blocks or 256x256x256 cells
	s32		hy;
	s32		hz;
	
	myc_chunk	**chunks; // array of pointers to chunks

} myc_world;



#ifdef __cplusplus
extern "C" {
#endif

LUALIB_API int luaopen_wetgenes_mycroft_core (lua_State *l);

#ifdef __cplusplus
};
#endif

