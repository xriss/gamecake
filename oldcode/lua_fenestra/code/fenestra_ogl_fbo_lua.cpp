/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss Daniels 2003 http://www.XIXs.com
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
#include "all.h"

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// lua
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int core_fbo_setup(lua_State *l)
{
	struct fenestra_ogl *core = (struct fenestra_ogl *)lua_touserdata(l, 1 );
		
	struct fogl_fbo *fbo = (struct fogl_fbo *)lua_newuserdata(l, sizeof(fogl_fbo) );
	
	fbo->width=(s32)lua_tonumber(l,2);
	fbo->height=(s32)lua_tonumber(l,3);
	fbo->depth=0;
	if(lua_isnumber(l,4)) { fbo->depth=(s32)lua_tonumber(l,4); } // optional depth buffer should be 0 or 24
	
	if(fbo)
	{
		core->fbo_setup(fbo);
	}
			
	return 1;
}
static int core_fbo_clean(lua_State *l)
{
	struct fenestra_ogl *core = (struct fenestra_ogl *)lua_touserdata(l, 1 );
	
	struct fogl_fbo *fbo = (struct fogl_fbo *)lua_touserdata(l, 2 );
	
	if(fbo)
	{
		core->fbo_clean(fbo);
	}
			
	return 0;
}
static int core_fbo_bind(lua_State *l)
{
	struct fenestra_ogl *core = (struct fenestra_ogl *)lua_touserdata(l, 1 );
	
	struct fogl_fbo *fbo = (struct fogl_fbo *)lua_touserdata(l, 2 );
	
	core->fbo_bind(fbo); // call even if fbo is nil
	
	return 0;
}
static int core_fbo_texture(lua_State *l)
{
	struct fenestra_ogl *core = (struct fenestra_ogl *)lua_touserdata(l, 1 );
	
	struct fogl_fbo *fbo = (struct fogl_fbo *)lua_touserdata(l, 2 );
	
	if(fbo)
	{
		core->fbo_texture(fbo);
	}

	return 0;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// lua
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static const struct luaL_reg core_lib[] = {

	{"fbo_setup",				core_fbo_setup},
	{"fbo_clean",				core_fbo_clean},
	{"fbo_bind",				core_fbo_bind},
	{"fbo_texture",				core_fbo_texture},
	
	{NULL, NULL},
};

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// main lua functions
//
/*+-----------------------------------------------------------------------------------------------------------------+*/

LUALIB_API int luaopen_fenestra_core_ogl_fbo (lua_State *l) {

	luaL_openlib (l, "fenestra.core.ogl", core_lib, 0);
	

	return 1;
}

