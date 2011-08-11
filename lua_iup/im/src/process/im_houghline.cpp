/** \file
 * \brief Hough Transform
 *
 * See Copyright Notice in im_lib.h
 * $Id: im_houghline.cpp,v 1.1 2008/10/17 06:16:33 scuri Exp $
 */

#include <im.h>
#include <im_util.h>
#include <im_complex.h>
#include <im_convert.h>
#include <im_counter.h>

#include "im_process_glo.h"

#include <stdlib.h>
#include <memory.h>


#ifndef M_PI
#define M_PI    3.14159265358979323846
#endif

static double *costab=NULL, *sintab=NULL;

static int hgAbs(int x)
{
  return x < 0? -x: x;
}

typedef struct _point
{
  int rho, theta, count;
} point;

typedef struct _listnode
{
  struct _listnode *next;
  point pt;
} listnode;

static listnode* listnew(point *pt)
{
  listnode* node = (listnode*)malloc(sizeof(listnode));
  node->next = NULL;
  node->pt = *pt;
  return node;
}

static listnode* listadd(listnode* node, point *pt)
{
  node->next = listnew(pt);
  return node->next;
}

/* minimum angle to match similar angles */
#define THETA_DELTA1   0.05  /* radians */
#define THETA_DELTA2   3     /* degrees */

static int ptNear(point* pt1, point* pt2, int rho_delta)
{
  int theta_diff = hgAbs(pt1->theta - pt2->theta);
  if ((hgAbs(pt1->rho - pt2->rho) < rho_delta && theta_diff < THETA_DELTA2) ||
      (hgAbs(pt1->rho + pt2->rho) < rho_delta && 180-theta_diff < THETA_DELTA2))
  {
    if (pt2->count > pt1->count)
      return 2;   /* replace the line */
    else
      return 1;
  }
  else
    return 0;
}

static listnode* listadd_filtered(listnode* list, listnode* cur_node, point *pt, int rho_delta)
{
  int ret;
  listnode* lt = list;
  while (lt)
  {
    ret = ptNear(&lt->pt, pt, rho_delta);
    if (ret)
    {
      if (ret == 2)
        lt->pt = *pt;  /* replace the line */
      return cur_node;
    }
    lt = lt->next;
  }

  cur_node->next = listnew(pt);
  return cur_node->next;
}

/*C* Initial version from XITE

        houghLine
        $Id: im_houghline.cpp,v 1.1 2008/10/17 06:16:33 scuri Exp $
        Copyright 1990, Blab, UiO
        Image processing lab, Department of Informatics
        University of Oslo
        E-mail: blab@ifi.uio.no
________________________________________________________________

  houghLine - Hough transform for line detection

  Description:
    Performs a Hough transform to detect lines. Every band in the
    input image 'inimage' is transformed to a two dimensional
    Hough space, a (theta, rho) space.

		After creating the transform, the Hough space may be searched
		for local maxima. Within each band, only the largest local
		maximum (maxima) within a 'ws'x'ws' area is registered.
		Besides, only maxima with number of updates above a limit
		given by the ul option are used.

		updateLimit determines the minimum number of updates for a maximum
		to be used. The minimum number is determined from 'updateLimit'
		and the size of the hough space image:
		| updateLimit * MAX(horizontal size, vertical size)
		Default: 0.1.

    All pixels above zero in the 'input' band are
		transformed to (theta,rho) space in the 'output'
		band. The 'input' band may have any size, while
		the 'output' band currently must be at least
		| xsize: 180
		| ysize: 2 * sqrt(inputXsize*inputXsize +
		|             inputYsize*inputYsize) + 1

		Notice that band x coordinates 1..180 correspond
		to angles theta = 0 .. 179, and y coordinates
		1..YSIZE correspond to rho = -(ysize/2) .. ysize/2.

  Restrictions:
    'input' must have pixel type imbyte.
    'output' must have pixel type int.

  Author:		Tor Lønnestad, BLAB, Ifi, UiO
*/


static int houghLine(const imImage* input, imImage* output, int counter)
{
  int ixsize, iysize, ixhalf, iyhalf, thetamax, x, y, rho, theta, rhomax;
  imbyte *input_map = (imbyte*)input->data[0];
  int *output_map = (int*)output->data[0];

  ixsize = input->width;
  iysize = input->height;
  ixhalf = ixsize/2;
  iyhalf = iysize/2;

  thetamax = output->width;   /* theta max = 180 */
  rhomax = output->height/2;  /* rho shift to 0, -rmax <= r <= +rmax */

  costab = (double*)malloc(thetamax*sizeof(double));
  sintab = (double*)malloc(thetamax*sizeof(double));

  for (theta=0; theta < thetamax; theta++)
  {
    double th = (M_PI*theta)/thetamax;
    costab[theta] = cos(th);
    sintab[theta] = sin(th);
  }

  for (y=0; y < iysize; y++)
  {
    for (x=0; x < ixsize; x++)
    {
      if (input_map[y*ixsize + x])
      {
        for (theta=0; theta < thetamax; theta++)
        {
          rho = imRound((x-ixhalf)*costab[theta] + (y-iyhalf)*sintab[theta]);
          if (rho > rhomax) continue;
          if (rho < -rhomax) continue;
          output_map[(rho+rhomax)*thetamax + theta]++;
	      }
      }
    }

    if (!imCounterInc(counter))
    {
      free(costab); costab = NULL;
      free(sintab); sintab = NULL;
      return 0;
    }
  }

  free(costab); costab = NULL;
  free(sintab); sintab = NULL;

  return 1;
}

static listnode* findMaxima(const imImage* hough_points, int *line_count, const imImage* hough)
{
  int x, y, xsize, ysize, rhomax, offset, rho_delta = 0;
  listnode* maxima = NULL, *cur_node = NULL;
  point pt;
  imbyte *map = (imbyte*)hough_points->data[0];
  int *hough_map = NULL;

  xsize = hough_points->width;   /* X = theta */
  ysize = hough_points->height;  /* Y = rho   */
  rhomax = ysize/2;
  
  if (hough)
  {
    hough_map = (int*)hough->data[0];
    rho_delta = (int)(rhomax*tan(THETA_DELTA1));
  }

  for (y=0; y < ysize; y++)
  {
    for (x=0; x < xsize; x++)
    {
      offset = y*xsize + x;

      if (map[offset])
      {
        pt.theta = x;
        pt.rho = y-rhomax;

        if (!maxima)
        {
          cur_node = maxima = listnew(&pt);
          (*line_count)++;
        }
        else
        {
          if (hough_map)
          {
            listnode* old_node = cur_node;
            pt.count = hough_map[offset];
            cur_node = listadd_filtered(maxima, cur_node, &pt, rho_delta);
            if (cur_node != old_node)
              (*line_count)++;
          }
          else
          {
            cur_node = listadd(cur_node, &pt);
            (*line_count)++;
          }
        }
	    }
    }
  }

  return maxima;
}

#define SWAPINT(a, b) {int t = a; a = b; b = t; }

static void drawLine(imImage* image, int theta, int rho)
{
  int xsize, ysize, xstart, xstop, ystart, ystop, xhalf, yhalf;
  float a, b;
  imbyte *map = (imbyte*)image->data[0];

  xsize = image->width;
  ysize = image->height;
  xhalf = xsize/2;
  yhalf = ysize/2;

  if (theta == 0)  /* vertical line */
  {
    int y;
    if (rho+xhalf < 0 || rho+xhalf > xsize-1) return;
    for (y=0; y < ysize; y++)
      map[y*xsize + rho+xhalf]=254;

    return;
  }

  if (theta == 90)  /* horizontal line */
  {
    int x;
    if (rho+yhalf < 0 || rho+yhalf > ysize-1) return;
    for (x=0; x < xsize; x++)
      map[(rho+yhalf)*xsize + x]=254;

    return;
  }

  a = (float)(-costab[theta]/sintab[theta]);
  b = (float)((rho + xhalf*costab[theta] + yhalf*sintab[theta])/sintab[theta]);

  {
    int x[2];
    int y[2];
    int c = 0;
    int y1 = imRound(b);              /* x = 0 */
    int y2 = imRound(a*(xsize-1)+b);  /* x = xsize-1 */

    int x1 = imRound(-b/a);           /* y = 0 */
    int x2 = imRound((ysize-1-b)/a);  /* y = ysize-1 */

    if (y1 >= 0 && y1 < ysize)
    {
      y[c] = y1;
      x[c] = 0;
      c++;
    }

    if (y2 >= 0 && y2 < ysize)
    {
      y[c] = y2;
      x[c] = xsize-1;
      c++;
    }

    if (c < 2 && x1 >= 0 && x1 < xsize)
    {
      x[c] = x1;
      y[c] = 0;
      c++;
    }

    if (c < 2 && x2 >= 0 && x2 < xsize)
    {
      x[c] = x2;
      y[c] = ysize-1;
      c++;
    }

    if (c < 2) return;

    ystart = y[0];
    xstart = x[0];
    ystop = y[1];
    xstop = x[1];
  }

  {
    int x, y;
    if (45 <= theta && theta <= 135)
    {
      if (xstart > xstop)
        SWAPINT(xstart, xstop);

      for (x=xstart; x <= xstop; x++)
      {
        y = imRound(a*x + b);
        if (y < 0) continue;
        if (y > ysize-1) continue;
        map[y*xsize + x]=254;
      }
    }
    else
    {
      if (ystart > ystop)
        SWAPINT(ystart, ystop);

      for (y=ystart; y <= ystop; y++)
      {
        x = imRound((y-b)/a);
        if (x < 0) continue;
        if (x > xsize-1) continue;
        map[y*xsize + x]=254;
      }
    }
  }
}

int imProcessHoughLines(const imImage* image, imImage *NewImage)
{
  int counter = imCounterBegin("Hough Line Transform");
  imCounterTotal(counter, image->height, "Processing...");

  int ret = houghLine(image, NewImage, counter);

  imCounterEnd(counter);

  return ret;
}

static void DrawPoints(imImage *image, listnode* maxima)
{
  listnode* cur_node;
  while (maxima)
  {
    cur_node = maxima;
    drawLine(image, cur_node->pt.theta, cur_node->pt.rho);
    maxima = cur_node->next;
    free(cur_node);
  }
}

static void ReplaceColor(imImage* NewImage)
{
  int i;
  imbyte* map = (imbyte*)NewImage->data[0];

  NewImage->color_space = IM_MAP;
  NewImage->palette[254] = imColorEncode(255, 0, 0);

  for (i = 0; i < NewImage->count; i++)
  {
    if (map[i] == 254)
      map[i] = 255;
  }
}

int imProcessHoughLinesDraw(const imImage* original_image, const imImage *hough, const imImage *hough_points, imImage *NewImage)
{
  int theta, line_count = 0;

  if (original_image != NewImage)
    imImageCopyData(original_image, NewImage);

  listnode* maxima = findMaxima(hough_points, &line_count, hough);

  ReplaceColor(NewImage);

  costab = (double*)malloc(180*sizeof(double));
  sintab = (double*)malloc(180*sizeof(double));

  for (theta=0; theta < 180; theta++)
  {
    double th = (M_PI*theta)/180.;
    costab[theta] = cos(th);
    sintab[theta] = sin(th);
  }

  DrawPoints(NewImage, maxima);

  free(costab); costab = NULL;
  free(sintab); sintab = NULL;

  return line_count;
}

