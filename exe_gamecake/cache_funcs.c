
#include "lua.h"
#include "lauxlib.h"
#include <string.h>

extern const char* wetgenes_cache_lua_modnames[];
extern const char* wetgenes_cache_lua_files[];

static const char *wetgenes_cache_find_file(const char *name)
{
	const char *data=(const char *)0;
	int i;
	for(i=0;wetgenes_cache_lua_files[i];i+=2)
	{
		if(strcmp(name,wetgenes_cache_lua_files[i])==0)
		{
			data=wetgenes_cache_lua_files[i+1];
			break;
		}
	}
	return data;
}

static const char *wetgenes_cache_find_mod(const char *name)
{
	const char *data=(const char *)0;
	int i;
	for(i=0;wetgenes_cache_lua_modnames[i];i+=2)
	{
		if(strcmp(name,wetgenes_cache_lua_modnames[i])==0)
		{
			data=wetgenes_cache_lua_modnames[i+1];
			break;
		}
	}
	if(!data) { return data; }
	return wetgenes_cache_find_file(data);
}

extern int wetgenes_cache_loader(lua_State *L)
{
	const char *name = (const char *)luaL_checkstring(L, 1);
	const char *data=wetgenes_cache_find_mod(name);
	if (!data) 
	{
		lua_pushfstring(L,"\n\tno str '%s'",name);
		return 1;  /* library not found in this path */
	}

	if( (data[0]=='#') && (data[1]=='!') ) // skip first #! line
	{
		int len=strlen(data);
		for( const char *cp=data ; cp<data+len ; cp++)
		{
			if(*cp=='\n') { data=cp; break; } // start on empty first line
		}
	}

 	if( luaL_loadbuffer(L, data,strlen(data),name) != 0 )
	{
		luaL_error(L, "error loading module %s from file %s:\n\t%s",
			name, "internal", lua_tostring(L, -1));
	}

	return 1;
}

extern void wetgenes_cache_preloader(lua_State *L)
{
// setup the ziploader (which is written in lua)
	const char *name="wetgenes.zipsloader";
	const char *data=wetgenes_cache_find_mod(name);

	int numLoaders = 0;

	lua_getglobal(L,"package");
//	lua_getfield(L, LUA_GLOBALSINDEX, "package");	// push "package"
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

	if(data)
	{
		lua_pushinteger(L, numLoaders + 1);

		if( luaL_loadbuffer(L, data,strlen(data),name) != 0 )
		{
			luaL_error(L, "error loading module %s from file %s:\n\t%s",
				name, "internal", lua_tostring(L, -1));
		}

		lua_rawset(L, -3);
		numLoaders++;
	}

// setup the string loader
	lua_pushinteger(L, numLoaders + 1);
	lua_pushcfunction(L, wetgenes_cache_loader);
	lua_rawset(L, -3);

	// Table is still on the stack.  Get rid of it now.
	lua_pop(L, 1);
}

