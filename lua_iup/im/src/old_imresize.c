/** \file
 * \brief Old resize/stretch functions
 *
 * See Copyright Notice in im_lib.h
 * $Id: old_imresize.c,v 1.1 2008/10/17 06:10:16 scuri Exp $
 */

#include "old_im.h"

#include <stdlib.h>
#include <math.h>
#include <memory.h>


/*
  BILINEAR INTERPOLATION:

  x' = floor(x)                                        f(x'   , y')    = fll
  y' = floor(y)                                        f(x'   , y'+ 1) = flh
                                                       f(x'+ 1, y')    = fhl
  t = x - x'                                           f(x'+ 1, y'+ 1) = fhh
  u = y - y'

  f(x,y) = (1-t) * (1-u) * f(x'   , y'),
           (1-t) *    u  * f(x'   , y'+ 1),
               t * (1-u) * f(x'+ 1, y'),
               t *    u  * f(x'+ 1, y'+ 1)

  f(x,y) =          fll +                              (re-arranging)
               t * (fhl - fll),
               u * (flh - fll),
           u * t * (fhh - flh - fhl + fll)
          
*/

void imResize(int src_width, int src_height, unsigned char *src_map, int dst_width, int dst_height, unsigned char *dst_map)
{
  /* Do bilinear interpolation */

	unsigned char *line_mapl, *line_maph;
  double t, u, src_x, src_y, factor;
  int fhh, fll, fhl, flh, xl, yl, xh, yh, x, y;

  int *XL = (int*)malloc(dst_width * sizeof(int));
  double *T = (double*)malloc(dst_width * sizeof(double));

	factor = (double)(src_width-1) / (double)(dst_width-1);
  for (x = 0; x < dst_width; x++)
  {
	  src_x = x * factor;
	  xl = (int)floor(src_x);
	  T[x] = src_x - xl;
	  XL[x] = xl;
  }

	factor = (double)(src_height-1) / (double)(dst_height-1);

  for (y = 0; y < dst_height; y++)
  {
	  src_y = y * factor;
	  yl = (int)floor(src_y);
    yh = (yl == src_height-1)? yl: yl + 1;
	  u = src_y - yl;

	  line_mapl = src_map + yl * src_width;
	  line_maph = src_map + yh * src_width;

	  for (x = 0; x < dst_width; x++)
	  {
		  xl = XL[x];
      xh = (xl == src_width-1)? xl: xl + 1;
		  t = T[x];

		  fll = line_mapl[xl];
		  fhl = line_mapl[xh];
		  flh = line_maph[xl];
		  fhh = line_maph[xh];

		  *(dst_map++) = (unsigned char)(u * t * (fhh - flh - fhl + fll) + t * (fhl - fll) + u * (flh - fll) + fll);
	  }
  }

  free(XL);
  free(T);
}

void imStretch(int src_width, int src_height, unsigned char *src_map, int dst_width, int dst_height, unsigned char *dst_map)
{
  int x, y, offset;
  double factor;
  unsigned char *line_map;
	int* XTab = (int*)malloc(dst_width*sizeof(int));

  /* initialize convertion tables to speed up the stretch process */
	factor = (double)(src_width-1) / (double)(dst_width-1);
	for(x = 0; x < dst_width; x++)
		XTab[x] = (int)(factor * x + 0.5);

	factor = (double)(src_height-1) / (double)(dst_height-1);

  line_map = src_map;

  for (y = 0; y < dst_height; y++)
  {
    for (x = 0; x < dst_width; x++)
    {
      offset = XTab[x];
      *(dst_map++) = line_map[offset];
    }

    offset = ((int)(factor * y + 0.5)) * src_width;
		line_map = src_map + offset;
  }

  free(XTab);
}

