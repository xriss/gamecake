/** \file
 * \brief Windows GDI+ Clipboard Driver
 *
 * See Copyright Notice in cd.h
 */

#include "cdwinp.h"
#include "cdclipbd.h"
#include "cdmf.h"
#include "cdemf.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "cdmf_private.h"


static void cdkillcanvasCLIPBDMF (cdCtxCanvas* ctxcanvas)
{
  HANDLE Handle, hFile;
  char* buffer;
  DWORD dwSize, nBytesRead;
  char filename[10240];
  cdCanvasMF* mfcanvas = (cdCanvasMF*)ctxcanvas;

  /* guardar antes de remover o canvas */
  strcpy(filename, mfcanvas->filename);
  
  OpenClipboard(NULL);
  EmptyClipboard();        
  
  cdkillcanvasMF(mfcanvas); /* this will close the file */
  
  hFile = CreateFile(filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_FLAG_DELETE_ON_CLOSE, NULL);
  dwSize = GetFileSize (hFile, NULL) ; 
  
  Handle = GlobalAlloc(GMEM_MOVEABLE, dwSize+1);
  buffer = (char*)GlobalLock(Handle);
  ReadFile(hFile, buffer, dwSize, &nBytesRead, NULL);
  buffer[dwSize] = 0;
  GlobalUnlock(Handle);
  
  CloseHandle(hFile);
  
  SetClipboardData(CF_TEXT, Handle);
  
  CloseClipboard();
}

static void cdkillcanvas (cdCtxCanvas* ctxcanvas)
{
  cdwpKillCanvas(ctxcanvas);
  
  OpenClipboard(NULL);
  EmptyClipboard();
  
  if (ctxcanvas->wtype == CDW_EMF)
  {
    HENHMETAFILE hEmf = ctxcanvas->metafile->GetHENHMETAFILE();
    SetClipboardData(CF_ENHMETAFILE, hEmf);
    delete ctxcanvas->metafile;
  }
  else
  {
    HBITMAP hBitmap;
    ctxcanvas->bitmap->GetHBITMAP(ctxcanvas->bg, &hBitmap);
    SetClipboardData(CF_BITMAP, hBitmap);
    delete ctxcanvas->bitmap;
  }
  
  CloseClipboard();
  
  delete ctxcanvas;
}

/*
%F cdCreateCanvas para Clipboard
O DC pode ser um WMF ou um BITMAP.
*/
static void cdcreatecanvas(cdCanvas* canvas, void *data)
{
  char* strsize = (char*)data;
  int w = 0, h = 0, wtype = CDW_EMF; /* default clipboard type */
  double res = 0; /* used only for BMP */
  Metafile* metafile = NULL;
  Bitmap* bitmap = NULL;
  Graphics* graphics;
  
  /* Inicializa parametros */
  if (strsize == NULL) 
    return;
  
  if (strstr(strsize, "-b") != NULL)
    wtype = CDW_BMP;
  else if (strstr(strsize, "-m") != NULL)
    wtype = -1; /* CD METAFILE */
  
  if (wtype != -1)
  {
    sscanf(strsize,"%dx%d %g",&w, &h, &res); 
    if (w == 0 || h == 0)
      return;
  }

  if (wtype == CDW_EMF)
  {
    HDC ScreenDC = GetDC(NULL);
    /* LOGPIXELS can not be used for EMF */
    canvas->xres = (double)GetDeviceCaps(ScreenDC, HORZRES) / (double)GetDeviceCaps(ScreenDC, HORZSIZE);
    canvas->yres = (double)GetDeviceCaps(ScreenDC, VERTRES) / (double)GetDeviceCaps(ScreenDC, VERTSIZE);

    Rect frameRect(0, 0, (int)(100 * w / canvas->xres), (int)(100 * h / canvas->yres));

    metafile = new Metafile(ScreenDC, frameRect, MetafileFrameUnitGdi, EmfTypeEmfPlusDual, NULL);
    ReleaseDC(NULL, ScreenDC);

    if (!metafile)
      return;

    graphics = new Graphics(metafile);
  }
  else if (wtype == CDW_BMP)
  {
    if (!res)
    {
      HDC ScreenDC = GetDC(NULL);
      canvas->xres = (double)GetDeviceCaps(ScreenDC, HORZRES) / (double)GetDeviceCaps(ScreenDC, HORZSIZE);
      canvas->yres = (double)GetDeviceCaps(ScreenDC, VERTRES) / (double)GetDeviceCaps(ScreenDC, VERTSIZE);
      ReleaseDC(NULL, ScreenDC);
    }
    else
    {
      canvas->xres = res;
      canvas->yres = res;
    }
    
    bitmap = new Bitmap(w, h, PixelFormat24bppRGB);
    if (!bitmap)
      return;
    
    bitmap->SetResolution((REAL)(canvas->xres*25.4), (REAL)(canvas->xres*25.4));

    graphics = new Graphics(bitmap);
  }
  
  if (wtype == -1)
  {
    char filename[10240]; 
    char str[10240];
    
    if (!cdStrTmpFileName(filename))
      return;
    
    sprintf(str, "%s %s", filename, strsize);
    cdcreatecanvasMF(canvas, str);
  }                    
  else
  {
    cdCtxCanvas* ctxcanvas = cdwpCreateCanvas(canvas, graphics, wtype);

    canvas->w = w;
    canvas->h = h;
    canvas->bpp = 24;

    if (wtype == CDW_BMP)
      ctxcanvas->bitmap = bitmap;
    else
      ctxcanvas->metafile = metafile;
  }
}

static void cdinittable(cdCanvas* canvas)
{
  if (canvas->invert_yaxis == 0) /* a simple way to distinguish MF from WIN */
  {
    cdinittableMF(canvas);
    canvas->cxKillCanvas = cdkillcanvasCLIPBDMF;
  }
  else
  {
    cdwpInitTable(canvas);
    canvas->cxKillCanvas = cdkillcanvas;
  }
}

static cdContext cdClipboardContext =
{
  CD_CAP_ALL & ~(CD_CAP_CLEAR | CD_CAP_FLUSH | CD_CAP_YAXIS | CD_CAP_PLAY |
                 CD_CAP_IMAGERGBA | CD_CAP_GETIMAGERGB | CD_CAP_IMAGESRV ),
  1,
  cdcreatecanvas,  
  cdinittable,
  NULL,          
  NULL
};

extern "C" {
cdContext* cdContextClipboardPlus(void)
{
  return &cdClipboardContext;
}
}
