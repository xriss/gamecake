/** \file
 * \brief Windows GDI+ Printer Driver
 *
 * See Copyright Notice in cd.h
 */

#include "cdwinp.h"
#include "cdprint.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/* 
%F cdKillCanvas para Printer.
Termina a pagina e termina o documento, enviando-o para a impressora.
*/
static void cdkillcanvas (cdCtxCanvas* ctxcanvas)
{
  cdwpKillCanvas(ctxcanvas);
  
  EndPage(ctxcanvas->hDC);
  EndDoc(ctxcanvas->hDC);

  ClosePrinter(ctxcanvas->printerHandle);
  DeleteDC(ctxcanvas->hDC);
  
  delete ctxcanvas;
}

/* 
%F cdFlush para Printer.
Termina uma pagina e inicia outra.
*/
static void cdflush(cdCtxCanvas* ctxcanvas)
{
  delete ctxcanvas->graphics;
  EndPage(ctxcanvas->hDC);

  StartPage(ctxcanvas->hDC);
  ctxcanvas->graphics = new Graphics(ctxcanvas->hDC, ctxcanvas->printerHandle);

  cdwpUpdateCanvas(ctxcanvas);
}

static void cdcreatecanvas(cdCanvas* canvas, void *data)
{
  char *data_str = (char*) data;
  char docname[256] = "CD - Canvas Draw Document";
  int dialog = 0;

  /* Inicializa parametros */
  if (data_str == NULL) 
    return;
  
  if (data_str[0] != 0)
  {
    const char *ptr = strstr(data_str, "-d");

    if (ptr != NULL)
      dialog = 1;

    if (data_str[0] != '-')
    {
      strcpy(docname, data_str);

      if (dialog)
        docname[ptr - data_str - 1] = 0;
    }
  }
  
  PRINTDLG pd;
  ZeroMemory(&pd, sizeof(PRINTDLG));
  pd.lStructSize = sizeof(PRINTDLG);
  pd.nCopies = 1;

  if (dialog)
  {
    pd.Flags = PD_RETURNDC | PD_USEDEVMODECOPIESANDCOLLATE | PD_COLLATE | PD_NOPAGENUMS | PD_NOSELECTION;
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

  HDC hDC = pd.hDC; 

  DEVMODE* devideMode = (DEVMODE*)GlobalLock(pd.hDevMode);
  HANDLE printerHandle;
  OpenPrinter((char*)devideMode->dmDeviceName, &printerHandle, NULL);
  GlobalUnlock(pd.hDevMode);

  {
    /* Inicializa documento */
    DOCINFO docInfo;
    ZeroMemory(&docInfo, sizeof(docInfo));
    docInfo.cbSize = sizeof(docInfo);
    docInfo.lpszDocName = docname;

    StartDoc(hDC, &docInfo);
  }
  
  StartPage(hDC);
  
  canvas->w = GetDeviceCaps(hDC, HORZRES);
  canvas->h = GetDeviceCaps(hDC, VERTRES);
  canvas->bpp = GetDeviceCaps(hDC, BITSPIXEL);

  Graphics* graphics = new Graphics(hDC, printerHandle);

  /* Inicializa driver WIN32 */
  cdCtxCanvas* ctxcanvas = cdwpCreateCanvas(canvas, graphics, CDW_EMF);

  ctxcanvas->hDC = hDC;
  ctxcanvas->printerHandle = printerHandle;

  if(pd.hDevMode) 
    GlobalFree(pd.hDevMode);
  if(pd.hDevNames) 
    GlobalFree(pd.hDevNames);
}

static void cdinittable(cdCanvas* canvas)
{
  cdwpInitTable(canvas);

  canvas->cxKillCanvas = cdkillcanvas;
  canvas->cxFlush = cdflush;
}

static cdContext cdPrinterContext =
{
  CD_CAP_ALL & ~(CD_CAP_CLEAR | CD_CAP_PLAY | CD_CAP_YAXIS | 
                 CD_CAP_GETIMAGERGB | CD_CAP_IMAGESRV),
  1,
  cdcreatecanvas,  
  cdinittable,
  NULL,                 
  NULL
};

extern "C" {
cdContext* cdContextPrinterPlus(void)
{
  return &cdPrinterContext;
}
}

