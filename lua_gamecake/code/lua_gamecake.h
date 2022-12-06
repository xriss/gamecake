/*
-- Copyright (C) 2012 Kriss Blank < Kriss@XIXs.com >
-- This file is distributed under the terms of the MIT license.
-- http://en.wikipedia.org/wiki/MIT_License
-- Please ping me if you use it for anything cool...
*/


#ifdef __cplusplus
extern "C" {
#endif


struct gamecake_fontdata_char
{
	int image;
	float add;
	float x;
	float y;
	float w;
	float h;
	float u1;
	float v1;
	float u2;
	float v2;
};

struct gamecake_fontdata
{
	float size;
	struct gamecake_fontdata_char chars[256];
};
				
struct gamecake_canvas_font
{
	struct gamecake_fontdata *fontdata;
	float x;
	float y;
	float size;
	float add;
};
				

LUALIB_API int luaopen_wetgenes_gamecake_core (lua_State *l);

#ifdef __cplusplus
};
#endif

