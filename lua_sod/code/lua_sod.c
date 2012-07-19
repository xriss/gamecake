/*
-- Copyright (C) 2012 Kriss Blank < Kriss@XIXs.com >
-- This file is distributed under the terms of the MIT license.
-- http://en.wikipedia.org/wiki/MIT_License
-- Please ping me if you use it for anything cool...
*/
#include "all.h"

//
// we can use either this string as a string identifier
// or its address as a light userdata identifier, both unique
//
const char *lua_sod_meta_name="sod*ptr";


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// create
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_sod_create(lua_State *l)
{
	sod **sd;

// create a sod userdata pointer pointer
	sd = (sod**)lua_newuserdata(l, sizeof(sod**));
	(*sd)=0;
	luaL_getmetatable(l, lua_sod_meta_name);
	lua_setmetatable(l, -2);

//open the actual sod
	(*sd)=sod_alloc();
	if(!(*sd)) { return 0; } // error failed to alloc

//return the userdata	
	return 1;
}
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// get **ptr and error if it is not the right udata
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
sod ** lua_sod_ptr (lua_State *l,int idx)
{
sod **sd;
	sd = (sod**)luaL_checkudata(l, idx , lua_sod_meta_name);
	return sd;
}
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// get *ptr and error if it is 0
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
sod * lua_sod_check (lua_State *l,int idx)
{	
sod **sd;
	sd = lua_sod_ptr (l,idx);
	if(!*sd)
	{
		luaL_error(l,"sod is null");
	}
	return *sd;
}
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// __GC for ptr (may be null)
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_sod_destroy (lua_State *l)
{	
sod **sd;

	sd = lua_sod_ptr(l, 1 );
	
	if(*sd)
	{
		sod_free(*sd);
		(*sd)=0;
	}
	
	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// fill a table with info
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_sod_info (lua_State *l)
{	
sod *sd;
const char *s;

	sd = lua_sod_check(l, 1 );
	if(lua_istable(l,2)) // reuse
	{
		lua_pushvalue(l,2);
	}
	else // new
	{
		lua_newtable(l);
	}
	
	lua_pushstring(l,"fmt");			lua_pushnumber(l,sd->fmt);			lua_settable(l,-3);	
	lua_pushstring(l,"sample_size");	lua_pushnumber(l,sd->sample_size);	lua_settable(l,-3);
	lua_pushstring(l,"samples");		lua_pushnumber(l,sd->samples);		lua_settable(l,-3);
	lua_pushstring(l,"chanels");		lua_pushnumber(l,sd->chanels);		lua_settable(l,-3);
	lua_pushstring(l,"data");			lua_pushlightuserdata(l,sd->data);	lua_settable(l,-3);
	lua_pushstring(l,"data_sizeof");	lua_pushnumber(l,sd->data_sizeof);	lua_settable(l,-3);
	lua_pushstring(l,"freq");			lua_pushnumber(l,sd->freq);			lua_settable(l,-3);

	if(sd->err)
	{
		lua_pushstring(l,"err"); lua_pushstring(l,sd->err); lua_settable(l,-3);
	}
	else
	{
		lua_pushstring(l,"err"); lua_pushnil(l); lua_settable(l,-3);
	}

	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// load some sounds
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_sod_load_file (lua_State *l)
{	
sod *sd;
const char *s;

	sd = lua_sod_check(l, 1 );
	s = luaL_checkstring(l, 2 );

	sod_load_file(sd,s,0);

	if(sd->err)	{ lua_pushnil(l); lua_pushstring(l,sd->err); return 2; } // nil,err on failure
	lua_pushvalue(l,1); return 1; // return the sod on success
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// load some sounds
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_sod_load_data (lua_State *l)
{	
sod *sd;
const unsigned char *s;
size_t len;

	sd = lua_sod_check(l, 1 );
	s =(const unsigned char *) luaL_checklstring(l, 2 ,&len);

	sod_load_data(sd,s,(int)len,0);

	if(sd->err)	{ lua_pushnil(l); lua_pushstring(l,sd->err); return 2; } // nil,err on failure
	lua_pushvalue(l,1); return 1; // return the sod on success
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// open library.
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
LUALIB_API int luaopen_wetgenes_sod_core (lua_State *l)
{
	const luaL_reg lib[] =
	{
		{"create",			lua_sod_create},
		{"destroy",			lua_sod_destroy},

		{"info",			lua_sod_info},
		
		{"load_data",		lua_sod_load_data},
		{"load_file",		lua_sod_load_file},
//		{"save",			lua_sod_save},
		
		{0,0}
	};
	const luaL_reg meta_sod[] =
	{
		{"__gc",			lua_sod_destroy},
		{0,0}
	};


	luaL_newmetatable(l, lua_sod_meta_name);
	luaL_openlib(l, NULL, meta_sod, 0);
	lua_pop(l,1);
		
	lua_newtable(l);
	luaL_openlib(l, NULL, lib, 0);
	return 1;
}

