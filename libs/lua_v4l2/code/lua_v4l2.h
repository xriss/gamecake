/*
-- Copyright (C) 2012 Kriss Blank < Kriss@XIXs.com >
-- This file is distributed under the terms of the MIT license.
-- http://en.wikipedia.org/wiki/MIT_License
-- Please ping me if you use it for anything cool...
*/


#ifdef __cplusplus
extern "C" {
#endif

LUALIB_API int luaopen_wetgenes_v4l2_core (lua_State *l);

#ifdef __cplusplus
};
#endif


struct lua_v4l2_struct
{
	int fd;
	int buffer_count	;	// number of buffers
	int buffer_size;		// size of each buffer
	uint8_t *buffer_data;	// pointer to buffers
	int width;
	int height;
	int format;
};
