/** \file
 * \brief CD Metafile driver
 *
 * See Copyright Notice in cd.h
 */

#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <limits.h> 

#include "cd.h"
#include "wd.h"
#include "cd_private.h"
#include "cdmf.h"
#include "cdmf_private.h"


/* codes for the primitives and attributes in the metafile 
   Can NOT be changed, only added for backward compatibility.
*/
enum 
{
  CDMF_FLUSH,                    /*  0 */
  CDMF_CLEAR,                    /*  1 */
  CDMF_CLIP,                     /*  2 */
  CDMF_CLIPAREA,                 /*  3 */
  CDMF_LINE,                     /*  4 */
  CDMF_BOX,                      /*  5 */
  CDMF_ARC,                      /*  6 */
  CDMF_SECTOR,                   /*  7 */
  CDMF_TEXT,                     /*  8 */
  CDMF_BEGIN,                    /*  9 */
  CDMF_VERTEX,                   /* 10 */
  CDMF_END,                      /* 11 */
  CDMF_MARK,                     /* 12 */
  CDMF_BACKOPACITY,              /* 13 */
  CDMF_WRITEMODE,                /* 14 */
  CDMF_LINESTYLE,                /* 15 */
  CDMF_LINEWIDTH,                /* 16 */
  CDMF_INTERIORSTYLE,            /* 17 */
  CDMF_HATCH,                    /* 18 */
  CDMF_STIPPLE,                  /* 19 */
  CDMF_PATTERN,                  /* 20 */
  CDMF_OLDFONT,                  /* 21 */
  CDMF_NATIVEFONT,               /* 22 */
  CDMF_TEXTALIGNMENT,            /* 23 */
  CDMF_MARKTYPE,                 /* 24 */
  CDMF_MARKSIZE,                 /* 25 */
  CDMF_PALETTE,                  /* 26 */
  CDMF_BACKGROUND,               /* 27 */
  CDMF_FOREGROUND,               /* 28 */
  CDMF_PUTIMAGERGB,              /* 29 */
  CDMF_PUTIMAGEMAP,              /* 30 */
  CDMF_PIXEL,                    /* 31 */
  CDMF_SCROLLAREA,               /* 32 */
  CDMF_TEXTORIENTATION,          /* 33 */
  CDMF_RECT,                     /* 34 */
  CDMF_PUTIMAGERGBA,             /* 35 */
  CDMF_WLINE,                    /* 36 */
  CDMF_WRECT,                    /* 37 */
  CDMF_WBOX,                     /* 38 */
  CDMF_WARC,                     /* 39 */
  CDMF_WSECTOR,                  /* 40 */
  CDMF_WTEXT,                    /* 41 */
  CDMF_WVERTEX,                  /* 42 */
  CDMF_WMARK,                    /* 43 */
  CDMF_VECTORTEXT,               /* 44 */
  CDMF_MULTILINEVECTORTEXT,      /* 45 */
  CDMF_WVECTORTEXT,              /* 46 */
  CDMF_WMULTILINEVECTORTEXT,     /* 47 */
  CDMF_WINDOW,                   /* 48 */
  CDMF_WCLIPAREA,                /* 49 */
  CDMF_VECTORFONT,               /* 50 */
  CDMF_VECTORTEXTDIRECTION,      /* 51 */
  CDMF_VECTORTEXTTRANSFORM,      /* 52 */
  CDMF_VECTORTEXTSIZE,           /* 53 */
  CDMF_VECTORCHARSIZE,           /* 54 */
  CDMF_WVECTORTEXTDIRECTION,     /* 55 */
  CDMF_WVECTORTEXTSIZE,          /* 56 */
  CDMF_WVECTORCHARSIZE,          /* 57 */
  CDMF_FILLMODE,                 /* 58 */
  CDMF_LINESTYLEDASHES,          /* 59 */
  CDMF_LINECAP,                  /* 60 */
  CDMF_LINEJOIN,                 /* 61 */
  CDMF_CHORD,                    /* 62 */
  CDMF_WCHORD,                   /* 63 */
  CDMF_FLINE,                    /* 64 */
  CDMF_FRECT,                    /* 65 */
  CDMF_FBOX,                     /* 66 */
  CDMF_FARC,                     /* 67 */
  CDMF_FSECTOR,                  /* 68 */
  CDMF_FTEXT,                    /* 69 */
  CDMF_FVERTEX,                  /* 70 */
  CDMF_MATRIX,                   /* 71 */
  CDMF_FCHORD,                   /* 72 */ 
  CDMF_FCLIPAREA,                /* 73 */
  CDMF_FONT,                     /* 74 */
  CDMF_RESETMATRIX,              /* 75 */
  CDMF_PATHSET                   /* 76 */
};                                  
                                    
struct _cdCtxCanvas 
{
  /* public */
  cdCanvas* canvas;
  char* filename;       
  void* data;     /* used by other drivers */

  /* private */
  int last_line_style;
  int last_fill_mode;
  FILE* file;
};

void cdkillcanvasMF(cdCanvasMF *mfcanvas)
{
  cdCtxCanvas *ctxcanvas = (cdCtxCanvas*)mfcanvas;
  free(ctxcanvas->filename);
  fclose(ctxcanvas->file);
  memset(ctxcanvas, 0, sizeof(cdCtxCanvas));
  free(ctxcanvas);
}

static void cdflush(cdCtxCanvas *ctxcanvas)
{
  fflush(ctxcanvas->file);
  fprintf(ctxcanvas->file, "%d\n", CDMF_FLUSH);
}

static void cdclear(cdCtxCanvas *ctxcanvas)
{
  fprintf(ctxcanvas->file, "%d\n", CDMF_CLEAR);
}

static int cdclip(cdCtxCanvas *ctxcanvas, int mode)
{
  fprintf(ctxcanvas->file, "%d %d\n", CDMF_CLIP, mode);
  return mode;
}

static void cdcliparea(cdCtxCanvas *ctxcanvas, int xmin, int xmax, int ymin, int ymax)
{
  fprintf(ctxcanvas->file, "%d %d %d %d %d\n", CDMF_CLIPAREA, xmin, xmax, ymin, ymax);
}

static void cdfcliparea(cdCtxCanvas *ctxcanvas, double xmin, double xmax, double ymin, double ymax)
{
  fprintf(ctxcanvas->file, "%d %g %g %g %g\n", CDMF_FCLIPAREA, xmin, xmax, ymin, ymax);
}

static void cdtransform(cdCtxCanvas *ctxcanvas, const double* matrix)
{
  if (matrix)
    fprintf(ctxcanvas->file, "%d %g %g %g %g %g %g\n", CDMF_MATRIX, matrix[0], matrix[1], matrix[2], matrix[3], matrix[4], matrix[5]);
  else
    fprintf(ctxcanvas->file, "%d\n", CDMF_RESETMATRIX);
}

static void cdline(cdCtxCanvas *ctxcanvas, int x1, int y1, int x2, int y2)
{
  fprintf(ctxcanvas->file, "%d %d %d %d %d\n", CDMF_LINE, x1, y1, x2, y2);
}

static void cdfline(cdCtxCanvas *ctxcanvas, double x1, double y1, double x2, double y2)
{
  fprintf(ctxcanvas->file, "%d %g %g %g %g\n", CDMF_FLINE, x1, y1, x2, y2);
}

static void cdrect(cdCtxCanvas *ctxcanvas, int xmin, int xmax, int ymin, int ymax)
{
  fprintf(ctxcanvas->file, "%d %d %d %d %d\n", CDMF_RECT, xmin, xmax, ymin, ymax);
}

static void cdfrect(cdCtxCanvas *ctxcanvas, double xmin, double xmax, double ymin, double ymax)
{
  fprintf(ctxcanvas->file, "%d %g %g %g %g\n", CDMF_FRECT, xmin, xmax, ymin, ymax);
}

static void cdbox(cdCtxCanvas *ctxcanvas, int xmin, int xmax, int ymin, int ymax)
{
  fprintf(ctxcanvas->file, "%d %d %d %d %d\n", CDMF_BOX, xmin, xmax, ymin, ymax);
}

static void cdfbox(cdCtxCanvas *ctxcanvas, double xmin, double xmax, double ymin, double ymax)
{
  fprintf(ctxcanvas->file, "%d %g %g %g %g\n", CDMF_FBOX, xmin, xmax, ymin, ymax);
}

static void cdarc(cdCtxCanvas *ctxcanvas, int xc, int yc, int w, int h, double a1, double a2)
{
  fprintf(ctxcanvas->file, "%d %d %d %d %d %g %g\n", CDMF_ARC, xc, yc, w, h, a1, a2);
}

static void cdfarc(cdCtxCanvas *ctxcanvas, double xc, double yc, double w, double h, double a1, double a2)
{
  fprintf(ctxcanvas->file, "%d %g %g %g %g %g %g\n", CDMF_FARC, xc, yc, w, h, a1, a2);
}

static void cdsector(cdCtxCanvas *ctxcanvas, int xc, int yc, int w, int h, double a1, double a2)
{
  fprintf(ctxcanvas->file, "%d %d %d %d %d %g %g\n", CDMF_SECTOR, xc, yc, w, h, a1, a2);
}

static void cdfsector(cdCtxCanvas *ctxcanvas, double xc, double yc, double w, double h, double a1, double a2)
{
  fprintf(ctxcanvas->file, "%d %g %g %g %g %g %g\n", CDMF_FSECTOR, xc, yc, w, h, a1, a2);
}

static void cdchord(cdCtxCanvas *ctxcanvas, int xc, int yc, int w, int h, double a1, double a2)
{
  fprintf(ctxcanvas->file, "%d %d %d %d %d %g %g\n", CDMF_CHORD, xc, yc, w, h, a1, a2);
}

static void cdfchord(cdCtxCanvas *ctxcanvas, double xc, double yc, double w, double h, double a1, double a2)
{
  fprintf(ctxcanvas->file, "%d %g %g %g %g %g %g\n", CDMF_FCHORD, xc, yc, w, h, a1, a2);
}

static void cdtext(cdCtxCanvas *ctxcanvas, int x, int y, const char *text, int len)
{
  text = cdStrDupN(text, len);
  fprintf(ctxcanvas->file, "%d %d %d %s\n", CDMF_TEXT, x, y, text);
  free((char*)text);
}

static void cdftext(cdCtxCanvas *ctxcanvas, double x, double y, const char *text, int len)
{
  text = cdStrDupN(text, len);
  fprintf(ctxcanvas->file, "%d %g %g %s\n", CDMF_FTEXT, x, y, text);
  free((char*)text);
}

static void cdpoly(cdCtxCanvas *ctxcanvas, int mode, cdPoint* poly, int n)
{
  int i;

  if (mode == CD_FILL && ctxcanvas->canvas->fill_mode != ctxcanvas->last_fill_mode)
  {
    fprintf(ctxcanvas->file, "%d %d\n", CDMF_FILLMODE, ctxcanvas->canvas->fill_mode);
    ctxcanvas->last_fill_mode = ctxcanvas->canvas->fill_mode;
  }

  fprintf(ctxcanvas->file, "%d %d\n", CDMF_BEGIN, mode);

  if (mode == CD_PATH)
  {
    int p;

    i = 0;
    for (p=0; p<ctxcanvas->canvas->path_n; p++)
    {
      fprintf(ctxcanvas->file, "%d %d\n", CDMF_PATHSET, ctxcanvas->canvas->path[p]);

      switch(ctxcanvas->canvas->path[p])
      {
      case CD_PATH_MOVETO:
      case CD_PATH_LINETO:
        if (i+1 > n) 
        {
          fprintf(ctxcanvas->file, "ERROR: not enough points in path\n");
          return;
        }
        fprintf(ctxcanvas->file, "%d %d %d\n", CDMF_VERTEX, poly[i].x, poly[i].y);
        i++;
        break;
      case CD_PATH_CURVETO:
      case CD_PATH_ARC:
        {
          if (i+3 > n)
          {
            fprintf(ctxcanvas->file, "ERROR: not enough points in path\n");
            return;
          }
          fprintf(ctxcanvas->file, "%d %d %d\n", CDMF_VERTEX, poly[i].x, poly[i].y);
          fprintf(ctxcanvas->file, "%d %d %d\n", CDMF_VERTEX, poly[i+1].x, poly[i+1].y);
          fprintf(ctxcanvas->file, "%d %d %d\n", CDMF_VERTEX, poly[i+2].x, poly[i+2].y);
          i += 3;
        }
        break;
      }
    }
  }
  else
  {
    for(i = 0; i<n; i++)
      fprintf(ctxcanvas->file, "%d %d %d\n", CDMF_VERTEX, poly[i].x, poly[i].y);
  }

  fprintf(ctxcanvas->file, "%d\n", CDMF_END);
}

static void cdfpoly(cdCtxCanvas *ctxcanvas, int mode, cdfPoint* poly, int n)
{
  int i;

  if (mode == CD_FILL && ctxcanvas->canvas->fill_mode != ctxcanvas->last_fill_mode)
  {
    fprintf(ctxcanvas->file, "%d %d\n", CDMF_FILLMODE, ctxcanvas->canvas->fill_mode);
    ctxcanvas->last_fill_mode = ctxcanvas->canvas->fill_mode;
  }

  fprintf(ctxcanvas->file, "%d %d\n", CDMF_BEGIN, mode);

  if (mode == CD_PATH)
  {
    int p;

    i = 0;
    for (p=0; p<ctxcanvas->canvas->path_n; p++)
    {
      fprintf(ctxcanvas->file, "%d %d\n", CDMF_PATHSET, ctxcanvas->canvas->path[p]);

      switch(ctxcanvas->canvas->path[p])
      {
      case CD_PATH_MOVETO:
      case CD_PATH_LINETO:
        if (i+1 > n) 
        {
          fprintf(ctxcanvas->file, "ERROR: not enough points in path\n");
          return;
        }
        fprintf(ctxcanvas->file, "%d %g %g\n", CDMF_FVERTEX, poly[i].x, poly[i].y);
        i++;
        break;
      case CD_PATH_CURVETO:
      case CD_PATH_ARC:
        {
          if (i+3 > n)
          {
            fprintf(ctxcanvas->file, "ERROR: not enough points in path\n");
            return;
          }
          fprintf(ctxcanvas->file, "%d %g %g\n", CDMF_FVERTEX, poly[i].x, poly[i].y);
          fprintf(ctxcanvas->file, "%d %g %g\n", CDMF_FVERTEX, poly[i+1].x, poly[i+1].y);
          fprintf(ctxcanvas->file, "%d %g %g\n", CDMF_FVERTEX, poly[i+2].x, poly[i+2].y);
          i += 3;
        }
        break;
      }
    }
  }
  else
  {
    for(i = 0; i<n; i++)
      fprintf(ctxcanvas->file, "%d %g %g\n", CDMF_FVERTEX, poly[i].x, poly[i].y);
  }

  fprintf(ctxcanvas->file, "%d\n", CDMF_END);
}

static int cdbackopacity(cdCtxCanvas *ctxcanvas, int opacity)
{
  fprintf(ctxcanvas->file, "%d %d\n", CDMF_BACKOPACITY, opacity);
  return opacity;
}

static int cdwritemode(cdCtxCanvas *ctxcanvas, int mode)
{
  fprintf(ctxcanvas->file, "%d %d\n", CDMF_WRITEMODE, mode);
  return mode;
}

static int cdlinestyle(cdCtxCanvas *ctxcanvas, int style)
{
  if (style == CD_CUSTOM && ctxcanvas->canvas->line_style != ctxcanvas->last_line_style)
  {
    int i;
    fprintf(ctxcanvas->file, "%d %d", CDMF_LINESTYLEDASHES, ctxcanvas->canvas->line_dashes_count);
    for (i = 0; i < ctxcanvas->canvas->line_dashes_count; i++)
      fprintf(ctxcanvas->file, " %d", ctxcanvas->canvas->line_dashes[i]);
    fprintf(ctxcanvas->file, "\n");
    ctxcanvas->last_line_style = ctxcanvas->canvas->line_style;
  }

  fprintf(ctxcanvas->file, "%d %d\n", CDMF_LINESTYLE, style);
  return style;
}

static int cdlinewidth(cdCtxCanvas *ctxcanvas, int width)
{
  fprintf(ctxcanvas->file, "%d %d\n", CDMF_LINEWIDTH, width);
  return width;
}

static int cdlinecap(cdCtxCanvas *ctxcanvas, int cap)
{
  fprintf(ctxcanvas->file, "%d %d\n", CDMF_LINECAP, cap);
  return cap;
}

static int cdlinejoin(cdCtxCanvas *ctxcanvas, int join)
{
  fprintf(ctxcanvas->file, "%d %d\n", CDMF_LINEJOIN, join);
  return join;
}

static int cdinteriorstyle(cdCtxCanvas *ctxcanvas, int style)
{
  fprintf(ctxcanvas->file, "%d %d\n", CDMF_INTERIORSTYLE, style);
  return style;
}

static int cdhatch(cdCtxCanvas *ctxcanvas, int style)
{
  fprintf(ctxcanvas->file, "%d %d\n", CDMF_HATCH, style);
  return style;
}

static void cdstipple(cdCtxCanvas *ctxcanvas, int w, int h, const unsigned char *stipple)
{
  int c, t;

  fprintf(ctxcanvas->file, "%d %d %d\n", CDMF_STIPPLE, w, h);

  t = w * h;

  for (c = 0; c < t; c++)
  {
    fprintf(ctxcanvas->file, "%d ", (int)*stipple++);
    if ((c + 1) % w == 0)
      fprintf(ctxcanvas->file, "\n");
  }
}

static void cdpattern(cdCtxCanvas *ctxcanvas, int w, int h, const long int *pattern)
{
  int c, t;
  unsigned char r, g, b;

  fprintf(ctxcanvas->file, "%d %d %d\n", CDMF_PATTERN, w, h);

  t = w * h;

  /* stores the pattern with separeted RGB values */
  for (c = 0; c < t; c++)
  {
    cdDecodeColor(*pattern++, &r, &g, &b);
    fprintf(ctxcanvas->file, "%d %d %d ", (int)r, (int)g, (int)b);
    if ((c + 1) % w == 0)
      fprintf(ctxcanvas->file, "\n");
  }
}

static int cdfont(cdCtxCanvas *ctxcanvas, const char* type_face, int style, int size)
{
  fprintf(ctxcanvas->file, "%d %d %d %s\n", CDMF_FONT, style, size, type_face);
  return 1;
}

static int cdnativefont(cdCtxCanvas *ctxcanvas, const char* font)
{
  fprintf(ctxcanvas->file, "%d %s\n", CDMF_NATIVEFONT, font);
  return 1;
}

static int cdtextalignment(cdCtxCanvas *ctxcanvas, int alignment)
{
  fprintf(ctxcanvas->file, "%d %d\n", CDMF_TEXTALIGNMENT, alignment);
  return alignment;
}

static double cdtextorientation(cdCtxCanvas *ctxcanvas, double angle)
{
  fprintf(ctxcanvas->file, "%d %g\n", CDMF_TEXTORIENTATION, angle);
  return angle;
}

static void cdpalette(cdCtxCanvas *ctxcanvas, int n, const long int *palette, int mode)
{
  int c;
  unsigned char r, g, b;

  fprintf(ctxcanvas->file, "%d %d %d\n", CDMF_PALETTE, n, mode);

  for (c = 0; c < n; c++)
  {
    cdDecodeColor(*palette++, &r, &g, &b);
    fprintf(ctxcanvas->file, "%d %d %d\n", (int)r, (int)g, (int)b);
  }
}

static long cdbackground(cdCtxCanvas *ctxcanvas, long int color)
{
  unsigned char r, g, b;
  cdDecodeColor(color, &r, &g, &b);
  fprintf(ctxcanvas->file, "%d %d %d %d\n", CDMF_BACKGROUND, (int)r, (int)g, (int)b);
  return color;
}

static long cdforeground(cdCtxCanvas *ctxcanvas, long int color)
{
  unsigned char r, g, b;
  cdDecodeColor(color, &r, &g, &b);
	fprintf(ctxcanvas->file, "%d %d %d %d\n", CDMF_FOREGROUND, (int)r, (int)g, (int)b);
  return color;
}

static void cdputimagerectrgb(cdCtxCanvas *ctxcanvas, int iw, int ih, const unsigned char *r, const unsigned char *g, const unsigned char *b, int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax)
{
  int c, l, offset;

  fprintf(ctxcanvas->file, "%d %d %d %d %d %d %d\n", CDMF_PUTIMAGERGB, iw, ih, x, y, w, h);

  offset = ymin*iw + xmin;
  r += offset;
  g += offset;
  b += offset;

  offset = iw - (xmax-xmin+1);

  for (l = ymin; l <= ymax; l++)
  {
    for (c = xmin; c <= xmax; c++)
    {
      fprintf(ctxcanvas->file, "%d %d %d ", (int)*r++, (int)*g++, (int)*b++);
    }

    r += offset;
    g += offset;
    b += offset;

    fprintf(ctxcanvas->file, "\n");
  }
}

static void cdputimagerectrgba(cdCtxCanvas *ctxcanvas, int iw, int ih, const unsigned char *r, const unsigned char *g, const unsigned char *b, const unsigned char *a, int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax)
{
  int c, l, offset;

  fprintf(ctxcanvas->file, "%d %d %d %d %d %d %d\n", CDMF_PUTIMAGERGBA, iw, ih, x, y, w, h);

  offset = ymin*iw + xmin;
  r += offset;
  g += offset;
  b += offset;
  a += offset;

  offset = iw - (xmax-xmin+1);

  for (l = ymin; l <= ymax; l++)
  {
    for (c = xmin; c <= xmax; c++)
    {
      fprintf(ctxcanvas->file, "%d %d %d %d ", (int)*r++, (int)*g++, (int)*b++, (int)*a++);
    }

    r += offset;
    g += offset;
    b += offset;
    a += offset;

    fprintf(ctxcanvas->file, "\n");
  }
}

static void cdputimagerectmap(cdCtxCanvas *ctxcanvas, int iw, int ih, const unsigned char *index, const long int *colors, int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax)
{
  int c, l, n = 0, offset;
  unsigned char r, g, b;

  fprintf(ctxcanvas->file, "%d %d %d %d %d %d %d\n", CDMF_PUTIMAGEMAP, iw, ih, x, y, w, h);

  index += ymin*iw + xmin;
  offset = iw - (xmax-xmin+1);

  for (l = ymin; l <= ymax; l++)
  {
    for (c = xmin; c <= xmax; c++)
    {
      if (*index > n)
        n = *index;

      fprintf(ctxcanvas->file, "%d ", (int)*index++);
    }

    index += offset;

    fprintf(ctxcanvas->file, "\n");
  }

  n++;

  for (c = 0; c < n; c++)
  {
    cdDecodeColor(*colors++, &r, &g, &b);
    fprintf(ctxcanvas->file, "%d %d %d\n", (int)r, (int)g, (int)b);
  }
}

static void cdpixel(cdCtxCanvas *ctxcanvas, int x, int y, long int color)
{
  unsigned char r, g, b;
  cdDecodeColor(color, &r, &g, &b);
  fprintf(ctxcanvas->file, "%d %d %d %d %d %d\n", CDMF_PIXEL, x, y, (int)r, (int)g, (int)b);
}

static void cdscrollarea(cdCtxCanvas *ctxcanvas, int xmin,int xmax, int ymin,int ymax, int dx,int dy)
{
  fprintf(ctxcanvas->file, "%d %d %d %d %d %d %d\n", CDMF_SCROLLAREA, xmin, xmax, ymin, ymax, dx, dy);
}


/**********/
/* cdPlay */
/**********/


static double factorX = 1;
static double factorY = 1;
static int offsetX = 0;
static int offsetY = 0;
static double factorS = 1;

static int sScaleX(int x)
{
  return cdRound(x * factorX + offsetX);
}

static int sScaleY(int y)
{
  return cdRound(y * factorY + offsetY);
}

static double sfScaleX(double x)
{
  return x * factorX + offsetX;
}

static double sfScaleY(double y)
{
  return y * factorY + offsetY;
}

static int sScaleW(int w)
{
  w = (int)(w * factorX + 0.5);  /* always positive */
  return w == 0? 1: w;
}

static double sfScaleH(double h)
{
  h = h * factorY;
  return h == 0? 1: h;
}

static double sfScaleW(double w)
{
  w = w * factorX;  /* always positive */
  return w == 0? 1: w;
}

static int sScaleH(int h)
{
  h = (int)(h * factorY + 0.5);
  return h == 0? 1: h;
}

static int sScaleS(int s)
{
  s = (int)(s * factorS + 0.5);
  return s == 0? 1: s;
}

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

static int cdplay(cdCanvas* canvas, int xmin, int xmax, int ymin, int ymax, void *data)
{
  char* filename = (char*)data;
  FILE* file;
  char TextBuffer[512];
  int iparam1, iparam2, iparam3, iparam4, iparam5, iparam6, iparam7, iparam8, iparam9, iparam10;
  int c, t, n, w, h, func;
  double dparam1, dparam2, dparam3, dparam4, dparam5, dparam6;
  unsigned char* stipple, * _stipple, *red, *green, *blue, *_red, *_green, *_blue, *index, *_index, *_alpha, *alpha;
  long int *pattern, *palette, *_pattern, *_palette, *colors, *_colors;
  int* dashes;
  double matrix[6];
  const char * font_family[] = 
  {
    "System",       /* CD_SYSTEM */
    "Courier",      /* CD_COURIER */
    "Times",        /* CD_TIMES_ROMAN */
    "Helvetica"     /* CD_HELVETICA */
  };
  
  file = fopen(filename, "r");
  if (!file)
    return CD_ERROR;

  func = -1;
  w = 0;
  h = 0;

  factorX = 1;
  factorY = 1;
  offsetX = 0;
  offsetY = 0;
  factorS = 1;

  fscanf(file, "%s %d %d", TextBuffer, &w, &h);

  if (strcmp(TextBuffer, "CDMF") != 0)
  {
    fclose(file);
    return CD_ERROR;
  }

  if (w>1 && h>1 && xmax!=0 && ymax!=0)
  {
    offsetX = xmin;
    offsetY = ymin;
    factorX = ((double)(xmax-xmin)) / (w-1);
    factorY = ((double)(ymax-ymin)) / (h-1);

    if (factorX < factorY)
      factorS = factorX;
    else
      factorS = factorY;
  }

  if (cdsizecb)
  {
    int err;
    err = cdsizecb(canvas, w, h, w, h);
    if (err)
      return CD_ERROR;
  }

  while (!feof(file))
  {
    fscanf(file, "%d", &func);
    if (feof(file))
      break;

    switch (func)
    {
    case CDMF_FLUSH:
      cdCanvasFlush(canvas);
      break;
    case CDMF_CLEAR:
      cdCanvasClear(canvas);
      break;
    case CDMF_CLIP:
      fscanf(file, "%d", &iparam1);
      cdCanvasClip(canvas, iparam1);
      break;
    case CDMF_CLIPAREA:
      fscanf(file, "%d %d %d %d", &iparam1, &iparam2, &iparam3, &iparam4);
      cdCanvasClipArea(canvas, sScaleX(iparam1), sScaleX(iparam2), sScaleY(iparam3), sScaleY(iparam4));
      break;
    case CDMF_FCLIPAREA:
      fscanf(file, "%lg %lg %lg %lg", &dparam1, &dparam2, &dparam3, &dparam4);
      cdfCanvasClipArea(canvas, sfScaleX(dparam1), sfScaleX(dparam2), sfScaleY(dparam3), sfScaleY(dparam4));
      break;
    case CDMF_MATRIX:
      fscanf(file, "%lg %lg %lg %lg %lg %lg", &matrix[0], &matrix[1], &matrix[2], &matrix[3], &matrix[4], &matrix[5]);
      cdCanvasTransform(canvas, matrix);
      break;
    case CDMF_RESETMATRIX:
      cdCanvasTransform(canvas, NULL);
      break;
    case CDMF_WCLIPAREA:
      fscanf(file, "%lg %lg %lg %lg", &dparam1, &dparam2, &dparam3, &dparam4);
      wdCanvasClipArea(canvas, dparam1, dparam2, dparam3, dparam4);
      break;
    case CDMF_LINE:
      fscanf(file, "%d %d %d %d", &iparam1, &iparam2, &iparam3, &iparam4);
      cdCanvasLine(canvas, sScaleX(iparam1), sScaleY(iparam2), sScaleX(iparam3), sScaleY(iparam4));
      break;
    case CDMF_FLINE:
      fscanf(file, "%lg %lg %lg %lg", &dparam1, &dparam2, &dparam3, &dparam4);
      cdfCanvasLine(canvas, sfScaleX(dparam1), sfScaleY(dparam2), sfScaleX(dparam3), sfScaleY(dparam4));
      break;
    case CDMF_WLINE:
      fscanf(file, "%lg %lg %lg %lg", &dparam1, &dparam2, &dparam3, &dparam4);
      wdCanvasLine(canvas, dparam1, dparam2, dparam3, dparam4);
      break;
    case CDMF_RECT:
      fscanf(file, "%d %d %d %d", &iparam1, &iparam2, &iparam3, &iparam4);
      cdCanvasRect(canvas, sScaleX(iparam1), sScaleX(iparam2), sScaleY(iparam3), sScaleY(iparam4));
      break;
    case CDMF_FRECT:
      fscanf(file, "%lg %lg %lg %lg", &dparam1, &dparam2, &dparam3, &dparam4);
      cdfCanvasRect(canvas, sfScaleX(dparam1), sfScaleX(dparam2), sfScaleY(dparam3), sfScaleY(dparam4));
      break;
    case CDMF_WRECT:
      fscanf(file, "%lg %lg %lg %lg", &dparam1, &dparam2, &dparam3, &dparam4);
      wdCanvasRect(canvas, dparam1, dparam2, dparam3, dparam4);
      break;
    case CDMF_BOX:
      fscanf(file, "%d %d %d %d", &iparam1, &iparam2, &iparam3, &iparam4);
      cdCanvasBox(canvas, sScaleX(iparam1), sScaleX(iparam2), sScaleY(iparam3), sScaleY(iparam4));
      break;
    case CDMF_WBOX:
      fscanf(file, "%lg %lg %lg %lg", &dparam1, &dparam2, &dparam3, &dparam4);
      wdCanvasBox(canvas, dparam1, dparam2, dparam3, dparam4);
      break;
    case CDMF_FBOX:
      fscanf(file, "%lg %lg %lg %lg", &dparam1, &dparam2, &dparam3, &dparam4);
      cdfCanvasBox(canvas, sfScaleX(dparam1), sfScaleX(dparam2), sfScaleY(dparam3), sfScaleY(dparam4));
      break;
    case CDMF_ARC:
      fscanf(file, "%d %d %d %d %lg %lg", &iparam1, &iparam2, &iparam3, &iparam4, &dparam1, &dparam2);
      cdCanvasArc(canvas, sScaleX(iparam1), sScaleY(iparam2), sScaleW(iparam3), sScaleH(iparam4), dparam1, dparam2);
      break;
    case CDMF_FARC:
      fscanf(file, "%lg %lg %lg %lg %lg %lg", &dparam1, &dparam2, &dparam3, &dparam4, &dparam5, &dparam6);
      cdfCanvasArc(canvas, sfScaleX(dparam1), sfScaleY(dparam2), sfScaleW(dparam3), sfScaleH(dparam4), dparam5, dparam6);
      break;
    case CDMF_WARC:
      fscanf(file, "%lg %lg %lg %lg %lg %lg", &dparam1, &dparam2, &dparam3, &dparam4, &dparam5, &dparam6);
      wdCanvasArc(canvas, dparam1, dparam2, dparam3, dparam4, dparam5, dparam6);
      break;
    case CDMF_SECTOR:
      fscanf(file, "%d %d %d %d %lg %lg", &iparam1, &iparam2, &iparam3, &iparam4, &dparam1, &dparam2);
      cdCanvasSector(canvas, sScaleX(iparam1), sScaleY(iparam2), sScaleW(iparam3), sScaleH(iparam4), dparam1, dparam2);
      break;
    case CDMF_FSECTOR:
      fscanf(file, "%lg %lg %lg %lg %lg %lg", &dparam1, &dparam2, &dparam3, &dparam4, &dparam5, &dparam6);
      cdfCanvasSector(canvas, sfScaleX(dparam1), sfScaleY(dparam2), sfScaleW(dparam3), sfScaleH(dparam4), dparam5, dparam6);
      break;
    case CDMF_WSECTOR:
      fscanf(file, "%lg %lg %lg %lg %lg %lg", &dparam1, &dparam2, &dparam3, &dparam4, &dparam5, &dparam6);
      wdCanvasSector(canvas, dparam1, dparam2, dparam3, dparam4, dparam5, dparam6);
      break;
    case CDMF_CHORD:
      fscanf(file, "%d %d %d %d %lg %lg", &iparam1, &iparam2, &iparam3, &iparam4, &dparam1, &dparam2);
      cdCanvasChord(canvas, sScaleX(iparam1), sScaleY(iparam2), sScaleW(iparam3), sScaleH(iparam4), dparam1, dparam2);
      break;
    case CDMF_FCHORD:
      fscanf(file, "%lg %lg %lg %lg %lg %lg", &dparam1, &dparam2, &dparam3, &dparam4, &dparam5, &dparam6);
      cdfCanvasChord(canvas, sfScaleX(dparam1), sfScaleY(dparam2), sfScaleW(dparam3), sfScaleH(dparam4), dparam5, dparam6);
      break;
    case CDMF_WCHORD:
      fscanf(file, "%lg %lg %lg %lg %lg %lg", &dparam1, &dparam2, &dparam3, &dparam4, &dparam5, &dparam6);
      wdCanvasChord(canvas, dparam1, dparam2, dparam3, dparam4, dparam5, dparam6);
      break;
    case CDMF_TEXT:
      fscanf(file, "%d %d %[^\n\r]", &iparam1, &iparam2, TextBuffer);
      cdCanvasText(canvas, sScaleX(iparam1), sScaleY(iparam2), TextBuffer);
      break;
    case CDMF_FTEXT:
      fscanf(file, "%lg %lg %[^\n\r]", &dparam1, &dparam2, TextBuffer);
      cdfCanvasText(canvas, sfScaleX(dparam1), sfScaleY(dparam2), TextBuffer);
      break;
    case CDMF_WTEXT:
      fscanf(file, "%lg %lg %[^\n\r]", &dparam1, &dparam2, TextBuffer);
      wdCanvasText(canvas, dparam1, dparam2, TextBuffer);
      break;
    case CDMF_BEGIN:
      fscanf(file, "%d", &iparam1);
      cdCanvasBegin(canvas, iparam1);
      break;
    case CDMF_VERTEX:
      fscanf(file, "%d %d", &iparam1, &iparam2);
      cdCanvasVertex(canvas, sScaleX(iparam1), sScaleY(iparam2));
      break;
    case CDMF_FVERTEX:
      fscanf(file, "%lg %lg", &dparam1, &dparam2);
      cdfCanvasVertex(canvas, sfScaleX(dparam1), sfScaleY(dparam2));
      break;
    case CDMF_WVERTEX:
      fscanf(file, "%lg %lg", &dparam1, &dparam2);
      wdCanvasVertex(canvas, dparam1, dparam2);
      break;
    case CDMF_END:
      cdCanvasEnd(canvas);
      break;
    case CDMF_MARK:
      fscanf(file, "%d %d", &iparam1, &iparam2);
      cdCanvasMark(canvas, sScaleX(iparam1), sScaleY(iparam2));
      break;
    case CDMF_WMARK:
      fscanf(file, "%lg %lg", &dparam1, &dparam2);
      wdCanvasMark(canvas, dparam1, dparam2);
      break;
    case CDMF_BACKOPACITY:
      fscanf(file, "%d", &iparam1);
      cdCanvasBackOpacity(canvas, iparam1);
      break;
    case CDMF_WRITEMODE:
      fscanf(file, "%d", &iparam1);
      cdCanvasWriteMode(canvas, iparam1);
      break;
    case CDMF_LINESTYLE:
      fscanf(file, "%d", &iparam1);
      cdCanvasLineStyle(canvas, iparam1);
      break;
    case CDMF_LINEWIDTH:
      fscanf(file, "%d", &iparam1);
      cdCanvasLineWidth(canvas, sScaleS(iparam1));
      break;
    case CDMF_LINECAP:
      fscanf(file, "%d", &iparam1);
      cdCanvasLineCap(canvas, iparam1);
      break;
    case CDMF_LINEJOIN:
      fscanf(file, "%d", &iparam1);
      cdCanvasLineJoin(canvas, iparam1);
      break;
    case CDMF_LINESTYLEDASHES:
      fscanf(file, "%d", &iparam1);
      dashes = (int*)malloc(iparam1*sizeof(int));
      for (c = 0; c < iparam1; c++)
        fscanf(file, "%d", &dashes[c]);
      cdCanvasLineStyleDashes(canvas, dashes, iparam1);
      free(dashes);
      break;
    case CDMF_FILLMODE:
      fscanf(file, "%d", &iparam1);
      cdCanvasFillMode(canvas, iparam1);
      break;
    case CDMF_INTERIORSTYLE:
      fscanf(file, "%d", &iparam1);
      cdCanvasInteriorStyle(canvas, iparam1);
      break;
    case CDMF_HATCH:
      fscanf(file, "%d", &iparam1);
      cdCanvasHatch(canvas, iparam1);
      break;
    case CDMF_STIPPLE:
      fscanf(file, "%d %d", &iparam1, &iparam2);
      t = iparam1 * iparam2;
      stipple = (unsigned char*)malloc(t);
      _stipple = stipple;
      for (c = 0; c < t; c++)
      {
        fscanf(file, "%d", &iparam3);
        *_stipple++ = (unsigned char)iparam3;
      }
      cdCanvasStipple(canvas, iparam1, iparam2, stipple);
      free(stipple);
      break;
    case CDMF_PATTERN:
      fscanf(file, "%d %d", &iparam1, &iparam2);
      t = iparam1 * iparam2;
      pattern = (long int*)malloc(t * sizeof(long));
      _pattern = pattern;
      for (c = 0; c < t; c++)
      {
        fscanf(file, "%d %d %d", &iparam3, &iparam4, &iparam5);
        *_pattern++ = cdEncodeColor((unsigned char)iparam3, (unsigned char)iparam4, (unsigned char)iparam5);
      }
      cdCanvasPattern(canvas, iparam1, iparam2, pattern);
      free(pattern);
      break;
    case CDMF_OLDFONT:
      fscanf(file, "%d %d %d", &iparam1, &iparam2, &iparam3);
      if (iparam1 < 0 || iparam1 > 3) break;
      if (iparam3 < 0)
      {
        iparam3 = -sScaleH(abs(iparam3));
        if (iparam3 > -5) iparam3 = -5;
      }
      else
      {
        iparam3 = sScaleH(abs(iparam3));
        if (iparam3 < 5) iparam3 = 5;
      }
      cdCanvasFont(canvas, font_family[iparam1], iparam2, iparam3);
      break;
    case CDMF_FONT:
      fscanf(file, "%d %d %[^\n\r]", &iparam2, &iparam3, TextBuffer);
      if (iparam3 < 0)
      {
        iparam3 = -sScaleH(abs(iparam3));
        if (iparam3 > -5) iparam3 = -5;
      }
      else
      {
        iparam3 = sScaleH(abs(iparam3));
        if (iparam3 < 5) iparam3 = 5;
      }
      cdCanvasFont(canvas, TextBuffer, iparam2, iparam3);
      break;
    case CDMF_NATIVEFONT:
      fscanf(file, "%[^\n\r]", TextBuffer);
      cdCanvasNativeFont(canvas, TextBuffer);
      break;
    case CDMF_TEXTALIGNMENT:
      fscanf(file, "%d", &iparam1);
      cdCanvasTextAlignment(canvas, iparam1);
      break;
    case CDMF_TEXTORIENTATION:
      fscanf(file, "%lg", &dparam1);
      cdCanvasTextOrientation(canvas, dparam1);
      break;
    case CDMF_MARKTYPE:
      fscanf(file, "%d", &iparam1);
      cdCanvasMarkType(canvas, iparam1);
      break;
    case CDMF_MARKSIZE:
      fscanf(file, "%d", &iparam1);
      cdCanvasMarkSize(canvas, sScaleS(iparam1));
      break;
    case CDMF_PALETTE:
      fscanf(file, "%d %d", &iparam1, &iparam2);
      _palette = palette = (long int*)malloc(iparam1);
      for (c = 0; c < iparam1; c++)
      {
        fscanf(file, "%d %d %d", &iparam3, &iparam4, &iparam5);
        *_palette++ = cdEncodeColor((unsigned char)iparam3, (unsigned char)iparam4, (unsigned char)iparam5);
      }
      cdCanvasPalette(canvas, iparam1, palette, iparam2);
      free(palette);
      break;
    case CDMF_BACKGROUND:
      fscanf(file, "%d %d %d", &iparam1, &iparam2, &iparam3);
      cdCanvasSetBackground(canvas, cdEncodeColor((unsigned char)iparam1, (unsigned char)iparam2, (unsigned char)iparam3));
      break;
    case CDMF_FOREGROUND:
      fscanf(file, "%d %d %d", &iparam1, &iparam2, &iparam3);
      cdCanvasSetForeground(canvas, cdEncodeColor((unsigned char)iparam1, (unsigned char)iparam2, (unsigned char)iparam3));
      break;
    case CDMF_PUTIMAGERGB:
      fscanf(file, "%d %d %d %d %d %d", &iparam1, &iparam2, &iparam3, &iparam4, &iparam5, &iparam6);
      t = iparam1 * iparam2;
      _red = red = (unsigned char*) malloc(t);
      _green = green = (unsigned char*) malloc(t);
      _blue = blue = (unsigned char*) malloc(t);
      for (c = 0; c < t; c++)
      {
        fscanf(file, "%d %d %d", &iparam7, &iparam8, &iparam9);
        *_red++ = (unsigned char)iparam7;
        *_green++ = (unsigned char)iparam8;
        *_blue++ = (unsigned char)iparam9;
      }
      cdCanvasPutImageRectRGB(canvas, iparam1, iparam2, red, green, blue, sScaleX(iparam3), sScaleY(iparam4), sScaleW(iparam5), sScaleH(iparam6), 0, 0, 0, 0);
      free(red);
      free(green);
      free(blue);
      break;
    case CDMF_PUTIMAGERGBA:
      fscanf(file, "%d %d %d %d %d %d", &iparam1, &iparam2, &iparam3, &iparam4, &iparam5, &iparam6);
      t = iparam1 * iparam2;
      _red = red = (unsigned char*) malloc(t);
      _green = green = (unsigned char*) malloc(t);
      _blue = blue = (unsigned char*) malloc(t);
      _alpha = alpha = (unsigned char*) malloc(t);
      for (c = 0; c < t; c++)
      {
        fscanf(file, "%d %d %d %d", &iparam7, &iparam8, &iparam9, &iparam10);
        *_red++ = (unsigned char)iparam7;
        *_green++ = (unsigned char)iparam8;
        *_blue++ = (unsigned char)iparam9;
        *_alpha++ = (unsigned char)iparam10;
      }
      cdCanvasPutImageRectRGBA(canvas, iparam1, iparam2, red, green, blue, alpha, sScaleX(iparam3), sScaleY(iparam4), sScaleW(iparam5), sScaleH(iparam6), 0, 0, 0, 0);
      free(red);
      free(green);
      free(blue);
      free(alpha);
      break;
    case CDMF_PUTIMAGEMAP:
      fscanf(file, "%d %d %d %d %d %d", &iparam1, &iparam2, &iparam3, &iparam4, &iparam5, &iparam6);
      t = iparam1 * iparam2;
      n = 0;
      _index = index = (unsigned char*) malloc(t);
      for (c = 0; c < t; c++)
      {
        fscanf(file, "%d", &iparam7);
        *_index++ = (unsigned char)iparam7;
        if (iparam7 > n)
          n = iparam7;
      }
      _colors = colors = (long int*)malloc(n);
      for (c = 0; c < n; c++)
      {
        fscanf(file, "%d %d %d", &iparam7, &iparam8, &iparam9);
        *_colors++ = cdEncodeColor((unsigned char)iparam7, (unsigned char)iparam8, (unsigned char)iparam9);
      }
      cdCanvasPutImageRectMap(canvas, iparam1, iparam2, index, colors, sScaleX(iparam3), sScaleY(iparam4), sScaleW(iparam5), sScaleH(iparam6), 0, 0, 0, 0);
      free(index);
      free(colors);
      break;
    case CDMF_PIXEL:
      fscanf(file, "%d %d %d %d %d", &iparam1, &iparam2, &iparam3, &iparam4, &iparam5);
      cdCanvasPixel(canvas, sScaleX(iparam1), sScaleY(iparam2), cdEncodeColor((unsigned char)iparam3, (unsigned char)iparam4, (unsigned char)iparam5));
      break;
    case CDMF_SCROLLAREA:
      fscanf(file, "%d %d %d %d %d %d", &iparam1, &iparam2, &iparam3, &iparam4, &iparam5, &iparam6);
      cdCanvasScrollArea(canvas, sScaleX(iparam1), sScaleX(iparam2), sScaleY(iparam3), sScaleY(iparam4), sScaleX(iparam5), sScaleY(iparam6));
      break;
    case CDMF_WVECTORTEXT:
      fscanf(file, "%lg %lg %[^\n\r]", &dparam1, &dparam2, TextBuffer);
      wdCanvasVectorText(canvas, dparam1, dparam2, TextBuffer);
      break;
    case CDMF_WMULTILINEVECTORTEXT:
      fscanf(file, "%lg %lg %[^\n\r]", &dparam1, &dparam2, TextBuffer);
      wdCanvasVectorText(canvas, dparam1, dparam2, TextBuffer);
      break;
    case CDMF_VECTORTEXT:
      fscanf(file, "%d %d %[^\n\r]", &iparam1, &iparam2, TextBuffer);
      cdCanvasVectorText(canvas, iparam1, iparam2, TextBuffer);
      break;
    case CDMF_MULTILINEVECTORTEXT:
      fscanf(file, "%d %d %[^\n\r]", &iparam1, &iparam2, TextBuffer);
      cdCanvasVectorText(canvas, iparam1, iparam2, TextBuffer);
      break;
    case CDMF_WVECTORCHARSIZE:
      fscanf(file, "%lg", &dparam1);
      wdCanvasVectorCharSize(canvas, dparam1);
      break;
    case CDMF_WVECTORTEXTSIZE:
      fscanf(file, "%lg %lg %[^\n\r]", &dparam1, &dparam2, TextBuffer);
      wdCanvasVectorTextSize(canvas, dparam1, dparam2, TextBuffer);
      break;
    case CDMF_WVECTORTEXTDIRECTION:
      fscanf(file, "%lg %lg %lg %lg", &dparam1, &dparam2, &dparam3, &dparam4);
      wdCanvasVectorTextDirection(canvas, dparam1, dparam2, dparam3, dparam4);
      break;
    case CDMF_VECTORCHARSIZE:
      fscanf(file, "%d", &iparam1);
      cdCanvasVectorCharSize(canvas, iparam1);
      break;
    case CDMF_VECTORTEXTSIZE:
      fscanf(file, "%d %d %[^\n\r]", &iparam1, &iparam2, TextBuffer);
      cdCanvasVectorTextSize(canvas, iparam1, iparam2, TextBuffer);
      break;
    case CDMF_VECTORTEXTDIRECTION:
      fscanf(file, "%d %d %d %d", &iparam1, &iparam2, &iparam3, &iparam4);
      cdCanvasVectorTextDirection(canvas, iparam1, iparam2, iparam3, iparam4);
      break;
    case CDMF_VECTORFONT:
      fscanf(file, "%[^\n\r]", TextBuffer);
      cdCanvasVectorFont(canvas, TextBuffer);
      break;
    case CDMF_VECTORTEXTTRANSFORM:
      fscanf(file, "%lg %lg %lg %lg %lg %lg", &matrix[0], &matrix[1], &matrix[2], &matrix[3], &matrix[4], &matrix[5]);
      cdCanvasVectorTextTransform(canvas, matrix);
      break;
    case CDMF_WINDOW:
      fscanf(file, "%lg %lg %lg %lg", &dparam1, &dparam2, &dparam3, &dparam4);
      wdCanvasWindow(canvas, dparam1, dparam2, dparam3, dparam4);
      break;
    default:
      fclose(file);
      return CD_ERROR;
    }
  }

  fclose(file);

  return CD_OK;
}

/*******************/
/* Canvas Creation */
/*******************/

void cdcreatecanvasMF(cdCanvas *canvas, void *data)
{
  char filename[10240] = "";
  char* strdata = (char*)data;
  double w_mm = INT_MAX*3.78, h_mm = INT_MAX*3.78, res = 3.78;
  cdCtxCanvas* ctxcanvas;
  int size;

  strdata += cdGetFileName(strdata, filename);
  if (filename[0] == 0)
    return;

  sscanf(strdata, "%lgx%lg %lg", &w_mm, &h_mm, &res);

  ctxcanvas = (cdCtxCanvas *)malloc(sizeof(cdCtxCanvas));
  memset(ctxcanvas, 0, sizeof(cdCtxCanvas));

  ctxcanvas->file = fopen(filename, "w");
  if (!ctxcanvas->file)
  {
    free(ctxcanvas);
    return;
  }

  size = strlen(filename);
  ctxcanvas->filename = malloc(size+1);
  memcpy(ctxcanvas->filename, filename, size+1);

  ctxcanvas->canvas = canvas;

  /* update canvas context */
  canvas->w = (int)(w_mm * res);
  canvas->h = (int)(h_mm * res);
  canvas->w_mm = w_mm;
  canvas->h_mm = h_mm;
  canvas->bpp = 24;
  canvas->xres = res;
  canvas->yres = res;
  canvas->ctxcanvas = ctxcanvas;

  ctxcanvas->last_line_style = -1;
  ctxcanvas->last_fill_mode = -1;

  fprintf(ctxcanvas->file, "CDMF %d %d\n", canvas->w, canvas->h);
}

void cdinittableMF(cdCanvas* canvas)
{
  canvas->cxFlush = cdflush;
  canvas->cxClear = cdclear;
  canvas->cxPixel = cdpixel;
  canvas->cxLine = cdline;
  canvas->cxPoly = cdpoly;
  canvas->cxRect = cdrect;
  canvas->cxBox = cdbox;
  canvas->cxArc = cdarc;
  canvas->cxSector = cdsector;
  canvas->cxChord = cdchord;
  canvas->cxText = cdtext;
  canvas->cxPutImageRectRGB = cdputimagerectrgb;
  canvas->cxPutImageRectRGBA = cdputimagerectrgba;
  canvas->cxPutImageRectMap = cdputimagerectmap;
  canvas->cxScrollArea = cdscrollarea;
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
  canvas->cxBackOpacity = cdbackopacity;
  canvas->cxWriteMode = cdwritemode;
  canvas->cxLineStyle = cdlinestyle;
  canvas->cxLineWidth = cdlinewidth;
  canvas->cxLineCap = cdlinecap;
  canvas->cxLineJoin = cdlinejoin;
  canvas->cxInteriorStyle = cdinteriorstyle;
  canvas->cxHatch = cdhatch;
  canvas->cxStipple = cdstipple;
  canvas->cxPattern = cdpattern;
  canvas->cxFont = cdfont;
  canvas->cxNativeFont = cdnativefont;
  canvas->cxTextAlignment = cdtextalignment;
  canvas->cxTextOrientation = cdtextorientation;
  canvas->cxPalette = cdpalette;
  canvas->cxBackground = cdbackground;
  canvas->cxForeground = cdforeground;
  canvas->cxFClipArea = cdfcliparea;
  canvas->cxTransform = cdtransform;

  canvas->cxKillCanvas = (void (*)(cdCtxCanvas*))cdkillcanvasMF;
}

static cdContext cdMetafileContext =
{
  CD_CAP_ALL & ~(CD_CAP_GETIMAGERGB | CD_CAP_IMAGESRV | 
                 CD_CAP_REGION | CD_CAP_FONTDIM | CD_CAP_TEXTSIZE),
  0,
  cdcreatecanvasMF,
  cdinittableMF,
  cdplay,
  cdregistercallback,
};

cdContext* cdContextMetafile(void)
{
  return &cdMetafileContext;
}
