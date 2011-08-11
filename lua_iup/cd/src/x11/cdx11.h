/** \file
 * \brief X-Windows Base Driver
 *
 * See Copyright Notice in cd.h
 */

#ifndef __CDX11_H
#define __CDX11_H

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "cd.h"
#include "cd_private.h"


/* Hidden declaration for the Context Plus driver */
typedef struct _cdxContextPlus cdxContextPlus;

struct _cdCtxImage {
  unsigned int w, h, depth;
  Pixmap img;
  Display *dpy;
  int scr;
  Visual *vis;
};

struct _cdCtxCanvas {
  cdCanvas* canvas;
  Display* dpy;          /* display da aplicacao no X */
  Visual* vis;           /* visual usado pela aplicacao */
  int scr;               /* screen da aplicacao */
  GC gc;                 /* contexto grafico */
  Drawable wnd;          /* drawable */
  long int fg;
  Pixmap last_hatch;     /* ultimo hatch setado pelo usuario */
  Pixmap last_stipple;   /* ultimo stipple setado pelo usuario */
  Pixmap last_pattern;   /* ultimo pattern setado pelo usuario */
  GC last_stipple_gc;
  int last_stipple_w;
  int last_stipple_h;
  GC last_pattern_gc;
  int last_pattern_w;
  int last_pattern_h;
  XFontStruct *font;     /* fonte de caracteres no X */
  unsigned int depth;    /* depth do canvas */
  Pixmap clip_polygon;   /* poligono de clipping */
  Pixmap new_region, region_aux;
  GC region_aux_gc;
  void *data;            /* informacoes especificas do driver */
  long int *xidata;      /* ximage cache */
  int xisize;
  Colormap colormap;          /* colormap para todos os canvas */
  XColor color_table[256];    /* tabela de cores do colormap */
  int num_colors;             /* tamanho maximo da tabela de cores  */
  int rshift;                 /* constante red para calculo truecolor */
  int gshift;                 /* constante green para calculo truecolor */
  int bshift;                 /* constante blue para calculo truecolor */
  double xmatrix[6];          /* transformation matrix that includes axis inversion */
  float  rotate_angle;
  int    rotate_center_x,
         rotate_center_y;

  cdImage* image_dbuffer; /* utilizado pelo driver de Double buffer */
  cdCanvas* canvas_dbuffer;

  cdxContextPlus* ctxplus;
};

#define cdCOLOR8TO16(_x) (_x*257)  /* 65535/255 = 257 */
#define cdCOLOR16TO8(_x) ((unsigned char)(_x/257))

extern unsigned long (*cdxGetPixel)(cdCtxCanvas *ctxcanvas, unsigned long rgb);
extern void (*cdxGetRGB)(cdCtxCanvas *ctxcanvas, unsigned long pixel, 
                                                 unsigned char* red, 
                                                 unsigned char* green, 
                                                 unsigned char* blue);

cdCtxCanvas *cdxCreateCanvas(cdCanvas* canvas, Display *dpy, int scr, Drawable wnd, Visual *vis);
void cdxInitTable(cdCanvas* canvas);
void cdxKillCanvas(cdCtxCanvas *ctxcanvas);
int cdxClip(cdCtxCanvas *ctxcanvas, int clip_mode);
void cdxPoly(cdCtxCanvas *ctxcanvas, int mode, cdPoint* poly, int n);

#endif
