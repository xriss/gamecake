/** \file
 * \brief cdBitmap implementation
 *
 * See Copyright Notice in cd.h
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <memory.h>
#include <math.h>
#include <stdarg.h>


#include "cd.h"
#include "cdirgb.h"

#include "cd_private.h"


typedef struct _cdBitmapData 
{
  void *buffer;

  unsigned char *index;  /* pointers into buffer */
  unsigned char *r;
  unsigned char *g;
  unsigned char *b;
  unsigned char *a;

  long* colors;

  int xmin, xmax, ymin, ymax;
} cdBitmapData;


cdBitmap* cdCreateBitmap(int w, int h, int type)
{
  int num_channel;
  int size = w * h;
  cdBitmap* bitmap;
  cdBitmapData* data;

  assert(w>0);
  assert(h>0);
  if (w <= 0) return NULL;
  if (h <= 0) return NULL;

  switch (type)
  {
  case CD_RGB:
    num_channel = 3;
    break;
  case CD_RGBA:
    num_channel = 4;
    break;
  case CD_MAP:
    num_channel = 1;
    break;
  default:
    return NULL;
  }

  bitmap = (cdBitmap*)malloc(sizeof(cdBitmap));
  data = (cdBitmapData*)malloc(sizeof(cdBitmapData));
  memset(data, 0, sizeof(cdBitmapData));

  bitmap->w = w;
  bitmap->h = h;
  bitmap->type = type;
  bitmap->data = data;

  data->buffer = malloc(size*num_channel);
  if (!data->buffer)
  {
    free(data);
    free(bitmap);
    return NULL;
  }

  if (type == CD_RGB)
  {
    data->r = data->buffer;
    data->g = data->r + size;
    data->b = data->g + size;
    memset(data->r, 255, 3*size);    /* white */
  }

  if (type == CD_RGBA)
  {
    data->a = data->b + size;
    memset(data->a, 0, size);    /* transparent */
  }

  if (type == CD_MAP)
  {
    data->index = data->buffer;
    data->colors = (long*)calloc(256, sizeof(long));
    memset(data->index, 0, size);    /* index=0 */
  }

  data->xmin = 0;
  data->ymin = 0;
  data->xmax = bitmap->w-1;
  data->ymax = bitmap->h-1;

  return bitmap;
}

cdBitmap* cdInitBitmap(int w, int h, int type, ...)
{
  va_list arglist;
  cdBitmap* bitmap;
  cdBitmapData* data;

  assert(w>0);
  assert(h>0);
  if (w <= 0) return NULL;
  if (h <= 0) return NULL;

  if (type != CD_RGB && 
      type != CD_RGBA &&
      type != CD_MAP)
    return NULL;

  bitmap = (cdBitmap*)malloc(sizeof(cdBitmap));
  data = (cdBitmapData*)malloc(sizeof(cdBitmapData));
  memset(data, 0, sizeof(cdBitmapData));

  bitmap->w = w;
  bitmap->h = h;
  bitmap->type = type;
  bitmap->data = data;

  va_start(arglist, type);

  if (type == CD_RGB)
  {
    data->r = va_arg(arglist, unsigned char*);
    data->g = va_arg(arglist, unsigned char*);
    data->b = va_arg(arglist, unsigned char*);
  }

  if (type == CD_RGBA)
    data->a = va_arg(arglist, unsigned char*);

  if (type == CD_MAP)
  {
    data->index = va_arg(arglist, unsigned char*);
    data->colors = va_arg(arglist, long*);
  }

  data->xmin = 0;
  data->ymin = 0;
  data->xmax = bitmap->w-1;
  data->ymax = bitmap->h-1;

  return bitmap;
}

void cdKillBitmap(cdBitmap* bitmap)
{
  cdBitmapData* data;

  assert(bitmap);
  assert(bitmap->data);
  if (!bitmap) return;
  if (!bitmap->data) return;

  data = (cdBitmapData*)bitmap->data;

  if (data->buffer)
  {
    free(data->buffer);

    if (bitmap->type == CD_MAP)
      free(data->colors);
  }

  free(data);
  free(bitmap);
}

void cdCanvasGetBitmap(cdCanvas* canvas, cdBitmap* bitmap, int x, int y)
{
  cdBitmapData* data;

  assert(bitmap);
  assert(bitmap->data);
  if (!bitmap) return;
  if (!bitmap->data) return;

  data = (cdBitmapData*)bitmap->data;

  if (bitmap->type == CD_RGB || bitmap->type == CD_RGBA)
    cdCanvasGetImageRGB(canvas, data->r, data->g, data->b, x, y, bitmap->w, bitmap->h);
}

void cdBitmapRGB2Map(cdBitmap* bitmap_rgb, cdBitmap* bitmap_map)
{
  cdBitmapData* data_rgb;
  cdBitmapData* data_map;

  assert(bitmap_rgb);
  assert(bitmap_rgb->data);
  assert(bitmap_map);
  assert(bitmap_map->data);
  if (!bitmap_rgb) return;
  if (!bitmap_rgb->data) return;
  if (!bitmap_map) return;
  if (!bitmap_map->data) return;

  data_rgb = (cdBitmapData*)bitmap_rgb->data;
  data_map = (cdBitmapData*)bitmap_map->data;

  if ((bitmap_rgb->type != CD_RGB && bitmap_rgb->type != CD_RGBA) || (bitmap_map->type != CD_MAP))
    return;

  cdRGB2Map(bitmap_rgb->w, bitmap_rgb->h, data_rgb->r, data_rgb->g, data_rgb->b, data_map->index, bitmap_map->type, data_map->colors);
}

unsigned char* cdBitmapGetData(cdBitmap* bitmap, int dataptr)
{
  cdBitmapData* data;

  assert(bitmap);
  assert(bitmap->data);
  if (!bitmap) return NULL;
  if (!bitmap->data) return NULL;

  data = (cdBitmapData*)bitmap->data;

  switch(dataptr)
  {
  case CD_IRED:
    return data->r;
  case CD_IGREEN:
    return data->g;
  case CD_IBLUE:
    return data->b;
  case CD_IALPHA:
    return data->a;
  case CD_INDEX:
    return data->index;
  case CD_COLORS:
    return (unsigned char*)data->colors;
  }

  return NULL;
}

void cdBitmapSetRect(cdBitmap* bitmap, int xmin, int xmax, int ymin, int ymax)
{
  cdBitmapData* data;

  assert(bitmap);
  assert(bitmap->data);
  if (!bitmap) return;
  if (!bitmap->data) return;

  data = (cdBitmapData*)bitmap->data;

  data->xmin = xmin;
  data->xmax = xmax;
  data->ymin = ymin;
  data->ymax = ymax;
}

void cdCanvasPutBitmap(cdCanvas* canvas, cdBitmap* bitmap, int x, int y, int w, int h)
{
  cdBitmapData* data;

  assert(bitmap);
  assert(bitmap->data);
  if (!bitmap) return;
  if (!bitmap->data) return;

  data = (cdBitmapData*)bitmap->data;

  switch(bitmap->type)
  {
  case CD_RGB:
    cdCanvasPutImageRectRGB(canvas, bitmap->w, bitmap->h, data->r, data->g, data->b, x, y, w, h, data->xmin, data->xmax, data->ymin, data->ymax);
    break;
  case CD_RGBA:
    cdCanvasPutImageRectRGBA(canvas, bitmap->w, bitmap->h, data->r, data->g, data->b, data->a, x, y, w, h, data->xmin, data->xmax, data->ymin, data->ymax);
    break;
  case CD_MAP:
    cdCanvasPutImageRectMap(canvas, bitmap->w, bitmap->h, data->index, data->colors, x, y, w, h, data->xmin, data->xmax, data->ymin, data->ymax);
    break;
  }
}
