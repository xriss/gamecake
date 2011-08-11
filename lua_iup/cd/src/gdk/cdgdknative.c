/** \file
 * \brief Gdk Native Window Driver
 *
 * See Copyright Notice in cd.h
 */

#include <stdlib.h>
#include <stdio.h>

#include "cdgdk.h"
#include "cdnative.h"


int cdGetScreenColorPlanes(void)
{
  static int first = 1;
  static int bpp;

  if (first)
  {
    GdkVisual* info = gdk_visual_get_system();

    if (info != NULL)
    {
			bpp = info->depth;
      return bpp;
    }

    bpp = 2;
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
    GdkScreen* drv_screen = gdk_screen_get_default();

    dpy_width = gdk_screen_get_width(drv_screen);
    dpy_height = gdk_screen_get_height(drv_screen);
    dpy_width_mm = gdk_screen_get_width_mm(drv_screen);
    dpy_height_mm = gdk_screen_get_height_mm(drv_screen);

    first = 0;
  }

  if (width) *width = dpy_width;
  if (height) *height = dpy_height;
  if (width_mm) *width_mm = dpy_width_mm;
  if (height_mm) *height_mm = dpy_height_mm;
}

static void cdkillcanvas(cdCtxCanvas *ctxcanvas)
{
  cdgdkKillCanvas(ctxcanvas);
}

static int cdactivate(cdCtxCanvas *ctxcanvas)
{
  gdk_drawable_get_size(ctxcanvas->wnd, &ctxcanvas->canvas->w, &ctxcanvas->canvas->h);

  ctxcanvas->canvas->w_mm = ((double)ctxcanvas->canvas->w) / ctxcanvas->canvas->xres;
  ctxcanvas->canvas->h_mm = ((double)ctxcanvas->canvas->h) / ctxcanvas->canvas->yres;

  if (ctxcanvas->canvas->use_matrix)
    ctxcanvas->canvas->cxTransform(ctxcanvas, ctxcanvas->canvas->matrix);

  return CD_OK;
}

static void cdcreatecanvas(cdCanvas* canvas, void *data)
{
  GdkDrawable* wnd = (GdkDrawable*)data;
  if (!wnd) 
    return;

  cdgdkCreateCanvas(canvas, wnd, gdk_drawable_get_screen(wnd), gdk_drawable_get_visual(wnd));
}

static void cdinittable(cdCanvas* canvas)
{
  cdgdkInitTable(canvas);

  canvas->cxKillCanvas = cdkillcanvas;
  canvas->cxActivate = cdactivate;
}

/******************************************************/

static cdContext cdNativeWindowContext =
{
  CD_CAP_ALL & ~(CD_CAP_PLAY | CD_CAP_YAXIS | CD_CAP_FPRIMTIVES | CD_CAP_PATH | CD_CAP_BEZIER ),
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
