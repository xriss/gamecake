/** \file
 * \brief CD+IM Lua 5 Binding
 *
 * See Copyright Notice in im_lib.h
 */

#include <string.h>
#include <memory.h>

#define CD_NO_OLD_INTERFACE

#include <im.h>
#include <im_image.h>

#include "cd.h"
#include "cdirgb.h"
#include "wd.h"

#include <lua.h>
#include <lauxlib.h>

#include <imlua.h>

#include "cdlua.h"
#include "cdlua5_private.h"


/*****************************************************************************\
 image:cdInitBitmap() -> cdBitmap
\*****************************************************************************/
static int imlua_cdInitBitmap(lua_State *L)
{
  cdBitmap* bitmap;
  imImage *image = imlua_checkimage(L, 1);

  if (!imImageIsBitmap(image))
    luaL_argerror(L, 1, "image is not a bitmap");

  if (image->color_space == IM_RGB)
  {
    if (image->has_alpha)
      bitmap = cdInitBitmap(image->width, image->height, CD_RGBA, image->data[0], image->data[1], image->data[2], image->data[3]);
    else
      bitmap = cdInitBitmap(image->width, image->height, CD_RGB, image->data[0], image->data[1], image->data[2]);
  }
  else
    bitmap = cdInitBitmap(image->width, image->height, CD_MAP, image->data[0], image->palette);

  if (!bitmap)
    luaL_error(L, "insuficient memory to create bitmap");

  cdlua_pushbitmap(L, bitmap);
  return 1;
}

/*****************************************************************************\
 image:cdCreateBitmap() -> cdBitmap
\*****************************************************************************/
static int imlua_cdCreateBitmap(lua_State *L)
{
  cdBitmap* bitmap;
  imImage *image = imlua_checkimage(L, 1);

  if (!imImageIsBitmap(image))
    luaL_argerror(L, 1, "image is not a bitmap");

  if (image->color_space == IM_RGB)
  {
    if (image->has_alpha)
      bitmap = cdCreateBitmap(image->width, image->height, CD_RGBA);
    else
      bitmap = cdCreateBitmap(image->width, image->height, CD_RGB);
  }
  else
    bitmap = cdCreateBitmap(image->width, image->height, CD_MAP);

  if (!bitmap)
    luaL_error(L, "insuficient memory to create bitmap");

  if (image->color_space == IM_RGB)
  {
    memcpy(cdBitmapGetData(bitmap, CD_IRED), image->data[0], image->plane_size);
    memcpy(cdBitmapGetData(bitmap, CD_IGREEN), image->data[1], image->plane_size);
    memcpy(cdBitmapGetData(bitmap, CD_IBLUE), image->data[2], image->plane_size);

    if (image->has_alpha)
      memcpy(cdBitmapGetData(bitmap, CD_IALPHA), image->data[3], image->plane_size);
  }
  else
  {
    memcpy(cdBitmapGetData(bitmap, CD_INDEX), image->data[0], image->plane_size);
    memcpy(cdBitmapGetData(bitmap, CD_COLORS), image->palette, image->palette_count*sizeof(long int));
  }

  cdlua_pushbitmap(L, bitmap);
  return 1;
}

/*****************************************************************************\
 cd:imImageCreate(bitmap: cdBitmap) -> imImage
\*****************************************************************************/
static int cdlua_imImageCreate(lua_State *L)
{
  imImage *image;
  cdBitmap* bitmap = cdlua_checkbitmap(L, 1);

  if (bitmap->type == CD_RGB || bitmap->type == CD_RGBA)
    image = imImageCreate(bitmap->w, bitmap->h, IM_RGB, IM_BYTE);
  else
    image = imImageCreate(bitmap->w, bitmap->h, IM_MAP, IM_BYTE);

  if (!image)
    luaL_error(L, "insuficient memory to create image");

  if (bitmap->type == CD_RGB || bitmap->type == CD_RGBA)
  {
    memcpy(image->data[0], cdBitmapGetData(bitmap, CD_IRED),   image->plane_size);
    memcpy(image->data[1], cdBitmapGetData(bitmap, CD_IGREEN), image->plane_size);
    memcpy(image->data[2], cdBitmapGetData(bitmap, CD_IBLUE),  image->plane_size);

    if (bitmap->type == CD_RGBA)
      memcpy(image->data[3], cdBitmapGetData(bitmap, CD_IALPHA),  image->plane_size);
  }
  else
  {
    memcpy(image->data[0], cdBitmapGetData(bitmap, CD_INDEX),  image->plane_size);
    memcpy(image->palette, cdBitmapGetData(bitmap, CD_COLORS), 256*sizeof(long int));
  }

  imlua_pushimage(L, image);
  return 1;
}

/*****************************************************************************\
 image:wdCanvasPutImageRect(_canvas, _x, _y, _w, _h, _xmin, _xmax, _ymin, _ymax)
\*****************************************************************************/
static int imlua_wdCanvasPutImageRect(lua_State *L)
{
  int xr, yr, wr, hr;
  imImage *image = imlua_checkimage(L, 1);
  cdCanvas* canvas = cdlua_checkcanvas(L, 2);
  double x = luaL_checknumber(L, 3);
  double y = luaL_checknumber(L, 4);
  double w = luaL_checknumber(L, 5);
  double h = luaL_checknumber(L, 6);
  int xmin = luaL_optint(L, 7, 0);
  int xmax = luaL_optint(L, 8, 0);
  int ymin = luaL_optint(L, 9, 0);
  int ymax = luaL_optint(L, 10, 0);

  if (!imImageIsBitmap(image))
    luaL_argerror(L, 1, "image is not a bitmap");

  wdCanvasWorld2Canvas(canvas, x, y, &xr, &yr);
  wdCanvasWorld2CanvasSize(canvas, w, h, &wr, &hr);

  imcdCanvasPutImage(canvas, image, xr, yr, wr, hr, xmin, xmax, ymin, ymax);
  return 0;
}

/*****************************************************************************\
 image:cdCanvasPutImageRect(_canvas, _x, _y, _w, _h, _xmin, _xmax, _ymin, _ymax)
\*****************************************************************************/
static int imlua_cdCanvasPutImageRect(lua_State *L)
{
  imImage *image = imlua_checkimage(L, 1);
  cdCanvas* canvas = cdlua_checkcanvas(L, 2);
  int x = luaL_checkint(L, 3);
  int y = luaL_checkint(L, 4);
  int w = luaL_checkint(L, 5);
  int h = luaL_checkint(L, 6);
  int xmin = luaL_optint(L, 7, 0);
  int xmax = luaL_optint(L, 8, 0);
  int ymin = luaL_optint(L, 9, 0);
  int ymax = luaL_optint(L, 10, 0);

  if (!imImageIsBitmap(image))
    luaL_argerror(L, 1, "image is not a bitmap");

  imcdCanvasPutImage(canvas, image, x, y, w, h, xmin, xmax, ymin, ymax);
  return 0;
}

/***************************************************************************\
* image:cdCanvasGetImage(_canvas, x, y: number)                             *
\***************************************************************************/
static int imlua_cdCanvasGetImage(lua_State *L)
{
  imImage *image = imlua_checkimage(L, 1);
  cdCanvas* canvas = cdlua_checkcanvas(L, 2);
  int x = luaL_optint(L, 3, 0);
  int y = luaL_optint(L, 4, 0);

  if (image->color_space != IM_RGB || image->data_type != IM_BYTE)
    luaL_argerror(L, 1, "image is not RGB/byte");

  cdCanvasGetImageRGB(canvas, image->data[0], image->data[1], image->data[2], x, y, image->width, image->height);
  return 0;
}

/***************************************************************************\
* image:cdCreateCanvas(res: number) -> cdCanvas                               *
\***************************************************************************/
static int imlua_cdCreateCanvas(lua_State * L) 
{
  cdCanvas**canvas_p, *canvas;
  char data_s[100];  

  imImage *image = imlua_checkimage(L, 1);

  if (image->color_space != IM_RGB || image->data_type != IM_BYTE)
    luaL_argerror(L, 1, "image is not RGB/byte");

  if (lua_isnoneornil(L, 2))
  {
    if (image->has_alpha)
      sprintf(data_s, "%dx%d %p %p %p %p -a", image->width, image->height,
                                              image->data[0], image->data[1], image->data[2], image->data[3]);
    else
      sprintf(data_s, "%dx%d %p %p %p", image->width, image->height,
                                        image->data[0], image->data[1], image->data[2]);
  }
  else
  {
    double res_f = luaL_checknumber(L, 2);
    if (image->has_alpha)
      sprintf(data_s, "%dx%d %p %p %p %p -r%g -a", image->width, image->height,
                                                   image->data[0], image->data[1], image->data[2], image->data[3], res_f);
    else
      sprintf(data_s, "%dx%d %p %p %p -r%g", image->width, image->height,
                                             image->data[0], image->data[1], image->data[2], res_f);
  }

  canvas = cdCreateCanvas(CD_IMAGERGB, data_s);
  if (!canvas) 
  {
    lua_pushnil(L);
    return 1;
  }

  canvas_p = (cdCanvas**) lua_newuserdata(L, sizeof(cdCanvas*));
  luaL_getmetatable(L, "cdCanvas");
  lua_setmetatable(L, -2);
  *canvas_p = canvas;

  return 1;
}

static const luaL_reg cdim_metalib[] = {
  {"imImageCreate", cdlua_imImageCreate},
  {NULL, NULL}
};

static const luaL_reg imcd_metalib[] = {
  {"cdCreateBitmap", imlua_cdCreateBitmap},
  {"cdInitBitmap", imlua_cdInitBitmap},
  {"cdCreateCanvas", imlua_cdCreateCanvas},
  {"wdCanvasPutImageRect", imlua_wdCanvasPutImageRect},
  {"cdCanvasPutImageRect", imlua_cdCanvasPutImageRect},
  {"cdCanvasGetImage", imlua_cdCanvasGetImage},

  {NULL, NULL}
};

static void createmeta (lua_State *L) 
{
  /* add methods to already created metatables */

  luaL_getmetatable(L, "imImage");
  luaL_register(L, NULL, imcd_metalib);   /* register methods */
  lua_pop(L, 1);  /* removes the metatable from the top of the stack */

  luaL_getmetatable(L, "cdBitmap");
  luaL_register(L, NULL, cdim_metalib);   /* register methods */
  lua_pop(L, 1);  /* removes the metatable from the top of the stack */
}

int cdluaim_open(lua_State *L)
{
  createmeta(L);
  return 0;
}

int luaopen_cdluaim(lua_State *L)
{
  return cdluaim_open(L);
}
