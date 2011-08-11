/** \file
 * \brief RGB2Map Conversion
 *
 * See Copyright Notice in cd.h
 */


#include <stdlib.h>
#include <stdio.h>

#include "cd.h"

#define PARM(a) a
typedef unsigned char byte;
/* RANGE forces a to be in the range b..c (inclusive) */
#define RANGE(a,b,c) { if (a < b) a = b;  if (a > c) a = c; }
typedef unsigned long  uu_long;
typedef unsigned short uu_short;
#define INT32 int
#define TRUE 1
#define FALSE 0


static void xvbzero(char *s, size_t len)
{
  for ( ; len>0; len--) *s++ = 0;
}

static void xvbcopy(char *src, char *dst, size_t len)
{
  /* determine if the regions overlap
   *
   * 3 cases:  src=dst, src<dst, src>dst
   *
   * if src=dst, they overlap completely, but nothing needs to be moved
   * if src<dst and src+len>dst then they overlap
   * if src>dst and src<dst+len then they overlap
   */

  if (src==dst || len<=0) return;    /* nothin' to do */
  
  if (src<dst && src+len>dst) {  /* do a backward copy */
    src = src + len - 1;
    dst = dst + len - 1;
    for ( ; len>0; len--, src--, dst--) *dst = *src;
  }

  else {  /* they either overlap (src>dst) or they don't overlap */
    /* do a forward copy */
    for ( ; len>0; len--, src++, dst++) *dst = *src;
  }
}

/****************************/
static int quick_map(const byte *red, const byte *green, const byte *blue, int w, int h, byte *map, byte *rmap, byte *gmap, byte *bmap, int *maxcol)
{
/* scans picture until it finds more than 'maxcol' different colors.  If it
finds more than 'maxcol' colors, it returns '0'.  If it DOESN'T, it does
the 24-to-8 conversion by simply sticking the colors it found into
a colormap, and changing instances of a color in pic24 into colormap
  indicies (in pic8) */
  
  unsigned long colors[256],col;
  int           i, nc, low, high, mid;
  const byte *pred, *pgreen, *pblue;
  byte *pix;
  
  /* put the first color in the table by hand */
  nc = 0;  mid = 0;  
  
  for (i=w*h,pred=red,pgreen=green,pblue=blue; i; i--) 
  {
    col  = (((uu_long) *pred++) << 16);  
    col += (((uu_long) *pgreen++) << 8);
    col +=  *pblue++;
    
    /* binary search the 'colors' array to see if it's in there */
    low = 0;  high = nc-1;
    while (low <= high) 
    {
      mid = (low+high)/2;
      if      (col < colors[mid]) high = mid - 1;
      else if (col > colors[mid]) low  = mid + 1;
      else break;
    }
    
    if (high < low) 
    { /* didn't find color in list, add it. */
      if (nc>=*maxcol) 
        return 0;

      xvbcopy((char *) &colors[low], (char *) &colors[low+1], (nc - low) * sizeof(uu_long));
      colors[low] = col;
      nc++;
    }
  }
  
  /* run through the data a second time, this time mapping pixel values in
  pic24 into colormap offsets into 'colors' */
  
  for (i=w*h,pred=red,pgreen=green,pblue=blue, pix=map; i; i--,pix++) 
  {
    col  = (((uu_long) *pred++) << 16);  
    col += (((uu_long) *pgreen++) << 8);
    col +=  *pblue++;
    
    /* binary search the 'colors' array.  It *IS* in there */
    low = 0;  high = nc-1;
    while (low <= high) 
    {
      mid = (low+high)/2;
      if      (col < colors[mid]) high = mid - 1;
      else if (col > colors[mid]) low  = mid + 1;
      else break;
    }
    
    if (high < low) 
      return 0;
    
    *pix = (unsigned char)mid;
  }
  
  /* and load up the 'desired colormap' */
  for (i=0; i<nc; i++) 
  {
    rmap[i] = (unsigned char)( colors[i]>>16);  
    gmap[i] = (unsigned char)((colors[i]>>8) & 0xff);
    bmap[i] = (unsigned char)( colors[i]     & 0xff);
  }
  
  *maxcol = nc;

  return 1;
}



/***************************************************************/
/* The following is based on jquant2.c from version 5          */
/* of the IJG JPEG software, which is                          */
/* Copyright (C) 1991-1994, Thomas G. Lane.                    */
/***************************************************************/

#define MAXNUMCOLORS  256	/* maximum size of colormap */

#define C0_SCALE 2		/* scale R distances by this much */
#define C1_SCALE 3		/* scale G distances by this much */
#define C2_SCALE 1		/* and B by this much */

#define HIST_C0_BITS  5		/* bits of precision in R histogram */
#define HIST_C1_BITS  6		/* bits of precision in G histogram */
#define HIST_C2_BITS  5		/* bits of precision in B histogram */

/* Number of elements along histogram axes. */
#define HIST_C0_ELEMS  (1<<HIST_C0_BITS)
#define HIST_C1_ELEMS  (1<<HIST_C1_BITS)
#define HIST_C2_ELEMS  (1<<HIST_C2_BITS)

/* These are the amounts to shift an input value to get a histogram index. */
#define C0_SHIFT  (8-HIST_C0_BITS)
#define C1_SHIFT  (8-HIST_C1_BITS)
#define C2_SHIFT  (8-HIST_C2_BITS)


typedef unsigned char JSAMPLE;
typedef JSAMPLE * JSAMPROW;

typedef uu_short histcell;	/* histogram cell; prefer an unsigned type */

typedef histcell * histptr;	/* for pointers to histogram cells */

typedef histcell hist1d[HIST_C2_ELEMS]; /* typedefs for the histogram array */
typedef hist1d hist2d[HIST_C1_ELEMS];
typedef hist2d hist3d[HIST_C0_ELEMS];

typedef short FSERROR;		/* 16 bits should be enough */
typedef int LOCFSERROR;		/* use 'int' for calculation temps */

typedef FSERROR *FSERRPTR;	/* pointer to error array */

typedef struct {
  /* The bounds of the box (inclusive); expressed as histogram indexes */
  int c0min, c0max;
  int c1min, c1max;
  int c2min, c2max;
  /* The volume (actually 2-norm) of the box */
  INT32 volume;
  /* The number of nonzero histogram cells within this box */
  long colorcount;
} box;
typedef box * boxptr;

/* Local state for the IJG quantizer */

static hist2d * sl_histogram;	/* pointer to the 3D histogram array */
static FSERRPTR sl_fserrors;	/* accumulated-errors array */
static int * sl_error_limiter;	/* table for clamping the applied error */
static int sl_on_odd_row;	/* flag to remember which row we are on */
static JSAMPROW sl_colormap[3];	/* selected colormap */
static int sl_num_colors;	/* number of selected colors */


static void   slow_fill_histogram PARM((const byte*, const byte*, const byte*, int));
static boxptr find_biggest_color_pop PARM((boxptr, int));
static boxptr find_biggest_volume PARM((boxptr, int));
static void   update_box PARM((boxptr));
static int    median_cut PARM((boxptr, int, int));
static void   compute_color PARM((boxptr, int));
static void   slow_select_colors PARM((int*));
static int    find_nearby_colors PARM((int, int, int, JSAMPLE []));
static void   find_best_colors PARM((int,int,int,int, JSAMPLE [], JSAMPLE []));
static void   fill_inverse_cmap PARM((int, int, int));
static void   slow_map_pixels PARM((const byte*, const byte*, const byte*, int, int, byte*));
static void   init_error_limit PARM((void));


/* Master control for slow quantizer. */
static int slow_quant(const byte *red, const byte *green, const byte *blue, int w, int h, byte *map, byte *rm, byte *gm, byte *bm, int *descols)
{
  size_t fs_arraysize = (w + 2) * (3 * sizeof(FSERROR));
  
  /* Allocate all the temporary storage needed */
  init_error_limit();

  sl_histogram = (hist2d *) malloc(sizeof(hist3d));
  sl_fserrors = (FSERRPTR) malloc(fs_arraysize);
  
  if (! sl_error_limiter || ! sl_histogram || ! sl_fserrors) 
  {
    if (sl_error_limiter) free(sl_error_limiter-255);
    if (sl_fserrors) free(sl_fserrors);
    if (sl_histogram) free(sl_histogram);
    return 1;
  }
  
  sl_colormap[0] = (JSAMPROW) rm;
  sl_colormap[1] = (JSAMPROW) gm;
  sl_colormap[2] = (JSAMPROW) bm;
  
  /* Compute the color histogram */
  slow_fill_histogram(red, green, blue, w*h);
  
  /* Select the colormap */
  slow_select_colors(descols);
  
  /* Zero the histogram: now to be used as inverse color map */
  xvbzero((char *) sl_histogram, sizeof(hist3d));
  
  /* Initialize the propagated errors to zero. */
  xvbzero((char *) sl_fserrors, fs_arraysize);
  sl_on_odd_row = FALSE;
  
  /* Map the image. */
  slow_map_pixels(red, green, blue, w, h, map);
  
  /* Release working memory. */
  free(sl_histogram);
  free(sl_error_limiter-255);
  free(sl_fserrors);

  return 0;
}


static void slow_fill_histogram(register const byte *red, register const byte *green, register const byte *blue, int numpixels)
{
  register histptr histp;
  register hist2d * histogram = sl_histogram;
  
  xvbzero((char *) histogram, sizeof(hist3d));
  
  while (numpixels-- > 0) 
  {
    /* get pixel value and index into the histogram */
    histp = & histogram[*red >> C0_SHIFT] [*green >> C1_SHIFT] [*blue >> C2_SHIFT];

    /* increment, check for overflow and undo increment if so. */
    if (++(*histp) <= 0)
      (*histp)--;

    red++;
    green++;
    blue++;
  }
}


static boxptr find_biggest_color_pop (boxptr boxlist, int numboxes)
{
  register boxptr boxp;
  register int i;
  register long maxc = 0;
  boxptr which = NULL;
  
  for (i = 0, boxp = boxlist; i < numboxes; i++, boxp++) {
    if (boxp->colorcount > maxc && boxp->volume > 0) {
      which = boxp;
      maxc = boxp->colorcount;
    }
  }
  return which;
}


static boxptr find_biggest_volume (boxptr boxlist, int numboxes)
{
  register boxptr boxp;
  register int i;
  register INT32 maxv = 0;
  boxptr which = NULL;
  
  for (i = 0, boxp = boxlist; i < numboxes; i++, boxp++) {
    if (boxp->volume > maxv) {
      which = boxp;
      maxv = boxp->volume;
    }
  }
  return which;
}


static void update_box (boxptr boxp)
{
  hist2d * histogram = sl_histogram;
  histptr histp;
  int c0,c1,c2;
  int c0min,c0max,c1min,c1max,c2min,c2max;
  INT32 dist0,dist1,dist2;
  long ccount;
  
  c0min = boxp->c0min;  c0max = boxp->c0max;
  c1min = boxp->c1min;  c1max = boxp->c1max;
  c2min = boxp->c2min;  c2max = boxp->c2max;
  
  if (c0max > c0min)
    for (c0 = c0min; c0 <= c0max; c0++)
      for (c1 = c1min; c1 <= c1max; c1++) {
        histp = & histogram[c0][c1][c2min];
        for (c2 = c2min; c2 <= c2max; c2++)
          if (*histp++ != 0) {
            boxp->c0min = c0min = c0;
            goto have_c0min;
          }
      }
have_c0min:
      if (c0max > c0min)
        for (c0 = c0max; c0 >= c0min; c0--)
          for (c1 = c1min; c1 <= c1max; c1++) {
            histp = & histogram[c0][c1][c2min];
            for (c2 = c2min; c2 <= c2max; c2++)
              if (*histp++ != 0) {
                boxp->c0max = c0max = c0;
                goto have_c0max;
              }
          }
have_c0max:
          if (c1max > c1min)
            for (c1 = c1min; c1 <= c1max; c1++)
              for (c0 = c0min; c0 <= c0max; c0++) {
                histp = & histogram[c0][c1][c2min];
                for (c2 = c2min; c2 <= c2max; c2++)
                  if (*histp++ != 0) {
                    boxp->c1min = c1min = c1;
                    goto have_c1min;
                  }
              }
have_c1min:
              if (c1max > c1min)
                for (c1 = c1max; c1 >= c1min; c1--)
                  for (c0 = c0min; c0 <= c0max; c0++) {
                    histp = & histogram[c0][c1][c2min];
                    for (c2 = c2min; c2 <= c2max; c2++)
                      if (*histp++ != 0) {
                        boxp->c1max = c1max = c1;
                        goto have_c1max;
                      }
                  }
have_c1max:
                  if (c2max > c2min)
                    for (c2 = c2min; c2 <= c2max; c2++)
                      for (c0 = c0min; c0 <= c0max; c0++) {
                        histp = & histogram[c0][c1min][c2];
                        for (c1 = c1min; c1 <= c1max; c1++, histp += HIST_C2_ELEMS)
                          if (*histp != 0) {
                            boxp->c2min = c2min = c2;
                            goto have_c2min;
                          }
                      }
have_c2min:
                      if (c2max > c2min)
                        for (c2 = c2max; c2 >= c2min; c2--)
                          for (c0 = c0min; c0 <= c0max; c0++) {
                            histp = & histogram[c0][c1min][c2];
                            for (c1 = c1min; c1 <= c1max; c1++, histp += HIST_C2_ELEMS)
                              if (*histp != 0) {
                                boxp->c2max = c2max = c2;
                                goto have_c2max;
                              }
                          }
have_c2max:
                          
                          dist0 = ((c0max - c0min) << C0_SHIFT) * C0_SCALE;
                          dist1 = ((c1max - c1min) << C1_SHIFT) * C1_SCALE;
                          dist2 = ((c2max - c2min) << C2_SHIFT) * C2_SCALE;
                          boxp->volume = dist0*dist0 + dist1*dist1 + dist2*dist2;
                          
                          ccount = 0;
                          for (c0 = c0min; c0 <= c0max; c0++)
                            for (c1 = c1min; c1 <= c1max; c1++) {
                              histp = & histogram[c0][c1][c2min];
                              for (c2 = c2min; c2 <= c2max; c2++, histp++)
                                if (*histp != 0) {
                                  ccount++;
                                }
                            }
                            boxp->colorcount = ccount;
}


static int median_cut (boxptr boxlist, int numboxes, int desired_colors)
{
  int n,lb;
  int c0,c1,c2,cmax;
  register boxptr b1,b2;
  
  while (numboxes < desired_colors) {
  /* Select box to split.
  * Current algorithm: by population for first half, then by volume.
    */
    if (numboxes*2 <= desired_colors) {
      b1 = find_biggest_color_pop(boxlist, numboxes);
    } else {
      b1 = find_biggest_volume(boxlist, numboxes);
    }
    if (b1 == NULL)		/* no splittable boxes left! */
      break;
    b2 = &boxlist[numboxes];	/* where new box will go */
    /* Copy the color bounds to the new box. */
    b2->c0max = b1->c0max; b2->c1max = b1->c1max; b2->c2max = b1->c2max;
    b2->c0min = b1->c0min; b2->c1min = b1->c1min; b2->c2min = b1->c2min;
    /* Choose which axis to split the box on.
    */
    c0 = ((b1->c0max - b1->c0min) << C0_SHIFT) * C0_SCALE;
    c1 = ((b1->c1max - b1->c1min) << C1_SHIFT) * C1_SCALE;
    c2 = ((b1->c2max - b1->c2min) << C2_SHIFT) * C2_SCALE;
    cmax = c1; n = 1;
    if (c0 > cmax) { cmax = c0; n = 0; }
    if (c2 > cmax) { n = 2; }
    switch (n) {
    case 0:
      lb = (b1->c0max + b1->c0min) / 2;
      b1->c0max = lb;
      b2->c0min = lb+1;
      break;
    case 1:
      lb = (b1->c1max + b1->c1min) / 2;
      b1->c1max = lb;
      b2->c1min = lb+1;
      break;
    case 2:
      lb = (b1->c2max + b1->c2min) / 2;
      b1->c2max = lb;
      b2->c2min = lb+1;
      break;
    }
    /* Update stats for boxes */
    update_box(b1);
    update_box(b2);
    numboxes++;
  }
  return numboxes;
}


static void compute_color (boxptr boxp, int icolor)
{
  /* Current algorithm: mean weighted by pixels (not colors) */
  /* Note it is important to get the rounding correct! */
  hist2d * histogram = sl_histogram;
  histptr histp;
  int c0,c1,c2;
  int c0min,c0max,c1min,c1max,c2min,c2max;
  long count;
  long total = 0;
  long c0total = 0;
  long c1total = 0;
  long c2total = 0;
  
  c0min = boxp->c0min;  c0max = boxp->c0max;
  c1min = boxp->c1min;  c1max = boxp->c1max;
  c2min = boxp->c2min;  c2max = boxp->c2max;
  
  for (c0 = c0min; c0 <= c0max; c0++)
    for (c1 = c1min; c1 <= c1max; c1++) {
      histp = & histogram[c0][c1][c2min];
      for (c2 = c2min; c2 <= c2max; c2++) {
        if ((count = *histp++) != 0) {
          total += count;
          c0total += ((c0 << C0_SHIFT) + ((1<<C0_SHIFT)>>1)) * count;
          c1total += ((c1 << C1_SHIFT) + ((1<<C1_SHIFT)>>1)) * count;
          c2total += ((c2 << C2_SHIFT) + ((1<<C2_SHIFT)>>1)) * count;
        }
      }
    }
    
    sl_colormap[0][icolor] = (JSAMPLE) ((c0total + (total>>1)) / total);
    sl_colormap[1][icolor] = (JSAMPLE) ((c1total + (total>>1)) / total);
    sl_colormap[2][icolor] = (JSAMPLE) ((c2total + (total>>1)) / total);
}


static void slow_select_colors (int *descolors)
/* Master routine for color selection */
{
  box boxlist[MAXNUMCOLORS];
  int numboxes;
  int i;
  
  /* Initialize one box containing whole space */
  numboxes = 1;
  boxlist[0].c0min = 0;
  boxlist[0].c0max = 255 >> C0_SHIFT;
  boxlist[0].c1min = 0;
  boxlist[0].c1max = 255 >> C1_SHIFT;
  boxlist[0].c2min = 0;
  boxlist[0].c2max = 255 >> C2_SHIFT;
  /* Shrink it to actually-used volume and set its statistics */
  update_box(& boxlist[0]);
  /* Perform median-cut to produce final box list */
  numboxes = median_cut(boxlist, numboxes, *descolors);
  /* Compute the representative color for each box, fill colormap */
  for (i = 0; i < numboxes; i++)
    compute_color(& boxlist[i], i);
  sl_num_colors = numboxes;

  *descolors = sl_num_colors;
}


/* log2(histogram cells in update box) for each axis; this can be adjusted */
#define BOX_C0_LOG  (HIST_C0_BITS-3)
#define BOX_C1_LOG  (HIST_C1_BITS-3)
#define BOX_C2_LOG  (HIST_C2_BITS-3)

#define BOX_C0_ELEMS  (1<<BOX_C0_LOG) /* # of hist cells in update box */
#define BOX_C1_ELEMS  (1<<BOX_C1_LOG)
#define BOX_C2_ELEMS  (1<<BOX_C2_LOG)

#define BOX_C0_SHIFT  (C0_SHIFT + BOX_C0_LOG)
#define BOX_C1_SHIFT  (C1_SHIFT + BOX_C1_LOG)
#define BOX_C2_SHIFT  (C2_SHIFT + BOX_C2_LOG)


static int find_nearby_colors (int minc0, int minc1, int minc2, JSAMPLE colorlist[])
{
  int numcolors = sl_num_colors;
  int maxc0, maxc1, maxc2;
  int centerc0, centerc1, centerc2;
  int i, x, ncolors;
  INT32 minmaxdist, min_dist, max_dist, tdist;
  INT32 mindist[MAXNUMCOLORS];	/* min distance to colormap entry i */
  
  maxc0 = minc0 + ((1 << BOX_C0_SHIFT) - (1 << C0_SHIFT));
  centerc0 = (minc0 + maxc0) >> 1;
  maxc1 = minc1 + ((1 << BOX_C1_SHIFT) - (1 << C1_SHIFT));
  centerc1 = (minc1 + maxc1) >> 1;
  maxc2 = minc2 + ((1 << BOX_C2_SHIFT) - (1 << C2_SHIFT));
  centerc2 = (minc2 + maxc2) >> 1;
  
  minmaxdist = 0x7FFFFFFFL;
  
  for (i = 0; i < numcolors; i++) {
    /* We compute the squared-c0-distance term, then add in the other two. */
    x = sl_colormap[0][i];
    if (x < minc0) {
      tdist = (x - minc0) * C0_SCALE;
      min_dist = tdist*tdist;
      tdist = (x - maxc0) * C0_SCALE;
      max_dist = tdist*tdist;
    } else if (x > maxc0) {
      tdist = (x - maxc0) * C0_SCALE;
      min_dist = tdist*tdist;
      tdist = (x - minc0) * C0_SCALE;
      max_dist = tdist*tdist;
    } else {
      /* within cell range so no contribution to min_dist */
      min_dist = 0;
      if (x <= centerc0) {
        tdist = (x - maxc0) * C0_SCALE;
        max_dist = tdist*tdist;
      } else {
        tdist = (x - minc0) * C0_SCALE;
        max_dist = tdist*tdist;
      }
    }
    
    x = sl_colormap[1][i];
    if (x < minc1) {
      tdist = (x - minc1) * C1_SCALE;
      min_dist += tdist*tdist;
      tdist = (x - maxc1) * C1_SCALE;
      max_dist += tdist*tdist;
    } else if (x > maxc1) {
      tdist = (x - maxc1) * C1_SCALE;
      min_dist += tdist*tdist;
      tdist = (x - minc1) * C1_SCALE;
      max_dist += tdist*tdist;
    } else {
      /* within cell range so no contribution to min_dist */
      if (x <= centerc1) {
        tdist = (x - maxc1) * C1_SCALE;
        max_dist += tdist*tdist;
      } else {
        tdist = (x - minc1) * C1_SCALE;
        max_dist += tdist*tdist;
      }
    }
    
    x = sl_colormap[2][i];
    if (x < minc2) {
      tdist = (x - minc2) * C2_SCALE;
      min_dist += tdist*tdist;
      tdist = (x - maxc2) * C2_SCALE;
      max_dist += tdist*tdist;
    } else if (x > maxc2) {
      tdist = (x - maxc2) * C2_SCALE;
      min_dist += tdist*tdist;
      tdist = (x - minc2) * C2_SCALE;
      max_dist += tdist*tdist;
    } else {
      /* within cell range so no contribution to min_dist */
      if (x <= centerc2) {
        tdist = (x - maxc2) * C2_SCALE;
        max_dist += tdist*tdist;
      } else {
        tdist = (x - minc2) * C2_SCALE;
        max_dist += tdist*tdist;
      }
    }
    
    mindist[i] = min_dist;	/* save away the results */
    if (max_dist < minmaxdist)
      minmaxdist = max_dist;
  }
  
  ncolors = 0;
  for (i = 0; i < numcolors; i++) {
    if (mindist[i] <= minmaxdist)
      colorlist[ncolors++] = (JSAMPLE) i;
  }
  return ncolors;
}


static void find_best_colors (int minc0, int minc1, int minc2, int numcolors,
                              JSAMPLE colorlist[], JSAMPLE bestcolor[])
{
  int ic0, ic1, ic2;
  int i, icolor;
  register INT32 * bptr;	/* pointer into bestdist[] array */
  JSAMPLE * cptr;		/* pointer into bestcolor[] array */
  INT32 dist0, dist1;		/* initial distance values */
  register INT32 dist2;		/* current distance in inner loop */
  INT32 xx0, xx1;		/* distance increments */
  register INT32 xx2;
  INT32 inc0, inc1, inc2;	/* initial values for increments */
  /* This array holds the distance to the nearest-so-far color for each cell */
  INT32 bestdist[BOX_C0_ELEMS * BOX_C1_ELEMS * BOX_C2_ELEMS];
  
  /* Initialize best-distance for each cell of the update box */
  bptr = bestdist;
  for (i = BOX_C0_ELEMS*BOX_C1_ELEMS*BOX_C2_ELEMS-1; i >= 0; i--)
    *bptr++ = 0x7FFFFFFFL;
  
  /* Nominal steps between cell centers ("x" in Thomas article) */
#define STEP_C0  ((1 << C0_SHIFT) * C0_SCALE)
#define STEP_C1  ((1 << C1_SHIFT) * C1_SCALE)
#define STEP_C2  ((1 << C2_SHIFT) * C2_SCALE)
  
  for (i = 0; i < numcolors; i++) {
    icolor = colorlist[i];
    /* Compute (square of) distance from minc0/c1/c2 to this color */
    inc0 = (minc0 - (int) sl_colormap[0][icolor]) * C0_SCALE;
    dist0 = inc0*inc0;
    inc1 = (minc1 - (int) sl_colormap[1][icolor]) * C1_SCALE;
    dist0 += inc1*inc1;
    inc2 = (minc2 - (int) sl_colormap[2][icolor]) * C2_SCALE;
    dist0 += inc2*inc2;
    /* Form the initial difference increments */
    inc0 = inc0 * (2 * STEP_C0) + STEP_C0 * STEP_C0;
    inc1 = inc1 * (2 * STEP_C1) + STEP_C1 * STEP_C1;
    inc2 = inc2 * (2 * STEP_C2) + STEP_C2 * STEP_C2;
    /* Now loop over all cells in box, updating distance per Thomas method */
    bptr = bestdist;
    cptr = bestcolor;
    xx0 = inc0;
    for (ic0 = BOX_C0_ELEMS-1; ic0 >= 0; ic0--) {
      dist1 = dist0;
      xx1 = inc1;
      for (ic1 = BOX_C1_ELEMS-1; ic1 >= 0; ic1--) {
        dist2 = dist1;
        xx2 = inc2;
        for (ic2 = BOX_C2_ELEMS-1; ic2 >= 0; ic2--) {
          if (dist2 < *bptr) {
            *bptr = dist2;
            *cptr = (JSAMPLE) icolor;
          }
          dist2 += xx2;
          xx2 += 2 * STEP_C2 * STEP_C2;
          bptr++;
          cptr++;
        }
        dist1 += xx1;
        xx1 += 2 * STEP_C1 * STEP_C1;
      }
      dist0 += xx0;
      xx0 += 2 * STEP_C0 * STEP_C0;
    }
  }
}


static void fill_inverse_cmap (int c0, int c1, int c2)
{
  hist2d * histogram = sl_histogram;
  int minc0, minc1, minc2;	/* lower left corner of update box */
  int ic0, ic1, ic2;
  register JSAMPLE * cptr;	/* pointer into bestcolor[] array */
  register histptr cachep;	/* pointer into main cache array */
  /* This array lists the candidate colormap indexes. */
  JSAMPLE colorlist[MAXNUMCOLORS];
  int numcolors;		/* number of candidate colors */
  /* This array holds the actually closest colormap index for each cell. */
  JSAMPLE bestcolor[BOX_C0_ELEMS * BOX_C1_ELEMS * BOX_C2_ELEMS];
  
  /* Convert cell coordinates to update box ID */
  c0 >>= BOX_C0_LOG;
  c1 >>= BOX_C1_LOG;
  c2 >>= BOX_C2_LOG;
  
  minc0 = (c0 << BOX_C0_SHIFT) + ((1 << C0_SHIFT) >> 1);
  minc1 = (c1 << BOX_C1_SHIFT) + ((1 << C1_SHIFT) >> 1);
  minc2 = (c2 << BOX_C2_SHIFT) + ((1 << C2_SHIFT) >> 1);
  
  numcolors = find_nearby_colors(minc0, minc1, minc2, colorlist);
  
  /* Determine the actually nearest colors. */
  find_best_colors(minc0, minc1, minc2, numcolors, colorlist, bestcolor);
  
  /* Save the best color numbers (plus 1) in the main cache array */
  c0 <<= BOX_C0_LOG;		/* convert ID back to base cell indexes */
  c1 <<= BOX_C1_LOG;
  c2 <<= BOX_C2_LOG;
  cptr = bestcolor;
  for (ic0 = 0; ic0 < BOX_C0_ELEMS; ic0++) {
    for (ic1 = 0; ic1 < BOX_C1_ELEMS; ic1++) {
      cachep = & histogram[c0+ic0][c1+ic1][c2];
      for (ic2 = 0; ic2 < BOX_C2_ELEMS; ic2++) {
        *cachep++ = (histcell) (*cptr++ + 1);
      }
    }
  }
}


static void slow_map_pixels(const byte *red, const byte *green, const byte *blue, int width, int height, byte *map)
{
  register LOCFSERROR cur0, cur1, cur2;	/* current error or pixel value */
  LOCFSERROR belowerr0, belowerr1, belowerr2; /* error for pixel below cur */
  LOCFSERROR bpreverr0, bpreverr1, bpreverr2; /* error for below/prev col */
  register FSERRPTR errorptr;	/* => fserrors[] at column before current */
  JSAMPROW inRptr, inGptr, inBptr;		/* => current input pixel */
  JSAMPROW outptr;		/* => current output pixel */
  histptr cachep;
  int dir;			/* +1 or -1 depending on direction */
  int dir3;			/* 3*dir, for advancing errorptr */
  int row, col, offset;
  int *error_limit = sl_error_limiter;
  JSAMPROW colormap0 = sl_colormap[0];
  JSAMPROW colormap1 = sl_colormap[1];
  JSAMPROW colormap2 = sl_colormap[2];
  hist2d * histogram = sl_histogram;
  
  for (row = 0; row < height; row++) 
  {
    offset = row * width;

    inRptr = (JSAMPROW)&red[offset];
    inGptr = (JSAMPROW)&green[offset];
    inBptr = (JSAMPROW)&blue[offset];
    outptr = &map[offset];

    if (sl_on_odd_row) 
    {
      /* work right to left in this row */
      offset = width-1;

      inRptr += offset;	/* so point to rightmost pixel */
      inGptr += offset;	/* so point to rightmost pixel */
      inBptr += offset;	/* so point to rightmost pixel */

      outptr += offset;

      dir = -1;
      dir3 = -3;
      errorptr = sl_fserrors + (width+1)*3; /* => entry after last column */
      sl_on_odd_row = FALSE;	/* flip for next time */
    } 
    else 
    {
      /* work left to right in this row */
      dir = 1;
      dir3 = 3;
      errorptr = sl_fserrors;	/* => entry before first real column */
      sl_on_odd_row = TRUE;	/* flip for next time */
    }

    /* Preset error values: no error propagated to first pixel from left */
    cur0 = cur1 = cur2 = 0;
    /* and no error propagated to row below yet */
    belowerr0 = belowerr1 = belowerr2 = 0;
    bpreverr0 = bpreverr1 = bpreverr2 = 0;
    
    for (col = width; col > 0; col--) 
    {
      cur0 = (cur0 + errorptr[dir3+0] + 8) >> 4;
      cur1 = (cur1 + errorptr[dir3+1] + 8) >> 4;
      cur2 = (cur2 + errorptr[dir3+2] + 8) >> 4;

      cur0 = error_limit[cur0];
      cur1 = error_limit[cur1];
      cur2 = error_limit[cur2];

      cur0 += inRptr[0];
      cur1 += inGptr[0];
      cur2 += inBptr[0];

      RANGE(cur0, 0, 255);
      RANGE(cur1, 0, 255);
      RANGE(cur2, 0, 255);

      /* Index into the cache with adjusted pixel value */
      cachep = & histogram[cur0>>C0_SHIFT][cur1>>C1_SHIFT][cur2>>C2_SHIFT];

      /* If we have not seen this color before, find nearest colormap */
      /* entry and update the cache */
      if (*cachep == 0)
        fill_inverse_cmap(cur0>>C0_SHIFT, cur1>>C1_SHIFT, cur2>>C2_SHIFT);

      /* Now emit the colormap index for this cell */
      {
        register int pixcode = *cachep - 1;
        *outptr = (JSAMPLE) pixcode;
        /* Compute representation error for this pixel */
        cur0 -= (int) colormap0[pixcode];
        cur1 -= (int) colormap1[pixcode];
        cur2 -= (int) colormap2[pixcode];
      }

      /* Compute error fractions to be propagated to adjacent pixels.
      * Add these into the running sums, and simultaneously shift the
      * next-line error sums left by 1 column. */
      { 
        register LOCFSERROR bnexterr, delta;
        bnexterr = cur0;	/* Process component 0 */
        delta = cur0 * 2;
        cur0 += delta;		/* form error * 3 */
        errorptr[0] = (FSERROR) (bpreverr0 + cur0);
        cur0 += delta;		/* form error * 5 */
        bpreverr0 = belowerr0 + cur0;
        belowerr0 = bnexterr;
        cur0 += delta;		/* form error * 7 */
        bnexterr = cur1;	/* Process component 1 */
        delta = cur1 * 2;
        cur1 += delta;		/* form error * 3 */
        errorptr[1] = (FSERROR) (bpreverr1 + cur1);
        cur1 += delta;		/* form error * 5 */
        bpreverr1 = belowerr1 + cur1;
        belowerr1 = bnexterr;
        cur1 += delta;		/* form error * 7 */
        bnexterr = cur2;	/* Process component 2 */
        delta = cur2 * 2;
        cur2 += delta;		/* form error * 3 */
        errorptr[2] = (FSERROR) (bpreverr2 + cur2);
        cur2 += delta;		/* form error * 5 */
        bpreverr2 = belowerr2 + cur2;
        belowerr2 = bnexterr;
        cur2 += delta;		/* form error * 7 */
      }

      /* At this point curN contains the 7/16 error value to be propagated
      * to the next pixel on the current line, and all the errors for the
      * next line have been shifted over.  We are therefore ready to move on.
      */
      inRptr += dir;		/* Advance pixel pointers to next column */
      inGptr += dir;		/* Advance pixel pointers to next column */
      inBptr += dir;		/* Advance pixel pointers to next column */
      outptr += dir;
      errorptr += dir3;		/* advance errorptr to current column */
    }

    /* Post-loop cleanup: we must unload the final error values into the
    * final fserrors[] entry.  Note we need not unload belowerrN because
    * it is for the dummy column before or after the actual array.
    */
    errorptr[0] = (FSERROR) bpreverr0; /* unload prev errs into array */
    errorptr[1] = (FSERROR) bpreverr1;
    errorptr[2] = (FSERROR) bpreverr2;
  }
}


/* Allocate and fill in the error_limiter table */
static void init_error_limit (void)
{
  int * table;
  int in, out, STEPSIZE;
  
  table = (int *) malloc((size_t) ((255*2+1) * sizeof(int)));
  if (! table) return;
  
  table += 255;		/* so can index -255 .. +255 */
  sl_error_limiter = table;
  
  STEPSIZE = ((255+1)/16);

  /* Map errors 1:1 up to +- 255/16 */
  out = 0;
  for (in = 0; in < STEPSIZE; in++, out++) 
  {
    table[in]  =  out; 
    table[-in] = -out;
  }

  /* Map errors 1:2 up to +- 3*255/16 */
  for (; in < STEPSIZE*3; in++, out += (in&1) ? 0 : 1) 
  {
    table[in]  =  out;
    table[-in] = -out;
  }

  /* Clamp the rest to final out value (which is (255+1)/8) */
  for (; in <= 255; in++) 
  {
    table[in]  =  out; 
    table[-in] = -out;
  }
}

void cdRGB2Map(int width, int height, const unsigned char *red, const unsigned char *green, const unsigned char *blue, unsigned char *map, int pal_size, long *colors)
{
  int i, err;
  byte rm[256], gm[256], bm[256];
  int num_colors;

  if (pal_size <= 0 || pal_size > 256)
    pal_size = 256;

  num_colors = pal_size;
  
  if (!quick_map(red, green, blue, width, height, map, rm, gm, bm, &num_colors))  
  {
    err = slow_quant(red, green, blue, width, height, map, rm, gm, bm, &num_colors);
    if (err)
      return;
  }
  
  for (i=0; i < num_colors; i++)
    *colors++ = cdEncodeColor(rm[i], gm[i], bm[i]);

  if (num_colors < pal_size)
  {
    for (i=num_colors; i < pal_size; i++)
      *colors++ = 0;
  }
}
