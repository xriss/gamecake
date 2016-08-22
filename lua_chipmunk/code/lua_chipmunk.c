/*
-- Copyright (C) 2016 Kriss Blank < Kriss@XIXs.com >
-- This file is distributed under the terms of the MIT license.
-- http://en.wikipedia.org/wiki/MIT_License
-- Please ping me if you use it for anything cool...
*/
#include "all.h"

//
// we can use either this string as a string identifier
// or its address as a light userdata identifier, both unique
//
const char *lua_chipmunk_space_meta_name="chipmunk_space*ptr";
const char *lua_chipmunk_body_meta_name="chipmunk_body*ptr";
const char *lua_chipmunk_shape_meta_name="chipmunk_shape*ptr";



/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// space create/destroy
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
cpSpace ** lua_chipmunk_space_ptr_ptr (lua_State *l,int idx)
{
cpSpace **pp;
	pp=(cpSpace**)luaL_checkudata(l, idx , lua_chipmunk_space_meta_name);
	return pp;
}

cpSpace *  lua_chipmunk_space_ptr (lua_State *l,int idx)
{
cpSpace **pp;
	pp=lua_chipmunk_space_ptr_ptr(l,idx);
	if(!*pp) { luaL_error(l,"chipmunk space is null"); }
	return *pp;
}

static int lua_chipmunk_space_create (lua_State *l)
{	
	cpSpace **pp;

// create ptr ptr userdata
	pp=(cpSpace**)lua_newuserdata(l, sizeof(cpSpace*));
	(*pp)=0;
	luaL_getmetatable(l, lua_chipmunk_space_meta_name);
	lua_setmetatable(l, -2);

// allocate cpSpace
	*pp=cpSpaceNew();

	return 1;
}

static int lua_chipmunk_space_destroy (lua_State *l)
{	
cpSpace **pp=lua_chipmunk_space_ptr_ptr(l, 1 );
	if(*pp)
	{
		cpSpaceFree(*pp);
		(*pp)=0;
	}	
	return 0;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// body create/destroy
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
cpBody ** lua_chipmunk_body_ptr_ptr (lua_State *l,int idx)
{
cpBody **pp;
	pp=(cpBody**)luaL_checkudata(l, idx , lua_chipmunk_body_meta_name);
	return pp;
}

cpBody *  lua_chipmunk_body_ptr (lua_State *l,int idx)
{
cpBody **pp;
	pp=lua_chipmunk_body_ptr_ptr(l,idx);
	if(!*pp) { luaL_error(l,"chipmunk body is null"); }
	return *pp;
}

static int lua_chipmunk_body_create (lua_State *l)
{	
	cpBody **pp;

// create ptr ptr userdata
	pp=(cpBody**)lua_newuserdata(l, sizeof(cpBody*));
	(*pp)=0;
	luaL_getmetatable(l, lua_chipmunk_body_meta_name);
	lua_setmetatable(l, -2);

// allocate cpBody
//	*pp=cpBodyNew();

	return 1;
}

static int lua_chipmunk_body_destroy (lua_State *l)
{	
cpBody **pp=lua_chipmunk_body_ptr_ptr(l, 1 );
	if(*pp)
	{
		cpBodyFree(*pp);
		(*pp)=0;
	}	
	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// shape create/destroy
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
cpShape ** lua_chipmunk_shape_ptr_ptr (lua_State *l,int idx)
{
cpShape **pp;
	pp=(cpShape**)luaL_checkudata(l, idx , lua_chipmunk_shape_meta_name);
	return pp;
}

cpShape *  lua_chipmunk_shape_ptr (lua_State *l,int idx)
{
cpShape **pp;
	pp=lua_chipmunk_shape_ptr_ptr(l,idx);
	if(!*pp) { luaL_error(l,"chipmunk shape is null"); }
	return *pp;
}

static int lua_chipmunk_shape_create (lua_State *l)
{	
	cpShape **pp;

// create ptr ptr userdata
	pp=(cpShape**)lua_newuserdata(l, sizeof(cpShape*));
	(*pp)=0;
	luaL_getmetatable(l, lua_chipmunk_shape_meta_name);
	lua_setmetatable(l, -2);

// allocate cpShape
//	*pp=cpShapeNew();

	return 1;
}

static int lua_chipmunk_shape_destroy (lua_State *l)
{	
cpShape **pp=lua_chipmunk_shape_ptr_ptr(l, 1 );
	if(*pp)
	{
		cpShapeFree(*pp);
		(*pp)=0;
	}	
	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// open library.
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
LUALIB_API int luaopen_wetgenes_chipmunk_core (lua_State *l)
{
	const luaL_reg lib[] =
	{
		{"space_create",			lua_chipmunk_space_create},
		{"space_destroy",			lua_chipmunk_space_destroy},
		
		{"body_create",				lua_chipmunk_body_create},
		{"body_destroy",			lua_chipmunk_body_destroy},

		{"shape_create",			lua_chipmunk_body_create},
		{"shape_destroy",			lua_chipmunk_body_destroy},

		{0,0}
	};

	const luaL_reg meta_space[] =
	{
		{"__gc",			lua_chipmunk_space_destroy},
		{0,0}
	};

	const luaL_reg meta_body[] =
	{
		{"__gc",			lua_chipmunk_body_destroy},
		{0,0}
	};

	const luaL_reg meta_shape[] =
	{
		{"__gc",			lua_chipmunk_shape_destroy},
		{0,0}
	};

	luaL_newmetatable(l, lua_chipmunk_space_meta_name);
	luaL_openlib(l, NULL, meta_space, 0);
	lua_pop(l,1);

	luaL_newmetatable(l, lua_chipmunk_body_meta_name);
	luaL_openlib(l, NULL, meta_body, 0);
	lua_pop(l,1);
		
	luaL_newmetatable(l, lua_chipmunk_shape_meta_name);
	luaL_openlib(l, NULL, meta_shape, 0);
	lua_pop(l,1);

	lua_newtable(l);
	luaL_openlib(l, NULL, lib, 0);
	return 1;
}

