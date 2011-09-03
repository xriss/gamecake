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
static int core_flat_begin(lua_State *l)
{
	struct fenestra_ogl *core = (struct fenestra_ogl *)lua_touserdata(l, 1 );

	core->flat_begin();

	return 0;
}
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// lua
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int core_flat_end(lua_State *l)
{
	struct fenestra_ogl *core = (struct fenestra_ogl *)lua_touserdata(l, 1 );

	core->flat_end();
	
	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// lua
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int core_flat_print(lua_State *l)
{
	struct fenestra_ogl *core = (struct fenestra_ogl *)lua_touserdata(l, 1 );
	
const char *s;

	s=0;

	if(lua_istable(l,2))
	{
		lua_getfield(l,2,"x");
		if(lua_isnumber(l,-1))
		{
			core->font_x=(f32)lua_tonumber(l,-1);
		}
		lua_pop(l,1);
		
		lua_getfield(l,2,"y");
		if(lua_isnumber(l,-1))
		{
			core->font_y=(f32)lua_tonumber(l,-1);
		}
		lua_pop(l,1);
		
		lua_getfield(l,2,"size");
		if(lua_isnumber(l,-1))
		{
			core->font_size=(f32)lua_tonumber(l,-1);
		}
		lua_pop(l,1);
		
		lua_getfield(l,2,"color");
		if(lua_isnumber(l,-1))
		{
			core->font_color=(u32)lua_tonumber(l,-1);
		}
		lua_pop(l,1);
		
		lua_getfield(l,2,"s");
		if(lua_isstring(l,-1))
		{
			s=lua_tostring(l,-1);
		}
		lua_pop(l,1);
	}
	else
	if(lua_isstring(l,2))
	{
		s=lua_tostring(l,2);
	}
	
	if(s)
	{
		core->font_draw_string(s);
	}
	
	
	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// lua
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int core_flat_measure(lua_State *l)
{
	struct fenestra_ogl *core = (struct fenestra_ogl *)lua_touserdata(l, 1 );
	
const char *s;

	s=0;

	if(lua_istable(l,2))
	{
		lua_getfield(l,2,"size");
		if(lua_isnumber(l,-1))
		{
			core->font_size=(f32)lua_tonumber(l,-1);
		}
		lua_pop(l,1);
		
		lua_getfield(l,2,"s");
		if(lua_isstring(l,-1))
		{
			s=lua_tostring(l,-1);
		}
		lua_pop(l,1);
	}
	else
	if(lua_isstring(l,2))
	{
		s=lua_tostring(l,2);
	}
	
	if(s)
	{
		lua_pushnumber(l, core->font_width_string(s) );
		return 1;
	}
	
	
	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// lua
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int core_flat_fits(lua_State *l)
{
	struct fenestra_ogl *core = (struct fenestra_ogl *)lua_touserdata(l, 1 );
	
const char *s;
f32 wide=0;

	s=0;

	if(lua_istable(l,2))
	{
		lua_getfield(l,2,"width");
		if(lua_isnumber(l,-1))
		{
			wide=(f32)lua_tonumber(l,-1);
		}
		lua_pop(l,1);

		lua_getfield(l,2,"size");
		if(lua_isnumber(l,-1))
		{
			core->font_size=(f32)lua_tonumber(l,-1);
		}
		lua_pop(l,1);
		
		lua_getfield(l,2,"s");
		if(lua_isstring(l,-1))
		{
			s=lua_tostring(l,-1);
		}
		lua_pop(l,1);
	}
	else
	if(lua_isstring(l,2))
	{
		s=lua_tostring(l,2);
	}
	
	if(s)
	{
		lua_pushnumber(l, core->font_fit_string(s,wide) );
		return 1;
	}
	
	
	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// lua
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static const struct luaL_reg core_lib[] = {
	
	{"flat_begin",				core_flat_begin},
	{"flat_end",				core_flat_end},
	{"flat_print",				core_flat_print},
	{"flat_measure",			core_flat_measure},
	{"flat_fits",				core_flat_fits},
	
	{NULL, NULL},
};

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// main lua functions
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
LUALIB_API int luaopen_fenestra_core_ogl_flat (lua_State *l) {

	luaL_openlib (l, "fenestra.core.ogl", core_lib, 0);
	return 1;
}

