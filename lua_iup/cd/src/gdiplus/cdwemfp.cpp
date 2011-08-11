/** \file
 * \brief Windows GDI+ EMF Driver
 *
 * See Copyright Notice in cd.h
 */

#include "cdwinp.h"
#include "cdemf.h"
#include <stdlib.h>
#include <stdio.h>



static void cdkillcanvas (cdCtxCanvas* ctxcanvas)
{
  cdwpKillCanvas(ctxcanvas);
  delete ctxcanvas->metafile;
  delete ctxcanvas;
}

/*
%F cdCreateCanvas para EMF.
O DC é um EMF em memoria.
*/
static void cdcreatecanvas(cdCanvas* canvas, void* data)
{
  char* strdata = (char*)data;
  char filename[10240] = "";
  Metafile* metafile;
  
  /* Inicializa parametros */
  if (strdata == NULL) 
    return;
  
  strdata += cdGetFileName(strdata, filename);
  if (filename[0] == 0)
    return;
  
  int w = 0, h = 0;
  sscanf(strdata,"%dx%d", &w, &h); 
  if (w == 0 || h == 0)
    return;
  
  {
    /* Verifica se o arquivo pode ser aberto para escrita */
    FILE* file = fopen(filename, "wb");
    if (file == NULL)
      return;
    fclose(file);
  }

  {
    HDC ScreenDC = GetDC(NULL);
    /* LOGPIXELS can not be used for EMF */
    canvas->xres = (double)GetDeviceCaps(ScreenDC, HORZRES) / (double)GetDeviceCaps(ScreenDC, HORZSIZE);
    canvas->yres = (double)GetDeviceCaps(ScreenDC, VERTRES) / (double)GetDeviceCaps(ScreenDC, VERTSIZE);

    Rect frameRect(0, 0, (int)(100 * w / canvas->xres), (int)(100 * h / canvas->yres));

    metafile = new Metafile(cdwpString2Unicode(filename, strlen(filename)), 
                            ScreenDC, frameRect, MetafileFrameUnitGdi, EmfTypeEmfPlusDual, NULL);

    ReleaseDC(NULL, ScreenDC);
  }

  canvas->w = w;
  canvas->h = h;
  canvas->bpp = 24;

  Graphics* graphics = new Graphics(metafile);
  
  /* Inicializa driver WIN32 */
  cdCtxCanvas* ctxcanvas = cdwpCreateCanvas(canvas, graphics, CDW_EMF);

  /* Inicializacao de variaveis particulares para o EMF */
  ctxcanvas->metafile = metafile;
}

static void cdinittable(cdCanvas* canvas)
{
  cdwpInitTable(canvas);

  canvas->cxKillCanvas = cdkillcanvas;
}

static cdContext cdEMFContext =
{
  CD_CAP_ALL & ~(CD_CAP_CLEAR | CD_CAP_PLAY | CD_CAP_FLUSH | CD_CAP_YAXIS | 
                 CD_CAP_GETIMAGERGB | CD_CAP_IMAGESRV),
  1,
  cdcreatecanvas,  
  cdinittable,
  NULL,          
  NULL
};

extern "C" {
cdContext* cdContextEMFPlus(void)
{
  return &cdEMFContext;
}
}


