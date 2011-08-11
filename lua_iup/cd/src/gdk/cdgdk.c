/** \file
 * \brief Gdk Base Driver
 *
 * See Copyright Notice in cd.h
 */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <math.h>

#include "cdgdk.h"

#define NUM_HATCHES  6
#define HATCH_WIDTH  8
#define HATCH_HEIGHT 8

#ifndef PANGO_VERSION_CHECK
#define PANGO_VERSION_CHECK(x,y,z) (0)
#endif

/* 
** 6 predefined patterns to be accessed through cdHatch(
   CD_HORIZONTAL | CD_VERTICAL | CD_FDIAGONAL | CD_BDIAGONAL |
   CD_CROSS      | CD_DIAGCROSS)

*/
static char hatches[NUM_HATCHES][8] = {
  {0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00},  /* HORIZONTAL */
  {0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22},  /* VERTICAL */
  {0x08,0x10,0x20,0x40,0x80,0x01,0x02,0x04},  /* FDIAGONAL */
  {0x10,0x08,0x04,0x02,0x01,0x80,0x40,0x20},  /* BDIAGONAL */
  {0x22,0x22,0xFF,0x22,0x22,0x22,0xFF,0x22},  /* CROSS */
  {0x18,0x18,0x24,0x42,0x81,0x81,0x42,0x24}   /* DIAGCROSS */
};

/******************************************************/

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

static char* sStrConvertToUTF8(cdCtxCanvas *ctxcanvas, const char* str, int length)  /* From CD to GDK */
{
  const char *charset = NULL;

  if (!str || *str == 0)
    return (char*)str;

  if (g_get_charset(&charset) == TRUE)  /* current locale is already UTF-8 */
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

static GdkColor cdColorToGdk(unsigned long rgb)
{
  GdkColor clrRGB;

  clrRGB.red   = cdCOLOR8TO16(cdRed(rgb));
  clrRGB.green = cdCOLOR8TO16(cdGreen(rgb));
  clrRGB.blue  = cdCOLOR8TO16(cdBlue(rgb));

  return clrRGB;
}

/******************************************************/

void cdgdkKillCanvas(cdCtxCanvas *ctxcanvas)
{
  if (ctxcanvas->canvas->bpp <= 8)
  {
    if (ctxcanvas->colormap != gdk_gc_get_colormap(ctxcanvas->gc))
      g_object_unref(ctxcanvas->colormap);
  }
 
  if (ctxcanvas->last_hatch) g_object_unref(ctxcanvas->last_hatch);

  if (ctxcanvas->fontdesc) pango_font_description_free(ctxcanvas->fontdesc);
  if (ctxcanvas->fontlayout)  g_object_unref(ctxcanvas->fontlayout);
  if (ctxcanvas->fontcontext) g_object_unref(ctxcanvas->fontcontext);

  if (ctxcanvas->new_rgn) gdk_region_destroy(ctxcanvas->new_rgn);
  if (ctxcanvas->clip_rgn) gdk_region_destroy(ctxcanvas->clip_rgn);

  if (ctxcanvas->last_pattern)
  {
    g_object_unref(ctxcanvas->last_pattern_gc); 
    g_object_unref(ctxcanvas->last_pattern);
  }

  if (ctxcanvas->last_stipple)
  {
    g_object_unref(ctxcanvas->last_stipple_gc); 
    g_object_unref(ctxcanvas->last_stipple);
  }

  if (ctxcanvas->strLastConvertUTF8)
    g_free(ctxcanvas->strLastConvertUTF8);

  g_object_unref(ctxcanvas->gc); 

  free(ctxcanvas);
}

/******************************************************/

static void cdflush(cdCtxCanvas *ctxcanvas)
{
  (void)ctxcanvas;
  gdk_error_trap_push();
  gdk_flush();
  gdk_error_trap_pop();
}

/******************************************************/

static int cdclip(cdCtxCanvas *ctxcanvas, int clip_mode)
{
  switch (clip_mode)
  {
  case CD_CLIPOFF:
    gdk_gc_set_clip_region(ctxcanvas->gc, NULL);
    break;
  case CD_CLIPAREA:
    {
      GdkRectangle rect;
      rect.x      = ctxcanvas->canvas->clip_rect.xmin;
      rect.y      = ctxcanvas->canvas->clip_rect.ymin;
      rect.width  = ctxcanvas->canvas->clip_rect.xmax - ctxcanvas->canvas->clip_rect.xmin;
      rect.height = ctxcanvas->canvas->clip_rect.ymax - ctxcanvas->canvas->clip_rect.ymin;
      gdk_gc_set_clip_rectangle(ctxcanvas->gc, &rect);
      break;
    }
  case CD_CLIPPOLYGON:
    if (ctxcanvas->clip_rgn)
      gdk_gc_set_clip_region(ctxcanvas->gc, ctxcanvas->clip_rgn);
    break;
  case CD_CLIPREGION:
    if (ctxcanvas->new_rgn)
      gdk_gc_set_clip_region(ctxcanvas->gc, ctxcanvas->new_rgn);
    break;
  }

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

static void cdnewregion(cdCtxCanvas *ctxcanvas)
{
  if (ctxcanvas->new_rgn)
    gdk_region_destroy(ctxcanvas->new_rgn);

  ctxcanvas->new_rgn = gdk_region_new(); 
}

static int cdispointinregion(cdCtxCanvas *ctxcanvas, int x, int y)
{
  if (!ctxcanvas->new_rgn)
    return 0;

  if (gdk_region_point_in(ctxcanvas->new_rgn, x, y))
    return 1;

  return 0;
}

static void cdoffsetregion(cdCtxCanvas *ctxcanvas, int x, int y)
{
  if (!ctxcanvas->new_rgn)
    return;
  
  gdk_region_offset(ctxcanvas->new_rgn, x, y);
}

static void cdgetregionbox(cdCtxCanvas *ctxcanvas, int *xmin, int *xmax, int *ymin, int *ymax)
{
  GdkRectangle rect;

  if (!ctxcanvas->new_rgn)
    return;

  gdk_region_get_clipbox(ctxcanvas->new_rgn, &rect);

  *xmin = rect.x;
  *xmax = rect.x + rect.width;
  *ymin = rect.y;
  *ymax = rect.y + rect.height;
}

static void sCombineRegion(cdCtxCanvas *ctxcanvas, GdkRegion* rgn)
{
  switch(ctxcanvas->canvas->combine_mode)
  {                          
  case CD_UNION:
    gdk_region_union(ctxcanvas->new_rgn, rgn);
    break;
  case CD_INTERSECT:   
    gdk_region_intersect(ctxcanvas->new_rgn, rgn);
    break;
  case CD_DIFFERENCE:           
    gdk_region_subtract(ctxcanvas->new_rgn, rgn);
    break;
  case CD_NOTINTERSECT:
    gdk_region_xor(ctxcanvas->new_rgn, rgn);
    break;
  }

  gdk_region_destroy(rgn);
}

/******************************************************/

static int cdwritemode(cdCtxCanvas *ctxcanvas, int write_mode)
{
  switch (write_mode)
  {
  case CD_REPLACE:
    gdk_gc_set_function(ctxcanvas->gc, GDK_COPY);
    break;
  case CD_XOR:
    gdk_gc_set_function(ctxcanvas->gc, GDK_XOR);
    break;
  case CD_NOT_XOR:
    gdk_gc_set_function(ctxcanvas->gc, GDK_EQUIV);
    break;
  }

  return write_mode;
}

static int cdinteriorstyle(cdCtxCanvas *ctxcanvas, int style)
{
  GdkFill sty = GDK_SOLID;

  switch (style)
  {
    case CD_SOLID:
      sty = GDK_SOLID;
      break;

    case CD_HATCH :
      if (!ctxcanvas->last_hatch) 
        return ctxcanvas->canvas->interior_style;

      gdk_gc_set_stipple(ctxcanvas->gc, ctxcanvas->last_hatch);

      if (ctxcanvas->canvas->back_opacity == CD_OPAQUE)
        sty = GDK_OPAQUE_STIPPLED;
      else
        sty = GDK_STIPPLED;
      break;

    case CD_STIPPLE:
      gdk_gc_set_stipple(ctxcanvas->gc, ctxcanvas->last_stipple);

      if (ctxcanvas->canvas->back_opacity == CD_OPAQUE)
        sty = GDK_OPAQUE_STIPPLED;
      else
        sty = GDK_STIPPLED;
      break;

    case CD_PATTERN:
      gdk_gc_set_tile(ctxcanvas->gc, ctxcanvas->last_pattern);
      sty = GDK_TILED;
      break;
  }

  gdk_gc_set_fill(ctxcanvas->gc, sty);

  return style;
}

static int cdhatch(cdCtxCanvas *ctxcanvas, int hatch_style)
{
  GdkColor fg, bg;

  if (ctxcanvas->last_hatch)
    g_object_unref(ctxcanvas->last_hatch);

  fg.pixel = 1;
  bg.pixel = 0;

  ctxcanvas->last_hatch = gdk_pixmap_create_from_data(ctxcanvas->wnd, hatches[hatch_style],
                                                      HATCH_WIDTH, HATCH_HEIGHT, 1, &fg, &bg);

  cdinteriorstyle(ctxcanvas, CD_HATCH);

  return hatch_style;
}

static void cdstipple(cdCtxCanvas *ctxcanvas, int w, int h, const unsigned char *data)
{
  int x, y;

  if (ctxcanvas->last_stipple == 0 || (ctxcanvas->last_stipple_w != w || ctxcanvas->last_stipple_h != h))
  {
    if (ctxcanvas->last_stipple != 0)
    {
      g_object_unref(ctxcanvas->last_stipple);
      g_object_unref(ctxcanvas->last_stipple_gc);
    }

    ctxcanvas->last_stipple = gdk_pixmap_new(ctxcanvas->wnd, w, h, 1);
    if (!ctxcanvas->last_stipple)
      return;
    
    ctxcanvas->last_stipple_gc = gdk_gc_new((GdkDrawable*)ctxcanvas->last_stipple);
    ctxcanvas->last_stipple_w = w;
    ctxcanvas->last_stipple_h = h;
  }

  for (y = 0; y < h; y++)
  {
    for (x = 0; x < w; x++)
    {
      GdkColor clr;

      if(data[y*w+x])
        clr.pixel = 1;
      else
        clr.pixel = 0;

      gdk_gc_set_foreground(ctxcanvas->last_stipple_gc, &clr);
      gdk_draw_point(ctxcanvas->last_stipple, ctxcanvas->last_stipple_gc, x, h-y-1);
    }
  }

  cdinteriorstyle(ctxcanvas, CD_STIPPLE);
}

static void cdpattern(cdCtxCanvas *ctxcanvas, int w, int h, const long int *colors)
{
  int x, y;
  GdkColor color;

  if (ctxcanvas->last_pattern == 0 || (ctxcanvas->last_pattern_w != w || ctxcanvas->last_pattern_h != h))
  {
    if (ctxcanvas->last_pattern != 0)
    {
      g_object_unref(ctxcanvas->last_pattern);
      g_object_unref(ctxcanvas->last_pattern_gc);
    }

    ctxcanvas->last_pattern = gdk_pixmap_new(ctxcanvas->wnd, w, h, ctxcanvas->depth);
    if (!ctxcanvas->last_pattern)
      return;

    ctxcanvas->last_pattern_gc = gdk_gc_new((GdkDrawable*)ctxcanvas->last_pattern);
    ctxcanvas->last_pattern_w = w;
    ctxcanvas->last_pattern_h = h;
  }

  for (y = 0; y < h; y++)
  {
    for (x = 0; x < w; x++)
    {
      color = cdColorToGdk(colors[y*w+x]);
      gdk_gc_set_rgb_fg_color(ctxcanvas->last_pattern_gc, &color);
      gdk_draw_point(ctxcanvas->last_pattern, ctxcanvas->last_pattern_gc, x, h-y-1);
    }
  }

  cdinteriorstyle(ctxcanvas, CD_PATTERN);
}

static int cdlinestyle(cdCtxCanvas *ctxcanvas, int style)
{
  switch (style)
  {
  case CD_CONTINUOUS:
    ctxcanvas->gcval.line_style = GDK_LINE_SOLID;
    break;
  case CD_DASHED:
  case CD_DOTTED:
  case CD_DASH_DOT:
  case CD_DASH_DOT_DOT:
    {
      static struct {
        int size;
        signed char list[6];
      } dashes[4] = {
        { 2, { 6, 2 } },
        { 2, { 2, 2 } },
        { 4, { 6, 2, 2, 2 } },
        { 6, { 6, 2, 2, 2, 2, 2 } }
      };

      if (ctxcanvas->canvas->back_opacity == CD_OPAQUE)
        ctxcanvas->gcval.line_style = GDK_LINE_DOUBLE_DASH;
      else
        ctxcanvas->gcval.line_style = GDK_LINE_ON_OFF_DASH;
        
      gdk_gc_set_dashes(ctxcanvas->gc, 0, dashes[style-CD_DASHED].list, dashes[style-CD_DASHED].size);
      break;
    }
  case CD_CUSTOM:        
    {
      int i;
      signed char* dash_style = (signed char*)malloc(ctxcanvas->canvas->line_dashes_count);
      for (i = 0; i < ctxcanvas->canvas->line_dashes_count; i++)
        dash_style[i] = (char)ctxcanvas->canvas->line_dashes[i];

      if (ctxcanvas->canvas->back_opacity == CD_OPAQUE)
        ctxcanvas->gcval.line_style = GDK_LINE_DOUBLE_DASH;
      else
        ctxcanvas->gcval.line_style = GDK_LINE_ON_OFF_DASH;

      gdk_gc_set_dashes(ctxcanvas->gc, 0, dash_style, ctxcanvas->canvas->line_dashes_count);
      free(dash_style);
      break;
    }
  }

  gdk_gc_set_values(ctxcanvas->gc, &ctxcanvas->gcval, GDK_GC_LINE_STYLE);

  return style;
}

static int cdlinewidth(cdCtxCanvas *ctxcanvas, int width)
{
  if (width == 1) 
    ctxcanvas->gcval.line_width = 0;
  else
    ctxcanvas->gcval.line_width = width;

  gdk_gc_set_values(ctxcanvas->gc, &ctxcanvas->gcval, GDK_GC_LINE_WIDTH);

  return width;
}

static int cdlinecap(cdCtxCanvas *ctxcanvas, int cap)
{
  int cd2x_cap[] =  {GDK_CAP_BUTT, GDK_CAP_PROJECTING, GDK_CAP_ROUND};

  ctxcanvas->gcval.cap_style = cd2x_cap[cap];
  gdk_gc_set_values(ctxcanvas->gc, &ctxcanvas->gcval, GDK_GC_CAP_STYLE);

  return cap;
}

static int cdlinejoin(cdCtxCanvas *ctxcanvas, int join)
{
  int cd2x_join[] = {GDK_JOIN_MITER, GDK_JOIN_BEVEL, GDK_JOIN_ROUND};

  ctxcanvas->gcval.join_style = cd2x_join[join];
  gdk_gc_set_values(ctxcanvas->gc, &ctxcanvas->gcval, GDK_GC_JOIN_STYLE);

  return join;
}

static int cdbackopacity(cdCtxCanvas *ctxcanvas, int opaque)
{
  ctxcanvas->canvas->back_opacity = opaque;
  cdinteriorstyle(ctxcanvas, ctxcanvas->canvas->interior_style);
  cdlinestyle(ctxcanvas, ctxcanvas->canvas->line_style);
  return opaque;
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

  return 1;
}

static void cdgetfontdim(cdCtxCanvas *ctxcanvas, int *max_width, int *height, int *ascent, int *descent)
{
  PangoFontMetrics* metrics;
  int charwidth, charheight, charascent, chardescent;

  if(!ctxcanvas->fontdesc)
    return;

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

static long int cdbackground(cdCtxCanvas *ctxcanvas, long int color)
{
  ctxcanvas->bg = cdColorToGdk(color);
  gdk_gc_set_rgb_bg_color(ctxcanvas->gc, &ctxcanvas->bg);
  return color;
}

static long int cdforeground(cdCtxCanvas *ctxcanvas, long int color)
{          
  ctxcanvas->fg = cdColorToGdk(color);
  gdk_gc_set_rgb_fg_color(ctxcanvas->gc, &ctxcanvas->fg);
  return color;
}

static void cdpalette(cdCtxCanvas *ctxcanvas, int n, const long int *palette, int mode)
{
  int i;
  GdkColor clr;

  if (mode == CD_FORCE)
  {
    /* if was POLITE then allocates own palette */
    if (ctxcanvas->colormap == gdk_gc_get_colormap(ctxcanvas->gc))
      ctxcanvas->colormap = gdk_colormap_new(ctxcanvas->vis, FALSE);

    /* allocate all the palette colors to the CD */
    for (i = 0; i < n; i++)
    {
      clr = cdColorToGdk(palette[i]);
      gdk_colormap_alloc_color(ctxcanvas->colormap, &clr, FALSE, FALSE);
    }

    /* set directly on the drawable */
    gdk_drawable_set_colormap(ctxcanvas->wnd, ctxcanvas->colormap);
  }
  else
  {
    /* if was FORCE, remove the own palette */
    if (ctxcanvas->colormap != gdk_gc_get_colormap(ctxcanvas->gc))
    {
      g_object_unref(ctxcanvas->colormap);
      ctxcanvas->colormap = gdk_gc_get_colormap(ctxcanvas->gc);
    }

    /* if POLITE then just try to allocate all the colors of the palette */
    for (i = 0; i < n; i++)
    {
      clr = cdColorToGdk(palette[i]);
      gdk_colormap_alloc_color(ctxcanvas->colormap, &clr, FALSE, TRUE);
    }
  }
}

/******************************************************/

static void cdgdkCheckSolidStyle(cdCtxCanvas *ctxcanvas, int set)
{
  if (ctxcanvas->canvas->interior_style == CD_SOLID)
    return;

  if (set)
    gdk_gc_set_fill(ctxcanvas->gc, GDK_SOLID);
  else
    cdinteriorstyle(ctxcanvas, ctxcanvas->canvas->interior_style);
}

static void cdclear(cdCtxCanvas* ctxcanvas)
{
  GdkColor clr;

  cdgdkCheckSolidStyle(ctxcanvas, 1);

  if (ctxcanvas->canvas->clip_mode != CD_CLIPOFF) 
    gdk_gc_set_clip_region(ctxcanvas->gc, NULL);

  clr = cdColorToGdk(ctxcanvas->canvas->background);
  gdk_gc_set_rgb_fg_color(ctxcanvas->gc, &clr);

  gdk_draw_rectangle(ctxcanvas->wnd, ctxcanvas->gc, TRUE, 0, 0, ctxcanvas->canvas->w, ctxcanvas->canvas->h);

  clr = cdColorToGdk(ctxcanvas->canvas->foreground);
  gdk_gc_set_rgb_fg_color(ctxcanvas->gc, &clr);

  if (ctxcanvas->canvas->clip_mode != CD_CLIPOFF) 
    cdclip(ctxcanvas, ctxcanvas->canvas->clip_mode);

  cdgdkCheckSolidStyle(ctxcanvas, 0);
}

static void cdline(cdCtxCanvas *ctxcanvas, int x1, int y1, int x2, int y2)
{ 
  if (ctxcanvas->canvas->use_matrix)
  {
    cdMatrixTransformPoint(ctxcanvas->xmatrix, x1, y1, &x1, &y1);
    cdMatrixTransformPoint(ctxcanvas->xmatrix, x2, y2, &x2, &y2);
  }

  cdgdkCheckSolidStyle(ctxcanvas, 1);
  gdk_draw_line(ctxcanvas->wnd, ctxcanvas->gc, x1, y1, x2, y2);
  cdgdkCheckSolidStyle(ctxcanvas, 0);
}

static void cdarc(cdCtxCanvas *ctxcanvas, int xc, int yc, int w, int h, double a1, double a2)
{
  if (ctxcanvas->canvas->use_matrix)
  {
    cdSimArc(ctxcanvas, xc, yc, w, h, a1, a2);
    return;
  }

  /* angles in 1/64ths of degrees counterclockwise, similar to CD */

  cdgdkCheckSolidStyle(ctxcanvas, 1);
  gdk_draw_arc(ctxcanvas->wnd, ctxcanvas->gc, FALSE, xc-w/2, yc-h/2, w, h, cdRound(a1*64), cdRound((a2 - a1)*64));
  cdgdkCheckSolidStyle(ctxcanvas, 0);
}

static void cdsector(cdCtxCanvas *ctxcanvas, int xc, int yc, int w, int h, double a1, double a2)
{
  if (ctxcanvas->canvas->use_matrix)
  {
    cdSimSector(ctxcanvas, xc, yc, w, h, a1, a2);
    return;
  }

  if (ctxcanvas->canvas->new_region)
  {
    cdSimSector(ctxcanvas, xc, yc, w, h, a1, a2);
  }
  else
  {
    /* "filled parameter = TRUE" produces an 'pie slice' */
    gdk_draw_arc(ctxcanvas->wnd, ctxcanvas->gc, TRUE, xc-w/2, yc-h/2, w, h, cdRound(a1*64), cdRound((a2 - a1)*64));
  }
}

static void cdrect(cdCtxCanvas *ctxcanvas, int xmin, int xmax, int ymin, int ymax)
{
  if (ctxcanvas->canvas->use_matrix)
  {
    cdSimRect(ctxcanvas, xmin, xmax, ymin, ymax);
    return;
  }

  cdgdkCheckSolidStyle(ctxcanvas, 1);
  gdk_draw_rectangle(ctxcanvas->wnd, ctxcanvas->gc, FALSE, xmin, ymin, xmax-xmin, ymax-ymin);  /* outlined rectangle is actually of size w+1,h+1 */
  cdgdkCheckSolidStyle(ctxcanvas, 0);
}

static void cdbox(cdCtxCanvas *ctxcanvas, int xmin, int xmax, int ymin, int ymax)
{
  if (ctxcanvas->canvas->use_matrix)
  {
    cdSimBox(ctxcanvas, xmin, xmax, ymin, ymax);
    return;
  }

  if (ctxcanvas->canvas->new_region)
  {
    GdkRegion *rgn;
    GdkRectangle rect;

    rect.x = xmin;  rect.width  = xmax-xmin;
    rect.y = ymin;  rect.height = ymax-ymin;
    rgn = gdk_region_rectangle(&rect);

    sCombineRegion(ctxcanvas, rgn);
  }
  else
    gdk_draw_rectangle(ctxcanvas->wnd, ctxcanvas->gc, TRUE, xmin, ymin, xmax-xmin+1, ymax-ymin+1);
}

static void cdtext(cdCtxCanvas *ctxcanvas, int x, int y, const char *s, int len)
{
  PangoFontMetrics* metrics;
  int w, h, desc, dir = -1;
  int ox = x, oy = y;

  pango_layout_set_text(ctxcanvas->fontlayout, sStrConvertToUTF8(ctxcanvas, s, len), -1);
  
	pango_layout_get_pixel_size(ctxcanvas->fontlayout, &w, &h);
  metrics = pango_context_get_metrics(ctxcanvas->fontcontext, ctxcanvas->fontdesc, pango_context_get_language(ctxcanvas->fontcontext));
  desc = (((pango_font_metrics_get_descent(metrics)) + PANGO_SCALE/2) / PANGO_SCALE);

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

  if(!ctxcanvas->canvas->use_matrix)
  {
    ctxcanvas->fontmatrix.xx = 1;     ctxcanvas->fontmatrix.xy = 0;
    ctxcanvas->fontmatrix.yx = 0;     ctxcanvas->fontmatrix.yy = 1;
    ctxcanvas->fontmatrix.x0 = 0;     ctxcanvas->fontmatrix.y0 = 0;
  }

  if (ctxcanvas->canvas->text_orientation != 0)
    pango_matrix_rotate(&ctxcanvas->fontmatrix, (double)ctxcanvas->canvas->text_orientation);

  if (ctxcanvas->canvas->use_matrix || ctxcanvas->canvas->text_orientation != 0)
  {
    PangoRectangle rect;
    double angle = CD_DEG2RAD*ctxcanvas->canvas->text_orientation;
    double cos_angle = cos(angle);
    double sin_angle = sin(angle);

    pango_context_set_matrix (ctxcanvas->fontcontext, &ctxcanvas->fontmatrix);
    pango_layout_context_changed (ctxcanvas->fontlayout);

    pango_layout_get_pixel_extents(ctxcanvas->fontlayout, NULL, &rect);
#if PANGO_VERSION_CHECK(1,16,0)
    pango_matrix_transform_pixel_rectangle(&ctxcanvas->fontmatrix, &rect);
#endif

    if (ctxcanvas->canvas->text_orientation)
      cdRotatePoint(ctxcanvas->canvas, x, y, ox, oy, &x, &y, sin_angle, cos_angle);
    
    if (ctxcanvas->canvas->use_matrix)
      cdMatrixTransformPoint(ctxcanvas->xmatrix, x, y, &x, &y);

    /* Defines the new position (X,Y), considering the Pango rectangle transformed */
    x += (int)rect.x;
    y += (int)rect.y;
  }

  cdgdkCheckSolidStyle(ctxcanvas, 1);

  if (ctxcanvas->canvas->new_region)
  {
    GdkRegion *rgn;
    gint *idx;
    gint range;

    pango_layout_line_get_x_ranges(pango_layout_get_line(ctxcanvas->fontlayout, 0), 0, len, &idx, &range);

    /* TODO: this is only the bounding box of the layout, not the text itself,
             must transform the text into a polygon. */
    rgn = gdk_pango_layout_get_clip_region(ctxcanvas->fontlayout, x, y, idx, range);

    sCombineRegion(ctxcanvas, rgn);
  }
  else
    gdk_draw_layout(ctxcanvas->wnd, ctxcanvas->gc, x, y, ctxcanvas->fontlayout);

  pango_context_set_matrix(ctxcanvas->fontcontext, NULL);

  cdgdkCheckSolidStyle(ctxcanvas, 0);

  pango_font_metrics_unref(metrics); 
}

static void cdgettextsize(cdCtxCanvas *ctxcanvas, const char *s, int len, int *width, int *height)
{
  if (!ctxcanvas->fontlayout)
    return;

  pango_layout_set_text(ctxcanvas->fontlayout, sStrConvertToUTF8(ctxcanvas, s, len), len);
  pango_layout_get_pixel_size(ctxcanvas->fontlayout, width, height);
}

static void cdpoly(cdCtxCanvas *ctxcanvas, int mode, cdPoint* poly, int n)
{
  int i;

  if (mode != CD_BEZIER && mode != CD_PATH)
  {
    for (i = 0; i < n; i++)
    {
      if (ctxcanvas->canvas->use_matrix)
        cdMatrixTransformPoint(ctxcanvas->xmatrix, poly[i].x, poly[i].y, &(poly[i].x), &(poly[i].y));
    }
  }

  switch( mode )
  {
  case CD_FILL:
    if (ctxcanvas->canvas->new_region)
    {
      GdkRegion* rgn = gdk_region_polygon((GdkPoint*)poly, n, ctxcanvas->canvas->fill_mode == CD_EVENODD ? GDK_EVEN_ODD_RULE : GDK_WINDING_RULE);
      sCombineRegion(ctxcanvas, rgn);
    }
    else
      gdk_draw_polygon(ctxcanvas->wnd, ctxcanvas->gc, TRUE, (GdkPoint*)poly, n);
    break;

  case CD_CLOSED_LINES:
    cdgdkCheckSolidStyle(ctxcanvas, 1);
    gdk_draw_polygon(ctxcanvas->wnd, ctxcanvas->gc, FALSE, (GdkPoint*)poly, n);
    cdgdkCheckSolidStyle(ctxcanvas, 0);
    break;

  case CD_OPEN_LINES:
    cdgdkCheckSolidStyle(ctxcanvas, 1);
    gdk_draw_lines(ctxcanvas->wnd, ctxcanvas->gc, (GdkPoint*)poly, n);
    cdgdkCheckSolidStyle(ctxcanvas, 0);
    break;

  case CD_CLIP:
    ctxcanvas->clip_rgn = gdk_region_polygon((GdkPoint*)poly, n, ctxcanvas->canvas->fill_mode == CD_EVENODD ? GDK_EVEN_ODD_RULE : GDK_WINDING_RULE);     
    if (ctxcanvas->canvas->clip_mode == CD_CLIPPOLYGON)
      cdclip(ctxcanvas, CD_CLIPPOLYGON);
    break;

  case CD_BEZIER:
    cdSimPolyBezier(ctxcanvas->canvas, poly, n);
    break;

  case CD_PATH:
    cdSimPolyPath(ctxcanvas->canvas, poly, n);
    break;
  }
}

/******************************************************/

static void cdgdkGetPixbufData(GdkPixbuf* pixbuf, unsigned char *r, unsigned char *g, unsigned char *b)
{
  int w, h, y, x, bpp;
  guchar *pixdata, *pixline_data;
  int rowstride, channels;

  w = gdk_pixbuf_get_width(pixbuf);
  h = gdk_pixbuf_get_height(pixbuf);
  bpp = gdk_pixbuf_get_bits_per_sample(pixbuf)*gdk_pixbuf_get_n_channels(pixbuf);

  if (bpp!=24 && bpp!=32)
    return;

  pixdata = gdk_pixbuf_get_pixels(pixbuf);
  rowstride = gdk_pixbuf_get_rowstride(pixbuf);
  channels = gdk_pixbuf_get_n_channels(pixbuf);

  /* planes are separated in imgdata */
  for (y=0; y<h; y++)
  {
    int lineoffset = (h-1 - y)*w;  /* imgdata is bottom up */
    pixline_data = pixdata + y * rowstride;
    for(x=0;x<w;x++)
    {
      int pos = x*channels;
      r[lineoffset+x] = pixline_data[pos];
      g[lineoffset+x] = pixline_data[pos+1];
      b[lineoffset+x] = pixline_data[pos+2];
    }
  }
}

static GdkPixbuf* cdgdkCreatePixbufRGBA(int width, int height, const unsigned char *r, const unsigned char *g, const unsigned char *b, const unsigned char *a, int ix, int iy, int image_width)
{
  GdkPixbuf* pixbuf;
  guchar *pixdata, *pixline_data;
  int rowstride, channels;
  int x, y;

  pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, a?TRUE:FALSE, 8, width, height);
  if (!pixbuf)
    return NULL;

  pixdata = gdk_pixbuf_get_pixels(pixbuf);
  rowstride = gdk_pixbuf_get_rowstride(pixbuf);
  channels = gdk_pixbuf_get_n_channels(pixbuf);

  /* GdkPixbuf is top-bottom */
  /* imgdata is bottom up */

  /* planes are separated in imgdata */
  for (y=0; y<height; y++)
  {
    int lineoffset = (height-1 - y + iy)*image_width;  /* imgdata is bottom up */
    pixline_data = pixdata + y * rowstride;
    for(x=0;x<width;x++)
    {
      int pos = x*channels;
      pixline_data[pos] = r[lineoffset+x+ix];
      pixline_data[pos+1] = g[lineoffset+x+ix];
      pixline_data[pos+2] = b[lineoffset+x+ix];

      if (a)
        pixline_data[pos+3] = a[lineoffset+x+ix];
    }
  }

  return pixbuf;
}

static GdkPixbuf* cdgdkCreatePixbufMap(int width, int height, const long* colors, const unsigned char *map, int ix, int iy, int image_width)
{
  GdkPixbuf* pixbuf;
  guchar *pixdata, *pixline_data;
  int rowstride, channels;
  const unsigned char *line_data;
  int x, y;

  pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8, width, height);
  if (!pixbuf)
    return NULL;

  pixdata = gdk_pixbuf_get_pixels(pixbuf);
  rowstride = gdk_pixbuf_get_rowstride(pixbuf);
  channels = gdk_pixbuf_get_n_channels(pixbuf);

  /* GdkPixbuf is top-bottom */
  /* map is bottom up */

  for (y=0; y<height; y++)
  {
    pixline_data = pixdata + y * rowstride;
    line_data = map + (height-1 - y + iy) * image_width;  /* map is bottom up */

    for (x=0; x<width; x++)
    {
      unsigned char index = line_data[x+ix];
      long c = colors[index];
      guchar *r = &pixline_data[channels*x],
             *g = r+1,
             *b = g+1;

      *r = cdRed(c);
      *g = cdGreen(c);
      *b = cdBlue(c);
    }
  }

  return pixbuf;
}

static void cdgetimagergb(cdCtxCanvas *ctxcanvas, unsigned char *r, unsigned char *g, unsigned char *b, int x, int y, int w, int h)
{
  GdkPixbuf* pixbuf = gdk_pixbuf_get_from_drawable(NULL, ctxcanvas->wnd, ctxcanvas->colormap, 
                                                   x, y-h+1, 
                                                   0, 0, w, h);
  if (!pixbuf)
    return;

  cdgdkGetPixbufData(pixbuf, r, g, b);
}

static void cdputimagerectrgba_matrix(cdCtxCanvas* ctxcanvas, int iw, int ih, const unsigned char *r, const unsigned char *g, const unsigned char *b, const unsigned char *a, int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax)
{
  int t_xmin, t_xmax, t_ymin, t_ymax, ew, eh,
      t_x, t_y, dst_offset, size, nc, doff, rect[8];
  float i_x, i_y, xfactor, yfactor;
  unsigned char *dst_r, *dst_g, *dst_b, *dst_a = NULL;
  double inv_matrix[6];

  /* calculate the destination limits */
  cdImageRGBCalcDstLimits(ctxcanvas->canvas, x, y, w, h, &t_xmin, &t_xmax, &t_ymin, &t_ymax, rect);

  /* Setup inverse transform (use the original transform here, NOT ctxcanvas->xmatrix) */
  cdImageRGBInitInverseTransform(w, h, xmin, xmax, ymin, ymax, &xfactor, &yfactor, ctxcanvas->canvas->matrix, inv_matrix);

  /* create an image for the destination area */
  ew = (t_xmax-t_xmin+1);
  eh = (t_ymax-t_ymin+1); 
  size = ew*eh;
  nc = 3;
  if (a) nc = 4;
  dst_r = malloc(nc*size);
  if (!dst_r)
    return;

  dst_g = dst_r + size;
  dst_b = dst_g + size;
  if (a) dst_a = dst_b + size;
  memset(dst_r, 0, nc*size);

  /* for all pixels in the destiny area */
  for(t_y = t_ymin; t_y <= t_ymax; t_y++)
  {
    dst_offset = (t_y-t_ymin) * ew;

    for(t_x = t_xmin; t_x <= t_xmax; t_x++)
    {
      cdImageRGBInverseTransform(t_x, t_y, &i_x, &i_y, xfactor, yfactor, xmin, ymin, x, y, inv_matrix);

      if (i_x > xmin && i_y > ymin && i_x < xmax+1 && i_y < ymax+1)
      {
        doff = (t_x-t_xmin) + dst_offset;
        *(dst_r+doff) = cdBilinearInterpolation(iw, ih, r, i_x, i_y);
        *(dst_g+doff) = cdBilinearInterpolation(iw, ih, g, i_x, i_y);
        *(dst_b+doff) = cdBilinearInterpolation(iw, ih, b, i_x, i_y);
        if (a) *(dst_a+doff) = cdBilinearInterpolation(iw, ih, a, i_x, i_y);
      }
    }
  }

  {
    int ex = t_xmin, 
        ey = t_ymin + eh-1;  /* GdkPixbuf origin is at top-left */
    GdkPixbuf *pixbuf;
    GdkRegion *clip_polygon;
    GdkPoint* pnt = g_malloc(64);

    /* Since the transformation used was the original transformation, */
    /* must invert the Y axis here. */
    ey = _cdInvertYAxis(ctxcanvas->canvas, ey);

    /* use clipping to select only the transformed rectangle */
    pnt[0].x = (short)rect[0]; pnt[0].y = (short)_cdInvertYAxis(ctxcanvas->canvas, rect[1]);
    pnt[1].x = (short)rect[2]; pnt[1].y = (short)_cdInvertYAxis(ctxcanvas->canvas, rect[3]);
    pnt[2].x = (short)rect[4]; pnt[2].y = (short)_cdInvertYAxis(ctxcanvas->canvas, rect[5]);
    pnt[3].x = (short)rect[6]; pnt[3].y = (short)_cdInvertYAxis(ctxcanvas->canvas, rect[7]);
    clip_polygon = gdk_region_polygon(pnt, 4, ctxcanvas->canvas->fill_mode == CD_EVENODD ? GDK_EVEN_ODD_RULE : GDK_WINDING_RULE);

    /* combine with the existing clipping */
    gdk_gc_set_function(ctxcanvas->gc, GDK_AND);
    gdk_gc_set_clip_region(ctxcanvas->gc, clip_polygon);

    cdwritemode(ctxcanvas, ctxcanvas->canvas->write_mode);  /* reset gdk_gc_set_function */

    pixbuf = cdgdkCreatePixbufRGBA(ew, eh, dst_r, dst_g, dst_b, dst_a, 0, 0, ew);
    if (!pixbuf)
      return;

    gdk_draw_pixbuf(ctxcanvas->wnd, ctxcanvas->gc, pixbuf, 0, 0, ex, ey, -1, -1, ctxcanvas->img_dither, 0, 0);

    /* reset clipping */
    gdk_region_destroy(clip_polygon);
    cdclip(ctxcanvas, ctxcanvas->canvas->clip_mode);

    g_object_unref(pixbuf);
    g_free(pnt);
  }

  free(dst_r);
}

static void cdputimagerectmap_matrix(cdCtxCanvas* ctxcanvas, int iw, int ih, const unsigned char *index, const long int *colors, int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax)
{
  int t_xmin, t_xmax, t_ymin, t_ymax, ew, eh,
      t_x, t_y, dst_offset, size, doff, rect[8];
  float i_x, i_y, xfactor, yfactor;
  unsigned char *dst_index;
  double inv_matrix[6];

  /* calculate the destination limits */
  cdImageRGBCalcDstLimits(ctxcanvas->canvas, x, y, w, h, &t_xmin, &t_xmax, &t_ymin, &t_ymax, rect);

  /* Setup inverse transform (use the original transform here, NOT ctxcanvas->xmatrix) */
  cdImageRGBInitInverseTransform(w, h, xmin, xmax, ymin, ymax, &xfactor, &yfactor, ctxcanvas->canvas->matrix, inv_matrix);

  /* create an image for the destination area */
  ew = (t_xmax-t_xmin+1);
  eh = (t_ymax-t_ymin+1); 
  size = ew*eh;
  dst_index = malloc(size);
  if (!dst_index)
    return;

  memset(dst_index, 0, size);

  /* for all pixels in the destiny area */
  for(t_y = t_ymin; t_y <= t_ymax; t_y++)
  {
    dst_offset = (t_y-t_ymin) * ew;

    for(t_x = t_xmin; t_x <= t_xmax; t_x++)
    {
      cdImageRGBInverseTransform(t_x, t_y, &i_x, &i_y, xfactor, yfactor, xmin, ymin, x, y, inv_matrix);

      if (i_x > xmin && i_y > ymin && i_x < xmax+1 && i_y < ymax+1)
      {
        doff = (t_x-t_xmin) + dst_offset;
        *(dst_index+doff) = cdZeroOrderInterpolation(iw, ih, index, i_x, i_y);
      }
    }
  }

  {
    int ex = t_xmin, 
        ey = t_ymin + eh-1;  /* GdkPixbuf* origin is at top-left */
    
    GdkPixbuf *pixbuf;
    GdkRegion *clip_polygon;
    GdkPoint pnt[4];

    /* Since the transformation used was the original transformation, */
    /* must invert the Y axis here. */
    ey = _cdInvertYAxis(ctxcanvas->canvas, ey);

    /* use clipping to select only the transformed rectangle */
    pnt[0].x = (short)rect[0]; pnt[0].y = (short)_cdInvertYAxis(ctxcanvas->canvas, rect[1]);
    pnt[1].x = (short)rect[2]; pnt[1].y = (short)_cdInvertYAxis(ctxcanvas->canvas, rect[3]);
    pnt[2].x = (short)rect[4]; pnt[2].y = (short)_cdInvertYAxis(ctxcanvas->canvas, rect[5]);
    pnt[3].x = (short)rect[6]; pnt[3].y = (short)_cdInvertYAxis(ctxcanvas->canvas, rect[7]);
    clip_polygon = gdk_region_polygon(pnt, 4, ctxcanvas->canvas->fill_mode == CD_EVENODD ? GDK_EVEN_ODD_RULE : GDK_WINDING_RULE);

    /* combine with the existing clipping */
    gdk_gc_set_function(ctxcanvas->gc, GDK_AND);
    gdk_gc_set_clip_region(ctxcanvas->gc, clip_polygon);

    cdwritemode(ctxcanvas, ctxcanvas->canvas->write_mode);  /* reset gdk_gc_set_function */

    pixbuf = cdgdkCreatePixbufMap(ew, eh, colors, dst_index, 0, 0, ew);
    if (!pixbuf)
      return;

    gdk_draw_pixbuf(ctxcanvas->wnd, ctxcanvas->gc, pixbuf, 0, 0, ex, ey, -1, -1, ctxcanvas->img_dither, 0, 0);

    /* reset clipping */
    gdk_region_destroy(clip_polygon);
    cdclip(ctxcanvas, ctxcanvas->canvas->clip_mode);

    g_object_unref(pixbuf);
  }

  free(dst_index);
}

static void cdputimagerectrgb(cdCtxCanvas *ctxcanvas, int iw, int ih, const unsigned char *r, const unsigned char *g, const unsigned char *b, int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax)
{
  int ew = w, eh = h, ex = x, ey = y;
  int bw = iw, bh = ih, bx = 0, by = 0;
  int rw, rh;
  GdkPixbuf* pixbuf;

  if (ctxcanvas->canvas->use_matrix)
  {
    cdputimagerectrgba_matrix(ctxcanvas, iw, ih, r, g, b, NULL, x, y, w, h, xmin, xmax, ymin, ymax);
    return;
  }

  rw = xmax-xmin+1;
  rh = ymax-ymin+1;
  y -= (h - 1);        /* GdkPixbuf origin is at top-left */

  if (!cdCalcZoom(ctxcanvas->canvas->w, x, w, &ex, &ew, xmin, rw, &bx, &bw, 1))
    return;
  
  if (!cdCalcZoom(ctxcanvas->canvas->h, y, h, &ey, &eh, ymin, rh, &by, &bh, 0))
    return;

  pixbuf = cdgdkCreatePixbufRGBA(bw, bh, r, g, b, NULL, bx, by, iw);
  if (!pixbuf)
    return;

  if (bw!=ew || bh!=eh)
  {
    GdkPixbuf *pixbuf_scaled = gdk_pixbuf_scale_simple(pixbuf, ew, eh, ctxcanvas->img_interp);
    gdk_draw_pixbuf(ctxcanvas->wnd, ctxcanvas->gc, pixbuf_scaled, 0, 0, ex, ey, -1, -1, ctxcanvas->img_dither, 0, 0);
    g_object_unref(pixbuf_scaled);
  }
  else
    gdk_draw_pixbuf(ctxcanvas->wnd, ctxcanvas->gc, pixbuf, 0, 0, ex, ey, -1, -1, ctxcanvas->img_dither, 0, 0);

  g_object_unref(pixbuf);
}

static void cdputimagerectrgba(cdCtxCanvas *ctxcanvas, int iw, int ih, const unsigned char *r, const unsigned char *g, const unsigned char *b, const unsigned char *a, int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax)
{
  GdkPixbuf *pixbuf;
  int ew = w, eh = h, ex = x, ey = y;
  int bw = iw, bh = ih, bx = 0, by = 0;
  int rw, rh;

  if (ctxcanvas->canvas->use_matrix)
  {
    cdputimagerectrgba_matrix(ctxcanvas, iw, ih, r, g, b, a, x, y, w, h, xmin, xmax, ymin, ymax);
    return;
  }

  rw = xmax-xmin+1;
  rh = ymax-ymin+1;
  y -= (h - 1);        /* GdkPixbuf origin is at top-left */

  if (!cdCalcZoom(ctxcanvas->canvas->w, x, w, &ex, &ew, xmin, rw, &bx, &bw, 1))
    return;
  
  if (!cdCalcZoom(ctxcanvas->canvas->h, y, h, &ey, &eh, ymin, rh, &by, &bh, 0))
    return;

  pixbuf = cdgdkCreatePixbufRGBA(bw, bh, r, g, b, a, bx, by, iw);
  if (!pixbuf)
    return;

  if (bw!=ew || bh!=eh)
  {
    GdkPixbuf *pixbuf_scaled = gdk_pixbuf_scale_simple(pixbuf, ew, eh, ctxcanvas->img_interp);
    gdk_draw_pixbuf(ctxcanvas->wnd, ctxcanvas->gc, pixbuf_scaled, 0, 0, ex, ey, -1, -1, ctxcanvas->img_dither, 0, 0);
    g_object_unref(pixbuf_scaled);
  }
  else
    gdk_draw_pixbuf(ctxcanvas->wnd, ctxcanvas->gc, pixbuf, 0, 0, ex, ey, -1, -1, ctxcanvas->img_dither, 0, 0);

  g_object_unref(pixbuf);
}

static void cdputimagerectmap(cdCtxCanvas *ctxcanvas, int iw, int ih, const unsigned char *index, const long int *colors, int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax)
{
  int ew = w, eh = h, ex = x, ey = y;
  int bw = iw, bh = ih, bx = 0, by = 0;
  int rw, rh;
  GdkPixbuf* pixbuf;

  if (ctxcanvas->canvas->use_matrix)
  {
    cdputimagerectmap_matrix(ctxcanvas, iw, ih, index, colors, x, y, w, h, xmin, xmax, ymin, ymax);
    return;
  }

  rw = xmax-xmin+1;
  rh = ymax-ymin+1;
  y -= (h - 1);        /* GdkPixbuf image origin is at top-left */

  if (!cdCalcZoom(ctxcanvas->canvas->w, x, w, &ex, &ew, xmin, rw, &bx, &bw, 1))
    return;
  
  if (!cdCalcZoom(ctxcanvas->canvas->h, y, h, &ey, &eh, ymin, rh, &by, &bh, 0))
    return;

  pixbuf = cdgdkCreatePixbufMap(bw, bh, colors, index, bx, by, iw);
  if (!pixbuf)
    return;

  if (bw!=ew || bh!=eh)
  {
    GdkPixbuf *pixbuf_scaled = gdk_pixbuf_scale_simple(pixbuf, ew, eh, ctxcanvas->img_interp);
    gdk_draw_pixbuf(ctxcanvas->wnd, ctxcanvas->gc, pixbuf_scaled, 0, 0, ex, ey, -1, -1, ctxcanvas->img_dither, 0, 0);
    g_object_unref(pixbuf_scaled);
  }
  else
    gdk_draw_pixbuf(ctxcanvas->wnd, ctxcanvas->gc, pixbuf, 0, 0, ex, ey, -1, -1, ctxcanvas->img_dither, 0, 0);

  g_object_unref(pixbuf);
}

static void cdpixel(cdCtxCanvas *ctxcanvas, int x, int y, long int color)
{
  if (ctxcanvas->canvas->foreground != color)
  {
    GdkColor clr = cdColorToGdk(color);
    gdk_gc_set_rgb_fg_color(ctxcanvas->gc, &clr);
  }

  if (ctxcanvas->canvas->use_matrix)
    cdMatrixTransformPoint(ctxcanvas->xmatrix, x, y, &x, &y);

  /* Draw pixel */
  gdk_draw_point(ctxcanvas->wnd, ctxcanvas->gc, x, y);

  if (ctxcanvas->canvas->foreground != color)
    gdk_gc_set_rgb_fg_color(ctxcanvas->gc, &ctxcanvas->fg);
}

static cdCtxImage *cdcreateimage (cdCtxCanvas *ctxcanvas, int w, int h)
{
  GdkGC* gc;
  cdCtxImage *ctximage = (cdCtxImage *)malloc(sizeof(cdCtxImage));
  GdkColor clr;

  ctximage->w = w;
  ctximage->h = h;
  ctximage->depth = ctxcanvas->depth;
  ctximage->scr   = ctxcanvas->scr;
  ctximage->vis   = ctxcanvas->vis;

  ctximage->img = gdk_pixmap_new(ctxcanvas->wnd, w, h, ctxcanvas->depth);

  if (!ctximage->img)
  {
    free(ctximage);
    return (void *)0;
  }

  gc = gdk_gc_new(ctximage->img);

  clr = cdColorToGdk(CD_WHITE);

  gdk_gc_set_rgb_fg_color(gc, &clr);
  gdk_draw_rectangle(ctximage->img, gc, TRUE, 0, 0, ctximage->w, ctxcanvas->canvas->h);

  g_object_unref(gc);

  return (void*)ctximage;
}

static void cdgetimage (cdCtxCanvas *ctxcanvas, cdCtxImage *ctximage, int x, int y)
{
  /* y is the bottom-left of the image in CD, must be at upper-left */
  y -= ctximage->h-1;

  gdk_draw_drawable(ctximage->img, ctxcanvas->gc,
                    ctxcanvas->wnd, x, y, 0, 0,
                    ctximage->w, ctximage->h);
}

static void cdputimagerect (cdCtxCanvas *ctxcanvas, cdCtxImage *ctximage, int x, int y, int xmin, int xmax, int ymin, int ymax)
{
  gdk_draw_drawable(ctxcanvas->wnd, ctxcanvas->gc,
                    ctximage->img, xmin, ctximage->h-ymax-1, x, y-(ymax-ymin+1)+1,
                    xmax-xmin+1, ymax-ymin+1);
}

static void cdkillimage (cdCtxImage *ctximage)
{
  g_object_unref(ctximage->img);
  free(ctximage);
}

static void cdscrollarea (cdCtxCanvas *ctxcanvas, int xmin, int xmax, int ymin, int ymax, int dx, int dy)
{
  gdk_draw_drawable(ctxcanvas->wnd, ctxcanvas->gc,
                    ctxcanvas->wnd, xmin, ymin, xmin+dx, ymin+dy,
                    xmax-xmin+1, ymax-ymin+1);
}

static void cdtransform(cdCtxCanvas *ctxcanvas, const double* matrix)
{
  if (matrix)
  {
    PangoMatrix tmpMtx = PANGO_MATRIX_INIT;

    /* configure a bottom-up coordinate system */
    ctxcanvas->xmatrix[0] = 1; 
    ctxcanvas->xmatrix[1] = 0;
    ctxcanvas->xmatrix[2] = 0; 
    ctxcanvas->xmatrix[3] = -1; 
    ctxcanvas->xmatrix[4] = 0; 
    ctxcanvas->xmatrix[5] = (ctxcanvas->canvas->h-1); 
    cdMatrixMultiply(matrix, ctxcanvas->xmatrix);

    /* Pango Matrix Transform */
    ctxcanvas->fontmatrix.xx = matrix[0] * tmpMtx.xx + matrix[1] * tmpMtx.xy;
    ctxcanvas->fontmatrix.xy = matrix[0] * tmpMtx.yx + matrix[1] * tmpMtx.yy;
    ctxcanvas->fontmatrix.yx = matrix[2] * tmpMtx.xx + matrix[3] * tmpMtx.xy;
    ctxcanvas->fontmatrix.yy = matrix[2] * tmpMtx.yx + matrix[3] * tmpMtx.yy;
    ctxcanvas->fontmatrix.x0 = 0;
    ctxcanvas->fontmatrix.y0 = 0;

    ctxcanvas->canvas->invert_yaxis = 0;
  }
  else
  {
    ctxcanvas->canvas->invert_yaxis = 1;
  }
}

/******************************************************************/

static void set_rotate_attrib(cdCtxCanvas* ctxcanvas, char* data)
{
  if (data)
  {
    /* use this configuration when there is NO native tranformation support */
    sscanf(data, "%g %d %d", &ctxcanvas->rotate_angle,
                             &ctxcanvas->rotate_center_x,
                             &ctxcanvas->rotate_center_y);

    cdCanvasTransformTranslate(ctxcanvas->canvas, ctxcanvas->rotate_center_x, ctxcanvas->rotate_center_y);
    cdCanvasTransformRotate(ctxcanvas->canvas, ctxcanvas->rotate_angle);
    cdCanvasTransformTranslate(ctxcanvas->canvas, -ctxcanvas->rotate_center_x, -ctxcanvas->rotate_center_y);
  }
  else
  {
    ctxcanvas->rotate_angle = 0;
    ctxcanvas->rotate_center_x = 0;
    ctxcanvas->rotate_center_y = 0;

    cdCanvasTransform(ctxcanvas->canvas, NULL);
  }
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

static void set_imgdither_attrib(cdCtxCanvas* ctxcanvas, char* data)
{
  if (data && cdStrEqualNoCase(data, "NORMAL"))
    ctxcanvas->img_dither = GDK_RGB_DITHER_NORMAL;
  else
    ctxcanvas->img_dither = GDK_RGB_DITHER_NONE;
}

static char* get_imgdither_attrib(cdCtxCanvas* ctxcanvas)
{
  if (ctxcanvas->img_dither)
    return "NORMAL";
  else
    return "NONE";
}

static cdAttribute imgdither_attrib =
{
  "IMGDITHER",
  set_imgdither_attrib,
  get_imgdither_attrib
}; 

static void set_interp_attrib(cdCtxCanvas* ctxcanvas, char* data)
{
  if (data && cdStrEqualNoCase(data, "BILINEAR"))
    ctxcanvas->img_interp = GDK_INTERP_BILINEAR;
  else
    ctxcanvas->img_interp = GDK_INTERP_NEAREST;
}

static char* get_interp_attrib(cdCtxCanvas* ctxcanvas)
{
  if (ctxcanvas->img_interp)
    return "BILINEAR";
  else
    return "NEAREST";
}

static cdAttribute interp_attrib =
{
  "IMGINTERP",
  set_interp_attrib,
  get_interp_attrib
}; 

static char* get_gc_attrib(cdCtxCanvas *ctxcanvas)
{
  return (char*)ctxcanvas->gc;
}

static cdAttribute gc_attrib =
{
  "GC",
  NULL,
  get_gc_attrib
}; 

static char* get_pangoversion_attrib(cdCtxCanvas* ctxcanvas)
{
  (void)ctxcanvas;
#if PANGO_VERSION_CHECK(1,16,0)
  return (char*)pango_version_string();
#else
  return "1.0";
#endif
}

static cdAttribute pangoversion_attrib =
{
  "PANGOVERSION",
  NULL,
  get_pangoversion_attrib
}; 

cdCtxCanvas *cdgdkCreateCanvas(cdCanvas* canvas, GdkDrawable* wnd, GdkScreen* scr, GdkVisual* vis)
{
  cdCtxCanvas *ctxcanvas = (cdCtxCanvas *)malloc(sizeof(cdCtxCanvas));
  memset(ctxcanvas, 0, sizeof(cdCtxCanvas));

  ctxcanvas->scr = scr;
  ctxcanvas->vis = vis;
  ctxcanvas->wnd = wnd;

  ctxcanvas->gc = gdk_gc_new(wnd);
  
  if (!ctxcanvas->gc) 
  {
    free(canvas);
    return NULL;
  }

  ctxcanvas->fontcontext = gdk_pango_context_get();
#if PANGO_VERSION_CHECK(1,16,0)
  pango_context_set_language(ctxcanvas->fontcontext, pango_language_get_default());
#endif

  ctxcanvas->canvas = canvas;
  canvas->ctxcanvas = ctxcanvas;
  
  gdk_drawable_get_size(wnd, &ctxcanvas->canvas->w, &ctxcanvas->canvas->h);
  ctxcanvas->depth = gdk_drawable_get_depth(wnd);

  canvas->bpp = ctxcanvas->depth;
  canvas->xres = ((double)gdk_screen_get_width(scr)  / (double)gdk_screen_get_width_mm(scr));
  canvas->yres = ((double)gdk_screen_get_height(scr) / (double)gdk_screen_get_height_mm(scr));
  canvas->w_mm = ((double)canvas->w) / canvas->xres;
  canvas->h_mm = ((double)canvas->h) / canvas->yres;
  canvas->invert_yaxis = 1;

  if (canvas->bpp <= 8)
  {
    ctxcanvas->colormap = gdk_gc_get_colormap(ctxcanvas->gc);
    if (!ctxcanvas->colormap)
    {
      ctxcanvas->colormap = gdk_colormap_get_system();
      gdk_gc_set_colormap(ctxcanvas->gc, ctxcanvas->colormap);
    }
    ctxcanvas->num_colors = ctxcanvas->colormap->size;
  }

  cdRegisterAttribute(canvas, &gc_attrib);
  cdRegisterAttribute(canvas, &rotate_attrib);
  cdRegisterAttribute(canvas, &pangoversion_attrib);
  cdRegisterAttribute(canvas, &imgdither_attrib);
  cdRegisterAttribute(canvas, &interp_attrib);

  return ctxcanvas;
}

void cdgdkInitTable(cdCanvas* canvas)
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
  canvas->cxChord  = cdSimChord;
  canvas->cxText   = cdtext;

  canvas->cxNewRegion = cdnewregion;
  canvas->cxIsPointInRegion = cdispointinregion;
  canvas->cxOffsetRegion = cdoffsetregion;
  canvas->cxGetRegionBox = cdgetregionbox;
  canvas->cxClip = cdclip;
  canvas->cxClipArea = cdcliparea;
  canvas->cxWriteMode = cdwritemode;
  canvas->cxLineStyle = cdlinestyle;
  canvas->cxLineWidth = cdlinewidth;
  canvas->cxLineCap = cdlinecap;
  canvas->cxLineJoin = cdlinejoin;
  canvas->cxBackOpacity = cdbackopacity;
  canvas->cxInteriorStyle = cdinteriorstyle;
  canvas->cxHatch = cdhatch;
  canvas->cxStipple = cdstipple;
  canvas->cxPattern = cdpattern;
  canvas->cxFont = cdfont;
  canvas->cxGetFontDim = cdgetfontdim;
  canvas->cxGetTextSize = cdgettextsize;
  canvas->cxPalette = cdpalette;
  canvas->cxBackground = cdbackground;
  canvas->cxForeground = cdforeground;
  canvas->cxTransform = cdtransform;
 
  canvas->cxScrollArea = cdscrollarea;
  canvas->cxCreateImage = cdcreateimage;
  canvas->cxGetImage = cdgetimage;
  canvas->cxPutImageRect = cdputimagerect;
  canvas->cxKillImage = cdkillimage;

  canvas->cxGetImageRGB = cdgetimagergb;
  canvas->cxPutImageRectRGB = cdputimagerectrgb;
  canvas->cxPutImageRectMap = cdputimagerectmap;
  canvas->cxPutImageRectRGBA = cdputimagerectrgba;
}

int cdBaseDriver(void)
{
  return CD_BASE_GDK;
}
