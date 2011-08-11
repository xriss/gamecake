/** \file
 * \brief Primitives of the Simulation Base Driver
 *
 * See Copyright Notice in cd.h
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <memory.h>

#include "cd.h"
#include "cd_private.h"


void cdSimLine(cdCtxCanvas* ctxcanvas, int x1, int y1, int x2, int y2)
{
  cdCanvas* canvas = ((cdCtxCanvasBase*)ctxcanvas)->canvas;
  cdPoint poly[2];
  poly[0].x = x1; poly[0].y = y1;
  poly[1].x = x2; poly[1].y = y2;
  cdCanvasPoly(canvas, CD_OPEN_LINES, poly, 2);
}

void cdfSimLine(cdCtxCanvas* ctxcanvas, double x1, double y1, double x2, double y2)
{
  cdCanvas* canvas = ((cdCtxCanvasBase*)ctxcanvas)->canvas;
  cdfPoint poly[2];
  poly[0].x = x1; poly[0].y = y1;
  poly[1].x = x2; poly[1].y = y2;
  canvas->cxFPoly(canvas->ctxcanvas, CD_OPEN_LINES, poly, 2);
}

void cdSimRect(cdCtxCanvas* ctxcanvas, int xmin, int xmax, int ymin, int ymax)
{
  cdCanvas* canvas = ((cdCtxCanvasBase*)ctxcanvas)->canvas;
  cdPoint poly[5]; /* leave room for one more point */
  poly[0].x = xmin; poly[0].y = ymin;
  poly[1].x = xmin; poly[1].y = ymax;
  poly[2].x = xmax; poly[2].y = ymax;
  poly[3].x = xmax; poly[3].y = ymin;
  cdCanvasPoly(canvas, CD_CLOSED_LINES, poly, 4);
}

void cdfSimRect(cdCtxCanvas *ctxcanvas, double xmin, double xmax, double ymin, double ymax)
{
  /* can be used only by drivers that implement cxFPoly */
  cdCanvas* canvas = ((cdCtxCanvasBase*)ctxcanvas)->canvas;
  cdfPoint poly[5]; /* leave room for one more point */
  poly[0].x = xmin; poly[0].y = ymin;
  poly[1].x = xmin; poly[1].y = ymax;
  poly[2].x = xmax; poly[2].y = ymax;
  poly[3].x = xmax; poly[3].y = ymin;
  canvas->cxFPoly(canvas->ctxcanvas, CD_CLOSED_LINES, poly, 4);
}

void cdSimBox(cdCtxCanvas* ctxcanvas, int xmin, int xmax, int ymin, int ymax)
{
  cdCanvas* canvas = ((cdCtxCanvasBase*)ctxcanvas)->canvas;
  cdPoint poly[5]; /* leave room for one more point */
  poly[0].x = xmin; poly[0].y = ymin;
  poly[1].x = xmin; poly[1].y = ymax;
  poly[2].x = xmax; poly[2].y = ymax;
  poly[3].x = xmax; poly[3].y = ymin;
  cdCanvasPoly(canvas, CD_FILL, poly, 4);
}

void cdfSimBox(cdCtxCanvas *ctxcanvas, double xmin, double xmax, double ymin, double ymax)
{
  /* can be used only by drivers that implement cxFPoly */
  cdCanvas* canvas = ((cdCtxCanvasBase*)ctxcanvas)->canvas;
  cdfPoint poly[5]; /* leave room for one more point */
  poly[0].x = xmin; poly[0].y = ymin;
  poly[1].x = xmin; poly[1].y = ymax;
  poly[2].x = xmax; poly[2].y = ymax;
  poly[3].x = xmax; poly[3].y = ymin;
  canvas->cxFPoly(canvas->ctxcanvas, CD_FILL, poly, 4);
}

static int sCalcEllipseNumSegments(cdCanvas* canvas, int xc, int yc, int width, int height, double angle1, double angle2)
{
  int K, dx, dy, hd;
  int w2 = width/2;
  int h2 = height/2;
  int x1 = xc-w2, 
      y1 = yc-h2, 
      x2 = xc+w2, 
      y2 = yc+h2;

  if (canvas->use_matrix)
  {
    cdMatrixTransformPoint(canvas->matrix, x1, y1, &x1, &y1);
    cdMatrixTransformPoint(canvas->matrix, x2, y2, &x2, &y2);
  }

  /* first calculate the number of segments of equivalent poligonal for a full ellipse */

  dx = (x1-x2);
  dy = (y1-y2);
  hd = (int)(sqrt(dx*dx + dy*dy)/2);

  /*  Estimation Heuristic:
  use half diagonal to estimate the number of segments for 360 degrees.
  Use the difference of the half diagonal and its projection to calculate the minimum angle:
  cos(min_angle) = hd / (hd + 1)     or   min_angle = acos(hd / (hd + 1))
  The number of segments will be 360 / min_angle.
  */

  K = (int)((360.0*CD_DEG2RAD) / acos((double)hd / (hd + 1.0)) + 0.5); /* round up */

  /* multiple of 4 */
  K = ((K + 3)/4)*4;

  /* minimum number is 4 */
  if (K < 4) K = 4;


  /* finally, calculate the number of segments for the arc */
  K = cdRound((fabs(angle2-angle1)*K)/(360*CD_DEG2RAD));
  if (K < 1) K = 1;

  return K;
}

static void sFixAngles(cdCanvas* canvas, double *a1, double *a2)
{
  /* computation in PolyAddArc is done as if the angles are counterclockwise, 
     and yaxis is NOT inverted. */

  if (canvas->invert_yaxis)
  {
    /* change orientation */
    *a1 *= -1;
    *a2 *= -1;

    /* no need to swap, because we will use (angle2-angle1) */
  }

  /* convert to radians */
  *a1 *= CD_DEG2RAD;
  *a2 *= CD_DEG2RAD;
}

static cdPoint* sPolyAddArc(cdCanvas* canvas, cdPoint* poly, int *n, int xc, int yc, int width, int height, double angle1, double angle2, cdPoint* current)
{
  double c, s, sx, sy, x, y, prev_x, prev_y;
  double da;
  int i, K, k, p, new_n;
  cdPoint* old_poly = poly;

  sFixAngles(canvas, &angle1, &angle2);

  /* number of segments for the arc */
  K = sCalcEllipseNumSegments(canvas, xc, yc, width, height, angle1, angle2);

  new_n = *n + K+1;  /* add room for K+1 samples */
  poly = (cdPoint*)realloc(poly, sizeof(cdPoint)*(new_n+2));  /* add room also for points at start and end */
  if (!poly) {free(old_poly); return NULL;}
  i = *n;

  /* generates arc points at origin with axis x and y */

  da = (angle2-angle1)/K;
  c  = cos(da);
  s  = sin(da);
  sx = -(width*s)/height;
  sy = (height*s)/width;

  x = (width/2.0f)*cos(angle1);
  y = (height/2.0f)*sin(angle1);
  prev_x = x;
  prev_y = y;

  if (current)
  {
    poly[i] = *current;
    i++;
    new_n++;  /* no need to reallocate */
  }

  poly[i].x = _cdRound(x)+xc;
  poly[i].y = _cdRound(y)+yc;

  p = i+1;
  for (k = 1; k < K+1; k++)
  {
    x =  c*prev_x + sx*prev_y;
    y = sy*prev_x +  c*prev_y;

    poly[p].x = _cdRound(x)+xc;
    poly[p].y = _cdRound(y)+yc;

    if (poly[p-1].x != poly[p].x || 
        poly[p-1].y != poly[p].y)
      p++;

    prev_x = x;
    prev_y = y;
  }

  *n = new_n;
  return poly;
}

static cdfPoint* sfPolyAddArc(cdCanvas* canvas, cdfPoint* poly, int *n, double xc, double yc, double width, double height, double angle1, double angle2, cdfPoint* current)
{
  double c, s, sx, sy, x, y, prev_x, prev_y, da;
  int i, k, K, p, new_n;
  cdfPoint* old_poly = poly;

  sFixAngles(canvas, &angle1, &angle2);

  /* number of segments for the arc */
  K = sCalcEllipseNumSegments(canvas, (int)xc, (int)yc, (int)width, (int)height, angle1, angle2);

  new_n = *n + K+1;  /* add room for K+1 samples */
  poly = (cdfPoint*)realloc(poly, sizeof(cdfPoint)*(new_n+2));  /* add room also for points at start and end */
  if (!poly) {free(old_poly); return NULL;}
  i = *n;

  /* generates arc points at origin with axis x and y */   
  da = (angle2-angle1)/K;
  c  = cos(da);
  s  = sin(da);
  sx = -(width*s)/height;
  sy = (height*s)/width;

  x = (width/2.0f)*cos(angle1);
  y = (height/2.0f)*sin(angle1);
  prev_x = x;
  prev_y = y;

  if (current)
  {
    poly[i] = *current;
    i++;
    new_n++;  /* no need to reallocate */
  }

  poly[i].x = x+xc;
  poly[i].y = y+yc;

  p = i+1;
  for (k = 1; k < K+1; k++)  /* K+1 points */
  {
    x =  c*prev_x + sx*prev_y;
    y = sy*prev_x +  c*prev_y;

    poly[p].x = x+xc;
    poly[p].y = y+yc;

    if (poly[p-1].x != poly[p].x || 
        poly[p-1].y != poly[p].y)
      p++;

    prev_x = x;
    prev_y = y;
  }

  *n = new_n;
  return poly;
}

void cdSimArc(cdCtxCanvas* ctxcanvas, int xc, int yc, int width, int height, double angle1, double angle2)
{
  cdCanvas* canvas = ((cdCtxCanvasBase*)ctxcanvas)->canvas;
  int n = 0;
  cdPoint* poly = NULL;

  if (canvas->line_width == 1 && canvas->cxFPoly)
  {
    cdfSimArc(ctxcanvas, (double)xc, (double)yc, (double)width, (double)height, angle1, angle2);
    return;
  }

  poly = sPolyAddArc(canvas, poly, &n, xc, yc, width, height, angle1, angle2, NULL);

  if (poly)
  {
    cdCanvasPoly(canvas, CD_OPEN_LINES, poly, n);
    free(poly);
  }
}

void cdfSimArc(cdCtxCanvas *ctxcanvas, double xc, double yc, double width, double height, double angle1, double angle2)
{
  /* can be used only by drivers that implement cxFPoly */
  cdCanvas* canvas = ((cdCtxCanvasBase*)ctxcanvas)->canvas;
  int n = 0;
  cdfPoint* poly = NULL;

  poly = sfPolyAddArc(canvas, poly, &n, xc, yc, width, height, angle1, angle2, NULL);

  if (poly)
  {
    canvas->cxFPoly(canvas->ctxcanvas, CD_OPEN_LINES, poly, n);
    free(poly);
  }
}

static void sElipse(cdCtxCanvas* ctxcanvas, int xc, int yc, int width, int height, double angle1, double angle2, int sector)
{
  cdCanvas* canvas = ((cdCtxCanvasBase*)ctxcanvas)->canvas;
  int n = 0;
  cdPoint* poly = NULL;

  poly = sPolyAddArc(canvas, poly, &n, xc, yc, width, height, angle1, angle2, NULL);
  if (!poly)
    return;

  if (poly[n-1].x != poly[0].x || 
      poly[n-1].y != poly[0].y)
  {
    n++; /* no need to reallocate */

    if (sector)  /* cdSector */
    {
      /* add center */
      poly[n-1].x = xc;
      poly[n-1].y = yc;
    }
    else         /* cdChord */
    {
      /* add initial point */
      poly[n-1].x = poly[0].x;
      poly[n-1].y = poly[0].y;
    }
  }

  if (poly)
  {
    cdCanvasPoly(canvas, CD_FILL, poly, n);
    free(poly);
  }
}

static void sfElipse(cdCtxCanvas* ctxcanvas, double xc, double yc, double width, double height, double angle1, double angle2, int sector)
{
  /* can be used only by drivers that implement cxFPoly */
  cdCanvas* canvas = ((cdCtxCanvasBase*)ctxcanvas)->canvas;
  int n = 0;
  cdfPoint* poly = NULL;

  poly = sfPolyAddArc(canvas, poly, &n, xc, yc, width, height, angle1, angle2, NULL);

  if (poly[n-1].x != poly[0].x || 
      poly[n-1].y != poly[0].y)
  {
    n++; /* no need to reallocate */

    if (sector)  /* cdSector */
    {
      /* add center */
      poly[n-1].x = xc;
      poly[n-1].y = yc;
    }
    else         /* cdChord */
    {
      /* add initial point */
      poly[n-1].x = poly[0].x;
      poly[n-1].y = poly[0].y;
    }
  }

  if (poly)
  {
    canvas->cxFPoly(canvas->ctxcanvas, CD_FILL, poly, n);
    free(poly);
  }
}

void cdSimSector(cdCtxCanvas* ctxcanvas, int xc, int yc, int width, int height, double angle1, double angle2)
{
  sElipse(ctxcanvas, xc, yc, width, height, angle1, angle2, 1);
}

void cdSimChord(cdCtxCanvas* ctxcanvas, int xc, int yc, int width, int height, double angle1, double angle2)
{
  sElipse(ctxcanvas, xc, yc, width, height, angle1, angle2, 0);
}

void cdfSimSector(cdCtxCanvas *ctxcanvas, double xc, double yc, double w, double h, double a1, double a2)
{
  sfElipse(ctxcanvas, xc, yc, w, h, a1, a2, 1);
}

void cdfSimChord(cdCtxCanvas *ctxcanvas, double xc, double yc, double w, double h, double a1, double a2)
{
  sfElipse(ctxcanvas, xc, yc, w, h, a1, a2, 0);
}

/**************************************************************/
/* Quick and Simple Bezier Curve Drawing --- Robert D. Miller */
/* Graphics GEMS V                                            */
/**************************************************************/

/* Setup Bezier coefficient array once for each control polygon. 
 */
static void sBezierForm(cdPoint start, const cdPoint* p, cdfPoint* c)
{
  int k; 
  static int choose[4] = {1, 3, 3, 1};
  for (k = 0; k < 4; k++) 
  {
    if (k == 0)
    {
      c[k].x = start.x * choose[k];
      c[k].y = start.y * choose[k];
    }
    else
    {
      c[k].x = p[k-1].x * choose[k];
      c[k].y = p[k-1].y * choose[k];
    }
  }
}

static void sfBezierForm(cdfPoint start, const cdfPoint* p, cdfPoint* c)
{
  int k; 
  static int choose[4] = {1, 3, 3, 1};
  for (k = 0; k < 4; k++) 
  {
    if (k == 0)
    {
      c[k].x = start.x * choose[k];
      c[k].y = start.y * choose[k];
    }
    else
    {
      c[k].x = p[k-1].x * choose[k];
      c[k].y = p[k-1].y * choose[k];
    }
  }
}

/* Return Point pt(t), t <= 0 <= 1 from C. 
 * sBezierForm must be called once for any given control polygon.
 */
static void sBezierCurve(const cdfPoint* c, cdfPoint *pt, double t)
{                  
  int k;
  double t1, tt, u;
  cdfPoint b[4];

  u = t;

  b[0].x = c[0].x;
  b[0].y = c[0].y;
  for(k = 1; k < 4; k++) 
  {
    b[k].x = c[k].x * u;
    b[k].y = c[k].y * u;
    u =u*t;
  }

  pt->x = b[3].x;  
  pt->y = b[3].y;
  t1 = 1-t;          
  tt = t1;
  for(k = 2; k >= 0; k--) 
  {
    pt->x += b[k].x * tt;
    pt->y += b[k].y * tt;
    tt =tt*t1;
  }
}

static int sBezierNumSegments(cdCanvas* canvas, cdPoint start, const cdPoint* p)
{
  int i, K, dx, dy, d,
    xmax = start.x, 
    ymax = start.y, 
    xmin = start.x, 
    ymin = start.y;

  for (i = 1; i < 4; i++)
  {
    if (p[i].x > xmax)
      xmax = p[i].x;
    if (p[i].y > ymax)
      ymax = p[i].y;
    if (p[i].x < xmin)
      xmin = p[i].x;
    if (p[i].y < ymin)
      ymin = p[i].y;
  }

  if (canvas->use_matrix)
  {
    cdMatrixTransformPoint(canvas->matrix, xmin, ymin, &xmin, &ymin);
    cdMatrixTransformPoint(canvas->matrix, xmax, ymax, &xmax, &ymax);
  }

  /* diagonal of the bouding box */
  dx = (xmax-xmin);
  dy = (ymax-ymin);
  d = (int)(sqrt(dx*dx + dy*dy));
  K = d / 8;
  if (K < 8) K = 8;
  return K;
}

static int sfBezierNumSegments(cdCanvas* canvas, cdfPoint start, const cdfPoint* p)
{
  int i, K, d;
  double dx, dy,
    xmax = start.x, 
    ymax = start.y, 
    xmin = start.x, 
    ymin = start.y;

  for (i = 1; i < 4; i++)
  {
    if (p[i].x > xmax)
      xmax = p[i].x;
    if (p[i].y > ymax)
      ymax = p[i].y;
    if (p[i].x < xmin)
      xmin = p[i].x;
    if (p[i].y < ymin)
      ymin = p[i].y;
  }

  if (canvas->use_matrix)
  {
    cdfMatrixTransformPoint(canvas->matrix, xmin, ymin, &xmin, &ymin);
    cdfMatrixTransformPoint(canvas->matrix, xmax, ymax, &xmax, &ymax);
  }

  /* diagonal of the bouding box */
  dx = (xmax-xmin);
  dy = (ymax-ymin);
  d = (int)(sqrt(dx*dx + dy*dy));
  K = d / 8;
  if (K < 8) K = 8;
  return K;
}

static cdPoint* sPolyAddBezier(cdCanvas* canvas, cdPoint* poly, int *n, cdPoint start, const cdPoint* points)
{
  int k, K, new_n, i;
  cdfPoint pt;
  cdfPoint bezier_control[4];
  cdPoint* old_poly = poly;

  sBezierForm(start, points, bezier_control);
  K = sBezierNumSegments(canvas, start, points);

  new_n = *n + K+1;  /* add room for K+1 samples */
  poly = realloc(poly, sizeof(cdPoint)*new_n);
  if (!poly) {free(old_poly); return NULL;}
  i = *n;

  /* first segment */
  sBezierCurve(bezier_control, &pt, 0);

  poly[i].x = _cdRound(pt.x);
  poly[i].y = _cdRound(pt.y);

  for(k = 1; k < K+1; k++) 
  {
    sBezierCurve(bezier_control, &pt, (double)k/(double)K);

    poly[i+k].x = _cdRound(pt.x);
    poly[i+k].y = _cdRound(pt.y);
  }

  *n = new_n;
  return poly;
}

static cdfPoint* sPolyFAddBezier(cdCanvas* canvas, cdfPoint* poly, int *n, cdfPoint start, const cdPoint* points)
{
  int k, K, new_n, i;
  cdfPoint pt;
  cdfPoint bezier_control[4], bezier[3];
  cdfPoint* old_poly = poly;

  bezier[0].x = points[0].x;  bezier[1].x = points[1].x;  bezier[2].x = points[2].x;
  bezier[0].y = points[0].y;  bezier[1].y = points[1].y;  bezier[2].y = points[2].y;

  sfBezierForm(start, bezier, bezier_control);
  K = sfBezierNumSegments(canvas, start, bezier);

  new_n = *n + K+1;  /* add room for K+1 samples */
  poly = realloc(poly, sizeof(cdfPoint)*new_n);
  if (!poly) {free(old_poly); return NULL;}
  i = *n;

  /* first segment */
  sBezierCurve(bezier_control, &pt, 0);

  poly[i] = pt;

  for(k = 1; k < K+1; k++) 
  {
    sBezierCurve(bezier_control, &pt, (double)k/(double)K);
    poly[i+k] = pt;
  }

  *n = new_n;
  return poly;
}

static cdfPoint* sfPolyAddBezier(cdCanvas* canvas, cdfPoint* poly, int *n, cdfPoint start, const cdfPoint* points)
{
  int k, K, new_n, i;
  cdfPoint pt;
  cdfPoint bezier_control[4];
  cdfPoint* old_poly = poly;

  sfBezierForm(start, points, bezier_control);
  K = sfBezierNumSegments(canvas, start, points);

  new_n = *n + K+1;  /* add room for K+1 samples */
  poly = realloc(poly, sizeof(cdfPoint)*new_n); 
  if (!poly) {free(old_poly); return NULL;}
  i = *n;

  /* first segment */
  sBezierCurve(bezier_control, &pt, 0);

  poly[i] = pt;

  for(k = 1; k < K+1; k++) 
  {
    sBezierCurve(bezier_control, &pt, (double)k/(double)K);
    poly[i+k] = pt;
  }

  *n = new_n;
  return poly;
}

static void sPolyFBezier(cdCanvas* canvas, const cdPoint* points, int n)
{
  int i = 0, poly_n = 0;
  cdfPoint* fpoly = NULL, start;

  start.x = points[0].x;
  start.y = points[0].y;

  n--; /* first n is 4 */
  while (n >= 3)
  {
    fpoly = sPolyFAddBezier(canvas, fpoly, &poly_n, start, points+i+1);
    start = fpoly[poly_n-1];
    n -= 3; i += 3;
  }

  if (fpoly)
  {
    canvas->cxFPoly(canvas->ctxcanvas, CD_OPEN_LINES, fpoly, poly_n);
    free(fpoly);
  }
}

void cdSimPolyBezier(cdCanvas* canvas, const cdPoint* points, int n)
{
  int i = 0, poly_n = 0;
  cdPoint* poly = NULL;

  if (canvas->line_width == 1 && canvas->cxFPoly)
  {
    sPolyFBezier(canvas, points, n);
    return;
  }

  n--; /* first n is 4 */
  while (n >= 3)
  {
    poly = sPolyAddBezier(canvas, poly, &poly_n, points[i], points+i+1);
    if (!poly) return;
    n -= 3; i += 3;
  }

  if (poly)
  {
    cdCanvasPoly(canvas, CD_OPEN_LINES, poly, poly_n);
    free(poly);
  }
}

void cdfSimPolyBezier(cdCanvas* canvas, const cdfPoint* points, int n)
{
  /* can be used only by drivers that implement cxFPoly */
  int i = 0, poly_n = 0;
  cdfPoint* poly = NULL;

  n--; /* first n is 4 */
  while (n >= 3)
  {
    poly = sfPolyAddBezier(canvas, poly, &poly_n, points[i], points+i+1);
    n -= 3; i += 3;
  }

  if (poly)
  {
    canvas->cxFPoly(canvas->ctxcanvas, CD_OPEN_LINES, poly, poly_n);
    free(poly);
  }
}

static cdPoint* sPolyAddLine(cdPoint* poly, int *n, cdPoint p1, cdPoint p2)
{
  int new_n, i;
  cdPoint* old_poly = poly;

  new_n = *n + 2;
  poly = realloc(poly, sizeof(cdPoint)*new_n);
  if (!poly) {free(old_poly); return NULL;}
  i = *n;

  poly[i] = p1;
  poly[i+1] = p2;

  *n = new_n;
  return poly;
}

static cdfPoint* sfPolyAddLine(cdfPoint* poly, int *n, cdfPoint p1, cdfPoint p2)
{
  int new_n, i;
  cdfPoint* old_poly = poly;

  new_n = *n + 2;
  poly = realloc(poly, sizeof(cdfPoint)*new_n);
  if (!poly) {free(old_poly); return NULL;}
  i = *n;

  poly[i] = p1;
  poly[i+1] = p2;

  *n = new_n;
  return poly;
}

void cdfSimPolyPath(cdCanvas* canvas, const cdfPoint* poly, int n)
{
  int p, i, current_set = 0, path_poly_n;
  cdfPoint current;
  cdfPoint* path_poly, *old_path_poly;

  current.x = 0; 
  current.y = 0; 
  current_set = 0;

  /* starts a new path */
  path_poly = NULL;
  path_poly_n = 0;

  i = 0;
  for (p=0; p<canvas->path_n; p++)
  {
    switch(canvas->path[p])
    {
    case CD_PATH_NEW:
      if (path_poly)
        free(path_poly);
      path_poly = NULL;
      path_poly_n = 0;
      current_set = 0;
      break;
    case CD_PATH_MOVETO:
      if (i+1 > n) break;
      current = poly[i];
      current_set = 1;
      i++;
      break;
    case CD_PATH_LINETO:
      if (i+1 > n) break;
      path_poly = sfPolyAddLine(path_poly, &path_poly_n, current, poly[i]);
      current = poly[i];
      current_set = 1;
      i++;
      break;
    case CD_PATH_ARC:
      {
        double xc, yc, w, h;
        double a1, a2;

        if (i+3 > n) break;

        if (!cdfCanvasGetArcPath(canvas, poly+i, &xc, &yc, &w, &h, &a1, &a2))
            return;

        if (current_set)
          path_poly = sfPolyAddArc(canvas, path_poly, &path_poly_n, xc, yc, w, h, a1, a2, &current);
        else
          path_poly = sfPolyAddArc(canvas, path_poly, &path_poly_n, xc, yc, w, h, a1, a2, NULL);

        current = path_poly[path_poly_n-1];
        current_set = 1;

        i += 3;
      }
      break;
    case CD_PATH_CURVETO:
      if (i+3 > n) break;
      if (!current_set)
      {
        current.x = poly[i].x;
        current.y = poly[i].y;
      }
      path_poly = sfPolyAddBezier(canvas, path_poly, &path_poly_n, current, poly+i);
      current = path_poly[path_poly_n-1];
      current_set = 1;
      i += 3;
      break;
    case CD_PATH_CLOSE:
      if (path_poly[path_poly_n-1].x != path_poly[0].x || 
          path_poly[path_poly_n-1].y != path_poly[0].y)
      {
        path_poly_n++;
        old_path_poly = path_poly;
        path_poly = (cdfPoint*)realloc(path_poly, sizeof(cdfPoint)*path_poly_n);  
        if (!path_poly) {free(old_path_poly); return;}

        /* add initial point */
        path_poly[path_poly_n-1].x = path_poly[0].x;
        path_poly[path_poly_n-1].y = path_poly[0].y;
      }
      break;
    case CD_PATH_FILL:
      if (poly)
        canvas->cxFPoly(canvas->ctxcanvas, CD_FILL, path_poly, path_poly_n);
      break;
    case CD_PATH_STROKE:
      if (poly)
        canvas->cxFPoly(canvas->ctxcanvas, CD_OPEN_LINES, path_poly, path_poly_n);
      break;
    case CD_PATH_FILLSTROKE:
      if (poly)
      {
        canvas->cxFPoly(canvas->ctxcanvas, CD_FILL, path_poly, path_poly_n);
        canvas->cxFPoly(canvas->ctxcanvas, CD_OPEN_LINES, path_poly, path_poly_n);
      }
      break;
    case CD_PATH_CLIP:
      if (poly)
        canvas->cxFPoly(canvas->ctxcanvas, CD_CLIP, path_poly, path_poly_n);
      break;
    }
  }

  if (path_poly)
    free(path_poly);
}

static void sSimPolyFPath(cdCanvas* canvas, const cdPoint* poly, int n)
{
  int p, i, current_set = 0, path_poly_n;
  cdfPoint current, pt;
  cdfPoint* path_poly, *old_path_poly;

  current.x = 0; 
  current.y = 0; 
  current_set = 0;

  /* starts a new path */
  path_poly = NULL;
  path_poly_n = 0;

  i = 0;
  for (p=0; p<canvas->path_n; p++)
  {
    switch(canvas->path[p])
    {
    case CD_PATH_NEW:
      if (path_poly)
        free(path_poly);
      path_poly = NULL;
      path_poly_n = 0;
      current_set = 0;
      break;
    case CD_PATH_MOVETO:
      if (i+1 > n) break;
      current.x = poly[i].x;
      current.y = poly[i].y;
      current_set = 1;
      i++;
      break;
    case CD_PATH_LINETO:
      if (i+1 > n) break;
      pt.x = poly[i].x;
      pt.y = poly[i].y;
      path_poly = sfPolyAddLine(path_poly, &path_poly_n, current, pt);
      current.x = poly[i].x;
      current.y = poly[i].y;
      current_set = 1;
      i++;
      break;
    case CD_PATH_ARC:
      {
        double xc, yc, w, h;
        double a1, a2;

        if (i+3 > n) break;

        if (!cdCanvasGetArcPathF(canvas, poly+i, &xc, &yc, &w, &h, &a1, &a2))
          return;

        if (current_set)
          path_poly = sfPolyAddArc(canvas, path_poly, &path_poly_n, xc, yc, w, h, a1, a2, &current);
        else
          path_poly = sfPolyAddArc(canvas, path_poly, &path_poly_n, xc, yc, w, h, a1, a2, NULL);

        current = path_poly[path_poly_n-1];
        current_set = 1;

        i += 3;
      }
      break;
    case CD_PATH_CURVETO:
      if (i+3 > n) break;
      if (!current_set)
      {
        current.x = poly[i].x;
        current.y = poly[i].y;
      }
      path_poly = sPolyFAddBezier(canvas, path_poly, &path_poly_n, current, poly+i);
      current = path_poly[path_poly_n-1];
      current_set = 1;
      i += 3;
      break;
    case CD_PATH_CLOSE:
      if (path_poly[path_poly_n-1].x != path_poly[0].x || 
          path_poly[path_poly_n-1].y != path_poly[0].y)
      {
        path_poly_n++;
        old_path_poly = path_poly;
        path_poly = (cdfPoint*)realloc(path_poly, sizeof(cdfPoint)*path_poly_n);  
        if (!path_poly) {free(old_path_poly); return;}

        /* add initial point */
        path_poly[path_poly_n-1].x = path_poly[0].x;
        path_poly[path_poly_n-1].y = path_poly[0].y;
      }
      break;
    case CD_PATH_FILL:
      if (poly)
        canvas->cxFPoly(canvas->ctxcanvas, CD_FILL, path_poly, path_poly_n);
      break;
    case CD_PATH_STROKE:
      if (poly)
        canvas->cxFPoly(canvas->ctxcanvas, CD_OPEN_LINES, path_poly, path_poly_n);
      break;
    case CD_PATH_FILLSTROKE:
      if (poly)
      {
        canvas->cxFPoly(canvas->ctxcanvas, CD_FILL, path_poly, path_poly_n);
        canvas->cxFPoly(canvas->ctxcanvas, CD_OPEN_LINES, path_poly, path_poly_n);
      }
      break;
    case CD_PATH_CLIP:
      if (poly)
        canvas->cxFPoly(canvas->ctxcanvas, CD_CLIP, path_poly, path_poly_n);
      break;
    }
  }

  if (path_poly)
    free(path_poly);
}

void cdSimPolyPath(cdCanvas* canvas, const cdPoint* poly, int n)
{
  int p, i, current_set = 0, path_poly_n;
  cdPoint current;
  cdPoint* path_poly, *old_path_poly;

  if (canvas->line_width == 1 && canvas->cxFPoly)
  {
    int has_curve = 0;
    for (p=0; p<canvas->path_n; p++)
    {
      if (canvas->path[p] == CD_PATH_ARC ||
          canvas->path[p] == CD_PATH_CURVETO)
        has_curve = 1;
      if (canvas->path[p] == CD_PATH_STROKE &&
          has_curve == 1)
      {
        sSimPolyFPath(canvas, poly, n);
        return;
      }
    }
  }

  current.x = 0; 
  current.y = 0; 
  current_set = 0;

  /* starts a new path */
  path_poly = NULL;
  path_poly_n = 0;

  i = 0;
  for (p=0; p<canvas->path_n; p++)
  {
    switch(canvas->path[p])
    {
    case CD_PATH_NEW:
      if (path_poly)
        free(path_poly);
      path_poly = NULL;
      path_poly_n = 0;
      current_set = 0;
      break;
    case CD_PATH_MOVETO:
      if (i+1 > n) break;
      current = poly[i];
      current_set = 1;
      i++;
      break;
    case CD_PATH_LINETO:
      if (i+1 > n) break;
      path_poly = sPolyAddLine(path_poly, &path_poly_n, current, poly[i]);
      if (!path_poly) return;
      current = poly[i];
      current_set = 1;
      i++;
      break;
    case CD_PATH_ARC:
      {
        int xc, yc, w, h;
        double a1, a2;

        if (i+3 > n) break;

        if (!cdCanvasGetArcPath(canvas, poly+i, &xc, &yc, &w, &h, &a1, &a2))
          return;

        if (current_set)
          path_poly = sPolyAddArc(canvas, path_poly, &path_poly_n, xc, yc, w, h, a1, a2, &current);
        else
          path_poly = sPolyAddArc(canvas, path_poly, &path_poly_n, xc, yc, w, h, a1, a2, NULL);
        if (!path_poly) return;

        current = path_poly[path_poly_n-1];
        current_set = 1;

        i += 3;
      }
      break;
    case CD_PATH_CURVETO:
      if (i+3 > n) break;
      if (!current_set)
      {
        current.x = poly[i].x;
        current.y = poly[i].y;
      }
      path_poly = sPolyAddBezier(canvas, path_poly, &path_poly_n, current, poly+i);
      if (!path_poly) return;
      current = path_poly[path_poly_n-1];
      current_set = 1;
      i += 3;
      break;
    case CD_PATH_CLOSE:
      if (path_poly[path_poly_n-1].x != path_poly[0].x || 
          path_poly[path_poly_n-1].y != path_poly[0].y)
      {
        path_poly_n++;
        old_path_poly = path_poly;
        path_poly = (cdPoint*)realloc(path_poly, sizeof(cdPoint)*path_poly_n);  
        if (!path_poly) {free(old_path_poly); return;}

        /* add initial point */
        path_poly[path_poly_n-1].x = path_poly[0].x;
        path_poly[path_poly_n-1].y = path_poly[0].y;
      }
      break;
    case CD_PATH_FILL:
      if (poly)
        cdCanvasPoly(canvas, CD_FILL, path_poly, path_poly_n);
      break;
    case CD_PATH_STROKE:
      if (poly)
        cdCanvasPoly(canvas, CD_OPEN_LINES, path_poly, path_poly_n);
      break;
    case CD_PATH_FILLSTROKE:
      if (poly)
      {
        cdCanvasPoly(canvas, CD_FILL, path_poly, path_poly_n);
        cdCanvasPoly(canvas, CD_OPEN_LINES, path_poly, path_poly_n);
      }
      break;
    case CD_PATH_CLIP:
      if (poly)
        cdCanvasPoly(canvas, CD_CLIP, path_poly, path_poly_n);
      break;
    }
  }

  if (path_poly)
    free(path_poly);
}

/************************************************************************/

/* Simulation functions that depend on the simulation base driver. */
static void cdSimPolyFill(cdCanvas* canvas, cdPoint* poly, int n);
static void cdSimPolyLine(cdCanvas* canvas, const cdPoint* poly, int n);
static void cdfSimPolyLine(cdCanvas* canvas, const cdfPoint* poly, int n);

void cdSimPoly(cdCtxCanvas* ctxcanvas, int mode, cdPoint* poly, int n)
{
  cdCanvas* canvas = ((cdCtxCanvasBase*)ctxcanvas)->canvas;

  switch(mode) 
  {
  case CD_CLOSED_LINES:
    poly[n] = poly[0];   /* can do that because poly is internal of the CD */
    n++;
    /* continue */
  case CD_OPEN_LINES:
    cdSimPolyLine(canvas, poly, n);
    break;
  case CD_BEZIER:
    cdSimPolyBezier(canvas, poly, n);
    break;
  case CD_PATH:
    cdSimPolyPath(canvas, poly, n);
    break;
  case CD_FILL:
    cdSimPolyFill(canvas, poly, n);
    break;
  }
}

void cdfSimPoly(cdCtxCanvas* ctxcanvas, int mode, cdfPoint* fpoly, int n)
{
  cdCanvas* canvas = ((cdCtxCanvasBase*)ctxcanvas)->canvas;

  switch(mode) 
  {
  case CD_CLOSED_LINES:
    fpoly[n] = fpoly[0];
    n++;
    /* continue */
  case CD_OPEN_LINES:
    cdfSimPolyLine(canvas, fpoly, n);
    break;
  case CD_BEZIER:
    cdfSimPolyBezier(canvas, fpoly, n);
    break;
  case CD_PATH:
    cdfSimPolyPath(canvas, fpoly, n);
    break;
  case CD_CLIP:
  case CD_FILL:
    {
      cdPoint* poly = malloc(sizeof(cdPoint)*n);
      int i;

      for (i = 0; i<n; i++)
      {
        poly[i].x = _cdRound(fpoly[i].x);
        poly[i].y = _cdRound(fpoly[i].y);
      }

      cdCanvasPoly(canvas, mode, poly, n);

      free(poly);
    }
    break;
  }
}

void cdSimMark(cdCanvas* canvas, int x, int y)
{
  int oldinteriorstyle = canvas->interior_style;
  int oldlinestyle = canvas->line_style;
  int oldlinewidth = canvas->line_width;
  int size = canvas->mark_size;
  int half_size = size/2;
  int bottom = y-half_size;
  int top    = y+half_size;
  int left   = x-half_size;
  int right  = x+half_size;

  if (canvas->interior_style != CD_SOLID &&
      (canvas->mark_type == CD_CIRCLE ||
       canvas->mark_type == CD_BOX ||
       canvas->mark_type == CD_DIAMOND))
    cdCanvasInteriorStyle(canvas, CD_SOLID);

  if (canvas->line_style != CD_CONTINUOUS &&
      (canvas->mark_type == CD_STAR ||
       canvas->mark_type == CD_PLUS ||
       canvas->mark_type == CD_X ||
       canvas->mark_type == CD_HOLLOW_BOX ||
       canvas->mark_type == CD_HOLLOW_CIRCLE ||
       canvas->mark_type == CD_HOLLOW_DIAMOND))
    cdCanvasLineStyle(canvas, CD_CONTINUOUS);

  if (canvas->line_width != 1 &&
      (canvas->mark_type == CD_STAR ||
       canvas->mark_type == CD_PLUS ||
       canvas->mark_type == CD_X ||
       canvas->mark_type == CD_HOLLOW_BOX ||
       canvas->mark_type == CD_HOLLOW_CIRCLE ||
       canvas->mark_type == CD_HOLLOW_DIAMOND))
    cdCanvasLineWidth(canvas, 1);

  switch (canvas->mark_type)
  {
  case CD_STAR:
    canvas->cxLine(canvas->ctxcanvas, left, bottom, right, top);
    canvas->cxLine(canvas->ctxcanvas, left, top, right, bottom);
    /* continue */
  case CD_PLUS:
    canvas->cxLine(canvas->ctxcanvas, left, y, right, y);
    canvas->cxLine(canvas->ctxcanvas, x, bottom, x, top);
    break;
  case CD_X:
    canvas->cxLine(canvas->ctxcanvas, left, bottom, right, top);
    canvas->cxLine(canvas->ctxcanvas, left, top, right, bottom);
    break;
  case CD_HOLLOW_CIRCLE:
    canvas->cxArc(canvas->ctxcanvas, x, y, size, size, 0, 360);
    break;
  case CD_HOLLOW_BOX:
    canvas->cxRect(canvas->ctxcanvas, left, right, bottom, top);
    break;
  case CD_CIRCLE:
    canvas->cxSector(canvas->ctxcanvas, x, y, size, size, 0, 360);
    break;
  case CD_BOX:
    canvas->cxBox(canvas->ctxcanvas, left, right, bottom, top);
    break;
  case CD_HOLLOW_DIAMOND:
  case CD_DIAMOND:
    {
      cdPoint poly[5];  /* leave room for one more point */
      poly[0].x = left;
      poly[0].y = y;
      poly[1].x = x;
      poly[1].y = top;
      poly[2].x = right;
      poly[2].y = y;
      poly[3].x = x;
      poly[3].y = bottom;

      if (canvas->mark_type == CD_DIAMOND)
        cdCanvasPoly(canvas, CD_FILL, poly, 4);
      else
        cdCanvasPoly(canvas, CD_CLOSED_LINES, poly, 4);
    }
    break;
  }

  if (canvas->interior_style != oldinteriorstyle &&
      (canvas->mark_type == CD_CIRCLE ||
       canvas->mark_type == CD_BOX ||
       canvas->mark_type == CD_DIAMOND))
    cdCanvasInteriorStyle(canvas, oldinteriorstyle);

  if (canvas->line_style != oldlinestyle &&
      (canvas->mark_type == CD_STAR ||
       canvas->mark_type == CD_PLUS ||
       canvas->mark_type == CD_X ||
       canvas->mark_type == CD_HOLLOW_BOX ||
       canvas->mark_type == CD_HOLLOW_CIRCLE ||
       canvas->mark_type == CD_HOLLOW_DIAMOND))
    cdCanvasLineStyle(canvas, oldlinestyle);

  if (canvas->line_width != oldlinewidth &&
      (canvas->mark_type == CD_STAR ||
       canvas->mark_type == CD_PLUS ||
       canvas->mark_type == CD_X ||
       canvas->mark_type == CD_HOLLOW_BOX ||
       canvas->mark_type == CD_HOLLOW_CIRCLE ||
       canvas->mark_type == CD_HOLLOW_DIAMOND))
    cdCanvasLineWidth(canvas, oldlinewidth);
}

void cdSimPutImageRectRGBA(cdCanvas* canvas, int iw, int ih, const unsigned char *r, const unsigned char *g, const unsigned char *b, const unsigned char *a, int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax)
{
  int size, i, j, dst, src, *fx, *fy, rw, rh;
  unsigned char *ar, *ag, *ab, al;
  (void)ih;

  size = w * h;
  ar = (unsigned char*)malloc(size*3);
  if (!ar) return;
  ag = ar + size;
  ab = ag + size;

  canvas->cxGetImageRGB(canvas->ctxcanvas, ar, ag, ab, x, y, w, h);

  rw = xmax-xmin+1;
  rh = ymax-ymin+1;

  fx = cdGetZoomTable(w, rw, xmin);
  fy = cdGetZoomTable(h, rh, ymin);

  for (j = 0; j < h; j++)
  {
    for (i = 0; i < w; i++)
    {
      dst = j * w + i;
      src = fy[j] * iw + fx[i];
      al = a[src];
      ar[dst] = CD_ALPHA_BLEND(r[src], ar[dst], al);
      ag[dst] = CD_ALPHA_BLEND(g[src], ag[dst], al);
      ab[dst] = CD_ALPHA_BLEND(b[src], ab[dst], al);
    }
  }

  canvas->cxPutImageRectRGB(canvas->ctxcanvas, w, h, ar, ag, ab, x, y, w, h, 0, 0, 0, 0);

  free(ar);

  free(fx);
  free(fy);
}

void cdSimPutImageRectRGB(cdCanvas* canvas, int iw, int ih, const unsigned char *r, const unsigned char *g, const unsigned char *b, int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax)
{
  int height = ymax-ymin+1;
  unsigned char* map;
  int pal_size = 1L << canvas->bpp;
  long colors[256];
  (void)ih;

  map = (unsigned char*)malloc(iw * height);
  if (!map)
    return;

  if (pal_size == 2) /* probably a laser printer, use a gray image for better results */
    cdRGB2Gray(iw, height, r+ymin*iw, g+ymin*iw, b+ymin*iw, map, colors);
  else
    cdRGB2Map(iw, height, r+ymin*iw, g+ymin*iw, b+ymin*iw, map, pal_size, colors);

  canvas->cxPutImageRectMap(canvas->ctxcanvas, iw, height, map, colors, x, y, w, h, xmin, xmax, 0, height-1);

  free(map);
}


/************************************************************************/

#include "cd_truetype.h"
#include "sim.h"

static void cdSimPolyLine(cdCanvas* canvas, const cdPoint* poly, int n)
{
  int i, reset = 1, transform = 0;
  int old_use_matrix = canvas->use_matrix;
  int x1, y1, x2, y2;

  if (canvas->use_matrix)
    transform = 1;

  /* disable line transformation */
  canvas->use_matrix = 0;

  /* prepare the line style for several lines */
  if (simLineStyleNoReset)
  {
    reset = 0;
    simLineStyleNoReset = 1;
  }

  x1 = poly[0].x;
  y1 = poly[0].y;

  if (transform)
    cdMatrixTransformPoint(canvas->matrix, x1, y1, &x1, &y1);

  for (i = 0; i < n-1; i++)
  {
    x2 = poly[i+1].x;
    y2 = poly[i+1].y;

    if (transform)
      cdMatrixTransformPoint(canvas->matrix, x2, y2, &x2, &y2);

    if(canvas->line_width > 1)
      simLineThick(canvas, x1, y1, x2, y2);
    else 
      simLineThin(canvas, x1, y1, x2, y2);

    x1 = x2;
    y1 = y2;
  }

  if (reset) simLineStyleNoReset = 0;
  canvas->use_matrix = old_use_matrix;
}

static void cdfSimPolyLine(cdCanvas* canvas, const cdfPoint* poly, int n)
{
  int i, reset = 1, transform = 0;
  int old_use_matrix = canvas->use_matrix;
  double x1, y1, x2, y2;
  int last_xi_a = -65535, 
      last_yi_a = -65535, 
      last_xi_b = -65535, 
      last_yi_b = -65535;

  if (canvas->use_matrix)
    transform = 1;

  /* disable line transformation */
  canvas->use_matrix = 0;

  /* prepare the line style for several lines */
  if (simLineStyleNoReset)
  {
    reset = 0;
    simLineStyleNoReset = 1;
  }

  x1 = poly[0].x;
  y1 = poly[0].y;

  if (transform)
    cdfMatrixTransformPoint(canvas->matrix, x1, y1, &x1, &y1);

  for (i = 0; i < n-1; i++)
  {
    x2 = poly[i+1].x;
    y2 = poly[i+1].y;

    if (transform)
      cdfMatrixTransformPoint(canvas->matrix, x2, y2, &x2, &y2);

    if(canvas->line_width > 1)
      simfLineThick(canvas, x1, y1, x2, y2);
    else 
      simfLineThin(canvas, x1, y1, x2, y2, &last_xi_a, &last_yi_a, &last_xi_b, &last_yi_b);

    x1 = x2;
    y1 = y2;
  }

  if (reset) simLineStyleNoReset = 0;
  canvas->use_matrix = old_use_matrix;
}

static int sCheckIsBox(cdPoint* poly)
{
  if (poly[0].x == poly[1].x && 
      poly[1].y == poly[2].y &&
      poly[2].x == poly[3].x &&
      poly[3].y == poly[0].y)
    return 1;

  if (poly[0].y == poly[1].y && 
      poly[1].x == poly[2].x &&
      poly[2].y == poly[3].y &&
      poly[3].x == poly[0].x)
    return 1;

  return 0;
}

static void sGetBox(cdPoint* poly, int *xmin, int *xmax, int *ymin, int *ymax)
{
  int i;
  *xmin = poly[0].x;
  *xmax = poly[0].x;
  *ymin = poly[0].y;
  *ymax = poly[0].y;
  for (i=1; i<4; i++)
  {
    if (poly[i].x < *xmin)
      *xmin = poly[i].x;
    if (poly[i].y < *ymin)
      *ymin = poly[i].y;
    if (poly[i].x > *xmax)
      *xmax = poly[i].x;
    if (poly[i].y > *ymin)
      *ymax = poly[i].y;
  }
}

static void cdSimPolyFill(cdCanvas* canvas, cdPoint* poly, int n)
{
  int old_use_matrix = canvas->use_matrix;

  if (canvas->use_matrix)
  {
    int i;
    for(i = 0; i < n; i++)    /* can do that because poly is internal of the CD, and it will NOT be stored */
      cdMatrixTransformPoint(canvas->matrix, poly[i].x, poly[i].y, &poly[i].x, &poly[i].y);
  }

  /* disable fill transformation */
  canvas->use_matrix = 0;

  if (n == 4 && sCheckIsBox(poly))
  {
    int xmin, xmax, ymin, ymax;
    sGetBox(poly, &xmin, &xmax, &ymin, &ymax);
    simFillHorizBox(canvas->simulation, xmin, xmax, ymin, ymax);
  }
  else
    simPolyFill(canvas->simulation, poly, n);

  canvas->use_matrix = old_use_matrix;
}

