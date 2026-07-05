/*

 Copyright (C) 2026 Kriss Blank < Kriss@XIXs.com >
 This file is distributed under the terms of the MIT license.
 http://en.wikipedia.org/wiki/MIT_License

*/
#include "all.h"
#include "box2d/box2d.h"

/*

we can use either this string as a string identifier
or its address as a light userdata identifier, both unique

*/
const char *lua_b2_world_meta_name="b2_world*ptr";
const char *lua_b2_body_meta_name ="b2_body*ptr";
const char *lua_b2_shape_meta_name="b2_shape*ptr";


/*+---------------------------------------------------------------------

Get library version as 3 numbers

*/
static int lua_b2_version (lua_State *l)
{
b2Version v=b2GetVersion();

	lua_pushnumber(l,v.major);
	lua_pushnumber(l,v.minor);
	lua_pushnumber(l,v.revision);

	return 3;
}

/*+---------------------------------------------------------------------

Set units per meter

*/
static int lua_b2_meter (lua_State *l)
{
float f=lua_tonumber(l,1);
	b2SetLengthUnitsPerMeter(f);
	return 0;
}



/*+---------------------------------------------------------------------

world create/destroy

*/
b2WorldId * lua_b2_world_ptr_ptr (lua_State *l,int idx)
{
b2WorldId *pp;
	pp=(b2WorldId*)luaL_checkudata(l, idx , lua_b2_world_meta_name);
	return pp;
}

b2WorldId   lua_b2_world_ptr (lua_State *l,int idx)
{
b2WorldId *pp;
	pp=lua_b2_world_ptr_ptr(l,idx);
	if(!pp) { luaL_error(l,"box2d world is null"); }
	return *pp;
}

static int lua_b2_world_create (lua_State *l)
{	
b2WorldId *pp;

	// defaults
	b2WorldDef def=b2DefaultWorldDef();

	// get def values if they are not nil
	if(lua_istable(l,2)) // got defs
	{
		lua_getfield(l,2,"contactDampingRatio");
		if(!lua_isnil(l,-1)) { def.contactDampingRatio = (float)lua_tonumber(l,-1); }
		lua_pop(l,1);
		lua_getfield(l,2,"contactHertz");
		if(!lua_isnil(l,-1)) { def.contactHertz = (float)lua_tonumber(l,-1); }
		lua_pop(l,1);
		lua_getfield(l,2,"enableContinuous");
		if(!lua_isnil(l,-1)) { def.enableContinuous = lua_toboolean(l,-1); }
		lua_pop(l,1);
		lua_getfield(l,2,"enableSleep");
		if(!lua_isnil(l,-1)) { def.enableSleep = lua_toboolean(l,-1); }
		lua_pop(l,1);
		lua_getfield(l,2,"gravity");
		if(!lua_isnil(l,-1))
		{
			lua_rawgeti(l,-1,1);
			lua_rawgeti(l,-2,2);
			def.gravity = (b2Vec2){(float)lua_tonumber(l,-2),(float)lua_tonumber(l,-1)};
			lua_pop(l,2);
		}
		lua_pop(l,1);
		lua_getfield(l,2,"hitEventThreshold");
		if(!lua_isnil(l,-1)) { def.hitEventThreshold = (float)lua_tonumber(l,-1); }
		lua_pop(l,1);
/*
		lua_getfield(l,2,"jointDampingRatio");
		if(!lua_isnil(l,-1)) { def.jointDampingRatio = (float)lua_tonumber(l,-1); }
		lua_pop(l,1);
		lua_getfield(l,2,"jointHertz");
		if(!lua_isnil(l,-1)) { def.jointHertz = (float)lua_tonumber(l,-1); }
		lua_pop(l,1);
		lua_getfield(l,2,"maxContactPushSpeed");
		if(!lua_isnil(l,-1)) { def.maxContactPushSpeed = (float)lua_tonumber(l,-1); }
		lua_pop(l,1);
*/
		lua_getfield(l,2,"maximumLinearSpeed");
		if(!lua_isnil(l,-1)) { def.maximumLinearSpeed = (float)lua_tonumber(l,-1); }
		lua_pop(l,1);
		lua_getfield(l,2,"restitutionThreshold");
		if(!lua_isnil(l,-1)) { def.restitutionThreshold = (float)lua_tonumber(l,-1); }
		lua_pop(l,1);
	}

// create ptr ptr userdata
	pp=(b2WorldId*)lua_newuserdata(l, sizeof(b2WorldId));
	*pp=(b2WorldId){0};
	luaL_getmetatable(l, lua_b2_world_meta_name);
	lua_setmetatable(l, -2);

// allocate b2WorldId
	*pp=b2CreateWorld(&def);

// use registry so we can find the world table from world ptr,
// this has the side effect that world MUST be destroyed,
// it will not be GCd as this will keep it alive.
	lua_pushlightuserdata(l,pp);
	lua_pushvalue(l,1); // this will be the lua world table
	lua_settable(l,LUA_REGISTRYINDEX);

	return 1;
}

static int lua_b2_world_destroy (lua_State *l)
{	
b2WorldId *pp=lua_b2_world_ptr_ptr(l, 1 );
	if(B2_IS_NON_NULL(*pp))
	{
// remove registry link
		lua_pushlightuserdata(l,pp);
		lua_pushnil(l);
		lua_settable(l,LUA_REGISTRYINDEX);
		
		b2DestroyWorld(*pp);
		*pp=(b2WorldId){0};
	}	
	return 0;
}

/*+---------------------------------------------------------------------

body create/destroy

*/
b2BodyId * lua_b2_body_ptr_ptr (lua_State *l,int idx)
{
b2BodyId *pp;
	pp=(b2BodyId*)luaL_checkudata(l, idx , lua_b2_body_meta_name);
	return pp;
}

b2BodyId   lua_b2_body_ptr (lua_State *l,int idx)
{
b2BodyId *pp;
	pp=lua_b2_body_ptr_ptr(l,idx);
	if(!pp) { luaL_error(l,"box2d body is null"); }
	return *pp;
}

static int lua_b2_body_create (lua_State *l)
{	
b2WorldId world;
b2BodyId *pp;

// defaults
	b2BodyDef def=b2DefaultBodyDef();

	// get def values if they are not nil
	if(lua_istable(l,2)) // got defs
	{
		lua_getfield(l,2,"allowFastRotation");
		if(!lua_isnil(l,-1)) { def.allowFastRotation = lua_toboolean(l,-1); }
		lua_pop(l,1);
		lua_getfield(l,2,"angularDamping");
		if(!lua_isnil(l,-1)) { def.angularDamping = (float)lua_tonumber(l,-1); }
		lua_pop(l,1);
		lua_getfield(l,2,"angularVelocity");
		if(!lua_isnil(l,-1)) { def.angularVelocity = (float)lua_tonumber(l,-1); }
		lua_pop(l,1);
		lua_getfield(l,2,"enableSleep");
		if(!lua_isnil(l,-1)) { def.enableSleep = lua_toboolean(l,-1); }
		lua_pop(l,1);
/*
		lua_getfield(l,2,"fixedRotation");
		if(!lua_isnil(l,-1)) { def.fixedRotation = lua_toboolean(l,-1); }
		lua_pop(l,1);
*/
		lua_getfield(l,2,"gravityScale");
		if(!lua_isnil(l,-1)) { def.gravityScale = (float)lua_tonumber(l,-1); }
		lua_pop(l,1);
		lua_getfield(l,2,"isAwake");
		if(!lua_isnil(l,-1)) { def.isAwake = lua_toboolean(l,-1); }
		lua_pop(l,1);
		lua_getfield(l,2,"isBullet");
		if(!lua_isnil(l,-1)) { def.isBullet = lua_toboolean(l,-1); }
		lua_pop(l,1);
		lua_getfield(l,2,"isEnabled");
		if(!lua_isnil(l,-1)) { def.isEnabled = lua_toboolean(l,-1); }
		lua_pop(l,1);
		lua_getfield(l,2,"linearDamping");
		if(!lua_isnil(l,-1)) { def.linearDamping = (float)lua_tonumber(l,-1); }
		lua_pop(l,1);
		lua_getfield(l,2,"linearVelocity");
		if(!lua_isnil(l,-1))
		{
			lua_rawgeti(l,-1,1);
			lua_rawgeti(l,-2,2);
			def.linearVelocity = (b2Vec2){(float)lua_tonumber(l,-2),(float)lua_tonumber(l,-1)};
			lua_pop(l,2);
		}
		lua_pop(l,1);
		lua_getfield(l,2,"name");
		if(!lua_isnil(l,-1)) { def.name = lua_tostring(l,-1); }
		lua_pop(l,1);
		lua_getfield(l,2,"position");
		if(!lua_isnil(l,-1))
		{
			lua_rawgeti(l,-1,1);
			lua_rawgeti(l,-2,2);
			def.position = (b2Vec2){(float)lua_tonumber(l,-2),(float)lua_tonumber(l,-1)};
			lua_pop(l,2);
		}
		lua_pop(l,1);
		lua_getfield(l,2,"rotation");
		if(!lua_isnil(l,-1)) { def.rotation = b2MakeRot((float)lua_tonumber(l,-1)); }
		lua_pop(l,1);
		lua_getfield(l,2,"sleepThreshold");
		if(!lua_isnil(l,-1)) { def.sleepThreshold = (float)lua_tonumber(l,-1); }
		lua_pop(l,1);
		lua_getfield(l,2,"type");
		if(lua_isstring(l,-1))
		{
			char *s=lua_tostring(l,-1);
			if(strcmp(s,"static")==0) { def.type = b2_staticBody; }
			else
			if(strcmp(s,"kinematic")==0) { def.type = b2_kinematicBody; }
			else
			if(strcmp(s,"dynamic")==0) { def.type = b2_dynamicBody; }
		}
		lua_pop(l,1);
	}

// create ptr ptr userdata
	pp=(b2BodyId*)lua_newuserdata(l, sizeof(b2BodyId));
	*pp=(b2BodyId){0};
	luaL_getmetatable(l, lua_b2_body_meta_name);
	lua_setmetatable(l, -2);

	world=lua_b2_world_ptr(l,1);

// allocate b2BodyId
	*pp=b2CreateBody(world,&def);

	return 1;
}

static int lua_b2_body_destroy (lua_State *l)
{	
b2BodyId *pp=lua_b2_body_ptr_ptr(l, 1 );
	if(B2_IS_NON_NULL(*pp))
	{
// remove registry link
		lua_pushlightuserdata(l,pp);
		lua_pushnil(l);
		lua_settable(l,LUA_REGISTRYINDEX);
		
		b2DestroyBody(*pp);
		*pp=(b2BodyId){0};
	}	
	return 0;
}

/*+---------------------------------------------------------------------

shape create/destroy

*/
b2ShapeId * lua_b2_shape_ptr_ptr (lua_State *l,int idx)
{
b2ShapeId *pp;
	pp=(b2ShapeId*)luaL_checkudata(l, idx , lua_b2_shape_meta_name);
	return pp;
}

b2ShapeId   lua_b2_shape_ptr (lua_State *l,int idx)
{
b2ShapeId *pp;
	pp=lua_b2_shape_ptr_ptr(l,idx);
	if(!pp) { luaL_error(l,"box2d shape is null"); }
	return *pp;
}

static int lua_b2_shape_create (lua_State *l)
{	
b2BodyId body;
b2ShapeId *pp;

// defaults
	b2ShapeDef def=b2DefaultShapeDef();

	// get def values if they are not nil
	if(lua_istable(l,2)) // got defs
	{
		lua_getfield(l,2,"density");
		if(!lua_isnil(l,-1)) { def.density = (float)lua_tonumber(l,-1); }
		lua_pop(l,1);
		lua_getfield(l,2,"enableContactEvents");
		if(!lua_isnil(l,-1)) { def.enableContactEvents = lua_toboolean(l,-1); }
		lua_pop(l,1);
		lua_getfield(l,2,"enableHitEvents");
		if(!lua_isnil(l,-1)) { def.enableHitEvents = lua_toboolean(l,-1); }
		lua_pop(l,1);
		lua_getfield(l,2,"enablePreSolveEvents");
		if(!lua_isnil(l,-1)) { def.enablePreSolveEvents = lua_toboolean(l,-1); }
		lua_pop(l,1);
		lua_getfield(l,2,"enableSensorEvents");
		if(!lua_isnil(l,-1)) { def.enableSensorEvents = lua_toboolean(l,-1); }
		lua_pop(l,1);
		lua_getfield(l,2,"filter"); // bitmasks are doubles so only 52 bits not 64
		if(!lua_isnil(l,-1))
		{
			lua_rawgeti(l,-1,1);
			lua_rawgeti(l,-2,2);
			lua_rawgeti(l,-3,3);
			def.filter = (b2Filter){(uint64_t)lua_tonumber(l,-3),(int)lua_tonumber(l,-2),(uint64_t)lua_tonumber(l,-1)};
			lua_pop(l,3);
		}
		lua_pop(l,1);
		lua_getfield(l,2,"invokeContactCreation");
		if(!lua_isnil(l,-1)) { def.invokeContactCreation = lua_toboolean(l,-1); }
		lua_pop(l,1);
		lua_getfield(l,2,"isSensor");
		if(!lua_isnil(l,-1)) { def.isSensor = lua_toboolean(l,-1); }
		lua_pop(l,1);
		lua_getfield(l,2,"material"); // bitmasks are doubles so only 52 bits not 64
		if(!lua_isnil(l,-1))
		{
			lua_getfield(l,-1,"customColor");
			if(!lua_isnil(l,-1)) { def.material.customColor = (uint32_t)lua_tonumber(l,-1); }
			lua_pop(l,1);
			lua_getfield(l,-1,"friction");
			if(!lua_isnil(l,-1)) { def.material.friction = (float)lua_tonumber(l,-1); }
			lua_pop(l,1);
			lua_getfield(l,-1,"restitution");
			if(!lua_isnil(l,-1)) { def.material.restitution = (float)lua_tonumber(l,-1); }
			lua_pop(l,1);
			lua_getfield(l,-1,"rollingResistance");
			if(!lua_isnil(l,-1)) { def.material.rollingResistance = (float)lua_tonumber(l,-1); }
			lua_pop(l,1);
			lua_getfield(l,-1,"tangentSpeed");
			if(!lua_isnil(l,-1)) { def.material.tangentSpeed = (float)lua_tonumber(l,-1); }
			lua_pop(l,1);
			lua_getfield(l,-1,"userMaterialId");
			if(!lua_isnil(l,-1)) { def.material.userMaterialId = (int)lua_tonumber(l,-1); }
			lua_pop(l,1);
		}
		lua_pop(l,1);
		lua_getfield(l,2,"updateBodyMass");
		if(!lua_isnil(l,-1)) { def.updateBodyMass = lua_toboolean(l,-1); }
		lua_pop(l,1);
	}
	
// create ptr ptr userdata
	pp=(b2ShapeId*)lua_newuserdata(l, sizeof(b2ShapeId));
	*pp=(b2ShapeId){0};
	luaL_getmetatable(l, lua_b2_shape_meta_name);
	lua_setmetatable(l, -2);

	body=lua_b2_body_ptr(l,1);

	// get shape values
	lua_getfield(l,3,"shape");
	char *shape_type=lua_tostring(l,-1);
	if(strcmp(shape_type,"circle")==0)
	{
		lua_pop(l,1);
// allocate b2ShapeId circle
		b2Circle circle;
		lua_getfield(l,3,"center"); // bitmasks are doubles so only 52 bits not 64
		if(!lua_isnil(l,-1))
		{
			lua_rawgeti(l,-1,1);
			lua_rawgeti(l,-2,2);
			circle.center = (b2Vec2){(float)lua_tonumber(l,-2),(float)lua_tonumber(l,-1)};
			lua_pop(l,2);
		}
		lua_pop(l,1);
		lua_getfield(l,3,"radius");
		if(!lua_isnil(l,-1)) { circle.radius = (float)lua_tonumber(l,-1); }
		lua_pop(l,1);

		*pp=b2CreateCircleShape(body,&def,&circle);
	}
	else
	if(strcmp(shape_type,"segment")==0)
	{
		lua_pop(l,1);
// allocate b2ShapeId segment
		b2Segment segment;
		lua_getfield(l,3,"point1");
		if(!lua_isnil(l,-1))
		{
			lua_rawgeti(l,-1,1);
			lua_rawgeti(l,-2,2);
			segment.point1 = (b2Vec2){(float)lua_tonumber(l,-2),(float)lua_tonumber(l,-1)};
			lua_pop(l,2);
		}
		lua_pop(l,1);
		lua_getfield(l,3,"point2");
		if(!lua_isnil(l,-1))
		{
			lua_rawgeti(l,-1,1);
			lua_rawgeti(l,-2,2);
			segment.point2 = (b2Vec2){(float)lua_tonumber(l,-2),(float)lua_tonumber(l,-1)};
			lua_pop(l,2);
		}
		lua_pop(l,1);

		*pp=b2CreateSegmentShape(body,&def,&segment);
	}
	else
	if(strcmp(shape_type,"capsule")==0)
	{
		lua_pop(l,1);
// allocate b2ShapeId capsule
		b2Capsule capsule;
		lua_getfield(l,3,"center1");
		if(!lua_isnil(l,-1))
		{
			lua_rawgeti(l,-1,1);
			lua_rawgeti(l,-2,2);
			capsule.center1 = (b2Vec2){(float)lua_tonumber(l,-2),(float)lua_tonumber(l,-1)};
			lua_pop(l,2);
		}
		lua_pop(l,1);
		lua_getfield(l,3,"center2");
		if(!lua_isnil(l,-1))
		{
			lua_rawgeti(l,-1,1);
			lua_rawgeti(l,-2,2);
			capsule.center2 = (b2Vec2){(float)lua_tonumber(l,-2),(float)lua_tonumber(l,-1)};
			lua_pop(l,2);
		}
		lua_pop(l,1);
		lua_getfield(l,3,"radius");
		if(!lua_isnil(l,-1)) { capsule.radius = (float)lua_tonumber(l,-1); }
		lua_pop(l,1);

		*pp=b2CreateCapsuleShape(body,&def,&capsule);
	}
	else
	if(strcmp(shape_type,"box")==0)
	{
		lua_pop(l,1);
// allocate b2ShapeId capsule
		b2Polygon polygon;

		float halfWidth=0.0f;
		lua_getfield(l,3,"halfWidth");
		if(!lua_isnil(l,-1)) { halfWidth = (float)lua_tonumber(l,-1); }
		lua_pop(l,1);

		float halfHeight=0.0f;
		lua_getfield(l,3,"halfHeight");
		if(!lua_isnil(l,-1)) { halfHeight = (float)lua_tonumber(l,-1); }
		lua_pop(l,1);

		b2Vec2 center=(b2Vec2){0.0f,0.0f};
		lua_getfield(l,3,"center");
		if(!lua_isnil(l,-1))
		{
			lua_rawgeti(l,-1,1);
			lua_rawgeti(l,-2,2);
			center = (b2Vec2){(float)lua_tonumber(l,-2),(float)lua_tonumber(l,-1)};
			lua_pop(l,2);
		}
		lua_pop(l,1);

		float rotation=0.0f;
		lua_getfield(l,3,"rotation");
		if(!lua_isnil(l,-1)) { rotation = (float)lua_tonumber(l,-1); }
		lua_pop(l,1);
		
		float radius=0.0f;
		lua_getfield(l,3,"radius");
		if(!lua_isnil(l,-1)) { radius = (float)lua_tonumber(l,-1); }
		lua_pop(l,1);

		polygon=b2MakeOffsetRoundedBox(halfWidth,halfHeight,center,b2MakeRot(rotation),radius);

		*pp=b2CreatePolygonShape(body,&def,&polygon);
	}
	else
	{
		lua_pop(l,1);
		lua_pushstring(l,"unknown shape type");
		lua_error(l);
	}

	return 1;
}

static int lua_b2_shape_destroy (lua_State *l)
{	
b2ShapeId *pp=lua_b2_shape_ptr_ptr(l, 1 );
	if(B2_IS_NON_NULL(*pp))
	{
// remove registry link
		lua_pushlightuserdata(l,pp);
		lua_pushnil(l);
		lua_settable(l,LUA_REGISTRYINDEX);
		
		int update=1;
		if(!lua_isnil(l,2)) // optional
		{
			update=lua_toboolean(l,2);
		}
		b2DestroyShape(*pp,update);
		*pp=(b2ShapeId){0};
	}	
	return 0;
}

/*+---------------------------------------------------------------------

open library.

*/
LUALIB_API int luaopen_box2d_core (lua_State *l)
{
	const luaL_Reg lib[] =
	{
		{"version",					lua_b2_version},
		{"meter",					lua_b2_meter},
		{"world_create",			lua_b2_world_create},
		{"world_destroy",			lua_b2_world_destroy},
		{"body_create",				lua_b2_body_create},
		{"body_destroy",			lua_b2_body_destroy},
		{"shape_create",			lua_b2_shape_create},
		{"shape_destroy",			lua_b2_shape_destroy},
		{0,0}
	};

	const luaL_Reg meta_world[] =
	{
		{"__gc",			lua_b2_world_destroy},
		{0,0}
	};

	const luaL_Reg meta_body[] =
	{
		{"__gc",			lua_b2_body_destroy},
		{0,0}
	};

	const luaL_Reg meta_shape[] =
	{
		{"__gc",			lua_b2_shape_destroy},
		{0,0}
	};

	luaL_newmetatable(l, lua_b2_world_meta_name);
	luaL_openlib(l, NULL, meta_world, 0);
	lua_pop(l,1);

	luaL_newmetatable(l, lua_b2_body_meta_name);
	luaL_openlib(l, NULL, meta_body, 0);
	lua_pop(l,1);

	luaL_newmetatable(l, lua_b2_shape_meta_name);
	luaL_openlib(l, NULL, meta_shape, 0);
	lua_pop(l,1);

	lua_newtable(l);
	luaL_openlib(l, NULL, lib, 0);
	return 1;
}

