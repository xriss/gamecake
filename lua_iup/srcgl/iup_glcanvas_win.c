/** \file
 * \brief iupgl control for Windows
 *
 * See Copyright Notice in "iup.h"
 */

#include <windows.h>
#include <GL/gl.h>

#include <stdlib.h>
#include <stdio.h>
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
  HWND window;
  HDC device;
  HGLRC context;
  HPALETTE palette;
  int is_owned_dc;
} IGlControlData;

static int wGLCanvasDefaultResize_CB(Ihandle *ih, int width, int height)
{
  IupGLMakeCurrent(ih);
  glViewport(0,0,width,height);
  return IUP_DEFAULT;
}

static int wGLCanvasCreateMethod(Ihandle* ih, void** params)
{
  IGlControlData* gldata;
  (void)params;

  gldata = (IGlControlData*)malloc(sizeof(IGlControlData));
  memset(gldata, 0, sizeof(IGlControlData));
  iupAttribSetStr(ih, "_IUP_GLCONTROLDATA", (char*)gldata);

  IupSetCallback(ih, "RESIZE_CB", (Icallback)wGLCanvasDefaultResize_CB);

  return IUP_NOERROR;
}

static int wGLCreateContext(Ihandle* ih, IGlControlData* gldata)
{
  Ihandle* ih_shared;
  int number;
  int isIndex = 0;
  int pixelFormat;
  PIXELFORMATDESCRIPTOR test_pfd;
  PIXELFORMATDESCRIPTOR pfd = { 
    sizeof(PIXELFORMATDESCRIPTOR),  /*  size of this pfd   */
      1,                     /* version number             */
      PFD_DRAW_TO_WINDOW |   /* support window             */
      PFD_SUPPORT_OPENGL,    /* support OpenGL             */
      PFD_TYPE_RGBA,         /* RGBA type                  */
      24,                    /* 24-bit color depth         */
      0, 0, 0, 0, 0, 0,      /* color bits ignored         */
      0,                     /* no alpha buffer            */
      0,                     /* shift bit ignored          */
      0,                     /* no accumulation buffer     */
      0, 0, 0, 0,            /* accum bits ignored         */
      16,                    /* 32-bit z-buffer             */
      0,                     /* no stencil buffer          */
      0,                     /* no auxiliary buffer        */
      PFD_MAIN_PLANE,        /* main layer                 */
      0,                     /* reserved                   */
      0, 0, 0                /* layer masks ignored        */
  };

  /* the IupCanvas is already mapped, just initialize the OpenGL context */

  /* double or single buffer */
  if (iupStrEqualNoCase(iupAttribGetStr(ih,"BUFFER"), "DOUBLE"))
    pfd.dwFlags |= PFD_DOUBLEBUFFER;

  /* stereo */
  if (iupAttribGetBoolean(ih,"STEREO"))
    pfd.dwFlags |= PFD_STEREO;

  /* rgba or index */ 
  if (iupStrEqualNoCase(iupAttribGetStr(ih,"COLOR"), "INDEX"))
  {
    isIndex = 1;
    pfd.iPixelType = PFD_TYPE_COLORINDEX;
    pfd.cColorBits = 8;  /* assume 8 bits when indexed */
    number = iupAttribGetInt(ih,"BUFFER_SIZE");
    if (number > 0) pfd.cColorBits = (BYTE)number;
  }

  /* red, green, blue bits */
  number = iupAttribGetInt(ih,"RED_SIZE");
  if (number > 0) pfd.cRedBits = (BYTE)number;
  pfd.cRedShift = 0;

  number = iupAttribGetInt(ih,"GREEN_SIZE");
  if (number > 0) pfd.cGreenBits = (BYTE)number;
  pfd.cGreenShift = pfd.cRedBits;

  number = iupAttribGetInt(ih,"BLUE_SIZE");
  if (number > 0) pfd.cBlueBits = (BYTE)number;
  pfd.cBlueShift = pfd.cRedBits + pfd.cGreenBits;

  number = iupAttribGetInt(ih,"ALPHA_SIZE");
  if (number > 0) pfd.cAlphaBits = (BYTE)number;
  pfd.cAlphaShift = pfd.cRedBits + pfd.cGreenBits + pfd.cBlueBits;

  /* depth and stencil size */
  number = iupAttribGetInt(ih,"DEPTH_SIZE");
  if (number > 0) pfd.cDepthBits = (BYTE)number;

  /* stencil */
  number = iupAttribGetInt(ih,"STENCIL_SIZE");
  if (number > 0) pfd.cStencilBits = (BYTE)number;

  /* red, green, blue accumulation bits */
  number = iupAttribGetInt(ih,"ACCUM_RED_SIZE");
  if (number > 0) pfd.cAccumRedBits = (BYTE)number;

  number = iupAttribGetInt(ih,"ACCUM_GREEN_SIZE");
  if (number > 0) pfd.cAccumGreenBits = (BYTE)number;

  number = iupAttribGetInt(ih,"ACCUM_BLUE_SIZE");
  if (number > 0) pfd.cAccumBlueBits = (BYTE)number;

  number = iupAttribGetInt(ih,"ACCUM_ALPHA_SIZE");
  if (number > 0) pfd.cAccumAlphaBits = (BYTE)number;

  pfd.cAccumBits = pfd.cAccumRedBits + pfd.cAccumGreenBits + pfd.cAccumBlueBits + pfd.cAccumAlphaBits;

  /* get a device context */
  {
    LONG style = GetClassLong(gldata->window, GCL_STYLE);
    gldata->is_owned_dc = (int) ((style & CS_OWNDC) || (style & CS_CLASSDC));
  }

  gldata->device = GetDC(gldata->window);
  iupAttribSetStr(ih, "VISUAL", (char*)gldata->device);

  /* choose pixel format */
  pixelFormat = ChoosePixelFormat(gldata->device, &pfd);
  if (pixelFormat == 0)
  {
    iupAttribSetStr(ih, "ERROR", "No appropriate pixel format.");
    return IUP_NOERROR;
  } 
  SetPixelFormat(gldata->device,pixelFormat,&pfd);

  /* create rendering context */
  gldata->context = wglCreateContext(gldata->device);
  if (!gldata->context)
  {
    iupAttribSetStr(ih, "ERROR", "Could not create a rendering context.");
    return IUP_NOERROR;
  }
  iupAttribSetStr(ih, "CONTEXT", (char*)gldata->context);

  ih_shared = IupGetAttributeHandle(ih, "SHAREDCONTEXT");
  if (ih_shared && IupClassMatch(ih_shared, "glcanvas"))  /* must be an IupGLCanvas */
  {
    IGlControlData* shared_gldata = (IGlControlData*)iupAttribGet(ih_shared, "_IUP_GLCONTROLDATA");
    wglShareLists(shared_gldata->context, gldata->context);
  }

  DescribePixelFormat(gldata->device, pixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &test_pfd);
  if ((pfd.dwFlags & PFD_STEREO) && !(test_pfd.dwFlags & PFD_STEREO))
  {
    iupAttribSetStr(ih, "ERROR", "Stereo not available.");
    return IUP_NOERROR;
  }

  /* create colormap for index mode */
  if (isIndex)
  {
    if (!gldata->palette)
    {
      LOGPALETTE lp = {0x300,1,{255,255,255,PC_NOCOLLAPSE}};  /* set first color as white */
      gldata->palette = CreatePalette(&lp);
      ResizePalette(gldata->palette,1<<pfd.cColorBits);
      iupAttribSetStr(ih, "COLORMAP", (char*)gldata->palette);
    }

    SelectPalette(gldata->device,gldata->palette,FALSE);
    RealizePalette(gldata->device);
  }

  return IUP_NOERROR;
}

static int wGLCanvasMapMethod(Ihandle* ih)
{
  IGlControlData* gldata = (IGlControlData*)iupAttribGet(ih, "_IUP_GLCONTROLDATA");

  /* get a device context */
  gldata->window = (HWND)iupAttribGet(ih, "HWND"); /* check first in the hash table, can be defined by the IupFileDlg */
  if (!gldata->window)
    gldata->window = (HWND)IupGetAttribute(ih, "HWND");  /* works for Win32 and GTK, only after mapping the IupCanvas */
  if (!gldata->window)
    return IUP_NOERROR;

  {
    LONG style = GetClassLong(gldata->window, GCL_STYLE);
    gldata->is_owned_dc = (int) ((style & CS_OWNDC) || (style & CS_CLASSDC));
  }

  return wGLCreateContext(ih, gldata);
}

static void wGLCanvasUnMapMethod(Ihandle* ih)
{
  IGlControlData* gldata = (IGlControlData*)iupAttribGet(ih, "_IUP_GLCONTROLDATA");

  if (gldata->context)
  {
    if (gldata->context == wglGetCurrentContext())
      wglMakeCurrent(NULL, NULL);

    wglDeleteContext(gldata->context);
  }

  if (gldata->palette)
    DeleteObject((HGDIOBJ)gldata->palette);

  if (gldata->device)
    ReleaseDC(gldata->window, gldata->device);

  memset(gldata, 0, sizeof(IGlControlData));
}

static void wGLCanvasDestroy(Ihandle* ih)
{
  IGlControlData* gldata = (IGlControlData*)iupAttribGet(ih, "_IUP_GLCONTROLDATA");
  free(gldata);
  iupAttribSetStr(ih, "_IUP_GLCONTROLDATA", NULL);
}

static int wGLCanvasSetRefreshContextAttrib(Ihandle* ih, const char* value)
{
  IGlControlData* gldata = (IGlControlData*)iupAttribGet(ih, "_IUP_GLCONTROLDATA");

  if (!gldata->is_owned_dc)
  {
    if (gldata->context)
    {
      if (gldata->context == wglGetCurrentContext())
        wglMakeCurrent(NULL, NULL);

      wglDeleteContext(gldata->context);
    }

    if (gldata->device)
      ReleaseDC(gldata->window, gldata->device);

    wGLCreateContext(ih, gldata);
  }

  (void)value;
  return 0;
}

static Iclass* wGlCanvasNewClass(void)
{
  Iclass* ic = iupClassNew(iupRegisterFindClass("canvas"));

  ic->name = "glcanvas";
  ic->format = "a"; /* one ACTION callback name */
  ic->nativetype = IUP_TYPECANVAS;
  ic->childtype = IUP_CHILDNONE;
  ic->is_interactive = 1;

  ic->New = wGlCanvasNewClass;
  ic->Create = wGLCanvasCreateMethod;
  ic->Destroy = wGLCanvasDestroy;
  ic->Map = wGLCanvasMapMethod;
  ic->UnMap = wGLCanvasUnMapMethod;

  iupClassRegisterAttribute(ic, "BUFFER", NULL, NULL, IUPAF_SAMEASSYSTEM, "SINGLE", IUPAF_DEFAULT);
  iupClassRegisterAttribute(ic, "COLOR", NULL, NULL, IUPAF_SAMEASSYSTEM, "RGBA", IUPAF_DEFAULT);

  iupClassRegisterAttribute(ic, "CONTEXT", NULL, NULL, NULL, NULL, IUPAF_READONLY|IUPAF_NO_STRING);
  iupClassRegisterAttribute(ic, "VISUAL", NULL, NULL, NULL, NULL, IUPAF_READONLY|IUPAF_NO_STRING);
  iupClassRegisterAttribute(ic, "COLORMAP", NULL, NULL, NULL, NULL, IUPAF_READONLY|IUPAF_NO_STRING);

  iupClassRegisterAttribute(ic, "REFRESHCONTEXT", NULL, wGLCanvasSetRefreshContextAttrib, NULL, NULL, IUPAF_WRITEONLY|IUPAF_NO_INHERIT);

  return ic;
}

/******************************************* Exported functions */

void IupGLCanvasOpen(void)
{
  if (!IupGetGlobal("_IUP_GLCANVAS_OPEN"))
  {
    iupRegisterClass(wGlCanvasNewClass());
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

  if (gldata->context == wglGetCurrentContext())
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

  wglMakeCurrent(gldata->device, gldata->context);
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

  SwapBuffers(gldata->device);
}

void IupGLPalette(Ihandle* ih, int index, float r, float g, float b)
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

  /* must have a palette */
  if (gldata->palette)
  {
    PALETTEENTRY entry;
    entry.peRed    = (BYTE)(r*255);
    entry.peGreen  = (BYTE)(g*255);
    entry.peBlue   = (BYTE)(b*255);
    entry.peFlags  = PC_NOCOLLAPSE;
    SetPaletteEntries(gldata->palette,index,1,&entry);
    UnrealizeObject(gldata->device);
    SelectPalette(gldata->device,gldata->palette,FALSE);
    RealizePalette(gldata->device);
  }
}

void IupGLUseFont(Ihandle* ih, int first, int count, int list_base)
{
  HFONT font;
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

  font = (HFONT)IupGetAttribute(ih, "HFONT");
  if (font)
  {
    HFONT old_font = SelectObject(gldata->device, font);
    wglUseFontBitmaps(gldata->device, first, count, list_base);
    SelectObject(gldata->device, old_font);
  }
}

void IupGLWait(int gl)
{
  if (gl)
    glFinish();
  else
    GdiFlush();
}
