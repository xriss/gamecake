/** \file
 * \brief World Coordinate Functions
 *
 * See Copyright Notice in cd.h
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include <memory.h>

#include "cd.h"
#include "wd.h"

#include "cd_private.h"


static void wdUpdateTransformation(cdCanvas* canvas)
{
  if (canvas->window.xmax != canvas->window.xmin)
    canvas->sx = (canvas->viewport.xmax - canvas->viewport.xmin)/(canvas->window.xmax - canvas->window.xmin);
  else
    canvas->sx = 0;
  canvas->tx =  canvas->viewport.xmin - canvas->window.xmin*canvas->sx;

  if (canvas->window.ymax != canvas->window.ymin)
    canvas->sy = (canvas->viewport.ymax - canvas->viewport.ymin)/(canvas->window.ymax - canvas->window.ymin);
  else
    canvas->sy = 0;
  canvas->ty =  canvas->viewport.ymin - canvas->window.ymin*canvas->sy;

  canvas->s = sqrt(canvas->sx * canvas->sx + canvas->sy * canvas->sy);
}

void wdCanvasSetTransform(cdCanvas* canvas, double sx, double sy, double tx, double ty)
{
  assert(canvas);
  if (!_cdCheckCanvas(canvas)) return;

  canvas->sx = sx;
  canvas->tx = tx;
  canvas->sy = sy;
  canvas->ty = ty;
  canvas->s = sqrt(canvas->sx * canvas->sx + canvas->sy * canvas->sy);
}

void wdCanvasGetTransform(cdCanvas* canvas, double *sx, double *sy, double *tx, double *ty)
{
  assert(canvas);
  if (!_cdCheckCanvas(canvas)) return;

  if (sx) *sx = canvas->sx;
  if (tx) *tx = canvas->tx;
  if (sy) *sy = canvas->sy;
  if (ty) *ty = canvas->ty;
}

void wdCanvasTranslate(cdCanvas* canvas, double dtx, double dty)
{
  assert(canvas);
  if (!_cdCheckCanvas(canvas)) return;

  canvas->tx += dtx;
  canvas->ty += dty;
}

void wdCanvasScale(cdCanvas* canvas, double dsx, double dsy)
{
  assert(canvas);
  if (!_cdCheckCanvas(canvas)) return;

  canvas->sx *= dsx;
  canvas->sy *= dsy;
}

void wdSetDefaults(cdCanvas* canvas)
{
  canvas->window.xmin = 0;
  canvas->window.xmax = canvas->w_mm;
  canvas->window.ymin = 0;
  canvas->window.ymax = canvas->h_mm;

  canvas->viewport.xmin = 0;
  canvas->viewport.xmax = canvas->w-1;
  canvas->viewport.ymin = 0;
  canvas->viewport.ymax = canvas->h-1;

  wdUpdateTransformation(canvas);
}

void wdCanvasWindow(cdCanvas* canvas, double xmin, double xmax, double  ymin, double ymax)
{
  assert(canvas);
  if (!_cdCheckCanvas(canvas)) return;

  canvas->window.xmin = xmin;
  canvas->window.xmax = xmax;
  canvas->window.ymin = ymin;
  canvas->window.ymax = ymax;

  wdUpdateTransformation(canvas);
}

void wdCanvasGetWindow (cdCanvas* canvas, double *xmin, double  *xmax,  double  *ymin, double *ymax)
{
  assert(canvas);
  if (!_cdCheckCanvas(canvas)) return;

  if (xmin) *xmin = canvas->window.xmin;
  if (xmax) *xmax = canvas->window.xmax;
  if (ymin) *ymin = canvas->window.ymin;
  if (ymax) *ymax = canvas->window.ymax;
}

void wdCanvasViewport(cdCanvas* canvas, int xmin, int xmax, int ymin, int ymax)
{
  assert(canvas);
  if (!_cdCheckCanvas(canvas)) return;

  canvas->viewport.xmin = xmin;
  canvas->viewport.xmax = xmax;
  canvas->viewport.ymin = ymin;
  canvas->viewport.ymax = ymax;

  wdUpdateTransformation(canvas);
}

void wdCanvasGetViewport(cdCanvas* canvas, int *xmin, int *xmax, int *ymin, int *ymax)
{
  assert(canvas);
  if (!_cdCheckCanvas(canvas)) return;

  if (xmin) *xmin = canvas->viewport.xmin;
  if (xmax) *xmax = canvas->viewport.xmax;
  if (ymin) *ymin = canvas->viewport.ymin;
  if (ymax) *ymax = canvas->viewport.ymax;
}

#define _wWorld2Canvas(_canvas, _xw, _yw, _xv, _yv)  \
{                                                    \
  _xv = cdRound(_canvas->sx*(_xw) + _canvas->tx);      \
  _yv = cdRound(_canvas->sy*(_yw) + _canvas->ty);      \
}

#define _wfWorld2Canvas(_canvas, _xw, _yw, _xv, _yv) \
{                                                    \
  _xv = (_canvas->sx*(_xw) + _canvas->tx);           \
  _yv = (_canvas->sy*(_yw) + _canvas->ty);           \
}

void wdCanvasWorld2Canvas(cdCanvas* canvas, double xw, double yw, int *xv, int *yv)
{
  if (xv) *xv = cdRound(canvas->sx*xw + canvas->tx);
  if (yv) *yv = cdRound(canvas->sy*yw + canvas->ty);
}

#define _wWorld2CanvasSize(_canvas, _Ww, _Hw, _Wv, _Hv)  \
{                                                        \
  _Wv = cdRound(_canvas->sx*(_Ww));                        \
  _Hv = cdRound(_canvas->sy*(_Hw));                        \
}

#define _wfWorld2CanvasSize(_canvas, _Ww, _Hw, _Wv, _Hv) \
{                                                        \
  _Wv = (_canvas->sx*(_Ww));                             \
  _Hv = (_canvas->sy*(_Hw));                             \
}

void wdCanvasWorld2CanvasSize(cdCanvas* canvas, double hw, double vw, int *hv, int *vv)
{
  if (hv) *hv = cdRound(canvas->sx*hw);
  if (vv) *vv = cdRound(canvas->sy*vw);
}

#define _wCanvas2World(_canvas, _xv, _yv, _xw, _yw)  \
{                                                    \
  _xw = ((double)(_xv) - _canvas->tx)/_canvas->sx;   \
  _yw = ((double)(_yv) - _canvas->ty)/_canvas->sy;   \
}

void wdCanvasCanvas2World(cdCanvas* canvas, int xv, int yv, double *xw, double *yw)
{
  if (xw) *xw = ((double)xv - canvas->tx)/canvas->sx;
  if (yw) *yw = ((double)yv - canvas->ty)/canvas->sy;
}

void wdCanvasClipArea(cdCanvas* canvas, double xmin, double xmax, double ymin, double ymax)
{
  int xminr, xmaxr, yminr, ymaxr;
  assert(canvas);
  if (!_cdCheckCanvas(canvas)) return;

  _wWorld2Canvas(canvas, xmin, ymin, xminr, yminr);
  _wWorld2Canvas(canvas, xmax, ymax, xmaxr, ymaxr);

  cdCanvasClipArea(canvas, xminr, xmaxr, yminr, ymaxr);
}

int wdCanvasIsPointInRegion(cdCanvas* canvas, double x, double y)
{
  int xr, yr;
  assert(canvas);
  if (!_cdCheckCanvas(canvas)) return CD_ERROR;

  _wWorld2Canvas(canvas, x, y, xr, yr);
  return cdCanvasIsPointInRegion(canvas, xr, yr);
}

void wdCanvasOffsetRegion(cdCanvas* canvas, double x, double y)
{
  int xr, yr;
  assert(canvas);
  if (!_cdCheckCanvas(canvas)) return;

  _wWorld2Canvas(canvas, x, y, xr, yr);
  cdCanvasOffsetRegion(canvas, xr, yr);
}

void wdCanvasGetRegionBox(cdCanvas* canvas, double *xmin, double *xmax, double *ymin, double *ymax)
{
  int xminr, xmaxr, yminr, ymaxr;
  assert(canvas);
  if (!_cdCheckCanvas(canvas)) return;

  cdCanvasGetRegionBox(canvas, &xminr, &xmaxr, &yminr, &ymaxr);
  _wCanvas2World(canvas, xminr, yminr, *xmin, *ymin);
  _wCanvas2World(canvas, xmaxr, ymaxr, *xmax, *ymax);
}

int wdCanvasGetClipArea(cdCanvas* canvas, double *xmin, double *xmax, double *ymin, double *ymax)
{
  int xminr, xmaxr, yminr, ymaxr, clip;
  assert(canvas);
  if (!_cdCheckCanvas(canvas)) return CD_ERROR;

  clip = cdCanvasGetClipArea(canvas, &xminr, &xmaxr, &yminr, &ymaxr);
  _wCanvas2World(canvas, xminr, yminr, *xmin, *ymin);
  _wCanvas2World(canvas, xmaxr, ymaxr, *xmax, *ymax);
  return clip;
}

void wdCanvasLine(cdCanvas* canvas, double x1, double y1, double x2, double y2)
{
  double xr1, xr2, yr1, yr2;
  assert(canvas);
  if (!_cdCheckCanvas(canvas)) return;

  _wfWorld2Canvas(canvas, x1, y1, xr1, yr1);
  _wfWorld2Canvas(canvas, x2, y2, xr2, yr2);
  cdfCanvasLine(canvas, xr1, yr1, xr2, yr2);
}

void wdCanvasBox(cdCanvas* canvas, double xmin, double xmax, double ymin, double ymax)
{
  double xminr, xmaxr, yminr, ymaxr;
  assert(canvas);
  if (!_cdCheckCanvas(canvas)) return;

  _wfWorld2Canvas(canvas, xmin, ymin, xminr, yminr);
  _wfWorld2Canvas(canvas, xmax, ymax, xmaxr, ymaxr);
  cdfCanvasBox(canvas, xminr, xmaxr, yminr, ymaxr);
}

void wdCanvasRect(cdCanvas* canvas, double xmin, double xmax, double ymin, double ymax)
{
  double xminr, xmaxr, yminr, ymaxr;
  assert(canvas);
  if (!_cdCheckCanvas(canvas)) return;

  _wfWorld2Canvas(canvas, xmin, ymin, xminr, yminr);
  _wfWorld2Canvas(canvas, xmax, ymax, xmaxr, ymaxr);
  cdfCanvasRect(canvas, xminr, xmaxr, yminr, ymaxr);
}

void wdCanvasArc(cdCanvas* canvas, double xc, double yc, double w, double h, double angle1, double angle2)
{
  double xcr, ycr, wr, hr;
  assert(canvas);
  if (!_cdCheckCanvas(canvas)) return;

  _wfWorld2Canvas(canvas, xc, yc, xcr, ycr);
  _wfWorld2CanvasSize(canvas, w, h, wr, hr);
  cdfCanvasArc(canvas, xcr, ycr, wr, hr, angle1, angle2);
}

void wdCanvasSector(cdCanvas* canvas, double xc, double yc, double w, double h, double angle1, double angle2)
{
  double xcr, ycr, wr, hr;
  assert(canvas);
  if (!_cdCheckCanvas(canvas)) return;

  _wfWorld2Canvas(canvas, xc, yc, xcr, ycr);
  _wfWorld2CanvasSize(canvas, w, h, wr, hr);
  cdfCanvasSector(canvas, xcr, ycr, wr, hr, angle1, angle2);
}

void wdCanvasChord(cdCanvas* canvas, double xc, double yc, double w, double h, double angle1, double angle2)
{
  double xcr, ycr, wr, hr;
  assert(canvas);
  if (!_cdCheckCanvas(canvas)) return;

  _wfWorld2Canvas(canvas, xc, yc, xcr, ycr);
  _wfWorld2CanvasSize(canvas, w, h, wr, hr);
  cdfCanvasChord(canvas, xcr, ycr, wr, hr, angle1, angle2);
}

void wdCanvasText(cdCanvas* canvas, double x, double y, const char *s)
{
  double xr, yr;
  assert(canvas);
  if (!_cdCheckCanvas(canvas)) return;

  _wfWorld2Canvas(canvas, x, y, xr, yr);
  cdfCanvasText(canvas, xr, yr, s);
}

void wdCanvasVertex(cdCanvas* canvas, double x, double y)
{
  double xr, yr;
  assert(canvas);
  if (!_cdCheckCanvas(canvas)) return;

  _wfWorld2Canvas(canvas, x, y, xr, yr);
  cdfCanvasVertex(canvas, xr, yr);
}

void wdCanvasMark(cdCanvas* canvas, double x, double y)
{
  int xr, yr;
  assert(canvas);
  if (!_cdCheckCanvas(canvas)) return;

  _wWorld2Canvas(canvas, x, y, xr, yr);
  cdCanvasMark(canvas, xr, yr);
}

void wdCanvasPixel(cdCanvas* canvas, double x, double y, long color)
{
  int xr, yr;
  assert(canvas);
  if (!_cdCheckCanvas(canvas)) return;

  _wWorld2Canvas(canvas, x, y, xr, yr);
  cdCanvasPixel(canvas, xr, yr, color);
}

void wdCanvasPutImageRect(cdCanvas* canvas, cdImage* image, double x, double y, int xmin, int xmax, int ymin, int ymax)
{
  int xr, yr;
  assert(canvas);
  if (!_cdCheckCanvas(canvas)) return;

  _wWorld2Canvas(canvas, x, y, xr, yr);
  cdCanvasPutImageRect(canvas, image, xr, yr, xmin, xmax, ymin, ymax);
}

void wdCanvasPutImageRectRGB(cdCanvas* canvas, int iw, int ih, const unsigned char *r, const unsigned char *g, const unsigned char *b, double x, double y, double w, double h, int xmin, int xmax, int ymin, int ymax)
{
  int xr, yr, wr, hr;
  assert(canvas);
  if (!_cdCheckCanvas(canvas)) return;

  _wWorld2Canvas(canvas, x, y, xr, yr);
  _wWorld2CanvasSize(canvas, w, h, wr, hr);
  cdCanvasPutImageRectRGB(canvas, iw, ih, r, g, b, xr, yr, wr, hr, xmin, xmax, ymin, ymax);
}

void wdCanvasPutImageRectRGBA(cdCanvas* canvas, int iw, int ih, const unsigned char *r, const unsigned char *g, const unsigned char *b, const unsigned char *a, double x, double y, double w, double h, int xmin, int xmax, int ymin, int ymax)
{
  int xr, yr, wr, hr;
  assert(canvas);
  if (!_cdCheckCanvas(canvas)) return;

  _wWorld2Canvas(canvas, x, y, xr, yr);
  _wWorld2CanvasSize(canvas, w, h, wr, hr);
  cdCanvasPutImageRectRGBA(canvas, iw, ih, r, g, b, a, xr, yr, wr, hr, xmin, xmax, ymin, ymax);
}

void wdCanvasPutImageRectMap(cdCanvas* canvas, int iw, int ih, const unsigned char *index, const long *colors, double x, double y, double w, double h, int xmin, int xmax, int ymin, int ymax)
{
  int xr, yr, wr, hr;
  assert(canvas);
  if (!_cdCheckCanvas(canvas)) return;

  _wWorld2Canvas(canvas, x, y, xr, yr);
  _wWorld2CanvasSize(canvas, w, h, wr, hr);
  cdCanvasPutImageRectMap(canvas, iw, ih, index, colors, xr, yr, wr, hr, xmin, xmax, ymin, ymax);
}

void wdCanvasPutBitmap(cdCanvas* canvas, cdBitmap* image, double x, double y, double w, double h)
{
  int xr, yr, wr, hr;
  assert(canvas);
  if (!_cdCheckCanvas(canvas)) return;

  _wWorld2Canvas(canvas, x, y, xr, yr);
  _wWorld2CanvasSize(canvas, w, h, wr, hr);
  cdCanvasPutBitmap(canvas, image, xr, yr, wr, hr);
}

double wdCanvasLineWidth(cdCanvas* canvas, double width_mm)
{
  int width;
  double line_width_mm;
  assert(canvas);
  if (!_cdCheckCanvas(canvas)) return CD_ERROR;

  line_width_mm = canvas->line_width/canvas->xres;
  if (width_mm == CD_QUERY)
    return line_width_mm;

  width = cdRound(width_mm*canvas->xres);
  if (width < 1) width = 1;

  cdCanvasLineWidth(canvas, width);

  return line_width_mm;
}

int wdCanvasFont(cdCanvas* canvas, const char* type_face, int style, double size_mm)
{
  return cdCanvasFont(canvas, type_face, style, cdRound(size_mm*CD_MM2PT));
}

void wdCanvasGetFont(cdCanvas* canvas, char *type_face, int *style, double *size)
{
  int point_size;
  cdCanvasGetFont(canvas, type_face, style, &point_size);
  if (point_size<0)
  {
    if (size) cdCanvasPixel2MM(canvas, -point_size, 0, size, NULL);
  }
  else
  {
    if (size) *size = ((double)point_size) / CD_MM2PT;
  }
}

double wdCanvasMarkSize(cdCanvas* canvas, double size_mm)
{
  int size;
  double mark_size_mm;
  assert(canvas);
  if (!_cdCheckCanvas(canvas)) return CD_ERROR;

  mark_size_mm = canvas->mark_size/canvas->xres;
  if (size_mm == CD_QUERY)
    return mark_size_mm;

  size = cdRound(size_mm*canvas->xres);
  if (size < 1) size = 1;

  canvas->mark_size = size;

  return mark_size_mm;
}

void wdCanvasGetFontDim(cdCanvas* canvas, double *max_width, double *height, double *ascent, double *descent)
{
  double origin_x, origin_y, tmp = 0;
  double distance_x, distance_y;
  int font_max_width, font_height, font_ascent, font_descent;
  assert(canvas);
  if (!_cdCheckCanvas(canvas)) return;

  cdCanvasGetFontDim(canvas, &font_max_width, &font_height, &font_ascent, &font_descent);
  _wCanvas2World(canvas, 0, 0, origin_x, origin_y);
  _wCanvas2World(canvas, font_max_width, font_height, distance_x, distance_y);
  if (max_width) *max_width = fabs(distance_x - origin_x);
  if (height) *height = fabs(distance_y - origin_y);
  _wCanvas2World(canvas, tmp, font_ascent, tmp, distance_y);
  if (ascent) *ascent = fabs(distance_y - origin_y);
  _wCanvas2World(canvas, tmp, font_descent, tmp, distance_y);
  if (descent) *descent = fabs(distance_y - origin_y);
}

void wdCanvasGetTextSize(cdCanvas* canvas, const char *s, double *width, double *height)
{
  int text_width, text_height;
  double origin_x, origin_y;
  double text_x, text_y;
  assert(canvas);
  if (!_cdCheckCanvas(canvas)) return;

  _wCanvas2World(canvas, 0, 0, origin_x, origin_y);
  cdCanvasGetTextSize(canvas, s, &text_width, &text_height);
  _wCanvas2World(canvas, text_width, text_height, text_x, text_y);
  if (width) *width = fabs(text_x - origin_x);
  if (height) *height = fabs(text_y - origin_y);
}

void wdCanvasGetTextBox(cdCanvas* canvas, double x, double y, const char *s, double *xmin, double *xmax, double *ymin, double *ymax)
{
  int rx, ry, rxmin, rxmax, rymin, rymax;
  assert(canvas);
  if (!_cdCheckCanvas(canvas)) return;

  _wWorld2Canvas(canvas, x, y, rx, ry);
  cdCanvasGetTextBox(canvas, rx, ry, s, &rxmin, &rxmax, &rymin, &rymax);

  _wCanvas2World(canvas, rxmin, rymin, *xmin, *ymin);
  _wCanvas2World(canvas, rxmax, rymax, *xmax, *ymax);
}

void wdCanvasGetTextBounds(cdCanvas* canvas, double x, double y, const char *s, double *rect)
{
  int rx, ry, rrect[8];
  assert(canvas);
  if (!_cdCheckCanvas(canvas)) return;

  _wWorld2Canvas(canvas, x, y, rx, ry);
  cdCanvasGetTextBounds(canvas, rx, ry, s, rrect);

  _wCanvas2World(canvas, rrect[0], rrect[1], rect[0], rect[1]);
  _wCanvas2World(canvas, rrect[2], rrect[3], rect[2], rect[3]);
  _wCanvas2World(canvas, rrect[4], rrect[5], rect[4], rect[5]);
  _wCanvas2World(canvas, rrect[6], rrect[7], rect[6], rect[7]);
}

void wdCanvasPattern(cdCanvas* canvas, int w, int h, const long *color, double w_mm, double h_mm)
{
  long *pattern = NULL;
  int w_pxl, h_pxl, x, y, cx, cy;
  int wratio, hratio;
  int *XTab, *YTab;
  assert(canvas);
  if (!_cdCheckCanvas(canvas)) return;

  cdCanvasMM2Pixel(canvas, w_mm, h_mm, &w_pxl, &h_pxl);

  /* to preserve the pattern characteristics must be an integer number */
  wratio = cdRound((double)w_pxl/(double)w);
  hratio = cdRound((double)h_pxl/(double)h);

  wratio = (wratio <= 0)? 1: wratio;
  hratio = (hratio <= 0)? 1: hratio;

  w_pxl = wratio * w;
  h_pxl = hratio * h;

  pattern = (long*)malloc(w_pxl*h_pxl*sizeof(long));

  XTab = cdGetZoomTable(w_pxl, w, 0);
  YTab = cdGetZoomTable(h_pxl, h, 0);

  for (y=0; y<h_pxl; y++)
  {
    cy = YTab[y];
    for (x=0; x<w_pxl; x++)
    {
      cx = XTab[x];
      pattern[x + y*w_pxl] = color[cx + cy*w];
    }
  }

  cdCanvasPattern(canvas, w_pxl, h_pxl, pattern);

  free(XTab);
  free(YTab);
  free(pattern);
}

void wdCanvasStipple(cdCanvas* canvas, int w, int h, const unsigned char *fgbg, double w_mm, double h_mm)
{
  unsigned char *stipple = NULL;
  int w_pxl, h_pxl, x, y, cx, cy;
  int wratio, hratio;
  int *XTab, *YTab;
  assert(canvas);
  if (!_cdCheckCanvas(canvas)) return;

  cdCanvasMM2Pixel(canvas, w_mm, h_mm, &w_pxl, &h_pxl);

  /* to preserve the pattern characteristics must be an integer number */
  wratio = cdRound((double)w_pxl/(double)w);
  hratio = cdRound((double)h_pxl/(double)h);

  wratio = (wratio <= 0)? 1: wratio;
  hratio = (hratio <= 0)? 1: hratio;

  w_pxl = wratio * w;
  h_pxl = hratio * h;

  stipple = (unsigned char*)malloc(w_pxl*h_pxl); 

  XTab = cdGetZoomTable(w_pxl, w, 0);
  YTab = cdGetZoomTable(h_pxl, h, 0);

  for (y=0; y<h_pxl; y++)
  {
    cy = YTab[y];
    for (x=0; x<w_pxl; x++)
    {
      cx = XTab[x];
      stipple[x + y*w_pxl] = fgbg[cx + cy*w];
    }
  }

  cdCanvasStipple(canvas, w_pxl, h_pxl, stipple);

  free(XTab);
  free(YTab);
  free(stipple);
}

