/** \file
 * \brief Windows GDI+ Double Buffer
 *
 * See Copyright Notice in cd.h
 */

#include "cdwinp.h"
#include "cddbuf.h"
#include <stdlib.h>
#include <stdio.h>


static void cdkillcanvas(cdCtxCanvas* ctxcanvas)
{
  cdwpKillCanvas(ctxcanvas);
  
  if (ctxcanvas->bitmap_dbuffer) delete ctxcanvas->bitmap_dbuffer;
  delete ctxcanvas->bitmap;
 
  delete ctxcanvas;
}

static void cddeactivate(cdCtxCanvas* ctxcanvas)
{
  cdCanvas* canvas_dbuffer = ctxcanvas->canvas_dbuffer;
  /* this is done in the canvas_dbuffer context */
  cdCanvasDeactivate(canvas_dbuffer);
}

static void cdflush(cdCtxCanvas* ctxcanvas)
{
  cdCanvas* canvas_dbuffer = ctxcanvas->canvas_dbuffer;
  
  if (ctxcanvas->dirty || ctxcanvas->bitmap_dbuffer == NULL)
  {
    ctxcanvas->dirty = 0;
    if (ctxcanvas->bitmap_dbuffer) delete ctxcanvas->bitmap_dbuffer;
    ctxcanvas->bitmap_dbuffer = new CachedBitmap(ctxcanvas->bitmap, canvas_dbuffer->ctxcanvas->graphics);
  }

  ctxcanvas->graphics->Flush(FlushIntentionSync);

  /* this is done in the canvas_dbuffer context */
  /* Flush can be affected by Origin and Clipping, but not WriteMode */
  canvas_dbuffer->ctxcanvas->graphics->ResetTransform();

  int x = 0, y = 0;
  if (canvas_dbuffer->use_origin)
  {
    x += canvas_dbuffer->origin.x;
    if (canvas_dbuffer->invert_yaxis)
      y -= canvas_dbuffer->origin.y; // top down shift
    else
      y += canvas_dbuffer->origin.y;
  }

  int old_writemode = cdCanvasWriteMode(canvas_dbuffer, CD_REPLACE);
  canvas_dbuffer->ctxcanvas->graphics->DrawCachedBitmap(ctxcanvas->bitmap_dbuffer, x, y);
  canvas_dbuffer->ctxcanvas->graphics->Flush(FlushIntentionSync);
  cdCanvasWriteMode(canvas_dbuffer, old_writemode);
}

static int cdactivate(cdCtxCanvas* ctxcanvas)
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
  if (w != ctxcanvas->canvas->w ||
      h != ctxcanvas->canvas->h)
  {
    delete ctxcanvas->graphics;
    delete ctxcanvas->bitmap;
    if (ctxcanvas->bitmap_dbuffer) delete ctxcanvas->bitmap_dbuffer;
    ctxcanvas->bitmap_dbuffer = NULL;

    Bitmap* bitmap = new Bitmap(w, h, PixelFormat24bppRGB);
    bitmap->SetResolution((REAL)(canvas_dbuffer->xres*25.4), (REAL)(canvas_dbuffer->yres*25.4));

    ctxcanvas->bitmap = bitmap;
    ctxcanvas->graphics = new Graphics(bitmap);

    ctxcanvas->canvas->w = w;
    ctxcanvas->canvas->h = h;

    ctxcanvas->dirty = 1;

    cdwpUpdateCanvas(ctxcanvas);
  }

  return CD_OK;
}

/*
%F cdCreateCanvas para DBuffer.
O DC é um BITMAP em memoria.
*/
static void cdcreatecanvas(cdCanvas* canvas, void *data)
{
  int w, h;
  cdCanvas* canvas_dbuffer = (cdCanvas*)data;
  if (!canvas_dbuffer)
    return;

  cdCanvasActivate(canvas_dbuffer); /* Update size */
  w = canvas_dbuffer->w;
  h = canvas_dbuffer->h;
  if (w==0) w=1;
  if (h==0) h=1;

  Bitmap* bitmap = new Bitmap(w, h, PixelFormat24bppRGB);
  bitmap->SetResolution((REAL)(canvas_dbuffer->xres*25.4), (REAL)(canvas_dbuffer->yres*25.4));

  Graphics imggraphics(bitmap);
  imggraphics.Clear(Color((ARGB)Color::White));

  Graphics* graphics = new Graphics(bitmap);

  canvas->w = w;
  canvas->h = h;
  canvas->bpp = 24;

  /* Initialize base driver */
  cdCtxCanvas* ctxcanvas = cdwpCreateCanvas(canvas, graphics, CDW_BMP);

  ctxcanvas->bitmap = bitmap;
  ctxcanvas->canvas_dbuffer = canvas_dbuffer;
}

static void cdinittable(cdCanvas* canvas)
{
  cdwpInitTable(canvas);

  canvas->cxActivate = cdactivate;
  canvas->cxDeactivate = cddeactivate;
  canvas->cxFlush = cdflush;
  canvas->cxKillCanvas = cdkillcanvas;
}

static cdContext cdDBufferContext =
{
  CD_CAP_ALL & ~(CD_CAP_PLAY | CD_CAP_YAXIS ),
  1,
  cdcreatecanvas,  
  cdinittable,
  NULL,             
  NULL, 
};

extern "C" {
cdContext* cdContextDBufferPlus(void)
{
  return &cdDBufferContext;
}
}
