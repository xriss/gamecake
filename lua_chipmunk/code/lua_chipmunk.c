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
cpSpace **space;

// check if we are given a space, try and return the default static body from it
 	space=(cpSpace**)luaL_testudata(l, idx , lua_chipmunk_space_meta_name);
	if(space&&*space) { return cpSpaceGetStaticBody(*space); }

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
		body=lua_chipmunk_body_ptr(l,1);
		tp=luaL_checkstring(l,2);
		if(0==strcmp(tp,"circle"))
		{
			cr=luaL_checknumber(l,3);
			cx=luaL_checknumber(l,4);
			cy=luaL_checknumber(l,5);
			*pp=cpCircleShapeNew(body,cr,cpv(cx,cy));
		}
		else
		if(0==strcmp(tp,"segment"))
		{
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
// space step
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_chipmunk_space_step (lua_State *l)
{	
cpSpace *space=lua_chipmunk_space_ptr(l,1);

	cpSpaceStep(space, luaL_checknumber(l,2) );

	return 0;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// space add body
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_chipmunk_space_add_body (lua_State *l)
{	
cpSpace *space=lua_chipmunk_space_ptr(l,1);
cpBody  *body=lua_chipmunk_body_ptr(l,2);

	cpSpaceAddBody(space,body);

	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// space remove body
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_chipmunk_space_remove_body (lua_State *l)
{	
cpSpace *space=lua_chipmunk_space_ptr(l,1);
cpBody  *body=lua_chipmunk_body_ptr(l,2);

	cpSpaceRemoveBody(space,body);

	return 0;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// space contains body
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_chipmunk_space_contains_body (lua_State *l)
{	
cpSpace *space=lua_chipmunk_space_ptr(l,1);
cpBody  *body=lua_chipmunk_body_ptr(l,2);

	lua_pushboolean(l,cpSpaceContainsBody(space,body));

	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// body get/set position
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_chipmunk_body_position (lua_State *l)
{	
cpBody *body=lua_chipmunk_body_ptr(l,1);
cpVect v;

	if(lua_isnumber(l,2))
	{
		v.x=luaL_checknumber(l,2);
		v.y=luaL_checknumber(l,3);
		cpBodySetPosition(body, v );
	}
	
	v=cpBodyGetPosition(body);
	lua_pushnumber(l,v.x);
	lua_pushnumber(l,v.y);

	return 2;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// body get/set angle
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_chipmunk_body_angle (lua_State *l)
{	
cpBody *body=lua_chipmunk_body_ptr(l,1);

	if(lua_isnumber(l,2))
	{
		cpBodySetAngle(body, luaL_checknumber(l,2) );
	}
	
	lua_pushnumber(l, cpBodyGetAngle(body) );

	return 1;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// shape get/set elasticity
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_chipmunk_shape_elasticity (lua_State *l)
{	
cpShape *shape=lua_chipmunk_shape_ptr(l,1);

	if(lua_isnumber(l,2))
	{
		cpShapeSetElasticity(shape, luaL_checknumber(l,2) );
	}
	
	lua_pushnumber(l, cpShapeGetElasticity(shape) );

	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// shape get/set friction
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_chipmunk_shape_friction (lua_State *l)
{	
cpShape *shape=lua_chipmunk_shape_ptr(l,1);

	if(lua_isnumber(l,2))
	{
		cpShapeSetFriction(shape, luaL_checknumber(l,2) );
	}
	
	lua_pushnumber(l, cpShapeGetFriction(shape) );

	return 1;
}

/*+-----------------------------------------------------------------1------------------------------------------------+*/
//
// space add shape
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_chipmunk_space_add_shape (lua_State *l)
{	
cpSpace *space=lua_chipmunk_space_ptr(l,1);
cpShape *shape=lua_chipmunk_shape_ptr(l,2);

	cpSpaceAddShape(space,shape);

	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// space remove shape
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_chipmunk_space_remove_shape (lua_State *l)
{	
cpSpace *space=lua_chipmunk_space_ptr(l,1);
cpShape *shape=lua_chipmunk_shape_ptr(l,2);

	cpSpaceRemoveShape(space,shape);

	return 0;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// space contains shape
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_chipmunk_space_contains_shape (lua_State *l)
{	
cpSpace *space=lua_chipmunk_space_ptr(l,1);
cpShape *shape=lua_chipmunk_shape_ptr(l,2);

	lua_pushboolean(l,cpSpaceContainsShape(space,shape));

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
		{"space_create",					lua_chipmunk_space_create},
		{"space_destroy",					lua_chipmunk_space_destroy},
		
		{"body_create",						lua_chipmunk_body_create},
		{"body_destroy",					lua_chipmunk_body_destroy},

		{"shape_create",					lua_chipmunk_shape_create},
		{"shape_destroy",					lua_chipmunk_shape_destroy},

//		{"constraint_create",				lua_chipmunk_constraint_create},
//		{"constraint_destroy",				lua_chipmunk_constraint_destroy},

		{"space_add_body",					lua_chipmunk_space_add_body},
		{"space_remove_body",				lua_chipmunk_space_remove_body},
		{"space_contains_body",				lua_chipmunk_space_contains_body},

		{"space_add_shape",					lua_chipmunk_space_add_shape},
		{"space_remove_shape",				lua_chipmunk_space_remove_shape},
		{"space_contains_shape",			lua_chipmunk_space_contains_shape},
		
//		{"space_add_constraint",			lua_chipmunk_space_add_constraint},
//		{"space_remove_constraint",			lua_chipmunk_space_remove_constraint},
//		{"space_contains_constraint",		lua_chipmunk_space_contains_constraint},

//		{"space_reindex_shape",				lua_chipmunk_space_reindex_shape},
//		{"space_reindex_shapes_for_body",	lua_chipmunk_space_reindex_shapes_for_body},
//		{"space_reindex_static",			lua_chipmunk_space_reindex_static},
		{"space_step",						lua_chipmunk_space_step},

//getset
		{"space_iterations",				lua_chipmunk_space_iterations},
		{"space_gravity",					lua_chipmunk_space_gravity},
		{"space_damping",					lua_chipmunk_space_damping},
//		{"space_idle_speed_threshold",		lua_chipmunk_space_idle_speed_threshold},
//		{"space_sleep_time_threshold",		lua_chipmunk_space_sleep_time_threshold},
//		{"space_collision_slop",			lua_chipmunk_space_collision_slop},
//		{"space_collision_bias",			lua_chipmunk_space_collision_bias},
//		{"space_collision_persistence",		lua_chipmunk_space_collision_persistence},
//		{"space_user_data",					lua_chipmunk_space_user_data},
//		{"space_current_time_step",			lua_chipmunk_space_current_time_step},
//		{"space_locked",					lua_chipmunk_space_locked},

//		{"body_type",						lua_chipmunk_body_type},
//		{"body_space",						lua_chipmunk_body_space},
//		{"body_mass",						lua_chipmunk_body_mass},
//		{"body_moment",						lua_chipmunk_body_moment},
		{"body_position",					lua_chipmunk_body_position},
//		{"body_center_of_gravity",			lua_chipmunk_body_center_of_gravity},
//		{"body_velocity",					lua_chipmunk_body_velocity},
//		{"body_force",						lua_chipmunk_body_force},
		{"body_angle",						lua_chipmunk_body_angle},
//		{"body_angular_velocity",			lua_chipmunk_body_angular_velocity},
//		{"body_torque",						lua_chipmunk_body_torque},
//		{"body_user_data",					lua_chipmunk_body_user_data},

//		{"shape_body",						lua_chipmunk_shape_body},
//		{"shape_bounding_box",				lua_chipmunk_shape_bounding_box},
//		{"shape_sensor",					lua_chipmunk_shape_sensor},
		{"shape_elasticity",				lua_chipmunk_shape_elasticity},
		{"shape_friction",					lua_chipmunk_shape_friction},
//		{"shape_surface_velocity",			lua_chipmunk_shape_surface_velocity},
//		{"shape_collision_type",			lua_chipmunk_shape_collision_type},
//		{"shape_filter",					lua_chipmunk_shape_filter},
//		{"shape_space",						lua_chipmunk_shape_space},
//		{"shape_user_data",					lua_chipmunk_shape_user_data},



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

