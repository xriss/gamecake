/** \file
 * \brief Windows Double Buffer
 *
 * See Copyright Notice in cd.h
 */

#include <stdlib.h>
#include <stdio.h>

#include "cdwin.h"
#include "cddbuf.h"


static void cdkillcanvas(cdCtxCanvas *ctxcanvas)
{
  cdKillImage(ctxcanvas->image_dbuffer);
  cdwKillCanvas(ctxcanvas);
  
  memset(ctxcanvas, 0, sizeof(cdCtxCanvas));
  free(ctxcanvas);
}

static void cddeactivate(cdCtxCanvas* ctxcanvas)
{
  cdCanvas* canvas_dbuffer = ctxcanvas->canvas_dbuffer;
  /* this is done in the canvas_dbuffer context */
  cdCanvasDeactivate(canvas_dbuffer);
}

static void cdflush(cdCtxCanvas *ctxcanvas)
{
  int old_writemode;
  cdImage* image_dbuffer = ctxcanvas->image_dbuffer;
  cdCanvas* canvas_dbuffer = ctxcanvas->canvas_dbuffer;

  GdiFlush();

  /* this is done in the canvas_dbuffer context */
  /* Flush can be affected by Origin and Clipping, but not WriteMode */
  old_writemode = cdCanvasWriteMode(canvas_dbuffer, CD_REPLACE);
  cdCanvasPutImageRect(canvas_dbuffer, image_dbuffer, 0, 0, 0, 0, 0, 0);
  cdCanvasWriteMode(canvas_dbuffer, old_writemode);
}

static void cdcreatecanvas(cdCanvas* canvas, cdCanvas* canvas_dbuffer)
{
  int w, h;
  cdCtxCanvas* ctxcanvas;
  cdImage* image_dbuffer;
  cdCtxImage* ctximage;

  cdCanvasActivate(canvas_dbuffer);
  w = canvas_dbuffer->w;
  h = canvas_dbuffer->h;
  if (w==0) w=1;
  if (h==0) h=1;

  /* this is done in the canvas_dbuffer context */
  image_dbuffer = cdCanvasCreateImage(canvas_dbuffer, w, h);
  if (!image_dbuffer) 
    return;

  ctximage = image_dbuffer->ctximage;
  
  /* Inicializa driver DBuffer */
  ctxcanvas = cdwCreateCanvas(canvas, NULL, ctximage->hDC, CDW_BMP);

  ctxcanvas->image_dbuffer = image_dbuffer;
  ctxcanvas->canvas_dbuffer = canvas_dbuffer;

  canvas->w = ctximage->w;
  canvas->h = ctximage->h;
  canvas->w_mm = ctximage->w_mm;
  canvas->h_mm = ctximage->h_mm;
  canvas->bpp = ctximage->bpp;
  canvas->xres = ctximage->xres;
  canvas->yres = ctximage->yres;

  ctxcanvas->clip_pnt[2].x = ctxcanvas->clip_pnt[1].x = ctximage->w - 1;
  ctxcanvas->clip_pnt[3].y = ctxcanvas->clip_pnt[2].y = ctximage->h - 1;
}

static int cdactivate(cdCtxCanvas *ctxcanvas)
{
  int w, h;
  cdCanvas* canvas_dbuffer = ctxcanvas->canvas_dbuffer;

  /* this is done in the canvas_dbuffer context */
  /* this will update canvas size */
  cdCanvasActivate(canvas_dbuffer);
  w = canvas_dbuffer->w;
  h = canvas_dbuffer->h;
  if (w==0) w=1;
  if (h==0) h=1;

  /* check if the size changed */
  if (w != ctxcanvas->image_dbuffer->w ||
      h != ctxcanvas->image_dbuffer->h)
  {
    cdCanvas* canvas = ctxcanvas->canvas;
    /* save the current, if the rebuild fail */
    cdImage* old_image_dbuffer = ctxcanvas->image_dbuffer;
    cdCtxCanvas* old_ctxcanvas = ctxcanvas;

    /* if the image is rebuild, the canvas that uses the image must be also rebuild */

    /* rebuild the image and the canvas */
    canvas->ctxcanvas = NULL;
    cdcreatecanvas(canvas, canvas_dbuffer);
    if (!canvas->ctxcanvas)
    {
      canvas->ctxcanvas = old_ctxcanvas;
      return CD_ERROR;
    }

    /* remove the old image and canvas */
    cdwKillCanvas(old_ctxcanvas);  /* ctxcanvas e ctxcanvas->hDC released in each driver */
    cdKillImage(old_image_dbuffer); /* the ctxcanvas->hDC is released here, so do it after the cdwKillCanvas */
    free(old_ctxcanvas);

    ctxcanvas = canvas->ctxcanvas;

    /* update canvas attributes */
    cdUpdateAttributes(canvas);
  }

  return CD_OK;
}

static void cdinittable(cdCanvas* canvas)
{
  cdwInitTable(canvas);

  canvas->cxActivate = cdactivate;
  canvas->cxDeactivate = cddeactivate;
  canvas->cxFlush = cdflush;
  canvas->cxKillCanvas = cdkillcanvas;
}

static cdContext cdDBufferContext =
{
  CD_CAP_ALL & ~(CD_CAP_PLAY | CD_CAP_YAXIS | 
                 CD_CAP_FPRIMTIVES ),
  0,
  cdcreatecanvas,  
  cdinittable,
  NULL,             
  NULL, 
};

cdContext* cdContextDBuffer(void)
{
  if (cdUseContextPlus(CD_QUERY))
  {
    cdContext* ctx = cdGetContextPlus(CD_CTX_DBUFFER);
    if (ctx != NULL)
      return ctx;
  }

  return &cdDBufferContext;
}
