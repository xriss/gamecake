/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss Daniels 2010 http://XIXs.com
//
/*+-----------------------------------------------------------------------------------------------------------------+*/

typedef struct sraspi_lua {

/*
 * 	int			fp_dsp;			// display as file
	fd_set		set_dsp;		// display as set
	Display		*dsp;
	int			screen;
	Window		win;
	
	GLXContext context;
*/

	EGLDisplay display;
	EGLSurface surface;
	EGLContext context;

	EGL_DISPMANX_WINDOW_T nativewindow;
	
	int screen_width;
	int screen_height;

	int width;
	int height;

} raspi_lua ;

typedef struct sraspi_lua_wrap { // this can be treated as wetwin_lua **

	raspi_lua	*p;				// p will point to a
	raspi_lua	a[1];			

} raspi_lua_wrap ;

extern const char *lua_raspi_ptr_name;


#ifdef __cplusplus
extern "C" {
#endif


LUALIB_API	int luaopen_wetgenes_raspi_core (lua_State *l);


#ifdef __cplusplus
};
#endif
