/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss Daniels 2003 http://www.XIXs.com
//
/*+-----------------------------------------------------------------------------------------------------------------+*/


#define LUA_MMAPLIBNAME	"mmap"
int luaopen_wetgenes_grdmap_core (lua_State *L);


#define MMAPHANDLE "mmap"
#define MMAPTHANDLE "mmapt"

#define LUA_grdmap_LIB_NAME "wetgenes.grdmap.core"



metamap *lua_mmap_to_mmap (lua_State *L, int index);
metamap *lua_mmap_to_mmap_from_table (lua_State *L, int index);
