/** \file
 * \brief SVG driver
 *
 * See Copyright Notice in cd.h
 */

#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <limits.h> 
#include <math.h>
#include <locale.h>

#include "cd.h"
#include "wd.h"
#include "cd_private.h"
#include "cdsvg.h"

#include "lodepng.h"
#include "base64.h"


struct _cdCtxCanvas 
{
  cdCanvas* canvas;

  char bgColor[20];
  char fgColor[20]; 
  char* linecap;
  char* linejoin;
  char linestyle[50];
  char pattern[50];

  char* old_locale;
  char* font_weight;
  char* font_style;
  char* font_decoration;
  char font_family[256];
  char font_size[10];

  double opacity;
  int hatchboxsize;

  /* private */
  int last_fill_mode;
  
  int last_clip_poly;
  int last_clip_rect;
  int clip_control;
  int clip_polygon;

  int transform_control;

  FILE* file;
};

static void cdtransform(cdCtxCanvas *ctxcanvas, const double* matrix);

static void cdkillcanvas(cdCtxCanvas* ctxcanvas)
{
  if (ctxcanvas->clip_control)
    fprintf(ctxcanvas->file, "</g>\n");  /* close clipping container */

  if (ctxcanvas->transform_control)
    fprintf(ctxcanvas->file, "</g>\n");  /* close transform container */

  fprintf(ctxcanvas->file, "</g>\n");  /* close global container */
  fprintf(ctxcanvas->file, "</svg>\n");

  fclose(ctxcanvas->file);

  if (ctxcanvas->old_locale)
  {
    setlocale(LC_NUMERIC, ctxcanvas->old_locale);
    free(ctxcanvas->old_locale);
  }

  memset(ctxcanvas, 0, sizeof(cdCtxCanvas));
  free(ctxcanvas);
}

static int cdclip(cdCtxCanvas *ctxcanvas, int clip_mode)
{
  if (ctxcanvas->clip_control)
  {
    int old_transform_control = ctxcanvas->transform_control;
    if (ctxcanvas->transform_control)
    {
      fprintf(ctxcanvas->file, "</g>\n");  /* close transform container */
      ctxcanvas->transform_control = 0;
    }

    fprintf(ctxcanvas->file, "</g>\n");  /* close clipping container */
    ctxcanvas->clip_control = 0;

    if (old_transform_control)
      cdtransform(ctxcanvas, ctxcanvas->canvas->matrix);  /* reopen transform container */
  }

  switch (clip_mode)
  {
    case CD_CLIPAREA:
      /* open clipping container */
      fprintf(ctxcanvas->file, "<g clip-path=\"url(#cliprect%d)\">\n", ctxcanvas->last_clip_rect);
      ctxcanvas->clip_control = 1;
      break;
    case CD_CLIPPOLYGON:
      if (ctxcanvas->clip_polygon)
      {
        /* open clipping container */
        fprintf(ctxcanvas->file, "<g clip-path=\"url(#clippoly%d)\" clip-rule:%s >\n", ctxcanvas->last_clip_poly, (ctxcanvas->canvas->fill_mode==CD_EVENODD)? "evenodd": "nonzero");
        ctxcanvas->clip_control = 1;
      }
      break;
  }

  return clip_mode;
}

static void cdfcliparea(cdCtxCanvas *ctxcanvas, double xmin, double xmax, double ymin, double ymax)
{
  double x, y, w, h;

  ctxcanvas->canvas->clip_rect.xmin = (int)xmin;
  ctxcanvas->canvas->clip_rect.ymin = (int)ymin;
  ctxcanvas->canvas->clip_rect.xmax = (int)xmax;
  ctxcanvas->canvas->clip_rect.ymax = (int)ymax;

  x = xmin;
  y = ymin;
  w = xmax - xmin + 1;
  h = ymax - ymin + 1;

  fprintf(ctxcanvas->file, "<clipPath id=\"cliprect%d\">\n", ++ctxcanvas->last_clip_rect);

  fprintf(ctxcanvas->file, "<rect x=\"%g\" y=\"%g\" width=\"%g\" height=\"%g\" />\n", x, y, w, h);

  fprintf(ctxcanvas->file, "</clipPath>\n");

  if (ctxcanvas->canvas->clip_mode == CD_CLIPAREA) 
    cdclip(ctxcanvas, CD_CLIPAREA);
}

static void cdcliparea(cdCtxCanvas *ctxcanvas, int xmin, int xmax, int ymin, int ymax)
{
  cdfcliparea(ctxcanvas, (double)xmin, (double)xmax, (double)ymin, (double)ymax);
}

static void cdtransform(cdCtxCanvas *ctxcanvas, const double* matrix)
{
  if (ctxcanvas->transform_control)
  {
    int old_clip_control = ctxcanvas->clip_control;
    if (ctxcanvas->clip_control)
    {
      fprintf(ctxcanvas->file, "</g>\n");  /* close clipping container */
      ctxcanvas->clip_control = 0;
    }

    fprintf(ctxcanvas->file, "</g>\n");  /* close transform container */
    ctxcanvas->transform_control = 0;

    if (old_clip_control)
      cdclip(ctxcanvas, ctxcanvas->canvas->clip_mode);  /* reopen clipping container */
  }

  if (matrix)
  {
    double xmatrix[6];

    /* Matrix identity + invert axis */
    xmatrix[0] = 1; 
    xmatrix[1] = 0;
    xmatrix[2] = 0; 
    xmatrix[3] = -1; 
    xmatrix[4] = 0; 
    xmatrix[5] = (ctxcanvas->canvas->h-1);

    /* compose transform */
    cdMatrixMultiply(matrix, xmatrix);

    /* open transform container */
    fprintf(ctxcanvas->file, "<g transform=\"matrix(%g %g %g %g %g %g)\">\n", xmatrix[0], xmatrix[1], xmatrix[2], xmatrix[3], xmatrix[4], xmatrix[5]);
    ctxcanvas->transform_control = 1;

    ctxcanvas->canvas->invert_yaxis = 0;
  }
  else
    ctxcanvas->canvas->invert_yaxis = 1;
}

static void cdfline(cdCtxCanvas *ctxcanvas, double x1, double y1, double x2, double y2)
{
  fprintf(ctxcanvas->file, "<line x1=\"%g\" y1=\"%g\" x2=\"%g\" y2=\"%g\" style=\"fill:none; stroke:%s; stroke-width:%d; stroke-linecap:%s; stroke-linejoin:%s; stroke-dasharray:%s; opacity:%g\" />\n",
          x1, y1, x2, y2, ctxcanvas->fgColor, ctxcanvas->canvas->line_width, ctxcanvas->linecap, ctxcanvas->linejoin, ctxcanvas->linestyle, ctxcanvas->opacity);
}

static void cdline(cdCtxCanvas *ctxcanvas, int x1, int y1, int x2, int y2)
{
  cdfline(ctxcanvas, (double)x1, (double)y1, (double)x2, (double)y2);
}

static void cdfrect(cdCtxCanvas *ctxcanvas, double xmin, double xmax, double ymin, double ymax)
{
  fprintf(ctxcanvas->file, "<rect x=\"%g\" y=\"%g\" width=\"%g\" height=\"%g\" style=\"fill:none; stroke:%s; stroke-width:%d; stroke-linecap:%s; stroke-linejoin:%s; stroke-dasharray:%s; opacity:%g\" />\n",
          xmin, ymin, xmax-xmin, ymax-ymin, ctxcanvas->fgColor, ctxcanvas->canvas->line_width, ctxcanvas->linecap, ctxcanvas->linejoin, ctxcanvas->linestyle, ctxcanvas->opacity);
}

static void cdrect(cdCtxCanvas *ctxcanvas, int xmin, int xmax, int ymin, int ymax)
{
  cdfrect(ctxcanvas, (double)xmin, (double)xmax, (double)ymin, (double)ymax);
}

static void cdfbox(cdCtxCanvas *ctxcanvas, double xmin, double xmax, double ymin, double ymax)
{
  fprintf(ctxcanvas->file, "<rect x=\"%g\" y=\"%g\" width=\"%g\" height=\"%g\" style=\"fill:%s; stroke:none; opacity:%g\" />\n",
          xmin, ymin, xmax-xmin, ymax-ymin, (ctxcanvas->canvas->interior_style == CD_SOLID) ? ctxcanvas->fgColor: ctxcanvas->pattern, ctxcanvas->opacity);
}

static void cdbox(cdCtxCanvas *ctxcanvas, int xmin, int xmax, int ymin, int ymax)
{
  cdfbox(ctxcanvas, (double)xmin, (double)xmax, (double)ymin, (double)ymax);
}

static void sCalcArc(cdCanvas* canvas, double xc, double yc, double w, double h, double a1, double a2, double *arcStartX, double *arcStartY, double *arcEndX, double *arcEndY, int *largeArc, int swap)
{
  /* computation is done as if the angles are counterclockwise, 
     and yaxis is NOT inverted. */

  cdfCanvasGetArcStartEnd(xc, yc, w, h, a1, a2, arcStartX, arcStartY, arcEndX, arcEndY);

  if (canvas->invert_yaxis)
  {
    /* fix axis orientation */
    *arcStartY = 2*yc - *arcStartY;
    *arcEndY = 2*yc - *arcEndY;
  }
  else
  {
    /* it is clock-wise when axis NOT inverted */
    if (swap)
    {
      _cdSwapDouble(*arcStartX, *arcEndX);
      _cdSwapDouble(*arcStartY, *arcEndY);
    }
  }

  if (fabs(a2-a1) > 180.0)
    *largeArc = 1;
  else
    *largeArc = 0;
}

static void cdfarc(cdCtxCanvas *ctxcanvas, double xc, double yc, double w, double h, double a1, double a2)
{
  double arcStartX, arcStartY, arcEndX, arcEndY;
  int largeArc;

  if((a1 == 0.0) && (a2 == 360.0)) /* an ellipse/circle */
  {
    fprintf(ctxcanvas->file, "<ellipse cx=\"%g\" cy=\"%g\" rx=\"%g\" ry=\"%g\" style=\"fill:none; stroke:%s; stroke-width:%d; stroke-linecap:%s; stroke-linejoin:%s; stroke-dasharray:%s; opacity:%g\" />\n",
      xc, yc, w/2, h/2, ctxcanvas->fgColor, ctxcanvas->canvas->line_width, ctxcanvas->linecap, ctxcanvas->linejoin, ctxcanvas->linestyle, ctxcanvas->opacity);
    return;
  }

  sCalcArc(ctxcanvas->canvas, xc, yc, w, h, a1, a2, &arcStartX, &arcStartY, &arcEndX, &arcEndY, &largeArc, 1);

  fprintf(ctxcanvas->file, "<path d=\"M%g,%g A%g,%g 0 %d,0 %g,%g\" style=\"fill:none; stroke:%s; stroke-width:%d; stroke-linecap:%s; stroke-linejoin:%s; stroke-dasharray:%s; opacity:%g\" />\n",
    arcStartX, arcStartY, w/2, h/2, largeArc, arcEndX, arcEndY, ctxcanvas->fgColor, ctxcanvas->canvas->line_width, ctxcanvas->linecap, ctxcanvas->linejoin, ctxcanvas->linestyle, ctxcanvas->opacity);
}

static void cdarc(cdCtxCanvas *ctxcanvas, int xc, int yc, int w, int h, double a1, double a2)
{
  cdfarc(ctxcanvas, (double)xc, (double)yc, (double)w, (double)h, a1, a2);
}

static void cdfsector(cdCtxCanvas *ctxcanvas, double xc, double yc, double w, double h, double a1, double a2)
{
  double arcStartX, arcStartY, arcEndX, arcEndY;
  int largeArc;

  if((a1 == 0.0) && (a2 == 360.0)) /* an ellipse/circle */
  {
    fprintf(ctxcanvas->file, "<ellipse cx=\"%g\" cy=\"%g\" rx=\"%g\" ry=\"%g\" style=\"fill:%s; stroke:none; opacity:%g\" />\n",
            xc, yc, w/2, h/2, (ctxcanvas->canvas->interior_style == CD_SOLID) ? ctxcanvas->fgColor: ctxcanvas->pattern, ctxcanvas->opacity);
    return;
  }

  sCalcArc(ctxcanvas->canvas, xc, yc, w, h, a1, a2, &arcStartX, &arcStartY, &arcEndX, &arcEndY, &largeArc, 1);

  fprintf(ctxcanvas->file, "<path d=\"M%g,%g L%g,%g A%g,%g 0 %d,0 %g,%g Z\" style=\"fill:%s; stroke:none; opacity:%g\" />\n",
          xc, yc, arcStartX, arcStartY, w/2, h/2, largeArc, arcEndX, arcEndY, (ctxcanvas->canvas->interior_style == CD_SOLID) ? ctxcanvas->fgColor: ctxcanvas->pattern, ctxcanvas->opacity);
}

static void cdsector(cdCtxCanvas *ctxcanvas, int xc, int yc, int w, int h, double a1, double a2)
{
  cdfsector(ctxcanvas, (double)xc, (double)yc, (double)w, (double)h, a1, a2);
}

static void cdfchord(cdCtxCanvas *ctxcanvas, double xc, double yc, double w, double h, double a1, double a2)
{
  double arcStartX, arcStartY, arcEndX, arcEndY;
  int largeArc;

  sCalcArc(ctxcanvas->canvas, xc, yc, w, h, a1, a2, &arcStartX, &arcStartY, &arcEndX, &arcEndY, &largeArc, 1);

  fprintf(ctxcanvas->file, "<path d=\"M%g,%g A%g,%g 0 %d,0 %g,%g Z\" style=\"fill:%s; stroke:none; opacity:%g\" />\n",
          arcStartX, arcStartY, w/2, h/2, largeArc, arcEndX, arcEndY, (ctxcanvas->canvas->interior_style == CD_SOLID) ? ctxcanvas->fgColor: ctxcanvas->pattern, ctxcanvas->opacity);
}

static void cdchord(cdCtxCanvas *ctxcanvas, int xc, int yc, int w, int h, double a1, double a2)
{
  cdfchord(ctxcanvas, (double)xc, (double)yc, (double)w, (double)h, a1, a2);
}

static void cdftext(cdCtxCanvas *ctxcanvas, double x, double y, const char *text, int len)
{
  char* anchor;
  char* alignment;
  int i;

  switch (ctxcanvas->canvas->text_alignment)
  {
    case CD_NORTH:
    case CD_NORTH_EAST:
    case CD_NORTH_WEST:
      alignment = "text-before-edge";
      break;
    case CD_SOUTH:
    case CD_SOUTH_EAST:
    case CD_SOUTH_WEST:
      alignment = "text-after-edge";
      break;
    case CD_CENTER:
    case CD_EAST:
    case CD_WEST:
      alignment = "middle";
      break;
    case CD_BASE_CENTER:
    case CD_BASE_LEFT:
    case CD_BASE_RIGHT:
    default:
      alignment = "baseline";
      break;
  }

  switch (ctxcanvas->canvas->text_alignment)
  {
    case CD_WEST:
    case CD_NORTH_WEST:
    case CD_SOUTH_WEST:
    case CD_BASE_LEFT:
      anchor = "start";
      break;
    case CD_CENTER:
    case CD_NORTH:
    case CD_SOUTH:
    case CD_BASE_CENTER:
      anchor = "middle";
      break;
    case CD_EAST:
    case CD_NORTH_EAST:
    case CD_SOUTH_EAST:
    case CD_BASE_RIGHT:
    default:
      anchor = "end";
      break;
  }

  /* Characters are putting in file using hexadecimal representations */
  /* <text> string </text> no print special characters, like cedilla and acutes */
  /* Future solution: use glyphs in embedded fonts (TO DO)*/

  if (ctxcanvas->canvas->text_orientation != 0)
  {
    double text_cos = cos(ctxcanvas->canvas->text_orientation*CD_DEG2RAD);
    double text_sin = sin(ctxcanvas->canvas->text_orientation*CD_DEG2RAD);

    if (ctxcanvas->canvas->use_matrix)  /* Transformation active */
      fprintf(ctxcanvas->file, "<text transform=\"matrix(%g %g %g %g %g %g)\" font-family=\"%s\" font-size=\"%s\" font-style=\"%s\" font-weight=\"%s\" text-decoration=\"%s\" text-anchor=\"%s\" dominant-baseline=\"%s\" fill=\"%s\">\n", 
         text_cos, text_sin, text_sin, -text_cos, x, y, ctxcanvas->font_family, ctxcanvas->font_size, ctxcanvas->font_style, ctxcanvas->font_weight, ctxcanvas->font_decoration, anchor, alignment, ctxcanvas->fgColor);
    else
      fprintf(ctxcanvas->file, "<text transform=\"matrix(%g %g %g %g %g %g)\" font-family=\"%s\" font-size=\"%s\" font-style=\"%s\" font-weight=\"%s\" text-decoration=\"%s\" text-anchor=\"%s\" dominant-baseline=\"%s\" fill=\"%s\">\n", 
         text_cos, -text_sin, text_sin, text_cos, x, y, ctxcanvas->font_family, ctxcanvas->font_size, ctxcanvas->font_style, ctxcanvas->font_weight, ctxcanvas->font_decoration, anchor, alignment, ctxcanvas->fgColor);
    
    for(i = 0; i < len; i++)
      fprintf(ctxcanvas->file, "&#x%02X;", (unsigned char)text[i]);

    fprintf(ctxcanvas->file, "\n</text>\n");
  }
  else
  {
    fprintf(ctxcanvas->file, "<text x=\"%g\" y=\"%g\" font-family=\"%s\" font-size=\"%s\" font-style=\"%s\" font-weight=\"%s\" text-decoration=\"%s\" text-anchor=\"%s\" dominant-baseline=\"%s\" fill=\"%s\">\n", 
      x, y, ctxcanvas->font_family, ctxcanvas->font_size, ctxcanvas->font_style, ctxcanvas->font_weight, ctxcanvas->font_decoration, anchor, alignment, ctxcanvas->fgColor);

    for(i = 0; i < len; i++)
      fprintf(ctxcanvas->file, "&#x%02X;", (unsigned char)text[i]);

    fprintf(ctxcanvas->file, "\n</text>\n");
  }
}

static void cdtext(cdCtxCanvas *ctxcanvas, int x, int y, const char *text, int len)
{
  cdftext(ctxcanvas, (double)x, (double)y, text, len);
}

static void sWritePointsF(cdCtxCanvas *ctxcanvas, cdfPoint* poly, int n, int close)
{
  int i;
  for(i = 0; i<n; i++)
    fprintf(ctxcanvas->file, "%g,%g ", poly[i].x, poly[i].y);
  if (close)
    fprintf(ctxcanvas->file, "%g,%g ", poly[0].x, poly[0].y);
}

static void sWritePoints(cdCtxCanvas *ctxcanvas, cdPoint* poly, int n, int close)
{
  int i;
  for(i = 0; i<n; i++)
    fprintf(ctxcanvas->file, "%d,%d ", poly[i].x, poly[i].y);
  if (close)
    fprintf(ctxcanvas->file, "%d,%d ", poly[0].x, poly[0].y);
}

static void cdfpoly(cdCtxCanvas *ctxcanvas, int mode, cdfPoint* poly, int n)
{
  char* rule;

  if (mode == CD_PATH)
  {
    int i, p, clip_path = 0, end_path, current_set;

    for (p=0; p<ctxcanvas->canvas->path_n; p++)
    {
      if (ctxcanvas->canvas->path[p] == CD_PATH_CLIP)
      {
        clip_path = 1;
        break;
      }
    }

    if (clip_path)
      fprintf(ctxcanvas->file, "<clipPath id=\"clippoly%d\">\n", ++ctxcanvas->last_clip_poly);

    /* starts a new path */
    fprintf(ctxcanvas->file, "<path d=\"");
    end_path = 0;
    current_set = 0;

    i = 0;
    for (p=0; p<ctxcanvas->canvas->path_n; p++)
    {
      switch(ctxcanvas->canvas->path[p])
      {
      case CD_PATH_NEW:
        if (!end_path)
          fprintf(ctxcanvas->file, "\" />\n");

        fprintf(ctxcanvas->file, "<path d=\"");
        end_path = 0;
        current_set = 0;
        break;
      case CD_PATH_MOVETO:
        if (i+1 > n) return;
        fprintf(ctxcanvas->file, "M %g %g ", poly[i].x, poly[i].y);
        current_set = 1;
        i++;
        break;
      case CD_PATH_LINETO:
        if (i+1 > n) return;
        fprintf(ctxcanvas->file, "L %g %g ", poly[i].x, poly[i].y);
        current_set = 1;
        i++;
        break;
      case CD_PATH_ARC:
        {
          double xc, yc, w, h, a1, a2;
          double arcStartX, arcStartY, arcEndX, arcEndY;
          int largeArc, sweep = 0;

          if (i+3 > n) return;

          if (!cdfCanvasGetArcPath(ctxcanvas->canvas, poly+i, &xc, &yc, &w, &h, &a1, &a2))
            return;

          sCalcArc(ctxcanvas->canvas, xc, yc, w, h, a1, a2, &arcStartX, &arcStartY, &arcEndX, &arcEndY, &largeArc, 0);

          if (ctxcanvas->canvas->invert_yaxis && (a2-a1)<0) /* can be clockwise */
            sweep = 1;

          if (current_set)
            fprintf(ctxcanvas->file, "L %g %g A %g %g 0 %d %d %g %g ",
                    arcStartX, arcStartY, w/2, h/2, largeArc, sweep, arcEndX, arcEndY);
          else
            fprintf(ctxcanvas->file, "M %g %g A %g %g 0 %d %d %g %g ",
                    arcStartX, arcStartY, w/2, h/2, largeArc, sweep, arcEndX, arcEndY);

          current_set = 1;
          i += 3;
        }
        break;
      case CD_PATH_CURVETO:
        if (i+3 > n) return;
        fprintf(ctxcanvas->file, "C %g %g %g %g %g %g ", poly[i].x,   poly[i].y, 
                                                         poly[i+1].x, poly[i+1].y, 
                                                         poly[i+2].x, poly[i+2].y);
        current_set = 1;
        i += 3;
        break;
      case CD_PATH_CLOSE:
        fprintf(ctxcanvas->file, "Z ");
        break;
      case CD_PATH_FILL:
        rule = (ctxcanvas->canvas->fill_mode==CD_EVENODD)? "evenodd": "nonzero";
        fprintf(ctxcanvas->file, "\" style=\"fill:%s; fill-rule:%s; stroke:none; opacity:%g\" />\n",
                (ctxcanvas->canvas->interior_style == CD_SOLID) ? ctxcanvas->fgColor: ctxcanvas->pattern, rule, ctxcanvas->opacity);
        end_path = 1;
        break;
      case CD_PATH_STROKE:
        fprintf(ctxcanvas->file, "\" style=\"fill:none; stroke:%s; stroke-width:%d; stroke-linecap:%s; stroke-linejoin:%s; stroke-dasharray:%s; opacity:%g\" />\n",
                ctxcanvas->fgColor, ctxcanvas->canvas->line_width, ctxcanvas->linecap, ctxcanvas->linejoin, ctxcanvas->linestyle, ctxcanvas->opacity);
        end_path = 1;
        break;
      case CD_PATH_FILLSTROKE:
        rule = (ctxcanvas->canvas->fill_mode==CD_EVENODD)? "evenodd": "nonzero";
        fprintf(ctxcanvas->file, "\" style=\"fill:%s; fill-rule:%s; stroke:%s; stroke-width:%d; stroke-linecap:%s; stroke-linejoin:%s; stroke-dasharray:%s; opacity:%g\" />\n",
                (ctxcanvas->canvas->interior_style == CD_SOLID) ? ctxcanvas->fgColor: ctxcanvas->pattern, rule, ctxcanvas->fgColor, ctxcanvas->canvas->line_width, ctxcanvas->linecap, ctxcanvas->linejoin, ctxcanvas->linestyle, ctxcanvas->opacity);
        end_path = 1;
        break;
      case CD_PATH_CLIP:
        fprintf(ctxcanvas->file, "\" />\n");
        fprintf(ctxcanvas->file, "</clipPath>\n");
        ctxcanvas->clip_polygon = 1;
        cdclip(ctxcanvas, CD_CLIPPOLYGON);
        end_path = 1;
        break;
      }
    }
    return;
  }

  switch (mode)
  {
  case CD_CLOSED_LINES:
    fprintf(ctxcanvas->file, "<polygon style=\"fill:none; stroke:%s; stroke-width:%d; stroke-linecap:%s; stroke-linejoin:%s; stroke-dasharray:%s; opacity:%g\" points=\"",
            ctxcanvas->fgColor, ctxcanvas->canvas->line_width, ctxcanvas->linecap, ctxcanvas->linejoin, ctxcanvas->linestyle, ctxcanvas->opacity);
    sWritePointsF(ctxcanvas, poly, n, 1);
    fprintf(ctxcanvas->file, "\" />\n");
    break;
  case CD_OPEN_LINES:
    fprintf(ctxcanvas->file, "<polyline style=\"fill:none; stroke:%s; stroke-width:%d; stroke-linecap:%s; stroke-linejoin:%s; stroke-dasharray:%s; opacity:%g\" points=\"",
            ctxcanvas->fgColor, ctxcanvas->canvas->line_width, ctxcanvas->linecap, ctxcanvas->linejoin, ctxcanvas->linestyle, ctxcanvas->opacity);
    sWritePointsF(ctxcanvas, poly, n, 0);
    fprintf(ctxcanvas->file, "\" />\n");
    break;
  case CD_BEZIER:
    fprintf(ctxcanvas->file, "<path d=\"M%g,%g C", poly[0].x, poly[0].y);
    sWritePointsF(ctxcanvas, poly+1, n-1, 0);
    fprintf(ctxcanvas->file, "\" style=\"fill:none; stroke:%s; stroke-width:%d; stroke-linecap:%s; stroke-linejoin:%s; stroke-dasharray:%s; opacity:%g\" />\n",
            ctxcanvas->fgColor, ctxcanvas->canvas->line_width, ctxcanvas->linecap, ctxcanvas->linejoin, ctxcanvas->linestyle, ctxcanvas->opacity);
    break;
  case CD_FILL:
    if(ctxcanvas->canvas->fill_mode==CD_EVENODD)
      rule = "evenodd";
    else
      rule = "nonzero";

    fprintf(ctxcanvas->file, "<polygon style=\"fill:%s; fill-rule:%s; stroke:none; opacity:%g\" points=\"",
            (ctxcanvas->canvas->interior_style == CD_SOLID) ? ctxcanvas->fgColor: ctxcanvas->pattern, rule, ctxcanvas->opacity);
    sWritePointsF(ctxcanvas, poly, n, 0);
    fprintf(ctxcanvas->file, "\" />\n");
    break;
  case CD_CLIP:
    fprintf(ctxcanvas->file, "<clipPath id=\"clippoly%d\">\n", ++ctxcanvas->last_clip_poly);

    fprintf(ctxcanvas->file, "<polygon points=\"");
    sWritePointsF(ctxcanvas, poly, n, 0);
    fprintf(ctxcanvas->file, "\" />\n");

    fprintf(ctxcanvas->file, "</clipPath>\n");
    
    ctxcanvas->clip_polygon = 1;

    if (ctxcanvas->canvas->clip_mode == CD_CLIPPOLYGON)
      cdclip(ctxcanvas, CD_CLIPPOLYGON);

    break;

  }
}

static void cdpoly(cdCtxCanvas *ctxcanvas, int mode, cdPoint* poly, int n)
{
  char* rule;

  if (mode == CD_PATH)
  {
    int i, p, clip_path = 0, end_path, current_set;

    for (p=0; p<ctxcanvas->canvas->path_n; p++)
    {
      if (ctxcanvas->canvas->path[p] == CD_PATH_CLIP)
      {
        clip_path = 1;
        break;
      }
    }

    if (clip_path)
      fprintf(ctxcanvas->file, "<clipPath id=\"clippoly%d\">\n", ++ctxcanvas->last_clip_poly);

    /* starts a new path */
    fprintf(ctxcanvas->file, "<path d=\"");
    end_path = 0;
    current_set = 0;

    i = 0;
    for (p=0; p<ctxcanvas->canvas->path_n; p++)
    {
      switch(ctxcanvas->canvas->path[p])
      {
      case CD_PATH_NEW:
        if (!end_path)
          fprintf(ctxcanvas->file, "\" />\n");

        fprintf(ctxcanvas->file, "<path d=\"");
        end_path = 0;
        current_set = 0;
        break;
      case CD_PATH_MOVETO:
        if (i+1 > n) return;
        fprintf(ctxcanvas->file, "M %d %d ", poly[i].x, poly[i].y);
        current_set = 1;
        i++;
        break;
      case CD_PATH_LINETO:
        if (i+1 > n) return;
        fprintf(ctxcanvas->file, "L %d %d ", poly[i].x, poly[i].y);
        current_set = 1;
        i++;
        break;
      case CD_PATH_ARC:
        {
          int xc, yc, w, h;
          double a1, a2;
          double arcStartX, arcStartY, arcEndX, arcEndY;
          int largeArc, sweep = 0;

          if (i+3 > n) return;

          if (!cdCanvasGetArcPath(ctxcanvas->canvas, poly+i, &xc, &yc, &w, &h, &a1, &a2))
            return;

          sCalcArc(ctxcanvas->canvas, xc, yc, w, h, a1, a2, &arcStartX, &arcStartY, &arcEndX, &arcEndY, &largeArc, 0);

          if (ctxcanvas->canvas->invert_yaxis && (a2-a1)<0) /* can be clockwise */
            sweep = 1;

          if (current_set)
            fprintf(ctxcanvas->file, "L %g %g A %d %d 0 %d %d %g %g ",
                    arcStartX, arcStartY, w/2, h/2, largeArc, sweep, arcEndX, arcEndY);
          else
            fprintf(ctxcanvas->file, "M %g %g A %d %d 0 %d %d %g %g ",
                    arcStartX, arcStartY, w/2, h/2, largeArc, sweep, arcEndX, arcEndY);

          current_set = 1;
          i += 3;
        }
        break;
      case CD_PATH_CURVETO:
        if (i+3 > n) return;
        fprintf(ctxcanvas->file, "C %d %d %d %d %d %d ", poly[i].x,   poly[i].y, 
                                                         poly[i+1].x, poly[i+1].y, 
                                                         poly[i+2].x, poly[i+2].y);
        current_set = 1;
        i += 3;
        break;
      case CD_PATH_CLOSE:
        fprintf(ctxcanvas->file, "Z ");
        break;
      case CD_PATH_FILL:
        rule = (ctxcanvas->canvas->fill_mode==CD_EVENODD)? "evenodd": "nonzero";
        fprintf(ctxcanvas->file, "\" style=\"fill:%s; fill-rule:%s; stroke:none; opacity:%g\" />\n",
                (ctxcanvas->canvas->interior_style == CD_SOLID) ? ctxcanvas->fgColor: ctxcanvas->pattern, rule, ctxcanvas->opacity);
        end_path = 1;
        break;
      case CD_PATH_STROKE:
        fprintf(ctxcanvas->file, "\" style=\"fill:none; stroke:%s; stroke-width:%d; stroke-linecap:%s; stroke-linejoin:%s; stroke-dasharray:%s; opacity:%g\" />\n",
                ctxcanvas->fgColor, ctxcanvas->canvas->line_width, ctxcanvas->linecap, ctxcanvas->linejoin, ctxcanvas->linestyle, ctxcanvas->opacity);
        end_path = 1;
        break;
      case CD_PATH_FILLSTROKE:
        rule = (ctxcanvas->canvas->fill_mode==CD_EVENODD)? "evenodd": "nonzero";
        fprintf(ctxcanvas->file, "\" style=\"fill:%s; fill-rule:%s; stroke:%s; stroke-width:%d; stroke-linecap:%s; stroke-linejoin:%s; stroke-dasharray:%s; opacity:%g\" />\n",
                (ctxcanvas->canvas->interior_style == CD_SOLID) ? ctxcanvas->fgColor: ctxcanvas->pattern, rule, ctxcanvas->fgColor, ctxcanvas->canvas->line_width, ctxcanvas->linecap, ctxcanvas->linejoin, ctxcanvas->linestyle, ctxcanvas->opacity);
        end_path = 1;
        break;
      case CD_PATH_CLIP:
        fprintf(ctxcanvas->file, "\" />\n");
        fprintf(ctxcanvas->file, "</clipPath>\n");
        ctxcanvas->clip_polygon = 1;
        cdclip(ctxcanvas, CD_CLIPPOLYGON);
        end_path = 1;
        break;
      }
    }
    return;
  }

  switch (mode)
  {
  case CD_CLOSED_LINES:
    fprintf(ctxcanvas->file, "<polygon style=\"fill:none; stroke:%s; stroke-width:%d; stroke-linecap:%s; stroke-linejoin:%s; stroke-dasharray:%s; opacity:%g\" points=\"",
            ctxcanvas->fgColor, ctxcanvas->canvas->line_width, ctxcanvas->linecap, ctxcanvas->linejoin, ctxcanvas->linestyle, ctxcanvas->opacity);
    sWritePoints(ctxcanvas, poly, n, 1);
    fprintf(ctxcanvas->file, "\" />\n");
    break;
  case CD_OPEN_LINES:
    fprintf(ctxcanvas->file, "<polyline style=\"fill:none; stroke:%s; stroke-width:%d; stroke-linecap:%s; stroke-linejoin:%s; stroke-dasharray:%s; opacity:%g\" points=\"",
            ctxcanvas->fgColor, ctxcanvas->canvas->line_width, ctxcanvas->linecap, ctxcanvas->linejoin, ctxcanvas->linestyle, ctxcanvas->opacity);
    sWritePoints(ctxcanvas, poly, n, 0);
    fprintf(ctxcanvas->file, "\" />\n");
    break;
  case CD_BEZIER:
    fprintf(ctxcanvas->file, "<path d=\"M%d,%d C", poly[0].x, poly[0].y);
    sWritePoints(ctxcanvas, poly+1, n-1, 0);
    fprintf(ctxcanvas->file, "\" style=\"fill:none; stroke:%s; stroke-width:%d; stroke-linecap:%s; stroke-linejoin:%s; stroke-dasharray:%s; opacity:%g\" />\n",
            ctxcanvas->fgColor, ctxcanvas->canvas->line_width, ctxcanvas->linecap, ctxcanvas->linejoin, ctxcanvas->linestyle, ctxcanvas->opacity);
    break;
  case CD_FILL:
    if(ctxcanvas->canvas->fill_mode==CD_EVENODD)
      rule = "evenodd";
    else
      rule = "nonzero";

    fprintf(ctxcanvas->file, "<polygon style=\"fill:%s; fill-rule:%s; stroke:none; opacity:%g\" points=\"",
            (ctxcanvas->canvas->interior_style == CD_SOLID) ? ctxcanvas->fgColor: ctxcanvas->pattern, rule, ctxcanvas->opacity);
    sWritePoints(ctxcanvas, poly, n, 0);
    fprintf(ctxcanvas->file, "\" />\n");
    break;
  case CD_CLIP:
    fprintf(ctxcanvas->file, "<clipPath id=\"clippoly%d\">\n", ++ctxcanvas->last_clip_poly);

    fprintf(ctxcanvas->file, "<polygon points=\"");
    sWritePoints(ctxcanvas, poly, n, 0);
    fprintf(ctxcanvas->file, "\" />\n");

    fprintf(ctxcanvas->file, "</clipPath>\n");
    
    ctxcanvas->clip_polygon = 1;

    if (ctxcanvas->canvas->clip_mode == CD_CLIPPOLYGON)
      cdclip(ctxcanvas, CD_CLIPPOLYGON);

    break;

  }
}

static int cdlinestyle(cdCtxCanvas *ctxcanvas, int style)
{
  switch (style)
  {
    case CD_CONTINUOUS: /* empty dash */
    default:
      sprintf(ctxcanvas->linestyle, "%s", "0");
      break;
    case CD_DASHED:
      sprintf(ctxcanvas->linestyle, "%s", "6,2");
      break;
    case CD_DOTTED:
      sprintf(ctxcanvas->linestyle, "%s", "2,2");
      break;
    case CD_DASH_DOT:
      sprintf(ctxcanvas->linestyle, "%s", "6,2,2,2");
      break;
    case CD_DASH_DOT_DOT:
      sprintf(ctxcanvas->linestyle, "%s", "6,2,2,2,2,2");
      break;
    case CD_CUSTOM:
      {
        int i;
        sprintf(ctxcanvas->linestyle, "%d", ctxcanvas->canvas->line_dashes[0]);

        for (i = 1; i < ctxcanvas->canvas->line_dashes_count; i++)
          sprintf(ctxcanvas->linestyle, "%s, %d", ctxcanvas->linestyle, ctxcanvas->canvas->line_dashes[i]);
      }
      break;
  }

  return style;
}

static int cdlinecap(cdCtxCanvas *ctxcanvas, int cap)
{
  if(cap == CD_CAPROUND)
    ctxcanvas->linecap = "round";
  else if(cap == CD_CAPSQUARE)
    ctxcanvas->linecap = "square";
  else  /* CD_CAPFLAT */
    ctxcanvas->linecap = "butt";
  return cap;
}

static int cdlinejoin(cdCtxCanvas *ctxcanvas, int join)
{
  if(join == CD_ROUND)
    ctxcanvas->linejoin = "round";
  else if(join == CD_BEVEL)
    ctxcanvas->linejoin = "bevel";
  else  /* CD_MITER */
    ctxcanvas->linejoin = "miter";
  return join;
}

static int cdhatch(cdCtxCanvas *ctxcanvas, int style)
{
  int hsize = ctxcanvas->hatchboxsize - 1;
  int hhalf = hsize / 2;

  sprintf(ctxcanvas->pattern, "url(#pattern%d)", ++ctxcanvas->last_fill_mode);
  fprintf(ctxcanvas->file, "<pattern id=\"pattern%d\" patternUnits=\"userSpaceOnUse\" x=\"0\" y=\"0\" width=\"%d\" height=\"%d\">\n", ctxcanvas->last_fill_mode, hsize, hsize);

  if (ctxcanvas->canvas->back_opacity==CD_OPAQUE)
  {
    fprintf(ctxcanvas->file, "<rect x=\"0\" y=\"0\" width=\"%d\" height=\"%d\" style=\"fill:%s; stroke:none; opacity:%g\" />\n",
            hsize, hsize, ctxcanvas->bgColor, ctxcanvas->opacity);
  }

  switch(style)
  {
  case CD_HORIZONTAL:
    fprintf(ctxcanvas->file, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" style=\"fill:none; stroke:%s; opacity:%g\" />\n",
      0, hhalf, hsize, hhalf, ctxcanvas->fgColor, ctxcanvas->opacity);
    break;
  case CD_VERTICAL:
    fprintf(ctxcanvas->file, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" style=\"fill:none; stroke:%s; opacity:%g\" />\n",
      hhalf, 0, hhalf, hsize, ctxcanvas->fgColor, ctxcanvas->opacity);
    break;
  case CD_BDIAGONAL:
    fprintf(ctxcanvas->file, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" style=\"fill:none; stroke:%s; opacity:%g\" />\n",
      0, hsize, hsize, 0, ctxcanvas->fgColor, ctxcanvas->opacity);
    break;
  case CD_FDIAGONAL:
    fprintf(ctxcanvas->file, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" style=\"fill:none; stroke:%s; opacity:%g\" />\n",
      0, 0, hsize, hsize, ctxcanvas->fgColor, ctxcanvas->opacity);
    break;
  case CD_CROSS:
    fprintf(ctxcanvas->file, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" style=\"fill:none; stroke:%s; opacity:%g\" />\n",
      hsize, 0, hsize, hsize, ctxcanvas->fgColor, ctxcanvas->opacity);
    fprintf(ctxcanvas->file, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" style=\"fill:none; stroke:%s; opacity:%g\" />\n",
      0, hhalf, hsize, hhalf, ctxcanvas->fgColor, ctxcanvas->opacity);
    break;
  case CD_DIAGCROSS:
    fprintf(ctxcanvas->file, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" style=\"fill:none; stroke:%s; opacity:%g\" />\n",
      0, 0, hsize, hsize, ctxcanvas->fgColor, ctxcanvas->opacity);
    fprintf(ctxcanvas->file, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" style=\"fill:none; stroke:%s; opacity:%g\" />\n",
      hsize, 0, 0, hsize, ctxcanvas->fgColor, ctxcanvas->opacity);
    break;
  }

  fprintf(ctxcanvas->file, "</pattern>\n");

  return style;
}

static void make_pattern(cdCtxCanvas *ctxcanvas, int n, int m, void* data, int (*data2rgb)(cdCtxCanvas *ctxcanvas, int n, int i, int j, void* data, unsigned char*r, unsigned char*g, unsigned char*b))
{
  int i, j, ret;
  unsigned char r, g, b;
  char color[20];

  sprintf(ctxcanvas->pattern, "url(#pattern%d)", ++ctxcanvas->last_fill_mode);
  fprintf(ctxcanvas->file, "<pattern id=\"pattern%d\" patternUnits=\"userSpaceOnUse\" x=\"0\" y=\"0\" width=\"%d\" height=\"%d\">\n", ctxcanvas->last_fill_mode, n, m);

  for (j = 0; j < m; j++)
  {
    for (i = 0; i < n; i++)
    {
      /* internal transform, affects also pattern orientation */
      if (ctxcanvas->canvas->invert_yaxis)
        ret = data2rgb(ctxcanvas, n, i, m-1 - j, data, &r, &g, &b);
      else
        ret = data2rgb(ctxcanvas, n, i, j, data, &r, &g, &b);

      if (ret == -1) continue;

      sprintf(color, "rgb(%d,%d,%d)", (int)r, (int)g, (int)b);

      fprintf(ctxcanvas->file, "<rect x=\"%g\" y=\"%g\" width=\"%g\" height=\"%g\" style=\"fill:%s; opacity:%g\" />\n",
        (double)i, (double)j, 1.0, 1.0, color, ctxcanvas->opacity);
    }
  }

  fprintf(ctxcanvas->file, "</pattern>\n");
}

static int long2rgb(cdCtxCanvas *ctxcanvas, int n, int i, int j, void* data, unsigned char*r, unsigned char*g, unsigned char*b)
{
  long* long_data = (long*)data;
  (void)ctxcanvas;
  cdDecodeColor(long_data[j*n+i], r, g, b);
  return 1;
}

static void cdpattern(cdCtxCanvas *ctxcanvas, int n, int m, const long int *pattern)
{
  make_pattern(ctxcanvas, n, m, (void*)pattern, long2rgb);
}

static int uchar2rgb(cdCtxCanvas *ctxcanvas, int n, int i, int j, void* data, unsigned char*r, unsigned char*g, unsigned char*b)
{
  unsigned char* uchar_data = (unsigned char*)data;
  if (uchar_data[j*n+i])
    cdDecodeColor(ctxcanvas->canvas->foreground, r, g, b);
  else
  {
    if (ctxcanvas->canvas->back_opacity==CD_TRANSPARENT)
      return -1;
    else
      cdDecodeColor(ctxcanvas->canvas->background, r, g, b);
  }

  return 1;
}

static void cdstipple(cdCtxCanvas *ctxcanvas, int n, int m, const unsigned char *stipple)
{
  make_pattern(ctxcanvas, n, m, (void*)stipple, uchar2rgb);
}

static int cdfont(cdCtxCanvas *ctxcanvas, const char* type_face, int style, int size)
{
  /* Define type_face and size */
  if (type_face != NULL)
    sprintf(ctxcanvas->font_family, "%s", type_face);
  
  if (size > 0)
    sprintf(ctxcanvas->font_size, "%dpt", size);
  else
    sprintf(ctxcanvas->font_size, "%dpx", (-1)*size);

  if (style != -1)
  {
    /* Define styles and decorations */
    if (style & CD_BOLD)
      ctxcanvas->font_weight = "bold";
    else
      ctxcanvas->font_weight = "normal";

    if (style & CD_ITALIC)
      ctxcanvas->font_style = "italic";
    else
      ctxcanvas->font_style = "normal";

    if (style & CD_STRIKEOUT || style & CD_UNDERLINE)
    {
      if (style & CD_STRIKEOUT && style & CD_UNDERLINE)
        ctxcanvas->font_decoration = "line-through underline";
      else
      {
        if (style & CD_STRIKEOUT)
          ctxcanvas->font_decoration = "line-through";

        if (style & CD_UNDERLINE)
          ctxcanvas->font_decoration = "underline";
      }
    }
    else
      ctxcanvas->font_decoration = "none";
  }

  return 1;
}

static long cdbackground(cdCtxCanvas *ctxcanvas, long int color)
{
  unsigned char r, g, b;
  cdDecodeColor(color, &r, &g, &b);
  sprintf(ctxcanvas->bgColor, "rgb(%d,%d,%d)", (int)r, (int)g, (int)b);
  return color;
}

static long cdforeground(cdCtxCanvas *ctxcanvas, long int color)
{
  unsigned char r, g, b;
  cdDecodeColor(color, &r, &g, &b);
  sprintf(ctxcanvas->fgColor, "rgb(%d,%d,%d)", (int)r, (int)g, (int)b);
  return color;
}

static void cdputimagerectrgb(cdCtxCanvas *ctxcanvas, int iw, int ih, const unsigned char *r, const unsigned char *g, const unsigned char *b, int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax)
{
  int i, j, d, rw, rh, rgb_size, target_size;
  unsigned char* rgb_data, *rgb_buffer;
  size_t buffer_size;
  LodePNG_Encoder encoder;
  char* rgb_target;

  if (xmin<0 || ymin<0 || xmax-xmin+1>iw || ymax-ymin+1>ih) return;

  rw = xmax-xmin+1;
  rh = ymax-ymin+1;

  rgb_size = 4*rw*rh;
  rgb_data = (unsigned char*)malloc(rgb_size);
  if (!rgb_data) return;

  d = 0;
  for (i=ymax; i>=ymin; i--)
  {
    for (j=xmin; j<=xmax; j++)
    {
      rgb_data[d] = r[i*iw+j]; d++;
      rgb_data[d] = g[i*iw+j]; d++;
      rgb_data[d] = b[i*iw+j]; d++;
      rgb_data[d] = (unsigned char)0xFF; d++;
    }
  }

  LodePNG_Encoder_init(&encoder);
  LodePNG_encode(&encoder, &rgb_buffer, &buffer_size, rgb_data, rw, rh);

  target_size = (buffer_size+2)/3*4+1;
  rgb_target = (char*)malloc(target_size);
  base64_encode(rgb_buffer, buffer_size, rgb_target, target_size);
 
  if (ctxcanvas->canvas->use_matrix)  /* Transformation active */
    fprintf(ctxcanvas->file, "<image transform=\"matrix(%d %d %d %d %d %d)\" width=\"%d\" height=\"%d\" xlink:href=\"data:image/png;base64,%s\"/>\n", 
            1, 0, 0, -1, x, y+h, w, h, rgb_target);
  else
    fprintf(ctxcanvas->file, "<image transform=\"matrix(%d %d %d %d %d %d)\" width=\"%d\" height=\"%d\" xlink:href=\"data:image/png;base64,%s\"/>\n", 
            1, 0, 0, 1, x, y-h, w, h, rgb_target);

  free(rgb_data);
  free(rgb_buffer);
  free(rgb_target);
  LodePNG_Encoder_cleanup(&encoder);
}

static void cdputimagerectrgba(cdCtxCanvas *ctxcanvas, int iw, int ih, const unsigned char *r, const unsigned char *g, const unsigned char *b, const unsigned char *a, int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax)
{
  int i, j, d, rw, rh, rgb_size, target_size;
  size_t buffer_size;
  unsigned char* rgb_data, *rgb_buffer;
  LodePNG_Encoder encoder;
  char* rgb_target;

  if (xmin<0 || ymin<0 || xmax-xmin+1>iw || ymax-ymin+1>ih) return;

  rw = xmax-xmin+1;
  rh = ymax-ymin+1;

  rgb_size = 4*rw*rh;
  rgb_data = (unsigned char*)malloc(rgb_size);
  if (!rgb_data) return;

  d = 0;
  for (i=ymax; i>=ymin; i--)
  {
    for (j=xmin; j<=xmax; j++)
    {
      rgb_data[d] = r[i*iw+j]; d++;
      rgb_data[d] = g[i*iw+j]; d++;
      rgb_data[d] = b[i*iw+j]; d++;
      rgb_data[d] = a[i*iw+j]; d++;
    }
  }

  LodePNG_Encoder_init(&encoder);
  LodePNG_encode(&encoder, &rgb_buffer, &buffer_size, rgb_data, rw, rh);

  target_size = (buffer_size+2)/3*4+1;
  rgb_target = (char*)malloc(target_size);
  base64_encode(rgb_buffer, buffer_size, rgb_target, target_size);

  if (ctxcanvas->canvas->use_matrix)  /* Transformation active */
    fprintf(ctxcanvas->file, "<image transform=\"matrix(%d %d %d %d %d %d)\" width=\"%d\" height=\"%d\" xlink:href=\"data:image/png;base64,%s\"/>\n", 
            1, 0, 0, -1, x, y+h, w, h, rgb_target);
  else
    fprintf(ctxcanvas->file, "<image transform=\"matrix(%d %d %d %d %d %d)\" width=\"%d\" height=\"%d\" xlink:href=\"data:image/png;base64,%s\"/>\n", 
            1, 0, 0, 1, x, y-h, w, h, rgb_target);

  free(rgb_data);
  free(rgb_buffer);
  free(rgb_target);
  LodePNG_Encoder_cleanup(&encoder);
}

static void cdputimagerectmap(cdCtxCanvas *ctxcanvas, int iw, int ih, const unsigned char *index, const long int *colors, int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax)
{
  int i, j, d, rw, rh, rgb_size, target_size;
  unsigned char* rgb_data, *rgb_buffer;
  size_t buffer_size;
  LodePNG_Encoder encoder;
  char* rgb_target;

  if (xmin<0 || ymin<0 || xmax-xmin+1>iw || ymax-ymin+1>ih) return;

  rw = xmax-xmin+1;
  rh = ymax-ymin+1;

  rgb_size = 4*rw*rh;
  rgb_data = (unsigned char*)malloc(rgb_size);
  if (!rgb_data) return;

  d = 0;
  for (i=ymax; i>=ymin; i--)
  {
    for (j=xmin; j<=xmax; j++)
    {
      unsigned char r, g, b;
      cdDecodeColor(colors[index[i*iw+j]], &r, &g, &b);
      rgb_data[d] = r; d++;
      rgb_data[d] = g; d++;
      rgb_data[d] = b; d++;
      rgb_data[d] = (unsigned char)0xFF; d++;
    }
  }

  LodePNG_Encoder_init(&encoder);
  LodePNG_encode(&encoder, &rgb_buffer, &buffer_size, rgb_data, rw, rh);

  target_size = (buffer_size+2)/3*4+1;
  rgb_target = (char*)malloc(target_size);
  base64_encode(rgb_buffer, buffer_size, rgb_target, target_size);

  if (ctxcanvas->canvas->use_matrix)  /* Transformation active */
    fprintf(ctxcanvas->file, "<image transform=\"matrix(%d %d %d %d %d %d)\" width=\"%d\" height=\"%d\" xlink:href=\"data:image/png;base64,%s\"/>\n", 
            1, 0, 0, -1, x, y+h, w, h, rgb_target);
  else
    fprintf(ctxcanvas->file, "<image transform=\"matrix(%d %d %d %d %d %d)\" width=\"%d\" height=\"%d\" xlink:href=\"data:image/png;base64,%s\"/>\n", 
            1, 0, 0, 1, x, y-h, w, h, rgb_target);

  free(rgb_data);
  free(rgb_buffer);
  free(rgb_target);
  LodePNG_Encoder_cleanup(&encoder);
}

static void cdpixel(cdCtxCanvas *ctxcanvas, int x, int y, long int color)
{
  unsigned char r, g, b;
  cdDecodeColor(color, &r, &g, &b);

  fprintf(ctxcanvas->file, "<circle cx=\"%d\" cy=\"%d\" r=\"0.5\" style=\"fill:rgb(%d,%d,%d); stroke:none; opacity:%g\" />\n",
          x, y, r, g, b, ctxcanvas->opacity);
}

static void cddeactivate (cdCtxCanvas* ctxcanvas)
{
  fflush(ctxcanvas->file);
}

static void cdflush (cdCtxCanvas* ctxcanvas)
{
  fflush(ctxcanvas->file);
}

static void set_hatchboxsize_attrib(cdCtxCanvas *ctxcanvas, char* data)
{
  int hatchboxsize;

  if (data == NULL)
  {
    ctxcanvas->hatchboxsize = 8;
    return;
  }

  sscanf(data, "%d", &hatchboxsize);
  ctxcanvas->hatchboxsize = hatchboxsize;
}

static char* get_hatchboxsize_attrib(cdCtxCanvas *ctxcanvas)
{
  static char size[10];
  sprintf(size, "%d", ctxcanvas->hatchboxsize);
  return size;
}

static cdAttribute hatchboxsize_attrib =
{
  "HATCHBOXSIZE",
  set_hatchboxsize_attrib,
  get_hatchboxsize_attrib
}; 

static void set_opacity_attrib(cdCtxCanvas *ctxcanvas, char* data)
{
  if (data)
  {
    int opacity = 255;
    sscanf(data, "%d", &opacity);
    if (opacity < 0) ctxcanvas->opacity = 0.0;
    else if (opacity > 255) ctxcanvas->opacity = 1.0;
    else ctxcanvas->opacity = (double)opacity/255.0;
  }
  else
    ctxcanvas->opacity = 1.0;
}

static char* get_opacity_attrib(cdCtxCanvas *ctxcanvas)
{
  static char data[50];
  sprintf(data, "%d", cdRound(ctxcanvas->opacity*255.0));
  return data;
}

static cdAttribute opacity_attrib =
{
  "OPACITY",
  set_opacity_attrib,
  get_opacity_attrib
}; 

static void set_cmd_attrib(cdCtxCanvas *ctxcanvas, char* data)
{
  fprintf(ctxcanvas->file, "%s", data);
}

static cdAttribute cmd_attrib =
{
  "CMD",
  set_cmd_attrib,
  NULL
}; 

static void cdcreatecanvas(cdCanvas *canvas, void *data)
{
  char filename[10240] = "";
  char* strdata = (char*)data;
  double w_mm = INT_MAX*3.78, h_mm = INT_MAX*3.78, res = 3.78;
  cdCtxCanvas* ctxcanvas;

  strdata += cdGetFileName(strdata, filename);
  if (filename[0] == 0)
    return;

  sscanf(strdata, "%lgx%lg %lg", &w_mm, &h_mm, &res);

  ctxcanvas = (cdCtxCanvas *)malloc(sizeof(cdCtxCanvas));
  memset(ctxcanvas, 0, sizeof(cdCtxCanvas));

  /* SVN specification states that number must use dot as decimal separator */
  ctxcanvas->old_locale = cdStrDup(setlocale(LC_NUMERIC, NULL));
  setlocale(LC_NUMERIC, "C");

  ctxcanvas->file = fopen(filename, "w");
  if (!ctxcanvas->file)
  {
    free(ctxcanvas);
    return;
  }

  /* store the base canvas */
  ctxcanvas->canvas = canvas;

  /* update canvas context */
  canvas->w = (int)(w_mm * res);
  canvas->h = (int)(h_mm * res);
  canvas->w_mm = w_mm;
  canvas->h_mm = h_mm;
  canvas->bpp = 24;
  canvas->xres = res;
  canvas->yres = res;
  canvas->invert_yaxis = 1;

  /* update canvas context */
  canvas->ctxcanvas = ctxcanvas;

  ctxcanvas->last_fill_mode = -1;
  ctxcanvas->last_clip_poly = -1;
  ctxcanvas->last_clip_rect = -1;
  
  ctxcanvas->clip_control  = 0;
  ctxcanvas->transform_control = 0;

  ctxcanvas->clip_polygon = 0;
  ctxcanvas->hatchboxsize = 8;
  ctxcanvas->opacity = 1.0;

  cdRegisterAttribute(canvas, &cmd_attrib);
  cdRegisterAttribute(canvas, &hatchboxsize_attrib);
  cdRegisterAttribute(canvas, &opacity_attrib);

  fprintf(ctxcanvas->file, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
  fprintf(ctxcanvas->file, "<svg xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" width=\"%gpt\" height=\"%gpt\" viewBox=\"0 0 %d %d\" version=\"1.1\">\n", CD_MM2PT*canvas->w_mm, CD_MM2PT*canvas->h_mm, canvas->w, canvas->h);
  fprintf(ctxcanvas->file, "<g>\n"); /* open global container */
}

static void cdinittable(cdCanvas* canvas)
{
  canvas->cxPixel = cdpixel;
  canvas->cxLine = cdline;
  canvas->cxPoly = cdpoly;
  canvas->cxRect = cdrect;
  canvas->cxBox = cdbox;
  canvas->cxArc = cdarc;
  canvas->cxSector = cdsector;
  canvas->cxChord = cdchord;
  canvas->cxText = cdtext;
  canvas->cxClip = cdclip;
  canvas->cxClipArea = cdcliparea;
  canvas->cxFont = cdfont;
  
  canvas->cxPutImageRectRGB = cdputimagerectrgb;
  canvas->cxPutImageRectRGBA = cdputimagerectrgba;
  canvas->cxPutImageRectMap = cdputimagerectmap;
  
  canvas->cxFLine = cdfline;
  canvas->cxFPoly = cdfpoly;
  canvas->cxFRect = cdfrect;
  canvas->cxFBox = cdfbox;
  canvas->cxFArc = cdfarc;
  canvas->cxFSector = cdfsector;
  canvas->cxFChord = cdfchord;
  canvas->cxFText = cdftext;
  canvas->cxFClipArea = cdfcliparea;
  
  canvas->cxLineStyle = cdlinestyle;
  canvas->cxLineCap = cdlinecap;
  canvas->cxLineJoin = cdlinejoin;

  canvas->cxHatch = cdhatch;
  canvas->cxStipple = cdstipple;
  canvas->cxPattern = cdpattern;

  canvas->cxBackground = cdbackground;
  canvas->cxForeground = cdforeground;

  canvas->cxTransform = cdtransform;

  canvas->cxFlush = cdflush;
  canvas->cxDeactivate = cddeactivate;
  canvas->cxKillCanvas = cdkillcanvas;
}

static cdContext cdSVGContext =
{
  CD_CAP_ALL & ~(CD_CAP_CLEAR | CD_CAP_PLAY | CD_CAP_PALETTE | 
                 CD_CAP_REGION | CD_CAP_IMAGESRV | CD_CAP_WRITEMODE | 
                 CD_CAP_FONTDIM | CD_CAP_TEXTSIZE | 
                 CD_CAP_GETIMAGERGB),
  0,
  cdcreatecanvas,
  cdinittable,
  NULL,
  NULL,
};

cdContext* cdContextSVG(void)
{
  return &cdSVGContext;
}
