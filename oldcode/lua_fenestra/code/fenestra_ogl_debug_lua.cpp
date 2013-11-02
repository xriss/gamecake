/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss@XIXs.com 2013
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
#include "all.h"


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// lua
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int core_draw_cube(lua_State *l)
{
float f;

	struct fenestra_ogl *core = (struct fenestra_ogl *)lua_touserdata(l, 1 );

	f=(float)lua_tonumber(l,2);
	
	core->draw_cube(f);

	return 0;
}



/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// lua
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int core_debug_begin(lua_State *l)
{
	struct fenestra_ogl *core = (struct fenestra_ogl *)lua_touserdata(l, 1 );

	core->debug_font_start();

	return 0;
}
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// lua
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int core_debug_end(lua_State *l)
{
	struct fenestra_ogl *core = (struct fenestra_ogl *)lua_touserdata(l, 1 );

	core->debug_font_end();
	
	return 0;
}
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// lua
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int core_debug_print(lua_State *l)
{
	struct fenestra_ogl *core = (struct fenestra_ogl *)lua_touserdata(l, 1 );
	
const char *s;

	s=0;

	if(lua_istable(l,2))
	{
		lua_getfield(l,2,"x");
		if(lua_isnumber(l,-1))
		{
			core->debug_font_x=(f32)lua_tonumber(l,-1);
		}
		lua_pop(l,1);
		
		lua_getfield(l,2,"y");
		if(lua_isnumber(l,-1))
		{
			core->debug_font_y=(f32)lua_tonumber(l,-1);
		}
		lua_pop(l,1);
		
		lua_getfield(l,2,"size");
		if(lua_isnumber(l,-1))
		{
			core->debug_font_size=(f32)lua_tonumber(l,-1);
		}
		lua_pop(l,1);
		
		lua_getfield(l,2,"color");
		if(lua_isnumber(l,-1))
		{
			core->debug_font_color=(u32)lua_tonumber(l,-1);
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
		core->debug_font_draw_string(s);
	}
	
	
	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// lua
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int core_debug_print_alt(lua_State *l)
{
	struct fenestra_ogl *core = (struct fenestra_ogl *)lua_touserdata(l, 1 );
	
const char *s;

	s=0;

	if(lua_istable(l,2))
	{
		lua_getfield(l,2,"x");
		if(lua_isnumber(l,-1))
		{
			core->debug_font_x=(f32)lua_tonumber(l,-1);
		}
		lua_pop(l,1);
		
		lua_getfield(l,2,"y");
		if(lua_isnumber(l,-1))
		{
			core->debug_font_y=(f32)lua_tonumber(l,-1);
		}
		lua_pop(l,1);
		
		lua_getfield(l,2,"size");
		if(lua_isnumber(l,-1))
		{
			core->debug_font_size=(f32)lua_tonumber(l,-1);
		}
		lua_pop(l,1);
		
		lua_getfield(l,2,"color");
		if(lua_isnumber(l,-1))
		{
			core->debug_font_color=(u32)lua_tonumber(l,-1);
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
		core->debug_font_draw_string_alt(s);
	}
	
	
	return 0;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// lua
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int core_debug_rect(lua_State *l)
{
	struct fenestra_ogl *core = (struct fenestra_ogl *)lua_touserdata(l, 1 );
	
	f32 x1=(f32)lua_tonumber(l,2);
	f32 y1=(f32)lua_tonumber(l,3);
	
	f32 x2=(f32)lua_tonumber(l,4);
	f32 y2=(f32)lua_tonumber(l,5);

	u32 color=(u32)lua_tonumber(l,6);
	
	core->debug_rect(x1,y1,x2,y2,color);
			
	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// lua
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int core_polygon_begin(lua_State *l)
{
	struct fenestra_ogl *core = (struct fenestra_ogl *)lua_touserdata(l, 1 );
	
	core->debug_polygon_begin();
			
	return 0;
}
static int core_polygon_vertex(lua_State *l)
{
	struct fenestra_ogl *core = (struct fenestra_ogl *)lua_touserdata(l, 1 );
	
	f32 x=(f32)lua_tonumber(l,2);
	f32 y=(f32)lua_tonumber(l,3);
	u32 color=(u32)lua_tonumber(l,4);
	
	core->debug_polygon_vertex(x,y,color);
			
	return 0;
}
static int core_polygon_end(lua_State *l)
{
	struct fenestra_ogl *core = (struct fenestra_ogl *)lua_touserdata(l, 1 );
	
	core->debug_polygon_end();
			
	return 0;
}



/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// lua
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static const struct luaL_reg core_lib[] = {
	
	{"draw_cube",				core_draw_cube},

	{"debug_begin",				core_debug_begin},
	{"debug_print",				core_debug_print},
	{"debug_rect",				core_debug_rect},
	{"debug_polygon_begin",		core_polygon_begin},
	{"debug_polygon_vertex",	core_polygon_vertex},
	{"debug_polygon_end",		core_polygon_end},
	{"debug_end",				core_debug_end},
	{"debug_print_alt",			core_debug_print_alt},
	
	{NULL, NULL},
};

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// lua
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
LUALIB_API int luaopen_fenestra_core_ogl_debug (lua_State *l) {

	luaL_openlib (l, "fenestra.core.ogl", core_lib, 0);

	return 1;
}

