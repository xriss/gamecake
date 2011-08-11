/** \file
 * \brief Histogram Based Operations
 *
 * See Copyright Notice in im_lib.h
 * $Id: im_histogram.cpp,v 1.1 2008/10/17 06:16:33 scuri Exp $
 */


#include <im.h>
#include <im_util.h>
#include <im_math.h>

#include "im_process_pon.h"
#include "im_process_ana.h"

#include <stdlib.h>
#include <memory.h>

static void iExpandHistogram(const imImage* src_image, imImage* dst_image, int low_level, int high_level)
{
  int i, value;

  imbyte re_map[256];
  memset(re_map, 0, 256);

  int range = high_level-low_level+1;
  float factor = 256.0f / (float)range;

  for (i = 0; i < 256; i++)
  {             
    if (i < low_level)
      re_map[i] = 0;
    else if (i > high_level)
      re_map[i] = 255;
    else
    {
      value = imResample(i - low_level, factor);
      re_map[i] = (imbyte)IM_BYTECROP(value);
    }
  }

  imbyte* dst_map = (imbyte*)dst_image->data[0];
  imbyte* src_map = (imbyte*)src_image->data[0];
  int total_count = src_image->count*src_image->depth;
  for (i = 0; i < total_count; i++)
    dst_map[i] = re_map[src_map[i]];
}

void imProcessExpandHistogram(const imImage* src_image, imImage* dst_image, float percent)
{
  unsigned long histo[256];
  imCalcGrayHistogram(src_image, histo, 0);

  unsigned long acum, cut = (unsigned long)((src_image->count * percent) / 100.0f);
  int low_level, high_level;

  acum = 0;
  for (low_level = 0; low_level < 256; low_level++)
  {  
    acum += histo[low_level];
    if (acum > cut)
      break;
  }

  acum = 0;
  for (high_level = 255; high_level > 0; high_level--)
  {  
    acum += histo[high_level];
    if (acum > cut)
      break;
  }

  if (low_level >= high_level)
  {
    low_level = 0;
    high_level = 255;
  }

  iExpandHistogram(src_image, dst_image, low_level, high_level);
}

void imProcessEqualizeHistogram(const imImage* src_image, imImage* dst_image)
{
  int i, value;

  imbyte re_map[256];
  memset(re_map, 0, 256);

  unsigned long histo[256];
  imCalcGrayHistogram(src_image, histo, 1);

  float factor = 256.0f / (float)src_image->count;

  for (i = 0; i < 256; i++)
  {             
    value = imResample(histo[i], factor);
    re_map[i] = (imbyte)IM_BYTECROP(value);
  }

  imbyte* dst_map = (imbyte*)dst_image->data[0];
  imbyte* src_map = (imbyte*)src_image->data[0];
  int total_count = src_image->count*src_image->depth;
  for (i = 0; i < total_count; i++)
    dst_map[i] = re_map[src_map[i]];
}
