/** \file
 * \brief Windows Printer Driver
 *
 * See Copyright Notice in cd.h
 */


#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "cdwin.h"
#include "cdprint.h"

#ifndef DC_COLORDEVICE
#define DC_COLORDEVICE          32   /* declared only if WINVER 0x0500 */
#endif

/* 
%F cdKillCanvas para Printer.
Termina a pagina e termina o documento, enviando-o para a impressora.
*/
static void cdkillcanvas (cdCtxCanvas *ctxcanvas)
{
  EndPage(ctxcanvas->hDC);
  EndDoc(ctxcanvas->hDC);
  
  cdwKillCanvas(ctxcanvas);
  
  DeleteDC(ctxcanvas->hDC);

  if (ctxcanvas->filename)
    free(ctxcanvas->filename);
  
  memset(ctxcanvas, 0, sizeof(cdCtxCanvas));
  free(ctxcanvas);
}

/* 
%F cdFlush para Printer.
Termina uma pagina e inicia outra.
*/
static void cdflush(cdCtxCanvas *ctxcanvas)
{
  GdiFlush();
  EndPage(ctxcanvas->hDC);

  StartPage(ctxcanvas->hDC);
  cdwRestoreDC(ctxcanvas);
}

static char* get_printername_attrib(cdCtxCanvas* ctxcanvas)
{
  return ctxcanvas->filename;
}

static cdAttribute printername_attrib =
{
  "PRINTERNAME",
  NULL,
  get_printername_attrib
}; 

/*
%F cdCreateCanvas para Impresora.
Usa a impressora default.
*/
static void cdcreatecanvas(cdCanvas* canvas, void *data)
{
  cdCtxCanvas* ctxcanvas;
  char *data_str = (char*) data;
  char docname[256] = "CD - Canvas Draw Document";
  DOCINFO di;
  HDC hDC;
  int dialog = 0, wtype;
  PRINTDLG pd;

  /* Inicializa parametros */
  if (data_str == NULL) 
    return;
  
  if (data_str[0] != 0)
  {
    char *ptr = strstr(data_str, "-d");

    if (ptr != NULL)
      dialog = 1;

    if (data_str[0] != '-')
    {
      strcpy(docname, data_str);

      if (dialog)
        docname[ptr - data_str - 1] = 0;
    }
  }
  
  ZeroMemory(&pd, sizeof(PRINTDLG));
  pd.lStructSize = sizeof(PRINTDLG);
  pd.nCopies = 1;

  if (dialog)
  {
    pd.Flags = PD_RETURNDC | PD_USEDEVMODECOPIES | PD_COLLATE | PD_NOPAGENUMS | PD_NOSELECTION;
    pd.hwndOwner = GetForegroundWindow();
  }
  else
  {
    pd.Flags = PD_RETURNDC | PD_RETURNDEFAULT;
  }
  
  if (!PrintDlg(&pd))
  {
    if(pd.hDevMode) 
      GlobalFree(pd.hDevMode);
    if(pd.hDevNames) 
      GlobalFree(pd.hDevNames);
    return;
  }

  hDC = pd.hDC; 

  /* Inicializa documento */
  di.cbSize = sizeof(DOCINFO);
  di.lpszDocName = docname;
  di.lpszOutput = (LPTSTR) NULL;
  di.lpszDatatype = (LPTSTR) NULL; 
  di.fwType = 0; 

  StartDoc(hDC, &di);
  
  StartPage(hDC);
  
  wtype = CDW_EMF;
  
  /* Inicializa driver WIN32 */
  ctxcanvas = cdwCreateCanvas(canvas, NULL, hDC, wtype);
  
  canvas->w = GetDeviceCaps(hDC, HORZRES);
  canvas->h = GetDeviceCaps(hDC, VERTRES);
  canvas->w_mm = (double)GetDeviceCaps(hDC, HORZSIZE);
  canvas->h_mm = (double)GetDeviceCaps(hDC, VERTSIZE);
  canvas->bpp = GetDeviceCaps(hDC, BITSPIXEL);
  canvas->xres = (double)canvas->w / canvas->w_mm;
  canvas->yres = (double)canvas->h / canvas->h_mm;
  ctxcanvas->clip_pnt[2].x = ctxcanvas->clip_pnt[1].x = canvas->w - 1;
  ctxcanvas->clip_pnt[3].y = ctxcanvas->clip_pnt[2].y = canvas->h - 1;

  if (pd.hDevNames)
  {
    unsigned char* devnames = (unsigned char*)GlobalLock(pd.hDevNames);
    DEVNAMES* dn = (DEVNAMES*)devnames;
    char* device = (char*)(devnames + dn->wDeviceOffset);

    ctxcanvas->filename = cdStrDup(device);
    cdRegisterAttribute(canvas, &printername_attrib);

    /* PDF Writer returns bpp=1, so we check if color is supported and overwrite this value */
    if (canvas->bpp==1)
    {
      char* port = (char*)(devnames + dn->wOutputOffset);
      if (DeviceCapabilities(device, port, DC_COLORDEVICE, NULL, NULL))
        canvas->bpp = 24;
    }

    GlobalUnlock(pd.hDevNames);
  }

  if(pd.hDevMode) 
    GlobalFree(pd.hDevMode);
  if(pd.hDevNames) 
    GlobalFree(pd.hDevNames);
}

static void cdinittable(cdCanvas* canvas)
{
  cdwInitTable(canvas);

  canvas->cxKillCanvas = cdkillcanvas;
  canvas->cxFlush = cdflush;
}

static cdContext cdPrinterContext =
{
  CD_CAP_ALL & ~(CD_CAP_CLEAR | CD_CAP_PLAY | CD_CAP_YAXIS | 
                 CD_CAP_IMAGERGBA | CD_CAP_GETIMAGERGB | CD_CAP_IMAGESRV | 
                 CD_CAP_FPRIMTIVES ),
  0,
  cdcreatecanvas,  
  cdinittable,
  NULL,                 
  NULL
};

cdContext* cdContextPrinter(void)
{
  if (cdUseContextPlus(CD_QUERY))
  {
    cdContext* ctx = cdGetContextPlus(CD_CTX_PRINTER);
    if (ctx != NULL)
      return ctx;
  }

  return &cdPrinterContext;
}
