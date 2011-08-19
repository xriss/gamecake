/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss 2011 http://xixs.com
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
#include "all.h"



#define UPVALUE_LIB 1
#define UPVALUE_PTR 2
#define UPVALUE_TAB 3

void lua_freetype_tab_openlib (lua_State *l, int upvalues);



//
// we can use either this string as a string identifier
// or its address as a light userdata identifier, both unique
//
const char *lua_freetype_ptr_name="freetype*ptr";


// the data pointer we are using

typedef struct freetype * part_ptr ;


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// test function
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_freetype_test (lua_State *l)
{
int error=0;
	FT_Library  library;   /* handle to library     */
	FT_Face     face;      /* handle to face object */

	error = FT_Init_FreeType( &library );
	if( !error )
	{
		error = FT_New_Face( library,
			"/usr/share/fonts/truetype/ttf-dejavu/DejaVuSans.ttf",
			0,
			&face );
	}

	lua_pushnumber(l, error);
	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// call open lib with our functions
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void lua_freetype_openlib (lua_State *l, int upvalues)
{
	const luaL_reg lib[] =
	{
		{	"test"		,	lua_freetype_test	},

		{0,0}
	};
	luaL_openlib(l, NULL, lib, upvalues);
};


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// open library.
//
/*+-----------------------------------------------------------------------------------------------------------------+*/


int luaopen_freetype (lua_State *l)
{


//	luaL_newmetatable(l, lua_freetype_ptr_name);
//	lua_freetype_ptr_openlib(l,0);
//	lua_pop(l,1);

	lua_newtable(l);
	lua_pushstring(l, LUA_freetype_LIB_NAME );
	lua_pushvalue(l, -2); // have this table as the first up value?
	lua_pushvalue(l, -1); // and we need to save one to return
	lua_freetype_openlib(l,1);
	lua_rawset(l, LUA_GLOBALSINDEX);

	return 1;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// close library.
//
/*+-----------------------------------------------------------------------------------------------------------------+*/

int luaclose_freetype (lua_State *l)
{
	lua_pushstring(l, LUA_freetype_LIB_NAME);
	lua_pushnil(l);
	lua_rawset(l, LUA_GLOBALSINDEX);

	return 0;
}
