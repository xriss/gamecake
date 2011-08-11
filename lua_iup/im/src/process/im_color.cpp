/** \file
 * \brief Color Processing Operations
 *
 * See Copyright Notice in im_lib.h
 * $Id: im_color.cpp,v 1.2 2010/01/06 20:16:30 scuri Exp $
 */

#include <im.h>
#include <im_util.h>
#include <im_colorhsi.h>
#include <im_palette.h>

#include "im_process_pon.h"

#include <stdlib.h>
#include <memory.h>


static void rgb2yrgb(imbyte* r, imbyte* g, imbyte* b, imbyte* y)
{
  int ri,gi,bi;

  *y = (imbyte)((299*(*r) + 587*(*g) + 114*(*b)) / 1000);
  ri = (*r) - (*y) + 128;
  gi = (*g) - (*y) + 128;
  bi = (*b) - (*y) + 128;

  if (ri < 0) ri = 0;
  if (gi < 0) gi = 0;
  if (bi < 0) bi = 0;

  *r = (imbyte)ri;
  *g = (imbyte)gi;
  *b = (imbyte)bi;
}

void imProcessSplitYChroma(const imImage* src_image, imImage* y_image, imImage* chroma_image)
{
  imbyte Y,
    *red=(imbyte*)src_image->data[0],
    *green=(imbyte*)src_image->data[1],
    *blue=(imbyte*)src_image->data[2],
    *red2=(imbyte*)chroma_image->data[0],
    *green2=(imbyte*)chroma_image->data[1],
    *blue2=(imbyte*)chroma_image->data[2],
    *map1=(imbyte*)y_image->data[0];

  for (int i = 0; i < src_image->count; i++)
  {
    imbyte R = red[i];
    imbyte G = green[i];
    imbyte B = blue[i];

    rgb2yrgb(&R, &G, &B, &Y);

    map1[i] = Y;

    red2[i] = R;
    green2[i] = G;
    blue2[i] = B;
  }
}

static void DoSplitHSIFloat(float** data, float* hue, float* saturation, float* intensity, int count)
{
  float *red=data[0],
      *green=data[1],
       *blue=data[2];

  for (int i = 0; i < count; i++)
  {
    imColorRGB2HSI(red[i], green[i], blue[i], &hue[i], &saturation[i], &intensity[i]);
  }
}

static void DoSplitHSIByte(imbyte** data, float* hue, float* saturation, float* intensity, int count)
{
  imbyte *red=data[0],
       *green=data[1],
        *blue=data[2];

  for (int i = 0; i < count; i++)
  {
    imColorRGB2HSIbyte(red[i], green[i], blue[i], &hue[i], &saturation[i], &intensity[i]);
  }
}

void imProcessSplitHSI(const imImage* image, imImage* image1, imImage* image2, imImage* image3)
{
  switch(image->data_type)
  {
  case IM_BYTE:
    DoSplitHSIByte((imbyte**)image->data, (float*)image1->data[0], (float*)image2->data[0], (float*)image3->data[0], image->count);
    break;                                                                                                                                    
  case IM_FLOAT:                                                                                                                               
    DoSplitHSIFloat((float**)image->data, (float*)image1->data[0], (float*)image2->data[0], (float*)image3->data[0], image->count);
    break;                                                                                
  }

  imImageSetPalette(image1, imPaletteHues(), 256);
}

static void DoMergeHSIFloat(float** data, float* hue, float* saturation, float* intensity, int count)
{
  float *red=data[0],
      *green=data[1],
       *blue=data[2];

  for (int i = 0; i < count; i++)
  {
    imColorHSI2RGB(hue[i], saturation[i], intensity[i], &red[i], &green[i], &blue[i]);
  }
}

static void DoMergeHSIByte(imbyte** data, float* hue, float* saturation, float* intensity, int count)
{
  imbyte *red=data[0],
       *green=data[1],
        *blue=data[2];

  for (int i = 0; i < count; i++)
  {
    imColorHSI2RGBbyte(hue[i], saturation[i], intensity[i], &red[i], &green[i], &blue[i]);
  }
}

void imProcessMergeHSI(const imImage* image1, const imImage* image2, const imImage* image3, imImage* image)
{
  switch(image->data_type)
  {
  case IM_BYTE:
    DoMergeHSIByte((imbyte**)image->data, (float*)image1->data[0], (float*)image2->data[0], (float*)image3->data[0], image->count);
    break;                                                                                                                                    
  case IM_FLOAT:                                                                                                                               
    DoMergeHSIFloat((float**)image->data, (float*)image1->data[0], (float*)image2->data[0], (float*)image3->data[0], image->count);
    break;                                                                                
  }
}

void imProcessSplitComponents(const imImage* src_image, imImage** dst_image)
{
  memcpy(dst_image[0]->data[0], src_image->data[0], src_image->plane_size);
  memcpy(dst_image[1]->data[0], src_image->data[1], src_image->plane_size);
  memcpy(dst_image[2]->data[0], src_image->data[2], src_image->plane_size);
  if (imColorModeDepth(src_image->color_space) == 4 || src_image->has_alpha) 
    memcpy(dst_image[3]->data[0], src_image->data[3], src_image->plane_size);
}

void imProcessMergeComponents(const imImage** src_image, imImage* dst_image)
{
  memcpy(dst_image->data[0], src_image[0]->data[0], dst_image->plane_size);
  memcpy(dst_image->data[1], src_image[1]->data[0], dst_image->plane_size);
  memcpy(dst_image->data[2], src_image[2]->data[0], dst_image->plane_size);
  if (imColorModeDepth(dst_image->color_space) == 4 || dst_image->has_alpha) 
    memcpy(dst_image->data[3], src_image[3]->data[0], dst_image->plane_size);
}

template <class T> 
static void DoNormalizeComp(T** src_data, float** dst_data, int count, int depth)
{
  int d;
  T* src_pdata[4];
  float* dst_pdata[4];

  for(d = 0; d < depth; d++)
  {
    dst_pdata[d] = dst_data[d];
    src_pdata[d] = src_data[d];
  }

  for (int i = 0; i < count; i++)
  {
    float sum = 0;
    for(d = 0; d < depth; d++)
      sum += (float)*(src_pdata[d]);

    for(d = 0; d < depth; d++)
    {
      if (sum == 0)
        *(dst_pdata[d]) = 0;
      else
        *(dst_pdata[d]) = (float)*(src_pdata[d]) / sum;

      dst_pdata[d]++;
      src_pdata[d]++;
    }
  }
}

void imProcessNormalizeComponents(const imImage* src_image, imImage* dst_image)
{
  switch(src_image->data_type)
  {
  case IM_BYTE:
    DoNormalizeComp((imbyte**)src_image->data, (float**)dst_image->data, src_image->count, src_image->depth);
    break;                                                                                                                                    
  case IM_USHORT:                                                                                                                               
    DoNormalizeComp((imushort**)src_image->data,  (float**)dst_image->data, src_image->count, src_image->depth);
    break;                                                                                                                                    
  case IM_INT:                                                                                                                               
    DoNormalizeComp((int**)src_image->data,  (float**)dst_image->data, src_image->count, src_image->depth);
    break;                                                                                                                                    
  case IM_FLOAT:                                                                                                                               
    DoNormalizeComp((float**)src_image->data, (float**)dst_image->data, src_image->count, src_image->depth);
    break;                                                                                
  }
}

template <class T> 
static void DoReplaceColor(T *src_data, T *dst_data, int width, int height, int depth, float* src_color, float* dst_color)
{
  int d, count = width*height;
  for (int i = 0; i < count; i++)
  {
    int equal = 1;
    for (d = 0; d < depth; d++)
    {
      if (*(src_data+d*count) != (T)src_color[d])
      {
        equal = 0;
        break;
      }
    }

    for (d = 0; d < depth; d++)
    {
      if (equal)
        *(dst_data+d*count) = (T)dst_color[d];
      else
        *(dst_data+d*count) = *(src_data+d*count);
    }

    src_data++;
    dst_data++;
  }
}

void imProcessReplaceColor(const imImage* src_image, imImage* dst_image, float* src_color, float* dst_color)
{
  switch(src_image->data_type)
  {
  case IM_BYTE:
    DoReplaceColor((imbyte*)src_image->data[0],   (imbyte*)dst_image->data[0],   src_image->width, src_image->height, src_image->depth, src_color, dst_color);
    break;                                                                                                           
  case IM_USHORT:                                                                                                      
    DoReplaceColor((imushort*)src_image->data[0], (imushort*)dst_image->data[0], src_image->width, src_image->height, src_image->depth, src_color, dst_color);
    break;                                                                                                           
  case IM_INT:                                                                                                      
    DoReplaceColor((int*)src_image->data[0],      (int*)dst_image->data[0],      src_image->width, src_image->height, src_image->depth, src_color, dst_color);
    break;                                                                                                           
  case IM_FLOAT:                                                                                                      
    DoReplaceColor((float*)src_image->data[0],    (float*)dst_image->data[0],    src_image->width, src_image->height, src_image->depth, src_color, dst_color);
    break;                                                                                
  }
}
