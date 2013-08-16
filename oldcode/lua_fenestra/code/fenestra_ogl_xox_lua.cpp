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
static int core_xox_setup(lua_State *l)
{
	struct fenestra_ogl *core = (struct fenestra_ogl *)lua_touserdata(l, 1 );
	
	struct XOX0_header *xox_head = (struct XOX0_header *)lua_touserdata(l, 2 );
	struct XOX0_info   *xox_info = xox_head->data();
	
	struct XOX *xox = (struct XOX *)lua_newuserdata(l, core->xox_sizeof(xox_info) );
	
	if(xox)
	{
		core->xox_setup(xox,xox_info);
	}
			
	return 1;
}
static int core_xox_clean(lua_State *l)
{
	struct fenestra_ogl *core = (struct fenestra_ogl *)lua_touserdata(l, 1 );
	
	struct XOX *xox = (struct XOX *)lua_touserdata(l, 2 );
	
	if(xox)
	{
		core->xox_clean(xox);
	}
			
	return 0;
}
static int core_xox_draw(lua_State *l)
{
	struct fenestra_ogl *core = (struct fenestra_ogl *)lua_touserdata(l, 1 );
	
	struct XOX *xox = (struct XOX *)lua_touserdata(l, 2 );
	
	if(xox)
	{
		core->xox_update(xox);
		core->xox_draw(xox);
	}
			
	return 0;
}
static int core_xox_get(lua_State *l)
{
XOX_surface *surf;
int i;

	struct fenestra_ogl *core = (struct fenestra_ogl *)lua_touserdata(l, 1 );
	
	struct XOX *xox = (struct XOX *)lua_touserdata(l, 2 );
	
	lua_newtable(l);

	if(xox)
	{
		lua_newtable(l);
		
		for( surf=xox->surfaces , i=1 ; surf < xox->surfaces + xox->numof_surfaces ; surf++ , i++ )
		{
			lua_newtable(l);
			
			lua_pushstring(l,surf->name);
			lua_setfield(l,-2,"name");
			
			lua_pushnumber(l,surf->argb);
			lua_setfield(l,-2,"argb");
			
			lua_pushnumber(l,surf->spec);
			lua_setfield(l,-2,"spec");
			
			lua_pushnumber(l,surf->gloss);
			lua_setfield(l,-2,"gloss");
			
			lua_rawseti(l,-2,i);
		}
		
		lua_setfield(l,-2,"surfaces");
	}
			
	return 1;
}
static int core_xox_set(lua_State *l)
{
XOX_surface *surf;
int i;

	struct fenestra_ogl *core = (struct fenestra_ogl *)lua_touserdata(l, 1 );
	
	struct XOX *xox = (struct XOX *)lua_touserdata(l, 2 );
	
	if(xox)
	{
		lua_getfield(l,3,"surfaces");
		
		for( surf=xox->surfaces , i=1 ; surf < xox->surfaces + xox->numof_surfaces ; surf++ , i++ )
		{
			lua_rawgeti(l,-1,i);
			
			lua_getfield(l,-1,"argb");
			surf->argb=(u32)lua_tonumber(l,-1);
			lua_pop(l,1);
			
			lua_getfield(l,-1,"spec");
			surf->spec=(u32)lua_tonumber(l,-1);
			lua_pop(l,1);
			
			lua_getfield(l,-1,"gloss");
			surf->gloss=(f32)lua_tonumber(l,-1);
			lua_pop(l,1);
			
			lua_pop(l,1);
		}
		
		lua_pop(l,1);
	}
			
	return 0;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// lua
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static const struct luaL_reg core_lib[] = {

	{"xox_setup",				core_xox_setup},
	{"xox_clean",				core_xox_clean},
	{"xox_draw",				core_xox_draw},
	{"xox_get",					core_xox_get},
	{"xox_set",					core_xox_set},
	
	{NULL, NULL},
};

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// main lua functions
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
LUALIB_API int luaopen_fenestra_core_ogl_xox (lua_State *l) {

	luaL_openlib (l, "fenestra.core.ogl", core_lib, 0);
	return 1;
}

