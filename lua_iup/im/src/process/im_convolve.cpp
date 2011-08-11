/** \file
 * \brief Convolution Operations
 *
 * See Copyright Notice in im_lib.h
 * $Id: im_convolve.cpp,v 1.2 2009/10/01 02:56:58 scuri Exp $
 */


#include <im.h>
#include <im_util.h>
#include <im_counter.h>
#include <im_complex.h>
#include <im_math_op.h>
#include <im_image.h>
#include <im_kernel.h>

#include "im_process_loc.h"
#include "im_process_pon.h"

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <string.h>
#include <math.h>

/* Rotating Kernels
3x3
  6 7 8   7 8 5
  3 4 5   6 4 2
  0 1 2   3 0 1

5x5
  20 21 22 23 24   22 23 24 19 14
  15 16 17 18 19   21 17 18 13  9
  10 11 12 13 14   20 16 12  8  4
   5  6  7  8  9   15 11  6  7  3
   0  1  2  3  4   10  5  0  1  2

7x7
  42 43 44 45 46 47 48     45 46 47 48 41 34 27
  35 36 37 38 39 40 41     44 38 39 40 33 26 20
  28 29 30 31 32 33 34     43 37 31 32 25 19 13
  21 22 23 24 25 26 27     42 36 30 24 18 12  6
  14 15 16 17 18 19 20     35 29 23 16 17 11  5
   7  8  9 10 11 12 13     28 22 15  8  9 10  4
   0  1  2  3  4  5  6     21 14  7  0  1  2  3

 TO DO: a generic odd rotation function...
*/

template <class KT> 
static void iRotateKernel(KT* kernel_map, int kernel_size)
{
  KT temp;

  switch (kernel_size)
  {
  case 3:
    {
      temp = kernel_map[0];
      kernel_map[0] = kernel_map[3];
      kernel_map[3] = kernel_map[6];
      kernel_map[6] = kernel_map[7];
      kernel_map[7] = kernel_map[8];
      kernel_map[8] = kernel_map[5];
      kernel_map[5] = kernel_map[2];
      kernel_map[2] = kernel_map[1];
      kernel_map[1] = temp;
    }
    break;
  case 5:
    {
      temp = kernel_map[0];
      kernel_map[0] = kernel_map[10];
      kernel_map[10] = kernel_map[20];
      kernel_map[20] = kernel_map[22];
      kernel_map[22] = kernel_map[24];
      kernel_map[24] = kernel_map[14];
      kernel_map[14] = kernel_map[4];
      kernel_map[4] = kernel_map[2];
      kernel_map[2] = temp;

      temp = kernel_map[5];
      kernel_map[5] = kernel_map[15];
      kernel_map[15] = kernel_map[21];
      kernel_map[21] = kernel_map[23];
      kernel_map[23] = kernel_map[19];
      kernel_map[19] = kernel_map[9];
      kernel_map[9] = kernel_map[3];
      kernel_map[3] = kernel_map[1];
      kernel_map[1] = temp;

      temp = kernel_map[6];
      kernel_map[6] = kernel_map[11];
      kernel_map[11] = kernel_map[16];
      kernel_map[16] = kernel_map[17];
      kernel_map[17] = kernel_map[18];
      kernel_map[18] = kernel_map[13];
      kernel_map[13] = kernel_map[8];
      kernel_map[8] = kernel_map[7];
      kernel_map[7] = temp;
    }
    break;
  case 7:
    {
      temp = kernel_map[2];
      kernel_map[2] = kernel_map[7];
      kernel_map[7] = kernel_map[28];
      kernel_map[28] = kernel_map[43];
      kernel_map[43] = kernel_map[46];
      kernel_map[46] = kernel_map[41];
      kernel_map[41] = kernel_map[20];
      kernel_map[20] = kernel_map[5];
      kernel_map[5] = temp;

      temp = kernel_map[1];
      kernel_map[1] = kernel_map[14];
      kernel_map[14] = kernel_map[35];
      kernel_map[35] = kernel_map[44];
      kernel_map[44] = kernel_map[47];
      kernel_map[47] = kernel_map[34];
      kernel_map[34] = kernel_map[13];
      kernel_map[13] = kernel_map[4];
      kernel_map[4] = temp;

      temp = kernel_map[0];
      kernel_map[0] = kernel_map[21];
      kernel_map[21] = kernel_map[42];
      kernel_map[42] = kernel_map[45];
      kernel_map[45] = kernel_map[48];
      kernel_map[48] = kernel_map[27];
      kernel_map[27] = kernel_map[6];
      kernel_map[6] = kernel_map[3];
      kernel_map[3] = temp;

      temp = kernel_map[9];
      kernel_map[9] = kernel_map[15];
      kernel_map[15] = kernel_map[29];
      kernel_map[29] = kernel_map[37];
      kernel_map[37] = kernel_map[39];
      kernel_map[39] = kernel_map[33];
      kernel_map[33] = kernel_map[19];
      kernel_map[19] = kernel_map[11];
      kernel_map[11] = temp;

      temp = kernel_map[8];
      kernel_map[8] = kernel_map[22];
      kernel_map[22] = kernel_map[36];
      kernel_map[36] = kernel_map[38];
      kernel_map[38] = kernel_map[40];
      kernel_map[40] = kernel_map[26];
      kernel_map[26] = kernel_map[12];
      kernel_map[12] = kernel_map[10];
      kernel_map[10] = temp;

      temp = kernel_map[16];
      kernel_map[16] = kernel_map[23];
      kernel_map[23] = kernel_map[30];
      kernel_map[30] = kernel_map[31];
      kernel_map[31] = kernel_map[32];
      kernel_map[32] = kernel_map[25];
      kernel_map[25] = kernel_map[18];
      kernel_map[18] = kernel_map[17];
      kernel_map[17] = temp;
    }
    break;
  }
}

void imProcessRotateKernel(imImage* kernel)
{
  if (kernel->data_type == IM_INT)
    iRotateKernel((int*)kernel->data[0], kernel->width);
  else
    iRotateKernel((float*)kernel->data[0], kernel->width);
}

template <class T, class KT, class CT> 
static int DoCompassConvolve(T* map, T* new_map, int width, int height, KT* orig_kernel_map, int kernel_size, int counter, CT)
{
  CT value;
  KT total, *kernel_line, kvalue;
  int offset, new_offset, i, j, x, y, kcount;

  // duplicate the kernel data so we can rotate it
  kcount = kernel_size*kernel_size;
  KT* kernel_map = (KT*)malloc(kcount*sizeof(KT));

  int ks2 = kernel_size/2;

  total = 0;
  for(j = 0; j < kcount; j++) 
  {
    kvalue = orig_kernel_map[j];
    kernel_map[j] = kvalue;
    total += kvalue;
  }

  if (total == 0)
    total = 1;

  for(j = 0; j < height; j++)
  {
    new_offset = j * width;

    for(i = 0; i < width; i++)
    {
      CT max_value = 0;

      for(int k = 0; k < 8; k++) // Rotate 8 times
      {
        value = 0;
      
        for(y = -ks2; y <= ks2; y++)
        {
          kernel_line = kernel_map + (y+ks2)*kernel_size;

          if (j + y < 0)             // pass the bottom border
            offset = -(y + j + 1) * width;
          else if (j + y >= height)  // pass the top border
            offset = (2*height - 1 - (j + y)) * width;
          else
            offset = (j + y) * width;

          for(x = -ks2; x <= ks2; x++)
          {
            if (i + x < 0)            // pass the left border
              value += kernel_line[x+ks2] * map[offset - (i + x + 1)];
            else if (i + x >= width)  // pass the right border
              value += kernel_line[x+ks2] * map[offset + 2*width - 1 - (i + x)];
            else if (offset != -1)
              value += kernel_line[x+ks2] * map[offset + (i + x)];
          }
        }

        if (abs_op(value) > max_value)
          max_value = abs_op(value);

        iRotateKernel(kernel_map, kernel_size);
      }  

      max_value /= total;

      int size_of = sizeof(imbyte);
      if (sizeof(T) == size_of)
        new_map[new_offset + i] = (T)IM_BYTECROP(max_value);
      else
        new_map[new_offset + i] = (T)max_value;
    }    

    if (!imCounterInc(counter))
    {
      free(kernel_map);
      return 0;
    }
  }

  free(kernel_map);
  return 1;
}

int imProcessCompassConvolve(const imImage* src_image, imImage* dst_image, imImage *kernel)
{
  int ret = 0;

  int counter = imCounterBegin("Compass Convolution");
  const char* msg = (const char*)imImageGetAttribute(kernel, "Description", NULL, NULL);
  if (!msg) msg = "Filtering...";
  imCounterTotal(counter, src_image->depth*src_image->height, msg);

  for (int i = 0; i < src_image->depth; i++)
  {
    switch(src_image->data_type)
    {
    case IM_BYTE:
      if (kernel->data_type == IM_INT)
        ret = DoCompassConvolve((imbyte*)src_image->data[i], (imbyte*)dst_image->data[i], src_image->width, src_image->height, (int*)kernel->data[0], kernel->width, counter, (int)0);
      else
        ret = DoCompassConvolve((imbyte*)src_image->data[i], (imbyte*)dst_image->data[i], src_image->width, src_image->height, (float*)kernel->data[0], kernel->width, counter, (float)0);
      break;                                                                                
    case IM_USHORT:                                                                           
      if (kernel->data_type == IM_INT)
        ret = DoCompassConvolve((imushort*)src_image->data[i], (imushort*)dst_image->data[i], src_image->width, src_image->height, (int*)kernel->data[0], kernel->width, counter, (int)0);
      else
        ret = DoCompassConvolve((imushort*)src_image->data[i], (imushort*)dst_image->data[i], src_image->width, src_image->height, (float*)kernel->data[0], kernel->width, counter, (float)0);
      break;                                                                                
    case IM_INT:                                                                           
      if (kernel->data_type == IM_INT)
        ret = DoCompassConvolve((int*)src_image->data[i], (int*)dst_image->data[i], src_image->width, src_image->height, (int*)kernel->data[0], kernel->width, counter, (int)0);
      else
        ret = DoCompassConvolve((int*)src_image->data[i], (int*)dst_image->data[i], src_image->width, src_image->height, (float*)kernel->data[0], kernel->width, counter, (float)0);
      break;                                                                                
    case IM_FLOAT:                                                                           
      if (kernel->data_type == IM_INT)
        ret = DoCompassConvolve((float*)src_image->data[i], (float*)dst_image->data[i], src_image->width, src_image->height, (int*)kernel->data[0], kernel->width, counter, (float)0);
      else
        ret = DoCompassConvolve((float*)src_image->data[i], (float*)dst_image->data[i], src_image->width, src_image->height, (float*)kernel->data[0], kernel->width, counter, (float)0);
      break;                                                                                
    }
    
    if (!ret) 
      break;
  }

  imCounterEnd(counter);

  return ret;
}

template <class T, class KT, class CT> 
static int DoConvolveDual(T* map, T* new_map, int width, int height, KT* kernel_map1, KT* kernel_map2, int kernel_width, int kernel_height, int counter, CT)
{
  CT value1, value2, value;
  KT total1, total2, *kernel_line;
  int offset, new_offset, i, j, x, y;

  int kh2 = kernel_height/2;
  int kw2 = kernel_width/2;

  if (kernel_height % 2 == 0) kh2--;
  if (kernel_width % 2 == 0) kw2--;

  total1 = 0;
  for(j = 0; j < kernel_height; j++) 
  {
    for(i = 0; i < kernel_width; i++)
      total1 += kernel_map1[j*kernel_width + i];
  }

  if (total1 == 0)
    total1 = 1;

  total2 = 0;
  for(j = 0; j < kernel_height; j++) 
  {
    for(i = 0; i < kernel_width; i++)
      total2 += kernel_map2[j*kernel_width + i];
  }

  if (total2 == 0)
    total2 = 1;

  for(j = 0; j < height; j++)
  {
    new_offset = j * width;

    for(i = 0; i < width; i++)
    {
      value1 = 0;
      value2 = 0;
    
      for(y = -kh2; y <= kh2; y++)
      {
        if (j + y < 0)             // pass the bottom border
          offset = -(y + j + 1) * width;
        else if (j + y >= height)  // pass the top border
          offset = (2*height - 1 - (j + y)) * width;
        else
          offset = (j + y) * width;

        kernel_line = kernel_map1 + (y+kh2)*kernel_width;
        for(x = -kw2; x <= kw2; x++)
        {
          if (i + x < 0)            // pass the left border
            value1 += kernel_line[x+kw2] * map[offset - (i + x + 1)];
          else if (i + x >= width)  // pass the right border
            value1 += kernel_line[x+kw2] * map[offset + 2*width - 1 - (i + x)];
          else if (offset != -1)
            value1 += kernel_line[x+kw2] * map[offset + (i + x)];
        }

        kernel_line = kernel_map2 + (y+kh2)*kernel_width;
        for(x = -kw2; x <= kw2; x++)
        {
          if (i + x < 0)            // pass the left border
            value2 += kernel_line[x+kw2] * map[offset - (i + x + 1)];
          else if (i + x >= width)  // pass the right border
            value2 += kernel_line[x+kw2] * map[offset + 2*width - 1 - (i + x)];
          else if (offset != -1)
            value2 += kernel_line[x+kw2] * map[offset + (i + x)];
        }
      }
      
      value1 /= total1;
      value2 /= total2;

      value = (CT)sqrt((double)(value1*value1 + value2*value2));

      int size_of = sizeof(imbyte);
      if (sizeof(T) == size_of)
        new_map[new_offset + i] = (T)IM_BYTECROP(value);
      else
        new_map[new_offset + i] = (T)value;
    }    

    if (!imCounterInc(counter))
      return 0;
  }

  return 1;
}

template <class KT> 
static int DoConvolveDualCpx(imcfloat* map, imcfloat* new_map, int width, int height, KT* kernel_map1, KT* kernel_map2, int kernel_width, int kernel_height, int counter)
{
  imcfloat value1, value2;
  KT total1, total2, *kernel_line;
  int offset, new_offset, i, j, x, y;

  int kh2 = kernel_height/2;
  int kw2 = kernel_width/2;

  if (kernel_height % 2 == 0) kh2--;
  if (kernel_width % 2 == 0) kw2--;

  total1 = 0;
  for(j = 0; j < kernel_height; j++) 
  {
    for(i = 0; i < kernel_width; i++)
      total1 += kernel_map1[j*kernel_width + i];
  }

  if (total1 == 0)
    total1 = 1;

  total2 = 0;
  for(j = 0; j < kernel_height; j++) 
  {
    for(i = 0; i < kernel_width; i++)
      total2 += kernel_map1[j*kernel_width + i];
  }

  if (total2 == 0)
    total2 = 1;

  for(j = 0; j < height; j++)
  {
    new_offset = j * width;

    for(i = 0; i < width; i++)
    {
      value1 = 0;
      value2 = 0;
    
      for(y = -kh2; y <= kh2; y++)
      {
        if (j + y < 0)             // pass the bottom border
          offset = -(y + j + 1) * width;
        else if (j + y >= height)  // pass the top border
          offset = (2*height - 1 - (j + y)) * width;
        else
          offset = (j + y) * width;

        kernel_line = kernel_map1 + (y+kh2)*kernel_width;
        for(x = -kw2; x <= kw2; x++)
        {
          if (i + x < 0)            // pass the left border
            value1 += map[offset - (i + x + 1)] * (float)kernel_line[x+kw2];
          else if (i + x >= width)  // pass the right border
            value1 += map[offset + 2*width - 1 - (i + x)] * (float)kernel_line[x+kw2];
          else if (offset != -1)
            value1 += map[offset + (i + x)] * (float)kernel_line[x+kw2];
        }

        kernel_line = kernel_map2 + (y+kh2)*kernel_width;
        for(x = -kw2; x <= kw2; x++)
        {
          if (i + x < 0)            // pass the left border
            value2 += map[offset - (i + x + 1)] * (float)kernel_line[x+kw2];
          else if (i + x >= width)  // pass the right border
            value2 += map[offset + 2*width - 1 - (i + x)] * (float)kernel_line[x+kw2];
          else if (offset != -1)
            value2 += map[offset + (i + x)] * (float)kernel_line[x+kw2];
        }
      }
      
      value1 /= (float)total1;
      value2 /= (float)total2;

      new_map[new_offset + i] = sqrt(value1*value1 + value2*value2);
    }    

    if (!imCounterInc(counter))
      return 0;
  }

  return 1;
}

int imProcessConvolveDual(const imImage* src_image, imImage* dst_image, const imImage *kernel1, const imImage *kernel2)
{
  int counter = imCounterBegin("Convolution");
  const char* msg = (const char*)imImageGetAttribute(kernel1, "Description", NULL, NULL);
  if (!msg) msg = "Filtering...";
  imCounterTotal(counter, src_image->depth*src_image->height, msg);

  int ret = 0;

  for (int i = 0; i < src_image->depth; i++)
  {
    switch(src_image->data_type)
    {
    case IM_BYTE:
      if (kernel1->data_type == IM_INT)
        ret = DoConvolveDual((imbyte*)src_image->data[i], (imbyte*)dst_image->data[i], src_image->width, src_image->height, (int*)kernel1->data[0], (int*)kernel2->data[0], kernel1->width, kernel1->height, counter, (int)0);
      else
        ret = DoConvolveDual((imbyte*)src_image->data[i], (imbyte*)dst_image->data[i], src_image->width, src_image->height, (float*)kernel1->data[0], (float*)kernel2->data[0], kernel1->width, kernel1->height, counter, (float)0);
      break;                                                                                
    case IM_USHORT:                                                                           
      if (kernel1->data_type == IM_INT)
        ret = DoConvolveDual((imushort*)src_image->data[i], (imushort*)dst_image->data[i], src_image->width, src_image->height, (int*)kernel1->data[0], (int*)kernel2->data[0], kernel1->width, kernel1->height, counter, (int)0);
      else
        ret = DoConvolveDual((imushort*)src_image->data[i], (imushort*)dst_image->data[i], src_image->width, src_image->height, (float*)kernel1->data[0], (float*)kernel2->data[0], kernel1->width, kernel1->height, counter, (float)0);
      break;                                                                                
    case IM_INT:                                                                           
      if (kernel1->data_type == IM_INT)
        ret = DoConvolveDual((int*)src_image->data[i], (int*)dst_image->data[i], src_image->width, src_image->height, (int*)kernel1->data[0], (int*)kernel2->data[0], kernel1->width, kernel1->height, counter, (int)0);
      else
        ret = DoConvolveDual((int*)src_image->data[i], (int*)dst_image->data[i], src_image->width, src_image->height, (float*)kernel1->data[0], (float*)kernel2->data[0], kernel1->width, kernel1->height, counter, (float)0);
      break;                                                                                
    case IM_FLOAT:                                                                           
      if (kernel1->data_type == IM_INT)
        ret = DoConvolveDual((float*)src_image->data[i], (float*)dst_image->data[i], src_image->width, src_image->height, (int*)kernel1->data[0], (int*)kernel2->data[0], kernel1->width, kernel1->height, counter, (float)0);
      else
        ret = DoConvolveDual((float*)src_image->data[i], (float*)dst_image->data[i], src_image->width, src_image->height, (float*)kernel1->data[0], (float*)kernel2->data[0], kernel1->width, kernel1->height, counter, (float)0);
      break;                                                                                
    case IM_CFLOAT:            
      if (kernel1->data_type == IM_INT)
        ret = DoConvolveDualCpx((imcfloat*)src_image->data[i], (imcfloat*)dst_image->data[i], src_image->width, src_image->height, (int*)kernel1->data[0], (int*)kernel2->data[0], kernel1->width, kernel1->height, counter);
      else
        ret = DoConvolveDualCpx((imcfloat*)src_image->data[i], (imcfloat*)dst_image->data[i], src_image->width, src_image->height, (float*)kernel1->data[0], (float*)kernel2->data[0], kernel1->width, kernel1->height, counter);
      break;
    }
    
    if (!ret) 
      break;
  }

  imCounterEnd(counter);

  return ret;
}

template <class T, class KT, class CT> 
static int DoConvolve(T* map, T* new_map, int width, int height, KT* kernel_map, int kernel_width, int kernel_height, int counter, CT)
{
  CT value;
  KT total, *kernel_line;
  int offset, new_offset, i, j, x, y;

  int kh2 = kernel_height/2;
  int kw2 = kernel_width/2;

  if (kernel_height % 2 == 0) kh2--;
  if (kernel_width % 2 == 0) kw2--;

  total = 0;
  for(j = 0; j < kernel_height; j++) 
  {
    for(i = 0; i < kernel_width; i++)
      total += kernel_map[j*kernel_width + i];
  }

  if (total == 0)
    total = 1;

  for(j = 0; j < height; j++)
  {
    new_offset = j * width;

    for(i = 0; i < width; i++)
    {
      value = 0;
    
      for(y = -kh2; y <= kh2; y++)
      {
        kernel_line = kernel_map + (y+kh2)*kernel_width;

        if (j + y < 0)             // pass the bottom border
          offset = -(y + j + 1) * width;
        else if (j + y >= height)  // pass the top border
          offset = (2*height - 1 - (j + y)) * width;
        else
          offset = (j + y) * width;

        for(x = -kw2; x <= kw2; x++)
        {
          if (i + x < 0)            // pass the left border
            value += kernel_line[x+kw2] * map[offset - (i + x + 1)];
          else if (i + x >= width)  // pass the right border
            value += kernel_line[x+kw2] * map[offset + 2*width - 1 - (i + x)];
          else if (offset != -1)
            value += kernel_line[x+kw2] * map[offset + (i + x)];
        }
      }
      
      value /= total;

      int size_of = sizeof(imbyte);
      if (sizeof(T) == size_of)
        new_map[new_offset + i] = (T)IM_BYTECROP(value);
      else
        new_map[new_offset + i] = (T)value;
    }    

    if (!imCounterInc(counter))
      return 0;
  }

  return 1;
}

template <class KT> 
static int DoConvolveCpx(imcfloat* map, imcfloat* new_map, int width, int height, KT* kernel_map, int kernel_width, int kernel_height, int counter)
{
  imcfloat value;
  KT total, *kernel_line;
  int offset, new_offset, i, j, x, y;

  int kh2 = kernel_height/2;
  int kw2 = kernel_width/2;

  if (kernel_height % 2 == 0) kh2--;
  if (kernel_width % 2 == 0) kw2--;

  total = 0;
  for(j = 0; j < kernel_height; j++) 
  {
    for(i = 0; i < kernel_width; i++)
      total += kernel_map[j*kernel_width + i];
  }

  if (total == 0)
    total = 1;

  for(j = 0; j < height; j++)
  {
    new_offset = j * width;

    for(i = 0; i < width; i++)
    {
      value = 0;
    
      for(y = -kh2; y <= kh2; y++)
      {
        kernel_line = kernel_map + (y+kh2)*kernel_width;

        if (j + y < 0)             // pass the bottom border
          offset = -(y + j + 1) * width;
        else if (j + y >= height)  // pass the top border
          offset = (2*height - 1 - (j + y)) * width;
        else
          offset = (j + y) * width;

        for(x = -kw2; x <= kw2; x++)
        {
          if (i + x < 0)            // pass the left border
            value += map[offset - (i + x + 1)] * (float)kernel_line[x+kw2];
          else if (i + x >= width)  // pass the right border
            value += map[offset + 2*width - 1 - (i + x)] * (float)kernel_line[x+kw2];
          else if (offset != -1)
            value += map[offset + (i + x)] * (float)kernel_line[x+kw2];
        }
      }
      
      value /= (float)total;

      new_map[new_offset + i] = value;
    }    

    if (!imCounterInc(counter))
      return 0;
  }

  return 1;
}

static int DoConvolveStep(const imImage* src_image, imImage* dst_image, const imImage *kernel, int counter)
{
  int ret = 0;

  for (int i = 0; i < src_image->depth; i++)
  {
    switch(src_image->data_type)
    {
    case IM_BYTE:
      if (kernel->data_type == IM_INT)
        ret = DoConvolve((imbyte*)src_image->data[i], (imbyte*)dst_image->data[i], src_image->width, src_image->height, (int*)kernel->data[0], kernel->width, kernel->height, counter, (int)0);
      else
        ret = DoConvolve((imbyte*)src_image->data[i], (imbyte*)dst_image->data[i], src_image->width, src_image->height, (float*)kernel->data[0], kernel->width, kernel->height, counter, (float)0);
      break;                                                                                
    case IM_USHORT:                                                                           
      if (kernel->data_type == IM_INT)
        ret = DoConvolve((imushort*)src_image->data[i], (imushort*)dst_image->data[i], src_image->width, src_image->height, (int*)kernel->data[0], kernel->width, kernel->height, counter, (int)0);
      else
        ret = DoConvolve((imushort*)src_image->data[i], (imushort*)dst_image->data[i], src_image->width, src_image->height, (float*)kernel->data[0], kernel->width, kernel->height, counter, (float)0);
      break;                                                                                
    case IM_INT:                                                                           
      if (kernel->data_type == IM_INT)
        ret = DoConvolve((int*)src_image->data[i], (int*)dst_image->data[i], src_image->width, src_image->height, (int*)kernel->data[0], kernel->width, kernel->height, counter, (int)0);
      else
        ret = DoConvolve((int*)src_image->data[i], (int*)dst_image->data[i], src_image->width, src_image->height, (float*)kernel->data[0], kernel->width, kernel->height, counter, (float)0);
      break;                                                                                
    case IM_FLOAT:                                                                           
      if (kernel->data_type == IM_INT)
        ret = DoConvolve((float*)src_image->data[i], (float*)dst_image->data[i], src_image->width, src_image->height, (int*)kernel->data[0], kernel->width, kernel->height, counter, (float)0);
      else
        ret = DoConvolve((float*)src_image->data[i], (float*)dst_image->data[i], src_image->width, src_image->height, (float*)kernel->data[0], kernel->width, kernel->height, counter, (float)0);
      break;                                                                                
    case IM_CFLOAT:            
      if (kernel->data_type == IM_INT)
        ret = DoConvolveCpx((imcfloat*)src_image->data[i], (imcfloat*)dst_image->data[i], src_image->width, src_image->height, (int*)kernel->data[0], kernel->width, kernel->height, counter);
      else
        ret = DoConvolveCpx((imcfloat*)src_image->data[i], (imcfloat*)dst_image->data[i], src_image->width, src_image->height, (float*)kernel->data[0], kernel->width, kernel->height, counter);
      break;
    }
    
    if (!ret) 
      break;
  }

  return ret;
}

int imProcessConvolve(const imImage* src_image, imImage* dst_image, const imImage *kernel)
{
  int counter = imCounterBegin("Convolution");
  const char* msg = (const char*)imImageGetAttribute(kernel, "Description", NULL, NULL);
  if (!msg) msg = "Filtering...";
  imCounterTotal(counter, src_image->depth*src_image->height, msg);

  int ret = DoConvolveStep(src_image, dst_image, kernel, counter);

  imCounterEnd(counter);

  return ret;
}

int imProcessConvolveRep(const imImage* src_image, imImage* dst_image, const imImage *kernel, int ntimes)
{
  imImage *AuxImage = imImageClone(dst_image);
  if (!AuxImage)
    return 0;

  int counter = imCounterBegin("Repeated Convolution");
  const char* msg = (const char*)imImageGetAttribute(kernel, "Description", NULL, NULL);
  if (!msg) msg = "Filtering...";
  imCounterTotal(counter, src_image->depth*src_image->height*ntimes, msg);

  const imImage *image1 = src_image;
  imImage *image2 = dst_image;

  for (int i = 0; i < ntimes; i++)
  {
    if (!DoConvolveStep(image1, image2, kernel, counter))
    {
      imCounterEnd(counter);
      imImageDestroy(AuxImage);
      return 0;
    }
    
    image1 = image2;

    if (image1 == dst_image)
      image2 = AuxImage;
    else
      image2 = dst_image;
  }

  // The result is in image1, if in the Aux swap the data
  if (image1 == AuxImage)
  {
    void** temp = (void**)dst_image->data;
    dst_image->data = AuxImage->data;
    AuxImage->data = (void**)temp;
  }

  imCounterEnd(counter);
  imImageDestroy(AuxImage);

  return 1;
}

template <class T, class KT, class CT> 
static int DoConvolveSep(T* map, T* new_map, int width, int height, KT* kernel_map, int kernel_width, int kernel_height, int counter, CT)
{
  CT value;
  KT totalV, totalH, *kernel_line;
  T* aux_line;
  int offset, new_offset, i, j;

  int kh2 = kernel_height/2;
  int kw2 = kernel_width/2;

  if (kernel_height % 2 == 0) kh2--;
  if (kernel_width % 2 == 0) kw2--;

  // use only the first line and the first column of the kernel

  totalV = 0;
  for(j = 0; j < kernel_height; j++) 
    totalV += kernel_map[j*kernel_width];

  if (totalV == 0)
    totalV = 1;

  totalH = 0;
  for(i = 0; i < kernel_width; i++)
    totalH += kernel_map[i];

  if (totalH == 0)
    totalH = 1;

  aux_line = (T*)malloc(width*sizeof(T));

  for(j = 0; j < height; j++)
  {
    new_offset = j * width;

    for(i = 0; i < width; i++)
    {
      int y;
      value = 0;

      // first pass, only for columns
    
      for(y = -kh2; y <= kh2; y++)
      {
        kernel_line = kernel_map + (y+kh2)*kernel_width;

        if (j + y < 0)             // pass the bottom border
          offset = -(y + j + 1) * width;
        else if (j + y >= height)  // pass the top border
          offset = (2*height - 1 - (j + y)) * width;
        else
          offset = (j + y) * width;

        if (offset != -1)
          value += kernel_line[0] * map[offset + i];
      }
      
      value /= totalV;

      int size_of = sizeof(imbyte);
      if (sizeof(T) == size_of)
        new_map[new_offset + i] = (T)IM_BYTECROP(value);
      else
        new_map[new_offset + i] = (T)value;
    }    

    if (!imCounterInc(counter))
    {
      free(aux_line);
      return 0;
    }
  }

  for(j = 0; j < height; j++)
  {
    offset = new_offset = j * width;

    for(i = 0; i < width; i++)
    {
      int x;
      value = 0;

      // second pass, only for lines, but has to use an auxiliar buffer
    
      kernel_line = kernel_map;

      for(x = -kw2; x <= kw2; x++)
      {
        if (i + x < 0)            // pass the left border
          value += kernel_line[x+kw2] * new_map[offset - (i + x + 1)];
        else if (i + x >= width)  // pass the right border
          value += kernel_line[x+kw2] * new_map[offset + 2*width - 1 - (i + x)];
        else
          value += kernel_line[x+kw2] * new_map[offset + (i + x)];
      }
      
      value /= totalH;

      int size_of = sizeof(imbyte);
      if (sizeof(T) == size_of)
        aux_line[i] = (T)IM_BYTECROP(value);
      else
        aux_line[i] = (T)value;
    }    

    memcpy(new_map + new_offset, aux_line, width*sizeof(T));

    if (!imCounterInc(counter))
    {
      free(aux_line);
      return 0;
    }
  }

  free(aux_line);
  return 1;
}


template <class KT> 
static int DoConvolveSepCpx(imcfloat* map, imcfloat* new_map, int width, int height, KT* kernel_map, int kernel_width, int kernel_height, int counter)
{
  imcfloat value;
  KT totalV, totalH, *kernel_line;
  imcfloat* aux_line;
  int offset, new_offset, i, j;

  int kh2 = kernel_height/2;
  int kw2 = kernel_width/2;

  if (kernel_height % 2 == 0) kh2--;
  if (kernel_width % 2 == 0) kw2--;

  // use only the first line and the first column of the kernel

  totalV = 0;
  for(j = 0; j < kernel_height; j++) 
    totalV += kernel_map[j*kernel_width];

  if (totalV == 0)
    totalV = 1;

  totalH = 0;
  for(i = 0; i < kernel_width; i++)
    totalH += kernel_map[i];

  if (totalH == 0)
    totalH = 1;

  aux_line = (imcfloat*)malloc(width*sizeof(imcfloat));

  for(j = 0; j < height; j++)
  {
    new_offset = j * width;

    for(i = 0; i < width; i++)
    {
      int y;
      value = 0;

      // first pass, only for columns
    
      for(y = -kh2; y <= kh2; y++)
      {
        kernel_line = kernel_map + (y+kh2)*kernel_width;

        if (j + y < 0)             // pass the bottom border
          offset = -(y + j + 1) * width;
        else if (j + y >= height)  // pass the top border
          offset = (2*height - 1 - (j + y)) * width;
        else
          offset = (j + y) * width;

        if (offset != -1)
          value += map[offset + i] * (float)kernel_line[0];
      }
      
      value /= (float)totalV;

      new_map[new_offset + i] = value;
    }    

    if (!imCounterInc(counter))
    {
      free(aux_line);
      return 0;
    }
  }

  for(j = 0; j < height; j++)
  {
    offset = new_offset = j * width;

    for(i = 0; i < width; i++)
    {
      int x;
      value = 0;

      // second pass, only for lines, but has to use an auxiliar buffer
    
      kernel_line = kernel_map;
    
      for(x = -kw2; x <= kw2; x++)
      {
        if (i + x < 0)            // pass the left border
          value += new_map[offset - (i + x + 1)] * (float)kernel_line[x+kw2];
        else if (i + x >= width)  // pass the right border
          value += new_map[offset + 2*width - 1 - (i + x)] * (float)kernel_line[x+kw2];
        else if (offset != -1)
          value += new_map[offset + (i + x)] * (float)kernel_line[x+kw2];
      }
      
      value /= (float)totalH;

      aux_line[i] = value;
    }    

    memcpy(new_map + new_offset, aux_line, width*sizeof(imcfloat));

    if (!imCounterInc(counter))
    {
      free(aux_line);
      return 0;
    }
  }

  free(aux_line);
  return 1;
}

int imProcessConvolveSep(const imImage* src_image, imImage* dst_image, const imImage *kernel)
{
  int counter = imCounterBegin("Separable Convolution");
  const char* msg = (const char*)imImageGetAttribute(kernel, "Description", NULL, NULL);
  if (!msg) msg = "Filtering...";
  imCounterTotal(counter, 2*src_image->depth*src_image->height, msg);

  int ret = 0;

  for (int i = 0; i < src_image->depth; i++)
  {
    switch(src_image->data_type)
    {
    case IM_BYTE:
      if (kernel->data_type == IM_INT)
        ret = DoConvolveSep((imbyte*)src_image->data[i], (imbyte*)dst_image->data[i], src_image->width, src_image->height, (int*)kernel->data[0], kernel->width, kernel->height, counter, (int)0);
      else
        ret = DoConvolveSep((imbyte*)src_image->data[i], (imbyte*)dst_image->data[i], src_image->width, src_image->height, (float*)kernel->data[0], kernel->width, kernel->height, counter, (float)0);
      break;                                                                                
    case IM_USHORT:                                                                           
      if (kernel->data_type == IM_INT)
        ret = DoConvolveSep((imushort*)src_image->data[i], (imushort*)dst_image->data[i], src_image->width, src_image->height, (int*)kernel->data[0], kernel->width, kernel->height, counter, (int)0);
      else
        ret = DoConvolveSep((imushort*)src_image->data[i], (imushort*)dst_image->data[i], src_image->width, src_image->height, (float*)kernel->data[0], kernel->width, kernel->height, counter, (float)0);
      break;                                                                                
    case IM_INT:                                                                           
      if (kernel->data_type == IM_INT)
        ret = DoConvolveSep((int*)src_image->data[i], (int*)dst_image->data[i], src_image->width, src_image->height, (int*)kernel->data[0], kernel->width, kernel->height, counter, (int)0);
      else
        ret = DoConvolveSep((int*)src_image->data[i], (int*)dst_image->data[i], src_image->width, src_image->height, (float*)kernel->data[0], kernel->width, kernel->height, counter, (float)0);
      break;                                                                                
    case IM_FLOAT:                                                                           
      if (kernel->data_type == IM_INT)
        ret = DoConvolveSep((float*)src_image->data[i], (float*)dst_image->data[i], src_image->width, src_image->height, (int*)kernel->data[0], kernel->width, kernel->height, counter, (float)0);
      else
        ret = DoConvolveSep((float*)src_image->data[i], (float*)dst_image->data[i], src_image->width, src_image->height, (float*)kernel->data[0], kernel->width, kernel->height, counter, (float)0);
      break;                                                                                
    case IM_CFLOAT:            
      if (kernel->data_type == IM_INT)
        ret = DoConvolveSepCpx((imcfloat*)src_image->data[i], (imcfloat*)dst_image->data[i], src_image->width, src_image->height, (int*)kernel->data[0], kernel->width, kernel->height, counter);
      else
        ret = DoConvolveSepCpx((imcfloat*)src_image->data[i], (imcfloat*)dst_image->data[i], src_image->width, src_image->height, (float*)kernel->data[0], kernel->width, kernel->height, counter);
      break;
    }
    
    if (!ret) 
      break;
  }

  imCounterEnd(counter);

  return ret;
}

/*
Description:	
    Can be used to find zero crossing of second derivative,
		laplace. Can also be used to determine any other kind
		of crossing. Pixels below or equal to 't' are set if the pixel
		to the right or below is above 't', pixels above 't' are
		set if the pixel to the right or below is below or equal to
		't'. Pixels that are "set" are set to the maximum absolute
		difference of the two neighbours, to indicate the strength
		of the edge.

		| IF (crossing t)
		|   out(x,y) = MAX(ABS(in(x,y)-in(x+1,y)), ABS(in(x,y)-in(x,y+1)))
		| ELSE
		|   out(x,y) = 0

Author:		Tor Lønnestad, BLAB, Ifi, UiO

Copyright 1991, Blab, UiO
Image processing lab, Department of Informatics
University of Oslo
*/
template <class T> 
static void do_crossing(T* iband, T* oband, int width, int height, T t)
{
  int x, y, offset00 = 0, offset10 = 0, offset01 = 0;
  T v, diff;

  for (y=0; y < height-1; y++)
  {
    offset00 = y*width;
    offset10 = (y+1)*width;
    offset01 = offset00 + 1;

    for (x=0; x < width-1; x++)
    {
      v = 0;

      if (iband[offset00] <= t)
      {
        if (iband[offset10] > t) 
          v = iband[offset10]-iband[offset00];

	      if (iband[offset01] > t) 
        {
          diff = iband[offset01]-iband[offset00];
          if (diff > v) v = diff;
        }
      }
      else
      {
	      if (iband[offset10] <= t) 
          v = iband[offset00]-iband[offset10];

	      if (iband[offset01] <= t) 
        {
          diff = iband[offset00]-iband[offset01];
          if (diff > v) v = diff;
        }
      }

      oband[offset00] = v;

      offset00++;
      offset10++;
      offset01++;
    }

    /* last pixel on line */
    offset00++;
    offset10++;

    v = 0;

    if (iband[offset00] <= t)
    {
      if (iband[offset10] > t)
        v = iband[offset10]-iband[offset00];
    }
    else
    {
      if (iband[offset10] <= t)
        v = iband[offset00]-iband[offset10];
    }

    oband[offset00] = v;
  }

  /* last line */
  offset00 = y*width;
  offset01 = offset00 + 1;

  for (x=0; x < width-1; x++)
  {
    v = 0;

    if (iband[offset00] <= t)
    {
      if (iband[offset01] > t)
        v = iband[offset01]-iband[offset00];
    }
    else
    {
      if (iband[offset01] <= t)
        v = iband[offset00]-iband[offset01];
    }

    oband[offset00] = v;

    offset00++;
    offset01++;
  }

  offset00++;

  /* last pixel */
  oband[offset00] = 0;
}

void imProcessZeroCrossing(const imImage* src_image, imImage* dst_image)
{
  for (int i = 0; i < src_image->depth; i++)
  {
    switch(src_image->data_type)
    {
    case IM_INT:                                                                           
      do_crossing((int*)src_image->data[i], (int*)dst_image->data[i], src_image->width, src_image->height, 0);
      break;                                                                                
    case IM_FLOAT:                                                                           
      do_crossing((float*)src_image->data[i], (float*)dst_image->data[i], src_image->width, src_image->height, 0.0f);
      break;                                                                                
    }
  }
}

int imProcessBarlettConvolve(const imImage* src_image, imImage* dst_image, int kernel_size)
{
  imImage* kernel = imImageCreate(kernel_size, kernel_size, IM_GRAY, IM_INT);
  if (!kernel)
    return 0;

  imImageSetAttribute(kernel, "Description", IM_BYTE, -1, (void*)"Barlett");

  int* kernel_data = (int*)kernel->data[0];
  int half = kernel_size / 2;
  for (int i = 0; i < kernel_size; i++)
  {
    if (i <= half)
      kernel_data[i] = i+1;
    else
      kernel_data[i] = kernel_size-i;
  }
  for (int j = 0; j < kernel_size; j++)
  {
    if (j <= half)
      kernel_data[j*kernel_size] = j+1;
    else
      kernel_data[j*kernel_size] = kernel_size-j;
  }

  int ret = imProcessConvolveSep(src_image, dst_image, kernel);

  imImageDestroy(kernel);

  return ret;
}

int imProcessSobelConvolve(const imImage* src_image, imImage* dst_image)
{
	int ret = 0;

  imImage* kernel1 = imKernelSobel();
  imImage* kernel2 = imImageCreate(3, 3, IM_GRAY, IM_INT);
  imProcessRotate90(kernel1, kernel2, 1);

  ret = imProcessConvolveDual(src_image, dst_image, kernel1, kernel2);

  imImageDestroy(kernel1);
  imImageDestroy(kernel2);

  return ret;
}

int imProcessPrewittConvolve(const imImage* src_image, imImage* dst_image)
{
	int ret = 0;

  imImage* kernel1 = imKernelPrewitt();
  imImage* kernel2 = imImageClone(kernel1);
  imProcessRotate90(kernel1, kernel2, 1);

  ret = imProcessConvolveDual(src_image, dst_image, kernel1, kernel2);

  imImageDestroy(kernel1);
  imImageDestroy(kernel2);

  return ret;
}

int imProcessSplineEdgeConvolve(const imImage* src_image, imImage* dst_image)
{
	int ret = 0;

  imImage* tmp_image = imImageClone(src_image);
  if (!tmp_image) return 0;

  imImage* kernel1 = imImageCreate(5, 5, IM_GRAY, IM_INT);
  imImageSetAttribute(kernel1, "Description", IM_BYTE, -1, (void*)"SplineEdge");

  int* kernel_data = (int*)kernel1->data[0];
  kernel_data[10] = -1;
  kernel_data[11] = 8;
  kernel_data[12] = 0;
  kernel_data[13] = -8;
  kernel_data[14] = 1;

  imImage* kernel2 = imImageClone(kernel1);
  imProcessRotate90(kernel1, kernel2, 1);

  imImage* kernel3 = imImageClone(kernel1);
  imProcessRotateKernel(kernel3);

  imImage* kernel4 = imImageClone(kernel1);
  imProcessRotate90(kernel3, kernel4, 1);

  ret = imProcessConvolveDual(src_image, tmp_image, kernel1, kernel2);
  ret = imProcessConvolveDual(src_image, dst_image, kernel3, kernel4);

  imProcessArithmeticConstOp(tmp_image, (float)sqrt(2.0), tmp_image, IM_BIN_MUL);
  imProcessArithmeticOp(tmp_image, dst_image, dst_image, IM_BIN_ADD);

  imImageDestroy(kernel1);
  imImageDestroy(kernel2);
  imImageDestroy(kernel3);
  imImageDestroy(kernel4);
  imImageDestroy(tmp_image);

  return ret;
}

int imGaussianStdDev2KernelSize(float stddev)
{
  if (stddev < 0)
    return (int)-stddev;
  else
  {
	  int width = (int)(3.35*stddev + 0.3333);
    return 2*width + 1;
  }
}

float imGaussianKernelSize2StdDev(int kernel_size)
{
  int width = (kernel_size - 1)/2;
	return (width - 0.3333f)/3.35f;
}

int imProcessGaussianConvolve(const imImage* src_image, imImage* dst_image, float stddev)
{
  int kernel_size = imGaussianStdDev2KernelSize(stddev);

  imImage* kernel = imImageCreate(kernel_size, kernel_size, IM_GRAY, IM_FLOAT);
  if (!kernel)
    return 0;

  imImageSetAttribute(kernel, "Description", IM_BYTE, -1, (void*)"Gaussian");
  imProcessRenderGaussian(kernel, stddev);

  int ret = imProcessConvolveSep(src_image, dst_image, kernel);

  imImageDestroy(kernel);

  return ret;
}

int imProcessLapOfGaussianConvolve(const imImage* src_image, imImage* dst_image, float stddev)
{
  int kernel_size = imGaussianStdDev2KernelSize(stddev);

  imImage* kernel = imImageCreate(kernel_size, kernel_size, IM_GRAY, IM_FLOAT);
  if (!kernel)
    return 0;

  imImageSetAttribute(kernel, "Description", IM_BYTE, -1, (void*)"Laplacian Of Gaussian");
  imProcessRenderLapOfGaussian(kernel, stddev);

  int ret;
  if (src_image->data_type == IM_BYTE || src_image->data_type == IM_USHORT)
  {
    imImage* aux_image = imImageClone(dst_image);
    if (!aux_image)
    {
      imImageDestroy(kernel);
      return 0;
    }

    imProcessUnArithmeticOp(src_image, aux_image, IM_UN_EQL);  // Convert to IM_INT
    ret = imProcessConvolve(aux_image, dst_image, kernel);
    imImageDestroy(aux_image);
  }
  else
    ret = imProcessConvolve(src_image, dst_image, kernel);

  imImageDestroy(kernel);

  return ret;
}

int imProcessDiffOfGaussianConvolve(const imImage* src_image, imImage* dst_image, float stddev1, float stddev2)
{
  imImage* aux_image1 = imImageClone(src_image);
  imImage* aux_image2 = imImageClone(src_image);
  if (!aux_image1 || !aux_image2)
  {
    if (aux_image1) imImageDestroy(aux_image1);
    return 0;
  }

  int kernel_size1 = imGaussianStdDev2KernelSize(stddev1);
  int kernel_size2 = imGaussianStdDev2KernelSize(stddev2);
  int size = kernel_size1;
  if (kernel_size1 < kernel_size2) size = kernel_size2;

  imImage* kernel1 = imImageCreate(size, size, IM_GRAY, IM_FLOAT);
  imImage* kernel2 = imImageCreate(size, size, IM_GRAY, IM_FLOAT);
  if (!kernel1 || !kernel2)
  {
    if (kernel1) imImageDestroy(kernel1);
    if (kernel2) imImageDestroy(kernel2);
    imImageDestroy(aux_image1);
    imImageDestroy(aux_image2);
    return 0;
  }

  imImageSetAttribute(kernel1, "Description", IM_BYTE, -1, (void*)"Gaussian1");
  imImageSetAttribute(kernel2, "Description", IM_BYTE, -1, (void*)"Gaussian2");

  imProcessRenderGaussian(kernel1, stddev1);
  imProcessRenderGaussian(kernel2, stddev2);

  if (!imProcessConvolve(src_image, aux_image1, kernel1) ||
      !imProcessConvolve(src_image, aux_image2, kernel2))
  {
    imImageDestroy(kernel1);
    imImageDestroy(kernel2);
    imImageDestroy(aux_image1);
    imImageDestroy(aux_image2);
    return 0;
  }

  imProcessArithmeticOp(aux_image1, aux_image2, dst_image, IM_BIN_SUB);

  imImageDestroy(kernel1);
  imImageDestroy(kernel2);
  imImageDestroy(aux_image1);
  imImageDestroy(aux_image2);

  return 1;
}

int imProcessMeanConvolve(const imImage* src_image, imImage* dst_image, int ks)
{
  int counter = imCounterBegin("Mean Convolve");
  imCounterTotal(counter, src_image->depth*src_image->height, "Filtering...");

  imImage* kernel = imImageCreate(ks, ks, IM_GRAY, IM_INT);

  int* kernel_data = (int*)kernel->data[0];

  int ks2 = ks/2;
  for(int ky = 0; ky < ks; ky++)
  {
    int ky2 = ky-ks2;
    ky2 = ky2*ky2;
    for(int kx = 0; kx < ks; kx++) 
    {
      int kx2 = kx-ks2;
      kx2 = kx2*kx2;
      int radius = imRound(sqrt(double(kx2 + ky2)));
      if (radius <= ks2)
        kernel_data[ky*ks + kx] = 1;
    }
  }

  int ret = DoConvolveStep(src_image, dst_image, kernel, counter);

  imImageDestroy(kernel);
  imCounterEnd(counter);

  return ret;
}

template <class T1, class T2> 
static void DoSharpOp(T1 *src_map, T1 *dst_map, int count, float amount, T2 threshold, int gauss)
{
  int i;
  T1 min, max;

  int size_of = sizeof(imbyte);
  if (sizeof(T1) == size_of)
  {
    min = 0;
    max = 255;
  }
  else
  {
    imMinMax(src_map, count, min, max);

    if (min == max)
    {
      max = min + 1;

      if (min != 0)
        min = min - 1;
    }
  }

  for (i = 0; i < count; i++)
  {
    T2 diff;
    
    if (gauss)
      diff = 20*(src_map[i] - dst_map[i]);  /* dst_map contains a gaussian filter of the source image, must compensate for small edge values */
    else
      diff = dst_map[i];  /* dst_map contains a laplacian filter of the source image */

    if (threshold && abs_op(2*diff) < threshold)
      diff = 0;

    T2 value = (T2)(src_map[i] + amount*diff);
    if (value < min)
      value = min;
    else if (value > max)
      value = max;

    dst_map[i] = (T1)value;
  }
}

static void doSharp(const imImage* src_image, imImage* dst_image, float amount, float threshold, int gauss)
{
  int count = src_image->count;

  for (int i = 0; i < src_image->depth; i++)
  {
    switch(src_image->data_type)
    {
    case IM_BYTE:
      DoSharpOp((imbyte*)src_image->data[i], (imbyte*)dst_image->data[i], count, amount, (int)threshold, gauss);
      break;
    case IM_USHORT:
      DoSharpOp((imushort*)src_image->data[i], (imushort*)dst_image->data[i], count, amount, (int)threshold, gauss);
      break;
    case IM_INT:
      DoSharpOp((int*)src_image->data[i], (int*)dst_image->data[i], count, amount, (int)threshold, gauss);
      break;
    case IM_FLOAT:
      DoSharpOp((float*)src_image->data[i], (float*)dst_image->data[i], count, amount, (float)threshold, gauss);
      break;
    }
  }
}

int imProcessUnsharp(const imImage* src_image, imImage* dst_image, float stddev, float amount, float threshold)
{
  int kernel_size = imGaussianStdDev2KernelSize(stddev);

  imImage* kernel = imImageCreate(kernel_size, kernel_size, IM_GRAY, IM_FLOAT);
  if (!kernel)
    return 0;

  imImageSetAttribute(kernel, "Description", IM_BYTE, -1, (void*)"Unsharp");
  imProcessRenderGaussian(kernel, stddev);

  int ret = imProcessConvolveSep(src_image, dst_image, kernel);
  doSharp(src_image, dst_image, amount, threshold, 1);

  imImageDestroy(kernel);

  return ret;
}

int imProcessSharp(const imImage* src_image, imImage* dst_image, float amount, float threshold)
{
  imImage* kernel = imKernelLaplacian8();
  if (!kernel)
    return 0;

  int ret = imProcessConvolve(src_image, dst_image, kernel);
  doSharp(src_image, dst_image, amount, threshold, 0);

  imImageDestroy(kernel);

  return ret;
}

static int iProcessCheckKernelType(const imImage* kernel)
{
  if (kernel->data_type == IM_INT)
  {
    int* kernel_data = (int*)kernel->data[0];
    for (int i = 0; i < kernel->count; i++)
    {
      if (kernel_data[i] < 0)   /* if there are negative values, assume kernel is an edge detector */
        return 0;
    }
  }
  else if (kernel->data_type == IM_FLOAT)
  {
    float* kernel_data = (float*)kernel->data[0];
    for (int i = 0; i < kernel->count; i++)
    {
      if (kernel_data[i] < 0)   /* if there are negative values, assume kernel is an edge detector */
        return 0;
    }
  }
  return 1;  /* default is kernel is a smooth filter */
}

int imProcessSharpKernel(const imImage* src_image, const imImage* kernel, imImage* dst_image, float amount, float threshold)
{
  int ret = imProcessConvolve(src_image, dst_image, kernel);
  doSharp(src_image, dst_image, amount, threshold, iProcessCheckKernelType(kernel));
  return ret;
}

