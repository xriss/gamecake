/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss Daniels 2003 http://www.XIXs.com
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
#include "all.h"


thunk3d T3D[1]={0};









/*----------------------------------------------------------------------------------------------------------------------------*/
//
// load a lwo into a t3d_objec,
//
/*----------------------------------------------------------------------------------------------------------------------------*/
static int luat3d_loadobj_lwo (lua_State *L)
{
lwObject *lwobj;
t3d_object *t3dobj;

unsigned int fail_ID;
int fail_pos;

char *filename;

	filename=(char*)luaL_checkstring(L, 1);


	if(!(t3dobj=T3D->AllocObject()))
	{
		return luaL_error(L, "Failed to allocate t3d object.");
	}

	if(!(lwobj=lwGetObject(filename,&fail_ID,&fail_pos)))
	{
		T3D->FreeObject(t3dobj);
		return luaL_error(L, "Failed to load lwo \"%s\" error=%d pos=%d.",filename,fail_ID,fail_pos);
	}


	if(!(t3dobj->FillObject(lwobj,1)))
	{
		T3D->FreeObject(t3dobj);
		lwFreeObject(lwobj);
		return luaL_error(L, "Failed to  parse lwo \"%s\" into t3d obj.",filename);
	}

	lwFreeObject(lwobj);

	lua_pushlightuserdata(L, (void*)t3dobj);
	return 1;
}


/*----------------------------------------------------------------------------------------------------------------------------*/
//
// execute a string,
//
/*----------------------------------------------------------------------------------------------------------------------------*/
static int luat3d_saveobj_xox (lua_State *L)
{
t3d_object *item;
const char *filename;


	if(!(lua_type(L,1) == LUA_TLIGHTUSERDATA))
	{
		return luaL_error(L, "not a pointer.");
	}
	item=(t3d_object *)lua_touserdata(L,1);


	if(!(lua_type(L,2) == LUA_TSTRING))
	{
		return luaL_error(L, "not a string.");
	}
	filename=lua_tostring(L,2);


	if(!item->SaveXOX(filename))
	{
		return luaL_error(L, "failed to save \"%s\" as .XOX file.",filename);
	}


	return 0;
}


/*----------------------------------------------------------------------------------------------------------------------------*/
//
// execute a string,
//
/*----------------------------------------------------------------------------------------------------------------------------*/
static int luat3d_freeobj (lua_State *L)
{
t3d_object *item;

	if(!lua_islightuserdata(L,1))
	{
		return luaL_error(L, "not a pointer.");
	}
	item=(t3d_object *)lua_touserdata(L,1);

	T3D->FreeObject(item);


	return 0;
}

/*----------------------------------------------------------------------------------------------------------------------------*/
//
// load a lws into a t3d_scene,
//
/*----------------------------------------------------------------------------------------------------------------------------*/
static int luat3d_loadscene_lws (lua_State *L)
{
t3d_scene *t3dscene;


char *filename;

	filename=(char*)luaL_checkstring(L, 1);


	if(!(t3dscene=T3D->AllocScene()))
	{
		return luaL_error(L, "Failed to allocate t3d scene.");
	}


	if(!(t3dscene->LoadLWS(filename)))
	{
		T3D->FreeScene(t3dscene);
		return luaL_error(L, "Failed to  parse lws \"%s\" into t3d scene.",filename);
	}

	lua_pushlightuserdata(L, (void*)t3dscene);
	return 1;
}

/*----------------------------------------------------------------------------------------------------------------------------*/
//
// execute a string,
//
/*----------------------------------------------------------------------------------------------------------------------------*/
static int luat3d_savescene_xsx(lua_State *L)
{
t3d_scene *item;
const char *filename;


	if(!(lua_type(L,1) == LUA_TLIGHTUSERDATA))
	{
		return luaL_error(L, "not a pointer.");
	}
	item=(t3d_scene *)lua_touserdata(L,1);


	if(!(lua_type(L,2) == LUA_TSTRING))
	{
		return luaL_error(L, "not a string.");
	}
	filename=lua_tostring(L,2);


	if(!item->SaveXSX(filename))
	{
		return luaL_error(L, "failed to save \"%s\" as .XSX file.",filename);
	}


	return 0;
}

/*----------------------------------------------------------------------------------------------------------------------------*/
//
// execute a string,
//
/*----------------------------------------------------------------------------------------------------------------------------*/
static int luat3d_freescene (lua_State *L)
{
t3d_scene *scene;

	if(!lua_islightuserdata(L,1))
	{
		return luaL_error(L, "not a pointer.");
	}
	scene=(t3d_scene *)lua_touserdata(L,1);

	T3D->FreeScene(scene);


	return 0;
}







/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// call open lib with our functions
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void lua_t3d_openlib (lua_State *l, int upvalues)
{
	const luaL_reg lib[] =
	{
		{	"create_object"	,	lua_t3d_object_create	},
		{	"create_scene"	,	lua_t3d_scene_create	},


// old functions
		{	"loadscene_lws"	,	luat3d_loadscene_lws	},
		{	"savescene_xsx"	,   luat3d_savescene_xsx	},
		{	"freescene"		,	luat3d_freescene		},

		{	"loadobj_lwo"	,   luat3d_loadobj_lwo		},
		{	"saveobj_xox"	,   luat3d_saveobj_xox		},
		{	"freeobj"		,   luat3d_freeobj			},
// old functions

		{0,0}
	};
	luaL_openlib(l, NULL, lib, upvalues);
};

/*----------------------------------------------------------------------------------------------------------------------------*/
//
// open library.
//
/*----------------------------------------------------------------------------------------------------------------------------*/

int luaopen_t3d (lua_State *l)
{

	luaL_newmetatable(l, lua_t3d_object_ptr_name);
	lua_t3d_object_ptr_openlib(l,0);
	lua_pop(l,1);

	luaL_newmetatable(l, lua_t3d_scene_ptr_name);
	lua_t3d_scene_ptr_openlib(l,0);
	lua_pop(l,1);


	lua_pushstring(l, LUA_T3DLIBNAME );
	lua_newtable(l);
	lua_pushvalue(l, -1); // have this table as the first up value
	lua_t3d_openlib(l,1);
	lua_rawset(l, LUA_GLOBALSINDEX);

	return 1;
}
