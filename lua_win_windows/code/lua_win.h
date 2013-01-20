/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss Daniels 2010 http://XIXs.com
//
/*+-----------------------------------------------------------------------------------------------------------------+*/

typedef struct swetwin_lua {

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
	
	HDC hDC;
	HGLRC hRC;

	int width;
	int height;
	
	int winclosed;

} wetwin_lua ;

typedef struct swetwin_lua_wrap { // this can be treated as wetwin_lua **

	wetwin_lua	*p;				// p will point to a
	wetwin_lua	a[1];			

} wetwin_lua_wrap ;

extern const char *lua_wetwin_ptr_name;


#ifdef __cplusplus
extern "C" {
#endif


LUALIB_API	int luaopen_wetgenes_win_windows (lua_State *l);


#ifdef __cplusplus
};
#endif
