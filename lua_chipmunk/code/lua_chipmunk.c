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
const char *lua_chipmunk_constraint_meta_name="chipmunk_constraint*ptr";



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

// use registry so we can find the space table from space ptr,
// this has the side effect that space MUST be destroyed,
// it will not be GCd as this will keep it alive.
	lua_pushlightuserdata(l,*pp);
	lua_pushvalue(l,1); // this will be the lua space table
	lua_settable(l,LUA_REGISTRYINDEX);


	return 1;
}

static int lua_chipmunk_space_destroy (lua_State *l)
{	
cpSpace **pp=lua_chipmunk_space_ptr_ptr(l, 1 );
	if(*pp)
	{
// remove registry link
		lua_pushlightuserdata(l,*pp);
		lua_pushnil(l);
		lua_settable(l,LUA_REGISTRYINDEX);
		
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

static int lua_chipmunk_body_lookup (lua_State *l)
{	
cpBody *p=lua_chipmunk_body_ptr(l, 1 );
	if( lua_isboolean(l,3) ) // forget
	{
		lua_pushlightuserdata(l,p);
		lua_pushnil(l);
		lua_settable(l,2);		
	}
	else
	if( lua_istable(l,3) ) // remember
	{
		lua_pushlightuserdata(l,p);
		lua_pushvalue(l,3);
		lua_settable(l,2);
	}

	lua_pushlightuserdata(l,p);
	lua_gettable(l,2);

	return 1;
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

static int lua_chipmunk_shape_lookup (lua_State *l)
{	
cpShape *p=lua_chipmunk_shape_ptr(l, 1 );
	if( lua_isboolean(l,3) ) // forget
	{
		lua_pushlightuserdata(l,p);
		lua_pushnil(l);
		lua_settable(l,2);		
	}
	else
	if( lua_istable(l,3) ) // remember
	{
		lua_pushlightuserdata(l,p);
		lua_pushvalue(l,3);
		lua_settable(l,2);
	}

	lua_pushlightuserdata(l,p);
	lua_gettable(l,2);

	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// constraint create/destroy
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
cpConstraint ** lua_chipmunk_constraint_ptr_ptr (lua_State *l,int idx)
{
cpConstraint **pp;
	pp=(cpConstraint**)luaL_checkudata(l, idx , lua_chipmunk_constraint_meta_name);
	return pp;
}

cpConstraint *  lua_chipmunk_constraint_ptr (lua_State *l,int idx)
{
cpConstraint **pp;
	pp=lua_chipmunk_constraint_ptr_ptr(l,idx);
	if(!*pp) { luaL_error(l,"chipmunk constraint is null"); }
	return *pp;
}

static int lua_chipmunk_constraint_create (lua_State *l)
{
const char *tp;
cpConstraint **pp;
cpBody *abody;
cpBody *bbody;
double ax,ay,bx,by,cx,cy;
double fa,fl,fh,fs,fd,fp,fr;
// create ptr ptr userdata
	pp=(cpConstraint**)lua_newuserdata(l, sizeof(cpConstraint*));
	(*pp)=0;
	luaL_getmetatable(l, lua_chipmunk_constraint_meta_name);
	lua_setmetatable(l, -2);

// allocate cpConstraint
		abody=lua_chipmunk_body_ptr(l,1);
		bbody=lua_chipmunk_body_ptr(l,2);
		tp=luaL_checkstring(l,3);
		if(0==strcmp(tp,"pin_joint"))
		{
			ax=luaL_checknumber(l,4);
			ay=luaL_checknumber(l,5);
			bx=luaL_checknumber(l,6);
			by=luaL_checknumber(l,7);
			*pp=cpPinJointNew(abody,bbody,cpv(ax,ay),cpv(bx,by));
		}
		else
		if(0==strcmp(tp,"slide_joint"))
		{
			ax=luaL_checknumber(l,4);
			ay=luaL_checknumber(l,5);
			bx=luaL_checknumber(l,6);
			by=luaL_checknumber(l,7);
			fl=luaL_checknumber(l,8);
			fh=luaL_checknumber(l,9);
			*pp=cpSlideJointNew(abody,bbody,cpv(ax,ay),cpv(bx,by),fl,fh);
		}
		else
		if(0==strcmp(tp,"pivot_joint"))
		{
			ax=luaL_checknumber(l,4);
			ay=luaL_checknumber(l,5);
			if(lua_isnumber(l,6)) // we have two local space pos
			{
				bx=luaL_checknumber(l,6);
				by=luaL_checknumber(l,7);
				*pp=cpPivotJointNew2(abody,bbody,cpv(ax,ay),cpv(bx,by));
			}
			else // we have 1 world space pos (will be converted to local for both both bodies)
			{
				*pp=cpPivotJointNew(abody,bbody,cpv(ax,ay));
			}
		}
		else
		if(0==strcmp(tp,"groove_joint"))
		{
			ax=luaL_checknumber(l,4);
			ay=luaL_checknumber(l,5);
			bx=luaL_checknumber(l,6);
			by=luaL_checknumber(l,7);
			cx=luaL_checknumber(l,8);
			cy=luaL_checknumber(l,9);
			*pp=cpGrooveJointNew(abody,bbody,cpv(ax,ay),cpv(bx,by),cpv(cx,cy));
		}
		else
		if(0==strcmp(tp,"damped_spring"))
		{
			ax=luaL_checknumber(l,4);
			ay=luaL_checknumber(l,5);
			bx=luaL_checknumber(l,6);
			by=luaL_checknumber(l,7);
			fl=luaL_checknumber(l,8);
			fs=luaL_checknumber(l,9);
			fd=luaL_checknumber(l,10);
			*pp=cpDampedSpringNew(abody,bbody,cpv(ax,ay),cpv(bx,by),fl,fs,fd);
		}
		else
		if(0==strcmp(tp,"damped_rotary_spring"))
		{
			fa=luaL_checknumber(l,4);
			fs=luaL_checknumber(l,5);
			fd=luaL_checknumber(l,6);
			*pp=cpDampedRotarySpringNew(abody,bbody,fa,fs,fd);
		}
		else
		if(0==strcmp(tp,"rotary_limit_joint"))
		{
			fl=luaL_checknumber(l,4);
			fh=luaL_checknumber(l,5);
			*pp=cpRotaryLimitJointNew(abody,bbody,fl,fh);
		}
		else
		if(0==strcmp(tp,"ratchet_joint"))
		{
			fp=luaL_checknumber(l,4);
			fr=luaL_checknumber(l,5);
			*pp=cpRatchetJointNew(abody,bbody,fp,fr);
		}
		else
		if(0==strcmp(tp,"gear_joint"))
		{
			fp=luaL_checknumber(l,4);
			fr=luaL_checknumber(l,5);
			*pp=cpGearJointNew(abody,bbody,fp,fr);
		}
		else
		if(0==strcmp(tp,"simple_motor"))
		{
			fr=luaL_checknumber(l,4);
			*pp=cpSimpleMotorNew(abody,bbody,fr);
		}
		else
		{
			lua_pushstring(l,"unknown constraint type"); lua_error(l);
		}

	return 1;
}

static int lua_chipmunk_constraint_destroy (lua_State *l)
{	
cpConstraint **pp=lua_chipmunk_constraint_ptr_ptr(l, 1 );
	if(*pp)
	{
		cpConstraintFree(*pp);
		(*pp)=0;
	}	
	return 0;
}

static int lua_chipmunk_constraint_lookup (lua_State *l)
{
cpConstraint *p=lua_chipmunk_constraint_ptr(l, 1 );
	if( lua_isboolean(l,3) ) // forget
	{
		lua_pushlightuserdata(l,p);
		lua_pushnil(l);
		lua_settable(l,2);		
	}
	else
	if( lua_istable(l,3) ) // remember
	{
		lua_pushlightuserdata(l,p);
		lua_pushvalue(l,3);
		lua_settable(l,2);
	}

	lua_pushlightuserdata(l,p);
	lua_gettable(l,2);

	return 1;
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
// space get/set collision slop
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_chipmunk_space_collision_slop (lua_State *l)
{	
cpSpace *space=lua_chipmunk_space_ptr(l,1);

	if(lua_isnumber(l,2))
	{
		cpSpaceSetCollisionSlop(space, luaL_checknumber(l,2) );
	}
	
	lua_pushnumber(l, cpSpaceGetCollisionSlop(space) );

	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// space get/set collision bias
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_chipmunk_space_collision_bias (lua_State *l)
{	
cpSpace *space=lua_chipmunk_space_ptr(l,1);

	if(lua_isnumber(l,2))
	{
		cpSpaceSetCollisionBias(space, luaL_checknumber(l,2) );
	}
	
	lua_pushnumber(l, cpSpaceGetCollisionBias(space) );

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
// space callbacks
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static void lua_chipmunk_space_callback_all(cpArbiter *arb, cpSpace *space, cpDataPointer data)
{
lua_State *l=*(lua_State **)data;
CP_ARBITER_GET_SHAPES(arb, a, b);

// get space table
	lua_pushlightuserdata(l,space);
	lua_gettable(l,LUA_REGISTRYINDEX);

//get space.shapes
	lua_getfield(l,-1,"shapes");
	lua_pushlightuserdata(l,a);
	lua_gettable(l,-2);
	lua_pushlightuserdata(l,b);
	lua_gettable(l,-3);

//get space.callbacks
	lua_getfield(l,-4,"callbacks");

//get space.callbacks[*]
	lua_pushlightuserdata(l,data);
	lua_gettable(l,-2);

//set shapes
	lua_pushvalue(l,-4);
	lua_setfield(l,-2,"shape_a");
	lua_pushvalue(l,-3);
	lua_setfield(l,-2,"shape_b");	

// set arbiter pointer
	lua_pushlightuserdata(l,arb);
	lua_rawseti(l,-2,0);

// 6 values are now on the stack
}

static cpBool lua_chipmunk_space_callback_begin(cpArbiter *arb, cpSpace *space, cpDataPointer data)
{
lua_State *l=*(lua_State **)data;
cpBool r;

	lua_chipmunk_space_callback_all(arb,space,data);

//get function to call
	lua_getfield(l,-1,"begin");
	lua_pushvalue(l,-2);
	lua_call(l,1,1);
	r=lua_toboolean(l,-1)?cpTrue:cpFalse;

// pop bool , tab , space.callbacks , shape_a , shape_b , shapes , space
	lua_pop(l,7);
	
	return r;
}

static cpBool lua_chipmunk_space_callback_presolve(cpArbiter *arb, cpSpace *space, cpDataPointer data)
{
lua_State *l=*(lua_State **)data;
cpBool r;

	lua_chipmunk_space_callback_all(arb,space,data);

//get function to call
	lua_getfield(l,-1,"presolve");
	lua_pushvalue(l,-2);
	lua_call(l,1,1);
	r=lua_toboolean(l,-1)?cpTrue:cpFalse;

// pop bool , tab , space.callbacks , shape_a , shape_b , shapes , space
	lua_pop(l,7);
	
	return r;
}

static void lua_chipmunk_space_callback_postsolve(cpArbiter *arb, cpSpace *space, cpDataPointer data)
{
lua_State *l=*(lua_State **)data;

	lua_chipmunk_space_callback_all(arb,space,data);

//get function to call
	lua_getfield(l,-1,"postsolve");
	lua_pushvalue(l,-2);
	lua_call(l,1,0);

// pop tab , space.callbacks , shape_a , shape_b , shapes , space
	lua_pop(l,6);
}

static void lua_chipmunk_space_callback_separate(cpArbiter *arb, cpSpace *space, cpDataPointer data)
{
lua_State *l=*(lua_State **)data;

	lua_chipmunk_space_callback_all(arb,space,data);

//get function to call
	lua_getfield(l,-1,"separate");
	lua_pushvalue(l,-2);
	lua_call(l,1,0);

// pop tab , space.callbacks , shape_a , shape_b , shapes , space
	lua_pop(l,6);
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// space add handler, this will setup collision callbacks for the given types
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_chipmunk_space_add_handler (lua_State *l)
{	
lua_State **ll;
cpSpace *space=lua_chipmunk_space_ptr(l,1);
cpCollisionHandler *handler=0;
	if( lua_isnumber(l,3) && lua_isnumber(l,4) ) // specific
	{
		handler=cpSpaceAddCollisionHandler(space,lua_tonumber(l,3),lua_tonumber(l,4));
	}
	else
	if( lua_isnumber(l,3) ) // wildcard
	{
		handler=cpSpaceAddWildcardHandler(space,lua_tonumber(l,3));
	}
	else // global
	{
		handler=cpSpaceAddDefaultCollisionHandler(space);
	}
	
	lua_getfield(l,2,"begin");
	if(!lua_isnil(l,-1))
	{
		handler->beginFunc=lua_chipmunk_space_callback_begin;
	}
	lua_pop(l,1);

	lua_getfield(l,2,"presolve");
	if(!lua_isnil(l,-1))
	{
		handler->preSolveFunc=lua_chipmunk_space_callback_presolve;
	}
	lua_pop(l,1);

	lua_getfield(l,2,"postsolve");
	if(!lua_isnil(l,-1))
	{
		handler->postSolveFunc=lua_chipmunk_space_callback_postsolve;
	}
	lua_pop(l,1);
		
	lua_getfield(l,2,"separate");
	if(!lua_isnil(l,-1))
	{
		handler->separateFunc=lua_chipmunk_space_callback_separate;
	}
	lua_pop(l,1);

	ll=(lua_State**)lua_newuserdata(l, sizeof(lua_State*));
	*ll=l;
	lua_pushvalue(l,-1); // table[userdata]=userdata as all want to do is keep a reference
	lua_settable(l,2); // keep it alive by putting it in the table
	handler->userData=ll; // this is a unique value and a way for us to get the lua state in the callback function

// get space table
	lua_pushlightuserdata(l,space);
	lua_gettable(l,LUA_REGISTRYINDEX);

//get space.callbacks
	lua_getfield(l,-1,"callbacks");

//remember callback function in space.callbacks
	lua_pushlightuserdata(l,ll);
	lua_pushvalue(l,2); // store the table, containing the newly allocated userdata and functions to call for each callback
	lua_settable(l,-3);

// pop space.callbacks , space
	lua_pop(l,2);

	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// arbiter get/set points ( any table of values passed in will be returned )
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_chipmunk_arbiter_points (lua_State *l)
{	
cpArbiter *arbiter=(cpArbiter *)lua_touserdata(l,1); if(!arbiter){ lua_pushstring(l,"missing arbiter"); lua_error(l); }
cpVect v;

	cpContactPointSet set = cpArbiterGetContactPointSet(arbiter);

	if(lua_istable(l,2))
	{
		lua_getfield(l,2,"normal_x"); set.normal.x=lua_tonumber(l,-1); lua_pop(l,1);
		lua_getfield(l,2,"normal_y"); set.normal.y=lua_tonumber(l,-1); lua_pop(l,1);

		for(int i=0; i<set.count; i++){
			lua_rawgeti(l,2,1+i*5); set.points[i].pointA.x=lua_tonumber(l,-1); lua_pop(l,1);
			lua_rawgeti(l,2,2+i*5); set.points[i].pointA.y=lua_tonumber(l,-1); lua_pop(l,1);
			lua_rawgeti(l,2,3+i*5); set.points[i].pointB.x=lua_tonumber(l,-1); lua_pop(l,1);
			lua_rawgeti(l,2,4+i*5); set.points[i].pointB.y=lua_tonumber(l,-1); lua_pop(l,1);
			lua_rawgeti(l,2,5+i*5); set.points[i].distance=lua_tonumber(l,-1); lua_pop(l,1);
		}
		
		cpArbiterSetContactPointSet(arbiter,&set);
		
		cpArbiterSetContactPointSet(arbiter,&set);
		lua_pushvalue(l,2); // return the same table
	}
	else
	{
		lua_newtable(l);
	}
	
	lua_pushnumber(l,set.normal.x); lua_setfield(l,-2,"normal_x");
	lua_pushnumber(l,set.normal.y); lua_setfield(l,-2,"normal_y");

// number of points in array
	lua_pushnumber(l,set.count); lua_setfield(l,-2,"point_count");

// size of each point in array (sanity, future proof)
	lua_pushnumber(l,5); lua_setfield(l,-2,"point_scan");

//get contact info ax,ay,bx,by,d for each point, fill up array of numbers (easy code here, clean it up lua side)

	for(int i=0; i<set.count; i++){
		lua_pushnumber(l,set.points[i].pointA.x); lua_rawseti(l,-2,1+i*5);
		lua_pushnumber(l,set.points[i].pointA.y); lua_rawseti(l,-2,2+i*5);
		lua_pushnumber(l,set.points[i].pointB.x); lua_rawseti(l,-2,3+i*5);
		lua_pushnumber(l,set.points[i].pointB.y); lua_rawseti(l,-2,4+i*5);
		lua_pushnumber(l,set.points[i].distance); lua_rawseti(l,-2,5+i*5);
	}

	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// arbiter get/set surface velocity
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_chipmunk_arbiter_surface_velocity (lua_State *l)
{	
cpArbiter *arbiter=(cpArbiter *)lua_touserdata(l,1); if(!arbiter){ lua_pushstring(l,"missing arbiter"); lua_error(l); }
cpVect v;

	if(lua_isnumber(l,2))
	{
		v.x=luaL_checknumber(l,2);
		v.y=luaL_checknumber(l,3);
		cpArbiterSetSurfaceVelocity(arbiter, v );
	}
	
	v=cpArbiterGetSurfaceVelocity(arbiter);
	lua_pushnumber(l,v.x);
	lua_pushnumber(l,v.y);

	return 2;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// arbiter ignore collision ( lasts until the objects separate )
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_chipmunk_arbiter_ignore (lua_State *l)
{	
cpArbiter *arbiter=(cpArbiter *)lua_touserdata(l,1); if(!arbiter){ lua_pushstring(l,"missing arbiter"); lua_error(l); }

	cpArbiterIgnore(arbiter);

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
// body get/set velocity
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_chipmunk_body_velocity (lua_State *l)
{	
cpBody *body=lua_chipmunk_body_ptr(l,1);
cpVect v;

	if(lua_isnumber(l,2))
	{
		v.x=luaL_checknumber(l,2);
		v.y=luaL_checknumber(l,3);
		cpBodySetVelocity(body, v );
	}
	
	v=cpBodyGetVelocity(body);
	lua_pushnumber(l,v.x);
	lua_pushnumber(l,v.y);

	return 2;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// body get/set force
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_chipmunk_body_force (lua_State *l)
{	
cpBody *body=lua_chipmunk_body_ptr(l,1);
cpVect v;

	if(lua_isnumber(l,2))
	{
		v.x=luaL_checknumber(l,2);
		v.y=luaL_checknumber(l,3);
		cpBodySetForce(body, v );
	}
	
	v=cpBodyGetForce(body);
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
// body get/set angle
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_chipmunk_body_angular_velocity (lua_State *l)
{	
cpBody *body=lua_chipmunk_body_ptr(l,1);

	if(lua_isnumber(l,2))
	{
		cpBodySetAngularVelocity(body, luaL_checknumber(l,2) );
	}
	
	lua_pushnumber(l, cpBodyGetAngularVelocity(body) );

	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// body apply force
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_chipmunk_body_apply_force_local_point (lua_State *l)
{	
cpBody *body=lua_chipmunk_body_ptr(l,1);
cpVect v,p;

	v.x=luaL_checknumber(l,2);
	v.y=luaL_checknumber(l,3);
	p.x=luaL_checknumber(l,4);
	p.y=luaL_checknumber(l,5);
	cpBodyApplyForceAtLocalPoint(body, v , p );

	return 0;
}
static int lua_chipmunk_body_apply_force_world_point (lua_State *l)
{	
cpBody *body=lua_chipmunk_body_ptr(l,1);
cpVect v,p;

	v.x=luaL_checknumber(l,2);
	v.y=luaL_checknumber(l,3);
	p.x=luaL_checknumber(l,4);
	p.y=luaL_checknumber(l,5);
	cpBodyApplyForceAtWorldPoint(body, v , p );

	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// body apply impulse
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_chipmunk_body_apply_impulse_local_point (lua_State *l)
{	
cpBody *body=lua_chipmunk_body_ptr(l,1);
cpVect v,p;

	v.x=luaL_checknumber(l,2);
	v.y=luaL_checknumber(l,3);
	p.x=luaL_checknumber(l,4);
	p.y=luaL_checknumber(l,5);
	cpBodyApplyImpulseAtLocalPoint(body, v , p );

	return 0;
}
static int lua_chipmunk_body_apply_impulse_world_point (lua_State *l)
{	
cpBody *body=lua_chipmunk_body_ptr(l,1);
cpVect v,p;

	v.x=luaL_checknumber(l,2);
	v.y=luaL_checknumber(l,3);
	p.x=luaL_checknumber(l,4);
	p.y=luaL_checknumber(l,5);
	cpBodyApplyImpulseAtWorldPoint(body, v , p );

	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// shape get bounding box (min_x,min_y,max_x,max_y)
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_chipmunk_shape_bounding_box (lua_State *l)
{	
cpShape *shape=lua_chipmunk_shape_ptr(l,1);
cpBB bb;
	
	bb=cpShapeGetBB(shape);
	lua_pushnumber(l,bb.l);
	lua_pushnumber(l,bb.b);
	lua_pushnumber(l,bb.r);
	lua_pushnumber(l,bb.t);

	return 4;
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

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// shape get/set surface velocity
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_chipmunk_shape_surface_velocity (lua_State *l)
{	
cpShape *shape=lua_chipmunk_shape_ptr(l,1);
cpVect v;

	if(lua_isnumber(l,2))
	{
		v.x=luaL_checknumber(l,2);
		v.y=luaL_checknumber(l,3);
		cpShapeSetSurfaceVelocity(shape, v );
	}
	
	v=cpShapeGetSurfaceVelocity(shape);
	lua_pushnumber(l,v.x);
	lua_pushnumber(l,v.y);

	return 2;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// shape get/set collision type
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_chipmunk_shape_collision_type (lua_State *l)
{	
cpShape *shape=lua_chipmunk_shape_ptr(l,1);

	if(lua_isnumber(l,2))
	{
		cpShapeSetCollisionType(shape, luaL_checknumber(l,2) );
	}
	
	lua_pushnumber(l, cpShapeGetCollisionType(shape) );

	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// shape get/set filter
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_chipmunk_shape_filter (lua_State *l)
{	
cpShape *shape=lua_chipmunk_shape_ptr(l,1);
cpShapeFilter filter;
	if(lua_isnumber(l,2))
	{
		filter.group=luaL_checknumber(l,2);
		filter.categories=luaL_checknumber(l,3);
		filter.mask=luaL_checknumber(l,4);
		cpShapeSetFilter(shape, filter );
	}
	
	filter=cpShapeGetFilter(shape);
	lua_pushnumber(l,filter.group);
	lua_pushnumber(l,filter.categories);
	lua_pushnumber(l,filter.mask);

	return 3;
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

/*+-----------------------------------------------------------------1------------------------------------------------+*/
//
// space add constraint
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_chipmunk_space_add_constraint (lua_State *l)
{	
cpSpace *space=lua_chipmunk_space_ptr(l,1);
cpConstraint *constraint=lua_chipmunk_constraint_ptr(l,2);

	cpSpaceAddConstraint(space,constraint);

	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// space remove constraint
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_chipmunk_space_remove_constraint (lua_State *l)
{	
cpSpace *space=lua_chipmunk_space_ptr(l,1);
cpConstraint *constraint=lua_chipmunk_constraint_ptr(l,2);

	cpSpaceRemoveConstraint(space,constraint);

	return 0;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// space contains constraint
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_chipmunk_space_contains_constraint (lua_State *l)
{	
cpSpace *space=lua_chipmunk_space_ptr(l,1);
cpConstraint *constraint=lua_chipmunk_constraint_ptr(l,2);

	lua_pushboolean(l,cpSpaceContainsConstraint(space,constraint));

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
		{"body_lookup",						lua_chipmunk_body_lookup},

		{"shape_create",					lua_chipmunk_shape_create},
		{"shape_destroy",					lua_chipmunk_shape_destroy},
		{"shape_lookup",					lua_chipmunk_shape_lookup},

		{"constraint_create",				lua_chipmunk_constraint_create},
		{"constraint_destroy",				lua_chipmunk_constraint_destroy},
		{"constraint_lookup",				lua_chipmunk_constraint_lookup},

		{"space_add_handler",				lua_chipmunk_space_add_handler},

		{"space_add_body",					lua_chipmunk_space_add_body},
		{"space_remove_body",				lua_chipmunk_space_remove_body},
		{"space_contains_body",				lua_chipmunk_space_contains_body},

		{"space_add_shape",					lua_chipmunk_space_add_shape},
		{"space_remove_shape",				lua_chipmunk_space_remove_shape},
		{"space_contains_shape",			lua_chipmunk_space_contains_shape},
		
		{"space_add_constraint",			lua_chipmunk_space_add_constraint},
		{"space_remove_constraint",			lua_chipmunk_space_remove_constraint},
		{"space_contains_constraint",		lua_chipmunk_space_contains_constraint},

//		{"space_reindex_shape",				lua_chipmunk_space_reindex_shape},
//		{"space_reindex_shapes_for_body",	lua_chipmunk_space_reindex_shapes_for_body},
//		{"space_reindex_static",			lua_chipmunk_space_reindex_static},
		{"space_step",						lua_chipmunk_space_step},
		{"space_iterations",				lua_chipmunk_space_iterations},
		{"space_gravity",					lua_chipmunk_space_gravity},
		{"space_damping",					lua_chipmunk_space_damping},
//		{"space_idle_speed_threshold",		lua_chipmunk_space_idle_speed_threshold},
//		{"space_sleep_time_threshold",		lua_chipmunk_space_sleep_time_threshold},
		{"space_collision_slop",			lua_chipmunk_space_collision_slop},
		{"space_collision_bias",			lua_chipmunk_space_collision_bias},
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
		{"body_velocity",					lua_chipmunk_body_velocity},
		{"body_force",						lua_chipmunk_body_force},
		{"body_angle",						lua_chipmunk_body_angle},
		{"body_angular_velocity",			lua_chipmunk_body_angular_velocity},
//		{"body_torque",						lua_chipmunk_body_torque},
		{"body_apply_force_local_point",	lua_chipmunk_body_apply_force_local_point},
		{"body_apply_force_world_point",	lua_chipmunk_body_apply_force_world_point},
		{"body_apply_impulse_local_point",	lua_chipmunk_body_apply_impulse_local_point},
		{"body_apply_impulse_world_point",	lua_chipmunk_body_apply_impulse_world_point},

		{"shape_bounding_box",				lua_chipmunk_shape_bounding_box},
//		{"shape_sensor",					lua_chipmunk_shape_sensor},
		{"shape_elasticity",				lua_chipmunk_shape_elasticity},
		{"shape_friction",					lua_chipmunk_shape_friction},
		{"shape_surface_velocity",			lua_chipmunk_shape_surface_velocity},
		{"shape_collision_type",			lua_chipmunk_shape_collision_type},
		{"shape_filter",					lua_chipmunk_shape_filter},

		{"arbiter_surface_velocity",		lua_chipmunk_arbiter_surface_velocity},
		{"arbiter_points",					lua_chipmunk_arbiter_points},
		{"arbiter_ignore",					lua_chipmunk_arbiter_ignore},


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

	const luaL_reg constraint_shape[] =
	{
		{"__gc",			lua_chipmunk_constraint_destroy},
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

	luaL_newmetatable(l, lua_chipmunk_constraint_meta_name);
	luaL_openlib(l, NULL, constraint_shape, 0);
	lua_pop(l,1);

	lua_newtable(l);
	luaL_openlib(l, NULL, lib, 0);
	return 1;
}

