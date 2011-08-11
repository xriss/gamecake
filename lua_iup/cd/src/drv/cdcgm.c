/** \file
 * \brief CGM driver
 *
 * See Copyright Notice in cd.h
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <limits.h>

#include "cd.h"
#include "cd_private.h"
#include "cdcgm.h"
#include "cgm.h"

#define get_red(_)   (((double)cdRed(_))/255.)
#define get_green(_) (((double)cdGreen(_))/255.)
#define get_blue(_)  (((double)cdBlue(_))/255.)


struct _cdCtxCanvas 
{
  cdCanvas* canvas;
  CGM  *cgm;
  
  char filename[10240];   /* Arquivo CGM */

  int  codificacao;     /* Codificacao */
  int  vdc_int_prec;
  int first;               /* Primeira primitiva a ser desenhada */
  long point;
  int hassize;
  int patindex;
  
  struct 
  {
    cdfRect bbox;
    int first;
  } clip;
  
  cdfRect b_box;
};

static double cve_black[] = { 0.0, 0.0, 0.0 };
static double cve_white[] = { 1.0, 1.0, 1.0 };
 
/* From INTCGM */
int cdplayCGM(cdCanvas* canvas, int xmin, int xmax, int ymin, int ymax, void *data);
int cdRegisterCallbackCGM(int cb, cdCallback func);


/*
%F Atualiza os valores do bounding box
*/
static void setbbox (cdCtxCanvas *ctxcanvas, double x, double y )
{
  if ( ctxcanvas->first )
  {
    ctxcanvas->b_box.xmin = x;
    ctxcanvas->b_box.xmax = x;
    ctxcanvas->b_box.ymin = y;
    ctxcanvas->b_box.ymax = y;
    ctxcanvas->first = 0;
  }
  
  if ( x<ctxcanvas->b_box.xmin ) ctxcanvas->b_box.xmin = x;
  if ( x>ctxcanvas->b_box.xmax ) ctxcanvas->b_box.xmax = x;
  if ( y<ctxcanvas->b_box.ymin ) ctxcanvas->b_box.ymin = y;
  if ( y>ctxcanvas->b_box.ymax ) ctxcanvas->b_box.ymax = y;
}


/*
%F metafile descriptor elements
*/
static void metafile_descriptor (cdCtxCanvas *ctxcanvas)
{
  const char *font_list[] = { "SYSTEM", "COURIER", "TIMES_ROMAN", "HELVETICA", 
    "SYSTEM_BOLD", "COURIER_BOLD", "TIMES_ROMAN_BOLD", "HELVETICA_BOLD",
    "SYSTEM_ITALIC", "COURIER_ITALIC", "TIMES_ROMAN_ITALIC",
    "HELVETICA_ITALIC", "SYSTEM_BOLDITALIC", "COURIER_BOLDITALIC",
    "TIMES_ROMAN_BOLDITALIC", "HELVETICA_BOLDITALIC", NULL };
  
  cgm_metafile_version        ( ctxcanvas->cgm, 1);
  cgm_metafile_description    ( ctxcanvas->cgm, "CD generated" );
  cgm_vdc_type                ( ctxcanvas->cgm, 0 /* integer */ );
  cgm_integer_precision       ( ctxcanvas->cgm, 16 );
  cgm_real_precision          ( ctxcanvas->cgm, 3 /* fixed 32 */ );
  cgm_index_precision         ( ctxcanvas->cgm, 16 );
  cgm_colour_precision        ( ctxcanvas->cgm, 8 );
  cgm_colour_index_precision  ( ctxcanvas->cgm, 8 );
  cgm_maximum_colour_index    ( ctxcanvas->cgm, 255ul );
  cgm_colour_value_extent     ( ctxcanvas->cgm, cve_black, cve_white );
  
  {
    static int classes[] = { -1 /* drawing set */ };
    static int ids    [] = {  1 /* plus control set */ };
    cgm_metafile_element_list   ( ctxcanvas->cgm, 1, classes, ids );
  }
  
  cgm_begin_metafile_defaults ( ctxcanvas->cgm );
  
  cgm_vdc_integer_precision ( ctxcanvas->cgm, ctxcanvas->vdc_int_prec );
  cgm_interior_style  ( ctxcanvas->cgm, 1 );  /* SOLID */
  cgm_edge_visibility  ( ctxcanvas->cgm, 0 );  /* OFF */
  
  cgm_end_metafile_defaults   ( ctxcanvas->cgm );
  
  cgm_font_list ( ctxcanvas->cgm, font_list );
}

/*
%F Pictire descriptor elements
*/
static void picture_descriptor (cdCtxCanvas *ctxcanvas)
{
  cgm_scaling_mode             ( ctxcanvas->cgm, 1 /* metric */, 1.0f );
  cgm_colour_selection_mode    ( ctxcanvas->cgm, 1 /* direct */ );
  cgm_line_width_specify_mode  ( ctxcanvas->cgm, 0 /* absolute=0, scaled=1 */ );
  cgm_marker_size_specify_mode ( ctxcanvas->cgm, 0 /* absolute=0, scaled=1 */ );
  
  ctxcanvas->point = ftell ( ctxcanvas->cgm->file );
  if ( ctxcanvas->codificacao == CD_CLEAR_TEXT )
  {
    fprintf ( ctxcanvas->cgm->file, "%80s\n", "" );
    fprintf ( ctxcanvas->cgm->file, "%80s\n", "" );
  }
  else
  {
    cgm_vdc_extent( ctxcanvas->cgm, 0, 0, (double)ctxcanvas->canvas->w, (double)ctxcanvas->canvas->h);
  }
}

/*
%F Control descriptor elements
*/
static void control_elements (cdCtxCanvas *ctxcanvas)
{
  double c[3] = {1.0,1.0,1.0};
  
  cgm_vdc_integer_precision ( ctxcanvas->cgm, ctxcanvas->vdc_int_prec );
  cgm_vdc_real_precision    ( ctxcanvas->cgm, 2 /* fixed 32 */ );
  cgm_auxiliary_colour      ( ctxcanvas->cgm, c );
}

static void cdkillcanvas(cdCtxCanvas *ctxcanvas)
{
  long pt;
  
  pt = ftell ( ctxcanvas->cgm->file );
  fseek ( ctxcanvas->cgm->file, ctxcanvas->point, SEEK_SET );

  if (ctxcanvas->hassize)
   cgm_vdc_extent  ( ctxcanvas->cgm, 0, 0, (double)ctxcanvas->canvas->w, (double)ctxcanvas->canvas->h);
  else
  {
   if ( ctxcanvas->clip.first )
    cgm_vdc_extent ( ctxcanvas->cgm, ctxcanvas->b_box.xmin, ctxcanvas->b_box.ymin, ctxcanvas->b_box.xmax, ctxcanvas->b_box.ymax );
   else
    cgm_vdc_extent ( ctxcanvas->cgm, ctxcanvas->clip.bbox.xmin, ctxcanvas->clip.bbox.ymin, ctxcanvas->clip.bbox.xmax, ctxcanvas->clip.bbox.ymax );
  }

  fseek ( ctxcanvas->cgm->file, pt, SEEK_SET );
  
  cgm_end_picture ( ctxcanvas->cgm );
  cgm_end_metafile ( ctxcanvas->cgm );
  
  memset(ctxcanvas, 0, sizeof(cdCtxCanvas));
  free(ctxcanvas);
}

static void cddeactivate(cdCtxCanvas *ctxcanvas)
{
  fflush(ctxcanvas->cgm->file);
}

/*
%F Comeca uma nova pagina.
*/
static void cdflush(cdCtxCanvas *ctxcanvas)
{
  long pt;

  fflush(ctxcanvas->cgm->file);
  
  pt = ftell ( ctxcanvas->cgm->file );
  fseek ( ctxcanvas->cgm->file, ctxcanvas->point, SEEK_SET );
  
  if (ctxcanvas->hassize)
   cgm_vdc_extent     ( ctxcanvas->cgm, 0, 0, (double)ctxcanvas->canvas->w, (double)ctxcanvas->canvas->h);
  else
  {
   if ( ctxcanvas->clip.first )
    cgm_vdc_extent ( ctxcanvas->cgm, ctxcanvas->b_box.xmin, ctxcanvas->b_box.ymin,
                     ctxcanvas->b_box.xmax, ctxcanvas->b_box.ymax );
   else
     cgm_vdc_extent ( ctxcanvas->cgm, ctxcanvas->clip.bbox.xmin, ctxcanvas->clip.bbox.ymin,
                      ctxcanvas->clip.bbox.xmax, ctxcanvas->clip.bbox.ymax );
  }
 
  fseek ( ctxcanvas->cgm->file, pt, SEEK_SET );
  
  cgm_end_picture        ( ctxcanvas->cgm );

  cgm_begin_picture      ( ctxcanvas->cgm, "Picture x" );
  picture_descriptor ( ctxcanvas);
  cgm_begin_picture_body ( ctxcanvas->cgm );
}


/******************************************************/
/* coordinate transformation                          */
/******************************************************/

static int cdclip(cdCtxCanvas *ctxcanvas, int mode)
{
  if (mode == CD_CLIPPOLYGON)
    return ctxcanvas->canvas->clip_mode;

  cgm_clip_indicator ( ctxcanvas->cgm, mode );

  if (mode == CD_CLIPAREA)
    cgm_clip_rectangle ( ctxcanvas->cgm, (double) ctxcanvas->canvas->clip_rect.xmin, (double) ctxcanvas->canvas->clip_rect.ymin,
                                          (double) ctxcanvas->canvas->clip_rect.xmax, (double) ctxcanvas->canvas->clip_rect.ymax );

  return mode;
}

static void clip_bbox (cdCtxCanvas *ctxcanvas, double xmin, double ymin, double xmax, double ymax)
{
  if ( ctxcanvas->clip.first )
  {
    ctxcanvas->clip.bbox.xmin = xmin;
    ctxcanvas->clip.bbox.xmax = xmax;
    ctxcanvas->clip.bbox.ymin = ymin;
    ctxcanvas->clip.bbox.ymax = ymax;
    ctxcanvas->clip.first = 0;
  }
  
  if ( xmin < ctxcanvas->clip.bbox.xmin ) ctxcanvas->clip.bbox.xmin = xmin;
  if ( ymin < ctxcanvas->clip.bbox.ymin ) ctxcanvas->clip.bbox.ymin = ymin;
  if ( xmax > ctxcanvas->clip.bbox.xmax ) ctxcanvas->clip.bbox.xmax = xmax;
  if ( ymax > ctxcanvas->clip.bbox.ymax ) ctxcanvas->clip.bbox.ymax = ymax;
}

static void cdcliparea(cdCtxCanvas *ctxcanvas, int xmin, int xmax, int ymin, int ymax)
{
  if (ctxcanvas->canvas->clip_mode == CD_CLIPAREA)
    cgm_clip_rectangle ( ctxcanvas->cgm, (double) xmin, (double) ymin,
                                         (double) xmax, (double) ymax );
  
  clip_bbox (ctxcanvas,  (double)xmin, (double)ymin, (double)xmax, (double)ymax );
}

static void cdfcliparea(cdCtxCanvas *ctxcanvas, double xmin, double xmax, double ymin, double ymax)
{
  if (ctxcanvas->canvas->clip_mode == CD_CLIPAREA)
    cgm_clip_rectangle ( ctxcanvas->cgm, xmin, ymin, xmax, ymax );
  
  clip_bbox (ctxcanvas, xmin, ymin, xmax, ymax );
}

/******************************************************/
/* primitives                                         */
/******************************************************/

static int cdinteriorstyle (cdCtxCanvas *ctxcanvas, int style);

static void cdline(cdCtxCanvas *ctxcanvas, int px1, int py1, int px2, int py2)
{
  double points[4];

  points[0] = (double)px1;
  points[1] = (double)py1;
  points[2] = (double)px2;
  points[3] = (double)py2;
  
  cgm_polyline( ctxcanvas->cgm, 2, points);
  
  setbbox (ctxcanvas,  points[0], points[1] );
  setbbox (ctxcanvas,  points[2], points[3] );
}

static void cdfline(cdCtxCanvas *ctxcanvas, double px1, double py1, double px2, double py2)
{
  double points[4];

  points[0] = px1;
  points[1] = py1;
  points[2] = px2;
  points[3] = py2;
  
  cgm_polyline( ctxcanvas->cgm, 2, points);
  
  setbbox (ctxcanvas,  points[0], points[1] );
  setbbox (ctxcanvas,  points[2], points[3] );
}

static void cdrect(cdCtxCanvas *ctxcanvas, int xmin, int xmax, int ymin, int ymax)
{
  double points[4];
  
  points[0] = (double)xmin;
  points[1] = (double)ymin;
  points[2] = (double)xmax;
  points[3] = (double)ymax;
  
  cgm_interior_style ( ctxcanvas->cgm, HOLLOW);
  cgm_rectangle( ctxcanvas->cgm, points);
  cdinteriorstyle(ctxcanvas, ctxcanvas->canvas->interior_style);
  
  setbbox (ctxcanvas,  points[0], points[1] );
  setbbox (ctxcanvas,  points[2], points[1] );
  setbbox (ctxcanvas,  points[2], points[3] );
  setbbox (ctxcanvas,  points[0], points[3] );
}

static void cdfrect(cdCtxCanvas *ctxcanvas, double xmin, double xmax, double ymin, double ymax)
{
  double points[4];
  
  points[0] = xmin;
  points[1] = ymin;
  points[2] = xmax;
  points[3] = ymax;
  
  cgm_interior_style ( ctxcanvas->cgm, HOLLOW);
  cgm_rectangle( ctxcanvas->cgm, points);
  cdinteriorstyle(ctxcanvas, ctxcanvas->canvas->interior_style);
  
  setbbox (ctxcanvas,  points[0], points[1] );
  setbbox (ctxcanvas,  points[2], points[1] );
  setbbox (ctxcanvas,  points[2], points[3] );
  setbbox (ctxcanvas,  points[0], points[3] );
}

static void cdbox(cdCtxCanvas *ctxcanvas, int xmin, int xmax, int ymin, int ymax)
{
  double points[4];
  
  points[0] = (double)xmin;
  points[1] = (double)ymin;
  points[2] = (double)xmax;
  points[3] = (double)ymax;
  
  cgm_rectangle( ctxcanvas->cgm, points);
  
  setbbox (ctxcanvas,  points[0], points[1] );
  setbbox (ctxcanvas,  points[2], points[1] );
  setbbox (ctxcanvas,  points[2], points[3] );
  setbbox (ctxcanvas,  points[0], points[3] );
}

static void cdfbox(cdCtxCanvas *ctxcanvas, double xmin, double xmax, double ymin, double ymax)
{
  double points[4];
  
  points[0] = xmin;
  points[1] = ymin;
  points[2] = xmax;
  points[3] = ymax;
  
  cgm_rectangle( ctxcanvas->cgm, points);
  
  setbbox (ctxcanvas,  points[0], points[1] );
  setbbox (ctxcanvas,  points[2], points[1] );
  setbbox (ctxcanvas,  points[2], points[3] );
  setbbox (ctxcanvas,  points[0], points[3] );
}

static void arc (cdCtxCanvas *ctxcanvas,  double xc, double yc, double w, double h, double a1, double a2,
                 double *center, double *first_end_point,
                 double *second_end_point, double *dx_start, double *dy_start,
                 double *dx_end, double *dy_end )
{
  double width, height;
  
  center[0] = xc;
  center[1] = yc;
  
  width = w/2;
  height = h/2;
  
  first_end_point[0] = center[0] + width;
  first_end_point[1] = center[1];
  
  second_end_point[0] = center[0];
  second_end_point[1] = center[1] + height;
  
  *dx_start = width*cos(a1*CD_DEG2RAD);
  *dy_start = height*sin(a1*CD_DEG2RAD);
  
  *dx_end = width*cos(a2*CD_DEG2RAD);
  *dy_end = height*sin(a2*CD_DEG2RAD);
  
  setbbox (ctxcanvas,  center[0]-width, center[1]-height );
  setbbox (ctxcanvas,  center[0]+width, center[1]-height );
  setbbox (ctxcanvas,  center[0]+width, center[1]+height );
  setbbox (ctxcanvas,  center[0]-width, center[1]+height );
}

static void cdarc(cdCtxCanvas *ctxcanvas, int xc, int yc, int w, int h, double a1, double a2)
{
  double center[2], first_end_point[2], second_end_point[2];
  double dx_start, dy_start, dx_end, dy_end;
  
  arc (ctxcanvas, (double)xc, (double)yc, (double)w, (double)h, a1, a2, center, first_end_point, second_end_point,
       &dx_start, &dy_start, &dx_end, &dy_end );
  
  cgm_elliptical_arc ( ctxcanvas->cgm, center, first_end_point, second_end_point, dx_start, dy_start, dx_end, dy_end );
}

static void cdfarc(cdCtxCanvas *ctxcanvas, double xc, double yc, double w, double h, double a1, double a2)
{
  double center[2], first_end_point[2], second_end_point[2];
  double dx_start, dy_start, dx_end, dy_end;
  
  arc (ctxcanvas, xc, yc, w, h, a1, a2, center, first_end_point, second_end_point,
       &dx_start, &dy_start, &dx_end, &dy_end );
  
  cgm_elliptical_arc ( ctxcanvas->cgm, center, first_end_point, second_end_point, dx_start, dy_start, dx_end, dy_end );
}

static void cdsector(cdCtxCanvas *ctxcanvas, int xc, int yc, int w, int h, double a1, double a2)
{
  double center[2], first_end_point[2], second_end_point[2];
  double dx_start, dy_start, dx_end, dy_end;
  
  arc (ctxcanvas, (double)xc, (double)yc, (double)w, (double)h, a1, a2, center, first_end_point, second_end_point,
       &dx_start, &dy_start, &dx_end, &dy_end );
  
  
  cgm_elliptical_arc_close ( ctxcanvas->cgm, center, first_end_point, second_end_point,
                             dx_start, dy_start, dx_end, dy_end, 0 );
  
  setbbox (ctxcanvas,  (double)xc-w/2., (double)yc-h/2. );
  setbbox (ctxcanvas,  (double)xc+w/2., (double)yc-h/2. );
  setbbox (ctxcanvas,  (double)xc+w/2., (double)yc+h/2. );
  setbbox (ctxcanvas,  (double)xc-w/2., (double)yc+h/2. );
}

static void cdfsector(cdCtxCanvas *ctxcanvas, double xc, double yc, double w, double h, double a1, double a2)
{
  double center[2], first_end_point[2], second_end_point[2];
  double dx_start, dy_start, dx_end, dy_end;
  
  arc (ctxcanvas, xc, yc, w, h, a1, a2, center, first_end_point, second_end_point,
       &dx_start, &dy_start, &dx_end, &dy_end );
  
  
  cgm_elliptical_arc_close ( ctxcanvas->cgm, center, first_end_point, second_end_point,
                             dx_start, dy_start, dx_end, dy_end, 0 );
  
  setbbox (ctxcanvas,  xc-w/2., yc-h/2. );
  setbbox (ctxcanvas,  xc+w/2., yc-h/2. );
  setbbox (ctxcanvas,  xc+w/2., yc+h/2. );
  setbbox (ctxcanvas,  xc-w/2., yc+h/2. );
}

static void settextbbox (cdCtxCanvas *ctxcanvas,  double x, double y, int width, int height )
{
  switch ( ctxcanvas->canvas->text_alignment )
  {
  case CD_NORTH:
    setbbox (ctxcanvas,  x-(width/2.), y-height );
    setbbox (ctxcanvas,  x+(width/2.), y );
    break;
  case CD_SOUTH:
    setbbox (ctxcanvas,  x-(width/2.), y+height );
    setbbox (ctxcanvas,  x+(width/2.), y );
    break;
  case CD_EAST:
    setbbox (ctxcanvas,  x-width, y-(height/2.) );
    setbbox (ctxcanvas,  x, y+(height/2.) );
    break;
  case CD_WEST:
    setbbox (ctxcanvas,  x, y-(height/2.) );
    setbbox (ctxcanvas,  x+width, y+(height/2.) );
    break;
  case CD_NORTH_EAST:
    setbbox (ctxcanvas,  x-width, y-height );
    setbbox (ctxcanvas,  x, y );
    break;
  case CD_NORTH_WEST:
    setbbox (ctxcanvas,  x, y-height );
    setbbox (ctxcanvas,  x+width, y );
    break;
  case CD_SOUTH_EAST:
    setbbox (ctxcanvas,  x-width, y );
    setbbox (ctxcanvas,  x, y+height );
    break;
  case CD_SOUTH_WEST:
    setbbox (ctxcanvas,  x, y );
    setbbox (ctxcanvas,  x+width, y+height );
    break;
  case CD_CENTER:
    setbbox (ctxcanvas,  x-(width/2.), y-(height/2.) );
    setbbox (ctxcanvas,  x+(width/2.), y+(height/2.) );
    break;
  case CD_BASE_LEFT:
    setbbox (ctxcanvas,  x, y );
    setbbox (ctxcanvas,  x+width, y+height );
    break;
  case CD_BASE_CENTER:
    setbbox (ctxcanvas,  x-(width/2.), y );
    setbbox (ctxcanvas,  x+(width/2.), y+height );
    break;
  case CD_BASE_RIGHT:
    setbbox (ctxcanvas,  x-width, y );
    setbbox (ctxcanvas,  x, y+height );
    break;
  }
}

static void cdtext(cdCtxCanvas *ctxcanvas, int x, int y, const char *s, int len)
{
  int width, height;
  
  cgm_text( ctxcanvas->cgm, 1 /* final */ , (double)x, (double)y, s, len );
  
  cdgettextsizeEX(ctxcanvas, s, len, &width, &height);
  
  settextbbox (ctxcanvas, (double) x, (double) y, width, height );
}

static void cdftext(cdCtxCanvas *ctxcanvas, double x, double y, const char *s, int len)
{
  int width, height;
  
  cgm_text( ctxcanvas->cgm, 1 /* final */ , x, y, s, len);
  
  cdgettextsizeEX(ctxcanvas, s, len, &width, &height);
  
  settextbbox (ctxcanvas, x, y, width, height );
}

static void cdpoly(cdCtxCanvas *ctxcanvas, int mode, cdPoint* poly, int n)
{
  int i;
  double *fpoly;
  
  fpoly = (double *)malloc(2 * (n+1) * sizeof(double));
  
  for (i = 0; i < n; i++)
  {
    fpoly[2*i] = (double) poly[i].x;
    fpoly[2*i+1] = (double) poly[i].y;

    setbbox (ctxcanvas, fpoly[2*i] , fpoly[2*i+1] );
  }

  switch ( mode )
  {
  case CD_OPEN_LINES:
    cgm_polyline( ctxcanvas->cgm, n, fpoly );
    break;
  case CD_CLOSED_LINES:
    fpoly[2*n] = fpoly[0];
    fpoly[2*n+1] = fpoly[1];
    n++;
    cgm_polyline( ctxcanvas->cgm, n, fpoly );
    break;
  case CD_FILL:
    cgm_polygon( ctxcanvas->cgm, n, fpoly);
    break;
  case CD_BEZIER:
    cdfSimPolyBezier(ctxcanvas->canvas, (cdfPoint*)fpoly, n);
    break;
  case CD_PATH:
    cdfSimPolyPath(ctxcanvas->canvas, (cdfPoint*)fpoly, n);
    break;
  }

  free(fpoly);
}

static void cdfpoly(cdCtxCanvas *ctxcanvas, int mode, cdfPoint* poly, int n)
{
  int i;
  double *fpoly = (double*)poly;
  
  for (i = 0; i < n; i++)
  {
    setbbox (ctxcanvas, fpoly[2*i] , fpoly[2*i+1] );
  }

  switch ( mode )
  {
  case CD_OPEN_LINES:
    cgm_polyline( ctxcanvas->cgm, n, fpoly );
    break;
  case CD_CLOSED_LINES:
    fpoly[2*n] = fpoly[0];
    fpoly[2*n+1] = fpoly[1];
    n++;
    cgm_polyline( ctxcanvas->cgm, n, fpoly );
    break;
  case CD_FILL:
    cgm_polygon( ctxcanvas->cgm, n, fpoly);
    break;
  case CD_BEZIER:
    cdfSimPolyBezier(ctxcanvas->canvas, poly, n);
    break;
  case CD_PATH:
    cdfSimPolyPath(ctxcanvas->canvas, poly, n);
    break;
  }
}


/******************************************************/
/* attributes                                         */
/******************************************************/

static int cdlinestyle(cdCtxCanvas *ctxcanvas, int style)
{
  cgm_line_type( ctxcanvas->cgm, (long)(style + 1));
  return style;
}

static int cdlinewidth(cdCtxCanvas *ctxcanvas, int width)
{
  cgm_line_width( ctxcanvas->cgm, (double)width );
  return width;
}

static int cdinteriorstyle (cdCtxCanvas *ctxcanvas,  int style )
{
  switch ( style )
  {
  case CD_SOLID:
    style = 1;
    break;
  case CD_STIPPLE:
  case CD_PATTERN:
    style = 2;
    break;
  case CD_HATCH:
    style = 3;
    break;
  }
  
  cgm_interior_style ( ctxcanvas->cgm, style );
  
  return style;
}

static int cdhatch(cdCtxCanvas *ctxcanvas, int style)
{
  int cgm_style = style;

  if ( cgm_style==2 ) 
    cgm_style = 3;
  else if ( cgm_style==3 ) 
    cgm_style = 2;

  cgm_hatch_index ( ctxcanvas->cgm, (long)cgm_style+1 );

  cgm_interior_style ( ctxcanvas->cgm, 3 );

  return style;
}

static void cdstipple(cdCtxCanvas *ctxcanvas, int n, int m, const unsigned char *stipple)
{
  double *pattab;
  int i, j=0;

  pattab = (double *) malloc ( n*m*3*sizeof(double));
  
  for ( i=0; i<n*m; i++ )
  {
    pattab[j+0] = ( stipple[i] ) ? get_red(ctxcanvas->canvas->foreground) : get_red(ctxcanvas->canvas->background);
    pattab[j+1] = ( stipple[i] ) ? get_green(ctxcanvas->canvas->foreground) : get_green(ctxcanvas->canvas->background);
    pattab[j+2] = ( stipple[i] ) ? get_blue(ctxcanvas->canvas->foreground) : get_blue(ctxcanvas->canvas->background);
    j+=3;
  }
  
  cgm_pattern_table ( ctxcanvas->cgm, (long) ctxcanvas->patindex, (long) n, (long) m, (int) 8, pattab );
  cgm_pattern_index ( ctxcanvas->cgm, (long) ctxcanvas->patindex++ );
  free(pattab);
  
  cgm_interior_style  ( ctxcanvas->cgm, 2 );  /* PATTERN */
}

static void cdpattern(cdCtxCanvas *ctxcanvas, int n, int m, const long int *pattern)
{
  double *pattab;
  int i, j=0;

  pattab = (double *) malloc ( n*m*3*sizeof(double) );
  
  for ( i=0; i<n*m; i++ )
  {
    pattab[j+0] = get_red(pattern[i]);
    pattab[j+1] = get_green(pattern[i]);
    pattab[j+2] = get_blue(pattern[i]);
    j+=3;
  }
  
  cgm_pattern_table ( ctxcanvas->cgm, (long) ctxcanvas->patindex, (long) n, (long) m, (int) 8, pattab );
  cgm_pattern_index ( ctxcanvas->cgm, (long) ctxcanvas->patindex++ );
  free(pattab);
  
  cgm_interior_style  ( ctxcanvas->cgm, 2 );  /* PATTERN */
}

static int cdfont(cdCtxCanvas *ctxcanvas, const char *type_face, int style, int size)
{
  long index = 0;
  
  if (cdStrEqualNoCase(type_face, "System"))
    switch (style&3)
    {
    case CD_PLAIN:
      index = 1;
      break;
    case CD_BOLD:
      index = 5;
      break;
    case CD_ITALIC:
      index = 9;
      break;
    case CD_BOLD_ITALIC:
      index = 13;
      break;
    }
  else if (cdStrEqualNoCase(type_face, "Courier"))
    switch (style&3)
    {
    case CD_PLAIN:
      index = 2;
      break;
    case CD_BOLD:
      index = 6;
      break;
    case CD_ITALIC:
      index = 10;
      break;
    case CD_BOLD_ITALIC:
      index = 14;
      break;
    }
  else if (cdStrEqualNoCase(type_face, "Times"))
    switch (style&3)
    {
    case CD_PLAIN:
      index = 3;
      break;
    case CD_BOLD:
      index = 7;
      break;
    case CD_ITALIC:
      index = 11;
      break;
    case CD_BOLD_ITALIC:
      index = 15;
      break;
    }
  else if (cdStrEqualNoCase(type_face, "Helvetica"))
    switch (style&3)
    {
    case CD_PLAIN:
      index = 4;
      break;
    case CD_BOLD:
      index = 8;
      break;
    case CD_ITALIC:
      index = 12;
      break;
    case CD_BOLD_ITALIC:
      index = 16;
      break;
    }

  if (index == 0) return 0;
  
  cgm_char_height ( ctxcanvas->cgm, cdGetFontSizePixels(ctxcanvas->canvas, size));
  cgm_text_font_index( ctxcanvas->cgm, index );

  return 1;
}

static int cdtextalignment(cdCtxCanvas *ctxcanvas, int alignment)
{
  int hor = 0, ver = 0;
  enum { NORMHORIZ, LEFT, CTR, RIGHT };
  enum { NORMVERT, TOP, CAP, HALF, BASE, BOTTOM };
  
  switch ( alignment )
  {
  case CD_NORTH:
    hor = CTR;
    ver = TOP;
    break;
  case CD_SOUTH:
    hor = CTR;
    ver = BOTTOM;
    break;
  case CD_EAST:
    hor = RIGHT;
    ver = HALF;
    break;
  case CD_WEST:
    hor = LEFT;
    ver = HALF;
    break;
  case CD_NORTH_EAST:
    hor = RIGHT;
    ver = TOP;
    break;
  case CD_NORTH_WEST:
    hor = LEFT;
    ver = TOP;
    break;
  case CD_SOUTH_EAST:
    hor = RIGHT;
    ver = BOTTOM;
    break;
  case CD_SOUTH_WEST:
    hor = LEFT;
    ver = BOTTOM;
    break;
  case CD_CENTER:
    hor = CTR;
    ver = HALF;
    break;
  case CD_BASE_LEFT:
    hor = LEFT;
    ver = BASE;
    break;
  case CD_BASE_CENTER:
    hor = CTR;
    ver = BASE;
    break;
  case CD_BASE_RIGHT:
    hor = RIGHT;
    ver = BASE;
    break;
  }
  
  cgm_text_alignment ( ctxcanvas->cgm, hor, ver , (double)0.0, (double)0.0 );
  
  return alignment;
}

/******************************************************/
/* color                                              */
/******************************************************/

static long int cdforeground(cdCtxCanvas *ctxcanvas, long int color)
{
  double cor[3];
  
  cor[0] = get_red(color);
  cor[1] = get_green(color);
  cor[2] = get_blue(color);
  
  cgm_text_colour( ctxcanvas->cgm, cor );
  cgm_fill_colour( ctxcanvas->cgm, cor );
  cgm_line_colour( ctxcanvas->cgm, cor );
  
  return color;
}

static long int cdbackground(cdCtxCanvas *ctxcanvas, long int color)
{
  double bc[3];
  
  bc[0] = get_red(color);
  bc[1] = get_green(color);
  bc[2] = get_blue(color);
  
  ctxcanvas->canvas->background = color;
  cgm_backgound_colour ( ctxcanvas->cgm, bc );
  
  return color;
}

static int cdbackopacity(cdCtxCanvas *ctxcanvas, int opaque)
{
  if (opaque == CD_TRANSPARENT)
    cgm_transparency(ctxcanvas->cgm, 1);
  else
    cgm_transparency(ctxcanvas->cgm, 0);
  return opaque;
}

/******************************************************/
/* client images                                      */
/******************************************************/

static void cdputimagerectrgb(cdCtxCanvas *ctxcanvas, int iw, int ih, const unsigned char *r, const unsigned char *g, const unsigned char *b, int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax)
{
  double p[6];
  double  *color_array;
  int i,j,index,c;
  int rw, rh;

  rw = xmax-xmin+1;
  rh = ymax-ymin+1;
  
  color_array = (double *) malloc ( rw*rh*3*sizeof(double) );
  if (!color_array)
    return;
  
  p[0] = (double) x;      p[1] = (double) (y+h);
  p[2] = (double) (x+w);  p[3] = (double) y;
  p[4] = (double) (x+w);  p[5] = (double) (y+h);
  
  for ( i=0; i<rh; i++ )
  {
    for ( j=0; j<rw; j++ )
    {
      index = (ih-i-1-ymin)*iw+j+xmin;
      c = i*rw*3+j*3;
      color_array[c]   = (double) r[index]/255.;
      color_array[c+1] = (double) g[index]/255.;
      color_array[c+2] = (double) b[index]/255.;
    }
  }
    
  cgm_cell_array ( ctxcanvas->cgm, p, (long)rw, (long)rh, 8, color_array );
  
  free(color_array);
  
  setbbox (ctxcanvas,  p[0], p[1] );
  setbbox (ctxcanvas,  p[2], p[3] );
}

static void cdputimagerectmap(cdCtxCanvas *ctxcanvas, int iw, int ih, const unsigned char *index, const long int *colors, int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax)
{
  double p[6];
  double *color_array;
  int i,j,c;
  unsigned char r, g, b;
  int rw, rh;

  rw = xmax-xmin+1;
  rh = ymax-ymin+1;
  
  color_array = (double *) malloc ( rw*rh*3*sizeof(double) );
  if (!color_array)
    return;
  
  p[0] = (double) x;      p[1] = (double) y;
  p[2] = (double) (x+w);  p[3] = (double) (y+h);
  p[4] = (double) (x+w);  p[5] = (double) y;
  
  for ( i=0; i<rh; i++ )
  {
    for ( j=0; j<rw; j++ )
    {
      c = i*rw*3+j*3;
      cdDecodeColor(colors[index[(ih-i-1-ymin)*iw+j+xmin]], &r,&b,&g);
      color_array[c]   = ((double)r)/255.;
      color_array[c+1] = ((double)g)/255.;
      color_array[c+2] = ((double)b)/255.;
    }
  }
    
  cgm_cell_array ( ctxcanvas->cgm, p, (long)rw, (long)rh, 8, color_array );
  
  free(color_array);
  
  setbbox (ctxcanvas, p[0], p[1] );
  setbbox (ctxcanvas, p[2], p[3] );
}

/******************************************************/
/* server images                                      */
/******************************************************/

static void cdpixel(cdCtxCanvas *ctxcanvas, int x, int y, long int color)
{
  double cor[3];
  double pts[2];

  pts[0] = (double) x;
  pts[1] = (double) y;
  
  cor[0] = get_red(color);
  cor[1] = get_green(color);
  cor[2] = get_blue(color);

  cgm_marker_colour( ctxcanvas->cgm, cor);
  cgm_polymarker ( ctxcanvas->cgm, 1, pts );
}

/*
%F Cria um canvas CGM.
Parametros passados em data:
[nome]   nome do arquivo de saida <= 255 caracteres
[size]   tamanho do papel
-t   codificacao clear text se nao binaria
*/
static void cdcreatecanvas(cdCanvas* canvas, void *data)
{
  cdCtxCanvas *ctxcanvas;
  char *line = (char *)data;
  char c;
  char words[4][256];
  char filename[10240] = "";
  double w=0, h=0, r=0;
  int p=0;
  int i, n;

  line += cdGetFileName(line, filename);
  if (filename[0] == 0)
    return;
  
  n = sscanf(line, "%s %s %s %s", words[0], words[1], words[2], words[3]);
  
  ctxcanvas = (cdCtxCanvas *)malloc(sizeof(cdCtxCanvas));

  canvas->ctxcanvas = ctxcanvas;
  ctxcanvas->canvas = canvas;

  strcpy(ctxcanvas->filename, filename);
  
  canvas->w_mm = (double)INT_MAX*3.78;
  canvas->h_mm = (double)INT_MAX*3.78;
  canvas->xres = 3.78;
  canvas->yres = 3.78;
  canvas->bpp = 24;

  ctxcanvas->vdc_int_prec = 16;
  ctxcanvas->codificacao = CD_BIN;
  ctxcanvas->first = 1;
  ctxcanvas->clip.first = 1;
  ctxcanvas->patindex = 1;
  
  ctxcanvas->hassize = 0;		/* Indica se foi passado um tamanho para o canvas */
  
  for (i = 0; i < n; i++)
  {
    if (sscanf ( words[i], "%lgx%lg",  &w, &h )== 2)
    {
      canvas->w_mm = w;
      canvas->h_mm = h;
      ctxcanvas->hassize = 1;
    }
    else if (sscanf ( words[i], "%lg", &r ) == 1)
      canvas->yres = canvas->xres = r;
    else if (sscanf ( words[i], "-%c%d", &c, &p )>0)
    {
      if ( c=='t' ) 
        ctxcanvas->codificacao = CD_CLEAR_TEXT;
      else if ( c=='p' ) 
        ctxcanvas->vdc_int_prec = p;
    }
  }
  
  if ( ctxcanvas->vdc_int_prec != 16 && w == 0.0 && h == 0.0 )
  {
    canvas->w_mm = (double) (pow(2,p)/2)-1;
    canvas->h_mm = (double) (pow(2,p)/2)-1;
  }
  
  /* update canvas context */
  canvas->w = (int)(canvas->w_mm * canvas->xres);
  canvas->h = (int)(canvas->h_mm * canvas->yres);
  
  ctxcanvas->cgm = cgm_begin_metafile ( ctxcanvas->filename, ctxcanvas->codificacao, "CD - CanvasDraw, Tecgraf/PUC-RIO" );
  
  metafile_descriptor(ctxcanvas);
  
  cgm_begin_picture ( ctxcanvas->cgm, "Picture x" );

  picture_descriptor (ctxcanvas);

  cgm_clip_rectangle ( ctxcanvas->cgm, 0, 0, (double)canvas->w, (double)canvas->h);
  cgm_clip_indicator (ctxcanvas->cgm, 0);

  cgm_begin_picture_body ( ctxcanvas->cgm );
  
  control_elements (ctxcanvas);

  cgm_marker_type( ctxcanvas->cgm, MARKER_DOT);
  cgm_marker_size( ctxcanvas->cgm, 1.0);
}

static void cdinittable(cdCanvas* canvas)
{
  /* initialize function table*/
  canvas->cxFlush = cdflush;
  canvas->cxPixel = cdpixel;
  canvas->cxLine = cdline;
  canvas->cxPoly = cdpoly;
  canvas->cxRect = cdrect;
  canvas->cxBox = cdbox;
  canvas->cxArc = cdarc;
  canvas->cxSector = cdsector;
  canvas->cxText = cdtext;
  canvas->cxFLine = cdfline;
  canvas->cxFPoly = cdfpoly;
  canvas->cxFRect = cdfrect;
  canvas->cxFBox = cdfbox;
  canvas->cxFArc = cdfarc;
  canvas->cxFSector = cdfsector;
  canvas->cxFText = cdftext;
  canvas->cxPutImageRectRGB = cdputimagerectrgb;
  canvas->cxPutImageRectMap = cdputimagerectmap;

  canvas->cxClip = cdclip;
  canvas->cxClipArea = cdcliparea;
  canvas->cxFClipArea = cdfcliparea;
  canvas->cxLineStyle = cdlinestyle;
  canvas->cxLineWidth = cdlinewidth;
  canvas->cxInteriorStyle = cdinteriorstyle;
  canvas->cxHatch = cdhatch;
  canvas->cxStipple = cdstipple;
  canvas->cxPattern = cdpattern;
  canvas->cxFont = cdfont;
  canvas->cxTextAlignment = cdtextalignment;
  canvas->cxBackground = cdbackground;
  canvas->cxForeground = cdforeground;
  canvas->cxBackOpacity = cdbackopacity;

  canvas->cxKillCanvas = cdkillcanvas;
  canvas->cxDeactivate = cddeactivate;
}

/******************************************************/

static cdContext cdCGMContext =
{
  CD_CAP_ALL & ~(CD_CAP_CLEAR | CD_CAP_PALETTE | 
                 CD_CAP_CLIPPOLY | CD_CAP_WRITEMODE |  CD_CAP_IMAGESRV | 
                 CD_CAP_LINECAP | CD_CAP_LINEJOIN | CD_CAP_REGION | CD_CAP_CHORD |
                 CD_CAP_FONTDIM | CD_CAP_TEXTSIZE | 
                 CD_CAP_IMAGERGBA | CD_CAP_GETIMAGERGB | 
                 CD_CAP_TEXTORIENTATION | CD_CAP_PATH | CD_CAP_BEZIER),
  0,
  cdcreatecanvas,
  cdinittable,
  cdplayCGM,
  cdRegisterCallbackCGM,
};

cdContext* cdContextCGM(void)
{
  return &cdCGMContext;
}

