#include <stdio.h>
#include <math.h>

#include <cd.h>

#include "list.h"
#include "types.h"
#include "intcgm.h"
#include "intcgm6.h"
#include "ellipse.h"

#ifndef PI
#define PI 3.14159265358979323846
#endif

#define ANGMIN 0.00001

/* Adjust the circle parametrization for the elipsis parametrization */
double cgm_AdjArc ( double arc, double w, double h )
{
 double value;

 if ((fabs(w*sin(arc))<1e-99) && (fabs(h*cos(arc))<1e-99))
   value = 0.0;
 else
   value = atan2(w*sin(arc), h*cos(arc));

 if ( arc >  PI  ) value += 2*PI;
 if ( arc < -PI  ) value -= 2*PI;

 return value;
}


/*  Desenha um arco de Elipse */

void cgm_ElpArc ( double xc, double yc, double w, double h, double r, double a1,
              double a2, int n, int tipo )

{

 /* Onde:

  (xc,yc) Centro
  (w,h)   Largura e altura (diametro na direcao dos eixos principais)
  r       Inclinacao dos eixos principais com relacao x e y
  a1,a2   Angulos incial e final do arco [-360,+360]
             Note-se que o sentido e' dado pela diferenca a2-a1,
             ou seja, 30 a 330 e' trigonometrico
                      30 a -30 e' horario
  n       Numero de segmentos da poligonal equivalente
          (64 parece ser um bom numero)
  tipo    0=aberto 1=fechado(torta) 2=fechado(corda) */

  double da, c, s, sx, sy, ang, xant, yant;
  double px, py, tx, ty, txant, tyant, dx, dy;
  int i, inicio;

/* Reduz de diametro a raio */

  w = w/2;
  h = h/2;

/* Transforma graus em radianos e ajusta a os angulos da parametrizacao */

  a1 = cgm_AdjArc(a1,w,h);

  a2 = cgm_AdjArc(a2,w,h);

  if ( a2>a1 )
   ang = a2 - a1;
  else
   ang = 2*PI - ( a1 - a2 );

/* Gera os pontos do arco centrado na origem com os eixos em x e y */

  da = ang/n;
  c  = cos(da);
  s  = sin(da);
  sx = -w*s/h;
  sy = h*s/w;

  if ( tipo==1 || tipo==2 )
   {
    long int cor;
    cor = cgm_getcolor ( intcgm_fill_att.color );
    cdCanvasSetForeground (intcgm_canvas,  cor );
    cdCanvasBegin(intcgm_canvas,  cgm_setintstyle(intcgm_fill_att.int_style) );
   }
  else
   {
    long int cor;
    int size = (int)floor(intcgm_line_att.width+.5);
    cdCanvasLineStyle (intcgm_canvas,  intcgm_line_att.type );
    cdCanvasLineWidth (intcgm_canvas,  size>0? size: 1 );
    cor = cgm_getcolor ( intcgm_line_att.color );
    cdCanvasSetForeground (intcgm_canvas,  cor );
   }

  dx = (fabs(r)>ANGMIN) ? sin(r) : 0;
  dy = (fabs(r)>ANGMIN) ? cos(r) : 1;

  xant  = w*cos(a1);
  yant  = h*sin(a1);

  txant = xc+dy*xant-dx*yant;   /* Inclina (se for o caso) e translada */
  tyant = yc+dx*xant+dy*yant;   /* Inclina (se for o caso) e translada */

  if ( tipo==1 )
   {
    cdCanvasVertex (intcgm_canvas,  cgm_vdcx2canvas(xc), cgm_vdcy2canvas(yc) );
    cdCanvasVertex (intcgm_canvas,  cgm_vdcx2canvas(txant), cgm_vdcy2canvas(tyant) );
    inicio = 2;
   }
  else if ( tipo==2 )
   {
    cdCanvasVertex (intcgm_canvas,  cgm_vdcx2canvas(txant), cgm_vdcy2canvas(tyant) );
    inicio = 1;
   }

  for ( i=inicio; i<=(n+inicio-1); i++ )
   {
    px = c*xant+sx*yant;
    py = sy*xant+c*yant;

    tx = xc+dy*px-dx*py;       /* Inclina (se for o caso) e translada */
    ty = yc+dx*px+dy*py;       /* Inclina (se for o caso) e translada */

    if ( tipo==0 )
     cdCanvasLine (intcgm_canvas,  cgm_vdcx2canvas(txant), cgm_vdcy2canvas(tyant), cgm_vdcx2canvas(tx), cgm_vdcy2canvas(ty) );
    else
     cdCanvasVertex (intcgm_canvas,  cgm_vdcx2canvas(tx), cgm_vdcy2canvas(ty) );

    xant = px;
    yant = py;
    txant = tx;
    tyant = ty;
  }

/* Desenha */

  if ( tipo==1 || tipo==2 ) cdCanvasEnd(intcgm_canvas);
}
