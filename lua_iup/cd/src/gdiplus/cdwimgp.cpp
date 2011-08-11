/** \file
 * \brief Windows GDI+ Image Driver
 *
 * See Copyright Notice in cd.h
 */

#include "cdwinp.h"
#include "cdimage.h"
#include <stdlib.h>
#include <stdio.h>


static void cdkillcanvas (cdCtxCanvas* ctxcanvas)
{
  cdwpKillCanvas(ctxcanvas);
  delete ctxcanvas;
}

static void cdcreatecanvas(cdCanvas* canvas, void *data)
{
  if (data == NULL)
    return;

  cdCtxImage* ctximage = ((cdImage*)data)->ctximage;

  /* Inicializa parametros */
  if (ctximage == NULL) 
    return;

  Graphics* graphics = new Graphics(ctximage->bitmap);

  canvas->w = ctximage->w;
  canvas->h = ctximage->h;
  canvas->bpp = ctximage->bpp;
  
  /* Inicializa driver Image */
  cdCtxCanvas* ctxcanvas = cdwpCreateCanvas(canvas, graphics, CDW_BMP);

  ctxcanvas->bitmap = ctximage->bitmap;
}

static void cdinittable(cdCanvas* canvas)
{
  cdwpInitTable(canvas);

  canvas->cxKillCanvas = cdkillcanvas;
}

static cdContext cdImageContext =
{
  CD_CAP_ALL & ~(CD_CAP_FLUSH | CD_CAP_PLAY | CD_CAP_YAXIS ),
  1,
  cdcreatecanvas,  
  cdinittable,
  NULL,             
  NULL, 
};

extern "C" {
cdContext* cdContextImagePlus(void)
{
  return &cdImageContext;
}
}
