/** \file
 * \brief Windows Image Driver
 *
 * See Copyright Notice in cd.h
 */

#include <stdlib.h>
#include <stdio.h>

#include "cdwin.h"
#include "cdimage.h"



static void cdkillcanvas (cdCtxCanvas *ctxcanvas)
{
  cdwKillCanvas(ctxcanvas);
  
  memset(ctxcanvas, 0, sizeof(cdCtxCanvas));
  free(ctxcanvas);
}

/*
%F cdCreateCanvas para Image.
O DC é um BITMAP em memoria.
*/
static void cdcreatecanvas(cdCanvas* canvas, void *data)
{
  cdCtxCanvas* ctxcanvas;
  cdCtxImage* ctximage;
  
  if (data == NULL)
    return;

  ctximage = ((cdImage*)data)->ctximage;

  /* Inicializa parametros */
  if (ctximage == NULL) 
    return;
  
  /* Inicializa driver Image */
  ctxcanvas = cdwCreateCanvas(canvas, NULL, ctximage->hDC, CDW_BMP);

  canvas->w = ctximage->w;
  canvas->h = ctximage->h;
  canvas->w_mm = ctximage->w_mm;
  canvas->h_mm = ctximage->h_mm;
  canvas->bpp = ctximage->bpp;
  canvas->xres = ctximage->xres;
  canvas->yres = ctximage->yres;
  ctxcanvas->clip_pnt[2].x = ctxcanvas->clip_pnt[1].x = canvas->w - 1;
  ctxcanvas->clip_pnt[3].y = ctxcanvas->clip_pnt[2].y = canvas->h - 1;
}

static void cdinittable(cdCanvas* canvas)
{
  cdwInitTable(canvas);

  canvas->cxKillCanvas = cdkillcanvas;
}

static cdContext cdImageContext =
{
  CD_CAP_ALL & ~(CD_CAP_PLAY | CD_CAP_YAXIS | 
                 CD_CAP_FPRIMTIVES ),
  0,
  cdcreatecanvas,  
  cdinittable,
  NULL,             
  NULL, 
};

cdContext* cdContextImage(void)
{
  if (cdUseContextPlus(CD_QUERY))
  {
    cdContext* ctx = cdGetContextPlus(CD_CTX_IMAGE);
    if (ctx != NULL)
      return ctx;
  }

  return &cdImageContext;
}
