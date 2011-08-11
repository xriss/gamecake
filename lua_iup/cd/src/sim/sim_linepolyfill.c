/** \file
 * \brief Primitives of the Simulation Base Driver
 *
 * See Copyright Notice in cd.h
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <memory.h>
#include <assert.h>

#include "cd.h"
#include "cd_private.h"
#include "cd_truetype.h"
#include "sim.h"


/* para estilos de linha usando rotacao de bits */
static const unsigned short int simLineStyleBitTable[5]=
{ 
  0xFFFF, /* CD_CONTINUOUS  */
  0xFF00, /* CD_DASHED      */
  0x1111, /* CD_DOTTED      */
  0xFE10, /* CD_DASH_DOT    */
  0xFF24, /* CD_DASH_DOT_DOT*/
};
int simLineStyleNoReset = 0;
static unsigned short int simLineStyleLastBits = 0;

#define simRotateLineStyle(_x) (((_x) & 0x8000)? ((_x) << 1)|(0x0001): ((_x) << 1))

#define INTENSITYSHIFT 8  /* # of bits by which to shift ErrorAcc to get intensity level */


/* Point in Polygon was obtained from:
   www.geometryalgorithms.com/Archive/algorithm_0103/algorithm_0103.htm

   Copyright 2001, softSurfer (www.softsurfer.com)
   This code may be freely used and modified for any purpose
   providing that this copyright notice is included with it.
   SoftSurfer makes no warranty for this code, and cannot be held
   liable for any real or imagined damage resulting from its use.
*/

#define isLeft( _P0, _P1, _x, _y )  ((_P1.x - _P0.x)*(_y - _P0.y) - (_x - _P0.x)*(_P1.y - _P0.y))

int simIsPointInPolyWind(cdPoint* poly, int n, int x, int y)
{
  int i, i1, 
      wn = 0;  /* the winding number counter  */

  for (i = 0; i < n; i++) 
  {   
    i1 = (i+1)%n; /* next point(i+1), next of last(n-1) is first(0) */

    if (poly[i].y <= y) 
    {         
      if (poly[i1].y > y)      /* an upward crossing */
        if (isLeft(poly[i], poly[i1], x, y) > 0)  /* P left of edge */
          ++wn;                /* have a valid up intersect */
    }
    else 
    {                       
      if (poly[i1].y <= y)     /* a downward crossing */
        if (isLeft(poly[i], poly[i1], x, y) < 0)  /* P right of edge */
          --wn;                /* have a valid down intersect */
    }
  }

  return wn;
}

static int compare_int(const int* xx1, const int* xx2)
{
  return *xx1 - *xx2;
}

int simAddSegment(simLineSegment* segment, int x1, int y1, int x2, int y2, int *y_max, int *y_min)
{
  if (x1==x2 && y1==y2)
    return 0;

  /* Make sure p2.y > p1.y */
  if (y1 > y2) 
  {
    _cdSwapInt(y1, y2);
    _cdSwapInt(x1, x2);
    segment->Swap = 1;
  }
  else
    segment->Swap = 0;

  segment->x1 = x1;
  segment->y1 = y1;
  segment->x2 = x2;
  segment->y2 = y2;

  segment->x = x2;  /* initial value */
  
  segment->DeltaY = y2 - y1;
  segment->DeltaX = x2 - x1;
  if (segment->DeltaX >= 0)
    segment->XDir = -1;  /* inverted from simLineThin since here is from p2 to p1 */
  else 
  {
    segment->XDir = 1;
    segment->DeltaX = -segment->DeltaX; /* make DeltaX positive */
  }

  segment->ErrorAcc = 0;  /* initialize the line error accumulator to 0 */

  /* Is this an X-major or Y-major line? */
  if (segment->DeltaY > segment->DeltaX) 
  {
    if (segment->DeltaY==0)  /* do not compute for horizontal segments */
      return 1;

    /* Y-major line; calculate 16-bit fixed-point fractional part of a
    pixel that X advances each time Y advances 1 pixel, truncating the
    result so that we won't overrun the endpoint along the X axis */
    segment->ErrorInc = (unsigned short)(((unsigned long)segment->DeltaX << 16) / (unsigned long)segment->DeltaY);
  }
  else
  {
    if (segment->DeltaX==0)  /* do not compute for vertical segments */
      return 1;

    /* It's an X-major line; calculate 16-bit fixed-point fractional part of a
    pixel that Y advances each time X advances 1 pixel, truncating the
    result to avoid overrunning the endpoint along the X axis */
    segment->ErrorInc = (unsigned short)(((unsigned long)segment->DeltaY << 16) / (unsigned long)segment->DeltaX);
  }

  /* also calculates y_max and y_min of the polygon */
  if (y2 > *y_max)
    *y_max = y2;
  if (y1 < *y_min)
    *y_min = y1;

  return 1;
}

int simSegmentInc(simLineSegment* segment)
{
  unsigned short ErrorAccTemp, Weighting;

  if (segment->DeltaY == 0) 
  {
    /* Horizontal line */
    while (segment->DeltaX-- != 0) 
      segment->x += segment->XDir;
    return segment->x;
  }

  if (segment->DeltaX == 0) 
  {
    /* Vertical line */
    segment->DeltaY--;
    return segment->x;
  }

  if (segment->DeltaX == segment->DeltaY) 
  {
    /* Perfect Diagonal line */
    segment->x += segment->XDir;
    segment->DeltaY--;
    return segment->x;
  }

  /* Is this an X-major or Y-major line? */
  if (segment->DeltaY > segment->DeltaX) 
  {
    /* Increment pixels other than the first and last */
    ErrorAccTemp = segment->ErrorAcc;   /* remember currrent accumulated error */
    segment->ErrorAcc += segment->ErrorInc;      /* calculate error for next pixel */
    if (segment->ErrorAcc <= ErrorAccTemp) 
    {
      /* The error accumulator turned over, so advance the X coord */
      segment->x += segment->XDir;
    }

    Weighting = segment->ErrorAcc >> INTENSITYSHIFT;

    if (Weighting < 128)
      return segment->x;
    else
      return segment->x + segment->XDir;
  }
  else
  {
    /* Increment all pixels other than the first and last */
    int hline_end = 0;
    while (!hline_end)
    {
      ErrorAccTemp = segment->ErrorAcc;   /* remember currrent accumulated error */
      segment->ErrorAcc += segment->ErrorInc;      /* calculate error for next pixel */
      if (segment->ErrorAcc <= ErrorAccTemp) 
      {
        /* The error accumulator turned over, so advance the Y coord */
        hline_end = 1;
      }

      segment->x += segment->XDir; /* X-major, so always advance X */
    }

    return segment->x;
  }
}

typedef struct _simIntervalList
{
  int* xx;
  int n, count;
} simIntervalList;

static int simFillCheckAAPixel(simIntervalList* line_int_list, int x)
{
  int i, *xx = line_int_list->xx;
  for (i = 0; i < line_int_list->n; i+=2)
  {
    if (xx[i] <= x && x <= xx[i+1])
      return 0; /* inside, already drawn, do not draw */
  }
  return 1;
}

static void simPolyAAPixels(cdCanvas *canvas, simIntervalList* line_int_list, int y_min, int y_max, int x1, int y1, int x2, int y2)
{
  unsigned short ErrorInc, ErrorAcc;
  unsigned short ErrorAccTemp, Weighting;
  int DeltaX, DeltaY, XDir;
  int no_antialias = !(canvas->simulation->antialias);

  /* Make sure p2.y > p1.y */
  if (y1 > y2) 
  {
    _cdSwapInt(y1, y2);
    _cdSwapInt(x1, x2);
  }

  DeltaX = x2 - x1;
  if (DeltaX >= 0)
    XDir = 1;
  else 
  {
    XDir = -1;
    DeltaX = -DeltaX; /* make DeltaX positive */
  }

  /* Special-case horizontal, vertical, and diagonal lines, which
  require no weighting because they go right through the center of
  every pixel */
  DeltaY = y2 - y1;
  if (DeltaY == 0 || DeltaX == 0 || DeltaX == DeltaY) return;

  /* Line is not horizontal, diagonal, or vertical */

  /* highest and lowest pixels are not necessary 
     since they are always drawn in the previous step. */

  ErrorAcc = 0;  /* initialize the line error accumulator to 0 */

  /* Is this an X-major or Y-major line? */
  if (DeltaY > DeltaX) 
  {
    ErrorInc = (unsigned short)(((unsigned long)DeltaX << 16) / (unsigned long)DeltaY);

    /* Draw all pixels other than the first and last */
    while (--DeltaY) 
    {
      ErrorAccTemp = ErrorAcc;   /* remember currrent accumulated error */
      ErrorAcc += ErrorInc;      /* calculate error for next pixel */
      if (ErrorAcc <= ErrorAccTemp) 
        x1 += XDir;

      y1++; /* Y-major, so always advance Y */

      Weighting = ErrorAcc >> INTENSITYSHIFT;

      if (y1 < y_min || y1 > y_max) continue;

      if (no_antialias)
      {
        if (Weighting < 128)
        {
          if (simFillCheckAAPixel(line_int_list+(y1-y_min), x1))
            simFillDrawAAPixel(canvas, x1, y1, 255);
        }
        else
        {
          if (simFillCheckAAPixel(line_int_list+(y1-y_min), x1 + XDir))
            simFillDrawAAPixel(canvas, x1 + XDir, y1, 255);
        }
      }
      else
      {
        if (simFillCheckAAPixel(line_int_list+(y1-y_min), x1))
          simFillDrawAAPixel(canvas, x1, y1, 255-Weighting);

        if (simFillCheckAAPixel(line_int_list+(y1-y_min), x1 + XDir))
          simFillDrawAAPixel(canvas, x1 + XDir, y1, Weighting);
      }
    }
  }
  else
  {
    ErrorInc = (unsigned short)(((unsigned long)DeltaY << 16) / (unsigned long)DeltaX);

    /* Draw all pixels other than the first and last */
    while (--DeltaX) 
    {
      ErrorAccTemp = ErrorAcc;   /* remember currrent accumulated error */
      ErrorAcc += ErrorInc;      /* calculate error for next pixel */
      if (ErrorAcc <= ErrorAccTemp) 
        y1++;

      x1 += XDir; /* X-major, so always advance X */

      Weighting = ErrorAcc >> INTENSITYSHIFT;

      if (y1 < y_min || y1 > y_max) continue;

      if (no_antialias)
      {
        if (Weighting < 128)
        {
          if (simFillCheckAAPixel(line_int_list+(y1-y_min), x1))
            simFillDrawAAPixel(canvas, x1, y1, 255);
        }
        else
        {
          if (y1+1 < y_min || y1+1 > y_max) continue;

          if (simFillCheckAAPixel(line_int_list+(y1+1-y_min), x1))
            simFillDrawAAPixel(canvas, x1, y1+1, 255);
        }
      }
      else
      {
        if (simFillCheckAAPixel(line_int_list+(y1-y_min), x1))
          simFillDrawAAPixel(canvas, x1, y1, 255-Weighting);

        if (y1+1 < y_min || y1+1 > y_max) continue;

        if (simFillCheckAAPixel(line_int_list+(y1+1-y_min), x1))
          simFillDrawAAPixel(canvas, x1, y1+1, Weighting);
      }
    }
  }
}

static void simLineIntervallInit(simIntervalList* line_int_list, int count)
{
  line_int_list->xx = malloc(sizeof(int)*count);
  line_int_list->n = 0;
  line_int_list->count = count;
}

static void simLineIntervallAdd(simIntervalList* line_int_list, int x1, int x2)
{
  int i = line_int_list->n;
  line_int_list->xx[i] = x1;
  line_int_list->xx[i+1] = x2;
  line_int_list->n += 2;
}

void simPolyMakeSegments(simLineSegment *segments, int *n_seg, cdPoint* poly, int n, int *max_hh, int *y_max, int *y_min)
{
  int i, i1;
  *y_max = poly[0].y;
  *y_min = poly[0].y;
  *max_hh=0, *n_seg = n;
  for(i = 0; i < n; i++)
  {
    i1 = (i+1)%n; /* next point(i+1), next of last(n-1) is first(0) */
    if (simAddSegment(segments, poly[i].x, poly[i].y, poly[i1].x, poly[i1].y, y_max, y_min))
    {
      if (poly[i].y == poly[i1].y)
        (*max_hh)++;  /* increment the number of horizontal segments */

      segments++;
    }
    else
      (*n_seg)--;
  }
}

static void simAddHxx(int *hh, int *hh_count, int x1, int x2)
{
  /* It will add a closed interval in a list of closed intervals.
     But if some intersect then they must be merged. */

  int i, count = *hh_count;

  assert(count%2==0);

  if (x1 > x2)
  {
    int t = x2;
    x2 = x1;
    x1 = t;
  }

  for (i=0; i<count; i+=2)  /* here we always have pairs */
  {
    /*  x_end >= h_begin AND x_begin <= h_end */
    if (x2 >= hh[i] && x1 <= hh[i+1])
    {
      /* there is some intersection, merge interval and remove point */
      if (hh[i] < x1)
        x1 = hh[i];
      if (hh[i+1] > x2)
        x2 = hh[i+1];

      memmove(hh+i, hh+i+2, (count-(i+2))*sizeof(int));
      count -= 2;
    }
  }

  /* no intersection, add both points at the end */
  hh[count] = x1;
  hh[count+1] = x2;
  *hh_count = count + 2;
}

static void simMergeHxx(int *xx, int *xx_count, int *hh, int hh_count)
{
  int hh_i;

  assert(hh_count%2==0);

  if (*xx_count == 0)
  {
    memcpy(xx, hh, hh_count*sizeof(int));
    *xx_count = hh_count;
    return;
  }

  /* remember that xx and hh both have an even number of points, 
     and all pairs are intervals.
     So infact this call will behave as simAddHxx for each pair of hh. */
  for (hh_i=0; hh_i<hh_count; hh_i+=2)
  {
    simAddHxx(xx, xx_count, hh[hh_i], hh[hh_i+1]);
  }
}

int simPolyFindHorizontalIntervals(simLineSegment *segments, int n_seg, int* xx, int *hh, int y, int height)
{
  simLineSegment *seg_i;
  int i, hh_count = 0;
  int xx_count = 0;  /* count the number of points in the horizontal line,
                        each pair will form an horizontal interval */

  /* for all segments, calculates the intervals to be filled 
     from the intersection with the horizontal line y. */
  for(i = 0; i < n_seg; i++)
  {
    seg_i = segments + i;

    /* if y is less than the minimum Y coordinate of the segment (y1), 
       or y is greater than the maximum Y coordinate of the segment (y2), 
       then ignore the segment. */
    if (y < seg_i->y1  || y > seg_i->y2)
      continue;

    /* if it is an horizontal line, then store the segment in a separate buffer. */
    if (seg_i->y1 == seg_i->y2)  /* because of the previous test, also implies "==y" */
    {
      int prev_y, next_y;
      int i_next = (i==n_seg-1)? 0: i+1;
      int i_prev = (i==0)? n_seg-1: i-1;
      simLineSegment *seg_i_next = segments + i_next;
      simLineSegment *seg_i_prev = segments + i_prev;

      simAddHxx(hh, &hh_count, seg_i->x1, seg_i->x2);

      /* include horizontal segments that are in a sequence */
      while (seg_i_next->y1 == seg_i_next->y2 && i < n_seg)
      {
        simAddHxx(hh, &hh_count, seg_i_next->x1, seg_i_next->x2);

        i++;

        if (i < n_seg)
        {
          i_next = (i==n_seg-1)? 0: i+1;
          seg_i_next = segments + i_next;
        }
      }

      if (i == n_seg)
        break;

      /* save the previous y, not in the horizontal line */
      if (seg_i_prev->y1 == y)
        prev_y = seg_i_prev->y2;
      else
        prev_y = seg_i_prev->y1;

      /* save the next y, not in the horizontal line */
      if (seg_i_next->y1 == y)
        next_y = seg_i_next->y2;
      else
        next_y = seg_i_next->y1;

      /* if the horizontal line is part of a step  |_  then compute a normal intersection in the middle */
      /*                                             |                                                  */
      if ((next_y > y && prev_y < y) ||
          (next_y < y && prev_y > y))
      {
        xx[xx_count++] = (seg_i->x1+seg_i->x2)/2;     /* save the intersection point, any value inside the segment will be fine */
      }
    }
    else if (y == seg_i->y1)  /* intersection at the lowest point (x1,y1) */
    {
      int i_next = (i==n_seg-1)? 0: i+1;
      int i_prev = (i==0)? n_seg-1: i-1;
      simLineSegment *seg_i_next = segments + i_next;
      simLineSegment *seg_i_prev = segments + i_prev;

      /* but add only if it does not belongs to an horizontal line */
      if (!((seg_i_next->y1 == y && seg_i_next->y2 == y) ||   /* next is an horizontal line */
            (seg_i_prev->y1 == y && seg_i_prev->y2 == y)))    /* previous is an horizontal line */
      {
        xx[xx_count++] = seg_i->x1;     /* save the intersection point */
      }
    }
    else if (y == seg_i->y2)  /* intersection at the highest point (x2,y2) */
    {
      int i_next = (i==n_seg-1)? 0: i+1;
      int i_prev = (i==0)? n_seg-1: i-1;
      simLineSegment *seg_i_next = segments + i_next;
      simLineSegment *seg_i_prev = segments + i_prev;

      /* Normally do nothing, because this point is duplicated in another segment,    
         i.e only save the intersection point for (y2) if not handled by (y1) of another segment.   
         The exception is the top-corner points (^). */
      /* first find if p2 is connected to next or previous */
      if ((!seg_i->Swap && seg_i_next->Swap && seg_i_next->y2 == y && seg_i_next->x2 == seg_i->x2 && seg_i_next->y1 != y) || 
          (seg_i->Swap && !seg_i_prev->Swap && seg_i_prev->y2 == y && seg_i_prev->x2 == seg_i->x2 && seg_i_prev->y1 != y))
      {
        xx[xx_count++] = seg_i->x2;     /* save the intersection point */
      }
    }
    else /* if ((y > seg_i->y1) && (y < seg_i->y2))  intersection inside the segment  */
    {                                             
      xx[xx_count++] = simSegmentInc(seg_i);  /* save the intersection point */
    }
  }

  /* if outside the canvas, ignore the intervals and */
  /* continue since the segments where updated in simSegmentInc. */
  if (y > height-1)
    return 0;

  /* sort the intervals */
  if (xx_count)
    qsort(xx, xx_count, sizeof(int), (int (*)(const void*,const void*))compare_int);

  /* add the horizontal segments. */
  if (hh_count)
  {
    simMergeHxx(xx, &xx_count, hh, hh_count);

    /* sort again */
    if (xx_count)
      qsort(xx, xx_count, sizeof(int), (int (*)(const void*,const void*))compare_int);
  }

  return xx_count;
}

void simPolyFill(cdSimulation* simulation, cdPoint* poly, int n) 
{
  /***********IMPORTANT: this function is used as a reference for irgbClipPoly in "cdirgb.c",
     if a change is made here, must be reflected there, and vice-versa */
  simIntervalList* line_int_list, *line_il;
  int y_max, y_min, i, y, i1, fill_mode, num_lines,
      xx_count, width, height, *xx, *hh, max_hh, n_seg;

  /* alloc maximum number of segments */
  simLineSegment *segments = (simLineSegment *)malloc(n*sizeof(simLineSegment));

  width = simulation->canvas->w;
  height = simulation->canvas->h;
  fill_mode = simulation->canvas->fill_mode;
  
  simPolyMakeSegments(segments, &n_seg, poly, n, &max_hh, &y_max, &y_min);
  
  if (y_min > height-1 || y_max < 0)
  {
    free(segments);
    return;
  }

  if (y_min < 0) 
    y_min = 0;

  /* number of horizontal lines */
  if (y_max > height-1)
    num_lines = height-y_min;
  else
    num_lines = y_max-y_min+1;

  /* will store all horizontal intervals for each horizontal line,
     will be used to draw the antialiased and incomplete pixels */
  line_int_list = malloc(sizeof(simIntervalList)*num_lines);
  memset(line_int_list, 0, sizeof(simIntervalList)*num_lines);

  /* buffer to store the current horizontal intervals during the fill of an horizontal line */
  xx = (int*)malloc((n+1)*sizeof(int));    /* allocated to the maximum number of possible intervals in one line */
  hh = (int*)malloc((2*max_hh)*sizeof(int));

  /* for all horizontal lines between y_max and y_min */
  for(y = y_max; y >= y_min; y--)
  {
    xx_count = simPolyFindHorizontalIntervals(segments, n_seg, xx, hh, y, height);
    if (xx_count < 2)
      continue;

    line_il = line_int_list+(y-y_min);
    simLineIntervallInit(line_il, xx_count*2);

    /* for all intervals, fill the interval */
    for(i = 0; i < xx_count; i += 2)  /* process only pairs */
    {
      /* fills only pairs of intervals, */          
      simFillHorizLine(simulation, xx[i], y, xx[i+1]);
      simLineIntervallAdd(line_il, xx[i], xx[i+1]);

      if ((fill_mode == CD_WINDING) &&                     /* NOT EVENODD */
          ((i+2 < xx_count) && (xx[i+1] < xx[i+2])) && /* avoid single point intervals */
           simIsPointInPolyWind(poly, n, (xx[i+1]+xx[i+2])/2, y)) /* the next interval is inside the polygon */
      {
        simFillHorizLine(simulation, xx[i+1], y, xx[i+2]);
        simLineIntervallAdd(line_il, xx[i+1], xx[i+2]);
      }
    }
  }

  free(xx);
  free(hh);
  free(segments);

  /* Once the polygon has been filled, now let's draw the
   * antialiased and incomplete pixels at the edges */

  if (y_max > height-1)
    y_max = height-1;

  /* Go through all line segments of the polygon */
  for(i = 0; i < n; i++)
  {
    i1 = (i+1)%n;
    simPolyAAPixels(simulation->canvas, line_int_list, y_min, y_max, poly[i].x, poly[i].y, poly[i1].x, poly[i1].y);
  }

  for (i = 0; i < num_lines; i++)
  {
    if (line_int_list[i].xx) 
      free(line_int_list[i].xx);
  }
  free(line_int_list);
}

/*************************************************************************************/
/*************************************************************************************/

#define _cdLineDrawPixel(_canvas, _x1, _y1, _ls, _fgcolor)      \
{                                                               \
  if (_ls & 1)                                                  \
    _canvas->cxPixel(_canvas->ctxcanvas, _x1, _y1, _fgcolor);   \
}

static double myhypot ( double x, double y )
{
 return sqrt ( x*x + y*y );
}

void simLineThick(cdCanvas* canvas, int x1, int y1, int x2, int y2)
{
  const int interior = canvas->interior_style;
  const int width = canvas->line_width;
  const int style = canvas->line_style;

  const int dx = x2-x1;
  const int dy = y2-y1;

  const double len = myhypot(dx,dy);

  const double dnx = dx/len;
  const double dny = dy/len;

  const int w1 = (int)width/2;
  const int w2 = width-w1;

  const int n1x = cdRound( w1*dny);
  const int n1y = cdRound(-w1*dnx);

  const int n2x = cdRound(-w2*dny);
  const int n2y = cdRound( w2*dnx);

  const int p1x = x1 + n1x;
  const int p1y = y1 + n1y;
  const int p2x = x1 + n2x;
  const int p2y = y1 + n2y;
  const int p3x = p2x + dx;
  const int p3y = p2y + dy;
  const int p4x = p1x + dx;
  const int p4y = p1y + dy;

  cdPoint poly[4];

  cdCanvasLineWidth(canvas, 1);
  cdCanvasInteriorStyle(canvas, CD_SOLID);
  cdCanvasLineStyle(canvas, CD_CONTINUOUS);

  poly[0].x = p1x;
  poly[0].y = p1y;
  poly[1].x = p2x;
  poly[1].y = p2y;
  poly[2].x = p3x;
  poly[2].y = p3y;
  poly[3].x = p4x;
  poly[3].y = p4y;

  simPolyFill(canvas->simulation, poly, 4);

  cdCanvasLineWidth(canvas, width);
  cdCanvasInteriorStyle(canvas, interior);
  cdCanvasLineStyle(canvas, style);
}

void simfLineThick(cdCanvas* canvas, double x1, double y1, double x2, double y2)
{
  const int interior = canvas->interior_style;
  const int width = canvas->line_width;
  const int style = canvas->line_style;

  const double dx = x2-x1;
  const double dy = y2-y1;

  const double len = myhypot(dx,dy);

  const double dnx = dx/len;
  const double dny = dy/len;

  const double w1 = width/2.0;
  const double w2 = width-w1;

  const double n1x =  w1*dny;
  const double n1y = -w1*dnx;

  const double n2x = -w2*dny;
  const double n2y =  w2*dnx;

  const double p1x = x1 + n1x;
  const double p1y = y1 + n1y;
  const double p2x = x1 + n2x;
  const double p2y = y1 + n2y;
  const double p3x = p2x + dx;
  const double p3y = p2y + dy;
  const double p4x = p1x + dx;
  const double p4y = p1y + dy;

  cdPoint poly[4];

  cdCanvasLineWidth(canvas, 1);
  cdCanvasInteriorStyle(canvas, CD_SOLID);
  cdCanvasLineStyle(canvas, CD_CONTINUOUS);

  poly[0].x = _cdRound(p1x);
  poly[0].y = _cdRound(p1y);
  poly[1].x = _cdRound(p2x);
  poly[1].y = _cdRound(p2y);
  poly[2].x = _cdRound(p3x);
  poly[2].y = _cdRound(p3y);
  poly[3].x = _cdRound(p4x);
  poly[3].y = _cdRound(p4y);

  simPolyFill(canvas->simulation, poly, 4);

  cdCanvasLineWidth(canvas, width);
  cdCanvasInteriorStyle(canvas, interior);
  cdCanvasLineStyle(canvas, style);
}

void simLineThin(cdCanvas* canvas, int x1, int y1, int x2, int y2)
{
  unsigned short ErrorInc, ErrorAcc;
  unsigned short ErrorAccTemp, Weighting;
  int DeltaX, DeltaY, XDir;
  long aa_fgcolor;
  unsigned char alpha = cdAlpha(canvas->foreground), aa_alpha1, aa_alpha2;
  int no_antialias = !(canvas->simulation->antialias);
  unsigned short int ls;
  long fgcolor = canvas->foreground;

  if (simLineStyleNoReset == 2)
    ls = simLineStyleLastBits;
  else
  {
    ls = simLineStyleBitTable[canvas->line_style];

    if (simLineStyleNoReset == 1)
      simLineStyleNoReset = 2;
  }

  /* Make sure p2.y > p1.y */
  if (y1 > y2) 
  {
    _cdSwapInt(y1, y2);
    _cdSwapInt(x1, x2);
  }

  /* Draw the initial pixel, which is always exactly intersected by
      the line and so needs no weighting */
  _cdLineDrawPixel(canvas, x1, y1, ls, fgcolor);
  ls = simRotateLineStyle(ls);

  DeltaX = x2 - x1;
  if (DeltaX >= 0)
    XDir = 1;
  else 
  {
    XDir = -1;
    DeltaX = -DeltaX; /* make DeltaX positive */
  }

  /* Special-case horizontal, vertical, and diagonal lines, which
  require no weighting because they go right through the center of
  every pixel */
  DeltaY = y2 - y1;
  if (DeltaY == 0) 
  {
    /* Horizontal line */
    while (DeltaX-- != 0) 
    {
      x1 += XDir;
      _cdLineDrawPixel(canvas, x1, y1, ls, fgcolor);
      ls = simRotateLineStyle(ls);
    }
    simLineStyleLastBits = ls;
    return;
  }

  if (DeltaX == 0) 
  {
    /* Vertical line */
    do 
    {
      y1++;
      _cdLineDrawPixel(canvas, x1, y1, ls, fgcolor);
      ls = simRotateLineStyle(ls);
    } while (--DeltaY != 0);
    simLineStyleLastBits = ls;
    return;
  }

  if (DeltaX == DeltaY) 
  {
    /* Perfect Diagonal line */
    do 
    {
      x1 += XDir;
      y1++;
      _cdLineDrawPixel(canvas, x1, y1, ls, fgcolor);
      ls = simRotateLineStyle(ls);
    } while (--DeltaY != 0);
    simLineStyleLastBits = ls;
    return;
  }

  /* Line is not horizontal, diagonal, or vertical */

  ErrorAcc = 0;  /* initialize the line error accumulator to 0 */

  /* Is this an X-major or Y-major line? */
  if (DeltaY > DeltaX) 
  {
    /* Y-major line; calculate 16-bit fixed-point fractional part of a
    pixel that X advances each time Y advances 1 pixel, truncating the
    result so that we won't overrun the endpoint along the X axis */
    ErrorInc = (unsigned short)(((unsigned long)DeltaX << 16) / (unsigned long)DeltaY);

    /* Draw all pixels other than the first and last */
    while (--DeltaY) 
    {
      ErrorAccTemp = ErrorAcc;   /* remember currrent accumulated error */
      ErrorAcc += ErrorInc;      /* calculate error for next pixel */
      if (ErrorAcc <= ErrorAccTemp) 
      {
        /* The error accumulator turned over, so advance the X coord */
        x1 += XDir;
      }

      y1++; /* Y-major, so always advance Y */

      Weighting = ErrorAcc >> INTENSITYSHIFT;

      if (no_antialias)
      {
        if (Weighting < 128)
          _cdLineDrawPixel(canvas, x1, y1, ls, fgcolor)
        else
          _cdLineDrawPixel(canvas, x1 + XDir, y1, ls, fgcolor)
        ls = simRotateLineStyle(ls);
      }
      else
      {
        /* The IntensityBits most significant bits of ErrorAcc give us the
        intensity weighting for this pixel, and the complement of the
        weighting for the paired pixel.
        Combine the Weighting with the existing alpha,
        When Weighting is zero alpha must be fully preserved. */
        aa_alpha1 = (unsigned char)(((255-Weighting) * alpha) / 255);
        aa_alpha2 = (unsigned char)((Weighting * alpha) / 255);
        
        aa_fgcolor = cdEncodeAlpha(fgcolor, aa_alpha1);
        _cdLineDrawPixel(canvas, x1, y1, ls, aa_fgcolor);
        aa_fgcolor = cdEncodeAlpha(fgcolor, aa_alpha2);
        _cdLineDrawPixel(canvas, x1 + XDir, y1, ls, aa_fgcolor);
        ls = simRotateLineStyle(ls);
      }
    }
    /* Draw the final pixel, which is always exactly intersected by the line
    and so needs no weighting */
    _cdLineDrawPixel(canvas, x2, y2, ls, fgcolor);
    ls = simRotateLineStyle(ls);
  }
  else
  {
    /* It's an X-major line; calculate 16-bit fixed-point fractional part of a
    pixel that Y advances each time X advances 1 pixel, truncating the
    result to avoid overrunning the endpoint along the X axis */
    ErrorInc = (unsigned short)(((unsigned long)DeltaY << 16) / (unsigned long)DeltaX);

    /* Draw all pixels other than the first and last */
    while (--DeltaX) 
    {
      ErrorAccTemp = ErrorAcc;   /* remember currrent accumulated error */
      ErrorAcc += ErrorInc;      /* calculate error for next pixel */
      if (ErrorAcc <= ErrorAccTemp) 
      {
        /* The error accumulator turned over, so advance the Y coord */
        y1++;
      }

      x1 += XDir; /* X-major, so always advance X */

      Weighting = ErrorAcc >> INTENSITYSHIFT;

      if (no_antialias)
      {
        if (Weighting < 128)
          _cdLineDrawPixel(canvas, x1, y1, ls, fgcolor)
        else
          _cdLineDrawPixel(canvas, x1, y1+1, ls, fgcolor)
        ls = simRotateLineStyle(ls);
      }
      else
      {
        /* The IntensityBits most significant bits of ErrorAcc give us the
        intensity weighting for this pixel, and the complement of the
        weighting for the paired pixel.
        Combine the Weighting with the existing alpha,
        When Weighting is zero alpha must be fully preserved. */
        aa_alpha1 = (unsigned char)(((255-Weighting) * alpha) / 255);
        aa_alpha2 = (unsigned char)((Weighting * alpha) / 255);

        aa_fgcolor = cdEncodeAlpha(fgcolor, aa_alpha1);
        _cdLineDrawPixel(canvas, x1, y1, ls, aa_fgcolor);
        aa_fgcolor = cdEncodeAlpha(fgcolor, aa_alpha2);
        _cdLineDrawPixel(canvas, x1, y1+1, ls, aa_fgcolor);
        ls = simRotateLineStyle(ls);
      }
    }

    /* Draw the final pixel, which is always exactly intersected by the line
    and so needs no weighting */
    _cdLineDrawPixel(canvas, x2, y2, ls, fgcolor);
    ls = simRotateLineStyle(ls);
  }

  simLineStyleLastBits = ls;
}

void simfLineThin(cdCanvas* canvas, double x1, double y1, double x2, double y2, int *last_xi_a, int *last_yi_a, int *last_xi_b, int *last_yi_b)
{
  double DeltaX, DeltaY, a, b;
  long aa_fgcolor;
  unsigned char alpha = cdAlpha(canvas->foreground), aa_alpha1, aa_alpha2;
  int no_antialias = !(canvas->simulation->antialias);
  int yi, xi, update_a = 1, update_b = 1;
  unsigned short int ls;
  long fgcolor = canvas->foreground;

  if (simLineStyleNoReset == 2)
    ls = simLineStyleLastBits;
  else
  {
    ls = simLineStyleBitTable[canvas->line_style];

    if (simLineStyleNoReset == 1)
      simLineStyleNoReset = 2;
  }

  DeltaX = fabs(x2 - x1);
  DeltaY = fabs(y2 - y1);

  if (DeltaX == 0 && DeltaY == 0) /* p1==p2 */
    return;

  if (DeltaX > 0.0001)
  {
    a = (y1-y2)/(x1-x2);
    b = y1 - a*x1;
  }
  else
  {
    a = 0;
    b = x1;
  }

  /* NOTICE: all the complexity of this function 
  is related to check and update the previous point */

  /* Is this an X-major or Y-major line? */
  if (DeltaY > DeltaX) 
  {
    /* Increment in Y */
    int y1i = _cdRound(y1), 
        y2i = _cdRound(y2);
    int yi_first = y1i;
    int yi_last = y2i, xi_last = 0;

    if (y1i > y2i)
      _cdSwapInt(y1i, y2i);

    for (yi = y1i; yi <= y2i; yi++)
    {
      double x;
      if (a)
        x = (yi - b)/a;
      else
        x = b;

      xi = (int)floor(x);

      /* if at the last pixel, store the return value */
      if (yi == yi_last)
        xi_last = xi;

      /* Combine the Weighting with the existing alpha,
      When Weighting is zero alpha must be fully preserved. */
      aa_alpha1 = (unsigned char)((1.0-(x - xi)) * alpha);
      aa_alpha2 = (unsigned char)((x - xi) * alpha);

      if (no_antialias)
      {
        if (aa_alpha1 > 128)
          _cdLineDrawPixel(canvas, xi, yi, ls, fgcolor)
        else
          _cdLineDrawPixel(canvas, xi+1, yi, ls, fgcolor)
      }
      else
      {
        if (yi == yi_first)
        {
          if (yi == yi_last)  /* one pixel only */
          {
            update_a = 0;
            update_b = 0;
          }

          /* if at first, compare with the last two previously drawn */
          /* if the new is equal to the previous, do NOT draw */
          if ((xi != *last_xi_a || yi != *last_yi_a) &&
              (xi != *last_xi_b || yi != *last_yi_b))
          {
            aa_fgcolor = cdEncodeAlpha(fgcolor, aa_alpha1);
            _cdLineDrawPixel(canvas, xi, yi, ls, aa_fgcolor);

            if (yi == yi_last)  /* one pixel only */
              update_a = 1;
          }

          if ((xi+1 != *last_xi_a || yi != *last_yi_a) &&
              (xi+1 != *last_xi_b || yi != *last_yi_b))
          {
            aa_fgcolor = cdEncodeAlpha(fgcolor, aa_alpha2);
            _cdLineDrawPixel(canvas, xi+1, yi, ls, aa_fgcolor);

            if (yi == yi_last)  /* one pixel only */
              update_b = 1;
          }
        }
        else
        {
          aa_fgcolor = cdEncodeAlpha(fgcolor, aa_alpha1);
          _cdLineDrawPixel(canvas, xi, yi, ls, aa_fgcolor);
          aa_fgcolor = cdEncodeAlpha(fgcolor, aa_alpha2);
          _cdLineDrawPixel(canvas, xi+1, yi, ls, aa_fgcolor);
        }
      }

      ls = simRotateLineStyle(ls);
    }

    if (update_a)
    {
      *last_xi_a = xi_last;
      *last_yi_a = yi_last;
    }
    if (update_b)
    {
      *last_xi_b = xi_last+1;
      *last_yi_b = yi_last;
    }
  }
  else
  {
    /* Increment in X */
    int x1i = _cdRound(x1), 
        x2i = _cdRound(x2);
    int xi_first = x1i;
    int xi_last = x2i, yi_last = 0;

    if (x1i > x2i)
      _cdSwapInt(x1i, x2i);

    for (xi = x1i; xi <= x2i; xi++)
    {
      double y = a*xi + b;
      yi = (int)floor(y);

      /* if at the last pixel, store the return value */
      if (xi == xi_last)
        yi_last = yi;

      /* Combine the Weighting with the existing alpha,
      When Weighting is zero alpha must be fully preserved. */
      aa_alpha1 = (unsigned char)((1.0-(y - yi)) * alpha);
      aa_alpha2 = (unsigned char)((y - yi) * alpha);

      if (no_antialias)
      {
        if (aa_alpha1 > 128)
          _cdLineDrawPixel(canvas, xi, yi, ls, fgcolor)
        else
          _cdLineDrawPixel(canvas, xi, yi+1, ls, fgcolor)
      }
      else
      {
        if (xi == xi_first)
        {
          if (xi == xi_last)  /* one pixel only */
          {
            update_a = 0;
            update_b = 0;
          }

          /* if at first, compare with the last to draw */
          /* if new is equal to the previous, do NOT draw */
          if ((xi != *last_xi_a || yi != *last_yi_a) &&
              (xi != *last_xi_b || yi != *last_yi_b))
          {
            aa_fgcolor = cdEncodeAlpha(fgcolor, aa_alpha1);
            _cdLineDrawPixel(canvas, xi, yi, ls, aa_fgcolor);

            if (xi == xi_last) /* one pixel only */
              update_a = 1;
          }

          if ((xi != *last_xi_a || yi+1 != *last_yi_a) &&
              (xi != *last_xi_b || yi+1 != *last_yi_b))
          {
            aa_fgcolor = cdEncodeAlpha(fgcolor, aa_alpha2);
            _cdLineDrawPixel(canvas, xi, yi+1, ls, aa_fgcolor);

            if (xi == xi_last) /* one pixel only */
              update_b = 1;
          }
        }
        else
        {
          aa_fgcolor = cdEncodeAlpha(fgcolor, aa_alpha1);
          _cdLineDrawPixel(canvas, xi, yi, ls, aa_fgcolor);
          aa_fgcolor = cdEncodeAlpha(fgcolor, aa_alpha2);
          _cdLineDrawPixel(canvas, xi, yi+1, ls, aa_fgcolor);
        }
      }

      ls = simRotateLineStyle(ls);
    }

    if (update_a)
    {
      *last_xi_a = xi_last;
      *last_yi_a = yi_last;
    }
    if (update_b)
    {
      *last_xi_b = xi_last;
      *last_yi_b = yi_last+1;
    }
  }

  simLineStyleLastBits = ls;
}
