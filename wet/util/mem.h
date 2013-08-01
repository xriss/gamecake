/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss Daniels 2003 http://www.XIXs.com
//
/*+-----------------------------------------------------------------------------------------------------------------+*/





struct mem
{

bool setup(void);
bool reset(void);
void clean(void);

u8 * malloc(s32 size);
u8 * calloc(s32 size);
u8 * realloc(void *mem,s32 size);
void free(void *mem);

	s32 total;
};



extern struct mem MEM[1];


//
// Turn off global allocator ? should probably dump the struct stuff and use globals/inlines
//
#if 0

#define MEM_malloc(a)		((u8*)::malloc(a))
#define MEM_calloc(a)		((u8*)::calloc(1,a))
#define MEM_realloc(a,b)	((u8*)::realloc((void*)(a),b))
#define MEM_free(a)			::free((void*)(a))

#else

#define MEM_malloc(a)		MEM->malloc(a)
#define MEM_calloc(a)		MEM->calloc(a)
#define MEM_realloc(a,b)	MEM->realloc(a,b)
#define MEM_free(a)			MEM->free((void*)(a))

#endif

