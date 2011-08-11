/*=========================================================================*/
/* DRIVERS.C - 10/02/95                                                    */
/* Suporte para os drivers do CD.                                          */
/*=========================================================================*/

/*- Bibliotecas padrao usadas: --------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*- Inclusao das bibliotecas IUP e CD: ------------------------------------*/
#include <iup.h>
#include <cd.h>
#include <cdiup.h>

/*- Prototypes e declaracoes do CDTest: -----------------------------------*/
#include "cdtest.h"

/*- Contexto do CDTest (declarado em CDTEST.C): ---------------------------*/
extern tCTC ctgc;

#ifdef CDTEST_WIN32
#define CLIPBOARD_WIN32
#define WMF
#define PRINTER
#endif

#define CLIPBOARD 
#define CGM
#define MF
#define PS
#define DXF
#define DGN
#define PDF
#define CDDBG
#define SVG

static int LoadCanvas(char* ctx_name, cdContext* ctx, char *filename)
{
  if (IupGetFile(filename) == 0) 
  { 
    newmetafile(filename, ctx);
    cdActivate(ctgc.iup_canvas);
    cdWriteMode(CD_REPLACE);
    cdLineStyle(CD_CONTINUOUS);
    cdLineWidth(1);
    cdBackground(CD_WHITE);
    cdBackOpacity(CD_TRANSPARENT);
    cdForeground(CD_BLACK);
    cdInteriorStyle(CD_SOLID);
    if (ctgc.stretch_play)
    {
      cdPlay(ctx, 0, ctgc.w-1, 0, ctgc.h-1, filename);
      sprintf(ctgc.status_line, "cdPlay(%s, 0, %d, 0, %d, %s)", ctx_name, ctgc.w-1, ctgc.h-1, filename);
    }
    else
    {
      cdPlay(ctx, 0, 0, 0, 0, filename);
      sprintf(ctgc.status_line, "cdPlay(%s, 0, 0, 0, 0, %s)", ctx_name, filename);
    }
    set_status();
  }

  return IUP_DEFAULT;
}

static int SaveCanvas(char* ctx_name, cdContext* ctx, char *data)
{
  cdCanvas *canvas;

  canvas = cdCreateCanvas(ctx, data);
  if (!canvas)
  {
    IupMessagef("Error!", "Could not create canvas of driver %s.", ctx_name);
    return IUP_DEFAULT;
  }

  cdActivate(canvas);

  cdPattern(10, 10, ctgc.pattern);
  cdStipple(10, 10, ctgc.stipple);
  cdInteriorStyle(CD_SOLID);

  if (ctgc.sim == 1)
    cdSimulate(CD_SIM_ALL);
  else
    cdSimulate(CD_SIM_NONE);

  putlist(canvas);

  cdKillCanvas(canvas);

  return IUP_DEFAULT;
}


/*-------------------------------------------------------------------------*/
/* Copia o conteudo do canvas para o clipboard do Windows, vetorial.       */
/*-------------------------------------------------------------------------*/
#ifdef CLIPBOARD
#include <cdclipbd.h>

static int fClipBoard(void)
{
  char data[1000];
  sprintf(data, "%dx%d %g", ctgc.w, ctgc.h, ctgc.res);
  return SaveCanvas("CD_CLIPBOARD", CD_CLIPBOARD, data);
}

/*-------------------------------------------------------------------------*/
/* Copia o conteudo do canvas para o clipboard do Windows, BitMap.         */
/*-------------------------------------------------------------------------*/
static int fClipBoardBitmap(void)
{
  char data[1000];
  sprintf(data, "%dx%d %g -b", ctgc.w, ctgc.h, ctgc.res);
  return SaveCanvas("CD_CLIPBOARD", CD_CLIPBOARD, data);
}

static int fClipBoardMetafile(void)
{
  char data[1000];
#ifdef WIN32
  sprintf(data, "%gx%g %g -m", ((double)ctgc.w)/ctgc.res, ((double)ctgc.h)/ctgc.res, ctgc.res);
#else
  sprintf(data, "%p %gx%g %g", IupGetAttribute(IupGetHandle("cnvMain"), "XDISPLAY"), ((double)ctgc.w)/ctgc.res, ((double)ctgc.h)/ctgc.res, ctgc.res);
#endif
  return SaveCanvas("CD_CLIPBOARD", CD_CLIPBOARD, data);
}

static int fClipBoardPaste(void)
{
  char* data;
  newmetafile("", CD_CLIPBOARD);
  cdActivate(ctgc.iup_canvas);
  cdWriteMode(CD_REPLACE);
  cdLineStyle(CD_CONTINUOUS);
  cdLineWidth(1);
  cdBackground(CD_WHITE);
  cdBackOpacity(CD_TRANSPARENT);
  cdForeground(CD_BLACK);
  cdInteriorStyle(CD_SOLID);
  
#ifdef WIN32
  data = "";
#else
  data = IupGetAttribute(IupGetHandle("cnvMain"), "XDISPLAY");
#endif
  
  if (ctgc.stretch_play)
  {
    cdPlay(CD_CLIPBOARD, 0, ctgc.w-1, 0, ctgc.h-1, data);
    sprintf(ctgc.status_line, "cdPlay(CD_CLIPBOARD, 0, %d, 0, %d, \"\")", ctgc.w-1, ctgc.h-1);
  }
  else
  {
    cdPlay(CD_CLIPBOARD, 0, 0, 0, 0, data);
    sprintf(ctgc.status_line, "cdPlay(CD_CLIPBOARD, 0, 0, 0, 0, \"\")");
  }
  set_status();
  return IUP_DEFAULT;
}

#endif

/*-------------------------------------------------------------------------*/
/* Copia o conteudo do canvas para um arquivo PostScript.                  */
/*-------------------------------------------------------------------------*/
#ifdef PS
#include <cdps.h>

static int fPS(void)
{
  char filename[1024]="*.ps";
  char data[1024];

  if (IupGetFile(filename)>=0) 
  { 
    sprintf(data, "%s -s%d", filename, (int)(ctgc.res * 25.4));
    return SaveCanvas("CD_PS", CD_PS, data);
  }

  return IUP_DEFAULT;
}

static int fEPS(void)
{
  char filename[1024]="*.eps";
  char data[1024];

  if (IupGetFile(filename)>=0) 
  { 
    sprintf(data, "%s -s%d -e -l0 -r0 -t0 -b0", filename, (int)(ctgc.res * 25.4));
    return SaveCanvas("CD_PS", CD_PS, data);
  }

  return IUP_DEFAULT;
}
#endif

/*-------------------------------------------------------------------------*/
/* Copia o conteudo do canvas para um arquivo SVG.                  */
/*-------------------------------------------------------------------------*/
#ifdef SVG
#include <cdsvg.h>

static int fSVG(void)
{
  char filename[1024]="*.svg";
  char data[1024];

  if (IupGetFile(filename)>=0) 
  { 
    sprintf(data, "%s %gx%g %g", filename, ((double)ctgc.w)/ctgc.res, ((double)ctgc.h)/ctgc.res, ctgc.res);
    return SaveCanvas("CD_SVG", CD_SVG, data);
  }

  return IUP_DEFAULT;
}
#endif

#ifdef PDF
#include <cdpdf.h>

static int fPDF(void)
{
  char filename[1024]="*.pdf";
  char data[1024];

  if (IupGetFile(filename)>=0) 
  { 
    sprintf(data, "%s -s%d", filename, (int)(ctgc.res * 25.4));
    return SaveCanvas("CD_PDF", CD_PDF, data);
  }

  return IUP_DEFAULT;
}
#endif

/*-------------------------------------------------------------------------*/
/* Copia o conteudo do canvas para um arquivo CGM.                         */
/*-------------------------------------------------------------------------*/
#ifdef CGM
#include <cdcgm.h>

static int fCGMb(void)
{
  char filename[1024]="*.cgm";
  char data[1000];

  if (IupGetFile(filename)>=0) 
  { 
    sprintf(data, "%s %gx%g %g", filename, ((double)ctgc.w)/ctgc.res, ((double)ctgc.h)/ctgc.res, ctgc.res);
    return SaveCanvas("CD_CGM", CD_CGM, data);
  }

  return IUP_DEFAULT;
}

static int fCGMt(void)
{
  char filename[1024]="*.cgm";
  char data[1000];

  if (IupGetFile(filename)>=0) 
  { 
    sprintf(data, "%s %gx%g %g -t", filename, ((double)ctgc.w)/ctgc.res, ((double)ctgc.h)/ctgc.res, ctgc.res);
    return SaveCanvas("CD_CGM", CD_CGM, data);
  }

  return IUP_DEFAULT;
}

static int fPlayCGM(void)
{
  char filename[1024]="*.cgm";
  return LoadCanvas("CD_CGM", CD_CGM, filename);
}

#endif

/*-------------------------------------------------------------------------*/
/* Copia o conteudo do canvas para um arquivo DXF.                         */
/*-------------------------------------------------------------------------*/
#ifdef DXF
#include <cddxf.h>

static int fDXF(void)
{
  char filename[1024]="*.dxf";
  char data[1000];

  if (IupGetFile(filename)>=0) 
  { 
    sprintf(data, "%s %gx%g %g", filename, ((double)ctgc.w)/ctgc.res, ((double)ctgc.h)/ctgc.res, ctgc.res);
    return SaveCanvas("CD_DXF", CD_DXF, data);
  }

  return IUP_DEFAULT;
}
#endif

/*-------------------------------------------------------------------------*/
/* Copia o conteudo do canvas para um arquivo DGN.                         */
/*-------------------------------------------------------------------------*/
#ifdef DGN
#include <cddgn.h>

static int fDGN(void)
{
  char filename[1024]="*.dgn";
  char data[1000];

  if (IupGetFile(filename)>=0) 
  { 
    sprintf(data, "%s %gx%g %g -sseed2d.dgn", filename, ((double)ctgc.w)/ctgc.res, ((double)ctgc.h)/ctgc.res, ctgc.res);
    return SaveCanvas("CD_DGN", CD_DGN, data);
  }

  return IUP_DEFAULT;
}
#endif

#ifdef MF
#include <cdmf.h>
/*-------------------------------------------------------------------------*/
/* Copia o conteudo do canvas para um arquivo metafile do CD.              */
/*-------------------------------------------------------------------------*/
static int fMF(void)
{
  char filename[1024]="*.mf";
  char data[1000];

  if (IupGetFile(filename)>=0) 
  { 
    sprintf(data, "%s %gx%g %g", filename, ((double)ctgc.w)/ctgc.res, ((double)ctgc.h)/ctgc.res, ctgc.res);
    return SaveCanvas("CD_METAFILE", CD_METAFILE, data);
  }

  return IUP_DEFAULT;
}

/*-------------------------------------------------------------------------*/
/* Copia o conteudo do canvas para um arquivo metafile do CD.              */
/*-------------------------------------------------------------------------*/
static int fPlayMF(void)
{
  char filename[1024]="*.mf";
  return LoadCanvas("CD_METAFILE", CD_METAFILE, filename);
}
#endif

#ifdef CDDBG
#include <cddebug.h>
/*-------------------------------------------------------------------------*/
/* Copia o conteudo do canvas para um arquivo metafile do CD.              */
/*-------------------------------------------------------------------------*/
static int fDebug(void)
{
  char filename[1024]="*.mf";
  char data[1000];

  if (IupGetFile(filename)>=0) 
  { 
    sprintf(data, "%s %gx%g %g", filename, ((double)ctgc.w)/ctgc.res, ((double)ctgc.h)/ctgc.res, ctgc.res);
    return SaveCanvas("CD_DEBUG", CD_DEBUG, data);
  }

  return IUP_DEFAULT;
}
#endif

/*-------------------------------------------------------------------------*/
/* Copia o conteudo do canvas para um arquivo WMF.                         */
/*-------------------------------------------------------------------------*/
#ifdef WMF
#include <cdwmf.h>

static int fWMF(void)
{
  char filename[1024]="*.wmf";
  char data[1000];

  if (IupGetFile(filename)>=0) 
  { 
    sprintf(data, "%s %dx%d %g", filename, ctgc.w, ctgc.h, ctgc.res);
    return SaveCanvas("CD_WMF", CD_WMF, data);
  }

  return IUP_DEFAULT;
}

static int fPlayWMF(void)
{
  char filename[1024]="*.wmf";
  return LoadCanvas("CD_WMF", CD_WMF, filename);
}

/*-------------------------------------------------------------------------*/
/* Copia o conteudo do canvas para um arquivo EMF.                         */
/*-------------------------------------------------------------------------*/
#include <cdemf.h>

static int fEMF(void)
{
  char filename[1024]="*.emf";
  char data[1000];

  if (IupGetFile(filename)>=0) 
  { 
    sprintf(data, "%s %dx%d %g", filename, ctgc.w, ctgc.h, ctgc.res);
    return SaveCanvas("CD_EMF", CD_EMF, data);
  }

  return IUP_DEFAULT;
}

static int fPlayEMF(void)
{
  char filename[1024]="*.emf";
  return LoadCanvas("CD_EMF", CD_EMF, filename);
}
#endif

/*-------------------------------------------------------------------------*/
/* Copia o conteudo do canvas para a impressora.                           */
/*-------------------------------------------------------------------------*/
#ifdef PRINTER
#include <cdprint.h>

static int fPrint(void)
{
  char *data = "CDTEST.PRN -d";
  return SaveCanvas("CD_PRINTER", CD_PRINTER, data);
}
#endif

/*-------------------------------------------------------------------------*/
/* Inicializa os menus de Save e Open.                                     */
/*-------------------------------------------------------------------------*/
void DriversInit(void)
{
#ifdef MF
  IupSetFunction("cmdMF", (Icallback) fMF);
  IupSetFunction("cmdPlayMF", (Icallback) fPlayMF);
#endif
#ifdef PS
  IupSetAttribute(IupGetHandle("itPS"), IUP_ACTIVE, IUP_YES);
  IupSetAttribute(IupGetHandle("itEPS"), IUP_ACTIVE, IUP_YES);
  IupSetFunction("cmdPS", (Icallback) fPS);
  IupSetFunction("cmdEPS", (Icallback) fEPS);
#endif
#ifdef SVG
  IupSetAttribute(IupGetHandle("itSVG"), IUP_ACTIVE, IUP_YES);
  IupSetFunction("cmdSVG", (Icallback) fSVG);
#endif
#ifdef PDF
  IupSetAttribute(IupGetHandle("itPDF"), IUP_ACTIVE, IUP_YES);
  IupSetFunction("cmdPDF", (Icallback) fPDF);
#endif
#ifdef CLIPBOARD
  IupSetAttribute(IupGetHandle("itClipBoardMetafile"), IUP_ACTIVE, IUP_YES);
  IupSetAttribute(IupGetHandle("itClipBoardPaste"), IUP_ACTIVE, IUP_YES);
  IupSetFunction("cmdClipBoardMetafile", (Icallback)fClipBoardMetafile);
  IupSetFunction("cmdClipBoardPaste", (Icallback)fClipBoardPaste);
#endif
#ifdef CLIPBOARD_WIN32
  IupSetAttribute(IupGetHandle("itClipBoardBitmap"), IUP_ACTIVE, IUP_YES);
  IupSetAttribute(IupGetHandle("itClipBoard"), IUP_ACTIVE, IUP_YES);
  IupSetFunction("cmdClipBoardBitmap", (Icallback)fClipBoardBitmap);
  IupSetFunction("cmdClipBoard", (Icallback)fClipBoard);
#endif
#ifdef DXF
  IupSetAttribute(IupGetHandle("itDXF"), IUP_ACTIVE, IUP_YES);
  IupSetFunction("cmdDXF", (Icallback) fDXF);
#endif
#ifdef DGN
  IupSetAttribute(IupGetHandle("itDGN"), IUP_ACTIVE, IUP_YES);
  IupSetFunction("cmdDGN", (Icallback) fDGN);
#endif
#ifdef CDDBG
  IupSetFunction("cmdDebug", (Icallback) fDebug);
#endif
#ifdef CGM
  IupSetAttribute(IupGetHandle("itCGMb"), IUP_ACTIVE, IUP_YES);
  IupSetFunction("cmdCGMb", (Icallback) fCGMb);
  IupSetAttribute(IupGetHandle("itCGMt"), IUP_ACTIVE, IUP_YES);
  IupSetFunction("cmdCGMt", (Icallback) fCGMt);
  IupSetAttribute(IupGetHandle("itPlayCGM"), IUP_ACTIVE, IUP_YES);
  IupSetFunction("cmdPlayCGM", (Icallback) fPlayCGM);
#endif
#ifdef WMF
  IupSetAttribute(IupGetHandle("itEMF"), IUP_ACTIVE, IUP_YES);
  IupSetAttribute(IupGetHandle("itWMF"), IUP_ACTIVE, IUP_YES);
  IupSetFunction("cmdEMF", (Icallback) fEMF);
  IupSetFunction("cmdWMF", (Icallback) fWMF);
  IupSetAttribute(IupGetHandle("itPlayEMF"), IUP_ACTIVE, IUP_YES);
  IupSetAttribute(IupGetHandle("itPlayWMF"), IUP_ACTIVE, IUP_YES);
  IupSetFunction("cmdPlayEMF", (Icallback) fPlayEMF);
  IupSetFunction("cmdPlayWMF", (Icallback) fPlayWMF);
#endif
#ifdef PRINTER
  IupSetAttribute(IupGetHandle("itPrint"), IUP_ACTIVE, IUP_YES);
  IupSetFunction("cmdPrint", (Icallback) fPrint);
#endif
}
