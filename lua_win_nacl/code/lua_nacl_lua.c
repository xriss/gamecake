
// this file handles the basic nacl interface setup
// the creation of the master lua state under nacl
// as well as the lua interface into nacl (wetgenes.nacl.core)

#include "lua_nacl.h"


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// manage some lua memory (userdata) which keeps itself alive with registry references
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// alloc some memory
//
void *lua_nacl_mem_alloc(lua_State *l,int size)
{
	void *mem=lua_newuserdata(l,size);
	
	if(mem)
	{
		lua_pushlightuserdata(l,mem);
		lua_pushvalue(l,-2);
		lua_settable(l,LUA_REGISTRYINDEX);
		
		lua_pop(l,1);
	}
	
	return mem;
}
//
// stop keeping this memory referenced, it will GC later on
//
void lua_nacl_mem_deref(lua_State *l,void *mem)
{
	lua_pushlightuserdata(l,mem);
	lua_pushnil(l);
	lua_settable(l,LUA_REGISTRYINDEX);
}
//
// push the userdata onto the stack
//
void lua_nacl_mem_push(lua_State *l,void *mem)
{
	lua_pushlightuserdata(l,mem);
	lua_gettable(l,LUA_REGISTRYINDEX);
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// nacl callback util functions, keep your state lua side
//
/*+-----------------------------------------------------------------------------------------------------------------+*/

//
// Free the callback
//
void lua_nacl_callback_free (lua_State *l, struct lua_nacl_callback * cb)
{
	if(cb)
	{
		lua_pushlightuserdata(l,cb);
		lua_pushnil(l);
		lua_settable(l,LUA_REGISTRYINDEX); // remove from registry
		
		free(cb); // free our memory
	}
}
//
// The actual nacl callback function, which then calls into the main lua thread
//
void lua_nacl_callback_func(void* user_data, int32_t result)
{
struct lua_nacl_callback *cb=(struct lua_nacl_callback *)user_data;
lua_State *l=cb->l; // use this lua state

		lua_pushlightuserdata(l,cb);
		lua_gettable(l,LUA_REGISTRYINDEX); // get the lua table associated with this callback
		
		if( lua_isfunction(l,-1) ) // sanity
		{
			lua_pushnumber(l,result); // give the result code ( use upvalues for state )
			if(cb->ret)
			{
				lua_nacl_mem_push(l,cb->ret); // add result userdata if we have any
				lua_call(l,2,0);
				lua_nacl_mem_deref(l,cb->ret); // let it be gc when finished with
			}
			else
			{
				lua_call(l,1,0);
			}
		}
		else // hmmm, insane, just remove whatever it was
		{
			lua_pop(l,1);
		}
		
		lua_nacl_callback_free(l,cb); // and free it
}
//
// Create a callback structure
//
struct lua_nacl_callback * lua_nacl_callback_alloc (lua_State *l,int func)
{
	struct lua_nacl_callback *cb=0;
	
	cb = calloc(1,sizeof(struct lua_nacl_callback));
	
	if(cb)
	{
		cb->l=l;
		
		cb->pp->flags=PP_COMPLETIONCALLBACK_FLAG_NONE;
		cb->pp->func=lua_nacl_callback_func;
		cb->pp->user_data=cb;
		
		lua_pushlightuserdata(l,cb);
		lua_pushvalue(l,func);

		lua_settable(l,LUA_REGISTRYINDEX); // store in registy so we can access from callback
	}

	return cb;
}

