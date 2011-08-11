/** \file
 * \brief Additional Image Quantization Operations
 *
 * See Copyright Notice in im_lib.h
 * $Id: im_quantize.cpp,v 1.2 2009/09/25 18:40:32 scuri Exp $
 */


#include <im.h>
#include <im_util.h>
#include <im_palette.h>
#include <im_math.h>

#include "im_process_pon.h"

#include <stdlib.h>
#include <memory.h>


void imProcessQuantizeRGBUniform(const imImage* src_image, imImage* dst_image, int dither)
{
  imbyte *dst_map=(imbyte*)dst_image->data[0], 
         *red_map=(imbyte*)src_image->data[0],
         *green_map=(imbyte*)src_image->data[1],
         *blue_map=(imbyte*)src_image->data[2];

  imImageSetPalette(dst_image, imPaletteUniform(), 256);

  for (int y = 0; y < src_image->height; y++)
  {
    for (int x = 0; x < src_image->width; x++)
    {
      if (dither)
        *dst_map++ = (imbyte)imPaletteUniformIndexHalftoned(imColorEncode(*red_map++, *green_map++, *blue_map++), x, y);
      else
        *dst_map++ = (imbyte)imPaletteUniformIndex(imColorEncode(*red_map++, *green_map++, *blue_map++));
    }
  }
}

void imProcessQuantizeGrayUniform(const imImage* src_image, imImage* dst_image, int grays)
{
  int i, value;

  imbyte *dst_map=(imbyte*)dst_image->data[0], 
         *src_map=(imbyte*)src_image->data[0];

  imbyte re_map[256];
  memset(re_map, 0, 256);

  float factor = (float)grays/256.0f;
  float factor256 = 256.0f/(float)grays;

  for (i = 0; i < 256; i++)
  {             
    value = imResample(i, factor);
    value = imResample(value, factor256);
    re_map[i] = (imbyte)IM_BYTECROP(value);
  }

  int total_count = src_image->count*src_image->depth;
  for (i = 0; i < total_count; i++)
    dst_map[i] = re_map[src_map[i]];
}
