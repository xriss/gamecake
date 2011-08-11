#include <lua.h>
#include <lauxlib.h>
#include <string.h>
#include "types.h"

static int priv_tostring(lua_State* L)
{
	luaL_checktype(L, 1, LUA_TUSERDATA);
	Object* obj = lua_touserdata(L, 1);

	char name[31];
	snprintf(name, 30, "Pointer: %p", obj->pointer);
	lua_pushstring(L, name);

	return 1;
}

static int priv_eq(lua_State* L)
{
	luaL_checktype(L, 1, LUA_TUSERDATA);
	luaL_checktype(L, 2, LUA_TUSERDATA);
	Object* obj1 = lua_touserdata(L, 1);
	Object* obj2 = lua_touserdata(L, 2);

	lua_pushboolean(L, obj1->pointer == obj2->pointer);
	
	return 1;
}

/**
 * Registers a special object (ie, an Object thats not a GObject)
 *
 * @param 1 Metatable name of the userdata
 * @param 2 Namespace name of the class (like lgob). Can be nil
 * @param 3 Class, as a string
 * @param 4 GC function
 * @param 5 tostring function
 * @param 6 eq function
 */
static int lgob_register_special(lua_State* L)
{
    const char* namespace = lua_tostring(L, 2);
	const char* class = lua_tostring(L, 3);
	
    /* __index */
	if(namespace)
	{
    	lua_getfield(L, LUA_GLOBALSINDEX, namespace);
    	lua_pushliteral(L, "__index");
		lua_getfield(L, -2, class);
	}
	else
	{
		lua_pushliteral(L, "__index");
		lua_getfield(L, LUA_GLOBALSINDEX, class);
	}
    
	lua_rawset(L, 1);
	
	/* __gc */
	if(lua_isfunction(L, 4))
	{
		lua_pushliteral(L, "__gc");	
		lua_pushvalue(L, 4);
		lua_rawset(L, 1);
	}
	
	/* __tostring */
	lua_pushliteral(L, "__tostring");
		
	if(lua_isfunction(L, 5))
		lua_pushvalue(L, 5);
	else
		lua_pushcfunction(L, priv_tostring);
		
	lua_rawset(L, 1);
	
	/* __eq */
	lua_pushliteral(L, "__eq");
	
	if(lua_isfunction(L, 6))
		lua_pushvalue(L, 6);
	else
		lua_pushcfunction(L, priv_eq);
		
	lua_rawset(L, 1);
	
	return 0;
}

int luaopen_lgob_common(lua_State* L)
{
    lua_pushcfunction(L, lgob_register_special);
	lua_setfield(L, LUA_REGISTRYINDEX, "lgobRegisterSpecial");
	
	lua_pushnil(L);
	return 1;
}
