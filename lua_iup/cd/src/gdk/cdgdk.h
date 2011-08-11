/** \file
 * \brief Gdk Base Driver
 *
 * See Copyright Notice in cd.h
 */

#ifndef __CDGDK_H
#define __CDGDK_H

#include <gdk/gdk.h>

#include "cd.h"
#include "cd_private.h"


struct _cdCtxImage {
  unsigned int w, h, depth;
  GdkPixmap* img;
  GdkScreen* scr;
  GdkVisual* vis;
};

struct _cdCtxCanvas {
  cdCanvas* canvas;
  GdkVisual* vis;            /* visual of the application */
  GdkScreen *scr;
  GdkGC* gc;                 /* graphic context */
  GdkDrawable* wnd;          /* drawable */
  GdkColor fg, bg;

  GdkGCValues gcval;

  GdkPixmap* last_hatch;     /* last hatch   set by user */
  GdkPixmap* last_stipple;   /* last stipple set by user */
  GdkPixmap* last_pattern;   /* last pattern set by user */
  GdkGC* last_stipple_gc;
  int last_stipple_w;
  int last_stipple_h;

  GdkGC* last_pattern_gc;
  int last_pattern_w;
  int last_pattern_h;

  unsigned int depth;        /* canvas depth */
  long int *xidata;          /* Image cache */
  int xisize;
  GdkColormap* colormap;     /* Color map */
  int num_colors;            /* Size of the color table  */
  double xmatrix[6];         /* Transformation matrix that includes axis inversion */
  float rotate_angle;
  int rotate_center_x;
  int rotate_center_y;
  int img_dither, img_interp;

  GdkRegion* new_rgn;
  GdkRegion* clip_rgn;

  PangoContext *fontcontext;
  PangoFontDescription *fontdesc;
  PangoLayout *fontlayout;
  PangoMatrix fontmatrix;
  char* strLastConvertUTF8;

  cdImage* image_dbuffer;       /* Used by double buffer driver */
  cdCanvas* canvas_dbuffer;
};

#define cdCOLOR8TO16(_x) (_x*257)  /* 65535/255 = 257 */
#define cdCOLOR16TO8(_x) ((unsigned char)(_x/257))

cdCtxCanvas *cdgdkCreateCanvas(cdCanvas* canvas, GdkDrawable* wnd, GdkScreen* scr, GdkVisual* vis);
void cdgdkInitTable(cdCanvas* canvas);
void cdgdkKillCanvas(cdCtxCanvas *ctxcanvas);

#endif
