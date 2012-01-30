/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss Daniels 2003 http://www.XIXs.com
//
/*+-----------------------------------------------------------------------------------------------------------------+*/


#define LUA_HMAPLIBNAME	"hmap"
int luaopen_hmap (lua_State *L);


#define HMAPHANDLE "hmap"



heightmap *lua_hmap_to_hmap (lua_State *L, int index);

