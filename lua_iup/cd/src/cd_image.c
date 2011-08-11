/** \file
 * \brief External API - Images
 *
 * See Copyright Notice in cd.h
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <memory.h>
#include <math.h>


#include "cd.h"
#include "cd_private.h"

                        
void cdCanvasGetImageRGB(cdCanvas* canvas, unsigned char *r, unsigned char *g, unsigned char *b, int x, int y, int w, int h)
{
  assert(canvas);
  assert(r);
  assert(g);
  assert(b);
  assert(w>0);
  assert(h>0);
  if (!_cdCheckCanvas(canvas)) return;

  if (canvas->use_origin)
  {
    x += canvas->origin.x;
    y += canvas->origin.y;
  }

  if (canvas->invert_yaxis)
    y = _cdInvertYAxis(canvas, y);

  if (canvas->cxGetImageRGB)
    canvas->cxGetImageRGB(canvas->ctxcanvas, r, g, b, x, y, w, h);
}

void cdRGB2Gray(int width, int height, const unsigned char* red, const unsigned char* green, const unsigned char* blue, unsigned char* index, long *color)
{
  int c, i, count;
  for (c = 0; c < 256; c++)
    color[c] = cdEncodeColor((unsigned char)c, (unsigned char)c, (unsigned char)c);

  count = width*height;
  for (i=0; i<count; i++)
  {
    *index = (unsigned char)(((*red)*30 + (*green)*59 + (*blue)*11)/100);

    index++;
    red++;
    green++;
    blue++;
  }
}

void cdCanvasPutImageRectRGB(cdCanvas* canvas, int iw, int ih, const unsigned char *r, const unsigned char *g, const unsigned char *b, int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax)
{
  assert(canvas);
  assert(iw>0);
  assert(ih>0);
  assert(r);
  assert(g);
  assert(b);
  if (!_cdCheckCanvas(canvas)) return;

  if (w == 0) w = iw;
  if (h == 0) h = ih;
  if (xmax == 0) xmax = iw - 1;
  if (ymax == 0) ymax = ih - 1;

  if (!cdCheckBoxSize(&xmin, &xmax, &ymin, &ymax))
    return;

  cdNormalizeLimits(iw, ih, &xmin, &xmax, &ymin, &ymax);

  if (canvas->use_origin)
  {
    x += canvas->origin.x;
    y += canvas->origin.y;
  }

  if (canvas->invert_yaxis)
    y = _cdInvertYAxis(canvas, y);

  if (canvas->cxPutImageRectMap && (canvas->bpp <= 8 || !canvas->cxPutImageRectRGB))
    cdSimPutImageRectRGB(canvas, iw, ih, r, g, b, x, y, w, h, xmin, xmax, ymin, ymax);
  else if (canvas->cxPutImageRectRGB)
    canvas->cxPutImageRectRGB(canvas->ctxcanvas, iw, ih, r, g, b, x, y, w, h, xmin, xmax, ymin, ymax);
}

void cdCanvasPutImageRectRGBA(cdCanvas* canvas, int iw, int ih, const unsigned char *r, const unsigned char *g, const unsigned char *b, const unsigned char *a, int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax)
{
  assert(canvas);
  assert(iw>0);
  assert(ih>0);
  assert(r);
  assert(g);
  assert(b);
  if (!_cdCheckCanvas(canvas)) return;

  if (w == 0) w = iw;
  if (h == 0) h = ih;
  if (xmax == 0) xmax = iw - 1;
  if (ymax == 0) ymax = ih - 1;

  if (!cdCheckBoxSize(&xmin, &xmax, &ymin, &ymax))
    return;

  cdNormalizeLimits(iw, ih, &xmin, &xmax, &ymin, &ymax);

  if (canvas->use_origin)
  {
    x += canvas->origin.x;
    y += canvas->origin.y;
  }

  if (canvas->invert_yaxis)
    y = _cdInvertYAxis(canvas, y);

  if (!canvas->cxPutImageRectRGBA)
  {
    if (canvas->cxGetImageRGB)
      cdSimPutImageRectRGBA(canvas, iw, ih, r, g, b, a, x, y, w, h, xmin, xmax, ymin, ymax);
    else if (!canvas->cxPutImageRectRGB)
    {
      if (canvas->cxPutImageRectMap)
        cdSimPutImageRectRGB(canvas, iw, ih, r, g, b, x, y, w, h, xmin, xmax, ymin, ymax);
    }
    else
      canvas->cxPutImageRectRGB(canvas->ctxcanvas, iw, ih, r, g, b, x, y, w, h, xmin, xmax, ymin, ymax);
  }
  else
    canvas->cxPutImageRectRGBA(canvas->ctxcanvas, iw, ih, r, g, b, a, x, y, w, h, xmin, xmax, ymin, ymax);
}

static long* cd_getgraycolormap(void)
{
  static long color_map[256] = {1};

  if (color_map[0])
  {
    int c;
    for (c = 0; c < 256; c++)
      color_map[c] = cdEncodeColor((unsigned char)c, (unsigned char)c, (unsigned char)c);
  }

  return color_map;
}

void cdCanvasPutImageRectMap(cdCanvas* canvas, int iw, int ih, const unsigned char *index, const long *colors, int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax)
{
  assert(canvas);
  assert(index);
  assert(iw>0);
  assert(ih>0);
  if (!_cdCheckCanvas(canvas)) return;

  if (w == 0) w = iw;
  if (h == 0) h = ih;
  if (xmax == 0) xmax = iw - 1;
  if (ymax == 0) ymax = ih - 1;

  if (!cdCheckBoxSize(&xmin, &xmax, &ymin, &ymax))
    return;

  cdNormalizeLimits(iw, ih, &xmin, &xmax, &ymin, &ymax);

  if (canvas->use_origin)
  {
    x += canvas->origin.x;
    y += canvas->origin.y;
  }

  if (canvas->invert_yaxis)
    y = _cdInvertYAxis(canvas, y);

  if (colors == NULL)
    colors = cd_getgraycolormap();

  canvas->cxPutImageRectMap(canvas->ctxcanvas, iw, ih, index, colors, x, y, w, h, xmin, xmax, ymin, ymax);
}

cdImage* cdCanvasCreateImage(cdCanvas* canvas, int w, int h)
{
  cdImage *image;
  cdCtxImage *ctximage;

  assert(canvas);
  assert(w>0);
  assert(h>0);
  if (!_cdCheckCanvas(canvas)) return NULL;
  if (w <= 0) return NULL;
  if (h <= 0) return NULL;
  if (!canvas->cxCreateImage) return NULL;

  ctximage = canvas->cxCreateImage(canvas->ctxcanvas, w, h);
  if (!ctximage)
    return NULL;

  image = (cdImage*)malloc(sizeof(cdImage));

  image->cxGetImage = canvas->cxGetImage;
  image->cxPutImageRect = canvas->cxPutImageRect;
  image->cxKillImage = canvas->cxKillImage;
  image->w = w;
  image->h = h;
  image->ctximage = ctximage;

  return image;
}

void cdCanvasGetImage(cdCanvas* canvas, cdImage* image, int x, int y)
{
  assert(canvas);
  assert(image);
  if (!_cdCheckCanvas(canvas)) return;
  if (!image) return;
  if (image->cxGetImage != canvas->cxGetImage) return;

  if (canvas->use_origin)
  {
    x += canvas->origin.x;
    y += canvas->origin.y;
  }

  if (canvas->invert_yaxis)
    y = _cdInvertYAxis(canvas, y);

  canvas->cxGetImage(canvas->ctxcanvas, image->ctximage, x, y);
}

void cdCanvasPutImageRect(cdCanvas* canvas, cdImage* image, int x, int y, int xmin, int xmax, int ymin, int ymax)
{
  assert(canvas);
  assert(image);
  if (!_cdCheckCanvas(canvas)) return;
  if (!image) return;
  if (image->cxPutImageRect != canvas->cxPutImageRect) return;

  if (xmax == 0) xmax = image->w - 1;
  if (ymax == 0) ymax = image->h - 1;

  if (!cdCheckBoxSize(&xmin, &xmax, &ymin, &ymax))
    return;

  cdNormalizeLimits(image->w, image->h, &xmin, &xmax, &ymin, &ymax);

  if (canvas->use_origin)
  {
    x += canvas->origin.x;
    y += canvas->origin.y;
  }

  if (canvas->invert_yaxis)
    y = _cdInvertYAxis(canvas, y);

  canvas->cxPutImageRect(canvas->ctxcanvas, image->ctximage, x, y, xmin, xmax, ymin, ymax);
}

void cdKillImage(cdImage* image)
{
  assert(image);
  if (!image) return;

  image->cxKillImage(image->ctximage);
  memset(image, 0, sizeof(cdImage));
  free(image);
}

void cdCanvasScrollArea(cdCanvas* canvas, int xmin, int xmax, int ymin, int ymax, int dx, int dy)
{
  assert(canvas);
  if (!_cdCheckCanvas(canvas)) return;
  if (!canvas->cxScrollArea) return;

  if (!cdCheckBoxSize(&xmin, &xmax, &ymin, &ymax))
    return;

  if (dx == 0 && dy == 0)
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
    dy = -dy;
    ymin = _cdInvertYAxis(canvas, ymin);
    ymax = _cdInvertYAxis(canvas, ymax);
    _cdSwapInt(ymin, ymax);
  }

  canvas->cxScrollArea(canvas->ctxcanvas, xmin, xmax, ymin, ymax, dx, dy);
}

unsigned char cdZeroOrderInterpolation(int width, int height, const unsigned char *map, float xl, float yl)
{
  int x0 = (int)(xl-0.5f);
  int y0 = (int)(yl-0.5f);
  x0 = x0<0? 0: x0>width-1? width-1: x0;
  y0 = y0<0? 0: y0>height-1? height-1: y0;
  return map[y0*width + x0];
}

unsigned char cdBilinearInterpolation(int width, int height, const unsigned char *map, float xl, float yl)
{
  unsigned char fll, fhl, flh, fhh;
  int x0, y0, x1, y1;
  float t, u;

  if (xl < 0.5)
  {
    x1 = x0 = 0; 
    t = 0;
  }
  else if (xl > width-0.5)
  {
    x1 = x0 = width-1;
    t = 0;
  }
  else
  {
    x0 = (int)(xl-0.5f);
    x1 = x0+1;
    t = xl - (x0+0.5f);
  }

  if (yl < 0.5)
  {
    y1 = y0 = 0; 
    u = 0;
  }
  else if (yl > height-0.5)
  {
    y1 = y0 = height-1;
    u = 0;
  }
  else
  {
    y0 = (int)(yl-0.5f);
    y1 = y0+1;
    u = yl - (y0+0.5f);
  }

  fll = map[y0*width + x0];
  fhl = map[y0*width + x1];
  flh = map[y1*width + x0];
  fhh = map[y1*width + x1];

  return (unsigned char)((fhh - flh - fhl + fll) * u * t +
                         (fhl - fll) * t +
                         (flh - fll) * u +
                                fll);
}

void cdImageRGBInitInverseTransform(int w, int h, int xmin, int xmax, int ymin, int ymax, float *xfactor, float *yfactor, const double* matrix, double* inv_matrix)
{
  *xfactor = (float)(xmax-xmin)/(float)(w-1);
  *yfactor = (float)(ymax-ymin)/(float)(h-1);
  cdMatrixInverse(matrix, inv_matrix);
}

void cdImageRGBInverseTransform(int t_x, int t_y, float *i_x, float *i_y, float xfactor, float yfactor, int xmin, int ymin, int x, int y, double *inv_matrix)
{
  double rx, ry;
  cdfMatrixTransformPoint(inv_matrix, t_x, t_y, &rx, &ry);
  *i_x = xfactor*((float)rx - x) + xmin;
  *i_y = yfactor*((float)ry - y) + ymin;
}

void cdImageRGBCalcDstLimits(cdCanvas* canvas, int x, int y, int w, int h, int *xmin, int *xmax, int *ymin, int *ymax, int* rect)
{
  int t_xmin, t_xmax, t_ymin, t_ymax,
      t_x, t_y, t_w, t_h; 

  /* calculate the bounding box of the transformed rectangle */
  cdMatrixTransformPoint(canvas->matrix, x, y, &t_x, &t_y);
  if (rect) { rect[0] = t_x; rect[1] = t_y; }
  t_xmax = t_xmin = t_x; t_ymax = t_ymin = t_y;
  cdMatrixTransformPoint(canvas->matrix, x+w-1, y, &t_x, &t_y);
  if (rect) { rect[2] = t_x; rect[3] = t_y; }
  if (t_x > t_xmax) t_xmax = t_x;
  if (t_x < t_xmin) t_xmin = t_x;
  if (t_y > t_ymax) t_ymax = t_y;
  if (t_y < t_ymin) t_ymin = t_y;
  cdMatrixTransformPoint(canvas->matrix, x+w-1, y+h-1, &t_x, &t_y);
  if (rect) { rect[4] = t_x; rect[5] = t_y; }
  if (t_x > t_xmax) t_xmax = t_x;
  if (t_x < t_xmin) t_xmin = t_x;
  if (t_y > t_ymax) t_ymax = t_y;
  if (t_y < t_ymin) t_ymin = t_y;
  cdMatrixTransformPoint(canvas->matrix, x, y+h-1, &t_x, &t_y);
  if (rect) { rect[6] = t_x; rect[7] = t_y; }
  if (t_x > t_xmax) t_xmax = t_x;
  if (t_x < t_xmin) t_xmin = t_x;
  if (t_y > t_ymax) t_ymax = t_y;
  if (t_y < t_ymin) t_ymin = t_y;

  t_x = t_xmin;
  t_y = t_ymin;
  t_w = t_xmax-t_xmin+1;
  t_h = t_ymax-t_ymin+1;

  /* check if inside the canvas */
  if (t_x > (canvas->w-1) || t_y > (canvas->h-1) || 
      (t_x+t_w) < 0 || (t_y+t_h) < 0)
    return;

  /* fit to canvas area */
  if (t_x < 0) t_x = 0;
  if (t_y < 0) t_y = 0;
  if ((t_x+t_w) > (canvas->w-1)) t_w = (canvas->w-1)-t_x;
  if ((t_y+t_h) > (canvas->h-1)) t_h = (canvas->h-1)-t_y;

  /* define the destiny limits */
  *xmin = t_x;
  *ymin = t_y;
  *xmax = t_x+t_w-1;
  *ymax = t_y+t_h-1;
}
