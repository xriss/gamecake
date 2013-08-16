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
const char *lua_t3d_scene_ptr_name="t3ds*ptr";


// the data pointer we are using

typedef t3d_scene * part_ptr ;

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// get userdata from upvalue, no need to test for type
// just error on null
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
part_ptr lua_t3d_scene_get_ptr (lua_State *l, int idx)
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
int lua_t3d_scene_create (lua_State *l)
{
part_ptr *p;
//const char *s;

int idx_ud;

	p = (part_ptr *)lua_newuserdata(l, sizeof(part_ptr));
	
	idx_ud=lua_gettop(l);

	(*p)=0;

	luaL_getmetatable(l, lua_t3d_scene_ptr_name);
	lua_setmetatable(l, -2);

	lua_newtable(l);

// main lib and userdata are stored as upvalues in the function calls for easy/fast access

	lua_pushvalue(l, lua_upvalueindex(UPVALUE_LIB) ); // get our base table
	lua_pushvalue(l, idx_ud ); // get our userdata,

	lua_t3d_scene_tab_openlib(l,2);

// remember the userdata in the table as well as the upvalue

	lua_pushstring(l, lua_t3d_scene_ptr_name );
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
int lua_t3d_scene_destroy_idx (lua_State *l, int idx)
{
part_ptr *p;
	
	p = (part_ptr *)luaL_checkudata(l, idx, lua_t3d_scene_ptr_name);

	if(*p)
	{
		T3D->FreeScene(*p);
	}
	(*p)=0;

	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// __GC for ptr
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_t3d_scene_destroy_ptr (lua_State *l)
{
	return lua_t3d_scene_destroy_idx(l,1);
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// delete the pointer data and set pointer to 0
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_t3d_scene_destroy (lua_State *l)
{
	return lua_t3d_scene_destroy_idx(l,lua_upvalueindex(UPVALUE_PTR));
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// return table containing information about this scene
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_t3d_scene_info (lua_State *l)
{
part_ptr p;

	p = lua_t3d_scene_get_ptr(l,lua_upvalueindex(UPVALUE_PTR));

	lua_newtable(l);

	lua_pushliteral(l,"numof_items");			lua_pushnumber(l,p->numof_items);			lua_rawset(l,-3);
//	lua_pushliteral(l,"numof_streams");			lua_pushnumber(l,p->numof_streams);			lua_rawset(l,-3);
//	lua_pushliteral(l,"numof_keys");			lua_pushnumber(l,p->numof_keys);			lua_rawset(l,-3);


	lua_pushliteral(l,"first_frame");			lua_pushnumber(l,p->first_frame);			lua_rawset(l,-3);
	lua_pushliteral(l,"last_frame");			lua_pushnumber(l,p->last_frame);			lua_rawset(l,-3);
	lua_pushliteral(l,"frames_per_second");		lua_pushnumber(l,p->frames_per_second);		lua_rawset(l,-3);


	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// get / set animation funcs
//
// table to get /set from is at -1 on the stack
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void lua_t3d_get_scene_item_stream_key ( lua_State *l , t3d_scene*scene , t3d_item*item , t3d_stream*stream , t3d_key*key )
{
const char *types[]={
		"TCB",
		"HERMITE",
		"",
		"LINEAR",
		"STEP",
		"BEZIER",
		"",
		"",
};

	lua_pushliteral(l,"id");			lua_pushnumber(l,key->id+1);				lua_rawset(l,-3);

	lua_pushliteral(l,"type");			lua_pushstring(l,types[key->type]);			lua_rawset(l,-3);

	lua_pushliteral(l,"time");			lua_pushnumber(l,key->time);				lua_rawset(l,-3);
	lua_pushliteral(l,"value");			lua_pushnumber(l,key->value);				lua_rawset(l,-3);

	lua_pushliteral(l,"time_b1");		lua_pushnumber(l,key->bezier_time[0]);		lua_rawset(l,-3);
	lua_pushliteral(l,"value_b1");		lua_pushnumber(l,key->bezier_value[0]);		lua_rawset(l,-3);

	lua_pushliteral(l,"time_b2");		lua_pushnumber(l,key->bezier_time[1]);		lua_rawset(l,-3);
	lua_pushliteral(l,"value_b2");		lua_pushnumber(l,key->bezier_value[1]);		lua_rawset(l,-3);

	lua_pushliteral(l,"t");				lua_pushnumber(l,key->t);					lua_rawset(l,-3);
	lua_pushliteral(l,"c");				lua_pushnumber(l,key->c);					lua_rawset(l,-3);
	lua_pushliteral(l,"b");				lua_pushnumber(l,key->b);					lua_rawset(l,-3);

}

void lua_t3d_set_scene_item_stream_key ( lua_State *l , t3d_scene*scene , t3d_item*item , t3d_stream*stream , t3d_key*key )
{
const char *s;
s32 i;
const char *types[]={
		"TCB",
		"HERMITE",
		"",
		"LINEAR",
		"STEP",
		"BEZIER",
		"",
		"",
};

	s=types[0];
	
	lua_pushliteral(l,"type");			lua_rawget(l,-2);	if(!lua_isnil(l,-1)){	s=lua_tostring(l,-1);						}	lua_pop(l,1);

	key->type=0;
	for(i=0;i<8;i++)
	{
		if(strcmp(s,types[i])==0)
		{
			key->type=i;
			break;
		}
	}

	lua_pushliteral(l,"time");			lua_rawget(l,-2);	if(!lua_isnil(l,-1)){	key->time=(f32)lua_tonumber(l,-1);				}	lua_pop(l,1);
	lua_pushliteral(l,"value");			lua_rawget(l,-2);	if(!lua_isnil(l,-1)){	key->value=(f32)lua_tonumber(l,-1);				}	lua_pop(l,1);

	lua_pushliteral(l,"time_b1");		lua_rawget(l,-2);	if(!lua_isnil(l,-1)){	key->bezier_time[0]=(f32)lua_tonumber(l,-1);	}	lua_pop(l,1);
	lua_pushliteral(l,"value_b1");		lua_rawget(l,-2);	if(!lua_isnil(l,-1)){	key->bezier_value[0]=(f32)lua_tonumber(l,-1);	}	lua_pop(l,1);

	lua_pushliteral(l,"time_b2");		lua_rawget(l,-2);	if(!lua_isnil(l,-1)){	key->bezier_time[1]=(f32)lua_tonumber(l,-1);	}	lua_pop(l,1);
	lua_pushliteral(l,"value_b2");		lua_rawget(l,-2);	if(!lua_isnil(l,-1)){	key->bezier_value[1]=(f32)lua_tonumber(l,-1);	}	lua_pop(l,1);


	lua_pushliteral(l,"t");				lua_rawget(l,-2);	if(!lua_isnil(l,-1)){	key->t=(f32)lua_tonumber(l,-1);					}	lua_pop(l,1);
	lua_pushliteral(l,"c");				lua_rawget(l,-2);	if(!lua_isnil(l,-1)){	key->c=(f32)lua_tonumber(l,-1);					}	lua_pop(l,1);
	lua_pushliteral(l,"b");				lua_rawget(l,-2);	if(!lua_isnil(l,-1)){	key->b=(f32)lua_tonumber(l,-1);					}	lua_pop(l,1);

}

void lua_t3d_get_scene_item_stream ( lua_State *l , t3d_scene*scene , t3d_item*item , t3d_stream*stream )
{
	lua_pushliteral(l,"name");					lua_pushstring(l,stream->name);				lua_rawset(l,-3);
	lua_pushliteral(l,"numof_keys");			lua_pushnumber(l,stream->numof_keys);		lua_rawset(l,-3);
}

void lua_t3d_get_scene_item (lua_State *l , t3d_scene*scene ,t3d_item*item)
{
t3d_item*child;
s32 i;

	lua_pushliteral(l,"name");			lua_pushstring(l,item->name);			lua_rawset(l,-3);
	lua_pushliteral(l,"type");			lua_pushstring(l,item->type);			lua_rawset(l,-3);
	lua_pushliteral(l,"id");			lua_pushnumber(l,item->id+1);			lua_rawset(l,-3);

	if( item->parent )
	{
		lua_pushliteral(l,"parent_id");		lua_pushnumber(l,item->parent->id+1);			lua_rawset(l,-3);
	}
	else
	{
		lua_pushliteral(l,"parent_id");		lua_pushnumber(l,0);			lua_rawset(l,-3);
	}

	item->fill_rest_values();

	lua_pushliteral(l,"posx");		lua_pushnumber(l,item->rest_values[0]);			lua_rawset(l,-3);
	lua_pushliteral(l,"posy");		lua_pushnumber(l,item->rest_values[1]);			lua_rawset(l,-3);
	lua_pushliteral(l,"posz");		lua_pushnumber(l,item->rest_values[2]);			lua_rawset(l,-3);

	lua_pushliteral(l,"rotx");		lua_pushnumber(l,item->rest_values[3]);			lua_rawset(l,-3);
	lua_pushliteral(l,"roty");		lua_pushnumber(l,item->rest_values[4]);			lua_rawset(l,-3);
	lua_pushliteral(l,"rotz");		lua_pushnumber(l,item->rest_values[5]);			lua_rawset(l,-3);

	lua_pushliteral(l,"sizx");		lua_pushnumber(l,item->rest_values[6]);			lua_rawset(l,-3);
	lua_pushliteral(l,"sizy");		lua_pushnumber(l,item->rest_values[7]);			lua_rawset(l,-3);
	lua_pushliteral(l,"sizz");		lua_pushnumber(l,item->rest_values[8]);			lua_rawset(l,-3);

	child=0;
	i=0;
	while(child=scene->child_item(item,child))
	{
		i++;
	}

	lua_pushliteral(l,"numof_children");			lua_pushnumber(l,i);							lua_rawset(l,-3);
	lua_pushliteral(l,"numof_streams");				lua_pushnumber(l,item->numof_streams);			lua_rawset(l,-3);

}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// return table containing information about the relationship between items, the item tree heirachy
//
/*+-----------------------------------------------------------------------------------------------------------------+*/

void lua_t3d_scene_tree_item (lua_State *l , t3d_scene*scene , t3d_item*item)
{
t3d_item*child;
s32 i;

	lua_checkstack(l,8); // make sure we have room to breath

	lua_t3d_get_scene_item(l,scene,item);


	child=0;
	i=0;
	while(child=scene->child_item(item,child))
	{
		i++;

		lua_pushnumber(l,i);
		lua_newtable(l);

		lua_t3d_scene_tree_item(l,scene,child);

		lua_rawset(l,-3);
	}
}

int lua_t3d_scene_tree (lua_State *l)
{
part_ptr p;
t3d_item*item;

	p = lua_t3d_scene_get_ptr(l,lua_upvalueindex(UPVALUE_PTR));

	lua_newtable(l);

	item=p->master_item();

	lua_t3d_scene_tree_item(l,p,item);
	

	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// return table containing information about this item (an item is a reference to an object, or a null reference, or a bone etc)
//
/*+-----------------------------------------------------------------------------------------------------------------+*/

int lua_t3d_scene_item (lua_State *l)
{
part_ptr p;
t3d_item*item;

	p = lua_t3d_scene_get_ptr(l,lua_upvalueindex(UPVALUE_PTR));

	item=0;

	if( lua_isnumber(l,2) )
	{
		item=p->find_item(((s32)lua_tonumber(l,2))-1);
	}

	if(item)
	{
		lua_newtable(l);

		lua_t3d_get_scene_item(l,p,item);

		return 1;
	}
	else
	{
		luaL_error(l, "couldn't find item" );
	}

	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// return table containing information about this stream, a stream is a collection of keys, 9 basic streams per item pos,rot,scale (x,y,z)
//
// but we may also have or need more
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_t3d_scene_stream (lua_State *l)
{
part_ptr p;
t3d_item*item;
t3d_stream*stream;

	p = lua_t3d_scene_get_ptr(l,lua_upvalueindex(UPVALUE_PTR));

	item=0;
	stream=0;

	if( lua_isnumber(l,2) )
	{
		item=p->find_item(((s32)lua_tonumber(l,2))-1);
	}

	if(item)
	{

		if( lua_isnumber(l,3) )
		{
			stream=item->find_stream(((s32)lua_tonumber(l,3))-1);
		}

		if(stream)
		{
			lua_newtable(l);

			lua_t3d_get_scene_item_stream(l,p,item,stream);

			return 1;
		}
		else
		{
			luaL_error(l, "couldn't find stream" );
		}
	}
	else
	{
		luaL_error(l, "couldn't find item" );
	}


	return 1;
}
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// delete all keys belonging to this stream
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_t3d_scene_stream_delete_keys (lua_State *l)
{
part_ptr p;
t3d_item*item;
t3d_stream*stream;

	p = lua_t3d_scene_get_ptr(l,lua_upvalueindex(UPVALUE_PTR));

	item=0;
	stream=0;

	if( lua_isnumber(l,2) )
	{
		item=p->find_item(((s32)lua_tonumber(l,2))-1);
	}

	if(item)
	{

		if( lua_isnumber(l,3) )
		{
			stream=item->find_stream(((s32)lua_tonumber(l,3))-1);
		}

		if(stream)
		{
			stream->delete_keys();

			return 1;
		}
		else
		{
			luaL_error(l, "couldn't find stream" );
		}
	}
	else
	{
		luaL_error(l, "couldn't find item" );
	}


	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// return table containing information about this key, a key is a point of time with a set value and control points
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_t3d_scene_key (lua_State *l)
{
part_ptr p;
t3d_item*item;
t3d_stream*stream;
t3d_key*key;

	p = lua_t3d_scene_get_ptr(l,lua_upvalueindex(UPVALUE_PTR));

	item=0;
	stream=0;

	if( lua_isnumber(l,2) )
	{
		item=p->find_item(((s32)lua_tonumber(l,2))-1);
	}

	if(item)
	{

		if( lua_isnumber(l,3) )
		{
			stream=item->find_stream(((s32)lua_tonumber(l,3))-1);
		}

		if(stream)
		{
			if( lua_isnumber(l,4) )
			{
				key=stream->find_key(((s32)lua_tonumber(l,4))-1);
			}

			if( lua_istable(l,5) )
			{
				if(!key)	// in this case we can create a new key
				{
					if( ! ( key = T3D->AllocKey() ) ) { luaL_error(l, "couldn't create key" ); }
					DLIST_CUTPASTE(stream->keys->last,key,0);
					stream->numof_keys++;
				}
			}

			if(key)
			{
				if( lua_istable(l,5) )
				{
					lua_pushvalue(l,5);

					lua_t3d_set_scene_item_stream_key(l,p,item,stream,key);

					stream->sort_keys();
					stream->reID_keys();

					lua_t3d_get_scene_item_stream_key(l,p,item,stream,key);

					return 1;
				}
				else
				{
					lua_newtable(l);

					lua_t3d_get_scene_item_stream_key(l,p,item,stream,key);

					return 1;
				}
			}
			else
			{
				luaL_error(l, "couldn't find key" );
			}
		}
		else
		{
			luaL_error(l, "couldn't find stream" );
		}
	}
	else
	{
		luaL_error(l, "couldn't find item" );
	}


	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// fill in object from an fbx data
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
#ifndef WET_DISABLE_FBX
int lua_t3d_scene_fbx (lua_State *l)
{
part_ptr *p;
FBXSDK_NAMESPACE::KFbxScene ** scene ;

	p = (part_ptr *)luaL_checkudata(l, lua_upvalueindex(UPVALUE_PTR), lua_t3d_scene_ptr_name);
	
// clean up any old object we had as we are going to create a brand spanking new one

	lua_t3d_scene_destroy_idx(l,lua_upvalueindex(UPVALUE_PTR));

// get scene pointer from arg2

	lua_pushstring(l, lua_fbx_ptr_name );
	lua_rawget(l,2);
	scene = (FBXSDK_NAMESPACE::KFbxScene **)luaL_checkudata(l, lua_gettop(l) , lua_fbx_ptr_name);

	if(!(*scene)) // couldnt find, so error
	{
		luaL_error(l, "couldn't find fbx scene" );
	}


	*p=fbx_scene_into_t3d_scene(*scene,T3D);

	return 0;
}
#endif
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// call open lib with our ptr functions
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void lua_t3d_scene_ptr_openlib (lua_State *l, int upvalues)
{
const luaL_reg lib[] =
	{
		{"__gc",			lua_t3d_scene_destroy_ptr},

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
void lua_t3d_scene_tab_openlib (lua_State *l, int upvalues)
{
const luaL_reg lib[] =
	{
#ifndef WET_DISABLE_FBX
		{"fbx",					lua_t3d_scene_fbx},
#endif
		{"destroy",				lua_t3d_scene_destroy},

		{"info",				lua_t3d_scene_info},
		{"tree",				lua_t3d_scene_tree},
		{"item",				lua_t3d_scene_item},
		{"stream",				lua_t3d_scene_stream},
		{"key",					lua_t3d_scene_key},

		{"stream_delete_keys",	lua_t3d_scene_stream_delete_keys},

		{0,0}
	};
	luaL_openlib(l, NULL, lib, upvalues);
}







