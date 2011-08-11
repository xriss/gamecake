/** \file
 * \brief iupgl control for X11
 *
 * See Copyright Notice in "iup.h"
 */

#include <X11/Xlib.h>
#include <GL/glx.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>

#include "iup.h"
#include "iupcbs.h"
#include "iupgl.h"

#include "iup_object.h"
#include "iup_attrib.h"
#include "iup_str.h"
#include "iup_stdcontrols.h"
#include "iup_assert.h"
#include "iup_register.h"


/* Do NOT use _IcontrolData to make inheritance easy
   when parent class in glcanvas */
typedef struct _IGlControlData
{
  Display* display;
  Drawable window;
  Colormap colormap;
  XVisualInfo *vinfo;
  GLXContext context;
} IGlControlData;

static int xGLCanvasDefaultResize(Ihandle *ih, int width, int height)
{
  IupGLMakeCurrent(ih);
  glViewport(0,0,width,height);
  return IUP_DEFAULT;
}

static int xGLCanvasCreateMethod(Ihandle* ih, void** params)
{
  IGlControlData* gldata;
  (void)params;

  gldata = (IGlControlData*)malloc(sizeof(IGlControlData));
  memset(gldata, 0, sizeof(IGlControlData));
  iupAttribSetStr(ih, "_IUP_GLCONTROLDATA", (char*)gldata);

  IupSetCallback(ih, "RESIZE_CB", (Icallback)xGLCanvasDefaultResize);

  return IUP_NOERROR;
}

static void xGLCanvasGetVisual(Ihandle* ih, IGlControlData* gldata)
{
  int erb, evb, number;
  int n = 0;
  int alist[40];

  if (!gldata->display)
    gldata->display = (Display*)IupGetGlobal("XDISPLAY");  /* works for Motif and GTK, can be called before mapped */
  if (!gldata->display)
    return;

  /* double or single buffer */
  if (iupStrEqualNoCase(iupAttribGetStr(ih,"BUFFER"), "DOUBLE"))
  {
    alist[n++] = GLX_DOUBLEBUFFER;
  }

  /* stereo */
  if (iupAttribGetBoolean(ih,"STEREO"))
  {
    alist[n++] = GLX_STEREO;
  }

  /* rgba or index */ 
  if (iupStrEqualNoCase(iupAttribGetStr(ih,"COLOR"), "INDEX"))
  {
    /* buffer size (for index mode) */
    number = iupAttribGetInt(ih,"BUFFER_SIZE");
    if (number > 0)
    {
      alist[n++] = GLX_BUFFER_SIZE;
      alist[n++] = number;
    }
  }
  else
  {
    alist[n++] = GLX_RGBA;      /* assume rgba as default */
  }

  /* red, green, blue bits */
  number = iupAttribGetInt(ih,"RED_SIZE");
  if (number > 0) 
  {
    alist[n++] = GLX_RED_SIZE;
    alist[n++] = number;
  }

  number = iupAttribGetInt(ih,"GREEN_SIZE");
  if (number > 0) 
  {
    alist[n++] = GLX_GREEN_SIZE;
    alist[n++] = number;
  }

  number = iupAttribGetInt(ih,"BLUE_SIZE");
  if (number > 0) 
  {
    alist[n++] = GLX_BLUE_SIZE;
    alist[n++] = number;
  }

  number = iupAttribGetInt(ih,"ALPHA_SIZE");
  if (number > 0) 
  {
    alist[n++] = GLX_ALPHA_SIZE;
    alist[n++] = number;
  }

  /* depth and stencil size */
  number = iupAttribGetInt(ih,"DEPTH_SIZE");
  if (number > 0) 
  {
    alist[n++] = GLX_DEPTH_SIZE;
    alist[n++] = number;
  }

  number = iupAttribGetInt(ih,"STENCIL_SIZE");
  if (number > 0) 
  {
    alist[n++] = GLX_STENCIL_SIZE;
    alist[n++] = number;
  }

  /* red, green, blue accumulation bits */
  number = iupAttribGetInt(ih,"ACCUM_RED_SIZE");
  if (number > 0) 
  {
    alist[n++] = GLX_ACCUM_RED_SIZE;
    alist[n++] = number;
  }

  number = iupAttribGetInt(ih,"ACCUM_GREEN_SIZE");
  if (number > 0) 
  {
    alist[n++] = GLX_ACCUM_GREEN_SIZE;
    alist[n++] = number;
  }

  number = iupAttribGetInt(ih,"ACCUM_BLUE_SIZE");
  if (number > 0) 
  {
    alist[n++] = GLX_ACCUM_BLUE_SIZE;
    alist[n++] = number;
  }

  number = iupAttribGetInt(ih,"ACCUM_ALPHA_SIZE");
  if (number > 0) 
  {
    alist[n++] = GLX_ACCUM_ALPHA_SIZE;
    alist[n++] = number;
  }
  alist[n++] = None;

  /* check out X extension */
  if (!glXQueryExtension(gldata->display, &erb, &evb))
  {
    iupAttribSetStr(ih, "ERROR", "X server has no OpenGL GLX extension");
    return;
  }

  /* choose visual */
  gldata->vinfo = glXChooseVisual(gldata->display, DefaultScreen(gldata->display), alist);
  if (!gldata->vinfo)
    iupAttribSetStr(ih, "ERROR", "No appropriate visual");
}

static char* xGLCanvasGetVisualAttrib(Ihandle *ih)
{
  IGlControlData* gldata = (IGlControlData*)iupAttribGet(ih, "_IUP_GLCONTROLDATA");

  /* This must be available before mapping, because IupCanvas uses it during map in GTK and Motif. */
  if (gldata->vinfo)
    return (char*)gldata->vinfo->visual;

  xGLCanvasGetVisual(ih, gldata);

  if (gldata->vinfo)
    return (char*)gldata->vinfo->visual;

  return NULL;
}

static int xGLCanvasMapMethod(Ihandle* ih)
{
  IGlControlData* gldata = (IGlControlData*)iupAttribGet(ih, "_IUP_GLCONTROLDATA");
  GLXContext shared_context = NULL;
  Ihandle* ih_shared;

  /* the IupCanvas is already mapped, just initialize the OpenGL context */

  if (!xGLCanvasGetVisualAttrib(ih))
    return IUP_NOERROR; /* do not abort mapping */

  gldata->window = (XID)iupAttribGet(ih, "XWINDOW"); /* check first in the hash table, can be defined by the IupFileDlg */
  if (!gldata->window)
    gldata->window = (XID)IupGetAttribute(ih, "XWINDOW");  /* works for Motif and GTK, only after mapping the IupCanvas */
  if (!gldata->window)
    return IUP_NOERROR;

  ih_shared = IupGetAttributeHandle(ih, "SHAREDCONTEXT");
  if (ih_shared && IupClassMatch(ih_shared, "glcanvas"))  /* must be an IupGLCanvas */
  {
    IGlControlData* shared_gldata = (IGlControlData*)iupAttribGet(ih_shared, "_IUP_GLCONTROLDATA");
    shared_context = shared_gldata->context;
  }

  /* create rendering context */
  gldata->context = glXCreateContext(gldata->display, gldata->vinfo, shared_context, GL_TRUE);
  if (!gldata->context)
  {
    iupAttribSetStr(ih, "ERROR", "Could not create a rendering context");
    return IUP_NOERROR;
  }
  iupAttribSetStr(ih, "CONTEXT", (char*)gldata->context);

  /* create colormap for index mode */
  if (iupStrEqualNoCase(iupAttribGetStr(ih,"COLOR"), "INDEX") && 
      gldata->vinfo->class != StaticColor && gldata->vinfo->class != StaticGray)
  {
    gldata->colormap = XCreateColormap(gldata->display, RootWindow(gldata->display, DefaultScreen(gldata->display)), gldata->vinfo->visual, AllocAll);
    iupAttribSetStr(ih, "COLORMAP", (char*)gldata->colormap);
  }

  if (gldata->colormap != None)
    IupGLPalette(ih,0,1,1,1);  /* set first color as white */

  return IUP_NOERROR;
}

static void xGLCanvasDestroy(Ihandle* ih)
{
  IGlControlData* gldata = (IGlControlData*)iupAttribGet(ih, "_IUP_GLCONTROLDATA");
  free(gldata);
  iupAttribSetStr(ih, "_IUP_GLCONTROLDATA", NULL);
}

static void xGLCanvasUnMapMethod(Ihandle* ih)
{
  IGlControlData* gldata = (IGlControlData*)iupAttribGet(ih, "_IUP_GLCONTROLDATA");

  if (gldata->context)
  {
    if (gldata->context == glXGetCurrentContext())
      glXMakeCurrent(gldata->display, None, NULL);

    glXDestroyContext(gldata->display, gldata->context);
  }

  if (gldata->colormap != None)
    XFreeColormap(gldata->display, gldata->colormap);

  if (gldata->vinfo)
    XFree(gldata->vinfo); 

  memset(gldata, 0, sizeof(IGlControlData));
}

static Iclass* xGlCanvasNewClass(void)
{
  Iclass* ic = iupClassNew(iupRegisterFindClass("canvas"));

  ic->name = "glcanvas";
  ic->format = "a"; /* one ACTION callback name */
  ic->nativetype = IUP_TYPECANVAS;
  ic->childtype = IUP_CHILDNONE;
  ic->is_interactive = 1;

  ic->New = xGlCanvasNewClass;
  ic->Create = xGLCanvasCreateMethod;
  ic->Destroy = xGLCanvasDestroy;
  ic->Map = xGLCanvasMapMethod;
  ic->UnMap = xGLCanvasUnMapMethod;

  iupClassRegisterAttribute(ic, "BUFFER", NULL, NULL, IUPAF_SAMEASSYSTEM, "SINGLE", IUPAF_DEFAULT);
  iupClassRegisterAttribute(ic, "COLOR", NULL, NULL, IUPAF_SAMEASSYSTEM, "RGBA", IUPAF_DEFAULT);

  iupClassRegisterAttribute(ic, "CONTEXT", NULL, NULL, NULL, NULL, IUPAF_READONLY|IUPAF_NO_STRING);
  iupClassRegisterAttribute(ic, "VISUAL", xGLCanvasGetVisualAttrib, NULL, NULL, NULL, IUPAF_READONLY|IUPAF_NO_STRING|IUPAF_NOT_MAPPED);
  iupClassRegisterAttribute(ic, "COLORMAP", NULL, NULL, NULL, NULL, IUPAF_READONLY|IUPAF_NO_STRING);

  return ic;
}


/******************************************* Exported functions */

void IupGLCanvasOpen(void)
{
  if (!IupGetGlobal("_IUP_GLCANVAS_OPEN"))
  {
    iupRegisterClass(xGlCanvasNewClass());
    IupSetGlobal("_IUP_GLCANVAS_OPEN", "1");
  }
}

Ihandle* IupGLCanvas(const char *action)
{
  void *params[2];
  params[0] = (void*)action;
  params[1] = NULL;
  return IupCreatev("glcanvas", params);
}

int IupGLIsCurrent(Ihandle* ih)
{
  IGlControlData* gldata;

  iupASSERT(iupObjectCheck(ih));
  if (!iupObjectCheck(ih))
    return 0;

  /* must be an IupGLCanvas */
  if (ih->iclass->nativetype != IUP_TYPECANVAS || 
      !IupClassMatch(ih, "glcanvas"))
    return 0;

  /* must be mapped */
  gldata = (IGlControlData*)iupAttribGet(ih, "_IUP_GLCONTROLDATA");
  if (!gldata->window)
    return 0;

  if (gldata->context == glXGetCurrentContext())
    return 1;

  return 0;
}

void IupGLMakeCurrent(Ihandle* ih)
{
  IGlControlData* gldata;

  iupASSERT(iupObjectCheck(ih));
  if (!iupObjectCheck(ih))
    return;

  /* must be an IupGLCanvas */
  if (ih->iclass->nativetype != IUP_TYPECANVAS || 
      !IupClassMatch(ih, "glcanvas"))
    return;

  /* must be mapped */
  gldata = (IGlControlData*)iupAttribGet(ih, "_IUP_GLCONTROLDATA");
  if (!gldata->window)
    return;

  glXMakeCurrent(gldata->display, gldata->window, gldata->context);
  glXWaitX();
}

void IupGLSwapBuffers(Ihandle* ih)
{
  IGlControlData* gldata;

  iupASSERT(iupObjectCheck(ih));
  if (!iupObjectCheck(ih))
    return;

  /* must be an IupGLCanvas */
  if (ih->iclass->nativetype != IUP_TYPECANVAS || 
      !IupClassMatch(ih, "glcanvas"))
    return;

  /* must be mapped */
  gldata = (IGlControlData*)iupAttribGet(ih, "_IUP_GLCONTROLDATA");
  if (!gldata->window)
    return;

  glXSwapBuffers(gldata->display, gldata->window);
}

static int xGLCanvasIgnoreError(Display *param1, XErrorEvent *param2)
{
  (void)param1;
  (void)param2;
  return 0;
}

void IupGLPalette(Ihandle* ih, int index, float r, float g, float b)
{
  IGlControlData* gldata;
  XColor color;
  int rShift, gShift, bShift;
  XVisualInfo *vinfo;
  XErrorHandler old_handler;

  iupASSERT(iupObjectCheck(ih));
  if (!iupObjectCheck(ih))
    return;

  /* must be an IupGLCanvas */
  if (ih->iclass->nativetype != IUP_TYPECANVAS || 
      !IupClassMatch(ih, "glcanvas"))
    return;

  /* must be mapped */
  gldata = (IGlControlData*)iupAttribGet(ih, "_IUP_GLCONTROLDATA");
  if (!gldata->window)
    return;

  /* must have a colormap */
  if (gldata->colormap == None)
    return;

  /* code fragment based on the toolkit library provided with OpenGL */
  old_handler = XSetErrorHandler(xGLCanvasIgnoreError);

  vinfo = gldata->vinfo;
  switch (vinfo->class) 
  {
  case DirectColor:
    rShift = ffs((unsigned int)vinfo->red_mask) - 1;
    gShift = ffs((unsigned int)vinfo->green_mask) - 1;
    bShift = ffs((unsigned int)vinfo->blue_mask) - 1;
    color.pixel = ((index << rShift) & vinfo->red_mask) |
                  ((index << gShift) & vinfo->green_mask) |
                  ((index << bShift) & vinfo->blue_mask);
    color.red = (unsigned short)(r * 65535.0 + 0.5);
    color.green = (unsigned short)(g * 65535.0 + 0.5);
    color.blue = (unsigned short)(b * 65535.0 + 0.5);
    color.flags = DoRed | DoGreen | DoBlue;
    XStoreColor(gldata->display, gldata->colormap, &color);
    break;
  case GrayScale:
  case PseudoColor:
    if (index < vinfo->colormap_size) 
    {
      color.pixel = index;
      color.red = (unsigned short)(r * 65535.0 + 0.5);
      color.green = (unsigned short)(g * 65535.0 + 0.5);
      color.blue = (unsigned short)(b * 65535.0 + 0.5);
      color.flags = DoRed | DoGreen | DoBlue;
      XStoreColor(gldata->display, gldata->colormap, &color);
    }
    break;
  }

  XSync(gldata->display, 0);
  XSetErrorHandler(old_handler);
}

void IupGLUseFont(Ihandle* ih, int first, int count, int list_base)
{
  IGlControlData* gldata;
  Font font;

  iupASSERT(iupObjectCheck(ih));
  if (!iupObjectCheck(ih))
    return;

  /* must be an IupGLCanvas */
  if (ih->iclass->nativetype != IUP_TYPECANVAS || 
      !IupClassMatch(ih, "glcanvas"))
    return;

  /* must be mapped */
  gldata = (IGlControlData*)iupAttribGet(ih, "_IUP_GLCONTROLDATA");
  if (!gldata->window)
    return;

  font = (Font)IupGetAttribute(ih, "XFONTID");
  if (font)
    glXUseXFont(font, first, count, list_base);
}

void IupGLWait(int gl)
{
  if (gl)
    glXWaitGL();
  else
    glXWaitX();
}
