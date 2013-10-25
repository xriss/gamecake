#include "all.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>

#import <Cocoa/Cocoa.h>
#include <OpenGL/OpenGL.h>

//			#include <OpenGL/gl.h>
//			#include <OpenGL/glu.h>


//#include "../fssimplewindow.h"

//
// we can use either this string as a string identifier
// or its address as a light userdata identifier, both will be unique
//
const char *lua_wetwin_ptr_name="wetwin*ptr";


@interface WetDelegate : NSObject /* < NSApplicationDelegate > */
/* Example: Fire has the same problem no explanation */
{
}
/* - (BOOL) applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)theApplication; */
@end

@implementation WetDelegate
- (BOOL) applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)theApplication
{
	return YES;
}

@end



@interface WetWindow : NSWindow
{
}

@end

@implementation WetWindow
- (id) initWithContentRect: (NSRect)rect styleMask:(NSUInteger)wndStyle backing:(NSBackingStoreType)bufferingType defer:(BOOL)deferFlg
{
	[super initWithContentRect:rect styleMask:wndStyle backing:bufferingType defer:deferFlg];

	[[NSNotificationCenter defaultCenter] 
		addObserver:self
		selector:@selector(windowDidResize:)
		name:NSWindowDidResizeNotification
		object:self];

	[[NSNotificationCenter defaultCenter]
	  addObserver:self
	  selector:@selector(windowWillClose:)
	  name:NSWindowWillCloseNotification
	  object:self];

	[self setAcceptsMouseMovedEvents:YES];

	printf("%s\n",__FUNCTION__);
	return self;
}

- (NSSize)windowWillResize:(NSWindow *)sender toSize:(NSSize)framesize
{
	return framesize;
}	

- (BOOL)windowShouldZoom:(NSWindow *)sender toFrame:(NSRect)newFrame
{
	return YES;
}
			   
				   
- (void) windowDidResize: (NSNotification *)notification
{
}

- (void) windowWillClose: (NSNotification *)notification
{
	[NSApp terminate:nil];	// This can also be exit(0);
}

@end


@interface WetView : NSOpenGLView 
{
}
- (void) drawRect: (NSRect) bounds;
@end

@implementation WetView
-(void) drawRect: (NSRect) bounds
{
	printf("%s\n",__FUNCTION__);
//	exposure=1;
}

-(void) prepareOpenGL
{
	printf("%s\n",__FUNCTION__);
}

@end




/*
void YsAddMenu(void)
{
 	NSAutoreleasePool *pool=[[NSAutoreleasePool alloc] init];

	NSMenu *mainMenu;

	mainMenu=[NSMenu alloc];
	[mainMenu initWithTitle:@"Minimum"];

	NSMenuItem *fileMenu;
	fileMenu=[[NSMenuItem alloc] initWithTitle:@"File" action:NULL keyEquivalent:[NSString string]];
	[mainMenu addItem:fileMenu];

	NSMenu *fileSubMenu;
	fileSubMenu=[[NSMenu alloc] initWithTitle:@"File"];
	[fileMenu setSubmenu:fileSubMenu];

	NSMenuItem *fileMenu_Quit;
	fileMenu_Quit=[[NSMenuItem alloc] initWithTitle:@"Quit"  action:@selector(terminate:) keyEquivalent:@"q"];
	[fileMenu_Quit setTarget:NSApp];
	[fileSubMenu addItem:fileMenu_Quit];

	[NSApp setMainMenu:mainMenu];

	[pool release];
}

void YsTestApplicationPath(void)
{
 	NSAutoreleasePool *pool=[[NSAutoreleasePool alloc] init];

	char cwd[256];
	getcwd(cwd,255);
	printf("CWD(Initial): %s\n",cwd);

	NSString *path;
	path=[[NSBundle mainBundle] bundlePath];
	printf("BundlePath:%s\n",[path UTF8String]);

	[[NSFileManager defaultManager] changeCurrentDirectoryPath:path];

	getcwd(cwd,255);
	printf("CWD(Changed): %s\n",cwd);

	[pool release];
}




static WetWindow *staticWindow=nil;
static WetView *staticView=nil;

void FsOpenWindowC(int x0,int y0,int wid,int hei,int useDoubleBuffer)
{
 	NSAutoreleasePool *pool=[[NSAutoreleasePool alloc] init];


	[NSApplication sharedApplication];
	[NSBundle loadNibNamed:@"MainMenu" owner:NSApp];

	WetDelegate *delegate;
	delegate=[WetDelegate alloc];
	[delegate init];
	[NSApp setDelegate: delegate];
	
	[NSApp finishLaunching];



	NSRect contRect;
	contRect=NSMakeRect(x0,y0,wid,hei);
	
	unsigned int winStyle=
	  NSTitledWindowMask|
	  NSClosableWindowMask|
	  NSMiniaturizableWindowMask|
	  NSResizableWindowMask;
	
	staticWindow=[WetWindow alloc];
	[staticWindow
		initWithContentRect:contRect
		styleMask:winStyle
		backing:NSBackingStoreBuffered 
		defer:NO];

	NSOpenGLPixelFormat *format;
	NSOpenGLPixelFormatAttribute formatAttrib[]=
	{
		NSOpenGLPFAWindow,
		NSOpenGLPFADepthSize,(NSOpenGLPixelFormatAttribute)32,
		NSOpenGLPFADoubleBuffer,
		0
	};

	if(useDoubleBuffer==0)
	{
		formatAttrib[3]=0;
	}

	format=[NSOpenGLPixelFormat alloc];
	[format initWithAttributes: formatAttrib];
	
	staticView=[WetView alloc];
	contRect=NSMakeRect(0,0,800,600);
	[staticView
		initWithFrame:contRect
		pixelFormat:format];
	
	[staticWindow setContentView:staticView];
	[staticWindow makeFirstResponder:staticView];

	[staticWindow makeKeyAndOrderFront:nil];
	[staticWindow makeMainWindow];

	[NSApp activateIgnoringOtherApps:YES];

	YsAddMenu();

	[pool release];


	int i;
	for(i=0; i<FSKEY_NUM_KEYCODE; i++)
	{
		fsKeyIsDown[i]=0;
	}


    glClearColor(1.0F,1.0F,1.0F,0.0F);
    glClearDepth(1.0F);
	glDisable(GL_DEPTH_TEST);

	glViewport(0,0,wid,hei);

    glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0,(float)wid-1,(float)hei-1,0,-1,1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glShadeModel(GL_FLAT);
	glPointSize(1);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glColor3ub(0,0,0);
}

void FsGetWindowSizeC(int *wid,int *hei)
{
	NSRect rect;
	rect=[staticView frame];
	*wid=rect.size.width;
	*hei=rect.size.height;
}

void FsPollDeviceC(void)
{
 	NSAutoreleasePool *pool=[[NSAutoreleasePool alloc] init];

	while(1)
	{
	 	[pool release];
	 	pool=[[NSAutoreleasePool alloc] init];
	
		NSEvent *event;
		event=[NSApp
			   nextEventMatchingMask:NSAnyEventMask
			   untilDate: [NSDate distantPast]
			   inMode: NSDefaultRunLoopMode
			   dequeue:YES];
		if([event type]==NSRightMouseDown)
		  {
		    printf("R mouse down event\n");
		  }
		if(event!=nil)
		{
			[NSApp sendEvent:event];
			[NSApp updateWindows];
		}
		else
		{
			break;
		}
	}

	[pool release];	
}

void FsSleepC(int ms)
{
	if(ms>0)
	{
		double sec;
		sec=(double)ms/1000.0;
		[NSThread sleepForTimeInterval:sec];
	}
}

int FsPassedTimeC(void)
{
	int ms;

 	NSAutoreleasePool *pool=[[NSAutoreleasePool alloc] init];

	static NSTimeInterval last=0.0;
	NSTimeInterval now;

	now=[[NSDate date] timeIntervalSince1970];

	NSTimeInterval passed;
	passed=now-last;
	ms=(int)(1000.0*passed);

	if(ms<0)
	{
		ms=1;
	}
	last=now;

	[pool release];	

	return ms;
}

void FsMouseC(int *lb,int *mb,int *rb,int *mx,int *my)
{
	*lb=mouseLb;
	*mb=mouseMb;
	*rb=mouseRb;

	NSPoint loc;
	loc=[NSEvent mouseLocation];
	loc=[staticWindow convertScreenToBase:loc];
	loc=[staticView convertPointFromBase:loc];

	NSRect rect;
	rect=[staticView frame];
	*mx=loc.x;
	*my=rect.size.height-1-loc.y;
}

int FsGetMouseEventC(int *lb,int *mb,int *rb,int *mx,int *my)
{
	if(0<nMosBufUsed)
	{
		const int eventType=mosBuffer[0].eventType;
		*lb=mosBuffer[0].lb;
		*mb=mosBuffer[0].mb;
		*rb=mosBuffer[0].rb;
		*mx=mosBuffer[0].mx;
		*my=mosBuffer[0].my;

		int i;
		for(i=0; i<nMosBufUsed-1; i++)
		{
			mosBuffer[i]=mosBuffer[i+1];
		}

		nMosBufUsed--;
		return eventType;
	}
	else
	{
		FsMouseC(lb,mb,rb,mx,my);
		return FSMOUSEEVENT_NONE;
	}
}

void FsSwapBufferC(void)
{
	[[staticView openGLContext] flushBuffer];
}

int FsInkeyC(void)
{
	if(nKeyBufUsed>0)
	{
		int i,fskey;
		fskey=keyBuffer[0];
		nKeyBufUsed--;
		for(i=0; i<nKeyBufUsed; i++)
		{
			keyBuffer[i]=keyBuffer[i+1];
		}
		return fskey;
	}
	return 0;
}

int FsInkeyCharC(void)
{
	if(nCharBufUsed>0)
	{
		int i,c;
		c=charBuffer[0];
		nCharBufUsed--;
		for(i=0; i<nCharBufUsed; i++)
		{
			charBuffer[i]=charBuffer[i+1];
		}
		return c;
	}
	return 0;
}

int FsKeyStateC(int fsKeyCode)
{
	if(0<=fsKeyCode && fsKeyCode<FSKEY_NUM_KEYCODE)
	{
		return fsKeyIsDown[fsKeyCode];
	}
	return 0;
}

void FsChangeToProgramDirC(void)
{
	NSString *path;
	path=[[NSBundle mainBundle] bundlePath];
	printf("BundlePath:%s\n",[path UTF8String]);

	[[NSFileManager defaultManager] changeCurrentDirectoryPath:path];
}

int FsCheckExposureC(void)
{
	int ret;
	ret=exposure;
	exposure=0;
	return ret;
}
*/

/* int main(int argc, char *argv[])
{
	YsTestApplicationPath();

	YsOpenWindow();

	printf("Going into the event loop\n");

	double angle;
	angle=0.0;
	while(1)
	{
		YsPollEvent();

		DrawTriangle(angle);
		angle=angle+0.05;

		YsSleep(20);
	}

	return 0;
	} */
	
	
	

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
//	Display		*dsp = XOpenDisplay( NULL );


	lua_pushnumber(l,800);//DisplayWidth(dsp,0));
	lua_pushnumber(l,600);//DisplayHeight(dsp,0));
	
	
//	XCloseDisplay( dsp );
	
	return 2;
}


static WetWindow *staticWindow=nil;
static WetView *staticView=nil;

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

const char *title=" http://gamecake.4lfa.com/ ";
	lua_getfield(l,1,"title");	if( lua_isstring(l,-1) ) { title=lua_tostring(l,-1);	} lua_pop(l,1);

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
	
	p->width=width;
	p->height=height;

 	NSAutoreleasePool *pool=[[NSAutoreleasePool alloc] init];

	[NSApplication sharedApplication];

	WetDelegate *delegate;
	delegate=[WetDelegate alloc];
	[delegate init];
	[NSApp setDelegate: delegate];
	
	[NSApp finishLaunching];

	NSRect contRect;
	contRect=NSMakeRect(x,y,width,height);
	
	unsigned int winStyle=
	  NSTitledWindowMask|
	  NSClosableWindowMask|
	  NSMiniaturizableWindowMask|
	  NSResizableWindowMask;
	
	staticWindow=[WetWindow alloc];
	[staticWindow
		initWithContentRect:contRect
		styleMask:winStyle
		backing:NSBackingStoreBuffered 
		defer:NO];
		
	NSOpenGLPixelFormat *format;
	NSOpenGLPixelFormatAttribute formatAttrib[]=
	{
		NSOpenGLPFAWindow,
		NSOpenGLPFADepthSize,(NSOpenGLPixelFormatAttribute)32,
		NSOpenGLPFADoubleBuffer,
		0
	};

	format=[NSOpenGLPixelFormat alloc];
	[format initWithAttributes: formatAttrib];
	
//	NSRect contRect;
	contRect=NSMakeRect(0,0,p->width,p->height);
	staticView=[WetView alloc];
	[staticView
		initWithFrame:contRect
		pixelFormat:format];
	
	[staticWindow setContentView:staticView];
	[staticWindow makeFirstResponder:staticView];
	
	[staticWindow makeKeyAndOrderFront:nil];
	[staticWindow makeMainWindow];

	[NSApp activateIgnoringOtherApps:YES];

	[pool release];


	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// fullscreen a window as best we can...
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_wetwin_show(lua_State *l)
{
//XEvent xev;

wetwin_lua *p=lua_wetwin_check_ptr(l,1);

/*

Atom _NET_WM_STATE 					= XInternAtom(p->dsp, "_NET_WM_STATE", False);
Atom _NET_WM_STATE_MAXIMIZED_VERT 	= XInternAtom(p->dsp, "_NET_WM_STATE_MAXIMIZED_VERT", False);
Atom _NET_WM_STATE_MAXIMIZED_HORZ 	= XInternAtom(p->dsp, "_NET_WM_STATE_MAXIMIZED_HORZ", False);
Atom _NET_WM_STATE_FULLSCREEN 		= XInternAtom(p->dsp, "_NET_WM_STATE_FULLSCREEN", False);
//Atom _NET_WM_STATE_FOCUSED 			= XInternAtom(p->dsp, "_NET_WM_STATE_FOCUSED", False);
*/

int max=0;
int full=0;

const char *s=lua_tostring(l,2);

	if(s)
	{
		if(strcmp("max",s)==0) { max=1; }   // maximised, can still see taskbar/title
		if(strcmp("full",s)==0) { full=1; } // try and take over the screen
	}

/*
	if(full)
	{
		memset(&xev, 0, sizeof(xev));
		xev.type = ClientMessage;
		xev.xclient.window = p->win;
		xev.xclient.message_type = _NET_WM_STATE;
		xev.xclient.format = 32;
		xev.xclient.data.l[0] = 1; // add
		xev.xclient.data.l[1] = _NET_WM_STATE_FULLSCREEN;
		xev.xclient.data.l[2] = _NET_WM_STATE_FULLSCREEN;
		XSendEvent(p->dsp, DefaultRootWindow(p->dsp), False, SubstructureNotifyMask, &xev);
	}
	else
    if(max)
    {
		memset(&xev, 0, sizeof(xev));
		xev.type = ClientMessage;
		xev.xclient.window = p->win;
		xev.xclient.message_type = _NET_WM_STATE;
		xev.xclient.format = 32;
		xev.xclient.data.l[0] = 1; // add
		xev.xclient.data.l[1] = _NET_WM_STATE_MAXIMIZED_VERT;
		xev.xclient.data.l[2] = _NET_WM_STATE_MAXIMIZED_HORZ;
		XSendEvent(p->dsp, DefaultRootWindow(p->dsp), False, SubstructureNotifyMask, &xev);
	}
    else // clear all flags
    {
		memset(&xev, 0, sizeof(xev));
		xev.type = ClientMessage;
		xev.xclient.window = p->win;
		xev.xclient.message_type = _NET_WM_STATE;
		xev.xclient.format = 32;
		xev.xclient.data.l[0] = 0; // remove
		xev.xclient.data.l[1] = _NET_WM_STATE_MAXIMIZED_VERT;
		xev.xclient.data.l[2] = _NET_WM_STATE_MAXIMIZED_HORZ;
		XSendEvent(p->dsp, DefaultRootWindow(p->dsp), False, SubstructureNotifyMask, &xev);

		memset(&xev, 0, sizeof(xev));
		xev.type = ClientMessage;
		xev.xclient.window = p->win;
		xev.xclient.message_type = _NET_WM_STATE;
		xev.xclient.format = 32;
		xev.xclient.data.l[0] = 0; // remove
		xev.xclient.data.l[1] = _NET_WM_STATE_FULLSCREEN;
		xev.xclient.data.l[2] = _NET_WM_STATE_FULLSCREEN;
		XSendEvent(p->dsp, DefaultRootWindow(p->dsp), False, SubstructureNotifyMask, &xev);
    }
*/

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

//[NSApp terminate:nil];

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

	[[staticView openGLContext] flushBuffer];

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
NSEvent *event;

 	NSAutoreleasePool *pool=[[NSAutoreleasePool alloc] init];

	event=[NSApp
		   nextEventMatchingMask:NSAnyEventMask
		   untilDate: [NSDate distantPast]
		   inMode: NSDefaultRunLoopMode
		   dequeue:NO];

	if(event!=nil)
	{
		lua_pushboolean(l,1);
	}
	else
	{
		lua_pushboolean(l,0);
	}
	
	[pool release];	
	
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

//	struct timeval tv;
//	tv.tv_sec = 0;
//	tv.tv_usec = 1000; // 1ms
	
//	select(p->fp_dsp, &p->set_dsp, 0, 0, &tv);

	[NSThread sleepForTimeInterval:0.001];

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

 	NSAutoreleasePool *pool=[[NSAutoreleasePool alloc] init];

	NSEvent *event;
	event=[NSApp
		   nextEventMatchingMask:NSAnyEventMask
		   untilDate: [NSDate distantPast]
		   inMode: NSDefaultRunLoopMode
		   dequeue:YES];
	if(event!=nil)
	{
		[NSApp sendEvent:event];
		[NSApp updateWindows];
	}

	[pool release];	

/*
 * 
char lua=' ';
int act=0;
int key=0;
int x=0;
int y=0;
char asc[255];
KeySym k;
double time;

	XEvent e[1];
	if( XPending( p->dsp ) > 0 )
	{
		XNextEvent(p->dsp, e);
		
		switch(e->type)
		{
			case KeyPress:
				time=(e->xkey.time/1000.0);
				lua='k';
				act=1;
			break;
			case KeyRelease:
				time=(e->xkey.time/1000.0);
				lua='k';
				act=-1;
			break;
			
			case ButtonPress:
				time=(e->xbutton.time/1000.0);
				lua='m';
				act=1;
				key=(e->xbutton.button);
				x=e->xbutton.x;
				y=e->xbutton.y;
			break;
			case ButtonRelease:
				time=(e->xbutton.time/1000.0);
				lua='m';
				act=-1;
				key=(e->xbutton.button);
				x=e->xbutton.x;
				y=e->xbutton.y;
			break;
			case MotionNotify:
				time=(e->xmotion.time/1000.0);
				lua='m';
				x=e->xmotion.x;
				y=e->xmotion.y;
			break;
			
			case ConfigureNotify:
//				time=(e->xconfigure.time/1000.0);
				lua='s';
				x=e->xconfigure.width;
				y=e->xconfigure.height;
				
				// remember this value
				p->width=x;
				p->height=y;
				
			break;
		}
	}
	
    if(lua=='m')
    {
		lua_newtable(l);
		lua_pushnumber(l,time);					lua_setfield(l,-2,"time");
		lua_pushstring(l,"mouse");				lua_setfield(l,-2,"class");
		lua_pushnumber(l,key);					lua_setfield(l,-2,"keycode");
		lua_pushnumber(l,act);					lua_setfield(l,-2,"action");
		lua_pushnumber(l,x);					lua_setfield(l,-2,"x");
		lua_pushnumber(l,y);					lua_setfield(l,-2,"y");
		return 1;
	}
    else
    if(lua=='k')
    {
		int ts=XLookupString(&e->xkey,asc,32,&k,0);
		asc[ts]=0; // null term the ascii

		lua_newtable(l);
		lua_pushnumber(l,time);					lua_setfield(l,-2,"time");
		lua_pushstring(l,"key");				lua_setfield(l,-2,"class");
		lua_pushstring(l,asc);					lua_setfield(l,-2,"ascii");
		lua_pushnumber(l,act);					lua_setfield(l,-2,"action");
		lua_pushnumber(l,k);					lua_setfield(l,-2,"keycode");
		lua_pushstring(l,XKeysymToString(k));	lua_setfield(l,-2,"keyname");
		return 1;
	}
    else
    if(lua=='s')
	{	
		lua_newtable(l);
//		lua_pushnumber(l,time);					lua_setfield(l,-2,"time");
		lua_pushstring(l,"size");				lua_setfield(l,-2,"class");
		lua_pushnumber(l,x);					lua_setfield(l,-2,"x");
		lua_pushnumber(l,y);					lua_setfield(l,-2,"y");
		return 1;
	}
*/

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

	[NSThread sleepForTimeInterval:n];

    return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// what time is it, with sub second resolution
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_wetwin_time (lua_State *l)
{
	struct timeval tv;
	int r=gettimeofday ( &tv, NULL );
	lua_pushnumber(l, ((double)tv.tv_sec) + ( ((double)tv.tv_usec) / 1000000.0 ) );

//printf("%d,%f\n",r,(float)((double)tv.tv_sec) + ( ((double)tv.tv_usec) / 1000000.0 ));

	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// this is fucking dangerous
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_wetwin_ioctl (lua_State *l)
{
int i1 = (int)luaL_checknumber(l, 1);
int i2 = (int)luaL_checknumber(l, 2);
int i3 = (int)luaL_checknumber(l, 3);
int r;
//	r=ioctl(i1,i2,i3);
	lua_pushnumber(l,(double)r);
	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// non blocking getc, returns nil if there is nothing there, otherwise a char code
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_wetwin_getc (lua_State *l)
{
char c;
    struct timeval tv;
    fd_set fds;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    FD_ZERO(&fds);
    FD_SET(0, &fds); //STDIN_FILENO is 0
    select(0+1, &fds, NULL, NULL, &tv);
    if( FD_ISSET(0, &fds) )
    {
		c=fgetc(stdin);
		lua_pushnumber(l,c);
		return 1;
	}
	
	return 0;
}
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// open library.
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
LUALIB_API int luaopen_wetgenes_win_osx_core(lua_State *l)
{
	const luaL_reg lib[] =
	{
		{"screen",			lua_wetwin_screen},
		
		{"create",			lua_wetwin_create},
		{"destroy",			lua_wetwin_destroy},
		{"info",			lua_wetwin_info},
		{"show",			lua_wetwin_show},

		{"context",			lua_wetwin_context},
		{"swap",			lua_wetwin_swap},

		{"peek",			lua_wetwin_peek},
		{"wait",			lua_wetwin_wait},
		{"msg",				lua_wetwin_msg},

		{"sleep",			lua_wetwin_sleep},
		{"time",			lua_wetwin_time},
		
//		{"jread",			lua_wetwin_jread},
		
//		{"ioctl",			lua_wetwin_ioctl},
//		{"getc",			lua_wetwin_getc},
		
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

