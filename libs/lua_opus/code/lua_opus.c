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

#include "../lua_pack/code/lua_pack.h"

#include "lua_opus.h"

#include "opus.h"
#include "speex/speex_echo.h"

typedef OpusEncoder    *opus_encoder_ptr;
typedef OpusDecoder    *opus_decoder_ptr;
typedef SpeexEchoState *opus_echo_ptr;

//
// we can use either these strings as a string identifier
// or the address as a light userdata identifier, both unique
//
const char *lua_opus_encoder_meta_name="opus_encoder*ptr";
const char *lua_opus_decoder_meta_name="opus_decoder*ptr";
const char *lua_opus_echo_meta_name="opus_echo*ptr";

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// get **ptr and error if it is not the right udata
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
opus_encoder_ptr * lua_opus_encoder_check_ptr (lua_State *l,int idx)
{
opus_encoder_ptr *ptr;
	ptr = (opus_encoder_ptr*)luaL_checkudata(l, idx , lua_opus_encoder_meta_name);
	return ptr;
}
opus_decoder_ptr * lua_opus_decoder_check_ptr (lua_State *l,int idx)
{
opus_decoder_ptr *ptr;
	ptr = (opus_decoder_ptr*)luaL_checkudata(l, idx , lua_opus_decoder_meta_name);
	return ptr;
}
opus_echo_ptr * lua_opus_echo_check_ptr (lua_State *l,int idx)
{
opus_echo_ptr *ptr;
	ptr = (opus_echo_ptr*)luaL_checkudata(l, idx , lua_opus_echo_meta_name);
	return ptr;
}
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// get *ptr and error if it is 0
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
opus_encoder_ptr lua_opus_encoder_check (lua_State *l,int idx)
{	
opus_encoder_ptr *ptr;
	ptr = lua_opus_encoder_check_ptr (l, idx);
	if(!*ptr)
	{
		luaL_error(l,"opus encoder ptr is null");
	}
	return *ptr;
}
opus_decoder_ptr lua_opus_decoder_check (lua_State *l,int idx)
{	
opus_decoder_ptr *ptr;
	ptr = lua_opus_decoder_check_ptr (l, idx);
	if(!*ptr)
	{
		luaL_error(l,"opus decoder ptr is null");
	}
	return *ptr;
}
opus_echo_ptr lua_opus_echo_check (lua_State *l,int idx)
{	
opus_echo_ptr *ptr;
	ptr = lua_opus_echo_check_ptr (l, idx);
	if(!*ptr)
	{
		luaL_error(l,"speex echo ptr is null");
	}
	return *ptr;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// free buffers
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_opus_encoder_destroy (lua_State *l)
{
opus_encoder_ptr *ptr;
	ptr = lua_opus_encoder_check_ptr (l, 1);
	if(*ptr)
	{
		opus_encoder_destroy(*ptr); (*ptr)=0;
	}

	return 0;
}
static int lua_opus_decoder_destroy (lua_State *l)
{
opus_decoder_ptr *ptr;
	ptr = lua_opus_decoder_check_ptr (l, 1);
	if(*ptr)
	{
		opus_decoder_destroy(*ptr); (*ptr)=0;
	}

	return 0;
}
static int lua_opus_echo_destroy (lua_State *l)
{
opus_echo_ptr *ptr;
	ptr = lua_opus_echo_check_ptr (l, 1);
	if(*ptr)
	{
		speex_echo_state_destroy(*ptr); (*ptr)=0;
	}

	return 0;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// alloc buffers
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_opus_encoder_create (lua_State *l)
{
	int freq=48000;
	int chan=1;
	int app=OPUS_APPLICATION_AUDIO;
	int bits=48000*8;
	int err=0;
	opus_encoder_ptr *ptr;

	if(lua_isnumber(l,1)) { freq=(int)lua_tonumber(l,1); }
	if(lua_isnumber(l,2)) { chan=(int)lua_tonumber(l,2); }
	if(lua_isnumber(l,3)) { app =(int)lua_tonumber(l,3); }
	if(lua_isnumber(l,4)) { bits=(int)lua_tonumber(l,4); }

// create an ptr userdata pointer pointer
	ptr = (opus_encoder_ptr*)lua_newuserdata(l, sizeof(opus_encoder_ptr*));	
	(*ptr)=0;
	luaL_getmetatable(l, lua_opus_encoder_meta_name);
	lua_setmetatable(l, -2);

//allocate and setup the base data
//	(*ptr)=calloc(sizeof(opusdat),1);
	(*ptr)=opus_encoder_create(freq,chan,app,&err);
	if((!(*ptr))||(err)) { return 0; }
	
	opus_encoder_ctl(*ptr, OPUS_SET_BITRATE(bits));

//return the userdata	
	return 1;
}
static int lua_opus_decoder_create (lua_State *l)
{
	int freq=48000;
	int chan=1;
	int err;
	opus_decoder_ptr *ptr;

	if(lua_isnumber(l,1)) { freq=(int)lua_tonumber(l,1); }
	if(lua_isnumber(l,2)) { chan=(int)lua_tonumber(l,2); }

// create an ptr userdata pointer pointer
	ptr = (opus_decoder_ptr*)lua_newuserdata(l, sizeof(opus_decoder_ptr*));	
	(*ptr)=0;
	luaL_getmetatable(l, lua_opus_decoder_meta_name);
	lua_setmetatable(l, -2);

//allocate and setup the base data
//	(*ptr)=calloc(sizeof(opusdat),1);
	(*ptr)=opus_decoder_create(freq,chan,&err);
	if((!(*ptr))||(err)) { return 0; }

//return the userdata	
	return 1;
}
static int lua_opus_echo_create (lua_State *l)
{
	int frames=20*48000/1000;
	int checks=100*48000/1000;
	int err;
	opus_echo_ptr *ptr;

	if(lua_isnumber(l,1)) { frames=(int)lua_tonumber(l,1); }
	if(lua_isnumber(l,2)) { checks=(int)lua_tonumber(l,2); }

// create an ptr userdata pointer pointer
	ptr = (opus_echo_ptr*)lua_newuserdata(l, sizeof(opus_echo_ptr*));	
	(*ptr)=0;
	luaL_getmetatable(l, lua_opus_echo_meta_name);
	lua_setmetatable(l, -2);

//allocate and setup the base data
	(*ptr)=speex_echo_state_init(frames,checks);
	if(!(*ptr)) { return 0; }

//return the userdata	
	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// encode opus
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_opus_encode (lua_State *l)
{
	opus_encoder_ptr ptr;
	
	const unsigned char *wav;
	size_t wav_len;
	unsigned char *dat;
	size_t dat_len;
	
	int ret=0;

	ptr=lua_opus_encoder_check(l,1);
	wav=lua_pack_to_const_buffer(l,2,&wav_len);
	dat=lua_pack_to_buffer(l,3,&dat_len);
	
	ret=opus_encode(ptr,(const opus_int16 *)wav,wav_len/2,dat,dat_len);

	lua_pushnumber(l,ret);
//return the actual size of data encoded
	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// decode opus
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_opus_decode (lua_State *l)
{
	int flag=0;
	opus_decoder_ptr ptr;
	
	unsigned char *wav;
	size_t wav_len;
	const unsigned char *dat;
	size_t dat_len;
	
	int ret=0;

	ptr=lua_opus_decoder_check(l,1);
	dat=lua_pack_to_const_buffer(l,2,&dat_len);
	wav=lua_pack_to_buffer(l,3,&wav_len);
	flag=lua_tonumber(l,4);

	ret=opus_decode(ptr,dat,dat_len,(opus_int16 *)wav,wav_len/2,flag);

	lua_pushnumber(l,ret);
//return the actual size of data encoded
	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// sample data to be ignored
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_opus_echo_playback (lua_State *l)
{
	opus_echo_ptr ptr;
	
	const unsigned char *wav;
	size_t wav_len;

	ptr=lua_opus_echo_check(l,1);
	wav=lua_pack_to_const_buffer(l,2,0);

	speex_echo_playback(ptr,(const spx_int16_t *)wav);

	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// remove previously set samples
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_opus_echo_capture (lua_State *l)
{
	opus_echo_ptr ptr;
	
	const unsigned char *wav_in;
	unsigned char *wav_out;

	ptr=lua_opus_echo_check(l,1);
	wav_in=lua_pack_to_const_buffer(l,2,0);
	wav_out=lua_pack_to_buffer(l,3,0);

	speex_echo_capture(ptr, (const spx_int16_t *)wav_in, (spx_int16_t *)wav_out);

	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// remove using these samples
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_opus_echo_cancel (lua_State *l)
{
	opus_echo_ptr ptr;
	
	const unsigned char *wav_in;
	const unsigned char *wav_echo;
	unsigned char *wav_out;

	ptr=lua_opus_echo_check(l,1);
	wav_in=lua_pack_to_const_buffer(l,2,0);
	wav_echo=lua_pack_to_const_buffer(l,3,0);
	wav_out=lua_pack_to_buffer(l,4,0);

	speex_echo_cancellation(ptr, (const spx_int16_t *)wav_in, (const spx_int16_t *)wav_echo, (spx_int16_t *)wav_out);

	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// open library.
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
LUALIB_API int luaopen_wetgenes_opus_core (lua_State *l)
{
	const luaL_Reg lib[] =
	{
		{"encoder_create",			lua_opus_encoder_create},
		{"encoder_destroy",			lua_opus_encoder_destroy},

		{"encode",					lua_opus_encode},

		{"decoder_create",			lua_opus_decoder_create},
		{"decoder_destroy",			lua_opus_decoder_destroy},

		{"decode",					lua_opus_decode},

		{"echo_create",				lua_opus_echo_create},
		{"echo_destroy",			lua_opus_echo_destroy},

		{"echo_cancel",				lua_opus_echo_cancel},

		{"echo_playback",			lua_opus_echo_playback},
		{"echo_capture",			lua_opus_echo_capture},


		{0,0}
	};
	const luaL_Reg encoder_meta[] =
	{
		{"__gc",			lua_opus_encoder_destroy},
		{0,0}
	};
	const luaL_Reg decoder_meta[] =
	{
		{"__gc",			lua_opus_decoder_destroy},
		{0,0}
	};
	const luaL_Reg echo_meta[] =
	{
		{"__gc",			lua_opus_echo_destroy},
		{0,0}
	};

	luaL_newmetatable(l, lua_opus_encoder_meta_name);
	luaL_openlib(l, NULL, encoder_meta, 0);
	lua_pop(l,1);

	luaL_newmetatable(l, lua_opus_decoder_meta_name);
	luaL_openlib(l, NULL, decoder_meta, 0);
	lua_pop(l,1);

	luaL_newmetatable(l, lua_opus_echo_meta_name);
	luaL_openlib(l, NULL, echo_meta, 0);
	lua_pop(l,1);

	lua_newtable(l);
	luaL_openlib(l, NULL, lib, 0);
	return 1;
}

