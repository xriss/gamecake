/** \file
 * \brief Cairo Image Driver
 *
 * See Copyright Notice in cd.h
 */

#include <stdlib.h>

#include "cdcairoctx.h"
#include "cdimage.h"


static void cdkillcanvas(cdCtxCanvas* ctxcanvas)
{
  cdcairoKillCanvas(ctxcanvas);
}

static void cdcreatecanvas(cdCanvas* canvas, void *data)
{
  cdCtxImage *ctximage = ((cdImage*)data)->ctximage;
  cdcairoCreateCanvas(canvas, (cairo_t*)ctximage->cr);
  canvas->w = ctximage->w;
  canvas->h = ctximage->h;
  canvas->w_mm = ctximage->w_mm;
  canvas->h_mm = ctximage->h_mm;
  canvas->bpp = ctximage->bpp;
  canvas->xres = ctximage->xres;
  canvas->yres = ctximage->yres;
}

static void cdinittable(cdCanvas* canvas)
{
  cdcairoInitTable(canvas);

  canvas->cxKillCanvas = cdkillcanvas;
}

static cdContext cdImageContext =
{
  CD_CAP_ALL & ~(CD_CAP_PLAY | CD_CAP_YAXIS | CD_CAP_REGION | CD_CAP_WRITEMODE | CD_CAP_PALETTE ),
  0,
  cdcreatecanvas,
  cdinittable,
  NULL,
  NULL
};

cdContext* cdContextCairoImage(void)
{
  return &cdImageContext;
}
