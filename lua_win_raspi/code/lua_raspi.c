

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <signal.h>
#include <string.h>

#include <unistd.h>
#include <sys/time.h>

#include "lua.h"
#include "lauxlib.h"

#include "bcm_host.h"
#include "EGL/egl.h"

#include "lua_raspi.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


//
// we can use either this string as a string identifier
// or its address as a light userdata identifier, both will be unique
//
const char *lua_raspi_ptr_name="raspi*ptr";

	
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// check that a userdata at the given index is a raspi object
// return the userdata if it does, otherwise return 0
// this userdata will be a pointer to the real data
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
raspi_lua **lua_raspi_ptr_ptr (lua_State *l, int idx)
{
raspi_lua **pp=0;

	pp = ((raspi_lua **)luaL_checkudata(l, idx , lua_raspi_ptr_name));

	return pp;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// *lua_raspi_check with auto error on 0 ptr
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
raspi_lua *lua_raspi_check_ptr (lua_State *l, int idx)
{
raspi_lua **pp=lua_raspi_ptr_ptr(l,idx);

	if (*pp == 0)
	{
		luaL_error(l, "bad raspi userdata" );
	}

	return *pp;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// fill a table in with the current settings
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_raspi_getinfo (lua_State *l, raspi_lua *p, int tab)
{
	if(p)
	{
		lua_pushnumber(l,p->width);		lua_setfield(l,tab,"width");
		lua_pushnumber(l,p->height);	lua_setfield(l,tab,"height");
	}
	else
	{
		lua_pushstring(l,"unbound raspi"); lua_setfield(l,tab,"err");
	}

	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// set info into the given table
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_raspi_info (lua_State *l)
{
raspi_lua *p=lua_raspi_check_ptr(l,1);
	lua_raspi_getinfo(l,p,2);
	return 0;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// get info about the screen size
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_raspi_screen (lua_State *l)
{
EGLBoolean result;
int32_t success = 0;
EGLDisplay display;

int screen_width;
int screen_height;

	// get an EGL display connection
	display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	assert(display!=EGL_NO_DISPLAY);

	// initialize the EGL display connection
	result = eglInitialize(display, NULL, NULL);
	assert(EGL_FALSE != result);

	// create an EGL window surface
	success = graphics_get_display_size(0 /* LCD */, &screen_width, &screen_height);
	assert( success >= 0 );

	eglTerminate( display );

	lua_pushnumber(l,screen_width);
	lua_pushnumber(l,screen_height);
	
	return 2;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// create and return the data
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_raspi_create (lua_State *l)
{
raspi_lua_wrap *wp;
raspi_lua *p;

int x=0;
int y=0;

const char *title="http://www.WetGenes.com/ - GameCake";

	DISPMANX_ELEMENT_HANDLE_T dispman_element;
	DISPMANX_DISPLAY_HANDLE_T dispman_display;
	DISPMANX_UPDATE_HANDLE_T dispman_update;

EGLBoolean result;
int32_t success = 0;
VC_RECT_T dst_rect;
VC_RECT_T src_rect;

EGLConfig config;
EGLint num_config;
EGLint attribute_list[] =
{
	
// hard hax, please not to be reordering
	EGL_RED_SIZE, 4,	//	[1]	r
	EGL_GREEN_SIZE, 4,	//	[3]	g
	EGL_BLUE_SIZE, 4,	//	[5]	b
	EGL_ALPHA_SIZE, 0,	//	[7]	a
	EGL_DEPTH_SIZE, 1,	//	[9]	depth

	EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
	EGL_NONE
};
EGLint context_attributes[] =
{
	EGL_CONTEXT_CLIENT_VERSION, 2,
	EGL_NONE
};
 
	lua_getfield(l,1,"r");		if( lua_isnumber(l,-1) ) { attribute_list[1]=(int)lua_tonumber(l,-1);	} lua_pop(l,1);
	lua_getfield(l,1,"g");		if( lua_isnumber(l,-1) ) { attribute_list[3]=(int)lua_tonumber(l,-1);	} lua_pop(l,1);
	lua_getfield(l,1,"b");		if( lua_isnumber(l,-1) ) { attribute_list[5]=(int)lua_tonumber(l,-1);	} lua_pop(l,1);
	lua_getfield(l,1,"a");		if( lua_isnumber(l,-1) ) { attribute_list[7]=(int)lua_tonumber(l,-1);	} lua_pop(l,1);
	lua_getfield(l,1,"depth");	if( lua_isnumber(l,-1) ) { attribute_list[9]=(int)lua_tonumber(l,-1);	} lua_pop(l,1);

	lua_getfield(l,1,"x");		if( lua_isnumber(l,-1) ) { x=lua_tonumber(l,-1); 		} lua_pop(l,1);
	lua_getfield(l,1,"y");		if( lua_isnumber(l,-1) ) { y=lua_tonumber(l,-1);		} lua_pop(l,1);

	wp = (raspi_lua_wrap *)lua_newuserdata(l, sizeof(raspi_lua_wrap)); // we need a pointer, this makes lua GC a bit easier
	memset(wp,0,sizeof(raspi_lua_wrap)); // make sure it is 0
	wp->p=wp->a; // point the pointer to the struct
	p=wp->p; // take an easy to use copy of the pointer
	luaL_getmetatable(l, lua_raspi_ptr_name);
	lua_setmetatable(l, -2);
	

	// get an EGL display connection
	p->display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	assert(p->display!=EGL_NO_DISPLAY);

	// initialize the EGL display connection
	result = eglInitialize(p->display, NULL, NULL);
	assert(EGL_FALSE != result);

	// create an EGL window surface
	success = graphics_get_display_size(0 /* LCD */, &p->screen_width, &p->screen_height);
	assert( success >= 0 );

	p->width=p->screen_width;	 // full width/height by default
	p->height=p->screen_height;

	lua_getfield(l,1,"width");	if( lua_isnumber(l,-1) ) { p->width=lua_tonumber(l,-1);	} lua_pop(l,1); // overide width/height
	lua_getfield(l,1,"height");	if( lua_isnumber(l,-1) ) { p->height=lua_tonumber(l,-1);} lua_pop(l,1); // if given

	dst_rect.x = x;
	dst_rect.y = y;
	dst_rect.width = p->width;
	dst_rect.height = p->height;
	  
	src_rect.x = 0;
	src_rect.y = 0;
	src_rect.width = p->width << 16;
	src_rect.height = p->height << 16;        

	dispman_display = vc_dispmanx_display_open( 0 /* LCD */);
	dispman_update = vc_dispmanx_update_start( 0 );
		 
	dispman_element = vc_dispmanx_element_add ( dispman_update, dispman_display,
	  0/*layer*/, &dst_rect, 0/*src*/,
	  &src_rect, DISPMANX_PROTECTION_NONE, 0 /*alpha*/, 0/*clamp*/, 0/*transform*/);
	  
	p->nativewindow.element = dispman_element;
	p->nativewindow.width = p->width;
	p->nativewindow.height = p->height;
	vc_dispmanx_update_submit_sync( dispman_update );

   // get an appropriate EGL frame buffer configuration
   result = eglChooseConfig(p->display, attribute_list, &config, 1, &num_config);
   assert(EGL_FALSE != result);

// create an EGL rendering context
   p->context = eglCreateContext(p->display, config, EGL_NO_CONTEXT, context_attributes);
   assert(p->context!=EGL_NO_CONTEXT);

// create surface
	p->surface = eglCreateWindowSurface( p->display, config, &p->nativewindow, NULL );
	assert(p->surface != EGL_NO_SURFACE);

	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// destroy data if it exists
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_raspi_destroy (lua_State *l)
{
raspi_lua **pp=lua_raspi_ptr_ptr(l,1);
raspi_lua *p;

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
int lua_raspi_context (lua_State *l)
{
raspi_lua *p=lua_raspi_check_ptr(l,1);

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
int lua_raspi_swap (lua_State *l)
{
raspi_lua *p=lua_raspi_check_ptr(l,1);

   eglSwapBuffers(p->display, p->surface);
   
//	glXSwapBuffers( p->dsp, p->win );

	return 0;
}



/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// time
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_raspi_time (lua_State *l)
{
	struct timeval tv ;
	gettimeofday ( & tv, NULL ) ;
	lua_pushnumber(l, (double) tv.tv_sec + (double) tv.tv_usec / 1000000.0 );
	return 1;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// non blocking getc, returns nil if there is nothing there, otherwise a char code
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_raspi_getc (lua_State *l)
{
char c;
    struct timeval tv;
    fd_set fds;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds); //STDIN_FILENO is 0
    select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
    if( FD_ISSET(STDIN_FILENO, &fds) )
    {
		c=fgetc(stdin);
		lua_pushnumber(l,c);
		return 1;
	}
	
	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// what time is it, with sub second resolution
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_raspi_jread (lua_State *l)
{
raspi_lua *p=lua_raspi_check_ptr(l,1);
int n = (int)luaL_checknumber(l, 2);

unsigned char b[16];
char s[256];

	strcpy(s,"/dev/input/js0");

	if(n<0) { n=0; }
	if(n>3) { n=3; }
	if(! p->joy_fd[n])
	{
		s[13]='0'+n;
		p->joy_fd[n] = open(s,O_RDONLY|O_NONBLOCK);
	}

	if(p->joy_fd[n])
	{
		if( read(p->joy_fd[n], b, 8) == 8 )
		{
			lua_pushlstring(l,b,8);
			return 1;
		}
	}
	return 0;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// open library.
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
LUALIB_API int luaopen_wetgenes_win_raspi(lua_State *l)
{
	const luaL_reg lib[] =
	{
		{"screen",			lua_raspi_screen},
		
		{"create",			lua_raspi_create},
		{"destroy",			lua_raspi_destroy},
		{"info",			lua_raspi_info},

		{"context",			lua_raspi_context},
		{"swap",			lua_raspi_swap},

//		{"peek",			lua_raspi_peek},
//		{"wait",			lua_raspi_wait},
//		{"msg",				lua_raspi_msg},

//		{"sleep",			lua_raspi_sleep},
		{"time",			lua_raspi_time},
		
		{"getc",			lua_raspi_getc},
		
		{"jread",			lua_raspi_jread},

		{0,0}
	};

 	const luaL_reg meta[] =
	{
		{"__gc",			lua_raspi_destroy},
		{0,0}
	};
	
	luaL_newmetatable(l, lua_raspi_ptr_name);
	luaL_openlib(l, NULL, meta, 0);
	lua_pop(l,1);

	lua_newtable(l);
	luaL_openlib(l, NULL, lib, 0);
	
	bcm_host_init(); // always need this bit?

	return 1;
}

