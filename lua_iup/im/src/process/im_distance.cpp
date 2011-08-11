/** \file
 * \brief Distance Transform
 *
 * See Copyright Notice in im_lib.h
 * $Id: im_distance.cpp,v 1.1 2008/10/17 06:16:33 scuri Exp $
 */

#include <im.h>
#include <im_util.h>

#include "im_process_glo.h"

#include <stdlib.h>
#include <memory.h>
#include <math.h>

const float DT_ONE    = 1.0f;             // 1x0
const float DT_SQRT2  = 1.414213562373f;  // 1x1
const float DT_SQRT5  = 2.2360679775f;    // 2x1
const float DT_SQRT10 = 3.1622776601684f; // 3x1
const float DT_SQRT13 = 3.605551275464f;  // 3x2
const float DT_SQRT17 = 4.12310562562f;   // 4x1
const float DT_SQRT25 = 5.0f;             // 4x3

static inline void setValue(int r, int r1, int r2, int r3, int r4, float *image_data, int f) 
{
  float v;
  float minv = image_data[r];        // (x,y)

  if (f)
    v = image_data[r - 1] + DT_ONE;      // (x-1,y)
  else
    v = image_data[r + 1] + DT_ONE;      // (x+1,y)
  if (v < minv) minv = v;

  v = image_data[r1] + DT_ONE;          // (x,y-1)           (x,y+1)
  if (v < minv) minv = v; 

  if (minv < DT_SQRT2)
    goto min_attrib;

  v = image_data[r1 - 1] + DT_SQRT2;    // (x-1,y-1)         (x-1,y+1)
  if (v < minv) minv = v;                                      
                                                             
  v = image_data[r1 + 1] + DT_SQRT2;    // (x+1,y-1)         (x+1,y+1)
  if (v < minv) minv = v;                                      

  if (minv < DT_SQRT5)
    goto min_attrib;

  v = image_data[r1 + 2] + DT_SQRT5;    // (x+2,y-1)         (x+2,y+1)
  if (v < minv) minv = v;                                      
                                                             
  v = image_data[r1 - 2] + DT_SQRT5;    // (x-2,y-1)         (x-2,y+1)
  if (v < minv) minv = v;                                      
                                                             
  v = image_data[r2 - 1] + DT_SQRT5;    // (x-1,y-2)         (x-1,y+2)
  if (v < minv) minv = v;                                      
                                                             
  v = image_data[r2 + 1] + DT_SQRT5;    // (x+1,y-2)         (x+1,y+2)
  if (v < minv) minv = v;                                      

  if (minv < DT_SQRT10)
    goto min_attrib;
                                                             
  v = image_data[r1 + 3] + DT_SQRT10;   // (x+3,y-1)         (x+3,y+1)
  if (v < minv) minv = v;                                      
                                                             
  v = image_data[r1 - 3] + DT_SQRT10;   // (x-3,y-1)         (x-3,y+1)
  if (v < minv) minv = v;                                      

  v = image_data[r3 - 1] + DT_SQRT10;   // (x-1,y-3)         (x-1,y+3)
  if (v < minv) minv = v;                                      
                                                             
  v = image_data[r3 + 1] + DT_SQRT10;   // (x+1,y-3)         (x+1,y+3)
  if (v < minv) minv = v;                                      

  if (minv < DT_SQRT13)
    goto min_attrib;

  v = image_data[r2 - 3] + DT_SQRT13;   // (x-3,y-2)         (x-3,y+2)
  if (v < minv) minv = v;                                      
                                                             
  v = image_data[r2 + 3] + DT_SQRT13;   // (x+3,y-2)         (x+3,y+2)
  if (v < minv) minv = v;                                      
                                                             
  v = image_data[r3 + 2] + DT_SQRT13;   // (x+2,y-3)         (x+2,y+3)
  if (v < minv) minv = v;                                      
                                                             
  v = image_data[r3 - 2] + DT_SQRT13;   // (x-2,y-3)         (x-2,y+3)
  if (v < minv) minv = v;

  if (minv < DT_SQRT17)
    goto min_attrib;
                                                             
  v = image_data[r1 + 4] + DT_SQRT17;   // (x+4,y-1)         (x+4,y+1)
  if (v < minv) minv = v;                                      
                                                             
  v = image_data[r1 - 4] + DT_SQRT17;   // (x-4,y-1)         (x-4,y+1)
  if (v < minv) minv = v;                                      

  v = image_data[r4 - 1] + DT_SQRT17;   // (x-1,y-4)         (x-1,y+4)
  if (v < minv) minv = v;                                      
                                                             
  v = image_data[r4 + 1] + DT_SQRT17;   // (x+1,y-4)         (x+1,y+4)
  if (v < minv) minv = v;                                      

  if (minv < DT_SQRT25)
    goto min_attrib;

  v = image_data[r3 - 4] + DT_SQRT25;   // (x-4,y-3)         (x-4,y+3)
  if (v < minv) minv = v;                                      
                                                             
  v = image_data[r3 + 4] + DT_SQRT25;   // (x+4,y-3)         (x+4,y+3)
  if (v < minv) minv = v;                                      
                                                             
  v = image_data[r4 + 3] + DT_SQRT25;   // (x+3,y-4)         (x+3,y+4)
  if (v < minv) minv = v;                                      
                                                             
  v = image_data[r4 - 3] + DT_SQRT25;   // (x-3,y-4)         (x-3,y+4)
  if (v < minv) minv = v;

min_attrib:
  image_data[r] = minv;
}

static inline void setValueForwardEdge(int r, int r1, int r2, int width, int x, int y, float *image_data) 
{
  float v;
  float minv = image_data[r];        // (x,y)

  if (y > 0)
  {
    v = image_data[r1] + DT_ONE;         // (x,y-1)
    if (v < minv) minv = v;
  }

  if (x > 0)
  {
    v = image_data[r - 1] + DT_ONE;      // (x-1,y)
    if (v < minv) minv = v;
  }

  if (x > 0 && y > 0)
  {
    v = image_data[r1 - 1] + DT_SQRT2;   // (x-1,y-1)
    if (v < minv) minv = v;
  }

  if (x < width-2 && y > 0)
  {
    v = image_data[r1 + 1] + DT_SQRT2;   // (x+1,y-1)
    if (v < minv) minv = v;
  }

  if (x > 0 && y > 1)
  {
    v = image_data[r2 - 1] + DT_SQRT5;   // (x-1,y-2)
    if (v < minv) minv = v;
  }

  if (x < width-2 && y > 1)
  {
    v = image_data[r2 + 1] + DT_SQRT5;   // (x+1,y-2)
    if (v < minv) minv = v;
  }

  if (x < width-3 && y > 0)
  {
    v = image_data[r1 + 2] + DT_SQRT5;   // (x+2,y-1)
    if (v < minv) minv = v;
  }

  if (x > 1 && y > 0)
  {
    v = image_data[r1 - 2] + DT_SQRT5;   // (x-2,y-1)
    if (v < minv) minv = v;
  }

  image_data[r] = minv;
}

static inline void setValueBackwardEdge(int r, int r1, int r2, int width, int height, int x, int y, float *image_data) 
{
  float  v;
  float minv = image_data[r];        // (x,y)

  if (x < width-2)
  {
    v = image_data[r + 1] + DT_ONE;      // (x+1,y)
    if (v < minv) minv = v;
  }

  if (y < height-2)
  {
    v = image_data[r1] + DT_ONE;         // (x,y+1)
    if (v < minv) minv = v;
  }

  if (y < height-2 && x > 0)
  {
    v = image_data[r1 - 1] + DT_SQRT2;   // (x-1,y+1)
    if (v < minv) minv = v;
  }

  if (y < height-2 && x < width-2)
  {
    v = image_data[r1 + 1] + DT_SQRT2;   // (x+1,y+1)
    if (v < minv) minv = v;
  }

  if (y < height-2 && x < width-3)
  {
    v = image_data[r1 + 2] + DT_SQRT5;   // (x+2,y+1)
    if (v < minv) minv = v;
  }

  if (y < height-3 && x < width-2)
  {
    v = image_data[r2 + 1] + DT_SQRT5;   // (x+1,y+2)
    if (v < minv) minv = v;
  }

  if (y < height-3 && x > 0)
  {
    v = image_data[r2 - 1] + DT_SQRT5;   // (x-1,y+2)
    if (v < minv) minv = v;
  }

  if (y < height-2 && x > 1)
  {
    v = image_data[r1 - 2] + DT_SQRT5;   // (x-2,y+1)
    if (v < minv) minv = v;
  }

  image_data[r] = minv;
}

void imProcessDistanceTransform(const imImage* src_image, imImage* dst_image)
{
  int i, x, y, 
    offset, offset1, offset2, offset3, offset4,
    width = src_image->width,
    height = src_image->height;

  imbyte* src_data = (imbyte*)src_image->data[0];
  float* dst_data = (float*)dst_image->data[0];

  float max_dist = (float)sqrt(double(width*width + height*height));

  for (i = 0; i < src_image->count; i++)
  {
    // if pixel is background, then distance is zero.
    if (src_data[i])
      dst_data[i] = max_dist;
  }

  /* down->top, left->right */
  for (y = 0; y < height; y++) 
  {
    offset = y * width;
    offset1 = offset - width;
    offset2 = offset - 2*width;
    offset3 = offset - 3*width;
    offset4 = offset - 4*width;

    for (x = 0; x < width; x++) 
    {
      if (src_data[offset])
      {
        if (x < 4 || x > width-5 || y < 4 || y > height-5)
          setValueForwardEdge(offset, offset1, offset2, width, x, y, dst_data);
        else
          setValue(offset, offset1, offset2, offset3, offset4, dst_data, 1);
      }

      offset++;
      offset1++;
      offset2++;
      offset3++;
      offset4++;
    }
  }

  /* top->down, right->left */
  for (y = height-1; y >= 0; y--) 
  {
    offset = y * width + width-1;
    offset1 = offset + width;
    offset2 = offset + 2*width;
    offset3 = offset + 3*width;
    offset4 = offset + 4*width;

    for (x = width-1; x >= 0; x--) 
    {
      if (src_data[offset]) 
      {
        if (x < 4 || x > width-5 || y < 4 || y > height-5)
          setValueBackwardEdge(offset, offset1, offset2, width, height, x, y, dst_data);
        else
          setValue(offset, offset1, offset2, offset3, offset4, dst_data, 0);
      }

      offset--;
      offset1--;
      offset2--;
      offset3--;
      offset4--;
    }
  }
}

static void iFillValue(imbyte* img_data, int x, int y, int width, int value)
{
  int r = y * width + x;
  int r1a = r - width;
  int r1b = r + width;
  int v;

  int old_value = img_data[r];
  img_data[r] = (imbyte)value;

  v = img_data[r1a];        // (x,y-1)
  if (v == old_value) 
    iFillValue(img_data, x, y-1, width, value);

  v = img_data[r - 1];      // (x-1,y)
  if (v == old_value) 
    iFillValue(img_data, x-1, y, width, value);

  v = img_data[r1a - 1];    // (x-1,y-1)
  if (v == old_value) 
    iFillValue(img_data, x-1, y-1, width, value);

  v = img_data[r1a + 1];    // (x+1,y-1)
  if (v == old_value) 
    iFillValue(img_data, x+1, y-1, width, value);

  v = img_data[r + 1];      // (x+1,y)
  if (v == old_value) 
    iFillValue(img_data, x+1, y, width, value);

  v = img_data[r1b];        // (x,y+1)
  if (v == old_value) 
    iFillValue(img_data, x, y+1, width, value);

  v = img_data[r1b - 1];    // (x-1,y+1)
  if (v == old_value) 
    iFillValue(img_data, x-1, y+1, width, value);

  v = img_data[r1b + 1];    // (x+1,y+1)
  if (v == old_value) 
    iFillValue(img_data, x+1, y+1, width, value);
}

static inline int iCheckFalseMaximum(int r, int r2a, int r2b, int width, float *src_data) 
{
  /* we are ignoring valeys of 1 pixel. */
  /* this is not 100% fail proof */
  float v;
  float maxv = src_data[r];  // (x,y)
  int r1a = r - width;
  int r1b = r + width;

  v = src_data[r2a - 1];    // (x-1,y-2)
  if (v > maxv) return 1;

  v = src_data[r2a];        // (x,y-2)
  if (v > maxv) return 1;

  v = src_data[r2a + 1];    // (x+1,y-2)
  if (v > maxv) return 1;

  v = src_data[r2b - 1];    // (x-1,y+2)
  if (v > maxv) return 1;

  v = src_data[r2b];        // (x,y+2)
  if (v > maxv) return 1;

  v = src_data[r2b + 1];    // (x+1,y+2)
  if (v > maxv) return 1;


  v = src_data[r2b - 2];    // (x-2,y+2)
  if (v > maxv) return 1;

  v = src_data[r1b - 2];    // (x-2,y+1)
  if (v > maxv) return 1;

  v = src_data[r - 2];      // (x-2,y)
  if (v > maxv) return 1;

  v = src_data[r1a - 2];    // (x-2,y-1)
  if (v > maxv) return 1;

  v = src_data[r2a - 2];    // (x-2,y-2)
  if (v > maxv) return 1;


  v = src_data[r2a + 2];    // (x+2,y-2)
  if (v > maxv) return 1;

  v = src_data[r1a + 2];    // (x+2,y-1)
  if (v > maxv) return 1;

  v = src_data[r + 2];      // (x+2,y)
  if (v > maxv) return 1;

  v = src_data[r1b + 2];    // (x+2,y+1)
  if (v > maxv) return 1;

  v = src_data[r2b + 2];    // (x+2,y+2)
  if (v > maxv) return 1;

  return 0;
}

static inline void iCheckMaximum(int r, int r1a, int r1b, float *src_data, imbyte* dst_data) 
{
  int unique = 1;
  float v;
  float maxv = src_data[r];  // (x,y)

  v = src_data[r1a];        // (x,y-1)
  if (v >= maxv) { maxv = v; unique = 0; }

  v = src_data[r - 1];      // (x-1,y)
  if (v >= maxv) { maxv = v; unique = 0; }

  v = src_data[r1a - 1];    // (x-1,y-1)
  if (v >= maxv) { maxv = v; unique = 0; }

  v = src_data[r1a + 1];    // (x+1,y-1)
  if (v >= maxv) { maxv = v; unique = 0; }

  v = src_data[r + 1];      // (x+1,y)
  if (v >= maxv) { maxv = v; unique = 0; }

  v = src_data[r1b];        // (x,y+1)
  if (v >= maxv) { maxv = v; unique = 0; }

  v = src_data[r1b - 1];    // (x-1,y+1)
  if (v >= maxv) { maxv = v; unique = 0; }

  v = src_data[r1b + 1];    // (x+1,y+1)
  if (v >= maxv) { maxv = v; unique = 0; }

  if (src_data[r] < maxv)   // not a maximum
    dst_data[r] = 0;
  else
  {
    if (unique)            // unique maximum
      dst_data[r] = 1;
    else                   // can be maximum
      dst_data[r] = 2;
  }
}

void imProcessRegionalMaximum(const imImage* src_image, imImage* dst_image)
{
  int i, x, y, offset, offsetA, offsetB,
    width = src_image->width,
    height = src_image->height;

  float* src_data = (float*)src_image->data[0];
  imbyte* dst_data = (imbyte*)dst_image->data[0];

  for (y = 1; y < height-1; y++) 
  {
    offset = y * width + 1;
    offsetA = offset - width;
    offsetB = offset + width;

    for (x = 1; x < width-1; x++) 
    {
      if (src_data[offset]) 
        iCheckMaximum(offset, offsetA, offsetB, src_data, dst_data);

      offset++;
      offsetA++; 
      offsetB++; 
    }
  }

  // remove false maximum
  for (y = 2; y < height-2; y++) 
  {
    offset = y * width + 2;
    offsetA = offset - 2*width;
    offsetB = offset + 2*width;

    for (x = 2; x < width-2; x++) 
    {
      if (dst_data[offset] == 2)
      {
        if (iCheckFalseMaximum(offset, offsetA, offsetB, width, src_data))
          iFillValue(dst_data, x, y, width, 0);
      }

      offset++;
      offsetA++; 
      offsetB++; 
    }
  }

  // update destiny with remaining maximum
  for (i = 0; i < src_image->count; i++) 
  {
    if (dst_data[i] == 2)
      dst_data[i] = 1;
  }
}
