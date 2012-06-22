

/** @file nacl-gl-sample.c
 * This example demonstrates how to use the Graphics3D interface from C.
 */
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

void Messaging_HandleMessage(PP_Instance instance,  struct PP_Var var_message) {
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
