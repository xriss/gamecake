#include <stdio.h>
#include <math.h>

#include <cd.h>

#include "list.h"
#include "types.h"
#include "intcgm.h"
#include "intcgm6.h"

#include "circle.h"

#define DIM 360

#ifndef PI
#define PI 3.14159265358979323846
#endif

#define PI180 PI/180.
#define TWOPI 2*PI
#define VARCS TWOPI/32.

int cgm_poly_circle ( double xc, double yc, double radius, double angi, double angf, int fechado )
{
 double coseno, seno;
 double xs, ys;
 
 coseno = cos (VARCS);
 seno   = sin (VARCS);

 xs = radius * cos(angi);
 ys = radius * sin(angi);

 cdCanvasBegin(intcgm_canvas,  cgm_setintstyle(intcgm_fill_att.int_style) );

 if ( fechado==CLOSED_PIE ) cdCanvasVertex (intcgm_canvas,  cgm_vdcx2canvas(xc), cgm_vdcy2canvas(yc) );

 cdCanvasVertex (intcgm_canvas,  cgm_vdcx2canvas(xc+xs), cgm_vdcy2canvas(yc+ys) );

 while ( (angi+VARCS) < angf )
  {
   double xe = xs;
   xs = xs * coseno - ys * seno;
   ys = ys * coseno + xe * seno;
   cdCanvasVertex (intcgm_canvas,  cgm_vdcx2canvas(xc+xs), cgm_vdcy2canvas(yc+ys) );
   angi += VARCS;
  }

 cdCanvasVertex (intcgm_canvas,  cgm_vdcx2canvas(xc+radius*cos(angf)), cgm_vdcy2canvas(yc+radius*sin(angf)) );

 cdCanvasEnd(intcgm_canvas);

 return 0;
}

int cgm_line_circle ( double xc, double yc, double radius, double angi, double angf, int fechado )
{
 double coseno, seno;
 double xant, yant, firstx, firsty;
 double xs, ys;

 /* GERA O DESENHO DO CIRCULO/ARCO */

 coseno = cos (VARCS);
 seno   = sin (VARCS);

 xs = radius * cos(angi);
 ys = radius * sin(angi);

 if ( fechado==CLOSED_PIE )
  cdCanvasLine (intcgm_canvas,  cgm_vdcx2canvas(xc), cgm_vdcy2canvas(yc), cgm_vdcx2canvas(xc+xs), cgm_vdcy2canvas(yc+ys) );

 xant = firstx = xc+xs;
 yant = firsty = yc+ys;

 while ( (angi+VARCS) < angf )
  {
   double xe = xs;
   xs = xs * coseno - ys * seno;
   ys = ys * coseno + xe * seno;
   cdCanvasLine (intcgm_canvas,  cgm_vdcx2canvas(xant), cgm_vdcy2canvas(yant), cgm_vdcx2canvas(xc+xs), cgm_vdcy2canvas(yc+ys) );
   xant = xc+xs;
   yant = yc+ys;
   angi += VARCS;
  }

 cdCanvasLine (intcgm_canvas,  cgm_vdcx2canvas(xant), cgm_vdcy2canvas(yant),
          cgm_vdcx2canvas(xc+radius*cos(angf)), cgm_vdcy2canvas(yc+radius*sin(angf)) );

 xant = xc+radius*cos(angf);
 yant = yc+radius*sin(angf);

 if ( fechado==CLOSED_PIE )
  cdCanvasLine (intcgm_canvas,  cgm_vdcx2canvas(xant), cgm_vdcy2canvas(yant), cgm_vdcx2canvas(xc), cgm_vdcy2canvas(yc) );
 else if ( fechado==CLOSED_CHORD )
  cdCanvasLine (intcgm_canvas,  cgm_vdcx2canvas(xant), cgm_vdcy2canvas(yant), cgm_vdcx2canvas(firstx), cgm_vdcy2canvas(firsty) );

 return 0;
}

