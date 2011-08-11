/** \file
 * \brief Cairo/GTK Printer Driver  (Win32 Only)
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


#ifndef DC_COLORDEVICE
#define DC_COLORDEVICE          32   /* declared only if WINVER 0x0500 */
#endif

static void cdkillcanvas(cdCtxCanvas *ctxcanvas)
{
  cairo_surface_t* surface = cairo_get_target(ctxcanvas->cr);
  HDC hDC = cairo_win32_surface_get_dc(surface);

  cairo_surface_finish(surface);

  EndDoc(hDC);
  DeleteDC(hDC);

  if (ctxcanvas->printername)
    free(ctxcanvas->printername);

  cdcairoKillCanvas(ctxcanvas);
}

static void cdflush(cdCtxCanvas *ctxcanvas)
{
  cairo_surface_t* surface = cairo_get_target(ctxcanvas->cr);
  HDC hDC = cairo_win32_surface_get_dc(surface);

  cairo_surface_flush(surface);
  cairo_show_page(ctxcanvas->cr);

  GdiFlush();
  EndPage(hDC);

  StartPage(hDC);
}

static char* get_printername_attrib(cdCtxCanvas* ctxcanvas)
{
  return ctxcanvas->printername;
}

static cdAttribute printername_attrib =
{
  "PRINTERNAME",
  NULL,
  get_printername_attrib
}; 

static void cdcreatecanvas(cdCanvas* canvas, void *data)
{
  cdCtxCanvas* ctxcanvas;
  char *data_str = (char*) data;
  char docname[256] = "CD - Canvas Draw Document";
  DOCINFO di;
  HDC hDC;
  int dialog = 0;
  PRINTDLG pd;
  HRGN clip_hrgn;

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

  canvas->w = GetDeviceCaps(hDC, HORZRES);
  canvas->h = GetDeviceCaps(hDC, VERTRES);
  canvas->w_mm = (double)GetDeviceCaps(hDC, HORZSIZE);
  canvas->h_mm = (double)GetDeviceCaps(hDC, VERTSIZE);
  canvas->bpp = GetDeviceCaps(hDC, BITSPIXEL);
  canvas->xres = (double)canvas->w / canvas->w_mm;
  canvas->yres = (double)canvas->h / canvas->h_mm;

  /* The DC will be queried for its initial clip extents, and this will be used as the size of the cairo surface. */
  clip_hrgn = CreateRectRgn(0, 0, canvas->w, canvas->h);
  SelectClipRgn(hDC, clip_hrgn);
  DeleteObject(clip_hrgn);

  ctxcanvas = cdcairoCreateCanvas(canvas, cairo_create(cairo_win32_printing_surface_create(hDC)));
  
  di.cbSize = sizeof(DOCINFO);
  di.lpszDocName = docname;
  di.lpszOutput = (LPTSTR) NULL;
  di.lpszDatatype = (LPTSTR) NULL; 
  di.fwType = 0; 

  StartDoc(hDC, &di);
  
  StartPage(hDC);

  if (pd.hDevNames)
  {
    unsigned char* devnames = (unsigned char*)GlobalLock(pd.hDevNames);
    DEVNAMES* dn = (DEVNAMES*)devnames;
    char* device = (char*)(devnames + dn->wDeviceOffset);

    ctxcanvas->printername = cdStrDup(device);
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
  cdcairoInitTable(canvas);
  
  canvas->cxFlush = cdflush;
  canvas->cxKillCanvas = cdkillcanvas;
}

static cdContext cdPrinterCairoContext =
{
  CD_CAP_ALL & ~(CD_CAP_PLAY | CD_CAP_YAXIS | CD_CAP_REGION | CD_CAP_GETIMAGERGB |
                 CD_CAP_WRITEMODE | CD_CAP_PALETTE | CD_CAP_IMAGESRV),
  0,
  cdcreatecanvas,
  cdinittable,
  NULL,
  NULL,
};

cdContext* cdContextCairoPrinter(void)
{
  return &cdPrinterCairoContext;
}
