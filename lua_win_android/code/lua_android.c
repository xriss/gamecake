#include <jni.h>
#include <errno.h>

#include <EGL/egl.h>
#include <GLES/gl.h>

#include <android/sensor.h>
#include <android/log.h>
#include <android_native_app_glue.h>


#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"


#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <signal.h>
#include <string.h>

#include <unistd.h>
#include <sys/time.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>


#include <android/log.h>

#include "lua.h"
#include "lauxlib.h"

#include "EGL/egl.h"
#include <GLES/gl.h>

#include "android_native_app_glue.h"

#include "lua_android.h"


#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "lua", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "lua", __VA_ARGS__))

static void llog (const char *msg) {
  LOGI(msg);
}

static int report (lua_State *L, int status) {
  if (status && !lua_isnil(L, -1)) {
    const char *msg = lua_tostring(L, -1);
    if (msg == NULL) msg = "(error object is not a string)";
    llog(msg);
    lua_pop(L, 1);
//    exit(20);
  }
  return status;
}


static int traceback (lua_State *L) {
  if (!lua_isstring(L, 1))  /* 'message' not a string? */
    return 1;  /* keep it intact */
  lua_getfield(L, LUA_GLOBALSINDEX, "debug");
  if (!lua_istable(L, -1)) {
    lua_pop(L, 1);
    return 1;
  }
  lua_getfield(L, -1, "traceback");
  if (!lua_isfunction(L, -1)) {
    lua_pop(L, 2);
    return 1;
  }
  lua_pushvalue(L, 1);  /* pass error message */
  lua_pushinteger(L, 2);  /* skip this function and traceback */
  lua_call(L, 2, 1);  /* call debug.traceback */
  return 1;
}


static int docall (lua_State *L, int narg, int clear) {
  int status;
  int base = lua_gettop(L) - narg;  /* function index */
  lua_pushcfunction(L, traceback);  /* push traceback function */
  lua_insert(L, base);  /* put it under chunk and args */
//  signal(SIGINT, laction);
  status = lua_pcall(L, narg, (clear ? 0 : LUA_MULTRET), base);
//  signal(SIGINT, SIG_DFL);
  lua_remove(L, base);  /* remove traceback function */
  /* force a complete garbage collection in case of errors */
  if (status != 0) lua_gc(L, LUA_GCCOLLECT, 0);
  return status;
}


static void print_version (void) {
  llog(LUA_RELEASE "  " LUA_COPYRIGHT);
}

static int dofile (lua_State *L, const char *name) {
  int status = luaL_loadfile(L, name) || docall(L, 0, 1);
  return report(L, status);
}


static int dostring (lua_State *L, const char *s, const char *name) {
  int status = luaL_loadbuffer(L, s, strlen(s), name) || docall(L, 0, 1);
  return report(L, status);
}


static int dolibrary (lua_State *L, const char *name) {
  lua_getglobal(L, "require");
  lua_pushstring(L, name);
  return report(L, docall(L, 1, 1));
}


/**
 * Shared state for our android app.
 */
struct engine {
	lua_State *L;

    struct android_app* app;

};

/**
 * Process the next input event.
 */
static int32_t engine_handle_input(struct android_app* app, AInputEvent* event) {
    struct engine* engine = (struct engine*)app->userData;
	lua_State *L=engine->L;

    int32_t type;
	int64_t eventtime;
    int32_t keycode;
    int32_t action;
    int32_t flags;
    int pointercount;
    float x,y;
    int id;
    int i;
    
	type=AInputEvent_getType(event);
	
    if (type == AINPUT_EVENT_TYPE_KEY)
    {
		eventtime=AKeyEvent_getEventTime(event);
		keycode=AKeyEvent_getKeyCode(event);
		action=AKeyEvent_getAction(event);
		flags=AKeyEvent_getFlags(event);
		
//		lua_getglobal(L,"mainstate");
//		if(lua_istable(L,-1)) // check we are setup
//		{
//			lua_getfield(L,-1,"android_msg");
//			lua_newtable(L);
			lua_pushstring(L,"key"); lua_setfield(L,-2,"event");
			lua_pushnumber(L,type); lua_setfield(L,-2,"type");
			lua_pushnumber(L,eventtime/1000000000.0); lua_setfield(L,-2,"eventtime");
			lua_pushnumber(L,keycode); lua_setfield(L,-2,"keycode");
			lua_pushnumber(L,action); lua_setfield(L,-2,"action");
			lua_pushnumber(L,flags); lua_setfield(L,-2,"flags");
//			report(L, docall(L, 1, 0) );
//		}
//		lua_settop(L,0);
		return 1;
	}
	else
    if (type == AINPUT_EVENT_TYPE_MOTION)
    {
		eventtime=AMotionEvent_getEventTime(event);
		pointercount=AMotionEvent_getPointerCount(event);
		action=AMotionEvent_getAction(event);
		flags=AMotionEvent_getFlags(event);
		
//		lua_getglobal(L,"mainstate");
//		if(lua_istable(L,-1)) // check we are setup
//		{
//			lua_getfield(L,-1,"android_msg");
//			lua_newtable(L);
			lua_pushstring(L,"motion"); lua_setfield(L,-2,"event");
			lua_pushnumber(L,type); lua_setfield(L,-2,"type");
			lua_pushnumber(L,eventtime/1000000000.0); lua_setfield(L,-2,"eventtime");
			lua_pushnumber(L,action); lua_setfield(L,-2,"action");
			lua_pushnumber(L,flags); lua_setfield(L,-2,"flags");
			lua_newtable(L);
			for(i=0;i<pointercount;i++)
			{
				lua_newtable(L);
				
				id=AMotionEvent_getPointerId(event,i);
				x=AMotionEvent_getX(event,i);
				y=AMotionEvent_getY(event,i);
				
				lua_pushnumber(L,id); lua_setfield(L,-2,"id");
				lua_pushnumber(L,x); lua_setfield(L,-2,"x");
				lua_pushnumber(L,y); lua_setfield(L,-2,"y");
				
				lua_rawseti(L,-2,i+1);
			}
			lua_setfield(L,-2,"pointers");
			
//			report(L, docall(L, 1, 0) );
//		}
//		lua_settop(L,0);
		return 1;
    }
    return 0;
}

static void engine_handle_cmd(struct android_app* app, int32_t cmd) {
    struct engine* engine = (struct engine*)app->userData;
	lua_State *L=engine->L;

    switch (cmd) {
		case APP_CMD_INPUT_CHANGED:
			lua_pushstring(L,"app"); lua_setfield(L,-2,"event");
			lua_pushstring(L,"input_changed"); lua_setfield(L,-2,"cmd");
		break;

		case APP_CMD_INIT_WINDOW:
			lua_pushstring(L,"app"); lua_setfield(L,-2,"event");
			lua_pushstring(L,"init_window"); lua_setfield(L,-2,"cmd");
//			if (engine->app->window != NULL) {
//			}
		break;

		case APP_CMD_TERM_WINDOW:
			lua_pushstring(L,"app"); lua_setfield(L,-2,"event");
			lua_pushstring(L,"term_window"); lua_setfield(L,-2,"cmd");
		break;

		case APP_CMD_WINDOW_RESIZED:
			lua_pushstring(L,"app"); lua_setfield(L,-2,"event");
			lua_pushstring(L,"window_resized"); lua_setfield(L,-2,"cmd");
		break;

		case APP_CMD_WINDOW_REDRAW_NEEDED:
			lua_pushstring(L,"app"); lua_setfield(L,-2,"event");
			lua_pushstring(L,"redraw_needed"); lua_setfield(L,-2,"cmd");
		break;
		
		case APP_CMD_CONTENT_RECT_CHANGED:
			lua_pushstring(L,"app"); lua_setfield(L,-2,"event");
			lua_pushstring(L,"rect_changed"); lua_setfield(L,-2,"cmd");
		break;

		case APP_CMD_GAINED_FOCUS:
			lua_pushstring(L,"app"); lua_setfield(L,-2,"event");
			lua_pushstring(L,"gained_focus"); lua_setfield(L,-2,"cmd");
		break;

		case APP_CMD_LOST_FOCUS:
			lua_pushstring(L,"app"); lua_setfield(L,-2,"event");
			lua_pushstring(L,"lost_focus"); lua_setfield(L,-2,"cmd");
		break;

		case APP_CMD_CONFIG_CHANGED:
			lua_pushstring(L,"app"); lua_setfield(L,-2,"event");
			lua_pushstring(L,"config_changed"); lua_setfield(L,-2,"cmd");
		break;

		case APP_CMD_LOW_MEMORY:
			lua_pushstring(L,"app"); lua_setfield(L,-2,"event");
			lua_pushstring(L,"low_memory"); lua_setfield(L,-2,"cmd");
		break;

		case APP_CMD_START:
			lua_pushstring(L,"app"); lua_setfield(L,-2,"event");
			lua_pushstring(L,"start"); lua_setfield(L,-2,"cmd");
		break;

		case APP_CMD_RESUME:
			lua_pushstring(L,"app"); lua_setfield(L,-2,"event");
			lua_pushstring(L,"resume"); lua_setfield(L,-2,"cmd");
		break;

		case APP_CMD_SAVE_STATE:
			lua_pushstring(L,"app"); lua_setfield(L,-2,"event");
			lua_pushstring(L,"save_state"); lua_setfield(L,-2,"cmd");
		break;

		case APP_CMD_PAUSE:
			lua_pushstring(L,"app"); lua_setfield(L,-2,"event");
			lua_pushstring(L,"pause"); lua_setfield(L,-2,"cmd");
		break;

		case APP_CMD_STOP:
			lua_pushstring(L,"app"); lua_setfield(L,-2,"event");
			lua_pushstring(L,"stop"); lua_setfield(L,-2,"cmd");
		break;

		case APP_CMD_DESTROY:
			lua_pushstring(L,"app"); lua_setfield(L,-2,"event");
			lua_pushstring(L,"destroy"); lua_setfield(L,-2,"cmd");
		break;

		default:
			lua_pushstring(L,"app"); lua_setfield(L,-2,"event");
			lua_pushstring(L,"unknown"); lua_setfield(L,-2,"cmd");
			lua_pushnumber(L,cmd); lua_setfield(L,-2,"cmid");
		break;
    }
}


struct android_app* master_android_app=0;

// switched to SLES so no longer need this
/*
extern jint JNICALL
JNI_OnLoad_openal(
	JavaVM *vm,
	void *reserved
);
*/

/**
 * This is the main entry point of a native application that is using
 * android_native_app_glue.  It runs in its own thread, with its own
 * event loop for receiving input events and doing other things.
 */
void android_main(struct android_app* state) {
    struct engine engine;
	lua_State *L;
	

	master_android_app=state;

	ANativeActivity* activity = state->activity;
	JNIEnv* env=0;

//    JNI_OnLoad_openal(activity->vm,0);

	(*activity->vm)->AttachCurrentThread(activity->vm, &env, 0);

	jclass clazz = (*env)->GetObjectClass(env, activity->clazz);
	jmethodID methodID = (*env)->GetMethodID(env, clazz, "getPackageCodePath", "()Ljava/lang/String;");
	jobject apk_result = (*env)->CallObjectMethod(env, activity->clazz, methodID);

	const char *apk;
	jboolean isCopy;
	apk = (*env)->GetStringUTFChars(env, (jstring)apk_result, &isCopy);

//		LOGI("Looked up package code path: %s", str);
   
    
    // Make sure glue isn't stripped.
    app_dummy();

    memset(&engine, 0, sizeof(engine));
    state->userData = &engine;
    state->onAppCmd = engine_handle_cmd;
    state->onInputEvent = engine_handle_input;
    engine.app = state;


	engine.L = lua_open();  /* create state */
	L=engine.L;
	
	luaL_openlibs(L);  /* open libraries */

	lua_getglobal(L,"require");
	lua_pushstring(L,"wetgenes.win");	
	report(L, docall(L, 1, 0) );
	
	lua_getfield(L,-1,"android_start");
	lua_pushstring(L,apk);

	(*env)->ReleaseStringUTFChars(env, (jstring)apk_result, apk);
	(*activity->vm)->DetachCurrentThread(activity->vm);

	report(L, docall(L, 1, 0) );		// setup and load and run lua/init.lua from the apks res/raw
	
/*

	lua_settop(L,0); // remove all the junk we just built up

	lua_getfield(L,-1,"android_start");
	report(L, docall(L, 0, 0) );		// begin the main loop ( load and call lua/init.lua )

    while (1)
    {
        // Read all pending events.
        int ident;
        int events;
		struct android_poll_source* source;
		while ((ident=ALooper_pollAll(0, NULL, &events,
			(void**)&source)) >= 0)
		{
			// Process this event.
			if (source != NULL) {
				source->process(state, source);
			}
			
		}
	
		lua_getglobal(L,"mainstate");
		if(lua_istable(L,-1)) // check we are setup
		{
			lua_getfield(L,-1,"android_serv");
			report(L, docall(L, 0, 0) );
		}
		lua_settop(L,0);

	}
*/

}





/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// Print
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_android_print (lua_State *l)
{
	const char *s=lua_tostring(l,1);

	__android_log_print(ANDROID_LOG_INFO, "lua", s);

	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// Print
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_android_time (lua_State *l)
{
	struct timeval tv ;
	gettimeofday ( & tv, NULL ) ;
	lua_pushnumber(l, (double) tv.tv_sec + (double) tv.tv_usec / 1000000.0 );
	return 1;
}



//
// we can use either this string as a string identifier
// or its address as a light userdata identifier, both will be unique
//
const char *lua_android_ptr_name="android*ptr";

	
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// check that a userdata at the given index is a android object
// return the userdata if it does, otherwise return 0
// this userdata will be a pointer to the real data
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
android_lua **lua_android_ptr_ptr (lua_State *l, int idx)
{
android_lua **pp=0;

	pp = ((android_lua **)luaL_checkudata(l, idx , lua_android_ptr_name));

	return pp;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// *lua_android_check with auto error on 0 ptr
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
android_lua *lua_android_check_ptr (lua_State *l, int idx)
{
android_lua **pp=lua_android_ptr_ptr(l,idx);

	if (*pp == 0)
	{
		luaL_error(l, "bad android userdata" );
	}

	return *pp;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// fill a table in with the current settings
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_android_getinfo (lua_State *l, android_lua *p, int tab)
{
	if(p)
	{
		if(p->display && p->surface)
		{
			eglQuerySurface(p->display, p->surface, EGL_WIDTH, &p->width);
			eglQuerySurface(p->display, p->surface, EGL_HEIGHT, &p->height);
		}
		lua_pushnumber(l,p->width);		lua_setfield(l,tab,"width");
		lua_pushnumber(l,p->height);	lua_setfield(l,tab,"height");
		
		if(master_android_app)
		{
			lua_pushnumber(l,AConfiguration_getDensity(master_android_app->config));
			lua_setfield(l,tab,"density");
		}
	}
	else
	{
		lua_pushstring(l,"unbound android"); lua_setfield(l,tab,"err");
	}

	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// set info into the given table
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_android_info (lua_State *l)
{
android_lua *p=lua_android_check_ptr(l,1);
	lua_android_getinfo(l,p,2);
	return 0;
}


static EGLint attribute_list[] =
{
	
// hard hax, please not to be reordering
	EGL_RED_SIZE, 1,	//	[1]	r
	EGL_GREEN_SIZE, 1,	//	[3]	g
	EGL_BLUE_SIZE, 1,	//	[5]	b
	EGL_ALPHA_SIZE, 0,	//	[7]	a
	EGL_DEPTH_SIZE, 1,	//	[9]	depth

	EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
	EGL_RENDERABLE_TYPE,4,
	EGL_NONE
};

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// create and return the data
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_android_create (lua_State *l)
{
android_lua_wrap *wp;
android_lua *p;

int x=0;
int y=0;

const char *title="http://www.WetGenes.com/ - GameCake";


EGLBoolean result;
int32_t success = 0;

EGLint format;

//EGLConfig config;
EGLint num_config;


	lua_getfield(l,1,"r");		if( lua_isnumber(l,-1) ) { attribute_list[1]=(int)lua_tonumber(l,-1);	} lua_pop(l,1);
	lua_getfield(l,1,"g");		if( lua_isnumber(l,-1) ) { attribute_list[3]=(int)lua_tonumber(l,-1);	} lua_pop(l,1);
	lua_getfield(l,1,"b");		if( lua_isnumber(l,-1) ) { attribute_list[5]=(int)lua_tonumber(l,-1);	} lua_pop(l,1);
	lua_getfield(l,1,"a");		if( lua_isnumber(l,-1) ) { attribute_list[7]=(int)lua_tonumber(l,-1);	} lua_pop(l,1);
	lua_getfield(l,1,"depth");	if( lua_isnumber(l,-1) ) { attribute_list[9]=(int)lua_tonumber(l,-1);	} lua_pop(l,1);

	wp = (android_lua_wrap *)lua_newuserdata(l, sizeof(android_lua_wrap)); // we need a pointer, this makes lua GC a bit easier
	memset(wp,0,sizeof(android_lua_wrap)); // make sure it is 0
	wp->p=wp->a; // point the pointer to the struct
	p=wp->p; // take an easy to use copy of the pointer
	luaL_getmetatable(l, lua_android_ptr_name);
	lua_setmetatable(l, -2);
	
	lua_android_start_p(p);

/*
	p->window=master_android_app->window;

	// get an EGL display connection
	p->display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	assert(p->display!=EGL_NO_DISPLAY);

	// initialize the EGL display connection
	result = eglInitialize(p->display, NULL, NULL);
	assert(EGL_FALSE != result);
 
  // get an appropriate EGL frame buffer configuration
   result = eglChooseConfig(p->display, attribute_list, &p->config, 1, &num_config);
   assert(EGL_FALSE != result);

//    eglGetConfigAttrib(p->display, config, EGL_NATIVE_VISUAL_ID, &format);
//    ANativeWindow_setBuffersGeometry(p->window, 0, 0, format);
    

// create an EGL rendering context
   p->context = eglCreateContext(p->display, p->config, EGL_NO_CONTEXT, NULL);
   assert(p->context!=EGL_NO_CONTEXT);

// create surface
	p->surface = eglCreateWindowSurface( p->display, p->config, p->window, NULL );
	assert(p->surface != EGL_NO_SURFACE);
*/
	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// start
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_android_start_p (android_lua *p)
{
EGLBoolean result;

EGLint num_config;

static EGLint attr_list[] =
{
	EGL_CONTEXT_CLIENT_VERSION,2,
	EGL_NONE
};

	if(!p->display)
	{
		p->window=master_android_app->window;

llog("START DISPLAY");
		// get an EGL display connection
		p->display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
		assert(p->display!=EGL_NO_DISPLAY);

		// initialize the EGL display connection
		result = eglInitialize(p->display, NULL, NULL);
		assert(EGL_FALSE != result);
	 
	  // get an appropriate EGL frame buffer configuration
	   result = eglChooseConfig(p->display, attribute_list, &p->config, 1, &num_config);
	   assert(EGL_FALSE != result);
	}

// create an EGL rendering context
	if(!p->context)
	{
llog("START CONTEXT");
		p->context = eglCreateContext(p->display, p->config, EGL_NO_CONTEXT, attr_list);
		assert(p->context!=EGL_NO_CONTEXT);
	}
// create surface
	if(!p->surface)
	{
llog("START SURFACE");
		p->surface = eglCreateWindowSurface( p->display, p->config, p->window, NULL );
		assert(p->surface != EGL_NO_SURFACE);
	}

// connect the context to the surface
   result = eglMakeCurrent(p->display, p->surface, p->surface, p->context);
   assert(EGL_FALSE != result);

}

int lua_android_start (lua_State *l)
{
android_lua *p=lua_android_check_ptr(l,1);
	lua_android_start_p(p);
	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// stop
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_android_stop (lua_State *l)
{
android_lua *p=lua_android_check_ptr(l,1);

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
	
	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// destroy data if it exists
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_android_destroy (lua_State *l)
{
android_lua **pp=lua_android_ptr_ptr(l,1);
android_lua *p;

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
int lua_android_context (lua_State *l)
{
android_lua *p=lua_android_check_ptr(l,1);

EGLBoolean result;

	if(!p->display || !p->surface || !p->context) { return 0; }
	
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
int lua_android_swap (lua_State *l)
{
android_lua *p=lua_android_check_ptr(l,1);

	if(!p->display || !p->surface || !p->context) { return 0; }

	eglSwapBuffers(p->display, p->surface);
   
	return 0;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// wait a while to see if any msgs turn up
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_android_sleep (lua_State *l)
{
    double n = luaL_checknumber(l, 1);

    struct timespec t, r;
    t.tv_sec = (int) n;
    n -= t.tv_sec;
    t.tv_nsec = (int) (n * 1000000000);
    if (t.tv_nsec >= 1000000000) t.tv_nsec = 999999999;
    while (nanosleep(&t, &r) != 0) {
        t.tv_sec = r.tv_sec;
        t.tv_nsec = r.tv_nsec;
    }
    return 0;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// read a msg if one is there
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_android_msg (lua_State *l)
{
	int ident;
	int events;
	struct android_poll_source* source;
	if ((ident=ALooper_pollAll(0, NULL, &events,
		(void**)&source)) >= 0)
	{
		if (source != NULL) {
			
			lua_newtable(l);
			source->process(master_android_app, source); // this fills in the table we just pushed with android specific msg info
			return 1;
		}
	}
		
	return 0;
}



/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// call a void function that returns a void
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_android_func_call_void_return_void (lua_State *l,char *funcname)
{
	JNIEnv *env=0;
	ANativeActivity *activity = master_android_app->activity;
	(*activity->vm)->AttachCurrentThread(activity->vm, &env, 0);	
	jclass clazz = (*env)->GetObjectClass(env, activity->clazz);

	jmethodID methodID = (*env)->GetMethodID(env, clazz, funcname, "()V");
	
	(*env)->CallVoidMethod(env, activity->clazz, methodID);

	(*activity->vm)->DetachCurrentThread(activity->vm);
	return 1;
}
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// call a void function that returns a string (pushed on lua stack)
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_android_func_call_void_return_string (lua_State *l,char *funcname)
{
	JNIEnv *env=0;
	ANativeActivity *activity = master_android_app->activity;
	(*activity->vm)->AttachCurrentThread(activity->vm, &env, 0);	
	jclass clazz = (*env)->GetObjectClass(env, activity->clazz);

	jmethodID methodID = (*env)->GetMethodID(env, clazz, funcname, "()Ljava/lang/String;");
	
	jobject str_result = (*env)->CallObjectMethod(env, activity->clazz, methodID);
	jboolean isCopy;
	const char *str= (*env)->GetStringUTFChars(env, (jstring)str_result, &isCopy);

	lua_pushstring(l,str);

	(*env)->ReleaseStringUTFChars(env, (jstring)str_result, str);
	(*activity->vm)->DetachCurrentThread(activity->vm);
	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// call a string function that returns a void
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_android_func_call_string_return_void (lua_State *l,char *funcname,const char *str)
{
	JNIEnv *env=0;
	ANativeActivity *activity = master_android_app->activity;
		
	(*activity->vm)->AttachCurrentThread(activity->vm, &env, 0);	
	
	jclass clazz = (*env)->GetObjectClass(env, activity->clazz);

	jmethodID methodID = (*env)->GetMethodID(env, clazz, funcname, "(Ljava/lang/String;)V");
	
	(*env)->CallVoidMethod(env, activity->clazz, methodID, (*env)->NewStringUTF(env,str) );
	(*activity->vm)->DetachCurrentThread(activity->vm);
	
	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// moveTaskToBack,
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_android_task_to_back (lua_State *l)
{
	return lua_android_func_call_void_return_void (l,"TaskToBack");
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// send intent msg, EG post something to twitter, user gets to choose where it gets posted,
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_android_send_intent (lua_State *l)
{
	const char *s="";	
	s=(const char *)lua_tostring(l,1);
	
	return lua_android_func_call_string_return_void(l,"SendIntent",s);
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// get a full path of a place to store config/user files
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_android_get_files_prefix (lua_State *l)
{
	return lua_android_func_call_void_return_string (l,"GetFilesPrefix");
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// get a full path of a place to store cache files
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_android_get_cache_prefix (lua_State *l)
{
	return lua_android_func_call_void_return_string (l,"GetCachePrefix");
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// open library.
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
LUALIB_API int luaopen_wetgenes_win_android_core(lua_State *l)
{
	const luaL_reg lib[] =
	{

		{"create",			lua_android_create},
		{"start",			lua_android_start},
		{"stop",			lua_android_stop},
		{"destroy",			lua_android_destroy},
		{"info",			lua_android_info},

		{"context",			lua_android_context},
		{"swap",			lua_android_swap},

//		{"peek",			lua_android_peek},
//		{"wait",			lua_android_wait},
		{"msg",				lua_android_msg},

		{"sleep",			lua_android_sleep},

		{"print",			lua_android_print},
		{"time",			lua_android_time},
		
		

// special extra android abilities

		{"task_to_back",				lua_android_task_to_back},

		{"send_intent",					lua_android_send_intent},

		{"get_files_prefix",			lua_android_get_files_prefix},
		{"get_cache_prefix",			lua_android_get_cache_prefix},

		{0,0}
	};
	

 	const luaL_reg meta[] =
	{
		{"__gc",			lua_android_destroy},
		{0,0}
	};

	luaL_newmetatable(l, lua_android_ptr_name);
	luaL_openlib(l, NULL, meta, 0);
	lua_pop(l,1);

	lua_newtable(l);
	luaL_openlib(l, NULL, lib, 0);
	
	return 1;
}



