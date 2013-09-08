/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss Daniels 2003 http://www.XIXs.com
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
#include "all.h"



#define UPVALUE_LIB 1
#define UPVALUE_PTR 2




//
// we can use either this string as a string identifier
// or its address as a light userdata identifier, both unique
//
const char *lua_t3d_object_ptr_name="t3do*ptr";


// the data pointer we are using

typedef t3d_object * part_ptr ;

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// get userdata from upvalue, no need to test for type
// just error on null
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
part_ptr lua_t3d_object_get_ptr (lua_State *l, int idx)
{
part_ptr p;

	p=(part_ptr )(*(void **)lua_touserdata(l,idx));

	if (p == NULL)
	{
		luaL_error(l, "null pointer in t3d object usedata" );
	}

	return p;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// alloc an item, returns table that you can modify and associate extra data with
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_t3d_object_create (lua_State *l)
{
part_ptr *p;
//const char *s;

int idx_ud;

	p = (part_ptr *)lua_newuserdata(l, sizeof(part_ptr));
	
	idx_ud=lua_gettop(l);

	(*p)=0;

	luaL_getmetatable(l, lua_t3d_object_ptr_name);
	lua_setmetatable(l, -2);

	lua_newtable(l);

// main lib and userdata are stored as upvalues in the function calls for easy/fast access

	lua_pushvalue(l, lua_upvalueindex(UPVALUE_LIB) ); // get our base table
	lua_pushvalue(l, idx_ud ); // get our userdata,

	lua_t3d_object_tab_openlib(l,2);

// remember the userdata in the table as well as the upvalue

	lua_pushstring(l, lua_t3d_object_ptr_name );
	lua_pushvalue(l, idx_ud ); // get our userdata,
	lua_rawset(l,-3);


	lua_remove(l, idx_ud );
	return 1;
}



/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// destroy pointer
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_t3d_object_destroy_idx (lua_State *l, int idx)
{
part_ptr *p;
	
	p = (part_ptr *)luaL_checkudata(l, idx, lua_t3d_object_ptr_name);

	if(*p)
	{
		T3D->FreeObject(*p);
	}
	(*p)=0;

	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// __GC for ptr
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_t3d_object_destroy_ptr (lua_State *l)
{
	return lua_t3d_object_destroy_idx(l,1);
}
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// delete the pointer data and set pointer to 0
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_t3d_object_destroy (lua_State *l)
{
	return lua_t3d_object_destroy_idx(l,lua_upvalueindex(UPVALUE_PTR));
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// return table containing information about this object
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_t3d_object_info (lua_State *l)
{
part_ptr p;

	p = lua_t3d_object_get_ptr(l,lua_upvalueindex(UPVALUE_PTR));

	lua_newtable(l);


	lua_pushliteral(l,"numof_bones");			lua_pushnumber(l,p->numof_bones);		lua_rawset(l,-3);
	lua_pushliteral(l,"numof_points");			lua_pushnumber(l,p->numof_points);		lua_rawset(l,-3);
	lua_pushliteral(l,"numof_maps");			lua_pushnumber(l,p->numof_maps);		lua_rawset(l,-3);
	lua_pushliteral(l,"numof_polys");			lua_pushnumber(l,p->numof_polys);		lua_rawset(l,-3);
	lua_pushliteral(l,"numof_polyindexs");		lua_pushnumber(l,p->numof_polyindexs);	lua_rawset(l,-3);
	lua_pushliteral(l,"numof_polystrips");		lua_pushnumber(l,p->numof_polystrips);	lua_rawset(l,-3);
	lua_pushliteral(l,"numof_surfaces");		lua_pushnumber(l,p->numof_surfaces);	lua_rawset(l,-3);
	lua_pushliteral(l,"numof_morphs");			lua_pushnumber(l,p->numof_morphs);		lua_rawset(l,-3);

	lua_pushliteral(l,"maxrad");				lua_pushnumber(l,p->maxrad);			lua_rawset(l,-3);

	lua_pushliteral(l,"min");
	lua_newtable(l);
	lua_pushliteral(l,"x");			lua_pushnumber(l,p->min->x);		lua_rawset(l,-3);
	lua_pushliteral(l,"y");			lua_pushnumber(l,p->min->y);		lua_rawset(l,-3);
	lua_pushliteral(l,"z");			lua_pushnumber(l,p->min->z);		lua_rawset(l,-3);
	lua_rawset(l,-3);

	lua_pushliteral(l,"max");
	lua_newtable(l);
	lua_pushliteral(l,"x");			lua_pushnumber(l,p->max->x);		lua_rawset(l,-3);
	lua_pushliteral(l,"y");			lua_pushnumber(l,p->max->y);		lua_rawset(l,-3);
	lua_pushliteral(l,"z");			lua_pushnumber(l,p->max->z);		lua_rawset(l,-3);
	lua_rawset(l,-3);


	return 1;
}
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// return table containing information about this objects surface of the given ID
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_t3d_object_surface (lua_State *l)
{
part_ptr p;
s32 num;
t3d_surface *surface;

bool input;

	p = lua_t3d_object_get_ptr(l,lua_upvalueindex(UPVALUE_PTR));

	num=(s32)lua_tonumber(l,2);

	input=lua_istable(l,3);

	if(input) // return table passed in
	{
		lua_pushvalue(l,3);
	}
	else
	{
		lua_newtable(l);
	}

	surface=p->findsurface(num-1);

	if( surface )
	{

		if(input) // set according to inputs
		{
		const char *s;
		f32 f;
			
			lua_pushliteral(l,"name");		lua_rawget(l,2);
			if(!lua_isnil(l,-1))
			{
				s=lua_tostring(l,-1);
				strncpy( surface->name , s , sizeof( surface->name ) ); surface->name[ sizeof( surface->name ) -1 ]=0;
			}
			lua_pop(l,1);

			lua_pushliteral(l,"a");			lua_rawget(l,2);
			if(!lua_isnil(l,-1))
			{
				f=(f32)lua_tonumber(l,-1);
				surface->a=f;
			}
			lua_pop(l,1);

			lua_pushliteral(l,"r");			lua_rawget(l,2);
			if(!lua_isnil(l,-1))
			{
				f=(f32)lua_tonumber(l,-1);
				surface->r=f;
			}
			lua_pop(l,1);

			lua_pushliteral(l,"g");			lua_rawget(l,2);
			if(!lua_isnil(l,-1))
			{
				f=(f32)lua_tonumber(l,-1);
				surface->g=f;
			}
			lua_pop(l,1);

			lua_pushliteral(l,"b");			lua_rawget(l,2);
			if(!lua_isnil(l,-1))
			{
				f=(f32)lua_tonumber(l,-1);
				surface->b=f;
			}
			lua_pop(l,1);

			lua_pushliteral(l,"sa");		lua_rawget(l,2);
			if(!lua_isnil(l,-1))
			{
				f=(f32)lua_tonumber(l,-1);
				surface->sa=f;
			}
			lua_pop(l,1);

			lua_pushliteral(l,"sr");		lua_rawget(l,2);
			if(!lua_isnil(l,-1))
			{
				f=(f32)lua_tonumber(l,-1);
				surface->sr=f;
			}
			lua_pop(l,1);

			lua_pushliteral(l,"sg");		lua_rawget(l,2);
			if(!lua_isnil(l,-1))
			{
				f=(f32)lua_tonumber(l,-1);
				surface->sg=f;
			}
			lua_pop(l,1);

			lua_pushliteral(l,"sb");		lua_rawget(l,2);
			if(!lua_isnil(l,-1))
			{
				f=(f32)lua_tonumber(l,-1);
				surface->sb=f;
			}
			lua_pop(l,1);

			lua_pushliteral(l,"gloss");		lua_rawget(l,2);
			if(!lua_isnil(l,-1))
			{
				f=(f32)lua_tonumber(l,-1);
				surface->gloss=f;
			}
			lua_pop(l,1);

		}

		// write current settings into output table (this may be the input table but it will pick up any rounding errors)

		lua_pushliteral(l,"name");			lua_pushstring(l,surface->name);		lua_rawset(l,-3);

		lua_pushliteral(l,"a");				lua_pushnumber(l,surface->a);			lua_rawset(l,-3);
		lua_pushliteral(l,"r");				lua_pushnumber(l,surface->r);			lua_rawset(l,-3);
		lua_pushliteral(l,"g");				lua_pushnumber(l,surface->g);			lua_rawset(l,-3);
		lua_pushliteral(l,"b");				lua_pushnumber(l,surface->b);			lua_rawset(l,-3);

		lua_pushliteral(l,"sa");			lua_pushnumber(l,surface->sa);			lua_rawset(l,-3);
		lua_pushliteral(l,"sr");			lua_pushnumber(l,surface->sr);			lua_rawset(l,-3);
		lua_pushliteral(l,"sg");			lua_pushnumber(l,surface->sg);			lua_rawset(l,-3);
		lua_pushliteral(l,"sb");			lua_pushnumber(l,surface->sb);			lua_rawset(l,-3);

		lua_pushliteral(l,"gloss");			lua_pushnumber(l,surface->gloss);		lua_rawset(l,-3);
	}

// return table, this may be the table we passed in, no other values will have been killed

	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// return table containing information about this objects point of the given ID
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_t3d_object_point (lua_State *l)
{
part_ptr p;
s32 num;
t3d_point *point;

bool input;

	p = lua_t3d_object_get_ptr(l,lua_upvalueindex(UPVALUE_PTR));

	num=(s32)lua_tonumber(l,2);

	input=lua_istable(l,3);

	if(input) // return table passed in
	{
		lua_pushvalue(l,3);
	}
	else
	{
		lua_newtable(l);
	}

	point=p->findpoint(num-1);

	if( point )
	{

		if(input) // set according to inputs
		{
		}

		// write current settings into output table (this may be the input table but it will pick up any rounding errors)

		lua_pushliteral(l,"id");			lua_pushnumber(l,point->id+1);		lua_rawset(l,-3);

		lua_pushliteral(l,"x");				lua_pushnumber(l,point->x);			lua_rawset(l,-3);
		lua_pushliteral(l,"y");				lua_pushnumber(l,point->y);			lua_rawset(l,-3);
		lua_pushliteral(l,"z");				lua_pushnumber(l,point->z);			lua_rawset(l,-3);

		lua_pushliteral(l,"nx");			lua_pushnumber(l,point->nx);			lua_rawset(l,-3);
		lua_pushliteral(l,"ny");			lua_pushnumber(l,point->ny);			lua_rawset(l,-3);
		lua_pushliteral(l,"nz");			lua_pushnumber(l,point->nz);			lua_rawset(l,-3);

		if( point->bone )
		{
			lua_pushliteral(l,"bone_id");			lua_pushnumber(l,point->bone->id+1);			lua_rawset(l,-3);
		}

	}

// return table, this may be the table we passed in, no other values except sub tables will have been killed

	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// return table containing information about this objects map of the given ID
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_t3d_object_map (lua_State *l)
{
part_ptr p;
s32 num;
t3d_point *point;

bool input;

	p = lua_t3d_object_get_ptr(l,lua_upvalueindex(UPVALUE_PTR));

	num=(s32)lua_tonumber(l,2);

	input=lua_istable(l,3);

	if(input) // return table passed in
	{
		lua_pushvalue(l,3);
	}
	else
	{
		lua_newtable(l);
	}

	point=p->findmap(num-1);

	if( point )
	{

		if(input) // set according to inputs
		{
		}

		// write current settings into output table (this may be the input table but it will pick up any rounding errors)

		lua_pushliteral(l,"id");			lua_pushnumber(l,point->id+1);		lua_rawset(l,-3);

		lua_pushliteral(l,"x");				lua_pushnumber(l,point->x);			lua_rawset(l,-3);
		lua_pushliteral(l,"y");				lua_pushnumber(l,point->y);			lua_rawset(l,-3);
	}

// return table, this may be the table we passed in, no other values will have been killed

	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// return table containing information about this objects poly of the given ID
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_t3d_object_poly (lua_State *l)
{
part_ptr p;
s32 num;
t3d_poly *poly;

bool input;

s32 i;

	p = lua_t3d_object_get_ptr(l,lua_upvalueindex(UPVALUE_PTR));

	num=(s32)lua_tonumber(l,2);

	input=lua_istable(l,3);

	if(input) // return table passed in
	{
		lua_pushvalue(l,3);
	}
	else
	{
		lua_newtable(l);
	}

	poly=p->findpoly(num-1);

	if( poly )
	{

		if(input) // set according to inputs
		{
		}

		// write current settings into output table (this may be the input table but it will pick up any rounding errors)

		lua_pushliteral(l,"id");			lua_pushnumber(l,poly->id+1);		lua_rawset(l,-3);

		if(poly->surface)
		{
			lua_pushliteral(l,"surface");		lua_pushnumber(l,poly->surface->id+1);		lua_rawset(l,-3);
		}


		for( i=0 ; i<3 ; i++ )
		{

			lua_pushnumber(l,i+1);
			lua_newtable(l);

			if(poly->points[i])
			{
				lua_pushliteral(l,"point");			lua_pushnumber(l,poly->points[i]->id+1);	lua_rawset(l,-3);
				lua_pushliteral(l,"x");				lua_pushnumber(l,poly->points[i]->x);		lua_rawset(l,-3);
				lua_pushliteral(l,"y");				lua_pushnumber(l,poly->points[i]->y);		lua_rawset(l,-3);
				lua_pushliteral(l,"z");				lua_pushnumber(l,poly->points[i]->z);		lua_rawset(l,-3);
				lua_pushliteral(l,"nx");			lua_pushnumber(l,poly->points[i]->nx);		lua_rawset(l,-3);
				lua_pushliteral(l,"ny");			lua_pushnumber(l,poly->points[i]->ny);		lua_rawset(l,-3);
				lua_pushliteral(l,"nz");			lua_pushnumber(l,poly->points[i]->nz);		lua_rawset(l,-3);

				if( poly->points[i]->bone )
				{
					lua_pushliteral(l,"bone");		lua_pushnumber(l,poly->points[i]->bone->id+1);		lua_rawset(l,-3);
				}
			}

			if(poly->maps[i])
			{
				lua_pushliteral(l,"map");			lua_pushnumber(l,poly->maps[i]->id+1);		lua_rawset(l,-3);
				lua_pushliteral(l,"u");				lua_pushnumber(l,poly->maps[i]->x);			lua_rawset(l,-3);
				lua_pushliteral(l,"v");				lua_pushnumber(l,poly->maps[i]->y);			lua_rawset(l,-3);
			}
			

			lua_rawset(l,-3);
		}
	}

// return table, this may be the table we passed in, no other values except sub tables will have been killed

	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// return table containing information about this objects bone of the given ID
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_t3d_object_bone (lua_State *l)
{
part_ptr p;
s32 num;
t3d_bone *bone;

bool input;

//s32 i;

	p = lua_t3d_object_get_ptr(l,lua_upvalueindex(UPVALUE_PTR));

	num=(s32)lua_tonumber(l,2);

	input=lua_istable(l,3);

	if(input) // return table passed in
	{
		lua_pushvalue(l,3);
	}
	else
	{
		lua_newtable(l);
	}

	bone=p->findbone(num-1);

	if( bone )
	{

		if(input) // set according to inputs
		{
		}

		// write current settings into output table (this may be the input table but it will pick up any rounding errors)

		lua_pushliteral(l,"id");			lua_pushnumber(l,bone->id+1);			lua_rawset(l,-3);
		lua_pushliteral(l,"name0");			lua_pushstring(l,bone->name[0]);		lua_rawset(l,-3);
		lua_pushliteral(l,"weight0");		lua_pushnumber(l,bone->weight[0]);		lua_rawset(l,-3);
		lua_pushliteral(l,"name1");			lua_pushstring(l,bone->name[1]);		lua_rawset(l,-3);
		lua_pushliteral(l,"weight1");		lua_pushnumber(l,bone->weight[1]);		lua_rawset(l,-3);


	}

// return table, this may be the table we passed in, no other values will have been killed

	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// fill in object from an fbx data
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
#ifndef WET_DISABLE_FBX
int lua_t3d_object_fbx (lua_State *l)
{
part_ptr *p;
FBXSDK_NAMESPACE::KFbxNode * node;
FBXSDK_NAMESPACE::KFbxScene ** scene ;

const char *s;

	s=lua_tostring(l,3);	// must always give name of node we are talking about

	p = (part_ptr *)luaL_checkudata(l, lua_upvalueindex(UPVALUE_PTR), lua_t3d_object_ptr_name);

	
// clean up any old object we had as we are going to create a brand spanking new one

	lua_t3d_object_destroy_idx(l,lua_upvalueindex(UPVALUE_PTR));

// get scene pointer from arg2

	lua_pushstring(l, lua_fbx_ptr_name );
	lua_rawget(l,2);
	scene = (FBXSDK_NAMESPACE::KFbxScene **)luaL_checkudata(l, lua_gettop(l) , lua_fbx_ptr_name);

	if(!(*scene)) // couldnt find, so error
	{
		luaL_error(l, "couldn't find fbx scene" );
	}

// find node, arg3 is name

	node=lua_fbx_find_node(l,(*scene)->GetRootNode(),s);

	if(!node) // couldnt find, so error
	{
		luaL_error(l, "couldn't find fbx node" );
	}

	*p=fbx_mesh_into_t3d_object(node,0);

	return 0;
}
#endif
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// build a missing part of an object
//
// specify one of the following strings, or everything will be built
//
// "bounds"
// "normals"
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_t3d_object_build (lua_State *l)
{
part_ptr p;
const char *s;

	s=0;

	if(lua_isstring(l,2))
	{
		s=lua_tostring(l,2);	// must always give name of node we are talking about
	}

	p = lua_t3d_object_get_ptr(l,lua_upvalueindex(UPVALUE_PTR));

	if( (!s) || (strcmp(s,"bounds")==0) )
	{
		p->BuildBounds();
	}

	if( (!s) || (strcmp(s,"normals")==0) )
	{
		p->BuildNormals();
	}

	if( (!s) || (strcmp(s,"bones")==0) )
	{
		p->BuildBones();
	}

	return 0;
}
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// sort part of an object
//
// specify one of the following strings, or everything will be sorted
//
// "points"
// "polys"
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_t3d_object_sort (lua_State *l)
{
part_ptr p;
const char *s;

	s=0;

	if(lua_isstring(l,2))
	{
		s=lua_tostring(l,2);
	}

	p = lua_t3d_object_get_ptr(l,lua_upvalueindex(UPVALUE_PTR));


	if( (!s) || (strcmp(s,"points")==0) )
	{
		p->SortPoints();
	}

	if( (!s) || (strcmp(s,"polys")==0) )
	{
		p->SortPolys();
	}


	return 0;
}



/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// call open lib with our ptr functions
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void lua_t3d_object_ptr_openlib (lua_State *l, int upvalues)
{
const luaL_reg lib[] =
	{
		{"__gc",			lua_t3d_object_destroy_ptr},

		{0,0}
	};
	luaL_openlib(l, NULL, lib, upvalues);
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// call open lib with our tab functions
//
// all functions expect the self table to be passed in as arg1
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void lua_t3d_object_tab_openlib (lua_State *l, int upvalues)
{
const luaL_reg lib[] =
	{
#ifndef WET_DISABLE_FBX
		{"fbx",				lua_t3d_object_fbx},
#endif
		{"destroy",			lua_t3d_object_destroy},

		{"info",			lua_t3d_object_info},
		{"surface",			lua_t3d_object_surface},
		{"point",			lua_t3d_object_point},
		{"map",				lua_t3d_object_map},
		{"poly",			lua_t3d_object_poly},
		{"bone",			lua_t3d_object_bone},

		{"build",			lua_t3d_object_build},
		{"sort",			lua_t3d_object_sort},

		{0,0}
	};
	luaL_openlib(l, NULL, lib, upvalues);
}







