/*
-- Copyright (C) 2012 Kriss Blank < Kriss@XIXs.com >
-- This file is distributed under the terms of the MIT license.
-- http://en.wikipedia.org/wiki/MIT_License
-- Please ping me if you use it for anything cool...
*/
#include "all.h"

#include "flite.h"

//extern cst_voice *register_cmu_us_slt();
extern cst_voice *register_cmu_us_kal();

static int init=0;
static cst_voice *voice=0;

// setup stuff if we must
static int lua_speak_setup (lua_State *l)
{
	if(!init)
	{
		flite_init();
	//	voice=register_cmu_us_slt();
		voice=register_cmu_us_kal();
		
		
		init=1;
	}
	
	return 0;
}


// render some text using the current voice
static int lua_speak_text (lua_State *l)
{
cst_wave *sound;
u16 *data;
const char *text;

	lua_speak_setup(l);

	text=luaL_checkstring(l,1);
	sound = flite_text_to_wave(text, voice);
	
	data = (u16*)lua_newuserdata(l,sound->num_samples*2);
	memcpy(data,sound->samples,sound->num_samples*2);
	lua_pushnumber(l,sound->num_samples*2);

	delete_wave(sound);

// return userdata,size
	return 2;
}

//        feat_set_float(voice->features,"int_f0_target_mean", pitch);
 //       feat_set_float(voice->features,"int_f0_target_stddev",variance);
  //      feat_set_float(voice->features,"duration_stretch",speed); 
  
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// set the current voice
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_speak_voice (lua_State *l)
{
	lua_speak_setup(l);

	return 0;
}
  
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// speach library.
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
LUALIB_API int luaopen_wetgenes_speak_core (lua_State *l)
{
	const luaL_reg lib[] =
	{
		{"test",			lua_speak_text},
		
		{"text",			lua_speak_text},
		{"voice",			lua_speak_voice},
		
		{0,0}
	};

	lua_newtable(l);
	luaL_openlib(l, NULL, lib, 0);
	return 1;
}

