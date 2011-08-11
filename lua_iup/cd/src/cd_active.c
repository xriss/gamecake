/** \file
 * \brief OLD API that needs the cdActivate call
 *
 * See Copyright Notice in cd.h
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>

#ifdef CD_NO_OLD_INTERFACE
#undef CD_NO_OLD_INTERFACE
#endif

#include "cd.h"
#include "wd.h"

static cdCanvas *active_canvas = NULL;

int cdActivate(cdCanvas *canvas)
{
  /* if is the active canvas, just update canvas state */
  if (active_canvas && canvas == active_canvas)
  {
    if (cdCanvasActivate(canvas) == CD_ERROR)
    {
      active_canvas = NULL;
      return CD_ERROR;
    }

    return CD_OK;
  }

  /* if exists an active canvas, deactivate it */
  if (active_canvas) 
    cdCanvasDeactivate(active_canvas);
  
  /* allow to active a NULL canvas, the user may restore a previous canvas that was NULL */
  if (canvas == NULL)
  {
    active_canvas = NULL;
    return CD_ERROR;
  }

  /* do the activation */
  active_canvas = canvas;
  
  if (cdCanvasActivate(canvas) == CD_ERROR)
  {
    active_canvas = NULL;
    return CD_ERROR;
  }
  
  return CD_OK;
}

cdCanvas* cdActiveCanvas(void)
{
  return active_canvas;
}

int cdSimulate(int mode)
{
  assert(active_canvas);
  if (!active_canvas) return CD_ERROR;
  return cdCanvasSimulate(active_canvas, mode);
}

cdState* cdSaveState(void)
{
  assert(active_canvas);
  if (!active_canvas) return NULL;
  return cdCanvasSaveState(active_canvas);
}

void cdRestoreState(cdState* state)
{
  assert(active_canvas);
  if (!active_canvas) return;
  cdCanvasRestoreState(active_canvas, state);
}

void cdFlush(void)
{
  assert(active_canvas);
  if (!active_canvas) return;
  cdCanvasFlush(active_canvas);
}

void cdClear(void)
{
  assert(active_canvas);
  if (!active_canvas) return;
  cdCanvasClear(active_canvas);
}

void cdSetAttribute(const char* name, char *data)
{
  assert(active_canvas);
  if (!active_canvas) return;
  cdCanvasSetAttribute(active_canvas, name, data);
}

void cdSetfAttribute(const char* name, const char* format, ...)
{
  char data[1024];
  va_list arglist;

  assert(active_canvas);
  if (!active_canvas) return;

  va_start(arglist, format);
  vsprintf(data, format, arglist);

  cdCanvasSetAttribute(active_canvas, name, data);
}

char* cdGetAttribute(const char* name)
{
  assert(active_canvas);
  if (!active_canvas) return NULL;
  return cdCanvasGetAttribute(active_canvas, name);
}

int cdPlay(cdContext* context, int xmin, int xmax, int ymin, int ymax, void *data)
{
  assert(active_canvas);
  if (!active_canvas) return CD_ERROR;
  return cdCanvasPlay(active_canvas, context, xmin, xmax, ymin, ymax, data);
}

int cdClip(int mode)
{
  assert(active_canvas);
  if (!active_canvas) return CD_ERROR;
  return cdCanvasClip(active_canvas, mode);
}

void cdClipArea(int xmin, int xmax, int ymin, int ymax)
{
  assert(active_canvas);
  if (!active_canvas) return;
  cdCanvasClipArea(active_canvas, xmin, xmax, ymin, ymax);
}

int cdGetClipArea(int *xmin, int *xmax, int *ymin, int *ymax)
{
  assert(active_canvas);
  if (!active_canvas) return CD_ERROR;
  return cdCanvasGetClipArea(active_canvas, xmin, xmax, ymin, ymax);
}

int cdPointInRegion(int x, int y)
{
  assert(active_canvas);
  if (!active_canvas) return CD_ERROR;
  return cdCanvasIsPointInRegion(active_canvas, x, y);
}

void cdOffsetRegion(int x, int y)
{
  assert(active_canvas);
  if (!active_canvas) return;
  cdCanvasOffsetRegion(active_canvas, x, y);
}

void cdRegionBox(int *xmin, int *xmax, int *ymin, int *ymax)
{
  assert(active_canvas);
  if (!active_canvas) return;
  cdCanvasGetRegionBox(active_canvas, xmin, xmax, ymin, ymax);
}

void cdGetCanvasSize(int *width, int *height, double *width_mm, double *height_mm)
{
  assert(active_canvas);
  if (!active_canvas) return;
  cdCanvasGetSize(active_canvas, width, height, width_mm, height_mm);
}

void cdMM2Pixel(double mm_dx, double mm_dy, int *dx, int *dy)
{
  assert(active_canvas);
  if (!active_canvas) return;
  cdCanvasMM2Pixel(active_canvas, mm_dx, mm_dy, dx, dy);
}

void cdPixel2MM(int dx, int dy, double *mm_dx, double *mm_dy)
{
  assert(active_canvas);
  if (!active_canvas) return;
  cdCanvasPixel2MM(active_canvas, dx, dy, mm_dx, mm_dy);
}

void cdOrigin(int x, int y)
{
  assert(active_canvas);
  if (!active_canvas) return;
  cdCanvasOrigin(active_canvas, x, y);
}

int cdUpdateYAxis(int *y)
{
  assert(active_canvas);
  if (!active_canvas) return CD_ERROR;
  return cdCanvasUpdateYAxis(active_canvas, y);
}

void cdPixel(int x, int y, long color)
{
  assert(active_canvas);
  if (!active_canvas) return;
  cdCanvasPixel(active_canvas, x, y, color);
}

void cdMark(int x, int y)
{
  assert(active_canvas);
  if (!active_canvas) return;
  cdCanvasMark(active_canvas, x, y);
}

void cdLine(int x1, int y1, int x2, int y2)
{
  assert(active_canvas);
  if (!active_canvas) return;
  cdCanvasLine(active_canvas, x1, y1, x2, y2);
}

void cdBegin(int mode)
{
  assert(active_canvas);
  if (!active_canvas) return;
  cdCanvasBegin(active_canvas, mode);
}

void cdVertex(int x, int y)
{
  assert(active_canvas);
  if (!active_canvas) return;
  cdCanvasVertex(active_canvas, x, y);
}

void cdEnd(void)
{
  assert(active_canvas);
  if (!active_canvas) return;
  cdCanvasEnd(active_canvas);
}

void cdRect(int xmin, int xmax, int ymin, int ymax)
{
  assert(active_canvas);
  if (!active_canvas) return;
  cdCanvasRect(active_canvas, xmin, xmax, ymin, ymax);
}

void cdBox(int xmin, int xmax, int ymin, int ymax)
{
  assert(active_canvas);
  if (!active_canvas) return;
  cdCanvasBox(active_canvas, xmin, xmax, ymin, ymax);
}

void cdArc(int xc, int yc, int w, int h, double angle1, double angle2)
{
  assert(active_canvas);
  if (!active_canvas) return;
  cdCanvasArc(active_canvas, xc, yc, w, h, angle1, angle2);
}

void cdSector(int xc, int yc, int w, int h, double angle1, double angle2)
{
  assert(active_canvas);
  if (!active_canvas) return;
  cdCanvasSector(active_canvas, xc, yc, w, h, angle1, angle2);
}

void cdChord(int xc, int yc, int w, int h, double angle1, double angle2)
{
  assert(active_canvas);
  if (!active_canvas) return;
  cdCanvasChord(active_canvas, xc, yc, w, h, angle1, angle2);
}

void cdText(int x, int y, const char *s)
{
  assert(active_canvas);
  if (!active_canvas) return;
  cdCanvasText(active_canvas, x, y, s);
}

int cdBackOpacity(int opacity)
{
  assert(active_canvas);
  if (!active_canvas) return CD_ERROR;
  return cdCanvasBackOpacity(active_canvas, opacity);
}

int cdWriteMode(int mode)
{
  assert(active_canvas);
  if (!active_canvas) return CD_ERROR;
  return cdCanvasWriteMode(active_canvas, mode);
}

int cdLineStyle(int style)
{
  assert(active_canvas);
  if (!active_canvas) return CD_ERROR;
  return cdCanvasLineStyle(active_canvas, style);
}

int cdLineJoin(int join)
{
  assert(active_canvas);
  if (!active_canvas) return CD_ERROR;
  return cdCanvasLineJoin(active_canvas, join);
}

int cdLineCap(int cap)
{
  assert(active_canvas);
  if (!active_canvas) return CD_ERROR;
  return cdCanvasLineCap(active_canvas, cap);
}

int cdRegionCombineMode(int mode)
{
  assert(active_canvas);
  if (!active_canvas) return CD_ERROR;
  return cdCanvasRegionCombineMode(active_canvas, mode);
}

void cdLineStyleDashes(const int* dashes, int count)
{
  assert(active_canvas);
  if (!active_canvas) return;
  cdCanvasLineStyleDashes(active_canvas, dashes, count);
}

int cdLineWidth(int width)
{
  assert(active_canvas);
  if (!active_canvas) return CD_ERROR;
  return cdCanvasLineWidth(active_canvas, width);
}

int cdFillMode(int mode)
{
  assert(active_canvas);
  if (!active_canvas) return CD_ERROR;
  return cdCanvasFillMode(active_canvas, mode);
}

int cdInteriorStyle (int style)
{
  assert(active_canvas);
  if (!active_canvas) return CD_ERROR;
  return cdCanvasInteriorStyle(active_canvas, style);
}

int cdHatch(int style)
{
  assert(active_canvas);
  if (!active_canvas) return CD_ERROR;
  return cdCanvasHatch(active_canvas, style);
}

void cdStipple(int w, int h, const unsigned char *stipple)
{
  assert(active_canvas);
  if (!active_canvas) return;
  cdCanvasStipple(active_canvas, w, h, stipple);
}

unsigned char* cdGetStipple(int *w, int *h)
{
  assert(active_canvas);
  if (!active_canvas) return NULL;
  return cdCanvasGetStipple(active_canvas, w, h);
}

void cdPattern(int w, int h, const long *pattern)
{
  assert(active_canvas);
  if (!active_canvas) return;
  cdCanvasPattern(active_canvas, w, h, pattern);
}

long* cdGetPattern(int* w, int* h)
{
  assert(active_canvas);
  if (!active_canvas) return NULL;
  return cdCanvasGetPattern(active_canvas, w, h);
}

void cdFont(int type_face, int style, int size)
{
  static const char * family[] = 
  {
    "System",       /* CD_SYSTEM */
    "Courier",      /* CD_COURIER */
    "Times",        /* CD_TIMES_ROMAN */
    "Helvetica"     /* CD_HELVETICA */
  };

  assert(active_canvas);
  assert(type_face>=CD_SYSTEM && type_face<=CD_NATIVE);
  if (!active_canvas) return;
  if (type_face<CD_SYSTEM || type_face>CD_NATIVE) return;

  if (type_face == CD_NATIVE)
  {
    char* native_font = cdCanvasNativeFont(active_canvas, NULL);
    cdCanvasNativeFont(active_canvas, native_font);
  }                                        
  else
    cdCanvasFont(active_canvas, family[type_face], style, size);
}

void cdGetFont(int *type_face, int *style, int *size)
{
  char family[1024];
  assert(active_canvas);
  if (!active_canvas) return;
  cdCanvasGetFont(active_canvas, family, style, size);

  if (type_face)
  {
    if (strcmp(family, "System")==0) 
      *type_face = CD_SYSTEM;
    else if (strcmp(family, "Courier")==0) 
      *type_face = CD_COURIER;
    else if (strcmp(family, "Times")==0) 
      *type_face = CD_TIMES_ROMAN;
    else if (strcmp(family, "Helvetica")==0) 
      *type_face = CD_HELVETICA;
    else
      *type_face = CD_NATIVE;
  }
}

char* cdNativeFont(const char* font)
{
  assert(active_canvas);
  if (!active_canvas) return NULL;
  return cdCanvasNativeFont(active_canvas, font);
}

int cdTextAlignment(int alignment)
{
  assert(active_canvas);
  if (!active_canvas) return CD_ERROR;
  return cdCanvasTextAlignment(active_canvas, alignment);
}

double cdTextOrientation(double angle)
{
  assert(active_canvas);
  if (!active_canvas) return CD_ERROR;
  return cdCanvasTextOrientation(active_canvas, angle);
}

int cdMarkType(int type)
{
  assert(active_canvas);
  if (!active_canvas) return CD_ERROR;
  return cdCanvasMarkType(active_canvas, type);
}

int cdMarkSize(int size)
{
  assert(active_canvas);
  if (!active_canvas) return CD_ERROR;
  return cdCanvasMarkSize(active_canvas, size);
}

long cdBackground(long color)
{
  assert(active_canvas);
  if (!active_canvas) return CD_ERROR;
  return cdCanvasBackground(active_canvas, color);
}

long cdForeground(long color)
{
  assert(active_canvas);
  if (!active_canvas) return CD_ERROR;
  return cdCanvasForeground(active_canvas, color);
}

void cdFontDim(int *max_width, int *height, int *ascent, int *descent)
{
  assert(active_canvas);
  if (!active_canvas) return;
  cdCanvasGetFontDim(active_canvas, max_width, height, ascent, descent);
}

void cdTextSize(const char *s, int *width, int *height)
{
  assert(active_canvas);
  if (!active_canvas) return;
  cdCanvasGetTextSize(active_canvas, s, width, height);
}

void cdTextBounds(int x, int y, const char *s, int *rect)
{
  assert(active_canvas);
  if (!active_canvas) return;
  cdCanvasGetTextBounds(active_canvas, x, y, s, rect);
}

void cdTextBox(int x, int y, const char *s, int *xmin, int *xmax, int *ymin, int *ymax)
{
  assert(active_canvas);
  if (!active_canvas) return;
  cdCanvasGetTextBox(active_canvas, x, y, s, xmin, xmax, ymin, ymax);
}

int cdGetColorPlanes(void)
{
  assert(active_canvas);
  if (!active_canvas) return CD_ERROR;
  return cdCanvasGetColorPlanes(active_canvas);
}

void cdPalette(int n, const long *palette, int mode)
{
  assert(active_canvas);
  if (!active_canvas) return;
  cdCanvasPalette(active_canvas, n, palette, mode);
}

void cdGetImageRGB(unsigned char *r, unsigned char *g, unsigned char *b, int x, int y, int w, int h)
{
  assert(active_canvas);
  if (!active_canvas) return;
  cdCanvasGetImageRGB(active_canvas, r, g, b, x, y, w, h);
}

void cdPutImageRectRGB(int iw, int ih, const unsigned char *r, const unsigned char *g, const unsigned char *b, int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax)
{
  assert(active_canvas);
  if (!active_canvas) return;
  cdCanvasPutImageRectRGB(active_canvas, iw, ih, r, g, b, x, y, w, h, xmin, xmax, ymin, ymax);
}

void cdPutImageRectRGBA(int iw, int ih, const unsigned char *r, const unsigned char *g, const unsigned char *b, const unsigned char *a, int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax)
{
  assert(active_canvas);
  if (!active_canvas) return;
  cdCanvasPutImageRectRGBA(active_canvas, iw, ih, r, g, b, a, x, y, w, h, xmin, xmax, ymin, ymax);
}

void cdPutImageRectMap(int iw, int ih, const unsigned char *index, const long *colors, int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax)
{
  assert(active_canvas);
  if (!active_canvas) return;
  cdCanvasPutImageRectMap(active_canvas, iw, ih, index, colors, x, y, w, h, xmin, xmax, ymin, ymax);
}

cdImage* cdCreateImage(int w, int h)
{
  assert(active_canvas);
  if (!active_canvas) return NULL;
  return cdCanvasCreateImage(active_canvas, w, h);
}

void cdGetImage(cdImage* image, int x, int y)
{
  assert(active_canvas);
  if (!active_canvas) return;
  cdCanvasGetImage(active_canvas, image, x, y);
}

void cdPutImageRect(cdImage* image, int x, int y, int xmin, int xmax, int ymin, int ymax)
{
  assert(active_canvas);
  if (!active_canvas) return;
  cdCanvasPutImageRect(active_canvas, image, x, y, xmin, xmax, ymin, ymax);
}

void cdScrollArea(int xmin, int xmax, int ymin, int ymax, int dx, int dy)
{
  assert(active_canvas);
  if (!active_canvas) return;
  cdCanvasScrollArea(active_canvas, xmin, xmax, ymin, ymax, dx, dy);
}

void cdPutBitmap(cdBitmap* bitmap, int x, int y, int w, int h)
{
  assert(active_canvas);
  if (!active_canvas) return;
  cdCanvasPutBitmap(active_canvas, bitmap, x, y, w, h);
}

void cdGetBitmap(cdBitmap* bitmap, int x, int y)
{
  assert(active_canvas);
  if (!active_canvas) return;
  cdCanvasGetBitmap(active_canvas, bitmap, x, y);
}

void wdWindow(double xmin, double xmax, double  ymin, double ymax)
{
  assert(active_canvas);
  if (!active_canvas) return;
  wdCanvasWindow(active_canvas, xmin, xmax, ymin, ymax);
}

void wdGetWindow (double *xmin, double  *xmax,  double  *ymin, double *ymax)
{
  assert(active_canvas);
  if (!active_canvas) return;
  wdCanvasGetWindow(active_canvas, xmin, xmax, ymin, ymax);
}

void wdViewport(int xmin, int xmax, int ymin, int ymax)
{
  assert(active_canvas);
  if (!active_canvas) return;
  wdCanvasViewport(active_canvas, xmin, xmax, ymin, ymax);
}

void wdGetViewport (int *xmin, int *xmax, int *ymin, int *ymax)
{
  assert(active_canvas);
  if (!active_canvas) return;
  wdCanvasGetViewport(active_canvas, xmin, xmax, ymin, ymax);
}

void wdWorld2Canvas(double xw, double yw, int *xv, int *yv)
{
  assert(active_canvas);
  if (!active_canvas) return;
  wdCanvasWorld2Canvas(active_canvas, xw, yw, xv, yv);
}

void wdWorld2CanvasSize(double hw, double vw, int *hv, int *vv)
{
  assert(active_canvas);
  if (!active_canvas) return;
  wdCanvasWorld2CanvasSize(active_canvas, hw, vw, hv, vv);
}

void wdCanvas2World(int xv, int yv, double *xw, double *yw)
{
  assert(active_canvas);
  if (!active_canvas) return;
  wdCanvasCanvas2World(active_canvas, xv, yv, xw, yw);
}

void wdClipArea(double xmin, double xmax, double ymin, double ymax)
{
  assert(active_canvas);
  if (!active_canvas) return;
  wdCanvasClipArea(active_canvas, xmin, xmax, ymin, ymax);
}

int wdPointInRegion(double x, double y)
{
  assert(active_canvas);
  if (!active_canvas) return CD_ERROR;
  return wdCanvasIsPointInRegion(active_canvas, x, y);
}

void wdOffsetRegion(double x, double y)
{
  assert(active_canvas);
  if (!active_canvas) return;
  wdCanvasOffsetRegion(active_canvas, x, y);
}

void wdRegionBox(double *xmin, double *xmax, double *ymin, double *ymax)
{
  assert(active_canvas);
  if (!active_canvas) return;
  wdCanvasGetRegionBox(active_canvas, xmin, xmax, ymin, ymax);
}

int wdGetClipArea(double *xmin, double *xmax, double *ymin, double *ymax)
{
  assert(active_canvas);
  if (!active_canvas) return CD_ERROR;
  return wdCanvasGetClipArea(active_canvas, xmin, xmax, ymin, ymax);
}

void wdLine (double x1, double y1, double x2, double y2)
{
  assert(active_canvas);
  if (!active_canvas) return;
  wdCanvasLine(active_canvas, x1, y1, x2, y2);
}

void wdBox (double xmin, double xmax, double ymin, double ymax)
{
  assert(active_canvas);
  if (!active_canvas) return;
  wdCanvasBox(active_canvas, xmin, xmax, ymin, ymax);
}

void wdRect(double xmin, double xmax, double ymin, double ymax)
{
  assert(active_canvas);
  if (!active_canvas) return;
  wdCanvasRect(active_canvas, xmin, xmax, ymin, ymax);
}

void wdArc(double xc, double yc, double w, double h, double angle1, double angle2)
{
  assert(active_canvas);
  if (!active_canvas) return;
  wdCanvasArc(active_canvas, xc, yc, w, h, angle1, angle2);
}

void wdSector(double xc, double yc, double w, double h, double angle1, double angle2)
{
  assert(active_canvas);
  if (!active_canvas) return;
  wdCanvasSector(active_canvas, xc, yc, w, h, angle1, angle2);
}

void wdChord(double xc, double yc, double w, double h, double angle1, double angle2)
{
  assert(active_canvas);
  if (!active_canvas) return;
  wdCanvasChord(active_canvas, xc, yc, w, h, angle1, angle2);
}

void wdText(double x, double y, const char *s)
{
  assert(active_canvas);
  if (!active_canvas) return;
  wdCanvasText(active_canvas, x, y, s);
}

void wdVertex(double x, double y)
{
  assert(active_canvas);
  if (!active_canvas) return;
  wdCanvasVertex(active_canvas, x, y);
}

void wdMark(double x, double y)
{
  assert(active_canvas);
  if (!active_canvas) return;
  wdCanvasMark(active_canvas, x, y);
}

void wdPixel(double x, double y, long color)
{
  assert(active_canvas);
  if (!active_canvas) return;
  wdCanvasPixel(active_canvas, x, y, color);
}

void wdPutImageRect(cdImage* image, double x, double y, int xmin, int xmax, int ymin, int ymax)
{
  assert(active_canvas);
  if (!active_canvas) return;
  wdCanvasPutImageRect(active_canvas, image, x, y, xmin, xmax, ymin, ymax);
}

void wdPutImageRectRGB(int iw, int ih, const unsigned char *r, const unsigned char *g, const unsigned char *b, double x, double y, double w, double h, int xmin, int xmax, int ymin, int ymax)
{
  assert(active_canvas);
  if (!active_canvas) return;
  wdCanvasPutImageRectRGB(active_canvas, iw, ih, r, g, b, x, y, w, h, xmin, xmax, ymin, ymax);
}

void wdPutImageRectRGBA(int iw, int ih, const unsigned char *r, const unsigned char *g, const unsigned char *b, const unsigned char *a, double x, double y, double w, double h, int xmin, int xmax, int ymin, int ymax)
{
  assert(active_canvas);
  if (!active_canvas) return;
  wdCanvasPutImageRectRGBA(active_canvas, iw, ih, r, g, b, a, x, y, w, h, xmin, xmax, ymin, ymax);
}

void wdPutImageRectMap(int iw, int ih, const unsigned char *index, const long *colors, double x, double y, double w, double h, int xmin, int xmax, int ymin, int ymax)
{
  assert(active_canvas);
  if (!active_canvas) return;
  wdCanvasPutImageRectMap(active_canvas, iw, ih, index, colors, x, y, w, h, xmin, xmax, ymin, ymax);
}

void wdPutBitmap(cdBitmap* bitmap, double x, double y, double w, double h)
{
  assert(active_canvas);
  if (!active_canvas) return;
  wdCanvasPutBitmap(active_canvas, bitmap, x, y, w, h);
}

double wdLineWidth(double width_mm)
{
  assert(active_canvas);
  if (!active_canvas) return CD_ERROR;
  return wdCanvasLineWidth(active_canvas, width_mm);
}

void wdFont(int type_face, int style, double size_mm)
{
  static const char * family[] = 
  {
    "System",       /* CD_SYSTEM */
    "Courier",      /* CD_COURIER */
    "Times Roman",  /* CD_TIMES_ROMAN */
    "Helvetica"     /* CD_HELVETICA */
  };

  assert(active_canvas);
  if (!active_canvas) return;
  if (type_face<CD_SYSTEM || type_face>CD_HELVETICA) return;

  wdCanvasFont(active_canvas, family[type_face], style, (int)(size_mm*CD_MM2PT + 0.5));
}

void wdGetFont(int *type_face, int *style, double *size)
{
  char family[1024];
  assert(active_canvas);
  if (!active_canvas) return;
  wdCanvasGetFont(active_canvas, family, style, size);

  if (type_face)
  {
    if (strcmp(family, "System")==0) 
      *type_face = CD_SYSTEM;
    else if (strcmp(family, "Courier")==0) 
      *type_face = CD_COURIER;
    else if (strcmp(family, "Times Roman")==0) 
      *type_face = CD_TIMES_ROMAN;
    else if (strcmp(family, "Helvetica")==0) 
      *type_face = CD_HELVETICA;
    else
      *type_face = CD_NATIVE;
  }
}

double wdMarkSize(double size_mm)
{
  assert(active_canvas);
  if (!active_canvas) return CD_ERROR;
  return wdCanvasMarkSize(active_canvas, size_mm);
}

void wdFontDim(double *max_width, double *height, double *ascent, double *descent)
{
  assert(active_canvas);
  if (!active_canvas) return;
  wdCanvasGetFontDim(active_canvas, max_width, height, ascent, descent);
}

void wdTextSize(const char *s, double *width, double *height)
{
  assert(active_canvas);
  if (!active_canvas) return;
  wdCanvasGetTextSize(active_canvas, s, width, height);
}

void wdTextBox(double x, double y, const char *s, double *xmin, double *xmax, double *ymin, double *ymax)
{
  assert(active_canvas);
  if (!active_canvas) return;
  wdCanvasGetTextBox(active_canvas, x, y, s, xmin, xmax, ymin, ymax);
}

void wdTextBounds(double x, double y, const char *s, double *rect)
{
  assert(active_canvas);
  if (!active_canvas) return;
  wdCanvasGetTextBounds(active_canvas, x, y, s, rect);
}

void wdPattern(int w, int h, const long *color, double w_mm, double h_mm)
{
  assert(active_canvas);
  if (!active_canvas) return;
  wdCanvasPattern(active_canvas, w, h, color, w_mm, h_mm);
}

void wdStipple(int w, int h, const unsigned char *fgbg, double w_mm, double h_mm)
{
  assert(active_canvas);
  if (!active_canvas) return;
  wdCanvasStipple(active_canvas, w, h, fgbg, w_mm, h_mm);
}

void wdHardcopy(cdContext* ctx, void *data, cdCanvas *canvas, void(*draw_func)(void))
{
  wdCanvasHardcopy(canvas, ctx, data, (void(*)(cdCanvas*))draw_func);
}

void cdVectorText(int x, int y, const char* s)
{
  assert(active_canvas);
  if (!active_canvas) return;
  cdCanvasVectorText(active_canvas, x, y, s);
}

void cdMultiLineVectorText(int x, int y, const char* s)
{
  assert(active_canvas);
  if (!active_canvas) return;
  cdCanvasMultiLineVectorText(active_canvas, x, y, s);
}

char *cdVectorFont(const char *filename)
{
  assert(active_canvas);
  if (!active_canvas) return NULL;
  return cdCanvasVectorFont(active_canvas, filename);
}

void cdVectorTextDirection(int x1, int y1, int x2, int y2)
{
  assert(active_canvas);
  if (!active_canvas) return;
  cdCanvasVectorTextDirection(active_canvas, x1, y1, x2, y2);
}

double* cdVectorTextTransform(const double* matrix)
{
  assert(active_canvas);
  if (!active_canvas) return NULL;
  return cdCanvasVectorTextTransform(active_canvas, matrix);
}

void cdVectorTextSize(int size_x, int size_y, const char* s)
{
  assert(active_canvas);
  if (!active_canvas) return;
  cdCanvasVectorTextSize(active_canvas, size_x, size_y, s);
}

int cdVectorCharSize(int size)
{
  assert(active_canvas);
  if (!active_canvas) return CD_ERROR;
  return cdCanvasVectorCharSize(active_canvas, size);
}

void cdGetVectorTextSize(const char* s, int *x, int *y)
{
  assert(active_canvas);
  if (!active_canvas) return;
  cdCanvasGetVectorTextSize(active_canvas, s, x, y);
}

void cdGetVectorTextBounds(const char* s, int x, int y, int *rect)
{
  assert(active_canvas);
  if (!active_canvas) return;
  cdCanvasGetVectorTextBounds(active_canvas, s, x, y, rect);
}

void wdVectorTextDirection(double x1, double y1, double x2, double y2)
{
  assert(active_canvas);
  if (!active_canvas) return;
  wdCanvasVectorTextDirection(active_canvas, x1, y1, x2, y2);
}

void wdVectorTextSize(double size_x, double size_y, const char* s)
{
  assert(active_canvas);
  if (!active_canvas) return;
  wdCanvasVectorTextSize(active_canvas, size_x, size_y, s);
}

void wdGetVectorTextSize(const char* s, double *x, double *y)
{
  assert(active_canvas);
  if (!active_canvas) return;
  wdCanvasGetVectorTextSize(active_canvas, s, x, y);
}

double wdVectorCharSize(double size)
{
  assert(active_canvas);
  if (!active_canvas) return CD_ERROR;
  return wdCanvasVectorCharSize(active_canvas, size);
}

void wdVectorText(double x, double y, const char* s)
{
  assert(active_canvas);
  if (!active_canvas) return;
  wdCanvasVectorText(active_canvas, x, y, s);
}

void wdMultiLineVectorText(double x, double y, const char* s)
{
  assert(active_canvas);
  if (!active_canvas) return;
  wdCanvasMultiLineVectorText(active_canvas, x, y, s);
}

void wdGetVectorTextBounds(const char* s, double x, double y, double *rect)
{
  assert(active_canvas);
  if (!active_canvas) return;
  wdCanvasGetVectorTextBounds(active_canvas, s, x, y, rect);
}
