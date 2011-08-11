/** \file
 * \brief Windows EMF Driver
 *
 * See Copyright Notice in cd.h
 */

#include <stdlib.h>
#include <stdio.h>

#include "cdwin.h"
#include "cdemf.h"



static void cdkillcanvas (cdCtxCanvas* ctxcanvas)
{
  HENHMETAFILE hmf;
  
  cdwKillCanvas(ctxcanvas);
  
  hmf = CloseEnhMetaFile(ctxcanvas->hDC);
  DeleteEnhMetaFile(hmf);
  
  memset(ctxcanvas, 0, sizeof(cdCtxCanvas));
  free(ctxcanvas);
}

/*
%F cdCreateCanvas para EMF.
O DC é um EMF em memoria.
*/
static void cdcreatecanvas(cdCanvas* canvas, void* data)
{
  cdCtxCanvas* ctxcanvas;
  char* strdata = (char*)data;
  int w = 0, h = 0;
  double xres, yres;
  FILE* file;
  char filename[10240] = "";
  HDC ScreenDC, hDC;
  RECT rect;
  
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
  rect.left = 0;
  rect.top = 0;
  rect.right = (int)(100. * w / xres);
  rect.bottom = (int)(100. * h / yres);
  hDC = CreateEnhMetaFile(ScreenDC,filename,&rect,NULL);
  ReleaseDC(NULL, ScreenDC);
  
  if(!hDC)
    return;
  
  /* Inicializa driver WIN32 */
  ctxcanvas = cdwCreateCanvas(canvas, NULL, hDC, CDW_EMF);

  canvas->w = w;
  canvas->h = h;
  canvas->xres = xres;
  canvas->yres = yres;
  canvas->w_mm = ((double)w) / xres;
  canvas->h_mm = ((double)h) / yres;
  canvas->bpp = 24;
  ctxcanvas->clip_pnt[2].x = ctxcanvas->clip_pnt[1].x = canvas->w - 1;
  ctxcanvas->clip_pnt[3].y = ctxcanvas->clip_pnt[2].y = canvas->h - 1;
}

static void cdinittable(cdCanvas* canvas)
{
  cdwInitTable(canvas);
  
  canvas->cxKillCanvas = cdkillcanvas;
}

static cdContext cdEMFContext =
{
  CD_CAP_ALL & ~(CD_CAP_CLEAR | CD_CAP_YAXIS | 
                 CD_CAP_IMAGERGBA | CD_CAP_GETIMAGERGB | CD_CAP_IMAGESRV | 
                 CD_CAP_FPRIMTIVES ),
  0,
  cdcreatecanvas,  
  cdinittable,
  cdplayEMF,          
  cdregistercallbackEMF
};

cdContext* cdContextEMF(void)
{
  if (cdUseContextPlus(CD_QUERY))
  {
    cdContext* ctx = cdGetContextPlus(CD_CTX_EMF);
    if (ctx != NULL)
      return ctx;
  }

  return &cdEMFContext;
}
