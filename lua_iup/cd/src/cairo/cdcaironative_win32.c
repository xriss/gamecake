/** \file
* \brief Cairo Native Window Driver
*
* See Copyright Notice in cd.h
*/

#include <stdlib.h>
#include <stdio.h>

#include "cdcairoctx.h"
#include "cdnative.h"

#include <windows.h>
#include <cairo-win32.h>


static void cdkillcanvas(cdCtxCanvas *ctxcanvas)
{
  if (ctxcanvas->hDC)
    ReleaseDC(ctxcanvas->hWnd, ctxcanvas->hDC);

  cdcairoKillCanvas(ctxcanvas);
}

int cdactivate(cdCtxCanvas *ctxcanvas)
{
  if (ctxcanvas->hWnd)
  {
    RECT rect;
    GetClientRect(ctxcanvas->hWnd, &rect);
    ctxcanvas->canvas->w = rect.right - rect.left;
    ctxcanvas->canvas->h = rect.bottom - rect.top;
  
    ctxcanvas->canvas->bpp = cdGetScreenColorPlanes();
  }

  /* Se nao e' ownwer, tem que restaurar o contexto */
  if (!ctxcanvas->isOwnedDC)
  {
    cairo_surface_t *surface;

    if (ctxcanvas->hDC) /* deactivate not called */
    {
      cairo_destroy(ctxcanvas->cr);
      ReleaseDC(ctxcanvas->hWnd, ctxcanvas->hDC);
    }

    ctxcanvas->hDC = GetDC(ctxcanvas->hWnd);
    surface = cairo_win32_surface_create(ctxcanvas->hDC);
    ctxcanvas->cr = cairo_create(surface);
    cairo_surface_destroy(surface);
  }

  ctxcanvas->canvas->w_mm = ((double)ctxcanvas->canvas->w) / ctxcanvas->canvas->xres;
  ctxcanvas->canvas->h_mm = ((double)ctxcanvas->canvas->h) / ctxcanvas->canvas->yres;

  if (ctxcanvas->canvas->use_matrix)
    ctxcanvas->canvas->cxTransform(ctxcanvas, ctxcanvas->canvas->matrix);

  return CD_OK;
}

static void cddeactivate(cdCtxCanvas *ctxcanvas)
{
  /* If not owner, release the DC */
  if (!ctxcanvas->isOwnedDC && ctxcanvas->hDC)
  {
    cairo_destroy(ctxcanvas->cr);
    ReleaseDC(ctxcanvas->hWnd, ctxcanvas->hDC);
    ctxcanvas->cr = NULL;
    ctxcanvas->hDC = NULL;
  }
}

static void cdcreatecanvas(cdCanvas* canvas, void *data)
{
  cdCtxCanvas* ctxcanvas;
  cairo_surface_t *surface;

  HWND hWnd = (HWND)data;
  HDC ScreenDC, hDC;
  HRGN clip_hrgn;

  ScreenDC = GetDC(NULL);
  canvas->bpp = GetDeviceCaps(ScreenDC, BITSPIXEL);
  canvas->xres = (float)(((double)GetDeviceCaps(ScreenDC, LOGPIXELSX)) / 25.4);
  canvas->yres = (float)(((double)GetDeviceCaps(ScreenDC, LOGPIXELSY)) / 25.4);
  ReleaseDC(NULL, ScreenDC);

  if (!data)
  {
    hDC = GetDC(NULL);
    canvas->w = GetDeviceCaps(hDC, HORZRES);
    canvas->h = GetDeviceCaps(hDC, VERTRES);
  }
  else 
  {
    RECT rect;
    hWnd = (HWND)data;

    hDC = GetDC(hWnd);
  
    GetClientRect(hWnd, &rect);
    canvas->w = rect.right - rect.left;
    canvas->h = rect.bottom - rect.top;
  }

  /* initial clip extents controls size */
  clip_hrgn = CreateRectRgn(0, 0, canvas->w, canvas->h);
  SelectClipRgn(hDC, clip_hrgn);
  DeleteObject(clip_hrgn);

  surface = cairo_win32_surface_create(hDC);

  canvas->w_mm = ((double)canvas->w) / canvas->xres;
  canvas->h_mm = ((double)canvas->h) / canvas->yres;

  ctxcanvas = cdcairoCreateCanvas(canvas, cairo_create(surface));
  cairo_surface_destroy(surface);

  ctxcanvas->hDC = hDC;
  ctxcanvas->hWnd = hWnd;

  if (hWnd)
  {
    LONG style = GetClassLong(hWnd, GCL_STYLE);
    ctxcanvas->isOwnedDC = (int) ((style & CS_OWNDC) || (style & CS_CLASSDC));
  }
  else
    ctxcanvas->isOwnedDC = 1;
}

static void cdinittable(cdCanvas* canvas)
{
  cdcairoInitTable(canvas);

  canvas->cxKillCanvas = cdkillcanvas;
  canvas->cxActivate = cdactivate;
  canvas->cxDeactivate = cddeactivate;
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

// cairo_win32_printing_surface_create  CD_PRINTER 
