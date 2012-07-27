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

//#include "GLES2/gl2.h"
//#include "GLES2/gl2ext.h"


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
		
		lua_getglobal(L,"mainstate");
		if(lua_istable(L,-1)) // check we are setup
		{
			lua_getfield(L,-1,"android_msg");
			lua_newtable(L);
			lua_pushnumber(L,type); lua_setfield(L,-2,"type");
			lua_pushnumber(L,eventtime/1000000000.0); lua_setfield(L,-2,"eventtime");
			lua_pushnumber(L,keycode); lua_setfield(L,-2,"keycode");
			lua_pushnumber(L,action); lua_setfield(L,-2,"action");
			lua_pushnumber(L,flags); lua_setfield(L,-2,"flags");
			report(L, docall(L, 1, 0) );
		}
		lua_settop(L,0);
		return 1;
	}
	else
    if (type == AINPUT_EVENT_TYPE_MOTION)
    {
		eventtime=AMotionEvent_getEventTime(event);
		pointercount=AMotionEvent_getPointerCount(event);
		action=AMotionEvent_getAction(event);
		flags=AMotionEvent_getFlags(event);
		
		lua_getglobal(L,"mainstate");
		if(lua_istable(L,-1)) // check we are setup
		{
			lua_getfield(L,-1,"android_msg");
			lua_newtable(L);
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
			
			report(L, docall(L, 1, 0) );
		}
		lua_settop(L,0);
		return 1;
		
//        engine->animating = 1;
 //       engine->state.x = AMotionEvent_getX(event, 0);
 //       engine->state.y = AMotionEvent_getY(event, 0);
 //       return 1;
    }
    return 0;
}

static void engine_handle_cmd(struct android_app* app, int32_t cmd) {
    struct engine* engine = (struct engine*)app->userData;
	lua_State *L=engine->L;

    switch (cmd) {
        case APP_CMD_SAVE_STATE:
/*            // The system has asked us to save our current state.  Do so.
            engine->app->savedState = malloc(sizeof(struct saved_state));
            *((struct saved_state*)engine->app->savedState) = engine->state;
            engine->app->savedStateSize = sizeof(struct saved_state);
*/            break;
        case APP_CMD_INIT_WINDOW:
            // The window is being shown, get it ready.
            if (engine->app->window != NULL) {

	lua_getglobal(L,"require");
	lua_pushstring(L,"mainstate");	
	report(L, docall(L, 1, 0) );
	lua_setglobal(L,"mainstate"); // for faster access
	
	lua_getglobal(L,"mainstate");
	lua_getfield(L,-1,"android_setup");
	report(L, docall(L, 0, 0) );

	lua_settop(L,0); // remove all the junk we just built up


//                engine_init_display(engine);
//                engine_draw_frame(engine);
            }
            break;
        case APP_CMD_TERM_WINDOW:
/*            // The window is being hidden or closed, clean it up.
            engine_term_display(engine);
*/
            break;
        case APP_CMD_START:
            break;
        case APP_CMD_GAINED_FOCUS:
        
	lua_getglobal(L,"mainstate");
	lua_getfield(L,-1,"android_start");
	report(L, docall(L, 0, 0) );
	lua_settop(L,0);


/*            // When our app gains focus, we start monitoring the accelerometer.
            if (engine->accelerometerSensor != NULL) {
                ASensorEventQueue_enableSensor(engine->sensorEventQueue,
                        engine->accelerometerSensor);
                // We'd like to get 60 events per second (in us).
                ASensorEventQueue_setEventRate(engine->sensorEventQueue,
                        engine->accelerometerSensor, (1000L/60)*1000);
            }
*/
            break;
        case APP_CMD_STOP:
            break;
        case APP_CMD_LOST_FOCUS:

	lua_getglobal(L,"mainstate");
	lua_getfield(L,-1,"android_stop");
	report(L, docall(L, 0, 0) );
	lua_settop(L,0);
	
/*            // When our app loses focus, we stop monitoring the accelerometer.
            // This is to avoid consuming battery while not being used.
            if (engine->accelerometerSensor != NULL) {
                ASensorEventQueue_disableSensor(engine->sensorEventQueue,
                        engine->accelerometerSensor);
            }
            // Also stop animating.
            engine->animating = 0;
            engine_draw_frame(engine);
*/            break;
    }
}


struct android_app* master_android_app=0;

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
	
	lua_getfield(L,-1,"android_setup");
	lua_pushstring(L,apk);
	report(L, docall(L, 1, 0) );

	(*env)->ReleaseStringUTFChars(env, (jstring)apk_result, apk);
	(*activity->vm)->DetachCurrentThread(activity->vm);


	lua_settop(L,0); // remove all the junk we just built up


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
		// Check if we are exiting.
/*
		if (state->destroyRequested != 0) {
			engine_term_display(&engine);
			return;
		}
*/		
		lua_getglobal(L,"mainstate");
		if(lua_istable(L,-1)) // check we are setup
		{
			lua_getfield(L,-1,"android_serv");
			report(L, docall(L, 0, 0) );
		}
		lua_settop(L,0);

	}
}

