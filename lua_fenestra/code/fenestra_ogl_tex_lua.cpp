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
static int core_tex_setup(lua_State *l)
{
	struct fenestra_ogl *core = (struct fenestra_ogl *)lua_touserdata(l, 1 );
		
	struct fogl_tex *tex = (struct fogl_tex *)lua_newuserdata(l, sizeof(fogl_tex) );
	
	struct grd *g=lua_grd_check(l, 2);
	
	if(tex)
	{
		core->tex_setup(tex,g);
	}
			
	return 1;
}
static int core_tex_clean(lua_State *l)
{
	struct fenestra_ogl *core = (struct fenestra_ogl *)lua_touserdata(l, 1 );
	
	struct fogl_tex *tex = (struct fogl_tex *)lua_touserdata(l, 2 );
	
	if(tex)
	{
		core->tex_clean(tex);
	}
			
	return 0;
}
static int core_tex_bind(lua_State *l)
{
	struct fenestra_ogl *core = (struct fenestra_ogl *)lua_touserdata(l, 1 );
	
	struct fogl_tex *tex = (struct fogl_tex *)lua_touserdata(l, 2 );
	
	core->tex_bind(tex); // call even if tex is nil
	
	return 0;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// lua
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static const struct luaL_reg core_lib[] = {

	{"tex_setup",				core_tex_setup},
	{"tex_clean",				core_tex_clean},
	{"tex_bind",				core_tex_bind},
	
	{NULL, NULL},
};

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// main lua functions
//
/*+-----------------------------------------------------------------------------------------------------------------+*/

LUALIB_API int luaopen_fenestra_core_ogl_tex (lua_State *l) {

	luaL_openlib (l, "fenestra.core.ogl", core_lib, 0);
	

	return 1;
}

