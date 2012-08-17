/*

the hard core of the lua box2d wrapper

*/

#define LUA_LIB

extern "C" {

#include "lua.h"
#include "lauxlib.h"

};

#include "Box2D.h"

#include <string.h>


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// lua
//
/*+-----------------------------------------------------------------------------------------------------------------+*/

static int core_setup(lua_State *l)
{
float32 x,y;

// our core is a pointer to a cpp class
	b2World *w = (b2World *)lua_newuserdata(l, sizeof(b2World));

	if(w)
	{
		// Define the size of the world. Simulation will still work
		// if bodies reach the end of the world, but it will be slower.
		b2AABB worldAABB;
		worldAABB.lowerBound.Set(-100.0f, -100.0f);
		worldAABB.upperBound.Set(100.0f, 100.0f);

		// Define the gravity vector.
		b2Vec2 gravity;
		gravity.Set(0.0f, -10.0f);

		// Do we want to let bodies sleep?
		bool doSleep = true;

		if( lua_istable(l,1) )
		{
			lua_getfield(l,1,"min");
			if( lua_istable(l,-1) )
			{
				lua_rawgeti(l,-1,1);
				x=(float32)lua_tonumber(l,-1); lua_pop(l,1);
				lua_rawgeti(l,-1,2);
				y=(float32)lua_tonumber(l,-1); lua_pop(l,1);
				worldAABB.lowerBound.Set(x,y);
			}
			lua_pop(l,1);
			
			lua_getfield(l,1,"max");
			if( lua_istable(l,-1) )
			{
				lua_rawgeti(l,-1,1);
				x=(float32)lua_tonumber(l,-1); lua_pop(l,1);
				lua_rawgeti(l,-1,2);
				y=(float32)lua_tonumber(l,-1); lua_pop(l,1);
				worldAABB.upperBound.Set(x,y);
			}
			lua_pop(l,1);
			
			lua_getfield(l,1,"gravity");
			if( lua_istable(l,-1) )
			{
				lua_rawgeti(l,-1,1);
				x=(float32)lua_tonumber(l,-1); lua_pop(l,1);
				lua_rawgeti(l,-1,2);
				y=(float32)lua_tonumber(l,-1); lua_pop(l,1);
				gravity.Set(x,y);
			}
			lua_pop(l,1);
			
			lua_getfield(l,1,"sleep");
			if( lua_isboolean(l,-1) )
			{
				doSleep=lua_toboolean(l,-1)?true:false;
			}
			lua_pop(l,1);
		}


		// Construct a world object, which will hold and simulate the rigid bodies.
		w=new ((void*)w) b2World(worldAABB, gravity, doSleep);
	//	*core = new b2World(worldAABB, gravity, doSleep);
	
		lua_pushlightuserdata(l,w);
		lua_pushvalue(l,1);
		lua_settable(l, LUA_REGISTRYINDEX); // save main tab associated with main struct
	}

	return 1;
}
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// lua
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int core_clean(lua_State *l)
{
	b2World *w = (b2World *) lua_touserdata(l, 1 );
	
	if(w)
	{
		w->~b2World();
	}

	lua_pushlightuserdata(l,w);
	lua_pushnil(l);
	lua_settable(l, LUA_REGISTRYINDEX); // clear main tab associated with main struct

	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// lua
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int core_set(lua_State *l)
{
	b2World *w = (b2World *) lua_touserdata(l, 1 );
	
	if(w)
	{
	}

	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// lua
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int core_get(lua_State *l)
{
	b2World *w = (b2World *) lua_touserdata(l, 1 );
	
	if(w)
	{
	}

	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// lua
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int core_body(lua_State *l)
{
	b2World *w = (b2World *) lua_touserdata(l, 1 );
	
b2BodyDef def;
b2Body *body=0;

float32 x,y;

// fill def with possible default information

	if( lua_istable(l,2) )
	{
		lua_getfield(l,2,"position");
		if( lua_istable(l,-1) )
		{
			lua_rawgeti(l,-1,1);
			x=(float32)lua_tonumber(l,-1); lua_pop(l,1);
			lua_rawgeti(l,-1,2);
			y=(float32)lua_tonumber(l,-1); lua_pop(l,1);
			def.position.Set(x,y);
		}
		lua_pop(l,1);
	 	
		lua_getfield(l,2,"angle");
		if( lua_isnumber(l,-1) )
		{
			def.angle=b2_pi*((float32)lua_tonumber(l,-1))/180.0f;
		}
		lua_pop(l,1);
		
		lua_getfield(l,2,"linear_damping");
		if( lua_isnumber(l,-1) )
		{
			def.linearDamping=(float32)lua_tonumber(l,-1);
		}
		lua_pop(l,1);
		
		lua_getfield(l,2,"angular_damping");
		if( lua_isnumber(l,-1) )
		{
			def.angularDamping=(float32)lua_tonumber(l,-1);
		}
		lua_pop(l,1);
		
		lua_getfield(l,2,"allow_sleep");
		if( lua_isboolean(l,-1) )
		{
			def.allowSleep=lua_toboolean(l,-1)?true:false;
		}
		lua_pop(l,1);
		
		lua_getfield(l,2,"sleeping");
		if( lua_isboolean(l,-1) )
		{
			def.isSleeping=lua_toboolean(l,-1)?true:false;
		}
		lua_pop(l,1);

		lua_getfield(l,2,"fixed_rotation");
		if( lua_isboolean(l,-1) )
		{
			def.fixedRotation=lua_toboolean(l,-1)?true:false;
		}
		lua_pop(l,1);

		lua_getfield(l,2,"bullet");
		if( lua_isboolean(l,-1) )
		{
			def.isBullet=lua_toboolean(l,-1)?true:false;
		}
		lua_pop(l,1);
	}

	if(w)
	{
		body=w->CreateBody(&def);
		
	}
	
// return a body ptr or a nil

	if(body)
	{
		lua_pushlightuserdata(l,body);
	}
	else
	{
		lua_pushnil(l);
	}

	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// lua
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int core_body_delete(lua_State *l)
{
	b2World *w = (b2World *) lua_touserdata(l, 1 );
	b2Body *body=(b2Body *)lua_touserdata(l,2);

	if(w && body)
	{
		w->DestroyBody(body);
	}

	return 0;
}
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// lua
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int core_body_get(lua_State *l)
{
	b2World *w = (b2World *) lua_touserdata(l, 1 );
	b2Body *body=(b2Body *)lua_touserdata(l,2);

b2Vec2 v;
float32 f;

	if(w && body)
	{
		if( lua_istable(l,3) ) // fill with info
		{
			v=body->GetPosition();
			lua_pushnumber(l,v.x);
			lua_setfield(l,3,"x");
			lua_pushnumber(l,v.y);
			lua_setfield(l,3,"y");
			
			v=body->GetLinearVelocity();
			lua_pushnumber(l,v.x);
			lua_setfield(l,3,"vx");
			lua_pushnumber(l,v.y);
			lua_setfield(l,3,"vy");
			
			f=180.0f*body->GetAngle()/b2_pi;
			lua_pushnumber(l,f);
			lua_setfield(l,3,"a");
			
			f=body->GetAngularVelocity();
			lua_pushnumber(l,f);
			lua_setfield(l,3,"va");
		}
	}

	return 0;
}
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// lua
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int core_body_set(lua_State *l)
{
	b2World *w = (b2World *) lua_touserdata(l, 1 );
	b2Body *body=(b2Body *)lua_touserdata(l,2);

float32 x=0,y=0,a=0;
b2Vec2 v;
bool do_xform=false;

	if(w && body)
	{
		if( lua_istable(l,3) ) // set from info
		{
			lua_getfield(l,3,"x");
			if( lua_isnumber(l,-1) )
			{
				x=(float32)lua_tonumber(l,-1);
				do_xform=true;
			}
			lua_pop(l,1);

			lua_getfield(l,3,"y");
			if( lua_isnumber(l,-1) )
			{
				y=(float32)lua_tonumber(l,-1);
				do_xform=true;
			}
			lua_pop(l,1);
			
			lua_getfield(l,3,"a");
			if( lua_isnumber(l,-1) )
			{
				a=(float32)lua_tonumber(l,-1);
				do_xform=true;
			}
			lua_pop(l,1);
			
			if(do_xform)
			{
				v.Set(x,y);
				body->SetXForm(v,a);
			}
			
			lua_getfield(l,3,"mass");
			if( lua_isstring(l,-1) )
			{
				if( strcmp(lua_tostring(l,-1),"shapes") == 0 )
				{
					body->SetMassFromShapes();
				}
			}
			lua_pop(l,1);
		}
	}

	return 0;
}
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// lua
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int core_body_shape(lua_State *l)
{
	b2World *w = (b2World *) lua_touserdata(l, 1 );
	b2Body *body=(b2Body *)lua_touserdata(l,2);

b2CircleDef def_circle;
b2PolygonDef def_polygon;
	
b2ShapeDef *def=&def_polygon;

b2Shape *shape=0;

float32 x,y,hx,hy,angle;
b2Vec2 center;

	if( lua_istable(l,3) )
	{
	
		lua_getfield(l,3,"circle"); // first check to flag a circle shape rather than the default poly/box?
		if( lua_istable(l,-1) )
		{
			def=&def_circle;
		}
		lua_pop(l,1);
		
		
		lua_getfield(l,3,"friction");
		if( lua_isnumber(l,-1) )
		{
			def->friction=(float32)lua_tonumber(l,-1);
		}
		lua_pop(l,1);
		
		lua_getfield(l,3,"restitution");
		if( lua_isnumber(l,-1) )
		{
			def->restitution=(float32)lua_tonumber(l,-1);
		}
		lua_pop(l,1);
		
		lua_getfield(l,3,"density");
		if( lua_isnumber(l,-1) )
		{
			def->density=(float32)lua_tonumber(l,-1);
		}
		lua_pop(l,1);
		
		lua_getfield(l,3,"category");
		if( lua_isnumber(l,-1) )
		{
			def->filter.categoryBits=(uint16)lua_tonumber(l,-1);
		}
		lua_pop(l,1);
		
		lua_getfield(l,3,"mask");
		if( lua_isnumber(l,-1) )
		{
			def->filter.maskBits=(uint16)lua_tonumber(l,-1);
		}
		lua_pop(l,1);

		lua_getfield(l,3,"group");
		if( lua_isnumber(l,-1) )
		{
			def->filter.groupIndex=(int16)lua_tonumber(l,-1);
		}
		lua_pop(l,1);
		
		
		lua_getfield(l,3,"sensor");
		if( lua_isboolean(l,-1) )
		{
			def->isSensor=lua_toboolean(l,-1)?true:false;
		}
		lua_pop(l,1);
		
		lua_getfield(l,3,"box"); // a box shape
		if( lua_istable(l,-1) )
		{
			hx=1;
			hy=1;
			center.Set(0,0);
			angle=0;
			
			lua_getfield(l,-1,"width");
			if( lua_isnumber(l,-1) )
			{
				hx=(float32)lua_tonumber(l,-1);
			}
			lua_pop(l,1);
			
			lua_getfield(l,-1,"height");
			if( lua_isnumber(l,-1) )
			{
				hy=(float32)lua_tonumber(l,-1);
			}
			lua_pop(l,1);
			
			lua_getfield(l,-1,"angle");
			if( lua_isnumber(l,-1) )
			{
				angle=(float32)lua_tonumber(l,-1);
			}
			lua_pop(l,1);
			
			lua_getfield(l,-1,"center");
			if( lua_istable(l,-1) )
			{
				lua_rawgeti(l,-1,1);
				x=(float32)lua_tonumber(l,-1); lua_pop(l,1);
				lua_rawgeti(l,-1,2);
				y=(float32)lua_tonumber(l,-1); lua_pop(l,1);
				center.Set(x,y);
			}
			lua_pop(l,1);
			
			def_polygon.SetAsBox(hx,hy,center,angle);
		}
		lua_pop(l,1);
		
		lua_getfield(l,3,"circle"); // a circle shape
		if( lua_istable(l,-1) )
		{
			lua_getfield(l,-1,"radius");
			if( lua_isnumber(l,-1) )
			{
				def_circle.radius=(float32)lua_tonumber(l,-1);
			}
			lua_pop(l,1);
			
			lua_getfield(l,-1,"position");
			if( lua_istable(l,-1) )
			{
				lua_rawgeti(l,-1,1);
				x=(float32)lua_tonumber(l,-1); lua_pop(l,1);
				lua_rawgeti(l,-1,2);
				y=(float32)lua_tonumber(l,-1); lua_pop(l,1);
				def_circle.localPosition.Set(x,y);
			}
			lua_pop(l,1);
		}
		lua_pop(l,1);
		
	}

	if(w && body)
	{
		shape=body->CreateShape(def);
	}

// return a shape ptr or a nil

	if(shape)
	{
		lua_pushlightuserdata(l,shape);
	}
	else
	{
		lua_pushnil(l);
	}


	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// lua
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int core_body_shape_delete(lua_State *l)
{
	b2World *w    =(b2World *)lua_touserdata(l,1);
	b2Body  *body =(b2Body *) lua_touserdata(l,2);
	b2Shape *shape=(b2Shape *)lua_touserdata(l,3);
	
	if(w && body && shape)
	{
		body->DestroyShape(shape);
	}

	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// lua
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int core_joint(lua_State *l)
{
	b2World *w    =(b2World *)lua_touserdata(l,1);
	
b2DistanceJointDef def;
b2Joint *joint=0;

float32 x,y;

// fill def with possible default information

	if( lua_istable(l,2) )
	{
		lua_getfield(l,2,"anchor1");
		if( lua_istable(l,-1) )
		{
			lua_rawgeti(l,-1,1);
			x=(float32)lua_tonumber(l,-1); lua_pop(l,1);
			lua_rawgeti(l,-1,2);
			y=(float32)lua_tonumber(l,-1); lua_pop(l,1);
			def.localAnchor1.Set(x,y);
		}
		lua_pop(l,1);
	 	
		lua_getfield(l,2,"anchor2");
		if( lua_istable(l,-1) )
		{
			lua_rawgeti(l,-1,1);
			x=(float32)lua_tonumber(l,-1); lua_pop(l,1);
			lua_rawgeti(l,-1,2);
			y=(float32)lua_tonumber(l,-1); lua_pop(l,1);
			def.localAnchor2.Set(x,y);
		}
		lua_pop(l,1);
		
		lua_getfield(l,2,"body1");
		if( lua_istable(l,-1) )
		{
			lua_getfield(l,-1,"core");
			def.body1=(b2Body *)lua_touserdata(l,-1);
			lua_pop(l,1);
		}
		lua_pop(l,1);
		
		lua_getfield(l,2,"body2");
		if( lua_istable(l,-1) )
		{
			lua_getfield(l,-1,"core");
			def.body2=(b2Body *)lua_touserdata(l,-1);
			lua_pop(l,1);
		}
		lua_pop(l,1);
		
		lua_getfield(l,2,"length");
		if( lua_isnumber(l,-1) )
		{
			def.length=(float32)lua_tonumber(l,-1);
		}
		lua_pop(l,1);
		
		lua_getfield(l,2,"freq");
		if( lua_isnumber(l,-1) )
		{
			def.frequencyHz=(float32)lua_tonumber(l,-1);
		}
		lua_pop(l,1);
		
		lua_getfield(l,2,"damp");
		if( lua_isnumber(l,-1) )
		{
			def.dampingRatio=(float32)lua_tonumber(l,-1);
		}
		lua_pop(l,1);
		
		lua_getfield(l,2,"collide");
		if( lua_isboolean(l,-1) )
		{
			def.collideConnected=lua_toboolean(l,-1)?true:false;
		}
		lua_pop(l,1);
		
	}

	if(w)
	{
		joint=w->CreateJoint(&def);
		
	}
	
// return a body ptr or a nil

	if(joint)
	{
		lua_pushlightuserdata(l,joint);
	}
	else
	{
		lua_pushnil(l);
	}

	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// lua
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int core_joint_delete(lua_State *l)
{
	b2World *w    =(b2World *)lua_touserdata(l,1);
	b2Joint *joint =(b2Joint *) lua_touserdata(l,2);
	
	if(w && joint)
	{
		w->DestroyJoint(joint);
	}

	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// lua
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int core_step(lua_State *l)
{
	b2World *w = (b2World *) lua_touserdata(l, 1 );
	
	float32 timeStep=(float32)lua_tonumber(l,2);
	int32 iterations=(int32)lua_tonumber(l,3);
	
	if(w)
	{
		w->Step(timeStep,iterations);
	}

	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// lua
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static const struct luaL_reg core_lib[] = {
	{"setup",				core_setup},
	{"clean",				core_clean},
	
	{"set",					core_set},
	{"get",					core_get},
	
	{"step",				core_step},
	
	{"body",				core_body},
	{"body_delete",			core_body_delete},
	{"body_get",			core_body_get},
	{"body_set",			core_body_set},
	
	{"body_shape",			core_body_shape},
	{"body_shape_delete",	core_body_shape_delete},
	
	{"joint",				core_joint},
	{"joint_delete",		core_joint_delete},
	
	{NULL, NULL},
};


extern "C" {

LUALIB_API int luaopen_box2d_core(lua_State *L)
{
  luaL_register(L, "box2d.core", core_lib);
  return 1;
}

};
