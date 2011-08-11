/** \file
 * \brief X-Windows Native Window Driver
 *
 * See Copyright Notice in cd.h
 */

#include <stdlib.h>
#include <stdio.h>

#include "cdx11.h"
#include "cdnative.h"


int cdGetScreenColorPlanes(void)
{
  static int first = 1;
  static int bpp;

  if (first)
  {
    int nitems;
    XVisualInfo info;
    Display* drv_display = XOpenDisplay(NULL);

    info.depth = 24;
    if (XGetVisualInfo(drv_display, VisualDepthMask, &info, &nitems) != NULL)
    {
      bpp = 24;
      XCloseDisplay(drv_display);
      return bpp;
    }

    info.depth = 16;
    if (XGetVisualInfo(drv_display, VisualDepthMask, &info, &nitems) != NULL)
    {
      bpp = 16;
      XCloseDisplay(drv_display);
      return bpp;
    }

    info.depth = 8;
    if (XGetVisualInfo(drv_display, VisualDepthMask, &info, &nitems) != NULL)
    {
      bpp = 8;
      XCloseDisplay(drv_display);
      return bpp;
    }

    info.depth = 4;
    if (XGetVisualInfo(drv_display, VisualDepthMask, &info, &nitems) != NULL)
    {
      bpp = 4;
      XCloseDisplay(drv_display);
      return bpp;
    }

    bpp = 2;
    XCloseDisplay(drv_display);

    first = 0;
  }

  return bpp;
}

void cdGetScreenSize(int *width, int *height, double *width_mm, double *height_mm)
{
  static int first = 1;
  static int dpy_width, dpy_height, dpy_width_mm, dpy_height_mm;

  if (first)
  {
    Display* drv_display = XOpenDisplay(NULL);
    int drv_screen  = DefaultScreen (drv_display);

    dpy_width = DisplayWidth(drv_display,drv_screen);
    dpy_height = DisplayHeight(drv_display,drv_screen);
    dpy_width_mm = DisplayWidthMM(drv_display,drv_screen);
    dpy_height_mm = DisplayHeightMM(drv_display,drv_screen);

    XCloseDisplay(drv_display);

    first = 0;
  }

  if (width) *width = dpy_width;
  if (height) *height = dpy_height;
  if (width_mm) *width_mm = dpy_width_mm;
  if (height_mm) *height_mm = dpy_height_mm;
}

static void cdkillcanvas(cdCtxCanvas *ctxcanvas)
{
  cdxKillCanvas(ctxcanvas);
}

static int cdactivate(cdCtxCanvas *ctxcanvas)
{
  Window root;
  int x, y;
  unsigned int bw, d;
  XGetGeometry(ctxcanvas->dpy, ctxcanvas->wnd, &root, &x, &y,
               (unsigned int*)&ctxcanvas->canvas->w, (unsigned int*)&ctxcanvas->canvas->h, &bw, &d);

  ctxcanvas->canvas->w_mm = ((double)ctxcanvas->canvas->w) / ctxcanvas->canvas->xres;
  ctxcanvas->canvas->h_mm = ((double)ctxcanvas->canvas->h) / ctxcanvas->canvas->yres;

  if (ctxcanvas->canvas->use_matrix)
    ctxcanvas->canvas->cxTransform(ctxcanvas, ctxcanvas->canvas->matrix);

  return CD_OK;
}

static void cdcreatecanvas(cdCanvas* canvas, void *data)
{
  char* data_str = (char*)data;
  Window wnd;
  Display *dpy;
  XWindowAttributes wa;

#ifdef SunOS_OLD
  sscanf(data_str, "%d %lu", &dpy, &wnd); 
#else
  sscanf(data_str, "%p %lu", &dpy, &wnd); 
#endif

  if (!dpy || !wnd) 
    return;

  XGetWindowAttributes(dpy, wnd, &wa);
  cdxCreateCanvas(canvas, dpy, XScreenNumberOfScreen(wa.screen), wnd, wa.visual);
}

static void cdinittable(cdCanvas* canvas)
{
  cdxInitTable(canvas);

  canvas->cxKillCanvas = cdkillcanvas;
  canvas->cxActivate = cdactivate;
}

/******************************************************/

static cdContext cdNativeWindowContext =
{
  CD_CAP_ALL & ~(CD_CAP_PLAY | CD_CAP_YAXIS | CD_CAP_FPRIMTIVES | CD_CAP_PATH | CD_CAP_BEZIER),
  1,
  cdcreatecanvas,
  cdinittable,
  NULL,
  NULL,
};


cdContext* cdContextNativeWindow(void)
{
  if (cdUseContextPlus(CD_QUERY))
  {
    cdContext* ctx = cdGetContextPlus(CD_CTX_NATIVEWINDOW);
    if (ctx != NULL)
      return ctx;
  }

  return &cdNativeWindowContext;
}
