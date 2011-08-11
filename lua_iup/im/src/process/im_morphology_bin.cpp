/** \file
 * \brief Morphology Operations for Binary Images
 *
 * See Copyright Notice in im_lib.h
 * $Id: im_morphology_bin.cpp,v 1.1 2008/10/17 06:16:33 scuri Exp $
 */


#include <im.h>
#include <im_util.h>
#include <im_counter.h>

#include "im_process_loc.h"
#include "im_process_pon.h"

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <string.h>
#include <math.h>

static int DoBinMorphConvolve(imbyte *map, imbyte* new_map, int width, int height, const imImage* kernel, int counter, int hit_value, int miss_value)
{
  int *kernel_line;
  int offset, new_offset, i, j, x, y;
  int kh, kw, kh2, kw2, hit;

  kh = kernel->height;
  kw = kernel->width;
  kh2 = kernel->height/2;
  kw2 = kernel->width/2;

  int* kernel_data = (int*)kernel->data[0];

  for(j = 0; j < height; j++)
  {
    new_offset = j * width;

    for(i = 0; i < width; i++)
    {
      hit = 1;
    
      for(y = -kh2; y <= kh2 && hit; y++)
      {
        kernel_line = kernel_data + (y+kh2)*kernel->width;

        if ((j + y < 0) ||       // pass the bottom border
            (j + y >= height))   // pass the top border
          offset = -1;
        else
          offset = (j + y) * width;

        for(x = -kw2; x <= kw2; x++)
        {
          if ((offset == -1) ||
              (i + x < 0) ||     // pass the left border
              (i + x >= width))  // pass the right border
          {
            if(kernel_line[x+kw2] != -1 && kernel_line[x+kw2] != 0)  // 0 extension beyond borders
              hit = 0;
          }
          else
          {
            if(kernel_line[x+kw2] != -1 && kernel_line[x+kw2] != map[offset + (i + x)])
              hit = 0;
          }
        }
      }

      new_map[new_offset + i] = (imbyte)(hit? hit_value: miss_value);
    }    

    if (!imCounterInc(counter))
      return 0;
  }

  return 1;
}

int imProcessBinMorphConvolve(const imImage* src_image, imImage* dst_image, const imImage *kernel, int hit_white, int iter)
{
  int j, ret = 0, hit_value, miss_value;
  void *tmp = NULL;
  int counter;

  if (hit_white)
  {
    hit_value = 1;
    miss_value = 0;
  }
  else
  {
    hit_value = 0;
    miss_value = 1;
  }

  counter = imCounterBegin("Binary Morphological Convolution");
  const char* msg = (const char*)imImageGetAttribute(kernel, "Description", NULL, NULL);
  if (!msg) msg = "Processing...";
  imCounterTotal(counter, src_image->height*iter, msg);

  if (iter > 1)
    tmp = malloc(src_image->size);

  for (j = 0; j < iter; j++)
  {
    if (j == 0)
      ret = DoBinMorphConvolve((imbyte*)src_image->data[0], (imbyte*)dst_image->data[0], src_image->width, src_image->height, kernel, counter, hit_value, miss_value);
    else
    {
      memcpy(tmp, dst_image->data[0], src_image->size);
      ret = DoBinMorphConvolve((imbyte*)tmp, (imbyte*)dst_image->data[0], src_image->width, src_image->height, kernel, counter, hit_value, miss_value);
    }

    if (!ret) 
      break;
  }

  if (tmp) free(tmp);
  imCounterEnd(counter);

  return ret;
}

int imProcessBinMorphErode(const imImage* src_image, imImage* dst_image, int kernel_size, int iter)
{
  imImage* kernel = imImageCreate(kernel_size, kernel_size, IM_GRAY, IM_INT);
  imImageSetAttribute(kernel, "Description", IM_BYTE, -1, (void*)"Erode");

  int* kernel_data = (int*)kernel->data[0];
  for(int i = 0; i < kernel->count; i++)
      kernel_data[i] = 1;

  int ret = imProcessBinMorphConvolve(src_image, dst_image, kernel, 1, iter);
  imImageDestroy(kernel);
  return ret;
}

int imProcessBinMorphDilate(const imImage* src_image, imImage* dst_image, int kernel_size, int iter)
{
  imImage* kernel = imImageCreate(kernel_size, kernel_size, IM_GRAY, IM_INT);
  imImageSetAttribute(kernel, "Description", IM_BYTE, -1, (void*)"Dilate");
  // Kernel is all zeros
  int ret = imProcessBinMorphConvolve(src_image, dst_image, kernel, 0, iter);
  imImageDestroy(kernel);
  return ret;
}

int imProcessBinMorphOpen(const imImage* src_image, imImage* dst_image, int kernel_size, int iter)
{
  imImage*temp = imImageClone(src_image);
  if (!temp)
    return 0;

  if (!imProcessBinMorphErode(src_image, temp, kernel_size, iter)) {imImageDestroy(temp); return 0;}
  if (!imProcessBinMorphDilate(temp, dst_image, kernel_size, iter)) {imImageDestroy(temp); return 0;}

  imImageDestroy(temp);
  return 1;
}

int imProcessBinMorphClose(const imImage* src_image, imImage* dst_image, int kernel_size, int iter)
{
  imImage*temp = imImageClone(src_image);
  if (!temp)
    return 0;

  if (!imProcessBinMorphDilate(src_image, temp, kernel_size, iter)) {imImageDestroy(temp); return 0;}
  if (!imProcessBinMorphErode(temp, dst_image, kernel_size, iter)) {imImageDestroy(temp); return 0;}

  imImageDestroy(temp);
  return 1;
}

int imProcessBinMorphOutline(const imImage* src_image, imImage* dst_image, int kernel_size, int iter)
{
  if (!imProcessBinMorphErode(src_image, dst_image, kernel_size, iter)) return 0;
  imProcessArithmeticOp(src_image, dst_image, dst_image, IM_BIN_DIFF);
  return 1;
}

/* Direction masks:      */
/*   N     S   W     E    */
static int masks[] = { 0200, 0002, 0040, 0010 };

/*  True if pixel neighbor map indicates the pixel is 8-simple and  */
/*  not an end point and thus can be deleted.  The neighborhood  */
/*  map is defined as an integer of bits abcdefghi with a non-zero  */
/*  bit representing a non-zero pixel.  The bit assignment for the  */
/*  neighborhood is:            */
/*                  */
/*        a b c          */
/*        d e f          */
/*        g h i          */

static unsigned char isdelete[512] = 
{
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 1, 0, 0, 1, 1, 0, 1, 1, 1, 0, 0, 1, 1,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 1, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  1, 0, 1, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};

static void DoThinImage(imbyte *map, int xsize, int ysize)
{
  int    x, y;    /* Pixel location    */
  int    i;    /* Pass index      */
  int    pc  = 0;  /* Pass count      */
  int    count  = 1;  /* Deleted pixel count    */
  int    p, q;    /* Neighborhood maps of adjacent cells      */
  imbyte    *qb;    /* Neighborhood maps of previous scanline      */
  int    m;    /* Deletion direction mask  */
  
  qb = (imbyte *) malloc(xsize);
  qb[xsize-1] = 0;    /* Used for lower-right pixel  */
  
  while ( count ) 
  {    
    /* Scan src_image while deletions  */
    pc++;
    count = 0;
    
    for ( i = 0 ; i < 4 ; i++ ) 
    {
      m = masks[i];
      
      /* Build initial previous scan buffer.      */
      
      p = map[0] != 0;
      for (x = 0 ; x < xsize-1 ; x++)
      {
        p = ((p<<1)&0006) | (map[x+1] != 0);
        qb[x] = (imbyte)p;
      }
      
      /* Scan src_image for pixel deletion candidates.    */
      
      for ( y = 0 ; y < ysize-1 ; y++ ) 
      {
        q = qb[0];
        p = ((q<<3)&0110) | (map[(y+1)*xsize] != 0);
        
        for ( x = 0 ; x < xsize-1 ; x++ ) 
        {
          q = qb[x];
          p = ((p<<1)&0666) | ((q<<3)&0110) | (map[(y+1)*xsize + x+1] != 0);
          qb[x] = (imbyte)p;

          if  (((p&m) == 0) && isdelete[p] ) 
          {
            count++;
            map[y*xsize + x] = 0;
          }
        }
        
        /* Process right edge pixel.      */
       
        p = (p<<1)&0666;
        if  ( (p&m) == 0 && isdelete[p] ) 
        {
          count++;
          map[y*xsize + xsize-1] = 0;
        }
      }
      
      /* Process bottom scan line.        */
      
      for ( x = 0 ; x < xsize ; x++ ) 
      {
        q = qb[x];
        p = ((p<<1)&0666) | ((q<<3)&0110);

        if  ( (p&m) == 0 && isdelete[p] ) 
        {
          count++;
          map[(ysize-1)*xsize + x] = 0;
        }
      }
    }
  }
  
  free (qb);
}

void imProcessBinMorphThin(const imImage* src_image, imImage* dst_image)
{
  imImageCopyData(src_image, dst_image);
  DoThinImage((imbyte*)dst_image->data[0], dst_image->width, dst_image->height);
}
