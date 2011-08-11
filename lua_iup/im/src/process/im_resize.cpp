/** \file
 * \brief Image Resize
 *
 * See Copyright Notice in im_lib.h
 * $Id: im_resize.cpp,v 1.2 2010/01/07 19:12:53 scuri Exp $
 */


#include <im.h>
#include <im_util.h>
#include <im_math.h>
#include <im_complex.h>
#include <im_counter.h>

#include "im_process_loc.h"

#include <stdlib.h>
#include <memory.h>


static inline void iResizeInverse(int x, int y, float *xl, float *yl, float x_invfactor, float y_invfactor)
{
  *xl = (x + 0.5f) * x_invfactor;
  *yl = (y + 0.5f) * y_invfactor;
}

template <class DT, class DTU> 
static int iResize(int src_width, int src_height, const DT *src_map, 
                         int dst_width, int dst_height, DT *dst_map, 
                         DTU Dummy, int order, int counter)
{
  float xl, yl;
  float x_invfactor = float(src_width)/float(dst_width);
  float y_invfactor = float(src_height)/float(dst_height);

  for (int y = 0; y < dst_height; y++)
  {
    for (int x = 0; x < dst_width; x++)
    {
      iResizeInverse(x, y, &xl, &yl, x_invfactor, y_invfactor);
                   
      // if inside the original image
      if (xl > 0.0 && yl > 0.0 && xl < src_width && yl < src_height)
      {
        if (order == 1)
          *dst_map = imBilinearInterpolation(src_width, src_height, src_map, xl, yl);
        else if (order == 3)
          *dst_map = imBicubicInterpolation(src_width, src_height, src_map, xl, yl, Dummy);
        else
          *dst_map = imZeroOrderInterpolation(src_width, src_height, src_map, xl, yl);
      }

      dst_map++;
    }

    if (!imCounterInc(counter))
      return 0;
  }

  return 1;
}

template <class DT, class DTU> 
static int iReduce(int src_width, int src_height, const DT *src_map, 
                         int dst_width, int dst_height, DT *dst_map, 
                         DTU Dummy, int order, int counter)
{
  float xl, yl;
  float x_invfactor = float(src_width)/float(dst_width);
  float y_invfactor = float(src_height)/float(dst_height);

  iResizeInverse(1, 1, &xl, &yl, x_invfactor, y_invfactor);
  float xl0 = xl, yl0 = yl;
  iResizeInverse(2, 2, &xl, &yl, x_invfactor, y_invfactor);
  float xl1 = xl, yl1 = yl;
  
  float box_width = xl1 - xl0;
  float box_height = yl1 - yl0;

  for (int y = 0; y < dst_height; y++)
  {
    for (int x = 0; x < dst_width; x++)
    {
      iResizeInverse(x, y, &xl, &yl, x_invfactor, y_invfactor);
                   
      // if inside the original image
      if (xl > 0.0 && yl > 0.0 && xl < src_width && yl < src_height)
      {
        if (order == 0)
          *dst_map = imZeroOrderDecimation(src_width, src_height, src_map, xl, yl, box_width, box_height, Dummy);
        else
          *dst_map = imBilinearDecimation(src_width, src_height, src_map, xl, yl, box_width, box_height, Dummy);
      }

      dst_map++;
    }

    if (!imCounterInc(counter))
      return 0;
  }
  return 1;
}

int imProcessReduce(const imImage* src_image, imImage* dst_image, int order)
{
  int ret = 0;
  int counter = imCounterBegin("Reduce Size");
  const char* int_msg = (order == 1)? "Bilinear Decimation": "Zero Order Decimation";
  int src_depth = src_image->has_alpha? src_image->depth+1: src_image->depth;
  imCounterTotal(counter, src_depth*dst_image->height, int_msg);

  for (int i = 0; i < src_depth; i++)
  {
    switch(src_image->data_type)
    {
    case IM_BYTE:
      ret = iReduce(src_image->width, src_image->height, (const imbyte*)src_image->data[i],  
                    dst_image->width, dst_image->height, (imbyte*)dst_image->data[i], 
                    float(0), order, counter);
      break;
    case IM_USHORT:
      ret = iReduce(src_image->width, src_image->height, (const imushort*)src_image->data[i],  
                    dst_image->width, dst_image->height, (imushort*)dst_image->data[i], 
                    float(0), order, counter);
      break;
    case IM_INT:
      ret = iReduce(src_image->width, src_image->height, (const int*)src_image->data[i],  
                    dst_image->width, dst_image->height, (int*)dst_image->data[i], 
                    float(0), order, counter);
      break;
    case IM_FLOAT:
      ret = iReduce(src_image->width, src_image->height, (const float*)src_image->data[i],  
                    dst_image->width, dst_image->height, (float*)dst_image->data[i], 
                    float(0), order, counter);
      break;
    case IM_CFLOAT:
      ret = iReduce(src_image->width, src_image->height, (const imcfloat*)src_image->data[i],  
                    dst_image->width, dst_image->height, (imcfloat*)dst_image->data[i], 
                    imcfloat(0,0), order, counter);
      break;
    }
  }

  imCounterEnd(counter);
  return ret;
}

int imProcessResize(const imImage* src_image, imImage* dst_image, int order)
{
  int ret = 0;
  int counter = imCounterBegin("Resize");
  const char* int_msg = (order == 3)? "Bicubic Interpolation": (order == 1)? "Bilinear Interpolation": "Zero Order Interpolation";
  int src_depth = src_image->has_alpha? src_image->depth+1: src_image->depth;
  imCounterTotal(counter, src_depth*dst_image->height, int_msg);

  for (int i = 0; i < src_depth; i++)
  {
    switch(src_image->data_type)
    {
    case IM_BYTE:
      ret = iResize(src_image->width, src_image->height, (const imbyte*)src_image->data[i],  
                    dst_image->width, dst_image->height, (imbyte*)dst_image->data[i], 
                    float(0), order, counter);
      break;
    case IM_USHORT:
      ret = iResize(src_image->width, src_image->height, (const imushort*)src_image->data[i],  
                    dst_image->width, dst_image->height, (imushort*)dst_image->data[i], 
                    float(0), order, counter);
      break;
    case IM_INT:
      ret = iResize(src_image->width, src_image->height, (const int*)src_image->data[i],  
                    dst_image->width, dst_image->height, (int*)dst_image->data[i], 
                    float(0), order, counter);
      break;
    case IM_FLOAT:
      ret = iResize(src_image->width, src_image->height, (const float*)src_image->data[i],  
                    dst_image->width, dst_image->height, (float*)dst_image->data[i], 
                    float(0), order, counter);
      break;
    case IM_CFLOAT:
      ret = iResize(src_image->width, src_image->height, (const imcfloat*)src_image->data[i],  
                    dst_image->width, dst_image->height, (imcfloat*)dst_image->data[i], 
                    imcfloat(0,0), order, counter);
      break;
    }
  }

  imCounterEnd(counter);
  return ret;
}

template <class DT> 
static void ReduceBy4(int src_width, 
                      int src_height, 
                      DT *src_map, 
                      int dst_width,
                      int dst_height,
                      DT *dst_map)
{
  int x,y,yd,xd;
  (void)dst_height;

  // make an even size
  int height = (src_height/2)*2;
  int width = (src_width/2)*2;

  for(y = 0 ; y < height ; y += 2)
  {
    yd = y/2;
    for(x = 0 ; x < width ; x += 2)
    {
      xd = x/2;
      dst_map[yd * dst_width + xd] = ((src_map[y * src_width + x] + 
                                       src_map[y * src_width + (x+1)] +
                                       src_map[(y+1) * src_width + x] +
                                       src_map[(y+1) * src_width + (x+1)])/4);
    }        
  }
}

void imProcessReduceBy4(const imImage* src_image, imImage* dst_image)
{
  int i;
  int src_depth = src_image->has_alpha? src_image->depth+1: src_image->depth;

  for (i = 0; i < src_depth; i++)
  {
    switch(src_image->data_type)
    {
    case IM_BYTE:
      ReduceBy4(src_image->width, src_image->height, (imbyte*)src_image->data[i],  dst_image->width, dst_image->height, (imbyte*)dst_image->data[i]);
      break;
    case IM_USHORT:
      ReduceBy4(src_image->width, src_image->height, (imushort*)src_image->data[i],  dst_image->width, dst_image->height, (imushort*)dst_image->data[i]);
      break;
    case IM_INT:
      ReduceBy4(src_image->width, src_image->height, (int*)src_image->data[i],  dst_image->width, dst_image->height, (int*)dst_image->data[i]);
      break;
    case IM_FLOAT:
      ReduceBy4(src_image->width, src_image->height, (float*)src_image->data[i],  dst_image->width, dst_image->height, (float*)dst_image->data[i]);
      break;
    case IM_CFLOAT:
      ReduceBy4(src_image->width, src_image->height, (imcfloat*)src_image->data[i],  dst_image->width, dst_image->height, (imcfloat*)dst_image->data[i]);
      break;
    }
  }
}

void imProcessCrop(const imImage* src_image, imImage* dst_image, int xmin, int ymin)
{
  int type_size = imDataTypeSize(src_image->data_type);
  int src_depth = src_image->has_alpha? src_image->depth+1: src_image->depth;
  for (int i = 0; i < src_depth; i++)
  {
    imbyte *src_map = (imbyte*)src_image->data[i];
    imbyte *dst_map = (imbyte*)dst_image->data[i];

    for (int y = 0; y < dst_image->height; y++)
    {
      int src_offset = (y + ymin)*src_image->line_size + xmin*type_size;
      int dst_offset = y*dst_image->line_size;

      memcpy(&dst_map[dst_offset], &src_map[src_offset], dst_image->line_size);
    }
  }
}

void imProcessInsert(const imImage* src_image, const imImage* rgn_image, imImage* dst_image, int xmin, int ymin)
{
  int type_size = imDataTypeSize(src_image->data_type);
  int dst_size1 = xmin*type_size;
  int dst_size2 = src_image->line_size - (rgn_image->line_size + dst_size1);
  int dst_offset2 = dst_size1+rgn_image->line_size;
  int ymax = ymin+rgn_image->height-1;
  int rgn_size = rgn_image->line_size;
  int src_depth = src_image->has_alpha? src_image->depth+1: src_image->depth;

  if (dst_size2 < 0)
  {
    dst_size2 = 0;
    rgn_size = src_image->line_size - dst_size1;
    dst_offset2 = dst_size1+rgn_size;
  }

  if (ymax > src_image->height-1)
    ymax = src_image->height-1;

  for (int i = 0; i < src_depth; i++)
  {
    imbyte *src_map = (imbyte*)src_image->data[i];
    imbyte *rgn_map = (imbyte*)rgn_image->data[i];
    imbyte *dst_map = (imbyte*)dst_image->data[i];

    for (int y = 0; y < src_image->height; y++)
    {
      if (y < ymin || y > ymax)
      {
        memcpy(dst_map, src_map, src_image->line_size);
      }
      else
      {
        if (dst_size1)
          memcpy(dst_map, src_map, dst_size1);

        memcpy(dst_map + dst_size1, rgn_map, rgn_size);

        if (dst_size2)
          memcpy(dst_map + dst_offset2, 
                 src_map + dst_offset2, dst_size2);

        rgn_map += rgn_image->line_size;
      }

      src_map += src_image->line_size;
      dst_map += dst_image->line_size;
    }
  }
}

void imProcessAddMargins(const imImage* src_image, imImage* dst_image, int xmin, int ymin)
{
  int type_size = imDataTypeSize(src_image->data_type);
  int src_depth = src_image->has_alpha? src_image->depth+1: src_image->depth;
  for (int i = 0; i < src_depth; i++)
  {
    imbyte *dst_map = (imbyte*)dst_image->data[i];
    imbyte *src_map = (imbyte*)src_image->data[i];

    for (int y = 0; y < src_image->height; y++)
    {
      int src_offset = y*src_image->line_size;
      int dst_offset = (y + ymin)*dst_image->line_size + xmin*type_size;

      memcpy(&dst_map[dst_offset], &src_map[src_offset], src_image->line_size);
    }
  }
}

