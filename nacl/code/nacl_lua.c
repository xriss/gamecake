

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

static PP_Module module_id = 0;
static PPB_Messaging* messaging_interface = NULL;
static PPB_Var* var_interface = NULL;
static PPB_Graphics3D* graphics3d_interface = NULL;
static PPB_Instance* instance_interface = NULL;

struct MessageInfo {
  PP_Instance instance;
  struct PP_Var message;
};

static const char* const kReverseTextMethodId = "reverseText";
static const char* const kFortyTwoMethodId = "fortyTwo";
static const char kMessageArgumentSeparator = ':';
static const char kNullTerminator = '\0';

//static struct PPB_Messaging* ppb_messaging_interface = NULL;
//static struct PPB_Var* ppb_var_interface = NULL;
//static PP_Module module_id = 0;


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
      c_str[len] = kNullTerminator;
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

/**
 * Reverse C string in-place.
 * @param[in,out] str C string to be reversed
 */
static void ReverseStr(char* str) {
  char* right = str + strlen(str) - 1;
  char* left = str;
  while (left < right) {
    char tmp = *left;
    *left++ = *right;
    *right-- = tmp;
  }
}

/**
 * A simple function that always returns 42.
 * @return always returns the integer 42
 */
static struct PP_Var FortyTwo() {
  return PP_MakeInt32(42);
}



void swap_callback(void* user_data, int32_t result)
{
  printf("swap result: %d\n", result);
}

static PP_Bool Instance_DidCreate(PP_Instance instance,
                                  uint32_t argc,
                                  const char* argn[],
                                  const char* argv[]) {
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

  return PP_TRUE;
}

static void Instance_DidDestroy(PP_Instance instance) {
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

void Messaging_HandleMessage(PP_Instance instance, struct PP_Var var_message) {
  if (var_message.type != PP_VARTYPE_STRING) {
    /* Only handle string messages */
    return;
  }
  char* message = VarToCStr(var_message);
  if (message == NULL)
    return;
  struct PP_Var var_result = PP_MakeUndefined();
  if (strncmp(message, kFortyTwoMethodId, strlen(kFortyTwoMethodId)) == 0) {
    var_result = FortyTwo();
  } else if (strncmp(message,
                     kReverseTextMethodId,
                     strlen(kReverseTextMethodId)) == 0) {
    /* Use everything after the ':' in |message| as the string argument. */
    char* string_arg = strchr(message, kMessageArgumentSeparator);
    if (string_arg != NULL) {
      string_arg += 1;  /* Advance past the ':' separator. */
      ReverseStr(string_arg);
      var_result = CStrToVar(string_arg);
    }
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
