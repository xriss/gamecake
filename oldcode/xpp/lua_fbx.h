/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss@XIXs.com 2013
//
/*+-----------------------------------------------------------------------------------------------------------------+*/

#ifndef WET_DISABLE_FBX

int luaopen_fbx (lua_State *l);
int luaclose_fbx (lua_State *l);


#define LUA_fbx_LIB_NAME "fbx"

extern const char *lua_fbx_ptr_name;

extern FBXSDK_NAMESPACE::KFbxNode* lua_fbx_find_node(lua_State *l,FBXSDK_NAMESPACE::KFbxNode* pNode,const char *name);

extern t3d_object * fbx_mesh_into_t3d_object(FBXSDK_NAMESPACE::KFbxNode* pNode, int layer);

extern t3d_scene * fbx_scene_into_t3d_scene(FBXSDK_NAMESPACE::KFbxScene* kscene, thunk3d *T3D);

#endif
