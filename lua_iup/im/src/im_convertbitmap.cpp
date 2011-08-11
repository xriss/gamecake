/** \file
 * \brief Image Conversion
 *
 * See Copyright Notice in im_lib.h
 * $Id: im_convertbitmap.cpp,v 1.4 2010/10/25 18:29:07 scuri Exp $
 */

#include "im.h"
#include "im_util.h"
#include "im_complex.h"
#include "im_image.h"
#include "im_convert.h"
#include "im_counter.h"

#include <stdlib.h>
#include <assert.h>
#include <memory.h>

int imConvertToBitmap(const imImage* src_image, imImage* dst_image, int cpx2real, float gamma, int abssolute, int cast_mode)
{
  assert(src_image);
  assert(dst_image);

  if (!imImageMatchSize(src_image, dst_image) || !imImageIsBitmap(dst_image))
    return IM_ERR_DATA;

  int counter = imCounterBegin("Building Bitmap");

  int ret;
  if (src_image->data_type == IM_BYTE)
  {
    // NO data type conversion, only color mode conversion
    ret = imConvertColorSpace(src_image, dst_image);
  }
  else
  {
    if (src_image->color_space == IM_RGB || 
        src_image->color_space == IM_GRAY)
    {
      // data type conversion, but NO color mode conversion
      ret = imConvertDataType(src_image, dst_image, cpx2real, gamma, abssolute, cast_mode);
    }
    else
    {
      // data type conversion AND color mode conversion
      imImage* temp_image = imImageCreate(src_image->width, src_image->height, dst_image->color_space, src_image->data_type);
      if (!temp_image)
        ret = IM_ERR_MEM;
      else
      {
        // first convert color_mode in the bigger precision
        ret = imConvertColorSpace(src_image, temp_image);
        if (ret == IM_ERR_NONE)
        {
          // second just convert data type
          ret = imConvertDataType(temp_image, dst_image, cpx2real, gamma, abssolute, cast_mode);
        }
        imImageDestroy(temp_image);
      }
    }
  }

  imCounterEnd(counter);
  return ret;
}


template <class T>
void iDoChangePacking(const T* src_data, T* dst_data, int width, int height, int src_depth, int dst_depth,
                             int src_is_packed)
{
  int count = width*height;
  if (src_is_packed)
  {
    for (int i = 0; i < count; i++)
    {
      for (int d = 0; d < dst_depth; d++)
      {
        *(dst_data + d*count) = *(src_data + d);
      }

      src_data += src_depth;
      dst_data++;
    }
  }
  else
  {
    for (int i = 0; i < count; i++)
    {
      for (int d = 0; d < src_depth; d++)
      {
        *(dst_data + d) = *(src_data + d*count);
      }

      dst_data += dst_depth;
      src_data++;
    }
  }
}

void imConvertPacking(const void* src_data, void* dst_data, int width, int height, int src_depth, int dst_depth, 
                      int data_type, int src_is_packed)
{
  switch(data_type)
  {
  case IM_BYTE:
    iDoChangePacking((const imbyte*)src_data, (imbyte*)dst_data, width, height, src_depth, dst_depth, src_is_packed); 
    break;
  case IM_USHORT:
    iDoChangePacking((const imushort*)src_data, (imushort*)dst_data, width, height, src_depth, dst_depth, src_is_packed); 
    break;
  case IM_INT:
    iDoChangePacking((const int*)src_data, (int*)dst_data, width, height, src_depth, dst_depth, src_is_packed); 
    break;
  case IM_FLOAT:
    iDoChangePacking((const float*)src_data, (float*)dst_data, width, height, src_depth, dst_depth, src_is_packed); 
    break;
  case IM_CFLOAT:
    iDoChangePacking((const imcfloat*)src_data, (imcfloat*)dst_data, width, height, src_depth, dst_depth, src_is_packed); 
    break;
  }
}

static void iImageMakeGray(imbyte *map, int gldepth, int count)
{
  for(int i = 0; i < count; i++)
  {
    if (*map)
      *map = 255;
    map += gldepth;
  }
}

static void iImageGLCopyMapAlpha(imbyte *map, imbyte *gldata, int gldepth, int count)
{
  /* gldata can be GL_RGBA or GL_LUMINANCE_ALPHA */
  gldata += gldepth-1; /* position at first alpha */
  for(int i = 0; i < count; i++)
  {
    *gldata = *map;
    map++;
    gldata += gldepth;  /* skip to next alpha */
  }
}

static void iImageGLSetTranspColor(imbyte *gldata, int count, imbyte r, imbyte g, imbyte b)
{
  /* gldata is GL_RGBA */
  for(int i = 0; i < count; i++)
  {
    if (*(gldata+0) == r &&
        *(gldata+1) == g &&
        *(gldata+2) == b)
      *(gldata+3) = 0;    /* transparent */
    else
      *(gldata+3) = 255;  /* opaque */
    gldata += 4;
  }
}

static void iImageGLSetTranspMap(imbyte *map, imbyte *gldata, int count, imbyte *transp_map, int transp_count)
{
  /* gldata is GL_RGBA */
  gldata += 3; /* position at first alpha */
  for(int i = 0; i < count; i++)
  {
    if (*map < transp_count)
      *gldata = transp_map[*map];
    else
      *gldata = 255;  /* opaque */

    map++;
    gldata += 4;
  }
}

static void iImageGLSetTranspIndex(imbyte *map, imbyte *gldata, int gldepth, int count, imbyte index)
{
  /* gldata can be GL_RGBA or GL_LUMINANCE_ALPHA */
  gldata += gldepth-1; /* position at first alpha */
  for(int i = 0; i < count; i++)
  {
    if (*map == index)
      *gldata = 0;    /* full transparent */
    else
      *gldata = 255;  /* opaque */

    map++;
    gldata += gldepth;  /* skip to next alpha */
  }
}

/* To avoid including gl.h */
#define GL_RGB                            0x1907
#define GL_RGBA                           0x1908
#define GL_LUMINANCE                      0x1909
#define GL_LUMINANCE_ALPHA                0x190A

imImage* imImageCreateFromOpenGLData(int width, int height, int glformat, const void* gldata)
{
  int color_space, has_alpha, depth;
  imImage* image;

  switch(glformat)
  {
  case GL_RGBA:
    color_space = IM_RGB;
    has_alpha = 1;
    depth = 4;
    break;
  case GL_RGB:
    color_space = IM_RGB;
    has_alpha = 0;
    depth = 3;
    break;
  case GL_LUMINANCE_ALPHA:
    color_space = IM_GRAY;
    depth = 2;
    has_alpha = 1;
  default: /* GL_LUMINANCE */
    color_space = IM_GRAY;
    depth = 1;
    has_alpha = 0;
    break;
  }

  image = imImageCreate(width, height, color_space, IM_BYTE);
  if (!image)
    return NULL;

  if (has_alpha)
    imImageAddAlpha(image);

  imConvertPacking(gldata, image->data[0], image->width, image->height, depth, depth, IM_BYTE, 1);

  return image;
}

void* imImageGetOpenGLData(const imImage* image, int *format)
{
  if (!imImageIsBitmap(image))
    return NULL;

  int transp_count;
  imbyte* transp_index = (imbyte*)imImageGetAttribute(image, "TransparencyIndex", NULL, NULL);
  imbyte* transp_map = (imbyte*)imImageGetAttribute(image, "TransparencyMap", NULL, &transp_count);
  imbyte* transp_color = (imbyte*)imImageGetAttribute(image, "TransparencyColor", NULL, NULL);

  int glformat;
  switch(image->color_space)
  {
  case IM_MAP:
    if (image->has_alpha || transp_map || transp_index)
      glformat = GL_RGBA;
    else
      glformat = GL_RGB;
    break;
  case IM_RGB:
    if (image->has_alpha || transp_color)
      glformat = GL_RGBA;
    else
      glformat = GL_RGB;
    break;
  case IM_BINARY:
  default: /* IM_GRAY */
    if (image->has_alpha || transp_map || transp_index)
      glformat = GL_LUMINANCE_ALPHA;
    else
      glformat = GL_LUMINANCE;
    break;
  }

  int gldepth;
  switch (glformat)
  {
  case GL_RGB:
    gldepth = 3;
    break;
  case GL_RGBA:
    gldepth = 4;
    break;
  case GL_LUMINANCE_ALPHA:
    gldepth = 2;
    break;
  default: /* GL_LUMINANCE */
    gldepth = 1;
    break;
  }

  int size = image->count*gldepth;
  imImageSetAttribute(image, "GLDATA", IM_BYTE, size, NULL);
  imbyte* gldata = (imbyte*)imImageGetAttribute(image, "GLDATA", NULL, NULL);

  int depth = image->depth;
  if (image->has_alpha)
    depth++;

  /* copy data, including alpha */
  if (image->color_space != IM_MAP)
  {
    imConvertPacking(image->data[0], gldata, image->width, image->height, depth, gldepth, IM_BYTE, 0);

    if (image->color_space == IM_BINARY)
      iImageMakeGray(gldata, gldepth, image->count);
  }
  else
  {
    /* copy map data */
    memcpy(gldata, image->data[0], image->size);  /* size does not include alpha */

    /* expand MAP to RGB or RGBA */
    imConvertMapToRGB(gldata, image->count, gldepth, 1, image->palette, image->palette_count);

    if (image->has_alpha)
      iImageGLCopyMapAlpha((imbyte*)image->data[1], gldata, gldepth, image->count);  /* copy the alpha plane */
  }

  /* set alpha based on attributes */
  if (!image->has_alpha)
  {
    if (image->color_space == IM_RGB)
    {
      if (transp_color)
        iImageGLSetTranspColor(gldata, image->count, *(transp_color+0), *(transp_color+1), *(transp_color+2));
    }
    else 
    {
      if (transp_map)
        iImageGLSetTranspMap((imbyte*)image->data[0], gldata, image->count, transp_map, transp_count);
      else if (transp_index)
        iImageGLSetTranspIndex((imbyte*)image->data[0], gldata, gldepth, image->count, *transp_index);
    }
  }

  if (format) *format = glformat;
  return gldata;
}

