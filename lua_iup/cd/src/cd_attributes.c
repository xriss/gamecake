/** \file
 * \brief External API
 *
 * See Copyright Notice in cd.h
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <memory.h>
#include <math.h>


#include "cd.h"
#include "cd_private.h"


int cdCanvasClip(cdCanvas* canvas, int mode)
{
  int clip_mode;

  assert(canvas);
  assert(mode==CD_QUERY || (mode>=CD_CLIPOFF && mode<=CD_CLIPREGION));
  if (!_cdCheckCanvas(canvas)) return CD_ERROR;
  if (mode<CD_QUERY || mode>CD_CLIPREGION) return CD_ERROR;

  clip_mode = canvas->clip_mode;

  if (mode == CD_QUERY || 
      mode == clip_mode || 
      (mode == CD_CLIPPOLYGON && !canvas->clip_poly && !canvas->clip_fpoly))
    return clip_mode;

  if (canvas->cxClip)
    canvas->clip_mode = canvas->cxClip(canvas->ctxcanvas, mode);
  else
    canvas->clip_mode = mode;

  return clip_mode;
}

void cdCanvasClipArea(cdCanvas* canvas, int xmin, int xmax, int ymin, int ymax)
{
  assert(canvas);
  if (!_cdCheckCanvas(canvas)) return;

  if (!cdCheckBoxSize(&xmin, &xmax, &ymin, &ymax))
    return;

  if (canvas->use_origin)
  {
    xmin += canvas->origin.x;
    xmax += canvas->origin.x;
    ymin += canvas->origin.y;
    ymax += canvas->origin.y;
  }

  if (canvas->invert_yaxis)
  {
    ymin = _cdInvertYAxis(canvas, ymin);
    ymax = _cdInvertYAxis(canvas, ymax);
    _cdSwapInt(ymin, ymax);
  }

  if (xmin == canvas->clip_rect.xmin && 
      xmax == canvas->clip_rect.xmax && 
      ymin == canvas->clip_rect.ymin && 
      ymax == canvas->clip_rect.ymax)
    return;

  if (canvas->cxClipArea)
    canvas->cxClipArea(canvas->ctxcanvas, xmin, xmax, ymin, ymax);
  else if (canvas->cxFClipArea)
    canvas->cxFClipArea(canvas->ctxcanvas, (double)xmin, (double)xmax, (double)ymin, (double)ymax);

  canvas->clip_rect.xmin = xmin;
  canvas->clip_rect.xmax = xmax;
  canvas->clip_rect.ymin = ymin;
  canvas->clip_rect.ymax = ymax;
  canvas->clip_frect.xmin = (double)xmin;
  canvas->clip_frect.xmax = (double)xmax;
  canvas->clip_frect.ymin = (double)ymin;
  canvas->clip_frect.ymax = (double)ymax;
}

int cdCanvasGetClipArea(cdCanvas* canvas, int *xmin, int *xmax, int *ymin, int *ymax)
{
  int _xmin, _xmax, _ymin, _ymax;

  assert(canvas);
  if (!_cdCheckCanvas(canvas)) return CD_ERROR;

  _xmin = canvas->clip_rect.xmin;
  _xmax = canvas->clip_rect.xmax;
  _ymin = canvas->clip_rect.ymin;
  _ymax = canvas->clip_rect.ymax;

  if (canvas->invert_yaxis)
  {
    _ymin = _cdInvertYAxis(canvas, _ymin);
    _ymax = _cdInvertYAxis(canvas, _ymax);
    _cdSwapInt(_ymin, _ymax);
  }

  if (canvas->use_origin)
  {
    _xmin -= canvas->origin.x;
    _xmax -= canvas->origin.x;
    _ymin -= canvas->origin.y;
    _ymax -= canvas->origin.y;
  }

  if (xmin) *xmin = _xmin;
  if (xmax) *xmax = _xmax;
  if (ymin) *ymin = _ymin;
  if (ymax) *ymax = _ymax;

  return canvas->clip_mode;
}

void cdfCanvasClipArea(cdCanvas* canvas, double xmin, double xmax, double ymin, double ymax)
{
  assert(canvas);
  if (!_cdCheckCanvas(canvas)) return;

  if (!cdfCheckBoxSize(&xmin, &xmax, &ymin, &ymax))
    return;

  if (canvas->use_origin)
  {
    xmin += canvas->forigin.x;
    xmax += canvas->forigin.x;
    ymin += canvas->forigin.y;
    ymax += canvas->forigin.y;
  }

  if (canvas->invert_yaxis)
  {
    ymin = _cdInvertYAxis(canvas, ymin);
    ymax = _cdInvertYAxis(canvas, ymax);
    _cdSwapDouble(ymin, ymax);
  }

  if (xmin == canvas->clip_frect.xmin && 
      xmax == canvas->clip_frect.xmax && 
      ymin == canvas->clip_frect.ymin && 
      ymax == canvas->clip_frect.ymax)
    return;

  if (canvas->cxFClipArea)
    canvas->cxFClipArea(canvas->ctxcanvas, xmin, xmax, ymin, ymax);
  else if (canvas->cxClipArea)
    canvas->cxClipArea(canvas->ctxcanvas, _cdRound(xmin), _cdRound(xmax), _cdRound(ymin), _cdRound(ymax));

  canvas->clip_frect.xmin = xmin;
  canvas->clip_frect.xmax = xmax;
  canvas->clip_frect.ymin = ymin;
  canvas->clip_frect.ymax = ymax;
  canvas->clip_rect.xmin = _cdRound(xmin);
  canvas->clip_rect.xmax = _cdRound(xmax);
  canvas->clip_rect.ymin = _cdRound(ymin);
  canvas->clip_rect.ymax = _cdRound(ymax);
}

int cdfCanvasGetClipArea(cdCanvas* canvas, double *xmin, double *xmax, double *ymin, double *ymax)
{
  double _xmin, _xmax, _ymin, _ymax;

  assert(canvas);
  if (!_cdCheckCanvas(canvas)) return CD_ERROR;

  _xmin = canvas->clip_frect.xmin;
  _xmax = canvas->clip_frect.xmax;
  _ymin = canvas->clip_frect.ymin;
  _ymax = canvas->clip_frect.ymax;

  if (canvas->invert_yaxis)
  {
    _ymin = _cdInvertYAxis(canvas, _ymin);
    _ymax = _cdInvertYAxis(canvas, _ymax);
    _cdSwapDouble(_ymin, _ymax);
  }

  if (canvas->use_origin)
  {
    _xmin -= canvas->forigin.x;
    _xmax -= canvas->forigin.x;
    _ymin -= canvas->forigin.y;
    _ymax -= canvas->forigin.y;
  }

  if (xmin) *xmin = _xmin;
  if (xmax) *xmax = _xmax;
  if (ymin) *ymin = _ymin;
  if (ymax) *ymax = _ymax;

  return canvas->clip_mode;
}

int cdCanvasIsPointInRegion(cdCanvas* canvas, int x, int y)
{
  assert(canvas);
  if (!_cdCheckCanvas(canvas) || !canvas->cxIsPointInRegion) return CD_ERROR;

  if (canvas->use_origin)
  {
    x += canvas->origin.x;
    y += canvas->origin.y;
  }

  if (canvas->invert_yaxis)
    y = _cdInvertYAxis(canvas, y);

  return canvas->cxIsPointInRegion(canvas->ctxcanvas, x, y);
}

void cdCanvasOffsetRegion(cdCanvas* canvas, int x, int y)
{
  assert(canvas);
  if (!_cdCheckCanvas(canvas) || !canvas->cxOffsetRegion) return;

  if (canvas->invert_yaxis)
    y = -y;

  canvas->cxOffsetRegion(canvas->ctxcanvas, x, y);
}

void cdCanvasGetRegionBox(cdCanvas* canvas, int *xmin, int *xmax, int *ymin, int *ymax)
{
  int _xmin, _xmax, _ymin, _ymax;
  assert(canvas);
  if (!_cdCheckCanvas(canvas) || !canvas->cxGetRegionBox) return;

  canvas->cxGetRegionBox(canvas->ctxcanvas, &_xmin, &_xmax, &_ymin, &_ymax);

  if (canvas->invert_yaxis)
  {
    _ymin = _cdInvertYAxis(canvas, _ymin);
    _ymax = _cdInvertYAxis(canvas, _ymax);
    _cdSwapInt(_ymin, _ymax);
  }

  if (canvas->use_origin)
  {
    _xmin -= canvas->origin.x;
    _xmax -= canvas->origin.x;
    _ymin -= canvas->origin.y;
    _ymax -= canvas->origin.y;
  }

  if (xmin) *xmin = _xmin;
  if (xmax) *xmax = _xmax;
  if (ymin) *ymin = _ymin;
  if (ymax) *ymax = _ymax;
}

void cdCanvasOrigin(cdCanvas* canvas, int x, int y)
{
  assert(canvas);
  if (!_cdCheckCanvas(canvas)) return;

  canvas->origin.x = x;
  canvas->origin.y = y;

  if (canvas->origin.x == 0 && canvas->origin.y == 0)
    canvas->use_origin = 0;
  else
    canvas->use_origin = 1;

  canvas->forigin.x = (double)canvas->origin.x;
  canvas->forigin.y = (double)canvas->origin.y;
}


void cdfCanvasOrigin(cdCanvas* canvas, double x, double y)
{
  assert(canvas);
  if (!_cdCheckCanvas(canvas)) return;

  canvas->forigin.x = x;
  canvas->forigin.y = y;

  if (canvas->forigin.x == 0 && canvas->forigin.y == 0)
    canvas->use_origin = 0;
  else
    canvas->use_origin = 1;

  canvas->origin.x = _cdRound(canvas->forigin.x);
  canvas->origin.y = _cdRound(canvas->forigin.y);
}

void cdCanvasGetOrigin(cdCanvas* canvas, int *x, int *y)
{
  assert(canvas);
  if (!_cdCheckCanvas(canvas)) return;

  if (x) *x = canvas->origin.x;
  if (y) *y = canvas->origin.y;
}

void cdfCanvasGetOrigin(cdCanvas* canvas, double *x, double *y)
{
  assert(canvas);
  if (!_cdCheckCanvas(canvas)) return;

  if (x) *x = canvas->forigin.x;
  if (y) *y = canvas->forigin.y;
}

#define CD_TRANSFORM_X(_x, _y, _matrix) (_x*_matrix[0] + _y*_matrix[2] + _matrix[4])
#define CD_TRANSFORM_Y(_x, _y, _matrix) (_x*_matrix[1] + _y*_matrix[3] + _matrix[5])

void cdfMatrixTransformPoint(double* matrix, double x, double y, double *rx, double *ry)
{
  *rx = CD_TRANSFORM_X(x, y, matrix);
  *ry = CD_TRANSFORM_Y(x, y, matrix);
}

void cdMatrixTransformPoint(double* matrix, int x, int y, int *rx, int *ry)
{
  double t;
  t = CD_TRANSFORM_X(x, y, matrix); *rx = _cdRound(t); 
  t = CD_TRANSFORM_Y(x, y, matrix); *ry = _cdRound(t); 
}

void cdCanvasTransformPoint(cdCanvas* canvas, int x, int y, int *tx, int *ty)
{
  double *matrix;
  double t;

  assert(canvas);
  if (!_cdCheckCanvas(canvas)) return;

  matrix = canvas->matrix;
  t = CD_TRANSFORM_X(x, y, matrix); *tx = _cdRound(t); 
  t = CD_TRANSFORM_Y(x, y, matrix); *ty = _cdRound(t); 
}

void cdfCanvasTransformPoint(cdCanvas* canvas, double x, double y, double *tx, double *ty)
{
  double *matrix;

  assert(canvas);
  if (!_cdCheckCanvas(canvas)) return;

  matrix = canvas->matrix;
  *tx = CD_TRANSFORM_X(x, y, matrix);
  *ty = CD_TRANSFORM_Y(x, y, matrix); 
}

void cdMatrixInverse(const double* matrix, double* inv_matrix)
{
  double det = matrix[0] * matrix[3] - matrix[1] * matrix[2];

  if (fabs(det) < 0.00001)
  {
    inv_matrix[0] = 1;
    inv_matrix[1] = 0;
    inv_matrix[2] = 0;
    inv_matrix[3] = 1;
    inv_matrix[4] = 0;
    inv_matrix[5] = 0;
    return;
  }

  inv_matrix[0] = matrix[3]/det;
  inv_matrix[1] = -matrix[1]/det;
  inv_matrix[2] = -matrix[2]/det;
  inv_matrix[3] = matrix[0]/det;
  inv_matrix[4] = -(matrix[4] * inv_matrix[0] + matrix[5] * inv_matrix[2]);
  inv_matrix[5] = -(matrix[4] * inv_matrix[1] + matrix[5] * inv_matrix[3]);
}

#define _cdIsIndentity(_matrix) (_matrix[0] == 1 && _matrix[1] == 0 && \
                                 _matrix[2] == 0 && _matrix[3] == 1 && \
                                 _matrix[4] == 0 && _matrix[5] == 0)

double* cdCanvasGetTransform(cdCanvas* canvas)
{
  static double matrix[6];

  assert(canvas);
  if (!_cdCheckCanvas(canvas)) return NULL;

  memcpy(matrix, canvas->matrix, sizeof(double)*6);
  return matrix;
}

void cdCanvasTransform(cdCanvas* canvas, const double* matrix)
{
  assert(canvas);
  if (!_cdCheckCanvas(canvas)) return;

  if (!matrix || _cdIsIndentity(matrix))
  {
    canvas->use_matrix = 0;
    memset(canvas->matrix, 0, sizeof(double)*6);
    canvas->matrix[0] = 1;  /* reset to identity */
    canvas->matrix[3] = 1;

    if (canvas->cxTransform)
      canvas->cxTransform(canvas->ctxcanvas, NULL);

    return;
  }

  if (canvas->cxTransform)
    canvas->cxTransform(canvas->ctxcanvas, matrix);

  memcpy(canvas->matrix, matrix, sizeof(double)*6);
  canvas->use_matrix = 1;
}

/* mul_matrix = matrix * mul_matrix (left multiply) */
void cdMatrixMultiply(const double* matrix, double* mul_matrix)
{
  double tmp_matrix[6];
  tmp_matrix[0] = matrix[0] * mul_matrix[0] + matrix[1] * mul_matrix[2];
  tmp_matrix[1] = matrix[0] * mul_matrix[1] + matrix[1] * mul_matrix[3];
  tmp_matrix[2] = matrix[2] * mul_matrix[0] + matrix[3] * mul_matrix[2];
  tmp_matrix[3] = matrix[2] * mul_matrix[1] + matrix[3] * mul_matrix[3];
  tmp_matrix[4] = matrix[4] * mul_matrix[0] + matrix[5] * mul_matrix[2] + mul_matrix[4];
  tmp_matrix[5] = matrix[4] * mul_matrix[1] + matrix[5] * mul_matrix[3] + mul_matrix[5];

  memcpy(mul_matrix, tmp_matrix, sizeof(double)*6);
}

void cdCanvasTransformMultiply(cdCanvas* canvas, const double* matrix)
{
  assert(canvas);
  if (!_cdCheckCanvas(canvas)) return;

  cdMatrixMultiply(matrix, canvas->matrix);

  if (_cdIsIndentity(canvas->matrix))
    canvas->use_matrix = 0;
  else
    canvas->use_matrix = 1;

  if (canvas->cxTransform)
    canvas->cxTransform(canvas->ctxcanvas, canvas->use_matrix? canvas->matrix: NULL);
}

void cdCanvasTransformRotate(cdCanvas* canvas, double angle)
{
  double tmp_matrix[4];
  double* matrix, cos_ang, sin_ang;
  assert(canvas);
  if (!_cdCheckCanvas(canvas)) return;
  matrix = canvas->matrix;

  cos_ang = cos(angle * CD_DEG2RAD);
  sin_ang = sin(angle * CD_DEG2RAD);

  tmp_matrix[0] =  cos_ang * matrix[0] + sin_ang * matrix[2];
  tmp_matrix[1] =  cos_ang * matrix[1] + sin_ang * matrix[3];
  tmp_matrix[2] = -sin_ang * matrix[0] + cos_ang * matrix[2];
  tmp_matrix[3] = -sin_ang * matrix[1] + cos_ang * matrix[3];

  matrix[0] = tmp_matrix[0];
  matrix[1] = tmp_matrix[1];
  matrix[2] = tmp_matrix[2];
  matrix[3] = tmp_matrix[3];

  if (_cdIsIndentity(matrix))
    canvas->use_matrix = 0;
  else
    canvas->use_matrix = 1;

  if (canvas->cxTransform)
    canvas->cxTransform(canvas->ctxcanvas, canvas->use_matrix? matrix: NULL);
}

void cdCanvasTransformScale(cdCanvas* canvas, double sx, double sy)
{
  double* matrix;
  assert(canvas);
  if (!_cdCheckCanvas(canvas)) return;
  matrix = canvas->matrix;

  matrix[0] = sx * matrix[0];
  matrix[1] = sx * matrix[1];
  matrix[2] = sy * matrix[2];
  matrix[3] = sy * matrix[3];

  if (_cdIsIndentity(matrix))
    canvas->use_matrix = 0;
  else
    canvas->use_matrix = 1;

  if (canvas->cxTransform)
    canvas->cxTransform(canvas->ctxcanvas, canvas->use_matrix? matrix: NULL);
}

void cdCanvasTransformTranslate(cdCanvas* canvas, double dx, double dy)
{
  double* matrix;
  assert(canvas);
  if (!_cdCheckCanvas(canvas)) return;
  matrix = canvas->matrix;

  matrix[4] = dx * matrix[0] + dy * matrix[2] + matrix[4];
  matrix[5] = dx * matrix[1] + dy * matrix[3] + matrix[5];

  if (_cdIsIndentity(matrix))
    canvas->use_matrix = 0;
  else
    canvas->use_matrix = 1;

  if (canvas->cxTransform)
    canvas->cxTransform(canvas->ctxcanvas, canvas->use_matrix? matrix: NULL);
}

int cdCanvasBackOpacity(cdCanvas* canvas, int opacity)
{
  int back_opacity;

  assert(canvas);
  assert(opacity==CD_QUERY || (opacity>=CD_OPAQUE && opacity<=CD_TRANSPARENT));
  if (!_cdCheckCanvas(canvas)) return CD_ERROR;
  if (opacity<CD_QUERY || opacity>CD_TRANSPARENT) return CD_ERROR;

  back_opacity = canvas->back_opacity;

  if (opacity == CD_QUERY || opacity == back_opacity)
    return back_opacity;

  if (canvas->cxBackOpacity)
    canvas->back_opacity = canvas->cxBackOpacity(canvas->ctxcanvas, opacity);
  else
    canvas->back_opacity = opacity;

  return back_opacity;
}

int cdCanvasWriteMode(cdCanvas* canvas, int mode)
{
  int write_mode;

  assert(canvas);
  assert(mode==CD_QUERY || (mode>=CD_REPLACE && mode<=CD_NOT_XOR));
  if (!_cdCheckCanvas(canvas)) return CD_ERROR;
  if (mode<CD_QUERY || mode>CD_NOT_XOR) return CD_ERROR;

  write_mode = canvas->write_mode;

  if (mode == CD_QUERY || mode == write_mode)
    return write_mode;

  if (canvas->cxWriteMode)
    canvas->write_mode = canvas->cxWriteMode(canvas->ctxcanvas, mode);
  else
    canvas->write_mode = mode;

  return write_mode;
}

int cdCanvasLineStyle(cdCanvas* canvas, int style)
{
  int line_style;

  assert(canvas);
  assert(style==CD_QUERY || (style>=CD_CONTINUOUS && style<=CD_CUSTOM));
  if (!_cdCheckCanvas(canvas)) return CD_ERROR;
  if (style<CD_QUERY || style>CD_CUSTOM) return CD_ERROR;

  line_style = canvas->line_style;

  if (style == CD_QUERY || style == line_style)
    return line_style;

  if (style == CD_CUSTOM && !canvas->line_dashes_count)
    return line_style;

  if (canvas->cxLineStyle)
    canvas->line_style = canvas->cxLineStyle(canvas->ctxcanvas, style);
  else
    canvas->line_style = style;

  return line_style;
}

int cdCanvasLineJoin(cdCanvas* canvas, int join)
{
  int line_join;

  assert(canvas);
  assert(join==CD_QUERY || (join>=CD_MITER && join<=CD_ROUND));
  if (!_cdCheckCanvas(canvas)) return CD_ERROR;
  if (join<CD_QUERY || join>CD_ROUND) return CD_ERROR;

  line_join = canvas->line_join;

  if (join == CD_QUERY || join == line_join)
    return line_join;

  if (canvas->cxLineJoin)
    canvas->line_join = canvas->cxLineJoin(canvas->ctxcanvas, join);
  else
    canvas->line_join = join;

  return line_join;
}

int cdCanvasLineCap(cdCanvas* canvas, int cap)
{
  int line_cap;

  assert(canvas);
  assert(cap==CD_QUERY || (cap>=CD_CAPFLAT && cap<=CD_CAPROUND));
  if (!_cdCheckCanvas(canvas)) return CD_ERROR;
  if (cap<CD_QUERY || cap>CD_CAPROUND) return CD_ERROR;

  line_cap = canvas->line_cap;

  if (cap == CD_QUERY || cap == line_cap)
    return line_cap;

  if (canvas->cxLineCap)
    canvas->line_cap = canvas->cxLineCap(canvas->ctxcanvas, cap);
  else
    canvas->line_cap = cap;

  return line_cap;
}

int cdCanvasRegionCombineMode(cdCanvas* canvas, int mode)
{
  int combine_mode;

  assert(canvas);
  assert(mode==CD_QUERY || (mode>=CD_UNION && mode<=CD_NOTINTERSECT));
  if (!_cdCheckCanvas(canvas)) return CD_ERROR;
  if (mode<CD_QUERY || mode>CD_NOTINTERSECT) return CD_ERROR;

  combine_mode = canvas->combine_mode;

  if (mode == CD_QUERY || mode == combine_mode)
    return combine_mode;

  canvas->combine_mode = mode;

  return combine_mode;
}

void cdCanvasLineStyleDashes(cdCanvas* canvas, const int* dashes, int count)
{
  assert(canvas);
  if (!_cdCheckCanvas(canvas)) return;

  if (canvas->line_dashes)
  {
    free(canvas->line_dashes);
    canvas->line_dashes = NULL;
  }

  if (dashes)
  {
    canvas->line_dashes = malloc(count*sizeof(int));
    canvas->line_dashes_count = count;
    memcpy(canvas->line_dashes, dashes, count*sizeof(int));
  }
}

int cdCanvasLineWidth(cdCanvas* canvas, int width)
{
  int line_width;

  assert(canvas);
  assert(width==CD_QUERY || width>0);
  if (!_cdCheckCanvas(canvas)) return CD_ERROR;
  if (width!=CD_QUERY && width<=0) return CD_ERROR;

  line_width = canvas->line_width;

  if (width == CD_QUERY || width == line_width)
    return line_width;

  if (canvas->cxLineWidth)
    canvas->line_width = canvas->cxLineWidth(canvas->ctxcanvas, width);
  else
    canvas->line_width = width;

  return line_width;
}

int  cdCanvasFillMode(cdCanvas* canvas, int mode)
{
  int fill_mode;

  assert(canvas);
  assert(mode==CD_QUERY || (mode>=CD_EVENODD && mode<=CD_WINDING));
  if (!_cdCheckCanvas(canvas)) return CD_ERROR;
  if (mode<CD_QUERY || mode>CD_WINDING) return CD_ERROR;
                                        
  fill_mode = canvas->fill_mode;

  if (mode == CD_QUERY || mode == fill_mode)
    return fill_mode;

  canvas->fill_mode = mode;

  return fill_mode;
}

int cdCanvasInteriorStyle (cdCanvas* canvas, int style)
{
  int interior_style;

  assert(canvas);
  assert(style==CD_QUERY || (style>=CD_SOLID && style<=CD_HOLLOW));
  if (!_cdCheckCanvas(canvas)) return CD_ERROR;
  if (style<CD_QUERY || style>CD_HOLLOW) return CD_ERROR;

  interior_style = canvas->interior_style;

  if ( style == CD_QUERY || style == interior_style)
    return interior_style;
    
  if ((style == CD_PATTERN && !canvas->pattern_size) || 
	    (style == CD_STIPPLE && !canvas->stipple_size))
  return interior_style;

  if (style == CD_HOLLOW)
  {
    canvas->interior_style = CD_HOLLOW;
    return interior_style;
  }

  if (canvas->cxInteriorStyle)
    canvas->interior_style = canvas->cxInteriorStyle(canvas->ctxcanvas, style);
  else
    canvas->interior_style = style;

  return interior_style;
}

int cdCanvasHatch(cdCanvas* canvas, int style)
{
  int hatch_style;

  assert(canvas);
  assert(style==CD_QUERY || (style>=CD_HORIZONTAL && style<=CD_DIAGCROSS));
  if (!_cdCheckCanvas(canvas)) return CD_ERROR;
  if (style<CD_QUERY || style>CD_DIAGCROSS) return CD_ERROR;

  hatch_style = canvas->hatch_style;

  if (style == CD_QUERY)
    return hatch_style;

  if (canvas->cxHatch)
    canvas->hatch_style = canvas->cxHatch(canvas->ctxcanvas, style);
  else
    canvas->hatch_style = style;

  canvas->interior_style = CD_HATCH;

  return hatch_style;
}

void cdCanvasStipple(cdCanvas* canvas, int w, int h, const unsigned char *stipple)
{
  assert(canvas);
  assert(stipple);
  assert(w>0);
  assert(h>0);
  if (!_cdCheckCanvas(canvas)) return;
  if (w <= 0 || h <= 0 || !stipple)
    return;

  if (canvas->cxStipple)
    canvas->cxStipple(canvas->ctxcanvas, w, h, stipple);

  if (w*h > canvas->stipple_size)       /* realoca array dos pontos */
  {
    int newsize = w*h;
    canvas->stipple = (unsigned char*)realloc(canvas->stipple, newsize);
    canvas->stipple_size = newsize;

    if (!canvas->stipple) 
    {
      canvas->stipple_size = 0;
      return;
    }
  }

  memcpy(canvas->stipple, stipple, w*h);

  canvas->interior_style = CD_STIPPLE;
  canvas->stipple_w = w;
  canvas->stipple_h = h;
}

unsigned char* cdCanvasGetStipple(cdCanvas* canvas, int* w, int* h)
{
  assert(canvas);
  if (!_cdCheckCanvas(canvas)) return NULL;

  if (!canvas->stipple_size)
    return NULL;

  if (w) *w = canvas->stipple_w;
  if (h) *h = canvas->stipple_h;

  return canvas->stipple;
}

void cdCanvasPattern(cdCanvas* canvas, int w, int h, const long *pattern)
{
  assert(canvas);
  assert(pattern);
  assert(w>0);
  assert(h>0);
  if (!_cdCheckCanvas(canvas)) return;
  if (w <= 0 || h <= 0 || !pattern)
    return;

  if (canvas->cxPattern)
    canvas->cxPattern(canvas->ctxcanvas, w, h, pattern);

  if (w*h > canvas->pattern_size)       /* realoca array dos pontos */
  {
    int newsize = w*h;

    if (canvas->pattern) free(canvas->pattern);
    canvas->pattern = (long*)malloc(newsize*sizeof(long));
    canvas->pattern_size = newsize;

    if (!canvas->pattern) 
    {
      canvas->pattern_size = 0;
      return;
    }
  }

  memcpy(canvas->pattern, pattern, w*h*sizeof(long));

  canvas->interior_style = CD_PATTERN;
  canvas->pattern_w = w;
  canvas->pattern_h = h;
}

long * cdCanvasGetPattern(cdCanvas* canvas, int* w, int* h)
{
  assert(canvas);
  if (!_cdCheckCanvas(canvas)) return NULL;

  if (!canvas->pattern_size)
    return NULL;

  if (w) *w = canvas->pattern_w;
  if (h) *h = canvas->pattern_h;

  return canvas->pattern;
}

int cdCanvasMarkType(cdCanvas* canvas, int type)
{
  int mark_type;

  assert(canvas);
  assert(type==CD_QUERY || (type>=CD_PLUS && type<=CD_HOLLOW_DIAMOND));
  if (!_cdCheckCanvas(canvas)) return CD_ERROR;
  if (type<CD_QUERY || type>CD_HOLLOW_DIAMOND) return CD_ERROR;

  mark_type = canvas->mark_type;

  if (type == CD_QUERY || type == mark_type)
    return mark_type;

  canvas->mark_type = type;

  return mark_type;
}

int cdCanvasMarkSize(cdCanvas* canvas, int size)
{
  int mark_size;

  assert(canvas);
  assert(size == CD_QUERY || size>0);
  if (!_cdCheckCanvas(canvas)) return CD_ERROR;
  if (size != CD_QUERY && size<=0) return CD_ERROR;

  mark_size = canvas->mark_size;

  if (size == CD_QUERY || size == mark_size)
    return mark_size;

  canvas->mark_size = size;

  return mark_size;
}

long cdCanvasBackground(cdCanvas* canvas, long color)
{
  long background;

  assert(canvas);
  if (!_cdCheckCanvas(canvas)) return CD_ERROR;

  background = canvas->background;

  if (color == CD_QUERY || color == background)
    return background;

  if (canvas->cxBackground)
    canvas->background = canvas->cxBackground(canvas->ctxcanvas, color);
  else
    canvas->background = color;

  return background;
}

long cdCanvasForeground(cdCanvas* canvas, long color)
{
  long foreground;

  assert(canvas);
  if (!_cdCheckCanvas(canvas)) return CD_ERROR;

  foreground = canvas->foreground;

  if (color == CD_QUERY || color == foreground)
    return foreground;

  if (canvas->cxForeground)
    canvas->foreground = canvas->cxForeground(canvas->ctxcanvas, color);
  else
    canvas->foreground = color;

  return foreground;
}

void cdCanvasSetBackground(cdCanvas* canvas, long color)
{
  assert(canvas);
  if (!_cdCheckCanvas(canvas)) return;

  if (color == canvas->background)
    return;

  if (canvas->cxBackground)
    canvas->background = canvas->cxBackground(canvas->ctxcanvas, color);
  else
    canvas->background = color;
}

void cdCanvasSetForeground(cdCanvas* canvas, long color)
{
  assert(canvas);
  if (!_cdCheckCanvas(canvas)) return;

  if (color == canvas->foreground)
    return;

  if (canvas->cxForeground)
    canvas->foreground = canvas->cxForeground(canvas->ctxcanvas, color);
  else
    canvas->foreground = color;
}

/****************/
/* color coding */
/****************/

long cdEncodeColor(unsigned char r, unsigned char g, unsigned char b)
{
  return (((unsigned long)r) << 16) |
         (((unsigned long)g) <<  8) |
         (((unsigned long)b) <<  0);
}

void cdDecodeColor(long color, unsigned char *r, unsigned char *g, unsigned char *b)
{
  *r = cdRed(color);
  *g = cdGreen(color);
  *b = cdBlue(color);
}

unsigned char cdDecodeAlpha(long color)
{
  unsigned char alpha = cdReserved(color);
  return ~alpha;
}

long cdEncodeAlpha(long color, unsigned char alpha)
{
  alpha = ~alpha;
  return (((unsigned long)alpha) << 24) | (color & 0xFFFFFF);
}

int cdCanvasGetColorPlanes(cdCanvas* canvas)
{
  assert(canvas);
  if (!_cdCheckCanvas(canvas)) return CD_ERROR;
  return canvas->bpp;
}

void cdCanvasPalette(cdCanvas* canvas, int n, const long *palette, int mode)
{
  assert(canvas);
  assert(n>0);
  assert(palette);
  assert(mode>=CD_POLITE && mode<=CD_FORCE);
  if (!_cdCheckCanvas(canvas)) return;
  if (!palette) return;
  if (n <= 0 || canvas->bpp > 8) 
    return;

  if (canvas->cxPalette)
    canvas->cxPalette(canvas->ctxcanvas, n, palette, mode);
}

