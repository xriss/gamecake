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
	RECT srect[1];

	SystemParametersInfo( SPI_GETWORKAREA , 0 , srect , 0);

	lua_pushnumber(l,srect->right-srect->left);
	lua_pushnumber(l,srect->bottom-srect->top);
	
	return 2;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// Main window msg handler
//
/*+-----------------------------------------------------------------------------------------------------------------+*/

LRESULT WINAPI msg_handler( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
wetwin_lua *p=(wetwin_lua *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
lua_State *l;

//bool active_window=true;


char char_buff256[256];
char char_buff16[16];

BYTE keystate[256];

int scancode;
char *act="";
char *key="";

char lua=' ';

	char_buff256[0]=0;
	char_buff16[0]=0;
	
/*
HWND front=GetForegroundWindow();
HWND parent=GetAncestor(hWnd,GA_ROOT);

	if( (front==parent) && IsWindowVisible(hWnd) )
	{
		active_window=true;
	}
	else
	{
		active_window=false;
	}
*/
	
	switch( msg )
    {
		case WM_CREATE: // remember who we are
		
			p=(wetwin_lua *) ((CREATESTRUCT *)lParam)->lpCreateParams;			
			SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG) p);
			
		break;
		
/*
        case WM_TIMER:

RECT srect[1];
			GetClientRect(hWnd,srect);
//			fenestra->ogl->setup_viewport(srect->right,srect->bottom);
			fenestra->width=srect->right;
			fenestra->height=srect->bottom;

			if(fenestra->call_update)
			{
				l=fenestra->l;
				lua_pushlightuserdata(l,fenestra);
				lua_gettable(l, LUA_REGISTRYINDEX);
				lua_getfield(l,-1,"update");
				if( lua_isfunction(l,-1) )
				{
					lua_call(l,0,0);
				}
				else
				{
					lua_pop(l,1);
				}
				lua_pop(l,1);
			}

			SetTimer(hWnd,0,1,0); // hit the msg loop repeatedly

		break;
*/

		case WM_DESTROY:
			PostQuitMessage( 0 );
			return 0;
		break;

        case WM_PAINT:
			ValidateRect( hWnd, NULL );
			return(0);
		break;

		case WM_LBUTTONDBLCLK:
//		case WM_NCLBUTTONDBLCLK:
			lua='m';
			act="double";
			key="left";
		break;
		case WM_LBUTTONDOWN:
		SetFocus(hWnd);
//		case WM_NCLBUTTONDOWN:
			lua='m';
			act="down";
			key="left";
			SetCapture(hWnd);
		break;
		case WM_LBUTTONUP:
//		case WM_NCLBUTTONUP:
			lua='m';
			act="up";
			key="left";
			ReleaseCapture();
		break;
		
		case WM_RBUTTONDBLCLK:
//		case WM_NCRBUTTONDBLCLK:
			lua='m';
			act="double";
			key="right";
		break;
		case WM_RBUTTONDOWN:
//		case WM_NCRBUTTONDOWN:
			lua='m';
			act="down";
			key="right";
			SetCapture(hWnd);
		break;
		case WM_RBUTTONUP:
//		case WM_NCRBUTTONUP:
			lua='m';
			act="up";
			key="right";
			ReleaseCapture();
		break;
		
		case WM_MBUTTONDBLCLK:
//		case WM_NCMBUTTONDBLCLK:
			lua='m';
			act="double";
			key="middle";
		break;
		case WM_MBUTTONDOWN:
//		case WM_NCMBUTTONDOWN:
			lua='m';
			act="down";
			key="middle";
			SetCapture(hWnd);
		break;
		case WM_MBUTTONUP:
//		case WM_NCMBUTTONUP:
			lua='m';
			act="up";
			key="middle";
			ReleaseCapture();
		break;
		
		case WM_XBUTTONDBLCLK:
//		case WM_NCXBUTTONDBLCLK:
			lua='m';
			act="double";
			key="x";
			if(HIWORD (wParam)==1 ) { key="x1"; }
			if(HIWORD (wParam)==2 ) { key="x2"; }
		break;
		case WM_XBUTTONDOWN:
//		case WM_NCXBUTTONDOWN:
			lua='m';
			act="down";
			key="x";
			if(HIWORD(wParam)==1 ) { key="x1"; }
			if(HIWORD (wParam)==2 ) { key="x2"; }
			SetCapture(hWnd);
		break;
		case WM_XBUTTONUP:
//		case WM_NCXBUTTONUP:
			lua='m';
			act="up";
			key="x";
			if(HIWORD (wParam)==1 ) { key="x1"; }
			if(HIWORD (wParam)==2 ) { key="x2"; }
			ReleaseCapture();
		break;
		
		case WM_MOUSEMOVE:
//		case WM_NCMOUSEMOVE:
			lua='m';
			act="move";
			key="none";
		break;
		
		case WM_KEYDOWN:
			lua='k';
			if(lParam&0x40000000)
			{
				act="repeat";
			}
			else
			{
				act="down";
			}
		break;
		
		case WM_KEYUP:
		
			lua='k';
			act="up";
			
		break;
		
		case WM_SIZE:
		

	
		break;
		
		case WM_EXITSIZEMOVE:
		break;

    }
    

/*
    if(lua=='m')
    {
		l=fenestra->l;
		lua_pushlightuserdata(l,fenestra);
		lua_gettable(l, LUA_REGISTRYINDEX);
		lua_getfield(l,-1,"mouse");
		if( lua_isfunction(l,-1) )
		{
			lua_pushstring(l,act);
			lua_pushnumber(l,GET_X_LPARAM(lParam));
			lua_pushnumber(l,GET_Y_LPARAM(lParam));
			lua_pushstring(l,key);
			lua_call(l,4,0);
		}
		else
		{
			lua_pop(l,1);
		}
		lua_pop(l,1);
    }
    else
    if(lua=='k')
    {
		scancode=(lParam>>16)&0xff;
		
		GetKeyboardState(keystate);
		
		ToAscii(wParam,scancode,keystate,(LPWORD)char_buff16,0);
		char_buff16[1]=0;

		GetKeyNameText(lParam,char_buff256,256);
	
		l=fenestra->l;
		lua_pushlightuserdata(l,fenestra);
		lua_gettable(l, LUA_REGISTRYINDEX);
		lua_getfield(l,-1,"keypress");
		if( lua_isfunction(l,-1) )
		{
			lua_pushstring(l,char_buff16);
			lua_pushstring(l,char_buff256);
			lua_pushstring(l,act);
			lua_call(l,3,0);
		}
		else
		{
			lua_pop(l,1);
		}
		lua_pop(l,1);
    }
*/
    return DefWindowProc( hWnd, msg, wParam, lParam );
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

	wp = (wetwin_lua_wrap *)lua_newuserdata(l, sizeof(wetwin_lua_wrap)); // we need a pointer, this makes lua GC a bit easier
	memset(wp,0,sizeof(wetwin_lua_wrap)); // make sure it is 0
	wp->p=wp->a; // point the pointer to the struct
	p=wp->p; // take an easy to use copy of the pointer
	luaL_getmetatable(l, lua_wetwin_ptr_name);
	lua_setmetatable(l, -2);

	p->width=640;
	p->height=480;

// get our filename

	GetModuleFileName(0, p->ModuleFileName, sizeof(p->ModuleFileName));

// get a handle to our console window if we have one

	p->console=FindWindow("ConsoleWindowClass", p->ModuleFileName);

// get our process handle

	if(!DuplicateHandle(GetCurrentProcess(),GetCurrentProcess(),GetCurrentProcess(),&p->proc,0,1,DUPLICATE_SAME_ACCESS))
	{
		goto bogus;
	}

	p->instance=GetModuleHandle(NULL);

    ZeroMemory( p->wc , sizeof( p->wc ) );

	p->wc->cbSize=sizeof(WNDCLASSEX);
	p->wc->style=CS_CLASSDC;
	p->wc->lpfnWndProc=msg_handler;
	p->wc->hInstance=p->instance;
	p->wc->lpszClassName="GameCake";
	p->wc->hIcon=LoadIcon(NULL, IDI_APPLICATION);
	p->wc->hCursor=LoadCursor(NULL,IDC_ARROW); 
	p->wc->hbrBackground=(HBRUSH)GetStockObject(BLACK_BRUSH);


    RegisterClassEx( p->wc );

	RECT rect[1];
	rect->left=80;
	rect->top=80;
	rect->right=rect->left+p->width;
	rect->bottom=rect->top+p->height;

	AdjustWindowRect(rect,WS_OVERLAPPEDWINDOW,0);

	p->hwnd = CreateWindow( "GameCake", "http://gamecake.4lfa.com/ ",
						  WS_OVERLAPPEDWINDOW, rect->left, rect->top, rect->right-rect->left, rect->bottom-rect->top,
						  GetDesktopWindow(), NULL, p->instance, p );

	ShowWindow( p->hwnd, SW_SHOWDEFAULT );
	
    UpdateWindow( p->hwnd );
    
	return 1;
	
bogus:
	return 0;
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
		if((*pp)->instance)
		{
			UnregisterClass( "GameCake", (*pp)->instance );
			(*pp)->instance=0;
		}
		if((*pp)->proc)
		{
			CloseHandle((*pp)->proc);
			(*pp)->proc=0;
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

    PIXELFORMATDESCRIPTOR pfd;
    int iFormat;

    // get the device context (DC)
    p->hDC = GetDC( p->hwnd );

    // set the pixel format for the DC
    ZeroMemory( &pfd, sizeof( pfd ) );
    pfd.nSize = sizeof( pfd );
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 24; // 16 is too small?
    pfd.iLayerType = PFD_MAIN_PLANE;
    iFormat = ChoosePixelFormat( p->hDC, &pfd );

    SetPixelFormat( p->hDC, iFormat, &pfd );

    // create and enable the render context (RC)
    p->hRC = wglCreateContext( p->hDC );

    wglMakeCurrent( p->hDC, p->hRC );
    
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

	SwapBuffers( p->hDC );

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

MSG msg;
	if( PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) )
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

MSG msg;

	MsgWaitForMultipleObjects(1, &p->proc,FALSE, 1, QS_ALLINPUT);

	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// grab the next msg, and return info about it
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_wetwin_msg (lua_State *l)
{
wetwin_lua *p=lua_wetwin_check_ptr(l,1);

int ret;
MSG msg;

	if( !GetMessage( &msg, NULL, 0, 0 ) ) { ret=0; } // Simon says shutdown
	TranslateMessage(&msg); 
	DispatchMessage(&msg);

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
    
	Sleep((int)(n*1000.0));
	
    return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// what time is it, with sub second resolution
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_wetwin_time (lua_State *l)
{
s64 rez;
s64 tim;

	QueryPerformanceFrequency( (LARGE_INTEGER *) (&rez) );
	QueryPerformanceCounter( (LARGE_INTEGER *) (&(tim)) );
	lua_pushnumber(l, ((double)tim)/((double)rez) );
	return 1;
	
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// open library.
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
LUALIB_API int luaopen_wetgenes_win_windows(lua_State *l)
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

