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

#include "wet_types.h"

extern u8 * lua_toluserdata (lua_State *L, int idx, size_t *len);

static u8 * lua_toptr (lua_State *L, int idx, size_t *len) {
	u8 *p=(u8*)lua_tolstring(L,idx,len);
	if(!p)
	{
		if(lua_islightuserdata(L,idx))
		{
			p=lua_toluserdata(L,idx,len);
			if(len)
			{
				if(lua_isnumber(L,idx+1))
				{
					*len=(size_t)lua_tonumber(L,idx+1);
				}
			}
		}
		else
		{
			p=lua_toluserdata(L,idx,len);
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

	ptr=lua_toptr(l,2,&len);
	if(!ptr)
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
// allocate, fft , apply a filter, inverse fft and then replace input with result
//
// number of samples
// input sample buffer (s16)[num]
// input filter buffer (f32)[num/2+1]
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_kissfft_filter (lua_State *l)
{
int i;

int len;
int lenf;

u8 *ptr_io=0;
size_t len_io;

const u8 *ptr_f=0;
size_t len_f;

kissfftdat *dat;

float f;

// default of 1024
	len=(int)lua_tonumber(l,1);
	if(len<=0) { len=1024; }
	if(len>KISSFFTDAT_LEN_MAX) { len=KISSFFTDAT_LEN_MAX; }
	lenf=(len/2)+1;
	
	if(lua_istable(l,2)) // table in/out
	{
		ptr_io=0;
	}
	else // memory in/out
	{
		ptr_io=lua_toptr(l,2,&len_io);
		if(!ptr_io)
		{
			lua_pushstring(l,"need s16 input/output buffer");
			return lua_error(l);
		}
		if(len*2!=len_io)
		{
			lua_pushstring(l,"s16 input/output buffer size mismatch");
			return lua_error(l);
		}
	}

	ptr_f=lua_toptr(l,3,&len_f);
	if(!ptr_f)
	{
		lua_pushstring(l,"need f32 filter buffer");
		return lua_error(l);
	}
	if(lenf*4!=len_f)
	{
		lua_pushstring(l,"f32 filter buffer size mismatch");
		return lua_error(l);
	}
	
	dat=calloc(sizeof(kissfftdat),1);
	if(!dat) { return 0; }
	dat->len=len;
	dat->nfreqs=lenf;
	
	dat->cfg=kiss_fftr_alloc(len,0,0,0);
	if(!dat->cfg) { return 0; }

	if(ptr_io)
	{
		for(i=0;i<dat->len;i++) // input
		{
			dat->din[i]=(float)(((const s16*)ptr_io)[i])*(1.0f/32767.0f);
		}
	}
	else
	{
		for(i=1;i<=dat->len;i++)
		{
			lua_rawgeti(l,2,i);
			dat->din[i-1]=(float)lua_tonumber(l,-1);
			lua_pop(l,1);
		}
	}
	kiss_fftr(dat->cfg,dat->din,dat->tmp);

	free(dat->cfg);
	dat->cfg=kiss_fftr_alloc(len,1,0,0); // setup inverse
	if(!dat->cfg) { return 0; }

	for (i=0;i<dat->nfreqs;i++)
	{
		f=((const float *)ptr_f)[i]/((float)dat->len);
		dat->tmp[i].r *= f;
		dat->tmp[i].i *= f;
	}

	kiss_fftri(dat->cfg,dat->tmp,dat->din);

	if(ptr_io)
	{
		for(i=0;i<dat->len;i++) // output
		{
			float f=dat->din[i]*(32767.0f);
			if(f> 32767.0f) { f= 32767.0f; }
			if(f<-32767.0f) { f=-32767.0f; }
			((s16*)ptr_io)[i]=(s16)f;
		}
	}
	else
	{
		for(i=1;i<=dat->len;i++)
		{
			lua_pushnumber(l,dat->din[i-1]);
			lua_rawseti(l,2,i);
		}
	}

	free(dat->cfg);
	free(dat);

//return the in/out s16 buffer on success or nil on failure
	lua_pushvalue(l,2);
	return 1;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// open library.
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
LUALIB_API int luaopen_kissfft_core (lua_State *l)
{
	const luaL_Reg lib[] =
	{
		{"start",			lua_kissfft_start},
		{"reset",			lua_kissfft_reset},
		{"clean",			lua_kissfft_clean},

		{"push",			lua_kissfft_push},
		{"pull",			lua_kissfft_pull},

		{"filter",			lua_kissfft_filter},

		{0,0}
	};
	const luaL_Reg meta[] =
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

