/** \file
 * \brief IM Lua 5 Binding
 *
 * See Copyright Notice in im_lib.h
 * $Id: imlua_file.c,v 1.3 2010/07/18 03:04:23 scuri Exp $
 */

#include <string.h>
#include <memory.h>
#include <stdlib.h>

#include "im.h"
#include "im_raw.h"
#include "im_image.h"
#include "im_util.h"

#include <lua.h>
#include <lauxlib.h>

#include "imlua.h"
#include "imlua_aux.h"
#include "imlua_image.h"
#include "imlua_palette.h"



static imFile** imlua_rawcheckfile(lua_State *L, int param)
{
  return (imFile**)luaL_checkudata(L, param, "imFile");
}

static imFile* imlua_checkfile (lua_State *L, int param)
{
  imFile** ifile_p = imlua_rawcheckfile(L, param);

  if (!(*ifile_p))
    luaL_argerror(L, param, "closed imFile");

  return *ifile_p;
}

static int imlua_pushifileerror(lua_State *L, imFile* ifile, int error)
{
  if (error)
  {
    lua_pushnil(L);
    imlua_pusherror(L, error);
    return 2;
  }
  else
  {
    imFile** ifile_p = (imFile**) lua_newuserdata(L, sizeof(imFile*));
    *ifile_p = ifile;
    luaL_getmetatable(L, "imFile");
    lua_setmetatable(L, -2);
    return 1;
  }
}


/*****************************************************************************\
 im.FileOpen(filename)
\*****************************************************************************/
static int imluaFileOpen (lua_State *L)
{
  const char *filename = luaL_checkstring(L, 1);
  int error;
  imFile *ifile = imFileOpen(filename, &error);
  return imlua_pushifileerror(L, ifile, error);
}

/*****************************************************************************\
 im.FileOpenAs(filename)
\*****************************************************************************/
static int imluaFileOpenAs (lua_State *L)
{
  const char *filename = luaL_checkstring(L, 1);
  const char *format = luaL_checkstring(L, 2);
  int error;
  imFile *ifile = imFileOpenAs(filename, format, &error);
  return imlua_pushifileerror(L, ifile, error);
}

/*****************************************************************************\
 im.FileOpenRaw(filename)
\*****************************************************************************/
static int imluaFileOpenRaw (lua_State *L)
{
  const char *filename = luaL_checkstring(L, 1);
  int error;
  imFile *ifile = imFileOpenRaw(filename, &error);
  return imlua_pushifileerror(L, ifile, error);
}

/*****************************************************************************\
 im.FileNew(filename, format)
\*****************************************************************************/
static int imluaFileNew (lua_State *L)
{
  const char *filename = luaL_checkstring(L, 1);
  const char *format = luaL_checkstring(L, 2);
  int error;

  imFile *ifile = imFileNew(filename, format, &error);
  return imlua_pushifileerror(L, ifile, error);
}

/*****************************************************************************\
 im.FileNewRaw(filename)
\*****************************************************************************/
static int imluaFileNewRaw (lua_State *L)
{
  const char *filename = luaL_checkstring(L, 1);
  int error;
  imFile *ifile = imFileNewRaw(filename, &error);
  return imlua_pushifileerror(L, ifile, error);
}

/*****************************************************************************\
 file:Handle()
\*****************************************************************************/
static int imluaFileHandle (lua_State *L)
{
  lua_pushlightuserdata(L, imFileHandle(imlua_checkfile(L, 1), luaL_checkint(L, 2)));
  return 1;
}

/*****************************************************************************\
 file:LoadImage()
\*****************************************************************************/
static int imluaFileLoadImage (lua_State *L)
{
  imFile *ifile = imlua_checkfile(L, 1);
  int index = luaL_optint(L, 2, 0);
  int error;
  imImage *image = imFileLoadImage(ifile, index, &error);
  return imlua_pushimageerror(L, image, error);
}

/*****************************************************************************\
 file:LoadImageFrame()
\*****************************************************************************/
static int imluaFileLoadImageFrame (lua_State *L)
{
  imFile *ifile = imlua_checkfile(L, 1);
  int index = luaL_checkint(L, 2);
  imImage *image = imlua_checkimage(L, 3);
  int error;

  imFileLoadImageFrame(ifile, index, image, &error);
  imlua_pusherror(L, error);

  return 1;
}

/*****************************************************************************\
 file:LoadImageRegion()
\*****************************************************************************/
static int imluaFileLoadImageRegion (lua_State *L)
{
  imFile *ifile = imlua_checkfile(L, 1);
  int index = luaL_checkint(L, 2);
  int bitmap = luaL_checkint(L, 3);
  int xmin = luaL_checkint(L, 4);
  int xmax = luaL_checkint(L, 5);
  int ymin = luaL_checkint(L, 6);
  int ymax = luaL_checkint(L, 7);
  int width = luaL_checkint(L, 8);
  int height = luaL_checkint(L, 9);
  int error;
  imImage *image = imFileLoadImageRegion(ifile, index, bitmap, &error, xmin, xmax, ymin, ymax, width, height);
  return imlua_pushimageerror(L, image, error);
}

/*****************************************************************************\
 file:LoadBitmap()
\*****************************************************************************/
static int imluaFileLoadBitmap (lua_State *L)
{
  imFile *ifile = imlua_checkfile(L, 1);
  int index = luaL_optint(L, 2, 0);
  int error;
  imImage *image = imFileLoadBitmap(ifile, index, &error);
  return imlua_pushimageerror(L, image, error);
}

/*****************************************************************************\
 file:LoadBitmapFrame()
\*****************************************************************************/
static int imluaFileLoadBitmapFrame (lua_State *L)
{
  imFile *ifile = imlua_checkfile(L, 1);
  int index = luaL_checkint(L, 2);
  imImage *image = imlua_checkimage(L, 3);
  int error;

  imFileLoadBitmapFrame(ifile, index, image, &error);
  imlua_pusherror(L, error);

  return 1;
}

/*****************************************************************************\
 file:SaveImage()
\*****************************************************************************/
static int imluaFileSaveImage (lua_State *L)
{
  imFile *ifile = imlua_checkfile(L, 1);
  imImage *image = imlua_checkimage(L, 2);

  imlua_pusherror(L, imFileSaveImage(ifile, image));
  return 1;
}

/*****************************************************************************\
 file:GetInfo()
\*****************************************************************************/
static int imluaFileGetInfo (lua_State *L)
{
  int image_count;
  char format[10];
  char compression[20];

  imFile *ifile = imlua_checkfile(L, 1);

  imFileGetInfo(ifile, format, compression, &image_count);

  lua_pushstring(L, format);
  lua_pushstring(L, compression);
  lua_pushnumber(L, image_count);

  return 3;
}

/*****************************************************************************\
 file:SetInfo()
\*****************************************************************************/
static int imluaFileSetInfo (lua_State *L)
{
  imFile *ifile = imlua_checkfile(L, 1);
  const char *compression = luaL_checkstring(L, 2);

  imFileSetInfo(ifile, compression);

  return 0;
}

/*****************************************************************************\
 file:SetAttribute(attrib, data_type, data)
\*****************************************************************************/
static int imluaFileSetAttribute (lua_State *L)
{
  int i, count = 0;
  void *data = NULL;

  imFile *ifile = imlua_checkfile(L, 1);
  const char *attrib = luaL_checkstring(L, 2);
  int data_type = luaL_checkint(L, 3);

  if (!lua_isnil(L, 4))
  {
    if (lua_isstring(L, 4) && data_type != IM_BYTE)
      luaL_argerror(L, 4, "if value is a string, then data type must be byte");
    else
    {
      luaL_checktype(L, 4, LUA_TTABLE);
      count = imlua_getn(L, 4);
      data = malloc(imDataTypeSize(data_type) * count);
    }

    switch (data_type)
    {
    case IM_BYTE:
      {
        if (lua_isstring(L, 4))
        {
          const char* str = lua_tostring(L, 4);
          count = strlen(str)+1;
          data = malloc(imDataTypeSize(data_type) * count);
          memcpy(data, str, count);
        }
        else
        {
          imbyte *d = (imbyte*) data;
          for (i = 0; i < count; i++)
          {
            lua_rawgeti(L, 4, i+1);
            d[i] = (imbyte) luaL_checkint(L, -1);
            lua_pop(L, 1);
          }
        }
      }
      break;

    case IM_USHORT:
      {
        imushort *d = (imushort*) data;
        for (i = 0; i < count; i++)
        {
          lua_rawgeti(L, 4, i+1);
          d[i] = (imushort) luaL_checkint(L, -1);
          lua_pop(L, 1);
        }
      }
      break;

    case IM_INT:
      {
        int *d = (int*) data;
        for (i = 0; i < count; i++)
        {
          lua_rawgeti(L, 4, i+1);
          d[i] = luaL_checkint(L, -1);
          lua_pop(L, 1);
        }
      }
      break;

    case IM_FLOAT:
      {
        float *d = (float*) data;
        for (i = 0; i < count; i++)
        {
          lua_rawgeti(L, 4, i+1);
          d[i] = (float) luaL_checknumber(L, -1);
          lua_pop(L, 1);
        }
      }
      break;

    case IM_CFLOAT:
      {
        float *data_float = (float*) data;
        for (i = 0; i < count; i++)
        {
          int two;
          float *value = imlua_toarrayfloat(L, -1, &two, 1);
          if (two != 2)
          {
            free(value);
            luaL_argerror(L, 4, "invalid value");
          }

          data_float[i] = value[0];
          data_float[i+1] = value[1];
          free(value);
          lua_pop(L, 1);
        }        
      }
      break;
    }
  }

  imFileSetAttribute(ifile, attrib, data_type, count, data);
  return 0;
}

/*****************************************************************************\
 file:GetAttribute(attrib)
\*****************************************************************************/
static int imluaFileGetAttribute (lua_State *L)
{
  int data_type;
  int i, count;
  const void *data;
  int as_string = 0;

  imFile *ifile = imlua_checkfile(L, 1);
  const char *attrib = luaL_checkstring(L, 2);

  data = imFileGetAttribute(ifile, attrib, &data_type, &count);
  if (!data)
  {
    lua_pushnil(L);
    return 1;
  }

  if (data_type == IM_BYTE && lua_isboolean(L, 3))
    as_string = lua_toboolean(L, 3);

  if (!as_string)
    lua_newtable(L);

  switch (data_type)
  {
  case IM_BYTE:
    {
      if (as_string)
      {
        lua_pushstring(L, (const char*)data);
      }
      else
      {
        imbyte *data_byte = (imbyte*) data;
        for (i = 0; i < count; i++, data_byte++)
        {
          lua_pushnumber(L, *data_byte);
          lua_rawseti(L, -2, i+1);
        }
      }
    }
    break;

  case IM_USHORT:
    {
      imushort *data_ushort = (imushort*) data;
      for (i = 0; i < count; i++, data_ushort += 2)
      {
        lua_pushnumber(L, *data_ushort);
        lua_rawseti(L, -2, i+1);
      }
    }
    break;

  case IM_INT:
    {
      int *data_int = (int*) data;
      for (i = 0; i < count; i++, data_int++)
      {
        lua_pushnumber(L, *data_int);
        lua_rawseti(L, -2, i+1);
      }
    }
    break;

  case IM_FLOAT:
    {
      float *data_float = (float*) data;
      for (i = 0; i < count; i++, data_float++)
      {
        lua_pushnumber(L, *data_float);
        lua_rawseti(L, -2, i+1);
      }
    }
    break;

  case IM_CFLOAT:
    {
      float *data_float = (float*) data;
      for (i = 0; i < count; i++, data_float += 2)
      {
        imlua_newarrayfloat(L, data_float, 2, 1);
        lua_rawseti(L, -2, i+1);
      }        
    }
    break;
  }

  lua_pushnumber(L, data_type);

  return 2;
}

/*****************************************************************************\
 file:GetAttributeList()
\*****************************************************************************/
static int imluaFileGetAttributeList (lua_State *L)
{
  int i, attrib_count;
  char **attrib;

  imFile* ifile = imlua_checkfile(L, 1);

  imFileGetAttributeList(ifile, NULL, &attrib_count);

  attrib = (char**) malloc(attrib_count * sizeof(char*));

  imFileGetAttributeList(ifile, attrib, &attrib_count);

  lua_createtable(L, attrib_count, 0);
  for (i = 0; i < attrib_count; i++)
  {
    lua_pushstring(L, attrib[i]);
    lua_rawseti(L, -2, i+1);
  }

  return 1;
}

/*****************************************************************************\
 file:GetPalette()
\*****************************************************************************/
static int imluaFileGetPalette (lua_State *L)
{
  imFile *ifile = imlua_checkfile(L, 1);
  long* color = malloc(sizeof(long) * 256);
  int count;
  imFileGetPalette(ifile, color, &count);
  imlua_pushpalette(L, color, count);
  return 1;
}

/*****************************************************************************\
 file:SetPalette(pal)
\*****************************************************************************/
static int imluaFileSetPalette (lua_State *L)
{
  imFile *ifile = imlua_checkfile(L, 1);
  imluaPalette *pal = imlua_checkpalette(L, 2);
  imFileSetPalette(ifile, pal->color, pal->count);
  return 0;
}

/*****************************************************************************\
 file:ReadImageInfo()
\*****************************************************************************/
static int imluaFileReadImageInfo (lua_State *L)
{
  int width, height;
  int file_color_mode, file_data_type;
  int error;

  imFile *ifile = imlua_checkfile(L, 1);
  int index = luaL_optint(L, 2, 0);

  error = imFileReadImageInfo(ifile, index, &width, &height, &file_color_mode, &file_data_type);

  imlua_pusherror(L, error);
  if (error)
    return 1;

  lua_pushnumber(L, width);
  lua_pushnumber(L, height);
  lua_pushnumber(L, file_color_mode);
  lua_pushnumber(L, file_data_type);
  return 5;
}

/*****************************************************************************\
 file:WriteImageInfo(width, height, user_color_mode, user_data_type)
\*****************************************************************************/
static int imluaFileWriteImageInfo (lua_State *L)
{
  imFile *ifile = imlua_checkfile(L, 1);
  int width = luaL_checkint(L, 2);
  int height = luaL_checkint(L, 3);
  int user_color_mode = luaL_checkint(L, 4);
  int user_data_type = luaL_checkint(L, 5);

  imlua_pusherror(L, imFileWriteImageInfo(ifile, width, height, user_color_mode, user_data_type));
  return 1;
}

/*****************************************************************************\
 file:ReadImageData(data)
\*****************************************************************************/
static int imluaFileReadImageData (lua_State *L)
{
  imFile *ifile = imlua_checkfile(L, 1);
  void* data = lua_touserdata(L, 2);
  int convert2bitmap = lua_toboolean(L, 3);
  int color_mode_flags = luaL_checkint(L, 4);
  imlua_pusherror(L, imFileReadImageData(ifile, data, convert2bitmap, color_mode_flags));
  return 1;
}

/*****************************************************************************\
 file:WriteImageData(data)
\*****************************************************************************/
static int imluaFileWriteImageData (lua_State *L)
{
  imFile *ifile = imlua_checkfile(L, 1);
  void* data = lua_touserdata(L, 2);
  imlua_pusherror(L, imFileWriteImageData(ifile, data));
  return 1;
}

/*****************************************************************************\
 file:Close()
\*****************************************************************************/
static int imluaFileClose (lua_State *L)
{
  imFile** ifile_p = imlua_rawcheckfile(L, 1);
  if (!(*ifile_p))
    luaL_argerror(L, 1, "closed imFile");

  imFileClose(*ifile_p);
  *ifile_p = NULL;  /* mark as closed */
  return 0;
}

/*****************************************************************************\
 gc
\*****************************************************************************/
static int imluaFile_gc (lua_State *L)
{
  imFile **ifile_p = (imFile **)lua_touserdata(L, 1);
  if (ifile_p && *ifile_p)
  {
    imFileClose(*ifile_p);
    *ifile_p = NULL;  /* mark as closed */
  }
  return 0;
}

/*****************************************************************************\
 tostring
\*****************************************************************************/
static int imluaFile_tostring (lua_State *L)
{
  imFile **ifile_p = (imFile **)lua_touserdata(L, 1);
  lua_pushfstring(L, "imFile(%p)%s", ifile_p, (*ifile_p)? "": "-closed");
  return 1;
}

/*****************************************************************************\
\*****************************************************************************/
static const luaL_reg imfile_lib[] = {
  {"FileOpen", imluaFileOpen},
  {"FileOpenAs", imluaFileOpenAs},
  {"FileOpenRaw", imluaFileOpenRaw},
  {"FileNew", imluaFileNew},
  {"FileNewRaw", imluaFileNewRaw},
  {"FileClose", imluaFileClose},
  {NULL, NULL}
};

static const luaL_reg imfile_metalib[] = {
  {"Handle", imluaFileHandle},
  {"Close", imluaFileClose},
  {"LoadImage", imluaFileLoadImage},
  {"LoadImageFrame", imluaFileLoadImageFrame},
  {"LoadImageRegion", imluaFileLoadImageRegion},
  {"LoadBitmap", imluaFileLoadBitmap},
  {"LoadBitmapFrame", imluaFileLoadBitmapFrame},
  {"SaveImage", imluaFileSaveImage},
  {"GetInfo", imluaFileGetInfo},
  {"SetInfo", imluaFileSetInfo},
  {"SetAttribute", imluaFileSetAttribute},
  {"GetAttribute", imluaFileGetAttribute},
  {"GetAttributeList", imluaFileGetAttributeList},
  {"GetPalette", imluaFileGetPalette},
  {"SetPalette", imluaFileSetPalette},
  {"ReadImageInfo", imluaFileReadImageInfo},
  {"WriteImageInfo", imluaFileWriteImageInfo},
  {"ReadImageData", imluaFileReadImageData},
  {"WriteImageData", imluaFileWriteImageData},

  {"__gc", imluaFile_gc},
  {"__tostring", imluaFile_tostring},

  {NULL, NULL}
};

static void createmeta (lua_State *L) 
{
  /* Object Oriented Access */
  luaL_newmetatable(L, "imFile");  /* create new metatable for imFile handles */
  lua_pushliteral(L, "__index");
  lua_pushvalue(L, -2);  /* push metatable */
  lua_rawset(L, -3);  /* metatable.__index = metatable */
  luaL_register(L, NULL, imfile_metalib);  /* register methods */
  lua_pop(L, 1);  /* removes the metatable from the top of the stack */
}

void imlua_open_file (lua_State *L)
{
  /* "im" table is at the top of the stack */
  createmeta(L);
  luaL_register(L, NULL, imfile_lib);
}
