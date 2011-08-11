/** \file
 * \brief Windows Base Driver
 *
 * See Copyright Notice in cd.h
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "cdwin.h"

#ifndef AC_SRC_ALPHA
#define AC_SRC_ALPHA                0x01
#endif

/* CD region combine to WIN32 region combine */
static int sCombineRegion2win [] ={RGN_OR, RGN_AND, RGN_DIFF, RGN_XOR};

typedef BOOL (CALLBACK* AlphaBlendFunc)( HDC hdcDest, 
                             int xoriginDest, int yoriginDest, 
                             int wDest, int hDest, HDC hdcSrc, 
                             int xoriginSrc, int yoriginSrc, 
                             int wSrc, int hSrc, 
                             BLENDFUNCTION ftn);
static AlphaBlendFunc cdwAlphaBlend = NULL;

static void cdgettextsize (cdCtxCanvas* ctxcanvas, const char *s, int len, int *width, int *height);

/*
%F Libera memoria e handles alocados pelo driver Windows.
*/
void cdwKillCanvas(cdCtxCanvas* ctxcanvas)
{
  if (ctxcanvas->clip_pnt != NULL) 
    free(ctxcanvas->clip_pnt);

  if (ctxcanvas->dib_bits != NULL) 
    free(ctxcanvas->dib_bits);
  
  /* apaga as areas de memoria do windows para os padroes  */

  if (ctxcanvas->clip_hrgn) DeleteObject(ctxcanvas->clip_hrgn);
  if (ctxcanvas->new_rgn) DeleteObject(ctxcanvas->new_rgn);

  if (ctxcanvas->hOldBitmapPat) SelectObject(ctxcanvas->hDCMemPat, ctxcanvas->hOldBitmapPat);
  if (ctxcanvas->hBitmapPat) DeleteObject(ctxcanvas->hBitmapPat);
  if (ctxcanvas->hDCMemPat) DeleteDC(ctxcanvas->hDCMemPat);
  
  if (ctxcanvas->hOldBitmapStip) SelectObject(ctxcanvas->hDCMemStip, ctxcanvas->hOldBitmapStip);
  if (ctxcanvas->hBitmapStip) DeleteObject(ctxcanvas->hBitmapStip);
  if (ctxcanvas->hDCMemStip) DeleteDC(ctxcanvas->hDCMemStip);

  if (ctxcanvas->img_mask) DeleteObject(ctxcanvas->img_mask);
  
  SelectObject(ctxcanvas->hDC, ctxcanvas->hOldFont);
  DeleteObject(ctxcanvas->hFont);      /* Fonte */
  
  SelectObject(ctxcanvas->hDC, ctxcanvas->hOldPen);       /* restaura os objetos */
  DeleteObject(ctxcanvas->hPen);       /* Pen corrente */
  
  SelectObject(ctxcanvas->hDC, ctxcanvas->hOldBrush);     /* default do canvas   */
  DeleteObject(ctxcanvas->hBrush);   /* Brush corrente */
  
  DeleteObject(ctxcanvas->hNullPen);   /* Pen para tirar borda */
  DeleteObject(ctxcanvas->hBkBrush);     /* Brush para o background */
  
  /* ctxcanvas e ctxcanvas->hDC sao liberados em cada driver */
}

/*
Restaura os atributos do CD que sao guardados no DC do Windows quando
ha uma troca de DC. Usado pelos drivers Native Window e Printer.
*/
void cdwRestoreDC(cdCtxCanvas *ctxcanvas)
{
  /* cdClipArea */
  SelectClipRgn(ctxcanvas->hDC, ctxcanvas->clip_hrgn);
  
  /* cdForeground */
  SetTextColor(ctxcanvas->hDC, ctxcanvas->fg);

  /* cdBackground */
  SetBkColor(ctxcanvas->hDC, ctxcanvas->bg);

  /* cdBackOpacity */
  switch (ctxcanvas->canvas->back_opacity) 
  {
  case CD_TRANSPARENT:
    SetBkMode(ctxcanvas->hDC, TRANSPARENT);
    break;
  case CD_OPAQUE:
    SetBkMode(ctxcanvas->hDC, OPAQUE);
    break;
  }
  
  /* cdWriteMode */
  switch (ctxcanvas->canvas->write_mode) 
  {
  case CD_REPLACE:
    SetROP2(ctxcanvas->hDC, R2_COPYPEN);
    break;
  case CD_XOR:
    SetROP2(ctxcanvas->hDC, R2_XORPEN);
    break;
  case CD_NOT_XOR:
    SetROP2(ctxcanvas->hDC, R2_NOTXORPEN);
    break;
  }
  
  /* Text Alignment is calculated from this state */
  SetTextAlign(ctxcanvas->hDC,TA_LEFT|TA_BASELINE);
  
  /* cdLineStyle e cdLineWidth */
  ctxcanvas->hOldPen = SelectObject(ctxcanvas->hDC, ctxcanvas->hPen);
  
  /* cdInteriorStyle */
  ctxcanvas->hOldBrush = SelectObject(ctxcanvas->hDC, ctxcanvas->hBrush);

  /* cdFont */
  ctxcanvas->hOldFont = SelectObject(ctxcanvas->hDC, ctxcanvas->hFont);
}


/*********************************************************************/
/*
%S                            Cor                                    
*/
/*********************************************************************/

static long int sColorFromWindows(COLORREF color)
{
  return cdEncodeColor(GetRValue(color),GetGValue(color),GetBValue(color));
}

static COLORREF sColorToWindows(cdCtxCanvas* ctxcanvas, long int cd_color)
{
  unsigned char red,green,blue;
  COLORREF color;
  
  cdDecodeColor(cd_color,&red,&green,&blue);
  
  if (ctxcanvas->canvas->bpp <= 8)
    color=PALETTERGB((BYTE)red,(BYTE)green,(BYTE)blue);
  else
    color=RGB((BYTE)red,(BYTE)green,(BYTE)blue);
  
  return color;
}

static long int cdforeground (cdCtxCanvas* ctxcanvas, long int color)
{
  ctxcanvas->fg = sColorToWindows(ctxcanvas, color);
  SetTextColor(ctxcanvas->hDC, ctxcanvas->fg);
  ctxcanvas->rebuild_pen = 1;
  return color;
}

static void sCreatePen(cdCtxCanvas* ctxcanvas)
{
  int cd2win_cap[] =  {PS_ENDCAP_FLAT, PS_ENDCAP_SQUARE, PS_ENDCAP_ROUND};
  int cd2win_join[] = {PS_JOIN_MITER, PS_JOIN_BEVEL, PS_JOIN_ROUND};

  ctxcanvas->logPen.lopnColor = ctxcanvas->fg;
  
  if (ctxcanvas->hOldPen) SelectObject(ctxcanvas->hDC, ctxcanvas->hOldPen);
  if (ctxcanvas->hPen) DeleteObject(ctxcanvas->hPen);
  
  if (ctxcanvas->logPen.lopnWidth.x == 1)
  {
    LOGBRUSH LogBrush;
    LogBrush.lbStyle = BS_SOLID; 
    LogBrush.lbColor = ctxcanvas->logPen.lopnColor; 
    LogBrush.lbHatch = 0; 

    if (ctxcanvas->canvas->line_style == CD_CUSTOM)
    {
      ctxcanvas->hPen = ExtCreatePen(PS_COSMETIC | PS_USERSTYLE, 
                                            1, &LogBrush, 
                                            ctxcanvas->canvas->line_dashes_count, (DWORD*)ctxcanvas->canvas->line_dashes);
    }
    else
    {
      ctxcanvas->hPen = ExtCreatePen(PS_COSMETIC | ctxcanvas->logPen.lopnStyle, 
                                            1, &LogBrush, 
                                            0, NULL);
    }
  }
  else
  {
    int style = PS_GEOMETRIC;
    LOGBRUSH LogBrush;
    LogBrush.lbStyle = BS_SOLID; 
    LogBrush.lbColor = ctxcanvas->logPen.lopnColor; 
    LogBrush.lbHatch = 0; 

    style |= cd2win_cap[ctxcanvas->canvas->line_cap];
    style |= cd2win_join[ctxcanvas->canvas->line_join];
    
    if (ctxcanvas->canvas->line_style == CD_CUSTOM)
    {
      ctxcanvas->hPen = ExtCreatePen( PS_USERSTYLE | style, 
                                             ctxcanvas->logPen.lopnWidth.x, &LogBrush, 
                                             ctxcanvas->canvas->line_dashes_count, (DWORD*)ctxcanvas->canvas->line_dashes);
    }
    else
      ctxcanvas->hPen = ExtCreatePen( ctxcanvas->logPen.lopnStyle | style, 
                                             ctxcanvas->logPen.lopnWidth.x, &LogBrush, 
                                             0, NULL);
  }
  
  ctxcanvas->hOldPen = SelectObject(ctxcanvas->hDC, ctxcanvas->hPen);
  ctxcanvas->rebuild_pen = 0;
}

static int cdbackopacity (cdCtxCanvas* ctxcanvas, int opacity)
{
  switch (opacity)
  {
  case CD_TRANSPARENT:
    SetBkMode(ctxcanvas->hDC, TRANSPARENT);
    break;
  case CD_OPAQUE:
    SetBkMode(ctxcanvas->hDC, OPAQUE);
    break;
  }
  
  return opacity;
}

static int cdwritemode (cdCtxCanvas* ctxcanvas, int mode)
{
  switch (mode)
  {
  case CD_REPLACE:
    SetROP2(ctxcanvas->hDC, R2_COPYPEN);
    ctxcanvas->RopBlt = SRCCOPY;
    break;
  case CD_XOR:
    SetROP2(ctxcanvas->hDC, R2_XORPEN);
    ctxcanvas->RopBlt = SRCINVERT;
    break;
  case CD_NOT_XOR:
    SetROP2(ctxcanvas->hDC, R2_NOTXORPEN);
    ctxcanvas->RopBlt = SRCINVERT;
    break;
  }
  
  return mode;
}

static long int cdbackground (cdCtxCanvas* ctxcanvas, long int color)
{
  ctxcanvas->bg = sColorToWindows(ctxcanvas, color);
  SetBkColor(ctxcanvas->hDC, ctxcanvas->bg);
  
  if (ctxcanvas->hBkBrush) DeleteObject(ctxcanvas->hBkBrush);
  ctxcanvas->hBkBrush  = CreateSolidBrush(ctxcanvas->bg);
  
  return color;
}

static void cdpalette(cdCtxCanvas* ctxcanvas, int n, const long int *palette, int mode)
{
  LOGPALETTE* pLogPal;      
  unsigned char  red,green,blue;
  int k, np = n;
  (void)mode;
  
  if (ctxcanvas->canvas->bpp > 8) /* se o sistema for true color */
    return; 
  
  if (n < 246)
    np += 10;
  
  pLogPal = (LOGPALETTE*)malloc(sizeof(LOGPALETTE) + np * sizeof(PALETTEENTRY));
  pLogPal->palVersion    = 0x300; 
  pLogPal->palNumEntries = (WORD)np;
  
  if (n < 246)
  {
    k = 10;
    GetSystemPaletteEntries(ctxcanvas->hDC, 0, 10, pLogPal->palPalEntry);
  }
  else
    k=0;
  
  for (; k < np; k++) 
  {
    cdDecodeColor(palette[k],&red,&green,&blue);
    
    pLogPal->palPalEntry[k].peRed   = (BYTE)red;
    pLogPal->palPalEntry[k].peGreen = (BYTE)green;
    pLogPal->palPalEntry[k].peBlue  = (BYTE)blue;
    pLogPal->palPalEntry[k].peFlags = PC_NOCOLLAPSE;
  }
  
  if (ctxcanvas->hPal)
  {
    if (ctxcanvas->hOldPal) SelectPalette(ctxcanvas->hDC, ctxcanvas->hOldPal, FALSE);
    DeleteObject(ctxcanvas->hPal); 
  }
  
  ctxcanvas->hPal = CreatePalette(pLogPal);
  ctxcanvas->hOldPal = SelectPalette(ctxcanvas->hDC, ctxcanvas->hPal, FALSE);
  
  RealizePalette(ctxcanvas->hDC);
  
  free(pLogPal);
}


/*********************************************************************/
/*
%S                 Canvas e clipping
*/
/*********************************************************************/

static HRGN sClipRect(cdCtxCanvas* ctxcanvas)
{
  HRGN clip_hrgn;

  if (ctxcanvas->clip_hrgn)
    DeleteObject(ctxcanvas->clip_hrgn);

  clip_hrgn = CreateRectRgn(ctxcanvas->canvas->clip_rect.xmin, ctxcanvas->canvas->clip_rect.ymin,
                            ctxcanvas->canvas->clip_rect.xmax+1, ctxcanvas->canvas->clip_rect.ymax+1);
  
  SelectClipRgn(ctxcanvas->hDC, clip_hrgn);
  return clip_hrgn;
}

static HRGN sClipPoly(cdCtxCanvas* ctxcanvas)
{
  HRGN clip_hrgn;

  if (ctxcanvas->clip_hrgn)
    DeleteObject(ctxcanvas->clip_hrgn);

  clip_hrgn = CreatePolygonRgn(ctxcanvas->clip_pnt, 
                               ctxcanvas->clip_pnt_n, 
                               ctxcanvas->canvas->fill_mode==CD_EVENODD?ALTERNATE:WINDING);
  SelectClipRgn(ctxcanvas->hDC, clip_hrgn);
  return clip_hrgn;
}

static int cdclip (cdCtxCanvas* ctxcanvas, int clip_mode)
{
  if (ctxcanvas->wtype == CDW_WMF)
    return clip_mode;

  switch (clip_mode) 
  {
  case CD_CLIPOFF:
    SelectClipRgn(ctxcanvas->hDC, NULL);   /* toda 'area do canvas */
    if (ctxcanvas->clip_hrgn)
      DeleteObject(ctxcanvas->clip_hrgn);
    ctxcanvas->clip_hrgn = NULL;
    break;
  case CD_CLIPAREA:
    ctxcanvas->clip_hrgn = sClipRect(ctxcanvas);
    break;
  case CD_CLIPPOLYGON:
    ctxcanvas->clip_hrgn = sClipPoly(ctxcanvas);
    break;
  case CD_CLIPREGION:
    if (ctxcanvas->clip_hrgn)
      DeleteObject(ctxcanvas->clip_hrgn);
    ctxcanvas->clip_hrgn = CreateRectRgn(0,0,0,0);
    CombineRgn(ctxcanvas->clip_hrgn, ctxcanvas->new_rgn, NULL, RGN_COPY);
    SelectClipRgn(ctxcanvas->hDC, ctxcanvas->clip_hrgn);
    break;
  }

  return clip_mode;
}

static void cdcliparea(cdCtxCanvas* ctxcanvas, int xmin, int xmax, int ymin, int ymax)
{
  if (ctxcanvas->wtype == CDW_WMF)
    return;

  if (ctxcanvas->canvas->clip_mode == CD_CLIPAREA) 
  {
    ctxcanvas->canvas->clip_rect.xmin = xmin;
    ctxcanvas->canvas->clip_rect.xmax = xmax;
    ctxcanvas->canvas->clip_rect.ymin = ymin;
    ctxcanvas->canvas->clip_rect.ymax = ymax;
    ctxcanvas->clip_hrgn = sClipRect(ctxcanvas);
  }
}

static void cdnewregion(cdCtxCanvas* ctxcanvas)
{
  if (ctxcanvas->new_rgn)
    DeleteObject(ctxcanvas->new_rgn);
  ctxcanvas->new_rgn = CreateRectRgn(0, 0, 0, 0); 
}

static int cdispointinregion(cdCtxCanvas* ctxcanvas, int x, int y)
{
  if (!ctxcanvas->new_rgn)
    return 0;

  if (PtInRegion(ctxcanvas->new_rgn, x, y))
    return 1;

  return 0;
}

static void cdoffsetregion(cdCtxCanvas* ctxcanvas, int x, int y)
{
  if (!ctxcanvas->new_rgn)
    return;

  OffsetRgn(ctxcanvas->new_rgn, x, y);
}

static void cdgetregionbox(cdCtxCanvas* ctxcanvas, int *xmin, int *xmax, int *ymin, int *ymax)
{
  RECT rect;

  if (!ctxcanvas->new_rgn)
    return;

  GetRgnBox(ctxcanvas->new_rgn, &rect);

  /* RECT in Windows does not includes the right, bottom. */
  *xmin = rect.left;
  *xmax = rect.right-1;
  *ymin = rect.top;
  *ymax = rect.bottom-1;
}

/******************************************************************/
/*
%S                  Primitivas e seus atributos                        
*/
/******************************************************************/

static int cdlinestyle (cdCtxCanvas* ctxcanvas, int style)
{
  switch (style)
  {
  case CD_CONTINUOUS:
    ctxcanvas->logPen.lopnStyle = PS_SOLID;
    break;
  case CD_DASHED:                
    ctxcanvas->logPen.lopnStyle = PS_DASH;
    break;
  case CD_DOTTED:                  
    ctxcanvas->logPen.lopnStyle = PS_DOT;
    break;
  case CD_DASH_DOT:               
    ctxcanvas->logPen.lopnStyle = PS_DASHDOT;
    break;
  case CD_DASH_DOT_DOT:        
    ctxcanvas->logPen.lopnStyle = PS_DASHDOTDOT;
    break;
  }
  
  ctxcanvas->rebuild_pen = 1;
  
  return style;
}

static int cdlinewidth (cdCtxCanvas* ctxcanvas, int width)
{
  ctxcanvas->logPen.lopnWidth.x = width;
  ctxcanvas->rebuild_pen = 1;
  return width;
}

static int cdlinecap (cdCtxCanvas* ctxcanvas, int cap)
{
  ctxcanvas->rebuild_pen = 1;
  return cap;
}

static int cdlinejoin (cdCtxCanvas* ctxcanvas, int join)
{
  ctxcanvas->rebuild_pen = 1;
  return join;
}

static int cdhatch (cdCtxCanvas* ctxcanvas, int hatch_style)
{
  switch (hatch_style)
  {
  case CD_HORIZONTAL:
    ctxcanvas->logBrush.lbHatch = HS_HORIZONTAL;
    break;
  case CD_VERTICAL:
    ctxcanvas->logBrush.lbHatch = HS_VERTICAL;
    break;
  case CD_FDIAGONAL:
    ctxcanvas->logBrush.lbHatch = HS_FDIAGONAL;
    break;
  case CD_BDIAGONAL:
    ctxcanvas->logBrush.lbHatch = HS_BDIAGONAL;
    break;
  case CD_CROSS:
    ctxcanvas->logBrush.lbHatch = HS_CROSS;
    break;
  case CD_DIAGCROSS:
    ctxcanvas->logBrush.lbHatch = HS_DIAGCROSS;
    break;
  }
  
  ctxcanvas->logBrush.lbColor=ctxcanvas->fg;
  ctxcanvas->logBrush.lbStyle=BS_HATCHED;
  
  if (ctxcanvas->hOldBrush) SelectObject(ctxcanvas->hDC, ctxcanvas->hOldBrush);
  if (ctxcanvas->hBrush) DeleteObject(ctxcanvas->hBrush);

  ctxcanvas->hBrush = CreateBrushIndirect(&ctxcanvas->logBrush);
  ctxcanvas->hOldBrush = SelectObject(ctxcanvas->hDC, ctxcanvas->hBrush);
  
  return hatch_style;
}

static HBITMAP Stipple2Bitmap(int w, int h, const unsigned char *index, int negative)
{
  HBITMAP hBitmap;
  BYTE *buffer;
  
  int nb;  /* number of bytes per line */
  int x,y,k,offset;
  
  /* Cria um bitmap com os indices dados */
  nb = ((w + 15) / 16) * 2; /* Must be in a word boundary. */
  buffer = (BYTE *) malloc (nb*h);
  memset(buffer, 0xff, nb*h);
  
  for (y=0; y<h; y++)
  {
    k=y*nb;
    offset = ((h - 1) - y)*w;  /* always consider a top-down bitmap */
    
    for (x=0;x<w;x++)
    {
      if ((x % 8 == 0) && (x != 0)) 
        k++; 
      
      /* In Windows: 0 is foreground, 1 is background. */
      if (index[offset + x] != 0)
        buffer[k] &= (BYTE)~(1 << (7 - x % 8));
    }
  }

  if (negative)
  {
    for (k = 0; k < nb*h; k++)
      buffer[k] = ~buffer[k];
  }
  
  hBitmap = CreateBitmap(w,h,1,1,(LPSTR)buffer);
  
  free(buffer);

  return hBitmap;
}

static void cdstipple(cdCtxCanvas* ctxcanvas, int w, int h, const unsigned char *index)
{
  HBITMAP hBitmap = Stipple2Bitmap(w, h, index, 0);
  
  /* Cria um pincel com o Bitmap */
  if (ctxcanvas->hOldBrush) SelectObject(ctxcanvas->hDC, ctxcanvas->hOldBrush);
  if (ctxcanvas->hBrush) DeleteObject(ctxcanvas->hBrush);

  ctxcanvas->hBrush = CreatePatternBrush(hBitmap);
  ctxcanvas->hOldBrush = SelectObject(ctxcanvas->hDC, ctxcanvas->hBrush);
 
  DeleteObject(hBitmap);
}

static void set_dib_res(cdwDIB* dib, cdCtxCanvas* ctxcanvas)
{
  dib->bmih->biXPelsPerMeter = (LONG)(ctxcanvas->canvas->xres*1000);
  dib->bmih->biYPelsPerMeter = (LONG)(ctxcanvas->canvas->yres*1000);
}

static void cdpattern(cdCtxCanvas* ctxcanvas, int w, int h, const long int *colors)
{
  cdwDIB dib;
  HBRUSH hBrush;
  
  if (ctxcanvas->wtype == CDW_WMF)
    return;

  dib.w = w;
  dib.h = h;
  dib.type = 0; 
  if (!cdwCreateDIB(&dib))
    return;

  /* trying to preserve pattern size during printing */
  set_dib_res(&dib, ctxcanvas);

  cdwDIBEncodePattern(&dib, colors);
  hBrush = CreateDIBPatternBrushPt(dib.dib, DIB_RGB_COLORS);
  cdwKillDIB(&dib);

  if (hBrush)
  {
    if (ctxcanvas->hOldBrush) SelectObject(ctxcanvas->hDC, ctxcanvas->hOldBrush);
    if (ctxcanvas->hBrush) DeleteObject(ctxcanvas->hBrush);

    ctxcanvas->hBrush = hBrush;
    ctxcanvas->hOldBrush = SelectObject(ctxcanvas->hDC, ctxcanvas->hBrush);
  }
}

static int cdinteriorstyle (cdCtxCanvas* ctxcanvas, int style)
{
  switch (style)
  {
  case CD_SOLID:
    ctxcanvas->logBrush.lbStyle=BS_SOLID;
    ctxcanvas->logBrush.lbColor=ctxcanvas->fg;

    if (ctxcanvas->hOldBrush) SelectObject(ctxcanvas->hDC, ctxcanvas->hOldBrush);
    if (ctxcanvas->hBrush) DeleteObject(ctxcanvas->hBrush);

    ctxcanvas->hBrush = CreateBrushIndirect(&ctxcanvas->logBrush);
    ctxcanvas->hOldBrush = SelectObject(ctxcanvas->hDC, ctxcanvas->hBrush);
    break;
    /* the remaining styles must recreate the current brush */
  case CD_HATCH:
    cdhatch(ctxcanvas, ctxcanvas->canvas->hatch_style);
    break;
  case CD_STIPPLE:
    cdstipple(ctxcanvas, ctxcanvas->canvas->stipple_w, ctxcanvas->canvas->stipple_h, ctxcanvas->canvas->stipple);
    break;
  case CD_PATTERN:
    if (ctxcanvas->wtype == CDW_WMF)
      return style;
    cdpattern(ctxcanvas, ctxcanvas->canvas->pattern_w, ctxcanvas->canvas->pattern_h, ctxcanvas->canvas->pattern);
    break;
  }
  
  return style;
}

static void sUpdateFill(cdCtxCanvas* ctxcanvas, int fill)
{
  if (fill)
  {
    if ((ctxcanvas->logBrush.lbColor != ctxcanvas->fg) && 
      (ctxcanvas->canvas->interior_style != CD_PATTERN)) 
      cdinteriorstyle(ctxcanvas, ctxcanvas->canvas->interior_style);
  }
  else
  {
    if (ctxcanvas->rebuild_pen) 
      sCreatePen(ctxcanvas);
  }
}

/*******************************************************************************/

static void cdline (cdCtxCanvas* ctxcanvas, int x1, int y1, int x2, int y2)
{
  sUpdateFill(ctxcanvas, 0);
  
  MoveToEx( ctxcanvas->hDC, x1, y1, NULL );
  LineTo( ctxcanvas->hDC, x2, y2 );
  SetPixelV(ctxcanvas->hDC, x2, y2, ctxcanvas->fg);
}

static void cdrect (cdCtxCanvas* ctxcanvas, int xmin, int xmax, int ymin, int ymax)
{
  HBRUSH oldBrush;
  
  sUpdateFill(ctxcanvas, 0);
  
  oldBrush = SelectObject(ctxcanvas->hDC, GetStockObject(NULL_BRUSH));  /* tira o desenho do interior */
  Rectangle(ctxcanvas->hDC, xmin, ymin, xmax+1, ymax+1);     /* +1 porque nao inclue right/bottom */
  SelectObject(ctxcanvas->hDC, oldBrush);                    /* restaura o brush corrente */
}

static void cdbox (cdCtxCanvas* ctxcanvas, int xmin, int xmax, int ymin, int ymax)
{
  sUpdateFill(ctxcanvas, 1);
  
  if (ctxcanvas->canvas->new_region)
  {
    HRGN rgn = CreateRectRgn(xmin, ymin, xmax+1, ymax+1); 
    CombineRgn(ctxcanvas->new_rgn, ctxcanvas->new_rgn, rgn, sCombineRegion2win[ctxcanvas->canvas->combine_mode]);
    DeleteObject(rgn);
  }
  else
  {
    SelectObject(ctxcanvas->hDC, ctxcanvas->hNullPen);  /* tira o desenho da borda */
    Rectangle(ctxcanvas->hDC, xmin, ymin, xmax+2, ymax+2);     /* +2 porque a pena e' NULL NULL e o nao inclue right/bottom */
    SelectObject(ctxcanvas->hDC, ctxcanvas->hPen);      /* restaura a Pen corrente */
  }
}

typedef struct _winArcParam
{
  int LeftRect,	 /* x-coordinate of upper-left corner of bounding rectangle */
    TopRect,	   /* y-coordinate of upper-left corner of bounding rectangle */
    RightRect,	 /* x-coordinate of lower-right corner of bounding rectangle */
    BottomRect,	 /* y-coordinate of lower-right corner of bounding rectangle */
    XStartArc,	 /* first radial ending point */
    YStartArc,	 /* first radial ending point */
    XEndArc,	   /* second radial ending point */
    YEndArc; 	   /* second radial ending point */
} winArcParam;

static void sCalcArc(cdCanvas* canvas, int xc, int yc, int w, int h, double a1, double a2, winArcParam* arc, int swap)
{
  /* computation is done as if the angles are counter-clockwise, 
     and yaxis is NOT inverted. */

  arc->LeftRect   = xc - w/2;
  arc->TopRect    = yc - h/2;
  arc->RightRect  = xc + w/2 + 1;
  arc->BottomRect = yc + h/2 + 1;

  /* GDI orientation is the same as CD */

  if (!canvas->invert_yaxis)
    _cdSwapInt(arc->BottomRect, arc->TopRect);  /* not necessary, but done for clarity */

  cdCanvasGetArcStartEnd(xc, yc, w, h, a1, a2, &(arc->XStartArc), &(arc->YStartArc), &(arc->XEndArc), &(arc->YEndArc));

  if (canvas->invert_yaxis)
  {
    /* fix axis orientation */
    arc->YStartArc = 2*yc - arc->YStartArc;
    arc->YEndArc = 2*yc - arc->YEndArc;
  }
  else
  {
    /* it is clock-wise when axis NOT inverted */
    if (swap)
    {
      _cdSwapInt(arc->XStartArc, arc->XEndArc);
      _cdSwapInt(arc->YStartArc, arc->YEndArc);
    }
  }
}

static void cdarc(cdCtxCanvas* ctxcanvas, int xc, int yc, int w, int h, double angle1, double angle2)
{
  winArcParam arc;
  sCalcArc(ctxcanvas->canvas, xc, yc, w, h, angle1, angle2, &arc, 1);
  
  sUpdateFill(ctxcanvas, 0);
  
  Arc(ctxcanvas->hDC, arc.LeftRect, arc.TopRect, arc.RightRect, arc.BottomRect, arc.XStartArc, arc.YStartArc, arc.XEndArc, arc.YEndArc);
}

static void cdsector(cdCtxCanvas* ctxcanvas, int xc, int yc, int w, int h, double angle1, double angle2)
{
  winArcParam arc;
  sCalcArc(ctxcanvas->canvas, xc, yc, w, h, angle1, angle2, &arc, 1);

  sUpdateFill(ctxcanvas, 1);
  
  if (angle1==0 && angle2==360)
  {
    if (ctxcanvas->canvas->new_region)
    {
      HRGN rgn = CreateEllipticRgn(arc.LeftRect, arc.TopRect, arc.RightRect+1, arc.BottomRect+1); 
      CombineRgn(ctxcanvas->new_rgn, ctxcanvas->new_rgn, rgn, sCombineRegion2win[ctxcanvas->canvas->combine_mode]);
      DeleteObject(rgn);
    }
    else
    {
      SelectObject(ctxcanvas->hDC, ctxcanvas->hNullPen);  /* tira o desenho da borda */
      Ellipse(ctxcanvas->hDC,arc.LeftRect, arc.TopRect, arc.RightRect+1, arc.BottomRect+1);  /* +1 porque a pena e' NULL e +1 porque nao inclue right/bottom */
      SelectObject(ctxcanvas->hDC, ctxcanvas->hPen);      /* restaura a Pen corrente */
    }
  }
  else
  {
    if (ctxcanvas->canvas->new_region)
      BeginPath(ctxcanvas->hDC);
   
    SelectObject(ctxcanvas->hDC, ctxcanvas->hNullPen);  /* tira o desenho da borda */
    Pie(ctxcanvas->hDC,arc.LeftRect, arc.TopRect, arc.RightRect+1, arc.BottomRect+1,arc.XStartArc, arc.YStartArc, arc.XEndArc, arc.YEndArc); /* +1 porque a pena e' NULL e +1 porque nao inclue right/bottom */
    SelectObject(ctxcanvas->hDC, ctxcanvas->hPen);      /* restaura a Pen corrente */

    if (ctxcanvas->canvas->new_region)
    {
      HRGN rgn;
      EndPath(ctxcanvas->hDC);
      rgn = PathToRegion(ctxcanvas->hDC);
      CombineRgn(ctxcanvas->new_rgn, ctxcanvas->new_rgn, rgn, sCombineRegion2win[ctxcanvas->canvas->combine_mode]);
      DeleteObject(rgn);
    }
  }
}

static void cdchord(cdCtxCanvas* ctxcanvas, int xc, int yc, int w, int h, double angle1, double angle2)
{
  winArcParam arc;
  sCalcArc(ctxcanvas->canvas, xc, yc, w, h, angle1, angle2, &arc, 1);

  sUpdateFill(ctxcanvas, 1);
  
  if (angle1==0 && angle2==360)
  {
    if (ctxcanvas->canvas->new_region)
    {
      HRGN rgn = CreateEllipticRgn(arc.LeftRect, arc.TopRect, arc.RightRect+1, arc.BottomRect+1); 
      CombineRgn(ctxcanvas->new_rgn, ctxcanvas->new_rgn, rgn, sCombineRegion2win[ctxcanvas->canvas->combine_mode]);
      DeleteObject(rgn);
    }
    else
    {
      SelectObject(ctxcanvas->hDC, ctxcanvas->hNullPen);  /* tira o desenho da borda */
      Ellipse(ctxcanvas->hDC,arc.LeftRect, arc.TopRect, arc.RightRect+1, arc.BottomRect+1);   /* +1 porque a pena e' NULL e +1 porque nao inclue right/bottom */
      SelectObject(ctxcanvas->hDC, ctxcanvas->hPen);      /* restaura a Pen corrente */
    }
  }
  else
  {
    if (ctxcanvas->canvas->new_region)
      BeginPath(ctxcanvas->hDC);
    
    SelectObject(ctxcanvas->hDC, ctxcanvas->hNullPen);  /* tira o desenho da borda */
    Chord(ctxcanvas->hDC,arc.LeftRect, arc.TopRect, arc.RightRect+1, arc.BottomRect+1,arc.XStartArc, arc.YStartArc, arc.XEndArc, arc.YEndArc); /* +2 porque a pena e' NULL e o nao inclue right/bottom */
    SelectObject(ctxcanvas->hDC, ctxcanvas->hPen);      /* restaura a Pen corrente */

    if (ctxcanvas->canvas->new_region)
    {
      HRGN rgn;
      EndPath(ctxcanvas->hDC);
      rgn = PathToRegion(ctxcanvas->hDC);
      CombineRgn(ctxcanvas->new_rgn, ctxcanvas->new_rgn, rgn, sCombineRegion2win[ctxcanvas->canvas->combine_mode]);
      DeleteObject(rgn);
    }
  }
}

static void cdpoly(cdCtxCanvas* ctxcanvas, int mode, cdPoint* poly, int n)
{
  int i, t, nc;
  POINT* pnt;
  HPEN oldPen = NULL, Pen = NULL;

  if (mode == CD_PATH)
  {
    int p, current_set;

    /* if there is any current path, remove it */
    BeginPath(ctxcanvas->hDC);
    current_set = 0;

    i = 0;
    for (p=0; p<ctxcanvas->canvas->path_n; p++)
    {
      switch(ctxcanvas->canvas->path[p])
      {
      case CD_PATH_NEW:
        EndPath(ctxcanvas->hDC);
        BeginPath(ctxcanvas->hDC);
        current_set = 0;
        break;
      case CD_PATH_MOVETO:
        if (i+1 > n) return;
        MoveToEx(ctxcanvas->hDC, poly[i].x, poly[i].y, NULL);
        current_set = 1;
        i++;
        break;
      case CD_PATH_LINETO:
        if (i+1 > n) return;
        LineTo(ctxcanvas->hDC, poly[i].x, poly[i].y);
        current_set = 1;
        i++;
        break;
      case CD_PATH_ARC:
        {
          int xc, yc, w, h, old_arcmode = 0;
          double a1, a2;
          winArcParam arc;

          if (i+3 > n) return;

          if (!cdCanvasGetArcPath(ctxcanvas->canvas, poly+i, &xc, &yc, &w, &h, &a1, &a2)) 
            return;

          sCalcArc(ctxcanvas->canvas, xc, yc, w, h, a1, a2, &arc, 0);

          if (current_set)
            LineTo(ctxcanvas->hDC, arc.XStartArc, arc.YStartArc);
          
          if ((a2-a1)<0) /* can be clockwise */
          {
            /* Arc behave diferent when GM_ADVANCED is set */
            old_arcmode = SetArcDirection(ctxcanvas->hDC, ctxcanvas->canvas->invert_yaxis? AD_CLOCKWISE: AD_COUNTERCLOCKWISE);
          }

          ArcTo(ctxcanvas->hDC, arc.LeftRect, arc.TopRect, arc.RightRect, arc.BottomRect, arc.XStartArc, arc.YStartArc, arc.XEndArc, arc.YEndArc);

          if (old_arcmode) /* restore */
            SetArcDirection(ctxcanvas->hDC, old_arcmode);

          current_set = 1;

          i += 3;
        }
        break;
      case CD_PATH_CURVETO:
        if (i+3 > n) return;
        PolyBezierTo(ctxcanvas->hDC, (POINT*)(poly + i), 3);
        current_set = 1;
        i += 3;
        break;
      case CD_PATH_CLOSE:
        CloseFigure(ctxcanvas->hDC);
        break;
      case CD_PATH_FILL:
        sUpdateFill(ctxcanvas, 1);
        SetPolyFillMode(ctxcanvas->hDC, ctxcanvas->canvas->fill_mode==CD_EVENODD?ALTERNATE:WINDING);
        EndPath(ctxcanvas->hDC);
        FillPath(ctxcanvas->hDC);
        break;
      case CD_PATH_STROKE:
        sUpdateFill(ctxcanvas, 0);
        EndPath(ctxcanvas->hDC);
        StrokePath(ctxcanvas->hDC);
        break;
      case CD_PATH_FILLSTROKE:
        sUpdateFill(ctxcanvas, 1);
        sUpdateFill(ctxcanvas, 0);
        SetPolyFillMode(ctxcanvas->hDC, ctxcanvas->canvas->fill_mode==CD_EVENODD?ALTERNATE:WINDING);
        EndPath(ctxcanvas->hDC);
        StrokeAndFillPath(ctxcanvas->hDC);
        break;
      case CD_PATH_CLIP:
        SetPolyFillMode(ctxcanvas->hDC, ctxcanvas->canvas->fill_mode==CD_EVENODD?ALTERNATE:WINDING);
        EndPath(ctxcanvas->hDC);
        SelectClipPath(ctxcanvas->hDC, RGN_AND);
        break;
      }
    }
    return;
  }

  switch( mode )
  {
  case CD_CLOSED_LINES:
    poly[n].x = poly[0].x;
    poly[n].y = poly[0].y;
    n++;
    /* continua */
  case CD_OPEN_LINES:
    sUpdateFill(ctxcanvas, 0);
    Polyline(ctxcanvas->hDC, (POINT*)poly, n);
    break;
  case CD_BEZIER:
    sUpdateFill(ctxcanvas, 0);
    PolyBezier(ctxcanvas->hDC, (POINT*)poly, n);
    break;
  case CD_FILL:
    poly[n].x = poly[0].x;
    poly[n].y = poly[0].y;
    n++;
    if (ctxcanvas->canvas->new_region)
    {
      HRGN rgn = CreatePolygonRgn((POINT*)poly, n, ctxcanvas->canvas->fill_mode==CD_EVENODD?ALTERNATE:WINDING); 
      CombineRgn(ctxcanvas->new_rgn, ctxcanvas->new_rgn, rgn, sCombineRegion2win[ctxcanvas->canvas->combine_mode]);
      DeleteObject(rgn);
    }
    else 
    {
      sUpdateFill(ctxcanvas, 1);
      
      if (ctxcanvas->canvas->interior_style != CD_SOLID || ctxcanvas->fill_attrib[0] == '0') 
        SelectObject(ctxcanvas->hDC, ctxcanvas->hNullPen);  /* tira o desenho da borda */
      else
      {
        Pen = CreatePen(PS_SOLID, 1, ctxcanvas->fg);
        oldPen = SelectObject(ctxcanvas->hDC, Pen);
      }

      SetPolyFillMode(ctxcanvas->hDC, ctxcanvas->canvas->fill_mode==CD_EVENODD?ALTERNATE:WINDING);
      Polygon(ctxcanvas->hDC, (POINT*)poly, n);

      if (ctxcanvas->canvas->interior_style != CD_SOLID || ctxcanvas->fill_attrib[0] == '0')
        SelectObject(ctxcanvas->hDC, ctxcanvas->hPen);      /* restaura a Pen corrente */
      else
      {
        SelectObject(ctxcanvas->hDC, oldPen);
        DeleteObject(Pen);
      }
    }
    break;
  case CD_CLIP:
    poly[n].x = poly[0].x;
    poly[n].y = poly[0].y;
    n++;
    
    if (ctxcanvas->wtype == CDW_WMF)
      return;

    if (ctxcanvas->clip_pnt)
      free(ctxcanvas->clip_pnt);
    
    ctxcanvas->clip_pnt = (POINT*)malloc(n*sizeof(POINT));

    pnt = (POINT*)poly;
    t = n;
    nc = 1;

    ctxcanvas->clip_pnt[0] = *pnt;
    pnt++;

    for (i = 1; i < t-1; i++, pnt++)
    {
      if (!((pnt->x == ctxcanvas->clip_pnt[nc-1].x && pnt->x == (pnt + 1)->x) || 
            (pnt->y == ctxcanvas->clip_pnt[nc-1].y && pnt->y == (pnt + 1)->y)))
      {
        ctxcanvas->clip_pnt[nc] = *pnt;
        nc++;
      }
    }

    ctxcanvas->clip_pnt_n = nc;
    
    if (ctxcanvas->canvas->clip_mode == CD_CLIPPOLYGON) 
      ctxcanvas->clip_hrgn = sClipPoly(ctxcanvas);
    
    break;
  }
}

static void cdtransform(cdCtxCanvas *ctxcanvas, const double* matrix)
{
  if (matrix)
  {
    XFORM xForm;
    SetGraphicsMode(ctxcanvas->hDC, GM_ADVANCED);
    ModifyWorldTransform(ctxcanvas->hDC, NULL, MWT_IDENTITY);

    /* configure a bottom-up coordinate system */

    /* Equivalent of:
    SetMapMode(ctxcanvas->hDC, MM_ISOTROPIC); 
    SetWindowExtEx(ctxcanvas->hDC, ctxcanvas->canvas->w-1, ctxcanvas->canvas->h-1, NULL); 
    SetWindowOrgEx(ctxcanvas->hDC, 0, 0, NULL);
    SetViewportExtEx(ctxcanvas->hDC, ctxcanvas->canvas->w-1, -(ctxcanvas->canvas->h-1), NULL); 
    SetViewportOrgEx(ctxcanvas->hDC, 0, ctxcanvas->canvas->h-1, NULL);
    */

    xForm.eM11 = (FLOAT)1; 
    xForm.eM12 = (FLOAT)0;
    xForm.eM21 = (FLOAT)0; 
    xForm.eM22 = (FLOAT)-1; 
    xForm.eDx  = (FLOAT)0; 
    xForm.eDy  = (FLOAT)(ctxcanvas->canvas->h-1); 
    ModifyWorldTransform(ctxcanvas->hDC, &xForm, MWT_LEFTMULTIPLY);

    ctxcanvas->canvas->invert_yaxis = 0;

    xForm.eM11 = (FLOAT)matrix[0]; 
    xForm.eM12 = (FLOAT)matrix[1];
    xForm.eM21 = (FLOAT)matrix[2]; 
    xForm.eM22 = (FLOAT)matrix[3]; 
    xForm.eDx  = (FLOAT)matrix[4]; 
    xForm.eDy  = (FLOAT)matrix[5]; 
    ModifyWorldTransform(ctxcanvas->hDC, &xForm, MWT_LEFTMULTIPLY);
  }
  else
  {
    ctxcanvas->canvas->invert_yaxis = 1;

    if (ctxcanvas->rotate_angle)
    {
      XFORM xForm;

      /* the rotation  must be corrected because of the Y axis orientation */

      SetGraphicsMode(ctxcanvas->hDC, GM_ADVANCED);
      ModifyWorldTransform(ctxcanvas->hDC, NULL, MWT_IDENTITY);

      xForm.eM11 = (FLOAT) cos(-CD_DEG2RAD*ctxcanvas->rotate_angle); 
      xForm.eM12 = (FLOAT) sin(-CD_DEG2RAD*ctxcanvas->rotate_angle); 
      xForm.eM21 = (FLOAT) -xForm.eM12; 
      xForm.eM22 = (FLOAT) xForm.eM11; 
      xForm.eDx  = (FLOAT) ctxcanvas->rotate_center_x; 
      xForm.eDy  = (FLOAT) _cdInvertYAxis(ctxcanvas->canvas, ctxcanvas->rotate_center_y); 
      ModifyWorldTransform(ctxcanvas->hDC, &xForm, MWT_LEFTMULTIPLY);

      xForm.eM11 = (FLOAT) 1; 
      xForm.eM12 = (FLOAT) 0; 
      xForm.eM21 = (FLOAT) 0; 
      xForm.eM22 = (FLOAT) 1; 
      xForm.eDx  = (FLOAT) -ctxcanvas->rotate_center_x; 
      xForm.eDy  = (FLOAT) -_cdInvertYAxis(ctxcanvas->canvas, ctxcanvas->rotate_center_y); 
      ModifyWorldTransform(ctxcanvas->hDC, &xForm, MWT_LEFTMULTIPLY);
    }
    else
    {
      ModifyWorldTransform(ctxcanvas->hDC, NULL, MWT_IDENTITY);
      SetGraphicsMode(ctxcanvas->hDC, GM_COMPATIBLE);
    }
  }
}

static void sTextOutBlt(cdCtxCanvas* ctxcanvas, int px, int py, const char* s, int len)
{
  HDC hBitmapDC;
  HBITMAP hBitmap, hOldBitmap;
  HFONT hOldFont;
  int w, h, wt, ht, x, y, off, px_off = 0, py_off = 0;
  double teta = ctxcanvas->canvas->text_orientation*CD_DEG2RAD;
  double cos_teta = cos(teta);
  double sin_teta = sin(teta);
  
  cdgettextsize(ctxcanvas, s, len, &w, &h);
  wt = w;
  ht = h;

  if (ctxcanvas->canvas->text_orientation != 0)
  {
    /* novo tamanho da imagem */
    w = (int)(w * cos_teta + h * sin_teta);
    h = (int)(h * cos_teta + w * sin_teta);
  }

  /* coloca no centro da imagem */
  y = h/2;
  x = w/2; 

  /* corrige alinhamento do centro */
  off = ht/2 - ctxcanvas->font.descent;
  if (ctxcanvas->canvas->text_orientation != 0)
  {
    y += (int)(off * cos_teta);
    x += (int)(off * sin_teta);
  }
  else
    y += off;

  /* calcula o alinhamento da imagem no canvas */
  if (ctxcanvas->canvas->text_orientation != 0)
  {
    switch (ctxcanvas->canvas->text_alignment)
    {
    case CD_CENTER:
      py_off = 0;
      px_off = 0;
      break;
    case CD_BASE_LEFT:
      py_off = - (int)(off * cos_teta + w/2 * sin_teta);
      px_off =   (int)(w/2 * cos_teta - off * sin_teta);         
      break;
    case CD_BASE_CENTER:
      py_off = - (int)(off * cos_teta);
      px_off = - (int)(off * sin_teta);
      break;
    case CD_BASE_RIGHT:
      py_off = - (int)(off * cos_teta - w/2 * sin_teta);
      px_off = - (int)(w/2 * cos_teta + off * sin_teta);
      break;
    case CD_NORTH:
      py_off = (int)(ht/2 * cos_teta);
      px_off = (int)(ht/2 * sin_teta);  
      break;
    case CD_SOUTH:
      py_off = - (int)(ht/2 * cos_teta);
      px_off = - (int)(ht/2 * sin_teta);  
      break;
    case CD_EAST:
      py_off =   (int)(wt/2 * sin_teta);
      px_off = - (int)(wt/2 * cos_teta);
      break;
    case CD_WEST:
      py_off = - (int)(wt/2 * sin_teta);
      px_off =   (int)(wt/2 * cos_teta);         
      break;
    case CD_NORTH_EAST:
      py_off = (int)(ht/2 * cos_teta + wt/2 * sin_teta);
      px_off = (int)(ht/2 * sin_teta - wt/2 * cos_teta);  
      break;
    case CD_SOUTH_WEST:
      py_off = - (int)(ht/2 * cos_teta + wt/2 * sin_teta);
      px_off = - (int)(ht/2 * sin_teta - wt/2 * cos_teta);  
      break;
    case CD_NORTH_WEST:
      py_off = (int)(ht/2 * cos_teta - wt/2 * sin_teta);
      px_off = (int)(ht/2 * sin_teta + wt/2 * cos_teta);  
      break;
    case CD_SOUTH_EAST:
      py_off = - (int)(ht/2 * cos_teta - wt/2 * sin_teta);
      px_off = - (int)(ht/2 * sin_teta + wt/2 * cos_teta);  
      break;
    }
  }
  else
  {
    switch (ctxcanvas->canvas->text_alignment)
    {
    case CD_BASE_RIGHT:
    case CD_NORTH_EAST:
    case CD_EAST:
    case CD_SOUTH_EAST:
      px_off = - w/2;
      break;
    case CD_BASE_CENTER:
    case CD_CENTER:
    case CD_NORTH:
    case CD_SOUTH:
      px_off = 0;  
      break;
    case CD_BASE_LEFT:
    case CD_NORTH_WEST:
    case CD_WEST:
    case CD_SOUTH_WEST:
      px_off = w/2;         
      break;
    }

    switch (ctxcanvas->canvas->text_alignment)
    {
    case CD_BASE_LEFT:
    case CD_BASE_CENTER:
    case CD_BASE_RIGHT:
      py_off = - off;
      break;
    case CD_SOUTH_EAST:
    case CD_SOUTH_WEST:
    case CD_SOUTH:
      py_off = - h/2;
      break;
    case CD_NORTH_EAST:
    case CD_NORTH:
    case CD_NORTH_WEST:
      py_off = + h/2;
      break;
    case CD_CENTER:
    case CD_EAST:
    case CD_WEST:
      py_off = 0;
      break;
    }
  }

  /* move do centro da imagem para o canto superior esquerdo da imagem */
  px_off -= w/2;
  py_off -= h/2;

  /* desloca o ponto dado */
  if (ctxcanvas->canvas->invert_yaxis)
  {
    px += px_off;
    py += py_off;
  }
  else
  {
    px += px_off;
    py -= py_off;
  }

  hBitmap = CreateCompatibleBitmap(ctxcanvas->hDC, w, h);
  hBitmapDC = CreateCompatibleDC(ctxcanvas->hDC);
  
  hOldBitmap = SelectObject(hBitmapDC, hBitmap);

  /* copia a area do canvas para o bitmap */
  BitBlt(hBitmapDC, 0, 0, w, h, ctxcanvas->hDC, px, py, SRCCOPY);

  /* compensa a ROP antes de desenhar */
  BitBlt(hBitmapDC, 0, 0, w, h, ctxcanvas->hDC, px, py, ctxcanvas->RopBlt);

  SetBkMode(hBitmapDC, TRANSPARENT);
  SetBkColor(hBitmapDC, ctxcanvas->bg);
  SetTextColor(hBitmapDC, ctxcanvas->fg);
  SetTextAlign(hBitmapDC, TA_CENTER | TA_BASELINE);
  hOldFont = SelectObject(hBitmapDC, ctxcanvas->hFont);
  
  TextOut(hBitmapDC, x, y, s, len);
  
  if (ctxcanvas->canvas->invert_yaxis)
    BitBlt(ctxcanvas->hDC, px, py, w, h, hBitmapDC, 0, 0, ctxcanvas->RopBlt);
  else
    StretchBlt(ctxcanvas->hDC, px, py, w, -h, hBitmapDC, 0, 0, w, h, ctxcanvas->RopBlt);

  SelectObject(hBitmapDC, hOldFont);
  SelectObject(hBitmapDC, hOldBitmap);
  
  DeleteObject(hBitmap);
  DeleteDC(hBitmapDC);
}

static void cdgettextsize (cdCtxCanvas* ctxcanvas, const char *s, int len, int *width, int *height)
{
  SIZE size;
  
  GetTextExtentPoint32(ctxcanvas->hDC, s, len, &size);
  
  if (width)  
    *width  = size.cx;
  
  if (height) 
    *height = size.cy;
}

static void cdwCanvasGetTextHeight(cdCanvas* canvas, int x, int y, const char *s, int len, int *hbox, int *hoff)
{
  int w, h, ascent, height, baseline;
  int xmin, xmax, ymin, ymax;
  
  cdgettextsize(canvas->ctxcanvas, s, len, &w, &h);
  cdCanvasGetFontDim(canvas, NULL, &height, &ascent, NULL);
  baseline = height - ascent;

  /* move to bottom-left */
  cdTextTranslatePoint(canvas, x, y, w, h, baseline, &xmin, &ymin);

  *hoff = y - ymin;

  xmax = xmin + w-1;
  ymax = ymin + h-1;

  if (canvas->text_orientation)
  {
    double cos_theta = cos(canvas->text_orientation*CD_DEG2RAD);
    double sin_theta = sin(canvas->text_orientation*CD_DEG2RAD);
    int rectY[4];

    *hoff = (int)(*hoff * cos_theta);

    cdRotatePointY(canvas, xmin, ymin, x, y, &rectY[0], sin_theta, cos_theta);
    cdRotatePointY(canvas, xmax, ymin, x, y, &rectY[1], sin_theta, cos_theta);
    cdRotatePointY(canvas, xmax, ymax, x, y, &rectY[2], sin_theta, cos_theta);
    cdRotatePointY(canvas, xmin, ymax, x, y, &rectY[3], sin_theta, cos_theta);

    ymin = ymax = rectY[0];
    if (rectY[1] < ymin) ymin = rectY[1];
    if (rectY[2] < ymin) ymin = rectY[2];
    if (rectY[3] < ymin) ymin = rectY[3];
    if (rectY[1] > ymax) ymax = rectY[1];
    if (rectY[2] > ymax) ymax = rectY[2];
    if (rectY[3] > ymax) ymax = rectY[3];
  }

  *hbox = ymax-ymin+1;
}

static void cdwTextTransform(cdCtxCanvas* ctxcanvas, const char* s, int len, int *x, int *y)
{
  XFORM xForm;
  int hoff, h;

  cdwCanvasGetTextHeight(ctxcanvas->canvas, *x, *y, s, len, &h, &hoff);

  /* move to (x,y) and remove a vertical offset since text reference point is top-left */
  xForm.eM11 = (FLOAT)1; 
  xForm.eM12 = (FLOAT)0;
  xForm.eM21 = (FLOAT)0; 
  xForm.eM22 = (FLOAT)1; 
  xForm.eDx  = (FLOAT)*x; 
  xForm.eDy  = (FLOAT)(*y - (h-1) - hoff); 
  ModifyWorldTransform(ctxcanvas->hDC, &xForm, MWT_LEFTMULTIPLY);

  /* invert the text vertical orientation, relative to itself */
  xForm.eM11 = (FLOAT)1; 
  xForm.eM12 = (FLOAT)0;
  xForm.eM21 = (FLOAT)0; 
  xForm.eM22 = (FLOAT)-1; 
  xForm.eDx  = (FLOAT)0; 
  xForm.eDy  = (FLOAT)(h-1); 
  ModifyWorldTransform(ctxcanvas->hDC, &xForm, MWT_LEFTMULTIPLY);

  *x = 0;
  *y = 0;
}

static void cdtext(cdCtxCanvas* ctxcanvas, int x, int y, const char *s, int len)
{
  if (ctxcanvas->canvas->write_mode == CD_REPLACE || 
      ctxcanvas->wtype == CDW_EMF ||
      ctxcanvas->wtype == CDW_WMF ||
      ctxcanvas->canvas->new_region)
  {
    int h = -1;

    if ((ctxcanvas->canvas->text_alignment == CD_CENTER ||
        ctxcanvas->canvas->text_alignment == CD_EAST ||
        ctxcanvas->canvas->text_alignment == CD_WEST) &&
        ctxcanvas->wtype != CDW_WMF)
    {
      /* compensa deficiencia do alinhamento no windows */
      int off;
      cdgettextsize(ctxcanvas, s, len, NULL, &h);
      off = h/2 - ctxcanvas->font.descent;

      if (ctxcanvas->canvas->text_orientation != 0)
      {
        y += (int)(off * cos(ctxcanvas->canvas->text_orientation*CD_DEG2RAD));
        x += (int)(off * sin(ctxcanvas->canvas->text_orientation*CD_DEG2RAD));
      }
      else
        y += off;
    }

    if (ctxcanvas->canvas->back_opacity == CD_OPAQUE)
      SetBkMode(ctxcanvas->hDC, TRANSPARENT);

    if (ctxcanvas->canvas->new_region)
      BeginPath(ctxcanvas->hDC);
        
    if (ctxcanvas->canvas->use_matrix)
      cdwTextTransform(ctxcanvas, s, len, &x, &y);

    TextOut(ctxcanvas->hDC, x, y+1, s, len); /* compensa erro de desenho com +1 */

    if (ctxcanvas->canvas->use_matrix)
      cdtransform(ctxcanvas, ctxcanvas->canvas->matrix);

    if (ctxcanvas->canvas->new_region)
    {
      HRGN rgn;
      EndPath(ctxcanvas->hDC);
      rgn = PathToRegion(ctxcanvas->hDC);
      CombineRgn(ctxcanvas->new_rgn, ctxcanvas->new_rgn, rgn, sCombineRegion2win[ctxcanvas->canvas->combine_mode]);
      DeleteObject(rgn);
    }

    if (ctxcanvas->canvas->back_opacity == CD_OPAQUE)
      SetBkMode(ctxcanvas->hDC, OPAQUE);
  }
  else
    sTextOutBlt(ctxcanvas, x, y+1, s, len);
}

static int cdtextalignment(cdCtxCanvas* ctxcanvas, int text_align)
{
  int align = TA_NOUPDATECP;

  switch (text_align)
  {
  case CD_BASE_RIGHT:
  case CD_NORTH_EAST:
  case CD_EAST:
  case CD_SOUTH_EAST:
    align |= TA_RIGHT;
    break;
  case CD_BASE_CENTER:
  case CD_CENTER:
  case CD_NORTH:
  case CD_SOUTH:
    align |= TA_CENTER;
    break;
  case CD_BASE_LEFT:
  case CD_NORTH_WEST:
  case CD_WEST:
  case CD_SOUTH_WEST:
    align |= TA_LEFT;
    break;
  }

  switch (text_align)
  {
  case CD_BASE_LEFT:
  case CD_BASE_CENTER:
  case CD_BASE_RIGHT:
    align |= TA_BASELINE;
    break;
  case CD_SOUTH_EAST:
  case CD_SOUTH_WEST:
  case CD_SOUTH:
    align |= TA_BOTTOM;
    break;
  case CD_NORTH_EAST:
  case CD_NORTH:
  case CD_NORTH_WEST:
    align |= TA_TOP;
    break;
  case CD_CENTER:
  case CD_EAST:
  case CD_WEST:
    align |= TA_BASELINE; /* tem que compensar ao desenhar o texto */
    break;
  }

  SetTextAlign(ctxcanvas->hDC, align);
           
  return text_align;
}

static int cdfont(cdCtxCanvas* ctxcanvas, const char *type_face, int style, int size)
{
  TEXTMETRIC   tm;
  DWORD bold, italic = 0, underline = 0, strikeout = 0;
  int angle, size_pixel;
  HFONT hFont;
  
  if (style&CD_BOLD)
    bold = FW_BOLD;
  else
    bold = FW_NORMAL;
  
  if (style&CD_ITALIC)
    italic = 1;
  
  if (style&CD_UNDERLINE)
    underline = 1;
  
  if (style&CD_STRIKEOUT)
    strikeout = 1;
  
  angle = ctxcanvas->font_angle;
  
  if (cdStrEqualNoCase(type_face, "Courier") || cdStrEqualNoCase(type_face, "Monospace"))
    type_face = "Courier New";
  else if (cdStrEqualNoCase(type_face, "Times") || cdStrEqualNoCase(type_face, "Serif"))
    type_face = "Times New Roman";
  else if (cdStrEqualNoCase(type_face, "Helvetica") || cdStrEqualNoCase(type_face, "Sans"))
    type_face = "Arial";

  size_pixel = cdGetFontSizePixels(ctxcanvas->canvas, size);
  hFont = CreateFont(-size_pixel, 0, angle, angle, bold, italic, underline, strikeout,
                     DEFAULT_CHARSET,OUT_TT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,FF_DONTCARE|DEFAULT_PITCH,
                     type_face);
  if (!hFont) return 0;

  if (ctxcanvas->hOldFont) SelectObject(ctxcanvas->hDC, ctxcanvas->hOldFont);
  if (ctxcanvas->hFont) DeleteObject(ctxcanvas->hFont);
  ctxcanvas->hFont = hFont;
  ctxcanvas->hOldFont = SelectObject(ctxcanvas->hDC, ctxcanvas->hFont);
  
  GetTextMetrics (ctxcanvas->hDC, &tm);
  ctxcanvas->font.max_width  = tm.tmMaxCharWidth;
  ctxcanvas->font.height     = tm.tmHeight + tm.tmExternalLeading ;
  ctxcanvas->font.ascent     = tm.tmAscent;
  ctxcanvas->font.descent    = tm.tmDescent;

  return 1;
}

static int cdnativefont (cdCtxCanvas* ctxcanvas, const char* nativefont)
{
  TEXTMETRIC tm;
  HFONT hFont;
  int size = 12, bold = FW_NORMAL, italic = 0, 
      style = CD_PLAIN, underline = 0, strikeout = 0,
      size_pixel;
  char type_face[1024];
  
  if (nativefont[0] == '-' && nativefont[1] == 'd')
  {
    COLORREF rgbColors;
    CHOOSEFONT cf;
    LOGFONT lf;

    ZeroMemory(&cf, sizeof(CHOOSEFONT));
    
    cf.lStructSize = sizeof(CHOOSEFONT);
    cf.hwndOwner = GetForegroundWindow();
    cf.lpLogFont = &lf;
    cf.Flags = CF_SCREENFONTS | CF_EFFECTS | CF_INITTOLOGFONTSTRUCT;
    rgbColors = cf.rgbColors = ctxcanvas->fg;

    GetTextFace(ctxcanvas->hDC, 50, type_face);
    GetTextMetrics(ctxcanvas->hDC, &tm);   /* get the current selected nativefont */

    size_pixel = cdGetFontSizePixels(ctxcanvas->canvas, ctxcanvas->canvas->font_size);

    strcpy(lf.lfFaceName, type_face);
    lf.lfWeight = tm.tmWeight;
    lf.lfHeight = -size_pixel;
    lf.lfItalic = tm.tmItalic;
    lf.lfUnderline = tm.tmUnderlined;
    lf.lfStrikeOut = tm.tmStruckOut;
    lf.lfCharSet = tm.tmCharSet;
    lf.lfEscapement = ctxcanvas->font_angle; 
    lf.lfOrientation = ctxcanvas->font_angle;
    lf.lfWidth = 0; 
    lf.lfOutPrecision = OUT_TT_PRECIS; 
    lf.lfClipPrecision = CLIP_DEFAULT_PRECIS; 
    lf.lfQuality = DEFAULT_QUALITY; 
    lf.lfPitchAndFamily = FF_DONTCARE|DEFAULT_PITCH; 

    if (ChooseFont(&cf))
    {
      if (rgbColors != cf.rgbColors)
        cdCanvasSetForeground(ctxcanvas->canvas, sColorFromWindows(cf.rgbColors));
        
      hFont = CreateFontIndirect(&lf);
    }
    else
      return 0;

    bold = lf.lfWeight;
    italic = lf.lfItalic;
    size = lf.lfHeight;
    strcpy(type_face, lf.lfFaceName);
    underline = lf.lfUnderline;
    strikeout = lf.lfStrikeOut;

    if (bold!=FW_NORMAL) style |= CD_BOLD;
    if (italic) style |= CD_ITALIC;
    if (underline) style |= CD_UNDERLINE;
    if (strikeout) style |= CD_STRIKEOUT;
  }                                     
  else
  {
    if (!cdParseIupWinFont(nativefont, type_face, &style, &size))
    {
      if (!cdParsePangoFont(nativefont, type_face, &style, &size))
        return 0;
    }
      
    if (style&CD_BOLD)
      bold = FW_BOLD;
    if (style&CD_ITALIC)
      italic = 1;
    if (style&CD_UNDERLINE)
      underline = 1;
    if (style&CD_STRIKEOUT)
      strikeout = 1;

    size_pixel = cdGetFontSizePixels(ctxcanvas->canvas, size);
    
    hFont = CreateFont(-size_pixel, 0, ctxcanvas->font_angle, ctxcanvas->font_angle, 
                       bold, italic, underline, strikeout,
                       DEFAULT_CHARSET,OUT_TT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,FF_DONTCARE|DEFAULT_PITCH, type_face);
    if (!hFont) return 0;
  }
  
  if (ctxcanvas->hOldFont) SelectObject(ctxcanvas->hDC, ctxcanvas->hOldFont);
  DeleteObject(ctxcanvas->hFont);
  ctxcanvas->hFont = hFont; 
  ctxcanvas->hOldFont = SelectObject(ctxcanvas->hDC, ctxcanvas->hFont);
  
  GetTextMetrics(ctxcanvas->hDC, &tm);
  ctxcanvas->font.max_width  = tm.tmMaxCharWidth;
  ctxcanvas->font.height     = tm.tmHeight + tm.tmExternalLeading;
  ctxcanvas->font.ascent     = tm.tmAscent;
  ctxcanvas->font.descent    = tm.tmDescent;

  /* update cdfont parameters */
  ctxcanvas->canvas->font_style = style;
  ctxcanvas->canvas->font_size = size;
  strcpy(ctxcanvas->canvas->font_type_face, type_face);

  return 1;
}

static double cdtextorientation(cdCtxCanvas* ctxcanvas, double angle)
{
  if (ctxcanvas->font_angle == angle) /* first time angle=0, do not create font twice */
    return angle;

  ctxcanvas->font_angle = (int)(angle * 10);

  cdfont(ctxcanvas, ctxcanvas->canvas->font_type_face, ctxcanvas->canvas->font_style, ctxcanvas->canvas->font_size);

  return angle;
}

static void cdgetfontdim (cdCtxCanvas* ctxcanvas, int *max_width, int *line_height, int *ascent, int *descent)
{
  if (max_width)   
    *max_width   = ctxcanvas->font.max_width;
  
  if (line_height) 
    *line_height = ctxcanvas->font.height;
  
  if (ascent)      
    *ascent      = ctxcanvas->font.ascent;
  
  if (descent)     
    *descent     = ctxcanvas->font.descent;
  
  return;
}

/*
%F Desenha um retangulo no canvas todo com a cor do fundo.
*/
static void cdclear(cdCtxCanvas* ctxcanvas)
{
  RECT rect;
  
  if (ctxcanvas->canvas->clip_mode != CD_CLIPOFF) 
    SelectClipRgn( ctxcanvas->hDC, NULL );   /* toda 'area do canvas */
  
  SetRect(&rect, 0, 0, ctxcanvas->canvas->w, ctxcanvas->canvas->h);
  FillRect(ctxcanvas->hDC, &rect, ctxcanvas->hBkBrush);
  
  if (ctxcanvas->canvas->clip_mode != CD_CLIPOFF) 
    cdclip(ctxcanvas, ctxcanvas->canvas->clip_mode);
}


/******************************************************************/
/*
%S             Funcoes de imagens do cliente                      
*/
/******************************************************************/

static void cdgetimagergb(cdCtxCanvas* ctxcanvas, unsigned char *red, unsigned char *green, unsigned char *blue, int x, int y, int w, int h)
{
  XFORM xForm;
  cdwDIB dib;
  HDC     hDCMem;
  HBITMAP hOldBitmap,hBitmap;
  int yr;
  
  hBitmap = CreateCompatibleBitmap(ctxcanvas->hDC, w, h);
  if (hBitmap == NULL)
    return;

  hDCMem = CreateCompatibleDC(ctxcanvas->hDC);
  
  hOldBitmap = SelectObject(hDCMem, hBitmap);
  
  if (GetGraphicsMode(ctxcanvas->hDC) == GM_ADVANCED)
  {
    /* reset to the identity. */
    GetWorldTransform(ctxcanvas->hDC, &xForm);
    ModifyWorldTransform(ctxcanvas->hDC, NULL, MWT_IDENTITY);
  }

  if (ctxcanvas->canvas->invert_yaxis==0) /* if 0, invert because the transform was reset here */
    y = _cdInvertYAxis(ctxcanvas->canvas, y);
  
  yr = y - (h - 1);  /* y starts at the bottom of the image */
  BitBlt(hDCMem,0,0,w,h,ctxcanvas->hDC, x, yr, SRCCOPY);

  if (GetGraphicsMode(ctxcanvas->hDC) == GM_ADVANCED)
    ModifyWorldTransform(ctxcanvas->hDC, &xForm, MWT_LEFTMULTIPLY);
  
  dib.w = w;
  dib.h = h;
  dib.type = 0; 
  
  if (!cdwCreateDIB(&dib))
  {
    SelectObject(hDCMem, hOldBitmap);
    DeleteObject(hBitmap);
    DeleteDC(hDCMem);
    return;
  }
  
  GetDIBits(ctxcanvas->hDC, hBitmap, 0, h, dib.bits, dib.bmi, DIB_RGB_COLORS);	
  
  SelectObject(hDCMem, hOldBitmap);
  DeleteObject(hBitmap);
  DeleteDC(hDCMem);
  
  cdwDIBDecodeRGB(&dib, red, green, blue);
  
  cdwKillDIB(&dib);
}

static void sFixImageY(cdCanvas* canvas, int *y, int *h)
{
  /* Here, y is from top to bottom,
     is at the bottom-left corner of the image if h>0
     is at the top-left corner of the image if h<0. (Undocumented feature)

     cdCalcZoom expects Y at top-left if h>0
                   and  Y at bottom-left if h<0
     if h<0 then eh<0 to StretchDIBits mirror the image.
     BUT!!!!!! AlphaBlend will NOT mirror the image. 
     So it must be manually made there. */

  if (!canvas->invert_yaxis)
    *h = -(*h);

  if (*h < 0)
    *y -= ((*h) + 1);  /* compensate for cdCalcZoom */
  else
    *y -= ((*h) - 1);  /* move Y to top-left corner, since it was at the bottom of the image */
}

static void cdputimagerectmap(cdCtxCanvas* ctxcanvas, int width, int height, const unsigned char *index, const long int *colors, int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax)
{
  cdwDIB dib;
  int ew, eh, ex, ey; /* posicao da imagem com zoom no canvas e tamanho da imagem com zoom depois de otimizado */
  int bw, bh, bx, by; /* posicao dentro da imagem e tamanho dentro da imagem do pedaco que sera desenhado depois de otimizado */
  int rw, rh;         /* tamanho dentro da imagem antes de otimizado */

  rw = xmax-xmin+1;
  rh = ymax-ymin+1;

  sFixImageY(ctxcanvas->canvas, &y, &h);

  if (!cdCalcZoom(ctxcanvas->canvas->w, x, w, &ex, &ew, xmin, rw, &bx, &bw, 1))
    return;
  
  if (!cdCalcZoom(ctxcanvas->canvas->h, y, h, &ey, &eh, ymin, rh, &by, &bh, 0))
    return;

  dib.w = bw;
  dib.h = bh;
  dib.type = 1;
  
  if (!cdwCreateDIBRefBuffer(&dib, &ctxcanvas->dib_bits, &ctxcanvas->bits_size))
    return;

  cdwDIBEncodeMapRect(&dib, index, colors, bx, by, width, height);
  
  StretchDIBits(ctxcanvas->hDC, 
                ex, ey, ew, eh, 
                0, 0, bw, bh, 
                dib.bits, dib.bmi, DIB_RGB_COLORS, ctxcanvas->RopBlt);
}

static void cdputimagerectrgb(cdCtxCanvas* ctxcanvas, int width, int height, const unsigned char *red, 
                    const unsigned char *green, const unsigned char *blue, int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax)
{
  cdwDIB dib;
  int ew, eh, ex, ey;
  int bw, bh, bx, by;
  int rw, rh;

  rw = xmax-xmin+1;
  rh = ymax-ymin+1;

  sFixImageY(ctxcanvas->canvas, &y, &h);

  if (!cdCalcZoom(ctxcanvas->canvas->w, x, w, &ex, &ew, xmin, rw, &bx, &bw, 1))
    return;
  
  if (!cdCalcZoom(ctxcanvas->canvas->h, y, h, &ey, &eh, ymin, rh, &by, &bh, 0))
    return;

  dib.w = bw;
  dib.h = bh;
  dib.type = 0;
  
  if (!cdwCreateDIBRefBuffer(&dib, &ctxcanvas->dib_bits, &ctxcanvas->bits_size))
    return;

  cdwDIBEncodeRGBRect(&dib, red, green, blue, bx, by, width, height);
  
  StretchDIBits(ctxcanvas->hDC, 
                ex, ey, ew, eh, 
                0, 0, bw, bh, 
                dib.bits, dib.bmi, DIB_RGB_COLORS, ctxcanvas->RopBlt);
}

static void cdputimagerectrgba(cdCtxCanvas* ctxcanvas, int width, int height, const unsigned char *red, 
                     const unsigned char *green, const unsigned char *blue, const unsigned char *alpha, int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax)
{
  cdwDIB dib;
  HDC hDCMem;
  HBITMAP hOldBitmap, hBitmap;
  int ew, eh, ex, ey;
  int bw, bh, bx, by;
  int rw, rh;

  rw = xmax-xmin+1;
  rh = ymax-ymin+1;

  sFixImageY(ctxcanvas->canvas, &y, &h);

  if (!cdCalcZoom(ctxcanvas->canvas->w, x, w, &ex, &ew, xmin, rw, &bx, &bw, 1))
    return;
  
  if (!cdCalcZoom(ctxcanvas->canvas->h, y, h, &ey, &eh, ymin, rh, &by, &bh, 0))
    return;

  hDCMem = CreateCompatibleDC(ctxcanvas->hDC);

  if (cdwAlphaBlend)
  {
    BLENDFUNCTION blendfunc;

    dib.w = bw;
    dib.h = bh;
    dib.type = 2; /* RGBA */

    hBitmap = cdwCreateDIBSection(&dib, hDCMem);
    if (!hBitmap)
    {
      DeleteDC(hDCMem);
      return;
    }

    if (eh < 0) /* must mirror the image */
    {
      /* Fix position */
      eh = -eh;
      ey = ey - eh; 

      cdwDIBEncodeRGBARectMirror(&dib, red, green, blue, alpha, bx, by, width, height);
    }
    else
      cdwDIBEncodeRGBARect(&dib, red, green, blue, alpha, bx, by, width, height);

    hOldBitmap = SelectObject(hDCMem, hBitmap);

    blendfunc.BlendOp = AC_SRC_OVER;
    blendfunc.BlendFlags = 0;
    blendfunc.SourceConstantAlpha = 0xFF;
    blendfunc.AlphaFormat = AC_SRC_ALPHA;

    cdwAlphaBlend(ctxcanvas->hDC,
                 ex, ey, ew, eh, 
                 hDCMem,
                 0, 0, bw, bh, 
                 blendfunc);
  }
  else
  {
    hBitmap = CreateCompatibleBitmap(ctxcanvas->hDC, ew, eh); /* captura do tamanho do destino */
    if (!hBitmap)
    {
      DeleteDC(hDCMem);
      return;
    }
    
    hOldBitmap = SelectObject(hDCMem, hBitmap);
    
    BitBlt(hDCMem, 0, 0, ew, eh, ctxcanvas->hDC, ex, ey, SRCCOPY);
    
    dib.w = ew;  /* neste caso o tamanho usado e´ o de destino */
    dib.h = eh;
    dib.type = 0;
    
    if (!cdwCreateDIB(&dib))
    {
      SelectObject(hDCMem, hOldBitmap);
      DeleteObject(hBitmap);
      DeleteDC(hDCMem);
      return;
    }
    
    GetDIBits(hDCMem, hBitmap, 0, eh, dib.bits, dib.bmi, DIB_RGB_COLORS);	
    
    cdwDIBEncodeRGBARectZoom(&dib, red, green, blue, alpha, width, height, bx, by, bw, bh);
    
    StretchDIBits(ctxcanvas->hDC, 
                  ex, ey, ew, eh, 
                  0, 0, ew, eh,   /* Nao tem zoom neste caso, pois e´ feito manualmente pela EncodeRGBA */
                  dib.bits, dib.bmi, DIB_RGB_COLORS, ctxcanvas->RopBlt);
  }

  SelectObject(hDCMem, hOldBitmap);
  DeleteObject(hBitmap);
  DeleteDC(hDCMem);
  cdwKillDIB(&dib);
}


/********************************************************************/
/*
%S        Funcoes de imagens do servidor                          
*/
/********************************************************************/

static void cdpixel(cdCtxCanvas* ctxcanvas, int x, int y, long int cd_color)
{
  SetPixelV(ctxcanvas->hDC, x, y, sColorToWindows(ctxcanvas, cd_color));
}

static cdCtxImage *cdcreateimage(cdCtxCanvas* ctxcanvas, int width, int height)
{
  HDC hDCMem;
  HBITMAP hOldBitmap,hBitmap;
  cdCtxImage *ctximage;
  void* rgba_dib = NULL;
  unsigned char* alpha = NULL;

  if (ctxcanvas->img_format)
  {
    cdwDIB dib;

    dib.w = width;
    dib.h = height;
    if (ctxcanvas->img_format == 32)
      dib.type = CDW_RGBA; 
    else
      dib.type = CDW_RGB; 

    hBitmap = cdwCreateDIBSection(&dib, ctxcanvas->hDC);
    if (!hBitmap)
      return NULL;

    rgba_dib = dib.bits;
    alpha = ctxcanvas->img_alpha;

    cdwKillDIB(&dib); /* this will just remove the headers not the dib bits in this case */
  }
  else
  {
    hBitmap = CreateCompatibleBitmap(ctxcanvas->hDC, width, height);
    if (!hBitmap)
      return NULL;
  }
  
  hDCMem = CreateCompatibleDC(ctxcanvas->hDC);
  hOldBitmap = SelectObject(hDCMem, hBitmap);
  
  PatBlt(hDCMem, 0, 0, width, height, WHITENESS);
  
  /* salva o contexto desta imagem */
  ctximage = (cdCtxImage*)malloc(sizeof(cdCtxImage));
  
  ctximage->hDC        = hDCMem;
  ctximage->hBitmap    = hBitmap;
  ctximage->hOldBitmap = hOldBitmap;
  ctximage->w          = width;
  ctximage->h          = height;
  ctximage->rgba_dib   = rgba_dib;
  ctximage->alpha      = alpha;

  ctximage->bpp = ctxcanvas->canvas->bpp;
  ctximage->xres = ctxcanvas->canvas->xres;
  ctximage->yres = ctxcanvas->canvas->yres;

  ctximage->w_mm = ctximage->w / ctximage->xres;
  ctximage->h_mm = ctximage->h / ctximage->yres;
  
  return ctximage;
}

static void cdgetimage(cdCtxCanvas* ctxcanvas, cdCtxImage *ctximage, int x, int y)
{
  XFORM xForm;

  if (GetGraphicsMode(ctxcanvas->hDC) == GM_ADVANCED)
  {
    /* reset to the identity. */
    GetWorldTransform(ctxcanvas->hDC, &xForm);
    ModifyWorldTransform(ctxcanvas->hDC, NULL, MWT_IDENTITY);
  }

  if (ctxcanvas->canvas->invert_yaxis==0)  /* if 0, invert because the transform was reset here */
    y = _cdInvertYAxis(ctxcanvas->canvas, y);

  /* y is the bottom-left of the image in CD, must be at upper-left */
  y -= ctximage->h-1;
  BitBlt(ctximage->hDC, 0, 0, ctximage->w, ctximage->h, ctxcanvas->hDC, x, y, SRCCOPY);

  if (GetGraphicsMode(ctxcanvas->hDC) == GM_ADVANCED)
    ModifyWorldTransform(ctxcanvas->hDC, &xForm, MWT_LEFTMULTIPLY);
}

static void cdputimagerect(cdCtxCanvas* ctxcanvas, cdCtxImage *ctximage, int x0, int y0, int xmin, int xmax, int ymin, int ymax)
{
  int yr = y0 - (ymax-ymin+1)+1; /* y0 starts at the bottom of the image */

  if (ctximage->alpha && ctximage->bpp == 32 && cdwAlphaBlend)
  {
    cdwDIB dib;
    BLENDFUNCTION blendfunc;
    blendfunc.BlendOp = AC_SRC_OVER;
    blendfunc.BlendFlags = 0;
    blendfunc.SourceConstantAlpha = 0xFF;
    blendfunc.AlphaFormat = AC_SRC_ALPHA;

    dib.w = ctximage->w;
    dib.h = ctximage->h;
    dib.type = CDW_RGBA; 
    cdwCreateDIBRefBits(&dib, ctximage->rgba_dib);

    cdwDIBEncodeAlphaRect(&dib, ctximage->alpha, 0, 0, ctximage->w, ctximage->h);

    GdiFlush();
    cdwAlphaBlend(ctxcanvas->hDC,
                x0, yr, xmax-xmin+1, ymax-ymin+1, 
                ctximage->hDC,
                xmin, ctximage->h-ymax-1, xmax-xmin+1, ymax-ymin+1, 
                blendfunc);

    cdwKillDIB(&dib);
  }
  else if(ctxcanvas->use_img_points)
  {
    POINT pts[3];
    pts[0] = ctxcanvas->img_points[0];
    pts[1] = ctxcanvas->img_points[1];
    pts[2] = ctxcanvas->img_points[2];
    if (ctxcanvas->canvas->invert_yaxis)
    {
      pts[0].y = _cdInvertYAxis(ctxcanvas->canvas, pts[0].y);
      pts[1].y = _cdInvertYAxis(ctxcanvas->canvas, pts[1].y);
      pts[2].y = _cdInvertYAxis(ctxcanvas->canvas, pts[2].y);
    }
    PlgBlt(ctxcanvas->hDC, pts, ctximage->hDC, xmin, ctximage->h-ymax-1, xmax-xmin+1, ymax-ymin+1, ctxcanvas->img_mask, 0, 0);
  }
  else if (ctxcanvas->img_mask)
    MaskBlt(ctxcanvas->hDC,x0,yr, xmax-xmin+1, ymax-ymin+1, ctximage->hDC, xmin, ctximage->h-ymax-1, ctxcanvas->img_mask, 0, 0, MAKEROP4(ctxcanvas->RopBlt, 0xAA0000));
  else
    BitBlt(ctxcanvas->hDC,x0,yr, xmax-xmin+1, ymax-ymin+1, ctximage->hDC, xmin, ctximage->h-ymax-1, ctxcanvas->RopBlt);
}

static void  cdkillimage(cdCtxImage *ctximage)
{
  SelectObject(ctximage->hDC, ctximage->hOldBitmap);
  DeleteObject(ctximage->hBitmap);
  DeleteDC(ctximage->hDC);
  free(ctximage);
}

static void cdscrollarea(cdCtxCanvas* ctxcanvas, int xmin, int xmax, int ymin, int ymax, int dx, int dy)
{
  XFORM xForm;
  RECT rect;
  
  if (GetGraphicsMode(ctxcanvas->hDC) == GM_ADVANCED)
  {
    /* reset to the identity. */
    GetWorldTransform(ctxcanvas->hDC, &xForm);
    ModifyWorldTransform(ctxcanvas->hDC, NULL, MWT_IDENTITY);
  }

  if (ctxcanvas->canvas->invert_yaxis==0)  /* if 0, invert because the transform was reset here */
  {
    dy = -dy;
    ymin = _cdInvertYAxis(ctxcanvas->canvas, ymin);
    ymax = _cdInvertYAxis(ctxcanvas->canvas, ymax);
    _cdSwapInt(ymin, ymax);
  }

  rect.left   = xmin;          
  rect.right  = xmax+1;
  rect.top    = ymin;
  rect.bottom = ymax+1; 

  ScrollDC(ctxcanvas->hDC, dx, dy, &rect, NULL, NULL, NULL);

  if (GetGraphicsMode(ctxcanvas->hDC) == GM_ADVANCED)
    ModifyWorldTransform(ctxcanvas->hDC, &xForm, MWT_LEFTMULTIPLY);
}

static void cdflush(cdCtxCanvas* ctxcanvas)
{
  (void)ctxcanvas;
  GdiFlush();
}

/********************************************************************/
/*
%S        Atributos personalizados                          
*/
/********************************************************************/

static void set_img_format_attrib(cdCtxCanvas* ctxcanvas, char* data)
{
  if (!data)
    ctxcanvas->img_format = 0;
  else
  {
    int bpp = 0;
    sscanf(data, "%d", &bpp);
    if (bpp == 0)
      return;

    if (bpp == 32)
      ctxcanvas->img_format = 32;
    else
      ctxcanvas->img_format = 24;
  }
}

static char* get_img_format_attrib(cdCtxCanvas* ctxcanvas)
{
  if (!ctxcanvas->img_format)
    return NULL;

  if (ctxcanvas->img_format == 32)
    return "32";
  else
    return "24";
}

static cdAttribute img_format_attrib =
{
  "IMAGEFORMAT",
  set_img_format_attrib,
  get_img_format_attrib
}; 

static void set_img_alpha_attrib(cdCtxCanvas* ctxcanvas, char* data)
{
  if (!data)
    ctxcanvas->img_alpha = NULL;
  else
    ctxcanvas->img_alpha = (unsigned char*)data;
}

static char* get_img_alpha_attrib(cdCtxCanvas* ctxcanvas)
{
  return (char*)ctxcanvas->img_alpha;
}

static cdAttribute img_alpha_attrib =
{
  "IMAGEALPHA",
  set_img_alpha_attrib,
  get_img_alpha_attrib
}; 

static void set_img_mask_attrib(cdCtxCanvas* ctxcanvas, char* data)
{
  if (!data)
  {
    if (ctxcanvas->img_mask) DeleteObject(ctxcanvas->img_mask);
    ctxcanvas->img_mask = NULL;
  }
  else
  {
    int w = 0, h = 0;
    unsigned char *index = 0;
    sscanf(data, "%d %d %p", &w, &h, &index); 
    if (w && h && index)
      ctxcanvas->img_mask = Stipple2Bitmap(w, h, index, 1);
  }
}

static cdAttribute img_mask_attrib =
{
  "IMAGEMASK",
  set_img_mask_attrib,
  NULL
}; 

static void set_img_points_attrib(cdCtxCanvas* ctxcanvas, char* data)
{
  int p[6];

  if (!data)
  {
    ctxcanvas->use_img_points = 0;
    return;
  }

  sscanf(data, "%d %d %d %d %d %d", &p[0], &p[1], &p[2], &p[3], &p[4], &p[5]);

  ctxcanvas->img_points[0].x = p[0];
  ctxcanvas->img_points[0].y = p[1];
  ctxcanvas->img_points[1].x = p[2];
  ctxcanvas->img_points[1].y = p[3];
  ctxcanvas->img_points[2].x = p[4];
  ctxcanvas->img_points[2].y = p[5];

  ctxcanvas->use_img_points = 1;
}

static char* get_img_points_attrib(cdCtxCanvas* ctxcanvas)
{
  static char data[100];

  if (!ctxcanvas->use_img_points)
    return NULL;

  sprintf(data, "%d %d %d %d %d %d", ctxcanvas->img_points[0].x,
                                     ctxcanvas->img_points[0].y,
                                     ctxcanvas->img_points[1].x,
                                     ctxcanvas->img_points[1].y,
                                     ctxcanvas->img_points[2].x,
                                     ctxcanvas->img_points[2].y);

  return data;
}

static cdAttribute img_points_attrib =
{
  "IMAGEPOINTS",
  set_img_points_attrib,
  get_img_points_attrib
}; 

static void set_rotate_attrib(cdCtxCanvas* ctxcanvas, char* data)
{
  /* ignore ROTATE if transform is set, 
     because there is native support for transformations */
  if (ctxcanvas->canvas->use_matrix)
    return;

  if (data)
  {
    sscanf(data, "%g %d %d", &ctxcanvas->rotate_angle,
                             &ctxcanvas->rotate_center_x,
                             &ctxcanvas->rotate_center_y);
  }
  else
  {
    ctxcanvas->rotate_angle = 0;
    ctxcanvas->rotate_center_x = 0;
    ctxcanvas->rotate_center_y = 0;
  }

  cdtransform(ctxcanvas, NULL);
}

static char* get_rotate_attrib(cdCtxCanvas* ctxcanvas)
{
  static char data[100];

  if (!ctxcanvas->rotate_angle)
    return NULL;

  sprintf(data, "%g %d %d", (double)ctxcanvas->rotate_angle,
                            ctxcanvas->rotate_center_x,
                            ctxcanvas->rotate_center_y);

  return data;
}

static cdAttribute rotate_attrib =
{
  "ROTATE",
  set_rotate_attrib,
  get_rotate_attrib
}; 

static void set_fill_attrib(cdCtxCanvas* ctxcanvas, char* data)
{
  ctxcanvas->fill_attrib[0] = data[0];
}

static char* get_fill_attrib(cdCtxCanvas* ctxcanvas)
{
  return ctxcanvas->fill_attrib;
}

static cdAttribute fill_attrib =
{
  "PENFILLPOLY",
  set_fill_attrib,
  get_fill_attrib
}; 

static void set_window_rgn(cdCtxCanvas* ctxcanvas, char* data)
{
  if (data)
  {
    HRGN hrgn = CreateRectRgn(0,0,0,0);
    CombineRgn(hrgn, ctxcanvas->new_rgn, NULL, RGN_COPY);
    SetWindowRgn(ctxcanvas->hWnd, hrgn, TRUE);
  }
  else
    SetWindowRgn(ctxcanvas->hWnd, NULL, TRUE);
}

static cdAttribute window_rgn_attrib =
{
  "WINDOWRGN",
  set_window_rgn,
  NULL
}; 

static char* get_hdc_attrib(cdCtxCanvas* ctxcanvas)
{
  return (char*)ctxcanvas->hDC;
}

static cdAttribute hdc_attrib =
{
  "HDC",
  NULL,
  get_hdc_attrib
}; 

/* 
%F Cria o canvas para o driver Windows. 
*/
cdCtxCanvas *cdwCreateCanvas(cdCanvas* canvas, HWND hWnd, HDC hDC, int wtype)
{
  cdCtxCanvas* ctxcanvas;
  LOGPEN  logNullPen;   
  
  ctxcanvas = (cdCtxCanvas*)malloc(sizeof(cdCtxCanvas));
  memset(ctxcanvas, 0, sizeof(cdCtxCanvas));

  /* store the base canvas */
  ctxcanvas->canvas = canvas;

  /* update canvas context */
  canvas->ctxcanvas = ctxcanvas;
  
  ctxcanvas->hWnd = hWnd;
  ctxcanvas->hDC = hDC;
  canvas->invert_yaxis = 1;

  /* linha nula para fill de interior apenas */
  logNullPen.lopnStyle = PS_NULL;
  ctxcanvas->hNullPen = CreatePenIndirect(&logNullPen);
  
  ctxcanvas->logPen.lopnStyle = PS_SOLID;
  ctxcanvas->logPen.lopnWidth.x = 1;      /* 1 para que a linha possa ter estilo */
  ctxcanvas->logPen.lopnColor = 0;
  ctxcanvas->rebuild_pen = 1;
  
  ctxcanvas->logBrush.lbStyle = BS_SOLID;
  ctxcanvas->logBrush.lbColor = 0;
  ctxcanvas->logBrush.lbHatch = HS_BDIAGONAL;
  
  ctxcanvas->clip_pnt = (POINT*)malloc(sizeof(POINT)*4);
  memset(ctxcanvas->clip_pnt, 0, sizeof(POINT)*4);
  ctxcanvas->clip_pnt_n = 4;

  ctxcanvas->wtype = wtype;

  SetStretchBltMode(ctxcanvas->hDC, COLORONCOLOR);

  ctxcanvas->fill_attrib[0] = '1';
  ctxcanvas->fill_attrib[1] = 0;

  cdRegisterAttribute(canvas, &hdc_attrib);
  cdRegisterAttribute(canvas, &fill_attrib);
  cdRegisterAttribute(canvas, &img_points_attrib);
  cdRegisterAttribute(canvas, &img_mask_attrib);
  cdRegisterAttribute(canvas, &rotate_attrib);
  cdRegisterAttribute(canvas, &img_alpha_attrib);
  cdRegisterAttribute(canvas, &img_format_attrib);
  cdRegisterAttribute(canvas, &window_rgn_attrib);

  if (!cdwAlphaBlend)
  {
    HINSTANCE lib = LoadLibrary("Msimg32");
    if (lib)
      cdwAlphaBlend = (AlphaBlendFunc)GetProcAddress(lib, "AlphaBlend");
  }
                                       
  return ctxcanvas;
}

void cdwInitTable(cdCanvas* canvas)
{
  cdCtxCanvas* ctxcanvas = canvas->ctxcanvas;

  canvas->cxPixel = cdpixel;
  canvas->cxLine = cdline;
  canvas->cxPoly = cdpoly;
  canvas->cxRect = cdrect;
  canvas->cxBox = cdbox;
  canvas->cxArc = cdarc;
  canvas->cxSector = cdsector;
  canvas->cxChord = cdchord;
  canvas->cxText = cdtext;

  canvas->cxGetFontDim = cdgetfontdim;
  canvas->cxGetTextSize = cdgettextsize; 
  canvas->cxPutImageRectRGB = cdputimagerectrgb;
  canvas->cxPutImageRectMap = cdputimagerectmap;
  canvas->cxScrollArea = cdscrollarea; 
  canvas->cxNewRegion = cdnewregion;
  canvas->cxIsPointInRegion = cdispointinregion;
  canvas->cxOffsetRegion = cdoffsetregion;
  canvas->cxGetRegionBox = cdgetregionbox;

  canvas->cxClip = cdclip;
  canvas->cxClipArea = cdcliparea; 
  canvas->cxBackOpacity = cdbackopacity;
  canvas->cxWriteMode = cdwritemode;
  canvas->cxLineStyle = cdlinestyle;
  canvas->cxLineWidth = cdlinewidth;
  canvas->cxLineCap = cdlinecap;
  canvas->cxLineJoin = cdlinejoin;
  canvas->cxInteriorStyle = cdinteriorstyle;
  canvas->cxHatch = cdhatch;
  canvas->cxStipple = cdstipple; 
  canvas->cxPattern = cdpattern;
  canvas->cxFont = cdfont;
  canvas->cxNativeFont = cdnativefont;
  canvas->cxTextOrientation = cdtextorientation; 
  canvas->cxTextAlignment = cdtextalignment; 
  canvas->cxPalette = cdpalette; 
  canvas->cxBackground = cdbackground; 
  canvas->cxForeground = cdforeground;
  canvas->cxTransform = cdtransform;

  canvas->cxKillCanvas = cdwKillCanvas;
  canvas->cxFlush = cdflush;

  if (ctxcanvas->wtype == CDW_WIN || ctxcanvas->wtype == CDW_BMP)
  {
    canvas->cxClear = cdclear; 
    canvas->cxGetImageRGB = cdgetimagergb; 
    canvas->cxPutImageRectRGBA = cdputimagerectrgba; 
    canvas->cxCreateImage = cdcreateimage; 
    canvas->cxGetImage = cdgetimage; 
    canvas->cxPutImageRect = cdputimagerect; 
    canvas->cxKillImage = cdkillimage; 
  }

  if (ctxcanvas->wtype == CDW_EMF)
    canvas->cxPutImageRectRGBA = cdputimagerectrgba; 
}

int cdBaseDriver(void)
{
  return CD_BASE_WIN;
}

