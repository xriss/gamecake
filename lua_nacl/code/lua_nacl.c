
// this file handles the basic nacl interface setup
// the creation of the master lua state under nacl
// as well as the lua interface into nacl (wetgenes.nacl.core)

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "ppapi/c/pp_errors.h"
#include "ppapi/c/pp_module.h"
#include "ppapi/c/pp_var.h"
#include "ppapi/c/ppb.h"
#include "ppapi/c/ppb_instance.h"
#include "ppapi/c/ppb_messaging.h"
#include "ppapi/c/ppb_var.h"
#include "ppapi/c/ppp.h"
#include "ppapi/c/ppp_instance.h"
#include "ppapi/c/ppp_messaging.h"
#include "ppapi/c/pp_completion_callback.h"
#include "ppapi/c/ppb_graphics_3d.h"
#include "ppapi/gles2/gl2ext_ppapi.h"
#include "GLES2/gl2.h"

#include "lua.h"
#include "lauxlib.h"

static PP_Instance nacl_instance=0;
static PP_Resource nacl_context;

static PP_Module module_id = 0;
static PPB_Messaging* messaging_interface = NULL;
static PPB_Var* var_interface = NULL;
static PPB_Graphics3D* graphics3d_interface = NULL;
static PPB_Instance* instance_interface = NULL;

static lua_State *L=0;


/**
 * Returns a mutable C string contained in the @a var or NULL if @a var is not
 * string.  This makes a copy of the string in the @ var and adds a NULL
 * terminator.  Note that VarToUtf8() does not guarantee the NULL terminator on
 * the returned string.  See the comments for VatToUtf8() in ppapi/c/ppb_var.h
 * for more info.  The caller is responsible for freeing the returned memory.
 * @param[in] var PP_Var containing string.
 * @return a C string representation of @a var.
 * @note The caller is responsible for freeing the returned string.
 */
static char* VarToCStr(struct PP_Var var) {
  uint32_t len = 0;
  if (var_interface != NULL) {
    const char* var_c_str = var_interface->VarToUtf8(var, &len);
    if (len > 0) {
      char* c_str = (char*)malloc(len + 1);
      memcpy(c_str, var_c_str, len);
      c_str[len] = 0;
      return c_str;
    }
  }
  return NULL;
}

/**
 * Creates new string PP_Var from C string. The resulting object will be a
 * refcounted string object. It will be AddRef()ed for the caller. When the
 * caller is done with it, it should be Release()d.
 * @param[in] str C string to be converted to PP_Var
 * @return PP_Var containing string.
 */
static struct PP_Var CStrToVar(const char* str) {
  if (var_interface != NULL) {
    return var_interface->VarFromUtf8(/*module_id,*/ str, strlen(str));
  }
  return PP_MakeUndefined();
}




void swap_callback(void* user_data, int32_t result)
{
  printf("swap result: %d\n", result);
}

static PP_Bool Instance_DidCreate(PP_Instance instance,
                                  uint32_t argc,
                                  const char* argn[],
                                  const char* argv[]) {

nacl_instance=instance;
									  
	L = lua_open();  /* create state */
	luaL_openlibs(L);  /* open libraries */
									  
									  
									  
/*									  
  PP_Resource context;
  int32_t attribs[] = {PP_GRAPHICS3DATTRIB_WIDTH, 640,
                       PP_GRAPHICS3DATTRIB_HEIGHT, 480,
                       PP_GRAPHICS3DATTRIB_NONE};
  context = graphics3d_interface->Create(instance, 0, attribs);
  if (context == 0) {
    printf("failed to create graphics3d context\n");
    return PP_FALSE;
  }

  glSetCurrentContextPPAPI(context);

  if (!instance_interface->BindGraphics(instance, context)) {
    printf("failed to bind graphics3d context\n");
    return PP_FALSE;
  }

  glClearColor(1.0f, 0.9f, 0.4f, 0.9f);
  glClear(GL_COLOR_BUFFER_BIT);

   struct PP_CompletionCallback callback = { swap_callback, NULL, PP_COMPLETIONCALLBACK_FLAG_NONE };
  int32_t ret = graphics3d_interface->SwapBuffers(context, callback);
  if (ret != PP_OK && ret != PP_OK_COMPLETIONPENDING) {
    printf("SwapBuffers failed with code %d\n", ret);
    return PP_FALSE;
  }
*/

  return PP_TRUE;
}

static void Instance_DidDestroy(PP_Instance instance) {
	
	lua_close(L);
	L=0;
	
}

static void Instance_DidChangeView(PP_Instance instance,
                                   const struct PP_Rect* position,
                                   const struct PP_Rect* clip) {
}

static void Instance_DidChangeFocus(PP_Instance instance,
                                    PP_Bool has_focus) {
}

static PP_Bool Instance_HandleDocumentLoad(PP_Instance instance,
                                           PP_Resource url_loader) {
  return PP_FALSE;
}

static void l_message (const char *pname, const char *msg) {
  if (pname) fprintf(stderr, "%s: ", pname);
  fprintf(stderr, "%s\n", msg);
  fflush(stderr);
}


static int report (lua_State *L, int status) {
  if (status && !lua_isnil(L, -1)) {
    const char *msg = lua_tostring(L, -1);
    if (msg == NULL) msg = "(error object is not a string)";
    l_message("", msg);
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

static int dostringr (lua_State *L, const char *s, const char *name) {
  int status = luaL_loadbuffer(L, s, strlen(s), name) || docall(L, 0, 0);
  return report(L, status);
}


void Messaging_HandleMessage(PP_Instance instance, struct PP_Var var_message) {

nacl_instance=instance;

  if (var_message.type != PP_VARTYPE_STRING) {
    /* Only handle string messages */
    return;
  }
  char* message = VarToCStr(var_message);
  if (message == NULL) { return; }
  
  struct PP_Var var_result = PP_MakeUndefined();
  if (strncmp(message, "lua\n", strlen("lua\n")) == 0)
  {
		int top=lua_gettop(L);
		
		dostringr(L,message+4,message+4);
		if(lua_isnumber(L,-1))
		{
			var_result = PP_MakeDouble( lua_tonumber(L,-1) );
		}
		else
		if(lua_isstring(L,-1))
		{
			var_result = CStrToVar( lua_tostring(L,-1) );
		}
		
		lua_settop(L,top);
  }
  free(message);

  /* Echo the return result back to browser.  Note that HandleMessage is always
   * called on the main thread, so it's OK to post the message back to the
   * browser directly from here.  This return post is asynchronous.
   */
  messaging_interface->PostMessage(instance, var_result);
  /* If the message was created using VarFromUtf8() it needs to be released.
   * See the comments about VarFromUtf8() in ppapi/c/ppb_var.h for more
   * information.
   */
  if (var_result.type == PP_VARTYPE_STRING) {
    var_interface->Release(var_result);
  }
}


PP_EXPORT int32_t PPP_InitializeModule(PP_Module a_module_id,
                                       PPB_GetInterface get_browser_interface) {
  module_id = a_module_id;
  var_interface = ( PPB_Var*)(get_browser_interface(PPB_VAR_INTERFACE));
  messaging_interface =
      ( PPB_Messaging*)(get_browser_interface(PPB_MESSAGING_INTERFACE));
  graphics3d_interface =
      ( PPB_Graphics3D*)(get_browser_interface(PPB_GRAPHICS_3D_INTERFACE));
  instance_interface =
      ( PPB_Instance*)(get_browser_interface(PPB_INSTANCE_INTERFACE));

  if (!glInitializePPAPI(get_browser_interface)) {
    printf("glInitializePPAPI failed\n");
    return PP_ERROR_FAILED;
  }

  return PP_OK;
}

PP_EXPORT const void* PPP_GetInterface(const char* interface_name) {
  if (strcmp(interface_name, PPP_INSTANCE_INTERFACE) == 0) {
    static  PPP_Instance instance_interface = {
      &Instance_DidCreate,
      &Instance_DidDestroy,
      &Instance_DidChangeView,
      &Instance_DidChangeFocus,
      &Instance_HandleDocumentLoad
    };
    return &instance_interface;
  } else if (strcmp(interface_name, PPP_MESSAGING_INTERFACE) == 0) {
    static  PPP_Messaging messaging_interface = {
      &Messaging_HandleMessage
    };
    return &messaging_interface;
  }
  return NULL;
}

PP_EXPORT void PPP_ShutdownModule() {
}



/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// prepare a gl surface in the window
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_nacl_context (lua_State *l)
{
									  
  int32_t attribs[] = {PP_GRAPHICS3DATTRIB_WIDTH, 640,
                       PP_GRAPHICS3DATTRIB_HEIGHT, 480,
                       PP_GRAPHICS3DATTRIB_NONE};
  nacl_context = graphics3d_interface->Create(nacl_instance, 0, attribs);
  if (nacl_context == 0)
  {
    lua_pushstring(l,"failed to create graphics3d context");
    lua_error(l);
  }

  glSetCurrentContextPPAPI(nacl_context);

  if (!instance_interface->BindGraphics(nacl_instance, nacl_context))
  {
    lua_pushstring(l,"failed to bind graphics3d context");
    lua_error(l);
  }


/*
 * wetwin_lua *p=lua_wetwin_check_ptr(l,1);

	int attrcount;
	int AttributeList[] = {
			GLX_RED_SIZE, 1,
			GLX_GREEN_SIZE, 1,
			GLX_BLUE_SIZE, 1,
			GLX_ALPHA_SIZE, 0,
			GLX_DEPTH_SIZE, 1,
			GLX_STENCIL_SIZE, 0,
			GLX_X_RENDERABLE,1,
			GLX_DOUBLEBUFFER,1,
			None};
			
	GLXFBConfig *conf=glXChooseFBConfig(p->dsp,p->screen,AttributeList,&attrcount);

	p->context=glXCreateNewContext( p->dsp , *conf , GLX_RGBA_TYPE , NULL , 1 );

	glXMakeContextCurrent( p->dsp , p->win , p->win, p->context );
*/
	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// swap a gl surface 
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_nacl_swap (lua_State *l)
{
//  glClearColor(1.0f, 0.9f, 0.4f, 0.9f);
//  glClear(GL_COLOR_BUFFER_BIT);

   struct PP_CompletionCallback callback = { swap_callback, NULL, PP_COMPLETIONCALLBACK_FLAG_NONE };
  int32_t ret = graphics3d_interface->SwapBuffers(nacl_context, callback);
  if (ret != PP_OK && ret != PP_OK_COMPLETIONPENDING) {
    lua_pushfstring(l,"SwapBuffers failed with code %d\n", ret);
    lua_error(l);
  }


/*
 * wetwin_lua *p=lua_wetwin_check_ptr(l,1);

	glXSwapBuffers( p->dsp, p->win );
*/
	return 0;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// open library.
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
LUALIB_API int luaopen_wetgenes_nacl_core(lua_State *l)
{
	const luaL_reg lib[] =
	{
//		{"create",			lua_nacl_create},
		
		{"context",			lua_nacl_context},
		{"swap",			lua_nacl_swap},
		
		{0,0}
	};

	lua_newtable(l);
	luaL_openlib(l, NULL, lib, 0);
	
	return 1;
}


