/** \file
* \brief Cairo Base Driver
*
* See Copyright Notice in cd.h
*/

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <math.h>

#include <glib.h>
#include <pango/pangocairo.h>

#include "cdcairoctx.h"


#ifndef PANGO_VERSION_CHECK
#define PANGO_VERSION_CHECK(x,y,z) (0)
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static int sStrIsAscii(const char* str)
{
  while(*str)
  {
    int c = *str;
    if (c < 0)
      return 0;
    str++;
  }
  return 1;
}

static char* sStrToUTF8(const char *str, const char* charset, int length)
{
  return g_convert(str, length, "UTF-8", charset, NULL, NULL, NULL);
}

static char* sStrConvertToUTF8(cdCtxCanvas *ctxcanvas, const char* str, int length)
{
  const char *charset = NULL;

  if (!str || *str == 0)
    return (char*)str;

  if (g_get_charset(&charset))  /* current locale is already UTF-8 */
  {
    if (g_utf8_validate(str, -1, NULL))
    {
      return (char*)str;
    }
    else
    {
      ctxcanvas->strLastConvertUTF8 = sStrToUTF8(str, "ISO8859-1", length);   /* if string is not UTF-8, assume ISO8859-1 */

      if (!ctxcanvas->strLastConvertUTF8)
        return (char*)str;

      return ctxcanvas->strLastConvertUTF8;
    }
  }
  else
  {
    if (sStrIsAscii(str) || !charset)
    {
      return (char*)str;
    }
    else if (charset)
    {    
      ctxcanvas->strLastConvertUTF8 = sStrToUTF8(str, charset, length);

      if (!ctxcanvas->strLastConvertUTF8)
        return (char*)str;

      return ctxcanvas->strLastConvertUTF8;
    }
  }

  return (char*)str;
}

static void sUpdateFill(cdCtxCanvas *ctxcanvas, int fill)
{
  if (fill == 0 || ctxcanvas->canvas->interior_style == CD_SOLID)
  {
    if (ctxcanvas->last_source == 0)
      return;

    cairo_set_source(ctxcanvas->cr, ctxcanvas->solid);
    ctxcanvas->last_source = 0;
  }
  else
  {
    if (ctxcanvas->last_source == 1)
      return;

    cairo_set_source(ctxcanvas->cr, ctxcanvas->pattern);
    ctxcanvas->last_source = 1;
  }
}

/******************************************************/

void cdcairoKillCanvas(cdCtxCanvas *ctxcanvas)
{
  if (ctxcanvas->solid)
    cairo_pattern_destroy(ctxcanvas->solid);

  if (ctxcanvas->pattern)
    cairo_pattern_destroy(ctxcanvas->pattern);

  if (ctxcanvas->fontdesc) pango_font_description_free(ctxcanvas->fontdesc);
  if (ctxcanvas->fontlayout)  g_object_unref(ctxcanvas->fontlayout);
  if (ctxcanvas->fontcontext) g_object_unref(ctxcanvas->fontcontext);

  if (ctxcanvas->strLastConvertUTF8)
    g_free(ctxcanvas->strLastConvertUTF8);

  if (ctxcanvas->cr)
    cairo_destroy(ctxcanvas->cr);

  memset(ctxcanvas, 0, sizeof(cdCtxCanvas));
  free(ctxcanvas);
}

/******************************************************/

static void cdflush(cdCtxCanvas *ctxcanvas)
{
  cairo_surface_flush(cairo_get_target(ctxcanvas->cr));
  cairo_show_page(ctxcanvas->cr);
}

/******************************************************/

static void cdfcliparea(cdCtxCanvas *ctxcanvas, double xmin, double xmax, double ymin, double ymax)
{
  if (ctxcanvas->canvas->clip_mode != CD_CLIPAREA)
    return;

  cairo_reset_clip(ctxcanvas->cr);
  cairo_rectangle(ctxcanvas->cr, xmin, ymin, xmax-xmin+1, ymax-ymin+1);
  cairo_clip(ctxcanvas->cr);
}

static void cdcliparea(cdCtxCanvas *ctxcanvas, int xmin, int xmax, int ymin, int ymax)
{
  cdfcliparea(ctxcanvas, (double)xmin, (double)xmax, (double)ymin, (double)ymax);
}

static int cdclip(cdCtxCanvas *ctxcanvas, int mode)
{
  cairo_reset_clip(ctxcanvas->cr);

  switch (mode)
  {
  case CD_CLIPOFF:
    cairo_rectangle(ctxcanvas->cr, 0, 0, ctxcanvas->canvas->w, ctxcanvas->canvas->h);
    break;
  case CD_CLIPAREA:
      cairo_rectangle(ctxcanvas->cr, ctxcanvas->canvas->clip_frect.xmin, 
                                     ctxcanvas->canvas->clip_frect.ymin, 
                                     ctxcanvas->canvas->clip_frect.xmax, 
                                     ctxcanvas->canvas->clip_frect.ymax);
      break;
  case CD_CLIPPOLYGON:
    {
      int hole_index = 0;
      int i;

      if (ctxcanvas->canvas->clip_poly)
      {
        cdPoint *poly = ctxcanvas->canvas->clip_poly; 
        cairo_move_to(ctxcanvas->cr, poly[0].x, poly[0].y);
        for (i=1; i<ctxcanvas->canvas->clip_poly_n; i++)
        {
          if (ctxcanvas->holes && i == ctxcanvas->poly_holes[hole_index])
          {
            cairo_move_to(ctxcanvas->cr, poly[i].x, poly[i].y);
            hole_index++;
          }
          else
            cairo_line_to(ctxcanvas->cr, poly[i].x, poly[i].y);
        }
      }
      else if (ctxcanvas->canvas->clip_fpoly)
      {
        cdfPoint *poly = ctxcanvas->canvas->clip_fpoly; 
        cairo_move_to(ctxcanvas->cr, poly[0].x, poly[0].y);
        for (i=1; i<ctxcanvas->canvas->clip_poly_n; i++)
        {
          if (ctxcanvas->holes && i == ctxcanvas->poly_holes[hole_index])
          {
            cairo_move_to(ctxcanvas->cr, poly[i].x, poly[i].y);
            hole_index++;
          }
          else
            cairo_line_to(ctxcanvas->cr, poly[i].x, poly[i].y);
        }
      }
      break;
    }
  case CD_CLIPREGION:
    break;
  }

  cairo_clip(ctxcanvas->cr);

  return mode;
}

/******************************************************/

#define CD_ALPHAPRE(_src, _alpha) (((_src)*(_alpha))/255)

static unsigned int sEncodeRGBA(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
  /* Pre-multiplied alpha */
  if (a != 255)
  {
    r = CD_ALPHAPRE(r, a);
    g = CD_ALPHAPRE(g, a);
    b = CD_ALPHAPRE(b, a);
  }

  return (((unsigned int)a) << 24) |
         (((unsigned int)r) << 16) |
         (((unsigned int)g) <<  8) |
         (((unsigned int)b) <<  0);
}

static void make_pattern(cdCtxCanvas *ctxcanvas, int n, int m, void* userdata, int (*data2rgb)(cdCtxCanvas *ctxcanvas, int n, int i, int j, void* userdata, unsigned char*r, unsigned char*g, unsigned char*b, unsigned char*a))
{
  int i, j, offset, ret, stride;
  unsigned char r, g, b, a;
  cairo_surface_t* pattern_surface;
  unsigned int* data;

  /* CAIRO_FORMAT_ARGB32 each pixel is a 32-bit quantity, with alpha in the upper 8 bits, then red, then green, then blue. 
     The 32-bit quantities are stored native-endian. 
     Pre-multiplied alpha is used. (That is, 50% transparent red is 0x80800000, not 0x80ff0000.) */
  pattern_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, n, m);
  if (cairo_surface_status(pattern_surface) != CAIRO_STATUS_SUCCESS)
  {
    cairo_surface_destroy(pattern_surface);
    return;
  }

  data = (unsigned int*)cairo_image_surface_get_data(pattern_surface);
  stride = cairo_image_surface_get_stride(pattern_surface);
  offset = stride/4 - n;

  for (j = 0; j < m; j++)
  {
    for (i = 0; i < n; i++)
    {
      /* internal transform, affects also pattern orientation */
      if (ctxcanvas->canvas->invert_yaxis)
        ret = data2rgb(ctxcanvas, n, i, m-1-j, userdata, &r, &g, &b, &a);
      else
        ret = data2rgb(ctxcanvas, n, i, j, userdata, &r, &g, &b, &a);

      if (ret == -1)
      {
        data++;  /* already transparent */
        continue;
      }

      *data++ = sEncodeRGBA(r, g, b, a);
    }

    if (offset)
      data += offset;
  }

  if (ctxcanvas->pattern)
    cairo_pattern_destroy(ctxcanvas->pattern);

  ctxcanvas->pattern = cairo_pattern_create_for_surface(pattern_surface);
  cairo_pattern_reference(ctxcanvas->pattern);
  cairo_pattern_set_extend(ctxcanvas->pattern, CAIRO_EXTEND_REPEAT);

  cairo_surface_destroy(pattern_surface);
}

static int long2rgb(cdCtxCanvas *ctxcanvas, int n, int i, int j, void* data, unsigned char*r, unsigned char*g, unsigned char*b, unsigned char*a)
{
  long* long_data = (long*)data;
  long c = long_data[j*n+i];
  (void)ctxcanvas;
  cdDecodeColor(c, r, g, b);
  *a = cdDecodeAlpha(c);
  return 1;
}

static void cdpattern(cdCtxCanvas *ctxcanvas, int n, int m, const long *pattern)
{
  make_pattern(ctxcanvas, n, m, (void*)pattern, long2rgb);
  cairo_set_source(ctxcanvas->cr, ctxcanvas->pattern);
  ctxcanvas->last_source = 1;
}

static int uchar2rgb(cdCtxCanvas *ctxcanvas, int n, int i, int j, void* data, unsigned char*r, unsigned char*g, unsigned char*b, unsigned char*a)
{
  unsigned char* uchar_data = (unsigned char*)data;
  if (uchar_data[j*n+i])
  {
    cdDecodeColor(ctxcanvas->canvas->foreground, r, g, b);
    *a = cdDecodeAlpha(ctxcanvas->canvas->foreground);
  }
  else
  {
    if (ctxcanvas->canvas->back_opacity == CD_TRANSPARENT)
      return -1;
    else
    {
      cdDecodeColor(ctxcanvas->canvas->background, r, g, b);
      *a = cdDecodeAlpha(ctxcanvas->canvas->background);
    }
  }

  return 1;
}

static void cdstipple(cdCtxCanvas *ctxcanvas, int n, int m, const unsigned char *stipple)
{
  make_pattern(ctxcanvas, n, m, (void*)stipple, uchar2rgb);
  cairo_set_source(ctxcanvas->cr, ctxcanvas->pattern);
  ctxcanvas->last_source = 1;
}

static int cdhatch(cdCtxCanvas *ctxcanvas, int style)
{
  int hsize = ctxcanvas->hatchboxsize;
  int hhalf = hsize / 2;
  cairo_surface_t* hatch_surface;
  cairo_t* cr;

  hatch_surface = cairo_surface_create_similar(cairo_get_target(ctxcanvas->cr), CAIRO_CONTENT_COLOR_ALPHA, hsize, hsize);
  cr = cairo_create(hatch_surface);

  if (ctxcanvas->canvas->back_opacity == CD_OPAQUE)
  {
    cairo_set_source_rgba(cr, cdCairoGetRed(ctxcanvas->canvas->background), cdCairoGetGreen(ctxcanvas->canvas->background), cdCairoGetBlue(ctxcanvas->canvas->background), cdCairoGetAlpha(ctxcanvas->canvas->background));
    cairo_rectangle(cr, 0, 0, hsize, hsize);
    cairo_fill(cr);
  }

  cairo_set_source_rgba(cr, cdCairoGetRed(ctxcanvas->canvas->foreground), cdCairoGetGreen(ctxcanvas->canvas->foreground), cdCairoGetBlue(ctxcanvas->canvas->foreground), cdCairoGetAlpha(ctxcanvas->canvas->foreground));

  cairo_set_line_cap(cr, CAIRO_LINE_CAP_SQUARE); 
  cairo_set_line_width(cr, 1);

  switch(style)
  {
  case CD_HORIZONTAL:
    cairo_move_to(cr, 0.0, (double)hhalf);
    cairo_line_to(cr, (double)hsize, (double)hhalf);
    break;
  case CD_VERTICAL:
    cairo_move_to(cr, (double)hhalf, 0.0);
    cairo_line_to(cr, (double)hhalf, (double)hsize);
    break;
  case CD_BDIAGONAL:
    cairo_move_to(cr, 0.0, (double)hsize);
    cairo_line_to(cr, (double)hsize, 0.0);
    break;
  case CD_FDIAGONAL:
    cairo_move_to(cr, 0.0, 0.0);
    cairo_line_to(cr, (double)hsize, (double)hsize);
    break;
  case CD_CROSS:
    cairo_move_to(cr, (double)hsize, 0.0);
    cairo_line_to(cr, (double)hsize, (double)hsize);
    cairo_move_to(cr, 0.0, (double)hhalf);
    cairo_line_to(cr, (double)hsize, (double)hhalf);
    break;
  case CD_DIAGCROSS:
    cairo_move_to(cr, 0.0, 0.0);
    cairo_line_to(cr, (double)hsize, (double)hsize);
    cairo_move_to(cr, (double)hsize, 0.0);
    cairo_line_to(cr, 0.0, (double)hsize);
    break;
  }

  cairo_stroke(cr);

  if (ctxcanvas->pattern)
    cairo_pattern_destroy(ctxcanvas->pattern);

  ctxcanvas->pattern = cairo_pattern_create_for_surface(hatch_surface);
  cairo_pattern_reference(ctxcanvas->pattern);
  cairo_pattern_set_extend(ctxcanvas->pattern, CAIRO_EXTEND_REPEAT);

  cairo_surface_destroy(hatch_surface);
  cairo_destroy(cr);

  cairo_set_source(ctxcanvas->cr, ctxcanvas->pattern);
  ctxcanvas->last_source = 1;

  return style;
}

/******************************************************/
/* attributes                                         */
/******************************************************/

static int cdinteriorstyle (cdCtxCanvas* ctxcanvas, int style)
{
  switch (style)
  {
  case CD_SOLID:
    cairo_set_source(ctxcanvas->cr, ctxcanvas->solid);
    ctxcanvas->last_source = 0;
    break;
  /* must recriate the current pattern */
  case CD_HATCH:
    cdhatch(ctxcanvas, ctxcanvas->canvas->hatch_style);
    break;
  case CD_STIPPLE:
    cdstipple(ctxcanvas, ctxcanvas->canvas->stipple_w, ctxcanvas->canvas->stipple_h, ctxcanvas->canvas->stipple);
    break;
  case CD_PATTERN:
    cdpattern(ctxcanvas, ctxcanvas->canvas->pattern_w, ctxcanvas->canvas->pattern_h, ctxcanvas->canvas->pattern);
    break;
  }

  return style;
}

static int cdlinestyle(cdCtxCanvas *ctxcanvas, int style)
{
  double dashes[10];

  switch (style)
  {
  case CD_CONTINUOUS : /* empty dash */
    cairo_set_dash(ctxcanvas->cr, 0, 0, 0);
    break;
  case CD_DASHED :
    dashes[0] = 6.0;  dashes[1] = 2.0;
    cairo_set_dash(ctxcanvas->cr, dashes, 2, 0);
    break;
  case CD_DOTTED :
    dashes[0] = 2.0;  dashes[1] = 2.0;
    cairo_set_dash(ctxcanvas->cr, dashes, 2, 0);
    break;
  case CD_DASH_DOT :
    dashes[0] = 6.0;  dashes[1] = 2.0;
    dashes[2] = 2.0;  dashes[3] = 2.0;
    cairo_set_dash(ctxcanvas->cr, dashes, 4, 0);
    break;
  case CD_DASH_DOT_DOT :
    dashes[0] = 6.0;  dashes[1] = 2.0;
    dashes[2] = 2.0;  dashes[3] = 2.0;
    dashes[4] = 2.0;  dashes[5] = 2.0;
    cairo_set_dash(ctxcanvas->cr, dashes, 6, 0);
    break;
  case CD_CUSTOM :
    {
      int i;
      double* dash_style = (double*)malloc(sizeof(double)*ctxcanvas->canvas->line_dashes_count);

      for (i = 0; i < ctxcanvas->canvas->line_dashes_count; i++)
        dash_style[i] = (double)ctxcanvas->canvas->line_dashes[i];

      cairo_set_dash(ctxcanvas->cr, dash_style, ctxcanvas->canvas->line_dashes_count, 0);

      free(dash_style);
    }
    break;
  }

  return style;
}

static int cdlinewidth(cdCtxCanvas *ctxcanvas, int width)
{
  if(width == 0)
    width = 1;

  cairo_set_line_width(ctxcanvas->cr, (double)width);

  return width;
}

static int cdlinejoin(cdCtxCanvas *ctxcanvas, int join)
{
  int cd2ps_join[] = {CAIRO_LINE_JOIN_MITER, CAIRO_LINE_JOIN_BEVEL, CAIRO_LINE_JOIN_ROUND};

  cairo_set_line_join(ctxcanvas->cr, cd2ps_join[join]); 

  return join;
}

static int cdlinecap(cdCtxCanvas *ctxcanvas, int cap)
{
  int cd2pdf_cap[] = {CAIRO_LINE_CAP_BUTT, CAIRO_LINE_CAP_SQUARE, CAIRO_LINE_CAP_ROUND};

  cairo_set_line_cap(ctxcanvas->cr, cd2pdf_cap[cap]); 

  return cap;
}

static int cdfont(cdCtxCanvas *ctxcanvas, const char *typeface, int style, int size)
{
  int is_italic = 0, is_bold = 0;   /* default is CD_PLAIN */
  int is_strikeout = 0, is_underline = 0;
  char font[256];
  PangoAttrList *attrs;

  if (cdStrEqualNoCase(typeface, "Courier") || cdStrEqualNoCase(typeface, "Courier New"))
    typeface = "Monospace";
  else if (cdStrEqualNoCase(typeface, "Times") || cdStrEqualNoCase(typeface, "Times New Roman"))
    typeface = "Serif";
  else if (cdStrEqualNoCase(typeface, "Helvetica") || cdStrEqualNoCase(typeface, "Arial"))
    typeface = "Sans";

  if (style & CD_BOLD)
    is_bold = 1;

  if (style & CD_ITALIC)
    is_italic = 1;

  if (style & CD_UNDERLINE)
    is_underline = 1;

  if (style & CD_STRIKEOUT)
    is_strikeout = 1;

  size = cdGetFontSizePoints(ctxcanvas->canvas, size);

  sprintf(font, "%s, %s%s%d", typeface, is_bold?"Bold ":"", is_italic?"Italic ":"", size);

  if (ctxcanvas->fontdesc) 
    pango_font_description_free(ctxcanvas->fontdesc);

  ctxcanvas->fontdesc = pango_font_description_from_string(font);

  if (!ctxcanvas->fontdesc)
    return 0;

  if (ctxcanvas->fontlayout)  
    g_object_unref(ctxcanvas->fontlayout);

  ctxcanvas->fontlayout = pango_layout_new(ctxcanvas->fontcontext);
  pango_layout_set_font_description(ctxcanvas->fontlayout, ctxcanvas->fontdesc);

  attrs = pango_attr_list_new();
  pango_attr_list_insert(attrs, pango_attribute_copy(pango_attr_strikethrough_new(is_strikeout ? TRUE : FALSE)));
  pango_attr_list_insert(attrs, pango_attribute_copy(pango_attr_underline_new(is_underline ? PANGO_UNDERLINE_SINGLE : PANGO_UNDERLINE_NONE)));
  pango_layout_set_attributes(ctxcanvas->fontlayout, attrs);

  pango_attr_list_unref(attrs);

  pango_cairo_update_layout(ctxcanvas->cr, ctxcanvas->fontlayout);

  return 1;
}

static void cdgetfontdim(cdCtxCanvas *ctxcanvas, int *max_width, int *height, int *ascent, int *descent)
{
  PangoFontMetrics* metrics;
  int charwidth, charheight, charascent, chardescent;

  if(!ctxcanvas->fontdesc)
    return;

  pango_cairo_update_layout(ctxcanvas->cr, ctxcanvas->fontlayout);
  metrics = pango_context_get_metrics(ctxcanvas->fontcontext, ctxcanvas->fontdesc, pango_context_get_language(ctxcanvas->fontcontext));
  charascent  = pango_font_metrics_get_ascent(metrics);
  chardescent = pango_font_metrics_get_descent(metrics);
  charheight  = charascent + chardescent;
  charwidth   = pango_font_metrics_get_approximate_char_width(metrics);

  if (max_width) *max_width = (((charwidth)   + PANGO_SCALE/2) / PANGO_SCALE);
  if (height)    *height    = (((charheight)  + PANGO_SCALE/2) / PANGO_SCALE);
  if (ascent)    *ascent    = (((charascent)  + PANGO_SCALE/2) / PANGO_SCALE);
  if (descent)   *descent   = (((chardescent) + PANGO_SCALE/2) / PANGO_SCALE);

  pango_font_metrics_unref(metrics); 
}

static long cdforeground(cdCtxCanvas *ctxcanvas, long color)
{
  if (ctxcanvas->solid)
    cairo_pattern_destroy(ctxcanvas->solid);

  cairo_set_source_rgba(ctxcanvas->cr, cdCairoGetRed(color),
                                       cdCairoGetGreen(color),
                                       cdCairoGetBlue(color),
                                       cdCairoGetAlpha(color));
  ctxcanvas->solid = cairo_get_source(ctxcanvas->cr);
  cairo_pattern_reference(ctxcanvas->solid);
  ctxcanvas->last_source = 0;
  return color;
}


/******************************************************/

static void sSetTransform(cdCtxCanvas *ctxcanvas, const double* matrix)
{
  if (matrix)
  {
    cairo_matrix_t mtx;

    /* configure a bottom-up coordinate system */
    mtx.xx = 1; mtx.yx = 0;
    mtx.xy = 0; mtx.yy = -1;
    mtx.x0 = 0; mtx.y0 = (ctxcanvas->canvas->h-1);
    cairo_transform(ctxcanvas->cr, &mtx);

    mtx.xx = matrix[0]; mtx.yx = matrix[1];
    mtx.xy = matrix[2]; mtx.yy = matrix[3];
    mtx.x0 = matrix[4]; mtx.y0 = matrix[5];
    cairo_transform(ctxcanvas->cr, &mtx);
  }
  else if (ctxcanvas->rotate_angle)
  {
    /* rotation = translate to point + rotation + translate back */
    /* the rotation must be corrected because of the Y axis orientation */
    cairo_translate(ctxcanvas->cr, ctxcanvas->rotate_center_x, _cdInvertYAxis(ctxcanvas->canvas, ctxcanvas->rotate_center_y));
    cairo_rotate(ctxcanvas->cr, (double)-ctxcanvas->rotate_angle * CD_DEG2RAD);
    cairo_translate(ctxcanvas->cr, -ctxcanvas->rotate_center_x, -_cdInvertYAxis(ctxcanvas->canvas, ctxcanvas->rotate_center_y));
  }
}

static void cdclear(cdCtxCanvas* ctxcanvas)
{
  cairo_save (ctxcanvas->cr);
  cairo_identity_matrix(ctxcanvas->cr);
  cairo_reset_clip(ctxcanvas->cr);
  cairo_rectangle(ctxcanvas->cr, 0, 0, ctxcanvas->canvas->w, ctxcanvas->canvas->h);
  cairo_clip(ctxcanvas->cr);
  cairo_set_source_rgba(ctxcanvas->cr, cdCairoGetRed(ctxcanvas->canvas->background), cdCairoGetGreen(ctxcanvas->canvas->background), cdCairoGetBlue(ctxcanvas->canvas->background), cdCairoGetAlpha(ctxcanvas->canvas->background));
  cairo_set_operator (ctxcanvas->cr, CAIRO_OPERATOR_SOURCE);
  cairo_paint (ctxcanvas->cr);  /* paints the current source everywhere within the current clip region. */
  cairo_restore (ctxcanvas->cr);
}

static void cdfline(cdCtxCanvas *ctxcanvas, double x1, double y1, double x2, double y2)
{ 
  sUpdateFill(ctxcanvas, 0);

  cairo_move_to(ctxcanvas->cr, x1, y1);
  cairo_line_to(ctxcanvas->cr, x2, y2);
  cairo_stroke(ctxcanvas->cr);
}

static void cdline(cdCtxCanvas *ctxcanvas, int x1, int y1, int x2, int y2)
{
  cdfline(ctxcanvas, (double)x1, (double)y1, (double)x2, (double)y2);
}

static void sFixAngles(cdCanvas* canvas, double *a1, double *a2, int swap)
{
  /* Cairo angles are clock-wise by default, in radians */

  /* if NOT inverted means a transformation is set, 
     so the angle will follow the transformation that includes the axis invertion,
     then it is already counter-clockwise */

  if (canvas->invert_yaxis)
  {
    /* change orientation */
    *a1 *= -1;
    *a2 *= -1;

    /* swap, so the start angle is the smaller */
    if (swap)
    {
      double t = *a1;
      *a1 = *a2;
      *a2 = t;
    }
  }

  /* convert to radians */
  *a1 *= CD_DEG2RAD;
  *a2 *= CD_DEG2RAD;
}

static void cdfarc(cdCtxCanvas *ctxcanvas, double xc, double yc, double w, double h, double a1, double a2)
{
  sUpdateFill(ctxcanvas, 0);

  sFixAngles(ctxcanvas->canvas, &a1, &a2, 1);

  if (w == h)
  {
    cairo_arc(ctxcanvas->cr, xc, yc, 0.5*w, a1, a2);
    cairo_stroke(ctxcanvas->cr);
  }
  else  /* Ellipse: change the scale to create from the circle */
  {
    cairo_save(ctxcanvas->cr);  /* save to use the local transform */

    cairo_translate(ctxcanvas->cr, xc, yc);
    cairo_scale(ctxcanvas->cr, w/h, 1.0);
    cairo_translate(ctxcanvas->cr, -xc, -yc);

    cairo_arc(ctxcanvas->cr, xc, yc, 0.5*h, a1, a2);
    cairo_stroke(ctxcanvas->cr);

    cairo_restore(ctxcanvas->cr);  /* restore from local */
  }
}

static void cdarc(cdCtxCanvas *ctxcanvas, int xc, int yc, int w, int h, double a1, double a2)
{
  cdfarc(ctxcanvas, (double)xc, (double)yc, (double)w, (double)h, a1, a2);
}

static void cdfsector(cdCtxCanvas *ctxcanvas, double xc, double yc, double w, double h, double a1, double a2)
{
  sUpdateFill(ctxcanvas, 1);

  sFixAngles(ctxcanvas->canvas, &a1, &a2, 1);

  if (w == h)
  {
    cairo_move_to(ctxcanvas->cr, xc, yc);
    cairo_arc(ctxcanvas->cr, xc, yc, 0.5*h, a1, a2);
    cairo_fill(ctxcanvas->cr);
  }
  else  /* Ellipse: change the scale to create from the circle */
  {
    cairo_save(ctxcanvas->cr);  /* save to use the local transform */

    cairo_translate(ctxcanvas->cr, xc, yc);
    cairo_scale(ctxcanvas->cr, w/h, 1.0);
    cairo_translate(ctxcanvas->cr, -xc, -yc);

    cairo_move_to(ctxcanvas->cr, xc, yc);
    cairo_arc(ctxcanvas->cr, xc, yc, 0.5*h, a1, a2);

    cairo_fill(ctxcanvas->cr);

    cairo_restore(ctxcanvas->cr);  /* restore from local */
  }
}

static void cdsector(cdCtxCanvas *ctxcanvas, int xc, int yc, int w, int h, double a1, double a2)
{
  cdfsector(ctxcanvas, (double)xc, (double)yc, (double)w, (double)h, a1, a2);
}

static void cdfchord(cdCtxCanvas *ctxcanvas, double xc, double yc, double w, double h, double a1, double a2)
{
  sUpdateFill(ctxcanvas, 1);

  sFixAngles(ctxcanvas->canvas, &a1, &a2, 1);

  if (w == h)
  {
    cairo_arc(ctxcanvas->cr, xc, yc, 0.5*w, a1, a2);
    cairo_fill(ctxcanvas->cr);
  }
  else  /* Ellipse: change the scale to create from the circle */
  {
    cairo_save(ctxcanvas->cr);  /* save to use the local transform */

    /* local transform */
    cairo_translate(ctxcanvas->cr, xc, yc);
    cairo_scale(ctxcanvas->cr, w/h, 1.0);
    cairo_translate(ctxcanvas->cr, -xc, -yc);

    cairo_arc(ctxcanvas->cr, xc, yc, 0.5*h, a1, a2);
    cairo_fill(ctxcanvas->cr);

    cairo_restore(ctxcanvas->cr);  /* restore from local */
  }
}

static void cdchord(cdCtxCanvas *ctxcanvas, int xc, int yc, int w, int h, double a1, double a2)
{
  cdfchord(ctxcanvas, (double)xc, (double)yc, (double)w, (double)h, a1, a2);
}

static void cdfrect(cdCtxCanvas *ctxcanvas, double xmin, double xmax, double ymin, double ymax)
{
  sUpdateFill(ctxcanvas, 0);
  cairo_rectangle(ctxcanvas->cr, xmin, ymin, xmax-xmin+1, ymax-ymin+1);
  cairo_stroke(ctxcanvas->cr);
}

static void cdrect(cdCtxCanvas *ctxcanvas, int xmin, int xmax, int ymin, int ymax)
{
  cdfrect(ctxcanvas, (double)xmin, (double)xmax, (double)ymin, (double)ymax);
}

static void cdfbox(cdCtxCanvas *ctxcanvas, double xmin, double xmax, double ymin, double ymax)
{
  sUpdateFill(ctxcanvas, 1);
  cairo_rectangle(ctxcanvas->cr, xmin, ymin, xmax-xmin+1, ymax-ymin+1);
  cairo_fill(ctxcanvas->cr);
}

static void cdbox(cdCtxCanvas *ctxcanvas, int xmin, int xmax, int ymin, int ymax)
{
  cdfbox(ctxcanvas, (double)xmin, (double)xmax, (double)ymin, (double)ymax);
}

static void sGetTransformTextHeight(cdCanvas* canvas, int x, int y, int w, int h, int *hbox)
{
  int xmin, xmax, ymin, ymax;
  int baseline, height, ascent;

  /* distance from bottom to baseline */
  cdgetfontdim(canvas->ctxcanvas, NULL, &height, &ascent, NULL);
  baseline = height - ascent; 

  /* move to bottom-left */
  cdTextTranslatePoint(canvas, x, y, w, h, baseline, &xmin, &ymin);

  xmax = xmin + w-1;
  ymax = ymin + h-1;

  if (canvas->text_orientation)
  {
    double cos_theta = cos(canvas->text_orientation*CD_DEG2RAD);
    double sin_theta = sin(canvas->text_orientation*CD_DEG2RAD);
    int rectY[4];

    cdRotatePointY(canvas, xmin, ymin, x, y, &rectY[0], sin_theta, cos_theta);
    cdRotatePointY(canvas, xmax, ymin, x, y, &rectY[1], sin_theta, cos_theta);
    cdRotatePointY(canvas, xmax, ymax, x, y, &rectY[2], sin_theta, cos_theta);
    cdRotatePointY(canvas, xmin, ymax, x, y, &rectY[3], sin_theta, cos_theta);

    ymin = ymax = rectY[0];
    if (rectY[1] < ymin) ymin = rectY[1];
    if (rectY[2] < ymin) ymin = rectY[2];
    if (rectY[3] < ymin) ymin = rectY[3];
    if (rectY[1] > ymax) ymax = rectY[1];
    if (rectY[2] > ymax) ymax = rectY[2];
    if (rectY[3] > ymax) ymax = rectY[3];
  }

  *hbox = ymax-ymin+1;
}

static void sSetTextTransform(cdCtxCanvas* ctxcanvas, double *x, double *y, int w, int h)
{
  int hbox;
  cairo_matrix_t mtx;

  sGetTransformTextHeight(ctxcanvas->canvas, (int)*x, (int)*y, w, h, &hbox);

  /* move to (x,y) and remove a vertical offset since text reference point is top-left */
  mtx.xx = 1; mtx.yx = 0;
  mtx.xy = 0; mtx.yy = 1;
  mtx.x0 = *x; mtx.y0 = *y - (hbox-1);
  cairo_transform(ctxcanvas->cr, &mtx);

  /* invert the text vertical orientation, relative to itself */
  mtx.xx = 1; mtx.yx = 0;
  mtx.xy = 0; mtx.yy = -1;
  mtx.x0 = 0; mtx.y0 = hbox-1;
  cairo_transform(ctxcanvas->cr, &mtx);

  *x = 0;
  *y = 0;
}

static void cdftext(cdCtxCanvas *ctxcanvas, double x, double y, const char *s, int len)
{
  PangoFontMetrics* metrics;
  int w, h, desc, dir = -1, reset_transform = 0;

  pango_layout_set_text(ctxcanvas->fontlayout, sStrConvertToUTF8(ctxcanvas, s, len), -1);
  
	pango_layout_get_pixel_size(ctxcanvas->fontlayout, &w, &h);
  metrics = pango_context_get_metrics(ctxcanvas->fontcontext, ctxcanvas->fontdesc, pango_context_get_language(ctxcanvas->fontcontext));
  desc = (((pango_font_metrics_get_descent(metrics)) + PANGO_SCALE/2) / PANGO_SCALE);

  if (ctxcanvas->canvas->text_orientation || 
      ctxcanvas->canvas->use_matrix ||
      ctxcanvas->rotate_angle)
    reset_transform = 1;

  if (reset_transform)
  {
    cairo_save (ctxcanvas->cr);
    cairo_identity_matrix(ctxcanvas->cr);

    if (ctxcanvas->job)
      cairo_scale(ctxcanvas->cr, 0.25, 0.25);  /* ??? */
  }

  if (ctxcanvas->canvas->text_orientation)
  {
    cairo_translate(ctxcanvas->cr, x, y);
    cairo_rotate(ctxcanvas->cr, -ctxcanvas->canvas->text_orientation*CD_DEG2RAD);
    cairo_translate(ctxcanvas->cr, -x, -y);
  }

  /* move to top-left corner of the text */
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
      y = y - (dir*h - desc);
      break;
    case CD_SOUTH_EAST:
    case CD_SOUTH_WEST:
    case CD_SOUTH:
      y = y - (dir*h);
      break;
    case CD_NORTH_EAST:
    case CD_NORTH:
    case CD_NORTH_WEST:
      y = y;
      break;
    case CD_CENTER:
    case CD_EAST:
    case CD_WEST:
      y = y - (dir*(h/2));
      break;
  }

  if (ctxcanvas->canvas->use_matrix)
  {
    double* matrix = ctxcanvas->canvas->matrix;
    sSetTransform(ctxcanvas, matrix);
    sSetTextTransform(ctxcanvas, &x, &y, w, h);
  }
  else 
    sSetTransform(ctxcanvas, NULL);

  /* Inform Pango to re-layout the text with the new transformation */
  pango_cairo_update_layout(ctxcanvas->cr, ctxcanvas->fontlayout);

  sUpdateFill(ctxcanvas, 0);

  cairo_move_to(ctxcanvas->cr, x, y);
  pango_cairo_show_layout(ctxcanvas->cr, ctxcanvas->fontlayout);

  if (reset_transform)
    cairo_restore(ctxcanvas->cr);

  pango_font_metrics_unref(metrics); 
}

static void cdtext(cdCtxCanvas *ctxcanvas, int x, int y, const char *s, int len)
{
  cdftext(ctxcanvas, (double)x, (double)y, s, len);
}

static void cdgettextsize(cdCtxCanvas *ctxcanvas, const char *s, int len, int *width, int *height)
{
  if (!ctxcanvas->fontlayout)
    return;

  pango_cairo_update_layout(ctxcanvas->cr, ctxcanvas->fontlayout);
  pango_layout_set_text(ctxcanvas->fontlayout, sStrConvertToUTF8(ctxcanvas, s, len), len);
  pango_layout_get_pixel_size(ctxcanvas->fontlayout, width, height);
}

static void cdpoly(cdCtxCanvas *ctxcanvas, int mode, cdPoint* poly, int n)
{
  int i;

  if (mode == CD_CLIP)
    return;

  if (mode == CD_PATH)
  {
    int p;

    /* if there is any current path, remove it */
    cairo_new_path(ctxcanvas->cr);

    i = 0;
    for (p=0; p<ctxcanvas->canvas->path_n; p++)
    {
      switch(ctxcanvas->canvas->path[p])
      {
      case CD_PATH_NEW:
        cairo_new_path(ctxcanvas->cr);
        break;
      case CD_PATH_MOVETO:
        if (i+1 > n) return;
        cairo_move_to(ctxcanvas->cr, poly[i].x, poly[i].y);
        i++;
        break;
      case CD_PATH_LINETO:
        if (i+1 > n) return;
        cairo_line_to(ctxcanvas->cr, poly[i].x, poly[i].y);
        i++;
        break;
      case CD_PATH_ARC:
        {
          double xc, yc, w, h, a1, a2;

          if (i+3 > n) return;

          if (!cdCanvasGetArcPathF(ctxcanvas->canvas, poly+i, &xc, &yc, &w, &h, &a1, &a2))
            return;

          sFixAngles(ctxcanvas->canvas, &a1, &a2, 0);  /* do not swap because we handle negative arcs here */

          if (w == h)
          {
            if ((a2-a1)<0)
              cairo_arc_negative(ctxcanvas->cr, xc, yc, 0.5*w, a1, a2);
            else
              cairo_arc(ctxcanvas->cr, xc, yc, 0.5*w, a1, a2);
          }
          else  /* Ellipse: change the scale to create from the circle */
          {
            cairo_save(ctxcanvas->cr);  /* save to use the local transform */

            cairo_translate(ctxcanvas->cr, xc, yc);
            cairo_scale(ctxcanvas->cr, w/h, 1.0);
            cairo_translate(ctxcanvas->cr, -xc, -yc);

            if ((a2-a1)<0)
              cairo_arc_negative(ctxcanvas->cr, xc, yc, 0.5*h, a1, a2);
            else
              cairo_arc(ctxcanvas->cr, xc, yc, 0.5*h, a1, a2);

            cairo_restore(ctxcanvas->cr);  /* restore from local */
          }

          i += 3;
        }
        break;
      case CD_PATH_CURVETO:
        if (i+3 > n) return;
        cairo_curve_to(ctxcanvas->cr, poly[i].x, poly[i].y, poly[i+1].x, poly[i+1].y, poly[i+2].x, poly[i+2].y);
        i += 3;
        break;
      case CD_PATH_CLOSE:
        cairo_close_path(ctxcanvas->cr);
        break;
      case CD_PATH_FILL:
        sUpdateFill(ctxcanvas, 1);
        cairo_set_fill_rule(ctxcanvas->cr, ctxcanvas->canvas->fill_mode==CD_EVENODD? CAIRO_FILL_RULE_EVEN_ODD: CAIRO_FILL_RULE_WINDING);
        cairo_fill(ctxcanvas->cr);
        break;
      case CD_PATH_STROKE:
        sUpdateFill(ctxcanvas, 0);
        cairo_stroke(ctxcanvas->cr);
        break;
      case CD_PATH_FILLSTROKE:
        sUpdateFill(ctxcanvas, 1);
        cairo_set_fill_rule(ctxcanvas->cr, ctxcanvas->canvas->fill_mode==CD_EVENODD? CAIRO_FILL_RULE_EVEN_ODD: CAIRO_FILL_RULE_WINDING);
        cairo_fill_preserve(ctxcanvas->cr);
        sUpdateFill(ctxcanvas, 0);
        cairo_stroke(ctxcanvas->cr);
        break;
      case CD_PATH_CLIP:
        cairo_set_fill_rule(ctxcanvas->cr, ctxcanvas->canvas->fill_mode==CD_EVENODD? CAIRO_FILL_RULE_EVEN_ODD: CAIRO_FILL_RULE_WINDING);
        cairo_clip(ctxcanvas->cr);
        break;
      }
    }
    return;
  }

  if (mode == CD_FILL)
  {
    sUpdateFill(ctxcanvas, 1);

    if (ctxcanvas->holes || ctxcanvas->canvas->fill_mode==CD_EVENODD)
      cairo_set_fill_rule(ctxcanvas->cr, CAIRO_FILL_RULE_EVEN_ODD);
    else
      cairo_set_fill_rule(ctxcanvas->cr, CAIRO_FILL_RULE_WINDING);
  }
  else
    sUpdateFill(ctxcanvas, 0);

  cairo_move_to(ctxcanvas->cr, poly[0].x, poly[0].y);

  if (mode == CD_BEZIER)
  {
    for (i=1; i<n; i+=3)
      cairo_curve_to(ctxcanvas->cr, poly[i].x, poly[i].y, poly[i+1].x, poly[i+1].y, poly[i+2].x, poly[i+2].y);
  }
  else
  {
    int hole_index = 0;

    for (i=1; i<n; i++)
    {
      if (ctxcanvas->holes && i == ctxcanvas->poly_holes[hole_index])
      {
        cairo_move_to(ctxcanvas->cr, poly[i].x, poly[i].y);
        hole_index++;
      }
      else
        cairo_line_to(ctxcanvas->cr, poly[i].x, poly[i].y);
    }
  }

  switch (mode)
  {
  case CD_CLOSED_LINES :
    cairo_close_path(ctxcanvas->cr);
    cairo_stroke(ctxcanvas->cr);
    break;
  case CD_OPEN_LINES :
    cairo_stroke(ctxcanvas->cr);
    break;
  case CD_BEZIER :
    cairo_stroke(ctxcanvas->cr);
    break;
  case CD_FILL :
    cairo_fill(ctxcanvas->cr);
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
    int p;

    /* if there is any current path, remove it */
    cairo_new_path(ctxcanvas->cr);

    i = 0;
    for (p=0; p<ctxcanvas->canvas->path_n; p++)
    {
      switch(ctxcanvas->canvas->path[p])
      {
      case CD_PATH_NEW:
        cairo_new_path(ctxcanvas->cr);
        break;
      case CD_PATH_MOVETO:
        if (i+1 > n) return;
        cairo_move_to(ctxcanvas->cr, poly[i].x, poly[i].y);
        i++;
        break;
      case CD_PATH_LINETO:
        if (i+1 > n) return;
        cairo_line_to(ctxcanvas->cr, poly[i].x, poly[i].y);
        i++;
        break;
      case CD_PATH_ARC:
        {
          double xc, yc, w, h, a1, a2;

          if (i+3 > n) return;

          if (!cdfCanvasGetArcPath(ctxcanvas->canvas, poly+i, &xc, &yc, &w, &h, &a1, &a2))
            return;

          sFixAngles(ctxcanvas->canvas, &a1, &a2, 0);  /* do not swap because we handle negative arcs here */

          if (w == h)
          {
            if ((a2-a1)<0)
              cairo_arc_negative(ctxcanvas->cr, xc, yc, 0.5*w, a1, a2);
            else
              cairo_arc(ctxcanvas->cr, xc, yc, 0.5*w, a1, a2);
          }
          else  /* Ellipse: change the scale to create from the circle */
          {
            cairo_save(ctxcanvas->cr);  /* save to use the local transform */

            cairo_translate(ctxcanvas->cr, xc, yc);
            cairo_scale(ctxcanvas->cr, w/h, 1.0);
            cairo_translate(ctxcanvas->cr, -xc, -yc);

            if ((a2-a1)<0)
              cairo_arc_negative(ctxcanvas->cr, xc, yc, 0.5*h, a1, a2);
            else
              cairo_arc(ctxcanvas->cr, xc, yc, 0.5*h, a1, a2);

            cairo_restore(ctxcanvas->cr);  /* restore from local */
          }

          i += 3;
        }
        break;
      case CD_PATH_CURVETO:
        if (i+3 > n) return;
        cairo_curve_to(ctxcanvas->cr, poly[i].x, poly[i].y, poly[i+1].x, poly[i+1].y, poly[i+2].x, poly[i+2].y);
        i += 3;
        break;
      case CD_PATH_CLOSE:
        cairo_close_path(ctxcanvas->cr);
        break;
      case CD_PATH_FILL:
        sUpdateFill(ctxcanvas, 1);
        cairo_set_fill_rule(ctxcanvas->cr, ctxcanvas->canvas->fill_mode==CD_EVENODD? CAIRO_FILL_RULE_EVEN_ODD: CAIRO_FILL_RULE_WINDING);
        cairo_fill(ctxcanvas->cr);
        break;
      case CD_PATH_STROKE:
        sUpdateFill(ctxcanvas, 0);
        cairo_stroke(ctxcanvas->cr);
        break;
      case CD_PATH_FILLSTROKE:
        sUpdateFill(ctxcanvas, 1);
        cairo_set_fill_rule(ctxcanvas->cr, ctxcanvas->canvas->fill_mode==CD_EVENODD? CAIRO_FILL_RULE_EVEN_ODD: CAIRO_FILL_RULE_WINDING);
        cairo_fill_preserve(ctxcanvas->cr);
        sUpdateFill(ctxcanvas, 0);
        cairo_stroke(ctxcanvas->cr);
        break;
      case CD_PATH_CLIP:
        cairo_set_fill_rule(ctxcanvas->cr, ctxcanvas->canvas->fill_mode==CD_EVENODD? CAIRO_FILL_RULE_EVEN_ODD: CAIRO_FILL_RULE_WINDING);
        cairo_clip(ctxcanvas->cr);
        break;
      }
    }
    return;
  }

  if (mode == CD_FILL)
  {
    sUpdateFill(ctxcanvas, 1);

    if (ctxcanvas->holes || ctxcanvas->canvas->fill_mode==CD_EVENODD)
      cairo_set_fill_rule(ctxcanvas->cr, CAIRO_FILL_RULE_EVEN_ODD);
    else
      cairo_set_fill_rule(ctxcanvas->cr, CAIRO_FILL_RULE_WINDING);
  }
  else
    sUpdateFill(ctxcanvas, 0);

  cairo_move_to(ctxcanvas->cr, poly[0].x, poly[0].y);

  if (mode == CD_BEZIER)
  {
    for (i=1; i<n; i+=3)
      cairo_curve_to(ctxcanvas->cr, poly[i].x, poly[i].y, poly[i+1].x, poly[i+1].y, poly[i+2].x, poly[i+2].y);
  }
  else
  {
    int hole_index = 0;

    for (i=1; i<n; i++)
    {
      if (ctxcanvas->holes && i == ctxcanvas->poly_holes[hole_index])
      {
        cairo_move_to(ctxcanvas->cr, poly[i].x, poly[i].y);
        hole_index++;
      }
      else
        cairo_line_to(ctxcanvas->cr, poly[i].x, poly[i].y);
    }
  }

  switch (mode)
  {
  case CD_CLOSED_LINES :
    cairo_close_path(ctxcanvas->cr);
    cairo_stroke(ctxcanvas->cr);
    break;
  case CD_OPEN_LINES :
    cairo_stroke(ctxcanvas->cr);
    break;
  case CD_BEZIER :
    cairo_stroke(ctxcanvas->cr);
    break;
  case CD_FILL :
    cairo_fill(ctxcanvas->cr);
    break;
  }
}

/******************************************************/

static void cdgetimagergb(cdCtxCanvas *ctxcanvas, unsigned char *r, unsigned char *g, unsigned char *b, int x, int y, int w, int h)
{
  int i, j, pos, offset, stride;
  unsigned int* data;
  cairo_surface_t* image_surface;
  cairo_t* cr;

  cairo_save (ctxcanvas->cr);

  /* reset to the identity. */
  cairo_identity_matrix(ctxcanvas->cr);

  if (ctxcanvas->canvas->invert_yaxis==0) /* if 0, invert because the transform was reset here */
    y = _cdInvertYAxis(ctxcanvas->canvas, y);

  /* y is the bottom-left of the image in CD, must be at upper-left */
  y -= h-1;

  /* CAIRO_FORMAT_RGB24	each pixel is a 32-bit quantity, with the upper 8 bits unused. 
     Red, Green, and Blue are stored in the remaining 24 bits in that order. */
  image_surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, w, h);
  if (cairo_surface_status(image_surface) != CAIRO_STATUS_SUCCESS)
  {
    cairo_surface_destroy(image_surface);
    return;
  }

  cr = cairo_create(image_surface);

  /* creates a pattern from the canvas and sets it as source in the image. */
  cairo_set_source_surface(cr, cairo_get_target(ctxcanvas->cr), -x, -y);

  cairo_pattern_set_extend (cairo_get_source(cr), CAIRO_EXTEND_NONE); 
  cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
  cairo_paint(cr);  /* paints the current source everywhere within the current clip region. */

  data = (unsigned int*)cairo_image_surface_get_data(image_surface);
  stride = cairo_image_surface_get_stride(image_surface);
  offset = stride/4 - w;

  for (i=0; i<h; i++)
  {
    for (j=0; j<w; j++)
    {
      pos = i*w+j;
      r[pos] = cdRed(*data);
      g[pos] = cdGreen(*data);
      b[pos] = cdBlue(*data);
      data++;
    }

    if (offset)
      data += offset;
  }

  cairo_surface_destroy(image_surface);
  cairo_destroy(cr);

  cairo_restore(ctxcanvas->cr);
}

static void sFixImageY(cdCanvas* canvas, int *topdown, int *y, int h)
{
  if (canvas->invert_yaxis)
    *topdown = 0;
  else
    *topdown = 1;

  if (!(*topdown))
    *y -= (h - 1);  /* move Y to top-left corner, since it was at the bottom of the image */
}

static void cdputimagerectrgb(cdCtxCanvas *ctxcanvas, int iw, int ih, const unsigned char *r, const unsigned char *g, const unsigned char *b, int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax)
{
  int i, j, rw, rh, pos, offset, topdown, stride;
  unsigned int* data;
  cairo_surface_t* image_surface;

  if (xmin<0 || ymin<0 || xmax-xmin+1>iw || ymax-ymin+1>ih) return;

  rw = xmax-xmin+1;
  rh = ymax-ymin+1;

  /* CAIRO_FORMAT_RGB24	each pixel is a 32-bit quantity, with the upper 8 bits unused. 
     Red, Green, and Blue are stored in the remaining 24 bits in that order. */
  image_surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, rw, rh);
  if (cairo_surface_status(image_surface) != CAIRO_STATUS_SUCCESS)
  {
    cairo_surface_destroy(image_surface);
    return;
  }

  data = (unsigned int*)cairo_image_surface_get_data(image_surface);
  stride = cairo_image_surface_get_stride(image_surface);
  offset = stride/4 - rw;

  sFixImageY(ctxcanvas->canvas, &topdown, &y, h);

  for (i=ymin; i<=ymax; i++)
  {
    for (j=xmin; j<=xmax; j++)
    {
      if (topdown)
        pos = i*iw+j;
      else
        pos = (ymax+ymin - i)*iw+j;
      *data++ = sEncodeRGBA(r[pos], g[pos], b[pos], 255);
    }

    if (offset)
      data += offset;
  }

  cairo_save (ctxcanvas->cr);

  cairo_rectangle(ctxcanvas->cr, x, y, w, h);
  cairo_clip(ctxcanvas->cr);

  if (w != rw || h != rh)
  {
    /* Scale *before* setting the source surface (1) */
    cairo_translate(ctxcanvas->cr, x, y);
    cairo_scale (ctxcanvas->cr, (double)w / rw, (double)h / rh);
    cairo_translate(ctxcanvas->cr, -x, -y);
  }

  cairo_set_source_surface(ctxcanvas->cr, image_surface, x, y);
  cairo_paint(ctxcanvas->cr);

  cairo_surface_destroy(image_surface);
  cairo_restore (ctxcanvas->cr);
}

static void cdputimagerectrgba(cdCtxCanvas *ctxcanvas, int iw, int ih, const unsigned char *r, const unsigned char *g, const unsigned char *b, const unsigned char *a, int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax)
{
  int i, j, rw, rh, pos, offset, topdown, stride;
  unsigned int* data;
  cairo_surface_t* image_surface;

  if (xmin<0 || ymin<0 || xmax-xmin+1>iw || ymax-ymin+1>ih) return;

  rw = xmax-xmin+1;
  rh = ymax-ymin+1;

  /* CAIRO_FORMAT_ARGB32 each pixel is a 32-bit quantity, with alpha in the upper 8 bits, then red, then green, then blue. 
     The 32-bit quantities are stored native-endian. 
     Pre-multiplied alpha is used. (That is, 50% transparent red is 0x80800000, not 0x80ff0000.) */
  image_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, rw, rh);
  if (cairo_surface_status(image_surface) != CAIRO_STATUS_SUCCESS)
  {
    cairo_surface_destroy(image_surface);
    return;
  }

  data = (unsigned int*)cairo_image_surface_get_data(image_surface);
  stride = cairo_image_surface_get_stride(image_surface);
  offset = stride/4 - rw;

  sFixImageY(ctxcanvas->canvas, &topdown, &y, h);

  for (i=ymin; i<=ymax; i++)
  {
    for (j=xmin; j<=xmax; j++)
    {
      if (topdown)
        pos = i*iw+j;
      else
        pos = (ymax+ymin - i)*iw+j;
      *data++ = sEncodeRGBA(r[pos], g[pos], b[pos], a[pos]);
    }

    if (offset)
      data += offset;
  }

  cairo_save (ctxcanvas->cr);

  cairo_rectangle(ctxcanvas->cr, x, y, w, h);
  cairo_clip(ctxcanvas->cr);

  if (w != rw || h != rh)
  {
    /* Scale *before* setting the source surface (1) */
    cairo_translate(ctxcanvas->cr, x, y);
    cairo_scale (ctxcanvas->cr, (double)w / rw, (double)h / rh);
    cairo_translate(ctxcanvas->cr, -x, -y);
  }

  cairo_set_source_surface(ctxcanvas->cr, image_surface, x, y);
  cairo_paint(ctxcanvas->cr);

  cairo_surface_destroy(image_surface);
  cairo_restore (ctxcanvas->cr);
}

static int sCalcPalSize(int size, const unsigned char *index)
{
  int i, pal_size = 0;

  for (i = 0; i < size; i++)
  {
    if (index[i] > pal_size)
      pal_size = index[i];
  }

  pal_size++;
  return pal_size;
}

static void cdputimagerectmap(cdCtxCanvas *ctxcanvas, int iw, int ih, const unsigned char *index, const long *colors, int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax)
{
  int i, j, rw, rh, pos, offset, pal_size, topdown, stride;
  unsigned int* data, cairo_colors[256];
  long c;
  cairo_surface_t* image_surface;

  if (xmin<0 || ymin<0 || xmax-xmin+1>iw || ymax-ymin+1>ih) return;

  rw = xmax-xmin+1;
  rh = ymax-ymin+1;

  /* CAIRO_FORMAT_RGB24	each pixel is a 32-bit quantity, with the upper 8 bits unused. 
     Red, Green, and Blue are stored in the remaining 24 bits in that order. */
  image_surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, rw, rh);
  if (cairo_surface_status(image_surface) != CAIRO_STATUS_SUCCESS)
  {
    cairo_surface_destroy(image_surface);
    return;
  }

  data = (unsigned int*)cairo_image_surface_get_data(image_surface);
  stride = cairo_image_surface_get_stride(image_surface);
  offset = stride/4 - rw;

  pal_size = sCalcPalSize(iw*ih, index);
  for (i=0; i<pal_size; i++)
  {
    c = colors[i];
    cairo_colors[i] = sEncodeRGBA(cdRed(c), cdGreen(c), cdBlue(c), 255);
  }

  sFixImageY(ctxcanvas->canvas, &topdown, &y, h);

  for (i=ymin; i<=ymax; i++)
  {
    for (j=xmin; j<=xmax; j++)
    {
      if (topdown)
        pos = i*iw+j;
      else
        pos = (ymax+ymin - i)*iw+j;
      *data++ = cairo_colors[index[pos]];
    }

    if (offset)
      data += offset;
  }

  cairo_save (ctxcanvas->cr);

  cairo_rectangle(ctxcanvas->cr, x, y, w, h);
  cairo_clip(ctxcanvas->cr);

  if (w != rw || h != rh)
  {
    /* Scale *before* setting the source surface (1) */
    cairo_translate(ctxcanvas->cr, x, y);
    cairo_scale (ctxcanvas->cr, (double)w / rw, (double)h / rh);
    cairo_translate(ctxcanvas->cr, -x, -y);
  }

  cairo_set_source_surface(ctxcanvas->cr, image_surface, x, y);
  cairo_paint(ctxcanvas->cr);

  cairo_surface_destroy(image_surface);
  cairo_restore (ctxcanvas->cr);
}

static void cdpixel(cdCtxCanvas *ctxcanvas, int x, int y, long color)
{
  cairo_pattern_t* old_source = cairo_get_source(ctxcanvas->cr);
  cairo_set_source_rgba(ctxcanvas->cr, cdCairoGetRed(color), cdCairoGetGreen(color), cdCairoGetBlue(color), cdCairoGetAlpha(color));

  cairo_move_to(ctxcanvas->cr, (double)x, (double)y);
  cairo_arc(ctxcanvas->cr, (double)x, (double)y, 0.5, 0.0, 2 * M_PI);

  cairo_fill(ctxcanvas->cr);
  cairo_set_source(ctxcanvas->cr, old_source);
}

static cdCtxImage *cdcreateimage (cdCtxCanvas *ctxcanvas, int w, int h)
{
  cdCtxImage *ctximage = (cdCtxImage *)malloc(sizeof(cdCtxImage));
  cairo_surface_t* img_surface;

  ctximage->w = w;
  ctximage->h = h;
  ctximage->bpp = ctxcanvas->canvas->bpp;
  ctximage->xres = ctxcanvas->canvas->xres;
  ctximage->yres = ctxcanvas->canvas->yres;
  ctximage->w_mm = ctximage->w / ctximage->xres;
  ctximage->h_mm = ctximage->h / ctximage->yres;

  img_surface = cairo_surface_create_similar(cairo_get_target(ctxcanvas->cr), CAIRO_CONTENT_COLOR_ALPHA, w, h);
  ctximage->cr = cairo_create(img_surface);

  if (!ctximage->cr)
  {
    free(ctximage);
    return (void *)0;
  }

  cairo_rectangle(ctximage->cr, 0, 0, ctximage->w, ctximage->h);
  cairo_set_source_rgba(ctximage->cr, 1.0, 0.0, 0.0, 1.0); /* white opaque */
  cairo_fill(ctximage->cr);

  cairo_surface_destroy(img_surface);

  return (void*)ctximage;
}

static void cdkillimage (cdCtxImage *ctximage)
{
  cairo_destroy(ctximage->cr);
  free(ctximage);
}

static void cdgetimage (cdCtxCanvas *ctxcanvas, cdCtxImage *ctximage, int x, int y)
{
  cairo_save (ctximage->cr);

  /* reset to the identity. */
  cairo_identity_matrix(ctximage->cr);

  cairo_reset_clip(ctximage->cr);

  if (ctxcanvas->canvas->invert_yaxis==0)  /* if 0, invert because the transform was reset here */
    y = _cdInvertYAxis(ctxcanvas->canvas, y);

  /* y is the bottom-left of the image in CD, must be at upper-left */
  y -= ctximage->h-1;

  /* creates a pattern from the canvas and sets it as source in the image. */
  cairo_set_source_surface(ctximage->cr, cairo_get_target(ctxcanvas->cr), -x, -y);

  cairo_pattern_set_extend (cairo_get_source(ctximage->cr), CAIRO_EXTEND_NONE); 
  cairo_set_operator (ctximage->cr, CAIRO_OPERATOR_SOURCE);
  cairo_paint(ctximage->cr);  /* paints the current source everywhere within the current clip region. */

  /* must restore matrix, clipping and source */
  cairo_restore (ctximage->cr);
}

static void cdputimagerect (cdCtxCanvas *ctxcanvas, cdCtxImage *ctximage, int x, int y, int xmin, int xmax, int ymin, int ymax)
{
  cairo_save (ctxcanvas->cr);

  /* y is the bottom-left of the image region in CD */
  y -= (ymax-ymin+1)-1;

  cairo_rectangle(ctxcanvas->cr, x, y, xmax-xmin+1, ymax-ymin+1);
  cairo_clip(ctxcanvas->cr);

  /* creates a pattern from the image and sets it as source in the canvas. */
  cairo_set_source_surface(ctxcanvas->cr, cairo_get_target(ctximage->cr), x, y);

  cairo_pattern_set_extend (cairo_get_source(ctxcanvas->cr), CAIRO_EXTEND_NONE); 
  cairo_set_operator (ctxcanvas->cr, CAIRO_OPERATOR_SOURCE);
  cairo_paint(ctxcanvas->cr);  /* paints the current source everywhere within the current clip region. */

  /* must restore clipping and source */
  cairo_restore (ctxcanvas->cr);
}

static void cdscrollarea (cdCtxCanvas *ctxcanvas, int xmin, int xmax, int ymin, int ymax, int dx, int dy)
{
  cairo_save (ctxcanvas->cr);

  /* reset to identity */
  cairo_identity_matrix(ctxcanvas->cr);

  if (ctxcanvas->canvas->invert_yaxis==0)  /* if 0, invert because the transform was reset here */
  {
    dy = -dy;
    ymin = _cdInvertYAxis(ctxcanvas->canvas, ymin);
    ymax = _cdInvertYAxis(ctxcanvas->canvas, ymax);
    _cdSwapInt(ymin, ymax);
  }

  cairo_rectangle(ctxcanvas->cr, xmin+dx, ymin+dy, xmax-xmin+1, ymax-ymin+1);
  cairo_clip(ctxcanvas->cr);

  /* creates a pattern from the canvas and sets it as source in the canvas. */
  cairo_set_source_surface(ctxcanvas->cr, cairo_get_target(ctxcanvas->cr), xmin, ymin);

  cairo_pattern_set_extend (cairo_get_source(ctxcanvas->cr), CAIRO_EXTEND_NONE); 
  cairo_set_operator (ctxcanvas->cr, CAIRO_OPERATOR_SOURCE);
  cairo_paint(ctxcanvas->cr);  /* paints the current source everywhere within the current clip region. */

  /* must restore matrix, clipping and source */
  cairo_restore (ctxcanvas->cr);
}

static void cdtransform(cdCtxCanvas *ctxcanvas, const double* matrix)
{
  /* reset to identity */
  cairo_identity_matrix(ctxcanvas->cr);
  
  if (ctxcanvas->job)
    cairo_scale(ctxcanvas->cr, 0.25, 0.25);  /* ??? */

  if (matrix)
    ctxcanvas->canvas->invert_yaxis = 0;
  else
    ctxcanvas->canvas->invert_yaxis = 1;

  sSetTransform(ctxcanvas, matrix);
}

/******************************************************************/

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

static void set_rotate_attrib(cdCtxCanvas* ctxcanvas, char* data)
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

static char* get_rotate_attrib(cdCtxCanvas* ctxcanvas)
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

static void set_aa_attrib(cdCtxCanvas* ctxcanvas, char* data)
{
  if (!data || data[0] == '0')
    cairo_set_antialias(ctxcanvas->cr, CAIRO_ANTIALIAS_NONE);
  else
    cairo_set_antialias(ctxcanvas->cr, CAIRO_ANTIALIAS_DEFAULT);
}

static char* get_aa_attrib(cdCtxCanvas* ctxcanvas)
{
  if (cairo_get_antialias(ctxcanvas->cr) != CAIRO_ANTIALIAS_NONE)
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

static void set_pattern_image_attrib(cdCtxCanvas *ctxcanvas, char* data)
{
  if (data)
  {
    cdCtxImage *ctximage = (cdCtxImage *)data;

    if (ctxcanvas->pattern)
      cairo_pattern_destroy(ctxcanvas->pattern);

    ctxcanvas->pattern = cairo_pattern_create_for_surface(cairo_get_target(ctximage->cr));
    cairo_pattern_reference(ctxcanvas->pattern);
    cairo_pattern_set_extend(ctxcanvas->pattern, CAIRO_EXTEND_REPEAT);

    cairo_set_source(ctxcanvas->cr, ctxcanvas->pattern);
    ctxcanvas->last_source = 1;
  }
}

static cdAttribute pattern_image_attrib =
{
  "PATTERNIMAGE",
  set_pattern_image_attrib,
  NULL
}; 

static void set_linegradient_attrib(cdCtxCanvas* ctxcanvas, char* data)
{
  if (data)
  {
    int x1, y1, x2, y2;
    double offset;
    int count = 1;

    sscanf(data, "%d %d %d %d", &x1, &y1, &x2, &y2);

    if (ctxcanvas->canvas->invert_yaxis)
    {
      y1 = _cdInvertYAxis(ctxcanvas->canvas, y1);
      y2 = _cdInvertYAxis(ctxcanvas->canvas, y2);
    }

    if (ctxcanvas->pattern)
      cairo_pattern_destroy(ctxcanvas->pattern);

    ctxcanvas->pattern = cairo_pattern_create_linear((double)x1, (double)y1, (double)x2, (double)y2);
    cairo_pattern_reference(ctxcanvas->pattern);

    for(offset = 0.1; offset < 1.0; offset += 0.1)
    {
      if ( count % 2 )
      {
        cairo_pattern_add_color_stop_rgb(ctxcanvas->pattern, offset,
          cdCairoGetRed(ctxcanvas->canvas->foreground),
          cdCairoGetGreen(ctxcanvas->canvas->foreground),
          cdCairoGetBlue(ctxcanvas->canvas->foreground));
      }
      else
      {
        cairo_pattern_add_color_stop_rgb(ctxcanvas->pattern, offset,
          cdCairoGetRed(ctxcanvas->canvas->background),
          cdCairoGetGreen(ctxcanvas->canvas->background),
          cdCairoGetBlue(ctxcanvas->canvas->background));
      }
      count++;
    }

    cairo_pattern_set_extend(ctxcanvas->pattern, CAIRO_EXTEND_REPEAT);

    cairo_set_source(ctxcanvas->cr, ctxcanvas->pattern);
    ctxcanvas->last_source = 1;
  }
}

static char* get_linegradient_attrib(cdCtxCanvas* ctxcanvas)
{
  double x1, y1, x2, y2;

#if (CAIRO_VERSION_MAJOR>1 || (CAIRO_VERSION_MAJOR==1 && CAIRO_VERSION_MINOR>=4))
  if (cairo_pattern_get_linear_points(ctxcanvas->pattern, &x1, &y1, &x2, &y2) == CAIRO_STATUS_SUCCESS)
  {
    static char data[100];
    sprintf(data, "%d %d %d %d", (int)x1, (int)y1, (int)x2, (int)y2);
    return data;
  }
  else
#endif
    return NULL;
}

static cdAttribute linegradient_attrib =
{
  "LINEGRADIENT",
  set_linegradient_attrib,
  get_linegradient_attrib
}; 

static void set_radialgradient_attrib(cdCtxCanvas* ctxcanvas, char* data)
{
  if (data)
  {
    int cx1, cy1, cx2, cy2;
    float rad1, rad2;
    double offset;
    int count = 1;

    sscanf(data, "%d %d %g %d %d %g", &cx1, &cy1, &rad1, &cx2, &cy2, &rad2);

    if (ctxcanvas->canvas->invert_yaxis)
    {
      cy1 = _cdInvertYAxis(ctxcanvas->canvas, cy1);
      cy2 = _cdInvertYAxis(ctxcanvas->canvas, cy2);
    }

    if (ctxcanvas->pattern)
      cairo_pattern_destroy(ctxcanvas->pattern);

    ctxcanvas->pattern = cairo_pattern_create_radial((double)cx1, (double)cx1, (double)rad1, (double)cx2, (double)cx2, (double)rad2);
    cairo_pattern_reference(ctxcanvas->pattern);

    for(offset = 0.1; offset < 1.0; offset += 0.1)
    {
      if ( count % 2 )
      {
        cairo_pattern_add_color_stop_rgb(ctxcanvas->pattern, offset,
          cdCairoGetRed(ctxcanvas->canvas->foreground),
          cdCairoGetGreen(ctxcanvas->canvas->foreground),
          cdCairoGetBlue(ctxcanvas->canvas->foreground));
      }
      else
      {
        cairo_pattern_add_color_stop_rgb(ctxcanvas->pattern, offset,
          cdCairoGetRed(ctxcanvas->canvas->background),
          cdCairoGetGreen(ctxcanvas->canvas->background),
          cdCairoGetBlue(ctxcanvas->canvas->background));
      }
      count++;
    }

    cairo_pattern_set_extend(ctxcanvas->pattern, CAIRO_EXTEND_REPEAT);

    cairo_set_source(ctxcanvas->cr, ctxcanvas->pattern);
    ctxcanvas->last_source = 1;
  }
}

static char* get_radialgradient_attrib(cdCtxCanvas* ctxcanvas)
{
  double cx1, cy1, rad1, cx2, cy2, rad2;

#if (CAIRO_VERSION_MAJOR>1 || (CAIRO_VERSION_MAJOR==1 && CAIRO_VERSION_MINOR>=4))
  if (cairo_pattern_get_radial_circles(ctxcanvas->pattern, &cx1, &cy1, &rad1, &cx2, &cy2, &rad2) == CAIRO_STATUS_SUCCESS)
  {
    static char data[100];
    sprintf(data, "%d %d %g %d %d %g", (int)cx1, (int)cy1, (float)rad1, (int)cx2, (int)cy2, (float)rad2);
    return data;
  }
  else
#endif
    return NULL;
}

static cdAttribute radialgradient_attrib =
{
  "RADIALGRADIENT",
  set_radialgradient_attrib,
  get_radialgradient_attrib
}; 

static char* get_version_attrib(cdCtxCanvas* ctxcanvas)
{
  (void)ctxcanvas;
  return (char*)cairo_version_string();
}

static cdAttribute version_attrib =
{
  "CAIROVERSION",
  NULL,
  get_version_attrib
};

static void set_interp_attrib(cdCtxCanvas* ctxcanvas, char* data)
{
  if (data && cdStrEqualNoCase(data, "BEST"))
    cairo_pattern_set_filter(cairo_get_source(ctxcanvas->cr), CAIRO_FILTER_BEST);
  else if (data && cdStrEqualNoCase(data, "NEAREST"))
    cairo_pattern_set_filter(cairo_get_source(ctxcanvas->cr), CAIRO_FILTER_NEAREST);
  else if (data && cdStrEqualNoCase(data, "FAST"))
    cairo_pattern_set_filter(cairo_get_source(ctxcanvas->cr), CAIRO_FILTER_FAST);
  else if (data && cdStrEqualNoCase(data, "BILINEAR"))
    cairo_pattern_set_filter(cairo_get_source(ctxcanvas->cr), CAIRO_FILTER_BILINEAR);
  else
    cairo_pattern_set_filter(cairo_get_source(ctxcanvas->cr), CAIRO_FILTER_GOOD);
}

static char* get_interp_attrib(cdCtxCanvas* ctxcanvas)
{
  if(cairo_pattern_get_filter(cairo_get_source(ctxcanvas->cr)) == CAIRO_FILTER_BEST)
    return "BEST";
  else if(cairo_pattern_get_filter(cairo_get_source(ctxcanvas->cr)) == CAIRO_FILTER_NEAREST)
    return "NEAREST";
  else if(cairo_pattern_get_filter(cairo_get_source(ctxcanvas->cr)) == CAIRO_FILTER_FAST)
    return "FAST";
  else if(cairo_pattern_get_filter(cairo_get_source(ctxcanvas->cr)) == CAIRO_FILTER_BILINEAR)
    return "BILINEAR";
  else
    return "GOOD";
}

static cdAttribute interp_attrib =
{
  "IMGINTERP",
  set_interp_attrib,
  get_interp_attrib
};

static char* get_cairodc_attrib(cdCtxCanvas *ctxcanvas)
{
  return (char*)ctxcanvas->cr;
}

static cdAttribute cairodc_attrib =
{
  "CAIRODC",
  NULL,
  get_cairodc_attrib
}; 

#if !PANGO_VERSION_CHECK(1,22,0)
static PangoContext * cd_pango_cairo_create_context (cairo_t *cr)
{
  PangoFontMap *fontmap = pango_cairo_font_map_get_default ();
  PangoContext *context = pango_context_new();
  pango_context_set_font_map (context, fontmap);
  pango_cairo_update_context (cr, context);
  return context;
}
#endif

cdCtxCanvas *cdcairoCreateCanvas(cdCanvas* canvas, cairo_t* cr)
{
  cdCtxCanvas *ctxcanvas = (cdCtxCanvas *)malloc(sizeof(cdCtxCanvas));
  memset(ctxcanvas, 0, sizeof(cdCtxCanvas));

  ctxcanvas->cr = cr;
  ctxcanvas->canvas = canvas;
  ctxcanvas->last_source = -1;
  ctxcanvas->hatchboxsize = 8;

  canvas->ctxcanvas = ctxcanvas;
  canvas->invert_yaxis = 1;

#if PANGO_VERSION_CHECK(1,22,0)
  ctxcanvas->fontcontext = pango_cairo_create_context(ctxcanvas->cr);
#else
  ctxcanvas->fontcontext = cd_pango_cairo_create_context(ctxcanvas->cr);
#endif
#if PANGO_VERSION_CHECK(1,16,0)
  pango_context_set_language(ctxcanvas->fontcontext, pango_language_get_default());
#endif

  cdRegisterAttribute(canvas, &rotate_attrib);
  cdRegisterAttribute(canvas, &version_attrib);
  cdRegisterAttribute(canvas, &poly_attrib);
  cdRegisterAttribute(canvas, &aa_attrib);
  cdRegisterAttribute(canvas, &linegradient_attrib);
  cdRegisterAttribute(canvas, &radialgradient_attrib);
  cdRegisterAttribute(canvas, &interp_attrib);
  cdRegisterAttribute(canvas, &cairodc_attrib);
  cdRegisterAttribute(canvas, &hatchboxsize_attrib);
  cdRegisterAttribute(canvas, &pattern_image_attrib);

  cairo_save(ctxcanvas->cr);
  cairo_set_operator(ctxcanvas->cr, CAIRO_OPERATOR_OVER);

  return ctxcanvas;
}

void cdcairoInitTable(cdCanvas* canvas)
{
  canvas->cxFlush = cdflush;
  canvas->cxClear = cdclear;

  canvas->cxPixel  = cdpixel;

  canvas->cxLine   = cdline;
  canvas->cxPoly   = cdpoly;
  canvas->cxRect   = cdrect;
  canvas->cxBox    = cdbox;
  canvas->cxArc    = cdarc;
  canvas->cxSector = cdsector;
  canvas->cxChord  = cdchord;
  canvas->cxText   = cdtext;

  canvas->cxFLine = cdfline;
  canvas->cxFPoly = cdfpoly;
  canvas->cxFRect = cdfrect;
  canvas->cxFBox = cdfbox;
  canvas->cxFArc = cdfarc;
  canvas->cxFSector = cdfsector;
  canvas->cxFChord = cdfchord;
  canvas->cxFText = cdftext;

  canvas->cxClip = cdclip;
  canvas->cxClipArea = cdcliparea;
  canvas->cxFClipArea = cdfcliparea;
  canvas->cxLineStyle = cdlinestyle;
  canvas->cxLineWidth = cdlinewidth;
  canvas->cxLineCap = cdlinecap;
  canvas->cxLineJoin = cdlinejoin;
  canvas->cxInteriorStyle = cdinteriorstyle;
  canvas->cxHatch = cdhatch;
  canvas->cxStipple = cdstipple;
  canvas->cxPattern = cdpattern;
  canvas->cxFont = cdfont;
  canvas->cxGetFontDim = cdgetfontdim;
  canvas->cxGetTextSize = cdgettextsize;
  canvas->cxTransform = cdtransform;
  canvas->cxForeground = cdforeground;

  canvas->cxGetImageRGB = cdgetimagergb;
  canvas->cxScrollArea = cdscrollarea;

  canvas->cxCreateImage = cdcreateimage;
  canvas->cxGetImage = cdgetimage;
  canvas->cxPutImageRect = cdputimagerect;
  canvas->cxKillImage = cdkillimage;

  canvas->cxPutImageRectRGB = cdputimagerectrgb;
  canvas->cxPutImageRectMap = cdputimagerectmap;
  canvas->cxPutImageRectRGBA = cdputimagerectrgba;
}
