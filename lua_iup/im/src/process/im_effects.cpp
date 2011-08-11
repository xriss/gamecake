/** \file
 * \brief Effects
 *
 * See Copyright Notice in im_lib.h
 * $Id: im_effects.cpp,v 1.1 2008/10/17 06:16:33 scuri Exp $
 */


#include <im.h>
#include <im_util.h>
#include <im_math.h>
#include <im_complex.h>

#include "im_process_pon.h"
#include "im_math_op.h"

#include <stdlib.h>
#include <memory.h>

static unsigned char BoxMean(imbyte *map, int offset, int shift, int hbox_size, int vbox_size)
{
  map += offset;
  int acum = 0;
  for (int i = 0; i < vbox_size; i++)
  {
    for (int j = 0; j < hbox_size; j++)
    {
      acum += *map++;
    }

    map += shift;
  }

  return (unsigned char)(acum / (vbox_size*hbox_size));
}

static void BoxSet(imbyte *map, int offset, int shift, int hbox_size, int vbox_size, unsigned char value)
{
  map += offset;
  for (int i = 0; i < vbox_size; i++)
  {
    for (int j = 0; j < hbox_size; j++)
    {
      *map++ = value;
    }

    map += shift;
  }
}

void imProcessPixelate(const imImage* src_image, imImage* dst_image, int box_size)
{
  int hbox = ((src_image->width + box_size-1)/ box_size);
  int vbox = ((src_image->height + box_size-1)/ box_size);

  for (int i = 0; i < src_image->depth; i++)
  {
    imbyte *src_map=(imbyte*)src_image->data[i];
    imbyte *dst_map=(imbyte*)dst_image->data[i];
    int vbox_size = box_size;

    for (int bv = 0; bv < vbox; bv++)
    {
      int bv_pos = bv*box_size;
      if (bv == vbox-1) vbox_size = src_image->height - bv_pos;
      int hbox_size = box_size;

      for (int bh = 0; bh < hbox; bh++)
      {
        int bh_pos = bh*box_size;
        if (bh == hbox-1) hbox_size = src_image->width - bh_pos;
        int offset = bv_pos*src_image->width + bh_pos;
        int shift = src_image->width - hbox_size;
        unsigned char mean = BoxMean(src_map, offset, shift, hbox_size, vbox_size);
        BoxSet(dst_map, offset, shift, hbox_size, vbox_size, mean);
      }
    }
  }
}

void imProcessPosterize(const imImage* src_image, imImage* dst_image, int level)
{
  unsigned char mask = (unsigned char)(0xFF << level);
  imProcessBitMask(src_image, dst_image, mask, IM_BIT_AND);
}

