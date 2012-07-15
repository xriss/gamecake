
#include "lua.h"
#include "lauxlib.h"

#include <android/log.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// Print
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
// Print
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_android_time (lua_State *l)
{
	struct timeval tv ;
	gettimeofday ( & tv, NULL ) ;
	lua_pushnumber(l, (double) tv.tv_sec + (double) tv.tv_usec / 1000000.0 );
	return 1;
}



/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// open library.
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
LUALIB_API int luaopen_wetgenes_win_android(lua_State *l)
{
	const luaL_reg lib[] =
	{

		{"print",				lua_android_print},
		{"time",				lua_android_time},
		
		{0,0}
	};

	lua_newtable(l);
	luaL_openlib(l, NULL, lib, 0);
	
	return 1;
}

