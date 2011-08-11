/** \file
 * \brief PDF Driver
 *
 * See Copyright Notice in cd.h
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "cd.h"
#include "cd_private.h"
#include "cdpdf.h"

#include "pdflib.h"


/*
** dada uma cor do CD, obtem uma de suas componentes, na faixa 0-1.
*/
#define get_red(_)   (((double)cdRed(_))/255.)
#define get_green(_) (((double)cdGreen(_))/255.)
#define get_blue(_)  (((double)cdBlue(_))/255.)

struct _cdCtxCanvas 
{
  cdCanvas* canvas;

  PDF *pdf;              /* Arquivo PDF */
  int res;               /* Resolucao - DPI */
  int pages;             /* Numero total de paginas */
  double width_pt;       /* Largura do papel (points) */ 
  double height_pt;      /* Altura do papel (points) */  
  double width_mm;       /* Largura do papel (mm) */ 
  double height_mm;      /* Altura do papel (mm) */  
  double scale;          /* Fator de conversao de coordenadas (pixel2points) */
  int landscape;         /* page orientation */
  float  rotate_angle;
  int    rotate_center_x,
         rotate_center_y;

  int font;
  int underline;
  int strikeover;

  int hatchboxsize;
  int pattern;
  int opacity;
  int opacity_states[256];

  int poly_holes[500];
  int holes;
};


/*
%F Registra os valores default para impressao.
*/
static void setpdfdefaultvalues(cdCtxCanvas* ctxcanvas)
{
  int i;

  /* all the other values are set to 0 */
  cdSetPaperSize(CD_A4, &ctxcanvas->width_pt, &ctxcanvas->height_pt);
  ctxcanvas->width_mm = ctxcanvas->width_pt/CD_MM2PT;
  ctxcanvas->height_mm = ctxcanvas->height_pt/CD_MM2PT;
  ctxcanvas->res = 300;
  ctxcanvas->hatchboxsize = 8;
  ctxcanvas->opacity = 255; /* full opaque */

  for (i=0; i<256; i++)
    ctxcanvas->opacity_states[i] = -1;
}

static void update_state(cdCtxCanvas *ctxcanvas)
{
  cdCanvas* canvas = ctxcanvas->canvas;

  if (!canvas->cxFont)  /* just check if the first time */
    return;

  /* must set the current transform and line style if different from the default */

  if (canvas->line_style != CD_CONTINUOUS)
    canvas->cxLineStyle(ctxcanvas, canvas->line_style);
  if (canvas->line_width != 1)
    canvas->cxLineWidth(ctxcanvas, canvas->line_width);
  if (canvas->line_cap != CD_CAPFLAT)
    canvas->cxLineCap(ctxcanvas, canvas->line_cap);
  if (canvas->line_join != CD_MITER)
    canvas->cxLineJoin(ctxcanvas, canvas->line_join);
  if (canvas->use_matrix)
    canvas->cxTransform(ctxcanvas, canvas->matrix);
  canvas->cxFont(ctxcanvas, canvas->font_type_face, canvas->font_style, canvas->font_size);
}

static void begin_page(cdCtxCanvas *ctxcanvas)
{
  PDF_begin_page_ext(ctxcanvas->pdf, ctxcanvas->width_pt, ctxcanvas->height_pt, "");

  /* default coordinate system is in points, change it to pixels. */
  PDF_scale(ctxcanvas->pdf, ctxcanvas->scale, ctxcanvas->scale); 

  PDF_save(ctxcanvas->pdf);  /* save the initial configuration, to be used when clipping is reset. */

  update_state(ctxcanvas);
}

static void init_pdf(cdCtxCanvas *ctxcanvas)
{
  ctxcanvas->scale = 72.0/ctxcanvas->res;
  ctxcanvas->canvas->xres = ctxcanvas->res/25.4;
  ctxcanvas->canvas->yres = ctxcanvas->canvas->xres;

  ctxcanvas->canvas->w_mm = ctxcanvas->width_mm; 
  ctxcanvas->canvas->h_mm = ctxcanvas->height_mm;

  ctxcanvas->canvas->w = cdRound(ctxcanvas->canvas->xres*ctxcanvas->canvas->w_mm);
  ctxcanvas->canvas->h = cdRound(ctxcanvas->canvas->yres*ctxcanvas->canvas->h_mm);

  ctxcanvas->canvas->bpp = 24;

  begin_page(ctxcanvas);
}

static void cdkillcanvas(cdCtxCanvas *ctxcanvas)
{
  PDF_restore(ctxcanvas->pdf);  /* restore to match the save of the initial configuration. */
  PDF_end_page_ext(ctxcanvas->pdf, "");
  PDF_end_document(ctxcanvas->pdf, "");
  PDF_delete(ctxcanvas->pdf);

  memset(ctxcanvas, 0, sizeof(cdCtxCanvas));
  free(ctxcanvas);
}

static void sUpdateFill(cdCtxCanvas *ctxcanvas, int fill)
{
  if (fill == 0)
  {
    /* called before a NON filled primitive */
    PDF_setcolor(ctxcanvas->pdf, "stroke", "rgb", get_red(ctxcanvas->canvas->foreground), 
                                                  get_green(ctxcanvas->canvas->foreground), 
                                                  get_blue(ctxcanvas->canvas->foreground), 0);

  }
  else
  {
    /* called before a filled primitive */
    if (ctxcanvas->canvas->interior_style == CD_SOLID)
    {
      PDF_setcolor(ctxcanvas->pdf, "fill", "rgb", get_red(ctxcanvas->canvas->foreground), 
                                                  get_green(ctxcanvas->canvas->foreground), 
                                                  get_blue(ctxcanvas->canvas->foreground), 0);
    }
    else
      PDF_setcolor(ctxcanvas->pdf, "fill", "pattern", (float)ctxcanvas->pattern, 0, 0, 0);
  }
}

/*
%F Comeca uma nova pagina.
*/
static void cdflush(cdCtxCanvas *ctxcanvas)
{
  PDF_restore(ctxcanvas->pdf);  /* restore to match the save of the initial configuration */

  PDF_end_page_ext(ctxcanvas->pdf, "");

  begin_page(ctxcanvas);
}


/******************************************************/
/* coordinate transformation                          */
/******************************************************/

static void resetcliprect(cdCtxCanvas* ctxcanvas)
{
  /* clipping is reset, by restoring the initial state */
  /* this will also reset the current transformation and line style */
  PDF_restore(ctxcanvas->pdf);
  PDF_save(ctxcanvas->pdf);

  update_state(ctxcanvas);
}

static void setcliprect(cdCtxCanvas* ctxcanvas, double xmin, double ymin, double xmax, double ymax)
{
  resetcliprect(ctxcanvas);

  PDF_moveto(ctxcanvas->pdf, xmin, ymin);
  PDF_lineto(ctxcanvas->pdf, xmax, ymin);
  PDF_lineto(ctxcanvas->pdf, xmax, ymax);
  PDF_lineto(ctxcanvas->pdf, xmin, ymax);
  
  PDF_clip(ctxcanvas->pdf);
}

static void cdfcliparea(cdCtxCanvas *ctxcanvas, double xmin, double xmax, double ymin, double ymax)
{
  if (ctxcanvas->canvas->clip_mode != CD_CLIPAREA)
    return;

  setcliprect(ctxcanvas, xmin, ymin, xmax, ymax);
}

static void cdcliparea(cdCtxCanvas *ctxcanvas, int xmin, int xmax, int ymin, int ymax)
{
  cdfcliparea(ctxcanvas, (double)xmin, (double)xmax, (double)ymin, (double)ymax);
}

static int cdclip(cdCtxCanvas *ctxcanvas, int mode)
{
  if (mode == CD_CLIPAREA)
  {
    ctxcanvas->canvas->clip_mode = CD_CLIPAREA;

    setcliprect(ctxcanvas, (double)ctxcanvas->canvas->clip_rect.xmin, 
                           (double)ctxcanvas->canvas->clip_rect.ymin, 
                           (double)ctxcanvas->canvas->clip_rect.xmax, 
                           (double)ctxcanvas->canvas->clip_rect.ymax);
  }
  else if (mode == CD_CLIPPOLYGON)
  {
    int hole_index = 0;
    int i;

    resetcliprect(ctxcanvas);

    if (ctxcanvas->canvas->clip_poly)
    {
      cdPoint *poly = ctxcanvas->canvas->clip_poly; 

      PDF_moveto(ctxcanvas->pdf, poly[0].x, poly[0].y);

      for (i=1; i<ctxcanvas->canvas->clip_poly_n; i++)
      {
        if (ctxcanvas->holes && i == ctxcanvas->poly_holes[hole_index])
        {
          PDF_moveto(ctxcanvas->pdf, poly[i].x, poly[i].y);
          hole_index++;
        }
        else
          PDF_lineto(ctxcanvas->pdf, poly[i].x, poly[i].y);
      }
    }
    else if (ctxcanvas->canvas->clip_fpoly)
    {
      cdfPoint *poly = ctxcanvas->canvas->clip_fpoly; 

      PDF_moveto(ctxcanvas->pdf, poly[0].x, poly[0].y);

      for (i=1; i<ctxcanvas->canvas->clip_poly_n; i++)
      {
        if (ctxcanvas->holes && i == ctxcanvas->poly_holes[hole_index])
        {
          PDF_moveto(ctxcanvas->pdf, poly[i].x, poly[i].y);
          hole_index++;
        }
        else
          PDF_lineto(ctxcanvas->pdf, poly[i].x, poly[i].y);
      }
    }
    
    PDF_clip(ctxcanvas->pdf);
  }
  else if (mode == CD_CLIPOFF)
  {
    resetcliprect(ctxcanvas);
  }

  return mode;
}

/******************************************************/
/* primitives                                         */
/******************************************************/

static void cdfline(cdCtxCanvas *ctxcanvas, double x1, double y1, double x2, double y2)
{
  sUpdateFill(ctxcanvas, 0);

  PDF_moveto(ctxcanvas->pdf, x1, y1);
  PDF_lineto(ctxcanvas->pdf, x2, y2);
  PDF_stroke(ctxcanvas->pdf);
}

static void cdline(cdCtxCanvas *ctxcanvas, int x1, int y1, int x2, int y2)
{
  cdfline(ctxcanvas, (double)x1, (double)y1, (double)x2, (double)y2);
}

static void cdfrect(cdCtxCanvas *ctxcanvas, double xmin, double xmax, double ymin, double ymax)
{
  sUpdateFill(ctxcanvas, 0);

  PDF_rect(ctxcanvas->pdf, xmin, ymin, xmax-xmin, ymax-ymin);
  PDF_stroke(ctxcanvas->pdf);
}

static void cdrect(cdCtxCanvas *ctxcanvas, int xmin, int xmax, int ymin, int ymax)
{
  cdfrect(ctxcanvas, (double)xmin, (double)xmax, (double)ymin, (double)ymax);
}

static void cdfbox(cdCtxCanvas *ctxcanvas, double xmin, double xmax, double ymin, double ymax)
{
  sUpdateFill(ctxcanvas, 1);

  PDF_moveto(ctxcanvas->pdf, xmin, ymin);
  PDF_lineto(ctxcanvas->pdf, xmax, ymin);
  PDF_lineto(ctxcanvas->pdf, xmax, ymax);
  PDF_lineto(ctxcanvas->pdf, xmin, ymax);
  PDF_fill(ctxcanvas->pdf);
}

static void cdbox(cdCtxCanvas *ctxcanvas, int xmin, int xmax, int ymin, int ymax)
{
  cdfbox(ctxcanvas, (double)xmin, (double)xmax, (double)ymin, (double)ymax);
}

static void cdfarc(cdCtxCanvas *ctxcanvas, double xc, double yc, double w, double h, double a1, double a2)
{
  sUpdateFill(ctxcanvas, 0);

  /* angles in degrees counterclockwise, same as CD */

  if (w==h)
  {
    PDF_arc(ctxcanvas->pdf, xc, yc, 0.5*w, a1, a2);
    PDF_stroke(ctxcanvas->pdf);
  }
  else /* Ellipse: change the scale to create from the circle */
  {
    PDF_save(ctxcanvas->pdf);  /* save to use the local transform */

    PDF_translate(ctxcanvas->pdf, xc, yc);
    PDF_scale(ctxcanvas->pdf, w/h, 1);
    PDF_translate(ctxcanvas->pdf, -xc, -yc);

    PDF_arc(ctxcanvas->pdf, xc, yc, 0.5*h, a1, a2);
    PDF_stroke(ctxcanvas->pdf);

    PDF_restore(ctxcanvas->pdf);  /* restore from local */
  }
}

static void cdarc(cdCtxCanvas *ctxcanvas, int xc, int yc, int w, int h, double a1, double a2)
{
  cdfarc(ctxcanvas, (double)xc, (double)yc, (double)w, (double)h, a1, a2);
}

static void cdfsector(cdCtxCanvas *ctxcanvas, double xc, double yc, double w, double h, double a1, double a2)
{
  sUpdateFill(ctxcanvas, 1);

  if (w==h)
  {
    PDF_moveto(ctxcanvas->pdf, xc, yc);
    PDF_arc(ctxcanvas->pdf, xc, yc, 0.5*h, a1, a2);
    PDF_fill(ctxcanvas->pdf);
  }
  else /* Elipse: mudar a escala p/ criar a partir do circulo */
  {
    PDF_save(ctxcanvas->pdf);  /* save to use the local transform */

    PDF_translate(ctxcanvas->pdf, xc, yc);
    PDF_scale(ctxcanvas->pdf, w/h, 1);
    PDF_translate(ctxcanvas->pdf, -xc, -yc);

    PDF_moveto(ctxcanvas->pdf, xc, yc);
    PDF_arc(ctxcanvas->pdf, xc, yc, 0.5*h, a1, a2);
    PDF_fill(ctxcanvas->pdf);

    PDF_restore(ctxcanvas->pdf);  /* restore from local */
  }
}

static void cdsector(cdCtxCanvas *ctxcanvas, int xc, int yc, int w, int h, double a1, double a2)
{
  cdfsector(ctxcanvas, (double)xc, (double)yc, (double)w, (double)h, a1, a2);
}

static void cdfchord(cdCtxCanvas *ctxcanvas, double xc, double yc, double w, double h, double a1, double a2)
{
  sUpdateFill(ctxcanvas, 1);

  if (w==h)
  {
    PDF_arc(ctxcanvas->pdf, xc, yc, 0.5*h, a1, a2);
    PDF_fill(ctxcanvas->pdf);
  }
  else /* Elipse: mudar a escala p/ criar a partir do circulo */
  {
    PDF_save(ctxcanvas->pdf);  /* save to use the local transform */

    /* local transform */
    PDF_translate(ctxcanvas->pdf, xc, yc);
    PDF_scale(ctxcanvas->pdf, w/h, 1);
    PDF_translate(ctxcanvas->pdf, -xc, -yc);

    PDF_arc(ctxcanvas->pdf, xc, yc, 0.5*h, a1, a2);
    PDF_fill(ctxcanvas->pdf);

    PDF_restore(ctxcanvas->pdf);  /* restore from local */
  }
}

static void cdchord(cdCtxCanvas *ctxcanvas, int xc, int yc, int w, int h, double a1, double a2)
{
  cdfchord(ctxcanvas, (double)xc, (double)yc, (double)w, (double)h, a1, a2);
}

static void cdgetfontdim(cdCtxCanvas *ctxcanvas, int *max_width, int *height, int *ascent, int *descent)
{
  double fontsize, a, d, linegap;

  if (ctxcanvas->font<0)
    return;

  fontsize = PDF_get_value(ctxcanvas->pdf, "fontsize", 0);
  a = PDF_get_value(ctxcanvas->pdf, "ascender", 0);
  d = PDF_get_value(ctxcanvas->pdf, "descender", 0);

  /* linegap = PDF_info_font(ctxcanvas->pdf, 1, "linegap", ""); - not supported call */
  linegap = 0.23 * a;   /* use default value for linegap */
  a += linegap;
  d += linegap;  /* since d<0, it is a subtraction */
                 
  a *= fontsize;
  d *= fontsize;

  if (ascent) *ascent = (int)a;
  if (descent) *descent = (int)(-d);
  if (height) *height = (int)(a - d);
  if (max_width) *max_width = (int)(PDF_info_textline(ctxcanvas->pdf, "W", 0, "width", "")/ctxcanvas->scale);
}

static void cdgettextsize(cdCtxCanvas *ctxcanvas, const char *s, int len, int *width, int *height)
{
  if (ctxcanvas->font<0)
    return;
  if (height) cdgetfontdim(ctxcanvas, NULL, height, NULL, NULL);
  if (width) *width = (int)(PDF_info_textline(ctxcanvas->pdf, s, len, "width", "")/ctxcanvas->scale);
}

static void cdftext(cdCtxCanvas *ctxcanvas, double x, double y, const char *s, int len)
{
  char temp[200], options[200];

  PDF_setcolor(ctxcanvas->pdf, "fill", "rgb", get_red(ctxcanvas->canvas->foreground), 
                                              get_green(ctxcanvas->canvas->foreground), 
                                              get_blue(ctxcanvas->canvas->foreground), 0);

  strcpy(options, "");

  sprintf(temp, "rotate=%g ", ctxcanvas->canvas->text_orientation);
  strcat(options, temp);

  if (ctxcanvas->underline != 0)
    strcat(options, "underline=true ");
  else
    strcat(options, "underline=false ");

  if (ctxcanvas->strikeover != 0)
    strcat(options, "strikeout=true ");
  else
    strcat(options, "strikeout=false ");

  switch (ctxcanvas->canvas->text_alignment)
  {
  case CD_NORTH:
    sprintf(temp, "position={50 100} matchbox { boxheight={ascender descender} }");
    strcat(options, temp);
    break;
  case CD_NORTH_EAST:                                                           
    sprintf(temp, "position={100 100} matchbox { boxheight={ascender descender} }");
    strcat(options, temp);
    break;
  case CD_NORTH_WEST:
    sprintf(temp, "position={0 100} matchbox { boxheight={ascender descender} }");
    strcat(options, temp);
    break;
  case CD_EAST:
    sprintf(temp, "position={100 50} matchbox { boxheight={ascender descender} }");
    strcat(options, temp);
    break;
  case CD_WEST:
    sprintf(temp, "position={0 50} matchbox { boxheight={ascender descender} }");
    strcat(options, temp);
    break;
  case CD_CENTER:
    sprintf(temp, "position={50 50} matchbox { boxheight={ascender descender} }");
    strcat(options, temp);
    break;
  case CD_SOUTH_EAST:
    sprintf(temp, "position={100 0} matchbox { boxheight={ascender descender} }");
    strcat(options, temp);
    break;
  case CD_SOUTH:
    sprintf(temp, "position={50 0} matchbox { boxheight={ascender descender} }");
    strcat(options, temp);
    break;
  case CD_SOUTH_WEST:
    sprintf(temp, "position={0 0} matchbox { boxheight={ascender descender} }");
    strcat(options, temp);
    break;
  case CD_BASE_RIGHT:
    sprintf(temp, "position={100 0} matchbox { boxheight={ascender none} }");
    strcat(options, temp);
    break;
  case CD_BASE_CENTER:
    sprintf(temp, "position={50 0} matchbox { boxheight={ascender none} }");
    strcat(options, temp);
    break;
  case CD_BASE_LEFT:
    sprintf(temp, "position={0 0} matchbox { boxheight={ascender none} }");
    strcat(options, temp);
    break;
  }

  PDF_fit_textline(ctxcanvas->pdf, s, len, x, y, options);
}

static void cdtext(cdCtxCanvas *ctxcanvas, int x, int y, const char *s, int len)
{
  cdftext(ctxcanvas, (double)x, (double)y, s, len);
}

static void cdpoly(cdCtxCanvas *ctxcanvas, int mode, cdPoint* poly, int n)
{
  int i;

  if (mode == CD_CLIP)
    return;

  if (mode == CD_PATH)
  {
    int p, fill = 0;

    /* if there is any current path, remove it */
    /* Don't use PDF_endpath because here usually there will be no path scope */

    for (p=0; p<ctxcanvas->canvas->path_n; p++)
    {
      if (ctxcanvas->canvas->path[p] == CD_PATH_FILL ||
          ctxcanvas->canvas->path[p] == CD_PATH_FILLSTROKE)
      {
        fill = 1;
        break;
      }
    }

    /* must be set before starting path scope */
    sUpdateFill(ctxcanvas, 0);  /* set always */
    if (fill)
    {
      PDF_set_parameter(ctxcanvas->pdf, "fillrule", ctxcanvas->canvas->fill_mode==CD_EVENODD? "evenodd": "winding");
      sUpdateFill(ctxcanvas, fill);
    }

    i = 0;
    for (p=0; p<ctxcanvas->canvas->path_n; p++)
    {
      switch(ctxcanvas->canvas->path[p])
      {
      case CD_PATH_NEW:
        /* Don't use PDF_endpath because here usually there will be no path scope */
        break;
      case CD_PATH_MOVETO:
        if (i+1 > n) return;
        PDF_moveto(ctxcanvas->pdf, poly[i].x, poly[i].y);
        i++;
        break;
      case CD_PATH_LINETO:
        if (i+1 > n) return;
        PDF_lineto(ctxcanvas->pdf, poly[i].x, poly[i].y);
        i++;
        break;
      case CD_PATH_ARC:
        {
          double xc, yc, w, h, a1, a2;

          if (i+3 > n) return;

          if (!cdCanvasGetArcPathF(ctxcanvas->canvas, poly+i, &xc, &yc, &w, &h, &a1, &a2))
            return;

          if (w==h)
          {
            if ((a2-a1)<0)
              PDF_arcn(ctxcanvas->pdf, xc, yc, 0.5*w, a1, a2);
            else
              PDF_arc(ctxcanvas->pdf, xc, yc, 0.5*w, a1, a2);
          }
          else /* Ellipse: change the scale to create from the circle */
          {
            /* NOT SUPPORTED IN PATH SCOPE!!!!
            PDF_save(ctxcanvas->pdf);

            PDF_translate(ctxcanvas->pdf, xc, yc);
            PDF_scale(ctxcanvas->pdf, w/h, 1);
            PDF_translate(ctxcanvas->pdf, -xc, -yc); */
            double s = h;
            if (w > h)
              s = w;

            if ((a2-a1)<0)
              PDF_arcn(ctxcanvas->pdf, xc, yc, 0.5*s, a1, a2);
            else
              PDF_arc(ctxcanvas->pdf, xc, yc, 0.5*s, a1, a2);

            /* PDF_restore(ctxcanvas->pdf); */
          }

          i += 3;
        }
        break;
      case CD_PATH_CURVETO:
        if (i+3 > n) return;
        PDF_curveto(ctxcanvas->pdf, poly[i].x, poly[i].y, poly[i+1].x, poly[i+1].y, poly[i+2].x, poly[i+2].y);
        i += 3;
        break;
      case CD_PATH_CLOSE:
        PDF_closepath(ctxcanvas->pdf);
        break;
      case CD_PATH_FILL:
        PDF_fill(ctxcanvas->pdf);
        break;
      case CD_PATH_STROKE:
        PDF_stroke(ctxcanvas->pdf);
        break;
      case CD_PATH_FILLSTROKE:
        PDF_fill_stroke(ctxcanvas->pdf);
        break;
      case CD_PATH_CLIP:
        PDF_clip(ctxcanvas->pdf);
        break;
      }
    }
    return;
  }

  if (mode == CD_FILL)
    sUpdateFill(ctxcanvas, 1);
  else
    sUpdateFill(ctxcanvas, 0);

  if (mode==CD_FILL)
  {
    /* must be set before starting path scope */
    if (ctxcanvas->holes || ctxcanvas->canvas->fill_mode==CD_EVENODD)
      PDF_set_parameter(ctxcanvas->pdf, "fillrule", "evenodd");
    else
      PDF_set_parameter(ctxcanvas->pdf, "fillrule", "winding");
  }

  PDF_moveto(ctxcanvas->pdf, poly[0].x, poly[0].y);

  if (mode == CD_BEZIER)
  {
    for (i=1; i<n; i+=3)
      PDF_curveto(ctxcanvas->pdf, poly[i].x, poly[i].y,
                                  poly[i+1].x, poly[i+1].y, 
                                  poly[i+2].x, poly[i+2].y);
  }
  else
  {
    int hole_index = 0;

    for (i=1; i<n; i++)
    {
      if (ctxcanvas->holes && i == ctxcanvas->poly_holes[hole_index])
      {
        PDF_moveto(ctxcanvas->pdf, poly[i].x, poly[i].y);
        hole_index++;
      }
      else
        PDF_lineto(ctxcanvas->pdf, poly[i].x, poly[i].y);
    }
  }

  switch (mode)
  {
  case CD_CLOSED_LINES :
    PDF_closepath_stroke(ctxcanvas->pdf);
    break;
  case CD_OPEN_LINES :
    PDF_stroke(ctxcanvas->pdf);
    break;
  case CD_BEZIER :
    PDF_stroke(ctxcanvas->pdf);
    break;
  case CD_FILL :
    PDF_fill(ctxcanvas->pdf);
    break;
  }
}

static void cdfpoly(cdCtxCanvas *ctxcanvas, int mode, cdfPoint* poly, int n)
{
  int i;

  if (mode == CD_CLIP)
    return;

  if (mode == CD_PATH)
  {
    int p, fill = 0;

    /* if there is any current path, remove it */
    /* Don't use PDF_endpath because here usually there will be no path scope */

    for (p=0; p<ctxcanvas->canvas->path_n; p++)
    {
      if (ctxcanvas->canvas->path[p] == CD_PATH_FILL ||
          ctxcanvas->canvas->path[p] == CD_PATH_FILLSTROKE)
      {
        fill = 1;
        break;
      }
    }

    /* must be set before starting path scope */
    sUpdateFill(ctxcanvas, 0);  /* set always */
    if (fill)
    {
      PDF_set_parameter(ctxcanvas->pdf, "fillrule", ctxcanvas->canvas->fill_mode==CD_EVENODD? "evenodd": "winding");
      sUpdateFill(ctxcanvas, fill);
    }

    i = 0;
    for (p=0; p<ctxcanvas->canvas->path_n; p++)
    {
      switch(ctxcanvas->canvas->path[p])
      {
      case CD_PATH_NEW:
        /* Don't use PDF_endpath because here usually there will be no path scope */
        break;
      case CD_PATH_MOVETO:
        if (i+1 > n) return;
        PDF_moveto(ctxcanvas->pdf, poly[i].x, poly[i].y);
        i++;
        break;
      case CD_PATH_LINETO:
        if (i+1 > n) return;
        PDF_lineto(ctxcanvas->pdf, poly[i].x, poly[i].y);
        i++;
        break;
      case CD_PATH_ARC:
        {
          double xc, yc, w, h, a1, a2;

          if (i+3 > n) return;

          if (!cdfCanvasGetArcPath(ctxcanvas->canvas, poly+i, &xc, &yc, &w, &h, &a1, &a2))
            return;

          if (w==h)
          {
            if ((a2-a1)<0)
              PDF_arcn(ctxcanvas->pdf, xc, yc, 0.5*w, a1, a2);
            else
              PDF_arc(ctxcanvas->pdf, xc, yc, 0.5*w, a1, a2);
          }
          else /* Ellipse: change the scale to create from the circle */
          {
            /* NOT SUPPORTED IN PATH SCOPE!!!!
            PDF_save(ctxcanvas->pdf);

            PDF_translate(ctxcanvas->pdf, xc, yc);
            PDF_scale(ctxcanvas->pdf, w/h, 1);
            PDF_translate(ctxcanvas->pdf, -xc, -yc);  */
            double s = h;
            if (w > h)
              s = w;

            if ((a2-a1)<0)
              PDF_arcn(ctxcanvas->pdf, xc, yc, 0.5*s, a1, a2);
            else
              PDF_arc(ctxcanvas->pdf, xc, yc, 0.5*s, a1, a2);

            /* PDF_restore(ctxcanvas->pdf);  */
          }

          i += 3;
        }
        break;
      case CD_PATH_CURVETO:
        if (i+3 > n) return;
        PDF_curveto(ctxcanvas->pdf, poly[i].x, poly[i].y, poly[i+1].x, poly[i+1].y, poly[i+2].x, poly[i+2].y);
        i += 3;
        break;
      case CD_PATH_CLOSE:
        PDF_closepath(ctxcanvas->pdf);
        break;
      case CD_PATH_FILL:
        PDF_fill(ctxcanvas->pdf);
        break;
      case CD_PATH_STROKE:
        PDF_stroke(ctxcanvas->pdf);
        break;
      case CD_PATH_FILLSTROKE:
        PDF_fill_stroke(ctxcanvas->pdf);
        break;
      case CD_PATH_CLIP:
        PDF_clip(ctxcanvas->pdf);
        break;
      }
    }
    return;
  }

  if (mode == CD_FILL)
    sUpdateFill(ctxcanvas, 1);
  else
    sUpdateFill(ctxcanvas, 0);

  if (mode==CD_FILL)
  {
    /* must be set before starting path scope */
    if (ctxcanvas->holes || ctxcanvas->canvas->fill_mode==CD_EVENODD)
      PDF_set_parameter(ctxcanvas->pdf, "fillrule", "evenodd");
    else
      PDF_set_parameter(ctxcanvas->pdf, "fillrule", "winding");
  }

  PDF_moveto(ctxcanvas->pdf, poly[0].x, poly[0].y);

  if (mode == CD_BEZIER)
  {
    for (i=1; i<n; i+=3)
      PDF_curveto(ctxcanvas->pdf, poly[i].x, poly[i].y,
                                  poly[i+1].x, poly[i+1].y, 
                                  poly[i+2].x, poly[i+2].y);
  }
  else
  {
    int hole_index = 0;

    for (i=1; i<n; i++)
    {
      if (ctxcanvas->holes && i == ctxcanvas->poly_holes[hole_index])
      {
        PDF_moveto(ctxcanvas->pdf, poly[i].x, poly[i].y);
        hole_index++;
      }
      else
        PDF_lineto(ctxcanvas->pdf, poly[i].x, poly[i].y);
    }
  }

  switch (mode)
  {
  case CD_CLOSED_LINES :
    PDF_closepath_stroke(ctxcanvas->pdf);
    break;
  case CD_OPEN_LINES :
    PDF_stroke(ctxcanvas->pdf);
    break;
  case CD_BEZIER :
    PDF_stroke(ctxcanvas->pdf);
    break;
  case CD_FILL :
    PDF_fill(ctxcanvas->pdf);
    break;
  }
}

/******************************************************/
/* attributes                                         */
/******************************************************/

static int cdlinestyle(cdCtxCanvas *ctxcanvas, int style)
{
  double mm = ctxcanvas->canvas->xres;
  char options[200];

  switch (style)
  {
  case CD_CONTINUOUS : /* empty dash */
    PDF_setdash(ctxcanvas->pdf, 0, 0);
    break;
  case CD_DASHED :
    PDF_setdash(ctxcanvas->pdf, 3*mm, mm);
    break;
  case CD_DOTTED :
    PDF_setdash(ctxcanvas->pdf, mm, mm);
    break;
  case CD_DASH_DOT :
    sprintf(options, "dasharray={%g %g %g %g}", 3*mm, mm, mm, mm);
    PDF_setdashpattern(ctxcanvas->pdf, options);
    break;
  case CD_DASH_DOT_DOT :
    sprintf(options, "dasharray={%g %g %g %g %g %g}", 3*mm, mm, mm, mm, mm, mm);
    PDF_setdashpattern(ctxcanvas->pdf, options);
    break;
  case CD_CUSTOM :
    {
      int i;
      /* size here is in pixels, do not use mm */
      strcpy(options, "dasharray={");
      for (i = 0; i < ctxcanvas->canvas->line_dashes_count; i++)
      {
        char tmp[80];
        sprintf(tmp, "%g ", (double)ctxcanvas->canvas->line_dashes[i]);
        strcat(options, tmp);
      }
      strcat(options, "}");
      PDF_setdashpattern(ctxcanvas->pdf, options);
    }
    break;
  }

  return style;
}

static int cdlinewidth(cdCtxCanvas *ctxcanvas, int width)
{
  if (width==0) width = 1;

  PDF_setlinewidth(ctxcanvas->pdf, width);

  return width;
}

static int cdlinejoin(cdCtxCanvas *ctxcanvas, int join)
{
  int cd2ps_join[] = {0, 2, 1};
  PDF_setlinejoin(ctxcanvas->pdf, cd2ps_join[join]);
  return join;
}

static int cdlinecap(cdCtxCanvas *ctxcanvas, int cap)
{
  int cd2pdf_cap[] =  {0, 2, 1};
  PDF_setlinecap(ctxcanvas->pdf, cd2pdf_cap[cap]);
  return cap;
}

static void make_pattern(cdCtxCanvas *ctxcanvas, int n, int m, void* data, int (*data2rgb)(cdCtxCanvas *ctxcanvas, int n, int i, int j, void* data, unsigned char*r, unsigned char*g, unsigned char*b))
{
  int i, j;
  unsigned char r, g, b;

  PDF_suspend_page(ctxcanvas->pdf, "");
  ctxcanvas->pattern = PDF_begin_pattern(ctxcanvas->pdf, n, m,
      ((double)n)*ctxcanvas->scale, ((double)m)*ctxcanvas->scale, 1);
  PDF_scale(ctxcanvas->pdf, ctxcanvas->scale, ctxcanvas->scale);

  for (j=0; j<m; j++)
  {
    for (i=0; i<n; i++)
    {
      int ret = data2rgb(ctxcanvas, n, i, j, data, &r, &g, &b);
      if (ret==-1) continue;
      PDF_setcolor(ctxcanvas->pdf, "fill", "rgb", ((double)r)/255, ((double)g)/255, ((double)b)/255, 0);
      PDF_rect(ctxcanvas->pdf, i, j, 1, 1);
      PDF_fill(ctxcanvas->pdf);
    }
  }

  PDF_end_pattern(ctxcanvas->pdf);
  PDF_resume_page(ctxcanvas->pdf, "");
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

static int cdhatch(cdCtxCanvas *ctxcanvas, int style)
{
  unsigned char r, g, b;
  int hsize = ctxcanvas->hatchboxsize - 1;
  int hhalf = hsize / 2;

  PDF_suspend_page(ctxcanvas->pdf, "");
  ctxcanvas->pattern = PDF_begin_pattern(ctxcanvas->pdf, hsize, hsize,
      ((double)hsize)*ctxcanvas->scale, ((double)hsize)*ctxcanvas->scale, 1);

  PDF_scale(ctxcanvas->pdf, ctxcanvas->scale, ctxcanvas->scale);

  if (ctxcanvas->canvas->back_opacity==CD_OPAQUE)
  {
    cdDecodeColor(ctxcanvas->canvas->background, &r, &g, &b);
    PDF_setcolor(ctxcanvas->pdf, "fill", "rgb", ((double)r)/255, ((double)g)/255, ((double)b)/255, 0);
    PDF_rect(ctxcanvas->pdf, 0, 0, hsize, hsize);
    PDF_fill(ctxcanvas->pdf);
  }

  cdDecodeColor(ctxcanvas->canvas->foreground, &r, &g, &b);
  PDF_setcolor(ctxcanvas->pdf, "stroke", "rgb", ((double)r)/255, ((double)g)/255, ((double)b)/255, 0);

  switch(style)
  {
  case CD_HORIZONTAL:
    PDF_moveto(ctxcanvas->pdf, 0, hhalf);
    PDF_lineto(ctxcanvas->pdf, hsize, hhalf);
    break;
  case CD_VERTICAL:
    PDF_moveto(ctxcanvas->pdf, hhalf, 0);
    PDF_lineto(ctxcanvas->pdf, hhalf, hsize);
    break;
  case CD_BDIAGONAL:
    PDF_moveto(ctxcanvas->pdf, 0, hsize);
    PDF_lineto(ctxcanvas->pdf, hsize, 0);
    break;
  case CD_FDIAGONAL:
    PDF_moveto(ctxcanvas->pdf, 0, 0);
    PDF_lineto(ctxcanvas->pdf, hsize, hsize);
    break;
  case CD_CROSS:
    PDF_moveto(ctxcanvas->pdf, hsize, 0);
    PDF_lineto(ctxcanvas->pdf, hsize, hsize);
    PDF_moveto(ctxcanvas->pdf, 0, hhalf);
    PDF_lineto(ctxcanvas->pdf, hsize, hhalf);
    break;
  case CD_DIAGCROSS:
    PDF_moveto(ctxcanvas->pdf, 0, 0);
    PDF_lineto(ctxcanvas->pdf, hsize, hsize);
    PDF_moveto(ctxcanvas->pdf, hsize, 0);
    PDF_lineto(ctxcanvas->pdf, 0, hsize);
    break;
  }

  PDF_stroke(ctxcanvas->pdf);

  PDF_end_pattern(ctxcanvas->pdf);
  PDF_resume_page(ctxcanvas->pdf, "");
  return style;
}

static int cdfont(cdCtxCanvas *ctxcanvas, const char *type_face, int style, int size)
{
  int newfont, sizepixel;
  char nativefontname[1024];
  const char* options = "";

  if (cdStrEqualNoCase(type_face, "System"))
    type_face = "Courier";

  strcpy(nativefontname, type_face);

  if (cdStrEqualNoCase(type_face, "Courier") ||
      cdStrEqualNoCase(type_face, "Helvetica"))
  {
    if (style&CD_BOLD && style&CD_ITALIC)
      strcat(nativefontname, "-BoldOblique");
    else
    {
      if (style&CD_BOLD)
        strcat(nativefontname, "-Bold");

      if (style&CD_ITALIC)
        strcat(nativefontname, "-Oblique");
    }
  }
  else if (cdStrEqualNoCase(type_face, "Times"))
  {
    if ((style&3) == CD_PLAIN)
      strcat(nativefontname, "-Roman");
    if (style&CD_BOLD && style&CD_ITALIC)
      strcat(nativefontname, "-BoldItalic");
    else
    {
      if (style&CD_BOLD)
        strcat(nativefontname, "-Bold");

      if (style&CD_ITALIC)
        strcat(nativefontname, "-Italic");
    }
  }
  else
  {
    switch(style&3)
    {
      case CD_PLAIN:
        options = "fontstyle=normal";
        break;
      case CD_BOLD:
        options = "fontstyle=bold";
        break;
      case CD_ITALIC:
        options = "fontstyle=italic";
        break;
      case CD_BOLD_ITALIC:
        options = "fontstyle=bolditalic";
        break;
    }
  }

  newfont = PDF_load_font(ctxcanvas->pdf, nativefontname, 0, "auto", options);
  if (newfont<0) 
  {
    /* must reload the previous one */
    return 0;
  }
  ctxcanvas->font = newfont;

  sizepixel = cdGetFontSizePixels(ctxcanvas->canvas, size);
  PDF_setfont(ctxcanvas->pdf, ctxcanvas->font, sizepixel);

  if (style&CD_UNDERLINE)
    ctxcanvas->underline = 1;
  else
    ctxcanvas->underline = 0;

  if (style&CD_STRIKEOUT)
    ctxcanvas->strikeover = 1;
  else
    ctxcanvas->strikeover = 0;

  return 1;
}

static void cdtransform(cdCtxCanvas *ctxcanvas, const double* matrix)
{
  /* reset to identity */
  PDF_setmatrix(ctxcanvas->pdf, 1, 0, 0, 1, 0, 0);

  /* default coordinate system is in points, change it to pixels. */
  PDF_scale(ctxcanvas->pdf, ctxcanvas->scale, ctxcanvas->scale); 

  if (matrix)
  {
    PDF_concat(ctxcanvas->pdf, matrix[0], matrix[1], matrix[2], matrix[3], matrix[4], matrix[5]);
  }
  else if (ctxcanvas->rotate_angle)
  {
    /* rotation = translate to point + rotation + translate back */
    PDF_translate(ctxcanvas->pdf, ctxcanvas->rotate_center_x, ctxcanvas->rotate_center_y);
    PDF_rotate(ctxcanvas->pdf, (double)ctxcanvas->rotate_angle);
    PDF_translate(ctxcanvas->pdf, -ctxcanvas->rotate_center_x, -ctxcanvas->rotate_center_y);
  }
}

/******************************************************/
/* client images                                      */
/******************************************************/

static void cdputimagerectrgb(cdCtxCanvas *ctxcanvas, int iw, int ih, const unsigned char *r, const unsigned char *g, const unsigned char *b, int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax)
{
  int i, j, d, image, rw, rh, rgb_size, pos;
  char options[80];
  unsigned char* rgb_data;

  if (xmin<0 || ymin<0 || xmax-xmin+1>iw || ymax-ymin+1>ih) return;

  rw = xmax-xmin+1;
  rh = ymax-ymin+1;

  rgb_size = 3*rw*rh;
  rgb_data = (unsigned char*)malloc(rgb_size);
  if (!rgb_data) return;

  d = 0;
  for (i=ymax; i>=ymin; i--)
    for (j=xmin; j<=xmax; j++)
    {
      pos = i*iw+j;
      rgb_data[d] = r[pos]; d++;
      rgb_data[d] = g[pos]; d++;
      rgb_data[d] = b[pos]; d++;
    }

  PDF_create_pvf(ctxcanvas->pdf, "cd_raw_rgb", 0, rgb_data, rgb_size, "");

  sprintf(options, "width=%d height=%d components=3 bpc=8", rw, rh);
  image = PDF_load_image(ctxcanvas->pdf, "raw", "cd_raw_rgb", 0, options);

  sprintf(options, "boxsize={%d %d} fitmethod=meet", w, h);
  PDF_fit_image(ctxcanvas->pdf, image, x, y, options);

  PDF_delete_pvf(ctxcanvas->pdf, "cd_raw_rgb", 0);
  free(rgb_data);
}

static void cdputimagerectrgba(cdCtxCanvas *ctxcanvas, int iw, int ih, const unsigned char *r, const unsigned char *g, const unsigned char *b, const unsigned char *a, int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax)
{
  int i, j, d, image, image_mask, rw, rh, alpha_size, rgb_size, pos;
  char options[80];
  unsigned char *rgb_data, *alpha_data;

  if (xmin<0 || ymin<0 || xmax-xmin+1>iw || ymax-ymin+1>ih) return;

  rw = xmax-xmin+1;
  rh = ymax-ymin+1;

  rgb_size = 3*rw*rh;
  rgb_data = (unsigned char*)malloc(rgb_size);
  if (!rgb_data) return;

  d = 0;
  for (i=ymax; i>=ymin; i--)
    for (j=xmin; j<=xmax; j++)
    {
      pos = i*iw+j;
      rgb_data[d] = r[pos]; d++;
      rgb_data[d] = g[pos]; d++;
      rgb_data[d] = b[pos]; d++;
    }

  alpha_size = rw*rh;
  alpha_data = (unsigned char*)malloc(alpha_size);
  if (!alpha_data) return;

  d = 0;
  for (i=ymax; i>=ymin; i--)
    for (j=xmin; j<=xmax; j++)
    {
      pos = i*iw+j;
      alpha_data[d] = a[pos]; d++;
    }

  PDF_create_pvf(ctxcanvas->pdf, "cd_raw_rgb", 0, rgb_data, rgb_size, "");
  PDF_create_pvf(ctxcanvas->pdf, "cd_raw_alpha", 0, alpha_data, alpha_size, "");

  sprintf(options, "width=%d height=%d components=1 bpc=8 imagewarning=true", rw, rh);
  image_mask = PDF_load_image(ctxcanvas->pdf, "raw", "cd_raw_alpha", 0, options);

  sprintf(options, "width=%d height=%d components=3 bpc=8 masked=%d", rw, rh, image_mask);
  image = PDF_load_image(ctxcanvas->pdf, "raw", "cd_raw_rgb", 0, options);

  sprintf(options, "boxsize={%d %d} fitmethod=meet", w, h);
  PDF_fit_image(ctxcanvas->pdf, image, x, y, options);

  PDF_delete_pvf(ctxcanvas->pdf, "cd_raw_alpha", 0);
  free(alpha_data);
  PDF_delete_pvf(ctxcanvas->pdf, "cd_raw_rgb", 0);
  free(rgb_data);
}

static void cdputimagerectmap(cdCtxCanvas *ctxcanvas, int iw, int ih, const unsigned char *index, const long int *colors, int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax)
{
  int i, j, d, rw, rh, image, rgb_size, pos;
  char options[80];
  unsigned char* rgb_data;
  unsigned char r, g, b;

  if (xmin<0 || ymin<0 || xmax-xmin+1>iw || ymax-ymin+1>ih) return;

  rw = xmax-xmin+1;
  rh = ymax-ymin+1;

  rgb_size = 3*rw*rh;
  rgb_data = (unsigned char*)malloc(rgb_size);
  if (!rgb_data) return;

  d = 0;
  for (i=ymax; i>=ymin; i--)
    for (j=xmin; j<=xmax; j++)
    {
      pos = i*iw+j;
      cdDecodeColor(colors[index[pos]], &r, &g, &b);
      rgb_data[d] = r; d++;
      rgb_data[d] = g; d++;
      rgb_data[d] = b; d++;
    }

  PDF_create_pvf(ctxcanvas->pdf, "cd_raw_rgb", 0, rgb_data, rgb_size, "");

  sprintf(options, "width=%d height=%d components=3 bpc=8", rw, rh);
  image = PDF_load_image(ctxcanvas->pdf, "raw", "cd_raw_rgb", 0, options);

  sprintf(options, "boxsize={%d %d} fitmethod=meet", w, h);
  PDF_fit_image(ctxcanvas->pdf, image, x, y, options);

  PDF_delete_pvf(ctxcanvas->pdf, "cd_raw_rgb", 0);
  free(rgb_data);
}

/******************************************************/
/* server images                                      */
/******************************************************/

static void cdpixel(cdCtxCanvas *ctxcanvas, int x, int y, long int color)
{
  PDF_setcolor(ctxcanvas->pdf, "fill", "rgb", get_red(color), get_green(color), get_blue(color),  0);

  PDF_moveto(ctxcanvas->pdf, x, y);
  PDF_circle(ctxcanvas->pdf, x, y, .5);

  PDF_fill(ctxcanvas->pdf);
}

/******************************************************/
/* custom attributes                                  */
/******************************************************/

static void set_poly_attrib(cdCtxCanvas *ctxcanvas, char* data)
{
  int hole;

  if (data == NULL)
  {
    ctxcanvas->holes = 0;
    return;
  }

  sscanf(data, "%d", &hole);
  ctxcanvas->poly_holes[ctxcanvas->holes] = hole;
  ctxcanvas->holes++;
}

static char* get_poly_attrib(cdCtxCanvas *ctxcanvas)
{
  static char holes[10];
  sprintf(holes, "%d", ctxcanvas->holes);
  return holes;
}

static cdAttribute poly_attrib =
{
  "POLYHOLE",
  set_poly_attrib,
  get_poly_attrib
}; 

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

static void set_rotate_attrib(cdCtxCanvas *ctxcanvas, char* data)
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

static char* get_rotate_attrib(cdCtxCanvas *ctxcanvas)
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

static void set_pattern_attrib(cdCtxCanvas *ctxcanvas, char* data)
{
  if (data)
  {
    int n, m;
    sscanf(data, "%dx%d", &n, &m);

    PDF_suspend_page(ctxcanvas->pdf, "");
    ctxcanvas->pattern = PDF_begin_pattern(ctxcanvas->pdf, n, m,
        ((double)n)*ctxcanvas->scale, ((double)m)*ctxcanvas->scale, 1);
    PDF_scale(ctxcanvas->pdf, ctxcanvas->scale, ctxcanvas->scale);
  }
  else
  {
    PDF_end_pattern(ctxcanvas->pdf);
    PDF_resume_page(ctxcanvas->pdf, "");
    ctxcanvas->canvas->interior_style = CD_PATTERN;
  }
}

static cdAttribute pattern_attrib =
{
  "PATTERN",
  set_pattern_attrib,
  NULL
}; 

static void set_subject_attrib(cdCtxCanvas *ctxcanvas, char* data)
{
  if (data)
    PDF_set_info(ctxcanvas->pdf, "Subject", data);
}

static cdAttribute subject_attrib =
{
  "SUBJECT",
  set_subject_attrib,
  NULL
}; 

static void set_title_attrib(cdCtxCanvas *ctxcanvas, char* data)
{
  if (data)
    PDF_set_info(ctxcanvas->pdf, "Title", data);
}

static cdAttribute title_attrib =
{
  "TITLE",
  set_title_attrib,
  NULL
}; 

static void set_creator_attrib(cdCtxCanvas *ctxcanvas, char* data)
{
  if (data)
    PDF_set_info(ctxcanvas->pdf, "Creator", data);
}

static cdAttribute creator_attrib =
{
  "CREATOR",
  set_creator_attrib,
  NULL
}; 

static void set_author_attrib(cdCtxCanvas *ctxcanvas, char* data)
{
  if (data)
    PDF_set_info(ctxcanvas->pdf, "Author", data);
}

static cdAttribute author_attrib =
{
  "AUTHOR",
  set_author_attrib,
  NULL
}; 

static void set_keywords_attrib(cdCtxCanvas *ctxcanvas, char* data)
{
  if (data)
    PDF_set_info(ctxcanvas->pdf, "Keywords", data);
}

static cdAttribute keywords_attrib =
{
  "KEYWORDS",
  set_keywords_attrib,
  NULL
}; 

static void set_opacity_attrib(cdCtxCanvas *ctxcanvas, char* data)
{
  int state;

  if (data)
  {
    sscanf(data, "%d", &ctxcanvas->opacity);
    if (ctxcanvas->opacity < 0) ctxcanvas->opacity = 0;
    if (ctxcanvas->opacity > 255) ctxcanvas->opacity = 255;
  }
  else
    ctxcanvas->opacity = 255;

  /* reuse the extended graphics state if the opacity is the same */
  if (ctxcanvas->opacity_states[ctxcanvas->opacity] == -1)
  {
    char options[50];
    sprintf(options, "opacityfill=%g opacitystroke=%g", ctxcanvas->opacity/255.0, ctxcanvas->opacity/255.0);
    state = PDF_create_gstate(ctxcanvas->pdf, options);
    ctxcanvas->opacity_states[ctxcanvas->opacity] = state;
  }
  else
    state = ctxcanvas->opacity_states[ctxcanvas->opacity];

  PDF_set_gstate(ctxcanvas->pdf, state);
}

static char* get_opacity_attrib(cdCtxCanvas *ctxcanvas)
{
  static char data[50];
  sprintf(data, "%d", ctxcanvas->opacity);
  return data;
}

static cdAttribute opacity_attrib =
{
  "OPACITY",
  set_opacity_attrib,
  get_opacity_attrib
}; 

static char* get_pdf_attrib(cdCtxCanvas *ctxcanvas)
{
  return (char*)ctxcanvas->pdf;
}

static cdAttribute pdf_attrib =
{
  "PDF",
  NULL,
  get_pdf_attrib
}; 

static char* get_version_attrib(cdCtxCanvas* ctxcanvas)
{
  (void)ctxcanvas;
  return (char*)PDF_get_parameter(ctxcanvas->pdf, "version", 0);
}

static cdAttribute version_attrib =
{
  "PDFLIBVERSION",
  NULL,
  get_version_attrib
}; 

static void cdcreatecanvas(cdCanvas* canvas, void *data)
{
  char *line = (char *)data;
  cdCtxCanvas *ctxcanvas;
  char filename[10240] = "";

  ctxcanvas = (cdCtxCanvas *)malloc(sizeof(cdCtxCanvas));
  memset(ctxcanvas, 0, sizeof(cdCtxCanvas));

  line += cdGetFileName(line, filename);
  if (filename[0] == 0)
    return;

  ctxcanvas->pdf = PDF_new();
  if (!ctxcanvas->pdf)
  {
    free(ctxcanvas);
    return;
  }

  if (PDF_begin_document(ctxcanvas->pdf, filename, 0, "") == -1)
  {
    PDF_delete(ctxcanvas->pdf);
    free(ctxcanvas);
    return;
  }

  PDF_set_parameter(ctxcanvas->pdf, "fontwarning", "false");
  PDF_set_parameter(ctxcanvas->pdf, "errorpolicy", "return");

  cdRegisterAttribute(canvas, &poly_attrib);
  cdRegisterAttribute(canvas, &hatchboxsize_attrib);
  cdRegisterAttribute(canvas, &rotate_attrib);
  cdRegisterAttribute(canvas, &opacity_attrib);
  cdRegisterAttribute(canvas, &pattern_attrib);
  cdRegisterAttribute(canvas, &pdf_attrib);
  cdRegisterAttribute(canvas, &subject_attrib);
  cdRegisterAttribute(canvas, &title_attrib);
  cdRegisterAttribute(canvas, &creator_attrib);
  cdRegisterAttribute(canvas, &author_attrib);
  cdRegisterAttribute(canvas, &keywords_attrib);
  cdRegisterAttribute(canvas, &version_attrib);

  setpdfdefaultvalues(ctxcanvas);

  while (*line != '\0')
  {
    while (*line != '\0' && *line != '-') 
      line++;

    if (*line != '\0')
    {
      float num;
      line++;
      switch (*line++)
      {
      case 'p':
        {
          int paper;
          sscanf(line, "%d", &paper);
          cdSetPaperSize(paper, &ctxcanvas->width_pt, &ctxcanvas->height_pt);
          ctxcanvas->width_mm = ctxcanvas->width_pt/CD_MM2PT;
          ctxcanvas->height_mm = ctxcanvas->height_pt/CD_MM2PT;
          break;
        }
      case 'w':
        sscanf(line, "%g", &num);
        ctxcanvas->width_mm = num;
        ctxcanvas->width_pt = CD_MM2PT*ctxcanvas->width_mm;
        break;
      case 'h':
        sscanf(line, "%g", &num);
        ctxcanvas->height_mm = num;
        ctxcanvas->height_pt = CD_MM2PT*ctxcanvas->height_mm;
        break;
      case 's':
        sscanf(line, "%d", &(ctxcanvas->res));
        break;
      case 'o':
        ctxcanvas->landscape = 1;
        break;
      }
    }

    while (*line != '\0' && *line != ' ') 
      line++;
  }

  /* store the base canvas */
  ctxcanvas->canvas = canvas;

  /* update canvas context */
  canvas->ctxcanvas = ctxcanvas;

  if (ctxcanvas->landscape)
  {
    _cdSwapDouble(ctxcanvas->width_pt, ctxcanvas->height_pt);
    _cdSwapDouble(ctxcanvas->width_mm, ctxcanvas->height_mm);
  }

  init_pdf(ctxcanvas);
}

static void cdinittable(cdCanvas* canvas)
{
  canvas->cxFlush = cdflush;

  canvas->cxPixel = cdpixel;

  canvas->cxLine = cdline;
  canvas->cxPoly = cdpoly;
  canvas->cxRect = cdrect;
  canvas->cxBox = cdbox;
  canvas->cxArc = cdarc;
  canvas->cxSector = cdsector;
  canvas->cxChord = cdchord;
  canvas->cxText = cdtext;

  canvas->cxFLine = cdfline;
  canvas->cxFPoly = cdfpoly;
  canvas->cxFRect = cdfrect;
  canvas->cxFBox = cdfbox;
  canvas->cxFArc = cdfarc;
  canvas->cxFSector = cdfsector;
  canvas->cxFChord = cdfchord;
  canvas->cxFText = cdftext;

  canvas->cxGetFontDim = cdgetfontdim;
  canvas->cxGetTextSize = cdgettextsize;
  canvas->cxPutImageRectRGB = cdputimagerectrgb;
  canvas->cxPutImageRectMap = cdputimagerectmap;
  canvas->cxPutImageRectRGBA = cdputimagerectrgba;

  canvas->cxClip = cdclip;
  canvas->cxClipArea = cdcliparea;
  canvas->cxFClipArea = cdfcliparea;
  canvas->cxLineStyle = cdlinestyle;
  canvas->cxLineWidth = cdlinewidth;
  canvas->cxLineCap = cdlinecap;
  canvas->cxLineJoin = cdlinejoin;
  canvas->cxPattern = cdpattern;
  canvas->cxStipple = cdstipple;
  canvas->cxHatch = cdhatch;
  canvas->cxFont = cdfont;
  canvas->cxTransform = cdtransform;

  canvas->cxKillCanvas = cdkillcanvas;
}

static cdContext cdPDFContext =
{
  CD_CAP_ALL & ~(CD_CAP_CLEAR | CD_CAP_PLAY | CD_CAP_PALETTE | 
                 CD_CAP_REGION | CD_CAP_IMAGESRV | CD_CAP_TEXTSIZE | 
                 CD_CAP_WRITEMODE | CD_CAP_GETIMAGERGB),
  0,
  cdcreatecanvas,
  cdinittable,
  NULL,
  NULL,
};

cdContext* cdContextPDF(void)
{
  return &cdPDFContext;
}
