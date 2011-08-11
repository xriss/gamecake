/** \file
 * \brief Tone Gamut Operations
 *
 * See Copyright Notice in im_lib.h
 * $Id: im_tonegamut.cpp,v 1.1 2008/10/17 06:16:33 scuri Exp $
 */


#include <im.h>
#include <im_util.h>
#include <im_math.h>

#include "im_process_pon.h"

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <string.h>
#include <math.h>


template <class T>
static inline T line_op(const T& v, const T& min, const T& max, const float& a, const float& b)
{
  float r = v * a + b;
  if (r > (float)max) return max;
  if (r < (float)min) return min;
  return (T)r;
}

template <class T>
static inline T normal_op(const T& v, const T& min, const T& range)
{
  return (T)(float(v - min) / float(range));
}

template <class T>
static inline T zerostart_op(const T& v, const T& min)
{
  return (T)(v - min);
}

template <class T>
static inline float invert_op(const T& v, const T& min, const T& range)
{
  return 1.0f - float(v - min) / float(range);
}

template <class T>
static inline T solarize_op(const T& v, const T& level, const float& A, const float& B)
{
  if (v > level)
    return (T)(v * A + B);
  else
    return v;
}

template <class T>
static inline T slice_op(const T& v, const T& min, const T& max, const T& start, const T& end, int bin)
{
  if (v < start || v > end)
    return min;
  else
  {
    if (bin)
      return max;
    else
      return v;
  }
}

template <class T>
static inline T tonecrop_op(const T& v, const T& start, const T& end)
{
  if (v < start)
    return start;
  if (v > end)
    return end;
  else
    return v;
}

template <class T>
static inline T expand_op(const T& v, const T& min, const T& max, const T& start, const float& norm)
{
  float r = (v - start)*norm + min;
  if (r > (float)max) return max;
  if (r < (float)min) return min;
  return (T)r;
}

template <class T>
static inline float norm_pow_op(const T& v, const T& min, const T& range, const float& gamma)
{
  return (float)pow(float(v - min) / float(range), gamma);
}

template <class T>
static inline float norm_log_op(const T& v, const T& min, const T& range, const float& norm, const float& K)
{
  return (float)(log(K * float(v - min) / float(range) + 1) / norm);
}

template <class T>
static inline float norm_exp_op(const T& v, const T& min, const T& range, const float& norm, const float& K)
{
  return (float)((exp(K * float(v - min) / float(range)) - 1) / norm);
}

template <class T> 
static void DoNormalizedUnaryOp(T *map, T *new_map, int count, int op, float *args)
{
  int i;
  T min, max, range;

  int size_of = sizeof(imbyte);
  if (sizeof(T) == size_of)
  {
    min = 0;
    max = 255;
  }
  else
  {
    imMinMax(map, count, min, max);

    if (min == max)
    {
      max = min + 1;

      if (min != 0)
        min = min - 1;
    }
  }

  range = max-min;
  
  switch(op)
  {
  case IM_GAMUT_NORMALIZE:
    {
      if (min >= 0 && max <= 1)
      {
        for (i = 0; i < count; i++)
          new_map[i] = (T)map[i];
      }
      else
      {
        for (i = 0; i < count; i++)
          new_map[i] = normal_op(map[i], min, range);
      }
      break;
    }
  case IM_GAMUT_INVERT:
    for (i = 0; i < count; i++)
      new_map[i] = (T)(invert_op(map[i], min, range)*range + min);
    break;
  case IM_GAMUT_ZEROSTART:
    for (i = 0; i < count; i++)
      new_map[i] = (T)zerostart_op(map[i], min);
    break;
  case IM_GAMUT_SOLARIZE:
    {
      T level =  (T)(((100 - args[0]) * range) / 100.0f + min);
      float A = float(level - min) / float(level - max);
      float B = float(level * range) / float(max - level);
      for (i = 0; i < count; i++)
        new_map[i] = solarize_op(map[i], level, A, B);
      break;
    }
  case IM_GAMUT_POW:
    for (i = 0; i < count; i++)
      new_map[i] = (T)(norm_pow_op(map[i], min, range, args[0])*range + min);
    break;
  case IM_GAMUT_LOG:
    {
      float norm = float(log(args[0] + 1));
      for (i = 0; i < count; i++)
        new_map[i] = (T)(norm_log_op(map[i], min, range, norm, args[0])*range + min);
      break;
    }
  case IM_GAMUT_EXP:
    {
      float norm = float(exp(args[0]) - 1);
      for (i = 0; i < count; i++)
        new_map[i] = (T)(norm_exp_op(map[i], min, range, norm, args[0])*range + min);
      break;
    }
  case IM_GAMUT_SLICE:
    {
      if (args[0] > args[1]) { float tmp = args[1]; args[1] = args[0]; args[0] = tmp; }
      if (args[1] > max) args[1] = (float)max;
      if (args[0] < min) args[0] = (float)min;
      for (i = 0; i < count; i++)
        new_map[i] = slice_op(map[i], min, max, (T)args[0], (T)args[1], (int)args[2]);
      break;
    }
  case IM_GAMUT_CROP:
    {
      if (args[0] > args[1]) { float tmp = args[1]; args[1] = args[0]; args[0] = tmp; }
      if (args[1] > max) args[1] = (float)max;
      if (args[0] < min) args[0] = (float)min;
      for (i = 0; i < count; i++)
        new_map[i] = tonecrop_op(map[i], (T)args[0], (T)args[1]);
      break;
    }
  case IM_GAMUT_EXPAND:
    {
      if (args[0] > args[1]) { float tmp = args[1]; args[1] = args[0]; args[0] = tmp; }
      if (args[1] > max) args[1] = (float)max;
      if (args[0] < min) args[0] = (float)min;
      float norm = float(max - min)/(args[1] - args[0]);
      for (i = 0; i < count; i++)
        new_map[i] = expand_op(map[i], min, max, (T)args[0], norm);
      break;
    }
  case IM_GAMUT_BRIGHTCONT:
    {
      float bs = (args[0] * range) / 100.0f;
      float a = (float)tan((45+args[1]*0.449999)/57.2957795);
      float b = bs + (float)range*(1.0f - a)/2.0f;
      for (i = 0; i < count; i++)
        new_map[i] = line_op(map[i], min, max, a, b);
      break;
    }
  }
}

void imProcessToneGamut(const imImage* src_image, imImage* dst_image, int op, float *args)
{
  int count = src_image->count*src_image->depth;

  switch(src_image->data_type)
  {
  case IM_BYTE:
    DoNormalizedUnaryOp((imbyte*)src_image->data[0], (imbyte*)dst_image->data[0], count, op, args);
    break;                                                                                
  case IM_USHORT:                                                                           
    DoNormalizedUnaryOp((imushort*)src_image->data[0], (imushort*)dst_image->data[0], count, op, args);
    break;                                                                                
  case IM_INT:                                                                           
    DoNormalizedUnaryOp((int*)src_image->data[0], (int*)dst_image->data[0], count, op, args);
    break;                                                                                
  case IM_FLOAT:                                                                           
    DoNormalizedUnaryOp((float*)src_image->data[0], (float*)dst_image->data[0], count, op, args);
    break;                                                                                
  }
}

void imProcessUnNormalize(const imImage* image, imImage* NewImage)
{
  int count = image->count*image->depth;

  float* map = (float*)image->data[0];
  imbyte* new_map = (imbyte*)NewImage->data[0];

  for (int i = 0; i < count; i++)
  {
    if (map[i] > 1)
      new_map[i] = (imbyte)255;
    else if (map[i] < 0)
      new_map[i] = (imbyte)0;
    else
      new_map[i] = (imbyte)(map[i]*255);
  }
}

template <class T> 
static void DoDirectConv(T* map, imbyte* new_map, int count)
{
  for (int i = 0; i < count; i++)
  {
    if (map[i] > 255)
      new_map[i] = (imbyte)255;
    else if (map[i] < 0)
      new_map[i] = (imbyte)0;
    else
      new_map[i] = (imbyte)(map[i]);
  }
}

void imProcessDirectConv(const imImage* image, imImage* NewImage)
{
  int count = image->count*image->depth;

  switch(image->data_type)
  {
  case IM_USHORT:                                                                           
    DoDirectConv((imushort*)image->data[0], (imbyte*)NewImage->data[0], count);
    break;                                                                                
  case IM_INT:                                                                           
    DoDirectConv((int*)image->data[0], (imbyte*)NewImage->data[0], count);
    break;                                                                                
  case IM_FLOAT:                                                                           
    DoDirectConv((float*)image->data[0], (imbyte*)NewImage->data[0], count);
    break;                                                                                
  }
}

void imProcessNegative(const imImage* src_image, imImage* dst_image)
{
  if (src_image->color_space == IM_MAP)
  {
    unsigned char r, g, b;
    for (int i = 0; i < src_image->palette_count; i++)
    {
      imColorDecode(&r, &g, &b, src_image->palette[i]);
      r = ~r; g = ~g; b = ~b;
      dst_image->palette[i] = imColorEncode(r, g, b);
    }

    imImageCopyData(src_image, dst_image);
  }
  else if (src_image->color_space == IM_BINARY)
  {
    imbyte* map1 = (imbyte*)src_image->data[0];
    imbyte* map = (imbyte*)dst_image->data[0];
    for (int i = 0; i < src_image->count; i++)
      map[i] = map1[i]? 0: 1;
  }
  else
    imProcessToneGamut(src_image, dst_image, IM_GAMUT_INVERT, NULL);
}
