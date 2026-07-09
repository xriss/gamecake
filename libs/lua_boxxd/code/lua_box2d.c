/*

 Copyright (C) 2026 Kriss Blank < Kriss@XIXs.com >
 This file is distributed under the terms of the MIT license.
 http://en.wikipedia.org/wiki/MIT_License

Note that much of the documentation for the C functions exposed to Lua 
can be found in the associated box2d.lua file.

*/

#include <string.h>

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#include "box2d/box2d.h"

/*

we can use either this string as a string identifier
or its address as a light userdata identifier, both unique

*/
const char *lua_b2_world_meta_name="b2_world*ptr";
const char *lua_b2_body_meta_name ="b2_body*ptr";
const char *lua_b2_shape_meta_name="b2_shape*ptr";
const char *lua_b2_joint_meta_name="b2_joint*ptr";


/*+---------------------------------------------------------------------

Get box2d variables

*/
static int lua_b2_get (lua_State *l)
{
	lua_newtable(l);
	
	b2Version version=b2GetVersion();
	lua_newtable(l);
	lua_pushnumber(l, version.major );
	lua_rawseti(l, -2 , 1 );
	lua_pushnumber(l, version.minor );
	lua_rawseti(l, -2 , 2 );
	lua_pushnumber(l, version.revision );
	lua_rawseti(l, -2 , 3 );
	lua_setfield(l, -2 , "version" );

	lua_pushnumber(l, b2GetLengthUnitsPerMeter() );
	lua_setfield(l, -2 , "lengthUnitsPerMeter" );

	lua_pushnumber(l, b2GetByteCount() );
	lua_setfield(l, -2 , "byteCount" );

	return 1;
}

/*+---------------------------------------------------------------------

Set box2d variables

*/
static int lua_b2_set (lua_State *l)
{
	lua_getfield(l,1,"lengthUnitsPerMeter");
	if(!lua_isnil(l,-1))
	{
		b2SetLengthUnitsPerMeter( lua_tonumber(l,-1) );
	}
	lua_pop(l,1);

	return 0;
}

/*+---------------------------------------------------------------------

Get contact data given an ID

*/
static int lua_b2_contact (lua_State *l)
{
	b2ContactId id=*((b2ContactId*)(lua_tostring(l,1))); // get ID from lua string

	b2ContactData contact = b2Contact_GetData(id);

	lua_pushnumber(l, contact.manifold.normal.x );
	lua_pushnumber(l, contact.manifold.normal.y );

	for( int i=0 ; i<contact.manifold.pointCount ; i++ )
	{
		b2ManifoldPoint p=contact.manifold.points[i];

		lua_newtable(l);

		lua_pushnumber(l, p.anchorA.x );
		lua_setfield(l,-2,"anchorA_x");
		lua_pushnumber(l, p.anchorA.y );
		lua_setfield(l,-2,"anchorA_y");

		lua_pushnumber(l, p.anchorB.x );
		lua_setfield(l,-2,"anchorB_x");
		lua_pushnumber(l, p.anchorB.y );
		lua_setfield(l,-2,"anchorB_y");

		lua_pushnumber(l, p.separation );
		lua_setfield(l,-2,"separation");

		lua_pushnumber(l, p.baseSeparation );
		lua_setfield(l,-2,"baseSeparation");

		lua_pushnumber(l, p.normalImpulse );
		lua_setfield(l,-2,"normalImpulse");

		lua_pushnumber(l, p.tangentImpulse );
		lua_setfield(l,-2,"tangentImpulse");

		lua_pushnumber(l, p.totalNormalImpulse );
		lua_setfield(l,-2,"totalNormalImpulse");

		lua_pushnumber(l, p.normalVelocity );
		lua_setfield(l,-2,"normalVelocity");

		lua_pushnumber(l, p.id );
		lua_setfield(l,-2,"id");

		lua_pushboolean(l, p.persisted );
		lua_setfield(l,-2,"persisted");
	}

	return 2+contact.manifold.pointCount;
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
	if(lua_istable(l,1)) // got defs
	{
		lua_getfield(l,1,"gravity");
		if(!lua_isnil(l,-1))
		{
			lua_pushinteger(l,1);	lua_gettable(l,-2);
			lua_pushinteger(l,2);	lua_gettable(l,-3);
			def.gravity = (b2Vec2){(float)lua_tonumber(l,-2),(float)lua_tonumber(l,-1)};
			lua_pop(l,2);
		}
		lua_pop(l,1);
		lua_getfield(l,1,"restitutionThreshold");
		if(!lua_isnil(l,-1)) { def.restitutionThreshold = (float)lua_tonumber(l,-1); }
		lua_pop(l,1);
		lua_getfield(l,1,"hitEventThreshold");
		if(!lua_isnil(l,-1)) { def.hitEventThreshold = (float)lua_tonumber(l,-1); }
		lua_pop(l,1);
		lua_getfield(l,1,"contactHertz");
		if(!lua_isnil(l,-1)) { def.contactHertz = (float)lua_tonumber(l,-1); }
		lua_pop(l,1);
		lua_getfield(l,1,"contactDampingRatio");
		if(!lua_isnil(l,-1)) { def.contactDampingRatio = (float)lua_tonumber(l,-1); }
		lua_pop(l,1);
		lua_getfield(l,1,"contactSpeed");
		if(!lua_isnil(l,-1)) { def.contactSpeed = (float)lua_tonumber(l,-1); }
		lua_pop(l,1);
		lua_getfield(l,1,"maximumLinearSpeed");
		if(!lua_isnil(l,-1)) { def.maximumLinearSpeed = (float)lua_tonumber(l,-1); }
		lua_pop(l,1);

		lua_getfield(l,1,"enableSleep");
		if(!lua_isnil(l,-1)) { def.enableSleep = lua_toboolean(l,-1); }
		lua_pop(l,1);

		lua_getfield(l,1,"enableContinuous");
		if(!lua_isnil(l,-1)) { def.enableContinuous = lua_toboolean(l,-1); }
		lua_pop(l,1);

		lua_getfield(l,1,"enableContactSoftening");
		if(!lua_isnil(l,-1)) { def.enableContactSoftening = lua_toboolean(l,-1); }
		lua_pop(l,1);
	}

// create ptr ptr userdata
	pp=(b2WorldId*)lua_newuserdata(l, sizeof(b2WorldId));
	*pp=(b2WorldId){0};
	luaL_getmetatable(l, lua_b2_world_meta_name);
	lua_setmetatable(l, -2);

// allocate b2WorldId
	*pp=b2CreateWorld(&def);

	lua_pushlstring(l,(const char *)pp,sizeof(b2WorldId)); // id to (non printable) string
	return 2;
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

Get world variables

*/
static int lua_b2_world_get (lua_State *l)
{
	b2WorldId world = lua_b2_world_ptr(l, 1 );

	lua_newtable(l);

	lua_pushboolean(l, b2World_IsSleepingEnabled(world) );
	lua_setfield(l, -2 , "enableSleeping" );

	lua_pushboolean(l, b2World_IsContinuousEnabled(world) );
	lua_setfield(l, -2 , "enableContinuous" );

	lua_pushnumber(l, b2World_GetRestitutionThreshold(world) );
	lua_setfield(l, -2 , "restitutionThreshold" );

	lua_pushnumber(l, b2World_GetHitEventThreshold(world) );
	lua_setfield(l, -2 , "hitEventThreshold" );

	b2Vec2 gravity = b2World_GetGravity(world);
	lua_newtable(l);
	lua_pushnumber(l, gravity.x );	lua_rawseti(l, -2 , 1 );
	lua_pushnumber(l, gravity.y );	lua_rawseti(l, -2 , 2 );
	lua_setfield(l, -2 , "gravity" );

	lua_pushnumber(l, b2World_GetMaximumLinearSpeed(world) );
	lua_setfield(l, -2 , "maximumLinearSpeed" );

	lua_pushboolean(l, b2World_IsWarmStartingEnabled(world) );
	lua_setfield(l, -2 , "enableWarmStarting" );


	return 1;
}


/*+---------------------------------------------------------------------

Set world variables

*/
static int lua_b2_world_set (lua_State *l)
{
	b2WorldId world = lua_b2_world_ptr(l, 1 );

	lua_getfield(l,1,"enableSleeping");
	if(!lua_isnil(l,-1))
	{
		b2World_EnableSleeping(world, lua_toboolean(l,-1) );
	}
	lua_pop(l,1);

	lua_getfield(l,1,"enableContinuous");
	if(!lua_isnil(l,-1))
	{
		b2World_EnableContinuous(world, lua_toboolean(l,-1) );
	}
	lua_pop(l,1);

	lua_getfield(l,1,"restitutionThreshold");
	if(!lua_isnil(l,-1))
	{
		b2World_SetRestitutionThreshold(world, lua_tonumber(l,-1) );
	}
	lua_pop(l,1);

	lua_getfield(l,1,"hitEventThreshold");
	if(!lua_isnil(l,-1))
	{
		b2World_SetHitEventThreshold(world, lua_tonumber(l,-1) );
	}
	lua_pop(l,1);

	lua_getfield(l,1,"gravity");
	if(!lua_isnil(l,-1))
	{
		b2Vec2 gravity;
		lua_pushinteger(l, 1 ); lua_gettable(l, -2 );
		gravity.x = lua_tonumber(l, -1 ); lua_pop(l, 1 );
		lua_pushinteger(l, 2 ); lua_gettable(l, -2 );
		gravity.y = lua_tonumber(l, -1 ); lua_pop(l, 1 );
		b2World_SetGravity(world, gravity );
	}
	lua_pop(l,1);

	lua_getfield(l,1,"contactTuning");
	if(!lua_isnil(l,-1))
	{
		lua_pushinteger(l, 1 ); lua_gettable(l, -2 );
		float hertz = lua_tonumber(l, -1 ); lua_pop(l, 1 );
		lua_pushinteger(l, 2 ); lua_gettable(l, -2 );
		float dampingRatio = lua_tonumber(l, -1 ); lua_pop(l, 1 );
		lua_pushinteger(l, 3 ); lua_gettable(l, -2 );
		float pushSpeed = lua_tonumber(l, -1 ); lua_pop(l, 1 );
		b2World_SetContactTuning(world, hertz , dampingRatio , pushSpeed );
	}
	lua_pop(l,1);

	lua_getfield(l,1,"maximumLinearSpeed");
	if(!lua_isnil(l,-1))
	{
		b2World_SetMaximumLinearSpeed(world, lua_tonumber(l,-1) );
	}
	lua_pop(l,1);

	lua_getfield(l,1,"enableWarmStarting");
	if(!lua_isnil(l,-1))
	{
		b2World_EnableWarmStarting(world, lua_toboolean(l,-1) );
	}
	lua_pop(l,1);

	return 0;
}

/*+---------------------------------------------------------------------

Move the world through time itself.

*/
static int lua_b2_world_step (lua_State *l)
{
	b2WorldId world = lua_b2_world_ptr(l, 1 );
	float     time  = lua_tonumber(l,     2 );
	int       count = lua_tointeger(l,    3 );

	b2World_Step(world,time,count);

	return 0;
}

/*+---------------------------------------------------------------------

Get body events in a packed array where each event is 5 values

	bodyId
	fellAsleep
	transform.p.x
	transform.p.y
	transform.q (converted into radians)


*/
static int lua_b2_world_body_events (lua_State *l)
{
	b2WorldId world = lua_b2_world_ptr(l, 1 );

	b2BodyEvents events = b2World_GetBodyEvents(world);

	lua_newtable(l);

	for( int i=0 ; i < events.moveCount ; i++ )
	{
		b2BodyMoveEvent *event=events.moveEvents+i;

		lua_pushlstring(l, (const char *)&event->bodyId,sizeof(b2BodyId)); // id to (non printable) string
		lua_rawseti(l, -2 , i*5+1 );

		lua_pushboolean(l, event->fellAsleep );
		lua_rawseti(l, -2 , i*5+2 );

		lua_pushnumber(l, event->transform.p.x );
		lua_rawseti(l, -2 , i*5+3 );
		lua_pushnumber(l, event->transform.p.y );
		lua_rawseti(l, -2 , i*5+4 );

		lua_pushnumber(l, b2Rot_GetAngle(event->transform.q) );
		lua_rawseti(l, -2 , i*5+5 );

	}

	return 1;
}

/*+---------------------------------------------------------------------

Get sensor events in a packed array where each event is 2 values

Returns two arrays, first is begin events second is end events both
contain pairs of shape ids like so.

	sensorShapeId
	visitorShapeId


*/
static int lua_b2_world_sensor_events (lua_State *l)
{
	b2WorldId world = lua_b2_world_ptr(l, 1 );

	b2SensorEvents events = b2World_GetSensorEvents(world);

	lua_newtable(l);

	for( int i=0 ; i < events.beginCount ; i++ )
	{
		b2SensorBeginTouchEvent *event=events.beginEvents+i;

		lua_pushlstring(l, (const char *)&event->sensorShapeId,sizeof(b2ShapeId)); // id to (non printable) string
		lua_rawseti(l, -2 , i*2+1 );

		lua_pushlstring(l, (const char *)&event->visitorShapeId,sizeof(b2ShapeId)); // id to (non printable) string
		lua_rawseti(l, -2 , i*2+2 );
	}

	lua_newtable(l);

	for( int i=0 ; i < events.endCount ; i++ )
	{
		b2SensorEndTouchEvent *event=events.endEvents+i;

		lua_pushlstring(l, (const char *)&event->sensorShapeId,sizeof(b2ShapeId)); // id to (non printable) string
		lua_rawseti(l, -2 , i*2+1 );

		lua_pushlstring(l, (const char *)&event->visitorShapeId,sizeof(b2ShapeId)); // id to (non printable) string
		lua_rawseti(l, -2 , i*2+2 );
	}

	return 2;
}

/*+---------------------------------------------------------------------

Get contact events in a packed arrays

Returns three arrays, first is begin events second is end events and third is hit events.

begin events is 3 values per event

	shapeIdA
	shapeIdB
	contactId

end events is 3 values per event

	shapeIdA
	shapeIdB
	contactId

hit events is 8 values per event

	shapeIdA
	shapeIdB
	contactId
	approachSpeed
	normal.x
	normal.y
	point.x
	point.y

*/
static int lua_b2_world_contact_events (lua_State *l)
{
	b2WorldId world = lua_b2_world_ptr(l, 1 );

	b2ContactEvents events = b2World_GetContactEvents(world);

	lua_newtable(l);

	for( int i=0 ; i < events.beginCount ; i++ )
	{
		b2ContactBeginTouchEvent *event=events.beginEvents+i;
		b2ContactData contact=b2Contact_GetData( event->contactId );

		lua_pushlstring(l, (const char *)&event->shapeIdA,sizeof(b2ShapeId)); // id to (non printable) string
		lua_rawseti(l, -2 , i*3+1 );

		lua_pushlstring(l, (const char *)&event->shapeIdB,sizeof(b2ShapeId)); // id to (non printable) string
		lua_rawseti(l, -2 , i*3+2 );

		lua_pushlstring(l, (const char *)&event->contactId,sizeof(b2ContactId)); // id to (non printable) string
		lua_rawseti(l, -2 , i*3+3 );
	}

	lua_newtable(l);

	for( int i=0 ; i < events.endCount ; i++ )
	{
		b2ContactEndTouchEvent *event=events.endEvents+i;

		lua_pushlstring(l, (const char *)&event->shapeIdA,sizeof(b2ShapeId)); // id to (non printable) string
		lua_rawseti(l, -2 , i*3+1 );

		lua_pushlstring(l, (const char *)&event->shapeIdB,sizeof(b2ShapeId)); // id to (non printable) string
		lua_rawseti(l, -2 , i*3+2 );

		lua_pushlstring(l, (const char *)&event->contactId,sizeof(b2ContactId)); // id to (non printable) string
		lua_rawseti(l, -2 , i*3+3 );
	}

	lua_newtable(l);

	for( int i=0 ; i < events.hitCount ; i++ )
	{
		b2ContactHitEvent *event=events.hitEvents+i;

		lua_pushlstring(l, (const char *)&event->shapeIdA,sizeof(b2ShapeId)); // id to (non printable) string
		lua_rawseti(l, -2 , i*8+1 );

		lua_pushlstring(l, (const char *)&event->shapeIdB,sizeof(b2ShapeId)); // id to (non printable) string
		lua_rawseti(l, -2 , i*8+2 );

		lua_pushlstring(l, (const char *)&event->contactId,sizeof(b2ContactId)); // id to (non printable) string
		lua_rawseti(l, -2 , i*8+3 );

		lua_pushnumber(l, event->approachSpeed );
		lua_rawseti(l, -2 , i*8+4 );

		lua_pushnumber(l, event->normal.x );
		lua_rawseti(l, -2 , i*8+5 );
		lua_pushnumber(l, event->normal.y );
		lua_rawseti(l, -2 , i*8+6 );

		lua_pushnumber(l, event->point.x );
		lua_rawseti(l, -2 , i*8+7 );
		lua_pushnumber(l, event->point.y );
		lua_rawseti(l, -2 , i*8+8 );
	}

	return 3;
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
		lua_getfield(l,2,"motionLocks_linearX");
		if(!lua_isnil(l,-1)) { def.motionLocks.linearX = lua_toboolean(l,-1); }
		lua_pop(l,1);
		lua_getfield(l,2,"motionLocks_linearY");
		if(!lua_isnil(l,-1)) { def.motionLocks.linearY = lua_toboolean(l,-1); }
		lua_pop(l,1);
		lua_getfield(l,2,"motionLocks_angularZ");
		if(!lua_isnil(l,-1)) { def.motionLocks.angularZ = lua_toboolean(l,-1); }
		lua_pop(l,1);
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
			lua_pushinteger(l,1);	lua_gettable(l,-2);
			lua_pushinteger(l,2);	lua_gettable(l,-3);
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
			lua_pushinteger(l,1);	lua_gettable(l,-2);
			lua_pushinteger(l,2);	lua_gettable(l,-3);
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
			if(strcmp(s,"static")==0)    { def.type = b2_staticBody;    } else
			if(strcmp(s,"kinematic")==0) { def.type = b2_kinematicBody; } else
			if(strcmp(s,"dynamic")==0)   { def.type = b2_dynamicBody;   }
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

	lua_pushlstring(l,(const char *)pp,sizeof(b2BodyId)); // id to (non printable) string
	return 2;
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

Get body variables

*/
static int lua_b2_body_get (lua_State *l)
{
	b2BodyId body = lua_b2_body_ptr(l, 1 );

	lua_newtable(l);

	lua_pushstring(l, b2Body_GetName(body) );
	lua_setfield(l, -2 , "name" );

	lua_pushnumber(l, b2Body_GetLinearDamping(body) );
	lua_setfield(l, -2 , "linearDamping" );

	lua_pushnumber(l, b2Body_GetAngularDamping(body) );
	lua_setfield(l, -2 , "angularDamping" );

	lua_pushnumber(l, b2Body_GetGravityScale(body) );
	lua_setfield(l, -2 , "gravityScale" );

	lua_pushboolean(l, b2Body_IsSleepEnabled(body) );
	lua_setfield(l, -2 , "enableSleep" );

	lua_pushnumber(l, b2Body_GetSleepThreshold(body) );
	lua_setfield(l, -2 , "sleepThreshold" );

	b2MotionLocks locks = b2Body_GetMotionLocks(body);
	lua_pushboolean(l, locks.linearX );
	lua_setfield(l, -2 , "motionLocks_linearX" );
	lua_pushboolean(l, locks.linearY );
	lua_setfield(l, -2 , "motionLocks_linearY" );
	lua_pushboolean(l, locks.angularZ );
	lua_setfield(l, -2 , "motionLocks_angularZ" );

	lua_pushboolean(l, b2Body_IsBullet(body) );
	lua_setfield(l, -2 , "bullet" );

	return 1;
}


/*+---------------------------------------------------------------------

Set body variables

*/
static int lua_b2_body_set (lua_State *l)
{
	b2BodyId body = lua_b2_body_ptr(l, 1 );

	lua_getfield(l,1,"name");
	if(!lua_isnil(l,-1))
	{
		b2Body_SetName(body, lua_tostring(l,-1) );
	}
	lua_pop(l,1);

	lua_getfield(l,1,"linearDamping");
	if(!lua_isnil(l,-1))
	{
		b2Body_SetLinearDamping(body, lua_tonumber(l,-1) );
	}
	lua_pop(l,1);

	lua_getfield(l,1,"angularDamping");
	if(!lua_isnil(l,-1))
	{
		b2Body_SetAngularDamping(body, lua_tonumber(l,-1) );
	}
	lua_pop(l,1);

	lua_getfield(l,1,"gravityScale");
	if(!lua_isnil(l,-1))
	{
		b2Body_SetGravityScale(body, lua_tonumber(l,-1) );
	}
	lua_pop(l,1);

	lua_getfield(l,1,"enableSleep");
	if(!lua_isnil(l,-1))
	{
		b2Body_EnableSleep(body, lua_toboolean(l,-1) );
	}
	lua_pop(l,1);

	lua_getfield(l,1,"sleepThreshold");
	if(!lua_isnil(l,-1))
	{
		b2Body_SetSleepThreshold(body, lua_tonumber(l,-1) );
	}
	lua_pop(l,1);


	int set_locks=0;
	b2MotionLocks locks = b2Body_GetMotionLocks(body);
	lua_getfield(l,1,"motionLocks_linearX");
	if(!lua_isnil(l,-1))
	{
		set_locks=1;
		locks.linearX = lua_toboolean(l,-1) ;
	}
	lua_pop(l,1);
	lua_getfield(l,1,"motionLocks_linearY");
	if(!lua_isnil(l,-1))
	{
		set_locks=1;
		locks.linearY = lua_toboolean(l,-1) ;
	}
	lua_pop(l,1);
	lua_getfield(l,1,"motionLocks_angularZ");
	if(!lua_isnil(l,-1))
	{
		set_locks=1;
		locks.angularZ = lua_toboolean(l,-1) ;
	}
	lua_pop(l,1);
	if(set_locks) { b2Body_SetMotionLocks(body,locks); }

	lua_getfield(l,1,"bullet");
	if(!lua_isnil(l,-1))
	{
		b2Body_SetBullet(body, lua_toboolean(l,-1) );
	}
	lua_pop(l,1);

	lua_getfield(l,1,"enableContactEvents");
	if(!lua_isnil(l,-1))
	{
		b2Body_EnableContactEvents(body, lua_toboolean(l,-1) );
	}
	lua_pop(l,1);

	lua_getfield(l,1,"enableHitEvents");
	if(!lua_isnil(l,-1))
	{
		b2Body_EnableHitEvents(body, lua_toboolean(l,-1) );
	}
	lua_pop(l,1);

	return 0;
}

/*+---------------------------------------------------------------------

Get/Set body type

*/
static int lua_b2_body_type (lua_State *l)
{
int b;

	b2BodyId body = lua_b2_body_ptr(l, 1 );

	if(lua_isstring(l,2)) // only set if given
	{
		b2BodyType type;
		char *s=lua_tostring(l,2);
		if(strcmp(s,"static")==0)    { type = b2_staticBody;    } else
		if(strcmp(s,"kinematic")==0) { type = b2_kinematicBody; } else
		if(strcmp(s,"dynamic")==0)   { type = b2_dynamicBody;   }
		b2Body_SetType(body,type);
	}

	b2BodyType type = b2Body_GetType(body);
	if(type==b2_staticBody)    { lua_pushstring(l,"static");    } else
	if(type==b2_kinematicBody) { lua_pushstring(l,"kinematic"); } else
	if(type==b2_dynamicBody)   { lua_pushstring(l,"dynamic");   }

	return 1;
}

/*+---------------------------------------------------------------------

Get/Set body awake

*/
static int lua_b2_body_awake (lua_State *l)
{
int b;

	b2BodyId body = lua_b2_body_ptr(l, 1 );

	if(!lua_isnil(l,2)) // only set if given
	{
		b=lua_toboolean(l, 2 );

		b2Body_SetAwake(body,b);
	}

	b=b2Body_IsAwake(body);

	lua_pushboolean(l, b );

	return 1;
}

/*+---------------------------------------------------------------------

Get/Set body transform

*/
static int lua_b2_body_transform (lua_State *l)
{
float r;
b2Transform t;

	b2BodyId body = lua_b2_body_ptr(l, 1 );

	if(!lua_isnil(l,4)) // only set if given
	{
		t.p.x=(float)lua_tonumber(l, 2 );
		t.p.y=(float)lua_tonumber(l, 3 );
		r=(float)lua_tonumber(l, 4 );
		t.q=b2MakeRot(r);

		b2Body_SetTransform(body,t.p,t.q);
	}

	t=b2Body_GetTransform(body);

	lua_pushnumber(l, t.p.x );
	lua_pushnumber(l, t.p.y );
	lua_pushnumber(l, b2Rot_GetAngle(t.q) );

	return 3;
}

/*+---------------------------------------------------------------------

Get/Set body velocity

*/
static int lua_b2_body_velocity (lua_State *l)
{
b2Vec2 p;
float r;

	b2BodyId body = lua_b2_body_ptr(l, 1 );

	if(!lua_isnil(l,4)) // only set if given
	{
		p.x=(float)lua_tonumber(l, 2 );
		p.y=(float)lua_tonumber(l, 3 );
		r=(float)lua_tonumber(l, 4 );

		b2Body_SetLinearVelocity(body,p);
		b2Body_SetAngularVelocity(body,r);
	}

	p=b2Body_GetLinearVelocity(body);
	r=b2Body_GetAngularVelocity(body);

	lua_pushnumber(l, p.x );
	lua_pushnumber(l, p.y );
	lua_pushnumber(l, r );

	return 3;
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

	b2Filter filter;
	lua_getfield(l,2,"invokeContactCreation");
	if(!lua_isnil(l,-1)) { def.invokeContactCreation = lua_toboolean(l,-1); }
	lua_pop(l,1);

	lua_getfield(l,2,"filter_categoryBits");
	if(!lua_isnil(l,-1))
	{
		def.filter.categoryBits = lua_tonumber(l,-1) ;
	}
	lua_pop(l,1);
	lua_getfield(l,2,"filter_groupIndex");
	if(!lua_isnil(l,-1))
	{
		def.filter.categoryBits = lua_tonumber(l,-1) ;
	}
	lua_pop(l,1);
	lua_getfield(l,2,"filter_maskBits");
	if(!lua_isnil(l,-1))
	{
		def.filter.categoryBits = lua_tonumber(l,-1) ;
	}
	lua_pop(l,1);

	lua_getfield(l,2,"invokeContactCreation");
	if(!lua_isnil(l,-1)) { def.invokeContactCreation = lua_toboolean(l,-1); }
	lua_pop(l,1);

	lua_getfield(l,2,"isSensor");
	if(!lua_isnil(l,-1)) { def.isSensor = lua_toboolean(l,-1); }
	lua_pop(l,1);

	lua_getfield(l,2,"material_customColor");
	if(!lua_isnil(l,-1)) { def.material.customColor = (uint32_t)lua_tonumber(l,-1); }
	lua_pop(l,1);
	lua_getfield(l,2,"material_friction");
	if(!lua_isnil(l,-1)) { def.material.friction = (float)lua_tonumber(l,-1); }
	lua_pop(l,1);
	lua_getfield(l,2,"material_restitution");
	if(!lua_isnil(l,-1)) { def.material.restitution = (float)lua_tonumber(l,-1); }
	lua_pop(l,1);
	lua_getfield(l,2,"material_rollingResistance");
	if(!lua_isnil(l,-1)) { def.material.rollingResistance = (float)lua_tonumber(l,-1); }
	lua_pop(l,1);
	lua_getfield(l,2,"material_tangentSpeed");
	if(!lua_isnil(l,-1)) { def.material.tangentSpeed = (float)lua_tonumber(l,-1); }
	lua_pop(l,1);
	lua_getfield(l,2,"material_userMaterialId");
	if(!lua_isnil(l,-1)) { def.material.userMaterialId = (int)lua_tonumber(l,-1); }
	lua_pop(l,1);

	lua_getfield(l,2,"updateBodyMass");
	if(!lua_isnil(l,-1)) { def.updateBodyMass = lua_toboolean(l,-1); }
	lua_pop(l,1);

// create ptr ptr userdata
	pp=(b2ShapeId*)lua_newuserdata(l, sizeof(b2ShapeId));
	*pp=(b2ShapeId){0};
	luaL_getmetatable(l, lua_b2_shape_meta_name);
	lua_setmetatable(l, -2);

	body=lua_b2_body_ptr(l,1);

	// shape type defaults to circle
	char *shape_type="circle";
	lua_getfield(l,2,"shape"); // the type of shape as lowercase string
	if(lua_isstring(l,-1))
	{
		shape_type=lua_tostring(l,-1);
	}
	if(strcmp(shape_type,"circle")==0)
	{
		lua_pop(l,1); // shape_type is no longer valid

		b2Circle circle;
		lua_getfield(l,2,"center");
		if(!lua_isnil(l,-1))
		{
			lua_pushinteger(l,1);	lua_gettable(l,-2);
			lua_pushinteger(l,2);	lua_gettable(l,-3);
			circle.center = (b2Vec2){(float)lua_tonumber(l,-2),(float)lua_tonumber(l,-1)};
			lua_pop(l,2);
		}
		lua_pop(l,1);
		lua_getfield(l,2,"radius");
		if(!lua_isnil(l,-1)) { circle.radius = (float)lua_tonumber(l,-1); }
		lua_pop(l,1);

		*pp=b2CreateCircleShape(body,&def,&circle);
	}
	else
	if(strcmp(shape_type,"segment")==0)
	{
		lua_pop(l,1); // shape_type is no longer valid

		b2Segment segment;
		lua_getfield(l,2,"point1");
		if(!lua_isnil(l,-1))
		{
			lua_pushinteger(l,1);	lua_gettable(l,-2);
			lua_pushinteger(l,2);	lua_gettable(l,-3);
			segment.point1 = (b2Vec2){(float)lua_tonumber(l,-2),(float)lua_tonumber(l,-1)};
			lua_pop(l,2);
		}
		lua_pop(l,1);
		lua_getfield(l,2,"point2");
		if(!lua_isnil(l,-1))
		{
			lua_pushinteger(l,1);	lua_gettable(l,-2);
			lua_pushinteger(l,2);	lua_gettable(l,-3);
			segment.point2 = (b2Vec2){(float)lua_tonumber(l,-2),(float)lua_tonumber(l,-1)};
			lua_pop(l,2);
		}
		lua_pop(l,1);

		*pp=b2CreateSegmentShape(body,&def,&segment);
	}
	else
	if(strcmp(shape_type,"capsule")==0)
	{
		lua_pop(l,1); // shape_type is no longer valid

		b2Capsule capsule;
		lua_getfield(l,2,"center1");
		if(!lua_isnil(l,-1))
		{
			lua_pushinteger(l,1);	lua_gettable(l,-2);
			lua_pushinteger(l,2);	lua_gettable(l,-3);
			capsule.center1 = (b2Vec2){(float)lua_tonumber(l,-2),(float)lua_tonumber(l,-1)};
			lua_pop(l,2);
		}
		lua_pop(l,1);
		lua_getfield(l,2,"center2");
		if(!lua_isnil(l,-1))
		{
			lua_pushinteger(l,1);	lua_gettable(l,-2);
			lua_pushinteger(l,2);	lua_gettable(l,-3);
			capsule.center2 = (b2Vec2){(float)lua_tonumber(l,-2),(float)lua_tonumber(l,-1)};
			lua_pop(l,2);
		}
		lua_pop(l,1);
		lua_getfield(l,2,"radius");
		if(!lua_isnil(l,-1)) { capsule.radius = (float)lua_tonumber(l,-1); }
		lua_pop(l,1);

		*pp=b2CreateCapsuleShape(body,&def,&capsule);
	}
	else
	if(strcmp(shape_type,"box")==0)
	{
		lua_pop(l,1); // shape_type is no longer valid

		b2Polygon polygon;

		float halfWidth=0.0f;
		lua_getfield(l,2,"halfWidth");
		if(!lua_isnil(l,-1)) { halfWidth = (float)lua_tonumber(l,-1); }
		lua_pop(l,1);

		float halfHeight=0.0f;
		lua_getfield(l,2,"halfHeight");
		if(!lua_isnil(l,-1)) { halfHeight = (float)lua_tonumber(l,-1); }
		lua_pop(l,1);

		b2Vec2 center=(b2Vec2){0.0f,0.0f};
		lua_getfield(l,2,"center");
		if(!lua_isnil(l,-1))
		{
			lua_pushinteger(l,1);	lua_gettable(l,-2);
			lua_pushinteger(l,2);	lua_gettable(l,-3);
			center = (b2Vec2){(float)lua_tonumber(l,-2),(float)lua_tonumber(l,-1)};
			lua_pop(l,2);
		}
		lua_pop(l,1);

		float rotation=0.0f;
		lua_getfield(l,2,"rotation");
		if(!lua_isnil(l,-1)) { rotation = (float)lua_tonumber(l,-1); }
		lua_pop(l,1);

		float radius=0.0f;
		lua_getfield(l,2,"radius");
		if(!lua_isnil(l,-1)) { radius = (float)lua_tonumber(l,-1); }
		lua_pop(l,1);

		polygon=b2MakeOffsetRoundedBox(halfWidth,halfHeight,center,b2MakeRot(rotation),radius);

		*pp=b2CreatePolygonShape(body,&def,&polygon);
	}
	else
	{
		lua_pop(l,1); // shape_type is no longer valid

		lua_pushstring(l,"unknown shape type");
		lua_error(l);
	}

	lua_pushlstring(l,(const char *)pp,sizeof(b2ShapeId)); // id to (non printable) string
	return 2;
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

Get shape variables

*/
static int lua_b2_shape_get (lua_State *l)
{
	b2ShapeId shape = lua_b2_shape_ptr(l, 1 );

	lua_newtable(l);
	
	b2ShapeType type = b2Shape_GetType(shape);
	if     ( type==b2_circleShape       ) { lua_pushstring(l, "circle"       ); }
	else if( type==b2_capsuleShape      ) { lua_pushstring(l, "capsule"      ); }
	else if( type==b2_segmentShape      ) { lua_pushstring(l, "segment"      ); }
	else if( type==b2_polygonShape      ) { lua_pushstring(l, "polygon"      ); }
	else if( type==b2_chainSegmentShape ) { lua_pushstring(l, "chainSegment" ); }
	else                                  { lua_pushstring(l, "unknown"      ); }
	lua_setfield(l, -2 , "type" );

	lua_pushnumber(l, b2Shape_GetFriction(shape) );
	lua_setfield(l, -2 , "friction" );

	lua_pushnumber(l, b2Shape_GetRestitution(shape) );
	lua_setfield(l, -2 , "restitution" );

	lua_pushnumber(l, b2Shape_GetUserMaterial(shape) );
	lua_setfield(l, -2 , "material_userMaterialId" );

	b2Filter filter = b2Shape_GetFilter(shape);
	lua_pushnumber(l, filter.categoryBits );
	lua_setfield(l, -2 , "filter_categoryBits" );
	lua_pushnumber(l, filter.groupIndex );
	lua_setfield(l, -2 , "filter_groupIndex" );
	lua_pushnumber(l, filter.maskBits );
	lua_setfield(l, -2 , "filter_maskBits" );
	
	lua_pushboolean(l, b2Shape_AreSensorEventsEnabled(shape) );
	lua_setfield(l, -2 , "enableSensorEvents" );

	lua_pushboolean(l, b2Shape_AreContactEventsEnabled(shape) );
	lua_setfield(l, -2 , "enableContactEvents" );
	
	lua_pushboolean(l, b2Shape_AreHitEventsEnabled(shape) );
	lua_setfield(l, -2 , "enableHitEvents" );

	return 1;
}


/*+---------------------------------------------------------------------

Set shape variables

*/
static int lua_b2_shape_set (lua_State *l)
{
	b2ShapeId shape = lua_b2_shape_ptr(l, 1 );

	lua_getfield(l,1,"friction");
	if(!lua_isnil(l,-1))
	{
		b2Shape_SetFriction(shape, lua_tonumber(l,-1) );
	}
	lua_pop(l,1);

	lua_getfield(l,1,"restitution");
	if(!lua_isnil(l,-1))
	{
		b2Shape_SetRestitution(shape, lua_tonumber(l,-1) );
	}
	lua_pop(l,1);

	lua_getfield(l,1,"material_userMaterialId");
	if(!lua_isnil(l,-1))
	{
		b2Shape_SetUserMaterial(shape, lua_tonumber(l,-1) );
	}
	lua_pop(l,1);

	int set_filter=0;
	b2Filter filter = b2Shape_GetFilter(shape);
	lua_getfield(l,1,"filter_categoryBits");
	if(!lua_isnil(l,-1))
	{
		set_filter=1;
		filter.categoryBits = lua_tonumber(l,-1) ;
	}
	lua_pop(l,1);
	lua_getfield(l,1,"filter_groupIndex");
	if(!lua_isnil(l,-1))
	{
		set_filter=1;
		filter.categoryBits = lua_tonumber(l,-1) ;
	}
	lua_pop(l,1);
	lua_getfield(l,1,"filter_maskBits");
	if(!lua_isnil(l,-1))
	{
		set_filter=1;
		filter.categoryBits = lua_tonumber(l,-1) ;
	}
	lua_pop(l,1);
	if(set_filter) { b2Shape_SetFilter(shape,filter); }

	lua_getfield(l,1,"enableSensorEvents");
	if(!lua_isnil(l,-1))
	{
		b2Shape_EnableSensorEvents(shape, lua_toboolean(l,-1) );
	}
	lua_pop(l,1);
	
	lua_getfield(l,1,"enablePreSolveEvents");
	if(!lua_isnil(l,-1))
	{
		b2Shape_EnablePreSolveEvents(shape, lua_toboolean(l,-1) );
	}
	lua_pop(l,1);

	lua_getfield(l,1,"enableContactEvents");
	if(!lua_isnil(l,-1))
	{
		b2Shape_EnableContactEvents(shape, lua_toboolean(l,-1) );
	}
	lua_pop(l,1);

	lua_getfield(l,1,"enableHitEvents");
	if(!lua_isnil(l,-1))
	{
		b2Shape_EnableHitEvents(shape, lua_toboolean(l,-1) );
	}
	lua_pop(l,1);

	return 0;
}

/*+---------------------------------------------------------------------

joint create/destroy

*/
b2JointId * lua_b2_joint_ptr_ptr (lua_State *l,int idx)
{
b2JointId *pp;
	pp=(b2JointId*)luaL_checkudata(l, idx , lua_b2_joint_meta_name);
	return pp;
}

b2JointId   lua_b2_joint_ptr (lua_State *l,int idx)
{
b2JointId *pp;
	pp=lua_b2_joint_ptr_ptr(l,idx);
	if(!pp) { luaL_error(l,"box2d joint is null"); }
	return *pp;
}

static int lua_b2_joint_create (lua_State *l)
{
b2JointId *pp;

	b2WorldId world=lua_b2_world_ptr(l,1);

// create ptr ptr userdata
	pp=(b2JointId*)lua_newuserdata(l, sizeof(b2JointId));
	*pp=(b2JointId){0};
	luaL_getmetatable(l, lua_b2_joint_meta_name);
	lua_setmetatable(l, -2);

	b2JointDef joint;
	// get generic joint values
	lua_getfield(l,2,"bodyIdA");
	if(!lua_isnil(l,-1)) { joint.bodyIdA=*((b2BodyId*)(lua_tostring(l,-1))); }
	lua_pop(l,1);
	lua_getfield(l,2,"bodyIdB");
	if(!lua_isnil(l,-1)) { joint.bodyIdB=*((b2BodyId*)(lua_tostring(l,-1))); }
	lua_pop(l,1);
	lua_getfield(l,2,"localFrameA");
	if(!lua_isnil(l,-1))
	{
			lua_pushinteger(l,1);	lua_gettable(l,-2);
			lua_pushinteger(l,2);	lua_gettable(l,-3);
			lua_pushinteger(l,3);	lua_gettable(l,-4);
			joint.localFrameA.p = (b2Vec2){(float)lua_tonumber(l,-3),(float)lua_tonumber(l,-2)};
			joint.localFrameA.q = b2MakeRot((float)lua_tonumber(l,-1));
			lua_pop(l,3);
	}
	lua_pop(l,1);
	lua_getfield(l,2,"localFrameB");
	if(!lua_isnil(l,-1))
	{
			lua_pushinteger(l,1);	lua_gettable(l,-2);
			lua_pushinteger(l,2);	lua_gettable(l,-3);
			lua_pushinteger(l,3);	lua_gettable(l,-4);
			joint.localFrameB.p = (b2Vec2){(float)lua_tonumber(l,-3),(float)lua_tonumber(l,-2)};
			joint.localFrameB.q = b2MakeRot((float)lua_tonumber(l,-1));
			lua_pop(l,3);
	}
	lua_pop(l,1);
	lua_getfield(l,2,"forceThreshold");
	if(!lua_isnil(l,-1)) { joint.forceThreshold = (float)lua_tonumber(l,-1); }
	lua_pop(l,1);
	lua_getfield(l,2,"torqueThreshold");
	if(!lua_isnil(l,-1)) { joint.torqueThreshold = (float)lua_tonumber(l,-1); }
	lua_pop(l,1);
	lua_getfield(l,2,"constraintHertz");
	if(!lua_isnil(l,-1)) { joint.constraintHertz = (float)lua_tonumber(l,-1); }
	lua_pop(l,1);
	lua_getfield(l,2,"constraintDampingRatio");
	if(!lua_isnil(l,-1)) { joint.constraintDampingRatio = (float)lua_tonumber(l,-1); }
	lua_pop(l,1);
	lua_getfield(l,2,"collideConnected");
	if(!lua_isnil(l,-1)) { joint.collideConnected = lua_toboolean(l,-1); }
	lua_pop(l,1);

	// joint type defaults to filter
	char *joint_type="filter";
	lua_getfield(l,2,"joint"); // the type of joint as lowercase string
	if(lua_isstring(l,-1))
	{
		joint_type=lua_tostring(l,-1);
	}
	if(strcmp(joint_type,"distance")==0)
	{
		lua_pop(l,1); // joint_type is no longer valid

		b2DistanceJointDef distance=b2DefaultDistanceJointDef();
		distance.base=joint; // generic values

		lua_getfield(l,2,"length");
		if(!lua_isnil(l,-1)) { distance.length = (float)lua_tonumber(l,-1); }
		lua_pop(l,1);

		lua_getfield(l,2,"enableSpring");
		if(!lua_isnil(l,-1)) { distance.enableSpring = lua_toboolean(l,-1); }
		lua_pop(l,1);
		lua_getfield(l,2,"lowerSpringForce");
		if(!lua_isnil(l,-1)) { distance.lowerSpringForce = (float)lua_tonumber(l,-1); }
		lua_pop(l,1);
		lua_getfield(l,2,"upperSpringForce");
		if(!lua_isnil(l,-1)) { distance.upperSpringForce = (float)lua_tonumber(l,-1); }
		lua_pop(l,1);
		lua_getfield(l,2,"hertz");
		if(!lua_isnil(l,-1)) { distance.hertz = (float)lua_tonumber(l,-1); }
		lua_pop(l,1);
		lua_getfield(l,2,"dampingRatio");
		if(!lua_isnil(l,-1)) { distance.dampingRatio = (float)lua_tonumber(l,-1); }
		lua_pop(l,1);

		lua_getfield(l,2,"enableLimit");
		if(!lua_isnil(l,-1)) { distance.enableLimit = lua_toboolean(l,-1); }
		lua_pop(l,1);
		lua_getfield(l,2,"minLength");
		if(!lua_isnil(l,-1)) { distance.minLength = (float)lua_tonumber(l,-1); }
		lua_pop(l,1);
		lua_getfield(l,2,"maxLength");
		if(!lua_isnil(l,-1)) { distance.maxLength = (float)lua_tonumber(l,-1); }
		lua_pop(l,1);

		lua_getfield(l,2,"enableMotor");
		if(!lua_isnil(l,-1)) { distance.enableMotor = lua_toboolean(l,-1); }
		lua_pop(l,1);
		lua_getfield(l,2,"maxMotorForce");
		if(!lua_isnil(l,-1)) { distance.maxMotorForce = (float)lua_tonumber(l,-1); }
		lua_pop(l,1);
		lua_getfield(l,2,"motorSpeed");
		if(!lua_isnil(l,-1)) { distance.motorSpeed = (float)lua_tonumber(l,-1); }
		lua_pop(l,1);

		*pp=b2CreateDistanceJoint(world,&distance);
	}
	else
	if(strcmp(joint_type,"motor")==0)
	{
		lua_pop(l,1); // joint_type is no longer valid

		b2MotorJointDef motor=b2DefaultMotorJointDef();
		motor.base=joint; // generic values

		lua_getfield(l,2,"linearVelocity");
		if(!lua_isnil(l,-1))
		{
			lua_pushinteger(l,1);	lua_gettable(l,-2);
			lua_pushinteger(l,2);	lua_gettable(l,-3);
			motor.linearVelocity = (b2Vec2){(float)lua_tonumber(l,-2),(float)lua_tonumber(l,-1)};
			lua_pop(l,2);
		}
		lua_pop(l,1);
		lua_getfield(l,2,"maxVelocityForce");
		if(!lua_isnil(l,-1)) { motor.maxVelocityForce = (float)lua_tonumber(l,-1); }
		lua_pop(l,1);
		lua_getfield(l,2,"angularVelocity");
		if(!lua_isnil(l,-1)) { motor.angularVelocity = (float)lua_tonumber(l,-1); }
		lua_pop(l,1);
		lua_getfield(l,2,"maxVelocityTorque");
		if(!lua_isnil(l,-1)) { motor.maxVelocityTorque = (float)lua_tonumber(l,-1); }
		lua_pop(l,1);
		lua_getfield(l,2,"linearHertz");
		if(!lua_isnil(l,-1)) { motor.linearHertz = (float)lua_tonumber(l,-1); }
		lua_pop(l,1);
		lua_getfield(l,2,"linearDampingRatio");
		if(!lua_isnil(l,-1)) { motor.linearDampingRatio = (float)lua_tonumber(l,-1); }
		lua_pop(l,1);
		lua_getfield(l,2,"maxSpringForce");
		if(!lua_isnil(l,-1)) { motor.maxSpringForce = (float)lua_tonumber(l,-1); }
		lua_pop(l,1);
		lua_getfield(l,2,"angularHertz");
		if(!lua_isnil(l,-1)) { motor.angularHertz = (float)lua_tonumber(l,-1); }
		lua_pop(l,1);
		lua_getfield(l,2,"angularDampingRatio");
		if(!lua_isnil(l,-1)) { motor.angularDampingRatio = (float)lua_tonumber(l,-1); }
		lua_pop(l,1);
		lua_getfield(l,2,"maxSpringTorque");
		if(!lua_isnil(l,-1)) { motor.maxSpringTorque = (float)lua_tonumber(l,-1); }
		lua_pop(l,1);

		*pp=b2CreateMotorJoint(world,&motor);
	}
	else
	if(strcmp(joint_type,"filter")==0)
	{
		lua_pop(l,1); // joint_type is no longer valid

		b2FilterJointDef filter=b2DefaultFilterJointDef();
		filter.base=joint; // generic values

		*pp=b2CreateFilterJoint(world,&filter);
	}
	else
	if(strcmp(joint_type,"prismatic")==0)
	{
		lua_pop(l,1); // joint_type is no longer valid

		b2PrismaticJointDef prismatic=b2DefaultPrismaticJointDef();
		prismatic.base=joint; // generic values

		lua_getfield(l,2,"enableSpring");
		if(!lua_isnil(l,-1)) { prismatic.enableSpring = lua_toboolean(l,-1); }
		lua_pop(l,1);
		lua_getfield(l,2,"hertz");
		if(!lua_isnil(l,-1)) { prismatic.hertz = (float)lua_tonumber(l,-1); }
		lua_pop(l,1);
		lua_getfield(l,2,"dampingRatio");
		if(!lua_isnil(l,-1)) { prismatic.dampingRatio = (float)lua_tonumber(l,-1); }
		lua_pop(l,1);
		lua_getfield(l,2,"targetTranslation");
		if(!lua_isnil(l,-1)) { prismatic.targetTranslation = (float)lua_tonumber(l,-1); }
		lua_pop(l,1);

		lua_getfield(l,2,"enableLimit");
		if(!lua_isnil(l,-1)) { prismatic.enableLimit = lua_toboolean(l,-1); }
		lua_pop(l,1);
		lua_getfield(l,2,"lowerTranslation");
		if(!lua_isnil(l,-1)) { prismatic.lowerTranslation = (float)lua_tonumber(l,-1); }
		lua_pop(l,1);
		lua_getfield(l,2,"upperTranslation");
		if(!lua_isnil(l,-1)) { prismatic.upperTranslation = (float)lua_tonumber(l,-1); }
		lua_pop(l,1);

		lua_getfield(l,2,"enableMotor");
		if(!lua_isnil(l,-1)) { prismatic.enableMotor = lua_toboolean(l,-1); }
		lua_pop(l,1);
		lua_getfield(l,2,"maxMotorForce");
		if(!lua_isnil(l,-1)) { prismatic.maxMotorForce = (float)lua_tonumber(l,-1); }
		lua_pop(l,1);
		lua_getfield(l,2,"motorSpeed");
		if(!lua_isnil(l,-1)) { prismatic.motorSpeed = (float)lua_tonumber(l,-1); }
		lua_pop(l,1);

		*pp=b2CreatePrismaticJoint(world,&prismatic);
	}
	else
	if(strcmp(joint_type,"revolute")==0)
	{
		lua_pop(l,1); // joint_type is no longer valid

		b2RevoluteJointDef revolute=b2DefaultRevoluteJointDef();
		revolute.base=joint; // generic values

		lua_getfield(l,2,"targetAngle");
		if(!lua_isnil(l,-1)) { revolute.targetAngle = (float)lua_tonumber(l,-1); }
		lua_pop(l,1);

		lua_getfield(l,2,"enableSpring");
		if(!lua_isnil(l,-1)) { revolute.enableSpring = lua_toboolean(l,-1); }
		lua_pop(l,1);
		lua_getfield(l,2,"hertz");
		if(!lua_isnil(l,-1)) { revolute.hertz = (float)lua_tonumber(l,-1); }
		lua_pop(l,1);
		lua_getfield(l,2,"dampingRatio");
		if(!lua_isnil(l,-1)) { revolute.dampingRatio = (float)lua_tonumber(l,-1); }
		lua_pop(l,1);

		lua_getfield(l,2,"enableLimit");
		if(!lua_isnil(l,-1)) { revolute.enableLimit = lua_toboolean(l,-1); }
		lua_pop(l,1);
		lua_getfield(l,2,"lowerAngle");
		if(!lua_isnil(l,-1)) { revolute.lowerAngle = (float)lua_tonumber(l,-1); }
		lua_pop(l,1);
		lua_getfield(l,2,"upperAngle");
		if(!lua_isnil(l,-1)) { revolute.upperAngle = (float)lua_tonumber(l,-1); }
		lua_pop(l,1);

		lua_getfield(l,2,"enableMotor");
		if(!lua_isnil(l,-1)) { revolute.enableMotor = lua_toboolean(l,-1); }
		lua_pop(l,1);
		lua_getfield(l,2,"maxMotorTorque");
		if(!lua_isnil(l,-1)) { revolute.maxMotorTorque = (float)lua_tonumber(l,-1); }
		lua_pop(l,1);
		lua_getfield(l,2,"motorSpeed");
		if(!lua_isnil(l,-1)) { revolute.motorSpeed = (float)lua_tonumber(l,-1); }
		lua_pop(l,1);

		*pp=b2CreateRevoluteJoint(world,&revolute);
	}
	else
	if(strcmp(joint_type,"weld")==0)
	{
		lua_pop(l,1); // joint_type is no longer valid

		b2WeldJointDef weld=b2DefaultWeldJointDef();
		weld.base=joint; // generic values

		lua_getfield(l,2,"linearHertz");
		if(!lua_isnil(l,-1)) { weld.linearHertz = (float)lua_tonumber(l,-1); }
		lua_pop(l,1);
		lua_getfield(l,2,"angularHertz");
		if(!lua_isnil(l,-1)) { weld.angularHertz = (float)lua_tonumber(l,-1); }
		lua_pop(l,1);
		lua_getfield(l,2,"linearDampingRatio");
		if(!lua_isnil(l,-1)) { weld.linearDampingRatio = (float)lua_tonumber(l,-1); }
		lua_pop(l,1);
		lua_getfield(l,2,"angularDampingRatio");
		if(!lua_isnil(l,-1)) { weld.angularDampingRatio = (float)lua_tonumber(l,-1); }
		lua_pop(l,1);

		*pp=b2CreateWeldJoint(world,&weld);
	}
	else
	if(strcmp(joint_type,"wheel")==0)
	{
		lua_pop(l,1); // joint_type is no longer valid

		b2WheelJointDef wheel=b2DefaultWheelJointDef();
		wheel.base=joint; // generic values

		lua_getfield(l,2,"enableSpring");
		if(!lua_isnil(l,-1)) { wheel.enableSpring = lua_toboolean(l,-1); }
		lua_pop(l,1);
		lua_getfield(l,2,"hertz");
		if(!lua_isnil(l,-1)) { wheel.hertz = (float)lua_tonumber(l,-1); }
		lua_pop(l,1);
		lua_getfield(l,2,"dampingRatio");
		if(!lua_isnil(l,-1)) { wheel.dampingRatio = (float)lua_tonumber(l,-1); }
		lua_pop(l,1);

		lua_getfield(l,2,"enableLimit");
		if(!lua_isnil(l,-1)) { wheel.enableLimit = lua_toboolean(l,-1); }
		lua_pop(l,1);
		lua_getfield(l,2,"lowerTranslation");
		if(!lua_isnil(l,-1)) { wheel.lowerTranslation = (float)lua_tonumber(l,-1); }
		lua_pop(l,1);
		lua_getfield(l,2,"upperTranslation");
		if(!lua_isnil(l,-1)) { wheel.upperTranslation = (float)lua_tonumber(l,-1); }
		lua_pop(l,1);

		lua_getfield(l,2,"enableMotor");
		if(!lua_isnil(l,-1)) { wheel.enableMotor = lua_toboolean(l,-1); }
		lua_pop(l,1);
		lua_getfield(l,2,"maxMotorTorque");
		if(!lua_isnil(l,-1)) { wheel.maxMotorTorque = (float)lua_tonumber(l,-1); }
		lua_pop(l,1);
		lua_getfield(l,2,"motorSpeed");
		if(!lua_isnil(l,-1)) { wheel.motorSpeed = (float)lua_tonumber(l,-1); }
		lua_pop(l,1);

		*pp=b2CreateWheelJoint(world,&wheel);
	}
	else
	{
		lua_pop(l,1); // joint_type is no longer valid

		lua_pushstring(l,"unknown joint type");
		lua_error(l);
	}

	lua_pushlstring(l,(const char *)pp,sizeof(b2JointId)); // id to (non printable) string
	return 2;
}

static int lua_b2_joint_destroy (lua_State *l)
{
b2JointId *pp=lua_b2_joint_ptr_ptr(l, 1 );
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
		b2DestroyJoint(*pp,update);
		*pp=(b2JointId){0};
	}
	return 0;
}

/*+---------------------------------------------------------------------

Get joint variables

*/
static int lua_b2_joint_get (lua_State *l)
{
	b2JointId joint = lua_b2_joint_ptr(l, 1 );

	lua_newtable(l);

	b2JointType type = b2Joint_GetType(joint);
	if     ( type==b2_distanceJoint  ) { lua_pushstring(l, "distance"  ); }
	else if( type==b2_filterJoint    ) { lua_pushstring(l, "filter"    ); }
	else if( type==b2_motorJoint     ) { lua_pushstring(l, "motor"     ); }
	else if( type==b2_prismaticJoint ) { lua_pushstring(l, "prismatic" ); }
	else if( type==b2_revoluteJoint  ) { lua_pushstring(l, "revolute"  ); }
	else if( type==b2_weldJoint      ) { lua_pushstring(l, "weld"      ); }
	else if( type==b2_wheelJoint     ) { lua_pushstring(l, "wheel"     ); }
	else                               { lua_pushstring(l, "unknown"   ); }
	lua_setfield(l, -2 , "type" );
	
/*
	b2BodyId bodyA = b2Joint_GetBodyA(joint);
	lua_pushlstring(l,(const char *)&bodyA,sizeof(b2BodyId)); // id to (non printable) string
	lua_setfield(l, -2 , "bodyIdA" );

	b2BodyId bodyB = b2Joint_GetBodyB(joint);
	lua_pushlstring(l,(const char *)&bodyB,sizeof(b2BodyId)); // id to (non printable) string
	lua_setfield(l, -2 , "bodyIdB" );
*/

	lua_pushboolean(l, b2Joint_GetCollideConnected(joint) );
	lua_setfield(l, -2 , "collideConnected" );

	return 1;
}


/*+---------------------------------------------------------------------

Set joint variables

*/
static int lua_b2_joint_set (lua_State *l)
{
	b2JointId joint = lua_b2_joint_ptr(l, 1 );

	lua_getfield(l,1,"collideConnected");
	if(!lua_isnil(l,-1))
	{
		b2Joint_SetCollideConnected(joint, lua_toboolean(l,-1) );
	}
	lua_pop(l,1);


	return 0;
}


/*+---------------------------------------------------------------------

Lua Assert

*/
static thread_local lua_State *lua_b2_lua_state=0; // does this work?
static int lua_b2_assert (const char *condition, const char *fileName, int lineNumber)
{
	if( lua_b2_lua_state )
	{
		luaL_error( lua_b2_lua_state , "%s : %s : %d" , condition , fileName , lineNumber );
	}
	return 1;
}


/*+---------------------------------------------------------------------

open library.

*/
LUALIB_API int luaopen_box2d_core (lua_State *l)
{
	const luaL_Reg lib[] =
	{
		{"get",						lua_b2_get},
		{"set",						lua_b2_set},
		{"contact",					lua_b2_contact},

		{"world_create",			lua_b2_world_create},
		{"world_destroy",			lua_b2_world_destroy},
		{"world_get",				lua_b2_world_get},
		{"world_set",				lua_b2_world_set},
		{"world_step",				lua_b2_world_step},
		{"world_body_events",		lua_b2_world_body_events},
		{"world_sensor_events",		lua_b2_world_sensor_events},
		{"world_contact_events",	lua_b2_world_contact_events},

		{"body_create",				lua_b2_body_create},
		{"body_destroy",			lua_b2_body_destroy},
		{"body_get",				lua_b2_body_get},
		{"body_set",				lua_b2_body_set},
		{"body_type",				lua_b2_body_type},
		{"body_awake",				lua_b2_body_awake},
		{"body_transform",			lua_b2_body_transform},
		{"body_velocity",			lua_b2_body_velocity},

		{"shape_create",			lua_b2_shape_create},
		{"shape_destroy",			lua_b2_shape_destroy},
		{"shape_get",				lua_b2_shape_get},
		{"shape_set",				lua_b2_shape_set},

		{"joint_create",			lua_b2_joint_create},
		{"joint_destroy",			lua_b2_joint_destroy},
		{"joint_get",				lua_b2_joint_get},
		{"joint_set",				lua_b2_joint_set},

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

	const luaL_Reg meta_joint[] =
	{
		{"__gc",			lua_b2_joint_destroy},
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

	luaL_newmetatable(l, lua_b2_joint_meta_name);
	luaL_openlib(l, NULL, meta_joint, 0);
	lua_pop(l,1);

	lua_newtable(l);
	luaL_openlib(l, NULL, lib, 0);
	
	// lua error on box assert, this assumes one lua state per thread.
	// which is "reasonable" but could get you into trouble if you are doing something crazy.
	lua_b2_lua_state=l; // remember lua state in a thread_local
	b2SetAssertFcn( &lua_b2_assert ); // so we can have nice lua style errors
	
	return 1;
}

