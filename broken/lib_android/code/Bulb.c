
#include <jni.h>
#include <sys/time.h>
#include <time.h>
#include <android/log.h>
#include <stdint.h>
#include <string.h>

#include "com_wetgenes_Bulb_Bulb.h"

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#include "GLES/gl.h"
#include "GLES/glext.h"


static lua_State *L=0; // keep a global lua state here



static void llog (const char *msg) {
//  if (pname) fprintf(stderr, "%s: ", pname);
//  fprintf(stderr, "%s\n", msg);
//  fflush(stderr);
  __android_log_print(ANDROID_LOG_INFO, "Bulb", msg);
}


static int report (lua_State *L, int status) {
  if (status && !lua_isnil(L, -1)) {
    const char *msg = lua_tostring(L, -1);
    if (msg == NULL) msg = "(error object is not a string)";
    llog(msg);
    lua_pop(L, 1);
    exit(20);
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




JNIEXPORT void JNICALL
	Java_com_wetgenes_Bulb_Bulb_setup(
		JNIEnv*  env,
		jclass class,
		jstring apk_wank
	)
{
	if(L!=0) { llog("Bulb.setup FAIL"); return; } // already setup and clean has not been called yet
	
llog("Bulb.setup");

	jboolean wank;
	const char *apk=(*env)->GetStringUTFChars(env,apk_wank,&wank);


	L = lua_open();  /* create state */
	luaL_openlibs(L);  /* open libraries */


	lua_getglobal(L,"require");
	lua_pushstring(L,"bulbtest");	
	report(L, docall(L, 1, 0) );
	lua_setglobal(L,"bulbtest");

	lua_getglobal(L,"bulbtest");
	lua_getfield(L,-1,"setup");
	lua_pushstring(L,apk);
	report(L, docall(L, 1, 1) );
	lua_pop(L,1);

	(*env)->ReleaseStringUTFChars(env, apk_wank, apk);
}

JNIEXPORT void JNICALL
	Java_com_wetgenes_Bulb_Bulb_start(
		JNIEnv*  env,
		jclass class
	)
{
	if(L==0) { llog("Bulb.start FAIL"); return; }
llog("Bulb.start");
	lua_getglobal(L,"bulbtest");
	lua_getfield(L,-1,"start");
	report(L, docall(L, 0, 1) );
	lua_pop(L,1);
}

JNIEXPORT void JNICALL
	Java_com_wetgenes_Bulb_Bulb_stop(
		JNIEnv*  env,
		jclass class
	)
{
	if(L==0) { llog("Bulb.stop FAIL"); return; }
llog("Bulb.stop");
	lua_getglobal(L,"bulbtest");
	lua_getfield(L,-1,"stop");
	report(L, docall(L, 0, 1) );
	lua_pop(L,1);
}

JNIEXPORT void JNICALL
	Java_com_wetgenes_Bulb_Bulb_clean(
		JNIEnv*  env,
		jclass class
	)
{
	if(L==0) { llog("Bulb.clean FAIL"); return; }
llog("Bulb.clean");

	lua_getglobal(L,"bulbtest");
	lua_getfield(L,-1,"clean");
	report(L, docall(L, 0, 1) );
	lua_pop(L,1);
	
	lua_close(L);
	L=0;
}


JNIEXPORT void JNICALL
	Java_com_wetgenes_Bulb_Bulb_updatedraw(
		JNIEnv*  env,
		jclass class
	)
{
	if(L==0) { llog("Bulb.updatedraw FAIL"); return; }
	
	lua_getglobal(L,"bulbtest");
	lua_getfield(L,-1,"updatedraw");
	report(L, docall(L, 0, 1) );
	lua_pop(L,1);
}

JNIEXPORT void JNICALL
	Java_com_wetgenes_Bulb_Bulb_resize(
		JNIEnv *env,
		jclass class,
		jint width,
		jint height)
{
	if(L==0) { llog("Bulb.resize FAIL"); return; }
	
	lua_getglobal(L,"bulbtest");
	lua_getfield(L,-1,"resize");
	lua_pushnumber(L,width);
	lua_pushnumber(L,height);	
	report(L, docall(L, 2, 1) );
	lua_pop(L,1);
}

JNIEXPORT void JNICALL
	Java_com_wetgenes_Bulb_Bulb_draw(
		JNIEnv*  env,
		jclass class
		)
{
	if(L==0) { llog("Bulb.draw FAIL"); return; }
	
	lua_getglobal(L,"bulbtest");
	lua_getfield(L,-1,"draw");
	report(L, docall(L, 0, 1) );
	lua_pop(L,1);
}


JNIEXPORT void JNICALL
	Java_com_wetgenes_Bulb_Bulb_press(
		JNIEnv *env,
		jclass class,
		jint ascii,
		jint key,
		jint act)

{
	if(L==0) { llog("Bulb.press FAIL"); return; }
//    llog("press");

	lua_getglobal(L, "bulbtest");
	lua_getfield(L, -1, "press");
	
	lua_pushnumber(L,ascii);
	lua_pushnumber(L,key);
	lua_pushnumber(L,act);
	
	report(L, docall(L, 3, 1) );
	
	lua_pop(L,1); //remove junk from stack
}

JNIEXPORT void JNICALL
	Java_com_wetgenes_Bulb_Bulb_touch(
		JNIEnv *env,
		jclass class,
		jfloat x,
		jfloat y,
		jint key,
		jint act)

{
	if(L==0) { llog("Bulb.touch FAIL"); return; }
 //   llog("touch");

	lua_getglobal(L, "bulbtest");
	lua_getfield(L, -1, "touch");
	
	lua_pushnumber(L,x);
	lua_pushnumber(L,y);
	lua_pushnumber(L,key);
	lua_pushnumber(L,act);
	
	report(L, docall(L, 4, 1) );
	
	lua_pop(L,1); //remove junk from stack
}

JNIEXPORT jint JNICALL
	JNI_OnLoad(
		JavaVM *vm,
		void *reserved
	)
{
    llog("Booting Bulb...");
    print_version();
    
	return JNI_VERSION_1_6;
}
