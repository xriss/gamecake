/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss@XIXs.com 2013
//
/*+-----------------------------------------------------------------------------------------------------------------+*/



// a main window class to hangs everything off of

struct fenestra
{
	
	lua_State *l;

	struct fenestra_ogl  ogl[1];
	struct fenestra_data data[1];	

	double	time_start;
	double	time_last;

	double	time_min_chunk;
	double	time_last_chunk;

#if defined(WIN32)

// startup junk, filled passed in from WinMain if we have one...

    LPSTR lpCmdLine;
    int nCmdShow;

    HINSTANCE hInstance;
    HINSTANCE hPrevInstance;
    
// or if running as a plugin we must use this window
	HWND into_hwnd; // a window to hijack?
	WNDPROC into_hwnd_OldProc; // and its old proc

	char ModuleFileName[260];

// our process handle

	HANDLE proc;

	WNDCLASSEX wc[1]; //window class init info
	
// our window

	HWND hwnd;
	HWND console; // if we are a console app

	HINSTANCE instance;
	
// main loop timer information
	
	s64		time_rez;

// is window active

	bool	active_window;


#elif defined(X11)

	int fp_dsp; // display as file
	fd_set set_dsp; // display as set
	Display *dsp;
	int screen;
	Window win;
 
#endif

// a cached window size
	int width;
	int height;

	bool call_update;

	
// input args?

	int   argc;
	char **argv;

// or a raw

	char *command_line;


// functions

	void  prepare();

#if defined(WIN32)
	bool setup(HWND _into_hwnd);
#elif defined(X11)
	bool setup(void);
	bool event_handler( XEvent *e );
#endif

	void clean(void);

	double time(void);

};


#ifdef __cplusplus
extern "C" {
#endif

LUALIB_API	int luaopen_fenestra_core (lua_State *l);

#ifdef __cplusplus
};
#endif
