/** \file
* \brief Cairo Native Window Driver
*
* See Copyright Notice in cd.h
*/

#include <stdlib.h>
#include <stdio.h>

#include "cdcairoctx.h"
#include "cdnative.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <cairo-xlib.h>


static void cdkillcanvas(cdCtxCanvas *ctxcanvas)
{
  cdcairoKillCanvas(ctxcanvas);
}

int cdactivate(cdCtxCanvas *ctxcanvas)
{
  Window root;
  int x, y;
  unsigned int bw, d;
  XGetGeometry(ctxcanvas->dpy, ctxcanvas->wnd, &root, &x, &y,
               (unsigned int*)&ctxcanvas->canvas->w, (unsigned int*)&ctxcanvas->canvas->h, &bw, &d);

  ctxcanvas->canvas->w_mm = ((double)ctxcanvas->canvas->w) / ctxcanvas->canvas->xres;
  ctxcanvas->canvas->h_mm = ((double)ctxcanvas->canvas->h) / ctxcanvas->canvas->yres;

  return CD_OK;
}

static void cdcreatecanvas(cdCanvas* canvas, void *data)
{
  cdCtxCanvas* ctxcanvas;
  cairo_surface_t *surface;
  char* data_str = (char*)data;
  Window wnd, root;
  Display *dpy;
  XWindowAttributes wa;
  int x, y;
  unsigned int bw;

#ifdef SunOS_OLD
  sscanf(data_str, "%d %lu", &dpy, &wnd); 
#else
  sscanf(data_str, "%p %lu", &dpy, &wnd); 
#endif
  if (!dpy || !wnd) 
    return;

  XGetWindowAttributes(dpy, wnd, &wa);

  XGetGeometry(dpy, wnd, &root, &x, &y, (unsigned int*)&canvas->w, (unsigned int*)&canvas->h, &bw, (unsigned int*)&canvas->bpp);
  canvas->xres = ((double)DisplayWidth(dpy, XScreenNumberOfScreen(wa.screen)) / (double)DisplayWidthMM(dpy, XScreenNumberOfScreen(wa.screen)));
  canvas->yres = ((double)DisplayHeight(dpy, XScreenNumberOfScreen(wa.screen)) / (double)DisplayHeightMM(dpy, XScreenNumberOfScreen(wa.screen)));

  surface = cairo_xlib_surface_create(dpy, wnd, wa.visual, canvas->w, canvas->h);

  canvas->w_mm = ((double)canvas->w) / canvas->xres;
  canvas->h_mm = ((double)canvas->h) / canvas->yres;

  ctxcanvas = cdcairoCreateCanvas(canvas, cairo_create(surface));
  cairo_surface_destroy(surface);

  ctxcanvas->dpy = dpy;
  ctxcanvas->wnd = wnd;
}

static void cdinittable(cdCanvas* canvas)
{
  cdcairoInitTable(canvas);

  canvas->cxKillCanvas = cdkillcanvas;
  canvas->cxActivate = cdactivate;
}

/******************************************************/

static cdContext cdNativeWindowContext =
{
  CD_CAP_ALL & ~(CD_CAP_PLAY | CD_CAP_YAXIS | CD_CAP_REGION | CD_CAP_WRITEMODE | CD_CAP_PALETTE ),
  1,
  cdcreatecanvas,
  cdinittable,
  NULL,
  NULL,
};


cdContext* cdContextCairoNativeWindow(void)
{
  return &cdNativeWindowContext;
}
