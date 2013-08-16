/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss Daniels 2003 http://www.XIXs.com
//
/*+-----------------------------------------------------------------------------------------------------------------+*/


#define LUA_T3DLIBNAME	"t3d"
int luaopen_t3d (lua_State *L);


extern thunk3d T3D[1];



extern const char *lua_t3d_object_ptr_name;
extern int  lua_t3d_object_create (lua_State *l);
extern void lua_t3d_object_ptr_openlib (lua_State *l, int upvalues);
extern void lua_t3d_object_tab_openlib (lua_State *l, int upvalues);


extern const char *lua_t3d_scene_ptr_name;
extern int  lua_t3d_scene_create (lua_State *l);
extern void lua_t3d_scene_ptr_openlib (lua_State *l, int upvalues);
extern void lua_t3d_scene_tab_openlib (lua_State *l, int upvalues);


