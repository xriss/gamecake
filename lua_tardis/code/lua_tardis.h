/*
-- Copyright (C) 2012 Kriss Blank < Kriss@XIXs.com >
-- This file is distributed under the terms of the MIT license.
-- http://en.wikipedia.org/wiki/MIT_License
-- Please ping me if you use it for anything cool...
--
-- Time And Relative Dimensions In Space
--
-- Best, 3dmathlib, name, ever.
--
*/


#ifdef __cplusplus
extern "C" {
#endif

unsigned char * lua_tardis_uda (lua_State *l, int idx);
unsigned int lua_tardis_uda_count (lua_State *l, int idx);


LUALIB_API int luaopen_wetgenes_tardis_core (lua_State *l);

#ifdef __cplusplus
};
#endif

