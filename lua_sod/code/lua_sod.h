/*
-- Copyright (C) 2012 Kriss Blank < Kriss@XIXs.com >
-- This file is distributed under the terms of the MIT license.
-- http://en.wikipedia.org/wiki/MIT_License
-- Please ping me if you use it for anything cool...
*/


#ifdef __cplusplus
extern "C" {
#endif

LUALIB_API int luaopen_wetgenes_sod_core (lua_State *l);

sod ** lua_sod_ptr   (lua_State *l,int idx);
sod *  lua_sod_check (lua_State *l,int idx);

#ifdef __cplusplus
};
#endif

