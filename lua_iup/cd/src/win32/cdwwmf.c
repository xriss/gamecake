/** \file
 * \brief Windows WMF Driver
 * Aldus Placeable Metafile 
 *
 * See Copyright Notice in cd.h
 */


#include <stdlib.h>
#include <stdio.h>

#include "cdwin.h"
#include "cdwmf.h"


static void cdkillcanvas(cdCtxCanvas *ctxcanvas)
{
  HMETAFILE hmf;
  
  cdwKillCanvas(ctxcanvas);
  
  hmf = CloseMetaFile(ctxcanvas->hDC);
  wmfMakePlaceableMetafile(hmf, ctxcanvas->filename, ctxcanvas->canvas->w, ctxcanvas->canvas->h);
  DeleteMetaFile(hmf);

  free(ctxcanvas->filename);
  
  memset(ctxcanvas, 0, sizeof(cdCtxCanvas));
  free(ctxcanvas);
}

static void cdcreatecanvas(cdCanvas* canvas, void* data)
{
  cdCtxCanvas* ctxcanvas;
  char* strdata = (char*)data;
  int w = 0, h = 0;
  float res;
  HDC ScreenDC;
  FILE* fh;
  char filename[10240] = "";
  
  /* Inicializa parametros */
  if (strdata == NULL) 
    return;

  ScreenDC = GetDC(NULL);
  res = (float)(((double)GetDeviceCaps(ScreenDC, LOGPIXELSX)) / 25.4);
  ReleaseDC(NULL, ScreenDC);

  strdata += cdGetFileName(strdata, filename);
  if (filename[0] == 0)
    return;
  
  sscanf(strdata,"%dx%d %g", &w, &h, &res); 
  if (w == 0 || h == 0)
    return;
  
  /* Verifica se o arquivo pode ser aberto para escrita */
  fh = fopen(filename, "w");
  if (fh == 0)
    return;
  
  fclose(fh);
  
  /* Inicializa driver WIN32 */
  ctxcanvas = cdwCreateCanvas(canvas, NULL, CreateMetaFile(NULL), CDW_WMF);

  canvas->w = w;
  canvas->h = h;
  canvas->xres = res;
  canvas->yres = res;
  canvas->w_mm = ((double)w) / res;
  canvas->h_mm = ((double)h) / res;
  canvas->bpp = 24;
  ctxcanvas->clip_pnt[2].x = ctxcanvas->clip_pnt[1].x = canvas->w - 1;
  ctxcanvas->clip_pnt[3].y = ctxcanvas->clip_pnt[2].y = canvas->h - 1;
  
  /* Inicializacao de variaveis particulares para o WMF */
  ctxcanvas->filename = cdStrDup(filename);
}

static void cdinittable(cdCanvas* canvas)
{
  cdwInitTable(canvas);

  canvas->cxKillCanvas = cdkillcanvas;

  /* overwrite the base Win32 driver functions */
  canvas->cxGetTextSize = cdgettextsizeEX;  
}

static cdContext cdWMFContext =
{
  CD_CAP_ALL & ~(CD_CAP_CLEAR | CD_CAP_YAXIS | CD_CAP_TEXTSIZE | 
                 CD_CAP_CLIPAREA | CD_CAP_CLIPPOLY | CD_CAP_PATTERN | 
                 CD_CAP_IMAGERGBA | CD_CAP_GETIMAGERGB | CD_CAP_IMAGESRV | 
                 CD_CAP_LINECAP | CD_CAP_LINEJOIN |
                 CD_CAP_FPRIMTIVES ),
  0,
  cdcreatecanvas,  
  cdinittable,
  cdplayWMF,          
  cdregistercallbackWMF
};

cdContext* cdContextWMF(void)
{
  return &cdWMFContext;
}
