#include "all.h"


//
// we can use either this string as a string identifier
// or its address as a light userdata identifier, both will be unique
//
const char *lua_wetwin_ptr_name="wetwin*ptr";

	
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// check that a userdata at the given index is a wetwin object
// return the userdata if it does, otherwise return 0
// this userdata will be a pointer to the real data
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
wetwin_lua **lua_wetwin_ptr_ptr (lua_State *l, int idx)
{
wetwin_lua **pp=0;

	pp = ((wetwin_lua **)luaL_checkudata(l, idx , lua_wetwin_ptr_name));

	return pp;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// *lua_wetwin_check with auto error on 0 ptr
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
wetwin_lua *lua_wetwin_check_ptr (lua_State *l, int idx)
{
wetwin_lua **pp=lua_wetwin_ptr_ptr(l,idx);

	if (*pp == 0)
	{
		luaL_error(l, "bad wetwin userdata" );
	}

	return *pp;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// fill a table in with the current settings
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_wetwin_getinfo (lua_State *l, wetwin_lua *p, int tab)
{
	if(p)
	{
		lua_pushnumber(l,p->width);		lua_setfield(l,tab,"width");
		lua_pushnumber(l,p->height);	lua_setfield(l,tab,"height");
	}
	else
	{
		lua_pushstring(l,"unbound wetwin"); lua_setfield(l,tab,"err");
	}

	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// set info into the given table
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_wetwin_info (lua_State *l)
{
wetwin_lua *p=lua_wetwin_check_ptr(l,1);
	lua_wetwin_getinfo(l,p,2);
	return 0;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// get info about the screen size
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_wetwin_screen (lua_State *l)
{
	Display		*dsp = XOpenDisplay( NULL );


	lua_pushnumber(l,DisplayWidth(dsp,0));
	lua_pushnumber(l,DisplayHeight(dsp,0));
	
	
	XCloseDisplay( dsp );
	
	return 2;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// create and return the data
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_wetwin_create (lua_State *l)
{
wetwin_lua_wrap *wp;
wetwin_lua *p;

int x=20;
int y=20;

int width=640;
int height=480;

const char *title="http://www.WetGenes.com/ - fenestra";

	lua_getfield(l,1,"width");	if( lua_isnumber(l,-1) ) { width=lua_tonumber(l,-1);	} lua_pop(l,1);
	lua_getfield(l,1,"height");	if( lua_isnumber(l,-1) ) { height=lua_tonumber(l,-1);	} lua_pop(l,1);
	lua_getfield(l,1,"x");		if( lua_isnumber(l,-1) ) { x=lua_tonumber(l,-1); 		} lua_pop(l,1);
	lua_getfield(l,1,"y");		if( lua_isnumber(l,-1) ) { y=lua_tonumber(l,-1);		} lua_pop(l,1);

	wp = (wetwin_lua_wrap *)lua_newuserdata(l, sizeof(wetwin_lua_wrap)); // we need a pointer, this makes lua GC a bit easier
	memset(wp,0,sizeof(wetwin_lua_wrap)); // make sure it is 0
	wp->p=wp->a; // point the pointer to the struct
	p=wp->p; // take an easy to use copy of the pointer
	luaL_getmetatable(l, lua_wetwin_ptr_name);
	lua_setmetatable(l, -2);
	
	p->dsp = XOpenDisplay( NULL );
	if(p->dsp)
	{
		p->fp_dsp=ConnectionNumber(p->dsp);
		
		FD_ZERO(&p->set_dsp);
        FD_SET(p->fp_dsp, &p->set_dsp);
        
		p->screen = DefaultScreen(p->dsp);
		unsigned long white = WhitePixel(p->dsp,p->screen);
		unsigned long black = BlackPixel(p->dsp,p->screen);	
		p->win = XCreateSimpleWindow(p->dsp,
										DefaultRootWindow(p->dsp),
										x, y,   					// origin
										width, height, 				// size
										0, white, 					// border
										black );  					// backcolour
		if(p->win)
		{
			XMapWindow( p->dsp, p->win );

			XStoreName( p->dsp, p->win, title );

			XSelectInput( p->dsp , p->win ,
				KeyPressMask | KeyReleaseMask |
				ButtonPressMask | ButtonReleaseMask |
				PointerMotionMask | StructureNotifyMask );

			XMoveWindow( p->dsp , p->win , x,y);

			XFlush(p->dsp);
		}
	}

	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// destroy data if it exists
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_wetwin_destroy (lua_State *l)
{
wetwin_lua **pp=lua_wetwin_ptr_ptr(l,1);

	if(*pp)
	{
		if((*pp)->dsp)
		{
			if((*pp)->win)
			{
				if((*pp)->context)
				{
					glXMakeCurrent((*pp)->dsp,None,NULL);
					glXDestroyContext((*pp)->dsp,(*pp)->context);
				}
				XDestroyWindow( (*pp)->dsp, (*pp)->win );
			}
			XCloseDisplay( (*pp)->dsp );
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
int lua_wetwin_context (lua_State *l)
{
wetwin_lua *p=lua_wetwin_check_ptr(l,1);

	int attrcount;
	int AttributeList[] = {
			GLX_RED_SIZE, 1,
			GLX_GREEN_SIZE, 1,
			GLX_BLUE_SIZE, 1,
			GLX_ALPHA_SIZE, 0,
			GLX_DEPTH_SIZE, 1,
			GLX_STENCIL_SIZE, 0,
			GLX_X_RENDERABLE,1,
			GLX_DOUBLEBUFFER,1,
			None};
			
	GLXFBConfig *conf=glXChooseFBConfig(p->dsp,p->screen,AttributeList,&attrcount);

	p->context=glXCreateNewContext( p->dsp , *conf , GLX_RGBA_TYPE , NULL , 1 );

	glXMakeContextCurrent( p->dsp , p->win , p->win, p->context );

	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// swap a gl surface 
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_wetwin_swap (lua_State *l)
{
wetwin_lua *p=lua_wetwin_check_ptr(l,1);

	glXSwapBuffers( p->dsp, p->win );

	return 0;
}



/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// are there msgs waiting to be delt with?
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_wetwin_peek (lua_State *l)
{
wetwin_lua *p=lua_wetwin_check_ptr(l,1);

	if( XPending( p->dsp ) > 0 )
	{
		lua_pushboolean(l,1);
	}
	else
	{
		lua_pushboolean(l,0);
	}
	return 1;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// wait a while to see if any msgs turn up
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_wetwin_wait (lua_State *l)
{
wetwin_lua *p=lua_wetwin_check_ptr(l,1);

	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 1000; // 1ms
	
	select(p->fp_dsp, &p->set_dsp, 0, 0, &tv);
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// grab the next msg, and return info about it
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_wetwin_msg (lua_State *l)
{
wetwin_lua *p=lua_wetwin_check_ptr(l,1);

char lua=' ';
int act=0;
int key=0;
int x=0;
int y=0;
char asc[255];
KeySym k;

	XEvent e[1];
	if( XPending( p->dsp ) > 0 )
	{
		XNextEvent(p->dsp, e);
		
		switch(e->type)
		{
			case KeyPress:
				lua='k';
				act=1;
			break;
			case KeyRelease:
				lua='k';
				act=-1;
			break;
			
			case ButtonPress:
				lua='m';
				act=1;
				key=(e->xbutton.button);
				x=e->xbutton.x;
				y=e->xbutton.y;
			break;
			case ButtonRelease:
				lua='m';
				act=-1;
				key=(e->xbutton.button);
				x=e->xbutton.x;
				y=e->xbutton.y;
			break;
			case MotionNotify:
				lua='m';
				x=e->xmotion.x;
				y=e->xmotion.y;
			break;
			
			case ConfigureNotify:
				lua='s';
				x=e->xconfigure.width;
				y=e->xconfigure.height;
				p->width=x;
				p->height=y;
			break;
		}
	}
	
    if(lua=='m')
    {
		lua_pushstring(l,"mouse");
		lua_pushnumber(l,key);
		lua_pushnumber(l,act);
		lua_pushnumber(l,x);
		lua_pushnumber(l,y);
		return 5;
	}
    else
    if(lua=='k')
    {
		int ts=XLookupString(&e->xkey,asc,32,&k,0);
		asc[ts]=0; // null term the ascii

		lua_pushstring(l,"key");
		lua_pushstring(l,asc);
		lua_pushnumber(l,act);
		lua_pushnumber(l,k);
		lua_pushstring(l,XKeysymToString(k));
		return 5;
	}
    else
    if(lua=='s')
	{	
		lua_pushstring(l,"size");
		lua_pushnumber(l,x);
		lua_pushnumber(l,y);
		return 3;
	}
	
	return 0; // no more msgs
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// wait a while to see if any msgs turn up
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_wetwin_sleep (lua_State *l)
{
    double n = luaL_checknumber(l, 1);
//#ifdef _WIN32
//    Sleep((int)(n*1000));
//#else
    struct timespec t, r;
    t.tv_sec = (int) n;
    n -= t.tv_sec;
    t.tv_nsec = (int) (n * 1000000000);
    if (t.tv_nsec >= 1000000000) t.tv_nsec = 999999999;
    while (nanosleep(&t, &r) != 0) {
        t.tv_sec = r.tv_sec;
        t.tv_nsec = r.tv_nsec;
    }
//#endif
    return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// what time is it, with sub second resolution
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_wetwin_time (lua_State *l)
{

#if defined(WIN32)

s64 rez;
s64 tim;

	QueryPerformanceFrequency( (LARGE_INTEGER *) (&rez) );
	QueryPerformanceCounter( (LARGE_INTEGER *) (&(tim)) );
	lua_pushnumber(l, ((double)tim)/((double)rez) );
	return 1;
	
#else

	struct timeval tv;
	gettimeofday ( &tv, NULL );
	lua_pushnumber(l, ((double)tv.tv_sec) + ( ((double)tv.tv_usec) / 1000000.0 ) );
	return 1;
	
#endif
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// open library.
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
LUALIB_API int luaopen_wetgenes_win_linux(lua_State *l)
{
	const luaL_reg lib[] =
	{
		{"screen",			lua_wetwin_screen},
		
		{"create",			lua_wetwin_create},
		{"destroy",			lua_wetwin_destroy},
		{"info",			lua_wetwin_info},

		{"context",			lua_wetwin_context},
		{"swap",			lua_wetwin_swap},

		{"peek",			lua_wetwin_peek},
		{"wait",			lua_wetwin_wait},
		{"msg",				lua_wetwin_msg},

		{"sleep",			lua_wetwin_sleep},
		{"time",			lua_wetwin_time},
		
		{0,0}
	};

 	const luaL_reg meta[] =
	{
		{"__gc",			lua_wetwin_destroy},
		{0,0}
	};
	
	luaL_newmetatable(l, lua_wetwin_ptr_name);
	luaL_openlib(l, NULL, meta, 0);
	lua_pop(l,1);

	lua_newtable(l);
	luaL_openlib(l, NULL, lib, 0);
	
	return 1;
}

