/** \file
 * \brief Image Conversion
 *
 * See Copyright Notice in im_lib.h
 * $Id: im_convertcolor.cpp,v 1.3 2009/08/18 05:13:18 scuri Exp $
 */

#include "im.h"
#include "im_util.h"
#include "im_complex.h"
#include "im_image.h"
#include "im_convert.h"
#include "im_color.h"
#include "im_counter.h"

#include <stdlib.h>
#include <assert.h>
#include <memory.h>

void imConvertMapToRGB(unsigned char* data, int count, int depth, int packed, long* palette, int palette_count)
{
  int c, i, delta;
  unsigned char r[256], g[256], b[256];
  unsigned char *r_data, *g_data, *b_data;

  unsigned char* src_data = data + count-1;
  if (packed)
  {
    r_data = data + depth*(count-1);
    g_data = r_data + 1;
    b_data = r_data + 2;
    delta = depth;
  }
  else
  {
    r_data = data +   count - 1;
    g_data = data + 2*count - 1;
    b_data = data + 3*count - 1;
    delta = 1;
  }

  for (c = 0; c < palette_count; c++)
    imColorDecode(&r[c], &g[c], &b[c], palette[c]);

  for (i = 0; i < count; i++)
  {
    int index = *src_data;
    *r_data = r[index];
    *g_data = g[index];
    *b_data = b[index];

    r_data -= delta;
    g_data -= delta;
    b_data -= delta;
    src_data--;
  }
}

static void iConvertSetTranspMap(imbyte *src_map, imbyte *dst_alpha, int count, imbyte *transp_map, int transp_count)
{
  for(int i = 0; i < count; i++)
  {
    if (*src_map < transp_count)
      *dst_alpha = transp_map[*src_map];
    else
      *dst_alpha = 255;  /* opaque */

    src_map++;
    dst_alpha++;
  }
}

static void iConvertSetTranspIndex(imbyte *src_map, imbyte *dst_alpha, int count, imbyte index)
{
  for(int i = 0; i < count; i++)
  {
    if (*src_map == index)
      *dst_alpha = 0;    /* full transparent */
    else
      *dst_alpha = 255;  /* opaque */

    src_map++;
    dst_alpha++;
  }
}

static void iConvertSetTranspColor(imbyte **dst_data, int count, imbyte r, imbyte g, imbyte b)
{
  imbyte *pr = dst_data[0];
  imbyte *pg = dst_data[1];
  imbyte *pb = dst_data[2];
  imbyte *pa = dst_data[3];

  for(int i = 0; i < count; i++)
  {
    if (*pr == r &&
        *pg == g &&
        *pb == b)
      *pa = 0;    /* transparent */
    else
      *pa = 255;  /* opaque */
    
    pr++;
    pg++;
    pb++;
    pa++;
  }
}

// convert bin2gray and gray2bin
inline void iConvertBinary(imbyte* map, int count, imbyte value)
{
  imbyte thres = (value == 255)? 1: 128;

  // if gray2bin, check for invalid gray that already is binary
  if (value != 255)
  {
    imbyte vmax = 0, *pmap = map;
    for (int i = 0; i < count; i++)
    {
      if (*pmap > vmax)
        vmax = *pmap;

      pmap++;
    }

    if (vmax == 1)
      thres = 1;
    else
      thres = vmax / 2;
  }

  for (int i = 0; i < count; i++)
  {
    if (*map >= thres)
      *map = value;
    else
      *map = 0;

    map++;
  }
}

static void iConvertMap2Gray(const imbyte* src_map, imbyte* dst_map, int count, const long* palette, const int palette_count)
{
  imbyte r, g, b;
  imbyte remap[256];

  for (int c = 0; c < palette_count; c++)
  {
    imColorDecode(&r, &g, &b, palette[c]);
    remap[c] = imColorRGB2Luma(r, g, b);
  }

  for (int i = 0; i < count; i++)
  {
    *dst_map++ = remap[*src_map++];
  }
}

static void iConvertMapToRGB(const imbyte* src_map, imbyte* red, imbyte* green, imbyte* blue, int count, const long* palette, const int palette_count)
{
  imbyte r[256], g[256], b[256];
  for (int c = 0; c < palette_count; c++)
    imColorDecode(&r[c], &g[c], &b[c], palette[c]);

  for (int i = 0; i < count; i++)
  {
    int index = *src_map++;
    *red++ = r[index];
    *green++ = g[index];
    *blue++ = b[index];
  }
}

template <class T> 
int iDoConvert2Gray(int count, int data_type, 
                    const T** src_data, int src_color_space, T** dst_data, int counter)
{
  int i;
  T max;

  const T* src_map0 = src_data[0];
  const T* src_map1 = src_data[1];
  const T* src_map2 = src_data[2];
  const T* src_map3 = (src_color_space == IM_CMYK)? src_data[3]: 0;
  T* dst_map = dst_data[0];

  imCounterTotal(counter, count, "Converting To Gray...");

  switch(src_color_space)
  {
  case IM_XYZ: 
    max = (T)imColorMax(data_type);
    for (i = 0; i < count; i++)
    {
      // scale to 0-1
      float c1 = imColorReconstruct(*src_map1++, max);  // use only Y component

      // do gamma correction then scale back to 0-max
      *dst_map++ = imColorQuantize(imColorTransfer2Nonlinear(c1), max);

      if (!imCounterInc(counter))
        return IM_ERR_COUNTER;
    }
    break;
  case IM_CMYK: 
    max = (T)imColorMax(data_type);
    for (i = 0; i < count; i++)
    {
      T r, g, b;
      // result is still 0-max
      imColorCMYK2RGB(*src_map0++, *src_map1++, *src_map2++, *src_map3++, r, g, b, max);
      *dst_map++ = imColorRGB2Luma(r, g, b);

      if (!imCounterInc(counter))
        return IM_ERR_COUNTER;
    }
    break;
  case IM_RGB:
    for (i = 0; i < count; i++)
    {
      *dst_map++ = imColorRGB2Luma(*src_map0++, *src_map1++, *src_map2++);

      if (!imCounterInc(counter))
        return IM_ERR_COUNTER;
    }
    break;
  case IM_LUV:
  case IM_LAB:
    max = (T)imColorMax(data_type);
    for (i = 0; i < count; i++)
    {
      // to increase precision do intermediate conversions in float
      
      float c0 = imColorReconstruct(*src_map0++, max); // scale to 0-1
      c0 = imColorLightness2Luminance(c0);             // do the convertion

      // do gamma correction then scale back to 0-max
      *dst_map++ = imColorQuantize(imColorTransfer2Nonlinear(c0), max);

      if (!imCounterInc(counter))
        return IM_ERR_COUNTER;
    }
    break;
  default:
    return IM_ERR_DATA;
  }

  return IM_ERR_NONE;
}

template <class T> 
int iDoConvert2RGB(int count, int data_type, 
                   const T** src_data, int src_color_space, T** dst_data, int counter)
{
  int i;
  T max, zero;

  const T* src_map0 = src_data[0];
  const T* src_map1 = src_data[1];
  const T* src_map2 = src_data[2];
  const T* src_map3 = (src_color_space == IM_CMYK)? src_data[3]: 0;
  T* dst_map0 = dst_data[0];
  T* dst_map1 = dst_data[1];
  T* dst_map2 = dst_data[2];

  imCounterTotal(counter, count, "Converting To RGB...");

  switch(src_color_space)
  {
  case IM_XYZ: 
    max = (T)imColorMax(data_type);
    for (i = 0; i < count; i++)
    {
      // to increase precision do intermediate conversions in float

      // scale to 0-1
      float c0 = imColorReconstruct(*src_map0++, max);
      float c1 = imColorReconstruct(*src_map1++, max);
      float c2 = imColorReconstruct(*src_map2++, max);

      // result is still 0-1
      imColorXYZ2RGB(c0, c1, c2, 
                     c0, c1, c2, 1.0f);

      // do gamma correction then scale back to 0-max
      *dst_map0++ = imColorQuantize(imColorTransfer2Nonlinear(c0), max);
      *dst_map1++ = imColorQuantize(imColorTransfer2Nonlinear(c1), max);
      *dst_map2++ = imColorQuantize(imColorTransfer2Nonlinear(c2), max);

      if (!imCounterInc(counter))
        return IM_ERR_COUNTER;
    }
    break;
  case IM_YCBCR: 
    max = (T)imColorMax(data_type);
    zero = (T)imColorZero(data_type);
    for (i = 0; i < count; i++)
    {
      imColorYCbCr2RGB(*src_map0++, *src_map1++, *src_map2++, 
                       *dst_map0++, *dst_map1++, *dst_map2++, zero, max);
    }
    break;
  case IM_CMYK: 
    max = (T)imColorMax(data_type);
    for (i = 0; i < count; i++)
    {
      // result is still 0-max
      imColorCMYK2RGB(*src_map0++, *src_map1++, *src_map2++, *src_map3++, 
                      *dst_map0++, *dst_map1++, *dst_map2++, max);

      if (!imCounterInc(counter))
        return IM_ERR_COUNTER;
    }
    break;
  case IM_LUV:
  case IM_LAB:
    max = (T)imColorMax(data_type);
    for (i = 0; i < count; i++)
    {
      // to increase precision do intermediate conversions in float

      // scale to 0-1 and -0.5/+0.5
      float c0 = imColorReconstruct(*src_map0++, max);
      float c1 = imColorReconstruct(*src_map1++, max) - 0.5f;
      float c2 = imColorReconstruct(*src_map2++, max) - 0.5f;

      if (src_color_space == IM_LUV)
        imColorLuv2XYZ(c0, c1, c2,  // conversion in-place
                       c0, c1, c2);
      else
        imColorLab2XYZ(c0, c1, c2,  // conversion in-place
                       c0, c1, c2);

      imColorXYZ2RGB(c0, c1, c2,    // conversion in-place
                     c0, c1, c2, 1.0f);

      // do gamma correction then scale back to 0-max
      *dst_map0++ = imColorQuantize(imColorTransfer2Nonlinear(c0), max);
      *dst_map1++ = imColorQuantize(imColorTransfer2Nonlinear(c1), max);
      *dst_map2++ = imColorQuantize(imColorTransfer2Nonlinear(c2), max);

      if (!imCounterInc(counter))
        return IM_ERR_COUNTER;
    }
    break;
  default:
    return IM_ERR_DATA;
  }

  return IM_ERR_NONE;
}

template <class T> 
int iDoConvert2YCbCr(int count, int data_type, 
                     const T** src_data, int src_color_space, T** dst_data, int counter)
{
  int i;
  T zero;

  const T* src_map0 = src_data[0];
  const T* src_map1 = src_data[1];
  const T* src_map2 = src_data[2];
  T* dst_map0 = dst_data[0];
  T* dst_map1 = dst_data[1];
  T* dst_map2 = dst_data[2];

  imCounterTotal(counter, count, "Converting To YCbCr...");

  switch(src_color_space)
  {
  case IM_RGB: 
    zero = (T)imColorZero(data_type);
    for (i = 0; i < count; i++)
    {
      imColorRGB2YCbCr(*src_map0++, *src_map1++, *src_map2++, 
                       *dst_map0++, *dst_map1++, *dst_map2++, zero);

      if (!imCounterInc(counter))
        return IM_ERR_COUNTER;
    }
    break;
  default:
    return IM_ERR_DATA;
  }

  return IM_ERR_NONE;
}

template <class T> 
int iDoConvert2XYZ(int count, int data_type, 
                   const T** src_data, int src_color_space, T** dst_data, int counter)
{
  int i;
  T max;

  const T* src_map0 = src_data[0];
  const T* src_map1 = (src_color_space == IM_GRAY)? 0: src_data[1];
  const T* src_map2 = (src_color_space == IM_GRAY)? 0: src_data[2];
  T* dst_map0 = dst_data[0];
  T* dst_map1 = dst_data[1];
  T* dst_map2 = dst_data[2];

  imCounterTotal(counter, count, "Converting To XYZ...");

  switch(src_color_space)
  {
  case IM_GRAY: 
    max = (T)imColorMax(data_type);
    for (i = 0; i < count; i++)
    {
      // scale to 0-1
      float c0 = imColorReconstruct(*src_map0++, max);

      // do gamma correction
      c0 = imColorTransfer2Linear(c0);

      // then scale back to 0-max
      *dst_map0++ = imColorQuantize(c0*0.9505f, max);    // Compensate D65 white point
      *dst_map1++ = imColorQuantize(c0, max);
      *dst_map2++ = imColorQuantize(c0*1.0890f, max);

      if (!imCounterInc(counter))
        return IM_ERR_COUNTER;
    }
    break;
  case IM_RGB: 
    max = (T)imColorMax(data_type);
    for (i = 0; i < count; i++)
    {
      // to increase precision do intermediate conversions in float

      // scale to 0-1
      float c0 = imColorReconstruct(*src_map0++, max);
      float c1 = imColorReconstruct(*src_map1++, max);
      float c2 = imColorReconstruct(*src_map2++, max);

      // do gamma correction
      c0 = imColorTransfer2Linear(c0);
      c1 = imColorTransfer2Linear(c1);
      c2 = imColorTransfer2Linear(c2);

      // result is still 0-1
      imColorRGB2XYZ(c0, c1, c2, 
                     c0, c1, c2);

      // then scale back to 0-max
      *dst_map0++ = imColorQuantize(c0, max);
      *dst_map1++ = imColorQuantize(c1, max);
      *dst_map2++ = imColorQuantize(c2, max);

      if (!imCounterInc(counter))
        return IM_ERR_COUNTER;
    }
    break;
  case IM_LUV:
  case IM_LAB:
    max = (T)imColorMax(data_type);
    for (i = 0; i < count; i++)
    {
      // to increase precision do intermediate conversions in float
      // scale to 0-1 and -0.5/+0.5
      float c0 = imColorReconstruct(*src_map0++, max);
      float c1 = imColorReconstruct(*src_map1++, max) - 0.5f;
      float c2 = imColorReconstruct(*src_map2++, max) - 0.5f;

      if (src_color_space == IM_LUV)
        imColorLuv2XYZ(c0, c1, c2,  // convertion in-place
                       c0, c1, c2);
      else
        imColorLab2XYZ(c0, c1, c2,  // convertion in-place
                       c0, c1, c2);

      // scale back to 0-max
      *dst_map0++ = imColorQuantize(c0, max);
      *dst_map1++ = imColorQuantize(c1, max);
      *dst_map2++ = imColorQuantize(c2, max);

      if (!imCounterInc(counter))
        return IM_ERR_COUNTER;
    }
    break;
  default:
    return IM_ERR_DATA;
  }

  return IM_ERR_NONE;
}

template <class T> 
int iDoConvert2Lab(int count, int data_type, 
                   const T** src_data, int src_color_space, T** dst_data, int counter)
{
  int i;
  T max;

  const T* src_map0 = src_data[0];
  const T* src_map1 = (src_color_space == IM_GRAY)? 0: src_data[1];
  const T* src_map2 = (src_color_space == IM_GRAY)? 0: src_data[2];
  T* dst_map0 = dst_data[0];
  T* dst_map1 = dst_data[1];
  T* dst_map2 = dst_data[2];

  imCounterTotal(counter, count, "Converting To Lab...");

  switch(src_color_space)
  {
  case IM_GRAY: 
    max = (T)imColorMax(data_type);
    for (i = 0; i < count; i++)
    {
      // scale to 0-1
      float c0 = imColorReconstruct(*src_map0++, max);

      // do gamma correction
      c0 = imColorTransfer2Linear(c0);

      // do conversion
      c0 = imColorLuminance2Lightness(c0);

      // then scale back to 0-max
      *dst_map0++ = imColorQuantize(c0, max);  // update only the L component

      if (!imCounterInc(counter))
        return IM_ERR_COUNTER;
    }
    break;
  case IM_RGB: 
    max = (T)imColorMax(data_type);
    for (i = 0; i < count; i++)
    {
      // to increase precision do intermediate conversions in float

      // scale to 0-1
      float c0 = imColorReconstruct(*src_map0++, max);
      float c1 = imColorReconstruct(*src_map1++, max);
      float c2 = imColorReconstruct(*src_map2++, max);

      // do gamma correction
      c0 = imColorTransfer2Linear(c0);
      c1 = imColorTransfer2Linear(c1);
      c2 = imColorTransfer2Linear(c2);

      imColorRGB2XYZ(c0, c1, c2,  // convertion in-place
                     c0, c1, c2);

      imColorXYZ2Lab(c0, c1, c2,  // convertion in-place
                     c0, c1, c2);

      // then scale back to 0-max
      *dst_map0++ = imColorQuantize(c0, max);
      *dst_map1++ = imColorQuantize(c1 + 0.5f, max);
      *dst_map2++ = imColorQuantize(c2 + 0.5f, max);

      if (!imCounterInc(counter))
        return IM_ERR_COUNTER;
    }
    break;
  case IM_XYZ:
    max = (T)imColorMax(data_type);
    for (i = 0; i < count; i++)
    {
      // to increase precision do intermediate conversions in float
      // scale to 0-1 and -0.5/+0.5
      float c0 = imColorReconstruct(*src_map0++, max);
      float c1 = imColorReconstruct(*src_map1++, max);
      float c2 = imColorReconstruct(*src_map2++, max);

      imColorXYZ2Lab(c0, c1, c2,  // convertion in-place
                     c0, c1, c2);

      // scale back to 0-max
      *dst_map0++ = imColorQuantize(c0, max);
      *dst_map1++ = imColorQuantize(c1 + 0.5f, max);
      *dst_map2++ = imColorQuantize(c2 + 0.5f, max);

      if (!imCounterInc(counter))
        return IM_ERR_COUNTER;
    }
    break;
  case IM_LUV:
    max = (T)imColorMax(data_type);
    for (i = 0; i < count; i++)
    {
      // to increase precision do intermediate conversions in float
      // scale to 0-1 and -0.5/+0.5
      float c0 = imColorReconstruct(*src_map0++, max);
      float c1 = imColorReconstruct(*src_map1++, max) - 0.5f;
      float c2 = imColorReconstruct(*src_map2++, max) - 0.5f;

      imColorLuv2XYZ(c0, c1, c2,  // convertion in-place
                     c0, c1, c2);
      imColorXYZ2Lab(c0, c1, c2,  // convertion in-place
                     c0, c1, c2);

      // scale back to 0-max
      *dst_map0++ = imColorQuantize(c0, max);
      *dst_map1++ = imColorQuantize(c1 + 0.5f, max);
      *dst_map2++ = imColorQuantize(c2 + 0.5f, max);

      if (!imCounterInc(counter))
        return IM_ERR_COUNTER;
    }
    break;
  default:
    return IM_ERR_DATA;
  }

  return IM_ERR_NONE;
}

template <class T> 
int iDoConvert2Luv(int count, int data_type, 
                   const T** src_data, int src_color_space, T** dst_data, int counter)
{
  int i;
  T max;

  const T* src_map0 = src_data[0];
  const T* src_map1 = (src_color_space == IM_GRAY)? 0: src_data[1];
  const T* src_map2 = (src_color_space == IM_GRAY)? 0: src_data[2];
  T* dst_map0 = dst_data[0];
  T* dst_map1 = dst_data[1];
  T* dst_map2 = dst_data[2];

  imCounterTotal(counter, count, "Converting To Luv...");

  switch(src_color_space)
  {
  case IM_GRAY: 
    max = (T)imColorMax(data_type);
    for (i = 0; i < count; i++)
    {
      // scale to 0-1
      float c0 = imColorReconstruct(*src_map0++, max);

      // do gamma correction
      c0 = imColorTransfer2Linear(c0);

      // do conversion
      c0 = imColorLuminance2Lightness(c0);

      // then scale back to 0-max
      *dst_map0++ = imColorQuantize(c0, max);  // update only the L component

      if (!imCounterInc(counter))
        return IM_ERR_COUNTER;
    }
    break;
  case IM_RGB: 
    max = (T)imColorMax(data_type);
    for (i = 0; i < count; i++)
    {
      // to increase precision do intermediate conversions in float

      // scale to 0-1
      float c0 = imColorReconstruct(*src_map0++, max);
      float c1 = imColorReconstruct(*src_map1++, max);
      float c2 = imColorReconstruct(*src_map2++, max);

      // do gamma correction
      c0 = imColorTransfer2Linear(c0);
      c1 = imColorTransfer2Linear(c1);
      c2 = imColorTransfer2Linear(c2);

      imColorRGB2XYZ(c0, c1, c2,  // convertion in-place
                     c0, c1, c2);

      imColorXYZ2Luv(c0, c1, c2,  // convertion in-place
                     c0, c1, c2);

      // then scale back to 0-max
      *dst_map0++ = imColorQuantize(c0, max);
      *dst_map1++ = imColorQuantize(c1 + 0.5f, max);
      *dst_map2++ = imColorQuantize(c2 + 0.5f, max);

      if (!imCounterInc(counter))
        return IM_ERR_COUNTER;
    }
    break;
  case IM_XYZ:
    max = (T)imColorMax(data_type);
    for (i = 0; i < count; i++)
    {
      // to increase precision do intermediate conversions in float
      // scale to 0-1 and -0.5/+0.5
      float c0 = imColorReconstruct(*src_map0++, max);
      float c1 = imColorReconstruct(*src_map1++, max);
      float c2 = imColorReconstruct(*src_map2++, max);

      imColorXYZ2Luv(c0, c1, c2,  // convertion in-place
                     c0, c1, c2);

      // scale back to 0-max
      *dst_map0++ = imColorQuantize(c0, max);
      *dst_map1++ = imColorQuantize(c1 + 0.5f, max);
      *dst_map2++ = imColorQuantize(c2 + 0.5f, max);

      if (!imCounterInc(counter))
        return IM_ERR_COUNTER;
    }
    break;
  case IM_LAB:
    max = (T)imColorMax(data_type);
    for (i = 0; i < count; i++)
    {
      // to increase precision do intermediate conversions in float
      // scale to 0-1 and -0.5/+0.5
      float c0 = imColorReconstruct(*src_map0++, max);
      float c1 = imColorReconstruct(*src_map1++, max) - 0.5f;
      float c2 = imColorReconstruct(*src_map2++, max) - 0.5f;

      imColorLab2XYZ(c0, c1, c2,  // convertion in-place
                     c0, c1, c2);
      imColorXYZ2Luv(c0, c1, c2,  // convertion in-place
                     c0, c1, c2);

      // scale back to 0-max
      *dst_map0++ = imColorQuantize(c0, max);
      *dst_map1++ = imColorQuantize(c1 + 0.5f, max);
      *dst_map2++ = imColorQuantize(c2 + 0.5f, max);

      if (!imCounterInc(counter))
        return IM_ERR_COUNTER;
    }
    break;
  default:
    return IM_ERR_DATA;
  }

  return IM_ERR_NONE;
}

template <class T> 
int iDoConvertColorSpace(int count, int data_type, 
                                 const T** src_data, int src_color_space, 
                                       T** dst_data, int dst_color_space)
{
  int ret = IM_ERR_DATA, 
      convert2rgb = 0;

  if ((dst_color_space == IM_XYZ ||
       dst_color_space == IM_LAB ||
       dst_color_space == IM_LUV) && 
      (src_color_space == IM_CMYK ||
       src_color_space == IM_YCBCR))
  {
    convert2rgb = 1;
  }    

  if (dst_color_space == IM_YCBCR && src_color_space != IM_RGB)
    convert2rgb = 1;

  int counter = imCounterBegin("Convert Color Space");

  if (convert2rgb)
  {
    ret = iDoConvert2RGB(count, data_type, src_data, src_color_space, dst_data, counter);     

    if (ret != IM_ERR_NONE) 
    {
      imCounterEnd(counter);
      return ret;
    }

    src_data = (const T**)dst_data;
    src_color_space = IM_RGB;
  }

  switch(dst_color_space)
  {
  case IM_GRAY: 
    ret = iDoConvert2Gray(count, data_type, src_data, src_color_space, dst_data, counter);
    break;
  case IM_RGB: 
    ret = iDoConvert2RGB(count, data_type, src_data, src_color_space, dst_data, counter);
    break;
  case IM_YCBCR: 
    ret = iDoConvert2YCbCr(count, data_type, src_data, src_color_space, dst_data, counter); 
    break;
  case IM_XYZ: 
    ret = iDoConvert2XYZ(count, data_type, src_data, src_color_space, dst_data, counter);
    break;
  case IM_LAB: 
    ret = iDoConvert2Lab(count, data_type, src_data, src_color_space, dst_data, counter);
    break;
  case IM_LUV: 
    ret = iDoConvert2Luv(count, data_type, src_data, src_color_space, dst_data, counter);
    break;
  default:
    ret = IM_ERR_DATA;
    break;
  }

  imCounterEnd(counter);

  return ret;
}

static int iConvertColorSpace(const imImage* src_image, imImage* dst_image)
{
  switch(src_image->data_type)
  {
  case IM_BYTE:
    return iDoConvertColorSpace(src_image->count, src_image->data_type,
                         (const imbyte**)src_image->data, src_image->color_space, 
                               (imbyte**)dst_image->data, dst_image->color_space);
  case IM_USHORT:
    return iDoConvertColorSpace(src_image->count, src_image->data_type, 
                         (const imushort**)src_image->data, src_image->color_space, 
                               (imushort**)dst_image->data, dst_image->color_space);
  case IM_INT:
    return iDoConvertColorSpace(src_image->count, src_image->data_type,
                         (const int**)src_image->data, src_image->color_space, 
                               (int**)dst_image->data, dst_image->color_space);
  case IM_FLOAT:
    return iDoConvertColorSpace(src_image->count, src_image->data_type, 
                         (const float**)src_image->data, src_image->color_space, 
                               (float**)dst_image->data, dst_image->color_space);
  case IM_CFLOAT:
    /* treat complex as two real values */
    return iDoConvertColorSpace(2*src_image->count, src_image->data_type,
                         (const float**)src_image->data, src_image->color_space, 
                               (float**)dst_image->data, dst_image->color_space);
  }

  return IM_ERR_DATA;
}

int imConvertColorSpace(const imImage* src_image, imImage* dst_image)
{
  int ret = IM_ERR_NONE;
  assert(src_image);
  assert(dst_image);

  if (!imImageMatchDataType(src_image, dst_image))
    return IM_ERR_DATA;

  if (src_image->color_space != dst_image->color_space)
  {
    switch(dst_image->color_space)
    {
    case IM_RGB:
      switch(src_image->color_space)
      {
      case IM_BINARY:
          memcpy(dst_image->data[0], src_image->data[0], dst_image->plane_size);
          iConvertBinary((imbyte*)dst_image->data[0], dst_image->count, 255);
          memcpy(dst_image->data[1], dst_image->data[0], dst_image->plane_size);
          memcpy(dst_image->data[2], dst_image->data[0], dst_image->plane_size);
        ret = IM_ERR_NONE;
        break;
      case IM_MAP:
        iConvertMapToRGB((imbyte*)src_image->data[0], (imbyte*)dst_image->data[0], (imbyte*)dst_image->data[1], (imbyte*)dst_image->data[2], dst_image->count, src_image->palette, src_image->palette_count);
        ret = IM_ERR_NONE;
        break;
      case IM_GRAY:
          memcpy(dst_image->data[0], src_image->data[0], dst_image->plane_size);
          memcpy(dst_image->data[1], src_image->data[0], dst_image->plane_size);
          memcpy(dst_image->data[2], src_image->data[0], dst_image->plane_size);
        ret = IM_ERR_NONE;
        break;
      default: 
        ret = iConvertColorSpace(src_image, dst_image);
        break;
      }
      break;
    case IM_GRAY:  
      switch(src_image->color_space)
      {
      case IM_BINARY:
        memcpy(dst_image->data[0], src_image->data[0], dst_image->size);
        iConvertBinary((imbyte*)dst_image->data[0], dst_image->count, 255);
        ret = IM_ERR_NONE;
        break;
      case IM_MAP:
        iConvertMap2Gray((imbyte*)src_image->data[0], (imbyte*)dst_image->data[0], dst_image->count, src_image->palette, src_image->palette_count);
        ret = IM_ERR_NONE;
        break;
      case IM_YCBCR: 
        memcpy(dst_image->data[0], src_image->data[0], dst_image->plane_size);
        ret = IM_ERR_NONE;
        break;
      default:
        ret = iConvertColorSpace(src_image, dst_image);
        break;
      }
      break;
    case IM_MAP:   
      switch(src_image->color_space)
      {
      case IM_BINARY: // no break, same procedure as gray
      case IM_GRAY:
        memcpy(dst_image->data[0], src_image->data[0], dst_image->size);
        dst_image->palette_count = src_image->palette_count;
        memcpy(dst_image->palette, src_image->palette, dst_image->palette_count*sizeof(long));
        ret = IM_ERR_NONE;
        break;
      case IM_RGB:
        dst_image->palette_count = 256;
        ret = imConvertRGB2Map(src_image->width, src_image->height, 
                               (imbyte*)src_image->data[0], (imbyte*)src_image->data[1], (imbyte*)src_image->data[2], 
                               (imbyte*)dst_image->data[0], dst_image->palette, &dst_image->palette_count);
        break;
      default:
        ret = IM_ERR_DATA;
        break;
      }
      break;
    case IM_BINARY:
      switch(src_image->color_space)
      {
      case IM_GRAY:
        memcpy(dst_image->data[0], src_image->data[0], dst_image->size);
        iConvertBinary((imbyte*)dst_image->data[0], dst_image->count, 1);
        ret = IM_ERR_NONE;
        break;
      case IM_MAP:           // convert to gray, then convert to binary
        iConvertMap2Gray((imbyte*)src_image->data[0], (imbyte*)dst_image->data[0], dst_image->count, src_image->palette, src_image->palette_count);
        iConvertBinary((imbyte*)dst_image->data[0], dst_image->count, 1);
        ret = IM_ERR_NONE;
        break;
      case IM_YCBCR:         // convert to gray, then convert to binary
        memcpy(dst_image->data[0], src_image->data[0], dst_image->plane_size);
        iConvertBinary((imbyte*)dst_image->data[0], dst_image->count, 1);
        ret = IM_ERR_NONE;
        break;
      default:               // convert to gray, then convert to binary
        dst_image->color_space = IM_GRAY;
        ret = iConvertColorSpace(src_image, dst_image);
        dst_image->color_space = IM_BINARY;
        if (ret == IM_ERR_NONE)
          iConvertBinary((imbyte*)dst_image->data[0], dst_image->count, 1);
        ret = IM_ERR_NONE;
        break;
      }
      break;
    case IM_YCBCR: 
      switch(src_image->color_space)
      {
      case IM_GRAY:
        memcpy(dst_image->data[0], src_image->data[0], dst_image->plane_size);
        ret = IM_ERR_NONE;
        break;
      default:
        ret = iConvertColorSpace(src_image, dst_image);
        break;
      }
      break;
    default: 
      ret = iConvertColorSpace(src_image, dst_image);
      break;
    }
  }

  if (src_image->has_alpha && dst_image->has_alpha)
    memcpy(dst_image->data[dst_image->depth], src_image->data[src_image->depth], src_image->plane_size);
  else if (dst_image->color_space == IM_RGB && dst_image->data_type == IM_BYTE && dst_image->has_alpha)
  {
    if (src_image->color_space == IM_RGB)
    {
      imbyte* transp_color = (imbyte*)imImageGetAttribute(src_image, "TransparencyColor", NULL, NULL);
      if (transp_color)
        iConvertSetTranspColor((imbyte**)dst_image->data, dst_image->count, *(transp_color+0), *(transp_color+1), *(transp_color+2));
      else
        memset(dst_image->data[3], 255, dst_image->count);
    }
    else
    {
      int transp_count;
      imbyte* transp_index = (imbyte*)imImageGetAttribute(src_image, "TransparencyIndex", NULL, NULL);
      imbyte* transp_map = (imbyte*)imImageGetAttribute(src_image, "TransparencyMap", NULL, &transp_count);
      if (transp_map)
        iConvertSetTranspMap((imbyte*)src_image->data[0], (imbyte*)dst_image->data[3], dst_image->count, transp_map, transp_count);
      else if (transp_index)
        iConvertSetTranspIndex((imbyte*)src_image->data[0], (imbyte*)dst_image->data[3], dst_image->count, *transp_index);
      else
        memset(dst_image->data[3], 255, dst_image->count);
    }
  }

  return ret;
}

