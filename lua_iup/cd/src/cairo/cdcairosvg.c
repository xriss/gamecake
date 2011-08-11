/** \file
 * \brief Cairo SVG Driver
 *
 * See Copyright Notice in cd.h
 */

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

#include "cd.h"
#include "cdcairo.h"
#include "cdcairoctx.h"

#include <cairo-svg.h>


static void cdflush(cdCtxCanvas *ctxcanvas)
{
  (void)ctxcanvas;
  /* does nothing in SVG */
}

static void cdkillcanvas (cdCtxCanvas *ctxcanvas)
{
  cdcairoKillCanvas(ctxcanvas);
}

static void cdcreatecanvas(cdCanvas* canvas, void* data)
{
  cdCtxCanvas* ctxcanvas;
  char* strdata = (char*)data;
  char filename[10240] = "";
  double w_mm = INT_MAX*3.78, h_mm = INT_MAX*3.78, res = 3.78;
  cairo_surface_t *surface;

  /* Starting parameters */
  if (strdata == NULL) 
    return;

  strdata += cdGetFileName(strdata, filename);
  if (filename[0] == 0)
    return;

  sscanf(strdata, "%lgx%lg %lg", &w_mm, &h_mm, &res);
  
  /* update canvas context */
  canvas->w = (int)(w_mm * res);
  canvas->h = (int)(h_mm * res);
  canvas->w_mm = w_mm;
  canvas->h_mm = h_mm;
  canvas->bpp = 24;
  canvas->xres = res;
  canvas->yres = res;

	surface = cairo_svg_surface_create(filename, CD_MM2PT*w_mm, CD_MM2PT*h_mm);

  /* Starting Cairo driver */
  ctxcanvas = cdcairoCreateCanvas(canvas, cairo_create(surface));
  cairo_surface_destroy(surface);
}

static void cdinittable(cdCanvas* canvas)
{
  cdcairoInitTable(canvas);
  canvas->cxKillCanvas = cdkillcanvas;
  canvas->cxFlush = cdflush;
}

static cdContext cdCairoSVGContext =
{
  CD_CAP_ALL & ~(CD_CAP_PLAY | CD_CAP_YAXIS | CD_CAP_REGION | CD_CAP_WRITEMODE | CD_CAP_PALETTE),
  0,
  cdcreatecanvas,  
  cdinittable,
  NULL,                 
  NULL
};

cdContext* cdContextCairoSVG(void)
{
  return &cdCairoSVGContext;
}
