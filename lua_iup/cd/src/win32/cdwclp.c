/** \file
 * \brief Windows Clipboard Driver
 *
 * See Copyright Notice in cd.h
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "cdwin.h"
#include "cdclipbd.h"
#include "cdmf.h"
#include "cdemf.h"
#include "cdwmf.h"
#include "cdmf_private.h"


static cdSizeCB cdsizecb = NULL;

static int cdregistercallback(int cb, cdCallback func)
{
  switch (cb)
  {
  case CD_SIZECB:
    cdsizecb = (cdSizeCB)func;
    return CD_OK;
  }
  
  return CD_ERROR;
}

/* 
%F cdPlay para Clipboard.
Interpreta os dados do clipboard, seja metafile ou bitmap.
*/
static int cdplay(cdCanvas* canvas, int xmin, int xmax, int ymin, int ymax, void *data)
{
  char filename[10240]; 
  HANDLE hFile;
  DWORD dwSize, nBytesWrite;
  int err;
  unsigned char* buffer;
  (void)data;
  
  if (IsClipboardFormatAvailable(CF_TEXT))
  {
    HANDLE Handle;
    
    if (!cdStrTmpFileName(filename))
      return CD_ERROR;
    
    OpenClipboard(NULL);
    Handle = GetClipboardData(CF_TEXT);
    if (Handle == NULL)
    {
      CloseClipboard();
      return CD_ERROR;
    }
    
    buffer = (unsigned char*)GlobalLock(Handle);
    dwSize = (DWORD)GlobalSize(Handle); 
    
    hFile = CreateFile(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
    WriteFile(hFile, buffer, dwSize, &nBytesWrite, NULL);
    CloseHandle(hFile);
    
    GlobalUnlock(Handle);
    
    CloseClipboard();
    
    err = cdCanvasPlay(canvas, CD_METAFILE, xmin, xmax, ymin, ymax, filename);
    
    DeleteFile(filename);
    
    if (err == CD_OK)
      return err;
  }
  
  if (IsClipboardFormatAvailable(CF_ENHMETAFILE))
  {
    HENHMETAFILE Handle;
    
    if (!cdStrTmpFileName(filename))
      return CD_ERROR;
    
    OpenClipboard(NULL);
    Handle = (HENHMETAFILE)GetClipboardData(CF_ENHMETAFILE);
    if (Handle == NULL)
    {
      CloseClipboard();
      return CD_ERROR;
    }
    
    dwSize = GetEnhMetaFileBits(Handle, 0, NULL);
    
    buffer = (unsigned char*)malloc(dwSize);
    
    GetEnhMetaFileBits(Handle, dwSize, buffer);
    
    hFile = CreateFile(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
    WriteFile(hFile, buffer, dwSize, &nBytesWrite, NULL);
    CloseHandle(hFile);
    
    free(buffer);
    
    CloseClipboard();
    
    err = cdCanvasPlay(canvas, CD_EMF, xmin, xmax, ymin, ymax, filename);
    
    DeleteFile(filename);
    
    return err;
  }
  
  if (IsClipboardFormatAvailable(CF_METAFILEPICT))
  {
    HANDLE Handle;
    METAFILEPICT* lpMFP;
    
    if (!cdStrTmpFileName(filename))
      return CD_ERROR;
    
    OpenClipboard(NULL);
    Handle = GetClipboardData(CF_METAFILEPICT);
    if (Handle == NULL)
    {
      CloseClipboard();
      return CD_ERROR;
    }
    
    lpMFP = (METAFILEPICT*) GlobalLock(Handle);
    
    dwSize = GetMetaFileBitsEx(lpMFP->hMF, 0, NULL);
    buffer = (unsigned char*)malloc(dwSize);
    
    GetMetaFileBitsEx(lpMFP->hMF, dwSize, buffer);
    
    hFile = CreateFile(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
    wmfWritePlacebleFile(hFile, buffer, dwSize, lpMFP->mm, lpMFP->xExt, lpMFP->yExt);
    CloseHandle(hFile);
    
    GlobalUnlock(Handle);
    free(buffer);
    
    CloseClipboard();
    
    err = cdCanvasPlay(canvas, CD_WMF, xmin, xmax, ymin, ymax, filename);
    
    DeleteFile(filename);
    
    return err;
  }
  
  if (IsClipboardFormatAvailable(CF_DIB))
  {
    HANDLE Handle;
    int size;
    cdwDIB dib;
    
    OpenClipboard(NULL);
    Handle = GetClipboardData(CF_DIB);
    if (Handle == NULL)
    {
      CloseClipboard();
      return CD_ERROR;
    }
    
    cdwDIBReference(&dib, (BYTE*) GlobalLock(Handle), NULL);
    
    if (dib.type == -1)
    {
      GlobalUnlock(Handle);
      CloseClipboard();
      return CD_ERROR;
    }
    
    if (cdsizecb)
    {
      int err;
      err = cdsizecb(canvas, dib.w, dib.h, dib.w, dib.h);
      if (err)
      {
        GlobalUnlock(Handle);
        CloseClipboard();
        return CD_ERROR;
      }
    }
    
    size = dib.w*dib.h;
    
    if (xmax == 0) xmax = dib.w + xmin - 1;
    if (ymax == 0) ymax = dib.h + ymin - 1;
      
    if (dib.type == 0)
    {
      unsigned char *r, *g, *b;
      
      r = (unsigned char*)malloc(size);
      g = (unsigned char*)malloc(size);
      b = (unsigned char*)malloc(size);
      
      cdwDIBDecodeRGB(&dib, r, g, b);

      cdCanvasPutImageRectRGB(canvas, dib.w, dib.h, r, g, b, xmin, ymin, xmax - xmin + 1, ymax - ymin + 1, 0, 0, 0, 0);
      
      free(r);
      free(g);
      free(b);
    }
    else
    {
      unsigned char *index;
      long *colors;
      
      index = (unsigned char*)malloc(size);
      colors = (long*)malloc(256*sizeof(long));
      
      cdwDIBDecodeMap(&dib, index, colors);
      
      cdCanvasPutImageRectMap(canvas, dib.w, dib.h, index, colors, xmin, ymin, xmax - xmin + 1, ymax - ymin + 1, 0, 0, 0, 0);
      
      free(index);
      free(colors);
    }
    
    GlobalUnlock(Handle);
    
    CloseClipboard();
    
    return CD_ERROR;
  }
  
  if (IsClipboardFormatAvailable(CF_BITMAP))
  {
    HBITMAP Handle;
    int size, type;
    cdwDIB dib;
    HDC ScreenDC;
    SIZE sz;
    
    OpenClipboard(NULL);
    Handle = GetClipboardData(CF_BITMAP);
    if (Handle == NULL)
    {
      CloseClipboard();
      return CD_ERROR;
    }
    
    GetBitmapDimensionEx(Handle, &sz);
    
    ScreenDC = GetDC(NULL);
    if (GetDeviceCaps(ScreenDC, BITSPIXEL) > 8)
      type = 0;
    else
      type = 1;
    
    dib.w = sz.cx;
    dib.h = sz.cy;
    dib.type = type; 
    
    if (cdsizecb)
    {
      int err;
      err = cdsizecb(canvas, dib.w, dib.h, dib.w, dib.h);
      if (err)
      {
        ReleaseDC(NULL, ScreenDC);
        CloseClipboard();
        return CD_ERROR;
      }
    }
    
    cdwCreateDIB(&dib);
    
    GetDIBits(ScreenDC, Handle, 0, sz.cy, dib.bits, dib.bmi, DIB_RGB_COLORS);	
    ReleaseDC(NULL, ScreenDC);
    
    size = dib.w*dib.h;
    
    if (dib.type == 0)
    {
      unsigned char *r, *g, *b;
      
      r = (unsigned char*)malloc(size);
      g = (unsigned char*)malloc(size);
      b = (unsigned char*)malloc(size);
      
      cdwDIBDecodeRGB(&dib, r, g, b);
      
      cdCanvasPutImageRectRGB(canvas, dib.w, dib.h, r, g, b, 0, 0, dib.w, dib.h, 0, 0, 0, 0);
      
      free(r);
      free(g);
      free(b);
    }
    else
    {
      unsigned char *index;
      long *colors;
      
      index = (unsigned char*)malloc(size);
      colors = (long*)malloc(256*sizeof(long));
      
      cdwDIBDecodeMap(&dib, index, colors);
      
      cdCanvasPutImageRectMap(canvas, dib.w, dib.h, index, colors, 0, 0, dib.w, dib.h, 0, 0, 0, 0);
      
      free(index);
      free(colors);
    }
    
    cdwKillDIB(&dib);
    
    CloseClipboard();
    
    return CD_ERROR;
  }
  
  return CD_ERROR;
}

static void cdkillcanvasCLIPBDMF (cdCtxCanvas *ctxcanvas)
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

static void cdkillcanvas (cdCtxCanvas *ctxcanvas)
{
  cdwKillCanvas(ctxcanvas);
  
  OpenClipboard(NULL);
  EmptyClipboard();
  
  if (ctxcanvas->wtype == CDW_WMF)
  {
    HMETAFILE hmf = CloseMetaFile(ctxcanvas->hDC);
    
    HANDLE hMemG;
    METAFILEPICT* lpMFP;
    
    hMemG = GlobalAlloc(GHND|GMEM_DDESHARE, (DWORD)sizeof(METAFILEPICT));
    lpMFP = (METAFILEPICT*) GlobalLock(hMemG);
    
    lpMFP->mm   = MM_ANISOTROPIC;
    lpMFP->xExt = (long)(100 * ctxcanvas->canvas->w_mm);    
    lpMFP->yExt = (long)(100 * ctxcanvas->canvas->h_mm);
    
    lpMFP->hMF = hmf;
    
    GlobalUnlock(hMemG);
    SetClipboardData(CF_METAFILEPICT, hMemG);
  }
  else if (ctxcanvas->wtype == CDW_EMF)
  {
    HENHMETAFILE hmf = CloseEnhMetaFile(ctxcanvas->hDC);
    SetClipboardData(CF_ENHMETAFILE, hmf);
  }
  else
  {
    HANDLE hDib;

    GdiFlush();

    hDib = cdwCreateCopyHDIB(&ctxcanvas->bmiClip, ctxcanvas->bitsClip);

    SelectObject(ctxcanvas->hDC, ctxcanvas->hOldBitmapClip);
    DeleteObject(ctxcanvas->hBitmapClip);
    DeleteDC(ctxcanvas->hDC);

    SetClipboardData(CF_DIB, hDib);
  }
  
  CloseClipboard();
  
  memset(ctxcanvas, 0, sizeof(cdCtxCanvas));
  free(ctxcanvas);
}

static void cdcreatecanvas(cdCanvas* canvas, void *data)
{
  char* strsize = (char*)data;
  int w = 0, h = 0, wtype = CDW_EMF; /* default clipboard type */
  double xres=0, yres=0;
  HDC hDC;
  BITMAPINFO bmi;
  BYTE* bits;
  HBITMAP hBitmapClip, hOldBitmapClip;
  
  /* Inicializa parametros */
  if (strsize == NULL) 
    return;
  
  if (strstr(strsize, "-b") != NULL)
    wtype = CDW_BMP;
  else if (strstr(strsize, "-m") != NULL)
    wtype = -1; /* CD METAFILE */
  
  if (wtype != -1)
  {
    sscanf(strsize,"%dx%d",&w, &h); 
    if (w == 0 || h == 0)
      return;
  }

  if (wtype == CDW_EMF)
  {
    HDC ScreenDC = GetDC(NULL);
    RECT rect;
    /* LOGPIXELS can not be used for EMF */
    xres = (double)GetDeviceCaps(ScreenDC, HORZRES) / (double)GetDeviceCaps(ScreenDC, HORZSIZE);
    yres = (double)GetDeviceCaps(ScreenDC, VERTRES) / (double)GetDeviceCaps(ScreenDC, VERTSIZE);
    rect.left = 0;
    rect.top = 0;
    rect.right = (int)(100. * w / xres);
    rect.bottom = (int)(100. * h / yres);
    hDC = CreateEnhMetaFile(ScreenDC,NULL,&rect,NULL);
    ReleaseDC(NULL, ScreenDC);
  }
  else if (wtype == CDW_BMP)
  {
    HDC ScreenDC = GetDC(NULL);
    hDC = CreateCompatibleDC(ScreenDC);
    xres = (double)GetDeviceCaps(ScreenDC, HORZRES) / (double)GetDeviceCaps(ScreenDC, HORZSIZE);
    yres = (double)GetDeviceCaps(ScreenDC, VERTRES) / (double)GetDeviceCaps(ScreenDC, VERTSIZE);
    
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = w;
    bmi.bmiHeader.biHeight = h;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 24;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biSizeImage = 0;
    bmi.bmiHeader.biXPelsPerMeter = (long)(GetDeviceCaps(ScreenDC, LOGPIXELSX) / 0.0254);
    bmi.bmiHeader.biYPelsPerMeter = (long)(GetDeviceCaps(ScreenDC, LOGPIXELSY) / 0.0254);
    bmi.bmiHeader.biClrUsed = 0;
    bmi.bmiHeader.biClrImportant = 0;
    
    hBitmapClip = CreateDIBSection(hDC, &bmi, DIB_RGB_COLORS, &bits, NULL, 0);
    
    ReleaseDC(NULL, ScreenDC);

    if (!hBitmapClip)
      return;

    hOldBitmapClip = SelectObject(hDC, hBitmapClip);
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
    cdCtxCanvas* ctxcanvas;

    /* Inicializa driver WIN32 */
    ctxcanvas = cdwCreateCanvas(canvas, NULL, hDC, wtype);
  
    canvas->w = w;
    canvas->h = h;
    canvas->xres = xres;
    canvas->yres = yres;
    canvas->w_mm = ((double)w) / xres;
    canvas->h_mm = ((double)h) / yres;
    canvas->bpp = 24;
    ctxcanvas->clip_pnt[2].x = ctxcanvas->clip_pnt[1].x = canvas->w - 1;
    ctxcanvas->clip_pnt[3].y = ctxcanvas->clip_pnt[2].y = canvas->h - 1;

    if (wtype == CDW_BMP)
    {
      ctxcanvas->hBitmapClip = hBitmapClip;
      ctxcanvas->hOldBitmapClip = hOldBitmapClip;
      ctxcanvas->bmiClip = bmi;
      ctxcanvas->bitsClip = bits;
    }
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
    cdwInitTable(canvas);
    canvas->cxKillCanvas = cdkillcanvas;
  }
}

static cdContext cdClipboardContext =
{
  CD_CAP_ALL & ~(CD_CAP_CLEAR | CD_CAP_YAXIS | 
                 CD_CAP_IMAGERGBA | CD_CAP_GETIMAGERGB | CD_CAP_IMAGESRV | 
                 CD_CAP_FPRIMTIVES ),
  0,
  cdcreatecanvas,  
  cdinittable,
  cdplay,          
  cdregistercallback
};

cdContext* cdContextClipboard(void)
{
  if (cdUseContextPlus(CD_QUERY))
  {
    cdContext* ctx = cdGetContextPlus(CD_CTX_CLIPBOARD);
    if (ctx != NULL)
      return ctx;
  }

  return &cdClipboardContext;
}
