/** \file
 * \brief Windows Native Window Driver
 *
 * See Copyright Notice in cd.h
 */

#include <stdlib.h>
#include <stdio.h>

#include "cdwin.h"
#include "cdnative.h"


int cdGetScreenColorPlanes(void)
{
  int bpp;
  HDC ScreenDC = GetDC(NULL);
  bpp = GetDeviceCaps(ScreenDC, BITSPIXEL);
  ReleaseDC(NULL, ScreenDC);
  return bpp;
}

void cdGetScreenSize(int *width, int *height, double *width_mm, double *height_mm)
{
  HDC ScreenDC = GetDC(NULL);
  if (width) *width = GetDeviceCaps(ScreenDC, HORZRES);
  if (height) *height = GetDeviceCaps(ScreenDC, VERTRES);
  if (width_mm) *width_mm = ((GetDeviceCaps(ScreenDC, HORZRES) * 25.4) / GetDeviceCaps(ScreenDC, LOGPIXELSX));
  if (height_mm) *height_mm = ((GetDeviceCaps(ScreenDC, VERTRES) * 25.4) / GetDeviceCaps(ScreenDC, LOGPIXELSY));
  ReleaseDC(NULL, ScreenDC);
}

static void cdwReleaseDC(cdCtxCanvas *ctxcanvas)
{
  SelectObject(ctxcanvas->hDC, ctxcanvas->hOldBrush);     
  SelectObject(ctxcanvas->hDC, ctxcanvas->hOldPen);
  SelectObject(ctxcanvas->hDC, ctxcanvas->hOldFont);
  ReleaseDC(ctxcanvas->hWnd, ctxcanvas->hDC);
  ctxcanvas->hDC = NULL;
}

static int cdactivate(cdCtxCanvas *ctxcanvas)
{
  if (ctxcanvas->hWnd)
  {
    RECT rect;
    GetClientRect(ctxcanvas->hWnd, &rect);
    ctxcanvas->canvas->w = rect.right - rect.left;
    ctxcanvas->canvas->h = rect.bottom - rect.top;
  
    ctxcanvas->canvas->w_mm = ((double)ctxcanvas->canvas->w) / ctxcanvas->canvas->xres;
    ctxcanvas->canvas->h_mm = ((double)ctxcanvas->canvas->h) / ctxcanvas->canvas->yres;
  
    ctxcanvas->canvas->bpp = cdGetScreenColorPlanes();
  }
  
  /* Se nao e' ownwer, tem que restaurar o contexto */
  if (!ctxcanvas->isOwnedDC)
  {
    if (ctxcanvas->hDC) /* deactivate not called */
      cdwReleaseDC(ctxcanvas);

    ctxcanvas->hDC = GetDC(ctxcanvas->hWnd);
    cdwRestoreDC(ctxcanvas);
  }

  if (ctxcanvas->canvas->use_matrix)
    ctxcanvas->canvas->cxTransform(ctxcanvas, ctxcanvas->canvas->matrix);

  return CD_OK;
}

static void cddeactivate(cdCtxCanvas *ctxcanvas)
{
  /* Se nao e' ownwer, tem que liberar o contexto */
  if (!ctxcanvas->isOwnedDC && ctxcanvas->hDC)
    cdwReleaseDC(ctxcanvas);
}

static void cdkillcanvas(cdCtxCanvas *ctxcanvas)
{
  /* se nao e' owner e nao esta' ativo, simula ativacao */
  if (!ctxcanvas->isOwnedDC && !ctxcanvas->hDC)
  {
    ctxcanvas->hDC = GetDC(ctxcanvas->hWnd);
    cdwRestoreDC(ctxcanvas);
  }
  
  cdwKillCanvas(ctxcanvas);

  if (ctxcanvas->release_dc)
    ReleaseDC(ctxcanvas->hWnd, ctxcanvas->hDC);
  
  memset(ctxcanvas, 0, sizeof(cdCtxCanvas));
  free(ctxcanvas);
}

static void cdcreatecanvas(cdCanvas* canvas, void *data)
{
  cdCtxCanvas* ctxcanvas;
  HWND hWnd = NULL;
  HDC hDC, ScreenDC;
  int release_dc = 0;

  ScreenDC = GetDC(NULL);
  canvas->bpp = GetDeviceCaps(ScreenDC, BITSPIXEL);
  canvas->xres = (float)(((double)GetDeviceCaps(ScreenDC, LOGPIXELSX)) / 25.4);
  canvas->yres = (float)(((double)GetDeviceCaps(ScreenDC, LOGPIXELSY)) / 25.4);
  ReleaseDC(NULL, ScreenDC);

  if (!data)
  {
    hDC = GetDC(NULL);
    release_dc = 1;
    canvas->w = GetDeviceCaps(hDC, HORZRES);
    canvas->h = GetDeviceCaps(hDC, VERTRES);
  }
  else if (IsWindow((HWND)data)) 
  {
    RECT rect;
    hWnd = (HWND)data;

    hDC = GetDC(hWnd);
    release_dc = 1;
  
    GetClientRect(hWnd, &rect);
    canvas->w = rect.right - rect.left;
    canvas->h = rect.bottom - rect.top;
  }
  else  /* can be a HDC or a string */
  {
    DWORD objtype = GetObjectType((HGDIOBJ)data);
    if (objtype == OBJ_DC || objtype == OBJ_MEMDC || 
        objtype == OBJ_ENHMETADC || objtype == OBJ_METADC)   
    {
      hDC = (HDC)data;
      canvas->w = GetDeviceCaps(hDC, HORZRES);
      canvas->h = GetDeviceCaps(hDC, VERTRES);
    }
    else
    {
      hDC = NULL;
      canvas->w = 0;
      canvas->h = 0;
      sscanf((char*)data,"%p %dx%d", &hDC, &canvas->w, &canvas->h); 

      if (!hDC || !canvas->w || !canvas->h)
        return;
    }
    release_dc = 0;
  }
  
  canvas->w_mm = ((double)canvas->w) / canvas->xres;
  canvas->h_mm = ((double)canvas->h) / canvas->yres;

  /* Inicializa driver WIN32 */
  ctxcanvas = cdwCreateCanvas(canvas, hWnd, hDC, CDW_WIN);
  
  ctxcanvas->release_dc = release_dc;
  ctxcanvas->clip_pnt[2].x = ctxcanvas->clip_pnt[1].x = canvas->w - 1;
  ctxcanvas->clip_pnt[3].y = ctxcanvas->clip_pnt[2].y = canvas->h - 1;

  if (hWnd)
  {
    LONG style = GetClassLong(hWnd, GCL_STYLE);
    ctxcanvas->isOwnedDC = (int) ((style & CS_OWNDC) || (style & CS_CLASSDC));
  }
  else
    ctxcanvas->isOwnedDC = 1;

  /* Se nao e' ownwer, tem que liberar o contexto */
  if (!ctxcanvas->isOwnedDC)
    cdwReleaseDC(ctxcanvas);
}

static void cdinittable(cdCanvas* canvas)
{
  cdwInitTable(canvas);

  canvas->cxKillCanvas = cdkillcanvas;
  canvas->cxActivate = cdactivate;
  canvas->cxDeactivate = cddeactivate;
}

static cdContext cdNativeContext =
{
  CD_CAP_ALL & ~(CD_CAP_PLAY | CD_CAP_YAXIS | CD_CAP_FPRIMTIVES ),
  0,
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

  return &cdNativeContext;
}
