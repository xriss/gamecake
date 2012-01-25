/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss Daniels 2003 http://www.XIXs.com
//
/*+-----------------------------------------------------------------------------------------------------------------+*/



#ifdef __cplusplus
extern "C" {
#endif

LUALIB_API int luaopen_wetgenes_grd_core (lua_State *l);

#ifdef __cplusplus
};
#endif



#define LUA_grd_LIB_NAME "wetgenes.grd.core"

extern const char *lua_grd_ptr_name;

struct grd * lua_grd_check (lua_State *l, int idx);

