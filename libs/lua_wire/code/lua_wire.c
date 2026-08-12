/*

 Copyright (C) 2026 Kriss Blank < Kriss@XIXs.com >
 This file is distributed under the terms of the MIT license.
 http://en.wikipedia.org/wiki/MIT_License

Note that much of the documentation for the C functions exposed to Lua 
can be found in the associated wire.lua file.

Most of the low level error handling is crazy, in that if anything goes 
wrong its probably catastrophic so the error handling is pointless. 
however we try to be as graceful as possible and put things back how we 
found them etc etc. But for all I know this may actually make it worse.

*/


#include <stdlib.h>
#include <string.h>
#include <math.h>

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


// lightly hacked and we expose some of its functions directly in wire
#include "cmsgpack/lua_cmsgpack.c"


// allocate this many empty threads/fifos in a single chunk
#define WIRE_SLOTS_CHUNK_SIZE 16

// size of internal array of chunks
#define WIRE_SLOTS_CHUNK_COUNT 256

// total number of slots, this is a hard limit to threads and fifos
#define WIRE_SLOTS_MAX (WIRE_SLOTS_CHUNK_SIZE*WIRE_SLOTS_CHUNK_COUNT)

typedef struct wire_msg
{
	struct wire_msg *next; // we are a linked list
	int sender; // handle of the thread that sent this message (used to reply)
	void *id; // unique id for lifetime of this msg (used to reply)
	int size; // size of data
	const char *data; // ptr to data
} wire_msg ;


typedef struct wire_fifo
{
	int handle; // may be 0 if this is the corpse of a dead fifo
				// must have wire_slots lock before changing handle

	mtx_t mutex; // must have this lock before read/write this struct
	cnd_t msg; // condition wait for msg

	int count; // length of linked list updated as we add and remove
	wire_msg *first; // first message ptr, 0 if none

} wire_fifo ;


typedef struct wire_thread
{
	int handle; // may be 0 if this is the corpse of a dead thread
				// must have wire_slots lock before changing handle
	
	mtx_t mutex; // must have this lock before read/write this struct

	lua_CFunction preload; // optional function to preload lua libs ( setup loaders )
	const char *name; // need to free when thread ends
	const char *start; // lua code string to load then free then run
	int status; // 0 not running , 1 running , -1 running but please halt
	lua_State *l; // the lua state running in this thread
	thrd_t thread; // actual thread handle
	wire_fifo fifo; // this threads reply fifo
	
} wire_thread ;


typedef struct wire_slots
{
	mtx_t mutex; // must have this lock before write this struct
	cnd_t msg; // condition wait for any msg in any fifo or thread

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

Allocate a duplicate string.

returns 0 on error

*/
static const char * wire_dupe_string( const char *str )
{
	int len=strlen(str);

	char *s=(char *)calloc(1,len+1);
	if(!s){ return 0; }
	
	strcpy(s,str);

	return (const char *)s;
}

/*+---------------------------------------------------------------------

Allocate a new msg struct, data will be copied to end of struct and 
struct ptrs initialized. data may be 0 if size is 0.

Use generic free to free msg and associated data.

returns 0 on error

*/
static wire_msg * wire_msg_alloc( const char *data , int size )
{
	wire_msg * msg = calloc(1,sizeof(wire_msg)+size);
	if( !msg ) { return 0; }
	
	if( size )
	{
		msg->data=(const char *)(msg+1);
		msg->size=size;
		memcpy( (void*)msg->data , data , size );
	}

	return msg;
}


/*+---------------------------------------------------------------------

internal function to init a thread

returns 0 on error

*/
static wire_fifo * wire_fifo_init( wire_fifo * fifo )
{
	if( thrd_success != cnd_init( &(fifo->msg) ) )
	{ return 0; }
	
	if( thrd_success != mtx_init( &(fifo->mutex) , mtx_plain ) )
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

	if( thrd_success != mtx_init( &(thread->mutex) , mtx_plain ) )
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

Return the given item, do not grow.

returns 0 on error

*/
static void * wire_slots_get( wire_slots * slots , int idx )
{
	if( slots->free_idx < idx )
	{
		return 0;
	}
	
	int cnum=(idx-1) / WIRE_SLOTS_CHUNK_SIZE ;
	int inum=(idx-1) - ( WIRE_SLOTS_CHUNK_SIZE * cnum );

	char *ptr = (char*)slots->chunks[cnum];
	ptr=ptr+(inum*slots->item_size);
	
	return (void *)ptr;
}

/*+---------------------------------------------------------------------

Return the given item, growing available slots if need be.

returns 0 on error

*/
static void * wire_slots_grow_and_get( wire_slots * slots , int idx )
{
	if( slots->free_idx < idx )
	{
		if( !wire_slots_grow(slots,idx) ) { return 0; }
	}
	return wire_slots_get(slots,idx);
}


/*+---------------------------------------------------------------------

Allocate and return a new slot idx.

returns idx or 0 if empty

*/
static int wire_slots_alloc( wire_slots * slots , int idx )
{	
	if(idx==0) { idx=slots->used_idx+1; } // take next slot
	void *ptr=wire_slots_grow_and_get( slots , idx ); // maybe grow
	if(!ptr){ return 0; } // check
	if( idx>slots->used_idx ) { slots->used_idx=idx; } // update used

	return idx;
}

/*+---------------------------------------------------------------------

internal function to init a slots array

returns 0 on error

*/
static wire_slots * wire_slots_init( wire_slots * slots )
{
	if( thrd_success != mtx_init( &(slots->mutex) , mtx_plain ) )
	{ return 0; }
	
	if( ! wire_slots_grow(slots , 1 ) ) // force first chunk
	{ return 0; }
	
	return slots;
}

/*+---------------------------------------------------------------------

Check if this item exists or is available, may call with no locks 
but the answer may be stale immediately.

returns handle or 0 if empty

*/
static int wire_slots_handle( wire_slots * slots , int idx )
{
	if( slots->free_idx < idx ) // nothing to check
	{
		return 0;
	}
	
	int cnum=(idx-1) / WIRE_SLOTS_CHUNK_SIZE ;
	int inum=(idx-1) - ( WIRE_SLOTS_CHUNK_SIZE * cnum );
	char *ptr = (char*)slots->chunks[cnum];
	ptr=ptr+(inum*slots->item_size);
	
	return *((int *)ptr); // first int must be handle
}

/*+---------------------------------------------------------------------

Lock and unlock with lua errors on fail.

*/
static int lua_wire_slots_lock (lua_State *l, int lock , wire_slots *slots)
{
	if(lock) // do lock
	{
		if( thrd_success!=mtx_lock(&slots->mutex) )
		{ luaL_error(l, "wire slots lock failed"); }
	}
	else // do unlock
	{
		if( thrd_success!=mtx_unlock(&slots->mutex) )
		{ luaL_error(l, "wire slots unlock failed"); }
	}
	return 0;
}
static int lua_wire_thread_lock (lua_State *l, int lock , wire_thread *thread)
{
	if(lock) // do lock
	{
		if( thrd_success!=mtx_lock(&thread->fifo.mutex) )
		{ luaL_error(l, "wire thread fifo lock failed"); }
		if( thrd_success!=mtx_lock(&thread->mutex) )
		{
			mtx_unlock(&thread->fifo.mutex); // try? and keep things clean
			luaL_error(l, "wire thread lock failed");
		}
	}
	else // do unlock
	{
		if( thrd_success!=mtx_unlock(&thread->mutex) )
		{ luaL_error(l, "wire thread unlock failed"); }
		if( thrd_success!=mtx_unlock(&thread->fifo.mutex) )
		{ luaL_error(l, "wire thread fifo unlock failed"); }
	}
	return 0;
}
static int lua_wire_fifo_lock (lua_State *l, int lock , wire_fifo *fifo)
{
	if(lock) // do lock
	{
		if( thrd_success!=mtx_lock(&fifo->mutex) )
		{ luaL_error(l, "wire fifo lock failed"); }
	}
	else // do unlock
	{
		if( thrd_success!=mtx_unlock(&fifo->mutex) )
		{ luaL_error(l, "wire fifo unlock failed"); }
	}
	return 0;
}


/*+---------------------------------------------------------------------

Take a nap.

*/
static int lua_wire_sleep (lua_State *l)
{
	double d=lua_tonumber(l,1); // time in seconds to sleep for
	
	struct timespec duration = { d } ; // should get seconds
	duration.tv_nsec = (d-floor(d))*1000000000.0 ; // and the fraction

	thrd_sleep(&duration,0);
	
	return 0;
}

/*+---------------------------------------------------------------------

get the current time with a timespec_get TIME_UTC

*/
static int lua_wire_time (lua_State *l)
{
	struct timespec now; timespec_get(&now, TIME_UTC);
	double d=((double)(now.tv_sec))+(((double)(now.tv_nsec))/1000000000.0 );
	lua_pushnumber(l, d);
	return 1;
}

/*+---------------------------------------------------------------------

get the current time res with a timespec_getres TIME_UTC

*/
// no worky on windows
#if 0
static int lua_wire_timeres (lua_State *l)
{
	struct timespec now; timespec_getres(&now, TIME_UTC);
	double d=((double)(now.tv_sec))+(((double)(now.tv_nsec))/1000000000.0 );
	lua_pushnumber(l, d);
	return 1;
}
#endif

/*+---------------------------------------------------------------------

get a light userdata from a string/table/userdata/thread/function. We 
use this function to generate an ID from a table. This is currently 
unique ( across threads ) and constant while the table remains on the 
stack. Future versions of lua(jit) may update GC to move tables around 
and break this assumption but that seems unlikely.

Anyway, it works for now...

*/
static int lua_wire_pointer (lua_State *l)
{
	void *ptr=(void*)lua_topointer(l, 1);
	
	if(!ptr) { return 0; }

	lua_pushlightuserdata(l, ptr);
	
	return 1;
}

/*+---------------------------------------------------------------------

C function to start a lua state in a new thread

*/
static int lua_wire_thread_start_lua ( void *context )
{
	wire_thread *thread=(wire_thread *)context; // our thread struct
	
	lua_State *l=lua_open();
	luaL_openlibs(l);

	lua_wire_thread_lock(l,1,thread);
	
		thread->l=l; // remember state

		if( thread->preload ) { thread->preload(l); } // make more code available

		luaL_loadstring(l, thread->start); // start lua code
		free((void*)thread->start); thread->start=0; // free code
		
	lua_wire_thread_lock(l,0,thread);

	lua_call(l, 0, 0); // no returns

// lock the thread before modifying it...
	lua_wire_thread_lock(l,1,thread);

		thread->handle=0; // mark thread as deleted
		thread->status=0; // we have halted

		if(thread->name) // free thread name
		{
			free((void*)thread->name);
			thread->name=0;
		}
		
	lua_wire_thread_lock(l,0,thread);
	lua_close(l); // close/free everything
	
	return 0;
}

/*+---------------------------------------------------------------------

Start a thread running

*/
static int lua_wire_thread_start (lua_State *l)
{
	int handle=(int)lua_tonumber(l,1); // handle may be 0
	const char *name=lua_tostring(l,2); // name of thread
	const char *start=lua_tostring(l,3); // run this string
	lua_CFunction preload=0; // optional cfunction to preload more libs in a new lua state
	if(!lua_isnoneornil(l,4)) { preload=lua_tocfunction(l,4); }
	
	if( (handle>=0) || ((-handle)>WIRE_SLOTS_MAX) ) { luaL_error(l, "invalid thread handle"); }

	wire_thread *thread=(wire_thread *)wire_slots_get( &wire_threads , -handle ); 
	if(!thread)
	{
		luaL_error(l, "missing thread %d",handle);
	}
	lua_wire_thread_lock(l,1,thread); // we will be updating the thread
		
		thread->preload = preload;
		
		thread->name = wire_dupe_string(name); // need to free this in new thread
		if(!thread->name)
		{
			lua_wire_thread_lock(l,0,thread);
			luaL_error(l, "alloc failed");
		}

		thread->start = wire_dupe_string(start); // need to free this in new thread
		if(!thread->start)
		{
			lua_wire_thread_lock(l,0,thread);
			luaL_error(l, "alloc failed");
		}
		
		thread->status = 1; // mark as running
		if( thrd_success!=thrd_create( &thread->thread , lua_wire_thread_start_lua , (void*)thread ) )
		{
			thread->status=0;
			free((void*)thread->start);
			thread->start=0;
			lua_wire_thread_lock(l,0,thread);
			luaL_error(l, "thread start failed");
		}

	lua_wire_thread_lock(l,0,thread);
		
	return 0;
}


/*+---------------------------------------------------------------------

Create a thread 

*/
static int lua_wire_thread_create (lua_State *l)
{
	int handle=(int)lua_tonumber(l,1); // handle may be 0
	
	if( (handle>0) || ((-handle)>WIRE_SLOTS_MAX) ) { luaL_error(l, "invalid thread handle"); }
	
	lua_wire_slots_lock(l,1,&wire_threads);
	
		if( (handle) && wire_slots_handle(&wire_threads,-handle) ) // thread already exists
		{
			lua_wire_slots_lock(l,0,&wire_threads);
			luaL_error(l, "thread %d already exists",handle);
		}
		
		handle=-wire_slots_alloc(&wire_threads,-handle); // allocate thread handle
		if(!handle)
		{
			lua_wire_slots_lock(l,0,&wire_threads);
			luaL_error(l, "thread slots alloc failed");
		}
		
		wire_thread *thread=(wire_thread *)wire_slots_grow_and_get( &wire_threads , -handle ); 
		if(!thread)
		{
			lua_wire_slots_lock(l,0,&wire_threads);
			luaL_error(l, "thread slots alloc failed");
		}

		lua_wire_thread_lock(l,1,thread); // we will be updating the thread
		
			thread->handle=handle;
			thread->fifo.handle=handle;

		lua_wire_thread_lock(l,0,thread);
		
	lua_wire_slots_lock(l,0,&wire_threads);
	lua_pushnumber(l,handle);
	return 1;
}

/*+---------------------------------------------------------------------

Destroy a thread, this signals the thread to end and waits for the 
thread to exit.

*/
static int lua_wire_thread_destroy (lua_State *l)
{
	int handle=(int)lua_tonumber(l,1); // thread handle
	if( (handle>0) || ((-handle)>WIRE_SLOTS_MAX) ) { luaL_error(l, "invalid thread handle"); }

	lua_wire_slots_lock(l,1,&wire_threads);
		wire_thread *thread=(wire_thread *)wire_slots_get( &wire_threads , -handle ); 
		if(!thread)
		{
			lua_wire_slots_lock(l,0,&wire_threads);
			luaL_error(l, "missing thread %d",handle);
		}
		lua_wire_thread_lock(l,1,thread); // we will be updating the thread

			thrd_t t = thread->thread ; // cache thread while locked...

			if(thread->status) { thread->status=-1; } // request halt if running

		lua_wire_thread_lock(l,0,thread);
	lua_wire_slots_lock(l,0,&wire_threads);

	int res=0;
	thrd_join( t , &res ); // wait for thread to actually end

	lua_wire_thread_lock(l,1,thread);

		// make sure the threads handle and status has been set to 0
		thread->status=0;
		thread->handle=0;

	lua_wire_thread_lock(l,0,thread);

	return 0;
}

/*+---------------------------------------------------------------------

get/set thread status , returns nil if unknown thread does not error

*/
static int lua_wire_thread_status (lua_State *l)
{
	int handle=(int)lua_tonumber(l,1); // thread handle
	if( (handle>=0) || ((-handle)>WIRE_SLOTS_MAX) ) { return 0; }
	int status=0;
	if( lua_isnumber(l,2) ) // we want to set the status ( to halt if running )
	{
		status=(int)lua_tonumber(l,2);
	}
	
	wire_thread *thread=(wire_thread *)wire_slots_get( &wire_threads , -handle );
	if(!thread)	{ return 0; }

	if( status==-1 ) // we want to set the status ( to halt if running )
	{
		lua_wire_slots_lock(l,1,&wire_threads);
			lua_wire_thread_lock(l,1,thread); // we will be updating the thread
 				if(thread->status) { thread->status=-1; } // request halt if running
			lua_wire_thread_lock(l,0,thread);
		lua_wire_slots_lock(l,0,&wire_threads);
	}

	lua_pushnumber(l,thread->status); // no real point in locking?

	return 1;
}

/*+---------------------------------------------------------------------

Create a fifo and return its handle

Allocate a fifo to send work requests to, where one fifo may be shared 
across multiple worker threads each taking on work simultaneously.

Fifos have positive handles, threads have negative handles. Both 
handles represent a fifo, negative handles represents the threads fifo.

*/
static int lua_wire_fifo_create (lua_State *l)
{
	int handle=(int)lua_tonumber(l,1); // 0 allocates new or we try and reuse old handle
	
	if( (handle<0) || (handle>WIRE_SLOTS_MAX) ) { luaL_error(l, "invalid fifo handle"); }
	
	lua_wire_slots_lock(l,1,&wire_fifos);
	
	if( (handle) && wire_slots_handle(&wire_fifos,handle) ) // already exists
	{
		lua_wire_slots_lock(l,0,&wire_fifos);
		luaL_error(l, "fifo %d already exists",handle);
	}
	
	handle=wire_slots_alloc(&wire_fifos,handle); // allocate fifo handle
	if(!handle)
	{
		lua_wire_slots_lock(l,0,&wire_fifos);
		luaL_error(l, "fifo slots alloc failed");
	}
	
	wire_fifo *fifo=(wire_fifo *)wire_slots_grow_and_get( &wire_fifos , handle ); 
	if(!fifo)
	{
		lua_wire_slots_lock(l,0,&wire_fifos);
		luaL_error(l, "fifo slots alloc failed");
	}

	lua_wire_fifo_lock(l,1,fifo); // we will be updating the fifo
	
	fifo->handle=handle; // mark as allocated
	
	lua_wire_fifo_lock(l,0,fifo);
	lua_wire_slots_lock(l,0,&wire_fifos);
	
	lua_pushnumber(l,handle);
	return 1;
}

/*+---------------------------------------------------------------------

destroy a fifo

*/
static int lua_wire_fifo_destroy (lua_State *l)
{
	int handle=(int)lua_tonumber(l,1);
	
	if( (handle<=0) || (handle>WIRE_SLOTS_MAX) ) { luaL_error(l, "invalid fifo handle"); }
	
	lua_wire_slots_lock(l,1,&wire_fifos);
	
	if( ! wire_slots_handle(&wire_fifos,handle) ) // does not exist
	{
		lua_wire_slots_lock(l,0,&wire_fifos);
		luaL_error(l, "fifo %d does not exist",handle);
	}
	
	wire_fifo *fifo=(wire_fifo *)wire_slots_get( &wire_fifos , handle ); 
	if(!fifo)
	{
		lua_wire_slots_lock(l,0,&wire_fifos);
		luaL_error(l, "fifo slots get failed");
	}

	lua_wire_fifo_lock(l,1,fifo); // we will be updating the fifo
	
	fifo->handle=0; // mark as free
	
	// TODO: empty the msg list here
	
	lua_wire_fifo_lock(l,0,fifo);
	lua_wire_slots_lock(l,0,&wire_fifos);
	
	return 0;
}

/*+---------------------------------------------------------------------

convert handle at lua stack top into a valid fifo ptr

Note fifo may or not be active, we just know it exists in the slots, 
still need to lock and check the handle.

*/
static wire_fifo * lua_wire_fifo (lua_State *l , int top )
{
	int handle=(int)lua_tonumber(l,top);
	if( (handle==0) ) { luaL_error(l, "invalid fifo handle"); }

	wire_fifo *fifo=0;
	if( (handle<0) )
	{
		if( ((-handle)>WIRE_SLOTS_MAX) ) { luaL_error(l, "invalid fifo handle"); }
		wire_thread *thread=(wire_thread *)wire_slots_get( &wire_threads , -handle ); 
		if( !thread ) { luaL_error(l, "unknown thread %d",handle); }
		fifo=&thread->fifo;
	}
	else
	{
		if( ((handle)>WIRE_SLOTS_MAX) ) { luaL_error(l, "invalid fifo handle"); }
		fifo=(wire_fifo *)wire_slots_get( &wire_fifos , handle );
		if( !fifo ) { luaL_error(l, "unknown fifo %d",handle); }
	}

	return fifo;
}

/*+---------------------------------------------------------------------

fifo peek

Note that the returned data value is a light userdata not a string, the 
actual data has not been copied into the lua state yet and is not safe 
to access.

*/
static int lua_wire_fifo_peek (lua_State *l)
{
	// volatile helps maybe, need to check?
	wire_fifo *fifo=lua_wire_fifo(l , 1 );

	lua_wire_fifo_lock(l,1,fifo);
	
		wire_msg *msg=fifo->first;
	
		if(msg)
		{

			lua_pushlightuserdata(l,(void*)msg->data);
			lua_pushnumber(l, msg->sender); // fifo handle
			lua_pushlightuserdata(l, msg->id); // Unique ptr

			lua_wire_fifo_lock(l,0,fifo);
			return 3;
		}

	lua_wire_fifo_lock(l,0,fifo);
	return 0;
}

/*+---------------------------------------------------------------------

fifo pull

*/
static int lua_wire_fifo_pull (lua_State *l)
{
	wire_fifo *fifo=lua_wire_fifo(l , 1 );

	lua_wire_fifo_lock(l,1,fifo);
	
		wire_msg *msg=fifo->first; // get
		if( msg ) { fifo->first=msg->next; fifo->count--; } // remove

	lua_wire_fifo_lock(l,0,fifo);

	if(!msg) { return 0; } // no msg to return
	
	lua_pushlstring(l, msg->data , msg->size ); // copy data into lua state

	lua_pushnumber(l, msg->sender); // fifo handle
	lua_pushlightuserdata(l, msg->id); // Unique ptr

	free(msg);// msg was ours to free

	return 3;
}

/*+---------------------------------------------------------------------

fifo push

*/
static int lua_wire_fifo_push (lua_State *l)
{
	wire_fifo *fifo=lua_wire_fifo(l , 1 );

	size_t len=0;
	const char *ptr = lua_tolstring(l , 2 , &len );
	if(!ptr) { luaL_error(l, "wire msg data required"); }
	wire_msg *msg=wire_msg_alloc(ptr,len); // duplicate data
	if(!msg) { luaL_error(l, "wire msg alloc failed"); }

	msg->sender=lua_tonumber(l , 3 ); // fifo handle
	msg->id=(void*)lua_topointer(l , 4 ); // unique ptr

	lua_wire_fifo_lock(l,1,fifo);
	
//		int handle=fifo->handle); // remember fifo handle
	
		if(!fifo->first)
		{
			fifo->first=msg; // entire list is just this msg
		}
		else
		{
			wire_msg *last=fifo->first;
			while(last->next) { last=last->next; } // loop to last
			last->next=msg; // append to last
		}

	lua_wire_fifo_lock(l,0,fifo);
	
	cnd_signal( &(fifo->msg) ); // waiting for a msg to this fifo

// not sure what use this would be?
//	if( handle < 0 ) // this is a thread msg
//	{
//		cnd_broadcast( &(wire_threads->msg) ); // wake up threads only if this is sent to a threads fifo
//	}
// so only use the one in wire_fifos

	cnd_broadcast( &(wire_fifos.msg) ); // waiting for a msg to any fifo

	return 0;
}

/*+---------------------------------------------------------------------

fifo wait on msg

*/
static int lua_wire_fifo_wait (lua_State *l)
{
	wire_fifo *fifo=lua_wire_fifo(l , 1 );

	double d=lua_tonumber(l,2); // max time in seconds to wait

	// add now to d and create till with that time
	struct timespec now; timespec_get(&now, TIME_UTC);
	d+=((double)(now.tv_sec))+(((double)(now.tv_nsec))/1000000000.0 );
	struct timespec till = { floor(d) ,   (d-floor(d))*1000000000.0 };

	lua_wire_fifo_lock(l,1,fifo); // need to lock in order to wait...
	cnd_timedwait( &(fifo->msg) , &(fifo->mutex) , &till ); // do wait
	lua_wire_fifo_lock(l,0,fifo);

	return 0;
}

/*+---------------------------------------------------------------------

wait on any msg in any fifo

*/
static int lua_wire_wait (lua_State *l)
{
	double d=lua_tonumber(l,1); // max time in seconds to wait

	// add now to d and create till with that time
	struct timespec now; timespec_get(&now, TIME_UTC);
	d+=((double)(now.tv_sec))+(((double)(now.tv_nsec))/1000000000.0 );
	struct timespec till = { floor(d) ,   (d-floor(d))*1000000000.0 };

	lua_wire_slots_lock(l,1,&wire_fifos); // need to lock in order to wait...
	cnd_timedwait( &(wire_fifos.msg) , &(wire_fifos.mutex) , &till ); // do wait
	lua_wire_slots_lock(l,0,&wire_fifos);

	return 0;
}

/*+---------------------------------------------------------------------

check if a handle is valid, returns handle or nil

*/
static int lua_wire_handle (lua_State *l)
{
	if( lua_isnoneornil(l, 1 ) ) { return 0; }

	int handle=(int)lua_tonumber(l, 1 );
	if( (handle==0) ) { return 0; }

	wire_fifo *fifo=0;
	if( (handle<0) )
	{
		if( ((-handle)>WIRE_SLOTS_MAX) ) { return 0; }
		wire_thread *thread=(wire_thread *)wire_slots_get( &wire_threads , -handle ); 
		if( !thread ) { return 0; }
		if( thread->handle != handle ) { return 0; }
	}
	else
	{
		if( ((handle)>WIRE_SLOTS_MAX) ) { return 0; }
		fifo=(wire_fifo *)wire_slots_get( &wire_fifos , handle );
		if( !fifo ) { return 0; }
		if( fifo->handle != handle ) { return 0; }
	}

	lua_pushnumber(l, handle );
	return 1;
}


/*+---------------------------------------------------------------------

find the handle for the calling thread, this is an expensive search

*/
static int lua_wire_thread_handle (lua_State *l)
{
	int handle=0;
	thrd_t us=thrd_current();

	lua_wire_slots_lock(l,1,&wire_threads);
	
	// usually requested shortly after creation so search backwards
	for( int idx=wire_threads.used_idx ; idx>=1 ; idx-- )
	{
		wire_thread *thread=(wire_thread *)wire_slots_get( &wire_threads , idx );
		
//		if( thrd_equal( thread->thread , us ) )
		if( thread->thread == us )
		{
			handle=thread->handle;
		}

		if(handle) // found thread and it is a valid handle
		{
			lua_pushnumber(l, thread->handle );
			lua_pushstring(l, thread->name );
			lua_wire_slots_lock(l,0,&wire_threads);
			return 2;
		}
	}
	// thread not found
	lua_wire_slots_lock(l,0,&wire_threads);
	return 0;
}


/*+---------------------------------------------------------------------

unpack a string or userdata+length into a lua table

*/
static int lua_wire_unpack(lua_State *L)
{
	size_t len=0;
	const char *s=0;
    mp_cur c;

	s = luaL_checklstring(L,1,&len);

	mp_cur_init(&c,(const unsigned char *)s,len);
	mp_decode_to_lua_type(L,&c);

	if (c.err == MP_CUR_ERROR_EOF)
	{
		return luaL_error(L,"Missing bytes in input.");
	}
	else
	if (c.err == MP_CUR_ERROR_BADFMT)
	{
		return luaL_error(L,"Bad data format in input.");
	}

	return 1;
}

/*+---------------------------------------------------------------------

pack a lua table into a string or userdata,length

*/
static int lua_wire_pack(lua_State *L)
{
	mp_buf *buf=0;
	int data=0;

	// id second arg is true then return userdata
	if( lua_isboolean(L,2) ) { data=lua_toboolean(L,2); }

	buf = mp_buf_new(L);

	lua_pushvalue(L, 1);
	mp_encode_lua_type(L,buf,0);

	if(data)
	{
		lua_pushlightuserdata(L,(void*)buf->b);
		lua_pushnumber(L,buf->len);
		// note we do not free the string
		// must free later with
		// mp_realloc(buf->L, buf->b, buf->len, 0);
		// using the two values we returned
		mp_buf_dont_free(buf);
		return 2;
	}
	else
	{
		// return string
		lua_pushlstring(L,(char*)buf->b,buf->len);
		mp_buf_free(buf);
		return 1;
	}
}

/*+---------------------------------------------------------------------

free a userdata,length

*/
static int lua_wire_freepack(lua_State *L)
{
	void *ptr;
	int len;
	ptr=lua_touserdata(L,1);
	len=lua_tonumber(L,2);

	mp_realloc(L, ptr, len, 0);
	return 0;
}

/*+---------------------------------------------------------------------

open library.

*/
LUALIB_API int luaopen_wire_core (lua_State *l)
{
	const luaL_Reg lib[] =
	{
		// hacked messagepack functions
		{"unpack",						lua_wire_unpack},
		{"pack",						lua_wire_pack},
		{"freepack",					lua_wire_freepack},
		
		// internal helper functions
		{"time",						lua_wire_time},
		{"pointer",						lua_wire_pointer},
		{"thread_handle",				lua_wire_thread_handle},
		{"handle",						lua_wire_handle},
// no worky on windows
//		{"timeres",						lua_wire_timeres},

		// global function
		{"sleep",						lua_wire_sleep},
		{"wait",						lua_wire_wait},

		// thread function
		{"thread_create",				lua_wire_thread_create},
		{"thread_start",				lua_wire_thread_start},
		{"thread_destroy",				lua_wire_thread_destroy},
		{"thread_status",				lua_wire_thread_status},

		// fifo functions
		{"fifo_create",					lua_wire_fifo_create},
		{"fifo_destroy",				lua_wire_fifo_destroy},
		{"fifo_peek",					lua_wire_fifo_peek},
		{"fifo_pull",					lua_wire_fifo_pull},
		{"fifo_push",					lua_wire_fifo_push},
		{"fifo_wait",					lua_wire_fifo_wait},

		{0,0}
	};
	
	// first open must be on the main thread
	// dont need locks because they do not exist yet
	// allocate and fill in main thread ( -1 handle )
	if( wire_threads.free_idx==0 )
	{
		wire_fifos.item_size=sizeof(wire_fifo);
		wire_fifos.item_init=(void * (*)(void *))wire_fifo_init;
		if( !wire_slots_init(&wire_fifos) ) { luaL_error(l, "alloc wire slots fifos failed"); }

		wire_threads.item_size=sizeof(wire_thread);
		wire_threads.item_init=(void * (*)(void *))wire_thread_init;
		if( !wire_slots_init(&wire_threads) ) { luaL_error(l, "alloc wire slots threads failed"); }
		
		wire_thread *thread=(wire_thread *)wire_slots_grow_and_get(&wire_threads,1);
		if( !thread ) { luaL_error(l, "grow wire slots threads failed"); }
		wire_threads.used_idx=1; //  thread 1 is always the main thread

		thread->status = 1; // we are this thread and we are running
		thread->fifo.handle = -1; // threads have negative handles
		thread->handle = -1;
		thread->name = wire_dupe_string( "main" );
		thread->thread = thrd_current(); // main thread
	}

	lua_newtable(l);
	luaL_openlib(l, NULL, lib, 0);
	
	return 1;
}

