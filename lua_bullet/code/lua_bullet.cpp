/*+------------------------------------------------------------------+**

(C) Kriss@XIXs.com 2020 and released under the MIT license.

See https://github.com/xriss/gamecake for full notice.

**+------------------------------------------------------------------+*/

#include "all.h"


/*+------------------------------------------------------------------+**

A world plus all the other parts that a world needs allocated in one 
struct.

**+------------------------------------------------------------------+*/

const char *lua_bullet_world_meta_name="bullet_world*ptr";

typedef struct {
	btDefaultCollisionConfiguration     * config ;
	btCollisionDispatcher               * dispatcher ;
	btBroadphaseInterface               * phase ;
	btSequentialImpulseConstraintSolver * solver ;
	btDiscreteDynamicsWorld             * world ;
} bullet_world ;

bullet_world * lua_bullet_world_ptr (lua_State *l,int idx)
{
bullet_world *pp;
	pp=(bullet_world*)luaL_checkudata(l, idx , lua_bullet_world_meta_name);
	if(!pp->world) { luaL_error(l,"bullet world is null"); }
	return pp;
}

static void lua_bullet_world_delete (bullet_world *pp)
{
	if(pp->world)      { delete pp->world;      pp->world=0;      }
	if(pp->config)     { delete pp->config;     pp->config=0;     }
	if(pp->dispatcher) { delete pp->dispatcher; pp->dispatcher=0; }
	if(pp->phase)      { delete pp->phase;      pp->phase=0;      }
	if(pp->solver)     { delete pp->solver;     pp->solver=0;     }
}

static int lua_bullet_world_destroy (lua_State *l)
{	
bullet_world *pp=(bullet_world*)luaL_checkudata(l, 1 , lua_bullet_world_meta_name);

	// remove any registry link
	lua_pushlightuserdata(l,pp);
	lua_pushnil(l);
	lua_settable(l,LUA_REGISTRYINDEX);

	lua_bullet_world_delete(pp);

	return 0;
}

static int lua_bullet_world_create (lua_State *l)
{
bullet_world *pp;

	// create userdata
	pp=(bullet_world*)lua_newuserdata(l, sizeof(bullet_world));
	memset(pp,0,sizeof(bullet_world));
	luaL_getmetatable(l, lua_bullet_world_meta_name);
	lua_setmetatable(l, -2);

	try	// allocate all the parts we need for a bullet_world
	{
		pp->config     = new btDefaultCollisionConfiguration();
		pp->dispatcher = new btCollisionDispatcher( pp->config );
		pp->phase      = new btDbvtBroadphase();
		pp->solver     = new btSequentialImpulseConstraintSolver;
		pp->world      = new btDiscreteDynamicsWorld( pp->dispatcher, pp->phase, pp->solver, pp->config );
	}
	catch (std::exception const& e)
	{
		lua_bullet_world_delete(pp);
		return 0;
	}
	
	return 1;
}

static int lua_bullet_world_register (lua_State *l)
{
	lua_pushvalue(l,1); // this will be the userdata
	lua_pushvalue(l,2); // this will be the table
	lua_settable(l,LUA_REGISTRYINDEX);
	return 0;
}

/*+------------------------------------------------------------------+**

shape

**+------------------------------------------------------------------+*/

const char *lua_bullet_shape_meta_name="bullet_shape*ptr";


btCollisionShape *  lua_bullet_shape_ptr (lua_State *l,int idx)
{
btCollisionShape **pp;
	pp=(btCollisionShape**)luaL_checkudata(l, idx , lua_bullet_shape_meta_name);
	if(!*pp) { luaL_error(l,"bullet shape is null"); }
	return *pp;
}

static int lua_bullet_shape_destroy (lua_State *l)
{
btCollisionShape **pp=(btCollisionShape**)luaL_checkudata(l, 1 , lua_bullet_shape_meta_name);
	if(*pp)
	{
		delete *pp;
		(*pp)=0;
	}	
	return 0;
}

static int lua_bullet_shape_create (lua_State *l)
{
const char *tp;
btCollisionShape **pp;
double hx,hy,hz;
double radius;
int count;
int i;
// create ptr ptr userdata
	pp=(btCollisionShape**)lua_newuserdata(l, sizeof(btCollisionShape*));
	(*pp)=0;
	luaL_getmetatable(l, lua_bullet_shape_meta_name);
	lua_setmetatable(l, -2);

// allocate cpShape
		tp=luaL_checkstring(l,1);
		if(0==strcmp(tp,"box"))
		{
			hx=luaL_checknumber(l,2);
			hy=luaL_checknumber(l,3);
			hz=luaL_checknumber(l,4);
			*pp = new btBoxShape( btVector3(hx,hy,hz) );
		}
		else
		if(0==strcmp(tp,"sphere"))
		{
			radius=luaL_checknumber(l,2);
			*pp = new btSphereShape( btScalar(radius) );
		}
		else
		{
			lua_pushstring(l,"unknown shape type"); lua_error(l);
		}

	return 1;
}


/*+------------------------------------------------------------------+**

body

**+------------------------------------------------------------------+*/

const char *lua_bullet_body_meta_name="bullet_body*ptr";

btRigidBody *  lua_bullet_body_ptr (lua_State *l,int idx)
{
btRigidBody **pp=(btRigidBody**)luaL_checkudata(l, idx , lua_bullet_body_meta_name);
	if(!*pp) { luaL_error(l,"bullet body is null"); }
	return *pp;
}

static int lua_bullet_body_destroy (lua_State *l)
{	
btRigidBody **pp=(btRigidBody**)luaL_checkudata(l, 1 , lua_bullet_body_meta_name);
	if(*pp)
	{
		btMotionState * motion = (*pp)->getMotionState();
		if( motion ) { delete motion; }
		delete *pp;
		(*pp)=0;
	}	
	return 0;
}

static int lua_bullet_body_create (lua_State *l)
{	
const char *tp;
btRigidBody **pp;
btCollisionShape *shape;
double hx,hy,hz;
int count;
int i;
btScalar mass( 0.0f );
btTransform trans;

	trans.setIdentity();

// create ptr ptr userdata
	pp=(btRigidBody**)lua_newuserdata(l, sizeof(btRigidBody*));
	(*pp)=0;
	luaL_getmetatable(l, lua_bullet_body_meta_name);
	lua_setmetatable(l, -2);

// allocate cpbody
		tp=luaL_checkstring(l,1);
		if(0==strcmp(tp,"rigid"))
		{
			shape=lua_bullet_shape_ptr(l, 2 );

			mass=lua_tonumber(l,3);

			trans.setOrigin(btVector3( lua_tonumber(l,4) ,  lua_tonumber(l,5) , lua_tonumber(l,6) ));

			btVector3 localInertia(0, 0, 0);
			if(mass != 0.f)
			{
				shape->calculateLocalInertia(mass, localInertia);
			}

			btDefaultMotionState* myMotionState = new btDefaultMotionState(trans);
			btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, shape, localInertia);
			*pp = new btRigidBody(rbInfo);

		}
		else
		{
			lua_pushstring(l,"unknown body type"); lua_error(l);
		}

	return 1;
}



/*+------------------------------------------------------------------+**

get/set world gravity

**+------------------------------------------------------------------+*/
static int lua_bullet_world_gravity (lua_State *l)
{
btDiscreteDynamicsWorld *world=lua_bullet_world_ptr(l,1)->world;

	if( lua_isnumber(l,2) )
	{
		world->setGravity(btVector3( lua_tonumber(l,2) ,  lua_tonumber(l,3) , lua_tonumber(l,4) ));
	}

	btVector3 v=world->getGravity();

	lua_pushnumber(l,v.getX());
	lua_pushnumber(l,v.getY());
	lua_pushnumber(l,v.getZ());

	return 3;
}

/*+------------------------------------------------------------------+**

world step

**+------------------------------------------------------------------+*/
static int lua_bullet_world_step (lua_State *l)
{
btDiscreteDynamicsWorld *world=lua_bullet_world_ptr(l,1)->world;

	world->stepSimulation( lua_tonumber(l,2) ,  lua_tonumber(l,3) );

	return 0;
}

/*+------------------------------------------------------------------+**

add body to world

**+------------------------------------------------------------------+*/
static int lua_bullet_world_add_body (lua_State *l)
{
btDiscreteDynamicsWorld *world = lua_bullet_world_ptr(l,1)->world;
btRigidBody             *body  = lua_bullet_body_ptr(l, 2 );

	world->addRigidBody(body);

	return 0;
}

/*+------------------------------------------------------------------+**

remove body from world

**+------------------------------------------------------------------+*/
static int lua_bullet_world_remove_body (lua_State *l)
{
btDiscreteDynamicsWorld *world = lua_bullet_world_ptr(l,1)->world;
btRigidBody             *body  = lua_bullet_body_ptr(l, 2 );

	world->removeRigidBody(body);

	return 0;
}



/*+------------------------------------------------------------------+**

get body position and rotation ( 7 numbers )

**+------------------------------------------------------------------+*/
static int lua_bullet_body_transform (lua_State *l)
{
btRigidBody *body = lua_bullet_body_ptr(l, 1 );
btTransform trans;

	if( lua_isnumber(l,2) )
	{
		trans.setIdentity();
		trans.setOrigin(btVector3( lua_tonumber(l,2) ,  lua_tonumber(l,3) , lua_tonumber(l,4) ));
		if( lua_isnumber(l,5) )
		{
			trans.setRotation(btQuaternion( lua_tonumber(l,5) ,  lua_tonumber(l,6) , lua_tonumber(l,7) , lua_tonumber(l,8) ));
		}
		body->setWorldTransform(trans);
	}

	btMotionState *motion=body->getMotionState();
	if( motion ) { motion->getWorldTransform(trans); }
	else { trans = body->getWorldTransform(); }

	btVector3 v=trans.getOrigin();
	btQuaternion q=trans.getRotation();

	lua_pushnumber(l,v.getX());
	lua_pushnumber(l,v.getY());
	lua_pushnumber(l,v.getZ());

	lua_pushnumber(l,q.getX());
	lua_pushnumber(l,q.getY());
	lua_pushnumber(l,q.getZ());
	lua_pushnumber(l,q.getW());

	return 7;
}



/*+------------------------------------------------------------------+**

test

gamecake -e" local wbc=require('wetgenes.bullet.core') ; wbc.test( wbc.world_create({}) ) "

gamecake -e" require('apps').default_paths() ; require('wetgenes.bullet').test() "

**+------------------------------------------------------------------+*/

static int lua_bullet_test (lua_State *l)
{
bullet_world *pp=lua_bullet_world_ptr (l,1);

	int i;

	pp->world->setGravity(btVector3(0, -10, 0));

	///-----initialization_end-----

	//keep track of the shapes, we release memory at exit.
	//make sure to re-use collision shapes among rigid bodies whenever possible!
	btAlignedObjectArray<btCollisionShape*> collisionShapes;

	///create a few basic rigid bodies

	//the ground is a cube of side 100 at position y = -56.
	//the sphere will hit it at y = -6, with center at -5
	{
		btCollisionShape* groundShape = new btBoxShape(btVector3(btScalar(50.), btScalar(50.), btScalar(50.)));

		collisionShapes.push_back(groundShape);

		btTransform groundTransform;
		groundTransform.setIdentity();
		groundTransform.setOrigin(btVector3(0, 0, 0));

		btScalar mass(0.);

		//rigidbody is dynamic if and only if mass is non zero, otherwise static
		bool isDynamic = (mass != 0.f);

		btVector3 localInertia(0, 0, 0);
		if (isDynamic)
			groundShape->calculateLocalInertia(mass, localInertia);

		//using motionstate is optional, it provides interpolation capabilities, and only synchronizes 'active' objects
		btDefaultMotionState* myMotionState = new btDefaultMotionState(groundTransform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, groundShape, localInertia);
		btRigidBody* body = new btRigidBody(rbInfo);

		//add the body to the dynamics world
		pp->world->addRigidBody(body);
	}

	{
		//create a dynamic rigidbody

		//btCollisionShape* colShape = new btBoxShape(btVector3(1,1,1));
		btCollisionShape* colShape = new btSphereShape(btScalar(1.));
		collisionShapes.push_back(colShape);

		/// Create Dynamic Objects
		btTransform startTransform;
		startTransform.setIdentity();

		btScalar mass(1.f);

		//rigidbody is dynamic if and only if mass is non zero, otherwise static
		bool isDynamic = (mass != 0.f);

		btVector3 localInertia(0, 0, 0);
		if (isDynamic)
			colShape->calculateLocalInertia(mass, localInertia);

		startTransform.setOrigin(btVector3(2, 80, 0));

		//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
		btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, colShape, localInertia);
		btRigidBody* body = new btRigidBody(rbInfo);

		pp->world->addRigidBody(body);
	}

	/// Do some simulation

	///-----stepsimulation_start-----
	for (i = 0; i < 150; i++)
	{
		pp->world->stepSimulation(1.f / 60.f, 10);

		//print positions of all objects
		for (int j = pp->world->getNumCollisionObjects() - 1; j >= 0; j--)
		{
			btCollisionObject* obj = pp->world->getCollisionObjectArray()[j];
			btRigidBody* body = btRigidBody::upcast(obj);
			btTransform trans;
			if (body && body->getMotionState())
			{
				body->getMotionState()->getWorldTransform(trans);
			}
			else
			{
				trans = obj->getWorldTransform();
			}
			printf("world pos object %d = %f,%f,%f\n", j, float(trans.getOrigin().getX()), float(trans.getOrigin().getY()), float(trans.getOrigin().getZ()));
		}
	}

	///-----stepsimulation_end-----

	//cleanup in the reverse order of creation/initialization

	///-----cleanup_start-----

	//remove the rigidbodies from the dynamics world and delete them
	for (i = pp->world->getNumCollisionObjects() - 1; i >= 0; i--)
	{
		btCollisionObject* obj = pp->world->getCollisionObjectArray()[i];
		btRigidBody* body = btRigidBody::upcast(obj);
		if (body && body->getMotionState())
		{
			delete body->getMotionState();
		}
		pp->world->removeCollisionObject(obj);
		delete obj;
	}

	//delete collision shapes
	for (int j = 0; j < collisionShapes.size(); j++)
	{
		btCollisionShape* shape = collisionShapes[j];
		collisionShapes[j] = 0;
		delete shape;
	}

	//next line is optional: it will be cleared by the destructor when the array goes out of scope
	collisionShapes.clear();
	
	return 0;
}


/*+------------------------------------------------------------------+**

open library.

**+------------------------------------------------------------------+*/
LUALIB_API int luaopen_wetgenes_bullet_core (lua_State *l)
{

	const luaL_Reg meta_world[] =
	{
		{"__gc",			lua_bullet_world_destroy},
		{0,0}
	};

	luaL_newmetatable(l, lua_bullet_world_meta_name);
	luaL_openlib(l, NULL, meta_world, 0);
	lua_pop(l,1);


	const luaL_Reg meta_shape[] =
	{
		{"__gc",			lua_bullet_shape_destroy},
		{0,0}
	};

	luaL_newmetatable(l, lua_bullet_shape_meta_name);
	luaL_openlib(l, NULL, meta_shape, 0);
	lua_pop(l,1);

	const luaL_Reg meta_body[] =
	{
		{"__gc",			lua_bullet_body_destroy},
		{0,0}
	};

	luaL_newmetatable(l, lua_bullet_body_meta_name);
	luaL_openlib(l, NULL, meta_body, 0);
	lua_pop(l,1);


	const luaL_Reg lib[] =
	{
		{"world_create",					lua_bullet_world_create},
		{"world_destroy",					lua_bullet_world_destroy},
		{"world_register",					lua_bullet_world_register},

		{"shape_create",					lua_bullet_shape_create},
		{"shape_destroy",					lua_bullet_shape_destroy},

		{"body_create",						lua_bullet_body_create},
		{"body_destroy",					lua_bullet_body_destroy},

		{"world_gravity",					lua_bullet_world_gravity},
		{"world_step",						lua_bullet_world_step},
		{"world_add_body",					lua_bullet_world_add_body},
		{"world_remove_body",				lua_bullet_world_remove_body},

		{"body_transform",					lua_bullet_body_transform},


		{"test",				lua_bullet_test},

		{0,0}
	};

	lua_newtable(l);
	luaL_openlib(l, NULL, lib, 0);
	return 1;
}

