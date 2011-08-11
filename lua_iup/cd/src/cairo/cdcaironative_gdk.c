/** \file
* \brief Cairo Native Window Driver
*
* See Copyright Notice in cd.h
*/

#include <stdlib.h>
#include <stdio.h>

#include <gdk/gdk.h>

#include "cdcairoctx.h"
#include "cdnative.h"


static void cdkillcanvas(cdCtxCanvas *ctxcanvas)
{
  cdcairoKillCanvas(ctxcanvas);
}

int cdactivate(cdCtxCanvas *ctxcanvas)
{
  cdCanvas* canvas = ctxcanvas->canvas;
  int old_w = canvas->w;
  int old_h = canvas->h;

  gdk_drawable_get_size(ctxcanvas->drawable, &canvas->w, &canvas->h);

  ctxcanvas->canvas->w_mm = ((double)canvas->w) / canvas->xres;
  ctxcanvas->canvas->h_mm = ((double)canvas->h) / canvas->yres;

  if (old_w != canvas->w || old_h != canvas->h)
  {
    /* Re-create the context so internal size is updated. */
    cairo_destroy(ctxcanvas->cr);
    ctxcanvas->cr = gdk_cairo_create(ctxcanvas->drawable);

    ctxcanvas->last_source = -1;

    cairo_save(ctxcanvas->cr);
    cairo_set_operator(ctxcanvas->cr, CAIRO_OPERATOR_OVER);

    /* update canvas attributes */
    canvas->cxForeground(ctxcanvas, canvas->foreground);
    canvas->cxLineStyle(ctxcanvas, canvas->line_style);
    canvas->cxLineWidth(ctxcanvas, canvas->line_width);
    canvas->cxLineCap(ctxcanvas, canvas->line_cap);
    canvas->cxLineJoin(ctxcanvas, canvas->line_join);
    canvas->cxInteriorStyle(ctxcanvas, canvas->interior_style);
    if (canvas->clip_mode != CD_CLIPOFF) canvas->cxClip(ctxcanvas, canvas->clip_mode);
    if (canvas->use_matrix) canvas->cxTransform(ctxcanvas, canvas->matrix);
  }

  return CD_OK;
}

static void cdcreatecanvas(cdCanvas* canvas, void *data)
{
  cdCtxCanvas *ctxcanvas;
	cairo_t* cr;
  GdkScreen* screen;
  GdkDrawable* drawable = (GdkDrawable*)data;

  if (!GDK_IS_DRAWABLE(drawable))
    return;

  cr = gdk_cairo_create(drawable);
  if (!cr) 
    return;

  screen = gdk_drawable_get_screen(drawable);
  canvas->bpp = gdk_drawable_get_depth(drawable);
  canvas->xres = ((double)gdk_screen_get_width(screen)  / (double)gdk_screen_get_width_mm(screen));
  canvas->yres = ((double)gdk_screen_get_height(screen) / (double)gdk_screen_get_height_mm(screen));
  gdk_drawable_get_size(drawable, &canvas->w, &canvas->h);

  canvas->w_mm = ((double)canvas->w) / canvas->xres;
  canvas->h_mm = ((double)canvas->h) / canvas->yres;

  ctxcanvas = cdcairoCreateCanvas(canvas, cr);

  ctxcanvas->drawable = drawable;
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
  CD_CAP_ALL & ~(CD_CAP_PLAY | CD_CAP_YAXIS | CD_CAP_REGION | CD_CAP_WRITEMODE | CD_CAP_PALETTE),
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
