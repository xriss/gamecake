/** \file
 * \brief CD DEBUG driver
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
#include "cddebug.h"


#define CDDBG_FLUSH                  "Flush"                    
#define CDDBG_CLEAR                  "Clear"                    
#define CDDBG_CLIP                   "Clip"                     
#define CDDBG_CLIPAREA               "Cliparea"                 
#define CDDBG_LINE                   "Line"                     
#define CDDBG_BOX                    "Box"                      
#define CDDBG_ARC                    "Arc"                      
#define CDDBG_SECTOR                 "Sector"                   
#define CDDBG_TEXT                   "Text"                     
#define CDDBG_BEGIN                  "Begin"                    
#define CDDBG_VERTEX                 "Vertex"                   
#define CDDBG_END                    "End"                      
#define CDDBG_PATHSET                "PathSet"
#define CDDBG_MARK                   "Mark"                     
#define CDDBG_BACKOPACITY            "BackOpacity"              
#define CDDBG_WRITEMODE              "WriteMode"                
#define CDDBG_LINESTYLE              "LineStyle"                
#define CDDBG_LINEWIDTH              "LineWidth"                
#define CDDBG_INTERIORSTYLE          "InteriorStyle"            
#define CDDBG_HATCH                  "Hatch"                    
#define CDDBG_STIPPLE                "Stipple"                  
#define CDDBG_PATTERN                "Pattern"                  
#define CDDBG_NATIVEFONT             "NativeFont"               
#define CDDBG_TEXTALIGNMENT          "TextAlignment"            
#define CDDBG_PALETTE                "Palette"                  
#define CDDBG_BACKGROUND             "Background"               
#define CDDBG_FOREGROUND             "Foreground"               
#define CDDBG_PIXEL                  "Pixel"                    
#define CDDBG_SCROLLAREA             "ScrollArea"               
#define CDDBG_TEXTORIENTATION        "TextOrientation"          
#define CDDBG_RECT                   "Rect"                     
#define CDDBG_FILLMODE               "FillMode"                 
#define CDDBG_LINESTYLEDASHES        "LineStyleDashes"          
#define CDDBG_LINECAP                "LineCap"                  
#define CDDBG_LINEJOIN               "LineJoin"                 
#define CDDBG_CHORD                  "Chord"                    
#define CDDBG_FLINE                  "fLine"                    
#define CDDBG_FRECT                  "fRect"                    
#define CDDBG_FBOX                   "fBox"                     
#define CDDBG_FARC                   "fArc"                     
#define CDDBG_FSECTOR                "fSector"                  
#define CDDBG_FTEXT                  "fText"                    
#define CDDBG_FVERTEX                "fVertex"                  
#define CDDBG_MATRIX                 "Matrix"                   
#define CDDBG_FCHORD                 "fChord"                    
#define CDDBG_FCLIPAREA              "fClipArea"                
#define CDDBG_FONT                   "Font"                     
#define CDDBG_PUTIMAGERGB            "PutImageRGB"
#define CDDBG_PUTIMAGERGBA           "PutImageRGBA"
#define CDDBG_PUTIMAGEMAP            "PutImageMap"

struct _cdCtxCanvas 
{
  cdCanvas* canvas;
  FILE* file;
  int last_line_style;
  int last_fill_mode;
};

struct _cdCtxImage {
  cdCtxCanvas *ctxcanvas;
};

static void cdflush(cdCtxCanvas *ctxcanvas)
{
  fflush(ctxcanvas->file);
  fprintf(ctxcanvas->file, "%s()\n", CDDBG_FLUSH);
}

static void cdclear(cdCtxCanvas *ctxcanvas)
{
  fprintf(ctxcanvas->file, "%s()\n", CDDBG_CLEAR);
}

static int cdclip(cdCtxCanvas *ctxcanvas, int mode)
{
  const char* enum2str[] = {
   "CD_CLIPOFF",
   "CD_CLIPAREA",
   "CD_CLIPPOLYGON",
   "CD_CLIPREGION"
  };
  fprintf(ctxcanvas->file, "%s(%s)\n", CDDBG_CLIP, enum2str[mode]);
  return mode;
}

static void cdcliparea(cdCtxCanvas *ctxcanvas, int xmin, int xmax, int ymin, int ymax)
{
  fprintf(ctxcanvas->file, "%s(%d, %d, %d, %d)\n", CDDBG_CLIPAREA, xmin, xmax, ymin, ymax);
}

static void cdfcliparea(cdCtxCanvas *ctxcanvas, double xmin, double xmax, double ymin, double ymax)
{
  fprintf(ctxcanvas->file, "%s(%g, %g, %g, %g)\n", CDDBG_FCLIPAREA, xmin, xmax, ymin, ymax);
}

static void cdtransform(cdCtxCanvas *ctxcanvas, const double* matrix)
{
  if (matrix)
    fprintf(ctxcanvas->file, "%s(%g, %g, %g, %g, %g, %g)\n", CDDBG_MATRIX, matrix[0], matrix[1], matrix[2], matrix[3], matrix[4], matrix[5]);
  else
    fprintf(ctxcanvas->file, "%s(NULL)\n", CDDBG_MATRIX);
}

static void cdline(cdCtxCanvas *ctxcanvas, int x1, int y1, int x2, int y2)
{
  fprintf(ctxcanvas->file, "%s(%d, %d, %d, %d)\n", CDDBG_LINE, x1, y1, x2, y2);
}

static void cdfline(cdCtxCanvas *ctxcanvas, double x1, double y1, double x2, double y2)
{
  fprintf(ctxcanvas->file, "%s(%g, %g, %g, %g)\n", CDDBG_FLINE, x1, y1, x2, y2);
}

static void cdrect(cdCtxCanvas *ctxcanvas, int xmin, int xmax, int ymin, int ymax)
{
  fprintf(ctxcanvas->file, "%s(%d, %d, %d, %d)\n", CDDBG_RECT, xmin, xmax, ymin, ymax);
}

static void cdfrect(cdCtxCanvas *ctxcanvas, double xmin, double xmax, double ymin, double ymax)
{
  fprintf(ctxcanvas->file, "%s(%g, %g, %g, %g)\n", CDDBG_FRECT, xmin, xmax, ymin, ymax);
}

static const char* get_region_mode(int combine_mode)
{
  const char* enum2str[] = {                        
   "CD_UNION",
   "CD_INTERSECT",
   "CD_DIFFERENCE",
   "CD_NOTINTERSECT"
  };
  return enum2str[combine_mode];
}

static void cdbox(cdCtxCanvas *ctxcanvas, int xmin, int xmax, int ymin, int ymax)
{
  if (ctxcanvas->canvas->new_region)
    fprintf(ctxcanvas->file, "%sRegion(%d, %d, %d, %d, %s)\n", CDDBG_BOX, xmin, xmax, ymin, ymax, get_region_mode(ctxcanvas->canvas->combine_mode));
  else
    fprintf(ctxcanvas->file, "%s(%d, %d, %d, %d)\n", CDDBG_BOX, xmin, xmax, ymin, ymax);
}

static void cdfbox(cdCtxCanvas *ctxcanvas, double xmin, double xmax, double ymin, double ymax)
{
  if (ctxcanvas->canvas->new_region)
    fprintf(ctxcanvas->file, "%sRegion(%g, %g, %g, %g, %s)\n", CDDBG_FBOX, xmin, xmax, ymin, ymax, get_region_mode(ctxcanvas->canvas->combine_mode));
  else
    fprintf(ctxcanvas->file, "%s(%g, %g, %g, %g)\n", CDDBG_FBOX, xmin, xmax, ymin, ymax);
}

static void cdarc(cdCtxCanvas *ctxcanvas, int xc, int yc, int w, int h, double a1, double a2)
{
  fprintf(ctxcanvas->file, "%s(%d, %d, %d, %d, %g, %g)\n", CDDBG_ARC, xc, yc, w, h, a1, a2);
}

static void cdfarc(cdCtxCanvas *ctxcanvas, double xc, double yc, double w, double h, double a1, double a2)
{
  fprintf(ctxcanvas->file, "%s(%g, %g, %g, %g, %g, %g)\n", CDDBG_FARC, xc, yc, w, h, a1, a2);
}

static void cdsector(cdCtxCanvas *ctxcanvas, int xc, int yc, int w, int h, double a1, double a2)
{
  if (ctxcanvas->canvas->new_region)
    fprintf(ctxcanvas->file, "%sRegion(%d, %d, %d, %d, %g, %g, %s)\n", CDDBG_SECTOR, xc, yc, w, h, a1, a2, get_region_mode(ctxcanvas->canvas->combine_mode));
  else
    fprintf(ctxcanvas->file, "%s(%d, %d, %d, %d, %g, %g)\n", CDDBG_SECTOR, xc, yc, w, h, a1, a2);
}

static void cdfsector(cdCtxCanvas *ctxcanvas, double xc, double yc, double w, double h, double a1, double a2)
{
  if (ctxcanvas->canvas->new_region)
    fprintf(ctxcanvas->file, "%sRegion(%g, %g, %g, %g, %g, %g, %s)\n", CDDBG_FSECTOR, xc, yc, w, h, a1, a2, get_region_mode(ctxcanvas->canvas->combine_mode));
  else
    fprintf(ctxcanvas->file, "%s(%g, %g, %g, %g, %g, %g)\n", CDDBG_FSECTOR, xc, yc, w, h, a1, a2);
}

static void cdchord(cdCtxCanvas *ctxcanvas, int xc, int yc, int w, int h, double a1, double a2)
{
  if (ctxcanvas->canvas->new_region)
    fprintf(ctxcanvas->file, "%sRegion(%d, %d, %d, %d, %g, %g, %s)\n", CDDBG_CHORD, xc, yc, w, h, a1, a2, get_region_mode(ctxcanvas->canvas->combine_mode));
  else
    fprintf(ctxcanvas->file, "%s(%d, %d, %d, %d, %g, %g)\n", CDDBG_CHORD, xc, yc, w, h, a1, a2);
}

static void cdfchord(cdCtxCanvas *ctxcanvas, double xc, double yc, double w, double h, double a1, double a2)
{
  if (ctxcanvas->canvas->new_region)
    fprintf(ctxcanvas->file, "%sRegion(%g, %g, %g, %g, %g, %g, %s)\n", CDDBG_FCHORD, xc, yc, w, h, a1, a2, get_region_mode(ctxcanvas->canvas->combine_mode));
  else
    fprintf(ctxcanvas->file, "%s(%g, %g, %g, %g, %g, %g)\n", CDDBG_FCHORD, xc, yc, w, h, a1, a2);
}

static void cdtext(cdCtxCanvas *ctxcanvas, int x, int y, const char *text, int len)
{
  text = cdStrDupN(text, len);
  if (ctxcanvas->canvas->new_region)
    fprintf(ctxcanvas->file, "%sRegion(%d, %d, \"%s\", %s)\n", CDDBG_TEXT, x, y, text, get_region_mode(ctxcanvas->canvas->combine_mode));
  else
    fprintf(ctxcanvas->file, "%s(%d, %d, \"%s\")\n", CDDBG_TEXT, x, y, text);
  free((char*)text);
}

static void cdftext(cdCtxCanvas *ctxcanvas, double x, double y, const char *text, int len)
{
  text = cdStrDupN(text, len);
  if (ctxcanvas->canvas->new_region)
    fprintf(ctxcanvas->file, "%sRegion(%g, %g, \"%s\", %s)\n", CDDBG_FTEXT, x, y, text, get_region_mode(ctxcanvas->canvas->combine_mode));
  else
    fprintf(ctxcanvas->file, "%s(%g, %g, \"%s\")\n", CDDBG_FTEXT, x, y, text);
  free((char*)text);
}

static void cdpoly(cdCtxCanvas *ctxcanvas, int mode, cdPoint* poly, int n)
{
  int i;
  const char* enum2str[] = {                        
   "CD_FILL",
   "CD_OPEN_LINES",
   "CD_CLOSED_LINES",
   "CD_CLIP",
   "CD_BEZIER",
   "CD_REGION",
   "CD_PATH"
  };

  if (mode == CD_FILL && ctxcanvas->canvas->fill_mode != ctxcanvas->last_fill_mode)
  {
    const char* enum2str[] = {                        
     "CD_EVENODD",
     "CD_WINDING"
    };
    fprintf(ctxcanvas->file, "%s(%s)\n", CDDBG_FILLMODE, enum2str[ctxcanvas->canvas->fill_mode]);
    ctxcanvas->last_fill_mode = ctxcanvas->canvas->fill_mode;
  }

  if (ctxcanvas->canvas->new_region)
    fprintf(ctxcanvas->file, "%sRegion(%s, %s)\n", CDDBG_BEGIN, enum2str[mode], get_region_mode(ctxcanvas->canvas->combine_mode));
  else
    fprintf(ctxcanvas->file, "%s(%s)\n", CDDBG_BEGIN, enum2str[mode]);

  if (mode == CD_PATH)
  {
    const char* path2str[] = {                        
     "CD_PATH_NEW",
     "CD_PATH_MOVETO",
     "CD_PATH_LINETO",
     "CD_PATH_ARC",
     "CD_PATH_CURVETO",
     "CD_PATH_CLOSE",
     "CD_PATH_FILL",
     "CD_PATH_STROKE",
     "CD_PATH_FILLSTROKE",
     "CD_PATH_CLIP"
    };
    int p;

    i = 0;
    for (p=0; p<ctxcanvas->canvas->path_n; p++)
    {
      fprintf(ctxcanvas->file, "%s(%s)\n", CDDBG_PATHSET, path2str[ctxcanvas->canvas->path[p]]);

      switch(ctxcanvas->canvas->path[p])
      {
      case CD_PATH_MOVETO:
      case CD_PATH_LINETO:
        if (i+1 > n) 
        {
          fprintf(ctxcanvas->file, "ERROR: not enough points in path\n");
          return;
        }
        fprintf(ctxcanvas->file, "%s(%d, %d)\n", CDDBG_VERTEX, poly[i].x, poly[i].y);
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
          fprintf(ctxcanvas->file, "%s(%d, %d)\n", CDDBG_VERTEX, poly[i].x, poly[i].y);
          fprintf(ctxcanvas->file, "%s(%d, %d)\n", CDDBG_VERTEX, poly[i+1].x, poly[i+1].y);
          fprintf(ctxcanvas->file, "%s(%d, %d)\n", CDDBG_VERTEX, poly[i+2].x, poly[i+2].y);
          i += 3;
        }
        break;
      }
    }
  }
  else
  {
    for(i = 0; i<n; i++)
      fprintf(ctxcanvas->file, "%s(%d, %d)\n", CDDBG_VERTEX, poly[i].x, poly[i].y);
  }

  fprintf(ctxcanvas->file, "%s()\n", CDDBG_END);
}

static void cdfpoly(cdCtxCanvas *ctxcanvas, int mode, cdfPoint* poly, int n)
{
  int i;
  const char* enum2str[] = {                        
   "CD_FILL",
   "CD_OPEN_LINES",
   "CD_CLOSED_LINES",
   "CD_CLIP",
   "CD_BEZIER",
   "CD_REGION"
  };

  if (mode == CD_FILL && ctxcanvas->canvas->fill_mode != ctxcanvas->last_fill_mode)
  {
    const char* enum2str[] = {                        
     "CD_EVENODD",
     "CD_WINDING"
    };
    fprintf(ctxcanvas->file, "%s(%s)\n", CDDBG_FILLMODE, enum2str[ctxcanvas->canvas->fill_mode]);
    ctxcanvas->last_fill_mode = ctxcanvas->canvas->fill_mode;
  }

  fprintf(ctxcanvas->file, "%s(%s)\n", CDDBG_BEGIN, enum2str[mode]);

  if (mode == CD_PATH)
  {
    const char* path2str[] = {                        
     "CD_PATH_NEW",
     "CD_PATH_MOVETO",
     "CD_PATH_LINETO",
     "CD_PATH_ARC",
     "CD_PATH_CURVETO",
     "CD_PATH_CLOSE",
     "CD_PATH_FILL",
     "CD_PATH_STROKE",
     "CD_PATH_FILLSTROKE",
     "CD_PATH_CLIP"
    };
    int p;

    i = 0;
    for (p=0; p<ctxcanvas->canvas->path_n; p++)
    {
      fprintf(ctxcanvas->file, "%s(%s)\n", CDDBG_PATHSET, path2str[ctxcanvas->canvas->path[p]]);

      switch(ctxcanvas->canvas->path[p])
      {
      case CD_PATH_MOVETO:
      case CD_PATH_LINETO:
        if (i+1 > n) 
        {
          fprintf(ctxcanvas->file, "ERROR: not enough points in path\n");
          return;
        }
        fprintf(ctxcanvas->file, "%s(%g, %g)\n", CDDBG_VERTEX, poly[i].x, poly[i].y);
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
          fprintf(ctxcanvas->file, "%s(%g, %g)\n", CDDBG_VERTEX, poly[i].x, poly[i].y);
          fprintf(ctxcanvas->file, "%s(%g, %g)\n", CDDBG_VERTEX, poly[i+1].x, poly[i+1].y);
          fprintf(ctxcanvas->file, "%s(%g, %g)\n", CDDBG_VERTEX, poly[i+2].x, poly[i+2].y);
          i += 3;
        }
        break;
      }
    }
  }
  else
  {
    for(i = 0; i<n; i++)
      fprintf(ctxcanvas->file, "%s(%g, %g)\n", CDDBG_FVERTEX, poly[i].x, poly[i].y);
  }

  fprintf(ctxcanvas->file, "%s()\n", CDDBG_END);
}

static int cdbackopacity(cdCtxCanvas *ctxcanvas, int opacity)
{
  const char* enum2str[] = {                        
   "CD_OPAQUE",
   "CD_TRANSPARENT"
  };
  fprintf(ctxcanvas->file, "%s(%s)\n", CDDBG_BACKOPACITY, enum2str[opacity]);
  return opacity;
}

static int cdwritemode(cdCtxCanvas *ctxcanvas, int mode)
{
  const char* enum2str[] = {                        
   "CD_REPLACE",
   "CD_XOR",
   "CD_NOT_XOR"
  };
  fprintf(ctxcanvas->file, "%s(%s)\n", CDDBG_WRITEMODE, enum2str[mode]);
  return mode;
}

static int cdlinestyle(cdCtxCanvas *ctxcanvas, int style)
{
  const char* enum2str[] = {                        
   "CD_CONTINUOUS",
   "CD_DASHED",
   "CD_DOTTED",
   "CD_DASH_DOT",
   "CD_DASH_DOT_DOT",
   "CD_CUSTOM"
  };

  if (style == CD_CUSTOM && ctxcanvas->canvas->line_style != ctxcanvas->last_line_style)
  {
    int i;

    fprintf(ctxcanvas->file, "%s(%d", CDDBG_LINESTYLEDASHES, ctxcanvas->canvas->line_dashes_count);
    for (i = 0; i < ctxcanvas->canvas->line_dashes_count; i++)
      fprintf(ctxcanvas->file, ", %d", ctxcanvas->canvas->line_dashes[i]);
    fprintf(ctxcanvas->file, ")\n");

    ctxcanvas->last_line_style = ctxcanvas->canvas->line_style;
  }

  fprintf(ctxcanvas->file, "%s(%s)\n", CDDBG_LINESTYLE, enum2str[style]);
  return style;
}

static int cdlinewidth(cdCtxCanvas *ctxcanvas, int width)
{
  fprintf(ctxcanvas->file, "%s(%d)\n", CDDBG_LINEWIDTH, width);
  return width;
}

static int cdlinecap(cdCtxCanvas *ctxcanvas, int cap)
{
  const char* enum2str[] = {                        
   "CD_CAPFLAT",  
   "CD_CAPSQUARE",
   "CD_CAPROUND"
  };  
  fprintf(ctxcanvas->file, "%s(%s)\n", CDDBG_LINECAP, enum2str[cap]);
  return cap;
}

static int cdlinejoin(cdCtxCanvas *ctxcanvas, int join)
{
  const char* enum2str[] = {                        
   "CD_MITER",
   "CD_BEVEL",
   "CD_ROUND"
  };  
  fprintf(ctxcanvas->file, "%s(%s)\n", CDDBG_LINEJOIN, enum2str[join]);
  return join;
}

static int cdinteriorstyle(cdCtxCanvas *ctxcanvas, int style)
{
  const char* enum2str[] = {                        
   "CD_SOLID",
   "CD_HATCH",
   "CD_STIPPLE",
   "CD_PATTERN",
   "CD_HOLLOW"
  };
  fprintf(ctxcanvas->file, "%s(%s)\n", CDDBG_INTERIORSTYLE, enum2str[style]);
  return style;
}

static int cdhatch(cdCtxCanvas *ctxcanvas, int style)
{
  const char* enum2str[] = {                        
   "CD_HORIZONTAL",
   "CD_VERTICAL",
   "CD_FDIAGONAL",
   "CD_BDIAGONAL",
   "CD_CROSS",
   "CD_DIAGCROSS"
  };
  fprintf(ctxcanvas->file, "%s(%s)\n", CDDBG_HATCH, enum2str[style]);
  return style;
}

static void cdstipple(cdCtxCanvas *ctxcanvas, int w, int h, const unsigned char *stipple)
{
  fprintf(ctxcanvas->file, "%s(%d, %d, %p)\n", CDDBG_STIPPLE, w, h, stipple);
}

static void cdpattern(cdCtxCanvas *ctxcanvas, int w, int h, const long int *pattern)
{
  fprintf(ctxcanvas->file, "%s(%d, %d, %p)\n", CDDBG_PATTERN, w, h, pattern);
}

static int cdfont(cdCtxCanvas *ctxcanvas, const char* type_face, int style, int size)
{
  char style_str[50] = "";
  if (style & CD_BOLD) 
    strcat(style_str, "CD_BOLD");
  if (style & CD_ITALIC) 
  {
    if (style_str[0]!=0) strcat(style_str, "|"); 
    strcat(style_str, "CD_ITALIC");
  }
  if (style & CD_UNDERLINE) 
  {
    if (style_str[0]!=0) strcat(style_str, "|"); 
    strcat(style_str, "CD_UNDERLINE");
  }
  if (style & CD_STRIKEOUT) 
  {
    if (style_str[0]!=0) strcat(style_str, "|"); 
    strcat(style_str, "CD_STRIKEOUT");
  }
  if (style_str[0]==0) strcat(style_str, "CD_PLAIN"); 
  fprintf(ctxcanvas->file, "%s(\"%s\", %s, %d)\n", CDDBG_FONT, type_face, style_str, size);
  return 1;
}

static int cdnativefont(cdCtxCanvas *ctxcanvas, const char* font)
{
  fprintf(ctxcanvas->file, "%s(\"%s\")\n", CDDBG_NATIVEFONT, font);
  return 1;
}

static int cdtextalignment(cdCtxCanvas *ctxcanvas, int alignment)
{
  const char* enum2str[] = {                        
   "CD_NORTH",
   "CD_SOUTH",
   "CD_EAST",
   "CD_WEST",
   "CD_NORTH_EAST",
   "CD_NORTH_WEST",
   "CD_SOUTH_EAST",
   "CD_SOUTH_WEST",
   "CD_CENTER",
   "CD_BASE_LEFT",
   "CD_BASE_CENTER",
   "CD_BASE_RIGHT"
  };
  fprintf(ctxcanvas->file, "%s(%s)\n", CDDBG_TEXTALIGNMENT, enum2str[alignment]);
  return alignment;
}

static double cdtextorientation(cdCtxCanvas *ctxcanvas, double angle)
{
  fprintf(ctxcanvas->file, "%s(%g)\n", CDDBG_TEXTORIENTATION, angle);
  return angle;
}

static void cdpalette(cdCtxCanvas *ctxcanvas, int n, const long int *palette, int mode)
{
  const char* enum2str[] = {                        
   "CD_POLITE",
   "CD_FORCE"
  };
  fprintf(ctxcanvas->file, "%s(%d, %p, %s)\n", CDDBG_PALETTE, n, palette, enum2str[mode]);
}

static long cdbackground(cdCtxCanvas *ctxcanvas, long int color)
{
  unsigned char r, g, b;
  cdDecodeColor(color, &r, &g, &b);
  fprintf(ctxcanvas->file, "%s(%d, %d, %d)\n", CDDBG_BACKGROUND, (int)r, (int)g, (int)b);
  return color;
}

static long cdforeground(cdCtxCanvas *ctxcanvas, long int color)
{
  unsigned char r, g, b;
  cdDecodeColor(color, &r, &g, &b);
	fprintf(ctxcanvas->file, "%s(%d, %d, %d)\n", CDDBG_FOREGROUND, (int)r, (int)g, (int)b);
  return color;
}

static void cdputimagerectrgb(cdCtxCanvas *ctxcanvas, int iw, int ih, const unsigned char *r, const unsigned char *g, const unsigned char *b, int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax)
{
  fprintf(ctxcanvas->file, "%s(%d, %d, %p, %p, %p, %d, %d, %d, %d, %d, %d, %d, %d)\n", CDDBG_PUTIMAGERGB, iw, ih, r, g, b, x, y, w, h, xmin, xmax, ymin, ymax);
}

static void cdputimagerectrgba(cdCtxCanvas *ctxcanvas, int iw, int ih, const unsigned char *r, const unsigned char *g, const unsigned char *b, const unsigned char *a, int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax)
{
  fprintf(ctxcanvas->file, "%s(%d, %d, %p, %p, %p, %p, %d, %d, %d, %d, %d, %d, %d, %d)\n", CDDBG_PUTIMAGERGBA, iw, ih, r, g, b, a, x, y, w, h, xmin, xmax, ymin, ymax);
}

static void cdputimagerectmap(cdCtxCanvas *ctxcanvas, int iw, int ih, const unsigned char *index, const long int *colors, int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax)
{
  fprintf(ctxcanvas->file, "%s(%d, %d, %p, %p, %d, %d, %d, %d, %d, %d, %d, %d)\n", CDDBG_PUTIMAGEMAP, iw, ih, index, colors, x, y, w, h, xmin, xmax, ymin, ymax);
}

static void cdpixel(cdCtxCanvas *ctxcanvas, int x, int y, long int color)
{
  unsigned char r, g, b;
  cdDecodeColor(color, &r, &g, &b);
  fprintf(ctxcanvas->file, "%s(%d, %d, %d, %d, %d)\n", CDDBG_PIXEL, x, y, (int)r, (int)g, (int)b);
}

static void cdscrollarea(cdCtxCanvas *ctxcanvas, int xmin,int xmax, int ymin,int ymax, int dx,int dy)
{
  fprintf(ctxcanvas->file, "%s(%d, %d, %d, %d, %d, %d)\n", CDDBG_SCROLLAREA, xmin, xmax, ymin, ymax, dx, dy);
}

static void cdgetimagergb(cdCtxCanvas* ctxcanvas, unsigned char *r, unsigned char *g, unsigned char *b, int x, int y, int w, int h)
{
	fprintf(ctxcanvas->file, "%p, %p, %p = GetImageRGB(%d, %d, %d, %d)\n", r, g, b, x, y, w, h);
}

static cdCtxImage* cdcreateimage(cdCtxCanvas* ctxcanvas, int w, int h)
{
  cdCtxImage* ctximage = malloc(sizeof(cdCtxImage));
  ctximage->ctxcanvas = ctxcanvas;
	fprintf(ctxcanvas->file, "%p = GetImage(%d, %d)\n", ctximage, w, h);
  return ctximage;
}

static void cdkillimage(cdCtxImage* ctximage)
{
	fprintf(ctximage->ctxcanvas->file, "KillImage(%p)\n", ctximage);
  free(ctximage);
}

static void cdgetimage(cdCtxCanvas* ctxcanvas, cdCtxImage* ctximage, int x, int y)
{
	fprintf(ctxcanvas->file, "GetImage(%p, %d, %d)\n", ctximage, x, y);
}

static void cdputimagerect(cdCtxCanvas* ctxcanvas, cdCtxImage* ctximage, int x, int y, int xmin, int xmax, int ymin, int ymax)
{
	fprintf(ctxcanvas->file, "PutImage(%p, %d, %d, %d, %d, %d, %d)\n", ctximage, x, y, xmin, xmax, ymin, ymax);
}

static void cdnewregion(cdCtxCanvas* ctxcanvas)
{
	fprintf(ctxcanvas->file, "NewRegion()\n");
}

static int cdispointinregion(cdCtxCanvas* ctxcanvas, int x, int y)
{
	fprintf(ctxcanvas->file, "IsPointInRegion(%d, %d)\n", x, y);
  return 0;
}

static void cdoffsetregion(cdCtxCanvas* ctxcanvas, int x, int y)
{
	fprintf(ctxcanvas->file, "OffsetRegion(%d, %d)\n", x, y);
}

static void cdgetregionbox(cdCtxCanvas* ctxcanvas, int *xmin, int *xmax, int *ymin, int *ymax)
{
  (void)xmin;
  (void)ymin;
  (void)xmax;
  (void)ymax;
	fprintf(ctxcanvas->file, "GetRegionBox()\n");
}

static int cdactivate(cdCtxCanvas* ctxcanvas)
{
	fprintf(ctxcanvas->file, "Activate()\n");
  return CD_OK;
}

static void cddeactivate(cdCtxCanvas* ctxcanvas)
{
	fprintf(ctxcanvas->file, "Deactivate()\n");
}

static void cdgetfontdim(cdCtxCanvas* ctxcanvas, int *max_width, int *height, int *ascent, int *descent)
{
  int tmp_max_width, tmp_height, tmp_ascent, tmp_descent;
  if (!max_width) max_width = &tmp_max_width;
  if (!height) height = &tmp_height;
  if (!ascent) ascent = &tmp_ascent;
  if (!descent) descent = &tmp_descent;
  cdgetfontdimEX(ctxcanvas, max_width, height, ascent, descent);
	fprintf(ctxcanvas->file, "%d, %d, %d, %d = GetFontDim()\n", *max_width, *height, *ascent, *descent);
}

static void cdgettextsize(cdCtxCanvas* ctxcanvas, const char *s, int len, int *width, int *height)
{
  int tmp_width, tmp_height;
  if (!width) width = &tmp_width;
  if (!height) height = &tmp_height;
  cdgettextsizeEX(ctxcanvas, s, len, width, height);
	fprintf(ctxcanvas->file, "%d, %d = GetTextSize(\"%s\")\n", *width, *height, s);
}

static void cdkillcanvas(cdCtxCanvas *ctxcanvas)
{
	fprintf(ctxcanvas->file, "KillCanvas()\n");
  fclose(ctxcanvas->file);
  memset(ctxcanvas, 0, sizeof(cdCtxCanvas));
  free(ctxcanvas);
}

static void cdcreatecanvas(cdCanvas *canvas, void *data)
{
  char filename[10240] = "";
  char* strdata = (char*)data;
  double w_mm = INT_MAX*3.78, h_mm = INT_MAX*3.78, res = 3.78;
  cdCtxCanvas* ctxcanvas;

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

  fprintf(ctxcanvas->file, "CreateCanvas(CD_DEBUG, \"%s\")\n", (char*)data);
}

static void cdinittable(cdCanvas* canvas)
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
  canvas->cxKillCanvas = cdkillcanvas;
  canvas->cxGetImageRGB = cdgetimagergb;
  canvas->cxScrollArea = cdscrollarea;
  canvas->cxCreateImage = cdcreateimage;
  canvas->cxKillImage = cdkillimage;
  canvas->cxGetImage = cdgetimage;
  canvas->cxPutImageRect = cdputimagerect;
  canvas->cxNewRegion = cdnewregion;
  canvas->cxIsPointInRegion = cdispointinregion;
  canvas->cxOffsetRegion = cdoffsetregion;
  canvas->cxGetRegionBox = cdgetregionbox;
  canvas->cxActivate = cdactivate;
  canvas->cxDeactivate = cddeactivate;
  canvas->cxGetFontDim = cdgetfontdim;
  canvas->cxGetTextSize = cdgettextsize;
}

static cdContext cdDebugContext =
{
  CD_CAP_ALL,
  0,
  cdcreatecanvas,
  cdinittable,
  NULL,
  NULL,
};

cdContext* cdContextDebug(void)
{
  return &cdDebugContext;
}

