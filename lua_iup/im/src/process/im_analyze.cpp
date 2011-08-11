/** \file
 * \brief Image Analysis
 *
 * See Copyright Notice in im_lib.h
 * $Id: im_analyze.cpp,v 1.5 2010/01/22 19:47:56 scuri Exp $
 */


#include <im.h>
#include <im_util.h>
#include <im_math.h>

#include "im_process_ana.h"
#include "im_process_pon.h"

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <string.h>

#define MAX_COUNT 65536  // maximum number of regions

/* ajust the alias table to be a remap table (final step) */
static void alias_update(imushort* alias_table, int &region_count)
{
  int i, real_count = region_count;

  for (i = 0; i < region_count; i++)
  {
    if (alias_table[i])
    {
      // search for the first alias
      imushort prev = alias_table[i];
      while (alias_table[prev])
        prev = alias_table[prev];

      alias_table[i] = prev;
      real_count--;  // decrement aliases from the region count
    }
  }

  // now all the aliases in the same group point to only one alias
  // transform the alias table into a remap table

  alias_table[0] = 0;
  alias_table[1] = 0;  // border is mapped to background

  int r = 1;
  for (i = 2; i < region_count; i++)
  {
    if (!alias_table[i])
    {
      alias_table[i] = (imushort)r; // only non alias get real values
      r++;
    }
    else
      alias_table[i] = (imushort)(alias_table[alias_table[i]]);
  }

  region_count = real_count-2; // remove the regions (background,border) from the count 
}

/* find the smallest region number to be set as alias. */
static void alias_getmin(imushort* alias_table, imushort region, imushort &min)
{
  while (alias_table[region])
  {
    if (min > alias_table[region])
      min = alias_table[region];

    region = alias_table[region];
  }
}

/* replace all the aliases of a region by its smallest value. */
static void alias_setmin(imushort* alias_table, imushort region, imushort min)
{
  while (alias_table[region])
  {
    imushort next_region = alias_table[region];
    alias_table[region] = min;
    region = next_region;
  }

  if (region != min)
    alias_table[region] = min;
}

/* set a region number to be an alias of another */
static void alias_set(imushort* alias_table, imushort region1, imushort region2)
{
  if (region1 == region2)
    return;

  imushort min = region1<region2? region1: region2;

  alias_getmin(alias_table, region1, min);
  alias_getmin(alias_table, region2, min);

  if (region1 != min && alias_table[region1] != min)
    alias_setmin(alias_table, region1, min);
  if (region2 != min && alias_table[region2] != min)
    alias_setmin(alias_table, region2, min);
}

static int DoAnalyzeFindRegions(int width, int height, imbyte* map, imushort* new_map, int connect)
{
  int i, j;

  // mark the pixels that touch the border
  // if a region touch the border, is the invalid region 1

  imbyte* pmap = map;
  imushort* new_pmap = new_map;
  for (j = 0; j < width; j++)     // first line
  {
    if (pmap[j])
      new_pmap[j] = 1;
  }
  pmap += width;
  new_pmap += width;

  for (i = 1; i < height-1; i++)  // first column
  {
    if (pmap[0])
      new_pmap[0] = 1;

    pmap += width;
    new_pmap += width;
  }

  // find and connect the regions

  imbyte* pmap1 = map;         // previous line (line 0)
  imushort* new_pmap1 = new_map; 

  pmap = map + width;          // current line (line 1)
  new_pmap = new_map + width;

  int region_count = 2;  // 0- background, 1-border
  imushort* alias_table = new imushort [MAX_COUNT];
  memset(alias_table, 0, MAX_COUNT*sizeof(imushort)); // aliases are all zero at start (not used)

  for (i = 1; i < height; i++)
  {
    for (j = 1; j < width; j++)
    {
      int has_j1 = j < width-1? 1: 0;
      if (pmap[j])
      {
        if (pmap[j-1] || pmap1[j] || 
            (connect == 8 && (pmap1[j-1] || (has_j1&&pmap1[j+1])))) // 4 or 8 connected to the previous neighbors
        {
          imushort region = 0;
          if (i == height-1 || j == width-1)
          {
            region = new_pmap[j] = 1;
          }

          if (pmap[j-1])
          {
            if (!region)
              region = new_pmap[j-1];  // horizontal neighbor  -00
            else                       //                      X1
            {
              // this is a right border pixel that connects to an horizontal neighbor

              // this pixel can connect two different regions
              alias_set(alias_table, region, new_pmap[j-1]);
            }
          }

          if (pmap1[j])    // vertical neighbor
          {
            if (!region)
              region = new_pmap1[j];  // isolated vertical neighbor  -X-
            else                      //                             01
            {
              // an horizontal neighbor connects to a vertical neighbor  -X-
              //                                                         X1

              // this pixel can connect two different regions
              alias_set(alias_table, region, new_pmap1[j]);
            }
          }
          else if (region && connect==8 && (has_j1&&pmap1[j+1]))
          {
            // an horizontal neighbor connects to a right corner neighbor   00X
            //                                                              X1

            // this pixel can connect two different regions
            alias_set(alias_table, region, new_pmap1[j+1]);
          }

          if (connect == 8 && (pmap1[j-1] || (has_j1&&pmap1[j+1])) && !region) // isolated corner
          {
            // a left corner neighbor or a right corner neighbor  X0X
            //                                                    01

            if (pmap1[j-1])  // left corner
              region = new_pmap1[j-1];

            if (pmap1[j+1])  // right corner
            {
              if (!region) // isolated right corner
                region = new_pmap1[j+1];
              else
              {
                // this pixel can connect two different regions
                alias_set(alias_table, new_pmap1[j-1], new_pmap1[j+1]);
              }
            }
          }

          new_pmap[j] = region;
        }
        else
        {
          // this pixel touches no pixels

          if (i == height-1 || j == width-1)
            new_pmap[j] = 1;
          else
          {
            // create a new region  000
            //                      01
            new_pmap[j] = (imushort)region_count;
            region_count++;

            if (region_count > MAX_COUNT)
            {
              delete [] alias_table;
              return -1;
            }
          }
        }
      }
    }

    pmap1 = pmap;
    new_pmap1 = new_pmap;
    pmap += width;
    new_pmap += width;
  }

  // now all pixels are marked, 
  // but some marks are aliases to others

  // ajust the alias table to be a remap table
  // and return the real region count
  alias_update(alias_table, region_count);

  int count = width*height;
  for (i = 0; i < count; i++)
  {
    new_map[i] = alias_table[new_map[i]];
  }

  delete [] alias_table;

  return region_count;
}

static int DoAnalyzeFindRegionsBorder(int width, int height, imbyte* map, imushort* new_map, int connect)
{
  int i, j;

  imbyte* pmap1 = map - width;         // previous line (line -1 = invalid)
  imushort* new_pmap1 = new_map - width; 

  imbyte* pmap = map;                  // current line (line 0)
  imushort* new_pmap = new_map;

  int region_count = 2;  // still consider: 0- background, 1-border
  imushort* alias_table = new imushort [MAX_COUNT];
  memset(alias_table, 0, MAX_COUNT*sizeof(imushort)); // aliases are all zero at start (not used)

  for (i = 0; i < height; i++)
  {
    for (j = 0; j < width; j++)
    {
      if (pmap[j])
      {
        int b01 = j > 0? 1: 0; // valid for pmap[j-1]
        int b10 = i > 0? 1: 0; // valid for pmap1[j]
        int b11 = i > 0 && j > 0? 1: 0; // valid for pmap1[j-1]
        int b12 = i > 0 && j < width-1? 1: 0; // valid for pmap1[j+1]

        if ((b01&&pmap[j-1]) || (b10&&pmap1[j]) || 
            (connect == 8 && ((b11&&pmap1[j-1]) || (b12&&pmap1[j+1])))) // 4 or 8 connected to the previous neighbors
        {
          imushort region = 0;

          if (b01&&pmap[j-1])
          {
            if (!region)
              region = new_pmap[j-1];  // horizontal neighbor  -00
            else                       //                      X1
            {
              // this is a right border pixel that connects to an horizontal neighbor

              // this pixel can connect two different regions
              alias_set(alias_table, region, new_pmap[j-1]);
            }
          }

          if (b10&&pmap1[j])    // vertical neighbor
          {
            if (!region)
              region = new_pmap1[j];  // isolated vertical neighbor  -X-
            else                      //                             01
            {
              // an horizontal neighbor connects to a vertical neighbor  -X-
              //                                                         X1

              // this pixel can connect two different regions
              alias_set(alias_table, region, new_pmap1[j]);
            }
          }
          else if (region && connect == 8 && (b12&&pmap1[j+1]))
          {
            // an horizontal neighbor connects to a right corner neighbor   00X
            //                                                              X1

            // this pixel can connect two different regions
            alias_set(alias_table, region, new_pmap1[j+1]);
          }

          if (connect == 8 && ((b11&&pmap1[j-1]) || (b12&&pmap1[j+1])) && !region) // isolated corner
          {
            // a left corner neighbor or a right corner neighbor  X0X
            //                                                    01

            if (b11&&pmap1[j-1])  // left corner
              region = new_pmap1[j-1];

            if (b12&&pmap1[j+1])  // right corner
            {
              if (!region) // isolated right corner
                region = new_pmap1[j+1];
              else
              {
                // this pixel can connect two different regions
                alias_set(alias_table, new_pmap1[j-1], new_pmap1[j+1]);
              }
            }
          }

          new_pmap[j] = region;
        }
        else
        {
          // this pixel touches no pixels

          // create a new region  000
          //                      01
          new_pmap[j] = (imushort)region_count;
          region_count++;

          if (region_count > MAX_COUNT)
          {
            delete [] alias_table;
            return -1;
          }
        }
      }
    }

    pmap1 = pmap;
    new_pmap1 = new_pmap;
    pmap += width;
    new_pmap += width;
  }

  // now all pixels are marked, 
  // but some marks are aliases to others

  // ajust the alias table to be a remap table
  // and return the real region count
  alias_update(alias_table, region_count);

  int count = width*height;
  for (i = 0; i < count; i++)
  {
    new_map[i] = alias_table[new_map[i]];
  }

  delete [] alias_table;

  return region_count;
}

int imAnalyzeFindRegions(const imImage* image, imImage* NewImage, int connect, int touch_border)
{
  imImageSetAttribute(NewImage, "REGION_CONNECT", IM_BYTE, 1, connect==4?"4":"8");
  if (touch_border)
    return DoAnalyzeFindRegionsBorder(image->width, image->height, (imbyte*)image->data[0], (imushort*)NewImage->data[0], connect);
  else
    return DoAnalyzeFindRegions(image->width, image->height, (imbyte*)image->data[0], (imushort*)NewImage->data[0], connect);
}

void imAnalyzeMeasureArea(const imImage* image, int* data_area, int region_count)
{
  imushort* img_data = (imushort*)image->data[0];

  memset(data_area, 0, region_count*sizeof(int));

  for (int i = 0; i < image->count; i++)
  {
    if (*img_data)
      data_area[(*img_data) - 1]++;
    img_data++;
  }
}

void imAnalyzeMeasureCentroid(const imImage* image, const int* data_area, int region_count, float* data_cx, float* data_cy)
{
  imushort* img_data = (imushort*)image->data[0];
  int* local_data_area = 0;

  if (!data_area)
  {
    local_data_area = (int*)malloc(region_count*sizeof(int));
    imAnalyzeMeasureArea(image, local_data_area, region_count);
    data_area = (const int*)local_data_area;
  }

  if (data_cx) memset(data_cx, 0, region_count*sizeof(float));
  if (data_cy) memset(data_cy, 0, region_count*sizeof(float));

  for (int y = 0; y < image->height; y++) 
  {
    int offset = y*image->width;

    for (int x = 0; x < image->width; x++)
    {
      int region_index = img_data[offset+x];
      if (region_index)
      {
        if (data_cx) data_cx[region_index-1] += (float)x;
        if (data_cy) data_cy[region_index-1] += (float)y;
      }
    }
  }

  for (int i = 0; i < region_count; i++) 
  {
    if (data_cx) data_cx[i] /= (float)data_area[i];
    if (data_cy) data_cy[i] /= (float)data_area[i];
  }

  if (local_data_area)
    free(local_data_area);
}

static inline double ipow(double x, int j)
{
	double r = 1.0;
	for (int i = 0; i < j; i++) 
    r *= x;
	return r;
}

static void iCalcMoment(double* cm, int px, int py, const imImage* image, const float* cx, const float* cy, int region_count)
{
  imushort* img_data = (imushort*)image->data[0];

  memset(cm, 0, region_count*sizeof(double));

  for (int y = 0; y < image->height; y++) 
  {
    int offset = y*image->width;

    for (int x = 0; x < image->width; x++)
    {
      int region_index = img_data[offset+x];
      if (region_index)
      {
        int i = region_index-1;

        if (px == 0)
          cm[i] += ipow(y-cy[i],py);
        else if (py == 0)
          cm[i] += ipow(x-cx[i],px);
        else
          cm[i] += ipow(x-cx[i],px)*ipow(y-cy[i],py);
      }
    }
  }
}

template<class T>
static inline int IsPerimeterPoint(T* map, int width, int height, int x, int y)
{
  // map here points to the start of the line, even if its an invalid line.

  // if outside the image, then is not a perimeter line.
  if (x == -1 || x == width ||
      y == -1 || y == height)
    return 0;

  T v = map[x]; // here v is image(x,y)
  if (!v)
    return 0;

  // if touches the border, then is a perimeter line.
  if (x == 0 || x == width-1 ||
      y == 0 || y == height-1)
    return 1;

  // if has 4 connected neighbors, then is a perimeter line.
  if (map[width+x] != v ||
      map[x+1] != v ||
      map[x-1] != v ||
      map[-width+x] != v)
    return 1;

  return 0;
}

void imAnalyzeMeasurePrincipalAxis(const imImage* image, const int* data_area, const float* data_cx, const float* data_cy, 
                                   const int region_count, float* major_slope, float* major_length, 
                                                           float* minor_slope, float* minor_length)
{
  int i;
  int *local_data_area = 0;
  float *local_data_cx = 0, *local_data_cy = 0;

  if (!data_area)
  {
    local_data_area = (int*)malloc(region_count*sizeof(int));
    imAnalyzeMeasureArea(image, local_data_area, region_count);
    data_area = (const int*)local_data_area;
  }

  if (!data_cx || !data_cy)
  {
    if (!data_cx)
    {
      local_data_cx = (float*)malloc(region_count*sizeof(float));
      data_cx = (const float*)local_data_cx;
    }

    if (!data_cy)
    {
      local_data_cy = (float*)malloc(region_count*sizeof(float));
      data_cy = (const float*)local_data_cy;
    }

    if (local_data_cx && local_data_cy)
      imAnalyzeMeasureCentroid(image, data_area, region_count, local_data_cx, local_data_cy);
    else if (local_data_cx)
      imAnalyzeMeasureCentroid(image, data_area, region_count, local_data_cx, NULL);
    else if (local_data_cy)
      imAnalyzeMeasureCentroid(image, data_area, region_count, NULL, local_data_cy);
  }

  // additional moments
  double* cm20 = (double*)malloc(region_count*sizeof(double));
  double* cm02 = (double*)malloc(region_count*sizeof(double));
  double* cm11 = (double*)malloc(region_count*sizeof(double));
  
  iCalcMoment(cm20, 2, 0, image, data_cx, data_cy, region_count);
  iCalcMoment(cm02, 0, 2, image, data_cx, data_cy, region_count);
  iCalcMoment(cm11, 1, 1, image, data_cx, data_cy, region_count);

  float *local_major_slope = 0, *local_minor_slope = 0;
  if (!major_slope)
  {
    local_major_slope = (float*)malloc(region_count*sizeof(float));
    major_slope = local_major_slope;
  }
  if (!minor_slope)
  {
    local_minor_slope = (float*)malloc(region_count*sizeof(float));
    minor_slope = local_minor_slope;
  }

#define RAD2DEG  57.296

  // We are going to find 2 axis parameters.
  // Axis 1 are located in quadrants 1-3
  // Axis 2 are located in quadrants 2-4

  // Quadrants
  //    2 | 1
  //    -----
  //    3 | 4

  // line coeficients for lines that belongs to axis 1 and 2
  float* A1 = (float*)malloc(region_count*sizeof(float));
  float* A2 = (float*)malloc(region_count*sizeof(float));
  float* C1 = (float*)malloc(region_count*sizeof(float));
  float* C2 = (float*)malloc(region_count*sizeof(float));

  float *slope1 = major_slope; // Use major_slope as a storage place, 
  float *slope2 = minor_slope; // and create an alias to make code clear.

  for (i = 0; i < region_count; i++) 
  {
    if (cm11[i] == 0)
    {
      slope1[i] = 0;
      slope2[i] = 90;

      // These should not be used
      A1[i] = 0; 
      A2[i] = 0;  // infinite
      C1[i] = 0;  // data_cy[i]
      C2[i] = 0;  
    }
    else
    {
      double b = (cm20[i] - cm02[i])/cm11[i];
      double delta = sqrt(b*b + 4.0);
      double r1 = (-b-delta)/2.0;
      double r2 = (-b+delta)/2.0;
      float a1 = (float)(atan(r1)*RAD2DEG + 90);  // to avoid negative results
      float a2 = (float)(atan(r2)*RAD2DEG + 90);

      if (a1 == 180) a1 = 0;
      if (a2 == 180) a2 = 0;

      if (a1 < 90)             // a1 is quadrants q1-q3
      {                        
        slope1[i] = a1;   
        slope2[i] = a2;   
        A1[i] = (float)r1;
        A2[i] = (float)r2;
      }
      else                     // a2 is quadrants q1-q3
      {
        slope1[i] = a2;
        slope2[i] = a1;
        A1[i] = (float)r2;
        A2[i] = (float)r1;
      }

      C1[i] = data_cy[i] - A1[i] * data_cx[i];
      C2[i] = data_cy[i] - A2[i] * data_cx[i];
    }
  }

  // moments are not necessary anymore
  free(cm20); free(cm02); free(cm11);
  cm20 = 0; cm02 = 0; cm11 = 0;

  // maximum distance from a point in the perimeter to an axis in each side of the axis
  // D1 is distance to axis 1, a and b are sides
  float* D1a = (float*)malloc(region_count*sizeof(float));
  float* D1b = (float*)malloc(region_count*sizeof(float));
  float* D2a = (float*)malloc(region_count*sizeof(float));
  float* D2b = (float*)malloc(region_count*sizeof(float));
  memset(D1a, 0, region_count*sizeof(float));
  memset(D1b, 0, region_count*sizeof(float));
  memset(D2a, 0, region_count*sizeof(float));
  memset(D2b, 0, region_count*sizeof(float));

  imushort* img_data = (imushort*)image->data[0];
  int width = image->width;
  int height = image->height;
  for (int y = 0; y < height; y++) 
  {
    int offset = y*width;

    for (int x = 0; x < width; x++)
    {
      if (IsPerimeterPoint(img_data+offset, width, height, x, y))
      {
        i = img_data[offset+x] - 1;

        float d1, d2;
        if (slope2[i] == 90)
        {
          d2 = y - data_cy[i];   // I ckecked this many times, looks odd but it is correct.
          d1 = x - data_cx[i];
        }
        else
        {
          d1 = A1[i]*x - y + C1[i];
          d2 = A2[i]*x - y + C2[i];
        }

        if (d1 < 0)
        {
          d1 = (float)fabs(d1);
          if (d1 > D1a[i])         
            D1a[i] = d1;
        }
        else
        {
          if (d1 > D1b[i])
            D1b[i] = d1;
        }

        if (d2 < 0)
        {
          d2 = (float)fabs(d2);
          if (d2 > D2a[i])         
            D2a[i] = d2;
        }
        else
        {
          if (d2 > D2b[i])
            D2b[i] = d2;
        }
      }
    }
  }

  for (i = 0; i < region_count; i++) 
  {
    float AB1 = (float)sqrt(A1[i]*A1[i] + 1);
    float AB2 = (float)sqrt(A2[i]*A2[i] + 1);

    float D1 = (D1a[i] + D1b[i]) / AB1; 
    float D2 = (D2a[i] + D2b[i]) / AB2;

    if (D1 < D2) // Major Axis in 2-4 quadrants
    {
      // now remember that we did an alias before
      // slope1 -> major_slope
      // slope2 -> minor_slope

      float tmp = major_slope[i];
      major_slope[i] = minor_slope[i];
      minor_slope[i] = tmp;

      if (minor_length) minor_length[i] = D1;
      if (major_length) major_length[i] = D2;
    }
    else
    {
      if (minor_length) minor_length[i] = D2;
      if (major_length) major_length[i] = D1;
    }
  }

  if (local_major_slope) free(local_major_slope);
  if (local_minor_slope) free(local_minor_slope);
  if (local_data_area) free(local_data_area);
  if (local_data_cx) free(local_data_cx);
  if (local_data_cy) free(local_data_cy);

  free(A1);  
  free(A2);  
  free(C1);  
  free(C2);

  free(D1b); 
  free(D2b);
  free(D1a); 
  free(D2a); 
}

void imAnalyzeMeasureHoles(const imImage* image, int connect, int* count_data, int* area_data, float* perim_data)
{
  int i;
  imImage *inv_image = imImageCreate(image->width, image->height, IM_BINARY, IM_BYTE);
  imbyte* inv_data = (imbyte*)inv_image->data[0];
  imushort* img_data = (imushort*)image->data[0];

  // finds the holes in the inverted image
  for (i = 0; i < image->count; i++)
  {
    if (*img_data)
      *inv_data = 0;
    else
      *inv_data = 1;

    img_data++;
    inv_data++;
  }

  imImage *holes_image = imImageClone(image);
  if (!holes_image)
    return;

  int holes_count = imAnalyzeFindRegions(inv_image, holes_image, connect, 0);
  imImageDestroy(inv_image);

  if (!holes_count)
  {
    imImageDestroy(holes_image);
    return;
  }

  // measure the holes area
  int* holes_area = (int*)malloc(holes_count*sizeof(int));
  imAnalyzeMeasureArea(holes_image, holes_area, holes_count);

  float* holes_perim = 0;
  if (perim_data) 
  {
    holes_perim = (float*)malloc(holes_count*sizeof(int));
    imAnalyzeMeasurePerimeter(holes_image, holes_perim, holes_count);
  }

  imushort* holes_data = (imushort*)holes_image->data[0];
  img_data = (imushort*)image->data[0];

  // holes do not touch the border
  for (int y = 1; y < image->height-1; y++) 
  {
    int offset_up = (y+1)*image->width;
    int offset = y*image->width;
    int offset_dw = (y-1)*image->width;

    for (int x = 1; x < image->width-1; x++)
    {
      int hole_index = holes_data[offset+x];

      if (hole_index && holes_area[hole_index-1]) // a hole not yet used
      {
        // if the hole has not been used, 
        // it is the first time we encounter a pixel of this hole.
        // then it is a pixel from the hole border.
        // now find which region this hole is inside.
        // a 4 connected neighbour is necessarilly a valid region or 0.

        int region_index = 0;
        if (img_data[offset_up + x]) region_index = img_data[offset_up + x];
        else if (img_data[offset + x+1]) region_index = img_data[offset + x+1];
        else if (img_data[offset + x-1]) region_index = img_data[offset + x-1]; 
        else if (img_data[offset_dw+x]) region_index = img_data[offset_dw+x];

        if (!region_index) continue;

        if (count_data) count_data[region_index-1]++;
        if (area_data) area_data[region_index-1] += holes_area[hole_index-1];
        if (perim_data) perim_data[region_index-1] += holes_perim[hole_index-1];
        holes_area[hole_index-1] = 0; // mark hole as used
      }
    }
  }

  if (holes_perim) free(holes_perim);
  free(holes_area);
  imImageDestroy(holes_image);
}

template<class T>
static void DoPerimeterLine(T* map, T* new_map, int width, int height)
{
  int x, y, offset;

  for (y = 0; y < height; y++) 
  {
    offset = y*width;

    for (x = 0; x < width; x++)
    {
      if (IsPerimeterPoint(map+offset, width, height, x, y))
        new_map[offset+x] = map[offset+x];
      else
        new_map[offset+x] = 0;
    }
  }
}

void imProcessPerimeterLine(const imImage* src_image, imImage* dst_image)
{
  switch(src_image->data_type)
  {
  case IM_BYTE:
    DoPerimeterLine((imbyte*)src_image->data[0], (imbyte*)dst_image->data[0], src_image->width, src_image->height);
    break;                                                                                
  case IM_USHORT:
    DoPerimeterLine((imushort*)src_image->data[0], (imushort*)dst_image->data[0], src_image->width, src_image->height);
    break;                                                                                
  case IM_INT:                                                                           
    DoPerimeterLine((int*)src_image->data[0], (int*)dst_image->data[0], src_image->width, src_image->height);
    break;                                                                                
  }
}

/* Perimeter Templates idea based in
   Parker, Pratical Computer Vision Using C

For 1.414 (sqrt(2)/2 + sqrt(2)/2) [1]:
     1 0 0   0 0 1   1 0 0   0 0 1   0 0 0   1 0 1
     0 x 0   0 x 0   0 x 0   0 x 0   0 x 0   0 x 0
     0 0 1   1 0 0   1 0 0   0 0 1   1 0 1   0 0 0
      129      36     132      33      5      160

For 1.207 (sqrt(2)/2 + 1.0/2) [2]:
     0 0 0   0 0 1   0 1 0   0 1 0   1 0 0   0 0 1   0 0 0   1 0 0
     1 x 0   1 x 0   0 x 0   0 x 0   0 x 0   0 x 0   0 x 1   0 x 1
     0 0 1   0 0 0   1 0 0   0 0 1   0 1 0   0 1 0   1 0 0   0 0 0
       17      48      68      65     130      34       12     136

     0 0 0   1 0 0   1 1 0   0 1 1   0 0 1   0 0 0   0 0 0   0 0 0
     1 x 0   1 x 0   0 x 0   0 x 0   0 x 1   0 x 1   0 x 0   0 x 0
     1 0 0   0 0 0   0 0 0   0 0 0   0 0 0   0 0 1   0 1 1   1 1 0
       20     144     192      96      40      9       3       6

For 1.0 (1.0/2 + 1.0/2) [0]:
     0 0 0   0 1 0   0 0 0   0 0 0   0 1 0   0 1 0
     1 x 1   0 x 0   1 x 0   0 x 1   1 x 0   0 x 1
     0 0 0   0 1 0   0 1 0   0 1 0   0 0 0   0 0 0
       24      66      18      10      80      72

For 0.707 (sqrt(2)/2) [3]:
     1 0 0   0 0 1   0 0 0   0 0 0
     0 x 0   0 x 0   0 x 0   0 x 0         (For Line Length)
     0 0 0   0 0 0   0 0 1   1 0 0
      128      32      1       4

For 0.5 (1.0/2) [4]:
     0 1 0   0 0 0   0 0 0   0 0 0
     0 x 0   0 x 1   0 x 0   1 x 0         (For Line Length)
     0 0 0   0 0 0   0 1 0   0 0 0
       64      8       2      16

*/
static void iInitPerimTemplate(imbyte *templ, float *v)
{
  memset(templ, 0, 256);

  templ[129] = 1;
  templ[36]  = 1;
  templ[132] = 1;
  templ[33]  = 1;
  templ[5]   = 1;
  templ[160] = 1;

  templ[17]  = 2;
  templ[48]  = 2;
  templ[68]  = 2;
  templ[65]  = 2;
  templ[130] = 2;
  templ[34]  = 2;
  templ[12]  = 2;
  templ[136] = 2;
  templ[20]  = 2;
  templ[144] = 2;
  templ[192] = 2;
  templ[96]  = 2;
  templ[40]  = 2;
  templ[9]   = 2;
  templ[3]   = 2;
  templ[6]   = 2;

  templ[24] = 0;
  templ[66] = 0;
  templ[18] = 0;
  templ[10] = 0;
  templ[80] = 0;
  templ[72] = 0;

  templ[128] = 3;
  templ[32]  = 3;
  templ[1]   = 3;
  templ[4]   = 3;

  templ[64] = 4;
  templ[8]  = 4;
  templ[2]  = 4;
  templ[16] = 4;

const float DT_SQRT2   = 1.414213562373f;
const float DT_SQRT2D2 = 0.707106781187f;

  v[1] = DT_SQRT2;   
  v[2] = DT_SQRT2D2 + 0.5f;   
  v[0] = 1.0f;
  v[3] = DT_SQRT2D2;
  v[4] = 0.5f;
}

void imAnalyzeMeasurePerimeter(const imImage* image, float* perim_data, int region_count)
{
  static imbyte templ[256];
  static float vt[5];
  static int first = 1;
  if (first)
  {
    iInitPerimTemplate(templ, vt);
    first = 0;
  }

  imushort* map = (imushort*)image->data[0];

  memset(perim_data, 0, region_count*sizeof(float));

  int width = image->width;
  int height = image->height;
  for (int y = 0; y < height; y++) 
  {
    int offset = y*image->width;

    for (int x = 0; x < width; x++)
    {
      if (IsPerimeterPoint(map+offset, width, height, x, y))
      {
        int T = 0;

        // check the 8 neighboors if they belong to the perimeter
        if (IsPerimeterPoint(map+offset+width, width, height, x-1, y+1))
          T |= 0x01;
        if (IsPerimeterPoint(map+offset+width, width, height, x, y+1))
          T |= 0x02;
        if (IsPerimeterPoint(map+offset+width, width, height, x+1, y+1))
          T |= 0x04;

        if (IsPerimeterPoint(map+offset, width, height, x-1, y))
          T |= 0x08;
        if (IsPerimeterPoint(map+offset, width, height, x+1, y))
          T |= 0x10;

        if (IsPerimeterPoint(map+offset-width, width, height, x-1, y-1))
          T |= 0x20;
        if (IsPerimeterPoint(map+offset-width, width, height, x, y-1))
          T |= 0x40;
        if (IsPerimeterPoint(map+offset-width, width, height, x+1, y-1))
          T |= 0x80;

        if (T)
          perim_data[map[offset+x] - 1] += vt[templ[T]];
      }
    }
  }
}

/* Perimeter Area Templates

For "1.0" (0):

     1 1 1
     1 x 1
     1 1 1
      255

For "0.75" (1):

     1 1 1   1 1 1   0 1 1   1 1 0   1 1 1   1 1 1   1 1 1   1 0 1
     1 x 1   1 x 1   1 x 1   1 x 1   0 x 1   1 x 0   1 x 1   1 x 1
     0 1 1   1 1 0   1 1 1   1 1 1   1 1 1   1 1 1   1 0 1   1 1 1
      251     254     127     223     239     247     253     191

For "0.625" (2):

     1 1 1   0 0 1   0 1 1   1 1 0   1 1 1   1 1 1   1 1 1   1 0 0
     1 x 1   1 x 1   0 x 1   1 x 0   0 x 1   1 x 0   1 x 1   1 x 1
     0 0 1   1 1 1   1 1 1   1 1 1   0 1 1   1 1 0   1 0 0   1 1 1
      249     63      111     215     235     246     252     159

For "0.5" (3):

     0 0 0   0 1 1   1 1 1   1 1 0   1 1 1   0 0 1   1 0 0   1 1 1  
     1 x 1   0 x 1   1 x 1   1 x 0   0 x 1   0 x 1   1 x 0   1 x 0  
     1 1 1   0 1 1   0 0 0   1 1 0   0 0 1   1 1 1   1 1 1   1 0 0  
      31      107     248     214     233     47      151     244

For "0.375" (4):

     0 0 0   1 1 1   1 1 0   0 1 1   1 0 0   0 0 1   0 0 0   1 1 1
     1 x 0   1 x 0   1 x 0   0 x 1   1 x 0   0 x 1   0 x 1   0 x 1
     1 1 1   0 0 0   1 0 0   0 0 1   1 1 0   0 1 1   1 1 1   0 0 0
      23      240     212     105     150     43      15      232

For "0.25" (5):

     0 0 0   0 0 0   1 1 0   0 1 1   1 0 0   0 0 1   0 0 0   1 1 1
     1 x 0   0 x 1   1 x 0   0 x 1   1 x 0   0 x 1   0 x 0   0 x 0
     1 1 0   0 1 1   0 0 0   0 0 0   1 0 0   0 0 1   1 1 1   0 0 0
      22      11      208     104     148     41       7      224

For "0.125" (6):

     0 0 0   0 0 0   1 1 0   0 0 1   1 0 0   0 0 0   0 0 0   0 1 1
     1 x 0   0 x 0   0 x 0   0 x 1   1 x 0   0 x 1   0 x 0   0 x 0
     1 0 0   0 1 1   0 0 0   0 0 0   0 0 0   0 0 1   1 1 0   0 0 0
      20       3      192      40     144      9       6       96

*/
static void iInitPerimAreaTemplate(imbyte *templ, float *v)
{
  memset(templ, 0, 256);

  templ[255] = 0;

  templ[251] = 1;
  templ[254] = 1;
  templ[127] = 1;
  templ[223] = 1;
  templ[239] = 1;
  templ[247] = 1;
  templ[253] = 1;
  templ[191] = 1;
        
  templ[249] = 2;
  templ[63] = 2;
  templ[111] = 2;
  templ[215] = 2;
  templ[235] = 2;
  templ[246] = 2;
  templ[252] = 2;
  templ[159] = 2;
        
  templ[31] = 3;
  templ[107] = 3;
  templ[248] = 3;
  templ[214] = 3;
  templ[233] = 3;
  templ[47] = 3;
  templ[151] = 3;
  templ[244] = 3;
        
  templ[23] = 4;
  templ[240] = 4;
  templ[212] = 4;
  templ[105] = 4;
  templ[150] = 4;
  templ[43] = 4;
  templ[15] = 4;
  templ[232] = 4;
        
  templ[22] = 5;
  templ[11] = 5;
  templ[208] = 5;
  templ[104] = 5;
  templ[148] = 5;
  templ[41] = 5;
  templ[7] = 5;
  templ[224] = 5;
        
  templ[20] = 6;
  templ[3] = 6;
  templ[192] = 6;
  templ[40] = 6;
  templ[144] = 6;
  templ[9] = 6;
  templ[6] = 6;
  templ[96] = 6;

  v[0] = 1.0f;
  v[1] = 0.75f;  
  v[2] = 0.625f;  
  v[3] = 0.5f;
  v[4] = 0.375f;
  v[5] = 0.25f;
  v[6] = 0.125f;
}

void imAnalyzeMeasurePerimArea(const imImage* image, float* area_data)
{
  static imbyte templ[256];
  static float vt[7];
  static int first = 1;
  if (first)
  {
    iInitPerimAreaTemplate(templ, vt);
    first = 0;
  }

  imushort* map = (imushort*)image->data[0];

  int width = image->width;
  int height = image->height;
  for (int y = 0; y < height; y++) 
  {
    int offset_up = (y+1)*width;
    int offset = y*width;
    int offset_dw = (y-1)*width;

    for (int x = 0; x < width; x++)
    {
      imushort v = map[offset+x];
      if (v)
      {
        int T = 0;
        if (x>0 && y<height-1 &&       map[offset_up + x-1] == v) T |= 0x01;
        if (y<height-1 &&              map[offset_up + x  ] == v) T |= 0x02;
        if (x<width-1 && y<height-1 && map[offset_up + x+1] == v) T |= 0x04;
        if (x>0 &&                     map[offset    + x-1] == v) T |= 0x08;
        if (x<width-1 &&               map[offset    + x+1] == v) T |= 0x10; 
        if (x>0 && y>0 &&              map[offset_dw + x-1] == v) T |= 0x20;
        if (y>0 &&                     map[offset_dw + x  ] == v) T |= 0x40;
        if (x<width-1 && y>0 &&        map[offset_dw + x+1] == v) T |= 0x80;

        if (T)
          area_data[v-1] += vt[templ[T]];
      }
    }
  }
}

void imProcessRemoveByArea(const imImage* image, imImage* NewImage, int connect, int start_size, int end_size, int inside)
{
  imImage *region_image = imImageCreate(image->width, image->height, IM_GRAY, IM_USHORT);
  if (!region_image)
    return;

  int region_count = imAnalyzeFindRegions(image, region_image, connect, 1); 
  if (!region_count)
  {
    imImageClear(NewImage);
    imImageDestroy(region_image);
    return;
  }

  if (end_size == 0)
    end_size = image->width*image->height;

  int outside;
  if (inside)
  {
    /* remove from inside */
    inside = 0;
    outside = 1;
  }
  else
  {
    /* remove from outside */
    inside = 1;
    outside = 0;
  }

  int* area_data = (int*)malloc(region_count*sizeof(int));
  imAnalyzeMeasureArea(region_image, area_data, region_count);

  imushort* region_data = (imushort*)region_image->data[0];
  imbyte* img_data = (imbyte*)NewImage->data[0];

  for (int i = 0; i < image->count; i++)
  {
    if (*region_data)
    {
      int area = area_data[(*region_data) - 1];
      if (area < start_size || area > end_size)
        *img_data = (imbyte)outside;
      else
        *img_data = (imbyte)inside;
    }
    else
      *img_data = 0;

    region_data++;
    img_data++;
  }

  free(area_data);
  imImageDestroy(region_image);
}

void imProcessFillHoles(const imImage* image, imImage* NewImage, int connect)
{
  // finding regions in the inverted image will isolate only the holes.
  imProcessNegative(image, NewImage);

  imImage *region_image = imImageCreate(image->width, image->height, IM_GRAY, IM_USHORT);
  if (!region_image)
    return;

  int holes_count = imAnalyzeFindRegions(NewImage, region_image, connect, 0);
  if (!holes_count)
  {
    imImageCopy(image, NewImage);
    imImageDestroy(region_image);
    return;
  }

  imushort* region_data = (imushort*)region_image->data[0];
  imbyte* dst_data = (imbyte*)NewImage->data[0];

  for (int i = 0; i < image->count; i++)
  {
    if (*region_data)
      *dst_data = 1;
    else
      *dst_data = !(*dst_data);  // Fix negative data.

    region_data++;
    dst_data++;
  }

  imImageDestroy(region_image);
}
