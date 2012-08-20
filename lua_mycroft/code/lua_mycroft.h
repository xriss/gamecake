

// this is going to be a bit wobbiliy and redefinable, somehow...
typedef struct {
	
	u32		id;
	
} myc_cell;


typedef struct {
	
	s32		px;		// position (within parent)
	s32		py;
	
	s32		hx;		// size (number of cells) probably 8x8
	s32		hy;
	
	myc_cell default_cell[1];	// if data is 0 then all cells in this block are...	
	
	void	*data;
	
} myc_block;

typedef struct {
	
	s32		px;		// position (within parent)
	s32		py;
	
	s32		hx;		// size (number of blocks) probably 32*32
	s32		hy;
	
	myc_cell default_block[1];	// if data is 0 then all blocks in this chunk are...	

	void	*data;

} myc_chunk;

typedef struct {
	
	s32		hx;		// size (number of chunkc)
	s32		hy;
	
	void	*data;

} myc_world;



#ifdef __cplusplus
extern "C" {
#endif

LUALIB_API int luaopen_wetgenes_mycroft_core (lua_State *l);

#ifdef __cplusplus
};
#endif

