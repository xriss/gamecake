/** \file
 * \brief Windows GDI+ Native Window Driver
 *
 * See Copyright Notice in cd.h
 */

#include "cdwinp.h"
#include "cdnative.h"
#include <stdlib.h>
#include <stdio.h>


static int cdactivate(cdCtxCanvas* ctxcanvas)
{
  if (ctxcanvas->hWnd)
  {
    RECT rect;
    GetClientRect(ctxcanvas->hWnd, &rect);
    int w = rect.right - rect.left;
    int h = rect.bottom - rect.top;
    int bpp = cdGetScreenColorPlanes();

    if(ctxcanvas->canvas->w != w ||
      ctxcanvas->canvas->h != h ||
      ctxcanvas->canvas->bpp != bpp)
    {
      ctxcanvas->canvas->w = w;
      ctxcanvas->canvas->h = h;
      ctxcanvas->canvas->bpp = bpp;

      delete ctxcanvas->graphics;
      ctxcanvas->graphics = new Graphics(ctxcanvas->hWnd);

      cdwpUpdateCanvas(ctxcanvas);
    }
  }
  
  return CD_OK;
}

static void cdkillcanvas(cdCtxCanvas* ctxcanvas)
{
  cdwpKillCanvas(ctxcanvas);

  if (ctxcanvas->release_dc)
    ReleaseDC(ctxcanvas->hWnd, ctxcanvas->hDC);

  delete ctxcanvas;
}

static void cdcreatecanvas(cdCanvas* canvas, void *data)
{
  HWND hWnd = NULL;
  HDC hDC = NULL;
  Graphics* graphics;
  int release_dc = 0;
  
  if (!data)
  {
    hDC = GetDC(NULL);
    release_dc = 1;
    
    canvas->w = GetDeviceCaps(hDC, HORZRES);
    canvas->h = GetDeviceCaps(hDC, VERTRES);

    graphics = new Graphics(hDC);
  }
  else if (IsWindow((HWND)data)) 
  {
    hWnd = (HWND)data;
    release_dc = 0;

    RECT rect;
    GetClientRect(hWnd, &rect);
    canvas->w = rect.right - rect.left;
    canvas->h = rect.bottom - rect.top;

    graphics = new Graphics(hWnd);
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

    graphics = new Graphics(hDC);
    release_dc = 0;
  }

  canvas->bpp = cdGetScreenColorPlanes();

  /* Inicializa driver WIN32 */
  cdCtxCanvas* ctxcanvas = cdwpCreateCanvas(canvas, graphics, CDW_WIN);
  ctxcanvas->hWnd = hWnd;
  ctxcanvas->hDC = hDC;
  ctxcanvas->release_dc = release_dc;
}

static void cdinittable(cdCanvas* canvas)
{
  cdwpInitTable(canvas);

  canvas->cxKillCanvas = cdkillcanvas;
  canvas->cxActivate = cdactivate;
}

static cdContext cdNativeContext =
{
  CD_CAP_ALL & ~(CD_CAP_FLUSH | CD_CAP_PLAY | CD_CAP_YAXIS ),
  1,
  cdcreatecanvas,
  cdinittable,
  NULL,              
  NULL,
};

extern "C" {
cdContext* cdContextNativeWindowPlus(void)
{
  return &cdNativeContext;
}
}
