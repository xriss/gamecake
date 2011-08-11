/** \file
 * \brief IM Lua 5 Binding
 *
 * See Copyright Notice in im_lib.h
 * $Id: imlua_aux.c,v 1.5 2010/07/18 03:04:23 scuri Exp $
 */

#include <memory.h>
#include <stdlib.h>
#include <string.h>

#include "im.h"
#include "im_image.h"
#include "im_util.h"

#include <lua.h>
#include <lauxlib.h>

#include "imlua.h"
#include "imlua_aux.h"
#include "imlua_image.h"


/*****************************************************************************\
\*****************************************************************************/
int imlua_getn (lua_State *L, int index)
{
  int n;
  lua_pushstring(L, "table");
#if LUA_VERSION_NUM > 501
  lua_pushglobaltable(L);
#else
  lua_gettable(L, LUA_GLOBALSINDEX);
#endif
  lua_pushstring(L, "getn");
  lua_gettable(L, -2);
  lua_pushvalue(L, index);
  lua_call(L, 1, 1);
  n = luaL_checkint(L, -1);
  lua_pop(L, 2);
  return n;
}

/*****************************************************************************\
 Creates an int array.
\*****************************************************************************/
int imlua_newarrayint (lua_State *L, int *value, int count, int start)
{
  int i;
  lua_createtable(L, count, 0);
  for (i = 0; i < count; i++)
  {
    lua_pushinteger(L, value[i]);
    lua_rawseti(L, -2, i+start);
  }
  return 1;
}

/*****************************************************************************\
 Creates an unsigned long array.
\*****************************************************************************/
int imlua_newarrayulong (lua_State *L, unsigned long *value, int count, int start)
{
  int i;
  lua_createtable(L, count, 0);
  for (i = 0; i < count; i++)
  {
    lua_pushnumber(L, value[i]);
    lua_rawseti(L, -2, i+start);
  }
  return 1;
}

/*****************************************************************************\
 Creates a float array.
\*****************************************************************************/
int imlua_newarrayfloat (lua_State *L, float *value, int count, int start)
{
  int i;
  lua_createtable(L, count, 0);
  for (i = 0; i < count; i++)
  {
    lua_pushnumber(L, value[i]);
    lua_rawseti(L, -2, i+start);
  }
  return 1;
}

/*****************************************************************************\
 Retrieve an int array.
\*****************************************************************************/
int *imlua_toarrayint (lua_State *L, int index, int *count, int start)
{
  int i, n;
  int *value = NULL;

  if (lua_istable(L, index))
  {
    n = imlua_getn(L, index);
    if (start == 0) n++;
    if (count) *count = n;

    value = (int*) malloc (sizeof(int) * n);
    for (i = 0; i < n; i++)
    {
      lua_rawgeti(L, index, i+start);
      value[i] = luaL_checkint(L, -1);
      lua_pop(L, 1);
    }
  }
  return value;
}

/*****************************************************************************\
 Retrieve an ulong array.
\*****************************************************************************/
unsigned long *imlua_toarrayulong (lua_State *L, int index, int *count, int start)
{
  int i, n;
  unsigned long *value = NULL;

  if (lua_istable(L, index))
  {
    n = imlua_getn(L, index);
    if (start == 0) n++;
    if (count) *count = n;

    value = (unsigned long*) malloc (sizeof(unsigned long) * n);
    for (i = 0; i < n; i++)
    {
      lua_rawgeti(L, index, i+start);
      value[i] = luaL_checkint(L, -1);
      lua_pop(L, 1);
    }
  }
  return value;
}

/*****************************************************************************\
 Retrieve a float array.
\*****************************************************************************/
float *imlua_toarrayfloat (lua_State *L, int index, int *count, int start)
{
  int i, n;
  float *value = NULL;

  if (lua_istable(L, index))
  {
    n = imlua_getn(L, index);
    if (start == 0) n++;
    if (count) *count = n;

    value = (float*) malloc (sizeof(float) * n);
    for (i = 0; i < n; i++)
    {
      lua_rawgeti(L, index, i+start);
      value[i] = (float) luaL_checknumber(L, -1);
      lua_pop(L, 1);
    }
  }
  return value;
}


/*****************************************************************************\
 Creates a bit mask based on a string formatted as "11000110".
\*****************************************************************************/
unsigned char imlua_checkmask (lua_State *L, int index)
{
  int i;
  unsigned char mask = 0;
  const char *str = luaL_checkstring(L, index);
  if (strlen(str) != 8)
    luaL_argerror(L, index, "invalid mask, must have 8 elements");

  for (i = 0; i < 8; i++)
  {
    char c = str[i];
    if (c != '0' && c != '1')
      luaL_argerror(L, index, "invalid mask, must have 0s or 1s only");

    mask |= (c - '0') << (7 - i);
  }

  return mask;
}

/*****************************************************************************\
 Checks data_type and color_space of an image. If it doesn't match throw a lua error.
\*****************************************************************************/
void imlua_checktype (lua_State *L, int index, imImage *image, int color_space, int data_type)
{
  if (image->data_type != data_type)
  {
    char msg[100] = "image data type must be ";
    strcat(msg, imDataTypeName(data_type));
    luaL_argerror(L, index, msg);
  }

  if (image->color_space != color_space)
  {
    char msg[100] = "image color space must be ";
    strcat(msg, imColorModeSpaceName(color_space));
    luaL_argerror(L, index, msg);
  }
}

/*****************************************************************************\
 Checks color_space of an image. If it doesn't match throw a lua error.
\*****************************************************************************/
void imlua_checkcolorspace (lua_State *L, int index, imImage *image, int color_space)
{
  if (image->color_space != color_space)
  {
    char msg[100] = "image color space must be ";
    strcat(msg, imColorModeSpaceName(color_space));
    luaL_argerror(L, index, msg);
  }
}

/*****************************************************************************\
 Checks a data_type of an image. If it doesn't match throw a lua error.
\*****************************************************************************/
void imlua_checkdatatype (lua_State *L, int index, imImage *image, int data_type)
{
  if (image->data_type != data_type)
  {
    char msg[100] = "image data type must be ";
    strcat(msg, imDataTypeName(data_type));
    luaL_argerror(L, index, msg);
  }
}

/*****************************************************************************\
 Checks if the size of the two images are equal. If it doesn't match throw a lua error.
\*****************************************************************************/
void imlua_matchsize(lua_State *L, imImage *image1, imImage *image2)
{
  imlua_matchcheck(L, imImageMatchSize(image1, image2), "images must have the same size");
}

void imlua_matchcolor(lua_State *L, imImage *image1, imImage *image2)
{
  imlua_matchcheck(L, imImageMatchColor(image1, image2), "images must have the same data type and color space");
}

void imlua_matchdatatype(lua_State *L, imImage *image1, imImage *image2)
{
  imlua_matchcheck(L, imImageMatchDataType(image1, image2), "images must have the same size and data type");
}

void imlua_matchcolorspace(lua_State *L, imImage *image1, imImage *image2)
{
  imlua_matchcheck(L, imImageMatchColorSpace(image1, image2), "images must have the same size and color space");
}

void imlua_match(lua_State *L, imImage *image1, imImage *image2)
{
  imlua_matchcheck(L, imImageMatch(image1, image2), "images must have the same size, data type and color space");
}
