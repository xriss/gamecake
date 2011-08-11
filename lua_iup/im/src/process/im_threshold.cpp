/** \file
 * \brief Threshold Operations
 *
 * See Copyright Notice in im_lib.h
 * $Id: im_threshold.cpp,v 1.1 2008/10/17 06:16:33 scuri Exp $
 */


#include <im.h>
#include <im_util.h>

#include "im_process_pon.h"
#include "im_process_ana.h"

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <string.h>
#include <math.h>


void imProcessSliceThreshold(const imImage* src_image, imImage* dst_image, int start_level, int end_level)
{
  float params[3];
  params[0] = (float)start_level;
  params[1] = (float)end_level;
  params[2] = (float)1; /* binarize 0-255 */
  imProcessToneGamut(src_image, dst_image, IM_GAMUT_SLICE, params);
  imImageMakeBinary(dst_image); /* this compensates the returned values in IM_GAMUT_SLICE */
}

void imProcessThresholdByDiff(const imImage* image1, const imImage* image2, imImage* NewImage)
{
  imbyte *src_map1 = (imbyte*)image1->data[0];
  imbyte *src_map2 = (imbyte*)image2->data[0];
  imbyte *dst_map = (imbyte*)NewImage->data[0];
  int size = image1->count;

  for (int i = 0; i < size; i++)
  {
    if (*src_map1++ <= *src_map2++)
      *dst_map++ = 0;
    else
      *dst_map++ = 1;
  }
}

template <class T> 
static void doThreshold(T *src_map, imbyte *dst_map, int count, int level, int value)
{
  for (int i = 0; i < count; i++)
  {
    if (*src_map++ <= level)
      *dst_map++ = 0;
    else
      *dst_map++ = (imbyte)value;
  }
}

void imProcessThreshold(const imImage* src_image, imImage* dst_image, int level, int value)
{
  switch(src_image->data_type)
  {
  case IM_BYTE:
    doThreshold((imbyte*)src_image->data[0], (imbyte*)dst_image->data[0], 
                             src_image->count, level, value);
    break;                                                                                
  case IM_USHORT:                                                                           
    doThreshold((imushort*)src_image->data[0], (imbyte*)dst_image->data[0], 
                             src_image->count, level, value);
    break;                                                                                
  case IM_INT:                                                                           
    doThreshold((int*)src_image->data[0], (imbyte*)dst_image->data[0], 
                             src_image->count, level, value);
    break;                                                                                
  }
}

static int compare_int(const void *elem1, const void *elem2) 
{
  int* v1 = (int*)elem1;
  int* v2 = (int*)elem2;

  if (*v1 < *v2)
    return -1;

  if (*v1 > *v2)
    return 1;

  return 0;
}

static int thresUniErr(unsigned char* band, int width, int height)
{
  int x, y, i, bottom, top, ant2x2, maks1, maks2, maks4, t;
  int xsize, ysize, offset1, offset2;
  double a, b, c, phi;
  int g[4], tab1[256], tab2[256], tab4[256];

  memset(tab1, 0, sizeof(int)*256);
  memset(tab2, 0, sizeof(int)*256);
  memset(tab4, 0, sizeof(int)*256);

  xsize = width;
  ysize = height;

  if (xsize%2 != 0)
    xsize--;

  if (ysize%2 != 0)
    ysize--;
  
  /* examine all 2x2 neighborhoods */

  for (y=0; y<ysize; y+=2)
  {
    offset1 = y*width;
    offset2 = (y+1)*width;

    for (x=0; x<xsize; x+=2) 
    {
      g[0] = band[offset1 + x];
      g[1] = band[offset1 + x+1];
      g[2] = band[offset2 + x];
      g[3] = band[offset2 + x+1];

      /* Sorting */
      qsort(g, 4, sizeof(int), compare_int);

      /* Accumulating */
      tab1[g[0]] += 1; 
      tab1[g[1]] += 1; 
      tab1[g[2]] += 1; 
      tab1[g[3]] += 1; 

      tab2[g[0]] +=3;
      tab2[g[1]] +=2;
      tab2[g[2]] +=1;

      tab4[g[0]] +=1;
    }
  }

  /* Summing */
  for (i=254; i>=0; i--) 
  {
    tab1[i] += tab1[i+1];
    tab2[i] += tab2[i+1];
    tab4[i] += tab4[i+1];
  }
  
  /* Tables are ready, find threshold */
  bottom = 0; top = 255;
  ant2x2 = (xsize/2)*(ysize/2);
  maks1 = tab1[0]; /* = ant2x2 * 4; */
  maks2 = tab2[0]; /* = ant2x2 * 6; */
  maks4 = tab4[0]; /* = ant2x2;     */

  /* binary search */
  t = 0;
  while (bottom != top-1) 
  {
    t = (int) ((bottom+top)/2);

    /* Calculate probabilities */
    a = (double) tab1[t+1]/maks1;
    b = (double) tab2[t+1]/maks2;
    c = (double) tab4[t+1]/maks4;

    phi = sqrt((b*b - c) / (a*a - b));

    if (phi> 1)  
      bottom = t;
    else                        
      top = t;
  }
  
  return t;
}

int imProcessUniformErrThreshold(const imImage* image, imImage* NewImage)
{
  int level = thresUniErr((imbyte*)image->data[0], image->width, image->height);
  imProcessThreshold(image, NewImage, level, 1);
  return level;
}

static void do_dither_error(imbyte* data1, imbyte* data2, int size, int t, int value)
{
  int i, error;
  float scale = (float)(t/(255.0-t));

  error = 0; /* always in [-127,127] */ 

  for (i = 0; i < size; i++)
  {
    if ((int)(*data1 + error) > t)
    {
      error -= (int)(((int)255 - (int)*data1++)*scale);
      *data2++ = (imbyte)value;
    }
    else
    {
      error += (int)*data1++;
      *data2++ = (imbyte)0;
    }
  }
}

void imProcessDifusionErrThreshold(const imImage* image, imImage* NewImage, int level)
{
  int value = image->depth > 1? 255: 1;
  int size = image->width * image->height;
  for (int i = 0; i < image->depth; i++)
  {
    do_dither_error((imbyte*)image->data[i], (imbyte*)NewImage->data[i], size, level, value);
  }
}

int imProcessPercentThreshold(const imImage* image, imImage* NewImage, float percent)
{
  unsigned long histo[256], cut;

  cut = (int)((image->width * image->height * percent)/100.);

  imCalcHistogram((imbyte*)image->data[0], image->width * image->height, histo, 1);

  int i;
  for (i = 0; i < 256; i++)
  {
    if (histo[i] > cut)
      break;
  }

  int level = (i==0? 0: i==256? 254: i-1);

  imProcessThreshold(image, NewImage, level, 1);
  return level;
}

static int MaximizeDiscriminantFunction(double * p)
{
  double mi_255 = 0;
  int k;
  for (k=0; k<256; k++) 
    mi_255 += k*p[k];

  int index = 0;
  double max = 0;
  double mi_k = 0;
  double w_k = 0;
  double value;
  for (k=0; k<256; k++) 
  {
    mi_k += k*p[k];
    w_k += p[k];
    value = ((w_k == 0) || (w_k == 1))? -1 : ((mi_255*w_k - mi_k)*(mi_255*w_k - mi_k))/(w_k*(1-w_k));
    if (value >= max) 
    {
      index = k;
      max = value;
    }
  }

  return index;
}

static unsigned char Otsu(const imImage *image)
{
  unsigned long histo[256];
  imCalcHistogram((imbyte*)image->data[0], image->count, histo, 0);

  double totalPixels = image->count;
  double p[256];
  for (int i=0; i<256; i++) 
    p[i] = histo[i]/totalPixels;

  return (unsigned char)MaximizeDiscriminantFunction(p);
}

int imProcessOtsuThreshold(const imImage* image, imImage* NewImage)
{
  int level = Otsu(image);
  imProcessThreshold(image, NewImage, level, 1);
  return level;
}

int imProcessMinMaxThreshold(const imImage* image, imImage* NewImage)
{
  imStats stats;
  imCalcImageStatistics(image, &stats);
  int level = (int)((stats.max - stats.min)/2.0f);
  imProcessThreshold(image, NewImage, level, 1);
  return level;
}

void imProcessHysteresisThresEstimate(const imImage* image, int *low_thres, int *high_thres)
{
  unsigned long hist[256];
  imCalcHistogram((imbyte*)image->data[0], image->count, hist, 0);

  /* The high threshold should be > 80 or 90% of the pixels */
  unsigned long cut = (int)(0.1*image->count);

  int k = 255;
  unsigned long count = hist[255];
  while (count < cut)
  {
    k--;
    count += hist[k];
  }
  *high_thres = k;

  k=0;
  while (hist[k]==0) k++;

  *low_thres = (int)((*high_thres + k)/2.0) + k;
}

void imProcessHysteresisThreshold(const imImage* image, imImage* NewImage, int low_thres, int high_thres)
{
  imbyte *src_map = (imbyte*)image->data[0];
  imbyte *dst_map = (imbyte*)NewImage->data[0];
  int i, j, size = image->count;

  for (i = 0; i < size; i++)
  {
    if (*src_map > high_thres)
      *dst_map++ = 1;
    else if (*src_map > low_thres)
      *dst_map++ = 2;          // mark for future replace
    else
      *dst_map++ = 0;

    src_map++;
  }

  // now loop multiple times until there is no "2"s or no one was changed
  dst_map = (imbyte*)NewImage->data[0];
  int changed = 1;
  while (changed) 
  {
    changed = 0;
    for (j=1; j<image->height-1; j++) 
    {
      for (i=1; i<image->width-1; i++)
      {
        int offset = i+j*image->width;
        if (dst_map[offset] == 2)
        {
          // if there is an edge neighbor mark this as edge too
          if (dst_map[offset+1] == 1 || dst_map[offset-1] == 1 ||
              dst_map[offset+image->width] == 1 || dst_map[offset-image->width] == 1 ||
              dst_map[offset+image->width-1] == 1 || dst_map[offset+image->width+1] == 1 ||
              dst_map[offset-image->width-1] == 1 || dst_map[offset-image->width+1] == 1)
          {
            dst_map[offset] = 1;
            changed = 1;
          }
        }
      }
    }
  }

  // Clear the remaining "2"s
  dst_map = (imbyte*)NewImage->data[0];
  for (i = 0; i < size; i++)
  {
    if (*dst_map == 2)
      *dst_map = 0;
    dst_map++;
  }
}

void imProcessLocalMaxThresEstimate(const imImage* image, int *thres)
{
  unsigned long hist[256];
  imCalcHistogram((imbyte*)image->data[0], image->count, hist, 0);

  int high_count = 0;
  int index = 255;
  while (high_count < 10 && index > 0)
  {
    if (hist[index] != 0)
      high_count++;

    index--;
  }
  *thres = index+1;
}

