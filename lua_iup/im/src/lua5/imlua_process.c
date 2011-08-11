/** \file
 * \brief IM Lua 5 Binding
 *
 * See Copyright Notice in im_lib.h
 * $Id: imlua_process.c,v 1.13 2010/06/10 20:17:40 scuri Exp $
 */

#include <memory.h>
#include <math.h>
#include <stdlib.h>

#include "im.h"
#include "im_image.h"
#include "im_process.h"
#include "im_util.h"

#include <lua.h>
#include <lauxlib.h>

#include "imlua.h"
#include "imlua_aux.h"
#include "imlua_image.h"



/*****************************************************************************\
 Image Statistics Calculations
\*****************************************************************************/

/*****************************************************************************\
 im.CalcRMSError(image1, image2)
\*****************************************************************************/
static int imluaCalcRMSError (lua_State *L)
{
  imImage* image1 = imlua_checkimage(L, 1);
  imImage* image2 = imlua_checkimage(L, 2);

  imlua_match(L, image1, image2);

  lua_pushnumber(L, imCalcRMSError(image1, image2));
  return 1;
}

/*****************************************************************************\
 im.CalcSNR(src_image, noise_image)
\*****************************************************************************/
static int imluaCalcSNR (lua_State *L)
{
  imImage* src_image = imlua_checkimage(L, 1);
  imImage* noise_image = imlua_checkimage(L, 2);

  imlua_match(L, src_image, noise_image);

  lua_pushnumber(L, imCalcSNR(src_image, noise_image));
  return 1;
}

/*****************************************************************************\
 im.CalcCountColors(src_image)
\*****************************************************************************/
static int imluaCalcCountColors (lua_State *L)
{
  imImage* src_image = imlua_checkimage(L, 1);

  imlua_checkdatatype(L, 1, src_image, IM_BYTE);
  if (src_image->color_space >= IM_CMYK)
    luaL_argerror(L, 1, "color space can be RGB, Gray, Binary or Map only");

  lua_pushnumber(L, imCalcCountColors(src_image));
  return 1;
}

/*****************************************************************************\
 im.CalcHistogram(src_image, plane, cumulative)
\*****************************************************************************/
static int imluaCalcHistogram (lua_State *L)
{
  imImage* src_image = imlua_checkimage(L, 1);
  int plane = luaL_checkint(L, 2);
  int cumulative = lua_toboolean(L, 3);

  switch (src_image->data_type)
  {
  case IM_BYTE:
    {
      unsigned long hist[256];
      imCalcHistogram((imbyte*) src_image->data[plane], src_image->count, hist, cumulative);
      imlua_newarrayulong(L, hist, 256, 0);
    }
    break;

  case IM_USHORT:
    {
      unsigned long hist[65535];
      imCalcUShortHistogram(src_image->data[plane], src_image->count, hist, cumulative);
      imlua_newarrayulong(L, hist, 65535, 0);
    }
    break;

  default:
    luaL_argerror(L, 1, "data_type can be byte or ushort only");
    break;
  }

  return 1;
}

/*****************************************************************************\
 im.CalcGrayHistogram(src_image, cumulative)
\*****************************************************************************/
static int imluaCalcGrayHistogram (lua_State *L)
{
  unsigned long hist[256];
  imImage* src_image = imlua_checkimage(L, 1);
  int cumulative = lua_toboolean(L, 2);

  imlua_checkdatatype(L, 1, src_image, IM_BYTE);
  if (src_image->color_space >= IM_CMYK)
    luaL_argerror(L, 1, "color space can be RGB, Gray, Binary or Map only");

  imCalcGrayHistogram(src_image, hist, cumulative);
  imlua_newarrayulong(L, hist, 256, 0);

  return 1;
}

/*****************************************************************************\
 im.CalcImageStatistics(src_image)
\*****************************************************************************/
static int imluaCalcImageStatistics (lua_State *L)
{
  imStats stats;
  imImage *image = imlua_checkimage(L, 1);

  if (image->data_type == IM_CFLOAT)
    luaL_argerror(L, 1, "data type can NOT be of type cfloat");

  imCalcImageStatistics(image, &stats);

  lua_newtable(L);
  lua_pushstring(L, "max");      lua_pushnumber(L, stats.max);      lua_settable(L, -3);
  lua_pushstring(L, "min");      lua_pushnumber(L, stats.min);      lua_settable(L, -3);
  lua_pushstring(L, "positive"); lua_pushnumber(L, stats.positive); lua_settable(L, -3);
  lua_pushstring(L, "negative"); lua_pushnumber(L, stats.negative); lua_settable(L, -3);
  lua_pushstring(L, "zeros");    lua_pushnumber(L, stats.zeros);    lua_settable(L, -3);
  lua_pushstring(L, "mean");     lua_pushnumber(L, stats.mean);     lua_settable(L, -3);
  lua_pushstring(L, "stddev");   lua_pushnumber(L, stats.stddev);   lua_settable(L, -3);
  return 1;
}

/*****************************************************************************\
 im.CalcHistogramStatistics(src_image)
\*****************************************************************************/
static int imluaCalcHistogramStatistics (lua_State *L)
{
  imStats stats;
  imImage *image = imlua_checkimage(L, 1);

  imlua_checkdatatype(L, 1, image, IM_BYTE);

  imCalcHistogramStatistics(image, &stats);

  lua_newtable(L);
  lua_pushstring(L, "max");      lua_pushnumber(L, stats.max);      lua_settable(L, -3);
  lua_pushstring(L, "min");      lua_pushnumber(L, stats.min);      lua_settable(L, -3);
  lua_pushstring(L, "positive"); lua_pushnumber(L, stats.positive); lua_settable(L, -3);
  lua_pushstring(L, "negative"); lua_pushnumber(L, stats.negative); lua_settable(L, -3);
  lua_pushstring(L, "zeros");    lua_pushnumber(L, stats.zeros);    lua_settable(L, -3);
  lua_pushstring(L, "mean");     lua_pushnumber(L, stats.mean);     lua_settable(L, -3);
  lua_pushstring(L, "stddev");   lua_pushnumber(L, stats.stddev);   lua_settable(L, -3);
  return 1;
}

/*****************************************************************************\
 im.CalcHistoImageStatistics
\*****************************************************************************/
static int imluaCalcHistoImageStatistics (lua_State *L)
{
  int* median;
  int* mode;

  imImage *image = imlua_checkimage(L, 1);

  imlua_checkdatatype(L, 1, image, IM_BYTE);

  median = (int*)malloc(sizeof(int)*image->depth);
  mode = (int*)malloc(sizeof(int)*image->depth);

  imCalcHistoImageStatistics(image, median, mode);

  imlua_newarrayint (L, median, image->depth, 0);
  imlua_newarrayint (L, mode, image->depth, 0);

  free(median);
  free(mode);

  return 2;
}

/*****************************************************************************\
 Image Analysis
\*****************************************************************************/

/*****************************************************************************\
 im.AnalyzeFindRegions(src_image, dst_image, connect, touch_border)
\*****************************************************************************/
static int imluaAnalyzeFindRegions (lua_State *L)
{
  imImage* src_image = imlua_checkimage(L, 1);
  imImage* dst_image = imlua_checkimage(L, 2);
  int connect = luaL_checkint(L, 3);
  int touch_border = lua_toboolean(L, 4);

  imlua_checkcolorspace(L, 1, src_image, IM_BINARY);
  imlua_checktype(L, 2, dst_image, IM_GRAY, IM_USHORT);

  luaL_argcheck(L, (connect == 4 || connect == 8), 3, "invalid connect value, must be 4 or 8");
  lua_pushnumber(L, imAnalyzeFindRegions(src_image, dst_image, connect, touch_border));
  return 1;
}

static int iGetMax(imImage* image)
{
  int max = 0;
  int i;

  imushort* data = (imushort*)image->data[0];
  for (i = 0; i < image->count; i++)
  {
    if (*data > max)
      max = *data;

    data++;
  }

  return max;
}

static int imlua_checkregioncount(lua_State *L, int narg, imImage* image)
{
  if (lua_isnoneornil(L, narg)) return iGetMax(image);
  else return (int)luaL_checknumber(L, narg);
}


/*****************************************************************************\
 im.AnalyzeMeasureArea(image, [count])
\*****************************************************************************/
static int imluaAnalyzeMeasureArea (lua_State *L)
{
  int count;
  int *area;

  imImage* image = imlua_checkimage(L, 1);

  imlua_checktype(L, 1, image, IM_GRAY, IM_USHORT);

  count = imlua_checkregioncount(L, 2, image);
  area = (int*) malloc(sizeof(int) * count);

  imAnalyzeMeasureArea(image, area, count);

  imlua_newarrayint(L, area, count, 0);
  free(area);

  return 1;
}

/*****************************************************************************\
 im.AnalyzeMeasurePerimArea(image)
\*****************************************************************************/
static int imluaAnalyzeMeasurePerimArea (lua_State *L)
{
  int count;
  float *perimarea;

  imImage* image = imlua_checkimage(L, 1);

  imlua_checktype(L, 1, image, IM_GRAY, IM_USHORT);

  count = imlua_checkregioncount(L, 2, image);
  perimarea = (float*) malloc(sizeof(float) * count);

  imAnalyzeMeasurePerimArea(image, perimarea);

  imlua_newarrayfloat (L, perimarea, count, 0);
  free(perimarea);

  return 1;
}

/*****************************************************************************\
 im.AnalyzeMeasureCentroid(image, [area], [count])
\*****************************************************************************/
static int imluaAnalyzeMeasureCentroid (lua_State *L)
{
  int count;
  float *cx, *cy;
  int *area;

  imImage* image = imlua_checkimage(L, 1);

  imlua_checktype(L, 1, image, IM_GRAY, IM_USHORT);

  area = imlua_toarrayint(L, 2, &count, 0);
  count = imlua_checkregioncount(L, 3, image);

  cx = (float*) malloc (sizeof(float) * count);
  cy = (float*) malloc (sizeof(float) * count);

  imAnalyzeMeasureCentroid(image, area, count, cx, cy);

  imlua_newarrayfloat(L, cx, count, 0);
  imlua_newarrayfloat(L, cy, count, 0);

  if (area)
    free(area);
  free(cx);
  free(cy);

  return 2;
}

/*****************************************************************************\
 im.AnalyzeMeasurePrincipalAxis(image, [area], [cx], [cy])
\*****************************************************************************/
static int imluaAnalyzeMeasurePrincipalAxis (lua_State *L)
{
  int count;
  float *cx, *cy;
  int *area;
  float *major_slope, *major_length, *minor_slope, *minor_length;

  imImage* image = imlua_checkimage(L, 1);

  imlua_checktype(L, 1, image, IM_GRAY, IM_USHORT);

  area = imlua_toarrayint(L, 2, &count, 0);
  cx = imlua_toarrayfloat(L, 3, NULL, 0);
  cy = imlua_toarrayfloat(L, 4, NULL, 0);
  count = imlua_checkregioncount(L, 5, image);

  major_slope = (float*) malloc (sizeof(float) * count);
  major_length = (float*) malloc (sizeof(float) * count);
  minor_slope = (float*) malloc (sizeof(float) * count);
  minor_length = (float*) malloc (sizeof(float) * count);

  imAnalyzeMeasurePrincipalAxis(image, area, cx, cy, count, major_slope, major_length, minor_slope, minor_length);

  imlua_newarrayfloat(L, major_slope, count, 0);
  imlua_newarrayfloat(L, major_length, count, 0);
  imlua_newarrayfloat(L, minor_slope, count, 0);
  imlua_newarrayfloat(L, minor_length, count, 0);

  if (area)
    free(area);
  if (cx)
    free(cx);
  if (cy)
    free(cy);

  free(major_slope);
  free(major_length);
  free(minor_slope);
  free(minor_length);

  return 4;
}

/*****************************************************************************\
 im.AnalyzeMeasureHoles
\*****************************************************************************/
static int imluaAnalyzeMeasureHoles (lua_State *L)
{
  int holes_count, count;
  int connect;
  int *area = NULL;
  float *perim = NULL;

  imImage* image = imlua_checkimage(L, 1);

  imlua_checktype(L, 1, image, IM_GRAY, IM_USHORT);

  connect = luaL_checkint(L, 2);
  count = imlua_checkregioncount(L, 3, image);

  area = (int*) malloc (sizeof(int) * count);
  perim = (float*) malloc (sizeof(float) * count);

  imAnalyzeMeasureHoles(image, connect, &holes_count, area, perim);

  lua_pushnumber(L, holes_count);
  imlua_newarrayint(L, area, holes_count, 0);
  imlua_newarrayfloat(L, perim, holes_count, 0);

  if (area)
    free(area);
  if (perim)
    free(perim);

  return 3;
}

/*****************************************************************************\
 im.AnalyzeMeasurePerimeter(image, [count])
\*****************************************************************************/
static int imluaAnalyzeMeasurePerimeter (lua_State *L)
{
  int count;
  float *perim;

  imImage* image = imlua_checkimage(L, 1);

  imlua_checktype(L, 1, image, IM_GRAY, IM_USHORT);

  count = imlua_checkregioncount(L, 2, image);
  perim = (float*) malloc(sizeof(float) * count);

  imAnalyzeMeasurePerimeter(image, perim, count);

  imlua_newarrayfloat(L, perim, count, 0);

  free(perim);

  return 1;
}

/*****************************************************************************\
 im.ProcessPerimeterLine(src_image, dst_image)
\*****************************************************************************/
static int imluaProcessPerimeterLine (lua_State *L)
{
  imImage* src_image = imlua_checkimage(L, 1);
  imImage* dst_image = imlua_checkimage(L, 2);

  luaL_argcheck(L, (src_image->data_type < IM_FLOAT), 1, "image data type can be integer only");
  imlua_match(L, src_image, dst_image);

  imProcessPerimeterLine(src_image, dst_image);
  return 0;
}

/*****************************************************************************\
 im.ProcessRemoveByArea(src_image, dst_image, connect, start_size, end_size, inside)
\*****************************************************************************/
static int imluaProcessRemoveByArea (lua_State *L)
{
  imImage* src_image = imlua_checkimage(L, 1);
  imImage* dst_image = imlua_checkimage(L, 2);
  int connect = luaL_checkint(L, 3);
  int start_size = luaL_checkint(L, 4);
  int end_size = luaL_checkint(L, 5);
  int inside = lua_toboolean(L, 6);

  imlua_checkcolorspace(L, 1, src_image, IM_BINARY);
  imlua_match(L, src_image, dst_image);
  luaL_argcheck(L, (connect == 4 || connect == 8), 3, "invalid connect value, must be 4 or 8");

  imProcessRemoveByArea(src_image, dst_image, connect, start_size, end_size, inside);
  return 0;
}

/*****************************************************************************\
 im.ProcessFillHoles(src_image, dst_image, connect)
\*****************************************************************************/
static int imluaProcessFillHoles (lua_State *L)
{
  imImage* src_image = imlua_checkimage(L, 1);
  imImage* dst_image = imlua_checkimage(L, 2);
  int connect = luaL_checkint(L, 3);

  imlua_checkcolorspace(L, 1, src_image, IM_BINARY);
  imlua_match(L, src_image, dst_image);
  luaL_argcheck(L, (connect == 4 || connect == 8), 3, "invalid connect value, must be 4 or 8");

  imProcessFillHoles(src_image, dst_image, connect);
  return 0;
}

static void imlua_checkhoughsize(lua_State *L, imImage* image, imImage* hough_image, int param)
{
#define IMSQR(_x) (_x*_x)
  int hough_rmax;
  if (hough_image->width != 180)
    luaL_argerror(L, param, "invalid image width");

  hough_rmax = (int)(sqrt((double)(IMSQR(image->width) + IMSQR(image->height)))/2.0);
  if (hough_image->height != 2*hough_rmax+1)
    luaL_argerror(L, param, "invalid image height");
}

/*****************************************************************************\
 im.ProcessHoughLines(src_image, dst_image)
\*****************************************************************************/
static int imluaProcessHoughLines (lua_State *L)
{
  imImage* src_image = imlua_checkimage(L, 1);
  imImage* dst_image = imlua_checkimage(L, 2);

  imlua_checkcolorspace(L, 1, src_image, IM_BINARY);
  imlua_checktype(L, 2, dst_image, IM_GRAY, IM_INT);
  imlua_checkhoughsize(L, src_image, dst_image, 2);

  lua_pushboolean(L, imProcessHoughLines(src_image, dst_image));
  return 0;
}

/*****************************************************************************\
 im.ProcessHoughLinesDraw(src_image, hough_points, dst_image)
\*****************************************************************************/
static int imluaProcessHoughLinesDraw (lua_State *L)
{
  imImage* src_image = imlua_checkimage(L, 1);
  imImage* hough_points = imlua_checkimage(L, 3);
  imImage* dst_image = imlua_checkimage(L, 4);
  imImage* hough = NULL;
  if (lua_isuserdata(L, 2))
  {
    hough = imlua_checkimage(L, 2);
    imlua_checktype(L, 2, hough, IM_GRAY, IM_INT);
    imlua_checkhoughsize(L, src_image, hough, 2);
  }

  imlua_checktype(L, 1, src_image, IM_GRAY, IM_BYTE);
  imlua_checkcolorspace(L, 3, hough_points, IM_BINARY);
  imlua_checkhoughsize(L, src_image, hough_points, 3);
  imlua_matchsize(L, src_image, dst_image);

  lua_pushnumber(L, imProcessHoughLinesDraw(src_image, hough, hough_points, dst_image));
  return 0;
}

/*****************************************************************************\
 im.ProcessDistanceTransform(src_image, dst_image)
\*****************************************************************************/
static int imluaProcessDistanceTransform (lua_State *L)
{
  imImage* src_image = imlua_checkimage(L, 1);
  imImage* dst_image = imlua_checkimage(L, 2);

  imlua_checkcolorspace(L, 1, src_image, IM_BINARY);
  imlua_checkdatatype(L, 2, dst_image, IM_FLOAT);
  imlua_matchsize(L, src_image, dst_image);

  imProcessDistanceTransform(src_image, dst_image);
  return 0;
}

/*****************************************************************************\
 im.ProcessRegionalMaximum(src_image, dst_image)
\*****************************************************************************/
static int imluaProcessRegionalMaximum (lua_State *L)
{
  imImage* src_image = imlua_checkimage(L, 1);
  imImage* dst_image = imlua_checkimage(L, 2);

  imlua_checktype(L, 1, src_image, IM_GRAY, IM_FLOAT);
  imlua_checkcolorspace(L, 2, dst_image, IM_BINARY);
  imlua_matchsize(L, src_image, dst_image);

  imProcessRegionalMaximum(src_image, dst_image);
  return 0;
}

/*****************************************************************************\
 Image Resize
\*****************************************************************************/

/*****************************************************************************\
 im.ProcessReduce(src_image, dst_image, order)
\*****************************************************************************/
static int imluaProcessReduce (lua_State *L)
{
  imImage* src_image = imlua_checkimage(L, 1);
  imImage* dst_image = imlua_checkimage(L, 2);
  int order = luaL_checkint(L, 3);

  imlua_matchcolor(L, src_image, dst_image);
  luaL_argcheck(L, (order == 0 || order == 1), 3, "invalid order, must be 0 or 1");

  lua_pushboolean(L, imProcessReduce(src_image, dst_image, order));
  return 1;
}

/*****************************************************************************\
 im.ProcessResize(src_image, dst_image, order)
\*****************************************************************************/
static int imluaProcessResize (lua_State *L)
{
  imImage* src_image = imlua_checkimage(L, 1);
  imImage* dst_image = imlua_checkimage(L, 2);
  int order = luaL_checkint(L, 3);

  imlua_matchcolor(L, src_image, dst_image);
  luaL_argcheck(L, (order == 0 || order == 1 || order == 3), 3, "invalid order, must be 0, 1 or 3");

  lua_pushboolean(L, imProcessResize(src_image, dst_image, order));
  return 1;
}

/*****************************************************************************\
 im.ProcessReduceBy4(src_image, dst_image)
\*****************************************************************************/
static int imluaProcessReduceBy4 (lua_State *L)
{
  imImage* src_image = imlua_checkimage(L, 1);
  imImage* dst_image = imlua_checkimage(L, 2);

  imlua_matchcolor(L, src_image, dst_image);
  luaL_argcheck(L,
    dst_image->width == (src_image->width / 2) &&
    dst_image->height == (src_image->height / 2), 3, "destiny image size must be source image width/2, height/2");

  imProcessReduceBy4(src_image, dst_image);
  return 0;
}

/*****************************************************************************\
 im.ProcessCrop(src_image, dst_image, xmin, ymin)
\*****************************************************************************/
static int imluaProcessCrop (lua_State *L)
{
  imImage* src_image = imlua_checkimage(L, 1);
  imImage* dst_image = imlua_checkimage(L, 2);
  int xmin = luaL_checkint(L, 3);
  int ymin = luaL_checkint(L, 4);

  imlua_matchcolor(L, src_image, dst_image);
  luaL_argcheck(L, xmin >= 0 && xmin < src_image->width, 3, "xmin must be >= 0 and < width");
  luaL_argcheck(L, ymin >= 0 && ymin < src_image->height, 4, "ymin must be >= 0 and < height");
  luaL_argcheck(L, dst_image->width <= (src_image->width - xmin), 2, "destiny image size must be smaller than source image width-xmin");
  luaL_argcheck(L, dst_image->height <= (src_image->height - ymin), 2, "destiny image size must be smaller than source image height-ymin");

  imProcessCrop(src_image, dst_image, xmin, ymin);
  return 0;
}

/*****************************************************************************\
 im.ProcessInsert(src_image, region_image, dst_image, xmin, ymin)
\*****************************************************************************/
static int imluaProcessInsert (lua_State *L)
{
  imImage* src_image = imlua_checkimage(L, 1);
  imImage* region_image = imlua_checkimage(L, 2);
  imImage* dst_image = imlua_checkimage(L, 3);
  int xmin = luaL_checkint(L, 4);
  int ymin = luaL_checkint(L, 5);

  imlua_matchcolor(L, src_image, dst_image);
  luaL_argcheck(L, xmin >= 0 && xmin < src_image->width, 3, "xmin must be >= 0 and < width");
  luaL_argcheck(L, ymin >= 0 && ymin < src_image->height, 3, "ymin must be >= 0 and < height");

  imProcessInsert(src_image, region_image, dst_image, xmin, ymin);
  return 0;
}

/*****************************************************************************\
 im.ProcessAddMargins(src_image, dst_image, xmin, ymin)
\*****************************************************************************/
static int imluaProcessAddMargins (lua_State *L)
{
  imImage* src_image = imlua_checkimage(L, 1);
  imImage* dst_image = imlua_checkimage(L, 2);
  int xmin = luaL_checkint(L, 3);
  int ymin = luaL_checkint(L, 4);

  imlua_matchcolor(L, src_image, dst_image);
  luaL_argcheck(L, dst_image->width > (src_image->width + xmin), 2, "destiny image size must be greatter than source image width+xmin, height+ymin");
  luaL_argcheck(L, dst_image->height > (src_image->height + ymin), 2, "destiny image size must be greatter than source image width+xmin, height+ymin");

  imProcessAddMargins(src_image, dst_image, xmin, ymin);
  return 0;
}



/*****************************************************************************\
 Geometric Operations
\*****************************************************************************/

/*****************************************************************************\
 im.ProcessCalcRotateSize
\*****************************************************************************/
static int imluaProcessCalcRotateSize (lua_State *L)
{
  int new_width, new_height;

  int width = luaL_checkint(L, 1);
  int height = luaL_checkint(L, 2);
  double cos0 = (double) luaL_checknumber(L, 3);
  double sin0 = (double) luaL_checknumber(L, 4);

  imProcessCalcRotateSize(width, height, &new_width, &new_height, cos0, sin0);
  lua_pushnumber(L, new_width);
  lua_pushnumber(L, new_height);
  return 2;
}

/*****************************************************************************\
 im.ProcessRotate
\*****************************************************************************/
static int imluaProcessRotate (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  double cos0 = (double) luaL_checknumber(L, 3);
  double sin0 = (double) luaL_checknumber(L, 4);
  int order = luaL_checkint(L, 5);

  imlua_matchcolor(L, src_image, dst_image);
  luaL_argcheck(L, (order == 0 || order == 1 || order == 3), 5, "invalid order, must be 0, 1 or 3");

  lua_pushboolean(L, imProcessRotate(src_image, dst_image, cos0, sin0, order));
  return 1;
}

/*****************************************************************************\
 im.ProcessRotateRef
\*****************************************************************************/
static int imluaProcessRotateRef (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  double cos0 = (double) luaL_checknumber(L, 3);
  double sin0 = (double) luaL_checknumber(L, 4);
  int x = luaL_checkint(L, 5);
  int y = luaL_checkint(L, 6);
  int to_origin = lua_toboolean(L, 7);
  int order = luaL_checkint(L, 8);

  imlua_matchcolor(L, src_image, dst_image);
  luaL_argcheck(L, (order == 0 || order == 1 || order == 3), 5, "invalid order, must be 0, 1, or 3");

  lua_pushboolean(L, imProcessRotateRef(src_image, dst_image, cos0, sin0, x, y, to_origin, order));
  return 1;
}

/*****************************************************************************\
 im.ProcessRotate90
\*****************************************************************************/
static int imluaProcessRotate90 (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  int dir = lua_toboolean(L, 3);

  imlua_matchcolor(L, src_image, dst_image);
  luaL_argcheck(L, dst_image->width == src_image->height && dst_image->height == src_image->width, 2, "destiny width and height must have the source height and width");
  luaL_argcheck(L, (dir == -1 || dir == 1), 3, "invalid dir, can be -1 or 1 only");

  imProcessRotate90(src_image, dst_image, dir);
  return 0;
}

/*****************************************************************************\
 im.ProcessRotate180
\*****************************************************************************/
static int imluaProcessRotate180 (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);

  imlua_match(L, src_image, dst_image);

  imProcessRotate180(src_image, dst_image);
  return 0;
}

/*****************************************************************************\
 im.ProcessMirror
\*****************************************************************************/
static int imluaProcessMirror (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);

  imlua_match(L, src_image, dst_image);

  imProcessMirror(src_image, dst_image);
  return 0;
}

/*****************************************************************************\
 im.ProcessFlip
\*****************************************************************************/
static int imluaProcessFlip (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);

  imlua_match(L, src_image, dst_image);

  imProcessFlip(src_image, dst_image);
  return 0;
}

/*****************************************************************************\
 im.ProcessInterlaceSplit
\*****************************************************************************/
static int imluaProcessInterlaceSplit (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image1 = imlua_checkimage(L, 2);
  imImage *dst_image2 = imlua_checkimage(L, 3);

  imlua_matchcolor(L, src_image, dst_image1);
  imlua_matchcolor(L, src_image, dst_image2);
  luaL_argcheck(L, dst_image1->width == src_image->width && dst_image2->width == src_image->width, 2, "destiny width must be equal to source width");

  if (src_image->height%2)
  {
    int dst_height1 = src_image->height/2 + 1;
    luaL_argcheck(L, dst_image1->height == dst_height1, 2, "destiny1 height must be equal to source height/2+1 if height odd");
  }
  else
    luaL_argcheck(L, dst_image1->height == src_image->height/2, 2, "destiny1 height must be equal to source height/2 if height even");

  luaL_argcheck(L, dst_image2->height == src_image->height/2, 2, "destiny2 height must be equal to source height/2");

  imProcessInterlaceSplit(src_image, dst_image1, dst_image2);
  return 0;
}

/*****************************************************************************\
 im.ProcessRadial
\*****************************************************************************/
static int imluaProcessRadial (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  float k1 = (float) luaL_checknumber(L, 3);
  int order = luaL_checkint(L, 4);

  imlua_match(L, src_image, dst_image);
  luaL_argcheck(L, (order == 0 || order == 1 || order == 3), 4, "invalid order");

  lua_pushboolean(L, imProcessRadial(src_image, dst_image, k1, order));
  return 1;
}

/*****************************************************************************\
 im.ProcessSwirl
\*****************************************************************************/
static int imluaProcessSwirl(lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  float k1 = (float) luaL_checknumber(L, 3);
  int order = luaL_checkint(L, 4);

  imlua_match(L, src_image, dst_image);
  luaL_argcheck(L, (order == 0 || order == 1 || order == 3), 4, "invalid order, can be 0, 1 or 3");

  lua_pushboolean(L, imProcessSwirl(src_image, dst_image, k1, order));
  return 1;
}

static void imlua_checknotcfloat(lua_State *L, imImage *image, int index)
{
  if (image->data_type == IM_CFLOAT)
    luaL_argerror(L, index, "image data type can NOT be cfloat");
}


/*****************************************************************************\
 Morphology Operations for Gray Images
\*****************************************************************************/

/*****************************************************************************\
 im.ProcessGrayMorphConvolve
\*****************************************************************************/
static int imluaProcessGrayMorphConvolve (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  imImage *kernel = imlua_checkimage(L, 3);
  int ismax = lua_toboolean(L, 4);

  imlua_checknotcfloat(L, src_image, 1);
  imlua_match(L, src_image, dst_image);
  imlua_checkdatatype(L, 3, kernel, IM_INT);
  imlua_matchsize(L, src_image, kernel);

  lua_pushboolean(L, imProcessGrayMorphConvolve(src_image, dst_image, kernel, ismax));
  return 1;
}

/*****************************************************************************\
 im.ProcessGrayMorphErode
\*****************************************************************************/
static int imluaProcessGrayMorphErode (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  int kernel_size = luaL_checkint(L, 3);

  imlua_checknotcfloat(L, src_image, 1);
  imlua_match(L, src_image, dst_image);

  lua_pushboolean(L, imProcessGrayMorphErode(src_image, dst_image, kernel_size));
  return 1;
}

/*****************************************************************************\
 im.ProcessGrayMorphDilate
\*****************************************************************************/
static int imluaProcessGrayMorphDilate (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  int kernel_size = luaL_checkint(L, 3);

  imlua_checknotcfloat(L, src_image, 1);
  imlua_match(L, src_image, dst_image);

  lua_pushboolean(L, imProcessGrayMorphDilate(src_image, dst_image, kernel_size));
  return 1;
}

/*****************************************************************************\
 im.ProcessGrayMorphOpen
\*****************************************************************************/
static int imluaProcessGrayMorphOpen (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  int kernel_size = luaL_checkint(L, 3);

  imlua_checknotcfloat(L, src_image, 1);
  imlua_match(L, src_image, dst_image);

  lua_pushboolean(L, imProcessGrayMorphOpen(src_image, dst_image, kernel_size));
  return 1;
}

/*****************************************************************************\
 im.ProcessGrayMorphClose
\*****************************************************************************/
static int imluaProcessGrayMorphClose (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  int kernel_size = luaL_checkint(L, 3);

  imlua_checknotcfloat(L, src_image, 1);
  imlua_match(L, src_image, dst_image);

  lua_pushboolean(L, imProcessGrayMorphClose(src_image, dst_image, kernel_size));
  return 1;
}

/*****************************************************************************\
 im.ProcessGrayMorphTopHat
\*****************************************************************************/
static int imluaProcessGrayMorphTopHat (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  int kernel_size = luaL_checkint(L, 3);

  imlua_checknotcfloat(L, src_image, 1);
  imlua_match(L, src_image, dst_image);

  lua_pushboolean(L, imProcessGrayMorphTopHat(src_image, dst_image, kernel_size));
  return 1;
}

/*****************************************************************************\
 im.ProcessGrayMorphWell
\*****************************************************************************/
static int imluaProcessGrayMorphWell (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  int kernel_size = luaL_checkint(L, 3);

  imlua_checknotcfloat(L, src_image, 1);
  imlua_match(L, src_image, dst_image);

  lua_pushboolean(L, imProcessGrayMorphWell(src_image, dst_image, kernel_size));
  return 1;
}

/*****************************************************************************\
 im.ProcessGrayMorphGradient
\*****************************************************************************/
static int imluaProcessGrayMorphGradient (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  int kernel_size = luaL_checkint(L, 3);

  imlua_checknotcfloat(L, src_image, 1);
  imlua_match(L, src_image, dst_image);

  lua_pushboolean(L, imProcessGrayMorphGradient(src_image, dst_image, kernel_size));
  return 1;
}



/*****************************************************************************\
 Morphology Operations for Binary Images
\*****************************************************************************/

/*****************************************************************************\
 im.ProcessBinMorphConvolve
\*****************************************************************************/
static int imluaProcessBinMorphConvolve (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  imImage *kernel = imlua_checkimage(L, 3);
  int hit_white = lua_toboolean(L, 4);
  int iter = luaL_checkint(L, 5);

  imlua_checkcolorspace(L, 1, src_image, IM_BINARY);
  imlua_match(L, src_image, dst_image);
  imlua_checkdatatype(L, 3, kernel, IM_INT);
  imlua_matchsize(L, src_image, kernel);

  lua_pushboolean(L, imProcessBinMorphConvolve(src_image, dst_image, kernel, hit_white, iter));
  return 1;
}

/*****************************************************************************\
 im.ProcessBinMorphErode
\*****************************************************************************/
static int imluaProcessBinMorphErode (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  int kernel_size = luaL_checkint(L, 3);
  int iter = luaL_checkint(L, 4);

  imlua_checkcolorspace(L, 1, src_image, IM_BINARY);
  imlua_match(L, src_image, dst_image);

  lua_pushboolean(L, imProcessBinMorphErode(src_image, dst_image, kernel_size, iter));
  return 1;
}

/*****************************************************************************\
 im.ProcessBinMorphDilate
\*****************************************************************************/
static int imluaProcessBinMorphDilate (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  int kernel_size = luaL_checkint(L, 3);
  int iter = luaL_checkint(L, 4);

  imlua_checkcolorspace(L, 1, src_image, IM_BINARY);
  imlua_match(L, src_image, dst_image);

  lua_pushboolean(L, imProcessBinMorphDilate(src_image, dst_image, kernel_size, iter));
  return 1;
}

/*****************************************************************************\
 im.ProcessBinMorphOpen
\*****************************************************************************/
static int imluaProcessBinMorphOpen (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  int kernel_size = luaL_checkint(L, 3);
  int iter = luaL_checkint(L, 4);

  imlua_checkcolorspace(L, 1, src_image, IM_BINARY);
  imlua_match(L, src_image, dst_image);

  lua_pushboolean(L, imProcessBinMorphOpen(src_image, dst_image, kernel_size, iter));
  return 1;
}

/*****************************************************************************\
 im.ProcessBinMorphClose
\*****************************************************************************/
static int imluaProcessBinMorphClose (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  int kernel_size = luaL_checkint(L, 3);
  int iter = luaL_checkint(L, 4);

  imlua_checkcolorspace(L, 1, src_image, IM_BINARY);
  imlua_match(L, src_image, dst_image);

  lua_pushboolean(L, imProcessBinMorphClose(src_image, dst_image, kernel_size, iter));
  return 1;
}

/*****************************************************************************\
 im.ProcessBinMorphOutline
\*****************************************************************************/
static int imluaProcessBinMorphOutline (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  int kernel_size = luaL_checkint(L, 3);
  int iter = luaL_checkint(L, 4);

  imlua_checkcolorspace(L, 1, src_image, IM_BINARY);
  imlua_match(L, src_image, dst_image);

  lua_pushboolean(L, imProcessBinMorphOutline(src_image, dst_image, kernel_size, iter));
  return 1;
}

/*****************************************************************************\
 im.ProcessBinMorphThin
\*****************************************************************************/
static int imluaProcessBinMorphThin (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);

  imlua_checkcolorspace(L, 1, src_image, IM_BINARY);
  imlua_match(L, src_image, dst_image);

  imProcessBinMorphThin(src_image, dst_image);
  return 0;
}



/*****************************************************************************\
 Rank Convolution Operations
\*****************************************************************************/

/*****************************************************************************\
 im.ProcessMedianConvolve
\*****************************************************************************/
static int imluaProcessMedianConvolve (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  int kernel_size = luaL_checkint(L, 3);

  imlua_checknotcfloat(L, src_image, 1);
  imlua_match(L, src_image, dst_image);

  lua_pushboolean(L, imProcessMedianConvolve(src_image, dst_image, kernel_size));
  return 1;
}

/*****************************************************************************\
 im.ProcessRangeConvolve
\*****************************************************************************/
static int imluaProcessRangeConvolve (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  int kernel_size = luaL_checkint(L, 3);

  imlua_checknotcfloat(L, src_image, 1);
  imlua_match(L, src_image, dst_image);

  lua_pushboolean(L, imProcessRangeConvolve(src_image, dst_image, kernel_size));
  return 1;
}

/*****************************************************************************\
 im.ProcessRankClosestConvolve
\*****************************************************************************/
static int imluaProcessRankClosestConvolve (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  int kernel_size = luaL_checkint(L, 3);

  imlua_checknotcfloat(L, src_image, 1);
  imlua_match(L, src_image, dst_image);

  lua_pushboolean(L, imProcessRankClosestConvolve(src_image, dst_image, kernel_size));
  return 1;
}

/*****************************************************************************\
 im.ProcessRankMaxConvolve
\*****************************************************************************/
static int imluaProcessRankMaxConvolve (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  int kernel_size = luaL_checkint(L, 3);

  imlua_checknotcfloat(L, src_image, 1);
  imlua_match(L, src_image, dst_image);

  lua_pushboolean(L, imProcessRankMaxConvolve(src_image, dst_image, kernel_size));
  return 1;
}

/*****************************************************************************\
 im.ProcessRankMinConvolve
\*****************************************************************************/
static int imluaProcessRankMinConvolve (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  int kernel_size = luaL_checkint(L, 3);

  imlua_checknotcfloat(L, src_image, 1);
  imlua_match(L, src_image, dst_image);

  lua_pushboolean(L, imProcessRankMinConvolve(src_image, dst_image, kernel_size));
  return 1;
}


/*****************************************************************************\
 Convolution Operations
\*****************************************************************************/

static void imlua_checkkernel(lua_State *L, imImage* kernel, int index)
{
  imlua_checkcolorspace(L, index, kernel, IM_GRAY);
  luaL_argcheck(L, kernel->data_type == IM_INT || kernel->data_type == IM_FLOAT, index, "kernel data type can be int or float only");
}

/*****************************************************************************\
 im.ProcessConvolve
\*****************************************************************************/
static int imluaProcessConvolve (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  imImage *kernel = imlua_checkimage(L, 3);

  imlua_match(L, src_image, dst_image);
  imlua_checkkernel(L, kernel, 3);

  lua_pushboolean(L, imProcessConvolve(src_image, dst_image, kernel));
  return 1;
}

/*****************************************************************************\
 im.ProcessConvolveDual
\*****************************************************************************/
static int imluaProcessConvolveDual (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  imImage *kernel1 = imlua_checkimage(L, 3);
  imImage *kernel2 = imlua_checkimage(L, 4);

  imlua_match(L, src_image, dst_image);
  imlua_checkkernel(L, kernel1, 3);
  imlua_checkkernel(L, kernel2, 4);

  lua_pushboolean(L, imProcessConvolveDual(src_image, dst_image, kernel1, kernel2));
  return 1;
}

/*****************************************************************************\
 im.ProcessConvolveRep
\*****************************************************************************/
static int imluaProcessConvolveRep (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  imImage *kernel = imlua_checkimage(L, 3);
  int count = luaL_checkint(L, 4);

  imlua_match(L, src_image, dst_image);
  imlua_checkkernel(L, kernel, 3);

  lua_pushboolean(L, imProcessConvolveRep(src_image, dst_image, kernel, count));
  return 1;
}

/*****************************************************************************\
 im.ProcessConvolveSep
\*****************************************************************************/
static int imluaProcessConvolveSep (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  imImage *kernel = imlua_checkimage(L, 3);

  imlua_match(L, src_image, dst_image);
  imlua_checkkernel(L, kernel, 3);

  lua_pushboolean(L, imProcessConvolveSep(src_image, dst_image, kernel));
  return 1;
}

/*****************************************************************************\
 im.ProcessCompassConvolve
\*****************************************************************************/
static int imluaProcessCompassConvolve (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  imImage *kernel = imlua_checkimage(L, 3);

  imlua_checknotcfloat(L, src_image, 1);
  imlua_match(L, src_image, dst_image);
  imlua_checkkernel(L, kernel, 3);

  lua_pushboolean(L, imProcessCompassConvolve(src_image, dst_image, kernel));
  return 1;
}

/*****************************************************************************\
 im.ProcessRotateKernel
\*****************************************************************************/
static int imluaProcessRotateKernel (lua_State *L)
{
  imProcessRotateKernel(imlua_checkimage(L, 1));
  return 0;
}

/*****************************************************************************\
 im.ProcessDiffOfGaussianConvolve
\*****************************************************************************/
static int imluaProcessDiffOfGaussianConvolve (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  float stddev1 = (float) luaL_checknumber(L, 3);
  float stddev2 = (float) luaL_checknumber(L, 4);

  if (src_image->data_type == IM_BYTE || src_image->data_type == IM_USHORT)
  {
    imlua_matchcolor(L, src_image, dst_image);
    imlua_checkdatatype(L, 2, dst_image, IM_INT);
  }
  else
    imlua_match(L, src_image, dst_image);

  lua_pushboolean(L, imProcessDiffOfGaussianConvolve(src_image, dst_image, stddev1, stddev2));
  return 1;
}

/*****************************************************************************\
 im.ProcessLapOfGaussianConvolve
\*****************************************************************************/
static int imluaProcessLapOfGaussianConvolve (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  float stddev = (float) luaL_checknumber(L, 3);

  if (src_image->data_type == IM_BYTE || src_image->data_type == IM_USHORT)
  {
    imlua_matchcolor(L, src_image, dst_image);
    imlua_checkdatatype(L, 2, dst_image, IM_INT);
  }
  else
    imlua_match(L, src_image, dst_image);

  lua_pushboolean(L, imProcessLapOfGaussianConvolve(src_image, dst_image, stddev));
  return 1;
}

/*****************************************************************************\
 im.ProcessMeanConvolve
\*****************************************************************************/
static int imluaProcessMeanConvolve (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  int kernel_size = luaL_checkint(L, 3);

  imlua_match(L, src_image, dst_image);

  lua_pushboolean(L, imProcessMeanConvolve(src_image, dst_image, kernel_size));
  return 1;
}

/*****************************************************************************\
 im.ProcessBarlettConvolve
\*****************************************************************************/
static int imluaProcessBarlettConvolve (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  int kernel_size = luaL_checkint(L, 3);

  imlua_match(L, src_image, dst_image);

  lua_pushboolean(L, imProcessBarlettConvolve(src_image, dst_image, kernel_size));
  return 1;
}

/*****************************************************************************\
 im.ProcessGaussianConvolve
\*****************************************************************************/
static int imluaProcessGaussianConvolve (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  float stddev = (float) luaL_checknumber(L, 3);

  imlua_match(L, src_image, dst_image);

  lua_pushboolean(L, imProcessGaussianConvolve(src_image, dst_image, stddev));
  return 1;
}

/*****************************************************************************\
 im.ProcessPrewittConvolve
\*****************************************************************************/
static int imluaProcessPrewittConvolve (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);

  imlua_match(L, src_image, dst_image);

  lua_pushboolean(L, imProcessPrewittConvolve(src_image, dst_image));
  return 1;
}

/*****************************************************************************\
 im.ProcessSplineEdgeConvolve
\*****************************************************************************/
static int imluaProcessSplineEdgeConvolve (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);

  imlua_match(L, src_image, dst_image);

  lua_pushboolean(L, imProcessSplineEdgeConvolve(src_image, dst_image));
  return 1;
}

/*****************************************************************************\
 im.ProcessSobelConvolve
\*****************************************************************************/
static int imluaProcessSobelConvolve (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);

  imlua_match(L, src_image, dst_image);

  lua_pushboolean(L, imProcessSobelConvolve(src_image, dst_image));
  return 1;
}

/*****************************************************************************\
 im.ProcessZeroCrossing
\*****************************************************************************/
static int imluaProcessZeroCrossing (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);

  luaL_argcheck(L, src_image->data_type == IM_INT || src_image->data_type == IM_FLOAT, 1, "image data type can be int or float only");
  imlua_match(L, src_image, dst_image);

  imProcessZeroCrossing(src_image, dst_image);
  return 0;
}

/*****************************************************************************\
 im.ProcessCanny
\*****************************************************************************/
static int imluaProcessCanny (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  float stddev = (float) luaL_checknumber(L, 3);

  imlua_checktype(L, 1, src_image, IM_GRAY, IM_BYTE);
  imlua_match(L, src_image, dst_image);

  imProcessCanny(src_image, dst_image, stddev);
  return 0;
}

/*****************************************************************************\
 im.ProcessUnsharp
\*****************************************************************************/
static int imluaProcessUnsharp(lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  float p1 = (float)luaL_checknumber(L, 3);
  float p2 = (float)luaL_checknumber(L, 4);
  float p3 = (float)luaL_checknumber(L, 5);

  imlua_match(L, src_image, dst_image);

  imProcessUnsharp(src_image, dst_image, p1, p2, p3);
  return 0;
}

/*****************************************************************************\
 im.ProcessSharp
\*****************************************************************************/
static int imluaProcessSharp(lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  float p1 = (float)luaL_checknumber(L, 3);
  float p2 = (float)luaL_checknumber(L, 4);

  imlua_match(L, src_image, dst_image);

  imProcessSharp(src_image, dst_image, p1, p2);
  return 0;
}

/*****************************************************************************\
 im.ProcessSharpKernel
\*****************************************************************************/
static int imluaProcessSharpKernel(lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *kernel = imlua_checkimage(L, 2);
  imImage *dst_image = imlua_checkimage(L, 3);
  float p1 = (float)luaL_checknumber(L, 4);
  float p2 = (float)luaL_checknumber(L, 5);

  imlua_match(L, src_image, dst_image);

  imProcessSharpKernel(src_image, kernel, dst_image, p1, p2);
  return 0;
}

/*****************************************************************************\
 im.GaussianStdDev2Repetitions
\*****************************************************************************/
static int imluaGaussianKernelSize2StdDev(lua_State *L)
{
  lua_pushnumber(L, imGaussianKernelSize2StdDev((int)luaL_checknumber(L, 1)));
  return 1;
}

/*****************************************************************************\
 im.GaussianStdDev2KernelSize
\*****************************************************************************/
static int imluaGaussianStdDev2KernelSize (lua_State *L)
{
  lua_pushnumber(L, imGaussianStdDev2KernelSize((float)luaL_checknumber(L, 1)));
  return 1;
}



/*****************************************************************************\
 Arithmetic Operations
\*****************************************************************************/

/*****************************************************************************\
 im.ProcessUnArithmeticOp
\*****************************************************************************/
static int imluaProcessUnArithmeticOp (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  int op = luaL_checkint(L, 3);

  imlua_matchcolorspace(L, src_image, dst_image);

  imProcessUnArithmeticOp(src_image, dst_image, op);
  return 0;
}

/*****************************************************************************\
 im.ProcessArithmeticOp
\*****************************************************************************/
static int imluaProcessArithmeticOp (lua_State *L)
{
  imImage *src_image1 = imlua_checkimage(L, 1);
  imImage *src_image2 = imlua_checkimage(L, 2);
  imImage *dst_image = imlua_checkimage(L, 3);
  int op = luaL_checkint(L, 4);

  imlua_match(L, src_image1, src_image2);
  imlua_matchsize(L, src_image1, dst_image);
  imlua_matchsize(L, src_image2, dst_image);

  switch (src_image1->data_type)
  {
  case IM_BYTE:
    luaL_argcheck(L,
      dst_image->data_type == IM_BYTE ||
      dst_image->data_type == IM_USHORT ||
      dst_image->data_type == IM_INT ||
      dst_image->data_type == IM_FLOAT,
      2, "source image is byte, destiny image data type can be byte, ushort, int and float only.");
    break;
  case IM_USHORT:
    luaL_argcheck(L,
      dst_image->data_type == IM_USHORT ||
      dst_image->data_type == IM_INT ||
      dst_image->data_type == IM_FLOAT,
      2, "source image is ushort, destiny image data type can be ushort, int and float only.");
    break;
  case IM_INT:
    luaL_argcheck(L,
      dst_image->data_type == IM_INT ||
      dst_image->data_type == IM_FLOAT,
      2, "source image is int, destiny image data type can be int and float only.");
    break;
  case IM_FLOAT:
    luaL_argcheck(L,
      dst_image->data_type == IM_FLOAT,
      2, "source image is float, destiny image data type can be float only.");
    break;
  case IM_CFLOAT:
    luaL_argcheck(L,
      dst_image->data_type == IM_CFLOAT,
      2, "source image is cfloat, destiny image data type can be cfloat only.");
    break;
  }

  imProcessArithmeticOp(src_image1, src_image2, dst_image, op);
  return 0;
}

/*****************************************************************************\
 im.ProcessArithmeticConstOp
\*****************************************************************************/
static int imluaProcessArithmeticConstOp (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  float src_const = (float) luaL_checknumber(L, 2);
  imImage *dst_image = imlua_checkimage(L, 3);
  int op = luaL_checkint(L, 4);

  imlua_matchsize(L, src_image, dst_image);

  switch (src_image->data_type)
  {
  case IM_BYTE:
    luaL_argcheck(L,
      dst_image->data_type == IM_BYTE ||
      dst_image->data_type == IM_USHORT ||
      dst_image->data_type == IM_INT ||
      dst_image->data_type == IM_FLOAT,
      2, "source image is byte, destiny image data type can be byte, ushort, int and float only.");
    break;
  case IM_USHORT:
    luaL_argcheck(L,
      dst_image->data_type == IM_USHORT ||
      dst_image->data_type == IM_INT ||
      dst_image->data_type == IM_FLOAT,
      2, "source image is ushort, destiny image data type can be ushort, int and float only.");
    break;
  case IM_INT:
    luaL_argcheck(L,
      dst_image->data_type == IM_INT ||
      dst_image->data_type == IM_FLOAT,
      2, "source image is int, destiny image data type can be int and float only.");
    break;
  case IM_FLOAT:
    luaL_argcheck(L,
      dst_image->data_type == IM_FLOAT,
      2, "source image is float, destiny image data type can be float only.");
    break;
  case IM_CFLOAT:
    luaL_argcheck(L,
      dst_image->data_type == IM_CFLOAT,
      2, "source image is cfloat, destiny image data type can be cfloat only.");
    break;
  }

  imProcessArithmeticConstOp(src_image, src_const, dst_image, op);
  return 0;
}

/*****************************************************************************\
 im.ProcessBlendConst
\*****************************************************************************/
static int imluaProcessBlendConst (lua_State *L)
{
  imImage *src_image1 = imlua_checkimage(L, 1);
  imImage *src_image2 = imlua_checkimage(L, 2);
  imImage *dst_image = imlua_checkimage(L, 3);
  float alpha = (float) luaL_checknumber(L, 4);

  imlua_match(L, src_image1, src_image2);
  imlua_match(L, src_image1, dst_image);

  imProcessBlendConst(src_image1, src_image2, dst_image, alpha);
  return 0;
}

/*****************************************************************************\
 im.ProcessBlend
\*****************************************************************************/
static int imluaProcessBlend (lua_State *L)
{
  imImage *src_image1 = imlua_checkimage(L, 1);
  imImage *src_image2 = imlua_checkimage(L, 2);
  imImage *alpha_image = imlua_checkimage(L, 3);
  imImage *dst_image = imlua_checkimage(L, 4);

  imlua_match(L, src_image1, src_image2);
  imlua_match(L, src_image1, dst_image);
  imlua_matchdatatype(L, src_image1, alpha_image);

  imProcessBlend(src_image1, src_image2, alpha_image, dst_image);
  return 0;
}

/*****************************************************************************\
 im.ProcessCompose
\*****************************************************************************/
static int imluaProcessCompose(lua_State *L)
{
  imImage *src_image1 = imlua_checkimage(L, 1);
  imImage *src_image2 = imlua_checkimage(L, 2);
  imImage *dst_image = imlua_checkimage(L, 3);

  imlua_match(L, src_image1, src_image2);
  imlua_match(L, src_image1, dst_image);

  imProcessCompose(src_image1, src_image2, dst_image);
  return 0;
}

/*****************************************************************************\
 im.ProcessSplitComplex
\*****************************************************************************/
static int imluaProcessSplitComplex (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image1 = imlua_checkimage(L, 2);
  imImage *dst_image2 = imlua_checkimage(L, 3);
  int polar = lua_toboolean(L, 4);

  imlua_checkdatatype(L, 1, src_image, IM_CFLOAT);
  imlua_checkdatatype(L, 2, dst_image1, IM_FLOAT);
  imlua_checkdatatype(L, 3, dst_image2, IM_FLOAT);
  imlua_matchcolorspace(L, src_image, dst_image1);
  imlua_matchcolorspace(L, src_image, dst_image2);

  imProcessSplitComplex(src_image, dst_image1, dst_image2, polar);
  return 0;
}

/*****************************************************************************\
 im.ProcessMergeComplex
\*****************************************************************************/
static int imluaProcessMergeComplex (lua_State *L)
{
  imImage *src_image1 = imlua_checkimage(L, 1);
  imImage *src_image2 = imlua_checkimage(L, 2);
  imImage *dst_image = imlua_checkimage(L, 3);
  int polar = lua_toboolean(L, 4);

  imlua_checkdatatype(L, 1, src_image1, IM_FLOAT);
  imlua_checkdatatype(L, 2, src_image2, IM_FLOAT);
  imlua_checkdatatype(L, 3, dst_image, IM_CFLOAT);
  imlua_matchcolorspace(L, src_image1, src_image2);
  imlua_matchcolorspace(L, src_image1, dst_image);

  imProcessMergeComplex(src_image1, src_image2, dst_image, polar);
  return 0;
}

/*****************************************************************************\
 im.ProcessMultipleMean
\*****************************************************************************/
static int imluaProcessMultipleMean (lua_State *L)
{
  int i, src_image_count;
  imImage *dst_image;
  imImage **src_image_list;

  luaL_checktype(L, 1, LUA_TTABLE);
  src_image_count = imlua_getn(L, 1);

  src_image_list = (imImage**)malloc(sizeof(imImage*)*src_image_count);

  for (i = 0; i < src_image_count; i++)
  {
    lua_rawgeti(L, 1, i+1);
    src_image_list[i] = imlua_checkimage(L, -1);
  }

  dst_image = imlua_checkimage(L, 2);

  for (i = 0; i < src_image_count; i++)
  {
    int check = imImageMatchDataType(src_image_list[i], dst_image);
    if (!check) free(src_image_list);
    imlua_matchcheck(L, check, "images must have the same size and data type");
  }

  imProcessMultipleMean((const imImage**)src_image_list, src_image_count, dst_image);
  free(src_image_list);
  return 0;
}

/*****************************************************************************\
 im.ProcessMultipleStdDev
\*****************************************************************************/
static int imluaProcessMultipleStdDev (lua_State *L)
{
  int i, src_image_count, check;
  imImage *dst_image, *mean_image;
  imImage **src_image_list;

  if (!lua_istable(L, 1))
    luaL_argerror(L, 1, "must be a table");

  lua_pushstring(L, "table");
#if LUA_VERSION_NUM > 501
  lua_pushglobaltable(L);
#else
  lua_gettable(L, LUA_GLOBALSINDEX);
#endif
  lua_pushstring(L, "getn");
  lua_gettable(L, -2);
  src_image_count = luaL_checkint(L, -1);
  lua_pop(L, 1);

  src_image_list = (imImage**) malloc(src_image_count * sizeof(imImage*));

  for (i = 0; i < src_image_count; i++)
  {
    lua_rawgeti(L, 1, i+1);
    src_image_list[i] = imlua_checkimage(L, -1);
  }

  mean_image = imlua_checkimage(L, 2);
  dst_image = imlua_checkimage(L, 3);

  for (i = 0; i < src_image_count; i++)
  {
    check = imImageMatchDataType(src_image_list[i], dst_image);
    if (!check) free(src_image_list);
    imlua_matchcheck(L, check, "images must have the same size and data type");
  }
  check = imImageMatchDataType(mean_image, dst_image);
  if (!check) free(src_image_list);
  imlua_matchcheck(L, check, "images must have the same size and data type");

  imProcessMultipleStdDev((const imImage**)src_image_list, src_image_count, mean_image, dst_image);
  free(src_image_list);
  return 0;
}

/*****************************************************************************\
 im.ProcessAutoCovariance
\*****************************************************************************/
static int imluaProcessAutoCovariance (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *mean_image = imlua_checkimage(L, 2);
  imImage *dst_image = imlua_checkimage(L, 3);

  imlua_match(L, src_image, mean_image);
  imlua_matchcolorspace(L, src_image, dst_image);
  imlua_checkdatatype(L, 3, dst_image, IM_FLOAT);

  lua_pushboolean(L, imProcessAutoCovariance(src_image, mean_image, dst_image));
  return 1;
}

/*****************************************************************************\
 im.ProcessMultiplyConj
\*****************************************************************************/
static int imluaProcessMultiplyConj (lua_State *L)
{
  imImage *src_image1 = imlua_checkimage(L, 1);
  imImage *src_image2 = imlua_checkimage(L, 2);
  imImage *dst_image = imlua_checkimage(L, 3);

  imlua_match(L, src_image1, src_image2);
  imlua_match(L, src_image1, dst_image);

  imProcessMultiplyConj(src_image1, src_image2, dst_image);
  return 0;
}


/*****************************************************************************\
 Additional Image Quantization Operations
\*****************************************************************************/

/*****************************************************************************\
 im.ProcessQuantizeRGBUniform
\*****************************************************************************/
static int imluaProcessQuantizeRGBUniform (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  int dither = lua_toboolean(L, 3);

  imlua_checktype(L, 1, src_image, IM_RGB, IM_BYTE);
  imlua_checkcolorspace(L, 2, dst_image, IM_MAP);
  imlua_matchsize(L, src_image, dst_image);

  imProcessQuantizeRGBUniform(src_image, dst_image, dither);
  return 0;
}

/*****************************************************************************\
 im.ProcessQuantizeGrayUniform
\*****************************************************************************/
static int imluaProcessQuantizeGrayUniform (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  int grays = luaL_checkint(L, 3);

  imlua_checktype(L, 1, src_image, IM_GRAY, IM_BYTE);
  imlua_checktype(L, 2, dst_image, IM_GRAY, IM_BYTE);
  imlua_match(L, src_image, dst_image);

  imProcessQuantizeGrayUniform(src_image, dst_image, grays);
  return 0;
}


/*****************************************************************************\
 Histogram Based Operations
\*****************************************************************************/

/*****************************************************************************\
 im.ProcessExpandHistogram
\*****************************************************************************/
static int imluaProcessExpandHistogram (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  float percent = (float) luaL_checknumber(L, 3);

  imlua_checkdatatype(L, 1, src_image, IM_BYTE);
  imlua_match(L, src_image, dst_image);
  luaL_argcheck(L, src_image->color_space == IM_RGB || src_image->color_space == IM_GRAY, 1, "color space can be RGB or Gray only");
  luaL_argcheck(L, dst_image->color_space == IM_RGB || dst_image->color_space == IM_GRAY, 2, "color space can be RGB or Gray only");

  imProcessExpandHistogram(src_image, dst_image, percent);
  return 0;
}

/*****************************************************************************\
 im.ProcessEqualizeHistogram
\*****************************************************************************/
static int imluaProcessEqualizeHistogram (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);

  imlua_checkdatatype(L, 1, src_image, IM_BYTE);
  imlua_match(L, src_image, dst_image);
  luaL_argcheck(L, src_image->color_space == IM_RGB || src_image->color_space == IM_GRAY, 1, "color space can be RGB or Gray only");
  luaL_argcheck(L, dst_image->color_space == IM_RGB || dst_image->color_space == IM_GRAY, 2, "color space can be RGB or Gray only");

  imProcessEqualizeHistogram(src_image, dst_image);
  return 0;
}



/*****************************************************************************\
 Color Processing Operations
\*****************************************************************************/

/*****************************************************************************\
 im.ProcessSplitYChroma
\*****************************************************************************/
static int imluaProcessSplitYChroma (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *y_image = imlua_checkimage(L, 2);
  imImage *chroma_image = imlua_checkimage(L, 3);

  imlua_checktype(L, 1, src_image, IM_RGB, IM_BYTE);
  imlua_checktype(L, 2, y_image, IM_GRAY, IM_BYTE);
  imlua_checktype(L, 3, chroma_image, IM_RGB, IM_BYTE);
  imlua_matchsize(L, src_image, y_image);
  imlua_matchsize(L, src_image, chroma_image);

  imProcessSplitYChroma(src_image, y_image, chroma_image);
  return 0;
}

/*****************************************************************************\
 im.ProcessSplitHSI
\*****************************************************************************/
static int imluaProcessSplitHSI (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *h_image = imlua_checkimage(L, 2);
  imImage *s_image = imlua_checkimage(L, 3);
  imImage *i_image = imlua_checkimage(L, 4);

  imlua_checkcolorspace(L, 1, src_image, IM_RGB);
  luaL_argcheck(L, src_image->data_type == IM_BYTE || src_image->data_type == IM_FLOAT, 1, "data type can be float or byte only");
  imlua_checktype(L, 2, h_image, IM_GRAY, IM_FLOAT);
  imlua_checktype(L, 3, s_image, IM_GRAY, IM_FLOAT);
  imlua_checktype(L, 4, i_image, IM_GRAY, IM_FLOAT);
  imlua_matchsize(L, src_image, h_image);
  imlua_matchsize(L, src_image, s_image);
  imlua_matchsize(L, src_image, i_image);

  imProcessSplitHSI(src_image, h_image, s_image, i_image);
  return 0;
}

/*****************************************************************************\
 im.ProcessMergeHSI
\*****************************************************************************/
static int imluaProcessMergeHSI (lua_State *L)
{
  imImage *h_image = imlua_checkimage(L, 1);
  imImage *s_image = imlua_checkimage(L, 2);
  imImage *i_image = imlua_checkimage(L, 3);
  imImage *dst_image = imlua_checkimage(L, 4);

  imlua_checktype(L, 1, h_image, IM_GRAY, IM_FLOAT);
  imlua_checktype(L, 2, s_image, IM_GRAY, IM_FLOAT);
  imlua_checktype(L, 3, i_image, IM_GRAY, IM_FLOAT);
  imlua_checkcolorspace(L, 4, dst_image, IM_RGB);
  luaL_argcheck(L, dst_image->data_type == IM_BYTE || dst_image->data_type == IM_FLOAT, 4, "data type can be float or byte only");
  imlua_matchsize(L, dst_image, h_image);
  imlua_matchsize(L, dst_image, s_image);
  imlua_matchsize(L, dst_image, i_image);

  imProcessMergeHSI(h_image, s_image, i_image, dst_image);
  return 0;
}

/*****************************************************************************\
 im.ProcessSplitComponents(src_image, { r, g, b} )
\*****************************************************************************/
static int imluaProcessSplitComponents (lua_State *L)
{
  int i, src_depth;
  imImage *src_image = imlua_checkimage(L, 1);
  imImage **dst_image_list;

  luaL_checktype(L, 2, LUA_TTABLE);

  src_depth = src_image->has_alpha? src_image->depth+1: src_image->depth;
  if (imlua_getn(L, 2) != src_depth)
    luaL_error(L, "number of destiny images must match the depth of the source image");

  dst_image_list = (imImage**)malloc(sizeof(imImage*)*src_depth);

  for (i = 0; i < src_depth; i++)
  {
    lua_pushnumber(L, i+1);
    lua_gettable(L, 2);
    dst_image_list[i] = imlua_checkimage(L, -1);
    imlua_checkcolorspace(L, 2, dst_image_list[i], IM_GRAY);  /* if error here, there will be a memory leak */
    lua_pop(L, 1);
  }

  for (i = 0; i < src_depth; i++)
  {
    int check = imImageMatchDataType(src_image, dst_image_list[i]);
    if (!check) free(dst_image_list);
    imlua_matchcheck(L, check, "images must have the same size and data type");
  }

  imProcessSplitComponents(src_image, dst_image_list);

  free(dst_image_list);

  return 0;
}

/*****************************************************************************\
 im.ProcessMergeComponents({r, g, b}, rgb)
\*****************************************************************************/
static int imluaProcessMergeComponents (lua_State *L)
{
  int i, dst_depth;
  imImage** src_image_list;
  imImage *dst_image;

  luaL_checktype(L, 1, LUA_TTABLE);
  dst_image = imlua_checkimage(L, 2);

  dst_depth = dst_image->has_alpha? dst_image->depth+1: dst_image->depth;
  if (imlua_getn(L, 1) != dst_depth)
    luaL_error(L, "number of source images must match the depth of the destination image");

  src_image_list = (imImage**)malloc(sizeof(imImage*)*dst_depth);

  for (i = 0; i < dst_depth; i++)
  {
    lua_pushnumber(L, i+1);
    lua_gettable(L, 1);
    src_image_list[i] = imlua_checkimage(L, -1);
    imlua_checkcolorspace(L, 1, src_image_list[i], IM_GRAY);
    lua_pop(L, 1);
  }

  for (i = 0; i < dst_depth; i++)
  {
    int check = imImageMatchDataType(src_image_list[i], dst_image);
    if (!check) free(src_image_list);
    imlua_matchcheck(L, check, "images must have the same size and data type");
  }

  imProcessMergeComponents((const imImage**)src_image_list, dst_image);

  free(src_image_list);

  return 0;
}

/*****************************************************************************\
 im.ProcessNormalizeComponents
\*****************************************************************************/
static int imluaProcessNormalizeComponents (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);

  imlua_checkdatatype(L, 2, dst_image, IM_FLOAT);
  imlua_matchcolorspace(L, src_image, dst_image);

  imProcessNormalizeComponents(src_image, dst_image);
  return 0;
}

/*****************************************************************************\
 im.ProcessReplaceColor
\*****************************************************************************/
static int imluaProcessReplaceColor (lua_State *L)
{
  int src_count, dst_count;
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  float *src_color = imlua_toarrayfloat(L, 3, &src_count, 1);
  float *dst_color = imlua_toarrayfloat(L, 4, &dst_count, 1);

  imlua_checknotcfloat(L, src_image, 1);
  imlua_match(L, src_image, dst_image);
  luaL_argcheck(L, src_count == src_image->depth, 3, "the colors must have the same number of components of the images");
  luaL_argcheck(L, dst_count == src_image->depth, 4, "the colors must have the same number of components of the images");

  imProcessReplaceColor(src_image, dst_image, src_color, dst_color);
  return 0;
}



/*****************************************************************************\
 Logical Arithmetic Operations
\*****************************************************************************/

/*****************************************************************************\
 im.ProcessBitwiseOp
\*****************************************************************************/
static int imluaProcessBitwiseOp (lua_State *L)
{
  imImage *src_image1 = imlua_checkimage(L, 1);
  imImage *src_image2 = imlua_checkimage(L, 2);
  imImage *dst_image = imlua_checkimage(L, 3);
  int op = luaL_checkint(L, 4);

  luaL_argcheck(L, (src_image1->data_type < IM_FLOAT), 1, "image data type can be integer only");
  imlua_match(L, src_image1, src_image2);
  imlua_match(L, src_image1, dst_image);

  imProcessBitwiseOp(src_image1, src_image2, dst_image, op);
  return 0;
}

/*****************************************************************************\
 im.ProcessBitwiseNot
\*****************************************************************************/
static int imluaProcessBitwiseNot (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);

  luaL_argcheck(L, (src_image->data_type < IM_FLOAT), 1, "image data type can be integer only");
  imlua_match(L, src_image, dst_image);

  imProcessBitwiseNot(src_image, dst_image);
  return 0;
}

/*****************************************************************************\
 im.ProcessBitMask(src_image, dst_image, mask, op)
\*****************************************************************************/
static int imluaProcessBitMask (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  unsigned char mask = imlua_checkmask(L, 3);
  int op = luaL_checkint(L, 4);

  imlua_checkdatatype(L, 1, src_image, IM_BYTE);
  imlua_match(L, src_image, dst_image);

  imProcessBitMask(src_image, dst_image, mask, op);
  return 0;
}

/*****************************************************************************\
 im.ProcessBitPlane(src_image, dst_image, plane, reset)
\*****************************************************************************/
static int imluaProcessBitPlane (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  int plane = luaL_checkint(L, 3);
  int reset = lua_toboolean(L, 4);

  imlua_checkdatatype(L, 1, src_image, IM_BYTE);
  imlua_match(L, src_image, dst_image);

  imProcessBitPlane(src_image, dst_image, plane, reset);
  return 0;
}



/*****************************************************************************\
 Synthetic Image Render
\*****************************************************************************/

/* NOTE: This breaks on multithread */
static lua_State *g_renderState = NULL;
int g_paramCount = 0;

static float imluaRenderFunc (int x, int y, int d, float *param)
{
  lua_State *L = g_renderState;

  luaL_checktype(L, 2, LUA_TFUNCTION);
  lua_pushvalue(L, 2);
  lua_pushnumber(L, x);
  lua_pushnumber(L, y);
  lua_pushnumber(L, d);
  imlua_newarrayfloat(L, param, g_paramCount, 1);

  lua_call(L, 4, 1);

  return (float) luaL_checknumber(L, -1);
}

/*****************************************************************************\
 im.ProcessRenderOp(image, function, name, param, plus)
\*****************************************************************************/
static int imluaProcessRenderOp (lua_State *L)
{
  int count;

  imImage *image = imlua_checkimage(L, 1);
  const char *render_name = luaL_checkstring(L, 3);
  float *param = imlua_toarrayfloat(L, 4, &count, 1);
  int plus = luaL_checkint(L, 5);

  imlua_checknotcfloat(L, image, 1);

  luaL_checktype(L, 2, LUA_TFUNCTION);

  g_renderState = L;
  g_paramCount = count;
  lua_pushboolean(L, imProcessRenderOp(image, imluaRenderFunc, (char*) render_name, param, plus));
  return 1;
}

static float imluaRenderCondFunc (int x, int y, int d, int *cond, float *param)
{
  lua_State *L = g_renderState;

  luaL_checktype(L, 2, LUA_TFUNCTION);
  lua_pushvalue(L, 2);
  lua_pushnumber(L, x);
  lua_pushnumber(L, y);
  lua_pushnumber(L, d);
  imlua_newarrayfloat(L, param, g_paramCount, 1);

  lua_call(L, 4, 2);

  *cond = lua_toboolean(L, -1);
  return (float) luaL_checknumber(L, -2);
}

/*****************************************************************************\
 im.ProcessRenderCondOp(image, function, name, param)
\*****************************************************************************/
static int imluaProcessRenderCondOp (lua_State *L)
{
  int count;

  imImage *image = imlua_checkimage(L, 1);
  const char *render_name = luaL_checkstring(L, 3);
  float *param = imlua_toarrayfloat(L, 4, &count, 1);

  imlua_checknotcfloat(L, image, 1);

  luaL_checktype(L, 2, LUA_TFUNCTION);

  g_renderState = L;
  g_paramCount = count;
  lua_pushboolean(L, imProcessRenderCondOp(image, imluaRenderCondFunc, (char*) render_name, param));
  return 1;
}

/*****************************************************************************\
 im.ProcessRenderAddSpeckleNoise
\*****************************************************************************/
static int imluaProcessRenderAddSpeckleNoise (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  float percent = (float) luaL_checknumber(L, 3);

  imlua_checknotcfloat(L, src_image, 1);
  imlua_match(L, src_image, dst_image);

  lua_pushboolean(L, imProcessRenderAddSpeckleNoise(src_image, dst_image, percent));
  return 1;
}

/*****************************************************************************\
 im.ProcessRenderAddGaussianNoise
\*****************************************************************************/
static int imluaProcessRenderAddGaussianNoise (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  float mean = (float) luaL_checknumber(L, 3);
  float stddev = (float) luaL_checknumber(L, 4);

  imlua_checknotcfloat(L, src_image, 1);
  imlua_match(L, src_image, dst_image);

  lua_pushboolean(L, imProcessRenderAddGaussianNoise(src_image, dst_image, mean, stddev));
  return 1;
}

/*****************************************************************************\
 im.ProcessRenderAddUniformNoise
\*****************************************************************************/
static int imluaProcessRenderAddUniformNoise (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  float mean = (float) luaL_checknumber(L, 3);
  float stddev = (float) luaL_checknumber(L, 4);

  imlua_checknotcfloat(L, src_image, 1);
  imlua_match(L, src_image, dst_image);

  lua_pushboolean(L, imProcessRenderAddUniformNoise(src_image, dst_image, mean, stddev));
  return 1;
}

/*****************************************************************************\
 im.ProcessRenderRandomNoise
\*****************************************************************************/
static int imluaProcessRenderRandomNoise (lua_State *L)
{
  imImage *image = imlua_checkimage(L, 1);
  imlua_checknotcfloat(L, image, 1);
  lua_pushboolean(L, imProcessRenderRandomNoise(image));
  return 1;
}

/*****************************************************************************\
 im.ProcessRenderConstant(image, [count])
\*****************************************************************************/
static int imluaProcessRenderConstant (lua_State *L)
{
  int i;
  float *value = NULL;

  imImage *image = imlua_checkimage(L, 1);
  int count = image->depth;

  imlua_checknotcfloat(L, image, 1);

  if (lua_istable(L, 2))
  {
    value = (float*) malloc (sizeof(float) * count);

    for (i = 0; i < count; i++)
    {
      lua_rawgeti(L, 2, i+1);
      value[i] = (float) lua_tonumber(L, -1);
      lua_pop(L, 1);
    }
  }

  lua_pushboolean(L, imProcessRenderConstant(image, value));

  if (value)
    free(value);

  return 1;
}

/*****************************************************************************\
 im.ProcessRenderWheel
\*****************************************************************************/
static int imluaProcessRenderWheel (lua_State *L)
{
  imImage *image = imlua_checkimage(L, 1);
  int int_radius = luaL_checkint(L, 2);
  int ext_radius = luaL_checkint(L, 3);

  imlua_checknotcfloat(L, image, 1);

  lua_pushboolean(L, imProcessRenderWheel(image, int_radius, ext_radius));
  return 1;
}

/*****************************************************************************\
 im.ProcessRenderCone
\*****************************************************************************/
static int imluaProcessRenderCone (lua_State *L)
{
  imImage *image = imlua_checkimage(L, 1);
  int radius = luaL_checkint(L, 2);

  imlua_checknotcfloat(L, image, 1);

  lua_pushboolean(L, imProcessRenderCone(image, radius));
  return 1;
}

/*****************************************************************************\
 im.ProcessRenderTent
\*****************************************************************************/
static int imluaProcessRenderTent (lua_State *L)
{
  imImage *image = imlua_checkimage(L, 1);
  int width = luaL_checkint(L, 2);
  int height = luaL_checkint(L, 3);

  imlua_checknotcfloat(L, image, 1);

  lua_pushboolean(L, imProcessRenderTent(image, width, height));
  return 1;
}

/*****************************************************************************\
 im.ProcessRenderRamp
\*****************************************************************************/
static int imluaProcessRenderRamp (lua_State *L)
{
  imImage *image = imlua_checkimage(L, 1);
  int start = luaL_checkint(L, 2);
  int end = luaL_checkint(L, 3);
  int dir = luaL_checkint(L, 4);

  imlua_checknotcfloat(L, image, 1);

  lua_pushboolean(L, imProcessRenderRamp(image, start, end, dir));
  return 1;
}

/*****************************************************************************\
 im.ProcessRenderBox
\*****************************************************************************/
static int imluaProcessRenderBox (lua_State *L)
{
  imImage *image = imlua_checkimage(L, 1);
  int width = luaL_checkint(L, 2);
  int height = luaL_checkint(L, 3);

  imlua_checknotcfloat(L, image, 1);

  lua_pushboolean(L, imProcessRenderBox(image, width, height));
  return 1;
}

/*****************************************************************************\
 im.ProcessRenderSinc
\*****************************************************************************/
static int imluaProcessRenderSinc (lua_State *L)
{
  imImage *image = imlua_checkimage(L, 1);
  float xperiod = (float) luaL_checknumber(L, 2);
  float yperiod = (float) luaL_checknumber(L, 3);

  imlua_checknotcfloat(L, image, 1);

  lua_pushboolean(L, imProcessRenderSinc(image, xperiod, yperiod));
  return 1;
}

/*****************************************************************************\
 im.ProcessRenderGaussian
\*****************************************************************************/
static int imluaProcessRenderGaussian (lua_State *L)
{
  imImage *image = imlua_checkimage(L, 1);
  float stddev = (float) luaL_checknumber(L, 2);

  imlua_checknotcfloat(L, image, 1);

  lua_pushboolean(L, imProcessRenderGaussian(image, stddev));
  return 1;
}

/*****************************************************************************\
 im.ProcessRenderLapOfGaussian
\*****************************************************************************/
static int imluaProcessRenderLapOfGaussian (lua_State *L)
{
  imImage *image = imlua_checkimage(L, 1);
  float stddev = (float) luaL_checknumber(L, 2);

  imlua_checknotcfloat(L, image, 1);

  lua_pushboolean(L, imProcessRenderLapOfGaussian(image, stddev));
  return 1;
}

/*****************************************************************************\
 im.ProcessRenderCosine
\*****************************************************************************/
static int imluaProcessRenderCosine (lua_State *L)
{
  imImage *image = imlua_checkimage(L, 1);
  float xperiod = (float) luaL_checknumber(L, 2);
  float yperiod = (float) luaL_checknumber(L, 3);

  imlua_checknotcfloat(L, image, 1);

  lua_pushboolean(L, imProcessRenderCosine(image, xperiod, yperiod));
  return 1;
}

/*****************************************************************************\
 im.ProcessRenderGrid
\*****************************************************************************/
static int imluaProcessRenderGrid (lua_State *L)
{
  imImage *image = imlua_checkimage(L, 1);
  int x_space = luaL_checkint(L, 2);
  int y_space = luaL_checkint(L, 3);

  imlua_checknotcfloat(L, image, 1);

  lua_pushboolean(L, imProcessRenderGrid(image, x_space, y_space));
  return 1;
}

/*****************************************************************************\
 im.ProcessRenderChessboard
\*****************************************************************************/
static int imluaProcessRenderChessboard (lua_State *L)
{
  imImage *image = imlua_checkimage(L, 1);
  int x_space = luaL_checkint(L, 2);
  int y_space = luaL_checkint(L, 3);

  imlua_checknotcfloat(L, image, 1);

  lua_pushboolean(L, imProcessRenderChessboard(image, x_space, y_space));
  return 1;
}



/*****************************************************************************\
 Tone Gamut Operations
\*****************************************************************************/

/*****************************************************************************\
 im.ProcessToneGamut
\*****************************************************************************/
static int imluaProcessToneGamut (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  int op = luaL_checkint(L, 3);
  float *param = NULL;

  imlua_checknotcfloat(L, src_image, 1);
  imlua_match(L, src_image, dst_image);

  param = imlua_toarrayfloat(L, 4, NULL, 1);

  imProcessToneGamut(src_image, dst_image, op, param);

  if (param)
    free(param);

  return 0;
}

/*****************************************************************************\
 im.ProcessUnNormalize
\*****************************************************************************/
static int imluaProcessUnNormalize (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);

  imlua_checkdatatype(L, 1, src_image, IM_FLOAT);
  imlua_checkdatatype(L, 2, dst_image, IM_BYTE);
  imlua_matchcolorspace(L, src_image, dst_image);

  imProcessUnNormalize(src_image, dst_image);
  return 0;
}

/*****************************************************************************\
 im.ProcessDirectConv
\*****************************************************************************/
static int imluaProcessDirectConv (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);

  luaL_argcheck(L,
    src_image->data_type == IM_USHORT ||
    src_image->data_type == IM_INT ||
    src_image->data_type == IM_FLOAT,
    1, "data type can be ushort, int or float only");
  imlua_checkdatatype(L, 2, dst_image, IM_BYTE);
  imlua_matchsize(L, src_image, dst_image);

  imProcessDirectConv(src_image, dst_image);
  return 0;
}

/*****************************************************************************\
 im.ProcessNegative
\*****************************************************************************/
static int imluaProcessNegative (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);

  imlua_checknotcfloat(L, src_image, 1);
  imlua_match(L, src_image, dst_image);

  imProcessNegative(src_image, dst_image);
  return 0;
}



/*****************************************************************************\
 Threshold Operations
\*****************************************************************************/

/*****************************************************************************\
 im.ProcessRangeContrastThreshold
\*****************************************************************************/
static int imluaProcessRangeContrastThreshold (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  int kernel_size = luaL_checkint(L, 3);
  int min_range = luaL_checkint(L, 4);

  imlua_checktype(L, 1, src_image, IM_GRAY, IM_BYTE);
  luaL_argcheck(L, (src_image->data_type < IM_FLOAT), 1, "image data type can be integer only");
  imlua_checkcolorspace(L, 2, dst_image, IM_BINARY);
  imlua_matchsize(L, src_image, dst_image);

  lua_pushboolean(L, imProcessRangeContrastThreshold(src_image, dst_image, kernel_size, min_range));
  return 1;
}

/*****************************************************************************\
 im.ProcessLocalMaxThreshold
\*****************************************************************************/
static int imluaProcessLocalMaxThreshold (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  int kernel_size = luaL_checkint(L, 3);
  int min_thres = luaL_checkint(L, 4);

  imlua_checktype(L, 1, src_image, IM_GRAY, IM_BYTE);
  luaL_argcheck(L, (src_image->data_type < IM_FLOAT), 1, "image data type can be integer only");
  imlua_checkcolorspace(L, 2, dst_image, IM_BINARY);
  imlua_matchsize(L, src_image, dst_image);

  lua_pushboolean(L, imProcessLocalMaxThreshold(src_image, dst_image, kernel_size, min_thres));
  return 1;
}

/*****************************************************************************\
 im.ProcessThreshold
\*****************************************************************************/
static int imluaProcessThreshold (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  int level = luaL_checkint(L, 3);
  int value = luaL_checkint(L, 4);

  imlua_checktype(L, 1, src_image, IM_GRAY, IM_BYTE);
  luaL_argcheck(L, (src_image->data_type < IM_FLOAT), 1, "image data type can be integer only");

  imlua_checkcolorspace(L, 2, dst_image, IM_BINARY);
  imlua_matchsize(L, src_image, dst_image);

  imProcessThreshold(src_image, dst_image, level, value);
  return 0;
}

/*****************************************************************************\
 im.ProcessThresholdByDiff
\*****************************************************************************/
static int imluaProcessThresholdByDiff (lua_State *L)
{
  imImage *src_image1 = imlua_checkimage(L, 1);
  imImage *src_image2 = imlua_checkimage(L, 2);
  imImage *dst_image = imlua_checkimage(L, 3);

  imlua_checktype(L, 1, src_image1, IM_GRAY, IM_BYTE);
  imlua_match(L, src_image1, src_image2);
  imlua_checkcolorspace(L, 2, dst_image, IM_BINARY);
  imlua_matchsize(L, src_image1, dst_image);

  imProcessThresholdByDiff(src_image1, src_image2, dst_image);
  return 0;
}

/*****************************************************************************\
 im.ProcessHysteresisThreshold
\*****************************************************************************/
static int imluaProcessHysteresisThreshold (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  int low_thres = luaL_checkint(L, 3);
  int high_thres = luaL_checkint(L, 4);

  imlua_checktype(L, 1, src_image, IM_GRAY, IM_BYTE);
  imlua_checkcolorspace(L, 2, dst_image, IM_BINARY);
  imlua_matchsize(L, src_image, dst_image);

  imProcessHysteresisThreshold(src_image, dst_image, low_thres, high_thres);
  return 0;
}

/*****************************************************************************\
 im.ProcessHysteresisThresEstimate
\*****************************************************************************/
static int imluaProcessHysteresisThresEstimate (lua_State *L)
{
  int low_thres, high_thres;

  imImage *src_image = imlua_checkimage(L, 1);

  imlua_checktype(L, 1, src_image, IM_GRAY, IM_BYTE);

  imProcessHysteresisThresEstimate(src_image, &low_thres, &high_thres);
  lua_pushnumber(L, low_thres);
  lua_pushnumber(L, high_thres);

  return 2;
}

/*****************************************************************************\
 im.ProcessUniformErrThreshold
\*****************************************************************************/
static int imluaProcessUniformErrThreshold (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);

  imlua_checktype(L, 1, src_image, IM_GRAY, IM_BYTE);
  imlua_checkcolorspace(L, 2, dst_image, IM_BINARY);
  imlua_matchsize(L, src_image, dst_image);

  lua_pushboolean(L, imProcessUniformErrThreshold(src_image, dst_image));
  return 1;
}

/*****************************************************************************\
 im.ProcessDifusionErrThreshold
\*****************************************************************************/
static int imluaProcessDifusionErrThreshold (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  int level = luaL_checkint(L, 3);

  imlua_checkdatatype(L, 1, src_image, IM_BYTE);
  imlua_checkdatatype(L, 2, dst_image, IM_BYTE);
  imlua_matchcheck(L, src_image->depth == dst_image->depth, "images must have the same depth");
  imlua_matchsize(L, src_image, dst_image);

  imProcessDifusionErrThreshold(src_image, dst_image, level);
  return 0;
}

/*****************************************************************************\
 im.ProcessPercentThreshold
\*****************************************************************************/
static int imluaProcessPercentThreshold (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  float percent = (float) luaL_checknumber(L, 3);

  imlua_checktype(L, 1, src_image, IM_GRAY, IM_BYTE);
  imlua_checkcolorspace(L, 2, dst_image, IM_BINARY);
  imlua_matchsize(L, src_image, dst_image);

  lua_pushboolean(L, imProcessPercentThreshold(src_image, dst_image, percent));
  return 1;
}

/*****************************************************************************\
 im.ProcessOtsuThreshold
\*****************************************************************************/
static int imluaProcessOtsuThreshold (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);

  imlua_checktype(L, 1, src_image, IM_GRAY, IM_BYTE);
  imlua_checkcolorspace(L, 2, dst_image, IM_BINARY);
  imlua_matchsize(L, src_image, dst_image);

  lua_pushnumber(L, imProcessOtsuThreshold(src_image, dst_image));
  return 1;
}

/*****************************************************************************\
 im.ProcessMinMaxThreshold
\*****************************************************************************/
static int imluaProcessMinMaxThreshold (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);

  imlua_checkcolorspace(L, 1, src_image, IM_GRAY);
  luaL_argcheck(L, (src_image->data_type < IM_FLOAT), 1, "image data type can be integer only");
  imlua_checkcolorspace(L, 2, dst_image, IM_BINARY);
  imlua_matchsize(L, src_image, dst_image);

  lua_pushboolean(L, imProcessMinMaxThreshold(src_image, dst_image));
  return 1;
}

/*****************************************************************************\
 im.ProcessLocalMaxThresEstimate
\*****************************************************************************/
static int imluaProcessLocalMaxThresEstimate (lua_State *L)
{
  int thres;
  imImage *image = imlua_checkimage(L, 1);

  imlua_checkdatatype(L, 1, image, IM_BYTE);

  imProcessLocalMaxThresEstimate(image, &thres);

  lua_pushnumber(L, thres);
  return 1;
}

/*****************************************************************************\
 im.ProcessSliceThreshold
\*****************************************************************************/
static int imluaProcessSliceThreshold (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);

  int start_level = luaL_checkint(L, 3);
  int end_level = luaL_checkint(L, 4);

  imlua_checkcolorspace(L, 1, src_image, IM_GRAY);
  luaL_argcheck(L, (src_image->data_type < IM_FLOAT), 1, "image data type can be integer only");
  imlua_checkcolorspace(L, 2, dst_image, IM_BINARY);
  imlua_matchsize(L, src_image, dst_image);

  imProcessSliceThreshold(src_image, dst_image, start_level, end_level);
  return 0;
}


/*****************************************************************************\
 Special Effects
\*****************************************************************************/

/*****************************************************************************\
 im.ProcessPixelate
\*****************************************************************************/
static int imluaProcessPixelate (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  int box_size = luaL_checkint(L, 3);

  imlua_checkdatatype(L, 1, src_image, IM_BYTE);
  imlua_match(L, src_image, dst_image);

  imProcessPixelate(src_image, dst_image, box_size);
  return 0;
}

/*****************************************************************************\
 im.ProcessPosterize
\*****************************************************************************/
static int imluaProcessPosterize (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);
  int level = luaL_checkint(L, 3);

  imlua_checkdatatype(L, 1, src_image, IM_BYTE);
  imlua_match(L, src_image, dst_image);
  luaL_argcheck(L, (level >= 1 && level <= 7), 3, "invalid level, must be >=1 and <=7");

  imProcessPosterize(src_image, dst_image, level);
  return 0;
}



static const luaL_reg improcess_lib[] = {
  {"CalcRMSError", imluaCalcRMSError},
  {"CalcSNR", imluaCalcSNR},
  {"CalcCountColors", imluaCalcCountColors},
  {"CalcHistogram", imluaCalcHistogram},
  /*{"CalcUShortHistogram", imluaCalcUShortHistogram}, done by imluaCalcHistogram */
  {"CalcGrayHistogram", imluaCalcGrayHistogram},
  {"CalcImageStatistics", imluaCalcImageStatistics},
  {"CalcHistogramStatistics", imluaCalcHistogramStatistics},
  {"CalcHistoImageStatistics", imluaCalcHistoImageStatistics},

  {"AnalyzeFindRegions", imluaAnalyzeFindRegions},
  {"AnalyzeMeasureArea", imluaAnalyzeMeasureArea},
  {"AnalyzeMeasurePerimArea", imluaAnalyzeMeasurePerimArea},
  {"AnalyzeMeasureCentroid", imluaAnalyzeMeasureCentroid},
  {"AnalyzeMeasurePrincipalAxis", imluaAnalyzeMeasurePrincipalAxis},
  {"AnalyzeMeasurePerimeter", imluaAnalyzeMeasurePerimeter},
  {"AnalyzeMeasureHoles", imluaAnalyzeMeasureHoles},

  {"ProcessPerimeterLine", imluaProcessPerimeterLine},
  {"ProcessRemoveByArea", imluaProcessRemoveByArea},
  {"ProcessFillHoles", imluaProcessFillHoles},

  {"ProcessHoughLines", imluaProcessHoughLines},
  {"ProcessHoughLinesDraw", imluaProcessHoughLinesDraw},
  {"ProcessDistanceTransform", imluaProcessDistanceTransform},
  {"ProcessRegionalMaximum", imluaProcessRegionalMaximum},

  {"ProcessReduce", imluaProcessReduce},
  {"ProcessResize", imluaProcessResize},
  {"ProcessReduceBy4", imluaProcessReduceBy4},
  {"ProcessCrop", imluaProcessCrop},
  {"ProcessAddMargins", imluaProcessAddMargins},
  {"ProcessInsert", imluaProcessInsert},

  {"ProcessCalcRotateSize", imluaProcessCalcRotateSize},
  {"ProcessRotate", imluaProcessRotate},
  {"ProcessRotateRef", imluaProcessRotateRef},
  {"ProcessRotate90", imluaProcessRotate90},
  {"ProcessRotate180", imluaProcessRotate180},
  {"ProcessMirror", imluaProcessMirror},
  {"ProcessFlip", imluaProcessFlip},
  {"ProcessRadial", imluaProcessRadial},
  {"ProcessSwirl", imluaProcessSwirl},
  {"ProcessInterlaceSplit", imluaProcessInterlaceSplit},

  {"ProcessGrayMorphConvolve", imluaProcessGrayMorphConvolve},
  {"ProcessGrayMorphErode", imluaProcessGrayMorphErode},
  {"ProcessGrayMorphDilate", imluaProcessGrayMorphDilate},
  {"ProcessGrayMorphOpen", imluaProcessGrayMorphOpen},
  {"ProcessGrayMorphClose", imluaProcessGrayMorphClose},
  {"ProcessGrayMorphTopHat", imluaProcessGrayMorphTopHat},
  {"ProcessGrayMorphWell", imluaProcessGrayMorphWell},
  {"ProcessGrayMorphGradient", imluaProcessGrayMorphGradient},

  {"ProcessBinMorphConvolve", imluaProcessBinMorphConvolve},
  {"ProcessBinMorphErode", imluaProcessBinMorphErode},
  {"ProcessBinMorphDilate", imluaProcessBinMorphDilate},
  {"ProcessBinMorphOpen", imluaProcessBinMorphOpen},
  {"ProcessBinMorphClose", imluaProcessBinMorphClose},
  {"ProcessBinMorphOutline", imluaProcessBinMorphOutline},
  {"ProcessBinMorphThin", imluaProcessBinMorphThin},

  {"ProcessMedianConvolve", imluaProcessMedianConvolve},
  {"ProcessRangeConvolve", imluaProcessRangeConvolve},
  {"ProcessRankClosestConvolve", imluaProcessRankClosestConvolve},
  {"ProcessRankMaxConvolve", imluaProcessRankMaxConvolve},
  {"ProcessRankMinConvolve", imluaProcessRankMinConvolve},

  {"ProcessConvolve", imluaProcessConvolve},
  {"ProcessConvolveDual", imluaProcessConvolveDual},
  {"ProcessConvolveRep", imluaProcessConvolveRep},
  {"ProcessConvolveSep", imluaProcessConvolveSep},
  {"ProcessCompassConvolve", imluaProcessCompassConvolve},
  {"ProcessRotateKernel", imluaProcessRotateKernel},
  {"ProcessDiffOfGaussianConvolve", imluaProcessDiffOfGaussianConvolve},
  {"ProcessLapOfGaussianConvolve", imluaProcessLapOfGaussianConvolve},
  {"ProcessMeanConvolve", imluaProcessMeanConvolve},
  {"ProcessBarlettConvolve", imluaProcessBarlettConvolve},
  {"ProcessGaussianConvolve", imluaProcessGaussianConvolve},
  {"ProcessSobelConvolve", imluaProcessSobelConvolve},
  {"ProcessPrewittConvolve", imluaProcessPrewittConvolve},
  {"ProcessSplineEdgeConvolve", imluaProcessSplineEdgeConvolve},
  {"ProcessZeroCrossing", imluaProcessZeroCrossing},
  {"ProcessCanny", imluaProcessCanny},
  {"ProcessUnsharp", imluaProcessUnsharp},
  {"ProcessSharp", imluaProcessSharp},
  {"ProcessSharpKernel", imluaProcessSharpKernel},
  {"GaussianKernelSize2StdDev", imluaGaussianKernelSize2StdDev},
  {"GaussianStdDev2KernelSize", imluaGaussianStdDev2KernelSize},

  {"ProcessUnArithmeticOp", imluaProcessUnArithmeticOp},
  {"ProcessArithmeticOp", imluaProcessArithmeticOp},
  {"ProcessArithmeticConstOp", imluaProcessArithmeticConstOp},
  {"ProcessBlendConst", imluaProcessBlendConst},
  {"ProcessBlend", imluaProcessBlend},
  {"ProcessCompose", imluaProcessCompose},
  {"ProcessSplitComplex", imluaProcessSplitComplex},
  {"ProcessMergeComplex", imluaProcessMergeComplex},
  {"ProcessMultipleMean", imluaProcessMultipleMean},
  {"ProcessMultipleStdDev", imluaProcessMultipleStdDev},
  {"ProcessAutoCovariance", imluaProcessAutoCovariance},
  {"ProcessMultiplyConj", imluaProcessMultiplyConj},

  {"ProcessQuantizeRGBUniform", imluaProcessQuantizeRGBUniform},
  {"ProcessQuantizeGrayUniform", imluaProcessQuantizeGrayUniform},

  {"ProcessExpandHistogram", imluaProcessExpandHistogram},
  {"ProcessEqualizeHistogram", imluaProcessEqualizeHistogram},

  {"ProcessSplitYChroma", imluaProcessSplitYChroma},
  {"ProcessSplitHSI", imluaProcessSplitHSI},
  {"ProcessMergeHSI", imluaProcessMergeHSI},
  {"ProcessSplitComponents", imluaProcessSplitComponents},
  {"ProcessMergeComponents", imluaProcessMergeComponents},
  {"ProcessNormalizeComponents", imluaProcessNormalizeComponents},
  {"ProcessReplaceColor", imluaProcessReplaceColor},

  {"ProcessBitwiseOp", imluaProcessBitwiseOp},
  {"ProcessBitwiseNot", imluaProcessBitwiseNot},
  {"ProcessBitMask", imluaProcessBitMask},
  {"ProcessBitPlane", imluaProcessBitPlane},

  {"ProcessRenderOp", imluaProcessRenderOp},
  {"ProcessRenderCondOp", imluaProcessRenderCondOp},
  {"ProcessRenderAddSpeckleNoise", imluaProcessRenderAddSpeckleNoise},
  {"ProcessRenderAddGaussianNoise", imluaProcessRenderAddGaussianNoise},
  {"ProcessRenderAddUniformNoise", imluaProcessRenderAddUniformNoise},
  {"ProcessRenderRandomNoise", imluaProcessRenderRandomNoise},
  {"ProcessRenderConstant", imluaProcessRenderConstant},
  {"ProcessRenderWheel", imluaProcessRenderWheel},
  {"ProcessRenderCone", imluaProcessRenderCone},
  {"ProcessRenderTent", imluaProcessRenderTent},
  {"ProcessRenderRamp", imluaProcessRenderRamp},
  {"ProcessRenderBox", imluaProcessRenderBox},
  {"ProcessRenderSinc", imluaProcessRenderSinc},
  {"ProcessRenderGaussian", imluaProcessRenderGaussian},
  {"ProcessRenderLapOfGaussian", imluaProcessRenderLapOfGaussian},
  {"ProcessRenderCosine", imluaProcessRenderCosine},
  {"ProcessRenderGrid", imluaProcessRenderGrid},
  {"ProcessRenderChessboard", imluaProcessRenderChessboard},

  {"ProcessToneGamut", imluaProcessToneGamut},
  {"ProcessUnNormalize", imluaProcessUnNormalize},
  {"ProcessDirectConv", imluaProcessDirectConv},
  {"ProcessNegative", imluaProcessNegative},

  {"ProcessRangeContrastThreshold", imluaProcessRangeContrastThreshold},
  {"ProcessLocalMaxThreshold", imluaProcessLocalMaxThreshold},
  {"ProcessThreshold", imluaProcessThreshold},
  {"ProcessThresholdByDiff", imluaProcessThresholdByDiff},
  {"ProcessHysteresisThreshold", imluaProcessHysteresisThreshold},
  {"ProcessHysteresisThresEstimate", imluaProcessHysteresisThresEstimate},
  {"ProcessUniformErrThreshold", imluaProcessUniformErrThreshold},
  {"ProcessDifusionErrThreshold", imluaProcessDifusionErrThreshold},
  {"ProcessPercentThreshold", imluaProcessPercentThreshold},
  {"ProcessOtsuThreshold", imluaProcessOtsuThreshold},
  {"ProcessMinMaxThreshold", imluaProcessMinMaxThreshold},
  {"ProcessLocalMaxThresEstimate", imluaProcessLocalMaxThresEstimate},
  {"ProcessSliceThreshold", imluaProcessSliceThreshold},

  {"ProcessPixelate", imluaProcessPixelate},
  {"ProcessPosterize", imluaProcessPosterize},

  {NULL, NULL}
};

/*****************************************************************************\
 Constants
\*****************************************************************************/
static const imlua_constant im_process_constants[] = {

  { "UN_EQL", IM_UN_EQL, NULL },
  { "UN_ABS", IM_UN_ABS, NULL },
  { "UN_LESS", IM_UN_LESS, NULL },
  { "UN_INV", IM_UN_INV, NULL },
  { "UN_SQR", IM_UN_SQR, NULL },
  { "UN_SQRT", IM_UN_SQRT, NULL },
  { "UN_LOG", IM_UN_LOG, NULL },
  { "UN_EXP", IM_UN_EXP, NULL },
  { "UN_SIN", IM_UN_SIN, NULL },
  { "UN_COS", IM_UN_COS, NULL },
  { "UN_CONJ", IM_UN_CONJ, NULL },
  { "UN_CPXNORM", IM_UN_CPXNORM, NULL },

  { "BIN_ADD", IM_BIN_ADD, NULL },
  { "BIN_SUB", IM_BIN_SUB, NULL },
  { "BIN_MUL", IM_BIN_MUL, NULL },
  { "BIN_DIV", IM_BIN_DIV, NULL },
  { "BIN_DIFF", IM_BIN_DIFF, NULL },
  { "BIN_POW", IM_BIN_POW, NULL },
  { "BIN_MIN", IM_BIN_MIN, NULL },
  { "BIN_MAX", IM_BIN_MAX, NULL },

  { "BIT_AND", IM_BIT_AND, NULL },
  { "BIT_OR", IM_BIT_OR, NULL },
  { "BIT_XOR", IM_BIT_XOR, NULL },

  { "GAMUT_NORMALIZE", IM_GAMUT_NORMALIZE, NULL },
  { "GAMUT_POW", IM_GAMUT_POW, NULL },
  { "GAMUT_LOG", IM_GAMUT_LOG, NULL },
  { "GAMUT_EXP", IM_GAMUT_EXP, NULL },
  { "GAMUT_INVERT", IM_GAMUT_INVERT, NULL },
  { "GAMUT_ZEROSTART", IM_GAMUT_ZEROSTART, NULL },
  { "GAMUT_SOLARIZE", IM_GAMUT_SOLARIZE, NULL },
  { "GAMUT_SLICE", IM_GAMUT_SLICE, NULL },
  { "GAMUT_EXPAND", IM_GAMUT_EXPAND, NULL },
  { "GAMUT_CROP", IM_GAMUT_CROP, NULL },
  { "GAMUT_BRIGHTCONT", IM_GAMUT_BRIGHTCONT, NULL },

  { NULL, -1, NULL },
};

/* from imlua_kernel.c */
void imlua_open_kernel(lua_State *L);

int imlua_open_process(lua_State *L)
{
  luaL_register(L, "im", improcess_lib);   /* leave "im" table at the top of the stack */
  imlua_regconstants(L, im_process_constants);

#ifdef IMLUA_USELOH
#include "im_process.loh"
#else
#ifdef IMLUA_USELZH
#include "im_process.lzh"
#else
  luaL_dofile(L, "im_process.lua");
#endif
#endif

  imlua_open_kernel(L);
  return 1;
}

int luaopen_imlua_process(lua_State *L)
{
  return imlua_open_process(L);
}
