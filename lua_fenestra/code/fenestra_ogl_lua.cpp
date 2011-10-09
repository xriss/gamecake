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

static int core_setup(lua_State *l)
{
	struct fenestra *fenestra = (struct fenestra *)lua_touserdata(l, 1 );

	fenestra->ogl->setup(fenestra);

	lua_pushlightuserdata(l,fenestra->ogl);
	
	lua_pushstring(l,(const char*)glGetString(GL_VERSION));

	return 2;
}
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// lua
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int core_clean(lua_State *l)
{
	struct fenestra_ogl *core = (struct fenestra_ogl *)lua_touserdata(l, 1 );

	core->clean();

	return 0;
}
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// lua
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int core_getset(lua_State *l)
{
	struct fenestra_ogl *core = (struct fenestra_ogl *)lua_touserdata(l, 1 );

const char *s=lua_tostring(l,2);
f32 f;
bool b;
	if( strcmp(s,"width")==0 )
	{
		lua_pushnumber(l,core->width);
		return 1;
	}
	else
	if( strcmp(s,"height")==0 )
	{
		lua_pushnumber(l,core->height);
		return 1;
	}
	else
	if( strcmp(s,"force_diffuse")==0 )
	{
		if(lua_isnumber(l,3))
		{
			core->force_diffuse=(u32)lua_tonumber(l,3);
		}
		lua_pushnumber(l,core->force_diffuse);
		return 1;
	}
	else
	if( strcmp(s,"force_spec")==0 )
	{
		if(lua_isnumber(l,3))
		{
			core->force_spec=(u32)lua_tonumber(l,3);
		}
		lua_pushnumber(l,core->force_spec);
		return 1;
	}
	else
	if( strcmp(s,"force_gloss")==0 )
	{
		if(lua_isnumber(l,3))
		{
			core->force_gloss=(f32)lua_tonumber(l,3);
		}
		lua_pushnumber(l,core->force_gloss);
		return 1;
	}
/*
	else
	if( strcmp(s,"blend_sub")==0 )
	{
		glBlendEquation(GL_FUNC_REVERSE_SUBTRACT);
		
glBlendFunc(GL_SRC_ALPHA,GL_ONE);
glBlendEquation(GL_FUNC_REVERSE_SUBTRACT);
glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE);
glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_ONE_MINUS_SRC_COLOR);
glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE);

	}
	else
	if( strcmp(s,"blend_add")==0 )
	{
		glBlendEquation(GL_FUNC_ADD);
	}
*/
	

	return 0;
}
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// lua
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
/*
static int core_test(lua_State *l)
{
	struct fenestra_ogl *core = (struct fenestra_ogl *)lua_touserdata(l, 1 );


	struct XOX0_header *head = (struct XOX0_header *)lua_touserdata(l, 2 );

	core->test(head->data());

	return 0;
}
*/

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// lua
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int core_begin(lua_State *l)
{
	struct fenestra_ogl *core = (struct fenestra_ogl *)lua_touserdata(l, 1 );
	
s32 w=0;
s32 h=0;

	if( lua_isnumber(l,2) )
	{
		w=lua_tonumber(l,2);
	}
	
	if( lua_isnumber(l,3) )
	{
		h=lua_tonumber(l,3);
	}
	
	core->begin(w,h);
	
	return 0;
}
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// lua
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int core_clip2d(lua_State *l)
{
float xp;
float yp;
float xh;
float yh;

	struct fenestra_ogl *core = (struct fenestra_ogl *)lua_touserdata(l, 1 );

	xp=(float)lua_tonumber(l,2);
	yp=(float)lua_tonumber(l,3);
	xh=(float)lua_tonumber(l,4);
	yh=(float)lua_tonumber(l,5);

	core->clip2d(xp,yp,xh,yh);
	
	return 0;
}
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// lua
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int core_project23d(lua_State *l)
{
float aspect;
float fov;
float depth;

	struct fenestra_ogl *core = (struct fenestra_ogl *)lua_touserdata(l, 1 );

	aspect=(float)lua_tonumber(l,2);
	fov=(float)lua_tonumber(l,3);
	depth=(float)lua_tonumber(l,4);

	core->project23d(aspect,fov,depth);
	
	return 0;
}
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// lua
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int core_swap(lua_State *l)
{
	struct fenestra_ogl *core = (struct fenestra_ogl *)lua_touserdata(l, 1 );

	core->swap();
	
	return 0;
}
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// lua
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int core_target(lua_State *l)
{
int w,h,r;

	struct fenestra_ogl *core = (struct fenestra_ogl *)lua_touserdata(l, 1 );

	w=(int)lua_tonumber(l,2);
	h=(int)lua_tonumber(l,3);

	r=core->set_target(w,h);
	
	lua_pushnumber(l,r);
	
	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// read pixels baby
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int core_readpixels(lua_State *l)
{
	struct fenestra_ogl *core = (struct fenestra_ogl *)lua_touserdata(l, 1 );

	int width=(int)(core->width);
	int height=(int)(core->height);
	int size=width*height*4;

	void *dat=malloc(size);
	if(dat)
	{
		glReadPixels(0, 0, width, height, GL_BGRA, GL_UNSIGNED_BYTE, dat);
		
		lua_pushnumber(l,width);
		lua_pushnumber(l,height);
		lua_pushlstring(l,(const char *)dat,size);
		
		free(dat);
		
		return 3;
	}
	
	return 0;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// lua
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static const struct luaL_reg core_lib[] = {
	{"setup",					core_setup},
	{"clean",					core_clean},
	{"begin",					core_begin},
	{"clip2d",					core_clip2d},
	{"project23d",				core_project23d},
	{"swap",					core_swap},

	{"get",						core_getset},
	{"set",						core_getset},

// test and debug functions

	{"target",					core_target},
	{"readpixels",				core_readpixels},
		
	{NULL, NULL},
};

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// main lua functions
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
LUALIB_API int luaopen_fenestra_core_ogl_debug (lua_State *l);
LUALIB_API int luaopen_fenestra_core_ogl_xox (lua_State *l);
LUALIB_API int luaopen_fenestra_core_ogl_xsx (lua_State *l);
LUALIB_API int luaopen_fenestra_core_ogl_flat (lua_State *l);
LUALIB_API int luaopen_fenestra_core_ogl_fbo (lua_State *l);
LUALIB_API int luaopen_fenestra_core_ogl_tex (lua_State *l);

LUALIB_API int luaopen_fenestra_core_ogl (lua_State *l) {

	luaL_openlib (l, "fenestra.core.ogl", core_lib, 0);
	
	luaopen_fenestra_core_ogl_debug(l);
	lua_pop(l,1);

	luaopen_fenestra_core_ogl_xox(l);
	lua_pop(l,1);

	luaopen_fenestra_core_ogl_xsx(l);
	lua_pop(l,1);

	luaopen_fenestra_core_ogl_flat(l);
	lua_pop(l,1);

	luaopen_fenestra_core_ogl_fbo(l);
	lua_pop(l,1);

	luaopen_fenestra_core_ogl_tex(l);
	lua_pop(l,1);

	return 1;
}

