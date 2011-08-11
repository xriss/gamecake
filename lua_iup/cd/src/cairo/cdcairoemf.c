/** \file
 * \brief EMF Printer Driver  (Win32 Only)
 *
 * See Copyright Notice in cd.h
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <windows.h>

#include "cdcairoctx.h"
#include "cdprint.h"

#include "cairo-win32.h"


static void cdkillcanvas(cdCtxCanvas *ctxcanvas)
{
  cairo_surface_t* surface = cairo_get_target(ctxcanvas->cr);
  HDC hDC = cairo_win32_surface_get_dc(surface);
  HENHMETAFILE hEMF;

  cairo_surface_finish(surface);

  hEMF = CloseEnhMetaFile (hDC);
  DeleteEnhMetaFile (hEMF);

  cdcairoKillCanvas(ctxcanvas);
}

static void cdflush(cdCtxCanvas *ctxcanvas)
{
  (void)ctxcanvas;
  /* does nothing in EMF */
}

static void cdcreatecanvas(cdCanvas* canvas, void *data)
{
  cdCtxCanvas* ctxcanvas;
  char* strdata = (char*)data;
  int w = 0, h = 0;
  double xres, yres;
  FILE* file;
  char filename[10240] = "";
  HDC ScreenDC, hDC;
  RECT rect;
  HRGN clip_hrgn;
  
  /* Inicializa parametros */
  if (strdata == NULL) 
    return;

  strdata += cdGetFileName(strdata, filename);
  if (filename[0] == 0)
    return;
 
  sscanf(strdata,"%dx%d", &w, &h); 
  if (w == 0 || h == 0)
    return;
  
  /* Verifica se o arquivo pode ser aberto para escrita */
  file = fopen(filename, "wb");
  if (file == NULL) return;
  fclose(file);
  
  ScreenDC = GetDC(NULL);
  /* LOGPIXELS can not be used for EMF */
  xres = (double)GetDeviceCaps(ScreenDC, HORZRES) / (double)GetDeviceCaps(ScreenDC, HORZSIZE);
  yres = (double)GetDeviceCaps(ScreenDC, VERTRES) / (double)GetDeviceCaps(ScreenDC, VERTSIZE);
  /* The rectangle dimensions are given in hundredths of a millimeter */
  rect.left = 0;
  rect.top = 0;
  rect.right = (int)(100. * w / xres);
  rect.bottom = (int)(100. * h / yres);
  hDC = CreateEnhMetaFile(ScreenDC,filename,&rect,NULL);
  ReleaseDC(NULL, ScreenDC);

  if(!hDC)
    return;

  canvas->w = w;
  canvas->h = h;
  canvas->xres = xres;
  canvas->yres = yres;
  canvas->w_mm = ((double)w) / xres;
  canvas->h_mm = ((double)h) / yres;
  canvas->bpp = 24;

  /* The DC will be queried for its initial clip extents, and this will be used as the size of the cairo surface. */
  clip_hrgn = CreateRectRgn(0, 0, canvas->w, canvas->h);
  SelectClipRgn(hDC, clip_hrgn);
  DeleteObject(clip_hrgn);

  ctxcanvas = cdcairoCreateCanvas(canvas, cairo_create(cairo_win32_printing_surface_create(hDC)));
}

static void cdinittable(cdCanvas* canvas)
{
  cdcairoInitTable(canvas);
  
  canvas->cxFlush = cdflush;
  canvas->cxKillCanvas = cdkillcanvas;
}

static cdContext cdEMFCairoContext =
{
  CD_CAP_ALL & ~(CD_CAP_PLAY | CD_CAP_YAXIS | CD_CAP_REGION | CD_CAP_GETIMAGERGB |
                 CD_CAP_WRITEMODE | CD_CAP_PALETTE | CD_CAP_IMAGESRV),
  0,
  cdcreatecanvas,
  cdinittable,
  NULL,
  NULL,
};

cdContext* cdContextCairoEMF(void)
{
  return &cdEMFCairoContext;
}

