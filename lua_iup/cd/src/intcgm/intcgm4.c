#define _INTCGM4_C_

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <cd.h>
#include <cdcgm.h>

#include "list.h"
#include "types.h"
#include "intcgm.h"
#include "intcgm2.h"
#include "intcgm4.h"
#include "intcgm6.h"
#include "ellipse.h"
#include "circle.h"
#include "sism.h"

#ifndef PI
#define PI 3.1415931
#endif
#define TWOPI 2*PI

static double myhypot ( double x, double y )
{
 return sqrt ( x*x + y*y );
}

int cgm_do_vdcext ( tpoint first, tpoint second )
{
 double width, height;
 double tmp;
 double razao_vp;
 double razao_wn;
 int w_pixel, h_pixel;
 double w_mm, h_mm;

 if ( first.x > second.x )
  {
   tmp = first.x;
   first.x = second.x;
   second.x = tmp;
  }

 if ( first.y > second.y )
  {
   tmp = first.y;
   first.y = second.y;
   second.y = tmp;
  }

 width = intcgm_view_xmax-intcgm_view_xmin+1;
 height = intcgm_view_ymax-intcgm_view_ymin+1;

 razao_vp = (double) width/ (double) height;
 razao_wn = (second.x-first.x) / (second.y-first.y);

 if ( razao_vp > razao_wn )
  {
   first.x -= ((second.y-first.y)*razao_vp - (second.x-first.x)) / 2.;
   second.x = (second.y-first.y)*razao_vp+first.x;
  }
 else
  {
   first.y -= ((second.x-first.x)/razao_vp - (second.y-first.y)) / 2.;
   second.y = (second.x-first.x)/razao_vp+first.y;
  }

 intcgm_vdc_ext.xmin = first.x;
 intcgm_vdc_ext.xmax = second.x;
 intcgm_vdc_ext.ymin = first.y;
 intcgm_vdc_ext.ymax = second.y;

 intcgm_scale_factor_x = width/(second.x-first.x);
 intcgm_scale_factor_y = height/(second.y-first.y);
 intcgm_scale_factor   = sqrt(width * width + height * height)/sqrt((second.x-first.x)+(second.y-first.y));
 cdCanvasGetSize (intcgm_canvas,  &w_pixel, &h_pixel, &w_mm, &h_mm );
 intcgm_scale_factor_mm_x = w_pixel/w_mm;
 intcgm_scale_factor_mm_y = h_pixel/h_mm;

 if ( intcgm_cgm.drawing_mode==ABSTRACT )
  {
   intcgm_clip_xmin = cgm_canvas2vdcx ( intcgm_view_xmin );
   intcgm_clip_xmax = cgm_canvas2vdcx ( intcgm_view_xmax );
   intcgm_clip_ymin = cgm_canvas2vdcy ( intcgm_view_ymin );
   intcgm_clip_ymax = cgm_canvas2vdcy ( intcgm_view_ymax );
  }
 else
  {
   intcgm_clip_xmin = intcgm_vdc_ext.xmin*intcgm_cgm.scaling_mode.scale_factor;
   intcgm_clip_xmax = intcgm_vdc_ext.xmax*intcgm_cgm.scaling_mode.scale_factor;
   intcgm_clip_ymin = intcgm_vdc_ext.ymin*intcgm_cgm.scaling_mode.scale_factor;
   intcgm_clip_ymax = intcgm_vdc_ext.ymax*intcgm_cgm.scaling_mode.scale_factor;
  }

 cdCanvasClipArea (intcgm_canvas, cgm_vdcx2canvas(first.x), cgm_vdcx2canvas(second.x),
              cgm_vdcy2canvas(first.y), cgm_vdcy2canvas(second.y) );
 cdCanvasClip(intcgm_canvas,  CD_CLIPOFF );

 return 0;
}

int cgm_do_bckcol ( trgb rgb )
{
 long int cor = cdEncodeColor ( (unsigned char) rgb.red,
                                (unsigned char) rgb.green,
                                (unsigned char) rgb.blue );

 cdCanvasSetBackground (intcgm_canvas, cor );

 return 0;
}

int cgm_do_transp ( int transparency )
{
 if ( transparency )
  cdCanvasBackOpacity (intcgm_canvas,  CD_TRANSPARENT );
 else
  cdCanvasBackOpacity (intcgm_canvas,  CD_OPAQUE );

 return 0;
}

int cgm_do_clprec ( tpoint first, tpoint second )
{
 double tmp;

 if ( first.x > second.x )
  {
   tmp = first.x;
   first.x = second.x;
   second.x = tmp;
  }

 if ( first.y > second.y )
  {
   tmp = first.y;
   first.y = second.y;
   second.y = tmp;
  }

 cdCanvasClipArea (intcgm_canvas,  cgm_vdcx2canvas(first.x), cgm_vdcx2canvas(second.x),
              cgm_vdcy2canvas(first.y), cgm_vdcy2canvas(second.y) );

 return 0;
}

int cgm_do_clpind ( int indicator )
{
 if ( indicator )
  cdCanvasClip(intcgm_canvas,  CD_CLIPAREA );
 else
  cdCanvasClip(intcgm_canvas,  CD_CLIPOFF );

 return 0;
}

int cgm_do_polyln ( int n_points, tpoint *pt )
{
 int i;
 long int cor;
 double bb_xmin, bb_ymin, bb_xmax, bb_ymax;

 cgm_getpolybbox ( pt, n_points, &bb_xmin, &bb_ymin, &bb_xmax, &bb_ymax );

#if 0
 if ( cgm_vdcx2canvas(bb_xmin) > intcgm_view_xmax || cgm_vdcx2canvas(bb_xmax) < intcgm_view_xmin ||
      cgm_vdcy2canvas(bb_ymin) > intcgm_view_ymax || cgm_vdcy2canvas(bb_ymax) < intcgm_view_ymin ) return 0;
#else
 if ( bb_xmin > intcgm_clip_xmax || bb_xmax < intcgm_clip_xmin ||
      bb_ymin > intcgm_clip_ymax || bb_ymax < intcgm_clip_ymin ) return 0;
#endif

 cgm_setlinestyle ( intcgm_line_att.type );
 cgm_setlinewidth ( intcgm_line_att.width );
 cor = cgm_getcolor ( intcgm_line_att.color );
 cdCanvasSetForeground (intcgm_canvas,  cor );

 cdCanvasBegin(intcgm_canvas,  CD_OPEN_LINES );

 for ( i=0; i<n_points; i++ )
   cdCanvasVertex (intcgm_canvas,  cgm_vdcx2canvas(pt[i].x), cgm_vdcy2canvas(pt[i].y) );

 cdCanvasEnd (intcgm_canvas);

 return 0;
}

int cgm_do_incply ( int n_points, tpoint *pt )
{
 int i;
 double x, y;
 long int cor;
 double bb_xmin, bb_ymin, bb_xmax, bb_ymax;

 cgm_getincpolybbox ( pt, n_points, &bb_xmin, &bb_ymin, &bb_xmax, &bb_ymax );

#if 0
 if ( cgm_vdcx2canvas(bb_xmin) > intcgm_view_xmax || cgm_vdcx2canvas(bb_xmax) < intcgm_view_xmin ||
      cgm_vdcy2canvas(bb_ymin) > intcgm_view_ymax || cgm_vdcy2canvas(bb_ymax) < intcgm_view_ymin ) return 0;
#else
 if ( bb_xmin > intcgm_clip_xmax || bb_xmax < intcgm_clip_xmin ||
      bb_ymin > intcgm_clip_ymax || bb_ymax < intcgm_clip_ymin ) return 0;
#endif

 cgm_setlinestyle ( intcgm_line_att.type );
 cgm_setlinewidth ( intcgm_line_att.width );
 cor = cgm_getcolor ( intcgm_line_att.color );
 cdCanvasSetForeground (intcgm_canvas,  cor );

 cdCanvasBegin(intcgm_canvas,  CD_OPEN_LINES );

 x = pt[0].x;    y = pt[0].y;
 cdCanvasVertex (intcgm_canvas,  cgm_vdcx2canvas(pt[0].x), cgm_vdcy2canvas(pt[0].y) );

 for ( i=1; i<n_points; i++ )
   {
    x += pt[i].x;      y += pt[i].x;
    cdCanvasVertex (intcgm_canvas,  cgm_vdcx2canvas(x), cgm_vdcy2canvas(y) );
   }

 cdCanvasEnd (intcgm_canvas);

 return 0;
}

int cgm_do_djtply ( int n_points, tpoint *pt )
{
 int i;
 long int cor;
 double bb_xmin, bb_ymin, bb_xmax, bb_ymax;

 cgm_getpolybbox ( pt, n_points, &bb_xmin, &bb_ymin, &bb_xmax, &bb_ymax );

#if 0
 if ( cgm_vdcx2canvas(bb_xmin) > intcgm_view_xmax || cgm_vdcx2canvas(bb_xmax) < intcgm_view_xmin ||
      cgm_vdcy2canvas(bb_ymin) > intcgm_view_ymax || cgm_vdcy2canvas(bb_ymax) < intcgm_view_ymin ) return 0;
#else
 if ( bb_xmin > intcgm_clip_xmax || bb_xmax < intcgm_clip_xmin ||
      bb_ymin > intcgm_clip_ymax || bb_ymax < intcgm_clip_ymin ) return 0;
#endif

 if ( n_points < 2 ) return 1;

 cgm_setlinestyle ( intcgm_line_att.type );
 cgm_setlinewidth ( intcgm_line_att.width );
 cor = cgm_getcolor ( intcgm_line_att.color );
 cdCanvasSetForeground (intcgm_canvas,  cor );

 for ( i=0; i<n_points; i=i+2 )
   cdCanvasLine (intcgm_canvas,  cgm_vdcx2canvas(pt[i].x), cgm_vdcy2canvas(pt[i].y), 
            cgm_vdcx2canvas(pt[i+1].x), cgm_vdcy2canvas(pt[i+1].y) );

 return 0;
}

int cgm_do_indpl ( int n_points, tpoint *pt )
{
 int i;
 long int cor;
 double bb_xmin, bb_ymin, bb_xmax, bb_ymax;

 cgm_getincpolybbox ( pt, n_points, &bb_xmin, &bb_ymin, &bb_xmax, &bb_ymax );

#if 0
 if ( cgm_vdcx2canvas(bb_xmin) > intcgm_view_xmax || cgm_vdcx2canvas(bb_xmax) < intcgm_view_xmin ||
      cgm_vdcy2canvas(bb_ymin) > intcgm_view_ymax || cgm_vdcy2canvas(bb_ymax) < intcgm_view_ymin ) return 0;
#else
 if ( bb_xmin > intcgm_clip_xmax || bb_xmax < intcgm_clip_xmin ||
      bb_ymin > intcgm_clip_ymax || bb_ymax < intcgm_clip_ymin ) return 0;
#endif

 if ( n_points < 2 ) return 1;

 cgm_setlinestyle ( intcgm_line_att.type );
 cgm_setlinewidth ( intcgm_line_att.width );
 cor = cgm_getcolor ( intcgm_line_att.color );
 cdCanvasSetForeground (intcgm_canvas,  cor );

 pt[1].x = pt[0].x + pt[1].x;    pt[1].y = pt[0].y + pt[1].y;
 cdCanvasLine (intcgm_canvas,  cgm_vdcx2canvas(pt[0].x), cgm_vdcy2canvas(pt[0].y), 
          cgm_vdcx2canvas(pt[1].x), cgm_vdcy2canvas(pt[1].y) );

 for ( i=2; i<n_points; i=i+2 )
   {
    pt[i].x += pt[i-1].x;    pt[i].y += pt[i-1].y;
    pt[i+1].x += pt[i].x;    pt[i+1].y += pt[i].y;
    cdCanvasLine (intcgm_canvas,  cgm_vdcx2canvas(pt[i].x), cgm_vdcy2canvas(pt[i].y),
             cgm_vdcx2canvas(pt[i+1].x), cgm_vdcy2canvas(pt[i+1].y) );
   }

 return 0;
}

int cgm_do_polymk ( int n_points, tpoint *pt )
{
 int i;
 long int cor;
 double bb_xmin, bb_ymin, bb_xmax, bb_ymax;

 cgm_getpolybbox ( pt, n_points, &bb_xmin, &bb_ymin, &bb_xmax, &bb_ymax );

#if 0
 if ( cgm_vdcx2canvas(bb_xmin) > intcgm_view_xmax || cgm_vdcx2canvas(bb_xmax) < intcgm_view_xmin ||
      cgm_vdcy2canvas(bb_ymin) > intcgm_view_ymax || cgm_vdcy2canvas(bb_ymax) < intcgm_view_ymin ) return 0;
#else
 if ( bb_xmin > intcgm_clip_xmax || bb_xmax < intcgm_clip_xmin ||
      bb_ymin > intcgm_clip_ymax || bb_ymax < intcgm_clip_ymin ) return 0;
#endif

 cgm_setmarktype ( intcgm_marker_att.type );
 cgm_setmarksize ( intcgm_marker_att.size );
 cor = cgm_getcolor ( intcgm_marker_att.color );
 cdCanvasSetForeground (intcgm_canvas,  cor );

 for ( i=0; i<n_points; i++ )
   cdCanvasMark (intcgm_canvas,  cgm_vdcx2canvas(pt[i].x), cgm_vdcy2canvas(pt[i].y) );

 return 0;
}

int cgm_do_incplm ( int n_points, tpoint *pt )
{
 int i;
 long int cor;
 double bb_xmin, bb_ymin, bb_xmax, bb_ymax;

 cgm_getincpolybbox ( pt, n_points, &bb_xmin, &bb_ymin, &bb_xmax, &bb_ymax );

#if 0
 if ( cgm_vdcx2canvas(bb_xmin) > intcgm_view_xmax || cgm_vdcx2canvas(bb_xmax) < intcgm_view_xmin ||
      cgm_vdcy2canvas(bb_ymin) > intcgm_view_ymax || cgm_vdcy2canvas(bb_ymax) < intcgm_view_ymin ) return 0;
#else
 if ( bb_xmin > intcgm_clip_xmax || bb_xmax < intcgm_clip_xmin ||
      bb_ymin > intcgm_clip_ymax || bb_ymax < intcgm_clip_ymin ) return 0;
#endif

 cgm_setmarktype ( intcgm_marker_att.type );
 cgm_setmarksize ( intcgm_marker_att.size );
 cor = cgm_getcolor ( intcgm_marker_att.color );
 cdCanvasSetForeground (intcgm_canvas,  cor );

 cdCanvasMark (intcgm_canvas,  cgm_vdcx2canvas(pt[0].x), cgm_vdcy2canvas(pt[0].y) );

 for ( i=1; i<n_points; i++ )
   {
    pt[i].x += pt[i-1].x;   pt[i].y += pt[i-1].y;
    cdCanvasMark (intcgm_canvas,  cgm_vdcx2canvas(pt[i].x), cgm_vdcy2canvas(pt[i].y) );
   }

 return 0;
}

int cgm_do_text ( int mode, char *string, tpoint pos )
{
 long int cor;

 cor = cgm_getcolor ( intcgm_text_att.color );
 cdCanvasSetForeground (intcgm_canvas,  cor );

 cgm_setfont ( intcgm_text_att.font, intcgm_text_att.style, intcgm_text_att.height );

 cdCanvasText (intcgm_canvas,  cgm_vdcx2canvas(pos.x), cgm_vdcy2canvas(pos.y), string );

 return 0;
}

int cgm_do_txtalign ( int hor, int ver )
{
 enum { NORMHORIZ, LEFT, CTR, RIGHT };
 enum { NORMVERT, TOP, CAP, HALF, BASE, BOTTOM };

 switch (hor)
  {
   case NORMHORIZ:
    switch (ver)
     {
      case NORMVERT:
	   cdCanvasTextAlignment(intcgm_canvas, CD_CENTER);
       break;
      case TOP:
	   cdCanvasTextAlignment(intcgm_canvas, CD_NORTH);
       break;
      case CAP:
	   cdCanvasTextAlignment(intcgm_canvas, CD_NORTH);
       break;
      case HALF:
	   cdCanvasTextAlignment(intcgm_canvas, CD_CENTER);
       break;
      case BASE:
	   cdCanvasTextAlignment(intcgm_canvas, CD_SOUTH);
       break;
      case BOTTOM:
	   cdCanvasTextAlignment(intcgm_canvas, CD_SOUTH);
       break;
     }
    break;
   case LEFT:
    switch (ver)
     {
      case NORMVERT:
	   cdCanvasTextAlignment(intcgm_canvas, CD_WEST);
       break;
      case TOP:
	   cdCanvasTextAlignment(intcgm_canvas, CD_NORTH_WEST);
       break;
      case CAP:
	   cdCanvasTextAlignment(intcgm_canvas, CD_NORTH_WEST);
       break;
      case HALF:
	   cdCanvasTextAlignment(intcgm_canvas, CD_WEST);
       break;
      case BASE:
	   cdCanvasTextAlignment(intcgm_canvas, CD_SOUTH_WEST);
       break;
      case BOTTOM:
	   cdCanvasTextAlignment(intcgm_canvas, CD_SOUTH_WEST);
       break;
     }
    break;
   case CTR:
    switch (ver)
     {
      case NORMVERT:
	   cdCanvasTextAlignment(intcgm_canvas, CD_CENTER);
       break;
      case TOP:
	   cdCanvasTextAlignment(intcgm_canvas, CD_NORTH);
       break;
      case CAP:
	   cdCanvasTextAlignment(intcgm_canvas, CD_NORTH);
       break;
      case HALF:
	   cdCanvasTextAlignment(intcgm_canvas, CD_CENTER);
       break;
      case BASE:
	   cdCanvasTextAlignment(intcgm_canvas, CD_SOUTH);
       break;
      case BOTTOM:
	   cdCanvasTextAlignment(intcgm_canvas, CD_SOUTH);
       break;
     }
    break;
   case RIGHT:
    switch (ver)
     {
      case NORMVERT:
	   cdCanvasTextAlignment(intcgm_canvas, CD_EAST);
       break;
      case TOP:
	   cdCanvasTextAlignment(intcgm_canvas, CD_NORTH_EAST);
       break;
      case CAP:
	   cdCanvasTextAlignment(intcgm_canvas, CD_NORTH_EAST);
       break;
      case HALF:
	   cdCanvasTextAlignment(intcgm_canvas, CD_EAST);
       break;
      case BASE:
	   cdCanvasTextAlignment(intcgm_canvas, CD_SOUTH_EAST);
       break;
      case BOTTOM:
	   cdCanvasTextAlignment(intcgm_canvas, CD_SOUTH_EAST);
       break;
     }
    break;
  }

 return 0;
}

int cgm_do_polygn ( int n_points, tpoint *pt )
{
 int i;
 long int cor;
 double bb_xmin, bb_ymin, bb_xmax, bb_ymax;

 cgm_getpolybbox ( pt, n_points, &bb_xmin, &bb_ymin, &bb_xmax, &bb_ymax );

#if 0
 if ( cgm_vdcx2canvas(bb_xmin) > intcgm_view_xmax || cgm_vdcx2canvas(bb_xmax) < intcgm_view_xmin ||
      cgm_vdcy2canvas(bb_ymin) > intcgm_view_ymax || cgm_vdcy2canvas(bb_ymax) < intcgm_view_ymin ) return 0;
#else
 if ( bb_xmin > intcgm_clip_xmax || bb_xmax < intcgm_clip_xmin ||
      bb_ymin > intcgm_clip_ymax || bb_ymax < intcgm_clip_ymin ) return 0;
#endif

 cor = cgm_getcolor ( intcgm_fill_att.color );
 cdCanvasSetForeground (intcgm_canvas,  cor );

 cgm_setlinewidth ( intcgm_line_att.width );

 if ( intcgm_fill_att.int_style!=HOLLOW && intcgm_fill_att.int_style!=EMPTY )
  {
   cdCanvasBegin(intcgm_canvas,  cgm_setintstyle ( intcgm_fill_att.int_style )  );

   for ( i=0; i<n_points; i++ )
    cdCanvasVertex (intcgm_canvas,  cgm_vdcx2canvas(pt[i].x), cgm_vdcy2canvas(pt[i].y) );
 
    cdCanvasEnd (intcgm_canvas);
  }

 if ( intcgm_edge_att.visibility==ON )
  {
   cor = cgm_getcolor ( intcgm_edge_att.color );
   cdCanvasSetForeground (intcgm_canvas,  cor );

   cgm_setlinewidth ( intcgm_edge_att.width );

   cdCanvasBegin(intcgm_canvas,  CD_CLOSED_LINES  );

   for ( i=0; i<n_points; i++ )
    cdCanvasVertex (intcgm_canvas,  cgm_vdcx2canvas(pt[i].x), cgm_vdcy2canvas(pt[i].y) );

   cdCanvasEnd (intcgm_canvas);
  }

 return 0;
}

int cgm_do_incplg ( int n_points, tpoint *pt )
{
 int i;
 tpoint p;
 long int cor;
 double bb_xmin, bb_ymin, bb_xmax, bb_ymax;

 cgm_getincpolybbox ( pt, n_points, &bb_xmin, &bb_ymin, &bb_xmax, &bb_ymax );

#if 0
 if ( cgm_vdcx2canvas(bb_xmin) > intcgm_view_xmax || cgm_vdcx2canvas(bb_xmax) < intcgm_view_xmin ||
      cgm_vdcy2canvas(bb_ymin) > intcgm_view_ymax || cgm_vdcy2canvas(bb_ymax) < intcgm_view_ymin ) return 0;
#else
 if ( bb_xmin > intcgm_clip_xmax || bb_xmax < intcgm_clip_xmin ||
      bb_ymin > intcgm_clip_ymax || bb_ymax < intcgm_clip_ymin ) return 0;
#endif
                    
 cor = cgm_getcolor ( intcgm_fill_att.color );
 cdCanvasSetForeground (intcgm_canvas,  cor );

 cdCanvasBegin(intcgm_canvas,  cgm_setintstyle ( intcgm_fill_att.int_style ) );

 p.x = pt[0].x;    p.y = pt[0].y;
 cdCanvasVertex (intcgm_canvas,  cgm_vdcx2canvas(pt[0].x), cgm_vdcy2canvas(pt[0].y) );

 for ( i=1; i<n_points; i++ )
  {
   p.x += pt[i].x;      p.y += pt[i].y;
   cdCanvasVertex (intcgm_canvas,  cgm_vdcx2canvas(p.x), cgm_vdcy2canvas(p.y) );
  }

 cdCanvasEnd (intcgm_canvas);

 return 0;
}

int cgm_do_plgset( int incremental, int n_points, tpoint *pt, short *flag )
{
 int i, j;
 tpoint closure;
 long int cor;
 int start;
 int vis = intcgm_edge_att.visibility;
 double bb_xmin, bb_ymin, bb_xmax, bb_ymax;

 cgm_getpolybbox ( pt, n_points, &bb_xmin, &bb_ymin, &bb_xmax, &bb_ymax );

#if 0
 if ( cgm_vdcx2canvas(bb_xmin) > intcgm_view_xmax || cgm_vdcx2canvas(bb_xmax) < intcgm_view_xmin ||
      cgm_vdcy2canvas(bb_ymin) > intcgm_view_ymax || cgm_vdcy2canvas(bb_ymax) < intcgm_view_ymin ) return 0;
#else
 if ( bb_xmin > intcgm_clip_xmax || bb_xmax < intcgm_clip_xmin ||
      bb_ymin > intcgm_clip_ymax || bb_ymax < intcgm_clip_ymin ) return 0;
#endif

 intcgm_edge_att.visibility = OFF;

 cor = cgm_getcolor ( intcgm_fill_att.color );
 cdCanvasSetForeground (intcgm_canvas,  cor );

 if ( intcgm_fill_att.int_style!=HOLLOW && intcgm_fill_att.int_style!=EMPTY )
  {
   cdCanvasBegin(intcgm_canvas,  cgm_setintstyle ( intcgm_fill_att.int_style )  );
   start = 1;
   for ( i=0; i<n_points; i++ )
    {
     if ( start )
      {
       closure.x = pt[i].x;
       closure.y = pt[i].y;
       start = 0;
	    }
     else if ( incremental==YES )
      {
       pt[i].x = pt[i].x+pt[i-1].x;
       pt[i].y = pt[i].y+pt[i-1].y;
      }

     if ( flag[i]==CLOSE_VISIBLE ||	flag[i]==CLOSE_INVISIBLE )
      {
       cdCanvasVertex (intcgm_canvas,  cgm_vdcx2canvas(pt[i].x), cgm_vdcy2canvas(pt[i].y) );
       cdCanvasVertex (intcgm_canvas,  cgm_vdcx2canvas(closure.x), cgm_vdcy2canvas(closure.y) );
	     start=1;
      }
	 else
      cdCanvasVertex (intcgm_canvas,  cgm_vdcx2canvas(pt[i].x), cgm_vdcy2canvas(pt[i].y) );
	}
   cdCanvasEnd (intcgm_canvas);
  }


 intcgm_edge_att.visibility = vis;

 if ( intcgm_edge_att.visibility==ON )
  {
   int np;

   cor = cgm_getcolor ( intcgm_edge_att.color );
   cdCanvasSetForeground (intcgm_canvas,  cor );

   cgm_setlinewidth ( intcgm_edge_att.width );

   np = 0;
   start = 0;
   for ( i=0; i<n_points; i++ )
    {
     if ( incremental==YES )
      {
       pt[i].x = pt[i].x+pt[i-1].x;
       pt[i].y = pt[i].y+pt[i-1].y;
      }

     if ( flag[i]==CLOSE_VISIBLE || (flag[i]==VISIBLE && i==n_points-1) )
      {
       cdCanvasBegin(intcgm_canvas,  CD_CLOSED_LINES );
       for ( j=0; j<np; j++ )
        cdCanvasVertex (intcgm_canvas,  cgm_vdcx2canvas(pt[start+j].x), cgm_vdcy2canvas(pt[start+j].y) );
       cdCanvasEnd(intcgm_canvas);
	     start += np;
       np = 0;
      }
     else if ( flag[i]==INVISIBLE || flag[i]==CLOSE_INVISIBLE )
      {
       cdCanvasBegin(intcgm_canvas,  CD_OPEN_LINES );
       for ( j=0; j<np; j++ )
        cdCanvasVertex (intcgm_canvas,  cgm_vdcx2canvas(pt[start+j].x), cgm_vdcy2canvas(pt[start+j].y) );
       cdCanvasEnd(intcgm_canvas);
	     start += np;
       np = 0;
      }
     else
      np++;
    }
  }

 return 0;
}

int cgm_do_cellar ( tpoint corner1, tpoint corner2, tpoint corner3, int nx,
                 int ny, long color_prec, tcolor *cell )
{
 int cx1, cy1, cx2, cy2, cx3, cy3, tmp, i, j;
 unsigned char *r, *g, *b;
 double bb_xmin, bb_ymin, bb_xmax, bb_ymax;

 bb_xmin = ( corner1.x < corner2.x ) ? corner1.x : corner2.x;
 bb_xmax = ( corner1.x > corner2.x ) ? corner1.x : corner2.x;
 bb_ymin = ( corner1.y < corner2.y ) ? corner1.y : corner2.y;
 bb_ymax = ( corner1.y > corner2.y ) ? corner1.y : corner2.y;

#if 0
 if ( cgm_vdcx2canvas(bb_xmin) > intcgm_view_xmax || cgm_vdcx2canvas(bb_xmax) < intcgm_view_xmin ||
      cgm_vdcy2canvas(bb_ymin) > intcgm_view_ymax || cgm_vdcy2canvas(bb_ymax) < intcgm_view_ymin ) return 0;
#else
 if ( bb_xmin > intcgm_clip_xmax || bb_xmax < intcgm_clip_xmin ||
      bb_ymin > intcgm_clip_ymax || bb_ymax < intcgm_clip_ymin ) return 0;
#endif

 r = (unsigned char *) malloc ( nx*ny*sizeof(unsigned char) );
 g = (unsigned char *) malloc ( nx*ny*sizeof(unsigned char) );
 b = (unsigned char *) malloc ( nx*ny*sizeof(unsigned char) );
 
 cx1 = cgm_vdcx2canvas(corner1.x);
 cy1 = cgm_vdcy2canvas(corner1.y);
 cx2 = cgm_vdcx2canvas(corner2.x);
 cy2 = cgm_vdcy2canvas(corner2.y);
 cx3 = cgm_vdcx2canvas(corner3.x);
 cy3 = cgm_vdcy2canvas(corner3.y);

#if 1
 if ( cx1<cx3 && cy1==cy3 && cx2==cx3 && cy2>cy3 )
  {
   for ( i=0; i<ny; i++ )
    for ( j=0; j<nx; j++ )
    {
     r[nx*i+j] = (unsigned char) cell[nx*i+j].rgb.red;
     g[nx*i+j] = (unsigned char) cell[nx*i+j].rgb.green;
     b[nx*i+j] = (unsigned char) cell[nx*i+j].rgb.blue;
    }
  }
 else if ( cx1==cx3 && cy1<cy3 && cx2>cx3 && cy2==cy3 )
  {
   tmp = nx;   nx = ny;    ny = tmp;
   for ( i=0; i<ny; i++ )
    for ( j=0; j<nx; j++ )
    {
     r[nx*i+j] = (unsigned char) cell[ny*j+i].rgb.red;
     g[nx*i+j] = (unsigned char) cell[ny*j+i].rgb.green;
     b[nx*i+j] = (unsigned char) cell[ny*j+i].rgb.blue;
    }
  }
 else if ( cx1<cx3 && cy1==cy3 && cx2==cx3 && cy2<cy3 )
  {
   for ( i=0; i<ny; i++ )
    for ( j=0; j<nx; j++ )
    {
     r[nx*i+j] = (unsigned char) cell[nx*(ny-i-1)+j].rgb.red;
     g[nx*i+j] = (unsigned char) cell[nx*(ny-i-1)+j].rgb.green;
     b[nx*i+j] = (unsigned char) cell[nx*(ny-i-1)+j].rgb.blue;
    }
  }
 else if ( cx1==cx3 && cy1>cy3 && cx2>cx3 && cy2==cy3 )
  {
   tmp = nx;   nx = ny;    ny = tmp;
   for ( i=0; i<ny; i++ )
    for ( j=0; j<nx; j++ )
    {
     r[nx*i+j] = (unsigned char) cell[ny*j+(ny-i-1)].rgb.red;
     g[nx*i+j] = (unsigned char) cell[ny*j+(ny-i-1)].rgb.green;
     b[nx*i+j] = (unsigned char) cell[ny*j+(ny-i-1)].rgb.blue;
    }
  }
 else if ( cx1>cx3 && cy1==cy3 && cx2==cx3 && cy2>cy3 )
  {
   for ( i=0; i<ny; i++ )
    for ( j=0; j<nx; j++ )
    {
     r[nx*i+j] = (unsigned char) cell[nx*i+(nx-j-1)].rgb.red;
     g[nx*i+j] = (unsigned char) cell[nx*i+(nx-j-1)].rgb.green;
     b[nx*i+j] = (unsigned char) cell[nx*i+(nx-j-1)].rgb.blue;
    }
  }
 else if ( cx1==cx3 && cy1>cy3 && cx2<cx3 && cy2==cy3 )
  {
   tmp = nx;   nx = ny;    ny = tmp;
   for ( i=0; i<ny; i++ )
    for ( j=0; j<nx; j++ )
    {
     r[nx*i+j] = (unsigned char) cell[ny*(nx-j-1)+(ny-i-1)].rgb.red;
     g[nx*i+j] = (unsigned char) cell[ny*(nx-j-1)+(ny-i-1)].rgb.green;
     b[nx*i+j] = (unsigned char) cell[ny*(nx-j-1)+(ny-i-1)].rgb.blue;
    }
  }
 else if ( cx1>cx3 && cy1==cy3 && cx2==cx3 && cy2<cy3 )
  {
   for ( i=0; i<ny; i++ )
    for ( j=0; j<nx; j++ )
    {
     r[nx*i+j] = (unsigned char) cell[nx*(ny-i-1)+(nx-j-1)].rgb.red;
     g[nx*i+j] = (unsigned char) cell[nx*(ny-i-1)+(nx-j-1)].rgb.green;
     b[nx*i+j] = (unsigned char) cell[nx*(ny-i-1)+(nx-j-1)].rgb.blue;
    }
  }
 else if ( cx1==cx3 && cy1<cy3 && cx2<cx3 && cy2==cy3 )
  {
   tmp = nx;   nx = ny;    ny = tmp;
   for ( i=0; i<ny; i++ )
    for ( j=0; j<nx; j++ )
    {
     r[nx*i+j] = (unsigned char) cell[ny*(nx-j-1)+i].rgb.red;
     g[nx*i+j] = (unsigned char) cell[ny*(nx-j-1)+i].rgb.green;
     b[nx*i+j] = (unsigned char) cell[ny*(nx-j-1)+i].rgb.blue;
    }
  }
#else
 for ( i=0; i<nx; i++ )
  for ( j=0; j<ny; j++ )
   {
    r[nx*j+i] = (unsigned char) cell[nx*j+i].rgb.red;
    g[nx*j+i] = (unsigned char) cell[nx*j+i].rgb.green;
    b[nx*j+i] = (unsigned char) cell[nx*j+i].rgb.blue;
   }
#endif

#if 1
 if ( cx1>cx2 )
  {
   tmp = cx1;
   cx1 = cx2;
   cx2 = tmp;
  }

 if ( cy1>cy2 )
  {
   tmp = cy1;
   cy1 = cy2;
   cy2 = tmp;
  }
#endif

 cdCanvasPutImageRectRGB (intcgm_canvas, nx, ny, r, g, b, cx1, cy1, cx2-cx1+1, cy2-cy1+1,0,0,0,0 );

 free(r);
 free(g);
 free(b);

 return 0;
}

int cgm_do_gdp ( int identifier, tpoint *pt, char *data_rec )
{
 if ( identifier==-4 )
  cgm_sism4 ( pt, data_rec );
 else if ( identifier==-5 )
  cgm_sism5 ( data_rec );

 return 0;
}

int cgm_do_rect ( tpoint point1, tpoint point2 )
{
 long int cor;
 double bb_xmin, bb_ymin, bb_xmax, bb_ymax;

 bb_xmin = ( point1.x < point2.x ) ? point1.x : point2.x;
 bb_xmax = ( point1.x > point2.x ) ? point1.x : point2.x;
 bb_ymin = ( point1.y < point2.y ) ? point1.y : point2.y;
 bb_ymax = ( point1.y > point2.y ) ? point1.y : point2.y;

#if 0
 if ( cgm_vdcx2canvas(bb_xmin) > intcgm_view_xmax || cgm_vdcx2canvas(bb_xmax) < intcgm_view_xmin ||
      cgm_vdcy2canvas(bb_ymin) > intcgm_view_ymax || cgm_vdcy2canvas(bb_ymax) < intcgm_view_ymin ) return 0;
#else
 if ( bb_xmin > intcgm_clip_xmax || bb_xmax < intcgm_clip_xmin ||
      bb_ymin > intcgm_clip_ymax || bb_ymax < intcgm_clip_ymin ) return 0;
#endif

 cor = cgm_getcolor ( intcgm_fill_att.color );
 cdCanvasSetForeground (intcgm_canvas,  cor );

 cdCanvasBegin(intcgm_canvas,  cgm_setintstyle ( intcgm_fill_att.int_style ) );

 cdCanvasVertex (intcgm_canvas,  cgm_vdcx2canvas(point1.x), cgm_vdcy2canvas(point1.y) );
 cdCanvasVertex (intcgm_canvas,  cgm_vdcx2canvas(point2.x), cgm_vdcy2canvas(point1.y) );
 cdCanvasVertex (intcgm_canvas,  cgm_vdcx2canvas(point2.x), cgm_vdcy2canvas(point2.y) );
 cdCanvasVertex (intcgm_canvas,  cgm_vdcx2canvas(point1.x), cgm_vdcy2canvas(point2.y) );

 cdCanvasEnd (intcgm_canvas);

 return 0;
}

int cgm_do_circle ( tpoint center, double radius )
{
 double bb_xmin, bb_ymin, bb_xmax, bb_ymax;

 bb_xmin = center.x - radius;
 bb_xmax = center.x + radius;
 bb_ymin = center.y - radius;
 bb_ymax = center.y + radius;

#if 0
 if ( cgm_vdcx2canvas(bb_xmin) > intcgm_view_xmax || cgm_vdcx2canvas(bb_xmax) < intcgm_view_xmin||
      cgm_vdcy2canvas(bb_ymin) > intcgm_view_ymax || cgm_vdcy2canvas(bb_ymax) < intcgm_view_ymin ) return 0;
#else
 if ( bb_xmin > intcgm_clip_xmax || bb_xmax < intcgm_clip_xmin ||
      bb_ymin > intcgm_clip_ymax || bb_ymax < intcgm_clip_ymin ) return 0;
#endif

 if ( intcgm_fill_att.int_style!=EMPTY )
  {
   long int cor;

   cor = cgm_getcolor ( intcgm_fill_att.color );
   cdCanvasSetForeground (intcgm_canvas,  cor );

   cgm_setlinestyle ( intcgm_line_att.type );
   cgm_setlinewidth ( intcgm_line_att.width );

   if ( intcgm_fill_att.int_style!=EMPTY )
    cgm_poly_circle ( center.x, center.y, radius, 0, TWOPI, CLOSED_CHORD );
   else
    cgm_line_circle ( center.x, center.y, radius, 0, TWOPI, CLOSED_CHORD );
  }

 if ( intcgm_edge_att.visibility==ON )
  {
   long int cor;

   cor = cgm_getcolor ( intcgm_edge_att.color );
   cdCanvasSetForeground (intcgm_canvas,  cor );

   cgm_setlinestyle ( intcgm_edge_att.type );
   cgm_setlinewidth ( intcgm_edge_att.width );

   cgm_line_circle ( center.x, center.y, radius, 0., TWOPI, CLOSED_CHORD );
  }

 return 0;
}

static double angulo ( double cx, double cy, double px, double py )
{
 double ang;
 if ((fabs(py-cy)<1e-9) && (fabs(px-cx)<1e-9))
   ang = 0.0;
 else
   ang = atan2 ( py - cy , px - cx );

 if ( ang<0.) ang = TWOPI - fabs(ang); 

 return ang;
}

static void solve2 ( double m[][2], double *b, double *x )
{
 double det;

 det = m[0][0]*m[1][1] - m[0][1]*m[1][0];

 if ( det==0.0 ) return;

 x[0] = ( b[0]*m[1][1] - b[1]*m[1][0] ) / det;
 x[1] = ( m[0][0]*b[1] - m[0][1]*b[0] ) / det;
}

static void getcenter ( double xs, double ys, double xi, double yi, double xe, double ye,
                        double *xc, double *yc )
{
 double c[2];

 double x2, y2, x3, y3, m[2][2], b[2];

 x2 = xi - xs;
 y2 = yi - ys;
 x3 = xe - xs;
 y3 = ye - ys;

 m[0][0] = 2*x2;
 m[1][0] = 2*y2;
 m[0][1] = 2*x3;
 m[1][1] = 2*y3;
 b[0]    = x2*x2 + y2*y2;
 b[1]    = x3*x3 + y3*y3;

 solve2 ( m, b, c );

 *xc = c[0] + xs;
 *yc = c[1] + ys;
}

int cgm_do_circ3p ( tpoint starting, tpoint intermediate, tpoint ending )
{
 long int cor;
 double xc, yc;
 double angi, angm, angf;
 double radius;
 double bb_xmin, bb_ymin, bb_xmax, bb_ymax;

 cor = cgm_getcolor ( intcgm_line_att.color );
 cdCanvasSetForeground (intcgm_canvas,  cor );

 cgm_setlinestyle ( intcgm_line_att.type );
 cgm_setlinewidth ( intcgm_line_att.width );

 getcenter ( starting.x, starting.y, intermediate.x, intermediate.y, ending.x, ending.y, &xc, &yc );

 angi = angulo( xc, yc, starting.x, starting.y );
 angm = angulo( xc, yc, intermediate.x, intermediate.y );
 angf = angulo( xc, yc, ending.x, ending.y );

 if ( angm<angi )
  {
   double tmp = angi;
   angi = angf;
   angf = tmp;
  } 

 if ( angf<angi ) angf = angf + TWOPI;

 radius = sqrt ( (starting.x-xc)*(starting.x-xc) + (starting.y-yc)*(starting.y-yc) );

 bb_xmin = xc - radius;
 bb_xmax = xc + radius;
 bb_ymin = yc - radius;
 bb_ymax = yc + radius;

#if 0
 if ( cgm_vdcx2canvas(bb_xmin) > intcgm_view_xmax || cgm_vdcx2canvas(bb_xmax) < intcgm_view_xmin ||
      cgm_vdcy2canvas(bb_ymin) > intcgm_view_ymax || cgm_vdcy2canvas(bb_ymax) < intcgm_view_ymin ) return 0;
#else
 if ( bb_xmin > intcgm_clip_xmax || bb_xmax < intcgm_clip_xmin ||
      bb_ymin > intcgm_clip_ymax || bb_ymax < intcgm_clip_ymin ) return 0;
#endif

 cgm_line_circle ( xc, yc, radius, angi, angf, OPEN );

 return 0;
}

int cgm_do_circ3pc ( tpoint starting, tpoint intermediate, tpoint ending, int close_type )
{
 long int cor;
 double xc, yc;
 double angi, angm, angf;
 double radius;
 double bb_xmin, bb_ymin, bb_xmax, bb_ymax;

 cor = cgm_getcolor ( intcgm_fill_att.color );
 cdCanvasSetForeground (intcgm_canvas,  cor );

 cgm_setlinestyle ( intcgm_line_att.type );
 cgm_setlinewidth ( intcgm_line_att.width );

 getcenter ( starting.x, starting.y, intermediate.x, intermediate.y, ending.x, ending.y, &xc, &yc );

 angi = angulo( xc, yc, starting.x, starting.y );
 angm = angulo( xc, yc, intermediate.x, intermediate.y );
 angf = angulo( xc, yc, ending.x, ending.y );

 radius = sqrt ( (starting.x-xc)*(starting.x-xc) + (starting.y-yc)*(starting.y-yc) );

 bb_xmin = xc - radius;
 bb_xmax = xc + radius;
 bb_ymin = yc - radius;
 bb_ymax = yc + radius;

#if 0
 if ( cgm_vdcx2canvas(bb_xmin) > intcgm_view_xmax || cgm_vdcx2canvas(bb_xmax) < intcgm_view_xmin ||
      cgm_vdcy2canvas(bb_ymin) > intcgm_view_ymax || cgm_vdcy2canvas(bb_ymax) < intcgm_view_ymin ) return 0;
#else
 if ( bb_xmin > intcgm_clip_xmax || bb_xmax < intcgm_clip_xmin ||
      bb_ymin > intcgm_clip_ymax || bb_ymax < intcgm_clip_ymin ) return 0;
#endif

 if ( angm<angi )
  {
   double tmp = angi;
   angi = angf;
   angf = tmp;
  } 

 if ( angf<angi ) angf = angf + TWOPI;

 if ( intcgm_fill_att.int_style!=EMPTY )
  {
   long int cor;

   cor = cgm_getcolor ( intcgm_fill_att.color );
   cdCanvasSetForeground (intcgm_canvas,  cor );

   cgm_setlinewidth ( intcgm_line_att.width );

   if ( intcgm_fill_att.int_style!=EMPTY )
    cgm_poly_circle ( xc, yc, radius, angi, angf, (close_type) ? CLOSED_CHORD : CLOSED_PIE );
   else
    cgm_line_circle ( xc, yc, radius, angi, angf, (close_type) ? CLOSED_CHORD : CLOSED_PIE );
  }

 if ( intcgm_edge_att.visibility==ON )
  {
   long int cor;

   cor = cgm_getcolor ( intcgm_edge_att.color );
   cdCanvasSetForeground (intcgm_canvas,  cor );

   cgm_setlinestyle ( intcgm_edge_att.type );
   cgm_setlinewidth ( intcgm_edge_att.width );

   cgm_line_circle ( xc, yc, radius, angi, angf, (close_type) ? CLOSED_CHORD : CLOSED_PIE );
  }

 return 0;
}

int cgm_do_circcnt ( tpoint center, tpoint start, tpoint end, double radius )
{
 long int cor;
 double angi, angf;
 double bb_xmin, bb_ymin, bb_xmax, bb_ymax;

 bb_xmin = center.x - radius;
 bb_xmax = center.x + radius;
 bb_ymin = center.y - radius;
 bb_ymax = center.y + radius;

#if 0
 if ( cgm_vdcx2canvas(bb_xmin) > intcgm_view_xmax || cgm_vdcx2canvas(bb_xmax) < intcgm_view_xmin ||
      cgm_vdcy2canvas(bb_ymin) > intcgm_view_ymax || cgm_vdcy2canvas(bb_ymax) < intcgm_view_ymin ) return 0;
#else
 if ( bb_xmin > intcgm_clip_xmax || bb_xmax < intcgm_clip_xmin ||
      bb_ymin > intcgm_clip_ymax || bb_ymax < intcgm_clip_ymin ) return 0;
#endif

 cor = cgm_getcolor ( intcgm_fill_att.color );
 cdCanvasSetForeground (intcgm_canvas,  cor );

 cgm_setlinewidth ( intcgm_line_att.width );

 angi = angulo( center.x, center.y, start.x, start.y );
 angf = angulo( center.x, center.y, end.x, end.y );

 if ( angf<angi )
  {
   double tmp = angi;
   angi = angf;
   angf = tmp;
  } 

 cgm_line_circle ( center.x, center.y, radius, angi, angf, OPEN );

 return 0;
}

int cgm_do_ccntcl ( tpoint center, tpoint start, tpoint end, double radius, int close_type )
{
 long int cor;
 double angi, angf;
 double bb_xmin, bb_ymin, bb_xmax, bb_ymax;

 bb_xmin = center.x - radius;
 bb_xmax = center.x + radius;
 bb_ymin = center.y - radius;
 bb_ymax = center.y + radius;

#if 0
 if ( cgm_vdcx2canvas(bb_xmin) > intcgm_view_xmax || cgm_vdcx2canvas(bb_xmax) < intcgm_view_xmin ||
      cgm_vdcy2canvas(bb_ymin) > intcgm_view_ymax || cgm_vdcy2canvas(bb_ymax) < intcgm_view_ymin ) return 0;
#else
 if ( bb_xmin > intcgm_clip_xmax || bb_xmax < intcgm_clip_xmin ||
      bb_ymin > intcgm_clip_ymax || bb_ymax < intcgm_clip_ymin ) return 0;
#endif

 cor = cgm_getcolor ( intcgm_fill_att.color );
 cdCanvasSetForeground (intcgm_canvas,  cor );

 cgm_setlinewidth ( intcgm_line_att.width );

 angi = angulo( center.x, center.y, start.x, start.y );
 angf = angulo( center.x, center.y, end.x, end.y );

 if ( angf<angi )
  {
   double tmp = angi;
   angi = angf;
   angf = tmp;
  } 

 if ( intcgm_fill_att.int_style!=HOLLOW && intcgm_fill_att.int_style!=EMPTY )
  {
   long int cor;

   cor = cgm_getcolor ( intcgm_fill_att.color );
   cdCanvasSetForeground (intcgm_canvas,  cor );

   cgm_setlinewidth ( intcgm_line_att.width );

   cgm_poly_circle ( center.x, center.y, radius, angi, angf, (close_type) ? CLOSED_CHORD : CLOSED_PIE );
  }

 if ( intcgm_edge_att.visibility==ON )
  {
   long int cor;

   cor = cgm_getcolor ( intcgm_edge_att.color );
   cdCanvasSetForeground (intcgm_canvas,  cor );

   cgm_setlinewidth ( intcgm_edge_att.width );

   cgm_line_circle ( center.x, center.y, radius, angi, angf, (close_type) ? CLOSED_CHORD : CLOSED_PIE );
  }

 return 0;
}

int cgm_do_ellips ( tpoint center, tpoint first_CDP, tpoint second_CDP )
{
 double w = myhypot ( first_CDP.x-center.x, first_CDP.y-center.y );
 double h = myhypot ( second_CDP.x-center.x, second_CDP.y-center.y );
 double inc =  atan2 ( first_CDP.y-center.y, first_CDP.x-center.x );
 double bb_xmin, bb_ymin, bb_xmax, bb_ymax;

 bb_xmin = center.x - w;
 bb_xmax = center.x + w;
 bb_ymin = center.y - h;
 bb_ymax = center.y + h;

#if 0
 if ( cgm_vdcx2canvas(bb_xmin) > intcgm_view_xmax || cgm_vdcx2canvas(bb_xmax) < intcgm_view_xmin ||
      cgm_vdcy2canvas(bb_ymin) > intcgm_view_ymax || cgm_vdcy2canvas(bb_ymax) < intcgm_view_ymin) return 0;
#else
 if ( bb_xmin > intcgm_clip_xmax || bb_xmax < intcgm_clip_xmin ||
      bb_ymin > intcgm_clip_ymax || bb_ymax < intcgm_clip_ymin ) return 0;
#endif

 cgm_ElpArc ( center.x, center.y, w*2., h*2., inc, 0-inc, TWOPI-inc, 64, 2  );

 return 0;
}

int cgm_do_ellarc ( tpoint center, tpoint first_CDP, tpoint second_CDP,
                tpoint start, tpoint end )
{
 double w = myhypot ( first_CDP.x-center.x, first_CDP.y-center.y );
 double h = myhypot ( second_CDP.x-center.x, second_CDP.y-center.y );
 double inc = atan2 ( first_CDP.y-center.y, first_CDP.x-center.x );
 double a1  = atan2 ( start.y, start.x );
 double a2  = atan2 ( end.y, end.x );
 double bb_xmin, bb_ymin, bb_xmax, bb_ymax;

 bb_xmin = center.x - w;
 bb_xmax = center.x + w;
 bb_ymin = center.y - h;
 bb_ymax = center.y + h;

#if 0
 if ( cgm_vdcx2canvas(bb_xmin) > intcgm_view_xmax || cgm_vdcx2canvas(bb_xmax) < intcgm_view_xmin ||
      cgm_vdcy2canvas(bb_ymin) > intcgm_view_ymax || cgm_vdcy2canvas(bb_ymax) < intcgm_view_ymin ) return 0;
#else
 if ( bb_xmin > intcgm_clip_xmax || bb_xmax < intcgm_clip_xmin ||
      bb_ymin > intcgm_clip_ymax || bb_ymax < intcgm_clip_ymin ) return 0;
#endif

 cgm_ElpArc ( center.x, center.y, w*2., h*2., inc, a1-inc, a2-inc, 64, 0 );

 return 0;
}

int cgm_do_ellacl ( tpoint center, tpoint first_CDP, tpoint second_CDP,
                tpoint start, tpoint end, int close_type )
{
 double w = myhypot ( first_CDP.x-center.x, first_CDP.y-center.y );
 double h = myhypot ( second_CDP.x-center.x, second_CDP.y-center.y );
 double inc = atan2 ( first_CDP.y-center.y, first_CDP.x-center.x );
 double a1  = atan2 ( start.y, start.x );
 double a2  = atan2 ( end.y, end.x );

 double bb_xmin, bb_ymin, bb_xmax, bb_ymax;

 bb_xmin = center.x - w;
 bb_xmax = center.x + w;
 bb_ymin = center.y - h;
 bb_ymax = center.y + h;

#if 0
 if ( cgm_vdcx2canvas(bb_xmin) > intcgm_view_xmax || cgm_vdcx2canvas(bb_xmax) < intcgm_view_xmin ||
      cgm_vdcy2canvas(bb_ymin) > intcgm_view_ymax || cgm_vdcy2canvas(bb_ymax) < intcgm_view_ymin ) return 0;
#else
 if ( bb_xmin > intcgm_clip_xmax || bb_xmax < intcgm_clip_xmin ||
      bb_ymin > intcgm_clip_ymax || bb_ymax < intcgm_clip_ymin ) return 0;
#endif

 cgm_ElpArc ( center.x, center.y, w*2., h*2., inc, a1-inc, a2-inc, 64, close_type+1 );

 return 0;
}

int cgm_do_text_height ( double height )
{
 cgm_setfont ( intcgm_text_att.font, intcgm_text_att.style, height );

 return 0;
}

