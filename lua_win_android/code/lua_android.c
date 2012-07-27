

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <signal.h>
#include <string.h>

#include <unistd.h>
#include <sys/time.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>


#include <android/log.h>

#include "lua.h"
#include "lauxlib.h"

#include "EGL/egl.h"
#include <GLES/gl.h>

#include "android_native_app_glue.h"

#include "lua_android.h"


#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "lua", __VA_ARGS__))



/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// Print
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_android_print (lua_State *l)
{
	const char *s=lua_tostring(l,1);

	__android_log_print(ANDROID_LOG_INFO, "lua", s);

	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// Print
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_android_time (lua_State *l)
{
	struct timeval tv ;
	gettimeofday ( & tv, NULL ) ;
	lua_pushnumber(l, (double) tv.tv_sec + (double) tv.tv_usec / 1000000.0 );
	return 1;
}



//
// we can use either this string as a string identifier
// or its address as a light userdata identifier, both will be unique
//
const char *lua_android_ptr_name="android*ptr";

	
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// check that a userdata at the given index is a android object
// return the userdata if it does, otherwise return 0
// this userdata will be a pointer to the real data
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
android_lua **lua_android_ptr_ptr (lua_State *l, int idx)
{
android_lua **pp=0;

	pp = ((android_lua **)luaL_checkudata(l, idx , lua_android_ptr_name));

	return pp;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// *lua_android_check with auto error on 0 ptr
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
android_lua *lua_android_check_ptr (lua_State *l, int idx)
{
android_lua **pp=lua_android_ptr_ptr(l,idx);

	if (*pp == 0)
	{
		luaL_error(l, "bad android userdata" );
	}

	return *pp;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// fill a table in with the current settings
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_android_getinfo (lua_State *l, android_lua *p, int tab)
{
	if(p)
	{
		if(p->display && p->surface)
		{
			eglQuerySurface(p->display, p->surface, EGL_WIDTH, &p->width);
			eglQuerySurface(p->display, p->surface, EGL_HEIGHT, &p->height);
		}
		lua_pushnumber(l,p->width);		lua_setfield(l,tab,"width");
		lua_pushnumber(l,p->height);	lua_setfield(l,tab,"height");
	}
	else
	{
		lua_pushstring(l,"unbound android"); lua_setfield(l,tab,"err");
	}

	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// set info into the given table
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_android_info (lua_State *l)
{
android_lua *p=lua_android_check_ptr(l,1);
	lua_android_getinfo(l,p,2);
	return 0;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// create and return the data
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_android_create (lua_State *l)
{
android_lua_wrap *wp;
android_lua *p;

int x=0;
int y=0;

const char *title="http://www.WetGenes.com/ - GameCake";


EGLBoolean result;
int32_t success = 0;

EGLint format;

EGLConfig config;
EGLint num_config;
EGLint attribute_list[] =
{
	
// hard hax, please not to be reordering
	EGL_RED_SIZE, 1,	//	[1]	r
	EGL_GREEN_SIZE, 1,	//	[3]	g
	EGL_BLUE_SIZE, 1,	//	[5]	b
	EGL_ALPHA_SIZE, 0,	//	[7]	a
	EGL_DEPTH_SIZE, 1,	//	[9]	depth

	EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
	EGL_NONE
};

	lua_getfield(l,1,"r");		if( lua_isnumber(l,-1) ) { attribute_list[1]=(int)lua_tonumber(l,-1);	} lua_pop(l,1);
	lua_getfield(l,1,"g");		if( lua_isnumber(l,-1) ) { attribute_list[3]=(int)lua_tonumber(l,-1);	} lua_pop(l,1);
	lua_getfield(l,1,"b");		if( lua_isnumber(l,-1) ) { attribute_list[5]=(int)lua_tonumber(l,-1);	} lua_pop(l,1);
	lua_getfield(l,1,"a");		if( lua_isnumber(l,-1) ) { attribute_list[7]=(int)lua_tonumber(l,-1);	} lua_pop(l,1);
	lua_getfield(l,1,"depth");	if( lua_isnumber(l,-1) ) { attribute_list[9]=(int)lua_tonumber(l,-1);	} lua_pop(l,1);

	wp = (android_lua_wrap *)lua_newuserdata(l, sizeof(android_lua_wrap)); // we need a pointer, this makes lua GC a bit easier
	memset(wp,0,sizeof(android_lua_wrap)); // make sure it is 0
	wp->p=wp->a; // point the pointer to the struct
	p=wp->p; // take an easy to use copy of the pointer
	luaL_getmetatable(l, lua_android_ptr_name);
	lua_setmetatable(l, -2);
	
	p->window=master_android_app->window;

	// get an EGL display connection
	p->display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	assert(p->display!=EGL_NO_DISPLAY);

	// initialize the EGL display connection
	result = eglInitialize(p->display, NULL, NULL);
	assert(EGL_FALSE != result);
 
  // get an appropriate EGL frame buffer configuration
   result = eglChooseConfig(p->display, attribute_list, &config, 1, &num_config);
   assert(EGL_FALSE != result);

//    eglGetConfigAttrib(p->display, config, EGL_NATIVE_VISUAL_ID, &format);
//    ANativeWindow_setBuffersGeometry(p->window, 0, 0, format);
    

// create an EGL rendering context
   p->context = eglCreateContext(p->display, config, EGL_NO_CONTEXT, NULL);
   assert(p->context!=EGL_NO_CONTEXT);

// create surface
	p->surface = eglCreateWindowSurface( p->display, config, p->window, NULL );
	assert(p->surface != EGL_NO_SURFACE);

	return 1;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// destroy data if it exists
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_android_destroy (lua_State *l)
{
android_lua **pp=lua_android_ptr_ptr(l,1);
android_lua *p;

	if(*pp)
	{
		p=*pp;
		
		if(p->display)
		{
			// Release OpenGL resources
			eglMakeCurrent( p->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT );
			if(p->surface)
			{
				eglDestroySurface( p->display, p->surface );
				p->surface=0;
			}
			if(p->context)
			{
				eglDestroyContext( p->display, p->context );
				p->context=0;
			}
			eglTerminate( p->display );
			p->display=0;
		}
	}
	(*pp)=0;

	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// prepare a gl surface in the window
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_android_context (lua_State *l)
{
android_lua *p=lua_android_check_ptr(l,1);

EGLBoolean result;
	
// connect the context to the surface
   result = eglMakeCurrent(p->display, p->surface, p->surface, p->context);
   assert(EGL_FALSE != result);


	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// swap a gl surface 
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_android_swap (lua_State *l)
{
android_lua *p=lua_android_check_ptr(l,1);

	eglSwapBuffers(p->display, p->surface);
   
	return 0;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// wait a while to see if any msgs turn up
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_android_sleep (lua_State *l)
{
    double n = luaL_checknumber(l, 1);

    struct timespec t, r;
    t.tv_sec = (int) n;
    n -= t.tv_sec;
    t.tv_nsec = (int) (n * 1000000000);
    if (t.tv_nsec >= 1000000000) t.tv_nsec = 999999999;
    while (nanosleep(&t, &r) != 0) {
        t.tv_sec = r.tv_sec;
        t.tv_nsec = r.tv_nsec;
    }
    return 0;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// open library.
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
LUALIB_API int luaopen_wetgenes_win_android_core(lua_State *l)
{
	const luaL_reg lib[] =
	{
//		{"screen",			lua_android_screen},
		
		{"create",			lua_android_create},
		{"destroy",			lua_android_destroy},
		{"info",			lua_android_info},

		{"context",			lua_android_context},
		{"swap",			lua_android_swap},

//		{"peek",			lua_android_peek},
//		{"wait",			lua_android_wait},
//		{"msg",				lua_android_msg},

		{"sleep",			lua_android_sleep},

		{"print",			lua_android_print},
		{"time",			lua_android_time},
		
		{0,0}
	};
	

 	const luaL_reg meta[] =
	{
		{"__gc",			lua_android_destroy},
		{0,0}
	};
	
	luaL_newmetatable(l, lua_android_ptr_name);
	luaL_openlib(l, NULL, meta, 0);
	lua_pop(l,1);

	lua_newtable(l);
	luaL_openlib(l, NULL, lib, 0);
	
	return 1;
}



