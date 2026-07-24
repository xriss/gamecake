/*

 Copyright (C) 2026 Kriss Blank < Kriss@XIXs.com >
 This file is distributed under the terms of the MIT license.
 http://en.wikipedia.org/wiki/MIT_License

Note that much of the documentation for the C functions exposed to Lua 
can be found in the associated box2d.lua file.

*/


#include <stdlib.h>
#include <string.h>

// currently need mingw hax for windows, must also link with small c file
// https://github.com/jtsiomb/c11threads
#ifdef __MINGW32__
#include "c11threads.h"
#else
#include "threads.h"
#endif

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"



// allocate this many empty threads/fifos in a single chunk
#define WIRE_SLOTS_CHUNK_SIZE 64

// size of internal array of chunks
#define WIRE_SLOTS_CHUNK_COUNT 64

// total number of slots, this is a hard limit to threads and fifos
#define WIRE_SLOTS_MAX (WIRE_SLOTS_CHUNK_SIZE*WIRE_SLOTS_CHUNK_COUNT)

typedef struct wire_msg
{
	struct wire_msg *next; // we are a linked list
	int handle; // handle of the thread that sent this message
	const void *id; // unique id for lifetime of this msg (used to reply)
	const int size; // size of data
	const char *data; // ptr to data
} wire_msg ;


typedef struct wire_fifo
{
	mtx_t mutex; // lock for this fifo
	int handle; // may be 0 if this is the corpse of a dead fifo
	int count; // length of linked list updated as we add and remove
	wire_msg *first; // first message ptr, 0 if none
} wire_fifo ;


typedef struct wire_thread
{
	mtx_t mutex; // lock for this thread
	int handle; // may be 0 if this is the corpse of a dead thread
	thrd_t thread; // actual thread handle
	wire_fifo fifo; // this threads reply fifo
} wire_thread ;


typedef struct wire_slots
{
	mtx_t mutex;	// we only need to lock on write

	int used_idx; 	// max used idx ( only ever increases )

	int free_idx;	// max available idx ( only ever increases )
					// will be a multiple of WIRE_SLOTS_CHUNK_SIZE

	int item_size;	// size of each item
	void * (*item_init)( void * fifo ); // init function for each item

	void *chunks[WIRE_SLOTS_CHUNK_COUNT]; // fixed size array of pointers
	// each of these pointers represents [WIRE_SLOTS_CHUNK_SIZE] items

} wire_slots ;


wire_slots wire_threads={0};
wire_slots wire_fifos={0};


/*+---------------------------------------------------------------------

internal function to init a thread

returns 0 on error

*/
static wire_fifo * wire_fifo_init( wire_fifo * fifo )
{
	if( thrd_success != mtx_init( &(fifo->mutex) , mtx_timed ) )
	{ return 0; }
	
	return fifo;
}


/*+---------------------------------------------------------------------

internal function to init a thread

returns 0 on error

*/
static wire_thread * wire_thread_init( wire_thread * thread )
{	
	if( !wire_fifo_init( &thread->fifo ) )
	{ return 0; }

	if( thrd_success != mtx_init( &(thread->mutex) , mtx_timed ) )
	{ return 0; }
	
	return thread;
}

/*+---------------------------------------------------------------------

internal function to grow a slots array so idx is available

returns 0 on error

An error here is catastrophic.

*/
static wire_slots * wire_slots_grow( wire_slots * slots , int idx )
{
	while( slots->free_idx < idx )
	{
		int count=slots->free_idx/WIRE_SLOTS_CHUNK_SIZE;
		slots->chunks[count]=calloc(WIRE_SLOTS_CHUNK_SIZE,slots->item_size);
		if( ! slots->chunks[count] ) { return 0; }
		char *ptr_min = (char*)slots->chunks[count];
		char *ptr_max = ptr_min+(WIRE_SLOTS_CHUNK_SIZE*slots->item_size);
		for( char *ptr = ptr_min ; ptr < ptr_max ; ptr += slots->item_size )
		{
			if( ! (*slots->item_init)(ptr) )
			{
				return 0;
			}
		}
		slots->free_idx+=WIRE_SLOTS_CHUNK_SIZE; // now available
	}
	
	return slots;
}

/*+---------------------------------------------------------------------

Return the given item, growing available slots if need be.

*/
static void * wire_slots_get( wire_slots * slots , int idx )
{
	if( slots->free_idx < idx )
	{
		if( !wire_slots_grow(slots,idx) ) { return 0; }
	}
	
	int cnum=(idx-1) / WIRE_SLOTS_CHUNK_SIZE ;
	int inum=(idx-1) - ( WIRE_SLOTS_CHUNK_SIZE * cnum );

	char *ptr = (char*)slots->chunks[cnum];
	ptr=ptr+(inum*slots->item_size);
	
	return (void *)ptr;
}

/*+---------------------------------------------------------------------

internal function to init a slots array

returns 0 on error

*/
static wire_slots * wire_slots_init( wire_slots * slots )
{
	if( thrd_success != mtx_init( &(slots->mutex) , mtx_timed ) )
	{ return 0; }
	
	if( ! wire_slots_grow(slots , 1 ) ) // force first chunk
	{ return 0; }
	
	return slots;
}

/*+---------------------------------------------------------------------

open library.

*/
LUALIB_API int luaopen_wire_core (lua_State *l)
{
	const luaL_Reg lib[] =
	{
//		{"thread_create",				lua_wire_thread_create},
//		{"thread_destroy",				lua_wire_thread_destroy},

		{0,0}
	};
	
	// first open must be on the main thread
	// so allocate and fill in main thread ( -1 handle )
	if( wire_threads.free_idx==0 )
	{
		wire_fifos.item_size=sizeof(wire_fifo);
		wire_fifos.item_init=(void * (*)(void *))wire_fifo_init;
		if( !wire_slots_init(&wire_fifos) ) { luaL_error(l, "wire slots alloc failed"); }

		wire_threads.item_size=sizeof(wire_thread);
		wire_threads.item_init=(void * (*)(void *))wire_thread_init;
		if( !wire_slots_init(&wire_threads) ) { luaL_error(l, "wire slots alloc failed"); }
		
		wire_thread *thread=(wire_thread *)wire_slots_get(&wire_threads,1);
		if( !thread ) { luaL_error(l, "wire slots alloc failed"); }
		wire_threads.used_idx=1; //  thread 1 is always the main thread

		thread->fifo.handle = -1;
		thread->handle = -1;
		thread->thread = thrd_current(); // main thread
	}

	lua_newtable(l);
	luaL_openlib(l, NULL, lib, 0);
	
	return 1;
}

