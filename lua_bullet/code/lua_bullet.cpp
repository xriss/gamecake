/*+------------------------------------------------------------------+**

(C) Kriss@XIXs.com 2020 and released under the MIT license.

See https://github.com/xriss/gamecake for full notice.

**+------------------------------------------------------------------+*/

#include "all.h"

//
// we can use either this string as a string identifier
// or its address as a light userdata identifier, both unique
//
const char *lua_bullet_body_meta_name ="bullet_body*ptr";
const char *lua_bullet_shape_meta_name="bullet_shape*ptr";



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
		if(pp->config)     { delete pp->config;     pp->config=0;     }
		if(pp->dispatcher) { delete pp->dispatcher; pp->dispatcher=0; }
		if(pp->phase)      { delete pp->phase;      pp->phase=0;      }
		if(pp->solver)     { delete pp->solver;     pp->solver=0;     }
		if(pp->world)      { delete pp->world;      pp->world=0;      }
		return 0;
	}
	
	// use registry so we can find the lua table from ptr,
	// this has the side effect that this MUST be destroyed,
	// it will not be GCd as this reference will keep it alive.
	lua_pushlightuserdata(l,pp);
	lua_pushvalue(l,1); // this will be the lua world table
	lua_settable(l,LUA_REGISTRYINDEX);

	return 1;
}

static int lua_bullet_world_destroy (lua_State *l)
{	
bullet_world *pp=(bullet_world*)luaL_checkudata(l, 1 , lua_bullet_world_meta_name);

	// remove registry link
	lua_pushlightuserdata(l,pp);
	lua_pushnil(l);
	lua_settable(l,LUA_REGISTRYINDEX);

	if(pp->config)     { delete pp->config;     pp->config=0;     }
	if(pp->dispatcher) { delete pp->dispatcher; pp->dispatcher=0; }
	if(pp->phase)      { delete pp->phase;      pp->phase=0;      }
	if(pp->solver)     { delete pp->solver;     pp->solver=0;     }
	if(pp->world)      { delete pp->world;      pp->world=0;      }

	return 0;
}



/*+------------------------------------------------------------------+**

test

gamecake -e" local wbc=require('wetgenes.bullet.core') ; wbc.test( wbc.world_create({}) ) "

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


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// open library.
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
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

	const luaL_Reg lib[] =
	{
		{"world_create",					lua_bullet_world_create},
		{"world_destroy",					lua_bullet_world_destroy},


		{"test",				lua_bullet_test},

		{0,0}
	};

	lua_newtable(l);
	luaL_openlib(l, NULL, lib, 0);
	return 1;
}

