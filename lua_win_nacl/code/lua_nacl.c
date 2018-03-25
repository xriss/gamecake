
// this file handles the basic nacl interface setup
// the creation of the master lua state under nacl
// as well as the lua interface into nacl (wetgenes.nacl.core)

#include "lua_nacl.h"


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// Global master static state, not expecting more than one of these per page so lets keep it simple.
//
/*+-----------------------------------------------------------------------------------------------------------------+*/

static PP_Instance nacl_instance=0;
static PP_Resource nacl_context;

static PPB_Core* core_interface = NULL;

static PP_Module module_id = 0;
static PPB_Messaging* messaging_interface = NULL;
static PPB_Var* var_interface = NULL;
static PPB_Graphics3D* graphics3d_interface = NULL;
static PPB_Instance* instance_interface = NULL;

static PPB_View* view_interface = NULL;
static	int view_width=640;
static	int view_height=480;
	

static PPB_URLLoader* url_loader_interface = NULL;
static PPB_URLRequestInfo* url_request_info_interface = NULL;
static PPB_URLResponseInfo* url_response_info_interface = NULL;

static PPB_InputEvent* input_event_interface = NULL;
static PPB_KeyboardInputEvent* keyboard_input_event_interface = NULL;
static PPB_MouseInputEvent* mouse_input_event_interface = NULL;
static PPB_WheelInputEvent* wheel_input_event_interface = NULL;
static PPB_GetInterface get_browser_interface = NULL;

static lua_State *L=0;

PP_EXPORT int32_t PPP_InitializeModule(PP_Module a_module_id,
                                       PPB_GetInterface _get_browser_interface) {
										   
  get_browser_interface=_get_browser_interface;
  

  module_id = a_module_id;
//printf("core\n");
  core_interface = ( PPB_Core*)(get_browser_interface(PPB_CORE_INTERFACE));
//printf("var\n");
  var_interface = ( PPB_Var*)(get_browser_interface(PPB_VAR_INTERFACE));

//printf("view\n");
  view_interface = ( PPB_View*)(get_browser_interface(PPB_VIEW_INTERFACE));

//printf("urlloader\n");
  url_loader_interface = ( PPB_URLLoader*)(get_browser_interface(PPB_URLLOADER_INTERFACE));
//printf("request\n");
  url_request_info_interface = ( PPB_URLRequestInfo*)(get_browser_interface(PPB_URLREQUESTINFO_INTERFACE));
//printf("response\n");
  url_response_info_interface = ( PPB_URLResponseInfo*)(get_browser_interface(PPB_URLRESPONSEINFO_INTERFACE));

//printf("messaging\n");
  messaging_interface = ( PPB_Messaging*)(get_browser_interface(PPB_MESSAGING_INTERFACE));
//printf("graphics3d\n");
  graphics3d_interface = ( PPB_Graphics3D*)(get_browser_interface(PPB_GRAPHICS_3D_INTERFACE));
//printf("instance\n");
  instance_interface = ( PPB_Instance*)(get_browser_interface(PPB_INSTANCE_INTERFACE));

//printf("event\n");
  input_event_interface = ( PPB_InputEvent*)(get_browser_interface(PPB_INPUT_EVENT_INTERFACE));
  
//printf("keyboard\n");
  keyboard_input_event_interface = ( PPB_KeyboardInputEvent*)(get_browser_interface(PPB_KEYBOARD_INPUT_EVENT_INTERFACE));
//printf("mouse\n");
  mouse_input_event_interface = ( PPB_MouseInputEvent*)(get_browser_interface(PPB_MOUSE_INPUT_EVENT_INTERFACE));
//printf("wheel\n");
  wheel_input_event_interface = ( PPB_WheelInputEvent*)(get_browser_interface(PPB_WHEEL_INPUT_EVENT_INTERFACE));

//printf("glinit\n");
  if (!glInitializePPAPI(get_browser_interface)) {
    printf("glInitializePPAPI failed\n");
    return PP_ERROR_FAILED;
  }
  
  return PP_OK;
}


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


extern void alSetPpapiInfo(PP_Instance instance, PPB_GetInterface get_interface);
extern void luaL_openlibs (lua_State *L);
extern void lua_preloadlibs(lua_State *L);

static PP_Bool Instance_DidCreate(PP_Instance instance,
                                  uint32_t argc,
                                  const char* argn[],
                                  const char* argv[]) {

	nacl_instance=instance;

#if !defined(PEPPER)
	alSetPpapiInfo( nacl_instance , get_browser_interface );
// setup fake file system
	nacl_io_init_ppapi(nacl_instance,get_browser_interface);
	umount("/");
	mount("", "/", "memfs", 0, "");
//	mount("", "/dev", "dev", 0, "");

#endif
									  
	L = lua_open();  /* create state */
	luaL_openlibs(L);  /* open libraries */
	lua_preloadlibs(L); /* preload gamecake builtin libs */
	

	input_event_interface->RequestInputEvents(nacl_instance , PP_INPUTEVENT_CLASS_WHEEL | PP_INPUTEVENT_CLASS_MOUSE | PP_INPUTEVENT_CLASS_KEYBOARD );

  return PP_TRUE;
}

static void Instance_DidDestroy(PP_Instance instance) {
	
	lua_close(L);
	L=0;
	
}

static void Instance_DidChangeView(PP_Instance instance,
									PP_Resource view_resource)
{
struct PP_Rect clip;

	if(view_interface)
	{
		view_interface->GetRect(view_resource,&clip);

		view_width=clip.size.width;
		view_height=clip.size.height;

		if(nacl_context)
		{
			graphics3d_interface->ResizeBuffers(nacl_context, view_width, view_height);
		}
	}
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
  if(L)
  {
	struct PP_Var var_result;

	lua_pushfstring(L,"cmd=print\n%s",msg);
	var_result = CStrToVar( lua_tostring(L,-1) );
	lua_pop(L,1);

	messaging_interface->PostMessage(nacl_instance, var_result);

	var_interface->Release(var_result);
  }
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

PP_Bool Input_HandleInputEvent( PP_Instance instance , PP_Resource input_event )
{

	lua_getfield(L,LUA_GLOBALSINDEX,"nacl_input_event");
	if(lua_isfunction(L,-1))
	{
		if (mouse_input_event_interface->IsMouseInputEvent(input_event))
		{
			PP_InputEvent_Type 			t = input_event_interface->GetType(input_event);
			PP_InputEvent_MouseButton 	b = mouse_input_event_interface->GetButton(input_event);
			struct PP_Point				p = mouse_input_event_interface->GetPosition(input_event);
			int							c = mouse_input_event_interface->GetClickCount(input_event);
			
			lua_pushstring(L,"mouse");
			lua_pushnumber(L,t);
			lua_pushnumber(L,b);
			lua_pushnumber(L,p.x);
			lua_pushnumber(L,p.y);
			lua_pushnumber(L,c);
			lua_call(L,6,0);
			return PP_TRUE;
		}
		else
		if (wheel_input_event_interface->IsWheelInputEvent(input_event))
		{
			PP_InputEvent_Type 		t = input_event_interface->GetType(input_event);
			struct PP_FloatPoint	d = wheel_input_event_interface->GetDelta(input_event);
			
			lua_pushstring(L,"wheel");
			lua_pushnumber(L,t);
			lua_pushnumber(L,d.x);
			lua_pushnumber(L,d.y);
			lua_call(L,4,0);
			return PP_TRUE;
		}
		else
		if( keyboard_input_event_interface->IsKeyboardInputEvent(input_event) )
		{
			PP_InputEvent_Type 			t = input_event_interface->GetType(input_event);

			int 			k = keyboard_input_event_interface->GetKeyCode(input_event);
			struct PP_Var	v = keyboard_input_event_interface->GetCharacterText(input_event);
			int 			sl=0;
			const char* 	s = var_interface->VarToUtf8(v, (uint32_t*)&sl);

			lua_pushstring(L,"key");
			lua_pushnumber(L,t);
			lua_pushnumber(L,k);
			lua_pushlstring(L,s,sl);
			lua_call(L,4,0);
			return PP_TRUE;
		}
	}
	else
	{
		lua_pop(L,1);
	}
	return PP_FALSE;
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
  int slen=strlen("cmd=lua\n");
  if (strncmp(message, "cmd=lua\n", slen) == 0)
  {
		int top=lua_gettop(L);
		
		dostringr(L,message+slen,message+slen);
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
  else
  {
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
  } else if (strcmp(interface_name, PPP_INPUT_EVENT_INTERFACE) == 0) {
    static  PPP_InputEvent input_interface = {
      &Input_HandleInputEvent
    };
    return &input_interface;
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
									  
  int32_t attribs[] = {
//        PP_GRAPHICS3DATTRIB_ALPHA_SIZE,     8,
        PP_GRAPHICS3DATTRIB_DEPTH_SIZE,     24,
//        PP_GRAPHICS3DATTRIB_STENCIL_SIZE,   8,
//        PP_GRAPHICS3DATTRIB_SAMPLES,      0,
//        PP_GRAPHICS3DATTRIB_SAMPLE_BUFFERS,   0,
		PP_GRAPHICS3DATTRIB_WIDTH, view_width,
		PP_GRAPHICS3DATTRIB_HEIGHT, view_height,
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
static float ffffff=0.0f;
int lua_nacl_swap (lua_State *l)
{
/*
ffffff=ffffff+0.01f;
if(ffffff>1.0f) { ffffff=0.0f; }
glSetCurrentContextPPAPI(nacl_context);
glClearColor(ffffff,ffffff, 1.0f-ffffff, 1.0f);
glClear(GL_COLOR_BUFFER_BIT);
//return 1;
*/


 
	struct lua_nacl_callback * cb=lua_nacl_callback_alloc(l,1); // arg is a callback function

	int32_t ret = graphics3d_interface->SwapBuffers(nacl_context, *cb->pp);

	if( ret != PP_OK_COMPLETIONPENDING )
	{
		lua_nacl_callback_free(l,cb); // it will never be called so free it
	}

	lua_pushnumber(l,ret);
	return 1;

}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// print sends a msg to the javascript side "cmd=print\n" and the rest of the string is what to print
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_nacl_print(lua_State *l)
{
struct PP_Var var_result;

	lua_pushfstring(l,"cmd=print\n%s",lua_tostring(l,1));
	var_result = CStrToVar( lua_tostring(l,-1) );

	messaging_interface->PostMessage(nacl_instance, var_result);

	var_interface->Release(var_result);
	
	lua_pop(l,1);
	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// post sends a msg to the javascript side (need javascript side code to handle it)
// I'm using a cmd=%s\n header, directly followed by custom data depending on the command
// the idea is this is url encoded args on first line and cmd=blah is just the normal use
// also remember js strings are 16bit not 8bit, so, you know, that is a bit shit.
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_nacl_js_post(lua_State *l)
{
struct PP_Var var_result;

	var_result = CStrToVar( lua_tostring(l,1) );

	messaging_interface->PostMessage(nacl_instance, var_result);

	var_interface->Release(var_result);

	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// very basic but simple, get of data from a URL, needs a callback as it is async
//
/*+-----------------------------------------------------------------------------------------------------------------+*/

void lua_nacl_getURL_callback(void* user_data, int32_t result)
{
struct lua_nacl_callback *cb=(struct lua_nacl_callback *)user_data;
lua_State *l=cb->l; // use this lua state

PP_Resource resp=0;

int64_t prog = 0;
int64_t size = 0;

int finished=0;

	cb->result=result;
	
	url_loader_interface->GetDownloadProgress(cb->r,&prog,&size);
	
	if(cb->state=='h')
	{
		if( size>0 )
		{
			cb->state='l';
			
			cb->ret=lua_nacl_mem_alloc(l,size);
			cb->ret_size=size;
			cb->ret_prog=0;

			url_loader_interface->ReadResponseBody(cb->r,cb->ret,cb->ret_size,*cb->pp);
			lua_nacl_callback_func(user_data,0);
			return;
		}
	}
	else
	if(cb->state=='l')
	{
		if(result>0)
		{
			cb->ret_prog+=result; 
			
			if( cb->ret_prog >= cb->ret_size)
			{
				lua_nacl_callback_func(user_data,0);
				lua_nacl_callback_free(l,cb); // and free it
				return;
			}
			else // keep loading
			{
				url_loader_interface->ReadResponseBody(cb->r,((char*)(cb->ret))+cb->ret_prog,cb->ret_size-cb->ret_prog,*cb->pp);
				lua_nacl_callback_func(user_data,0);
				return;
			}
		}
	}
	
	lua_nacl_callback_func(user_data,0);
	lua_nacl_callback_free(l,cb); // and free it
	return;
}


int lua_nacl_getURL(lua_State *l)
{
struct lua_nacl_callback * cb;
struct PP_Var v;
const char * s=0;
int ret=0;

struct PP_CompletionCallback callback = { lua_nacl_getURL_callback, NULL, PP_COMPLETIONCALLBACK_FLAG_NONE };

	PP_Resource load=0;
	PP_Resource req=0;

	load=url_loader_interface->Create(nacl_instance);
	req=url_request_info_interface->Create(nacl_instance);
	
	if((load==0)||(req==0))
	{
		lua_pushfstring(l,"getURL creation failed\n");
		lua_error(l);
	}
	
	s=lua_tostring(l,1); // what to get
	v=CStrToVar( s );
	url_request_info_interface->SetProperty(req,PP_URLREQUESTPROPERTY_URL,v);
	url_request_info_interface->SetProperty(req,PP_URLREQUESTPROPERTY_RECORDDOWNLOADPROGRESS,PP_MakeBool(1));
	url_request_info_interface->SetProperty(req,PP_URLREQUESTPROPERTY_ALLOWCROSSORIGINREQUESTS,PP_MakeBool(1));
	
	cb=lua_nacl_callback_alloc(l,2); // arg 2 is a callback function
	cb->r=load;
	cb->pp->func=lua_nacl_getURL_callback;
	cb->state='h';
	cb->autofree=0;
	
	ret=url_loader_interface->Open(load,req,*cb->pp);
	
	lua_pushnumber(l,ret);
	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// What time is it?
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_nacl_time (lua_State *l)
{
	struct timeval tv ;
	gettimeofday ( & tv, NULL ) ;
	lua_pushnumber(l, (double) tv.tv_sec + (double) tv.tv_usec / 1000000.0 );
	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// schedual a callback
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_nacl_call (lua_State *l)
{
struct lua_nacl_callback * cb;
int ret=0;
int ms=0;
int val=0;

	ms=lua_tointeger(l,1);
	cb=lua_nacl_callback_alloc(l,2); // arg 2 is a callback function	
	val=lua_tointeger(l,3);

	core_interface->CallOnMainThread(ms,*cb->pp,val);

	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// What size is our view?
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_nacl_info (lua_State *l)
{

	lua_pushnumber(l,view_width);	lua_setfield(l,2,"width");
	lua_pushnumber(l,view_height);	lua_setfield(l,2,"height");
	
	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// open library.
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
LUALIB_API int luaopen_wetgenes_win_nacl_core(lua_State *l)
{
	const luaL_Reg lib[] =
	{
//		{"create",			lua_nacl_create},
		
		{	"context",						lua_nacl_context					},
		{	"swap",							lua_nacl_swap						},
		{	"time",							lua_nacl_time						},

		{	"js_post",						lua_nacl_js_post					},
		{	"print",						lua_nacl_print						},

		{	"info",							lua_nacl_info						},
		
		{	"call",							lua_nacl_call						},
		{	"getURL",						lua_nacl_getURL						},

//		{	"PPB_URLLoader_create",			lua_nacl_PPB_URLLoader_create		},
//		{	"PPB_URLRequestInfo_create",	lua_nacl_PPB_URLRequestInfo_create	},


		{0,0}
	};

	lua_newtable(l);
	luaL_openlib(l, NULL, lib, 0);
	
	return 1;
}


