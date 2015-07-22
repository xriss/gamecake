
#include "lua.h"
#include "lauxlib.h"

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// open library.
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
LUALIB_API int luaopen_wetgenes_opus_core(lua_State *l)
{
	const luaL_reg lib[] =
	{
//		{"BufferData",			lua_al_BufferData},
		{0,0}
	};

	lua_newtable(l);
	luaL_openlib(l, NULL, lib, 0);
	
	return 1;
}

