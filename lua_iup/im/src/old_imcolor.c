/** \file
 * \brief Old resize/stretch functions
 *
 * See Copyright Notice in im_lib.h
 * $Id: old_imcolor.c,v 1.1 2008/10/17 06:10:16 scuri Exp $
 */

#include "old_im.h"
#include "im.h"
#include "im_util.h"
#include "im_image.h"
#include "im_convert.h"

void imRGB2Map(int width, int height, 
              unsigned char *red, unsigned char *green, unsigned char *blue, 
              unsigned char *map, int palette_count, long *palette)
{
  imConvertRGB2Map(width, height, 
                   red,  green, blue, 
                   map, palette, &palette_count);
}

void imMap2RGB(int width, int height, unsigned char *map, int palette_count, long *palette, unsigned char *red, unsigned char *green, unsigned char *blue)
{
  int i, count, c, index;
  unsigned char r[256], g[256], b[256];

  for (c = 0; c < palette_count; c++)
    imColorDecode(&r[c], &g[c], &b[c], palette[c]);

  count = width*height;
  for (i = 0; i < count; i++)
  {
    index = *map++;
    *red++ = r[index];
    *green++ = g[index];
    *blue++ = b[index];
  }
}

void imRGB2Gray(int width, int height, unsigned char *red, unsigned char *green, unsigned char *blue, unsigned char *map, long *grays)
{
  int i, count, c;

  for (c = 0; c < 256; c++)
    *grays++ = imColorEncode((unsigned char)c, (unsigned char)c, (unsigned char)c);

  count = width*height;
  for (i = 0; i < count; i++)
  {
    *map++ = (unsigned char)((*red++ * 30 + *green++ * 59 + *blue++ * 11) / 100);
  }
}

void imMap2Gray(int width, int height, unsigned char *map, int palette_count, long *palette, unsigned char *gray_map, long *grays)
{
  int i, count, c;
  unsigned char cnv_table[256];
  unsigned char r, g, b;

  for (c = 0; c < 256; c++)
    *grays++ = imColorEncode((unsigned char)c, (unsigned char)c, (unsigned char)c);

  for (c = 0; c < palette_count; c++)
  {
    imColorDecode(&r, &g, &b, palette[c]);
    cnv_table[c] = (unsigned char)((r * 30 + g * 59 + b * 11) / 100);
  }

  count = width*height;
  for (i = 0; i < count; i++)
  {
    *gray_map++ = cnv_table[*map++];
  }
}
