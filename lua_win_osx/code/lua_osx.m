#include "all.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>

#import <Cocoa/Cocoa.h>
#include <OpenGL/OpenGL.h>

//
// we can use either this string as a string identifier
// or its address as a light userdata identifier, both will be unique
//
const char *lua_wetwin_ptr_name="wetwin*ptr";

static wetwin_lua *staticP=nil;

@interface WetDelegate : NSObject
{
}
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

//	printf("%s\n",__FUNCTION__);
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
//- (void) drawRect: (NSRect) bounds;
@end

@implementation WetView
-(void) drawRect: (NSRect) bounds
{
	[[NSColor blackColor] set];
        NSRectFill( bounds );
        
   	if(staticP)
	{
		staticP->width=bounds.size.width;
		staticP->height=bounds.size.height;
	}
//	printf("%s\n",__FUNCTION__);
}

-(void) prepareOpenGL
{
//	printf("%s\n",__FUNCTION__);
}

@end


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

NSRect e = [[NSScreen mainScreen] frame];

	lua_pushnumber(l,e.size.height);
	lua_pushnumber(l,e.size.width);
	
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
	
	staticP=p;

	p->width=width;
	p->height=height;

 	NSAutoreleasePool *pool=[[NSAutoreleasePool alloc] init];

	[[NSApplication sharedApplication] setActivationPolicy: NSApplicationActivationPolicyRegular];


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
	
	[staticWindow setCollectionBehavior:NSWindowCollectionBehaviorFullScreenPrimary];
//	[staticWindow setRestorable:YES];
	[staticWindow makeKeyAndOrderFront:nil];
	[staticWindow makeMainWindow];

	[[staticWindow standardWindowButton:NSWindowZoomButton] setEnabled:YES];

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

wetwin_lua *p=lua_wetwin_check_ptr(l,1);

int max=0;
int full=0;

const char *s=lua_tostring(l,2);

	if(s)
	{
		if(strcmp("max",s)==0) { max=1; }   // maximised, can still see taskbar/title
		if(strcmp("full",s)==0) { full=1; } // try and take over the screen
	}

	if(full)
	{
		if( ([staticWindow styleMask] & NSFullScreenWindowMask) != NSFullScreenWindowMask )
		{
			[staticWindow toggleFullScreen:nil];
		}
	}
	else
    if(max)
    {
		if( ! [staticWindow isZoomed] )
		{
			[staticWindow zoom:nil];
		}
	}
    else // clear all flags
    {
		if( ([staticWindow styleMask] & NSFullScreenWindowMask) == NSFullScreenWindowMask )
		{
			[staticWindow toggleFullScreen:nil];
		}
		if( [staticWindow isZoomed] )
		{
			[staticWindow zoom:nil];
		}
    }

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

// already have a context

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

int typ=0;
double tim=0.0;

char lua=' ';
int act=0;
int key=0;
int x=0;
int y=0;

NSPoint loc;
NSString *str;
const char *asc="";

	NSEvent *event;
	event=[NSApp
		   nextEventMatchingMask:NSAnyEventMask
		   untilDate: [NSDate distantPast]
		   inMode: NSDefaultRunLoopMode
		   dequeue:YES];
		   
	if(event!=nil)
	{
		typ=[event type];
		tim=[event timestamp];
		
		switch(typ)
		{
			case NSKeyDown:
				act=1;
				lua='k';
			break;
			case NSKeyUp:
				act=-1;
				lua='k';
			break;
			case NSMouseMoved:
			case NSLeftMouseDragged:
			case NSRightMouseDragged:
			case NSOtherMouseDragged:
				lua='m';
				act=0;
			break;
			case NSLeftMouseDown:
				lua='m';
				act=1;
				key=1;
			break;
			case NSLeftMouseUp:
				lua='m';
				act=-1;
				key=1;
			break;
			case NSRightMouseDown:
				lua='m';
				act=1;
				key=3;
			break;
			case NSRightMouseUp:
				lua='m';
				act=-1;
				key=3;
			break;
			case NSOtherMouseDown:
				lua='m';
				act=1;
				key=2;
			break;
			case NSOtherMouseUp:
				lua='m';
				act=-1;
				key=2;
			break;
		}

		if(lua=='k')
		{
			str=[event characters];
			key=[event keyCode];
			asc=[str UTF8String];
		}

		if(lua=='m')
		{
			loc=[event locationInWindow];
			x=loc.x;
			y=staticP->height-loc.y;
		}
		

		if(lua!='k')
		{
			[NSApp sendEvent:event];
			[NSApp updateWindows];
		}
	}

	[pool release];	

//printf("%d,%g\t\t%d,%d\n",typ,tim,x,y);
	
    if(lua=='m')
    {
		lua_newtable(l);
		lua_pushnumber(l,tim);					lua_setfield(l,-2,"time");
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
		lua_newtable(l);
		lua_pushnumber(l,tim);					lua_setfield(l,-2,"time");
		lua_pushstring(l,"key");				lua_setfield(l,-2,"class");
		lua_pushstring(l,asc);					lua_setfield(l,-2,"ascii");
		lua_pushnumber(l,act);					lua_setfield(l,-2,"action");
		lua_pushnumber(l,key);					lua_setfield(l,-2,"keycode");
//		lua_pushstring(l,XKeysymToString(k));	lua_setfield(l,-2,"keyname");
		return 1;
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

