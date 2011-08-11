#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <cd.h>

#include "list.h"
#include "types.h"
#include "intcgm.h"
#include "intcgm2.h"
#include "intcgm6.h"

void cgm_setlinestyle ( int type )
{
  switch ( type )
  {
  case LINE_SOLID:
    cdCanvasLineStyle (intcgm_canvas,  CD_CONTINUOUS );
    break;
  case LINE_DASH:
    cdCanvasLineStyle (intcgm_canvas,  CD_DASHED );
    break;
  case LINE_DOT:
    cdCanvasLineStyle (intcgm_canvas,  CD_DOTTED );
    break;
  case LINE_DASH_DOT:
    cdCanvasLineStyle (intcgm_canvas,  CD_DASH_DOT );
    break;
  case LINE_DASH_DOT_DOT:
    cdCanvasLineStyle (intcgm_canvas,  CD_DASH_DOT_DOT );
    break;
  }
}

void cgm_setlinewidth ( double width )
{
  int w;
  if ( intcgm_cgm.lnwsm == ABSOLUTE )
    w = cgm_delta_vdc2canvas(width);
  else
    w = (int)floor ( width+0.5 );

  cdCanvasLineWidth (intcgm_canvas,  w>0?w:1 );
}

void cgm_setmarktype ( int type )
{
  switch ( type )
  {
  case MARK_DOT:
    cdCanvasMarkType (intcgm_canvas,  CD_STAR );
    break;
  case MARK_PLUS:
    cdCanvasMarkType (intcgm_canvas,  CD_PLUS );
    break;
  case MARK_ASTERISK:
    cdCanvasMarkType (intcgm_canvas,  CD_X );
    break;
  case MARK_CIRCLE:
    cdCanvasMarkType (intcgm_canvas,  CD_CIRCLE );
    break;
  case MARK_CROSS:
    cdCanvasMarkType (intcgm_canvas,  CD_PLUS );
    break;
  }
}

void cgm_setmarksize ( double size )
{
  if ( intcgm_cgm.lnwsm == ABSOLUTE )
    cdCanvasMarkSize (intcgm_canvas,  cgm_delta_vdc2canvas(size) );
  else
    cdCanvasMarkSize (intcgm_canvas,  (int)floor ( size*0.5 ) );
}

long int *cgm_setpattern ( pat_table pat )
{
  int i;
  long int *cor = (long int *) malloc ( pat.nx*pat.ny*sizeof(long int) );
  
  for ( i=0; i<pat.nx*pat.ny ; i++ )
    cor[i] = cgm_getcolor ( pat.pattern[i] );
  
  return cor;
}

int cgm_setintstyle ( int style )
{
  if ( style==HOLLOW )
    return CD_CLOSED_LINES;
  else if ( style==SOLID )
  {
    cdCanvasInteriorStyle (intcgm_canvas,  CD_SOLID );
    return CD_FILL;
  }
  else if ( style==PATTERN )
  {
    int i;
    pat_table *pat;
    long int *p;
    
    for ( i=1; (pat=(pat_table *)cgm_GetList( intcgm_fill_att.pat_list,i ))!=NULL; i++ )
    {
      if ( pat->index==intcgm_fill_att.pat_index ) break;
    }
    
    p = (long int *) malloc ( pat->nx*pat->ny*sizeof(long int) );
    
    for ( i=0; i<pat->nx*pat->ny; i++ )
    {
      if ( intcgm_cgm.clrsm==DIRECT )
        p[i] = cdEncodeColor ((unsigned char)(pat->pattern[i].rgb.red*255./intcgm_cgm.color_ext.white.red),
                              (unsigned char)(pat->pattern[i].rgb.green*255./intcgm_cgm.color_ext.white.green),
                              (unsigned char)(pat->pattern[i].rgb.blue*255./intcgm_cgm.color_ext.white.blue) );
      else
        p[i] = cdEncodeColor ((unsigned char)(intcgm_color_table[pat->pattern[i].ind].red*255/intcgm_cgm.color_ext.white.red),
                              (unsigned char)(intcgm_color_table[pat->pattern[i].ind].green*255/intcgm_cgm.color_ext.white.green),
                              (unsigned char)(intcgm_color_table[pat->pattern[i].ind].blue*255/intcgm_cgm.color_ext.white.blue) );
    }
    
    cdCanvasPattern(intcgm_canvas,  pat->nx, pat->ny, (long *) p );
    
    return CD_FILL;
  }
  else if ( style==HATCH )
  {
    cdCanvasHatch (intcgm_canvas,  intcgm_fill_att.hatch_index-1 );
    return CD_FILL;
  }
  else
    return CD_CLOSED_LINES;
}

long int cgm_getcolor ( tcolor cor )
{
  
  if ( intcgm_cgm.clrsm==INDEXED )
    return cdEncodeColor ((unsigned char)(intcgm_color_table[cor.ind].red*255/intcgm_cgm.color_ext.white.red),
                          (unsigned char)(intcgm_color_table[cor.ind].green*255/intcgm_cgm.color_ext.white.green),
                          (unsigned char)(intcgm_color_table[cor.ind].blue*255/intcgm_cgm.color_ext.white.blue) );
  else
    return cdEncodeColor ((unsigned char)(cor.rgb.red*255/intcgm_cgm.color_ext.white.red),
                          (unsigned char)(cor.rgb.green*255/intcgm_cgm.color_ext.white.green),
                          (unsigned char)(cor.rgb.blue*255/intcgm_cgm.color_ext.white.blue) );
}

int cgm_vdcx2canvas ( double vdc )
{
  if ( intcgm_cgm.drawing_mode==ABSTRACT )
    return (int) floor ( intcgm_scale_factor_x*(vdc-intcgm_vdc_ext.xmin)+.5 ) + intcgm_view_xmin;
  else
    return (int) floor ( intcgm_scale_factor_mm_x*(vdc-intcgm_vdc_ext.xmin)*intcgm_cgm.scaling_mode.scale_factor +
                         intcgm_vdc_ext.xmin*intcgm_cgm.scaling_mode.scale_factor + .5 );
}

int cgm_vdcy2canvas ( double vdc )
{
  if ( intcgm_cgm.drawing_mode==ABSTRACT )
    return (int) floor ( intcgm_scale_factor_y*(vdc-intcgm_vdc_ext.ymin)+.5 ) + intcgm_view_ymin;
  else
    return (int) floor ( intcgm_scale_factor_mm_y*(vdc-intcgm_vdc_ext.ymin)*intcgm_cgm.scaling_mode.scale_factor +
                         intcgm_vdc_ext.ymin*intcgm_cgm.scaling_mode.scale_factor + .5 );
}

int cgm_delta_vdc2canvas ( double vdc )
{
  int delta = (int) cgm_vdcx2canvas(intcgm_vdc_ext.xmin+vdc) - (int) cgm_vdcx2canvas(intcgm_vdc_ext.xmin);
  return delta;
}

double cgm_canvas2vdcx ( int x )
{
  if ( intcgm_cgm.drawing_mode==ABSTRACT )
    return (double) (x-intcgm_view_xmin)/intcgm_scale_factor_x + intcgm_vdc_ext.xmin;
  else
    return (double) (x-intcgm_view_xmin)/(intcgm_scale_factor_mm_x*intcgm_cgm.scaling_mode.scale_factor) + intcgm_vdc_ext.xmin;
}

double cgm_canvas2vdcy ( int y )
{
  if ( intcgm_cgm.drawing_mode==ABSTRACT )
    return (double) (y-intcgm_view_ymin)/intcgm_scale_factor_y + intcgm_vdc_ext.ymin;
  else
    return (double) (y-intcgm_view_ymin)/(intcgm_scale_factor_mm_y*intcgm_cgm.scaling_mode.scale_factor) + intcgm_vdc_ext.ymin;
}

void cgm_getpolybbox ( tpoint *pt, int n_points, double *bb_xmin, double *bb_ymin,
                  double *bb_xmax, double *bb_ymax )
{
  int i;
  
  *bb_xmin = *bb_xmax = pt[0].x;
  *bb_ymin = *bb_ymax = pt[0].y;
  
  for ( i=1; i<n_points; i++)
  {
    if ( pt[i].x < *bb_xmin ) *bb_xmin = pt[i].x;
    else if ( pt[i].x > *bb_xmax ) *bb_xmax = pt[i].x;
    if ( pt[i].y < *bb_ymin ) *bb_ymin = pt[i].y;
    else if ( pt[i].y > *bb_ymax ) *bb_ymax = pt[i].y;
  }
}

void cgm_getincpolybbox ( tpoint *pt, int n_points, double *bb_xmin, double *bb_ymin,
                     double *bb_xmax, double *bb_ymax )
{
  int i;
  double px, py;
  
  px = *bb_xmin = *bb_xmax = pt[0].x;
  py = *bb_ymin = *bb_ymax = pt[0].y;
  
  for ( i=1; i<n_points; i++)
  {
    px += pt[i].x;   py += pt[i].y;
    if ( px < *bb_xmin ) *bb_xmin = px;
    else if ( px > *bb_xmax ) *bb_xmax = px;
    if ( py < *bb_ymin ) *bb_ymin = py;
    else if ( py > *bb_ymax ) *bb_ymax = py;
  }
}

int cgm_setfont ( int font, int style, double height )
{
  int size;
  int cy;
  char* type_face;
  
  cy = cgm_delta_vdc2canvas ( height );
  size = (int) floor (((cy/intcgm_scale_factor_mm_y)/0.353)+0.5);
  switch (font)
  {
  case 0:
    type_face = "System";
    break;
  case 1:
    type_face = "Courier";
    break;
  case 2:
    type_face = "Times";
    break;
  case 3:
    type_face = "Helvetica";
    break;
  default:
    return 0;
  }
  
  cdCanvasFont(intcgm_canvas, type_face, style, size );
  
  return 0;
}
