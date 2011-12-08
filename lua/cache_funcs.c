extern const char* wetgenes_cache_lua_mods[];

#include "lua.h"
#include "lauxlib.h"
#include <string.h>

extern int wetgenes_cache_loader(lua_State *L)
{
	const char *data=(const char *)0;
	const char *name = (const char *)luaL_checkstring(L, 1);
	int i;
	for(i=0;wetgenes_cache_lua_mods[i];i+=2)
	{
		if(strcmp(name,wetgenes_cache_lua_mods[i])==0)
		{
			data=wetgenes_cache_lua_mods[i+1];
			break;
		}
	}
	if (!data) 
	{
		lua_pushfstring(L,"\nFailed to find internal lua string %s",name);
		return 1;  /* library not found in this path */
	}
	
//		lua_pushfstring(L,"\nFound internal lua string %s of %s",name,(data));

 	if( luaL_loadstring(L, data) != 0 )
	{
		luaL_error(L, "error loading module %q from file %q:\n\t%s",
			name, "internal", lua_tostring(L, -1));
	}

	return 1;
}

extern void wetgenes_cache_preloader(lua_State *L)
{
	int numLoaders = 0;

	lua_getfield(L, LUA_GLOBALSINDEX, "package");	// push "package"
	lua_getfield(L, -1, "loaders");					// push "package.loaders"
	lua_remove(L, -2);								// remove "package"

	// Count the number of entries in package.loaders.
	// Table is now at index -2, since 'nil' is right on top of it.
	// lua_next pushes a key and a value onto the stack.
	lua_pushnil(L);
	while (lua_next(L, -2) != 0) 
	{
		lua_pop(L, 1);
		numLoaders++;
	}

	lua_pushinteger(L, numLoaders + 1);
	lua_pushcfunction(L, wetgenes_cache_loader);
	lua_rawset(L, -3);

	// Table is still on the stack.  Get rid of it now.
	lua_pop(L, 1);
}

