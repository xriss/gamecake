/** \file
 * \brief IM Lua 5 Binding
 *
 * See Copyright Notice in im_lib.h
 * $Id: imlua_util.c,v 1.2 2010/10/25 18:29:07 scuri Exp $
 */

#include "im.h"
#include "im_util.h"
#include "im_image.h"

#include <lua.h>
#include <lauxlib.h>

#include "imlua.h"
#include "imlua_aux.h"

/*****************************************************************************\
 im.ImageDataSize(width, height, color_mode, data_type)
\*****************************************************************************/
static int imluaImageDataSize (lua_State *L)
{
  int width = luaL_checkint(L, 1);
  int height = luaL_checkint(L, 2);
  int color_mode = luaL_checkint(L, 3);
  int data_type = luaL_checkint(L, 4);

  lua_pushnumber(L, imImageDataSize(width, height, color_mode, data_type));
  return 1;
}

/*****************************************************************************\
 im.ImageLineSize(width, color_mode, data_type)
\*****************************************************************************/
static int imluaImageLineSize (lua_State *L)
{
  int width = luaL_checkint(L, 1);
  int color_mode = luaL_checkint(L, 2);
  int data_type = luaL_checkint(L, 3);

  lua_pushnumber(L, imImageLineSize(width, color_mode, data_type));
  return 1;
}

/*****************************************************************************\
 im.ImageLineCount(width, color_mode)
\*****************************************************************************/
static int imluaImageLineCount (lua_State *L)
{
  int width = luaL_checkint(L, 1);
  int color_mode = luaL_checkint(L, 2);

  lua_pushnumber(L, imImageLineCount(width, color_mode));
  return 1;
}

/*****************************************************************************\
 im.ImageCheckFormat(width, color_mode)
\*****************************************************************************/
static int imluaImageCheckFormat (lua_State *L)
{
  int color_mode = luaL_checkint(L, 1);
  int data_type = luaL_checkint(L, 2);

  lua_pushboolean(L, imImageCheckFormat(color_mode, data_type));
  return 1;
}

/*****************************************************************************\
 im.ColorModeSpaceName(color_mode)
\*****************************************************************************/
static int imluaColorModeSpaceName (lua_State *L)
{
  lua_pushstring(L, imColorModeSpaceName(luaL_checkint(L, 1)));
  return 1;
}

/*****************************************************************************\
 im.ColorModeDepth(color_mode)
\*****************************************************************************/
static int imluaColorModeDepth (lua_State *L)
{
  lua_pushnumber(L, imColorModeDepth(luaL_checkint(L, 1)));
  return 1;
}

/*****************************************************************************\
 
\*****************************************************************************/

/*****************************************************************************\
 im.ColorModeSpace(color_mode)
\*****************************************************************************/
static int imluaColorModeSpace (lua_State *L)
{
  lua_pushnumber(L, imColorModeSpace(luaL_checkint(L, 1)));
  return 1;
}

/*****************************************************************************\
 im.ColorModeHasAlpha(color_mode)
\*****************************************************************************/
static int imluaColorModeMatch (lua_State *L)
{
  lua_pushboolean(L, imColorModeMatch(luaL_checkint(L, 1), luaL_checkint(L, 1)));
  return 1;
}

/*****************************************************************************\
 im.ColorModeHasAlpha(color_mode)
\*****************************************************************************/
static int imluaColorModeHasAlpha (lua_State *L)
{
  lua_pushboolean(L, imColorModeHasAlpha(luaL_checkint(L, 1)));
  return 1;
}

/*****************************************************************************\
 im.ColorModeIsPacked(color_mode)
\*****************************************************************************/
static int imluaColorModeIsPacked (lua_State *L)
{
  lua_pushboolean(L, imColorModeIsPacked(luaL_checkint(L, 1)));
  return 1;
}

/*****************************************************************************\
 im.ColorModeIsTopDown(color_mode)
\*****************************************************************************/
static int imluaColorModeIsTopDown (lua_State *L)
{
  lua_pushboolean(L, imColorModeIsTopDown(luaL_checkint(L, 1)));
  return 1;
}

/*****************************************************************************\
 im.ColorModeToBitmap(color_mode)
\*****************************************************************************/
static int imluaColorModeToBitmap (lua_State *L)
{
  lua_pushnumber(L, imColorModeToBitmap(luaL_checkint(L, 1)));
  return 1;
}

/*****************************************************************************\
 im.ColorModeIsBitmap
\*****************************************************************************/
static int imluaColorModeIsBitmap (lua_State *L)
{
  int color_mode = luaL_checkint(L, 1);
  int data_type = luaL_checkint(L, 2);

  lua_pushboolean(L, imColorModeIsBitmap(color_mode, data_type));
  return 1;
}

/*****************************************************************************\
 im.DataTypeSize(data_type)
\*****************************************************************************/
static int imluaDataTypeSize (lua_State *L)
{
  lua_pushnumber(L, imDataTypeSize(luaL_checkint(L, 1)));
  return 1;
}

/*****************************************************************************\
 im.DataTypeName(data_type)
\*****************************************************************************/
static int imluaDataTypeName (lua_State *L)
{
  lua_pushstring(L, imDataTypeName(luaL_checkint(L, 1)));
  return 1;
}

/*****************************************************************************\
 im.DataTypeIntMax(data_type)
\*****************************************************************************/
static int imluaDataTypeIntMax(lua_State *L)
{
  lua_pushnumber(L, imDataTypeIntMax(luaL_checkint(L, 1)));
  return 1;
}

/*****************************************************************************\
 im.DataTypeIntMin(data_type)
\*****************************************************************************/
static int imluaDataTypeIntMin(lua_State *L)
{
  lua_pushnumber(L, imDataTypeIntMin(luaL_checkint(L, 1)));
  return 1;
}

/***************************************************************************\
* Creates a color as a light userdata. The color value is                   *
* placed in the (void *) value. Not beautiful, but works best.              *
* im.ColorEncode(r, g, b: number) -> (c: color)                             *
\***************************************************************************/
static int imlua_colorencode(lua_State *L)
{
  int red_f, green_f, blue_f;
  unsigned char red_i, green_i, blue_i;
  long color_i;

  red_f   = luaL_checkint(L, 1);
  green_f = luaL_checkint(L, 2);
  blue_f  = luaL_checkint(L, 3);

  if (red_f < 0 || red_f > 255)
    luaL_argerror(L, 1, "color components values should be in range [0, 255]");
  if (green_f < 0 || green_f > 255)
    luaL_argerror(L, 2, "color components values should be in range [0, 255]");
  if (blue_f < 0 ||  blue_f > 255)
    luaL_argerror(L, 3, "color components values should be in range [0, 255]");
  
  red_i   = (unsigned char) (red_f);
  green_i = (unsigned char) (green_f);
  blue_i  = (unsigned char) (blue_f);

  color_i = imColorEncode(red_i, green_i, blue_i);
  lua_pushlightuserdata(L, (void *)color_i);
  
  return 1;
}

/***************************************************************************\
* Decodes a color previously created.                                       *
* im.ColorDecode(c: color) -> (r, g, b: number)                             *
\***************************************************************************/
static int imlua_colordecode(lua_State *L)
{
  long color_i;
  unsigned char red_i, green_i, blue_i;

  if (!lua_islightuserdata(L, 1))
    luaL_argerror(L, 1, "color must be a light user data");

  color_i = (long)lua_touserdata(L,1);

  imColorDecode(&red_i, &green_i, &blue_i, color_i);
  lua_pushnumber(L, red_i);
  lua_pushnumber(L, green_i);
  lua_pushnumber(L, blue_i);

  return 3;
}

static const luaL_reg imutil_lib[] = {
  {"ImageDataSize", imluaImageDataSize},
  {"ImageLineSize", imluaImageLineSize},
  {"ImageLineCount", imluaImageLineCount},
  {"ImageCheckFormat", imluaImageCheckFormat},

  {"ColorModeSpace", imluaColorModeSpace},
  {"ColorModeSpaceName", imluaColorModeSpaceName},
  {"ColorModeDepth", imluaColorModeDepth},

  {"ColorModeToBitmap", imluaColorModeToBitmap},
  {"ColorModeIsBitmap", imluaColorModeIsBitmap},
  {"ColorModeMatch", imluaColorModeMatch},
  {"ColorModeHasAlpha", imluaColorModeHasAlpha},
  {"ColorModeIsPacked", imluaColorModeIsPacked},
  {"ColorModeIsTopDown", imluaColorModeIsTopDown},

  {"DataTypeSize", imluaDataTypeSize},
  {"DataTypeName", imluaDataTypeName},
  {"DataTypeIntMax", imluaDataTypeIntMax},
  {"DataTypeIntMin", imluaDataTypeIntMin},

  {"ColorEncode", imlua_colorencode},
  {"ColorDecode", imlua_colordecode},

  {NULL, NULL}
};

void imlua_open_util(lua_State *L)
{
  /* "im" table is at the top of the stack */
  luaL_register(L, NULL, imutil_lib);
}
