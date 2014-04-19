/*
-- Copyright (C) 2014 Kriss Blank < Kriss@XIXs.com >
-- This file is distributed under the terms of the MIT license.
-- http://en.wikipedia.org/wiki/MIT_License
-- Please ping me if you use it for anything cool...
*/

#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#include "lua_kissfft.h"

// hax to be honest, all this to create a lua_toluserdata function
// if lua or luajit change then this will break
// it is however still better than not having any bounds checking

#if defined(LIB_LUAJIT)
#include "../../lib_luajit/src/lj_obj.h"
#else
#include "../wet/util/pstdint.h"
#include "../lib_lua/src/lobject.h"
#endif

#include "../../wet/util/wet_types.h"

static u8 * lua_toluserdata (lua_State *L, int idx, size_t *len) {

#if defined(LIB_LUAJIT)
	GCudata *g;
#else
	Udata *g;
#endif

	u8 *p=lua_touserdata(L,idx);
	
	if(!p) { return 0; }
	
	if(len)
	{
		if(lua_islightuserdata(L,idx))
		{
			*len=0x7fffffff;
		}
		else
		{
#if defined(LIB_LUAJIT)
			g=(GCudata*)(p-sizeof(GCudata));
			*len=g->len;
#else
			g=(Udata*)(p-sizeof(Udata));
			*len=g->uv.len;
#endif
		}	
	}
	
	return p;
}


//
// we can use either these strings as a string identifier
// or the address as a light userdata identifier, both unique
//
const char *lua_kissfft_meta_name="kissfft*ptr";

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// get **ptr and error if it is not the right udata
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
kissfftdat ** lua_kissfft_get_ptr (lua_State *l,int idx)
{
kissfftdat **ptr;
	ptr = (kissfftdat**)luaL_checkudata(l, idx , lua_kissfft_meta_name);
	return ptr;
}
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// get *ptr and error if it is 0
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
kissfftdat * lua_kissfft_check_dat (lua_State *l,int idx)
{	
kissfftdat **ptr;
	ptr = lua_kissfft_get_ptr (l, idx);
	if(!*ptr)
	{
		luaL_error(l,"kissfft ptr is null");
	}
	if(!(*ptr)->cfg)
	{
		luaL_error(l,"kissfft ptr cfg is null");
	}
	return *ptr;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// prepare fft buffers and setup state into a chunk of memory
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_kissfft_start (lua_State *l)
{
int	len;
int i;

// default of 1024
	len=(int)lua_tonumber(l,1);
	if(len<=0) { len=1024; }
	if(len>KISSFFTDAT_LEN_MAX) { len=KISSFFTDAT_LEN_MAX; }
	
	kissfftdat **ptr;

// create an ptr userdata pointer pointer
	ptr = (kissfftdat**)lua_newuserdata(l, sizeof(kissfftdat**));	
	(*ptr)=0;
	luaL_getmetatable(l, lua_kissfft_meta_name);
	lua_setmetatable(l, -2);

//allocate and setup the base data
	(*ptr)=calloc(sizeof(kissfftdat),1);
	if(!(*ptr)) { return 0; }
	
	(*ptr)->cfg=kiss_fftr_alloc(len,0,0,0);
	
	(*ptr)->len=len;
	(*ptr)->nfreqs=(len/2)+1;

//return the userdata	
	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// clear fft buffers ( reset average counts )
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_kissfft_reset (lua_State *l)
{
kissfftdat *dat = lua_kissfft_check_dat (l, 1);
int i;

	for (i=0;i<dat->nfreqs;i++)
	{
		dat->dot[i] = 0;
	}
	dat->count=0;

	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// free fft buffers
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_kissfft_clean (lua_State *l)
{
kissfftdat **ptr;
	ptr = lua_kissfft_get_ptr (l, 1);
	if(*ptr)
	{
		if((*ptr)->cfg)
		{
			free((*ptr)->cfg); (*ptr)->cfg=0;
		}
		free(*ptr); (*ptr)=0;
	}

	return 0;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// push some data into the fft
//
// buffer (a string or userdata) , type ("s16")
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_kissfft_push (lua_State *l)
{
const u8 *ptr=0;
size_t len;

kissfftdat *dat = lua_kissfft_check_dat (l, 1);

int i,j;

	if(lua_isstring(l,2))
	{
		ptr=(const u8*)lua_tolstring(l,2,&len);
	}
	else
	if(lua_islightuserdata(l,2))
	{
		ptr=lua_toluserdata(l,2,&len);
		len=lua_tonumber(l,3);
		if(len<=0)
		{
			lua_pushstring(l,"need a string to load packed data from");
			lua_error(l);
		}
	}
	else
	if(lua_isuserdata(l,2)) // must check for light first...
	{
		ptr=lua_toluserdata(l,2,&len);
	}
	else
	{
		lua_pushstring(l,"need a string to load packed data from");
		lua_error(l);
	}
	


	for(j=0;j<len/(2*dat->len);j++)
	{
		float ss=1.0f/32768.0f;
		for(i=0;i<dat->len;i++)
		{
			dat->din[i]=(float)(((const s16*)ptr)[i])*ss;
		}

		kiss_fftr(dat->cfg,dat->din,dat->tmp);
		
        for (i=0;i<dat->nfreqs;i++)
        {
            dat->dot[i] += dat->tmp[i].r * dat->tmp[i].r + dat->tmp[i].i * dat->tmp[i].i;
		}
		dat->count++;

		ptr+=(2*dat->len);
	}

	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// pull some data out of the fft
// returns a float buffer array (averaged of multiple input chunks)
// This is a light userdata as the data is held in the cfg buffer
// also returns the length in the second return
//
// use the pack library to read the (f32) numbers
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_kissfft_pull (lua_State *l)
{
kissfftdat *dat = lua_kissfft_check_dat (l, 1);
int i;

	if(	dat->count > 0 )
	{
		for (i=0;i<dat->nfreqs;i++)
		{
			dat->dot[i] /= dat->count;
		}
	}
	
	lua_pushlightuserdata(l,dat->dot);
	lua_pushnumber(l,dat->nfreqs);

	return 2;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// open library.
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
LUALIB_API int luaopen_kissfft_core (lua_State *l)
{
	const luaL_reg lib[] =
	{
		{"start",			lua_kissfft_start},
		{"reset",			lua_kissfft_reset},
		{"clean",			lua_kissfft_clean},

		{"push",			lua_kissfft_push},
		{"pull",			lua_kissfft_pull},

		{0,0}
	};
	const luaL_reg meta[] =
	{
		{"__gc",			lua_kissfft_clean},
		{0,0}
	};

	luaL_newmetatable(l, lua_kissfft_meta_name);
	luaL_openlib(l, NULL, meta, 0);
	lua_pop(l,1);

		
	lua_newtable(l);
	luaL_openlib(l, NULL, lib, 0);
	return 1;
}

