/*
-- Copyright (C) 2012 Kriss Blank < Kriss@XIXs.com >
-- This file is distributed under the terms of the MIT license.
-- http://en.wikipedia.org/wiki/MIT_License
-- Please ping me if you use it for anything cool...
*/

#ifdef __cplusplus
extern "C" {
#endif


typedef struct
{
	ogg_sync_state   oy; /* sync and verify incoming physical bitstream */
	ogg_stream_state os; /* take physical pages, weld into a logical
						  stream of packets */
	ogg_page         og; /* one Ogg bitstream page. Vorbis packets are inside */
	ogg_packet       op; /* one raw packet of data for decode */

	vorbis_info      vi; /* struct that stores all the static vorbis bitstream
						  settings */
	vorbis_comment   vc; /* struct that stores all the bitstream user comments */
	vorbis_dsp_state vd; /* central working state for the packet->PCM decoder */
	vorbis_block     vb; /* local working space for packet->PCM decode */

// statemachine flags
	int done_open;
	int done_init;
	int done_synthesis_init;
	int done_block_init;
	int done_stream_init;
	
	int result_pageout;
	int result_head;
	
	int convsize;

	char *buffer;
	int  bytes;

    int eos;

	const char *err;	// can be used to pass error strings back to other code

	ogg_int16_t convbuffer[4096];

} dogg;



LUALIB_API int luaopen_wetgenes_ogg (lua_State *l);

#ifdef __cplusplus
};
#endif

