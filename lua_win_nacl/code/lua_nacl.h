
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <sys/time.h>

#include "ppapi/c/pp_errors.h"
#include "ppapi/c/pp_module.h"
#include "ppapi/c/pp_var.h"
#include "ppapi/c/pp_point.h"
#include "ppapi/c/pp_completion_callback.h"

#include "ppapi/c/ppb.h"
#include "ppapi/c/ppb_core.h"
#include "ppapi/c/ppb_view.h"
#include "ppapi/c/ppb_input_event.h"
#include "ppapi/c/ppb_url_loader.h"
#include "ppapi/c/ppb_url_request_info.h"
#include "ppapi/c/ppb_url_response_info.h"
#include "ppapi/c/ppb_instance.h"
#include "ppapi/c/ppb_messaging.h"
#include "ppapi/c/ppb_var.h"
#include "ppapi/c/ppb_graphics_3d.h"

#include "ppapi/c/ppp.h"
#include "ppapi/c/ppp_instance.h"
#include "ppapi/c/ppp_messaging.h"
#include "ppapi/c/ppp_input_event.h"

#include "ppapi/gles2/gl2ext_ppapi.h"
#include "GLES2/gl2.h"

#include "lua.h"
#include "lauxlib.h"


//
// Generic callback state
//
struct lua_nacl_callback
{
	struct PP_CompletionCallback pp[1];
	lua_State *l;
	PP_Resource r;
	
	void *ret; // return this memory if not 0
	int ret_size;
	int ret_prog;
	
	char state;
};

void *lua_nacl_mem_alloc(lua_State *l,int size);
void lua_nacl_mem_deref(lua_State *l,void *mem);
void lua_nacl_mem_push(lua_State *l,void *mem);

void lua_nacl_callback_free (lua_State *l, struct lua_nacl_callback * cb);
void lua_nacl_callback_func(void* user_data, int32_t result);
struct lua_nacl_callback * lua_nacl_callback_alloc (lua_State *l,int func);


