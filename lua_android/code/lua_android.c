
#include "lua.h"
#include "lauxlib.h"

#include <android/log.h>


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// Clear
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_android_print (lua_State *l)
{
	const char *s=lua_tostring(l,1);

	__android_log_print(ANDROID_LOG_INFO, "lua", s);

	return 0;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// open library.
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
LUALIB_API int luaopen_wetgenes_android_core(lua_State *l)
{
	const luaL_reg lib[] =
	{

		{"print",				lua_android_print},
		
		{0,0}
	};

	lua_newtable(l);
	luaL_openlib(l, NULL, lib, 0);
	
	return 1;
}

