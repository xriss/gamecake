/** \file
 * \brief CD Picture driver
 *
 * See Copyright Notice in cd.h
 */

#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <limits.h> 
#include <math.h> 

#include "cd.h"
#include "cd_private.h"
#include "cdpicture.h"


/* codes for the primitives.
*/
typedef enum _tPrim
{
  CDPIC_LINE,
  CDPIC_RECT,
  CDPIC_BOX,
  CDPIC_ARC,
  CDPIC_SECTOR,
  CDPIC_CHORD,
  CDPIC_TEXT,
  CDPIC_POLY,
  CDPIC_PATH,
  CDPIC_FLINE,
  CDPIC_FRECT,
  CDPIC_FBOX,
  CDPIC_FARC,
  CDPIC_FSECTOR,
  CDPIC_FCHORD,
  CDPIC_FTEXT,
  CDPIC_FPOLY,
  CDPIC_FPATH,
  CDPIC_PIXEL,
  CDPIC_IMAGEMAP,
  CDPIC_IMAGERGB,
  CDPIC_IMAGERGBA,
} tPrim;

typedef struct _tFillAttrib
{
  long foreground, background;
  int back_opacity;
  int interior_style, hatch_style;
  int fill_mode;
  int pattern_w, pattern_h;
  long* pattern;
  int stipple_w, stipple_h;
  unsigned char* stipple;
} tFillAttrib;

typedef struct _tLineAttrib
{
  long foreground, background;
  int back_opacity;
  int line_style, line_width;
  int line_cap, line_join;
  int* line_dashes;
  int line_dashes_count;
} tLineAttrib;

typedef struct _tTextAttrib
{
  long foreground;
  char* font_type_face;
  int font_style, font_size;
  int text_alignment;
  double text_orientation;
  char* native_font;
} tTextAttrib;

typedef struct _tLBR
{
  int x1, y1, x2, y2;
} tLBR;       /* Line or Box or Rect */

typedef struct _tfLBR
{
  double x1, y1, x2, y2;
} tfLBR;       /* Line or Box or Rect */

typedef struct _tASC
{
  int xc, yc, w, h;
  double angle1, angle2;
} tASC;       /* Arc or Sector or Chord */

typedef struct _tfASC
{
  double xc, yc, w, h;
  double angle1, angle2;
} tfASC;       /* Arc or Sector or Chord */

typedef struct _tPoly
{
  int mode;
  int n;
  cdPoint* points;
} tPoly;     /* Begin, Vertex and End */

typedef struct _tfPoly
{
  int mode;
  int n;
  cdfPoint* points;
} tfPoly;     /* Begin, Vertex and End */

typedef struct _tPath
{
  int fill;
  int n;
  cdPoint* points;
  int path_n;
  int *path;
} tPath;     /* Begin, PathSet, Vertex and End */

typedef struct _tfPath
{
  int fill;
  int n;
  cdfPoint* points;
  int path_n;
  int *path;
} tfPath;     /* Begin, PathSet, Vertex and End */

typedef struct _tText
{
  int x, y;
  char *s;
} tText;     /* Text */

typedef struct _tfText
{
  double x, y;
  char *s;
} tfText;     /* Text */

typedef struct _tPixel
{
  int x, y;
  long color;
} tPixel;    /* Pixel */

typedef struct _tImageMap
{
  int iw, ih;
  unsigned char *index;
  long int *colors;
  int x, y, w, h;
} tImageMap;

typedef struct _tImageRGBA
{
  int iw, ih;
  unsigned char *r;
  unsigned char *g;
  unsigned char *b;
  unsigned char *a;
  int x, y, w, h;
} tImageRGBA;

typedef struct _tPrimNode 
{
  tPrim type;
  void* param_buffer; /* dinamically allocated memory for the parameter */
  union {
    tLBR lineboxrect;
    tfLBR lineboxrectf;
    tASC arcsectorchord;
    tfASC arcsectorchordf;
    tPoly poly;
    tfPoly polyf;
    tPath path;
    tfPath pathf;
    tText text;
    tfText textf;
    tPixel pixel;
    tImageMap imagemap;
    tImageRGBA imagergba;
  } param;
  void* attrib_buffer; /* dinamically allocated memory for the attributes */
  union {
    tLineAttrib line;
    tFillAttrib fill;
    tTextAttrib text;
  } attrib;
  struct _tPrimNode *next;
} tPrimNode;

struct _cdCtxCanvas 
{
  cdCanvas* canvas;

  /* primitives list */
  tPrimNode *prim_first,
            *prim_last;
  int prim_n;

  /* bounding box */
  int xmin, xmax,
      ymin, ymax;
};

static void picUpdateSize(cdCtxCanvas *ctxcanvas)
{
  ctxcanvas->canvas->w = ctxcanvas->xmax-ctxcanvas->xmin+1;
  ctxcanvas->canvas->h = ctxcanvas->ymax-ctxcanvas->ymin+1;
  ctxcanvas->canvas->w_mm = ((double)ctxcanvas->canvas->w) / ctxcanvas->canvas->xres;
  ctxcanvas->canvas->h_mm = ((double)ctxcanvas->canvas->h) / ctxcanvas->canvas->yres;
}

static void picUpdateBBox(cdCtxCanvas *ctxcanvas, int x, int y, int ew)
{
  if (x+ew > ctxcanvas->xmax)
    ctxcanvas->xmax = x+ew;
  if (y+ew > ctxcanvas->ymax)
    ctxcanvas->ymax = y+ew;
  if (x-ew < ctxcanvas->xmin)
    ctxcanvas->xmin = x-ew;
  if (y-ew < ctxcanvas->ymin)
    ctxcanvas->ymin = y-ew;

  picUpdateSize(ctxcanvas);
}

static void picUpdateBBoxF(cdCtxCanvas *ctxcanvas, double x, double y, int ew)
{
  if ((int)ceil(x+ew) > ctxcanvas->xmax)
    ctxcanvas->xmax = (int)ceil(x+ew);
  if ((int)ceil(y+ew) > ctxcanvas->ymax)
    ctxcanvas->ymax = (int)ceil(y+ew);
  if ((int)floor(x-ew) < ctxcanvas->xmin)
    ctxcanvas->xmin = (int)floor(x-ew);
  if ((int)floor(y-ew) < ctxcanvas->ymin)
    ctxcanvas->ymin = (int)floor(y-ew);

  picUpdateSize(ctxcanvas);
}

static void picAddPrim(cdCtxCanvas *ctxcanvas, tPrimNode *prim)
{
  if (ctxcanvas->prim_n == 0)
    ctxcanvas->prim_first = prim;
  else
    ctxcanvas->prim_last->next = prim;

  ctxcanvas->prim_last = prim;
  ctxcanvas->prim_n++;
}

static tPrimNode* primCreate(tPrim type)
{
  tPrimNode *prim = malloc(sizeof(tPrimNode));
  memset(prim, 0, sizeof(tPrimNode));
  prim->type = type;
  return prim;
}

static void primDestroy(tPrimNode *prim)
{
  if (prim->param_buffer)
    free(prim->param_buffer);
  if (prim->attrib_buffer)
    free(prim->attrib_buffer);
  free(prim);
}

static void primAddAttrib_Line(tPrimNode *prim, cdCanvas *canvas)
{
  prim->attrib.line.foreground = canvas->foreground; 
  prim->attrib.line.background = canvas->background;
  prim->attrib.line.back_opacity = canvas->back_opacity;
  prim->attrib.line.line_style = canvas->line_style; 
  prim->attrib.line.line_width = canvas->line_width;
  prim->attrib.line.line_cap = canvas->line_cap; 
  prim->attrib.line.line_join = canvas->line_join;

  if (canvas->line_style==CD_CUSTOM && canvas->line_dashes)
  {
    prim->attrib.line.line_dashes_count = canvas->line_dashes_count;
    prim->attrib_buffer = malloc(canvas->line_dashes_count*sizeof(int));
    prim->attrib.line.line_dashes = prim->attrib_buffer;
    memcpy(prim->attrib.line.line_dashes, canvas->line_dashes, canvas->line_dashes_count*sizeof(int));
  }
}

static void primAddAttrib_Fill(tPrimNode *prim, cdCanvas *canvas)
{
  prim->attrib.fill.foreground = canvas->foreground; 
  prim->attrib.fill.background = canvas->background;
  prim->attrib.fill.back_opacity = canvas->back_opacity;
  prim->attrib.fill.interior_style = canvas->interior_style; 
  prim->attrib.fill.hatch_style = canvas->hatch_style;
  prim->attrib.fill.fill_mode = canvas->fill_mode; 
  prim->attrib.fill.pattern_w = canvas->pattern_w;
  prim->attrib.fill.pattern_h = canvas->pattern_h;
  prim->attrib.fill.stipple_w = canvas->stipple_w;
  prim->attrib.fill.stipple_h = canvas->stipple_h;

  if (canvas->interior_style==CD_PATTERN && canvas->pattern)
  {
    prim->attrib_buffer = malloc(canvas->pattern_size*sizeof(long));
    prim->attrib.fill.pattern = prim->attrib_buffer;
    memcpy(prim->attrib.fill.pattern, canvas->pattern, canvas->pattern_size*sizeof(long));
  }

  if (canvas->interior_style==CD_STIPPLE && canvas->stipple)
  {
    prim->attrib_buffer = malloc(canvas->stipple_size*sizeof(long));
    prim->attrib.fill.stipple = prim->attrib_buffer;
    memcpy(prim->attrib.fill.stipple, canvas->stipple, canvas->stipple_size*sizeof(long));
  }
}

static void primAddAttrib_Text(tPrimNode *prim, cdCanvas *canvas)
{
  prim->attrib.text.foreground = canvas->foreground; 

  prim->attrib.text.font_style = canvas->font_style;
  prim->attrib.text.font_size = canvas->font_size;
  prim->attrib.text.text_alignment = canvas->text_alignment; 
  prim->attrib.text.text_orientation = canvas->text_orientation;

  if (canvas->native_font[0])
  {
    prim->attrib_buffer = cdStrDup(canvas->native_font);
    prim->attrib.text.native_font = prim->attrib_buffer;
  }
  else
  {
    prim->attrib_buffer = cdStrDup(canvas->font_type_face);
    prim->attrib.text.font_type_face = prim->attrib_buffer;
  }
}

static void primUpdateAttrib_Line(tPrimNode *prim, cdCanvas *canvas)
{
  cdCanvasSetBackground(canvas, prim->attrib.line.background);
  cdCanvasSetForeground(canvas, prim->attrib.line.foreground);
  cdCanvasBackOpacity(canvas, prim->attrib.line.back_opacity);
  cdCanvasLineStyle(canvas, prim->attrib.line.line_style); 
  cdCanvasLineWidth(canvas, prim->attrib.line.line_width);
  cdCanvasLineCap(canvas, prim->attrib.line.line_cap);
  cdCanvasLineJoin(canvas, prim->attrib.line.line_join);

  if (prim->attrib.line.line_style==CD_CUSTOM && prim->attrib.line.line_dashes)
    cdCanvasLineStyleDashes(canvas, prim->attrib.line.line_dashes, prim->attrib.line.line_dashes_count);
}

void primUpdateAttrib_Fill(tPrimNode *prim, cdCanvas *canvas)
{
  cdCanvasSetBackground(canvas, prim->attrib.fill.background);
  cdCanvasSetForeground(canvas, prim->attrib.fill.foreground);
  cdCanvasBackOpacity(canvas, prim->attrib.fill.back_opacity);
  cdCanvasFillMode(canvas, prim->attrib.fill.fill_mode);

  if (prim->attrib.fill.interior_style==CD_HATCH)
    cdCanvasHatch(canvas, prim->attrib.fill.hatch_style);
  else if (prim->attrib.fill.interior_style==CD_PATTERN && prim->attrib.fill.pattern)
    cdCanvasPattern(canvas, prim->attrib.fill.pattern_w, prim->attrib.fill.pattern_h, prim->attrib.fill.pattern);
  else if (prim->attrib.fill.interior_style==CD_STIPPLE && prim->attrib.fill.stipple)
    cdCanvasStipple(canvas, prim->attrib.fill.stipple_w, prim->attrib.fill.stipple_h, prim->attrib.fill.stipple);

  cdCanvasInteriorStyle(canvas, prim->attrib.fill.interior_style);
}

void primUpdateAttrib_Text(tPrimNode *prim, cdCanvas *canvas)
{
  cdCanvasSetForeground(canvas, prim->attrib.text.foreground);
  cdCanvasTextAlignment(canvas, prim->attrib.text.text_alignment);
  cdCanvasTextOrientation(canvas, prim->attrib.text.text_orientation);

  if (canvas->native_font[0])
    cdCanvasNativeFont(canvas, prim->attrib.text.native_font);
  else
    cdCanvasFont(canvas, prim->attrib.text.font_type_face, prim->attrib.text.font_style, prim->attrib.text.font_size);
}

static int cdfont(cdCtxCanvas *ctxcanvas, const char *type_face, int style, int size)
{
  (void)ctxcanvas;
  (void)type_face;
  (void)style;
  (void)size;
  return 1;
}

static void cdclear(cdCtxCanvas *ctxcanvas)
{
  tPrimNode *prim;
  int i;
  for (i = 0; i < ctxcanvas->prim_n; i++)
  {
    prim = ctxcanvas->prim_first;
    ctxcanvas->prim_first = prim->next;

    primDestroy(prim);
  }

  ctxcanvas->prim_n = 0;
  ctxcanvas->prim_first = NULL;
  ctxcanvas->prim_last = NULL;
}

static void cdpixel(cdCtxCanvas *ctxcanvas, int x, int y, long int color)
{
  tPrimNode *prim = primCreate(CDPIC_PIXEL);
  prim->param.pixel.x = x;
  prim->param.pixel.y = y;
  prim->param.pixel.color = color;
  picAddPrim(ctxcanvas, prim);
  picUpdateBBox(ctxcanvas, x, y, 0);
}

static void cdline(cdCtxCanvas *ctxcanvas, int x1, int y1, int x2, int y2)
{
  tPrimNode *prim = primCreate(CDPIC_LINE);
  primAddAttrib_Line(prim, ctxcanvas->canvas);
  prim->param.lineboxrect.x1 = x1;
  prim->param.lineboxrect.y1 = y1;
  prim->param.lineboxrect.x2 = x2;
  prim->param.lineboxrect.y2 = y2;
  picAddPrim(ctxcanvas, prim);
  picUpdateBBox(ctxcanvas, x1, y1, ctxcanvas->canvas->line_width);
  picUpdateBBox(ctxcanvas, x2, y2, ctxcanvas->canvas->line_width);
}

static void cdfline(cdCtxCanvas *ctxcanvas, double x1, double y1, double x2, double y2)
{
  tPrimNode *prim = primCreate(CDPIC_FLINE);
  primAddAttrib_Line(prim, ctxcanvas->canvas);
  prim->param.lineboxrectf.x1 = x1;
  prim->param.lineboxrectf.y1 = y1;
  prim->param.lineboxrectf.x2 = x2;
  prim->param.lineboxrectf.y2 = y2;
  picAddPrim(ctxcanvas, prim);
  picUpdateBBoxF(ctxcanvas, x1, y1, ctxcanvas->canvas->line_width);
  picUpdateBBoxF(ctxcanvas, x2, y2, ctxcanvas->canvas->line_width);
}

static void cdrect(cdCtxCanvas *ctxcanvas, int xmin, int xmax, int ymin, int ymax)
{
  tPrimNode *prim = primCreate(CDPIC_RECT);
  primAddAttrib_Line(prim, ctxcanvas->canvas);
  prim->param.lineboxrect.x1 = xmin;
  prim->param.lineboxrect.y1 = ymin;
  prim->param.lineboxrect.x2 = xmax;
  prim->param.lineboxrect.y2 = ymax;
  picAddPrim(ctxcanvas, prim);
  picUpdateBBox(ctxcanvas, xmin, ymin, ctxcanvas->canvas->line_width);
  picUpdateBBox(ctxcanvas, xmax, ymax, ctxcanvas->canvas->line_width);
}

static void cdfrect(cdCtxCanvas *ctxcanvas, double xmin, double xmax, double ymin, double ymax)
{
  tPrimNode *prim = primCreate(CDPIC_FRECT);
  primAddAttrib_Line(prim, ctxcanvas->canvas);
  prim->param.lineboxrectf.x1 = xmin;
  prim->param.lineboxrectf.y1 = ymin;
  prim->param.lineboxrectf.x2 = xmax;
  prim->param.lineboxrectf.y2 = ymax;
  picAddPrim(ctxcanvas, prim);
  picUpdateBBoxF(ctxcanvas, xmin, ymin, ctxcanvas->canvas->line_width);
  picUpdateBBoxF(ctxcanvas, xmax, ymax, ctxcanvas->canvas->line_width);
}

static void cdbox(cdCtxCanvas *ctxcanvas, int xmin, int xmax, int ymin, int ymax)
{
  tPrimNode *prim = primCreate(CDPIC_BOX);
  primAddAttrib_Fill(prim, ctxcanvas->canvas);
  prim->param.lineboxrect.x1 = xmin;
  prim->param.lineboxrect.y1 = ymin;
  prim->param.lineboxrect.x2 = xmax;
  prim->param.lineboxrect.y2 = ymax;
  picAddPrim(ctxcanvas, prim);
  picUpdateBBox(ctxcanvas, xmin, ymin, 0);
  picUpdateBBox(ctxcanvas, xmax, ymax, 0);
}

static void cdfbox(cdCtxCanvas *ctxcanvas, double xmin, double xmax, double ymin, double ymax)
{
  tPrimNode *prim = primCreate(CDPIC_FBOX);
  primAddAttrib_Fill(prim, ctxcanvas->canvas);
  prim->param.lineboxrectf.x1 = xmin;
  prim->param.lineboxrectf.y1 = ymin;
  prim->param.lineboxrectf.x2 = xmax;
  prim->param.lineboxrectf.y2 = ymax;
  picAddPrim(ctxcanvas, prim);
  picUpdateBBoxF(ctxcanvas, xmin, ymin, 0);
  picUpdateBBoxF(ctxcanvas, xmax, ymax, 0);
}

static void cdarc(cdCtxCanvas *ctxcanvas, int xc, int yc, int w, int h, double a1, double a2)
{
  int xmin, xmax, ymin, ymax;
  tPrimNode *prim = primCreate(CDPIC_ARC);
  primAddAttrib_Line(prim, ctxcanvas->canvas);
  prim->param.arcsectorchord.xc = xc;
  prim->param.arcsectorchord.yc = yc;
  prim->param.arcsectorchord.w = w;
  prim->param.arcsectorchord.h = h;
  prim->param.arcsectorchord.angle1 = a1;
  prim->param.arcsectorchord.angle2 = a2;
  picAddPrim(ctxcanvas, prim);
  cdCanvasGetArcBox(xc, yc, w, h, a1, a2, &xmin, &xmax, &ymin, &ymax);
  picUpdateBBox(ctxcanvas, xmin, ymin, ctxcanvas->canvas->line_width);
  picUpdateBBox(ctxcanvas, xmax, ymax, ctxcanvas->canvas->line_width);
}

static void cdfarc(cdCtxCanvas *ctxcanvas, double xc, double yc, double w, double h, double a1, double a2)
{
  int xmin, xmax, ymin, ymax;
  tPrimNode *prim = primCreate(CDPIC_FARC);
  primAddAttrib_Line(prim, ctxcanvas->canvas);
  prim->param.arcsectorchordf.xc = xc;
  prim->param.arcsectorchordf.yc = yc;
  prim->param.arcsectorchordf.w = w;
  prim->param.arcsectorchordf.h = h;
  prim->param.arcsectorchordf.angle1 = a1;
  prim->param.arcsectorchordf.angle2 = a2;
  picAddPrim(ctxcanvas, prim);
  cdCanvasGetArcBox(_cdRound(xc), _cdRound(yc), _cdRound(w), _cdRound(h), a1, a2, &xmin, &xmax, &ymin, &ymax);
  picUpdateBBox(ctxcanvas, xmin, ymin, ctxcanvas->canvas->line_width);
  picUpdateBBox(ctxcanvas, xmax, ymax, ctxcanvas->canvas->line_width);
}

static void cdsector(cdCtxCanvas *ctxcanvas, int xc, int yc, int w, int h, double a1, double a2)
{
  int xmin, xmax, ymin, ymax;
  tPrimNode *prim = primCreate(CDPIC_SECTOR);
  primAddAttrib_Fill(prim, ctxcanvas->canvas);
  prim->param.arcsectorchord.xc = xc;
  prim->param.arcsectorchord.yc = yc;
  prim->param.arcsectorchord.w = w;
  prim->param.arcsectorchord.h = h;
  prim->param.arcsectorchord.angle1 = a1;
  prim->param.arcsectorchord.angle2 = a2;
  picAddPrim(ctxcanvas, prim);
  cdCanvasGetArcBox(xc, yc, w, h, a1, a2, &xmin, &xmax, &ymin, &ymax);
  picUpdateBBox(ctxcanvas, xmin, ymin, 0);
  picUpdateBBox(ctxcanvas, xmax, ymax, 0);
  picUpdateBBox(ctxcanvas, xc, yc, 0);
}

static void cdfsector(cdCtxCanvas *ctxcanvas, double xc, double yc, double w, double h, double a1, double a2)
{
  int xmin, xmax, ymin, ymax;
  tPrimNode *prim = primCreate(CDPIC_FSECTOR);
  primAddAttrib_Fill(prim, ctxcanvas->canvas);
  prim->param.arcsectorchordf.xc = xc;
  prim->param.arcsectorchordf.yc = yc;
  prim->param.arcsectorchordf.w = w;
  prim->param.arcsectorchordf.h = h;
  prim->param.arcsectorchordf.angle1 = a1;
  prim->param.arcsectorchordf.angle2 = a2;
  picAddPrim(ctxcanvas, prim);
  cdCanvasGetArcBox(_cdRound(xc), _cdRound(yc), _cdRound(w), _cdRound(h), a1, a2, &xmin, &xmax, &ymin, &ymax);
  picUpdateBBox(ctxcanvas, xmin, ymin, 0);
  picUpdateBBox(ctxcanvas, xmax, ymax, 0);
  picUpdateBBox(ctxcanvas, _cdRound(xc), _cdRound(yc), 0);
}

static void cdchord(cdCtxCanvas *ctxcanvas, int xc, int yc, int w, int h, double a1, double a2)
{
  int xmin, xmax, ymin, ymax;
  tPrimNode *prim = primCreate(CDPIC_CHORD);
  primAddAttrib_Fill(prim, ctxcanvas->canvas);
  prim->param.arcsectorchord.xc = xc;
  prim->param.arcsectorchord.yc = yc;
  prim->param.arcsectorchord.w = w;
  prim->param.arcsectorchord.h = h;
  prim->param.arcsectorchord.angle1 = a1;
  prim->param.arcsectorchord.angle2 = a2;
  picAddPrim(ctxcanvas, prim);
  cdCanvasGetArcBox(xc, yc, w, h, a1, a2, &xmin, &xmax, &ymin, &ymax);
  picUpdateBBox(ctxcanvas, xmin, ymin, 0);
  picUpdateBBox(ctxcanvas, xmax, ymax, 0);
}

static void cdfchord(cdCtxCanvas *ctxcanvas, double xc, double yc, double w, double h, double a1, double a2)
{
  int xmin, xmax, ymin, ymax;
  tPrimNode *prim = primCreate(CDPIC_FCHORD);
  primAddAttrib_Fill(prim, ctxcanvas->canvas);
  prim->param.arcsectorchordf.xc = xc;
  prim->param.arcsectorchordf.yc = yc;
  prim->param.arcsectorchordf.w = w;
  prim->param.arcsectorchordf.h = h;
  prim->param.arcsectorchordf.angle1 = a1;
  prim->param.arcsectorchordf.angle2 = a2;
  picAddPrim(ctxcanvas, prim);
  cdCanvasGetArcBox(_cdRound(xc), _cdRound(yc), _cdRound(w), _cdRound(h), a1, a2, &xmin, &xmax, &ymin, &ymax);
  picUpdateBBox(ctxcanvas, xmin, ymin, 0);
  picUpdateBBox(ctxcanvas, xmax, ymax, 0);
}

static void cdtext(cdCtxCanvas *ctxcanvas, int x, int y, const char *text, int len)
{
  int xmin, xmax, ymin, ymax;
  tPrimNode *prim = primCreate(CDPIC_TEXT);
  primAddAttrib_Text(prim, ctxcanvas->canvas);
  prim->param.text.x = x;
  prim->param.text.y = y;
  prim->param.text.s = cdStrDupN(text, len);
  prim->param_buffer = prim->param.text.s;
  picAddPrim(ctxcanvas, prim);
  cdCanvasGetTextBox(ctxcanvas->canvas, x, y, prim->param.text.s, &xmin, &xmax, &ymin, &ymax);
  picUpdateBBox(ctxcanvas, xmin, ymin, 0);
  picUpdateBBox(ctxcanvas, xmax, ymax, 0);
}

static void cdftext(cdCtxCanvas *ctxcanvas, double x, double y, const char *text, int len)
{
  int xmin, xmax, ymin, ymax;
  tPrimNode *prim = primCreate(CDPIC_FTEXT);
  primAddAttrib_Text(prim, ctxcanvas->canvas);
  prim->param.textf.x = x;
  prim->param.textf.y = y;
  prim->param.textf.s = cdStrDupN(text, len);
  prim->param_buffer = prim->param.textf.s;
  picAddPrim(ctxcanvas, prim);
  cdCanvasGetTextBox(ctxcanvas->canvas, _cdRound(x), _cdRound(y), prim->param.text.s, &xmin, &xmax, &ymin, &ymax);
  picUpdateBBox(ctxcanvas, xmin, ymin, 0);
  picUpdateBBox(ctxcanvas, xmax, ymax, 0);
}

static void cdpath(cdCtxCanvas *ctxcanvas, cdPoint* poly, int n)
{
  int i, p, fill = -1;
  tPrimNode *prim;

  for (p=0; p<ctxcanvas->canvas->path_n; p++)
  {
    if (ctxcanvas->canvas->path[p] == CD_PATH_CLIP)
      return;
    else if (ctxcanvas->canvas->path[p] == CD_PATH_FILL ||
             ctxcanvas->canvas->path[p] == CD_PATH_FILLSTROKE)  /* no support for both in cdPicture */
    {
      fill = 1;
      break;
    }
    else if (ctxcanvas->canvas->path[p] == CD_PATH_STROKE)
    {
      fill = -1;
      break;
    }
  }

  if (fill == -1)
    return;

  prim = primCreate(CDPIC_PATH);
  prim->param.path.fill = fill;

  if (fill)
    primAddAttrib_Fill(prim, ctxcanvas->canvas);
  else
    primAddAttrib_Line(prim, ctxcanvas->canvas);

  prim->param_buffer = malloc(n * sizeof(cdPoint) + ctxcanvas->canvas->path_n * sizeof(int));

  prim->param.path.n = n;
  prim->param.path.points = (cdPoint*)prim->param_buffer;
  memcpy(prim->param.path.points, poly, n * sizeof(cdPoint));
  prim->param.path.path = (int*)((unsigned char*)prim->param_buffer + n * sizeof(cdPoint));
  memcpy(prim->param.path.path, ctxcanvas->canvas->path, ctxcanvas->canvas->path_n * sizeof(int));
  prim->param.path.path_n = ctxcanvas->canvas->path_n;
  
  picAddPrim(ctxcanvas, prim);

  for (i = 0; i < n; i++)
  {
    picUpdateBBox(ctxcanvas, poly[i].x, poly[i].y, 0);
  }
}

static void cdpoly(cdCtxCanvas *ctxcanvas, int mode, cdPoint* poly, int n)
{
  int i;
  tPrimNode *prim;
  if (mode == CD_CLIP || mode == CD_REGION) return;
  if (mode == CD_PATH)
  {
    cdpath(ctxcanvas, poly, n);
    return;
  }
  prim = primCreate(CDPIC_POLY);
  if (mode == CD_FILL)
    primAddAttrib_Fill(prim, ctxcanvas->canvas);
  else
    primAddAttrib_Line(prim, ctxcanvas->canvas);
  prim->param.poly.mode = mode;
  prim->param.poly.n = n;
  prim->param.poly.points = malloc(n * sizeof(cdPoint));
  memcpy(prim->param.poly.points, poly, n * sizeof(cdPoint));
  prim->param_buffer = prim->param.poly.points;
  picAddPrim(ctxcanvas, prim);

  for (i = 0; i < n; i++)
  {
    if (mode == CD_FILL)
      picUpdateBBox(ctxcanvas, poly[i].x, poly[i].y, 0);
    else
      picUpdateBBox(ctxcanvas, poly[i].x, poly[i].y, ctxcanvas->canvas->line_width);
  }
}

static void cdfpath(cdCtxCanvas *ctxcanvas, cdfPoint* poly, int n)
{
  int i, p, fill = -1;
  tPrimNode *prim;

  for (p=0; p<ctxcanvas->canvas->path_n; p++)
  {
    if (ctxcanvas->canvas->path[p] == CD_PATH_CLIP)
      return;
    else if (ctxcanvas->canvas->path[p] == CD_PATH_FILL ||
             ctxcanvas->canvas->path[p] == CD_PATH_FILLSTROKE)  /* no support for both in cdPicture */
    {
      fill = 1;
      break;
    }
    else if (ctxcanvas->canvas->path[p] == CD_PATH_STROKE)
    {
      fill = -1;
      break;
    }
  }

  if (fill == -1)
    return;

  prim = primCreate(CDPIC_FPATH);
  prim->param.pathf.fill = fill;

  if (fill)
    primAddAttrib_Fill(prim, ctxcanvas->canvas);
  else
    primAddAttrib_Line(prim, ctxcanvas->canvas);

  prim->param_buffer = malloc(n * sizeof(cdfPoint) + ctxcanvas->canvas->path_n * sizeof(int));

  prim->param.pathf.n = n;
  prim->param.pathf.points = (cdfPoint*)prim->param_buffer;
  memcpy(prim->param.pathf.points, poly, n * sizeof(cdfPoint));
  prim->param.pathf.path = (int*)((unsigned char*)prim->param_buffer + n * sizeof(cdfPoint));
  memcpy(prim->param.pathf.path, ctxcanvas->canvas->path, ctxcanvas->canvas->path_n * sizeof(int));
  prim->param.pathf.path_n = ctxcanvas->canvas->path_n;
  
  picAddPrim(ctxcanvas, prim);

  for (i = 0; i < n; i++)
  {
    picUpdateBBox(ctxcanvas, _cdRound(poly[i].x), _cdRound(poly[i].y), 0);
  }
}

static void cdfpoly(cdCtxCanvas *ctxcanvas, int mode, cdfPoint* poly, int n)
{
  int i;
  tPrimNode *prim;
  if (mode == CD_CLIP || mode == CD_REGION) return;
  if (mode == CD_PATH)
  {
    cdfpath(ctxcanvas, poly, n);
    return;
  }
  prim = primCreate(CDPIC_FPOLY);
  if (mode == CD_FILL)
    primAddAttrib_Fill(prim, ctxcanvas->canvas);
  else
    primAddAttrib_Line(prim, ctxcanvas->canvas);
  prim->param.polyf.mode = mode;
  prim->param.polyf.n = n;
  prim->param.polyf.points = malloc(n * sizeof(cdfPoint));
  memcpy(prim->param.polyf.points, poly, n * sizeof(cdfPoint));
  prim->param_buffer = prim->param.polyf.points;
  picAddPrim(ctxcanvas, prim);

  for (i = 0; i < n; i++)
  {
    if (mode == CD_FILL)
      picUpdateBBox(ctxcanvas, _cdRound(poly[i].x), _cdRound(poly[i].y), 0);
    else
      picUpdateBBox(ctxcanvas, _cdRound(poly[i].x), _cdRound(poly[i].y), ctxcanvas->canvas->line_width);
  }
}

static void cdputimagerectrgb(cdCtxCanvas *ctxcanvas, int iw, int ih, const unsigned char *r, const unsigned char *g, const unsigned char *b, int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax)
{
  int l, offset, size;
  unsigned char *dr, *dg, *db;

  tPrimNode *prim = primCreate(CDPIC_IMAGERGB);
  prim->param.imagergba.iw = xmax-xmin+1;
  prim->param.imagergba.ih = ymax-ymin+1;
  prim->param.imagergba.x = x;
  prim->param.imagergba.y = y;
  prim->param.imagergba.w = w;
  prim->param.imagergba.h = h;

  size = prim->param.imagergba.iw*prim->param.imagergba.ih;
  prim->param_buffer = malloc(3*size);
  prim->param.imagergba.r = prim->param_buffer;
  prim->param.imagergba.g = prim->param.imagergba.r + size;
  prim->param.imagergba.b = prim->param.imagergba.g + size;

  offset = ymin*iw + xmin;
  r += offset;
  g += offset;
  b += offset;

  dr = prim->param.imagergba.r;
  dg = prim->param.imagergba.g;
  db = prim->param.imagergba.b;
  offset = prim->param.imagergba.iw;

  for (l = ymin; l <= ymax; l++)
  {
    memcpy(dr, r, offset);
    memcpy(dg, g, offset);
    memcpy(db, b, offset);

    r += iw;
    g += iw;
    b += iw;
    dr += offset;
    dg += offset;
    db += offset;
  }

  picUpdateBBox(ctxcanvas, x, y, 0);
  picUpdateBBox(ctxcanvas, x+prim->param.imagergba.iw-1, y+prim->param.imagergba.ih-1, 0);
}

static void cdputimagerectrgba(cdCtxCanvas *ctxcanvas, int iw, int ih, const unsigned char *r, const unsigned char *g, const unsigned char *b, const unsigned char *a, int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax)
{
  int l, offset, size;
  unsigned char *dr, *dg, *db, *da;

  tPrimNode *prim = primCreate(CDPIC_IMAGERGBA);
  prim->param.imagergba.iw = xmax-xmin+1;
  prim->param.imagergba.ih = ymax-ymin+1;
  prim->param.imagergba.x = x;
  prim->param.imagergba.y = y;
  prim->param.imagergba.w = w;
  prim->param.imagergba.h = h;

  size = prim->param.imagergba.iw*prim->param.imagergba.ih;
  prim->param_buffer = malloc(4*size);
  prim->param.imagergba.r = prim->param_buffer;
  prim->param.imagergba.g = prim->param.imagergba.r + size;
  prim->param.imagergba.b = prim->param.imagergba.g + size;
  prim->param.imagergba.a = prim->param.imagergba.b + size;

  offset = ymin*iw + xmin;
  r += offset;
  g += offset;
  b += offset;
  a += offset;

  dr = prim->param.imagergba.r;
  dg = prim->param.imagergba.g;
  db = prim->param.imagergba.b;
  da = prim->param.imagergba.a;
  offset = prim->param.imagergba.iw;

  for (l = ymin; l <= ymax; l++)
  {
    memcpy(dr, r, offset);
    memcpy(dg, g, offset);
    memcpy(db, b, offset);
    memcpy(da, a, offset);

    r += iw;
    g += iw;
    b += iw;
    a += iw;
    dr += offset;
    dg += offset;
    db += offset;
    da += offset;
  }

  picUpdateBBox(ctxcanvas, x, y, 0);
  picUpdateBBox(ctxcanvas, x+prim->param.imagergba.iw-1, y+prim->param.imagergba.ih-1, 0);
}

static void cdputimagerectmap(cdCtxCanvas *ctxcanvas, int iw, int ih, const unsigned char *index, const long int *colors, int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax)
{
  int c, l, n = 0, offset, size;
  unsigned char *dindex;
  long *dcolors;

  tPrimNode *prim = primCreate(CDPIC_IMAGEMAP);
  prim->param.imagemap.iw = xmax-xmin+1;
  prim->param.imagemap.ih = ymax-ymin+1;
  prim->param.imagemap.x = x;
  prim->param.imagemap.y = y;
  prim->param.imagemap.w = w;
  prim->param.imagemap.h = h;

  size = prim->param.imagemap.iw*prim->param.imagemap.ih;
  prim->param_buffer = malloc(size + 256);
  prim->param.imagemap.index = prim->param_buffer;
  prim->param.imagemap.colors = (long*)(prim->param.imagemap.index + size);

  offset = ymin*iw + xmin;
  index += offset;

  dindex = prim->param.imagemap.index;
  dcolors = prim->param.imagemap.colors;
  offset = iw - prim->param.imagemap.iw;

  for (l = ymin; l <= ymax; l++)
  {
    for (c = xmin; c <= xmax; c++)
    {
      if (*index > n)
        n = *index;

      *dindex++ = *index++;
    }

    index += offset;
  }

  n++;

  for (c = 0; c < n; c++)
    dcolors[c] = colors[c];

  picUpdateBBox(ctxcanvas, x, y, 0);
  picUpdateBBox(ctxcanvas, x+prim->param.imagemap.iw-1, y+prim->param.imagemap.ih-1, 0);
}


/**********/
/* cdPlay */
/**********/

typedef int(*_cdsizecb)(cdCanvas* canvas, int w, int h, double w_mm, double h_mm);
static _cdsizecb cdsizecb = NULL;

static int cdregistercallback(int cb, cdCallback func)
{
  switch (cb)
  {
  case CD_SIZECB:
    cdsizecb = (_cdsizecb)func;
    return CD_OK;
  }

  return CD_ERROR;
}

#define sMin1(_v) (_v == 0? 1: _v)

#define sScaleX(_x) cdRound((_x - pic_xmin) * factorX + xmin)
#define sScaleY(_y) cdRound((_y - pic_ymin) * factorY + ymin)
#define sScaleW(_w) sMin1(cdRound(_w * factorX))
#define sScaleH(_h) sMin1(cdRound(_h * factorY))

#define sfScaleX(_x) ((_x - pic_xmin) * factorX + xmin)
#define sfScaleY(_y) ((_y - pic_ymin) * factorY + ymin)
#define sfScaleW(_w) (_w * factorX)
#define sfScaleH(_h) (_h * factorY)

static int cdplay(cdCanvas* canvas, int xmin, int xmax, int ymin, int ymax, void *data)
{
  tPrimNode *prim;
  cdCanvas* pic_canvas = (cdCanvas*)data;
  cdCtxCanvas* ctxcanvas = pic_canvas->ctxcanvas;
  int p, i, n, scale = 0, 
      pic_xmin = ctxcanvas->xmin,
      pic_ymin = ctxcanvas->ymin;
  double factorX = 1, factorY = 1;
  
  if ((ctxcanvas->xmax-ctxcanvas->xmin)!=0 && 
      (ctxcanvas->ymax-ctxcanvas->ymin)!=0 && 
      (xmax-xmin)!=0 && 
      (ymax-ymin)!=0)
  {
    scale = 1;
    factorX = ((double)(xmax-xmin)) / (ctxcanvas->xmax-ctxcanvas->xmin);
    factorY = ((double)(ymax-ymin)) / (ctxcanvas->ymax-ctxcanvas->ymin);
  }

  if (cdsizecb)
  {
    int err;
    err = cdsizecb(canvas, pic_canvas->w, pic_canvas->h, pic_canvas->w_mm, pic_canvas->h_mm);
    if (err)
      return CD_ERROR;
  }

  prim = ctxcanvas->prim_first;
  for (i = 0; i < ctxcanvas->prim_n; i++)
  { 
    if (scale)
    {
      switch (prim->type)
      {
      case CDPIC_LINE:
        primUpdateAttrib_Line(prim, canvas);
        cdCanvasLine(canvas, sScaleX(prim->param.lineboxrect.x1), sScaleY(prim->param.lineboxrect.y1), sScaleX(prim->param.lineboxrect.x2), sScaleY(prim->param.lineboxrect.y2));
        break;
      case CDPIC_FLINE:
        primUpdateAttrib_Line(prim, canvas);
        cdfCanvasLine(canvas, sfScaleX(prim->param.lineboxrectf.x1), sfScaleY(prim->param.lineboxrectf.y1), sfScaleX(prim->param.lineboxrectf.x2), sfScaleY(prim->param.lineboxrectf.y2));
        break;
      case CDPIC_RECT:
        primUpdateAttrib_Line(prim, canvas);
        cdCanvasRect(canvas, sScaleX(prim->param.lineboxrect.x1), sScaleX(prim->param.lineboxrect.x2), sScaleY(prim->param.lineboxrect.y1), sScaleY(prim->param.lineboxrect.y2));
        break;
      case CDPIC_FRECT:
        primUpdateAttrib_Line(prim, canvas);
        cdfCanvasRect(canvas, sfScaleX(prim->param.lineboxrectf.x1), sfScaleX(prim->param.lineboxrectf.x2), sfScaleY(prim->param.lineboxrectf.y1), sfScaleY(prim->param.lineboxrectf.y2));
        break;
      case CDPIC_BOX:
        primUpdateAttrib_Fill(prim, canvas);
        cdCanvasBox(canvas, sScaleX(prim->param.lineboxrect.x1), sScaleX(prim->param.lineboxrect.x2), sScaleY(prim->param.lineboxrect.y1), sScaleY(prim->param.lineboxrect.y2));
        break;
      case CDPIC_FBOX:
        primUpdateAttrib_Fill(prim, canvas);
        cdfCanvasBox(canvas, sfScaleX(prim->param.lineboxrectf.x1), sfScaleX(prim->param.lineboxrectf.x2), sfScaleY(prim->param.lineboxrectf.y1), sfScaleY(prim->param.lineboxrectf.y2));
        break;
      case CDPIC_ARC:
        primUpdateAttrib_Line(prim, canvas);
        cdCanvasArc(canvas, sScaleX(prim->param.arcsectorchord.xc), sScaleY(prim->param.arcsectorchord.yc), sScaleW(prim->param.arcsectorchord.w), sScaleH(prim->param.arcsectorchord.h), prim->param.arcsectorchord.angle1, prim->param.arcsectorchord.angle2);
        break;
      case CDPIC_FARC:
        primUpdateAttrib_Line(prim, canvas);
        cdfCanvasArc(canvas, sfScaleX(prim->param.arcsectorchordf.xc), sfScaleY(prim->param.arcsectorchordf.yc), sfScaleW(prim->param.arcsectorchordf.w), sfScaleH(prim->param.arcsectorchordf.h), prim->param.arcsectorchord.angle1, prim->param.arcsectorchord.angle2);
        break;
      case CDPIC_SECTOR:
        primUpdateAttrib_Fill(prim, canvas);
        cdCanvasSector(canvas, sScaleX(prim->param.arcsectorchord.xc), sScaleY(prim->param.arcsectorchord.yc), sScaleW(prim->param.arcsectorchord.w), sScaleH(prim->param.arcsectorchord.h), prim->param.arcsectorchord.angle1, prim->param.arcsectorchord.angle2);
        break;
      case CDPIC_FSECTOR:
        primUpdateAttrib_Fill(prim, canvas);
        cdfCanvasSector(canvas, sfScaleX(prim->param.arcsectorchordf.xc), sfScaleY(prim->param.arcsectorchordf.yc), sfScaleW(prim->param.arcsectorchordf.w), sfScaleH(prim->param.arcsectorchordf.h), prim->param.arcsectorchord.angle1, prim->param.arcsectorchord.angle2);
        break;
      case CDPIC_CHORD:
        primUpdateAttrib_Fill(prim, canvas);
        cdCanvasChord(canvas, sScaleX(prim->param.arcsectorchord.xc), sScaleY(prim->param.arcsectorchord.yc), sScaleW(prim->param.arcsectorchord.w), sScaleH(prim->param.arcsectorchord.h), prim->param.arcsectorchord.angle1, prim->param.arcsectorchord.angle2);
        break;
      case CDPIC_FCHORD:
        primUpdateAttrib_Fill(prim, canvas);
        cdfCanvasChord(canvas, sfScaleX(prim->param.arcsectorchordf.xc), sfScaleY(prim->param.arcsectorchordf.yc), sfScaleW(prim->param.arcsectorchordf.w), sfScaleH(prim->param.arcsectorchordf.h), prim->param.arcsectorchord.angle1, prim->param.arcsectorchord.angle2);
        break;
      case CDPIC_TEXT:
        primUpdateAttrib_Text(prim, canvas);
        cdCanvasText(canvas, sScaleX(prim->param.text.x), sScaleY(prim->param.text.y), prim->param.text.s);
        break;
      case CDPIC_FTEXT:
        primUpdateAttrib_Text(prim, canvas);
        cdfCanvasText(canvas, sfScaleX(prim->param.textf.x), sfScaleY(prim->param.textf.y), prim->param.text.s);
        break;
      case CDPIC_POLY:
        if (prim->param.poly.mode == CD_FILL)
          primUpdateAttrib_Fill(prim, canvas);
        else
          primUpdateAttrib_Line(prim, canvas);
        cdCanvasBegin(canvas, prim->param.poly.mode);
        for (p = 0; p < prim->param.poly.n; p++)
          cdCanvasVertex(canvas, sScaleX(prim->param.poly.points[p].x), sScaleY(prim->param.poly.points[p].y));
        cdCanvasEnd(canvas);
        break;
      case CDPIC_FPOLY:
        if (prim->param.poly.mode == CD_FILL)
          primUpdateAttrib_Fill(prim, canvas);
        else
          primUpdateAttrib_Line(prim, canvas);
        cdCanvasBegin(canvas, prim->param.polyf.mode);
        for (p = 0; p < prim->param.polyf.n; p++)
          cdfCanvasVertex(canvas, sfScaleX(prim->param.polyf.points[p].x), sfScaleY(prim->param.polyf.points[p].y));
        cdCanvasEnd(canvas);
        break;
      case CDPIC_PATH:
        if (prim->param.path.fill)
          primUpdateAttrib_Fill(prim, canvas);
        else
          primUpdateAttrib_Line(prim, canvas);
        cdCanvasBegin(canvas, CD_PATH);
        n = 0;
        for (p=0; p<prim->param.path.path_n; p++)
        {
          cdCanvasPathSet(canvas, prim->param.path.path[p]);

          switch(prim->param.path.path[p])
          {
          case CD_PATH_MOVETO:
          case CD_PATH_LINETO:
            if (n+1 > n) break;
            cdCanvasVertex(canvas, sScaleX(prim->param.path.points[n].x), sScaleY(prim->param.path.points[n].y));
            n++;
            break;
          case CD_PATH_CURVETO:
          case CD_PATH_ARC:
            {
              if (n+3 > n) break;
              cdCanvasVertex(canvas, sScaleX(prim->param.path.points[n].x), sScaleY(prim->param.path.points[n].y));
              cdCanvasVertex(canvas, sScaleX(prim->param.path.points[n+1].x), sScaleY(prim->param.path.points[n+1].y));
              cdCanvasVertex(canvas, sScaleX(prim->param.path.points[n+2].x), sScaleY(prim->param.path.points[n+2].y));
              n += 3;
            }
            break;
          }
        }
        cdCanvasEnd(canvas);
        break;
      case CDPIC_FPATH:
        if (prim->param.path.fill)
          primUpdateAttrib_Fill(prim, canvas);
        else
          primUpdateAttrib_Line(prim, canvas);
        cdCanvasBegin(canvas, CD_PATH);
        n = 0;
        for (p=0; p<prim->param.pathf.path_n; p++)
        {
          cdCanvasPathSet(canvas, prim->param.pathf.path[p]);

          switch(prim->param.pathf.path[p])
          {
          case CD_PATH_MOVETO:
          case CD_PATH_LINETO:
            if (n+1 > n) break;
            cdfCanvasVertex(canvas, sfScaleX(prim->param.pathf.points[n].x), sfScaleY(prim->param.pathf.points[n].y));
            n++;
            break;
          case CD_PATH_CURVETO:
          case CD_PATH_ARC:
            {
              if (n+3 > n) break;
              cdfCanvasVertex(canvas, sfScaleX(prim->param.pathf.points[n].x), sfScaleY(prim->param.pathf.points[n].y));
              cdfCanvasVertex(canvas, sfScaleX(prim->param.pathf.points[n+1].x), sfScaleY(prim->param.pathf.points[n+1].y));
              cdfCanvasVertex(canvas, sfScaleX(prim->param.pathf.points[n+2].x), sfScaleY(prim->param.pathf.points[n+2].y));
              n += 3;
            }
            break;
          }
        }
        cdCanvasEnd(canvas);
        break;
      case CDPIC_IMAGERGB:
        cdCanvasPutImageRectRGB(canvas, prim->param.imagergba.iw, prim->param.imagergba.ih, prim->param.imagergba.r, prim->param.imagergba.g, prim->param.imagergba.b, sScaleX(prim->param.imagergba.x), sScaleY(prim->param.imagergba.y), sScaleW(prim->param.imagergba.w), sScaleH(prim->param.imagergba.h), 0, 0, 0, 0);
        break;
      case CDPIC_IMAGERGBA:
        cdCanvasPutImageRectRGBA(canvas, prim->param.imagergba.iw, prim->param.imagergba.ih, prim->param.imagergba.r, prim->param.imagergba.g, prim->param.imagergba.b, prim->param.imagergba.a, sScaleX(prim->param.imagergba.x), sScaleY(prim->param.imagergba.y), sScaleW(prim->param.imagergba.w), sScaleH(prim->param.imagergba.h), 0, 0, 0, 0);
        break;
      case CDPIC_IMAGEMAP:
        cdCanvasPutImageRectMap(canvas, prim->param.imagemap.iw, prim->param.imagemap.ih, prim->param.imagemap.index, prim->param.imagemap.colors, sScaleX(prim->param.imagemap.x), sScaleY(prim->param.imagemap.y), sScaleW(prim->param.imagemap.w), sScaleH(prim->param.imagemap.h), 0, 0, 0, 0);
        break;
      case CDPIC_PIXEL:
        cdCanvasPixel(canvas, sScaleX(prim->param.pixel.x), sScaleY(prim->param.pixel.y), prim->param.pixel.color);
        break;
      }
    }
    else
    {
      switch (prim->type)
      {
      case CDPIC_LINE:
        primUpdateAttrib_Line(prim, canvas);
        cdCanvasLine(canvas, prim->param.lineboxrect.x1, prim->param.lineboxrect.y1, prim->param.lineboxrect.x2, prim->param.lineboxrect.y2);
        break;
      case CDPIC_FLINE:
        primUpdateAttrib_Line(prim, canvas);
        cdfCanvasLine(canvas, prim->param.lineboxrectf.x1, prim->param.lineboxrectf.y1, prim->param.lineboxrectf.x2, prim->param.lineboxrectf.y2);
        break;
      case CDPIC_RECT:
        primUpdateAttrib_Line(prim, canvas);
        cdCanvasRect(canvas, prim->param.lineboxrect.x1, prim->param.lineboxrect.x2, prim->param.lineboxrect.y1, prim->param.lineboxrect.y2);
        break;
      case CDPIC_FRECT:
        primUpdateAttrib_Line(prim, canvas);
        cdfCanvasRect(canvas, prim->param.lineboxrectf.x1, prim->param.lineboxrectf.x2, prim->param.lineboxrectf.y1, prim->param.lineboxrectf.y2);
        break;
      case CDPIC_BOX:
        primUpdateAttrib_Fill(prim, canvas);
        cdCanvasBox(canvas, prim->param.lineboxrect.x1, prim->param.lineboxrect.x2, prim->param.lineboxrect.y1, prim->param.lineboxrect.y2);
        break;
      case CDPIC_FBOX:
        primUpdateAttrib_Fill(prim, canvas);
        cdfCanvasBox(canvas, prim->param.lineboxrectf.x1, prim->param.lineboxrectf.x2, prim->param.lineboxrectf.y1, prim->param.lineboxrectf.y2);
        break;
      case CDPIC_ARC:
        primUpdateAttrib_Line(prim, canvas);
        cdCanvasArc(canvas, prim->param.arcsectorchord.xc, prim->param.arcsectorchord.yc, prim->param.arcsectorchord.w, prim->param.arcsectorchord.h, prim->param.arcsectorchord.angle1, prim->param.arcsectorchord.angle2);
        break;
      case CDPIC_FARC:
        primUpdateAttrib_Line(prim, canvas);
        cdfCanvasArc(canvas, prim->param.arcsectorchordf.xc, prim->param.arcsectorchordf.yc, prim->param.arcsectorchordf.w, prim->param.arcsectorchordf.h, prim->param.arcsectorchord.angle1, prim->param.arcsectorchord.angle2);
        break;
      case CDPIC_SECTOR:
        primUpdateAttrib_Fill(prim, canvas);
        cdCanvasSector(canvas, prim->param.arcsectorchord.xc, prim->param.arcsectorchord.yc, prim->param.arcsectorchord.w, prim->param.arcsectorchord.h, prim->param.arcsectorchord.angle1, prim->param.arcsectorchord.angle2);
        break;
      case CDPIC_FSECTOR:
        primUpdateAttrib_Fill(prim, canvas);
        cdfCanvasSector(canvas, prim->param.arcsectorchordf.xc, prim->param.arcsectorchordf.yc, prim->param.arcsectorchordf.w, prim->param.arcsectorchordf.h, prim->param.arcsectorchord.angle1, prim->param.arcsectorchord.angle2);
        break;
      case CDPIC_CHORD:
        primUpdateAttrib_Fill(prim, canvas);
        cdCanvasChord(canvas, prim->param.arcsectorchord.xc, prim->param.arcsectorchord.yc, prim->param.arcsectorchord.w, prim->param.arcsectorchord.h, prim->param.arcsectorchord.angle1, prim->param.arcsectorchord.angle2);
        break;
      case CDPIC_FCHORD:
        primUpdateAttrib_Fill(prim, canvas);
        cdfCanvasChord(canvas, prim->param.arcsectorchordf.xc, prim->param.arcsectorchordf.yc, prim->param.arcsectorchordf.w, prim->param.arcsectorchordf.h, prim->param.arcsectorchord.angle1, prim->param.arcsectorchord.angle2);
        break;
      case CDPIC_TEXT:
        primUpdateAttrib_Text(prim, canvas);
        cdCanvasText(canvas, prim->param.text.x, prim->param.text.y, prim->param.text.s);
        break;
      case CDPIC_FTEXT:
        primUpdateAttrib_Text(prim, canvas);
        cdfCanvasText(canvas, prim->param.textf.x, prim->param.textf.y, prim->param.text.s);
        break;
      case CDPIC_POLY:
        if (prim->param.poly.mode == CD_FILL)
          primUpdateAttrib_Fill(prim, canvas);
        else
          primUpdateAttrib_Line(prim, canvas);
        cdCanvasBegin(canvas, prim->param.poly.mode);
        for (p = 0; p < prim->param.poly.n; p++)
          cdCanvasVertex(canvas, prim->param.poly.points[p].x, prim->param.poly.points[p].y);
        cdCanvasEnd(canvas);
        break;
      case CDPIC_FPOLY:
        if (prim->param.poly.mode == CD_FILL)
          primUpdateAttrib_Fill(prim, canvas);
        else
          primUpdateAttrib_Line(prim, canvas);
        cdCanvasBegin(canvas, prim->param.polyf.mode);
        for (p = 0; p < prim->param.polyf.n; p++)
          cdfCanvasVertex(canvas, prim->param.polyf.points[p].x, prim->param.polyf.points[p].y);
        cdCanvasEnd(canvas);
        break;
      case CDPIC_PATH:
        if (prim->param.path.fill)
          primUpdateAttrib_Fill(prim, canvas);
        else
          primUpdateAttrib_Line(prim, canvas);
        cdCanvasBegin(canvas, CD_PATH);
        n = 0;
        for (p=0; p<prim->param.path.path_n; p++)
        {
          cdCanvasPathSet(canvas, prim->param.path.path[p]);

          switch(prim->param.path.path[p])
          {
          case CD_PATH_MOVETO:
          case CD_PATH_LINETO:
            if (n+1 > n) break;
            cdCanvasVertex(canvas, prim->param.path.points[n].x, prim->param.path.points[n].y);
            n++;
            break;
          case CD_PATH_CURVETO:
          case CD_PATH_ARC:
            {
              if (n+3 > n) break;
              cdCanvasVertex(canvas, prim->param.path.points[n].x,   prim->param.path.points[n].y);
              cdCanvasVertex(canvas, prim->param.path.points[n+1].x, prim->param.path.points[n+1].y);
              cdCanvasVertex(canvas, prim->param.path.points[n+2].x, prim->param.path.points[n+2].y);
              n += 3;
            }
            break;
          }
        }
        cdCanvasEnd(canvas);
        break;
      case CDPIC_FPATH:
        if (prim->param.path.fill)
          primUpdateAttrib_Fill(prim, canvas);
        else
          primUpdateAttrib_Line(prim, canvas);
        cdCanvasBegin(canvas, CD_PATH);
        n = 0;
        for (p=0; p<prim->param.pathf.path_n; p++)
        {
          cdCanvasPathSet(canvas, prim->param.pathf.path[p]);

          switch(prim->param.pathf.path[p])
          {
          case CD_PATH_MOVETO:
          case CD_PATH_LINETO:
            if (n+1 > n) break;
            cdfCanvasVertex(canvas, prim->param.pathf.points[n].x, prim->param.pathf.points[n].y);
            n++;
            break;
          case CD_PATH_CURVETO:
          case CD_PATH_ARC:
            {
              if (n+3 > n) break;
              cdfCanvasVertex(canvas, prim->param.pathf.points[n].x,   prim->param.pathf.points[n].y);
              cdfCanvasVertex(canvas, prim->param.pathf.points[n+1].x, prim->param.pathf.points[n+1].y);
              cdfCanvasVertex(canvas, prim->param.pathf.points[n+2].x, prim->param.pathf.points[n+2].y);
              n += 3;
            }
            break;
          }
        }
        cdCanvasEnd(canvas);
        break;
      case CDPIC_IMAGERGB:
        cdCanvasPutImageRectRGB(canvas, prim->param.imagergba.iw, prim->param.imagergba.ih, prim->param.imagergba.r, prim->param.imagergba.g, prim->param.imagergba.b, prim->param.imagergba.x, prim->param.imagergba.y, prim->param.imagergba.w, prim->param.imagergba.h, 0, 0, 0, 0);
        break;
      case CDPIC_IMAGERGBA:
        cdCanvasPutImageRectRGBA(canvas, prim->param.imagergba.iw, prim->param.imagergba.ih, prim->param.imagergba.r, prim->param.imagergba.g, prim->param.imagergba.b, prim->param.imagergba.a, prim->param.imagergba.x, prim->param.imagergba.y, prim->param.imagergba.w, prim->param.imagergba.h, 0, 0, 0, 0);
        break;
      case CDPIC_IMAGEMAP:
        cdCanvasPutImageRectMap(canvas, prim->param.imagemap.iw, prim->param.imagemap.ih, prim->param.imagemap.index, prim->param.imagemap.colors, prim->param.imagemap.x, prim->param.imagemap.y, prim->param.imagemap.w, prim->param.imagemap.h, 0, 0, 0, 0);
        break;
      case CDPIC_PIXEL:
        cdCanvasPixel(canvas, prim->param.pixel.x, prim->param.pixel.y, prim->param.pixel.color);
        break;
      }
    }

    prim = prim->next;
  }

  return CD_OK;
}

static void cdkillcanvas(cdCtxCanvas *ctxcanvas)
{
  cdclear(ctxcanvas);
  memset(ctxcanvas, 0, sizeof(cdCtxCanvas));
  free(ctxcanvas);
}

/*******************/
/* Canvas Creation */
/*******************/

static void cdcreatecanvas(cdCanvas *canvas, void *data)
{
  char* strdata = (char*)data;
  double res = 3.78;
  cdCtxCanvas* ctxcanvas;

  sscanf(strdata, "%lg", &res);

  ctxcanvas = (cdCtxCanvas *)malloc(sizeof(cdCtxCanvas));
  memset(ctxcanvas, 0, sizeof(cdCtxCanvas));

  ctxcanvas->canvas = canvas;
  canvas->ctxcanvas = ctxcanvas;

  /* update canvas context */
  canvas->w = 0;
  canvas->h = 0;
  canvas->w_mm = 0;
  canvas->h_mm = 0;
  canvas->bpp = 24;
  canvas->xres = res;
  canvas->yres = res;
}

static void cdinittable(cdCanvas* canvas)
{
  canvas->cxFont = cdfont;
  canvas->cxClear = cdclear;
  canvas->cxPixel = cdpixel;
  canvas->cxLine = cdline;
  canvas->cxFLine = cdfline;
  canvas->cxRect = cdrect;
  canvas->cxFRect = cdfrect;
  canvas->cxBox = cdbox;
  canvas->cxFBox = cdfbox;
  canvas->cxArc = cdarc;
  canvas->cxFArc = cdfarc;
  canvas->cxSector = cdsector;
  canvas->cxFSector = cdfsector;
  canvas->cxChord = cdchord;
  canvas->cxFChord = cdfchord;
  canvas->cxText = cdtext;
  canvas->cxFText = cdftext;
  canvas->cxPoly = cdpoly;
  canvas->cxFPoly = cdfpoly;
  canvas->cxPutImageRectRGB = cdputimagerectrgb;
  canvas->cxPutImageRectRGBA = cdputimagerectrgba;
  canvas->cxPutImageRectMap = cdputimagerectmap;
  canvas->cxKillCanvas = cdkillcanvas;
}

static cdContext cdPictureContext =
{
  CD_CAP_ALL & ~(CD_CAP_GETIMAGERGB | CD_CAP_IMAGESRV | 
                 CD_CAP_REGION | CD_CAP_FONTDIM | CD_CAP_TEXTSIZE),
  0,
  cdcreatecanvas,
  cdinittable,
  cdplay,
  cdregistercallback,
};

cdContext* cdContextPicture(void)
{
  return &cdPictureContext;
}
