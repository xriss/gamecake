/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss@XIXs.com 2013
//
/*+-----------------------------------------------------------------------------------------------------------------+*/


#define LUA_grdmap_LIB_NAME "wetgenes.grdmap.core"

#ifdef __cplusplus
extern "C" {
#endif

LUALIB_API int luaopen_wetgenes_grdmap_core (lua_State *l);

#ifdef __cplusplus
};
#endif


struct grdmap *lua_grdmap_check(lua_State *L, int index);

