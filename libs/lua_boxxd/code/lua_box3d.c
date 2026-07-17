/*

 Copyright (C) 2026 Kriss Blank < Kriss@XIXs.com >
 This file is distributed under the terms of the MIT license.
 http://en.wikipedia.org/wiki/MIT_License

Note that much of the documentation for the C functions exposed to Lua 
can be found in the associated box3d.lua file.

*/

#include <string.h>

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

// lua versions hax
#if LUA_VERSION_NUM < 502

#define lua_rawlen lua_objlen

static int lua_absindex (lua_State *l, int idx) {
	if(idx<0)
	{
		return lua_gettop(l)+1+idx;
	}
	else
	{
		return idx;
	}
}
#endif

#include "box3d/box3d.h"

//
// BTWO or BTHREE will be defined here for conditional code gen
//
#define BTHREE 3
#define BSIZE 2


/*

we can use either this string as a string identifier
or its address as a light userdata identifier, both unique

*/
const char *lua_b3_world_meta_name="b3_world*ptr";
const char *lua_b3_body_meta_name ="b3_body*ptr";
const char *lua_b3_shape_meta_name="b3_shape*ptr";
const char *lua_b3_joint_meta_name="b3_joint*ptr";

/*+---------------------------------------------------------------------

get and return b3Vec3 from table at "top" of stack

*/
static b3Vec3 lua_b3_read_b3Vec3 (lua_State *l, int top)
{
	top=lua_absindex(l,top); // use -1 for real top

	b3Vec3 v;
	lua_pushinteger(l,1);	lua_gettable(l,top);
	lua_pushinteger(l,2);	lua_gettable(l,top);
	v.x=(float)lua_tonumber(l,-2);
	v.y=(float)lua_tonumber(l,-1);
	lua_pop(l,2);
	return v;
}

/*+---------------------------------------------------------------------

push a new table to stack containing b3Vec3 values

*/
static void lua_b3_push_b3Vec3 (lua_State *l, b3Vec3 v)
{
	lua_newtable(l);
	lua_pushnumber(l, v.x );
	lua_rawseti(l, -2 , 1 );
	lua_pushnumber(l, v.y );
	lua_rawseti(l, -2 , 2 );
}

/*+---------------------------------------------------------------------

get and return b3Vec3 from table at "top" of stack

*/
static b3Transform lua_b3_read_b3Transform (lua_State *l, int top)
{
	top=lua_absindex(l,top); // use -1 for real top

	b3Transform t;
	lua_pushinteger(l,1);	lua_gettable(l,top);
	lua_pushinteger(l,2);	lua_gettable(l,top);
	lua_pushinteger(l,3);	lua_gettable(l,top);
	t.p.x=(float)lua_tonumber(l,-3);
	t.p.y=(float)lua_tonumber(l,-2);
	t.q=b3MakeRot((float)lua_tonumber(l,-1));
	lua_pop(l,3);
	return t;
}

/*+---------------------------------------------------------------------

push a new table to stack containing b3Vec3 values

*/
static void lua_b3_push_b3Transform (lua_State *l, b3Transform t)
{
	lua_newtable(l);
	lua_pushnumber(l, t.p.x );
	lua_rawseti(l, -2 , 1 );
	lua_pushnumber(l, t.p.y );
	lua_rawseti(l, -2 , 2 );
	lua_pushnumber(l, b3Rot_GetAngle(t.q) );
	lua_rawseti(l, -2 , 3 );
}

/*+---------------------------------------------------------------------

get and return b3ShapeProxy from table at "top" of stack

*/
static b3ShapeProxy lua_b3_read_b3ShapeProxy (lua_State *l, int top)
{
	top=lua_absindex(l,top); // use -1 for real top

	b3ShapeProxy s;
	lua_getfield(l,top,"radius");
	if(!lua_isnil(l,-1)) { s.radius = lua_tonumber(l,-1); }
	lua_pop(l,1);
	
	lua_getfield(l,top,"points");
	if(!lua_isnil(l,-1))
	{
		int idx=0;
		while(1)
		{
			idx=idx+1;
			lua_pushinteger(l,idx);	lua_gettable(l,-2); // can be fake array
			if( lua_isnil(l,-1) || idx>(B3_MAX_POLYGON_VERTICES*2) ) // end
			{
				lua_pop(l,1);
				s.count=(idx-1)/2;
				break;
			}
			else
			{
				if(idx&1)
				{
					s.points[(idx-1)/2].x=(float)lua_tonumber(l,-1);
				}
				else
				{
					s.points[(idx-1)/2].y=(float)lua_tonumber(l,-1);
				}
				lua_pop(l,1);
			}
		}
	}
	lua_pop(l,1);

	return s;
}



/*+---------------------------------------------------------------------

Get box3d information

*/
static int lua_b3_info (lua_State *l)
{
	lua_newtable(l);
	
	b3Version version=b3GetVersion();
	lua_newtable(l);
	lua_pushnumber(l, version.major );
	lua_rawseti(l, -2 , 1 );
	lua_pushnumber(l, version.minor );
	lua_rawseti(l, -2 , 2 );
	lua_pushnumber(l, version.revision );
	lua_rawseti(l, -2 , 3 );
	lua_setfield(l, -2 , "version" );

	lua_pushnumber(l, b3GetByteCount() );
	lua_setfield(l, -2 , "byteCount" );

	return 1;
}

/*+---------------------------------------------------------------------

Get box3d variables

*/
static int lua_b3_get (lua_State *l)
{
	lua_newtable(l);
	
	lua_pushnumber(l, b3GetLengthUnitsPerMeter() );
	lua_setfield(l, -2 , "lengthUnitsPerMeter" );

	return 1;
}

/*+---------------------------------------------------------------------

Set box3d variables

*/
static int lua_b3_set (lua_State *l)
{
	lua_getfield(l,1,"lengthUnitsPerMeter");
	if(!lua_isnil(l,-1))
	{
		b3SetLengthUnitsPerMeter( lua_tonumber(l,-1) );
	}
	lua_pop(l,1);

	return 0;
}

/*+---------------------------------------------------------------------

Get contact data given an ID

*/
static int lua_b3_contact (lua_State *l)
{
	b3ContactId id=*((b3ContactId*)(lua_tostring(l,1))); // get ID from lua string
	
	if(!b3Contact_IsValid(id)) { return 0; } // return nothing

	b3ContactData contact = b3Contact_GetData(id);

	lua_newtable(l);

	lua_b3_push_b3Vec3(l,contact.manifold.normal);
	lua_setfield(l,-2,"manifold_normal");

	lua_newtable(l);
	for( int i=0 ; i<contact.manifold.pointCount ; i++ )
	{
		b3ManifoldPoint p=contact.manifold.points[i];

		lua_newtable(l);

		lua_b3_push_b3Vec3(l,p.anchorA);
		lua_setfield(l,-2,"anchorA");

		lua_b3_push_b3Vec3(l,p.anchorB);
		lua_setfield(l,-2,"anchorB");

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

		lua_rawseti(l, -2 , i+1 );
	}
	lua_setfield(l,-2,"manifold_points");

	return 1;
}

/*+---------------------------------------------------------------------

world create/destroy

*/
b3WorldId * lua_b3_world_ptr_ptr (lua_State *l,int idx)
{
b3WorldId *pp;
	pp=(b3WorldId*)luaL_checkudata(l, idx , lua_b3_world_meta_name);
	return pp;
}

b3WorldId   lua_b3_world_ptr (lua_State *l,int idx)
{
b3WorldId *pp;
	pp=lua_b3_world_ptr_ptr(l,idx);
	if(!pp) { luaL_error(l,"box3d world is null"); }
	return *pp;
}

static int lua_b3_world_create (lua_State *l)
{
b3WorldId *pp;

	// defaults
	b3WorldDef def=b3DefaultWorldDef();

	// get def values if they are not nil
	if(lua_istable(l,1)) // got defs
	{
		lua_getfield(l,1,"gravity");
		if(!lua_isnil(l,-1)) { def.gravity=lua_b3_read_b3Vec3(l,-1); }
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

//		lua_getfield(l,1,"enableContactSoftening");
//		if(!lua_isnil(l,-1)) { def.enableContactSoftening = lua_toboolean(l,-1); }
//		lua_pop(l,1);
	}

// create ptr ptr userdata
	pp=(b3WorldId*)lua_newuserdata(l, sizeof(b3WorldId));
	*pp=(b3WorldId){0};
	luaL_getmetatable(l, lua_b3_world_meta_name);
	lua_setmetatable(l, -2);

// allocate b3WorldId
	*pp=b3CreateWorld(&def);

	lua_pushlstring(l,(const char *)pp,sizeof(b3WorldId)); // id to (non printable) string
	return 2;
}

static int lua_b3_world_destroy (lua_State *l)
{
b3WorldId *pp=lua_b3_world_ptr_ptr(l, 1 );
	if(B3_IS_NON_NULL(*pp))
	{
		b3DestroyWorld(*pp);
		*pp=(b3WorldId){0};
	}
	return 0;
}


/*+---------------------------------------------------------------------

Get world information

*/
static int lua_b3_world_info (lua_State *l)
{
	b3WorldId world = lua_b3_world_ptr(l, 1 );

	lua_newtable(l);

	b3Profile 	profile = b3World_GetProfile (world);

	lua_pushnumber(l,     profile.step );
	lua_setfield(l, -2 , "profile_step" );
	lua_pushnumber(l,     profile.pairs );
	lua_setfield(l, -2 , "profile_pairs" );
	lua_pushnumber(l,     profile.collide );
	lua_setfield(l, -2 , "profile_collide" );
	lua_pushnumber(l,     profile.solve );
	lua_setfield(l, -2 , "profile_solve" );
	lua_pushnumber(l,     profile.solverSetup );
	lua_setfield(l, -2 , "profile_solverSetup" );
	lua_pushnumber(l,     profile.constraints );
	lua_setfield(l, -2 , "profile_constraints" );
	lua_pushnumber(l,     profile.prepareConstraints );
	lua_setfield(l, -2 , "profile_prepareConstraints" );
	lua_pushnumber(l,     profile.integrateVelocities );
	lua_setfield(l, -2 , "profile_integrateVelocities" );
	lua_pushnumber(l,     profile.warmStart );
	lua_setfield(l, -2 , "profile_warmStart" );
	lua_pushnumber(l,     profile.solveImpulses );
	lua_setfield(l, -2 , "profile_solveImpulses" );
	lua_pushnumber(l,     profile.integratePositions );
	lua_setfield(l, -2 , "profile_integratePositions" );
	lua_pushnumber(l,     profile.relaxImpulses );
	lua_setfield(l, -2 , "profile_relaxImpulses" );
	lua_pushnumber(l,     profile.applyRestitution );
	lua_setfield(l, -2 , "profile_applyRestitution" );
	lua_pushnumber(l,     profile.storeImpulses );
	lua_setfield(l, -2 , "profile_storeImpulses" );
	lua_pushnumber(l,     profile.splitIslands );
	lua_setfield(l, -2 , "profile_splitIslands" );
	lua_pushnumber(l,     profile.transforms );
	lua_setfield(l, -2 , "profile_transforms" );
	lua_pushnumber(l,     profile.sensorHits );
	lua_setfield(l, -2 , "profile_sensorHits" );
	lua_pushnumber(l,     profile.jointEvents );
	lua_setfield(l, -2 , "profile_jointEvents" );
	lua_pushnumber(l,     profile.hitEvents );
	lua_setfield(l, -2 , "profile_hitEvents" );
	lua_pushnumber(l,     profile.refit );
	lua_setfield(l, -2 , "profile_refit" );
	lua_pushnumber(l,     profile.bullets );
	lua_setfield(l, -2 , "profile_bullets" );
	lua_pushnumber(l,     profile.sleepIslands );
	lua_setfield(l, -2 , "profile_sleepIslands" );
	lua_pushnumber(l,     profile.sensors );
	lua_setfield(l, -2 , "profile_sensors" );

	b3Counters 	counters = b3World_GetCounters (world);

	lua_pushnumber(l,     counters.byteCount );
	lua_setfield(l, -2 , "counters_byteCount" );
	lua_pushnumber(l,     counters.bodyCount );
	lua_setfield(l, -2 , "counters_bodyCount" );
	lua_pushnumber(l,     counters.shapeCount );
	lua_setfield(l, -2 , "counters_shapeCount" );
	lua_pushnumber(l,     counters.contactCount );
	lua_setfield(l, -2 , "counters_contactCount" );
	lua_pushnumber(l,     counters.jointCount );
	lua_setfield(l, -2 , "counters_jointCount" );
	lua_pushnumber(l,     counters.islandCount );
	lua_setfield(l, -2 , "counters_islandCount" );
	lua_pushnumber(l,     counters.stackUsed );
	lua_setfield(l, -2 , "counters_stackUsed" );
	lua_pushnumber(l,     counters.staticTreeHeight );
	lua_setfield(l, -2 , "counters_staticTreeHeight" );
	lua_pushnumber(l,     counters.treeHeight );
	lua_setfield(l, -2 , "counters_treeHeight" );
	lua_pushnumber(l,     counters.taskCount );
	lua_setfield(l, -2 , "counters_taskCount" );
	lua_pushnumber(l,     counters.awakeContactCount );
	lua_setfield(l, -2 , "counters_awakeContactCount" );
	lua_pushnumber(l,     counters.recycledContactCount );
	lua_setfield(l, -2 , "counters_recycledContactCount" );

	lua_newtable(l);
	for( int i=0 ; i<24 ; i++ )
	{
		lua_pushnumber(l,     counters.colorCounts[i] );
		lua_rawseti(l, -2 , i+1 );
	}
	lua_setfield(l, -2 , "counters_colorCounts" );
	
	return 1;
}


/*+---------------------------------------------------------------------

Get world variables

*/
static int lua_b3_world_get (lua_State *l)
{
	b3WorldId world = lua_b3_world_ptr(l, 1 );

	lua_newtable(l);

	lua_pushboolean(l, b3World_IsSleepingEnabled(world) );
	lua_setfield(l, -2 , "enableSleeping" );

	lua_pushboolean(l, b3World_IsContinuousEnabled(world) );
	lua_setfield(l, -2 , "enableContinuous" );

	lua_pushnumber(l, b3World_GetRestitutionThreshold(world) );
	lua_setfield(l, -2 , "restitutionThreshold" );

	lua_pushnumber(l, b3World_GetHitEventThreshold(world) );
	lua_setfield(l, -2 , "hitEventThreshold" );

	lua_b3_push_b3Vec3(l, b3World_GetGravity(world) );
	lua_setfield(l, -2 , "gravity" );

	lua_pushnumber(l, b3World_GetMaximumLinearSpeed(world) );
	lua_setfield(l, -2 , "maximumLinearSpeed" );

	lua_pushboolean(l, b3World_IsWarmStartingEnabled(world) );
	lua_setfield(l, -2 , "enableWarmStarting" );


	return 1;
}

/*+---------------------------------------------------------------------

Set world variables

*/
static int lua_b3_world_set (lua_State *l)
{
	b3WorldId world = lua_b3_world_ptr(l, 1 );

	lua_getfield(l,1,"enableSleeping");
	if(!lua_isnil(l,-1))
	{
		b3World_EnableSleeping(world, lua_toboolean(l,-1) );
	}
	lua_pop(l,1);

	lua_getfield(l,1,"enableContinuous");
	if(!lua_isnil(l,-1))
	{
		b3World_EnableContinuous(world, lua_toboolean(l,-1) );
	}
	lua_pop(l,1);

	lua_getfield(l,1,"restitutionThreshold");
	if(!lua_isnil(l,-1))
	{
		b3World_SetRestitutionThreshold(world, lua_tonumber(l,-1) );
	}
	lua_pop(l,1);

	lua_getfield(l,1,"hitEventThreshold");
	if(!lua_isnil(l,-1))
	{
		b3World_SetHitEventThreshold(world, lua_tonumber(l,-1) );
	}
	lua_pop(l,1);

	lua_getfield(l,1,"gravity");
	if(!lua_isnil(l,-1))
	{
		b3World_SetGravity(world, lua_b3_read_b3Vec3(l,-1) );
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
		b3World_SetContactTuning(world, hertz , dampingRatio , pushSpeed );
	}
	lua_pop(l,1);

	lua_getfield(l,1,"maximumLinearSpeed");
	if(!lua_isnil(l,-1))
	{
		b3World_SetMaximumLinearSpeed(world, lua_tonumber(l,-1) );
	}
	lua_pop(l,1);

	lua_getfield(l,1,"enableWarmStarting");
	if(!lua_isnil(l,-1))
	{
		b3World_EnableWarmStarting(world, lua_toboolean(l,-1) );
	}
	lua_pop(l,1);

	return 0;
}

/*+---------------------------------------------------------------------

Move the world through time itself.

*/
static int lua_b3_world_step (lua_State *l)
{
	b3WorldId world = lua_b3_world_ptr(l, 1 );
	float     time  = lua_tonumber(l,     2 );
	int       count = lua_tointeger(l,    3 );

	b3World_Step(world,time,count);

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
static int lua_b3_world_body_events (lua_State *l)
{
	b3WorldId world = lua_b3_world_ptr(l, 1 );

	b3BodyEvents events = b3World_GetBodyEvents(world);

	lua_newtable(l);

	for( int i=0 ; i < events.moveCount ; i++ )
	{
		b3BodyMoveEvent *event=events.moveEvents+i;

		lua_pushlstring(l, (const char *)&event->bodyId,sizeof(b3BodyId)); // id to (non printable) string
		lua_rawseti(l, -2 , i*5+1 );

		lua_pushboolean(l, event->fellAsleep );
		lua_rawseti(l, -2 , i*5+2 );

		lua_pushnumber(l, event->transform.p.x );
		lua_rawseti(l, -2 , i*5+3 );
		lua_pushnumber(l, event->transform.p.y );
		lua_rawseti(l, -2 , i*5+4 );

		lua_pushnumber(l, b3Rot_GetAngle(event->transform.q) );
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
static int lua_b3_world_sensor_events (lua_State *l)
{
	b3WorldId world = lua_b3_world_ptr(l, 1 );

	b3SensorEvents events = b3World_GetSensorEvents(world);

	lua_newtable(l);

	for( int i=0 ; i < events.beginCount ; i++ )
	{
		b3SensorBeginTouchEvent *event=events.beginEvents+i;

		lua_pushlstring(l, (const char *)&event->sensorShapeId,sizeof(b3ShapeId)); // id to (non printable) string
		lua_rawseti(l, -2 , i*2+1 );

		lua_pushlstring(l, (const char *)&event->visitorShapeId,sizeof(b3ShapeId)); // id to (non printable) string
		lua_rawseti(l, -2 , i*2+2 );
	}

	lua_newtable(l);

	for( int i=0 ; i < events.endCount ; i++ )
	{
		b3SensorEndTouchEvent *event=events.endEvents+i;

		lua_pushlstring(l, (const char *)&event->sensorShapeId,sizeof(b3ShapeId)); // id to (non printable) string
		lua_rawseti(l, -2 , i*2+1 );

		lua_pushlstring(l, (const char *)&event->visitorShapeId,sizeof(b3ShapeId)); // id to (non printable) string
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
static int lua_b3_world_contact_events (lua_State *l)
{
	b3WorldId world = lua_b3_world_ptr(l, 1 );

	b3ContactEvents events = b3World_GetContactEvents(world);

	lua_newtable(l);

	for( int i=0 ; i < events.beginCount ; i++ )
	{
		b3ContactBeginTouchEvent *event=events.beginEvents+i;
		b3ContactData contact=b3Contact_GetData( event->contactId );

		lua_pushlstring(l, (const char *)&event->shapeIdA,sizeof(b3ShapeId)); // id to (non printable) string
		lua_rawseti(l, -2 , i*3+1 );

		lua_pushlstring(l, (const char *)&event->shapeIdB,sizeof(b3ShapeId)); // id to (non printable) string
		lua_rawseti(l, -2 , i*3+2 );

		lua_pushlstring(l, (const char *)&event->contactId,sizeof(b3ContactId)); // id to (non printable) string
		lua_rawseti(l, -2 , i*3+3 );
	}

	lua_newtable(l);

	for( int i=0 ; i < events.endCount ; i++ )
	{
		b3ContactEndTouchEvent *event=events.endEvents+i;

		lua_pushlstring(l, (const char *)&event->shapeIdA,sizeof(b3ShapeId)); // id to (non printable) string
		lua_rawseti(l, -2 , i*3+1 );

		lua_pushlstring(l, (const char *)&event->shapeIdB,sizeof(b3ShapeId)); // id to (non printable) string
		lua_rawseti(l, -2 , i*3+2 );

		lua_pushlstring(l, (const char *)&event->contactId,sizeof(b3ContactId)); // id to (non printable) string
		lua_rawseti(l, -2 , i*3+3 );
	}

	lua_newtable(l);

	for( int i=0 ; i < events.hitCount ; i++ )
	{
		b3ContactHitEvent *event=events.hitEvents+i;

		lua_pushlstring(l, (const char *)&event->shapeIdA,sizeof(b3ShapeId)); // id to (non printable) string
		lua_rawseti(l, -2 , i*8+1 );

		lua_pushlstring(l, (const char *)&event->shapeIdB,sizeof(b3ShapeId)); // id to (non printable) string
		lua_rawseti(l, -2 , i*8+2 );

		lua_pushlstring(l, (const char *)&event->contactId,sizeof(b3ContactId)); // id to (non printable) string
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

helper ray functions

*/
static void lua_b3_world_cast_cb_push(b3ShapeId shapeId, b3Vec3 point, b3Vec3 normal, float fraction, void *context)
{
	lua_State *l=(lua_State *)context;

	int len=(int)lua_rawlen(l,-1); // *expect* an array here

	lua_newtable(l); // single result

	lua_pushlstring(l, (const char *)&shapeId,sizeof(b3ShapeId)); // id to (non printable) string
	lua_setfield(l, -2 , "shapeId" );

	lua_b3_push_b3Vec3(l,point);
	lua_setfield(l, -2 , "point" );

	lua_b3_push_b3Vec3(l,normal);
	lua_setfield(l, -2 , "normal" );

	lua_pushnumber(l, fraction );
	lua_setfield(l, -2 , "fraction" );

	lua_rawseti(l, -2 , len+1 ); // append result to array

}
static float lua_b3_world_cast_cb_all(b3ShapeId shapeId, b3Vec3 point, b3Vec3 normal, float fraction, void *context)
{
	lua_b3_world_cast_cb_push(shapeId,point,normal,fraction,context);
	return 1.0f;
}

/*+---------------------------------------------------------------------

cast a ray or shape through the world

*/
static int lua_b3_world_cast (lua_State *l)
{
	b3WorldId world = lua_b3_world_ptr(l, 1 );
	
	b3Vec3 origin=(b3Vec3){0,0};
	b3Vec3 translation=(b3Vec3){0,0};
	b3QueryFilter filter=(b3QueryFilter){1,0xFFFFFFFFFFFFFFFFULL};

	lua_getfield(l,2,"origin");
	if(!lua_isnil(l,-1)) { origin = lua_b3_read_b3Vec3(l,-1); }
	lua_pop(l,1);

	lua_getfield(l,2,"translation");
	if(!lua_isnil(l,-1)) { translation = lua_b3_read_b3Vec3(l,-1); }
	lua_pop(l,1);

	lua_getfield(l,2,"filter_categoryBits");
	if(!lua_isnil(l,-1)) { filter.categoryBits = lua_tonumber(l,-1); }
	lua_pop(l,1);
	lua_getfield(l,2,"filter_maskBits");
	if(!lua_isnil(l,-1)) { filter.maskBits = lua_tonumber(l,-1); }
	lua_pop(l,1);
		
	b3ShapeProxy shape={0};
	b3ShapeProxy *pshape=0; // use ptr as a flag
	lua_getfield(l,2,"points"); // use points as a flag
	if(!lua_isnil(l,-1))
	{
		lua_pop(l,1);
		shape = lua_b3_read_b3ShapeProxy(l,2);
		pshape=&shape;
	}
	else
	{
		lua_pop(l,1);
	}
	
	lua_pop(l,1);
	lua_newtable(l); // return hits table

	b3TreeStats stats;
	
	lua_newtable(l); // return hits table
	if(pshape) // shape cast
	{
		stats = b3World_CastShape(world,origin,pshape,translation,filter,lua_b3_world_cast_cb_all,l);
	}
	else
	{
		stats = b3World_CastRay(world,origin,translation,filter,lua_b3_world_cast_cb_all,l);
	}

	// include stats in hits table
	lua_pushnumber(l, stats.leafVisits );
	lua_setfield(l, -2 , "leafVisits" );
	lua_pushnumber(l, stats.nodeVisits );
	lua_setfield(l, -2 , "nodeVisits" );

	return 1;
}

/*+---------------------------------------------------------------------

helper overlap functions

*/
static void lua_b3_world_overlap_cb_push(b3ShapeId shapeId, void *context)
{
	lua_State *l=(lua_State *)context;

	int len=(int)lua_rawlen(l,-1); // *expect* an array here

	lua_pushlstring(l, (const char *)&shapeId,sizeof(b3ShapeId)); // id to (non printable) string
	lua_rawseti(l, -2 , len+1 ); // append to array

}
static bool lua_b3_world_overlap_cb_all(b3ShapeId shapeId, void *context)
{
	lua_b3_world_overlap_cb_push(shapeId,context);
	return 1;
}

/*+---------------------------------------------------------------------

get shapes overlapping an aabb or a shape

*/
static int lua_b3_world_overlap (lua_State *l)
{
	b3WorldId world = lua_b3_world_ptr(l, 1 );
	
	b3QueryFilter filter=(b3QueryFilter){1,0xFFFFFFFFFFFFFFFFULL};
	b3AABB aabb=(b3AABB){0.0f,0.0f,0.0f,0.0f};

	b3ShapeProxy shape={0};
	b3ShapeProxy *pshape=0; // use ptr as a flag
	lua_getfield(l,2,"points"); // use points as a flag
	if(!lua_isnil(l,-1))
	{
		lua_pop(l,1);
		shape = lua_b3_read_b3ShapeProxy(l,2);
		pshape=&shape;
	}
	else
	{
		lua_pop(l,1);
		lua_getfield(l,2,"lowerBound");
		if(!lua_isnil(l,-1)) { aabb.lowerBound = lua_b3_read_b3Vec3(l,-1); }
		lua_pop(l,1);
		lua_getfield(l,2,"upperBound");
		if(!lua_isnil(l,-1)) { aabb.upperBound = lua_b3_read_b3Vec3(l,-1); }
		lua_pop(l,1);
	}

	b3Vec3 origin=(b3Vec3){0,0};

	lua_getfield(l,2,"origin");
	if(!lua_isnil(l,-1)) { origin = lua_b3_read_b3Vec3(l,-1); }
	lua_pop(l,1);

	lua_getfield(l,2,"filter_categoryBits");
	if(!lua_isnil(l,-1)) { filter.categoryBits = lua_tonumber(l,-1); }
	lua_pop(l,1);
	lua_getfield(l,2,"filter_maskBits");
	if(!lua_isnil(l,-1)) { filter.maskBits = lua_tonumber(l,-1); }
	lua_pop(l,1);

	lua_newtable(l); // return table

	b3TreeStats r;
	if(pshape)
	{
		lua_newtable(l); // list of shapeIds
		r=b3World_OverlapShape(world,origin,pshape,filter,lua_b3_world_overlap_cb_all,l);
		lua_setfield(l, -2 , "shapeIds" );
	}
	else
	{
		lua_newtable(l); // list of shapeIds
		r=b3World_OverlapAABB(world,origin,aabb,filter,lua_b3_world_overlap_cb_all,l);
		lua_setfield(l, -2 , "shapeIds" );
	}

	// include stats
	lua_pushnumber(l, r.leafVisits );
	lua_setfield(l, -2 , "leafVisits" );
	lua_pushnumber(l, r.nodeVisits );
	lua_setfield(l, -2 , "nodeVisits" );

	return 1;
}


/*+---------------------------------------------------------------------

body create/destroy

*/
b3BodyId * lua_b3_body_ptr_ptr (lua_State *l,int idx)
{
b3BodyId *pp;
	pp=(b3BodyId*)luaL_checkudata(l, idx , lua_b3_body_meta_name);
	return pp;
}

b3BodyId   lua_b3_body_ptr (lua_State *l,int idx)
{
b3BodyId *pp;
	pp=lua_b3_body_ptr_ptr(l,idx);
	if(!pp) { luaL_error(l,"box3d body is null"); }
	return *pp;
}

static int lua_b3_body_create (lua_State *l)
{
b3WorldId world;
b3BodyId *pp;

// defaults
	b3BodyDef def=b3DefaultBodyDef();

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
		if(!lua_isnil(l,-1)) { def.linearVelocity=lua_b3_read_b3Vec3(l,-1); }
		lua_pop(l,1);
		lua_getfield(l,2,"name");
		if(!lua_isnil(l,-1)) { def.name = lua_tostring(l,-1); }
		lua_pop(l,1);

		lua_getfield(l,2,"transform"); // position and rotation combined
		if(!lua_isnil(l,-1))
		{
			b3Transform t=lua_b3_read_b3Transform(l,-1);
			def.position=t.p;
			def.rotation=t.q;
		}
		lua_pop(l,1);
		lua_getfield(l,2,"position");
		if(!lua_isnil(l,-1)) { def.position=lua_b3_read_b3Vec3(l,-1); }
		lua_pop(l,1);
		lua_getfield(l,2,"rotation");
		if(!lua_isnil(l,-1)) { def.rotation = b3MakeRot((float)lua_tonumber(l,-1)); }
		lua_pop(l,1);

		lua_getfield(l,2,"sleepThreshold");
		if(!lua_isnil(l,-1)) { def.sleepThreshold = (float)lua_tonumber(l,-1); }
		lua_pop(l,1);
		lua_getfield(l,2,"type");
		if(lua_isstring(l,-1))
		{
			const char *s=lua_tostring(l,-1);
			if(strcmp(s,"static")==0)    { def.type = b3_staticBody;    } else
			if(strcmp(s,"kinematic")==0) { def.type = b3_kinematicBody; } else
			if(strcmp(s,"dynamic")==0)   { def.type = b3_dynamicBody;   }
		}
		lua_pop(l,1);
	}

// create ptr ptr userdata
	pp=(b3BodyId*)lua_newuserdata(l, sizeof(b3BodyId));
	*pp=(b3BodyId){0};
	luaL_getmetatable(l, lua_b3_body_meta_name);
	lua_setmetatable(l, -2);

	world=lua_b3_world_ptr(l,1);

// allocate b3BodyId
	*pp=b3CreateBody(world,&def);

	lua_pushlstring(l,(const char *)pp,sizeof(b3BodyId)); // id to (non printable) string
	return 2;
}

static int lua_b3_body_destroy (lua_State *l)
{
b3BodyId *pp=lua_b3_body_ptr_ptr(l, 1 );
	if(B3_IS_NON_NULL(*pp))
	{
		b3DestroyBody(*pp);
		*pp=(b3BodyId){0};
	}
	return 0;
}


/*+---------------------------------------------------------------------

Get body information

*/
static int lua_b3_body_info (lua_State *l)
{
	b3BodyId body = lua_b3_body_ptr(l, 1 );

	lua_newtable(l);

	return 1;
}


/*+---------------------------------------------------------------------

Get body variables

*/
static int lua_b3_body_get (lua_State *l)
{
	b3BodyId body = lua_b3_body_ptr(l, 1 );

	lua_newtable(l);

	lua_pushstring(l, b3Body_GetName(body) );
	lua_setfield(l, -2 , "name" );

	lua_pushnumber(l, b3Body_GetLinearDamping(body) );
	lua_setfield(l, -2 , "linearDamping" );

	lua_pushnumber(l, b3Body_GetAngularDamping(body) );
	lua_setfield(l, -2 , "angularDamping" );

	lua_pushnumber(l, b3Body_GetGravityScale(body) );
	lua_setfield(l, -2 , "gravityScale" );

	lua_pushboolean(l, b3Body_IsSleepEnabled(body) );
	lua_setfield(l, -2 , "enableSleep" );

	lua_pushnumber(l, b3Body_GetSleepThreshold(body) );
	lua_setfield(l, -2 , "sleepThreshold" );

	b3MotionLocks locks = b3Body_GetMotionLocks(body);
	lua_pushboolean(l, locks.linearX );
	lua_setfield(l, -2 , "motionLocks_linearX" );
	lua_pushboolean(l, locks.linearY );
	lua_setfield(l, -2 , "motionLocks_linearY" );
	lua_pushboolean(l, locks.angularZ );
	lua_setfield(l, -2 , "motionLocks_angularZ" );

	lua_pushboolean(l, b3Body_IsBullet(body) );
	lua_setfield(l, -2 , "bullet" );

	return 1;
}


/*+---------------------------------------------------------------------

Set body variables

*/
static int lua_b3_body_set (lua_State *l)
{
	b3BodyId body = lua_b3_body_ptr(l, 1 );

	lua_getfield(l,2,"name");
	if(!lua_isnil(l,-1))
	{
		b3Body_SetName(body, lua_tostring(l,-1) );
	}
	lua_pop(l,1);

	lua_getfield(l,2,"linearDamping");
	if(!lua_isnil(l,-1))
	{
		b3Body_SetLinearDamping(body, lua_tonumber(l,-1) );
	}
	lua_pop(l,1);

	lua_getfield(l,2,"angularDamping");
	if(!lua_isnil(l,-1))
	{
		b3Body_SetAngularDamping(body, lua_tonumber(l,-1) );
	}
	lua_pop(l,1);

	lua_getfield(l,2,"gravityScale");
	if(!lua_isnil(l,-1))
	{
		b3Body_SetGravityScale(body, lua_tonumber(l,-1) );
	}
	lua_pop(l,1);

	lua_getfield(l,2,"enableSleep");
	if(!lua_isnil(l,-1))
	{
		b3Body_EnableSleep(body, lua_toboolean(l,-1) );
	}
	lua_pop(l,1);

	lua_getfield(l,2,"sleepThreshold");
	if(!lua_isnil(l,-1))
	{
		b3Body_SetSleepThreshold(body, lua_tonumber(l,-1) );
	}
	lua_pop(l,1);


	int set_locks=0;
	b3MotionLocks locks = b3Body_GetMotionLocks(body);
	lua_getfield(l,2,"motionLocks_linearX");
	if(!lua_isnil(l,-1))
	{
		set_locks=1;
		locks.linearX = lua_toboolean(l,-1) ;
	}
	lua_pop(l,1);
	lua_getfield(l,2,"motionLocks_linearY");
	if(!lua_isnil(l,-1))
	{
		set_locks=1;
		locks.linearY = lua_toboolean(l,-1) ;
	}
	lua_pop(l,1);
	lua_getfield(l,2,"motionLocks_angularZ");
	if(!lua_isnil(l,-1))
	{
		set_locks=1;
		locks.angularZ = lua_toboolean(l,-1) ;
	}
	lua_pop(l,1);
	if(set_locks) { b3Body_SetMotionLocks(body,locks); }

	lua_getfield(l,2,"bullet");
	if(!lua_isnil(l,-1))
	{
		b3Body_SetBullet(body, lua_toboolean(l,-1) );
	}
	lua_pop(l,1);

	lua_getfield(l,2,"enableContactEvents");
	if(!lua_isnil(l,-1))
	{
		b3Body_EnableContactEvents(body, lua_toboolean(l,-1) );
	}
	lua_pop(l,1);

	lua_getfield(l,2,"enableHitEvents");
	if(!lua_isnil(l,-1))
	{
		b3Body_EnableHitEvents(body, lua_toboolean(l,-1) );
	}
	lua_pop(l,1);

	return 0;
}

/*+---------------------------------------------------------------------

Get/Set body type

*/
static int lua_b3_body_type (lua_State *l)
{
int b;

	b3BodyId body = lua_b3_body_ptr(l, 1 );

	if(lua_isstring(l,2)) // only set if given
	{
		b3BodyType type;
		const char *s=lua_tostring(l,2);
		if(strcmp(s,"static")==0)    { type = b3_staticBody;    } else
		if(strcmp(s,"kinematic")==0) { type = b3_kinematicBody; } else
		if(strcmp(s,"dynamic")==0)   { type = b3_dynamicBody;   }
		b3Body_SetType(body,type);
	}

	b3BodyType type = b3Body_GetType(body);
	if(type==b3_staticBody)    { lua_pushstring(l,"static");    } else
	if(type==b3_kinematicBody) { lua_pushstring(l,"kinematic"); } else
	if(type==b3_dynamicBody)   { lua_pushstring(l,"dynamic");   }

	return 1;
}

/*+---------------------------------------------------------------------

Get/Set body awake

*/
static int lua_b3_body_awake (lua_State *l)
{
int b;

	b3BodyId body = lua_b3_body_ptr(l, 1 );

	if(!lua_isnil(l, 2 )) // only set if given
	{
		b=lua_toboolean(l, 2 );

		b3Body_SetAwake(body,b);
	}

	b=b3Body_IsAwake(body);

	lua_pushboolean(l, b );

	return 1;
}

/*+---------------------------------------------------------------------

Get/Set body transform

*/
static int lua_b3_body_transform (lua_State *l)
{
float r;
b3Transform t;

	b3BodyId body = lua_b3_body_ptr(l, 1 );

	if(!lua_isnil(l,4)) // only set if given
	{
		t.p.x=(float)lua_tonumber(l, 2 );
		t.p.y=(float)lua_tonumber(l, 3 );
		r=(float)lua_tonumber(l, 4 );
		t.q=b3MakeRot(r);

		b3Body_SetTransform(body,t.p,t.q);
	}

	t=b3Body_GetTransform(body);

	lua_pushnumber(l, t.p.x );
	lua_pushnumber(l, t.p.y );
	lua_pushnumber(l, b3Rot_GetAngle(t.q) );

	return 3;
}

/*+---------------------------------------------------------------------

Get/Set body velocity

*/
static int lua_b3_body_velocity (lua_State *l)
{
b3Vec3 p;
float r;

	b3BodyId body = lua_b3_body_ptr(l, 1 );

	if(!lua_isnil(l,4)) // only set if given
	{
		p.x=(float)lua_tonumber(l, 2 );
		p.y=(float)lua_tonumber(l, 3 );
		r=(float)lua_tonumber(l, 4 );

		b3Body_SetLinearVelocity(body,p);
		b3Body_SetAngularVelocity(body,r);
	}

	p=b3Body_GetLinearVelocity(body);
	r=b3Body_GetAngularVelocity(body);

	lua_pushnumber(l, p.x );
	lua_pushnumber(l, p.y );
	lua_pushnumber(l, r );

	return 3;
}


/*+---------------------------------------------------------------------

Set body force this is not accumulative, it is a reset of the current 
force that will be used on this body at the next update.

*/
static int lua_b3_body_force (lua_State *l)
{
	b3BodyId body = lua_b3_body_ptr(l, 1 );

	b3Body_ClearForces(body); // always clear force and torque


	if(!lua_isnil(l,3)) // only set if given
	{
		b3Vec3 p;
		p.x=(float)lua_tonumber(l, 2 );
		p.y=(float)lua_tonumber(l, 3 );

		b3Body_ApplyForceToCenter(body,p,1);
	}
	if(!lua_isnil(l,4)) // only set torque if given ( 0 if missing )
	{
		float r=(float)lua_tonumber(l, 4 );
		
		b3Body_ApplyTorque(body,r,1);
	}

	return 0;
}


/*+---------------------------------------------------------------------

Set body force this is not accumulative, it is a reset of the current 
force that will be used on this body at the next update.

*/
static int lua_b3_body_acceleration (lua_State *l)
{
	b3BodyId body = lua_b3_body_ptr(l, 1 );

	b3Body_ClearForces(body); // always clear force and torque

	if(!lua_isnil(l,3)) // only set if given
	{
		b3Vec3 p;
		float mass=b3Body_GetMass(body);
		p.x=mass*(float)lua_tonumber(l, 2 );
		p.y=mass*(float)lua_tonumber(l, 3 );

		b3Body_ApplyForceToCenter(body,p,1);
	}
	if(!lua_isnil(l,4)) // only set torque if given ( 0 if missing )
	{
		float rotational_inertia=b3Body_GetRotationalInertia(body);
		float r=rotational_inertia*(float)lua_tonumber(l, 4 );
		
		b3Body_ApplyTorque(body,r,1);
	}

	return 0;
}

/*+---------------------------------------------------------------------

get/set mass

Note that this function will fight auto mass calculation from shapes.

*/
static int lua_b3_body_mass (lua_State *l)
{
	b3BodyId body = lua_b3_body_ptr(l, 1 );
	
	b3MassData md=b3Body_GetMassData(body);
	int changed=0;

	if(!lua_isnil(l,2)) // only set if given
	{
		changed=1;
		md.mass=(float)lua_tonumber(l, 2 );
	}
	if(!lua_isnil(l,3)) // only set if given
	{
		changed=1;
		md.rotationalInertia=(float)lua_tonumber(l, 3 );
	}
	if(!lua_isnil(l,5)) // only set if given
	{
		changed=1;
		md.center.x=(float)lua_tonumber(l, 4 );
		md.center.y=(float)lua_tonumber(l, 5 );
	}
	if(changed)
	{
		b3Body_SetMassData(body,md);
	}

	lua_pushnumber(l, md.mass );
	lua_pushnumber(l, md.rotationalInertia );
	lua_pushnumber(l, md.center.x );
	lua_pushnumber(l, md.center.y );

	return 4;
}

/*+---------------------------------------------------------------------

Convert a vector into a vector of a different space

hex conversion code is encoded like so, first two digits are input, 
last digit is output.

	1=point
	2=vector
	3=local
	4=world
	5=velocity

*/
static int lua_b3_body_convert (lua_State *l)
{
	b3BodyId body = lua_b3_body_ptr(l, 1 );

	b3Vec3 pa;
	b3Vec3 pb;

	pa.x=(float)lua_tonumber(l, 2 );
	pa.y=(float)lua_tonumber(l, 3 );

	switch( (int)lua_tonumber(l, 4 ) )
	{
		case 0x134: // point_local_to_world
			pb=b3Body_GetWorldPoint(body,pa);
		break;
		case 0x143: // point_world_to_local
			pb=b3Body_GetLocalPoint(body,pa);
		break;
		case 0x234: // vector_local_to_world
			pb=b3Body_GetWorldVector(body,pa);
		break;
		case 0x243: // vector_world_to_local
			pb=b3Body_GetLocalVector(body,pa);
		break;
		case 0x135: // point_local_to_velocity
			pb=b3Body_GetLocalPointVelocity(body,pa);
		break;
		case 0x145: // point_world_to_velocity
			pb=b3Body_GetWorldPointVelocity(body,pa);
		break;
	}

	lua_pushnumber(l, pb.x );
	lua_pushnumber(l, pb.y );

	return 2;
}

/*+---------------------------------------------------------------------

get body aabb lower then upper values

*/
static int lua_b3_body_aabb (lua_State *l)
{
	b3BodyId body = lua_b3_body_ptr(l, 1 );

	b3AABB aabb=b3Body_ComputeAABB(body);

	lua_pushnumber(l, aabb.lowerBound.x );
	lua_pushnumber(l, aabb.lowerBound.y );
	lua_pushnumber(l, aabb.upperBound.x );
	lua_pushnumber(l, aabb.upperBound.y );

	return 4;
}

/*+---------------------------------------------------------------------

shape create/destroy

*/
b3ShapeId * lua_b3_shape_ptr_ptr (lua_State *l,int idx)
{
b3ShapeId *pp;
	pp=(b3ShapeId*)luaL_checkudata(l, idx , lua_b3_shape_meta_name);
	return pp;
}

b3ShapeId   lua_b3_shape_ptr (lua_State *l,int idx)
{
b3ShapeId *pp;
	pp=lua_b3_shape_ptr_ptr(l,idx);
	if(!pp) { luaL_error(l,"box3d shape is null"); }
	return *pp;
}

static int lua_b3_shape_create (lua_State *l)
{
b3BodyId body;
b3ShapeId *pp;

// defaults
	b3ShapeDef def=b3DefaultShapeDef();

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

//	b3Filter filter;
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
		def.filter.groupIndex = lua_tonumber(l,-1) ;
	}
	lua_pop(l,1);
	lua_getfield(l,2,"filter_maskBits");
	if(!lua_isnil(l,-1))
	{
		def.filter.maskBits = lua_tonumber(l,-1) ;
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
	pp=(b3ShapeId*)lua_newuserdata(l, sizeof(b3ShapeId));
	*pp=(b3ShapeId){0};
	luaL_getmetatable(l, lua_b3_shape_meta_name);
	lua_setmetatable(l, -2);

	body=lua_b3_body_ptr(l,1);

	// shape type defaults to circle
	const char *shape_type="circle";
	lua_getfield(l,2,"shape"); // the type of shape as lowercase string
	if(lua_isstring(l,-1))
	{
		shape_type=lua_tostring(l,-1);
	}
	if(strcmp(shape_type,"circle")==0)
	{
		lua_pop(l,1); // shape_type is no longer valid

		b3Circle circle;
		lua_getfield(l,2,"center");
		if(!lua_isnil(l,-1)) { circle.center=lua_b3_read_b3Vec3(l,-1); }
		lua_pop(l,1);
		lua_getfield(l,2,"radius");
		if(!lua_isnil(l,-1)) { circle.radius = (float)lua_tonumber(l,-1); }
		lua_pop(l,1);

		*pp=b3CreateCircleShape(body,&def,&circle);
	}
	else
	if(strcmp(shape_type,"segment")==0)
	{
		lua_pop(l,1); // shape_type is no longer valid

		b3Segment segment;
		lua_getfield(l,2,"point1");
		if(!lua_isnil(l,-1)) { segment.point1=lua_b3_read_b3Vec3(l,-1); }
		lua_pop(l,1);
		lua_getfield(l,2,"point2");
		if(!lua_isnil(l,-1)) { segment.point2=lua_b3_read_b3Vec3(l,-1); }
		lua_pop(l,1);

		*pp=b3CreateSegmentShape(body,&def,&segment);
	}
	else
	if(strcmp(shape_type,"capsule")==0)
	{
		lua_pop(l,1); // shape_type is no longer valid

		b3Capsule capsule;
		lua_getfield(l,2,"center1");
		if(!lua_isnil(l,-1)) { capsule.center1=lua_b3_read_b3Vec3(l,-1); }
		lua_pop(l,1);
		lua_getfield(l,2,"center2");
		if(!lua_isnil(l,-1)) { capsule.center2=lua_b3_read_b3Vec3(l,-1); }
		lua_pop(l,1);
		lua_getfield(l,2,"radius");
		if(!lua_isnil(l,-1)) { capsule.radius = (float)lua_tonumber(l,-1); }
		lua_pop(l,1);

		*pp=b3CreateCapsuleShape(body,&def,&capsule);
	}
	else
	if(strcmp(shape_type,"box")==0)
	{
		lua_pop(l,1); // shape_type is no longer valid

		b3Polygon polygon;

		float halfWidth=0.0f;
		lua_getfield(l,2,"halfWidth");
		if(!lua_isnil(l,-1)) { halfWidth = (float)lua_tonumber(l,-1); }
		lua_pop(l,1);

		float halfHeight=0.0f;
		lua_getfield(l,2,"halfHeight");
		if(!lua_isnil(l,-1)) { halfHeight = (float)lua_tonumber(l,-1); }
		lua_pop(l,1);

		b3Vec3 center=(b3Vec3){0.0f,0.0f};
		lua_getfield(l,2,"center");
		if(!lua_isnil(l,-1)) { center=lua_b3_read_b3Vec3(l,-1); }
		lua_pop(l,1);

		float rotation=0.0f;
		lua_getfield(l,2,"rotation");
		if(!lua_isnil(l,-1)) { rotation = (float)lua_tonumber(l,-1); }
		lua_pop(l,1);

		float radius=0.0f;
		lua_getfield(l,2,"radius");
		if(!lua_isnil(l,-1)) { radius = (float)lua_tonumber(l,-1); }
		lua_pop(l,1);

		polygon=b3MakeOffsetRoundedBox(halfWidth,halfHeight,center,b3MakeRot(rotation),radius);

		*pp=b3CreatePolygonShape(body,&def,&polygon);
	}
	else
	{
		lua_pop(l,1); // shape_type is no longer valid

		luaL_error(l,"unknown shape type");
	}

	lua_pushlstring(l,(const char *)pp,sizeof(b3ShapeId)); // id to (non printable) string
	return 2;
}

static int lua_b3_shape_destroy (lua_State *l)
{
b3ShapeId *pp=lua_b3_shape_ptr_ptr(l, 1 );
	if(B3_IS_NON_NULL(*pp))
	{
		int update=1;
		if(!lua_isnil(l,2)) // optional
		{
			update=lua_toboolean(l,2);
		}
		b3DestroyShape(*pp,update);
		*pp=(b3ShapeId){0};
	}
	return 0;
}

/*+---------------------------------------------------------------------

Get shape information

*/
static int lua_b3_shape_info (lua_State *l)
{
	b3ShapeId shape = lua_b3_shape_ptr(l, 1 );

	lua_newtable(l);
	
	b3ShapeType type = b3Shape_GetType(shape);
	if     ( type==b3_circleShape       ) { lua_pushstring(l, "circle"       ); }
	else if( type==b3_capsuleShape      ) { lua_pushstring(l, "capsule"      ); }
	else if( type==b3_segmentShape      ) { lua_pushstring(l, "segment"      ); }
	else if( type==b3_polygonShape      ) { lua_pushstring(l, "polygon"      ); }
	else if( type==b3_chainSegmentShape ) { lua_pushstring(l, "chainSegment" ); }
	else                                  { lua_pushstring(l, "unknown"      ); }
	lua_setfield(l, -2 , "type" );

	return 1;
}


/*+---------------------------------------------------------------------

Get shape variables

*/
static int lua_b3_shape_get (lua_State *l)
{
	b3ShapeId shape = lua_b3_shape_ptr(l, 1 );

	lua_newtable(l);
	
	lua_pushnumber(l, b3Shape_GetFriction(shape) );
	lua_setfield(l, -2 , "friction" );

	lua_pushnumber(l, b3Shape_GetRestitution(shape) );
	lua_setfield(l, -2 , "restitution" );

	lua_pushnumber(l, b3Shape_GetUserMaterial(shape) );
	lua_setfield(l, -2 , "material_userMaterialId" );

	b3Filter filter = b3Shape_GetFilter(shape);
	lua_pushnumber(l, filter.categoryBits );
	lua_setfield(l, -2 , "filter_categoryBits" );
	lua_pushnumber(l, filter.groupIndex );
	lua_setfield(l, -2 , "filter_groupIndex" );
	lua_pushnumber(l, filter.maskBits );
	lua_setfield(l, -2 , "filter_maskBits" );
	
	lua_pushboolean(l, b3Shape_AreSensorEventsEnabled(shape) );
	lua_setfield(l, -2 , "enableSensorEvents" );

	lua_pushboolean(l, b3Shape_AreContactEventsEnabled(shape) );
	lua_setfield(l, -2 , "enableContactEvents" );
	
	lua_pushboolean(l, b3Shape_AreHitEventsEnabled(shape) );
	lua_setfield(l, -2 , "enableHitEvents" );

	return 1;
}

/*+---------------------------------------------------------------------

Set shape variables

*/
static int lua_b3_shape_set (lua_State *l)
{
	b3ShapeId shape = lua_b3_shape_ptr(l, 1 );

	lua_getfield(l,2,"friction");
	if(!lua_isnil(l,-1))
	{
		b3Shape_SetFriction(shape, lua_tonumber(l,-1) );
	}
	lua_pop(l,1);

	lua_getfield(l,2,"restitution");
	if(!lua_isnil(l,-1))
	{
		b3Shape_SetRestitution(shape, lua_tonumber(l,-1) );
	}
	lua_pop(l,1);

	lua_getfield(l,2,"material_userMaterialId");
	if(!lua_isnil(l,-1))
	{
		b3Shape_SetUserMaterial(shape, lua_tonumber(l,-1) );
	}
	lua_pop(l,1);

	int set_filter=0;
	b3Filter filter = b3Shape_GetFilter(shape);
	lua_getfield(l,2,"filter_categoryBits");
	if(!lua_isnil(l,-1))
	{
		set_filter=1;
		filter.categoryBits = lua_tonumber(l,-1) ;
	}
	lua_pop(l,1);
	lua_getfield(l,2,"filter_groupIndex");
	if(!lua_isnil(l,-1))
	{
		set_filter=1;
		filter.groupIndex = lua_tonumber(l,-1) ;
	}
	lua_pop(l,1);
	lua_getfield(l,2,"filter_maskBits");
	if(!lua_isnil(l,-1))
	{
		set_filter=1;
		filter.maskBits = lua_tonumber(l,-1) ;
	}
	lua_pop(l,1);
	if(set_filter) { b3Shape_SetFilter(shape,filter); }

	lua_getfield(l,2,"enableSensorEvents");
	if(!lua_isnil(l,-1))
	{
		b3Shape_EnableSensorEvents(shape, lua_toboolean(l,-1) );
	}
	lua_pop(l,1);
	
	lua_getfield(l,2,"enablePreSolveEvents");
	if(!lua_isnil(l,-1))
	{
		b3Shape_EnablePreSolveEvents(shape, lua_toboolean(l,-1) );
	}
	lua_pop(l,1);

	lua_getfield(l,2,"enableContactEvents");
	if(!lua_isnil(l,-1))
	{
		b3Shape_EnableContactEvents(shape, lua_toboolean(l,-1) );
	}
	lua_pop(l,1);

	lua_getfield(l,2,"enableHitEvents");
	if(!lua_isnil(l,-1))
	{
		b3Shape_EnableHitEvents(shape, lua_toboolean(l,-1) );
	}
	lua_pop(l,1);

	return 0;
}


/*+---------------------------------------------------------------------

Convert a vector into a vector of a different space

Currently only one conversion which is from world point to closest 
world point on shape.

*/
static int lua_b3_shape_convert (lua_State *l)
{
	b3ShapeId shape = lua_b3_shape_ptr(l, 1 );

	b3Vec3 pa;
	b3Vec3 pb;

	pa.x=(float)lua_tonumber(l, 2 );
	pa.y=(float)lua_tonumber(l, 3 );

	pb=b3Shape_GetClosestPoint(shape,pa);

	lua_pushnumber(l, pb.x );
	lua_pushnumber(l, pb.y );

	return 2;
}


/*+---------------------------------------------------------------------

get shape aabb lower then upper values

*/
static int lua_b3_shape_aabb (lua_State *l)
{
	b3ShapeId shape = lua_b3_shape_ptr(l, 1 );

	b3AABB aabb=b3Shape_GetAABB(shape);

	lua_pushnumber(l, aabb.lowerBound.x );
	lua_pushnumber(l, aabb.lowerBound.y );
	lua_pushnumber(l, aabb.upperBound.x );
	lua_pushnumber(l, aabb.upperBound.y );

	return 4;
}

/*+---------------------------------------------------------------------

joint create/destroy

*/
b3JointId * lua_b3_joint_ptr_ptr (lua_State *l,int idx)
{
b3JointId *pp;
	pp=(b3JointId*)luaL_checkudata(l, idx , lua_b3_joint_meta_name);
	return pp;
}

b3JointId   lua_b3_joint_ptr (lua_State *l,int idx)
{
b3JointId *pp;
	pp=lua_b3_joint_ptr_ptr(l,idx);
	if(!pp) { luaL_error(l,"box3d joint is null"); }
	return *pp;
}

static int lua_b3_joint_create (lua_State *l)
{
b3JointId *pp;

	b3WorldId world=lua_b3_world_ptr(l,1);

// create ptr ptr userdata
	pp=(b3JointId*)lua_newuserdata(l, sizeof(b3JointId));
	*pp=(b3JointId){0};
	luaL_getmetatable(l, lua_b3_joint_meta_name);
	lua_setmetatable(l, -2);

	b3JointDef joint=(b3JointDef){0};
	// get generic joint values
	lua_getfield(l,2,"bodyIdA");
	if(!lua_isnil(l,-1)) { joint.bodyIdA=*((b3BodyId*)(lua_tostring(l,-1))); }
	lua_pop(l,1);
	lua_getfield(l,2,"bodyIdB");
	if(!lua_isnil(l,-1)) { joint.bodyIdB=*((b3BodyId*)(lua_tostring(l,-1))); }
	lua_pop(l,1);
	lua_getfield(l,2,"localFrameA");
	if(!lua_isnil(l,-1)) { joint.localFrameA=lua_b3_read_b3Transform(l,-1); }
	lua_pop(l,1);
	lua_getfield(l,2,"localFrameB");
	if(!lua_isnil(l,-1)) { joint.localFrameB=lua_b3_read_b3Transform(l,-1); }
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
	const char *joint_type="filter";
	lua_getfield(l,2,"joint"); // the type of joint as lowercase string
	if(lua_isstring(l,-1))
	{
		joint_type=lua_tostring(l,-1);
	}
	if(strcmp(joint_type,"distance")==0)
	{
		lua_pop(l,1); // joint_type is no longer valid

		b3DistanceJointDef distance=b3DefaultDistanceJointDef();
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

		*pp=b3CreateDistanceJoint(world,&distance);
	}
	else
	if(strcmp(joint_type,"motor")==0)
	{
		lua_pop(l,1); // joint_type is no longer valid

		b3MotorJointDef motor=b3DefaultMotorJointDef();
		motor.base=joint; // generic values

		lua_getfield(l,2,"linearVelocity");
		if(!lua_isnil(l,-1)) { motor.linearVelocity=lua_b3_read_b3Vec3(l,-1); }
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

		*pp=b3CreateMotorJoint(world,&motor);
	}
	else
	if(strcmp(joint_type,"filter")==0)
	{
		lua_pop(l,1); // joint_type is no longer valid

		b3FilterJointDef filter=b3DefaultFilterJointDef();
		filter.base=joint; // generic values

		*pp=b3CreateFilterJoint(world,&filter);
	}
	else
	if(strcmp(joint_type,"prismatic")==0)
	{
		lua_pop(l,1); // joint_type is no longer valid

		b3PrismaticJointDef prismatic=b3DefaultPrismaticJointDef();
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

		*pp=b3CreatePrismaticJoint(world,&prismatic);
	}
	else
	if(strcmp(joint_type,"revolute")==0)
	{
		lua_pop(l,1); // joint_type is no longer valid

		b3RevoluteJointDef revolute=b3DefaultRevoluteJointDef();
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

		*pp=b3CreateRevoluteJoint(world,&revolute);
	}
	else
	if(strcmp(joint_type,"weld")==0)
	{
		lua_pop(l,1); // joint_type is no longer valid

		b3WeldJointDef weld=b3DefaultWeldJointDef();
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

		*pp=b3CreateWeldJoint(world,&weld);
	}
	else
	if(strcmp(joint_type,"wheel")==0)
	{
		lua_pop(l,1); // joint_type is no longer valid

		b3WheelJointDef wheel=b3DefaultWheelJointDef();
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

		*pp=b3CreateWheelJoint(world,&wheel);
	}
	else
	{
		lua_pop(l,1); // joint_type is no longer valid

		luaL_error(l,"unknown joint type");
	}

	lua_pushlstring(l,(const char *)pp,sizeof(b3JointId)); // id to (non printable) string
	return 2;
}

static int lua_b3_joint_destroy (lua_State *l)
{
b3JointId *pp=lua_b3_joint_ptr_ptr(l, 1 );
	if(B3_IS_NON_NULL(*pp))
	{
		int update=1;
		if(!lua_isnil(l,2)) // optional
		{
			update=lua_toboolean(l,2);
		}
		b3DestroyJoint(*pp,update);
		*pp=(b3JointId){0};
	}
	return 0;
}

/*+---------------------------------------------------------------------

Get joint information

*/
static int lua_b3_joint_info (lua_State *l)
{
	b3JointId joint = lua_b3_joint_ptr(l, 1 );

	lua_newtable(l);

	b3JointType type = b3Joint_GetType(joint);
	if     ( type==b3_distanceJoint  ) { lua_pushstring(l, "distance"  ); }
	else if( type==b3_filterJoint    ) { lua_pushstring(l, "filter"    ); }
	else if( type==b3_motorJoint     ) { lua_pushstring(l, "motor"     ); }
	else if( type==b3_prismaticJoint ) { lua_pushstring(l, "prismatic" ); }
	else if( type==b3_revoluteJoint  ) { lua_pushstring(l, "revolute"  ); }
	else if( type==b3_weldJoint      ) { lua_pushstring(l, "weld"      ); }
	else if( type==b3_wheelJoint     ) { lua_pushstring(l, "wheel"     ); }
	else                               { lua_pushstring(l, "unknown"   ); }
	lua_setfield(l, -2 , "type" );
	
	b3BodyId bodyA = b3Joint_GetBodyA(joint);
	lua_pushlstring(l,(const char *)&bodyA,sizeof(b3BodyId)); // id to (non printable) string
	lua_setfield(l, -2 , "bodyIdA" );

	b3BodyId bodyB = b3Joint_GetBodyB(joint);
	lua_pushlstring(l,(const char *)&bodyB,sizeof(b3BodyId)); // id to (non printable) string
	lua_setfield(l, -2 , "bodyIdB" );

	return 1;
}


/*+---------------------------------------------------------------------

Get joint variables

*/
static int lua_b3_joint_get (lua_State *l)
{
	b3JointId joint = lua_b3_joint_ptr(l, 1 );

	lua_newtable(l);

	lua_pushboolean(l, b3Joint_GetCollideConnected(joint) );
	lua_setfield(l, -2 , "collideConnected" );

	return 1;
}


/*+---------------------------------------------------------------------

Set joint variables

*/
static int lua_b3_joint_set (lua_State *l)
{
	b3JointId joint = lua_b3_joint_ptr(l, 1 );

	lua_getfield(l,2,"collideConnected");
	if(!lua_isnil(l,-1))
	{
		b3Joint_SetCollideConnected(joint, lua_toboolean(l,-1) );
	}
	lua_pop(l,1);

	return 0;
}


/*+---------------------------------------------------------------------

Lua Assert

*/
static thread_local lua_State *lua_b3_lua_state=0; // does this work?
static int lua_b3_assert (const char *condition, const char *fileName, int lineNumber)
{
	if( lua_b3_lua_state )
	{
		luaL_error( lua_b3_lua_state , "%s : %s : %d" , condition , fileName , lineNumber );
	}
	return 1;
}


/*+---------------------------------------------------------------------

open library.

*/
LUALIB_API int luaopen_box3d_core (lua_State *l)
{
	const luaL_Reg lib[] =
	{
		{"info",					lua_b3_info},
		{"get",						lua_b3_get},
		{"set",						lua_b3_set},
		{"contact",					lua_b3_contact},

		{"world_create",			lua_b3_world_create},
		{"world_destroy",			lua_b3_world_destroy},
		{"world_info",				lua_b3_world_info},
		{"world_get",				lua_b3_world_get},
		{"world_set",				lua_b3_world_set},
		{"world_step",				lua_b3_world_step},
		{"world_body_events",		lua_b3_world_body_events},
		{"world_sensor_events",		lua_b3_world_sensor_events},
		{"world_contact_events",	lua_b3_world_contact_events},
		{"world_cast",				lua_b3_world_cast},
		{"world_overlap",			lua_b3_world_overlap},

		{"body_create",				lua_b3_body_create},
		{"body_destroy",			lua_b3_body_destroy},
		{"body_info",				lua_b3_body_info},
		{"body_get",				lua_b3_body_get},
		{"body_set",				lua_b3_body_set},
		{"body_type",				lua_b3_body_type},
		{"body_awake",				lua_b3_body_awake},
		{"body_transform",			lua_b3_body_transform},
		{"body_velocity",			lua_b3_body_velocity},
		{"body_force",				lua_b3_body_force},
		{"body_acceleration",		lua_b3_body_acceleration},
		{"body_mass",				lua_b3_body_mass},
		{"body_convert",			lua_b3_body_convert},
		{"body_aabb",				lua_b3_body_aabb},

		{"shape_create",			lua_b3_shape_create},
		{"shape_destroy",			lua_b3_shape_destroy},
		{"shape_info",				lua_b3_shape_info},
		{"shape_get",				lua_b3_shape_get},
		{"shape_set",				lua_b3_shape_set},
		{"shape_convert",			lua_b3_shape_convert},
		{"shape_aabb",				lua_b3_shape_aabb},

		{"joint_create",			lua_b3_joint_create},
		{"joint_destroy",			lua_b3_joint_destroy},
		{"joint_info",				lua_b3_joint_info},
		{"joint_get",				lua_b3_joint_get},
		{"joint_set",				lua_b3_joint_set},

		{0,0}
	};

	const luaL_Reg meta_world[] =
	{
		{"__gc",			lua_b3_world_destroy},
		{0,0}
	};

	const luaL_Reg meta_body[] =
	{
		{"__gc",			lua_b3_body_destroy},
		{0,0}
	};

	const luaL_Reg meta_shape[] =
	{
		{"__gc",			lua_b3_shape_destroy},
		{0,0}
	};

	const luaL_Reg meta_joint[] =
	{
		{"__gc",			lua_b3_joint_destroy},
		{0,0}
	};

	luaL_newmetatable(l, lua_b3_world_meta_name);
	luaL_openlib(l, NULL, meta_world, 0);
	lua_pop(l,1);

	luaL_newmetatable(l, lua_b3_body_meta_name);
	luaL_openlib(l, NULL, meta_body, 0);
	lua_pop(l,1);

	luaL_newmetatable(l, lua_b3_shape_meta_name);
	luaL_openlib(l, NULL, meta_shape, 0);
	lua_pop(l,1);

	luaL_newmetatable(l, lua_b3_joint_meta_name);
	luaL_openlib(l, NULL, meta_joint, 0);
	lua_pop(l,1);

	lua_newtable(l);
	luaL_openlib(l, NULL, lib, 0);
	
	// lua error on box assert, this assumes one lua state per thread.
	// which is "reasonable" but could get you into trouble if you are doing something crazy.
	lua_b3_lua_state=l; // remember lua state in a thread_local
	b3SetAssertFcn( &lua_b3_assert ); // so we can have nice lua style errors
	
	return 1;
}

