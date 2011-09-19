/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss Daniels 2003 http://www.XIXs.com
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
#include "all.h"





/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// Pre Setup, clear the structure afterwhich you may add some pre initialised args before calling setup
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
 void fenestra::prepare()
{
    memset( this , 0, sizeof(*this) ); //doh, must get size right

	argv=(char**)&("");
	argc=0;
	
}





/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// Main window msg handler
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
#if defined(WIN32)

LRESULT WINAPI msg_handler( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{

struct fenestra *fenestra=(struct fenestra *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
lua_State *l;

bool active_window=true;

HWND front=GetForegroundWindow();
HWND parent=GetAncestor(hWnd,GA_ROOT);

char char_buff256[256];
char char_buff16[16];

BYTE keystate[256];

int scancode;
char *act="";
char *key="";

char lua=' ';

	char_buff256[0]=0;
	char_buff16[0]=0;
	
	if( (front==parent) && IsWindowVisible(hWnd) )
	{
		active_window=true;
	}
	else
	{
		active_window=false;
	}
	
	switch( msg )
    {
		case WM_CREATE: // remember who we are
		
			fenestra=(struct fenestra*) ((CREATESTRUCT *)lParam)->lpCreateParams;			
			SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG) fenestra);
			
		break;
		
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
			if(GET_XBUTTON_WPARAM (wParam)==1 ) { key="x1"; }
			if(GET_XBUTTON_WPARAM (wParam)==2 ) { key="x2"; }
		break;
		case WM_XBUTTONDOWN:
//		case WM_NCXBUTTONDOWN:
			lua='m';
			act="down";
			key="x";
			if(GET_XBUTTON_WPARAM (wParam)==1 ) { key="x1"; }
			if(GET_XBUTTON_WPARAM (wParam)==2 ) { key="x2"; }
			SetCapture(hWnd);
		break;
		case WM_XBUTTONUP:
//		case WM_NCXBUTTONUP:
			lua='m';
			act="up";
			key="x";
			if(GET_XBUTTON_WPARAM (wParam)==1 ) { key="x1"; }
			if(GET_XBUTTON_WPARAM (wParam)==2 ) { key="x2"; }
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

    return DefWindowProc( hWnd, msg, wParam, lParam );
}

#elif defined(X11)

static char * strlwr ( char *string )
{
        for(char *cp=string; *cp; ++cp)
        {
                if ('A' <= *cp && *cp <= 'Z') *cp += 'a' - 'A';
        }
        return(string);
}

const char * mbutname(int i)
{
static const char* mk[]={"0","left","right","middle","x1","x2"};
	if( (i<=5) && (i>=0) )
	{
		return mk[i];
	}
	return "";
}

bool fenestra::event_handler( XEvent *e )
{
	char lua=' ';
	const char *act="";
	char asc[255];
	char keyb[255];
	const char *key=keyb;
	KeySym k;
	int ts=0;
	int mx=0;
	int my=0;
	
	switch(e->type)
	{
		case KeyPress:
			lua='k';
			act="down";
		break;
		case KeyRelease:
			lua='k';
			act="up";
		break;
		case ButtonPress:
			lua='m';
			act="down";
			key=mbutname(e->xbutton.button);
			mx=e->xbutton.x;
			my=e->xbutton.y;
		break;
		case ButtonRelease:
			lua='m';
			act="up";
			key=mbutname(e->xbutton.button);
			mx=e->xbutton.x;
			my=e->xbutton.y;
		break;
		case MotionNotify:
			lua='m';
			act="move";
			key="none";
			mx=e->xmotion.x;
			my=e->xmotion.y;
		break;
		
		case ConfigureNotify:
//			ogl->setup_viewport(e->xconfigure.width,e->xconfigure.height);
			width=e->xconfigure.width;
			height=e->xconfigure.height;
		break;
	}

    if(lua=='m')
    {
		lua_pushlightuserdata(l,this);
		lua_gettable(l, LUA_REGISTRYINDEX);
		lua_getfield(l,-1,"mouse");
		if( lua_isfunction(l,-1) )
		{
			lua_pushstring(l,act);
			lua_pushnumber(l,mx);
			lua_pushnumber(l,my);
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
		lua_pushlightuserdata(l,this);
		lua_gettable(l, LUA_REGISTRYINDEX);
		lua_getfield(l,-1,"keypress");
		if( lua_isfunction(l,-1) )
		{
			ts=XLookupString(&e->xkey,asc,32,&k,0);
			asc[ts]=0;
			strcpy(keyb,XKeysymToString(k));
			strlwr(keyb);
			lua_pushstring(l,asc);
			lua_pushstring(l,keyb);
			lua_pushstring(l,act);
			lua_call(l,3,0);
		}
		else
		{
			lua_pop(l,1);
		}
		lua_pop(l,1);
	}
	
	return true;
}
#endif



/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// Setup junk
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
#if defined(WIN32)

bool fenestra::setup(HWND _into_hwnd)
{
//auto height
	if(width==0)  { width=640; }
	if(height==0) { height=480; }

	call_update=true;
		
	into_hwnd=_into_hwnd; // if non zero we are going to hijack this window
// get our filename

	GetModuleFileName(0, ModuleFileName, sizeof(ModuleFileName));

// get a handle to our console window if we have one

	console=FindWindow("ConsoleWindowClass", ModuleFileName);

// get our process handle

	if(!DuplicateHandle(GetCurrentProcess(),GetCurrentProcess(),GetCurrentProcess(),&proc,0,true,DUPLICATE_SAME_ACCESS))
	{
//		DBG_Error("Couldn't get handle on current process,\n");
		goto bogus;
	}

	instance=GetModuleHandle(NULL);

// grab timmer resolution for latter use

	QueryPerformanceFrequency( (LARGE_INTEGER *) (&time_rez) );

// set up frame timer

	time_min_chunk=1000/50;
	time_last_chunk=time_min_chunk;
	time_start=time();
	time_last=time_start;

    ZeroMemory( wc , sizeof(wc ) );

	wc->cbSize=sizeof(WNDCLASSEX);
	wc->style=CS_CLASSDC;
	wc->lpfnWndProc=msg_handler;
	wc->hInstance=instance;
	wc->lpszClassName="XIX:fenestra";
	wc->hIcon=LoadIcon(NULL, IDI_APPLICATION);
	wc->hCursor=LoadCursor(NULL,IDC_ARROW); 
	wc->hbrBackground=(HBRUSH)GetStockObject(BLACK_BRUSH);


    RegisterClassEx( wc );

RECT srect[1];

	SystemParametersInfo( SPI_GETWORKAREA , 0 , srect , 0);

	int x,y;

	x=srect->left + ( (( srect->right-srect->left ) - width )/2);
	y=srect->top + ( (( srect->bottom-srect->top ) - height )/2);

	if(x<40) { x=40; }
	if(y<40) { y=40; }

RECT rect[1];
	rect->left=x;
	rect->top=y;
	rect->right=rect->left+width;
	rect->bottom=rect->top+height;

	AdjustWindowRect(rect,WS_OVERLAPPEDWINDOW,false);

	if( into_hwnd )
	{
		GetClientRect(into_hwnd,rect);

		hwnd = CreateWindow( "XIX:fenestra", "http://www.WetGenes.com/ - fenestra", 
							  WS_CHILDWINDOW, rect->left, rect->top, rect->right-rect->left, rect->bottom-rect->top,
							  into_hwnd, NULL, instance, this );

		ShowWindow( hwnd, SW_SHOWDEFAULT );
		
		SetFocus(hwnd);
	}
	else
	{
		hwnd = CreateWindow( "XIX:fenestra", "http://www.WetGenes.com/ - fenestra", 
							  WS_OVERLAPPEDWINDOW, rect->left, rect->top, rect->right-rect->left, rect->bottom-rect->top,
							  GetDesktopWindow(), NULL, instance, this );

		ShowWindow( hwnd, SW_SHOWDEFAULT );
	}
	
    UpdateWindow( hwnd );

	SetTimer(hwnd,0,1,0); // hit the msg loop repeatedly

	return(true);
bogus:
	clean();
	return(false);
}

#elif defined(X11)

bool fenestra::setup()
{
//auto height
	if(width==0)  { width=640; }
	if(height==0) { height=480; }

	bool ret=false;
	
// set up time info

	time_min_chunk=1000/50;
	time_last_chunk=time_min_chunk;
	time_start=time();
	time_last=time_start;
	
	dsp = XOpenDisplay( NULL );
	if(dsp)
	{
		fp_dsp=ConnectionNumber(dsp);
		FD_ZERO(&set_dsp);
        FD_SET(fp_dsp, &set_dsp);
        
		screen = DefaultScreen(dsp);
		unsigned long white = WhitePixel(dsp,screen);
		unsigned long black = BlackPixel(dsp,screen);	
		win = XCreateSimpleWindow(dsp,
										DefaultRootWindow(dsp),
										50, 50,   // origin
										width, height, // size
										0, white, // border
										black );  // backgd

		XMapWindow( dsp, win );

		XSelectInput( dsp, win, KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask | StructureNotifyMask );
		XFlush(dsp);
		  
  		ret=true;
	}

	if(!ret) { clean(); }
	return(ret);
}

#endif

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// Clean junk
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void fenestra::clean(void)
{
#if defined(WIN32)

    UnregisterClass( "XIX:fenestra", instance );

	if(proc) { CloseHandle(proc); proc=0; }
	
#elif defined(X11)

	XDestroyWindow( dsp, win );
	XCloseDisplay( dsp );

#endif
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// get the current time(not an absolute just compare time elapsed) in seconds, as a double
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
double fenestra::time(void)
{
#if defined(WIN32)

s64 tim;

	QueryPerformanceCounter( (LARGE_INTEGER *) (&(tim)) );
	
//	tim=tim-time_start; // try and keep any possible precision loss slightly under control?
	
	return ((double)tim)/((double)time_rez);
	
#else

	struct timeval tv;
	gettimeofday ( &tv, NULL );
	return ((double)tv.tv_sec) + ( ((double)tv.tv_usec) / 1000000.0 ) ;
	
#endif
}


