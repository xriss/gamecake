
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#define DFT_C 1
#include "dumbft.h"




/*

We can use this string as a string identifier or its address as a light 
userdata identifier. Both are unique values.

*/
const char *lua_dft_ptr_name="dft*ptr";


/*

check that a userdata at the given index is a dft object
return the dft_state ** if it does, otherwise return 0

*/
static dft_state ** lua_dft_get_ptr (lua_State *l, int idx)
{
dft_state **ptrptr=0;

	ptrptr = lua_touserdata(l, idx);

	if(ptrptr)
	{
		if( lua_getmetatable(l, idx) )
		{
			luaL_getmetatable(l, lua_dft_ptr_name);
			if( !lua_rawequal(l, -1, -2) )
			{
				ptrptr = 0;
			}
			lua_pop(l, 2);
			return ptrptr;
		}
	}

	return ptrptr;
}


/*

call lua_dft_get_ptr and raise an error on null ptr or *ptr then return *ptr

*/
static dft_state * lua_dft_check_ptr (lua_State *l, int idx)
{
dft_state **ptrptr=lua_dft_get_ptr(l,idx);

	if(ptrptr == 0)
	{
		luaL_error(l, "not dft userdata" );
	}

	if(*ptrptr == 0)
	{
		luaL_error(l, "null dft userdata" );
	}

	return *ptrptr;
}

/*

alloc a ptr

*/
static dft_state ** lua_dft_alloc_ptr(lua_State *l)
{
dft_state **ptrptr;
	ptrptr = (dft_state **)lua_newuserdata(l, sizeof(dft_state *));
	(*ptrptr)=0;
	luaL_getmetatable(l, lua_dft_ptr_name);
	lua_setmetatable(l, -2);
	return ptrptr;
}

/*

free pointer at given index

*/
static int lua_dft_free_ptr (lua_State *l, int idx)
{
dft_state **ptrptr=lua_dft_get_ptr(l,idx);

	if(ptrptr)
	{
		if(*ptrptr)
		{
			dft_clean(*ptrptr);
		}
		(*ptrptr)=0;
	}
	return 0;
}




/*

allocate and setup a dft state

*/
static int lua_dft_setup (lua_State *l)
{
dft_state **ptrptr;
const char *s;
const char *opts=0;

	ptrptr=lua_dft_alloc_ptr(l);

	(*ptrptr)=dft_setup(0,0);

	return 1;
}

/*

clean and free a dft state

*/
static int lua_dft_clean (lua_State *l)
{
	lua_dft_free_ptr(l, 1);
	return 0;
}


LUALIB_API int luaopen_dumbft_core (lua_State *l)
{
	const luaL_Reg lib[] =
	{
		{"setup",					lua_dft_setup},
		{"clean",					lua_dft_clean},

		{0,0}
	};

	const luaL_Reg meta[] =
	{
		{"__gc",			lua_dft_clean},
		{0,0}
	};

	luaL_newmetatable(l, lua_dft_ptr_name);
	
#if defined(LUA_VERSION_NUM) && LUA_VERSION_NUM >= 502
	luaL_setfuncs(l, meta, 0);
#else
	luaL_openlib(l, NULL, meta, 0);
#endif

	lua_pop(l,1);


	lua_newtable(l);

#if defined(LUA_VERSION_NUM) && LUA_VERSION_NUM >= 502
	luaL_setfuncs(l, lib, 0);
#else
	luaL_openlib(l, NULL, lib, 0);
#endif

	return 1;
}

