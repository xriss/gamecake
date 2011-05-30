// Copyright 2008 The Native Client Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can
// be found in the LICENSE file.

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <nacl/nacl_npapi.h>

#include "lua.hpp"

struct NPO_DATA {
  NPClass *_class;
  uint32_t referenceCount;
  /*
   * Additional space may be allocated here by types of NPObjects
   */
	lua_State *L;
};


// handle all the basic lua gubbins

static void lstop (lua_State *L, lua_Debug *ar) {
  (void)ar;  /* unused arg. */
  lua_sethook(L, NULL, 0, 0);
  luaL_error(L, "interrupted!");
}


static void laction (int i) {
//  signal(i, SIG_DFL); /* if another SIGINT happens before lstop,
//                              terminate process (default action) */
//  lua_sethook(globalL, lstop, LUA_MASKCALL | LUA_MASKRET | LUA_MASKCOUNT, 1);
}



static void l_message (const char *pname, const char *msg) {
//  if (pname) fprintf(stderr, "%s: ", pname);
 // fprintf(stderr, "%s\n", msg);
 // fflush(stderr);
}


static int report (lua_State *L, int status) {
  if (status && !lua_isnil(L, -1)) {
    const char *msg = lua_tostring(L, -1);
    if (msg == NULL) msg = "(error object is not a string)";
    l_message("progname", msg);
    lua_pop(L, 1);
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



/*
 * static int dofile (lua_State *L, const char *name) {
  int status = luaL_loadfile(L, name) || docall(L, 0, 1);
  return report(L, status);
}
*/


static int dostring (lua_State *L, const char *s, const char *name) {
  int status = luaL_loadbuffer(L, s, strlen(s), name) || docall(L, 0, 1);
  return report(L, status);
}


static int dolibrary (lua_State *L, const char *name) {
  lua_getglobal(L, "require");
  lua_pushstring(L, name);
  return report(L, docall(L, 1, 1));
}








// These are the method names as JavaScript sees them.
static const char* kHelloWorldMethodId = "helloworld";
static const char* kFortyTwoMethodId = "fortytwo";

// This is the module's function that does the work to set the value of the
// result variable to '42'.  The Invoke() function that called this function
// then returns the result back to the browser as a JavaScript value.
static bool FortyTwo(lua_State *L  , NPVariant *result) {
  if (result) {

//  lua_State *L = lua_open();  /* create state */
/*
    if (L == NULL) {
    l_message(argv[0], "cannot create state: not enough memory");
    return EXIT_FAILURE;
  }
*/
//  s.argc = argc;
 // s.argv = argv;
//  status = lua_cpcall(L, &pmain, &s);
//  report(L, status);

//const char *s=" aa=(aa or "")..'+' ; return 'this is a lie'..aa ";
const char *s="aa=(aa or '')..'++';return 'this is a lie'..aa";

  int status = luaL_loadbuffer(L, s, strlen(s), "test") ;
  lua_call(L, 0, 1);

//    INT32_TO_NPVARIANT(status, *result);

    const char *msg = "hello, world.";
    
    msg=lua_tostring(L,1);
    lua_pop(L,1);
    
    const int msg_length = strlen(msg) + 1;
    // Note: |msg_copy| will be freed later on by the browser, so it needs to
    // be allocated here with NPN_MemAlloc().
    char *msg_copy = reinterpret_cast<char*>(NPN_MemAlloc(msg_length));
    strncpy(msg_copy, msg, msg_length);
    STRINGN_TO_NPVARIANT(msg_copy, msg_length - 1, *result);

  }
  return true;
}

// This function creates a string in the browser's memory pool and then returns
// a variable containing a pointer to that string.  The variable is later
// returned back to the browser by the Invoke() function that called this.
static bool HelloWorld(NPVariant *result) {
  if (result) {
    const char *msg = "hello, world.";
    const int msg_length = strlen(msg) + 1;
    // Note: |msg_copy| will be freed later on by the browser, so it needs to
    // be allocated here with NPN_MemAlloc().
    char *msg_copy = reinterpret_cast<char*>(NPN_MemAlloc(msg_length));
    strncpy(msg_copy, msg, msg_length);
    STRINGN_TO_NPVARIANT(msg_copy, msg_length - 1, *result);
  }
  return true;
}

// Creates the plugin-side instance of NPObject.
// Called by NPN_CreateObject, declared in npruntime.h
// Documentation URL: https://developer.mozilla.org/en/NPClass
static NPObject* Allocate(NPP npp, NPClass* npclass) {
	struct NPO_DATA *NPD=new struct NPO_DATA;
	NPD->_class=npclass;
	NPD->L=lua_open();
  return (NPObject*)NPD;
}

// Cleans up the plugin-side instance of an NPObject.
// Called by NPN_ReleaseObject, declared in npruntime.h
// Documentation URL: https://developer.mozilla.org/en/NPClass
static void Deallocate(NPObject* object) {
	struct NPO_DATA *NPD=(struct NPO_DATA *)object;
	  lua_close(NPD->L);

  delete NPD;
}

// Returns |true| if |method_name| is a recognized method.
// Called by NPN_HasMethod, declared in npruntime.h
// Documentation URL: https://developer.mozilla.org/en/NPClass
static bool HasMethod(NPObject* obj, NPIdentifier method_name) {
  char *name = NPN_UTF8FromIdentifier(method_name);
  bool is_method = false;
  if (!strcmp((const char *)name, kHelloWorldMethodId)) {
    is_method = true;
  } else if (!strcmp((const char*)name, kFortyTwoMethodId)) {
    is_method = true;
  }
  NPN_MemFree(name);
  return is_method;
}

// Called by the browser to invoke the default method on an NPObject.
// Returns null.
// Apparently the plugin won't load properly if we simply
// tell the browser we don't have this method.
// Called by NPN_InvokeDefault, declared in npruntime.h
// Documentation URL: https://developer.mozilla.org/en/NPClass
static bool InvokeDefault(NPObject *obj, const NPVariant *args,
                          uint32_t argCount, NPVariant *result) {
  if (result) {
    NULL_TO_NPVARIANT(*result);
  }
  return true;
}

// Called by the browser to invoke a function object whose name
// is |method_name|.
// Called by NPN_Invoke, declared in npruntime.h
// Documentation URL: https://developer.mozilla.org/en/NPClass
static bool Invoke(NPObject* obj,
                   NPIdentifier method_name,
                   const NPVariant *args,
                   uint32_t arg_count,
                   NPVariant *result) {
  NULL_TO_NPVARIANT(*result);
  char *name = NPN_UTF8FromIdentifier(method_name);
  if (name == NULL)
    return false;
  bool rval = false;

	struct NPO_DATA *NPD=(struct NPO_DATA *)obj;

  // Map the method name to a function call.  |result| is filled in by the
  // called function, then gets returned to the browser when Invoke() returns.
  if (!strcmp((const char *)name, kHelloWorldMethodId)) {
    rval = HelloWorld(result);
  } else if (!strcmp((const char*)name, kFortyTwoMethodId)) {
    rval = FortyTwo(NPD->L,result);
  }
  // Since name was allocated above by NPN_UTF8FromIdentifier,
  // it needs to be freed here.
  NPN_MemFree(name);
  return rval;
}

// Represents a class's interface, so that the browser knows what functions it
// can call on this plugin object.  The browser can use the methods in this
// class to discover the rest of the plugin's interface.
// Documentation URL: https://developer.mozilla.org/en/NPClass
static NPClass kHelloWorldClass = {
  NP_CLASS_STRUCT_VERSION,
  Allocate,
  Deallocate,
  NULL,  // Invalidate is not implemented
  HasMethod,
  Invoke,
  InvokeDefault,
  NULL,  // HasProperty is not implemented
  NULL,  // GetProperty is not implemented
  NULL,  // SetProperty is not implemented
};

// Called by NPP_GetScriptableInstance to get the scripting interface for
// this plugin.
NPClass *GetNPSimpleClass() {
  return &kHelloWorldClass;
}
