/** \file
 * \brief Image Statistics Calculations
 *
 * See Copyright Notice in im_lib.h
 * $Id: im_statistics.cpp,v 1.1 2008/10/17 06:16:33 scuri Exp $
 */


#include <im.h>
#include <im_util.h>
#include <im_math_op.h>

#include "im_process_ana.h"

#include <stdlib.h>
#include <memory.h>
#include <math.h>

static unsigned long count_map(const imImage* image)
{
  unsigned long histo[256];
  int size = image->width * image->height;
  imCalcHistogram((imbyte*)image->data[0], size, histo, 0);
  unsigned long numcolor = 0;

  for (int i = 0; i < 256; i++)
  {             
    if(histo[i] != 0)
      numcolor++;
  }

  return numcolor;
}

// will count also all the 3 components color spaces
static unsigned long count_rgb(const imImage* image)
{
  imbyte *count = (imbyte*)calloc(sizeof(imbyte), 1 << 21 ); /* (2^24)/8=2^21 ~ 2Mb */
  if (!count)
    return (unsigned long)-1;

  int size = image->width * image->height;
  imbyte *red = (imbyte*)image->data[0];
  imbyte *green = (imbyte*)image->data[1];
  imbyte *blue = (imbyte*)image->data[2];

  int index;
  unsigned long numcolor = 0;

  for(int i = 0; i < size; i++)
  {
    index = red[i] << 16 | green[i] << 8 | blue[i];

    if(imDataBitGet(count, index) == 0)
      numcolor++;

    imDataBitSet(count, index, 1);
  }

  free(count);

  return numcolor;
}

unsigned long imCalcCountColors(const imImage* image)
{
  if (imColorModeDepth(image->color_space) > 1)
    return count_rgb(image);
  else
    return count_map(image);
}

void imCalcHistogram(const imbyte* map, int size, unsigned long* histo, int cumulative)
{
  int i;

  memset(histo, 0, 256 * sizeof(unsigned long));

  for (i = 0; i < size; i++)
    histo[*map++]++;

  if (cumulative)
  {
    /* make cumulative histogram */
    for (i = 1; i < 256; i++)
      histo[i] += histo[i-1];
  }
}

void imCalcUShortHistogram(const imushort* map, int size, unsigned long* histo, int cumulative)
{
  int i;

  memset(histo, 0, 65535 * sizeof(unsigned long));

  for (i = 0; i < size; i++)
    histo[*map++]++;

  if (cumulative)
  {
    /* make cumulative histogram */
    for (i = 1; i < 65535; i++)
      histo[i] += histo[i-1];
  }
}

void imCalcGrayHistogram(const imImage* image, unsigned long* histo, int cumulative)
{
  int i;

  memset(histo, 0, 256 * sizeof(unsigned long));

  if (image->color_space == IM_GRAY)
  {
    imbyte* map = (imbyte*)image->data[0];
    for (i = 0; i < image->count; i++)
      histo[*map++]++;
  }
  else if (image->color_space == IM_MAP || image->color_space == IM_BINARY)
  {
    imbyte* map = (imbyte*)image->data[0];
    imbyte gray_map[256], r, g, b;

    for (i = 0; i < image->palette_count; i++)
    {
      imColorDecode(&r, &g, &b, image->palette[i]);
      gray_map[i] = (imbyte)((299*r + 587*g + 114*b) / 1000);
    }

    for (i = 0; i < image->count; i++)
    {
      int index = *map++;
      histo[gray_map[index]]++;
    }
  }
  else
  {
    imbyte gray;
    imbyte* r = (imbyte*)image->data[0];
    imbyte* g = (imbyte*)image->data[1];
    imbyte* b = (imbyte*)image->data[2];
    for (i = 0; i < image->count; i++)
    {
      gray = (imbyte)((299*(*r++) + 587*(*g++) + 114*(*b++)) / 1000);
      histo[gray]++;
    }
  }

  if (cumulative)
  {
    /* make cumulative histogram */
    for (i = 1; i < 256; i++)
      histo[i] += histo[i-1];
  }
}

template <class T>
static void DoStats(T* data, int count, imStats* stats)
{
  memset(stats, 0, sizeof(imStats));

  stats->min = (float)data[0];
  stats->max = (float)data[0];

  for (int i = 0; i < count; i++)
  {
		if (data[i] < stats->min)
		  stats->min = (float)data[i];

		if (data[i] > stats->max)
		  stats->max = (float)data[i];

    if (data[i] > 0)
      stats->positive++;

    if (data[i] < 0)
      stats->negative++;

    if (data[i] == 0)
      stats->zeros++;

    stats->mean += (float)data[i];
    stats->stddev += ((float)data[i])*((float)data[i]);
  }

  stats->mean /= float(count);
  stats->stddev = (float)sqrt((stats->stddev - count * stats->mean*stats->mean)/(count-1.0));
}

void imCalcImageStatistics(const imImage* image, imStats* stats)
{
  int count = image->width * image->height;

  for (int i = 0; i < image->depth; i++)
  {
    switch(image->data_type)
    {
    case IM_BYTE:
      DoStats((imbyte*)image->data[i], count, &stats[i]);
      break;                                                                                
    case IM_USHORT:                                                                           
      DoStats((imushort*)image->data[i], count, &stats[i]);
      break;                                                                                
    case IM_INT:                                                                           
      DoStats((int*)image->data[i], count, &stats[i]);
      break;                                                                                
    case IM_FLOAT:                                                                           
      DoStats((float*)image->data[i], count, &stats[i]);
      break;                                                                                
    }
  }
}

void imCalcHistogramStatistics(const imImage* image, imStats* stats)
{
  int image_size = image->width * image->height;
  unsigned long histo[256];

  for (int d = 0; d < image->depth; d++)
  {
    imCalcHistogram((imbyte*)image->data[d], image_size, histo, 0);
    DoStats((unsigned long*)histo, 256, &stats[d]);
  }
}

void imCalcHistoImageStatistics(const imImage* image, int* median, int* mode)
{
  unsigned long histo[256];

  for (int d = 0; d < image->depth; d++)
  {
    int i;
    imCalcHistogram((imbyte*)image->data[d], image->count, histo, 0);

    unsigned long half = image->count/2;
    unsigned long count = histo[0];
    for (i = 1; i < 256; i++)
    {
      if (count > half)
      {
        median[d] = i-1;
        break;
      }

      count += histo[i];
    }

    unsigned long max = histo[0];
    for (i = 1; i < 256; i++)
    {
      if (max < histo[i])
        max = histo[i];
    }

    int found_mode = 0;
    for (i = 0; i < 256; i++)
    {
      if (histo[i] == max)
      {
        if (found_mode)
        {
          mode[d] = -1;
          break;
        }

        mode[d] = i;
        found_mode = 1;
      }
    }
  }
}

float imCalcSNR(const imImage* image, const imImage* noise_image)
{
  imStats stats[3];
  imCalcImageStatistics((imImage*)image, stats);

  imStats noise_stats[3];
  imCalcImageStatistics((imImage*)noise_image, noise_stats);

  if (image->color_space == IM_RGB)
  {
    noise_stats[0].stddev += noise_stats[1].stddev;
    noise_stats[0].stddev += noise_stats[2].stddev;
    noise_stats[0].stddev /= 3;
    stats[0].stddev += stats[1].stddev;
    stats[0].stddev += stats[2].stddev;
    stats[0].stddev /= 3;
  }

  if (noise_stats[0].stddev == 0)
    return 0;

  return float(20.*log10(stats[0].stddev / noise_stats[0].stddev));
}

template <class T> 
static float DoRMSOp(T *map1, T *map2, int count)
{
  float rmserror = 0.0f;
  float diff;

  for (int i = 0; i < count; i++)
  {
    diff = float(map1[i] - map2[i]);
    rmserror += diff * diff;
  }

  return rmserror;
}
  
float imCalcRMSError(const imImage* image1, const imImage* image2)
{
  float rmserror = 0.0f;

  int count = image1->count*image1->depth;

  switch(image1->data_type)
  {
  case IM_BYTE:
    rmserror = DoRMSOp((imbyte*)image1->data[0], (imbyte*)image2->data[0], count);
    break;
  case IM_USHORT:
    rmserror = DoRMSOp((imushort*)image1->data[0], (imushort*)image2->data[0], count);
    break;
  case IM_INT:
    rmserror = DoRMSOp((int*)image1->data[0], (int*)image2->data[0], count);
    break;
  case IM_FLOAT:
    rmserror = DoRMSOp((float*)image1->data[0], (float*)image2->data[0], count);
    break;
  case IM_CFLOAT:
    rmserror = DoRMSOp((float*)image1->data[0], (float*)image2->data[0], 2*count);
    break;
  }

  rmserror = float(sqrt(rmserror / float((count * image1->depth))));

  return rmserror;
}

