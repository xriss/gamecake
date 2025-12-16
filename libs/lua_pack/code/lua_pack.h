/*
-- Copyright (C) 2012 Kriss Blank < Kriss@XIXs.com >
-- This file is distributed under the terms of the MIT license.
-- http://en.wikipedia.org/wiki/MIT_License
-- Please ping me if you use it for anything cool...
*/


#ifdef __cplusplus
extern "C" {
#endif

// these are smarter toluserdata functions that work with more possible input types
// a buffer has a length, it is either a userdata or a string, only userdata can be written to
// a table such as {buffer=data,offset=1024,sizeof=1024} can also be used to describe a buffer from a bigger buffer
// these will return 0 if given invalid inputs
extern       unsigned char * lua_pack_to_buffer       (lua_State *l, int idx, size_t *len);
extern const unsigned char * lua_pack_to_const_buffer (lua_State *l, int idx, size_t *len);

LUALIB_API int luaopen_wetgenes_pack (lua_State *l);

#ifdef __cplusplus
};
#endif

