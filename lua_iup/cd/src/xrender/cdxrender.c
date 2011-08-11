/** file
 *  brief XRender Base Driver
 *
 *  See Copyright Notice in cd.h
 */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <math.h>

#include <X11/Xft/Xft.h>
#include <X11/extensions/Xrender.h>

#include "cdx11.h"
#include "cddbuf.h"
#include "cdimage.h"
#include "cdnative.h"
#include "cd_truetype.h"
#include "sim.h"

#include <X11/Xproto.h>

#define NUM_HATCHES  6
static unsigned char hatches[NUM_HATCHES][8] = {
  {0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00},  /* HORIZONTAL */
  {0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22},  /* VERTICAL */
  {0x08,0x10,0x20,0x40,0x80,0x01,0x02,0x04},  /* FDIAGONAL */
  {0x10,0x08,0x04,0x02,0x01,0x80,0x40,0x20},  /* BDIAGONAL */
  {0x22,0x22,0xFF,0x22,0x22,0x22,0xFF,0x22},  /* CROSS */
  {0x18,0x18,0x24,0x42,0x81,0x81,0x42,0x24}   /* DIAGCROSS */
};

struct _cdxContextPlus
{
  XftDraw* draw;
  Picture solid_pic, pattern_pic, fill_picture, dst_picture;
  XftFont *font,
          *flat_font;  /* used only for text size when orientation!=0 */
  XRenderPictFormat* maskFormat;

  int antialias;

#if (RENDER_MAJOR>0 || RENDER_MINOR>=10)
  XLinearGradient linegradient;
  Picture linegradient_pic;
#endif

  void (*cxKillCanvas)(cdCtxCanvas* ctxcanvas);
};


static void xrInitColor(XRenderColor *rendercolor, long color)
{
  rendercolor->red = cdCOLOR8TO16(cdRed(color));
  rendercolor->green = cdCOLOR8TO16(cdGreen(color));
  rendercolor->blue = cdCOLOR8TO16(cdBlue(color));
  rendercolor->alpha = cdCOLOR8TO16(cdAlpha(color));
}

static void xrPolyFill(cdCtxCanvas* ctxcanvas, XPointDouble* fpoly, int n)
{
  XRenderCompositeDoublePoly(ctxcanvas->dpy, PictOpOver, ctxcanvas->ctxplus->fill_picture,
                             ctxcanvas->ctxplus->dst_picture, ctxcanvas->ctxplus->maskFormat, 0, 0, 0, 0,
                             fpoly, n, ctxcanvas->canvas->fill_mode==CD_EVENODD?EvenOddRule:WindingRule);
}

static void xrLine(cdCtxCanvas *ctxcanvas, XPointDouble* fpoly)
{
  XRenderCompositeDoublePoly(ctxcanvas->dpy, PictOpOver, ctxcanvas->ctxplus->solid_pic,
                             ctxcanvas->ctxplus->dst_picture, ctxcanvas->ctxplus->maskFormat, 0, 0, 0, 0,
                             fpoly, 4, 0);
}

static void xrSetClipMask(cdCtxCanvas* ctxcanvas, Pixmap clip_mask)
{
  XRenderPictureAttributes pa;
  pa.clip_mask = clip_mask;
  XRenderChangePicture(ctxcanvas->dpy, ctxcanvas->ctxplus->dst_picture, CPClipMask, &pa);
}

static void xrSetClipArea(cdCtxCanvas* ctxcanvas)
{
  cdRect* clip_rect = &ctxcanvas->canvas->clip_rect;
  XRectangle rect;
  rect.x      = (short)clip_rect->xmin;
  rect.y      = (short)clip_rect->ymin;
  rect.width  = (unsigned short)(clip_rect->xmax - clip_rect->xmin + 1);
  rect.height = (unsigned short)(clip_rect->ymax - clip_rect->ymin + 1);
  XRenderSetPictureClipRectangles(ctxcanvas->dpy, ctxcanvas->ctxplus->dst_picture, 0, 0, &rect, 1);
}

static int cdclip(cdCtxCanvas *ctxcanvas, int clip_mode)
{
  switch (clip_mode)
  {
  case CD_CLIPOFF:
    xrSetClipMask(ctxcanvas, None);
    break;
  case CD_CLIPAREA:
    xrSetClipArea(ctxcanvas);
    break;
  case CD_CLIPPOLYGON:
    if (ctxcanvas->clip_polygon)
      xrSetClipMask(ctxcanvas, ctxcanvas->clip_polygon);
    break;
  case CD_CLIPREGION:
    if (ctxcanvas->new_region)
      xrSetClipMask(ctxcanvas, ctxcanvas->new_region);
    break;
  }

  /* call original method, to set the clipping for the PutImage* methods */
  cdxClip(ctxcanvas, clip_mode);

  return clip_mode;
}

static void cdcliparea(cdCtxCanvas *ctxcanvas, int xmin, int xmax, int ymin, int ymax)
{
  if (ctxcanvas->canvas->clip_mode == CD_CLIPAREA) 
  {
    ctxcanvas->canvas->clip_rect.xmin = xmin;
    ctxcanvas->canvas->clip_rect.ymin = ymin;
    ctxcanvas->canvas->clip_rect.xmax = xmax;
    ctxcanvas->canvas->clip_rect.ymax = ymax;
    cdclip(ctxcanvas, CD_CLIPAREA);
  }
}

static void cdfline(cdCtxCanvas *ctxcanvas, double x1, double y1, double x2, double y2)
{
  int ix1, ix2, iy1, iy2;

#ifndef CD_XRENDER_MATRIX
  if (ctxcanvas->canvas->use_matrix)
  {
    cdfMatrixTransformPoint(ctxcanvas->xmatrix, x1, y1, &x1, &y1);
    cdfMatrixTransformPoint(ctxcanvas->xmatrix, x2, y2, &x2, &y2);
  }
#endif

  ix1 = _cdRound(x1);
  ix2 = _cdRound(x2);
  iy1 = _cdRound(y1);
  iy2 = _cdRound(y2);
  if ((ctxcanvas->canvas->line_width == 1) &&
     ((ix1 == ix2 && ix1==x1) || (iy1 == iy2 && iy1==y1)))
  {
    XRenderColor rendercolor;
    xrInitColor(&rendercolor, ctxcanvas->canvas->foreground);
    if (ix2 < ix1) _cdSwapInt(ix2, ix1);
    if (iy2 < iy1) _cdSwapInt(iy2, iy1);
    XRenderFillRectangle(ctxcanvas->dpy, PictOpSrc, ctxcanvas->ctxplus->dst_picture, &rendercolor, ix1, iy1, ix2-ix1+1, iy2-iy1+1);
  }
  else
  {
    double half_width = ctxcanvas->canvas->line_width/2.0;
    XPointDouble fpoly[4];

    /* XRender does not have a function to draw lines. 
       So we have to draw a poligon that covers the line area. */ 

    double dx = x2-x1;
    double dy = y2-y1;
    double d = half_width/hypot(dx, dy);
    double dnx = d*dx;
    double dny = d*dy;

    fpoly[0].x = x1 + dny;
    fpoly[0].y = y1 - dnx;
    fpoly[1].x = x1 - dny;
    fpoly[1].y = y1 + dnx;
    fpoly[2].x = fpoly[1].x + dx;
    fpoly[2].y = fpoly[1].y + dy;
    fpoly[3].x = fpoly[0].x + dx;
    fpoly[3].y = fpoly[0].y + dy;
    
    xrLine(ctxcanvas, fpoly);
  }
}

static void cdline(cdCtxCanvas *ctxcanvas, int x1, int y1, int x2, int y2)
{
  cdfline(ctxcanvas, (double)x1, (double)y1, (double)x2, (double)y2);
}

static void cdrect(cdCtxCanvas *ctxcanvas, int xmin, int xmax, int ymin, int ymax)
{
  cdfSimRect(ctxcanvas, (double)xmin, (double)xmax, (double)ymin, (double)ymax);
}

static void cdbox(cdCtxCanvas *ctxcanvas, int xmin, int xmax, int ymin, int ymax)
{
  cdfSimBox(ctxcanvas, (double)xmin, (double)xmax, (double)ymin, (double)ymax);
}

static void cdfpoly(cdCtxCanvas* ctxcanvas, int mode, cdfPoint* fpoly, int n)
{
  int i;

  switch(mode) 
  {
  case CD_CLOSED_LINES:
    fpoly[n] = fpoly[0];
    n++;
    /* continue */
  case CD_OPEN_LINES:
    for (i = 0; i< n - 1; i++)
    {
      /* because line styles are not supported, this is not a problem */
      cdfline(ctxcanvas, fpoly[i].x, fpoly[i].y, fpoly[i+1].x, fpoly[i+1].y);
    }
    break;
  case CD_BEZIER:
    cdfSimPolyBezier(ctxcanvas->canvas, fpoly, n);
    break;
  case CD_PATH:
    cdfSimPolyPath(ctxcanvas->canvas, fpoly, n);
    break;
  case CD_FILL:
    {
      if (ctxcanvas->canvas->new_region)
      {
        cdPoint* poly = malloc(sizeof(cdPoint)*n);

        for (i = 0; i<n; i++)
        {
          poly[i].x = _cdRound(fpoly[i].x);
          poly[i].y = _cdRound(fpoly[i].y);
        }

        cdxPoly(ctxcanvas, CD_FILL, poly, n);

        free(poly);
      }
      else
      {
        XPointDouble* poly = malloc(sizeof(XPointDouble)*n);

        for (i = 0; i<n; i++)
        {
          poly[i].x = fpoly[i].x;
          poly[i].y = fpoly[i].y;

#ifndef CD_XRENDER_MATRIX
          if (ctxcanvas->canvas->use_matrix)
            cdfMatrixTransformPoint(ctxcanvas->xmatrix, poly[i].x, poly[i].y, &poly[i].x, &poly[i].y);
#endif
        }

        xrPolyFill(ctxcanvas, poly, n);

        free(poly);
      }
    }
    break;
  case CD_CLIP:
    {
      cdPoint* poly = malloc(sizeof(cdPoint)*n);

      for (i = 0; i<n; i++)
      {
        poly[i].x = _cdRound(fpoly[i].x);
        poly[i].y = _cdRound(fpoly[i].y);
      }

      cdxPoly(ctxcanvas, CD_CLIP, poly, n);

      free(poly);
      if (ctxcanvas->canvas->clip_mode == CD_CLIPPOLYGON) cdclip(ctxcanvas, CD_CLIPPOLYGON);
    }
    break;
  }
}

static void cdpoly(cdCtxCanvas* ctxcanvas, int mode, cdPoint* poly, int n)
{
  int i;

  switch(mode) 
  {
  case CD_CLOSED_LINES:
    poly[n] = poly[0];
    n++;
    /* continue */
  case CD_OPEN_LINES:
    for (i = 0; i<n-1; i++)
    {
      /* because line styles are not supported, this is not a problem */
      cdfline(ctxcanvas, (double)poly[i].x, (double)poly[i].y, (double)poly[i+1].x, (double)poly[i+1].y);
    }
    break;
  case CD_PATH:
    cdSimPolyPath(ctxcanvas->canvas, poly, n);
    break;
  case CD_BEZIER:
    cdSimPolyBezier(ctxcanvas->canvas, poly, n);  
    break;
  case CD_FILL:
    {
      if (ctxcanvas->canvas->new_region)
      {
        cdxPoly(ctxcanvas, CD_FILL, poly, n);
      }
      else
      {
        XPointDouble* fpoly = malloc(sizeof(XPointDouble)*n);

        for (i = 0; i<n; i++)
        {
          fpoly[i].x = (double)poly[i].x;
          fpoly[i].y = (double)poly[i].y;

#ifndef CD_XRENDER_MATRIX
          if (ctxcanvas->canvas->use_matrix)
            cdfMatrixTransformPoint(ctxcanvas->xmatrix, fpoly[i].x, fpoly[i].y, &fpoly[i].x, &fpoly[i].y);
#endif
        }

        xrPolyFill(ctxcanvas, fpoly, n);

        free(fpoly);
      }
    }
    break;
  case CD_CLIP:
    cdxPoly(ctxcanvas, CD_CLIP, poly, n);
    if (ctxcanvas->canvas->clip_mode == CD_CLIPPOLYGON) cdclip(ctxcanvas, CD_CLIPPOLYGON);
    break;
  }
}

static void cdtransform(cdCtxCanvas *ctxcanvas, const double* matrix)
{
#ifndef CD_XRENDER_MATRIX
  if (matrix)
  {
    /* configure a bottom-up coordinate system */
    ctxcanvas->xmatrix[0] = 1; 
    ctxcanvas->xmatrix[1] = 0;
    ctxcanvas->xmatrix[2] = 0; 
    ctxcanvas->xmatrix[3] = -1; 
    ctxcanvas->xmatrix[4] = 0; 
    ctxcanvas->xmatrix[5] = (ctxcanvas->canvas->h-1); 
    cdMatrixMultiply(matrix, ctxcanvas->xmatrix);

    ctxcanvas->canvas->invert_yaxis = 0;
  }
  else
  {
    ctxcanvas->canvas->invert_yaxis = 1;
  }
#else
  XTransform transform;
  double identity[6] = {1, 0, 0, 1, 0, 0};

  if (!matrix)
    matrix = &identity[0];

  transform.matrix[0][0] = XDoubleToFixed(matrix[0]);       /* |m0   m2   m4|   |00   01   02| */
  transform.matrix[0][1] = XDoubleToFixed(matrix[2]);       /* |m1   m3   m5| = |10   11   12| */
  transform.matrix[0][2] = XDoubleToFixed(matrix[4]);       /* |0     0    1|   |20   21   22| */
  transform.matrix[1][0] = XDoubleToFixed(matrix[1]);
  transform.matrix[1][1] = XDoubleToFixed(matrix[3]);
  transform.matrix[1][2] = XDoubleToFixed(matrix[5]);
  transform.matrix[2][0] = XDoubleToFixed(0);
  transform.matrix[2][1] = XDoubleToFixed(0);
  transform.matrix[2][2] = XDoubleToFixed(1);

  /* TODO: This is not working. It gives a BadPicture error */
  XRenderSetPictureTransform(ctxcanvas->dpy, ctxcanvas->ctxplus->dst_picture, &transform);
#endif
}

static int cdinteriorstyle(cdCtxCanvas *ctxcanvas, int style)
{
  switch (style)
  {
    case CD_SOLID:
      if (!ctxcanvas->ctxplus->solid_pic) 
        return ctxcanvas->canvas->interior_style;
      ctxcanvas->ctxplus->fill_picture = ctxcanvas->ctxplus->solid_pic;
      break;
    case CD_HATCH:
    case CD_STIPPLE:
    case CD_PATTERN:
      if (!ctxcanvas->ctxplus->pattern_pic) 
        return ctxcanvas->canvas->interior_style;
      ctxcanvas->ctxplus->fill_picture = ctxcanvas->ctxplus->pattern_pic;
      break;
  }
  return style;
}

static void cdpattern(cdCtxCanvas *ctxcanvas, int w, int h, const long int *colors)
{
  int x, y;
  Pixmap pixmap;
  XRenderPictureAttributes pa;
  XRenderPictFormat* format;

  if (ctxcanvas->ctxplus->pattern_pic) 
    XRenderFreePicture(ctxcanvas->dpy, ctxcanvas->ctxplus->pattern_pic);

  format = XRenderFindStandardFormat(ctxcanvas->dpy, PictStandardARGB32);
  pixmap = XCreatePixmap(ctxcanvas->dpy, DefaultRootWindow(ctxcanvas->dpy), w, h, format->depth);
  pa.repeat = 1;
  ctxcanvas->ctxplus->pattern_pic = XRenderCreatePicture(ctxcanvas->dpy, pixmap, format, CPRepeat, &pa);

  for (y=0; y<h; y++)
  {
    for (x=0; x<w; x++)
    {
      XRenderColor rendercolor;
      xrInitColor(&rendercolor, colors[y*w+x]);
      XRenderFillRectangle(ctxcanvas->dpy, PictOpSrc, ctxcanvas->ctxplus->pattern_pic, &rendercolor, x, h-y-1, 1, 1);
    }
  }

  cdinteriorstyle(ctxcanvas, CD_PATTERN);
}

static void cdstipple(cdCtxCanvas *ctxcanvas, int w, int h, const unsigned char *data)
{
  int x, y, i;
  long transparent = cdEncodeAlpha(0, 0);
  long int *colors = malloc(sizeof(long)*w*h);

  for (y=0; y<h; y++)
  {
    for (x=0; x<w; x++)
    {
      i = y*w+x;
      if (ctxcanvas->canvas->back_opacity == CD_OPAQUE)
      {
        if (data[i])
          colors[i] = ctxcanvas->canvas->foreground;
        else
          colors[i] = ctxcanvas->canvas->background;
      }
      else
      {
        if (data[i])
          colors[i] = ctxcanvas->canvas->foreground;
        else
          colors[i] = transparent;
      }
    }
  }

  cdpattern(ctxcanvas, w, h, colors);
  free(colors);
}

static int cdhatch(cdCtxCanvas *ctxcanvas, int hatch_style)
{
  int y;
  unsigned char data[8*8];
  unsigned char *hatch = hatches[hatch_style];

  for (y=0; y<8; y++)
  {
    int i = y*8;
    unsigned char c = hatch[y];
    data[i+7] = (c&0x01)>>0;
    data[i+6] = (c&0x02)>>1;
    data[i+5] = (c&0x04)>>2;
    data[i+4] = (c&0x08)>>3;
    data[i+3] = (c&0x10)>>4;
    data[i+2] = (c&0x20)>>5;
    data[i+1] = (c&0x40)>>6;
    data[i+0] = (c&0x80)>>7;
  }

  cdstipple(ctxcanvas, 8, 8, data);
  return hatch_style;
}

static int cdbackopacity(cdCtxCanvas *ctxcanvas, int back_opacity)
{
  ctxcanvas->canvas->back_opacity = back_opacity;
  if (ctxcanvas->canvas->interior_style == CD_STIPPLE)
    cdstipple(ctxcanvas, ctxcanvas->canvas->stipple_w, ctxcanvas->canvas->stipple_h, ctxcanvas->canvas->stipple);
  else if (ctxcanvas->canvas->interior_style == CD_HATCH)
    cdhatch(ctxcanvas, ctxcanvas->canvas->hatch_style);
  return back_opacity;
}

static void cdclear(cdCtxCanvas* ctxcanvas)
{
  XRenderColor rendercolor;
  xrInitColor(&rendercolor, ctxcanvas->canvas->background);
  XRenderFillRectangle(ctxcanvas->dpy, PictOpSrc, ctxcanvas->ctxplus->dst_picture, &rendercolor, 0, 0, ctxcanvas->canvas->w, ctxcanvas->canvas->h);
}

static void cdpixel(cdCtxCanvas *ctxcanvas, int x, int y, long int color)
{
  XRenderColor rendercolor;
  xrInitColor(&rendercolor, color);
  XRenderFillRectangle(ctxcanvas->dpy, PictOpSrc, ctxcanvas->ctxplus->dst_picture, &rendercolor, x, y, 1, 1);
}

#if (RENDER_MAJOR==0 && RENDER_MINOR<10)
static Picture XRenderCreateSolidFill(Display *dpy, const XRenderColor *color)
{
  Picture pict;
  XRenderPictureAttributes pa;
  XRenderPictFormat* format = XRenderFindStandardFormat(dpy, PictStandardARGB32);
  Pixmap pix = XCreatePixmap(dpy, DefaultRootWindow(dpy), 1, 1, format->depth);
  pa.repeat = True;
  pict = XRenderCreatePicture(dpy, pix, format, CPRepeat, &pa);
  XFreePixmap(dpy, pix);

  XRenderFillRectangle(dpy, PictOpSrc, pict, color, 0, 0, 1, 1);
  return pict;
}
#endif

static long int cdforeground(cdCtxCanvas *ctxcanvas, long int color)
{          
  XRenderPictureAttributes pa;
  XRenderColor rendercolor;
  xrInitColor(&rendercolor, color);
  if (ctxcanvas->ctxplus->solid_pic) 
    XRenderFreePicture(ctxcanvas->dpy, ctxcanvas->ctxplus->solid_pic);
  ctxcanvas->ctxplus->solid_pic = XRenderCreateSolidFill(ctxcanvas->dpy, &rendercolor);
  pa.repeat = 1;
  XRenderChangePicture(ctxcanvas->dpy, ctxcanvas->ctxplus->solid_pic, CPRepeat, &pa);
  if (ctxcanvas->canvas->interior_style == CD_STIPPLE)
    cdstipple(ctxcanvas, ctxcanvas->canvas->stipple_w, ctxcanvas->canvas->stipple_h, ctxcanvas->canvas->stipple);
  else if (ctxcanvas->canvas->interior_style == CD_HATCH)
    cdhatch(ctxcanvas, ctxcanvas->canvas->hatch_style);
  else if (ctxcanvas->canvas->interior_style == CD_SOLID)
    cdinteriorstyle(ctxcanvas, CD_SOLID);
  return color;
}

static long int cdbackground(cdCtxCanvas *ctxcanvas, long int color)
{
  if (ctxcanvas->canvas->back_opacity == CD_OPAQUE)
  {
    if (ctxcanvas->canvas->interior_style == CD_STIPPLE)
      cdstipple(ctxcanvas, ctxcanvas->canvas->stipple_w, ctxcanvas->canvas->stipple_h, ctxcanvas->canvas->stipple);
    else if (ctxcanvas->canvas->interior_style == CD_HATCH)
      cdhatch(ctxcanvas, ctxcanvas->canvas->hatch_style);
  }
  return color;
}

static int cdfont(cdCtxCanvas *ctxcanvas, const char *type_face, int style, int size)
{
  char font_name[1024];
  XftFont *font;
  char matrix[200] = "";

  /* no underline or strikeout support */

  static char* type_style[] = 
  {
    "",  /* CD_PLAIN */
    ":bold",    /* CD_BOLD */
    ":slant=italic,oblique",  /* CD_ITALIC */
    ":bold:slant=italic,oblique"     /* CD_BOLD_ITALIC */
  };

  if (cdStrEqualNoCase(type_face, "Fixed") || cdStrEqualNoCase(type_face, "System"))
    type_face = "monospace";
  else if (cdStrEqualNoCase(type_face, "Courier") || cdStrEqualNoCase(type_face, "Courier New"))
    type_face = "monospace";
  else if (cdStrEqualNoCase(type_face, "Times") || cdStrEqualNoCase(type_face, "Times New Roman"))
    type_face = "serif";
  else if (cdStrEqualNoCase(type_face, "Helvetica") || cdStrEqualNoCase(type_face, "Arial"))
    type_face = "sans";

  if (ctxcanvas->canvas->text_orientation)
  {
    double angle = CD_DEG2RAD*ctxcanvas->canvas->text_orientation;
    double cos_angle = cos(angle);
    double sin_angle = sin(angle);

    sprintf(matrix,":matrix=%f %f %f %f", cos_angle, -sin_angle, sin_angle, cos_angle);
  }

  size = cdGetFontSizePoints(ctxcanvas->canvas, size);

  sprintf(font_name,"%s-%d%s%s", type_face, size, type_style[style&3], matrix);
  font = XftFontOpenName(ctxcanvas->dpy, ctxcanvas->scr, font_name);
  if (!font)
    return 0;

  if (ctxcanvas->ctxplus->font)
    XftFontClose(ctxcanvas->dpy, ctxcanvas->ctxplus->font);

  if (ctxcanvas->canvas->text_orientation)
  {
    /* XftTextExtents8 will return the size of the rotated text, but we want the size without orientation.
       So create a font without orientation just to return the correct text size. */

    if (ctxcanvas->ctxplus->flat_font)
      XftFontClose(ctxcanvas->dpy, ctxcanvas->ctxplus->flat_font);

    sprintf(font_name,"%s-%d%s", type_face, size, type_style[style&3]);
    ctxcanvas->ctxplus->flat_font = XftFontOpenName(ctxcanvas->dpy, ctxcanvas->scr, font_name);
  }

  ctxcanvas->ctxplus->font = font;

  return 1;
}

static int cdnativefont(cdCtxCanvas *ctxcanvas, const char* nativefont)
{
  int size = 12, style = CD_PLAIN;
  char type_face[1024];

  if (nativefont[0] == '-')
  {
    XftFont *font = XftFontOpenXlfd(ctxcanvas->dpy, ctxcanvas->scr, nativefont);
    if (!font)
      return 0;

    if (!cdParseXWinFont(nativefont, type_face, &style, &size))
    {
      XftFontClose(ctxcanvas->dpy, font);
      return 0;
    }

    if (ctxcanvas->ctxplus->font)
      XftFontClose(ctxcanvas->dpy, ctxcanvas->ctxplus->font);

    ctxcanvas->canvas->text_orientation = 0; /* orientation not supported when using XLFD */

    ctxcanvas->ctxplus->font = font;
  }
  else
  {
    if (!cdParsePangoFont(nativefont, type_face, &style, &size))
      return 0;

    if (!cdfont(ctxcanvas, type_face, style, size))
      return 0;
  }

  /* update cdfont parameters */
  ctxcanvas->canvas->font_style = style;
  ctxcanvas->canvas->font_size = size;
  strcpy(ctxcanvas->canvas->font_type_face, type_face);

  return 1;
}

static double cdtextorientation(cdCtxCanvas *ctxcanvas, double angle)
{
  /* must recriate the font if orientation changes */
  ctxcanvas->canvas->text_orientation = angle;
  cdfont(ctxcanvas, ctxcanvas->canvas->font_type_face, ctxcanvas->canvas->font_style, ctxcanvas->canvas->font_size);
  return angle;
}

static void cdgetfontdim(cdCtxCanvas *ctxcanvas, int *max_width, int *height, int *ascent, int *descent)
{
  if (!ctxcanvas->ctxplus->font)
    return;

  if (max_width) *max_width = ctxcanvas->ctxplus->font->max_advance_width;
  if (height)    *height    = ctxcanvas->ctxplus->font->ascent + ctxcanvas->ctxplus->font->descent;
  if (ascent)    *ascent    = ctxcanvas->ctxplus->font->ascent;
  if (descent)   *descent   = ctxcanvas->ctxplus->font->descent;
}

static void cdgettextsize(cdCtxCanvas *ctxcanvas, const char *text, int len, int *width, int *height)
{
  XGlyphInfo extents;
  if (!ctxcanvas->ctxplus->font) 
    return;

  if (ctxcanvas->canvas->text_orientation)
    XftTextExtents8(ctxcanvas->dpy, ctxcanvas->ctxplus->flat_font, (XftChar8*)text, len, &extents);
  else
    XftTextExtents8(ctxcanvas->dpy, ctxcanvas->ctxplus->font, (XftChar8*)text, len, &extents);

  if (width)  *width  = extents.width+extents.x;
  if (height) *height = extents.height+extents.y;
}

static void cdtext(cdCtxCanvas *ctxcanvas, int x, int y, const char *text, int len)
{
  XGlyphInfo extents;
  int ox, oy, w, h, descent, dir = -1;

  if (!ctxcanvas->ctxplus->font)
    return;

  if (ctxcanvas->canvas->text_orientation)
    XftTextExtents8(ctxcanvas->dpy, ctxcanvas->ctxplus->flat_font, (XftChar8*)text, len, &extents);
  else
    XftTextExtents8(ctxcanvas->dpy, ctxcanvas->ctxplus->font, (XftChar8*)text, len, &extents);
  w = extents.width+extents.x;
  h = extents.height+extents.y;

  descent = ctxcanvas->ctxplus->font->descent;

  ox = x; 
  oy = y;

  switch (ctxcanvas->canvas->text_alignment)
  {
  case CD_BASE_RIGHT:
  case CD_NORTH_EAST:
  case CD_EAST:
  case CD_SOUTH_EAST:
    x = x - w;    
    break;
  case CD_BASE_CENTER:
  case CD_CENTER:
  case CD_NORTH:
  case CD_SOUTH:
    x = x - w/2;  
    break;
  case CD_BASE_LEFT:
  case CD_NORTH_WEST:
  case CD_WEST:
  case CD_SOUTH_WEST:
    x = x;         
    break;
  }

  if (ctxcanvas->canvas->invert_yaxis)
    dir = 1;

  switch (ctxcanvas->canvas->text_alignment)
  {
  case CD_BASE_LEFT:
  case CD_BASE_CENTER:
  case CD_BASE_RIGHT:
    y = y;
    break;
  case CD_SOUTH_EAST:
  case CD_SOUTH_WEST:
  case CD_SOUTH:
    y = y - dir*descent;
    break;
  case CD_NORTH_EAST:
  case CD_NORTH:
  case CD_NORTH_WEST:
    y = y + dir*(h - descent);
    break;
  case CD_CENTER:
  case CD_EAST:
  case CD_WEST:
    y = y + dir*(h/2 - descent);
    break;
  }

  if (ctxcanvas->canvas->text_orientation)
  {
    double angle = CD_DEG2RAD*ctxcanvas->canvas->text_orientation;
    double cos_angle = cos(angle);
    double sin_angle = sin(angle);

    /* manually rotate the initial point */
    cdRotatePoint(ctxcanvas->canvas, x, y, ox, oy, &x, &y, sin_angle, cos_angle);
  }

#ifndef CD_XRENDER_MATRIX
  if (ctxcanvas->canvas->use_matrix)
    cdMatrixTransformPoint(ctxcanvas->xmatrix, x, y, &x, &y);
#endif

  if (!ctxcanvas->canvas->new_region)
  {
    XftColor xftcolor;
    XRenderColor rendercolor;
    xrInitColor(&rendercolor, ctxcanvas->canvas->foreground);
    XftColorAllocValue(ctxcanvas->dpy, ctxcanvas->vis, ctxcanvas->colormap, &rendercolor, &xftcolor);
    XftDrawString8(ctxcanvas->ctxplus->draw, &xftcolor, ctxcanvas->ctxplus->font, x, y, (XftChar8*)text, len);
  }
}

/******************************************************************/

#if (RENDER_MAJOR>0 || RENDER_MINOR>=10)
static void set_linegradient_attrib(cdCtxCanvas* ctxcanvas, char* data)
{
  if (ctxcanvas->ctxplus->linegradient_pic)
  {
    XRenderFreePicture(ctxcanvas->dpy, ctxcanvas->ctxplus->linegradient_pic);
    ctxcanvas->ctxplus->linegradient_pic = 0;
  }

  if (data)
  {
    XRenderPictureAttributes pa;
    XFixed stops[2];
    XRenderColor colors[2];
    int x1, y1, x2, y2;
    sscanf(data, "%d %d %d %d", &x1, &y1, &x2, &y2);

    if (ctxcanvas->canvas->invert_yaxis)
    {
      y1 = _cdInvertYAxis(ctxcanvas->canvas, y1);
      y2 = _cdInvertYAxis(ctxcanvas->canvas, y2);
    }

    stops[0] = XDoubleToFixed(0.0);
    stops[1] = XDoubleToFixed(1.0);

    xrInitColor(&colors[0], ctxcanvas->canvas->foreground);
    xrInitColor(&colors[1], ctxcanvas->canvas->background);

    ctxcanvas->ctxplus->linegradient.p1.x = XDoubleToFixed((double)x1); 
    ctxcanvas->ctxplus->linegradient.p1.y = XDoubleToFixed((double)y1); 
    ctxcanvas->ctxplus->linegradient.p2.x = XDoubleToFixed((double)x2); 
    ctxcanvas->ctxplus->linegradient.p2.y = XDoubleToFixed((double)y2); 

    ctxcanvas->ctxplus->linegradient_pic = XRenderCreateLinearGradient(ctxcanvas->dpy, &ctxcanvas->ctxplus->linegradient, stops, colors, 2);
    pa.repeat = 1;
    XRenderChangePicture(ctxcanvas->dpy, ctxcanvas->ctxplus->linegradient_pic, CPRepeat, &pa);

    ctxcanvas->ctxplus->fill_picture = ctxcanvas->ctxplus->linegradient_pic;
  }
  else
    cdinteriorstyle(ctxcanvas, ctxcanvas->canvas->interior_style);
}

static char* get_linegradient_attrib(cdCtxCanvas* ctxcanvas)
{
  static char data[100];

  sprintf(data, "%d %d %d %d", (int)XFixedToDouble(ctxcanvas->ctxplus->linegradient.p1.x),
                               (int)XFixedToDouble(ctxcanvas->ctxplus->linegradient.p1.y),
                               (int)XFixedToDouble(ctxcanvas->ctxplus->linegradient.p2.x),
                               (int)XFixedToDouble(ctxcanvas->ctxplus->linegradient.p2.y));

  return data;
}

static cdAttribute linegradient_attrib =
{
  "LINEGRADIENT",
  set_linegradient_attrib,
  get_linegradient_attrib
}; 
#endif

static void set_aa_attrib(cdCtxCanvas* ctxcanvas, char* data)
{
  if (!data || data[0] == '0')
    ctxcanvas->ctxplus->antialias = 0;
  else
    ctxcanvas->ctxplus->antialias = 1;

  ctxcanvas->ctxplus->maskFormat = XRenderFindStandardFormat(ctxcanvas->dpy, ctxcanvas->ctxplus->antialias? PictStandardA8: PictStandardA1);
}

static char* get_aa_attrib(cdCtxCanvas* ctxcanvas)
{
  if (ctxcanvas->ctxplus->antialias)
    return "1";
  else
    return "0";
}

static cdAttribute aa_attrib =
{
  "ANTIALIAS",
  set_aa_attrib,
  get_aa_attrib
}; 

static char cdxXRenderVersion[50] = "";

static char* get_version_attrib(cdCtxCanvas* ctxcanvas)
{
  if (ctxcanvas->ctxplus)
    return cdxXRenderVersion;
  else
    return "0";
}

static cdAttribute version_attrib =
{
  "XRENDERVERSION",
  NULL,
  get_version_attrib
}; 

static void cdkillcanvas(cdCtxCanvas *ctxcanvas)
{
  /* fill_picture is NOT released, since it is a pointer to one of the other pictures */
  if (ctxcanvas->ctxplus->solid_pic) 
    XRenderFreePicture(ctxcanvas->dpy, ctxcanvas->ctxplus->solid_pic);

  if (ctxcanvas->ctxplus->pattern_pic) 
    XRenderFreePicture(ctxcanvas->dpy, ctxcanvas->ctxplus->pattern_pic);

#if (RENDER_MAJOR>0 || RENDER_MINOR>=10)
  if (ctxcanvas->ctxplus->linegradient_pic) 
    XRenderFreePicture(ctxcanvas->dpy, ctxcanvas->ctxplus->linegradient_pic);
#endif

  if (ctxcanvas->ctxplus->flat_font)
    XftFontClose(ctxcanvas->dpy, ctxcanvas->ctxplus->flat_font);

  if (ctxcanvas->ctxplus->font)
    XftFontClose(ctxcanvas->dpy, ctxcanvas->ctxplus->font);

  /* call original method */
  ctxcanvas->ctxplus->cxKillCanvas(ctxcanvas);

  XftDrawDestroy(ctxcanvas->ctxplus->draw);
  free(ctxcanvas->ctxplus);
}

static void xrInitTable(cdCanvas* canvas)
{
  /* set new methods */
  canvas->cxKillCanvas = cdkillcanvas;
  canvas->cxClip = cdclip;
  canvas->cxClipArea = cdcliparea;
  canvas->cxTransform = cdtransform;
  canvas->cxInteriorStyle = cdinteriorstyle;
  canvas->cxBackOpacity = cdbackopacity;
  canvas->cxForeground = cdforeground;
  canvas->cxBackground = cdbackground;
  canvas->cxHatch = cdhatch;
  canvas->cxStipple = cdstipple;
  canvas->cxPattern = cdpattern;
  canvas->cxTextOrientation = cdtextorientation;

  canvas->cxPixel = cdpixel;
  canvas->cxClear = cdclear;

  canvas->cxLine = cdline;
  canvas->cxRect = cdrect;
  canvas->cxBox = cdbox;
  canvas->cxArc = cdSimArc;
  canvas->cxSector = cdSimSector;
  canvas->cxChord = cdSimChord;
  canvas->cxPoly = cdpoly;

  canvas->cxFLine = cdfline;
  canvas->cxFRect = cdfSimRect;
  canvas->cxFBox = cdfSimBox;
  canvas->cxFArc = cdfSimArc;
  canvas->cxFSector = cdfSimSector;
  canvas->cxFChord = cdfSimChord;
  canvas->cxFPoly = cdfpoly;

  /* TODO: canvas->cxPutImageRectRGBA = cdputimagerectrgba; */

  canvas->cxFont = cdfont;
  canvas->cxNativeFont = cdnativefont;
  canvas->cxGetTextSize = cdgettextsize;
  canvas->cxText = cdtext;
  canvas->cxGetFontDim = cdgetfontdim;
}

static void xrCreateContextPlus(cdCtxCanvas *ctxcanvas)
{
  ctxcanvas->ctxplus = (cdxContextPlus *)malloc(sizeof(cdxContextPlus));
  memset(ctxcanvas->ctxplus, 0, sizeof(cdxContextPlus));

  if (cdxXRenderVersion[0] == 0 && XftDefaultHasRender(ctxcanvas->dpy))
    sprintf(cdxXRenderVersion,"%d.%d", RENDER_MAJOR, RENDER_MINOR);

  cdRegisterAttribute(ctxcanvas->canvas, &aa_attrib);
  cdRegisterAttribute(ctxcanvas->canvas, &version_attrib);
#if (RENDER_MAJOR>0 || RENDER_MINOR>=10)
  cdRegisterAttribute(ctxcanvas->canvas, &linegradient_attrib);
#endif

  ctxcanvas->ctxplus->draw = XftDrawCreate(ctxcanvas->dpy, ctxcanvas->wnd, ctxcanvas->vis, ctxcanvas->colormap);
  ctxcanvas->ctxplus->dst_picture = XftDrawPicture(ctxcanvas->ctxplus->draw);
  ctxcanvas->ctxplus->maskFormat = XRenderFindStandardFormat(ctxcanvas->dpy, PictStandardA8);

  ctxcanvas->ctxplus->antialias = 1;
}

/*******************************************************************************************************/

static cdContext cdDBufferContext = {0,0,NULL,NULL,NULL,NULL};
static cdContext cdNativeWindowContext = {0,0,NULL,NULL,NULL,NULL};
static cdContext cdImageContext = {0,0,NULL,NULL,NULL,NULL};

static void (*cdcreatecanvasDBUFFER)(cdCanvas* canvas, void* data) = NULL;
static void (*cdcreatecanvasNATIVE)(cdCanvas* canvas, void* data) = NULL;
static void (*cdcreatecanvasIMAGE)(cdCanvas* canvas, void* data) = NULL;

static void (*cdinittableDBUFFER)(cdCanvas* canvas) = NULL;
static void (*cdinittableNATIVE)(cdCanvas* canvas) = NULL;
static void (*cdinittableIMAGE)(cdCanvas* canvas) = NULL;

static void (*cdkillcanvasDBUFFER)(cdCtxCanvas* ctxcanvas) = NULL;
static void (*cdkillcanvasNATIVE)(cdCtxCanvas* ctxcanvas) = NULL;
static void (*cdkillcanvasIMAGE)(cdCtxCanvas* ctxcanvas) = NULL;

static void xrCreateCanvasDBUFFER(cdCanvas* canvas, void *data)
{
  cdcreatecanvasDBUFFER(canvas, data);  /* call original first */
  xrCreateContextPlus(canvas->ctxcanvas);
}

static void xrInitTableDBUFFER(cdCanvas* canvas)
{
  cdinittableDBUFFER(canvas);
  if (!cdkillcanvasDBUFFER) cdkillcanvasDBUFFER = canvas->cxKillCanvas;
  canvas->ctxcanvas->ctxplus->cxKillCanvas = cdkillcanvasDBUFFER;
  xrInitTable(canvas);
}

cdContext* cdContextDBufferPlus(void)
{
  if (!cdDBufferContext.plus)
  {
    int old_plus = cdUseContextPlus(0);  /* disable context plus */
    cdDBufferContext = *cdContextDBuffer();  /* copy original context */
    cdDBufferContext.plus = 1; /* mark as plus */
    cdDBufferContext.caps |= CD_CAP_FPRIMTIVES;

    /* save original methods */
    cdcreatecanvasDBUFFER = cdDBufferContext.cxCreateCanvas;
    cdinittableDBUFFER = cdDBufferContext.cxInitTable;

    /* replace by new methods */
    cdDBufferContext.cxCreateCanvas = xrCreateCanvasDBUFFER;
    cdDBufferContext.cxInitTable = xrInitTableDBUFFER;

    cdUseContextPlus(old_plus);  /* enable context plus */
  }
  return &cdDBufferContext;
}

static void xrCreateCanvasNATIVE(cdCanvas* canvas, void *data)
{
  cdcreatecanvasNATIVE(canvas, data);
  xrCreateContextPlus(canvas->ctxcanvas);
}

static void xrInitTableNATIVE(cdCanvas* canvas)
{
  cdinittableNATIVE(canvas);
  if (!cdkillcanvasNATIVE) cdkillcanvasNATIVE = canvas->cxKillCanvas;
  canvas->ctxcanvas->ctxplus->cxKillCanvas = cdkillcanvasNATIVE;
  xrInitTable(canvas);
}

cdContext* cdContextNativeWindowPlus(void)
{
  if (!cdNativeWindowContext.plus)
  {
    int old_plus = cdUseContextPlus(0);
    cdNativeWindowContext = *cdContextNativeWindow();
    cdcreatecanvasNATIVE = cdNativeWindowContext.cxCreateCanvas;
    cdinittableNATIVE = cdNativeWindowContext.cxInitTable;
    cdNativeWindowContext.cxCreateCanvas = xrCreateCanvasNATIVE;
    cdNativeWindowContext.cxInitTable = xrInitTableNATIVE;
    cdNativeWindowContext.plus = 1;
    cdNativeWindowContext.caps |= CD_CAP_FPRIMTIVES;
    cdUseContextPlus(old_plus);
  }
  return &cdNativeWindowContext;
}

static void xrCreateCanvasIMAGE(cdCanvas* canvas, void *data)
{
  cdcreatecanvasIMAGE(canvas, data);
  xrCreateContextPlus(canvas->ctxcanvas);
}

static void xrInitTableIMAGE(cdCanvas* canvas)
{
  cdinittableIMAGE(canvas);
  if (!cdkillcanvasIMAGE) cdkillcanvasIMAGE = canvas->cxKillCanvas;
  canvas->ctxcanvas->ctxplus->cxKillCanvas = cdkillcanvasIMAGE;
  xrInitTable(canvas);
}

cdContext* cdContextImagePlus(void)
{
  if (!cdImageContext.plus)
  {
    int old_plus = cdUseContextPlus(0);
    cdImageContext = *cdContextImage();
    cdcreatecanvasIMAGE = cdImageContext.cxCreateCanvas;
    cdinittableIMAGE = cdImageContext.cxInitTable;
    cdImageContext.cxCreateCanvas = xrCreateCanvasIMAGE;
    cdImageContext.cxInitTable = xrInitTableIMAGE;
    cdImageContext.plus = 1;
    cdImageContext.caps |= CD_CAP_FPRIMTIVES;
    cdUseContextPlus(old_plus);
  }
  return &cdImageContext;
}
