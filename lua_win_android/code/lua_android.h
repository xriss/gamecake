/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss@XIXs.com 2013
//
/*+-----------------------------------------------------------------------------------------------------------------+*/

typedef struct sandroid_lua {

	EGLDisplay display;
	EGLSurface surface;
	EGLContext context;

	ANativeWindow* window;
	
	int screen_width;
	int screen_height;

	int width;
	int height;
	
	ASensorEventQueue *sensors;
	const ASensor *sensor_acc;
	const ASensor *sensor_gyro;
	const ASensor *sensor_mag;


EGLConfig config;

} android_lua ;

typedef struct sandroid_lua_wrap { // this can be treated as wetwin_lua **

	android_lua	*p;				// p will point to a
	android_lua	a[1];			

} android_lua_wrap ;

extern const char *lua_android_ptr_name;

extern struct android_app* master_android_app;


#ifdef __cplusplus
extern "C" {
#endif


LUALIB_API	int luaopen_wetgenes_win_android_core (lua_State *l);


#ifdef __cplusplus
};
#endif
