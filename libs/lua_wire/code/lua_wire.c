/*

 Copyright (C) 2026 Kriss Blank < Kriss@XIXs.com >
 This file is distributed under the terms of the MIT license.
 http://en.wikipedia.org/wiki/MIT_License

Note that much of the documentation for the C functions exposed to Lua 
can be found in the associated box2d.lua file.

*/

#include <string.h>

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

// currently need mingw hax for windows, must also link with small c file
// https://github.com/jtsiomb/c11threads
#include "c11threads.h"

typedef struct wire_thread
{
	int handle;

	mtx_t mutex; // lock for this thread
} wire_thread ;

typedef struct wire_fifo
{
	int handle;


	mtx_t mutex; // lock for this fifo
} wire_fifo ;


static mtx_t wire_thread_mutex; // lock for threads array
static int wire_thread_avail=0;
static int wire_thread_count=0;
static wire_fifo *wire_threads=0;

static mtx_t wire_fifo_mutex; // lock for fifos array
static int wire_fifo_avail=0;
static int wire_fifo_count=0;
static wire_fifo *wire_fifos=0;

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

	lua_newtable(l);
	luaL_openlib(l, NULL, lib, 0);
	
	return 1;
}

