/** \file
 * \brief DXF driver
 *
 * See Copyright Notice in cd.h
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>

#include "cd.h"
#include "cd_private.h"
#include "cddxf.h"


#ifndef ignore
#define ignore(x) (void)x
#endif

#ifndef max
#define max(x, y) ((x > y)? x : y)
#endif

#ifndef min
#define min(x, y) ((x < y)? x : y)
#endif

struct _cdCtxCanvas
{
  cdCanvas* canvas;

  FILE *file;                  /* pointer to file                        */
  int layer;                      /* layer                                  */

  int tf;                         /* text font                              */
  double th;                      /* text height (in points)                */
  int toa;                        /* text oblique angle (for italics)       */
  int tha, tva;                   /* text horizontal and vertical alignment */

  int fgcolor;                    /* foreground AutoCAD palette color       */

  int lt;                         /* line type                              */
  double lw;                      /* line width (in milimeters)             */
};


static void wnamline (cdCtxCanvas* ctxcanvas, int t)   /* write name of a line */
{

  static char *line[] =
  {"CONTINUOUS",
   "DASHED",
   "HIDDEN",
   "CENTER",
   "PHANTOM",
   "DOT",
   "DASHDOT",
   "BORDER",
   "DIVIDE"};

/*
   AutoCAD line styles ( see acad.lin ):

   0 CONTINUOUS  ____________________________________________
   1 DASHED      __ __ __ __ __ __ __ __ __ __ __ __ __ __ __
   2 HIDDEN      _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _
   3 CENTER      ____ _ ____ _ ____ _ ____ _ ____ _ ____ _ __
   4 PHANTOM     _____ _ _ _____ _ _ _____ _ _ _____ _ _ ____
   5 DOT         ............................................
   6 DASHDOT     __ . __ . __ . __ . __ . __ . __ . __ . __ .
   7 BORDER      __ __ . __ __ . __ __ . __ __ . __ __ . __ _
   8 DIVIDE      __ . . __ . . __ . . __ . . __ . . __ . . __

*/

  fprintf (ctxcanvas->file, "%s\n", line[t]);
}


static void wnamfont (cdCtxCanvas *ctxcanvas, int t)   /* write name of a font */
{
  static char *font[] =
  {
    "STANDARD",
    "ROMAN",
    "ROMAN_BOLD",
    "ROMANTIC",
    "ROMANTIC_BOLD",
    "SANSSERIF",
    "SANSSERIF_BOLD",
  };
/*
             CD Fonts / Style                 AutoCAD Fonts
      -------------------------------------------------------------------
       CD_SYSTEM                           0 STANDARD
       CD_COURIER     / CD_PLAIN           1 ROMAN
       CD_COURIER     / CD_BOLD            2 ROMAN_BOLD
       CD_COURIER     / CD_ITALIC          1 ROMAN         (ctxcanvas->toa = 15)
       CD_COURIER     / CD_BOLD_ITALIC     2 ROMAN_BOLD    (ctxcanvas->toa = 15)
       CD_TIMES_ROMAN / CD_PLAIN           3 ROMANTIC
       CD_TIMES_ROMAN / CD_BOLD            4 ROMANTIC_BOLD
       CD_TIMES_ROMAN / CD_ITALIC          3 ROMANTIC      (ctxcanvas->toa = 15)
       CD_TIMES_ROMAN / CD_BOLD_ITALIC     4 ROMANTIC_BOLD (ctxcanvas->toa = 15)
       CD_HELVETICA   / CD_PLAIN           5 SANSSERIF
       CD_HELVETICA   / CD_BOLD            6 SANSSERIF_BOLD
       CD_HELVETICA   / CD_ITALIC          5 SANSSERIF     (ctxcanvas->toa = 15)
       CD_HELVETICA   / CD_BOLD_ITALIC     6 SANSSERIF_BOLD(ctxcanvas->toa = 15)
*/

  fprintf (ctxcanvas->file, "%s\n", font[t]);
}


static void writepoly (cdCtxCanvas *ctxcanvas, cdPoint *poly, int nv) /* write polygon */
{
  int i;

  fprintf ( ctxcanvas->file, "0\n" );
  fprintf ( ctxcanvas->file, "POLYLINE\n" );
  fprintf ( ctxcanvas->file, "8\n" );
  fprintf ( ctxcanvas->file, "%d\n", ctxcanvas->layer); /* current layer */

  fprintf ( ctxcanvas->file, "6\n" );
  wnamline( ctxcanvas, ctxcanvas->lt );                    /* line type */

  fprintf ( ctxcanvas->file, "62\n" );
  fprintf ( ctxcanvas->file, "%d\n", ctxcanvas->fgcolor );
  fprintf ( ctxcanvas->file, "66\n" );
  fprintf ( ctxcanvas->file, "1\n" );
  fprintf ( ctxcanvas->file, "40\n" );
  fprintf ( ctxcanvas->file, "%f\n", ctxcanvas->lw/ctxcanvas->canvas->xres );
  fprintf ( ctxcanvas->file, "41\n" );           /* entire polygon line width */
  fprintf ( ctxcanvas->file, "%f\n", ctxcanvas->lw/ctxcanvas->canvas->xres );
  for ( i=0; i<nv; i++ )
  {
    fprintf ( ctxcanvas->file, "0\n" );
    fprintf ( ctxcanvas->file, "VERTEX\n" );
    fprintf ( ctxcanvas->file, "8\n" );
    fprintf ( ctxcanvas->file, "%d\n", ctxcanvas->layer); /* current layer */
    fprintf ( ctxcanvas->file, "10\n" );
    fprintf ( ctxcanvas->file, "%f\n", poly[i].x/ctxcanvas->canvas->xres );
    fprintf ( ctxcanvas->file, "20\n" );
    fprintf ( ctxcanvas->file, "%f\n", poly[i].y/ctxcanvas->canvas->xres );
  }
  fprintf ( ctxcanvas->file, "0\n" );
  fprintf ( ctxcanvas->file, "SEQEND\n" );
}

static void writepolyf (cdCtxCanvas *ctxcanvas, cdfPoint *poly, int nv) /* write polygon */
{
  int i;

  fprintf ( ctxcanvas->file, "0\n" );
  fprintf ( ctxcanvas->file, "POLYLINE\n" );
  fprintf ( ctxcanvas->file, "8\n" );
  fprintf ( ctxcanvas->file, "%d\n", ctxcanvas->layer); /* current layer */

  fprintf ( ctxcanvas->file, "6\n" );
  wnamline( ctxcanvas, ctxcanvas->lt );                    /* line type */

  fprintf ( ctxcanvas->file, "62\n" );
  fprintf ( ctxcanvas->file, "%d\n", ctxcanvas->fgcolor );
  fprintf ( ctxcanvas->file, "66\n" );
  fprintf ( ctxcanvas->file, "1\n" );
  fprintf ( ctxcanvas->file, "40\n" );
  fprintf ( ctxcanvas->file, "%f\n", ctxcanvas->lw/ctxcanvas->canvas->xres );
  fprintf ( ctxcanvas->file, "41\n" );           /* entire polygon line width */
  fprintf ( ctxcanvas->file, "%f\n", ctxcanvas->lw/ctxcanvas->canvas->xres );
  for ( i=0; i<nv; i++ )
  {
    fprintf ( ctxcanvas->file, "0\n" );
    fprintf ( ctxcanvas->file, "VERTEX\n" );
    fprintf ( ctxcanvas->file, "8\n" );
    fprintf ( ctxcanvas->file, "%d\n", ctxcanvas->layer); /* current layer */
    fprintf ( ctxcanvas->file, "10\n" );
    fprintf ( ctxcanvas->file, "%f\n", poly[i].x/ctxcanvas->canvas->xres );
    fprintf ( ctxcanvas->file, "20\n" );
    fprintf ( ctxcanvas->file, "%f\n", poly[i].y/ctxcanvas->canvas->xres );
  }
  fprintf ( ctxcanvas->file, "0\n" );
  fprintf ( ctxcanvas->file, "SEQEND\n" );
}

static void writehatch (cdCtxCanvas *ctxcanvas, cdPoint *poly, int nv) /* write polygon */
{
  int i;

  fprintf ( ctxcanvas->file, "0\n" );
  fprintf ( ctxcanvas->file, "HATCH\n" );
  fprintf ( ctxcanvas->file, "8\n" );
  fprintf ( ctxcanvas->file, "%d\n", ctxcanvas->layer); /* current layer */
  fprintf ( ctxcanvas->file, "62\n" );
  fprintf ( ctxcanvas->file, "%d\n", ctxcanvas->fgcolor );

  fprintf ( ctxcanvas->file, "10\n" );
  fprintf ( ctxcanvas->file, "0.0\n" );
  fprintf ( ctxcanvas->file, "20\n" );
  fprintf ( ctxcanvas->file, "0.0\n" );
  fprintf ( ctxcanvas->file, "30\n" );
  fprintf ( ctxcanvas->file, "0.0\n" );

  fprintf ( ctxcanvas->file, "210\n" );
  fprintf ( ctxcanvas->file, "0.0\n" );
  fprintf ( ctxcanvas->file, "220\n" );
  fprintf ( ctxcanvas->file, "0.0\n" );
  fprintf ( ctxcanvas->file, "230\n" );
  fprintf ( ctxcanvas->file, "1.0\n" );

  fprintf ( ctxcanvas->file, "2\n" );
  fprintf ( ctxcanvas->file, "SOLID\n" );
  fprintf ( ctxcanvas->file, "70\n" );
  fprintf ( ctxcanvas->file, "1\n" );
  fprintf ( ctxcanvas->file, "71\n" );
  fprintf ( ctxcanvas->file, "0\n" );
  fprintf ( ctxcanvas->file, "91\n" );
  fprintf ( ctxcanvas->file, "1\n" );

  fprintf ( ctxcanvas->file, "92\n" );
  fprintf ( ctxcanvas->file, "2\n" );
  fprintf ( ctxcanvas->file, "72\n" );
  fprintf ( ctxcanvas->file, "1\n" );

  fprintf ( ctxcanvas->file, "73\n" );
  fprintf ( ctxcanvas->file, "1\n" );
  fprintf ( ctxcanvas->file, "93\n" );           /* entire polygon line width */
  fprintf ( ctxcanvas->file, "%d\n", nv );
  for ( i=0; i<nv; i++ )
  {
    fprintf ( ctxcanvas->file, "10\n" );
    fprintf ( ctxcanvas->file, "%f\n", poly[i].x/ctxcanvas->canvas->xres );
    fprintf ( ctxcanvas->file, "20\n" );
    fprintf ( ctxcanvas->file, "%f\n", poly[i].y/ctxcanvas->canvas->xres );
  }

  fprintf ( ctxcanvas->file, "97\n" );
  fprintf ( ctxcanvas->file, "0\n" );
  fprintf ( ctxcanvas->file, "75\n" );
  fprintf ( ctxcanvas->file, "0\n" );
  fprintf ( ctxcanvas->file, "76\n" );
  fprintf ( ctxcanvas->file, "1\n" );
}

static void writehatchf (cdCtxCanvas *ctxcanvas, cdfPoint *poly, int nv) /* write polygon */
{
  int i;

  fprintf ( ctxcanvas->file, "0\n" );
  fprintf ( ctxcanvas->file, "HATCH\n" );
  fprintf ( ctxcanvas->file, "8\n" );
  fprintf ( ctxcanvas->file, "%d\n", ctxcanvas->layer); /* current layer */
  fprintf ( ctxcanvas->file, "62\n" );
  fprintf ( ctxcanvas->file, "%d\n", ctxcanvas->fgcolor );

  fprintf ( ctxcanvas->file, "10\n" );
  fprintf ( ctxcanvas->file, "0.0\n" );
  fprintf ( ctxcanvas->file, "20\n" );
  fprintf ( ctxcanvas->file, "0.0\n" );
  fprintf ( ctxcanvas->file, "30\n" );
  fprintf ( ctxcanvas->file, "0.0\n" );

  fprintf ( ctxcanvas->file, "210\n" );
  fprintf ( ctxcanvas->file, "0.0\n" );
  fprintf ( ctxcanvas->file, "220\n" );
  fprintf ( ctxcanvas->file, "0.0\n" );
  fprintf ( ctxcanvas->file, "230\n" );
  fprintf ( ctxcanvas->file, "1.0\n" );

  fprintf ( ctxcanvas->file, "2\n" );
  fprintf ( ctxcanvas->file, "SOLID\n" );
  fprintf ( ctxcanvas->file, "70\n" );
  fprintf ( ctxcanvas->file, "1\n" );
  fprintf ( ctxcanvas->file, "71\n" );
  fprintf ( ctxcanvas->file, "0\n" );
  fprintf ( ctxcanvas->file, "91\n" );           /* entire polygon line width */
  fprintf ( ctxcanvas->file, "1\n" );

  fprintf ( ctxcanvas->file, "92\n" );
  fprintf ( ctxcanvas->file, "2\n" );
  fprintf ( ctxcanvas->file, "72\n" );
  fprintf ( ctxcanvas->file, "1\n" );

  fprintf ( ctxcanvas->file, "73\n" );
  fprintf ( ctxcanvas->file, "1\n" );
  fprintf ( ctxcanvas->file, "93\n" );           /* entire polygon line width */
  fprintf ( ctxcanvas->file, "%d\n", nv );
  for ( i=0; i<nv; i++ )
  {
    fprintf ( ctxcanvas->file, "10\n" );
    fprintf ( ctxcanvas->file, "%f\n", poly[i].x/ctxcanvas->canvas->xres );
    fprintf ( ctxcanvas->file, "20\n" );
    fprintf ( ctxcanvas->file, "%f\n", poly[i].y/ctxcanvas->canvas->xres );
  }
  fprintf ( ctxcanvas->file, "97\n" );
  fprintf ( ctxcanvas->file, "0\n" );
  fprintf ( ctxcanvas->file, "75\n" );
  fprintf ( ctxcanvas->file, "0\n" );
  fprintf ( ctxcanvas->file, "76\n" );
  fprintf ( ctxcanvas->file, "1\n" );
}

static void deflines (cdCtxCanvas *ctxcanvas)    /* define lines */
{
  int i, j;
  static char *line[] =
  {"Solid line",
   "Dashed line",
   "Hidden line",
   "Center line",
   "Phantom line",
   "Dot line",
   "Dashdot line",
   "Border line",
   "Divide Line"};

#define TABSIZE (sizeof(tab)/sizeof(tab[0]))

  static int tab[][8] =
  {
    { 0,  0, 0 ,  0,  0,  0, 0,  0 },
    { 2, 15, 10, -5,  0,  0, 0,  0 },
    { 2, 10, 5 , -5,  0,  0, 0,  0 },
    { 4, 35, 20, -5,  5, -5, 0,  0 },
    { 6, 50, 25, -5,  5, -5, 5, -5 },
    { 2,  5, 0 , -5,  0,  0, 0,  0 },
    { 4, 20, 10, -5,  0, -5, 0,  0 },
    { 6, 35, 10, -5, 10, -5, 0, -5 },
    { 6, 25, 10, -5,  0, -5, 0, -5 }
  };

  fprintf (ctxcanvas->file, "0\n");
  fprintf (ctxcanvas->file, "TABLE\n");
  fprintf (ctxcanvas->file, "2\n");
  fprintf (ctxcanvas->file, "LTYPE\n");
  fprintf (ctxcanvas->file, "70\n");
  fprintf (ctxcanvas->file, "5\n");
  for (j = 0; j < TABSIZE; j++)
  {
    fprintf (ctxcanvas->file, "0\n");
    fprintf (ctxcanvas->file, "LTYPE\n");
    fprintf (ctxcanvas->file, "2\n");

    wnamline (ctxcanvas, j);                            /* line style */

    fprintf (ctxcanvas->file, "70\n");
    fprintf (ctxcanvas->file, "64\n");
    fprintf (ctxcanvas->file, "3\n");
    fprintf (ctxcanvas->file, "%s\n", line[j]);      /* line style */
    fprintf (ctxcanvas->file, "72\n");
    fprintf (ctxcanvas->file, "65\n");
    fprintf (ctxcanvas->file, "73\n");
    fprintf (ctxcanvas->file, "%d\n", tab[j][0]);    /* number of parameters */
    fprintf (ctxcanvas->file, "40\n");
    fprintf (ctxcanvas->file, "%d\n", tab[j][1]);
    for (i = 2; i < 2 + tab[j][0]; i++)
    {
      fprintf (ctxcanvas->file, "49\n");
      fprintf (ctxcanvas->file, "%d\n", tab[j][i]);  /* parameters */
    }
  }
  fprintf (ctxcanvas->file, "0\n");
  fprintf (ctxcanvas->file, "ENDTAB\n");
}


static void deffonts (cdCtxCanvas *ctxcanvas)    /* define fonts */
{
  int i;
  static char *font[] =
  {
    "romanc.shx"  ,
    "romant.shx"  ,
    "rom_____.pfb",
    "romb____.pfb",
    "sas_____.pfb",
    "sasb____.pfb"
  };

  fprintf (ctxcanvas->file, "0\n");
  fprintf (ctxcanvas->file, "TABLE\n");
  fprintf (ctxcanvas->file, "2\n");
  fprintf (ctxcanvas->file, "STYLE\n");
  fprintf (ctxcanvas->file, "70\n");
  fprintf (ctxcanvas->file, "5\n");
  for (i = 1; i < 7; i++)
  {
    fprintf (ctxcanvas->file, "0\n");
    fprintf (ctxcanvas->file, "STYLE\n");
    fprintf (ctxcanvas->file, "2\n");

    wnamfont (ctxcanvas, i);                            /* font style name */

    fprintf (ctxcanvas->file, "3\n");
    fprintf (ctxcanvas->file, "%s\n", font[i-1]);    /* font style file */
    fprintf (ctxcanvas->file, "70\n");
    fprintf (ctxcanvas->file, "64\n");
    fprintf (ctxcanvas->file, "71\n");
    fprintf (ctxcanvas->file, "0\n");
    fprintf (ctxcanvas->file, "40\n");
    fprintf (ctxcanvas->file, "0\n");
    fprintf (ctxcanvas->file, "41\n");
    fprintf (ctxcanvas->file, "1\n");
    fprintf (ctxcanvas->file, "42\n");
    fprintf (ctxcanvas->file, "0\n");
    fprintf (ctxcanvas->file, "50\n");
    fprintf (ctxcanvas->file, "0\n");
  }
  fprintf (ctxcanvas->file, "0\n");
  fprintf (ctxcanvas->file, "ENDTAB\n");
}

static void cddeactivate (cdCtxCanvas *ctxcanvas)
{
  fflush (ctxcanvas->file);               /* flush file */
}

static void cdkillcanvas(cdCtxCanvas *ctxcanvas)
{
  fprintf (ctxcanvas->file, "0\n");
  fprintf (ctxcanvas->file, "ENDSEC\n");
  fprintf (ctxcanvas->file, "0\n");
  fprintf (ctxcanvas->file, "EOF\n");     /* fputs eof */
  fprintf (ctxcanvas->file, " \n");

  fflush (ctxcanvas->file);               /* flush file */
  fclose (ctxcanvas->file);

  memset(ctxcanvas, 0, sizeof(cdCtxCanvas));
  free (ctxcanvas);
}

static void cdflush (cdCtxCanvas *ctxcanvas)
{
  fflush (ctxcanvas->file);               /* flush file */
  ctxcanvas->layer++;
}

/*==========================================================================*/
/* Primitives                                                               */
/*==========================================================================*/
static void cdpoly(cdCtxCanvas *ctxcanvas, int mode, cdPoint* poly, int n)
{
  if (mode == CD_BEZIER)
  {
    cdSimPolyBezier(ctxcanvas->canvas, poly, n);
    return;
  }
  if (mode == CD_PATH)
  {
    cdSimPolyPath(ctxcanvas->canvas, poly, n);
    return;
  }

  if (mode == CD_CLOSED_LINES || mode == CD_FILL)
  {
    poly[n].x = poly[0].x;
    poly[n].y = poly[0].y;
    n++;
  }

  if( mode == CD_FILL )
    writehatch (ctxcanvas, poly, n);               /* write fill area */
  else
    writepoly (ctxcanvas, poly, n);                /* write polygon */
}

static void cdline (cdCtxCanvas *ctxcanvas, int x1, int y1, int x2, int y2)
{
  cdPoint line[2];                   /* uses new array of points to avoid      */

  line[0].x = x1;                    /* starting point */
  line[0].y = y1;
  line[1].x = x2;                    /* ending point   */
  line[1].y = y2;
  writepoly (ctxcanvas, line, 2);    /* draw line as a polygon */
}

static void cdrect(cdCtxCanvas *ctxcanvas, int xmin, int xmax, int ymin, int ymax)
{
  cdPoint rect[5];                     /* uses new array of points to avoid      */

  rect[0].x = xmin;
  rect[0].y = ymin;
  rect[1].x = xmin;
  rect[1].y = ymax;
  rect[2].x = xmax;                  /* box edges */
  rect[2].y = ymax;
  rect[3].x = xmax;
  rect[3].y = ymin;
  rect[4].x = xmin;
  rect[4].y = ymin;
  writepoly (ctxcanvas, rect, 5);              /* draw box as a polygon */
}

static void cdbox(cdCtxCanvas *ctxcanvas, int xmin, int xmax, int ymin, int ymax)
{
  cdPoint rect[5];                     /* uses new array of points to avoid      */

  rect[0].x = xmin;
  rect[0].y = ymin;
  rect[1].x = xmin;
  rect[1].y = ymax;
  rect[2].x = xmax;                  /* box edges */
  rect[2].y = ymax;
  rect[3].x = xmax;
  rect[3].y = ymin;
  rect[4].x = xmin;
  rect[4].y = ymin;
  writehatch (ctxcanvas, rect, 5);               /* write fill area */
}

static void cdfpoly(cdCtxCanvas *ctxcanvas, int mode, cdfPoint* poly, int n)
{
  if (mode == CD_BEZIER)
  {
    cdfSimPolyBezier(ctxcanvas->canvas, poly, n);
    return;
  }
  if (mode == CD_PATH)
  {
    cdfSimPolyPath(ctxcanvas->canvas, poly, n);
    return;
  }

  if (mode == CD_CLOSED_LINES || mode == CD_FILL)
  {
    poly[n].x = poly[0].x;
    poly[n].y = poly[0].y;
    n++;
  }

  if( mode == CD_FILL )
    writehatchf (ctxcanvas, poly, n);               /* write fill area */
  else
    writepolyf (ctxcanvas, poly, n);                /* write polygon */
}

static void cdfline (cdCtxCanvas *ctxcanvas, double x1, double y1, double x2, double y2)
{
  cdfPoint line[2];                   /* uses new array of points to avoid      */

  line[0].x = x1;                    /* starting point */
  line[0].y = y1;
  line[1].x = x2;                    /* ending point   */
  line[1].y = y2;
  writepolyf (ctxcanvas, line, 2);    /* draw line as a polygon */
}

static void cdfrect(cdCtxCanvas *ctxcanvas, double xmin, double xmax, double ymin, double ymax)
{
  cdfPoint rect[5];                     /* uses new array of points to avoid      */

  rect[0].x = xmin;
  rect[0].y = ymin;
  rect[1].x = xmin;
  rect[1].y = ymax;
  rect[2].x = xmax;                  /* box edges */
  rect[2].y = ymax;
  rect[3].x = xmax;
  rect[3].y = ymin;
  rect[4].x = xmin;
  rect[4].y = ymin;
  writepolyf (ctxcanvas, rect, 5);              /* draw box as a polygon */
}

static void cdfbox(cdCtxCanvas *ctxcanvas, double xmin, double xmax, double ymin, double ymax)
{
  cdfPoint rect[5];                     /* uses new array of points to avoid      */

  rect[0].x = xmin;
  rect[0].y = ymin;
  rect[1].x = xmin;
  rect[1].y = ymax;
  rect[2].x = xmax;                  /* box edges */
  rect[2].y = ymax;
  rect[3].x = xmax;
  rect[3].y = ymin;
  rect[4].x = xmin;
  rect[4].y = ymin;
  writehatchf (ctxcanvas, rect, 5);               /* write fill area */
}

/*--------------------------------------------------------------------------*/
/* gives radius of the circle most resembling elliptic arc at angle t       */
/*--------------------------------------------------------------------------*/
static double calc_radius (double a, double b, double t)
{
  return (pow ((a*a*sin(t)*sin(t) + b*b*cos(t)*cos(t)), 1.5))/(a*b);
}

/*--------------------------------------------------------------------------*/
/* calculates bulge for a given circular arc segment (between points p1 and */
/* p2, with radius r). Bulge is the tangent of 1/4 the angle theta of the   */
/* arc segment(a bulge of 1 is a semicircle, which has an angle of 180 deg) */
/*--------------------------------------------------------------------------*/
static double calc_bulge (double a, double b, double t1, double t2)
{
  cdfPoint p1, p2;          /* initial and ending arc points                 */
  double r;               /* radius most resembling arc at angle (t1+t2)/2 */
  double theta;           /* angle of circular arc segment                 */
  double sin_theta;       /* sine of theta                                 */
  double dist_x;          /* distance between two points along the x axis  */
  double dist_y;          /* distance between two points along the y axis  */
  double halfdist;        /* half distance between two points              */

  p1.x = a*cos(t1);
  p1.y = b*sin(t1);
  p2.x = a*cos(t2);
  p2.y = b*sin(t2);
  r    = calc_radius (a, b, (t1+t2)/2);

  dist_x      = p2.x - p1.x;
  dist_y      = p2.y - p1.y;
  halfdist    = (sqrt (dist_x*dist_x + dist_y*dist_y))/2;
  sin_theta   = halfdist/r;
  if (sin_theta > 1)  sin_theta = 1;
  theta       = 2*asin(sin_theta);

  return tan(theta/4);
}

static void writevertex (cdCtxCanvas *ctxcanvas, int xc, int yc, double a, double b, double t, double bulge)
{
  cdfPoint p;
  p.x = (xc + a*cos(t))/ctxcanvas->canvas->xres;
  p.y = (yc + b*sin(t))/ctxcanvas->canvas->xres;

  fprintf ( ctxcanvas->file, "0\n" );
  fprintf ( ctxcanvas->file, "VERTEX\n" );
  fprintf ( ctxcanvas->file, "8\n" );
  fprintf ( ctxcanvas->file, "%d\n", ctxcanvas->layer);      /* current layer */
  fprintf ( ctxcanvas->file, "10\n" );
  fprintf ( ctxcanvas->file, "%f\n", p.x );
  fprintf ( ctxcanvas->file, "20\n" );               /* vertex coordinates     */
  fprintf ( ctxcanvas->file, "%f\n", p.y );
  fprintf ( ctxcanvas->file, "42\n" );               /* bulge from this vertex */
  fprintf ( ctxcanvas->file, "%f\n", bulge );         /* to the next one        */
}

static void cdarc (cdCtxCanvas *ctxcanvas, int xc, int yc, int w, int h, double a1, double a2)
{
  double bulge;        /* bulge is the tangent of 1/4 the angle for a given */
                       /* circle arc segment (a bulge of 1 is a semicircle) */
  double t;            /* current arc angle being calculated    */
  double t1;           /* a1 in radians                         */
  double t2;           /* a2 in radians                         */
  double a;            /* half horizontal axis                  */
  double b;            /* half vertical axis                    */
  double seg_angle;    /* angle of every arc segment            */
  double diff;         /* angle between a1 and a2               */
  int nseg;            /* number of arc segments                */
  int i;

  a         = w/2;
  b         = h/2;
  t1        = a1*CD_DEG2RAD;                /* a1 in radians */
  t2        = a2*CD_DEG2RAD;                /* a2 in radians */
  diff      = fabs(a2 - a1);
  nseg      = cdRound(diff)/(360/32); /* 32 segments in closed ellipse */
  nseg      = max(nseg, 1);
  seg_angle = (t2-t1)/nseg;

  fprintf ( ctxcanvas->file, "0\n" );
  fprintf ( ctxcanvas->file, "POLYLINE\n" );
  fprintf ( ctxcanvas->file, "8\n" );
  fprintf ( ctxcanvas->file, "%d\n", ctxcanvas->layer);  /* current layer */
  fprintf ( ctxcanvas->file, "6\n" );
  wnamline( ctxcanvas, ctxcanvas->lt );                     /* line type */
  fprintf ( ctxcanvas->file, "62\n" );
  fprintf ( ctxcanvas->file, "%3d\n", ctxcanvas->fgcolor ); /* color */
  fprintf ( ctxcanvas->file, "66\n" );
  fprintf ( ctxcanvas->file, "1\n" );
  fprintf ( ctxcanvas->file, "70\n" );
  fprintf ( ctxcanvas->file, "128\n" );
  fprintf ( ctxcanvas->file, "40\n" );
  fprintf ( ctxcanvas->file, "%f\n", ctxcanvas->lw );
  fprintf ( ctxcanvas->file, "41\n" );          /* entire arc line width */
  fprintf ( ctxcanvas->file, "%f\n", ctxcanvas->lw );

  for (i=0, t=t1; i<nseg; i++, t+=seg_angle)
  {                                            /* calculate bulge between t */
    bulge = calc_bulge (a, b, t, t+seg_angle); /* and t+seg_angle and write */
    writevertex (ctxcanvas, xc, yc, a, b, t, bulge);      /* vertex at t               */
  }

  writevertex (ctxcanvas, xc, yc, a, b, t2, 0);     /* bulge of last vertex is useless */

  fprintf ( ctxcanvas->file, "0\n" );
  fprintf ( ctxcanvas->file, "SEQEND\n" );
}

static void cdsector (cdCtxCanvas *ctxcanvas, int xc, int yc, int w, int h, double a1, double a2)
{
  double bulge;        /* bulge is the tangent of 1/4 the angle for a given */
                       /* circle arc segment (a bulge of 1 is a semicircle) */
  double t;            /* current arc angle being calculated    */
  double t1;           /* a1 in radians                         */
  double t2;           /* a2 in radians                         */
  double a;            /* half horizontal axis                  */
  double b;            /* half vertical axis                    */
  double seg_angle;    /* angle of every arc segment            */
  double diff;         /* angle between a1 and a2               */
  int nseg;            /* number of arc segments                */
  int i;

  a         = w/2;
  b         = h/2;
  t1        = a1*CD_DEG2RAD;              /* a1 in radians */
  t2        = a2*CD_DEG2RAD;              /* a2 in radians */
  diff      = fabs(a2 - a1);
  nseg      = cdRound(diff)/(360/32); /* 32 segments in closed ellipse */
  nseg      = max(nseg, 1);
  seg_angle = (t2-t1)/nseg;

  fprintf ( ctxcanvas->file, "0\n" );
  fprintf ( ctxcanvas->file, "POLYLINE\n" );
  fprintf ( ctxcanvas->file, "8\n" );
  fprintf ( ctxcanvas->file, "%d\n", ctxcanvas->layer); /* current layer */
  fprintf ( ctxcanvas->file, "6\n" );
  wnamline( ctxcanvas, ctxcanvas->lt );                     /* line type */
  fprintf ( ctxcanvas->file, "62\n" );
  fprintf ( ctxcanvas->file, "%3d\n", ctxcanvas->fgcolor ); /* color */
  fprintf ( ctxcanvas->file, "66\n" );
  fprintf ( ctxcanvas->file, "1\n" );
  fprintf ( ctxcanvas->file, "70\n" );
  fprintf ( ctxcanvas->file, "128\n" );
  fprintf ( ctxcanvas->file, "40\n" );
  fprintf ( ctxcanvas->file, "%f\n", ctxcanvas->lw );
  fprintf ( ctxcanvas->file, "41\n" );          /* entire arc line width */
  fprintf ( ctxcanvas->file, "%f\n", ctxcanvas->lw );

  if ((a2-a1) != 360)
    writevertex (ctxcanvas, xc, yc, 0, 0, 0, 0);    /* center */

  for (i=0, t=t1; i<nseg; i++, t+=seg_angle)
  {                                            /* calculate bulge between t */
    bulge = calc_bulge (a, b, t, t+seg_angle); /* and t+seg_angle and write */
    writevertex (ctxcanvas, xc, yc, a, b, t, bulge);      /* vertex at t               */
  }

  writevertex (ctxcanvas, xc, yc, a, b, t2, 0);     /* bulge of last vertex is useless */

  if ((a2-a1) != 360)
    writevertex (ctxcanvas, xc, yc, 0, 0, 0, 0);    /* center */

  fprintf ( ctxcanvas->file, "  0\n" );
  fprintf ( ctxcanvas->file, "SEQEND\n" );
}

static void cdtext (cdCtxCanvas *ctxcanvas, int x, int y, const char *s, int len)
{
  fprintf ( ctxcanvas->file, "0\n" );
  fprintf ( ctxcanvas->file, "TEXT\n" );
  fprintf ( ctxcanvas->file, "8\n" );
  fprintf ( ctxcanvas->file, "%d\n", ctxcanvas->layer);   /* current layer */
  fprintf ( ctxcanvas->file, "7\n" );
  wnamfont( ctxcanvas, ctxcanvas->tf );                      /* current font  */
  fprintf ( ctxcanvas->file, "62\n" );
  fprintf ( ctxcanvas->file, "%d\n", ctxcanvas->fgcolor );         /* color            */
  fprintf ( ctxcanvas->file, "10\n" );
  fprintf ( ctxcanvas->file, "%f\n", x/ctxcanvas->canvas->xres );    /* current position */
  fprintf ( ctxcanvas->file, "20\n" );
  fprintf ( ctxcanvas->file, "%f\n", y/ctxcanvas->canvas->xres );
  fprintf ( ctxcanvas->file, "11\n" );
  fprintf ( ctxcanvas->file, "%f\n", x/ctxcanvas->canvas->xres );    /* alignment point  */
  fprintf ( ctxcanvas->file, "21\n" );
  fprintf ( ctxcanvas->file, "%f\n", y/ctxcanvas->canvas->xres );
  fprintf ( ctxcanvas->file, "40\n" );
  fprintf ( ctxcanvas->file, "%f\n",  ctxcanvas->th );    /* text height */
  fprintf ( ctxcanvas->file, "50\n" );
  fprintf ( ctxcanvas->file, "%f\n",  ctxcanvas->canvas->text_orientation );    /* text orientation angle    */
  fprintf ( ctxcanvas->file, "51\n" );
  fprintf ( ctxcanvas->file, "%3d\n", ctxcanvas->toa );   /* text oblique angle        */
  fprintf ( ctxcanvas->file, "72\n" );
  fprintf ( ctxcanvas->file, "%3d\n", ctxcanvas->tha );   /* text horizontal alignment */
  fprintf ( ctxcanvas->file, "73\n" );
  fprintf ( ctxcanvas->file, "%3d\n", ctxcanvas->tva );   /* text vertical alignment   */
  fprintf ( ctxcanvas->file, "1\n" );

  s = cdStrDupN(s, len);
  fprintf ( ctxcanvas->file, "%s\n", s );          /* text */
  free((char*)s);
}


/*==========================================================================*/
/* Attributes                                                               */
/*==========================================================================*/

static int cdlinestyle (cdCtxCanvas *ctxcanvas, int style)
{
  switch (style)
  {
  case CD_CONTINUOUS:
    ctxcanvas->lt = 0;
    break;
  case CD_DASHED:
    ctxcanvas->lt = 1;
    break;
  case CD_DOTTED:
    ctxcanvas->lt = 5;
    break;
  case CD_DASH_DOT:
    ctxcanvas->lt = 6;
    break;
  case CD_DASH_DOT_DOT:
    ctxcanvas->lt = 8;
    break;
  }

  return style;
}

static int cdlinewidth (cdCtxCanvas *ctxcanvas, int width)
{
  ctxcanvas->lw = width/ctxcanvas->canvas->xres;
  return width;
}

static int cdfont (cdCtxCanvas *ctxcanvas, const char *type_face, int style, int size)
{
  /* obs: DXF's text height (ctxcanvas->th) corresponds to CD ascent */

  if (cdStrEqualNoCase(type_face, "System"))
  {
    ctxcanvas->tf = 0;
    ctxcanvas->toa = 0;
    ctxcanvas->th = 0.75;
  }
  else if (cdStrEqualNoCase(type_face, "Courier"))
  {
    switch (style&3)
    {
      case CD_PLAIN:
        ctxcanvas->tf = 1;
        ctxcanvas->toa = 0;
        break;

      case CD_BOLD:
        ctxcanvas->tf = 2;
        ctxcanvas->toa = 0;
        break;

      case CD_ITALIC:
        ctxcanvas->tf = 1;
        ctxcanvas->toa = 15;
        break;

      case CD_BOLD_ITALIC:
        ctxcanvas->tf = 2;
        ctxcanvas->toa = 15;
        break;
    }
    ctxcanvas->th = 0.75;
  }
  else if (cdStrEqualNoCase(type_face, "Times"))
  {
    switch (style&3)
    {
      case CD_PLAIN:
        ctxcanvas->tf = 3;
        ctxcanvas->toa = 0;
        break;

      case CD_BOLD:
        ctxcanvas->tf = 4;
        ctxcanvas->toa = 0;
        break;

      case CD_ITALIC:
        ctxcanvas->tf = 3;
        ctxcanvas->toa = 15;
        break;

      case CD_BOLD_ITALIC:
        ctxcanvas->tf = 4;
        ctxcanvas->toa = 15;
        break;
    }
    ctxcanvas->th = 1.125;
  }
  else if (cdStrEqualNoCase(type_face, "Helvetica"))
  {
    switch (style&3)
    {
      case CD_PLAIN:
        ctxcanvas->tf = 5;
        ctxcanvas->toa = 0;
        break;

      case CD_BOLD:
        ctxcanvas->tf = 6;
        ctxcanvas->toa = 0;
        break;

      case CD_ITALIC:
        ctxcanvas->tf = 5;
        ctxcanvas->toa = 15;
        break;

      case CD_BOLD_ITALIC:
        ctxcanvas->tf = 6;
        ctxcanvas->toa = 15;
        break;
    }
    ctxcanvas->th = 1.;
  }
  else
    return 0;

  ctxcanvas->th = ctxcanvas->th * cdGetFontSizePoints(ctxcanvas->canvas, size);

  return 1;
}

static void cdgetfontdim (cdCtxCanvas *ctxcanvas, int *max_width, int *height, int *ascent, int *descent)
{
  double tangent_ta;
  double pixel_th;

  tangent_ta = tan(ctxcanvas->toa*CD_DEG2RAD);
  pixel_th = (ctxcanvas->th*ctxcanvas->canvas->xres)/CD_MM2PT;  /* points to pixels */
  switch (ctxcanvas->tf)
  {
    case 0:                                  /* STANDARD font (CD_SYSTEM) */
      if (height)    *height    =  cdRound(pixel_th*4/3);
      if (ascent)    *ascent    = _cdRound(pixel_th);
      if (descent)   *descent   =  cdRound(pixel_th/3);
      if (max_width) *max_width = _cdRound(pixel_th);
      break;

    case 1:                                  /* ROMAN fonts (CD_COURIER)  */
    case 2:
      if (height)    *height    =  cdRound(pixel_th*4/3);
      if (ascent)    *ascent    = _cdRound(pixel_th);
      if (descent)   *descent   =  cdRound(pixel_th/3);
      if (max_width) *max_width =  cdRound((pixel_th*21/20) + tangent_ta*(*ascent));
      break;

    case 3:                            /* ROMANTIC fonts (CD_TIMES_ROMAN) */
      if (height)    *height    = cdRound(pixel_th*8/9);
      if (ascent)    *ascent    = cdRound(pixel_th*2/3);
      if (descent)   *descent   = cdRound(pixel_th*2/9);
      if (max_width) *max_width = cdRound((pixel_th*14/15) + tangent_ta*(*ascent));
      break;

    case 4:
      if (height)    *height    = cdRound(pixel_th*8/9);
      if (ascent)    *ascent    = cdRound(pixel_th*2/3);
      if (descent)   *descent   = cdRound(pixel_th*2/9);
      if (max_width) *max_width = cdRound((pixel_th*29/30) + tangent_ta*(*ascent));
      break;

    case 5:                            /* SANSSERIF fonts (CD_HELVETICA)  */
    case 6:
      if (height)    *height    = _cdRound(pixel_th);
      if (ascent)    *ascent    =  cdRound(pixel_th*3/4);
      if (descent)   *descent   =  cdRound(pixel_th/4);
      if (max_width) *max_width =  cdRound((pixel_th*15/16) + tangent_ta*(*ascent));
      break;
  }
}

static void cdgettextsize (cdCtxCanvas *ctxcanvas, const char *s, int len, int *width, int *height)
{
  int i;
  double tangent_ta;
  double pixel_th;
  (void)s;

  i = len;
  tangent_ta = tan(ctxcanvas->toa*CD_DEG2RAD);
  pixel_th = (ctxcanvas->th*ctxcanvas->canvas->xres)/CD_MM2PT;  /* points to pixels */

  switch (ctxcanvas->tf)  /* width return value based on maximum character width */
  {
    case 0:                                  /* STANDARD font (CD_SYSTEM) */
      if (height) *height = cdRound(pixel_th*4/3);
      if (width)  *width  = cdRound(pixel_th*i + (pixel_th/3)*(i-1));
      break;

    case 1:                                  /* ROMAN fonts (CD_COURIER)  */
    case 2:
      if (height) *height = cdRound(pixel_th*4/3);
      if (width)  *width  = cdRound((pixel_th*21/20)*i + (pixel_th/10)*(i-1) + tangent_ta*pixel_th);
      break;

    case 3:                            /* ROMANTIC fonts (CD_TIMES_ROMAN) */
      if (height) *height = cdRound(pixel_th*2/3 + pixel_th*2/9);
      if (width)  *width  = cdRound((pixel_th*14/15)*i + (pixel_th/45)*(i-1) + tangent_ta*pixel_th*2/3);
      break;

    case 4:
      if (height) *height = cdRound(pixel_th*2/3 + pixel_th*2/9);
      if (width)  *width  = cdRound((pixel_th*29/30)*i + (pixel_th*2/45)*(i-1) + tangent_ta*pixel_th*2/3);
      break;

    case 5:                            /* SANSSERIF fonts (CD_HELVETICA)  */
    case 6:
      if (height) *height = _cdRound(pixel_th);
      if (width)  *width  =  cdRound((pixel_th*15/16)*i + (pixel_th/45)*(i-1) + tangent_ta*pixel_th*3/4);
      break;
  }
}

static int cdtextalignment (cdCtxCanvas *ctxcanvas, int alignment)
{
  switch (alignment)          /* convert alignment to DXF format */
  {
    case CD_BASE_LEFT:
      ctxcanvas->tva = 0;
      ctxcanvas->tha = 0;
      break;

    case CD_BASE_CENTER:
      ctxcanvas->tva = 0;
      ctxcanvas->tha = 1;
      break;

    case CD_BASE_RIGHT:
      ctxcanvas->tva = 0;
      ctxcanvas->tha = 2;
      break;

    case CD_SOUTH_WEST:
      ctxcanvas->tva = 1;
      ctxcanvas->tha = 0;
      break;

    case CD_SOUTH:
      ctxcanvas->tva = 1;
      ctxcanvas->tha = 1;
      break;

    case CD_SOUTH_EAST:
      ctxcanvas->tva = 1;
      ctxcanvas->tha = 2;
      break;

    case CD_WEST:
      ctxcanvas->tva = 2;
      ctxcanvas->tha = 0;
      break;

    case CD_CENTER:
      ctxcanvas->tva = 2;
      ctxcanvas->tha = 1;
      break;

    case CD_EAST:
      ctxcanvas->tva = 2;
      ctxcanvas->tha = 2;
      break;

    case CD_NORTH_WEST:
      ctxcanvas->tva = 3;
      ctxcanvas->tha = 0;
      break;

    case CD_NORTH:
      ctxcanvas->tva = 3;
      ctxcanvas->tha = 1;
      break;

    case CD_NORTH_EAST:
      ctxcanvas->tva = 3;
      ctxcanvas->tha = 2;
      break;
  }

  return alignment;
}

/*==========================================================================*/
/* Color                                                                    */
/*==========================================================================*/

static void RGB_to_HSB (unsigned char r, unsigned char g, unsigned char b,
                        double *hue, double *sat, double *bright)
{
  double maximum;
  double minimum;
  double delta;
  double red   = r/255.;         /* red, green and blue range from 0 to 1 */
  double green = g/255.;
  double blue  = b/255.;

  maximum = max(max(red, green), blue);   /* stores higher index */
  minimum = min(min(red, green), blue);   /* stores lower index  */
  delta   = maximum - minimum;

  *bright = maximum*100;
  *sat    = 0;

  if (maximum != 0)     /* sat from 0 to 100 */
    *sat = (delta*100)/maximum;

  if (*sat != 0)        /* hue from 0 to 359 */
  {
    if (red   == maximum) *hue = (green - blue)/delta;
    if (green == maximum) *hue = 2 + (blue - red)/delta;
    if (blue  == maximum) *hue = 4 + (red - green)/delta;
    *hue *= 60;
    if (*hue < 0) *hue += 360;
  }
  else
    *hue = 0;           /* color is greyscale (hue is meaningless) */
}

static int HSB_to_AutoCAD_Palette (double hue, double sat, double bright)
{
  int index;
  int h, s, b;

  if (bright < 17)     /* 5 levels of brightness in AutoCAD palette, 6 with */
  {                    /* black. If bright < 17, index is black (7).        */
    index = 7;         /* 17 is 100/6 (rounded up)                          */
  }
  else if (sat < 10)              /* low saturation makes color tend to     */
  {                               /* grey/white. 6 levels of grey/white in  */
    b = (int)floor(bright/14.3)-1;/* palette WITHOUT black. 14.3 is 100/7   */
    index = 250 + b;              /* index is grey to white(255 in palette) */
  }
  else
  {
    h = cdRound(hue/15.) + 1;
    if (h > 24) h -= 24;          /* 15 is 360/24                           */
    h *= 10;                      /* h ranges from 10 to 240 in palette     */
    s = (sat < 55) ? 1 : 0;       /* s is 'high'(0) or 'low'(1) in palette  */
    b = (int)floor(bright/16.7)-1;/* b is 0, 2, 4, 6 or 8 in palette        */
    b = 2*(4 - b);                /* (from brightest to dimmest)            */
    index = h + s + b;            /* index is simple sum of h, s and b      */
  }
  return index;
}

static int get_palette_index (long int color)      /* gives closest palette */
{                                                  /* index to RGB color    */
  unsigned char red, green, blue;
  double hue, sat, bright;

  cdDecodeColor (color, &red, &green, &blue);         /* AutoCAD palette is */
  RGB_to_HSB (red, green, blue, &hue, &sat, &bright); /* based on HSB model */

  return HSB_to_AutoCAD_Palette (hue, sat, bright);
}

static long int cdforeground (cdCtxCanvas *ctxcanvas, long int color)
{
  ctxcanvas->fgcolor = get_palette_index (color);
  return color;
}


/*==========================================================================*/
/* Server Images                                                            */
/*==========================================================================*/

static void cdpixel (cdCtxCanvas *ctxcanvas, int x, int y, long int color)
{
  int oldcolor = ctxcanvas->fgcolor;                    /* put 'color' as current */
  cdforeground (ctxcanvas, color);                          /* foreground color */
  fprintf ( ctxcanvas->file, "0\n" );
  fprintf ( ctxcanvas->file, "POINT\n" );
  fprintf ( ctxcanvas->file, "8\n" );
  fprintf ( ctxcanvas->file, "%d\n", ctxcanvas->layer);     /* current layer */
  fprintf ( ctxcanvas->file, "62\n" );
  fprintf ( ctxcanvas->file, "%d\n", ctxcanvas->fgcolor );  /* color */
  fprintf ( ctxcanvas->file, "10\n" );
  fprintf ( ctxcanvas->file, "%f\n", x/ctxcanvas->canvas->xres );         /* position */
  fprintf ( ctxcanvas->file, " 20\n" );
  fprintf ( ctxcanvas->file, "%f\n", y/ctxcanvas->canvas->xres );
  ctxcanvas->fgcolor = oldcolor;                        /* retrieve old fgcolor */
}

/******************************************************/

static void cdcreatecanvas(cdCanvas* canvas, void *data)
{
  char filename[10240] = "";
  char* strdata = (char*)data;
  cdCtxCanvas *ctxcanvas;
  double param1, param2, param3;

  ctxcanvas = (cdCtxCanvas *) malloc (sizeof (cdCtxCanvas));

  param1 = 0;
  param2 = 0;
  param3 = 0;

  strdata += cdGetFileName(strdata, filename);
  if (filename[0] == 0)
    return;

  sscanf(strdata, "%lfx%lf %lf", &param1, &param2, &param3);

  ctxcanvas->file = fopen (filename, "w");
  if (ctxcanvas->file == NULL)
  {
    free(ctxcanvas);
    return;
  }

  if (!param1)
  {
    canvas->w_mm  = INT_MAX*3.78;
    canvas->h_mm = INT_MAX*3.78;
    canvas->xres = 3.78;
  }
  else if (!param2)
  {
    canvas->w_mm  = INT_MAX*param1;
    canvas->h_mm = INT_MAX*param1;
    canvas->xres = param1;
  }
  else if (!param3)
  {
    canvas->w_mm  = param1;
    canvas->h_mm = param2;
    canvas->xres = 3.78;
  }
  else
  {
    canvas->w_mm  = param1;
    canvas->h_mm = param2;
    canvas->xres = param3;
  }

  ctxcanvas->canvas = canvas;
  canvas->ctxcanvas = ctxcanvas;

  canvas->bpp = 8;

  canvas->yres = canvas->xres;

  canvas->w = (int)(canvas->w_mm * canvas->xres);
  canvas->h = (int)(canvas->h_mm * canvas->yres);

  ctxcanvas->layer = 0;            /* reset layer    */

  ctxcanvas->tf  = 0;              /* text font (0 is STANDARD)               */
  ctxcanvas->th  = 9;              /* text height                             */
  ctxcanvas->toa = 0;              /* text oblique angle                      */
  ctxcanvas->tva = 0;              /* text vertical alignment (0 is baseline) */
  ctxcanvas->tha = 0;              /* text horizontal alignment (0 is left)   */
  ctxcanvas->fgcolor   = 7;        /* foreground AutoCAD palette color        */

  fprintf (ctxcanvas->file, "0\n");
  fprintf (ctxcanvas->file, "SECTION\n");  /* header maker */
  fprintf (ctxcanvas->file, "2\n");
  fprintf (ctxcanvas->file, "HEADER\n");
  fprintf (ctxcanvas->file, "  9\n");
  fprintf (ctxcanvas->file, "$ACADVER\n");
  fprintf (ctxcanvas->file, "  1\n");
  fprintf (ctxcanvas->file, "AC1006\n"); /* AutoCad R10 */
  fprintf (ctxcanvas->file, "9\n");
  fprintf (ctxcanvas->file, "$LIMCHECK\n");
  fprintf (ctxcanvas->file, "70\n");
  fprintf (ctxcanvas->file, "1\n");
  fprintf (ctxcanvas->file, "9\n");
  fprintf (ctxcanvas->file, "$LIMMIN\n");
  fprintf (ctxcanvas->file, "10\n");
  fprintf (ctxcanvas->file, "0\n");
  fprintf (ctxcanvas->file, "20\n");
  fprintf (ctxcanvas->file, "0\n");
  fprintf (ctxcanvas->file, "9\n");
  fprintf (ctxcanvas->file, "$LIMMAX\n");
  fprintf (ctxcanvas->file, "10\n");
  fprintf (ctxcanvas->file, "%f\n", ctxcanvas->canvas->w_mm);
  fprintf (ctxcanvas->file, "20\n");
  fprintf (ctxcanvas->file, "%f\n", ctxcanvas->canvas->h_mm);
  fprintf (ctxcanvas->file, "9\n");
  fprintf (ctxcanvas->file, "$EXTMIN\n");
  fprintf (ctxcanvas->file, "10\n");
  fprintf (ctxcanvas->file, "0\n");
  fprintf (ctxcanvas->file, "20\n");
  fprintf (ctxcanvas->file, "0\n");
  fprintf (ctxcanvas->file, "9\n");
  fprintf (ctxcanvas->file, "$EXTMAX\n");
  fprintf (ctxcanvas->file, "10\n");
  fprintf (ctxcanvas->file, "%f\n", ctxcanvas->canvas->w_mm);
  fprintf (ctxcanvas->file, "20\n");
  fprintf (ctxcanvas->file, "%f\n", ctxcanvas->canvas->h_mm);
  fprintf (ctxcanvas->file, "9\n");
  fprintf (ctxcanvas->file, "$CLAYER\n");
  fprintf (ctxcanvas->file, "8\n");
  fprintf (ctxcanvas->file, "0\n");
  fprintf (ctxcanvas->file, "9\n");
  fprintf (ctxcanvas->file, "$LUNITS\n");
  fprintf (ctxcanvas->file, "70\n");
  fprintf (ctxcanvas->file, "2\n");
  fprintf (ctxcanvas->file, "9\n");
  fprintf (ctxcanvas->file, "$LUPREC\n");
  fprintf (ctxcanvas->file, "70\n");    /* precision (resolution dependant) */
  fprintf (ctxcanvas->file, "%d\n", (int)ceil(log10(ctxcanvas->canvas->xres)));
  fprintf (ctxcanvas->file, "9\n");
  fprintf (ctxcanvas->file, "$AUNITS\n");
  fprintf (ctxcanvas->file, "70\n");
  fprintf (ctxcanvas->file, "0\n");
  fprintf (ctxcanvas->file, "9\n");
  fprintf (ctxcanvas->file, "$AUPREC\n");
  fprintf (ctxcanvas->file, "70\n");
  fprintf (ctxcanvas->file, "2\n");
  fprintf (ctxcanvas->file, "9\n");
  fprintf (ctxcanvas->file, "$TEXTSTYLE\n");
  fprintf (ctxcanvas->file, "7\n");
  fprintf (ctxcanvas->file, "STANDARD\n");
  fprintf (ctxcanvas->file, "0\n");
  fprintf (ctxcanvas->file, "ENDSEC\n");
  fprintf (ctxcanvas->file, "0\n");
  fprintf (ctxcanvas->file, "SECTION\n");
  fprintf (ctxcanvas->file, "2\n");
  fprintf (ctxcanvas->file, "TABLES\n");

  deflines (ctxcanvas);      /* define lines */
  deffonts (ctxcanvas);      /* define fonts */

  fprintf (ctxcanvas->file, "0\n");
  fprintf (ctxcanvas->file, "ENDSEC\n");
  fprintf (ctxcanvas->file, "0\n");
  fprintf (ctxcanvas->file, "SECTION\n");
  fprintf (ctxcanvas->file, "2\n");
  fprintf (ctxcanvas->file, "ENTITIES\n");
}

static void cdinittable(cdCanvas* canvas)
{
  canvas->cxFlush = cdflush;
  canvas->cxPixel = cdpixel;
  canvas->cxLine = cdline;
  canvas->cxPoly = cdpoly;
  canvas->cxRect = cdrect;
  canvas->cxBox = cdbox;
  canvas->cxFLine = cdfline;
  canvas->cxFPoly = cdfpoly;
  canvas->cxFRect = cdfrect;
  canvas->cxFBox = cdfbox;
  canvas->cxArc = cdarc;
  canvas->cxSector = cdsector;
  canvas->cxText = cdtext;
  canvas->cxGetFontDim = cdgetfontdim;
  canvas->cxGetTextSize = cdgettextsize;

  canvas->cxLineStyle = cdlinestyle;
  canvas->cxLineWidth = cdlinewidth;
  canvas->cxFont = cdfont;
  canvas->cxTextAlignment = cdtextalignment;
  canvas->cxForeground = cdforeground;

  canvas->cxKillCanvas = cdkillcanvas;
  canvas->cxDeactivate = cddeactivate;
}

/******************************************************/

static cdContext cdDXFContext =
{
  CD_CAP_ALL & ~(CD_CAP_CLEAR | CD_CAP_PLAY | CD_CAP_PALETTE |
                 CD_CAP_CLIPAREA | CD_CAP_CLIPPOLY | CD_CAP_PATH | CD_CAP_BEZIER |
                 CD_CAP_LINECAP | CD_CAP_LINEJOIN | CD_CAP_REGION | CD_CAP_CHORD |
                 CD_CAP_IMAGERGB | CD_CAP_IMAGEMAP | CD_CAP_IMAGESRV |
                 CD_CAP_BACKGROUND | CD_CAP_BACKOPACITY | CD_CAP_WRITEMODE |
                 CD_CAP_HATCH | CD_CAP_STIPPLE | CD_CAP_PATTERN |
                 CD_CAP_IMAGERGBA | CD_CAP_GETIMAGERGB),
  0,
  cdcreatecanvas,
  cdinittable,
  NULL,
  NULL,
};

cdContext* cdContextDXF(void)
{
  return &cdDXFContext;
}

