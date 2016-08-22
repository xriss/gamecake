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
const char *lua_chipmunk_body_meta_name ="chipmunk_body*ptr";
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
const char *tp;
double bm,bi;

// create ptr ptr userdata
	pp=(cpBody**)lua_newuserdata(l, sizeof(cpBody*));
	(*pp)=0;
	luaL_getmetatable(l, lua_chipmunk_body_meta_name);
	lua_setmetatable(l, -2);

// allocate cpBody
	if(lua_isnumber(l,1)) // dynamic if given mass and inertia
	{
		bm=luaL_checknumber(l,1);
		bi=luaL_checknumber(l,2);
		*pp=cpBodyNew(bm,bi);
	}
	else
	{
		tp=luaL_checkstring(l,1);
		if(0==strcmp(tp,"kinematic"))
		{
			*pp=cpBodyNewKinematic();
		}
		else
		if(0==strcmp(tp,"static"))
		{
			*pp=cpBodyNewStatic();
		}
		else
		{
			lua_pushstring(l,"unknown body type"); lua_error(l);
		}
	}

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
const char *tp;
cpShape **pp;
cpBody *body;
double cr,cx,cy;
double sr,sxa,sya,sxb,syb;
cpBB bb;
double br;
// create ptr ptr userdata
	pp=(cpShape**)lua_newuserdata(l, sizeof(cpShape*));
	(*pp)=0;
	luaL_getmetatable(l, lua_chipmunk_shape_meta_name);
	lua_setmetatable(l, -2);

// allocate cpShape
		tp=luaL_checkstring(l,1);
		if(0==strcmp(tp,"circle"))
		{
			body=lua_chipmunk_body_ptr(l,2);
			cr=luaL_checknumber(l,3);
			cx=luaL_checknumber(l,4);
			cy=luaL_checknumber(l,5);
			*pp=cpCircleShapeNew(body,cr,cpv(cx,cy));
		}
		else
		if(0==strcmp(tp,"segment"))
		{
			body=lua_chipmunk_body_ptr(l,2);
			sxa=luaL_checknumber(l,3);
			sya=luaL_checknumber(l,4);
			sxb=luaL_checknumber(l,5);
			syb=luaL_checknumber(l,6);
			sr=luaL_checknumber(l,7);
			*pp=cpSegmentShapeNew(body,cpv(sxa,sya),cpv(sxb,syb),sr);
		}
//		else
//		if(0==strcmp(tp,"poly"))
//		{
//			*pp=cpPolyShapeNew();
//		}
		else
		if(0==strcmp(tp,"box"))
		{
			body=lua_chipmunk_body_ptr(l,2);
			bb.l=luaL_checknumber(l,3);
			bb.b=luaL_checknumber(l,4);
			bb.r=luaL_checknumber(l,5);
			bb.t=luaL_checknumber(l,6);
			br=luaL_checknumber(l,7);
			*pp=cpBoxShapeNew2(body,bb,br);
		}
		else
		{
			lua_pushstring(l,"unknown shape type"); lua_error(l);
		}

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
// space get/set iterations
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_chipmunk_space_iterations (lua_State *l)
{	
cpSpace *space=lua_chipmunk_space_ptr(l,1);

	if(lua_isnumber(l,2))
	{
		cpSpaceSetIterations(space,(int) luaL_checknumber(l,2) );
	}
	
	lua_pushnumber(l, cpSpaceGetIterations(space) );

	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// space get/set gravity
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_chipmunk_space_gravity (lua_State *l)
{	
cpSpace *space=lua_chipmunk_space_ptr(l,1);
cpVect v;

	if(lua_isnumber(l,2))
	{
		v.x=luaL_checknumber(l,2);
		v.y=luaL_checknumber(l,3);
		cpSpaceSetGravity(space,v);
	}
	
	v=cpSpaceGetGravity(space);
	lua_pushnumber(l,v.x);
	lua_pushnumber(l,v.y);

	return 2;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// space get/set damping
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_chipmunk_space_damping (lua_State *l)
{	
cpSpace *space=lua_chipmunk_space_ptr(l,1);

	if(lua_isnumber(l,2))
	{
		cpSpaceSetDamping(space, luaL_checknumber(l,2) );
	}
	
	lua_pushnumber(l, cpSpaceGetDamping(space) );

	return 1;
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
		
		{"space_iterations",		lua_chipmunk_space_iterations},
		{"space_gravity",			lua_chipmunk_space_gravity},
		{"space_damping",			lua_chipmunk_space_damping},

		{0,0}
	};

	const luaL_reg meta_space[] =
	{
		{"iterations",		lua_chipmunk_space_iterations},
		{"gravity",			lua_chipmunk_space_gravity},
		{"damping",			lua_chipmunk_space_damping},
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

