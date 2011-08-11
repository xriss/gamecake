/** \file
 * \brief LuaBinding for Lua 3
 *
 * See Copyright Notice in im_lib.h
 * $Id: im_lua3.c,v 1.1 2008/10/17 06:10:16 scuri Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <lua.h>

#include "im.h"
#include "im_lib.h"

#include <cd.h>
#include <cdlua3_private.h>

#include "imlua.h"


/***************************************************************************\
* Globals.                                                                  *
\***************************************************************************/
static int color_tag;
static int imagergb_tag;
static int imagergba_tag;
static int palette_tag;
static int imagemap_tag;
static int channel_tag;

static channel_t channel_info;

#define IMLUA_VERSION "IMLua 1.2"
/***************************************************************************\
* Creation and destruction functions.                                       *
\***************************************************************************/

/***************************************************************************\
* Creates a buffer for a RGB image.                                         *
\***************************************************************************/
static void imlua_createimagergb(void)
{
  lua_Object width, height;
  long int width_i, height_i;
  imagergb_t *imagergb_p;

  width = lua_getparam(1);
  height = lua_getparam(2);
  if (!(lua_isnumber(width) && lua_isnumber(height)))
    lua_error("imCreateImageRGB: invalid dimensions parameter!");
  width_i = (long int) lua_getnumber(width);
  height_i = (long int) lua_getnumber(height);
  if (width_i < 1 || height_i < 1)
    lua_error("imCreateImageRGB: image dimensions should be positive integers!");

  if (lua_getparam(3) != LUA_NOOBJECT)
    lua_error("imCreateImageRGB: too many parameters!");
  
  imagergb_p = (imagergb_t *) malloc(sizeof(imagergb_t));
  if (!imagergb_p) {
    lua_pushnil();
    return;
  }

  imagergb_p->width = width_i;
  imagergb_p->height = height_i;
  imagergb_p->size = width_i*height_i;
  imagergb_p->red = (unsigned char *) malloc(imagergb_p->size);
  imagergb_p->green = (unsigned char *) malloc(imagergb_p->size);
  imagergb_p->blue = (unsigned char *) malloc(imagergb_p->size);
  
  if (!(imagergb_p->red && imagergb_p->green && imagergb_p->blue)) { 
    if (imagergb_p->red) free(imagergb_p->red);
    if (imagergb_p->green) free(imagergb_p->green);
    if (imagergb_p->blue) free(imagergb_p->blue);
    free(imagergb_p);
    lua_pushnil();
    return;
  }

  memset(imagergb_p->red, 255, imagergb_p->size);
  memset(imagergb_p->green, 255, imagergb_p->size);
  memset(imagergb_p->blue, 255, imagergb_p->size);
  
  lua_pushusertag((void *) imagergb_p, imagergb_tag);
}

/***************************************************************************\
* Frees a previously allocated imagergb. We don't free imagergb_p to avoid  *
* problems if the user called killimagergb twice with the same object. The  *
* structure will be freed by a userdata "gc" fallback in LUA 3.0.           *
\***************************************************************************/
static void imlua_killimagergb(void)
{
  lua_Object imagergb;
  imagergb_t *imagergb_p;

  imagergb = lua_getparam(1);
  if (imagergb == LUA_NOOBJECT)
    lua_error("imKillImageRGB: imagergb parameter missing!");
  if (lua_isnil(imagergb))
    lua_error("imKillImageRGB: attempt to kill a NIL imagergb!");
  if (lua_tag(imagergb) != imagergb_tag)
    lua_error("imKillImageRGB: invalid imagergb parameter!");
  imagergb_p = (imagergb_t *) lua_getuserdata(imagergb);
  if (!(imagergb_p->red && imagergb_p->green && imagergb_p->blue)) 
    lua_error("imKillImageRGB: attempt to kill a killed imagergb!");

  if (lua_getparam(2) != LUA_NOOBJECT)
    lua_error("imKillImageRGB: too many parameters!");

  free(imagergb_p->red);
  free(imagergb_p->green);
  free(imagergb_p->blue);
  imagergb_p->red = NULL;
  imagergb_p->green = NULL;
  imagergb_p->blue = NULL;
}

/***************************************************************************\
* Creates a palette as a palette_tag usertag lua_Object. A palette can be   *
* considered and treated as a color table.                                  *
\***************************************************************************/
static void imlua_createpalette(void)
{
  lua_Object size;
  long int size_i;
  palette_t *palette_p;

  size = lua_getparam(1);
  if (!(lua_isnumber(size)))
    lua_error("imCreatePalette: invalid palette parameter!");
  size_i = (long int) lua_getnumber(size);
  if (size_i < 1)
    lua_error("imCreatePalette: palette size should be a positive integer!");

  if (lua_getparam(2) != LUA_NOOBJECT)
    lua_error("imCreatePalette: too many parameters!");

  palette_p = (palette_t *) malloc(sizeof(palette_t));
  if (!palette_p) {
    lua_pushnil();
    return;
  }

  palette_p->size = size_i;
  palette_p->color = (long int *) malloc(palette_p->size * sizeof(long int));
  if (!palette_p->color) {
    free(palette_p);
    lua_pushnil();
    return;
  }

  memset(palette_p->color, 255, palette_p->size * sizeof(long int));
  lua_pushusertag((void *) palette_p, palette_tag);
}

/***************************************************************************\
* Frees a previously allocated palette. We don't free palette_p to prevent  *
* a problem if the user called killpalette twice with the same object. The  *
* structure will be freed by a userdata "gc" fallback in LUA 3.0.           *
\***************************************************************************/
static void imlua_killpalette(void)
{
  lua_Object palette;
  palette_t *palette_p;

  palette = lua_getparam(1);
  if (palette == LUA_NOOBJECT)
    lua_error("imKillPalette: palette parameter missing!");
  if (lua_isnil(palette))
    lua_error("imKillPalette: attempt to kill a NIL palette!");
  if (lua_tag(palette) != palette_tag)
    lua_error("imKillPalette: invalid palette parameter!");
  palette_p = (palette_t *) lua_getuserdata(palette);
  if (!palette_p->color) 
    lua_error("imKillPalette: attempt to kill a killed palette!");

  if (lua_getparam(2) != LUA_NOOBJECT)
    lua_error("imKillPalette: too many parameters!");

  free(palette_p->color);
  palette_p->color = NULL;
}

/***************************************************************************\
* Creates a imagemap as a imagemap_tag usertag lua_Object.                  *
\***************************************************************************/
static void imlua_createimagemap(void)
{
  lua_Object width;
  lua_Object height;

  long int width_i;
  long int height_i;
  imagemap_t *imagemap_p;

  width = lua_getparam(1);
  height = lua_getparam(2);
  if (!(lua_isnumber(width) && lua_isnumber(height)))
    lua_error("imCreateImageMap: invalid dimensions parameter!");
  width_i = (long int) lua_getnumber(width);
  height_i = (long int) lua_getnumber(height);
  if (width_i < 1 || height_i < 1)
    lua_error("imCreateImageMap: imagemap dimensions should be positive integers!");

  if (lua_getparam(3) != LUA_NOOBJECT)
    lua_error("imCreateImageMap: too many parameters!");

  imagemap_p = (imagemap_t *) malloc(sizeof(imagemap_t));
  if (!imagemap_p) {
    lua_pushnil();
    return;
  }

  imagemap_p->size = width_i*height_i;
  imagemap_p->width = width_i;
  imagemap_p->height = height_i;
  imagemap_p->index = (unsigned char *) malloc(imagemap_p->size);
  if (!imagemap_p->index) {
    free(imagemap_p);
    lua_pushnil();
    return;
  }

  memset(imagemap_p->index, 0, imagemap_p->size);
  lua_pushusertag((void *) imagemap_p, imagemap_tag);
}

/***************************************************************************\
* Frees a previously allocated imagemap. We don't free imagemap_p to avoid  *
* problems if the user called killimagemap twice with the same object. The  *
* structure will be freed by a userdata "gc" fallback in LUA 3.0.           *
\***************************************************************************/
static void imlua_killimagemap(void)
{
  lua_Object imagemap;
  imagemap_t *imagemap_p;

  imagemap = lua_getparam(1);
  if (imagemap == LUA_NOOBJECT)
    lua_error("imKillImageMap: imagemap parameter missing!");
  if (lua_isnil(imagemap))
    lua_error("imKillImageMap: attempt to kill a NIL imagemap!");
  if (lua_tag(imagemap) != imagemap_tag)
    lua_error("imKillImageMap: invalid imagemap parameter!");
  imagemap_p = (imagemap_t *) lua_getuserdata(imagemap);
  if (!imagemap_p->index) 
    lua_error("imKillImageMap: attempt to kill a killed imagemap!");
    
  if (lua_getparam(2) != LUA_NOOBJECT)
    lua_error("imKillImageMap: too many parameters!");

  free(imagemap_p->index);
  imagemap_p->index = NULL;
}

/***************************************************************************\
* IM API Functions.                                                         *
\***************************************************************************/

/***************************************************************************\
* imFileFormat                                                              *
\***************************************************************************/
static void imlua_fileformat(void)
{
  lua_Object file;

  char *file_s;
  int format_i;
  int compress_i;

  int err;

  file = lua_getparam(1);
  if (!lua_isstring(file))
    lua_error("imFileFormat: invalid filename parameter!");
  file_s = (char *) lua_getstring(file);

  if (lua_getparam(2) != LUA_NOOBJECT)
    lua_error("imFileFormat: too many parameters!");

  err = imFileFormat(file_s, &format_i);

  compress_i = (format_i & IM_DEFAULT) ? 1 : 0;
  format_i = format_i & 0xFF;
  
  /* if success, return the format */
  if (err == IM_ERR_NONE) {
    lua_pushnumber( format_i);
    lua_pushnumber( compress_i);
  }
  /* if failure, return nil */
  else {
    lua_pushnil();
    lua_pushnil();
  }

  lua_pushnumber( err);
}

/***************************************************************************\
* imFileFormat                                                              *
\***************************************************************************/
static void imlua_imageinfo(void)
{
  lua_Object file;

  char *file_s;
  
  int width, height, image_type, pal_size;
  int err;
  
  file = lua_getparam(1);
  if (!lua_isstring(file))
    lua_error("imFileFormat: invalid filename parameter!");
  file_s = (char *) lua_getstring(file);

  if (lua_getparam(2) != LUA_NOOBJECT)
    lua_error("imFileFormat: too many parameters!");

  err = imImageInfo(file_s, &width, &height, &image_type, &pal_size);
  
  /* if success, return the format */
  if (err == IM_ERR_NONE) {
    lua_pushnumber( width);
    lua_pushnumber( height);
    lua_pushnumber( image_type);
    lua_pushnumber( pal_size);
  }
  /* if failure, return nil */
  else {
    lua_pushnil();
    lua_pushnil();
    lua_pushnil();
    lua_pushnil();
  }

  lua_pushnumber( err);
}

/***************************************************************************\
* imEncodeColor                                                             *
\***************************************************************************/
static void imlua_encodecolor(void)
{
  lua_Object red, green, blue;
  float red_f, green_f, blue_f;
  unsigned char red_i, green_i, blue_i;
  long int color_i;

  red = lua_getparam(1);
  green = lua_getparam(2);
  blue = lua_getparam(3);

  if (lua_getparam(4) != LUA_NOOBJECT)
    lua_error("imEncodeColor: too many parameters!");

  if (!(lua_isnumber(red) && lua_isnumber(green) && lua_isnumber(blue)))
    lua_error("imEncodeColor: invalid color component parameter!");

  red_f = (float)lua_getnumber(red);
  green_f = (float)lua_getnumber(green);
  blue_f = (float)lua_getnumber(blue);

  if (red_f < 0 || red_f > 255 || green_f < 0 || 
      green_f > 255 || blue_f < 0 || blue_f > 255)
    lua_error("imEncodeColor: color components values should be in range [0, 255]!");

  red_i = (unsigned char) (red_f);
  green_i = (unsigned char) (green_f);
  blue_i = (unsigned char) (blue_f);

  color_i = imEncodeColor(red_i, green_i, blue_i);
  lua_pushusertag((void *) color_i, color_tag);
}

/***************************************************************************\
* imDecodeColor                                                             *
\***************************************************************************/
static void imlua_decodecolor(void)
{
  lua_Object color;
  long int color_i;
  unsigned char red_i, green_i, blue_i;

  color = lua_getparam(1);
  if (lua_tag(color) != color_tag)
    lua_error("imDecodeColor: invalid color parameter!");
  color_i = (long int) lua_getuserdata(color);

  if (lua_getparam(2) != LUA_NOOBJECT)
    lua_error("imDecodeColor: too many parameters!");

  imDecodeColor(&red_i, &green_i, &blue_i, color_i);

  lua_pushnumber( red_i);
  lua_pushnumber( green_i);
  lua_pushnumber( blue_i);
}

/***************************************************************************\
* imLoadRGB                                                                 *
\***************************************************************************/
static void imlua_loadrgb(void)
{
  lua_Object file, imagergb;

  imagergb_t *imagergb_p;
  char *file_s;

  int err;

  file = lua_getparam(1);
  if (!lua_isstring(file))
    lua_error("imLoadRGB: invalid filename parameter!");
  file_s = (char *) lua_getstring(file);

  imagergb = lua_getparam(2);
  if (lua_tag(imagergb) != imagergb_tag)
    lua_error("imLoadRGB: invalid imagergb parameter!");
  imagergb_p = (imagergb_t *) lua_getuserdata(imagergb);

  if (lua_getparam(3) != LUA_NOOBJECT)
    lua_error("imLoadRGB: too many parameters!");

  err = imLoadRGB(file_s, imagergb_p->red, imagergb_p->green, imagergb_p->blue);
  lua_pushnumber( err);
}

/***************************************************************************\
* imLoadMap                                                                 *
\***************************************************************************/
static void imlua_loadmap(void)
{
  lua_Object file, imagemap, palette;

  imagemap_t *imagemap_p;
  palette_t *palette_p;
  char *file_s;

  int err;

  file = lua_getparam(1);
  if (!lua_isstring(file))
    lua_error("imLoadMap: invalid filename parameter!");
  file_s = (char *) lua_getstring(file);

  imagemap = lua_getparam(2);
  if (lua_tag(imagemap) != imagemap_tag)
    lua_error("imLoadMap: invalid imagemap parameter!");
  imagemap_p = (imagemap_t *) lua_getuserdata(imagemap);

  palette = lua_getparam(3);
  if (lua_tag(palette) != palette_tag)
    lua_error("imLoadMap: invalid palette parameter!");
  palette_p = (palette_t *) lua_getuserdata(palette);

  if (lua_getparam(4) != LUA_NOOBJECT)
    lua_error("imLoadMap: too many parameters!");

  err = imLoadMap(file_s, imagemap_p->index, palette_p->color);
  lua_pushnumber( err);
}

/***************************************************************************\
* imSaveRGB                                                                 *
\***************************************************************************/
static void imlua_savergb(void)
{
  lua_Object file, imagergb, format, compress;

  imagergb_t *imagergb_p;
  char *file_s;
  int format_i;
  int compress_i;

  int err;

  imagergb = lua_getparam(1);
  if (lua_tag(imagergb) != imagergb_tag)
    lua_error("imSaveRGB: invalid imagergb parameter!");
  imagergb_p = (imagergb_t *) lua_getuserdata(imagergb);

  format = lua_getparam(2);
  if (!lua_isnumber(format))
    lua_error("imSaveRGB: invalid format parameter!");
  format_i = (int) lua_getnumber(format);
  
  compress = lua_getparam(3);
  if (!lua_isnumber(compress))
    lua_error("imSaveRGB: invalid compression parameter!");
  compress_i = (int) lua_getnumber(compress);

  file = lua_getparam(4);
  if (!lua_isstring(file))
    lua_error("imSaveRGB: invalid filename parameter!");
  file_s = (char *) lua_getstring(file);

  if (lua_getparam(5) != LUA_NOOBJECT)
    lua_error("imSaveRGB: too many parameters!");

  err = imSaveRGB(imagergb_p->width, imagergb_p->height, format_i | (compress_i << 8), 
    imagergb_p->red, imagergb_p->green, imagergb_p->blue, file_s);
  lua_pushnumber( err);
}

/***************************************************************************\
* imSaveMap                                                                 *
\***************************************************************************/
static void imlua_savemap(void)
{
  lua_Object file, imagemap, palette, format, compress;

  imagemap_t *imagemap_p;
  palette_t *palette_p;
  char *file_s;
  int format_i;
  int compress_i;

  int err;

  imagemap = lua_getparam(1);
  if (lua_tag(imagemap) != imagemap_tag)
    lua_error("imSaveMap: invalid imagemap  parameter!");
  imagemap_p = (imagemap_t *) lua_getuserdata(imagemap);

  palette = lua_getparam(2);
  if (lua_tag(palette) != palette_tag)
    lua_error("imLoadMap: invalid palette parameter!");
  palette_p = (palette_t *) lua_getuserdata(palette);

  format = lua_getparam(3);
  if (!lua_isnumber(format))
    lua_error("imSaveMap: invalid format parameter!");
  format_i = (int) lua_getnumber(format);
  
  compress = lua_getparam(4);
  if (!lua_isnumber(compress))
    lua_error("imSaveMap: invalid compression parameter!");
  compress_i = (int) lua_getnumber(compress);

  file = lua_getparam(5);
  if (!lua_isstring(file))
    lua_error("imSaveMap: invalid filename parameter!");
  file_s = (char *) lua_getstring(file);

  if (lua_getparam(6) != LUA_NOOBJECT)
    lua_error("imSaveMap: too many parameters!");

  err = imSaveMap(imagemap_p->width, imagemap_p->height, format_i | (compress_i << 8), 
    imagemap_p->index, palette_p->size, palette_p->color, file_s);
  lua_pushnumber( err);
}

/***************************************************************************\
* imRGB2Map                                                                 *
\***************************************************************************/
static void imlua_rgb2map(void)
{
  lua_Object imagergb, imagemap, palette;

  imagemap_t *imagemap_p;
  palette_t *palette_p;
  imagergb_t *imagergb_p;

  imagergb = lua_getparam(1);
  if (lua_tag(imagergb) != imagergb_tag)
    lua_error("imRGB2Map: invalid imagergb parameter!");
  imagergb_p = (imagergb_t *) lua_getuserdata(imagergb);

  imagemap = lua_getparam(2);
  if (lua_tag(imagemap) != imagemap_tag)
    lua_error("imRGB2Map: invalid imagemap parameter!");
  imagemap_p = (imagemap_t *) lua_getuserdata(imagemap);

  palette = lua_getparam(3);
  if (lua_tag(palette) != palette_tag)
    lua_error("imRGB2Map: invalid palette parameter!");
  palette_p = (palette_t *) lua_getuserdata(palette);

  if (lua_getparam(4) != LUA_NOOBJECT)
    lua_error("imRGB2Map: too many parameters!");

  if (imagergb_p->size != imagemap_p->size)
    lua_error("imRGB2Map: images have incompatible dimensions!");
  
  imRGB2Map(imagergb_p->width, imagergb_p->height, imagergb_p->red, 
    imagergb_p->green, imagergb_p->blue, imagemap_p->index, palette_p->size, 
    palette_p->color);
}

/***************************************************************************\
* imMap2RGB                                                                 *
\***************************************************************************/
static void imlua_map2rgb(void)
{
  lua_Object imagergb, imagemap, palette;

  imagemap_t *imagemap_p;
  palette_t *palette_p;
  imagergb_t *imagergb_p;

  imagemap = lua_getparam(1);
  if (lua_tag(imagemap) != imagemap_tag)
    lua_error("imMap2RGB: invalid imagemap parameter!");
  imagemap_p = (imagemap_t *) lua_getuserdata(imagemap);

  palette = lua_getparam(2);
  if (lua_tag(palette) != palette_tag)
    lua_error("imMap2RGB: invalid palette parameter!");
  palette_p = (palette_t *) lua_getuserdata(palette);

  imagergb = lua_getparam(3);
  if (lua_tag(imagergb) != imagergb_tag)
    lua_error("imMap2RGB: invalid imagergb parameter!");
  imagergb_p = (imagergb_t *) lua_getuserdata(imagergb);

  if (lua_getparam(4) != LUA_NOOBJECT)
    lua_error("imMap2RGB: too many parameters!");

  if (imagergb_p->size != imagemap_p->size)
    lua_error("imMap2RGB: images have incompatible dimensions!");
  
  imMap2RGB(imagemap_p->width, imagemap_p->height, imagemap_p->index, palette_p->size, 
    palette_p->color, imagergb_p->red, imagergb_p->green, imagergb_p->blue);
}

/***************************************************************************\
* imMap2Gray                                                                *
\***************************************************************************/
static void imlua_map2gray(void)
{
  lua_Object imagemap, palette, graymap, grays;

  imagemap_t *imagemap_p;
  palette_t *palette_p;
  imagemap_t *graymap_p;
  palette_t *grays_p;

  imagemap = lua_getparam(1);
  if (lua_tag(imagemap) != imagemap_tag)
    lua_error("imMap2Gray: invalid imagemap parameter!");
  imagemap_p = (imagemap_t *) lua_getuserdata(imagemap);

  palette = lua_getparam(2);
  if (lua_tag(palette) != palette_tag)
    lua_error("imMap2Gray: invalid palette parameter!");
  palette_p = (palette_t *) lua_getuserdata(palette);

  graymap = lua_getparam(3);
  if (lua_tag(graymap) != imagemap_tag)
    lua_error("imMap2Gray: invalid graymap parameter!");
  graymap_p = (imagemap_t *) lua_getuserdata(graymap);

  grays = lua_getparam(4);
  if (lua_tag(grays) != palette_tag)
    lua_error("imMap2Gray: invalid grays parameter!");
  grays_p = (palette_t *) lua_getuserdata(grays);

  if (lua_getparam(5) != LUA_NOOBJECT)
    lua_error("imMap2Gray: too many parameters!");

  if (imagemap_p->size != graymap_p->size)
    lua_error("imMap2Gray: images have incompatible dimensions!");
  
  if (grays_p->size < 256)
    lua_error("imMap2Gray: grays palette should be of size 256!");

  imMap2Gray(imagemap_p->width, imagemap_p->height, imagemap_p->index, 
    palette_p->size, palette_p->color, graymap_p->index, grays_p->color);
}

/***************************************************************************\
* imRGB2Gray                                                                *
\***************************************************************************/
static void imlua_rgb2gray(void)
{
  lua_Object imagergb, graymap, grays;

  imagergb_t *imagergb_p;
  imagemap_t *graymap_p;
  palette_t *grays_p;

  imagergb = lua_getparam(1);
  if (lua_tag(imagergb) != imagergb_tag)
    lua_error("imRGB2Gray: invalid imagergb parameter!");
  imagergb_p = (imagergb_t *) lua_getuserdata(imagergb);

  graymap = lua_getparam(2);
  if (lua_tag(graymap) != imagemap_tag)
    lua_error("imRGB2Gray: invalid graymap parameter!");
  graymap_p = (imagemap_t *) lua_getuserdata(graymap);

  grays = lua_getparam(3);
  if (lua_tag(grays) != palette_tag)
    lua_error("imRGB2Gray: invalid grays parameter!");
  grays_p = (palette_t *) lua_getuserdata(grays);

  if (lua_getparam(4) != LUA_NOOBJECT)
    lua_error("imRGB2Gray: too many parameters!");

  if (imagergb_p->size != graymap_p->size)
    lua_error("imRGB2Gray: images have incompatible dimensions!");
  
  if (grays_p->size < 256)
    lua_error("imRGB2Gray: grays palette should be of size 256!");

  imRGB2Gray(imagergb_p->width, imagergb_p->height, imagergb_p->red,
    imagergb_p->green, imagergb_p->blue, graymap_p->index, grays_p->color);
}

/***************************************************************************\
* imStretch.                                                                *
\***************************************************************************/
static void imlua_stretch(void)
{
  lua_Object src, dst;

  imagemap_t *srcmap_p;
  imagemap_t *dstmap_p;
  imagergb_t *srcrgb_p;
  imagergb_t *dstrgb_p;

  src = lua_getparam(1);
  dst = lua_getparam(2);
  if (lua_getparam(3) != LUA_NOOBJECT)
    lua_error("imStretch: too many parameters!");

  if ((lua_tag(src) == imagergb_tag) && (lua_tag(dst) == imagergb_tag)) {
    srcrgb_p = (imagergb_t *) lua_getuserdata(src);
    dstrgb_p = (imagergb_t *) lua_getuserdata(dst);
    imStretch(srcrgb_p->width, srcrgb_p->height, srcrgb_p->red,
      dstrgb_p->width, dstrgb_p->height, dstrgb_p->red);
    imStretch(srcrgb_p->width, srcrgb_p->height, srcrgb_p->green,
      dstrgb_p->width, dstrgb_p->height, dstrgb_p->green);
    imStretch(srcrgb_p->width, srcrgb_p->height, srcrgb_p->blue,
      dstrgb_p->width, dstrgb_p->height, dstrgb_p->blue);
  } 
  else if ((lua_tag(src) == imagemap_tag) && (lua_tag(dst) == imagemap_tag)) {
    srcmap_p = (imagemap_t *) lua_getuserdata(src);
    dstmap_p = (imagemap_t *) lua_getuserdata(dst);
    imStretch(srcmap_p->width, srcmap_p->height, srcmap_p->index,
      dstmap_p->width, dstmap_p->height, dstmap_p->index);
  }
  else {
    lua_error("imStretch: inconsistent parameters!");
  }
}

/***************************************************************************\
* imResize.                                                                *
\***************************************************************************/
static void imlua_resize(void)
{
  lua_Object src, dst;

  imagergb_t *srcrgb_p;
  imagergb_t *dstrgb_p;

  src = lua_getparam(1);
  dst = lua_getparam(2);
  if (lua_getparam(3) != LUA_NOOBJECT)
    lua_error("imResize: too many parameters!");

  if ((lua_tag(src) == imagergb_tag) && (lua_tag(dst) == imagergb_tag)) {
    srcrgb_p = (imagergb_t *) lua_getuserdata(src);
    dstrgb_p = (imagergb_t *) lua_getuserdata(dst);
    imStretch(srcrgb_p->width, srcrgb_p->height, srcrgb_p->red,
      dstrgb_p->width, dstrgb_p->height, dstrgb_p->red);
    imStretch(srcrgb_p->width, srcrgb_p->height, srcrgb_p->green,
      dstrgb_p->width, dstrgb_p->height, dstrgb_p->green);
    imStretch(srcrgb_p->width, srcrgb_p->height, srcrgb_p->blue,
      dstrgb_p->width, dstrgb_p->height, dstrgb_p->blue);
  } 
  else {
    lua_error("imResize: parameters must be of type imagergb_tag!");
  }
}

/***************************************************************************\
* imVersion.                                                                *
\***************************************************************************/
static void imlua_version(void)
{
  if (lua_getparam(1) != LUA_NOOBJECT)
    lua_error("imVersion: too many parameters!");
 
  lua_pushstring(imVersion());
}

/***************************************************************************\
* Fallback implementation.                                                  *
\***************************************************************************/

/***************************************************************************\
* imagemap "settable" fallback.                                             *
\***************************************************************************/
static void imagemapsettable_fb(void)
{
  lua_Object imagemap, index, value;

  imagemap_t *imagemap_p;
  long int index_i;
  long int value_i;

  imagemap = lua_getparam(1);
  index = lua_getparam(2);
  value = lua_getparam(3);

  imagemap_p = (imagemap_t *) lua_getuserdata(imagemap);
  if (!imagemap_p) {
    lua_error("imagemap_tag \"settable\": invalid imagemap_tag object!");
  }

  if (!lua_isnumber(index)) {
    lua_error("imagemap_tag \"settable\": index should be a number!");
  }

  if (!lua_isnumber(value)) {
    lua_error("imagemap_tag \"settable\": value should be a number!");
  }
  
  value_i = (long int) lua_getnumber(value);
  if ((value_i < 0 || value_i > 255)) 
    lua_error("imagemap_tag \"settable\": value should be in range [0, 255]!");

  index_i = (long int) lua_getnumber(index);
  if (index_i < 0 || index_i >= imagemap_p->size)
    lua_error("imagemap_tag \"settable\": index is out of bounds!");

  imagemap_p->index[index_i] = (unsigned char) value_i;
}

/***************************************************************************\
* palette "settable" fallback.                                              *
\***************************************************************************/
static void palettesettable_fb(void)
{
  lua_Object palette, index, color;
  
  palette_t *palette_p;
  long int index_i;
  long int color_i;

  palette = lua_getparam(1);
  index = lua_getparam(2);
  color = lua_getparam(3);

  palette_p = (palette_t *) lua_getuserdata(palette);
  if (!palette_p) {
    lua_error("palette_tag \"settable\": invalid palette_tag object!");
  }

  if (!lua_isnumber(index)) {
    lua_error("palette_tag \"settable\": index should be a number!");
  }

  if (lua_tag(color) != color_tag) 
    lua_error("palette_tag \"settable\": value should be of type color_tag!");
  
  color_i = (long int) lua_getuserdata(color);
  
  index_i = (long int) lua_getnumber(index);
  if (index_i < 0 || index_i >= palette_p->size)
    lua_error("palette_tag \"settable\": index is out of bounds!");

  palette_p->color[index_i] = color_i;
}

/***************************************************************************\
* channel "settable" fallback. This fallback is called when a LUA line like *
* "imagergb.r[y*w + x] = c" is executed. The imagergb "gettable" fallback   *
* fills and returns a channel structure with info about the buffer. This    *
* structure is consulted and the value is assigned where it should.         *
\***************************************************************************/
static void channelsettable_fb(void)
{
  lua_Object channel, index, value;
  
  channel_t *channel_p;
  long int index_i;
  long int value_i;

  channel = lua_getparam(1);
  index = lua_getparam(2);
  value = lua_getparam(3);

  channel_p = (channel_t *) lua_getuserdata(channel);
  if (!channel_p) {
    lua_error("channel_tag \"settable\": invalid channel_tag object!");
  }

  if (!lua_isnumber(index)) {
    lua_error("channel_tag \"settable\": index should be a number!");
  }
  index_i = (long int) lua_getnumber(index);
  if (index_i < 0 || index_i >= channel_p->size) {
    lua_error("channel_tag \"settable\": index is out of bounds!");
  }
  
  if (!lua_isnumber(value)) {
    lua_error("channel_tag \"settable\": value should be a number!");
  }
  value_i = (long int) lua_getnumber(value);
  if ((value_i < 0 || value_i > 255)) {
    lua_error("channel_tag \"settable\": value should be in range [0, 255]!");
  }

  channel_p->value[index_i] = (unsigned char) value_i;
}

/***************************************************************************\
* imagemap "gettable" fallback.                                             *
\***************************************************************************/
static void imagemapgettable_fb(void)
{
  lua_Object imagemap, index;

  imagemap_t *imagemap_p;
  long int index_i;

  imagemap = lua_getparam(1);
  index = lua_getparam(2);

  imagemap_p = (imagemap_t *) lua_getuserdata(imagemap);
  if (!imagemap_p)
    lua_error("imagemap_tag \"gettable\": invalid imagemap_tag object!");

  if (!lua_isnumber(index)) {
    lua_error("imagemap_tag \"gettable\": index should be a number!");
  }

  index_i = (long int) lua_getnumber(index);
  if (index_i < 0 || index_i >= imagemap_p->size)
    lua_error("imagemap_tag \"gettable\": index is out of bounds!");

  lua_pushnumber( imagemap_p->index[index_i]);
}

/***************************************************************************\
* palette "gettable" fallback.                                              *
\***************************************************************************/
static void palettegettable_fb(void)
{
  lua_Object palette, index;
  
  palette_t *palette_p;
  long int index_i;

  palette = lua_getparam(1);
  index = lua_getparam(2);

  palette_p = (palette_t *) lua_getuserdata(palette);
  if (!palette_p)
    lua_error("palette_tag \"gettable\": invalid palette_tag object!");

  if (!lua_isnumber(index)) {
    lua_error("palette_tag \"gettable\": index should be a number!");
  }

  index_i = (long int) lua_getnumber(index);
  if (index_i < 0 || index_i >= palette_p->size)
    lua_error("palette_tag \"gettable\": index is out of bounds!");

  lua_pushusertag((void *) palette_p->color[index_i], color_tag);
}

/***************************************************************************\
* channel "gettable" fallback. This fallback is called when a LUA line like *
* "c = imagergb.r[y*w + x]" is executed. The imagergb "gettable" fallback   *
* fills and returns a channel structure with info about the buffer. This    *
* structure is consulted and the appropriate value is returned.             *
\***************************************************************************/
static void channelgettable_fb(void)
{
  lua_Object channel, index;
  
  channel_t *channel_p;
  long int index_i;

  channel = lua_getparam(1);
  index = lua_getparam(2);

  channel_p = (channel_t *) lua_getuserdata(channel);
  if (!channel_p) {
    lua_error("channel_tag \"gettable\": invalid channel_tag object!");
  }

  if (!lua_isnumber(index)) {
    lua_error("channel_tag \"gettable\": index should be a number!");
  }
  index_i = (long int) lua_getnumber(index);
  if (index_i < 0 || index_i >= channel_p->size) {
    lua_error("channel_tag \"gettable\": index is out of bounds!");
  }
  
  lua_pushnumber( channel_p->value[index_i]);
}

/***************************************************************************\
* imagergb "gettable" fallback. This fallback is called when a LUA line     *
* like "c = imagergb.r[y*w + x]" or "imagergb.r[y*w + x] = c" is executed.  *
* The channel_info global is filled and its address is returned with a      *
* channel_tag usertag lua_Object. The following "gettable" or "settable"    *
* then assigns or returns the appropriate value.                            *
\***************************************************************************/
static void imagergbgettable_fb(void)
{
  lua_Object imagergb, index;
  
  char *index_s;
  imagergb_t *imagergb_p;

  imagergb = lua_getparam(1);
  index = lua_getparam(2);

  imagergb_p = (imagergb_t *) lua_getuserdata(imagergb);
  if (!imagergb_p)
    lua_error("imagergb_tag \"gettable\": invalid imagergb_tag object!");

  if (!lua_isstring(index)) {
    lua_error("imagergb_tag \"gettable\": index should be a channel name!");
  }
  index_s = (char *) lua_getstring(index);

  channel_info.size = imagergb_p->size;
  
  if (*index_s == 'r' || *index_s == 'R') {
    channel_info.value = imagergb_p->red;
  }
  else if (*index_s == 'g' || *index_s == 'G') {
    channel_info.value = imagergb_p->green;
  }
  else if (*index_s == 'b' || *index_s == 'B') {
    channel_info.value = imagergb_p->blue;
  }
  else {
    lua_error("imagergb_tag \"gettable\": index is an invalid channel name!");
  }

  lua_pushusertag((void *) &channel_info, channel_tag);
}

/***************************************************************************\
* imagergba "gettable" fallback. This fallback is called when a LUA line    *
* like "c = imagergba.r[y*w + x]" or "imagergba.r[y*w + x] = c" is executed.*
* The channel_info global is filled and its address is returned with a      *
* channel_tag usertag lua_Object. The following "gettable" or "settable"    *
* then assigns or returns the appropriate value.                            *
\***************************************************************************/
static void imagergbagettable_fb(void)
{
  lua_Object imagergba, index;
  
  char *index_s;
  imagergba_t *imagergba_p;

  imagergba = lua_getparam(1);
  index = lua_getparam(2);

  imagergba_p = (imagergba_t *) lua_getuserdata(imagergba);
  if (!imagergba_p)
    lua_error("imagergba_tag \"gettable\": invalid imagergba_tag object!");

  if (!lua_isstring(index)) {
    lua_error("imagergba_tag \"gettable\": index should be a channel name!");
  }
  index_s = (char *) lua_getstring(index);

  channel_info.size = imagergba_p->size;
  
  if (*index_s == 'r' || *index_s == 'R') {
    channel_info.value = imagergba_p->red;
  }
  else if (*index_s == 'g' || *index_s == 'G') {
    channel_info.value = imagergba_p->green;
  }
  else if (*index_s == 'b' || *index_s == 'B') {
    channel_info.value = imagergba_p->blue;
  }
  else if (*index_s == 'a' || *index_s == 'A') {
    channel_info.value = imagergba_p->alpha;
  }
  else {
    lua_error("imagergba_tag \"gettable\": index is an invalid channel name!");
  }

  lua_pushusertag((void *) &channel_info, channel_tag);
}

/***************************************************************************\
* palette "gc" fallback.                                                    *
\***************************************************************************/
static void palettegc_fb(void)
{
  lua_Object palette;

  palette_t *palette_p;

  palette = lua_getparam(1);
  palette_p = (palette_t *) lua_getuserdata(palette);
  if (!palette_p)
    lua_error("palette_tag \"gc\": invalid palette_tag object!");

  /* if the palette has not been killed, kill it */
  if (palette_p->color) free(palette_p->color);

  /* free the palette_t structure */
  free(palette_p);
}

/***************************************************************************\
* imagergb "gc" fallback.                                                   *
\***************************************************************************/
static void imagergbgc_fb(void)
{
  lua_Object imagergb;

  imagergb_t *imagergb_p;

  imagergb = lua_getparam(1);
  imagergb_p = (imagergb_t *) lua_getuserdata(imagergb);
  if (!imagergb_p)
    lua_error("imagergb_tag \"gc\": invalid imagergb_tag object!");

  /* if the imagergb has not been killed, kill it */
  if (imagergb_p->red) free(imagergb_p->red);
  if (imagergb_p->green) free(imagergb_p->green);
  if (imagergb_p->blue) free(imagergb_p->blue);

  /* free the imagergb_t structure */
  free(imagergb_p);
}

/***************************************************************************\
* imagergba "gc" fallback.                                                   *
\***************************************************************************/
static void imagergbagc_fb(void)
{
  lua_Object imagergba;

  imagergba_t *imagergba_p;

  imagergba = lua_getparam(1);
  imagergba_p = (imagergba_t *) lua_getuserdata(imagergba);
  if (!imagergba_p)
    lua_error("imagergba_tag \"gc\": invalid imagergba_tag object!");

  /* if the imagergba has not been killed, kill it */
  if (imagergba_p->red) free(imagergba_p->red);
  if (imagergba_p->green) free(imagergba_p->green);
  if (imagergba_p->blue) free(imagergba_p->blue);
  if (imagergba_p->alpha) free(imagergba_p->alpha);

  /* free the imagergba_t structure */
  free(imagergba_p);
}

/***************************************************************************\
* imagemap "gc" fallback.                                                   *
\***************************************************************************/
static void imagemapgc_fb(void)
{
  lua_Object imagemap;

  imagemap_t *imagemap_p;

  imagemap = lua_getparam(1);
  imagemap_p = (imagemap_t *) lua_getuserdata(imagemap);
  if (!imagemap_p)
    lua_error("imagemap_tag \"gc\": invalid imagemap_tag object!");

  /* if the imagemap has not been killed, kill it */
  if (imagemap_p->index) free(imagemap_p->index);

  /* free the imagemap_t structure */
  free(imagemap_p);
}

/***************************************************************************\
* Initialization code.                                                      *
\***************************************************************************/

/***************************************************************************\
* Initializes IMLua.                                                        *
\***************************************************************************/
void imlua_open(void)
{
  lua_Object cdlua_tag;
  
  /* check if CD has been initialized */
  cdlua_tag = lua_getglobal("CDLUA_INSTALLED");

  /* get CD defined tags, let CD deal with the user tag objects  */
  if ((cdlua_tag != LUA_NOOBJECT) && (!lua_isnil(cdlua_tag))) {
    cdlua_tag = lua_getglobal("CDLUA_COLOR_TAG");
    color_tag = (int) lua_getnumber(cdlua_tag);
    cdlua_tag = lua_getglobal("CDLUA_IMAGERGB_TAG");
    imagergb_tag = (int) lua_getnumber(cdlua_tag);
    cdlua_tag = lua_getglobal("CDLUA_IMAGERGBA_TAG");
    imagergba_tag = (int) lua_getnumber(cdlua_tag);
    cdlua_tag = lua_getglobal("CDLUA_PALETTE_TAG");
    palette_tag = (int) lua_getnumber(cdlua_tag);
    cdlua_tag = lua_getglobal("CDLUA_IMAGEMAP_TAG");
    imagemap_tag = (int) lua_getnumber(cdlua_tag);
    cdlua_tag = lua_getglobal("CDLUA_CHANNEL_TAG");
    channel_tag = (int) lua_getnumber(cdlua_tag);
  }
  /* define IM own tags and fallbacks  */
  else {
    color_tag     = lua_newtag();
    imagergb_tag  = lua_newtag();
    imagergba_tag = lua_newtag();
    imagemap_tag  = lua_newtag();
    palette_tag   = lua_newtag();
    channel_tag   = lua_newtag();

    /* associate the fallbacks */
    lua_pushcfunction(palettesettable_fb); lua_settagmethod(palette_tag, "settable");
    lua_pushcfunction(channelsettable_fb); lua_settagmethod(channel_tag, "settable");
    lua_pushcfunction(imagemapsettable_fb); lua_settagmethod(imagemap_tag, "settable");
  
    lua_pushcfunction(imagergbgettable_fb); lua_settagmethod(imagergb_tag, "gettable");
    lua_pushcfunction(imagergbagettable_fb); lua_settagmethod(imagergba_tag, "gettable");
    lua_pushcfunction(palettegettable_fb); lua_settagmethod(palette_tag, "gettable");
    lua_pushcfunction(imagemapgettable_fb); lua_settagmethod(imagemap_tag, "gettable");
    lua_pushcfunction(channelgettable_fb); lua_settagmethod(channel_tag, "gettable");

    lua_pushcfunction(imagergbgc_fb); lua_settagmethod(imagergb_tag, "gc");
    lua_pushcfunction(imagergbagc_fb); lua_settagmethod(imagergba_tag, "gc");
    lua_pushcfunction(palettegc_fb); lua_settagmethod(palette_tag, "gc");
    lua_pushcfunction(imagemapgc_fb); lua_settagmethod(imagemap_tag, "gc");
  }

  /* register used tags in global context for other libraries use */
  lua_pushnumber(1.0f); lua_setglobal("IMLUA_INSTALLED");
  lua_pushnumber( color_tag); lua_setglobal("IMLUA_COLOR_TAG");
  lua_pushnumber( imagergb_tag); lua_setglobal("IMLUA_IMAGERGB_TAG");
  lua_pushnumber( imagergba_tag); lua_setglobal("IMLUA_IMAGERGBA_TAG");
  lua_pushnumber( imagemap_tag); lua_setglobal("IMLUA_IMAGEMAP_TAG");
  lua_pushnumber( palette_tag); lua_setglobal("IMLUA_PALETTE_TAG");
  lua_pushnumber( channel_tag); lua_setglobal("IMLUA_CHANNEL_TAG");

  /* registered IM functions */
  lua_register("imDecodeColor",         imlua_decodecolor);
  lua_register("imEncodeColor",         imlua_encodecolor);
  lua_register("imLoadRGB",             imlua_loadrgb);
  lua_register("imLoadMap",             imlua_loadmap);
  lua_register("imSaveRGB",             imlua_savergb);
  lua_register("imSaveMap",             imlua_savemap);
  lua_register("imFileFormat",          imlua_fileformat);
  lua_register("imImageInfo",           imlua_imageinfo);
  lua_register("imRGB2Map",             imlua_rgb2map);
  lua_register("imMap2RGB",             imlua_map2rgb);
  lua_register("imRGB2Gray",            imlua_rgb2gray);
  lua_register("imMap2Gray",            imlua_map2gray);
  lua_register("imVersion",             imlua_version);
  lua_register("imResize",              imlua_resize);
  lua_register("imStretch",             imlua_stretch);
  
  /* creation and destruction functions */
  lua_register("imCreateImageRGB",      imlua_createimagergb);
  lua_register("imCreateImageMap",      imlua_createimagemap);
  lua_register("imCreatePalette",       imlua_createpalette);
  lua_register("imKillImageRGB",        imlua_killimagergb);
  lua_register("imKillImageMap",        imlua_killimagemap);
  lua_register("imKillPalette",         imlua_killpalette);

  /* im constants */
  lua_pushnumber( IM_BMP); lua_setglobal("IM_BMP");
  lua_pushnumber( IM_PCX); lua_setglobal("IM_PCX");
  lua_pushnumber( IM_GIF); lua_setglobal("IM_GIF");
  lua_pushnumber( IM_TIF); lua_setglobal("IM_TIF");
  lua_pushnumber( IM_RAS); lua_setglobal("IM_RAS");
  lua_pushnumber( IM_SGI); lua_setglobal("IM_SGI");
  lua_pushnumber( IM_JPG); lua_setglobal("IM_JPG");
  lua_pushnumber( IM_LED); lua_setglobal("IM_LED");
  lua_pushnumber( IM_TGA); lua_setglobal("IM_TGA");

  lua_pushnumber( 0); lua_setglobal("IM_NONE");
  lua_pushnumber( 1); lua_setglobal("IM_DEFAULT");
  lua_pushnumber( 2); lua_setglobal("IM_COMPRESSED");

  lua_pushnumber( IM_RGB); lua_setglobal("IM_RGB");
  lua_pushnumber( IM_MAP); lua_setglobal("IM_MAP");

  lua_pushnumber( IM_ERR_NONE); lua_setglobal("IM_ERR_NONE");
  lua_pushnumber( IM_ERR_OPEN); lua_setglobal("IM_ERR_OPEN");
  lua_pushnumber( IM_ERR_READ); lua_setglobal("IM_ERR_READ");
  lua_pushnumber( IM_ERR_WRITE); lua_setglobal("IM_ERR_WRITE");
  lua_pushnumber( IM_ERR_FORMAT); lua_setglobal("IM_ERR_FORMAT");
  lua_pushnumber( IM_ERR_TYPE); lua_setglobal("IM_ERR_TYPE");
  lua_pushnumber( IM_ERR_COMP); lua_setglobal("IM_ERR_COMP");
}
