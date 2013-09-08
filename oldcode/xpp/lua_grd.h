/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss Daniels 2003 http://www.XIXs.com
//
/*+-----------------------------------------------------------------------------------------------------------------+*/


 int luaopen_grd (lua_State *l);
 int luaclose_grd (lua_State *l);


#define LUA_grd_LIB_NAME "grd"

extern const char *lua_grd_ptr_name;




s32 lua_grd_check (lua_State *l, int idx);

