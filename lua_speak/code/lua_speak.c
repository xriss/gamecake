/*
-- Copyright (C) 2012 Kriss Blank < Kriss@XIXs.com >
-- This file is distributed under the terms of the MIT license.
-- http://en.wikipedia.org/wiki/MIT_License
-- Please ping me if you use it for anything cool...
*/
#include "all.h"

#include "flite.h"

extern cst_voice *register_cmu_us_slt();

static int lua_speak_test (lua_State *l)
{
cst_wave *sound;
cst_voice *voice;
u16 *data;

const char *text;//="hello mr speak and spell";

	text=luaL_checkstring(l,1);

//	printf("%s\n",text);
//	printf("\n");

	flite_init();
	voice=register_cmu_us_slt();
	sound = flite_text_to_wave(text, voice);

	data = (u16*)lua_newuserdata(l,sound->num_samples*2);
	memcpy(data,sound->samples,sound->num_samples*2);
	lua_pushnumber(l,sound->num_samples*2);

	delete_wave(sound);

// return userdata and a size
	return 2;
}

//        feat_set_float(voice->features,"int_f0_target_mean", pitch);
 //       feat_set_float(voice->features,"int_f0_target_stddev",variance);
  //      feat_set_float(voice->features,"duration_stretch",speed); 
  
  
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// speach library.
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
LUALIB_API int luaopen_wetgenes_speak_core (lua_State *l)
{
	const luaL_reg lib[] =
	{
		{"test",			lua_speak_test},
		
		{0,0}
	};

	lua_newtable(l);
	luaL_openlib(l, NULL, lib, 0);
	return 1;
}

