/*+------------------------------------------------------------------+**

(C) Kriss@XIXs.com 2020 and released under the MIT license.

See https://github.com/xriss/gamecake for full notice.

*/

#include "all.h"



bool gContactAddedCallback_smooth_mesh(
	btManifoldPoint& cp,
	const btCollisionObjectWrapper* colObj0Wrap,
	int partId0,
	int index0,
	const btCollisionObjectWrapper* colObj1Wrap,
	int partId1,
	int index1)
{
	btAdjustInternalEdgeContacts(cp, colObj1Wrap, colObj0Wrap, partId1, index1, 1);
	return true;
}



/*+------------------------------------------------------------------+**

A world plus all the other parts that a world needs allocated in one 
struct.

*/

const char *lua_bullet_world_meta_name="bullet_world*ptr";

typedef struct {
	btDefaultCollisionConfiguration     * config ;
	btCollisionDispatcher               * dispatcher ;
	btGhostPairCallback                 * ghost ;
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
	if(pp->ghost)      { delete pp->ghost;      pp->ghost=0;      }
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
		pp->ghost      = new btGhostPairCallback();
		pp->phase->getOverlappingPairCache()->setInternalGhostPairCallback(pp->ghost);
		pp->solver     = new btSequentialImpulseConstraintSolver;
		pp->world      = new btDiscreteDynamicsWorld( pp->dispatcher, pp->phase, pp->solver, pp->config );
	}
	catch (std::exception const& e)
	{
		lua_bullet_world_delete(pp);
		return 0;
	}

// this does not really help
//	pp->world->setApplySpeculativeContactRestitution(true);
	
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

mesh

btTriangleIndexVertexMaterialArray::btTriangleIndexVertexMaterialArray 	(
		int  	numTriangles,
		int *  	triangleIndexBase,
		int  	triangleIndexStride,
		int  	numVertices,
		btScalar *  	vertexBase,
		int  	vertexStride,
		int  	numMaterials,
		unsigned char *  	materialBase,
		int  	materialStride,
		int *  	triangleMaterialsBase,
		int  	materialIndexStride 
	)
	

		btTriangleIndexVertexArray* meshInterface = new btTriangleIndexVertexArray();
		btIndexedMesh part;

		part.m_vertexBase = (const unsigned char*)LandscapeVtx[i];
		part.m_vertexStride = sizeof(btScalar) * 3;
		part.m_numVertices = LandscapeVtxCount[i];
		part.m_triangleIndexBase = (const unsigned char*)LandscapeIdx[i];
		part.m_triangleIndexStride = sizeof(short) * 3;
		part.m_numTriangles = LandscapeIdxCount[i] / 3;
		part.m_indexType = PHY_SHORT;

		meshInterface->addIndexedMesh(part, PHY_SHORT);
		
			
*/

const char *lua_bullet_mesh_meta_name="bullet_mesh*ptr";

btStridingMeshInterface * lua_bullet_mesh_ptr (lua_State *l,int idx)
{
btStridingMeshInterface **pp=(btStridingMeshInterface**)luaL_checkudata(l, idx , lua_bullet_mesh_meta_name);
	if(!*pp) { luaL_error(l,"bullet mesh is null"); }
	return *pp;
}

static int lua_bullet_mesh_destroy (lua_State *l)
{	
btStridingMeshInterface **pp=(btStridingMeshInterface**)luaL_checkudata(l, 1 , lua_bullet_mesh_meta_name);
	if(*pp)
	{
		delete *pp;
		(*pp)=0;
	}	
	return 0;
}

static int lua_bullet_mesh_create (lua_State *l)
{	
btStridingMeshInterface **pp;

int    tnum; // triangles
int   *tptr;
size_t tlen;
int    tsiz;

int       vnum; // vertexs
btScalar *vptr;
size_t    vlen;
int       vsiz;

int            mnum; // material data
unsigned char *mptr;
size_t         mlen;
int            msiz;

int   *iptr; // material indexs
size_t ilen;
int    isiz;

// create ptr ptr userdata
	pp=(btStridingMeshInterface**)lua_newuserdata(l, sizeof(btStridingMeshInterface*));
	(*pp)=0;
	luaL_getmetatable(l, lua_bullet_mesh_meta_name);
	lua_setmetatable(l, -2);

// check inputs

	tnum=luaL_checknumber(l,1);
	tptr=(int *)lua_pack_to_const_buffer(l,2,&tlen);
	tsiz=luaL_checknumber(l,3);

	vnum=luaL_checknumber(l,4);
	vptr=(btScalar *)lua_pack_to_const_buffer(l,5,&vlen);
	vsiz=luaL_checknumber(l,6);

	mptr=0; // do we have materials?
	iptr=0;
	if( lua_isnumber(l,7) )
	{
		mnum=luaL_checknumber(l,7);
		mptr=(unsigned char *)lua_pack_to_const_buffer(l,8,&mlen);
		msiz=luaL_checknumber(l,9);

		iptr=(int *)lua_pack_to_const_buffer(l,10,&ilen);
		isiz=luaL_checknumber(l,11);
	}
	
	if(mptr)
	{
		*pp=new btTriangleIndexVertexMaterialArray( tnum,tptr,tsiz, vnum,vptr,vsiz, mnum,mptr,msiz, iptr,isiz );
	}
	else
	{
		*pp=new btTriangleIndexVertexArray( tnum,tptr,tsiz, vnum,vptr,vsiz );
	}

	return 1;
}


/*+------------------------------------------------------------------+**

shape

	btBoxShape : "box" , x/2 , y/2 , z/2
	
	btSphereShape : "sphere" , radius
	
	btCapsuleShape : "capsule" , height , radius , axis

	btCylinderShape : "cylinder" , x/2 , y/2 , z/2 , axis

	btConeShape : "cone" , height , radius , axis
	
	btMultiSphereShape : "spheres" , { radius,x,y,z, ... }

	btConvexHullShape : "points" , { x,y,z, ... }

	btHeightfieldTerrainShape : "heights" , width , length , ptr , scale , min , max , axis , type , flip

	btBvhTriangleMeshShape : "mesh" , mesh

*/

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
		// auto delete infomaps thet would have been generated from btGenerateInternalEdgeInfo
		if( (*pp)->getShapeType() == TRIANGLE_MESH_SHAPE_PROXYTYPE )
		{
			btBvhTriangleMeshShape* trimesh = (btBvhTriangleMeshShape*)(*pp);
			if( trimesh->getTriangleInfoMap() )
			{
				delete trimesh->getTriangleInfoMap();
			}
		}
		else
		if( (*pp)->getShapeType() == TERRAIN_SHAPE_PROXYTYPE )
		{
			btHeightfieldTerrainShape* terrain = (btHeightfieldTerrainShape*)(*pp);
			if( terrain->getTriangleInfoMap() )
			{
				delete terrain->getTriangleInfoMap();
			}
		}

		delete *pp;
		(*pp)=0;
	}	
	return 0;
}

static int lua_bullet_shape_create (lua_State *l)
{
btVector3 *positions;
btScalar *sizes;

const char *ap;
const char *tp;
btCollisionShape **pp;
double hx,hy,hz;
double radius;
double qx,qy,qz,qw;
int count;
int width,length;
const unsigned char *dptr;
size_t dlen;
double scale;
double hmin,hmax;
int axis,dtype,flip;
int i;
btStridingMeshInterface *mesh;
btCollisionShape *shape;
btTransform trans;
btQuaternion quat;

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
		if(0==strcmp(tp,"capsule"))
		{
			radius=luaL_checknumber(l,2);
			hy=luaL_checknumber(l,3);
			axis=1; if( lua_isnumber(l,4) ) { axis=luaL_checknumber(l,4); }
			if(axis==0)
			{
				*pp = new btCapsuleShapeX( btScalar(radius) , btScalar(hy) );
			}
			else
			if(axis==2)
			{
				*pp = new btCapsuleShapeZ( btScalar(radius) , btScalar(hy) );
			}
			else
			{
				*pp = new btCapsuleShape( btScalar(radius) , btScalar(hy) );
			}
		}
		else
		if(0==strcmp(tp,"cylinder"))
		{
			hx=luaL_checknumber(l,2);
			hy=luaL_checknumber(l,3);
			hz=luaL_checknumber(l,4);
			axis=1; if( lua_isnumber(l,5) ) { axis=luaL_checknumber(l,5); }
			if(axis==0)
			{
				*pp = new btCylinderShapeX( btVector3(hx,hy,hz) );
			}
			else
			if(axis==2)
			{
				*pp = new btCylinderShapeZ( btVector3(hx,hy,hz) );
			}
			else
			{
				*pp = new btCylinderShape( btVector3(hx,hy,hz) );
			}
		}
		else
		if(0==strcmp(tp,"cone"))
		{
			radius=luaL_checknumber(l,2);
			hy=luaL_checknumber(l,3);
			axis=1; if( lua_isnumber(l,4) ) { axis=luaL_checknumber(l,4); }
			if(axis==0)
			{
				*pp = new btConeShapeX( btScalar(radius) , btScalar(hy) );
			}
			else
			if(axis==2)
			{
				*pp = new btConeShapeZ( btScalar(radius) , btScalar(hy) );
			}
			else
			{
				*pp = new btConeShape( btScalar(radius) , btScalar(hy) );
			}
		}
		else
		if(0==strcmp(tp,"spheres"))
		{
			count=lua_objlen(l,2); // must be table
			if( count>0 )
			{
				positions=new btVector3[count/4] ;
				sizes=new btScalar[count/4] ;

				for(i=0;i<count;i+=4)
				{
					lua_rawgeti(l,2,i+1); radius=luaL_checknumber(l,-1); lua_pop(l,1);
					lua_rawgeti(l,2,i+2); hx=luaL_checknumber(l,-1); lua_pop(l,1);
					lua_rawgeti(l,2,i+3); hy=luaL_checknumber(l,-1); lua_pop(l,1);
					lua_rawgeti(l,2,i+4); hz=luaL_checknumber(l,-1); lua_pop(l,1);

					sizes[i]=btScalar(radius);
					positions[i]=btVector3(hx,hy,hz);
				}
				*pp = new btMultiSphereShape( positions , sizes , count );

				delete[] positions;
				delete[] sizes;
			}
			else
			{
				lua_pushstring(l,"missing table"); lua_error(l);
			}
		}
		else
		if(0==strcmp(tp,"points"))
		{
			count=lua_objlen(l,2); // must be table
			if( count>0 )
			{
				*pp = new btConvexHullShape();
				for(i=0;i<count;i+=3)
				{
					lua_rawgeti(l,2,i+1); hx=luaL_checknumber(l,-1); lua_pop(l,1);
					lua_rawgeti(l,2,i+2); hy=luaL_checknumber(l,-1); lua_pop(l,1);
					lua_rawgeti(l,2,i+3); hz=luaL_checknumber(l,-1); lua_pop(l,1);
					
					((btConvexHullShape*)(*pp))->addPoint( btVector3(hx,hy,hz) );
				}
			}
			else
			{
				lua_pushstring(l,"missing table"); lua_error(l);
			}
		}
		else
		if(0==strcmp(tp,"heights"))
		{
			width=luaL_checknumber(l,2);
			length=luaL_checknumber(l,3);
			dptr=lua_pack_to_const_buffer(l,4,&dlen);
			scale=luaL_checknumber(l,5);
			hmin=luaL_checknumber(l,6);
			hmax=luaL_checknumber(l,7);
			axis=luaL_checknumber(l,8);
			dtype=luaL_checknumber(l,9);
			flip=lua_toboolean(l,10);
			
			*pp = new btHeightfieldTerrainShape( width , length , (const void *)dptr , btScalar(scale) , btScalar(hmin) , btScalar(hmax) , axis , (PHY_ScalarType)dtype , (bool)flip );

		}
		else
		if(0==strcmp(tp,"mesh"))
		{
				mesh=lua_bullet_mesh_ptr(l,2);
				btBvhTriangleMeshShape *tmesh=new btBvhTriangleMeshShape(mesh,true,true);
				*pp = tmesh;
				if( lua_toboolean(l,3) ) // smooth internal edge collisions
				{
					btTriangleInfoMap* triangleInfoMap = new btTriangleInfoMap();
					btGenerateInternalEdgeInfo( tmesh, triangleInfoMap);

					gContactAddedCallback = gContactAddedCallback_smooth_mesh;
				}
		}
		else
		if(0==strcmp(tp,"compound"))
		{
			count=lua_objlen(l,2); // must be table
			if( count>0 )
			{
				*pp = new btCompoundShape();
				for(i=0;i<count;i+=1)
				{
					lua_rawgeti(l,2,i+1);
					
					length=lua_objlen(l,-1); // must be a child table { shape , px,py,pz, qx,qy,qz,qw }
					
					trans.setIdentity();
			
					lua_rawgeti(l,-1,1); lua_rawgeti(l,-1,0); shape=lua_bullet_shape_ptr(l,-1); lua_pop(l,2); // must have shape table with shape in [0]

					if(length>=4) // have position
					{
						lua_rawgeti(l,-1,2); hx=luaL_checknumber(l,-1); lua_pop(l,1);
						lua_rawgeti(l,-1,3); hy=luaL_checknumber(l,-1); lua_pop(l,1);
						lua_rawgeti(l,-1,4); hz=luaL_checknumber(l,-1); lua_pop(l,1);

						trans.setOrigin( btVector3(hx,hy,hz) );
					}
					
					if(length>=8) // have rotation
					{
						lua_rawgeti(l,-1,5); qx=luaL_checknumber(l,-1); lua_pop(l,1);
						lua_rawgeti(l,-1,6); qy=luaL_checknumber(l,-1); lua_pop(l,1);
						lua_rawgeti(l,-1,7); qz=luaL_checknumber(l,-1); lua_pop(l,1);
						lua_rawgeti(l,-1,8); qw=luaL_checknumber(l,-1); lua_pop(l,1);

						trans.setRotation( btQuaternion(qx,qy,qz,qw) );
					}

					((btCompoundShape*)(*pp))->addChildShape( trans , shape );

					lua_pop(l,1);
				}
			}
			else
			{
				lua_pushstring(l,"missing table"); lua_error(l);
			}
		}
		else
		{
			lua_pushstring(l,"unknown shape"); lua_error(l);
		}

	return 1;
}


/*+------------------------------------------------------------------+**

body

*/

const char *lua_bullet_body_meta_name="bullet_body*ptr";

btCollisionObject *  lua_bullet_body_ptr (lua_State *l,int idx)
{
btCollisionObject **pp=(btCollisionObject**)luaL_checkudata(l, idx , lua_bullet_body_meta_name);
	if(!*pp) { luaL_error(l,"bullet body is null"); }
	return *pp;
}

static int lua_bullet_body_destroy (lua_State *l)
{	
btCollisionObject **pp=(btCollisionObject**)luaL_checkudata(l, 1 , lua_bullet_body_meta_name);
	if(*pp)
	{
		if( (*pp)->getInternalType()==4 ) // is ghost?
		{
			delete *pp;
			(*pp)=0;
		}
		else
		{
			btMotionState * motion = ((btRigidBody*)(*pp))->getMotionState();
			if( motion ) { delete motion; }
			delete *pp;
			(*pp)=0;
		}
	}	
	return 0;
}

static int lua_bullet_body_create (lua_State *l)
{	
const char *tp;
btCollisionObject **pp;
btCollisionShape *shape;
double hx,hy,hz;
int count;
int i;
btScalar mass( 0.0f );
btTransform trans;

	trans.setIdentity();

// create ptr ptr userdata
	pp=(btCollisionObject**)lua_newuserdata(l, sizeof(btRigidBody*));
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
	if(0==strcmp(tp,"ghost"))
	{
		shape=lua_bullet_shape_ptr(l, 2 );
		mass=lua_tonumber(l,3);
		trans.setOrigin(btVector3( lua_tonumber(l,4) ,  lua_tonumber(l,5) , lua_tonumber(l,6) ));

		*pp = (btCollisionObject*) new btPairCachingGhostObject();
		(*pp)->setCollisionShape(shape);
		(*pp)->setWorldTransform(trans);

		(*pp)->setCollisionFlags( (*pp)->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE );

	}
	else
	{
		lua_pushstring(l,"unknown body type"); lua_error(l);
	}

	return 1;
}


/*+------------------------------------------------------------------+**

get/set world gravity

*/
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

*/
static int lua_bullet_world_step (lua_State *l)
{
btDiscreteDynamicsWorld *world=lua_bullet_world_ptr(l,1)->world;

	world->stepSimulation( lua_tonumber(l,2) ,  lua_tonumber(l,3) ,  lua_tonumber(l,4) );

	return 0;
}

/*+------------------------------------------------------------------+**

add body to world

*/
static int lua_bullet_world_add_body (lua_State *l)
{
const char *tp;
btDiscreteDynamicsWorld *world = lua_bullet_world_ptr(l,1)->world;

	tp=luaL_checkstring(l,2);
	if(0==strcmp(tp,"rigid"))
	{
		btRigidBody             *body  = (btRigidBody*)lua_bullet_body_ptr(l, 3 );
		if( lua_isnumber(l,4) )
		{
			int group=lua_tonumber(l,4);
			int mask=lua_tonumber(l,5);
			world->addRigidBody(body,group,mask);
		}
		else
		{
			world->addRigidBody(body);
		}
	}
	else
	if(0==strcmp(tp,"ghost"))
	{
		btGhostObject             *body  = (btGhostObject*)lua_bullet_body_ptr(l, 3 );
		if( lua_isnumber(l,4) )
		{
			int group=lua_tonumber(l,4);
			int mask=lua_tonumber(l,5);
			world->addCollisionObject(body,group,mask);
		}
		else
		{
			world->addCollisionObject(body);
		}
	}
	else
	{
		lua_pushstring(l,"unknown body type"); lua_error(l);
	}

	
	return 0;
}

/*+------------------------------------------------------------------+**

remove body from world

*/
static int lua_bullet_world_remove_body (lua_State *l)
{
btDiscreteDynamicsWorld *world = lua_bullet_world_ptr(l,1)->world;
btRigidBody             *body  = (btRigidBody*)lua_bullet_body_ptr(l, 2 );

	world->removeRigidBody(body);

	return 0;
}



/*+------------------------------------------------------------------+**

Perform a raytest

*/
static int lua_bullet_world_ray_test (lua_State *l)
{
btDiscreteDynamicsWorld *world = lua_bullet_world_ptr(l,1)->world;

btVector3 from;
btVector3 to;

double x,y,z;

	lua_pushstring(l,"ray"); lua_gettable(l,2);

	lua_pushnumber(l,1); lua_gettable(l,-2);
	lua_pushnumber(l,1); lua_gettable(l,-2); x=luaL_checknumber(l,-1); lua_pop(l,1);
	lua_pushnumber(l,2); lua_gettable(l,-2); y=luaL_checknumber(l,-1); lua_pop(l,1);
	lua_pushnumber(l,3); lua_gettable(l,-2); z=luaL_checknumber(l,-1); lua_pop(l,1);
	lua_pop(l,1);
	from=btVector3(x,y,z);

	lua_pushnumber(l,2); lua_gettable(l,-2);
	lua_pushnumber(l,1); lua_gettable(l,-2); x=luaL_checknumber(l,-1); lua_pop(l,1);
	lua_pushnumber(l,2); lua_gettable(l,-2); y=luaL_checknumber(l,-1); lua_pop(l,1);
	lua_pushnumber(l,3); lua_gettable(l,-2); z=luaL_checknumber(l,-1); lua_pop(l,1);
	lua_pop(l,1);
	to=btVector3(x,y,z);

	lua_pop(l,1);


	btCollisionWorld::ClosestRayResultCallback closestResults(from, to);
//	closestResults.m_flags |= btTriangleRaycastCallback::kF_FilterBackfaces;


	lua_pushstring(l,"cmask"); lua_gettable(l,2);
	if( lua_isnumber(l,-1) )
	{
		closestResults.m_collisionFilterMask=luaL_checknumber(l,-1);
	}
	lua_pop(l,1);


	world->rayTest(from, to, closestResults);

	if(closestResults.hasHit())
	{
		lua_newtable(l);
		lua_pushstring(l,"hit"); lua_pushvalue(l, -2 ); lua_settable(l,2);

		lua_pushstring(l,"fraction"); lua_pushnumber(l, closestResults.m_closestHitFraction ); lua_settable(l,-3);

		lua_newtable(l);
		lua_pushstring(l,"normal"); lua_pushvalue(l, -2 ); lua_settable(l,-4);
		lua_pushnumber(l,1); lua_pushnumber(l, closestResults.m_hitNormalWorld.getX() ); lua_settable(l,-3);
		lua_pushnumber(l,2); lua_pushnumber(l, closestResults.m_hitNormalWorld.getY() ); lua_settable(l,-3);
		lua_pushnumber(l,3); lua_pushnumber(l, closestResults.m_hitNormalWorld.getZ() ); lua_settable(l,-3);
		lua_pop(l,1);
		
		lua_pushstring(l,"body_ptr"); lua_pushlightuserdata(l, (void*)closestResults.m_collisionObject ); lua_settable(l,-3);
		
		lua_pop(l,1);
	}
	else
	{
		lua_pushstring(l,"hit"); lua_pushnil(l); lua_settable(l,2);
	}

	lua_pushvalue(l,2);

	return 1;
}

			

/*+------------------------------------------------------------------+**

get/set shape margin values

*/
static int lua_bullet_shape_margin (lua_State *l)
{
btCollisionShape *shape = lua_bullet_shape_ptr(l, 1 );

	if( lua_isnumber(l,2) )
	{
		shape->setMargin( lua_tonumber(l,2) );

/*
		switch( shape->getShapeType() )
		{
			case TRIANGLE_MESH_SHAPE_PROXYTYPE :
				((btBvhTriangleMeshShape*)shape)->recalcLocalAabb();
			break;
		}
*/

	}

	lua_pushnumber(l,shape->getMargin());

	return 1;
}


/*+------------------------------------------------------------------+**

get shape pointer

*/
static int lua_bullet_shape_ptr (lua_State *l)
{
btCollisionShape *shape = lua_bullet_shape_ptr(l, 1 );

	lua_pushlightuserdata(l,shape);

	return 1;
}

/*+------------------------------------------------------------------+**

get body position and rotation ( 7 numbers )

*/
static int lua_bullet_body_transform (lua_State *l)
{
btCollisionObject *body = (btCollisionObject*)lua_bullet_body_ptr(l, 1 );
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

//	btMotionState *motion=body->getMotionState();
//	if( motion ) { motion->getWorldTransform(trans); }
//	else { trans = body->getWorldTransform(); }

	trans = body->getWorldTransform(); // I'm not sure about what we get from the motionstate...

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

get/set body velocity

*/
static int lua_bullet_body_velocity (lua_State *l)
{
btRigidBody *body = (btRigidBody*)lua_bullet_body_ptr(l, 1 );

	if( lua_isnumber(l,2) )
	{
		body->setLinearVelocity( btVector3( lua_tonumber(l,2) ,  lua_tonumber(l,3) , lua_tonumber(l,4) ) );
	}

	btVector3 v=body->getLinearVelocity();

	lua_pushnumber(l,v.getX());
	lua_pushnumber(l,v.getY());
	lua_pushnumber(l,v.getZ());

	return 3;
}

/*+------------------------------------------------------------------+**

get/set body angular velocity

*/
static int lua_bullet_body_angular_velocity (lua_State *l)
{
btRigidBody *body = (btRigidBody*)lua_bullet_body_ptr(l, 1 );

	if( lua_isnumber(l,2) )
	{
		body->setAngularVelocity( btVector3( lua_tonumber(l,2) ,  lua_tonumber(l,3) , lua_tonumber(l,4) ) );
	}

	btVector3 v=body->getAngularVelocity();

	lua_pushnumber(l,v.getX());
	lua_pushnumber(l,v.getY());
	lua_pushnumber(l,v.getZ());

	return 3;
}

/*+------------------------------------------------------------------+**

get/set body restitution

*/
static int lua_bullet_body_restitution (lua_State *l)
{
btRigidBody *body = (btRigidBody*)lua_bullet_body_ptr(l, 1 );

	if( lua_isnumber(l,2) )
	{
		body->setRestitution( lua_tonumber(l,2) );
	}

	lua_pushnumber(l,body->getRestitution());

	return 1;
}

/*+------------------------------------------------------------------+**

get/set friction values linear,angular

*/
static int lua_bullet_body_friction (lua_State *l)
{
btRigidBody *body = (btRigidBody*)lua_bullet_body_ptr(l, 1 );

	if( lua_isnumber(l,2) )
	{
		body->setFriction( lua_tonumber(l,2) );
	}
	if( lua_isnumber(l,3) )
	{
		body->setRollingFriction( lua_tonumber(l,3) );
	}
	if( lua_isnumber(l,4) )
	{
		body->setSpinningFriction( lua_tonumber(l,4) );
	}

	lua_pushnumber(l,body->getFriction());
	lua_pushnumber(l,body->getRollingFriction());
	lua_pushnumber(l,body->getSpinningFriction());

	return 3;
}

/*+------------------------------------------------------------------+**

get/set damping values linear,angular

*/
static int lua_bullet_body_damping (lua_State *l)
{
btRigidBody *body = (btRigidBody*)lua_bullet_body_ptr(l, 1 );

	if( lua_isnumber(l,2) )
	{
		body->setDamping( lua_tonumber(l,2) , lua_tonumber(l,3) );
	}

	lua_pushnumber(l,body->getLinearDamping());
	lua_pushnumber(l,body->getAngularDamping());

	return 2;
}

/*+------------------------------------------------------------------+**

get/set ccd values radius,threshold

set both to 0 to disable CCD which is the starting default

*/
static int lua_bullet_body_ccd (lua_State *l)
{
btRigidBody *body = (btRigidBody*)lua_bullet_body_ptr(l, 1 );

	if( lua_isnumber(l,2) )
	{
		body->setCcdSweptSphereRadius( lua_tonumber(l,2) );
		body->setCcdMotionThreshold( lua_tonumber(l,3) );
	}

	lua_pushnumber(l,body->getCcdSweptSphereRadius());
	lua_pushnumber(l,body->getCcdMotionThreshold());

	return 2;
}

/*+------------------------------------------------------------------+**

get/set active state

*/
static int lua_bullet_body_active (lua_State *l)
{
btRigidBody *body = (btRigidBody*)lua_bullet_body_ptr(l, 1 );

	if( lua_isboolean(l,2) )
	{
		if( lua_toboolean(l,2) )
		{
			body->activate();
		}
		else
		{
			body->forceActivationState(WANTS_DEACTIVATION);
		}
	}

	lua_pushboolean(l, body->isActive() ? 1 : 0 );

	return 1;
}

/*+------------------------------------------------------------------+**

get/set linear factor

*/
static int lua_bullet_body_factor (lua_State *l)
{
btRigidBody *body = (btRigidBody*)lua_bullet_body_ptr(l, 1 );

	if( lua_isnumber(l,2) )
	{
		body->setLinearFactor( btVector3( lua_tonumber(l,2) ,  lua_tonumber(l,3) , lua_tonumber(l,4) ) );
	}

	btVector3 v=body->getLinearFactor();

	lua_pushnumber(l,v.getX());
	lua_pushnumber(l,v.getY());
	lua_pushnumber(l,v.getZ());

	return 3;
}


/*+------------------------------------------------------------------+**

get/set angular factor

set to non zero to enable rolling of an object when it tries to slide across a
surface.

*/
static int lua_bullet_body_angular_factor (lua_State *l)
{
btRigidBody *body = (btRigidBody*)lua_bullet_body_ptr(l, 1 );

	if( lua_isnumber(l,2) )
	{
		body->setAngularFactor( btVector3( lua_tonumber(l,2) ,  lua_tonumber(l,3) , lua_tonumber(l,4) ) );
	}

	btVector3 v=body->getAngularFactor();

	lua_pushnumber(l,v.getX());
	lua_pushnumber(l,v.getY());
	lua_pushnumber(l,v.getZ());

	return 3;
}

/*+------------------------------------------------------------------+**

get/set custom material callback flag

If set this enables the trimesh smoothing for this object.

*/
static int lua_bullet_body_custom_material_callback (lua_State *l)
{
btRigidBody *body = (btRigidBody*)lua_bullet_body_ptr(l, 1 );

	if( lua_isboolean(l,2) )
	{
		if( lua_toboolean(l,2) )
		{
			body->setCollisionFlags( body->getCollisionFlags() | btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK );
		}
		else
		{
			body->setCollisionFlags( body->getCollisionFlags() & ~btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK );
		}
	}

	lua_pushboolean(l, body->getCollisionFlags() & btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK );

	return 1;
}

/*+------------------------------------------------------------------+**

get/set body gravity

*/
static int lua_bullet_body_gravity (lua_State *l)
{
btRigidBody *body = (btRigidBody*)lua_bullet_body_ptr(l, 1 );

	if( lua_isnumber(l,2) )
	{
		body->setGravity(btVector3( lua_tonumber(l,2) ,  lua_tonumber(l,3) , lua_tonumber(l,4) ));
	}

	btVector3 v=body->getGravity();

	lua_pushnumber(l,v.getX());
	lua_pushnumber(l,v.getY());
	lua_pushnumber(l,v.getZ());

	return 3;
}


/*+------------------------------------------------------------------+**

get/set body cgroup

*/
static int lua_bullet_body_cgroup (lua_State *l)
{
btCollisionObject *body = lua_bullet_body_ptr(l, 1 );
btBroadphaseProxy *broad=body->getBroadphaseHandle();

	if( lua_isnumber(l,2) )
	{
		broad->m_collisionFilterGroup=lua_tonumber(l,2);
	}
	lua_pushnumber(l,broad->m_collisionFilterGroup);

	return 1;
}

/*+------------------------------------------------------------------+**

get/set body cmask

*/
static int lua_bullet_body_cmask (lua_State *l)
{
btCollisionObject *body = lua_bullet_body_ptr(l, 1 );
btBroadphaseProxy *broad=body->getBroadphaseHandle();

	if( lua_isnumber(l,2) )
	{
		broad->m_collisionFilterMask=lua_tonumber(l,2);
	}
	lua_pushnumber(l,broad->m_collisionFilterMask);

	return 1;
}

/*+------------------------------------------------------------------+**

get ghost body overlaps

*/
static int lua_bullet_body_overlaps (lua_State *l)
{
btPairCachingGhostObject *ghost = (btPairCachingGhostObject *)lua_bullet_body_ptr(l, 1 );

	lua_newtable(l); // return value

	for( int i = 0; i < ghost->getNumOverlappingObjects(); i++ )
	{
		btCollisionObject *body = ghost->getOverlappingObject(i);
		lua_pushnumber(l,i+1);
		lua_pushlightuserdata(l,body);
		lua_settable(l,-3);
	}
	
	return 1;
}

/*+------------------------------------------------------------------+**

get body suport point in given direction

*/
static int lua_bullet_body_support (lua_State *l)
{
btCollisionObject *body = lua_bullet_body_ptr(l, 1 );

	btVector3 direction(0,-1,0); // default to floor

	if( lua_isnumber(l,2) ) // optional direction
	{
		direction[0]=lua_tonumber(l,2);
		direction[1]=lua_tonumber(l,3);
		direction[2]=lua_tonumber(l,4);
	}
	
	btTransform trans = body->getWorldTransform();
	btConvexShape *shape=(btConvexShape*)body->getCollisionShape();

	btVector3 local_direction = (direction)* trans.getBasis();
	btVector3 local_v = shape->localGetSupportVertexWithoutMarginNonVirtual(local_direction);
	btVector3 world_v = trans(local_v);

	lua_pushnumber(l,world_v.getX());
	lua_pushnumber(l,world_v.getY());
	lua_pushnumber(l,world_v.getZ());

	return 3;
}

/*+------------------------------------------------------------------+**

Apply force.

*/
static int lua_bullet_body_force (lua_State *l)
{
btRigidBody *body = (btRigidBody*)lua_bullet_body_ptr(l, 1 );

	btVector3 position(0,0,0);
	btVector3 direction(0,0,0);

	if( lua_isnumber(l,2) ) // force
	{
		direction[0]=lua_tonumber(l,2);
		direction[1]=lua_tonumber(l,3);
		direction[2]=lua_tonumber(l,4);
	}
	
	if( lua_isnumber(l,5) ) // position
	{
		position[0]=lua_tonumber(l,5);
		position[1]=lua_tonumber(l,6);
		position[2]=lua_tonumber(l,7);
	}
	
	body->applyForce(direction,position);
	
	return 0;
}

/*+------------------------------------------------------------------+**

Apply Impulse.

*/
static int lua_bullet_body_impulse (lua_State *l)
{
btRigidBody *body = (btRigidBody*)lua_bullet_body_ptr(l, 1 );

	btVector3 position(0,0,0);
	btVector3 direction(0,0,0);

	if( lua_isnumber(l,2) ) // impulse
	{
		direction[0]=lua_tonumber(l,2);
		direction[1]=lua_tonumber(l,3);
		direction[2]=lua_tonumber(l,4);
	}
	
	if( lua_isnumber(l,5) ) // position
	{
		position[0]=lua_tonumber(l,5);
		position[1]=lua_tonumber(l,6);
		position[2]=lua_tonumber(l,7);
	}
	
	body->applyImpulse(direction,position);
	
	return 0;
}

/*+------------------------------------------------------------------+**

Remove body from world before calling this

call this function (body,shape,mass) to set new shape

then add body back into world

returns ptr to old shape which can now be freed.

*/
static int lua_bullet_body_shape (lua_State *l)
{
btRigidBody *body = (btRigidBody*)lua_bullet_body_ptr(l, 1 );
btCollisionShape *shape=lua_bullet_shape_ptr(l, 2 );
btScalar mass=lua_tonumber(l,3);

	lua_pushlightuserdata(l, body->getCollisionShape() ); // return old
	body->setCollisionShape(shape); // set new

	btVector3 localInertia(0, 0, 0);
	if(mass != 0.f)
	{
		shape->calculateLocalInertia(mass, localInertia);
	}
	body->setMassProps(mass, localInertia);
	
	return 1;
}



/*+------------------------------------------------------------------+**

get real body ptr

*/
static int lua_bullet_body_ptr (lua_State *l)
{
btCollisionObject *body = lua_bullet_body_ptr(l, 1 );

	lua_pushlightuserdata(l,body);

	return 1;
}

/*+------------------------------------------------------------------+**

open library.

*/
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


	const luaL_Reg meta_mesh[] =
	{
		{"__gc",			lua_bullet_mesh_destroy},
		{0,0}
	};

	luaL_newmetatable(l, lua_bullet_mesh_meta_name);
	luaL_openlib(l, NULL, meta_mesh, 0);
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

		{"mesh_create",						lua_bullet_mesh_create},
		{"mesh_destroy",					lua_bullet_mesh_destroy},

		{"body_create",						lua_bullet_body_create},
		{"body_destroy",					lua_bullet_body_destroy},

		{"world_gravity",					lua_bullet_world_gravity},
		{"world_step",						lua_bullet_world_step},
		{"world_add_body",					lua_bullet_world_add_body},
		{"world_remove_body",				lua_bullet_world_remove_body},
		{"world_ray_test",					lua_bullet_world_ray_test},

		{"shape_margin",					lua_bullet_shape_margin},
		{"shape_ptr",						lua_bullet_shape_ptr},

		{"body_transform",					lua_bullet_body_transform},
		{"body_velocity",					lua_bullet_body_velocity},
		{"body_restitution",				lua_bullet_body_restitution},
		{"body_friction",					lua_bullet_body_friction},
		{"body_damping",					lua_bullet_body_damping},
		{"body_ccd",						lua_bullet_body_ccd},
		{"body_active",						lua_bullet_body_active},
		{"body_factor",						lua_bullet_body_factor},
		{"body_gravity",					lua_bullet_body_gravity},
		{"body_cgroup",						lua_bullet_body_cgroup},
		{"body_cmask",						lua_bullet_body_cmask},
		{"body_ptr",						lua_bullet_body_ptr},

		{"body_angular_velocity",			lua_bullet_body_angular_velocity},
		{"body_angular_factor",				lua_bullet_body_angular_factor},

		{"body_custom_material_callback",	lua_bullet_body_custom_material_callback},
		
		{"body_overlaps",					lua_bullet_body_overlaps},
		{"body_support",					lua_bullet_body_support},

		{"body_force",						lua_bullet_body_force},
		{"body_impulse",					lua_bullet_body_impulse},

		{"body_shape",						lua_bullet_body_shape},

		{0,0}
	};

	lua_newtable(l);
	luaL_openlib(l, NULL, lib, 0);
	return 1;
}

