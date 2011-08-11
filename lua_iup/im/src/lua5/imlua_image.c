/** \file
 * \brief IM Lua 5 Binding
 *
 * See Copyright Notice in im_lib.h
 * $Id: imlua_image.c,v 1.13 2010/10/25 18:29:07 scuri Exp $
 */

#include <string.h>
#include <memory.h>
#include <stdlib.h>

#include "im.h"
#include "im_image.h"
#include "im_util.h"
#include "im_convert.h"

#include <lua.h>
#include <lauxlib.h>

#include "imlua.h"
#include "imlua_image.h"
#include "imlua_palette.h"
#include "imlua_aux.h"


static imImage** imlua_rawcheckimage(lua_State *L, int param)
{
  return (imImage**) luaL_checkudata(L, param, "imImage");
}

imImage* imlua_checkimage(lua_State *L, int param)
{
  imImage** image_p = imlua_rawcheckimage(L, param);

  if (!(*image_p))
    luaL_argerror(L, param, "destroyed imImage");

  return *image_p;
}

int imlua_pushimageerror(lua_State *L, imImage* image, int error)
{
  if (error)
  {
    lua_pushnil(L);
    imlua_pusherror(L, error);
    return 2;
  }
  else
  {
    imlua_pushimage(L, image);
    return 1;
  }
}

void imlua_pushimage(lua_State *L, imImage* image)
{
  if (!image)
    lua_pushnil(L);
  else
  {
    imImage **image_p = (imImage**) lua_newuserdata(L, sizeof(imImage*));
    *image_p = image;
    luaL_getmetatable(L, "imImage");
    lua_setmetatable(L, -2);
  }
}

/*****************************************************************************\
 image channel, for indexing
\*****************************************************************************/
static imluaImageChannel *imlua_newimagechannel (lua_State *L, imImage *image, int channel)
{
  imluaImageChannel* imagechannel = (imluaImageChannel*) lua_newuserdata(L, sizeof(imluaImageChannel));
  imagechannel->image = image;
  imagechannel->channel = channel;
  luaL_getmetatable(L, "imImageChannel");
  lua_setmetatable(L, -2);
  return imagechannel;
}

static imluaImageChannel* imlua_checkimagechannel (lua_State *L, int param)
{
  return (imluaImageChannel*) luaL_checkudata(L, param, "imImageChannel");
}

/*****************************************************************************\
 image row, for indexing
\*****************************************************************************/
static imluaImageRow *imlua_newimagerow (lua_State *L, imImage *image, int channel, int row)
{
  imluaImageRow* imagerow = (imluaImageRow*) lua_newuserdata(L, sizeof(imluaImageRow));
  imagerow->image = image;
  imagerow->channel = channel;
  imagerow->row = row;
  luaL_getmetatable(L, "imImageChannelRow");
  lua_setmetatable(L, -2);
  return imagerow;
}

static imluaImageRow* imlua_checkimagerow (lua_State *L, int param)
{
  return (imluaImageRow*) luaL_checkudata(L, param, "imImageChannelRow");
}

/*****************************************************************************\
 im.ImageCreate(width, height, color_space, data_type)
\*****************************************************************************/
static int imluaImageCreate (lua_State *L)
{
  int width = luaL_checkint(L, 1);
  int height = luaL_checkint(L, 2);
  int color_space = luaL_checkint(L, 3);
  int data_type = luaL_checkint(L, 4);

  imImage *image = imImageCreate(width, height, color_space, data_type);
  imlua_pushimage(L, image);
  return 1;
}

/*****************************************************************************\
 im.ImageCreateFromOpenGLData(width, height, glformat, gldata)
\*****************************************************************************/
static int imluaImageCreateFromOpenGLData (lua_State *L)
{
  int width = luaL_checkint(L, 1);
  int height = luaL_checkint(L, 2);
  int glformat = luaL_checkint(L, 3);
  void* gldata = lua_touserdata(L, 4);
  imImage *image = imImageCreateFromOpenGLData(width, height, glformat, gldata);
  imlua_pushimage(L, image);
  return 1;
}

/*****************************************************************************\
 image:AddAlpha()
\*****************************************************************************/
static int imluaImageAddAlpha (lua_State *L)
{
  imImageAddAlpha(imlua_checkimage(L, 1));
  return 0;
}

/*****************************************************************************\
 image:SetAlpha()
\*****************************************************************************/
static int imluaImageSetAlpha (lua_State *L)
{
  imImageSetAlpha(imlua_checkimage(L, 1), (float)luaL_checknumber(L, 2));
  return 0;
}

/*****************************************************************************\
 image:Reshape()
\*****************************************************************************/
static int imluaImageReshape (lua_State *L)
{
  imImage* im = imlua_checkimage(L, 1);
  int width = luaL_checkint(L, 2);
  int height = luaL_checkint(L, 3);

  imImageReshape(im, width, height);
  return 0;
}

/*****************************************************************************\
 image:Copy()
\*****************************************************************************/
static int imluaImageCopy (lua_State *L)
{
  imImage* src_image = imlua_checkimage(L, 1);
  imImage* dst_image = imlua_checkimage(L, 2);

  imlua_match(L, src_image, dst_image);
  imImageCopy(src_image, dst_image);
  return 0;
}

/*****************************************************************************\
 image:CopyData()
\*****************************************************************************/
static int imluaImageCopyData (lua_State *L)
{
  imImage* src_image = imlua_checkimage(L, 1);
  imImage* dst_image = imlua_checkimage(L, 2);

  imlua_match(L, src_image, dst_image);
  imImageCopyData(src_image, dst_image);
  return 0;
}

/*****************************************************************************\
 image:CopyPlane()
\*****************************************************************************/
static int imluaImageCopyPlane(lua_State *L)
{
  imImage* src_image = imlua_checkimage(L, 1);
  int src_plane = luaL_checkint(L, 2);
  imImage* dst_image = imlua_checkimage(L, 3);
  int dst_plane = luaL_checkint(L, 4);
  int src_depth, dst_depth;

  imlua_matchdatatype(L, src_image, dst_image);

  src_depth = src_image->has_alpha? src_image->depth+1: src_image->depth;
  if (src_plane < 0 || src_plane >= src_depth)
    luaL_argerror(L, 2, "invalid source channel, out of bounds");

  dst_depth = dst_image->has_alpha? dst_image->depth+1: dst_image->depth;
  if (dst_plane < 0 || dst_plane >= dst_depth)
    luaL_argerror(L, 4, "invalid destiny channel, out of bounds");

  imImageCopyPlane(src_image, src_plane, dst_image, dst_plane);
  return 0;
}

/*****************************************************************************\
 image:Duplicate()
\*****************************************************************************/
static int imluaImageDuplicate (lua_State *L)
{
  imImage* image = imlua_checkimage(L, 1);
  imImage *new_image = imImageDuplicate(image);
  imlua_pushimage(L, new_image);
  return 1;
}

/*****************************************************************************\
 image:Clone()
\*****************************************************************************/
static int imluaImageClone (lua_State *L)
{
  imImage* image = imlua_checkimage(L, 1);
  imImage *new_image = imImageClone(image);
  imlua_pushimage(L, new_image);
  return 1;
}

/*****************************************************************************\
 image:SetAttribute(attrib, data_type, count, data)
\*****************************************************************************/
static int imluaImageSetAttribute (lua_State *L)
{
  int i, count = 0;
  void *data = NULL;

  imImage* image = imlua_checkimage(L, 1);
  const char *attrib = luaL_checkstring(L, 2);
  int data_type = luaL_checkint(L, 3);

  if (!lua_isnil(L, 4))
  {
    if (lua_isstring(L, 4) && data_type != IM_BYTE)
      luaL_argerror(L, 4, "if value is string, then data type must be byte");
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
          imbyte *data_byte = (imbyte*) data;
          for (i = 0; i < count; i++)
          {
            lua_rawgeti(L, 4, i+1);
            data_byte[i] = (imbyte)luaL_checkint(L, -1);
            lua_pop(L, 1);
          }
        }
      }
      break;

    case IM_USHORT:
      {
        imushort *data_ushort = (imushort*) data;
        for (i = 0; i < count; i++)
        {
          lua_rawgeti(L, 4, i+1);
          data_ushort[i] = (imushort)luaL_checkint(L, -1);
          lua_pop(L, 1);
        }
      }
      break;

    case IM_INT:
      {
        int *data_int = (int*) data;
        for (i = 0; i < count; i++)
        {
          lua_rawgeti(L, 4, i+1);
          data_int[i] = luaL_checkint(L, -1);
          lua_pop(L, 1);
        }
      }
      break;

    case IM_FLOAT:
      {
        float *data_float = (float*) data;
        for (i = 0; i < count; i++)
        {
          lua_rawgeti(L, 4, i+1);
          data_float[i] = (float) luaL_checknumber(L, -1);
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

  imImageSetAttribute(image, attrib, data_type, count, data);
  return 0;
}

/*****************************************************************************\
 image:GetAttribute(attrib)
\*****************************************************************************/
static int imluaImageGetAttribute (lua_State *L)
{
  int data_type;
  int i, count;
  const void *data;
  int as_string = 0;

  imImage* image = imlua_checkimage(L, 1);
  const char *attrib = luaL_checkstring(L, 2);

  data = imImageGetAttribute(image, attrib, &data_type, &count);
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
 image:GetAttributeList()
\*****************************************************************************/
static int imluaImageGetAttributeList (lua_State *L)
{
  int i, attrib_count;
  char **attrib;

  imImage* image = imlua_checkimage(L, 1);

  imImageGetAttributeList(image, NULL, &attrib_count);

  attrib = (char**) malloc(attrib_count * sizeof(char*));

  imImageGetAttributeList(image, attrib, &attrib_count);

  lua_createtable(L, attrib_count, 0);
  for (i = 0; i < attrib_count; i++)
  {
    lua_pushstring(L, attrib[i]);
    lua_rawseti(L, -2, i+1);
  }

  return 1;
}

/*****************************************************************************\
 image:Clear()
\*****************************************************************************/
static int imluaImageClear (lua_State *L)
{
  imImageClear(imlua_checkimage(L, 1));
  return 0;
}

/*****************************************************************************\
 image:isBitmap()
\*****************************************************************************/
static int imluaImageIsBitmap (lua_State *L)
{
  lua_pushboolean(L, imImageIsBitmap(imlua_checkimage(L, 1)));
  return 1;
}


/*****************************************************************************\
 image:GetOpenGLData()
\*****************************************************************************/
static int imluaImageGetOpenGLData (lua_State *L)
{
  int format;
  imbyte* gldata;
  imImage *image = imlua_checkimage(L, 1);

  gldata = imImageGetOpenGLData(image, &format);
  if (!gldata)
  {
    lua_pushnil(L);
    return 1;
  }

  lua_pushlightuserdata(L, gldata);
  lua_pushinteger(L, format);
  return 2;
}

/*****************************************************************************\
 image:GetPalette()
\*****************************************************************************/
static int imluaImageGetPalette (lua_State *L)
{
  imImage *image = imlua_checkimage(L, 1);
  long* color = malloc(sizeof(long) * 256);
  memcpy(color, image->palette, sizeof(long) * 256);
  imlua_pushpalette(L, color, 256);
  return 1;
}

/*****************************************************************************\
 image:SetPalette
\*****************************************************************************/
static int imluaImageSetPalette (lua_State *L)
{
  imImage *image = imlua_checkimage(L, 1);
  imluaPalette *pal = imlua_checkpalette(L, 2);
  long* palette = (long*)malloc(sizeof(long)*256);
  memcpy(palette, pal->color, pal->count*sizeof(long));
  imImageSetPalette(image, palette, pal->count);
  return 0;
}

/*****************************************************************************\
 image:CopyAttributes(dst_image)
\*****************************************************************************/
static int imluaImageCopyAttributes (lua_State *L)
{
  imImage *src_image = imlua_checkimage(L, 1);
  imImage *dst_image = imlua_checkimage(L, 2);

  imImageCopyAttributes(src_image, dst_image);
  return 0;
}

/*****************************************************************************\
 image:MatchSize(image2)
\*****************************************************************************/
static int imluaImageMatchSize (lua_State *L)
{
  imImage *image1 = imlua_checkimage(L, 1);
  imImage *image2 = imlua_checkimage(L, 2);

  lua_pushboolean(L, imImageMatchSize(image1, image2));
  return 1;
}

/*****************************************************************************\
 image:MatchColor(image2)
\*****************************************************************************/
static int imluaImageMatchColor (lua_State *L)
{
  imImage *image1 = imlua_checkimage(L, 1);
  imImage *image2 = imlua_checkimage(L, 2);

  lua_pushboolean(L, imImageMatchColor(image1, image2));
  return 1;
}

/*****************************************************************************\
 image:MatchDataType(image2)
\*****************************************************************************/
static int imluaImageMatchDataType (lua_State *L)
{
  imImage *image1 = imlua_checkimage(L, 1);
  imImage *image2 = imlua_checkimage(L, 2);

  lua_pushboolean(L, imImageMatchDataType(image1, image2));
  return 1;
}

/*****************************************************************************\
 image:MatchColorSpace(image2)
\*****************************************************************************/
static int imluaImageMatchColorSpace (lua_State *L)
{
  imImage *image1 = imlua_checkimage(L, 1);
  imImage *image2 = imlua_checkimage(L, 2);

  lua_pushboolean(L, imImageMatchColorSpace(image1, image2));
  return 1;
}

/*****************************************************************************\
 image:Match(image2)
\*****************************************************************************/
static int imluaImageMatch (lua_State *L)
{
  imImage *image1 = imlua_checkimage(L, 1);
  imImage *image2 = imlua_checkimage(L, 2);

  lua_pushboolean(L, imImageMatch(image1, image2));
  return 1;
}

/*****************************************************************************\
 image:SetBinary()
\*****************************************************************************/
static int imluaImageSetBinary (lua_State *L)
{
  imImageSetBinary(imlua_checkimage(L, 1));
  return 0;
}

/*****************************************************************************\
 image:MakeBinary()
\*****************************************************************************/
static int imluaImageMakeBinary (lua_State *L)
{
  imImageMakeBinary(imlua_checkimage(L, 1));
  return 0;
}

/*****************************************************************************\
 image:MakeGray()
\*****************************************************************************/
static int imluaImageMakeGray (lua_State *L)
{
  imImageMakeGray(imlua_checkimage(L, 1));
  return 0;
}

/*****************************************************************************\
 image:Width()
\*****************************************************************************/
static int imluaImageWidth(lua_State *L)
{
  imImage *im = imlua_checkimage(L, 1);
  lua_pushnumber(L, im->width);
  return 1;
}

/*****************************************************************************\
 image:Height()
\*****************************************************************************/
static int imluaImageHeight(lua_State *L)
{
  imImage *im = imlua_checkimage(L, 1);
  lua_pushnumber(L, im->height);
  return 1;
}

/*****************************************************************************\
 image:Depth()
\*****************************************************************************/
static int imluaImageDepth(lua_State *L)
{
  imImage *im = imlua_checkimage(L, 1);
  lua_pushnumber(L, im->depth);
  return 1;
}

/*****************************************************************************\
 image:DataType()
\*****************************************************************************/
static int imluaImageDataType(lua_State *L)
{
  imImage *im = imlua_checkimage(L, 1);
  lua_pushnumber(L, im->data_type);
  return 1;
}

/*****************************************************************************\
 image:ColorSpace()
\*****************************************************************************/
static int imluaImageColorSpace(lua_State *L)
{
  imImage *im = imlua_checkimage(L, 1);
  lua_pushnumber(L, im->color_space);
  return 1;
}

/*****************************************************************************\
 image:HasAlpha()
\*****************************************************************************/
static int imluaImageHasAlpha(lua_State *L)
{
  imImage *im = imlua_checkimage(L, 1);
  lua_pushboolean(L, im->has_alpha);
  return 1;
}

/*****************************************************************************\
 im.FileImageLoad(filename, [index])
\*****************************************************************************/
static int imluaFileImageLoad (lua_State *L)
{
  const char *filename = luaL_checkstring(L, 1);
  int index = luaL_optint(L, 2, 0);
  int error;
  imImage *image = imFileImageLoad(filename, index, &error);
  return imlua_pushimageerror(L, image, error);
}

/*****************************************************************************\
 im.FileImageLoadRegion(filename, [index])
\*****************************************************************************/
static int imluaFileImageLoadRegion (lua_State *L)
{
  const char *filename = luaL_checkstring(L, 1);
  int index = luaL_checkint(L, 2);
  int bitmap = luaL_checkint(L, 3);
  int xmin = luaL_checkint(L, 4);
  int xmax = luaL_checkint(L, 5);
  int ymin = luaL_checkint(L, 6);
  int ymax = luaL_checkint(L, 7);
  int width = luaL_checkint(L, 8);
  int height = luaL_checkint(L, 9);
  int error;
  imImage *image = imFileImageLoadRegion(filename, index, bitmap, &error, xmin, xmax, ymin, ymax, width, height);
  return imlua_pushimageerror(L, image, error);
}

/*****************************************************************************\
 im.FileImageLoadBitmap(filename, [index])
\*****************************************************************************/
static int imluaFileImageLoadBitmap (lua_State *L)
{
  const char *filename = luaL_checkstring(L, 1);
  int index = luaL_optint(L, 2, 0);
  int error;
  imImage *image = imFileImageLoadBitmap(filename, index, &error);
  return imlua_pushimageerror(L, image, error);
}

/*****************************************************************************\
 im.FileImageSave(filename, format, image)
\*****************************************************************************/
static int imluaFileImageSave (lua_State *L)
{
  const char *file_name = luaL_checkstring(L, 1);
  const char *format = luaL_checkstring(L, 2);
  imImage *image = imlua_checkimage(L, 3);

  imlua_pusherror(L, imFileImageSave(file_name, format, image));
  return 1;
}

/*****************************************************************************\
 image:Save(filename, format)
\*****************************************************************************/
static int imluaImageSave (lua_State *L)
{
  imImage *image = imlua_checkimage(L, 1);
  const char *file_name = luaL_checkstring(L, 2);
  const char *format = luaL_checkstring(L, 3);

  imlua_pusherror(L, imFileImageSave(file_name, format, image));
  return 1;
}

/*****************************************************************************\
 image:Destroy()
\*****************************************************************************/
static int imluaImageDestroy (lua_State *L)
{
  imImage** image_p = imlua_rawcheckimage(L, 1);
  if (!(*image_p))
    luaL_argerror(L, 1, "destroyed imImage");

  imImageDestroy(*image_p);
  *image_p = NULL; /* mark as destroyed */
  return 0;
}

/*****************************************************************************\
 gc
\*****************************************************************************/
static int imluaImage_gc (lua_State *L)
{
  imImage** image_p = imlua_rawcheckimage(L, 1);
  if (*image_p)
  {
    imImageDestroy(*image_p);
    *image_p = NULL; /* mark as destroyed */
  }

  return 0;
}

/*****************************************************************************\
 image tostring
\*****************************************************************************/
static int imluaImage_tostring (lua_State *L)
{
  imImage** image_p = (imImage**)lua_touserdata(L, 1);
  if (*image_p)
  {
    imImage *image = *image_p;
    lua_pushfstring(L, "imImage(%p) [width=%d,height=%d,color_space=%s,data_type=%s,depth=%d,has_alpha=%s]", 
      image_p,
      image->width, 
      image->height,
      imColorModeSpaceName(image->color_space),
      imDataTypeName(image->data_type),
      image->depth,
      image->has_alpha? "yes": "no"
    );
  }
  else
  {
    lua_pushfstring(L, "imImage(%p)-destroyed", image_p);
  }

  return 1;
}

/*****************************************************************************\
 imagechannel tostring
\*****************************************************************************/
static int imluaImageChannel_tostring (lua_State *L)
{
  imluaImageChannel *imagechannel = imlua_checkimagechannel(L, 1);
  lua_pushfstring(L, "imImageChannel(%p) [channel=%d]", 
    imagechannel, 
    imagechannel->channel
  );
  return 1;
}

/*****************************************************************************\
 imagerow tostring
\*****************************************************************************/
static int imluaImageRow_tostring (lua_State *L)
{
  char buff[32];
  imluaImageRow *imagerow = imlua_checkimagerow(L, 1);

  sprintf(buff, "%p", lua_touserdata(L, 1));
  lua_pushfstring(L, "imImageRow(%s) [channel=%d,row=%d]", 
    buff, 
    imagerow->channel,
    imagerow->row
  );
  return 1;
}

/*****************************************************************************\
 image row indexing
\*****************************************************************************/
static int imluaImageRow_index (lua_State *L)
{
  int index;
  imluaImageRow *imagerow = imlua_checkimagerow(L, 1);
  imImage *image = imagerow->image;
  int channel = imagerow->channel;
  int row = imagerow->row;
  int column = luaL_checkint(L, 2);

  if (column < 0 || column >= imagerow->image->width)
    luaL_argerror(L, 2, "invalid column, out of bounds");

  index = channel * image->width * image->height + row * image->width + column;

  switch (image->data_type)
  {
  case IM_BYTE:
    {
      imbyte *bdata = (imbyte*) image->data[0];
      lua_pushnumber(L, (lua_Number) bdata[index]);
    }
    break;

  case IM_USHORT:
    {
      imushort *udata = (imushort*) image->data[0];
      lua_pushnumber(L, (lua_Number) udata[index]);
    }
    break;

  case IM_INT:
    {
      int *idata = (int*) image->data[0];
      lua_pushnumber(L, (lua_Number) idata[index]);
    }
    break;

  case IM_FLOAT:
    {
      float *fdata = (float*) image->data[0];
      lua_pushnumber(L, (lua_Number) fdata[index]);
    }
    break;
    
  case IM_CFLOAT:
    {
      float *cdata = (float*) image->data[0];
      imlua_newarrayfloat(L, cdata + (2*index), 2, 1);
    }
    break;
  }

  return 1;
}

/*****************************************************************************\
 image row new index
\*****************************************************************************/
static int imluaImageRow_newindex (lua_State *L)
{
  int index;
  imluaImageRow *imagerow = imlua_checkimagerow(L, 1);
  imImage *image = imagerow->image;
  int channel = imagerow->channel;
  int row = imagerow->row;
  int column = luaL_checkint(L, 2);

  if (column < 0 || column >= imagerow->image->width)
    luaL_argerror(L, 2, "invalid column, out of bounds");

  index = channel * image->width * image->height + row * image->width + column;

  switch (image->data_type)
  {
  case IM_BYTE:
    {
      lua_Number value = luaL_checknumber(L, 3);
      imbyte *bdata = (imbyte*) image->data[0];
      bdata[index] = (imbyte) value;
    }
    break;

  case IM_USHORT:
    {
      lua_Number value = luaL_checknumber(L, 3);
      imushort *udata = (imushort*) image->data[0];
      udata[index] = (imushort) value;
    }
    break;

  case IM_INT:
    {
      lua_Number value = luaL_checknumber(L, 3);
      int *idata = (int*) image->data[0];
      idata[index] = (int) value;
    }
    break;

  case IM_FLOAT:
    {
      lua_Number value = luaL_checknumber(L, 3);
      float *fdata = (float*) image->data[0];
      fdata[index] = (float) value;
    }
    break;
    
  case IM_CFLOAT:
    {
      int count;
      float *cdata = (float*) image->data[0];
      float *value = imlua_toarrayfloat(L, 3, &count, 1);
      if (count != 2)
      {
        free(value);
        luaL_argerror(L, 3, "invalid value");
      }

      cdata[2*index] = value[0];
      cdata[2*index+1] = value[1];
      free(value);
    }
    break;
  }

  return 0;
}

/*****************************************************************************\
 image channel indexing
\*****************************************************************************/
static int imluaImageChannel_index (lua_State *L)
{
  imluaImageChannel *imagechannel = imlua_checkimagechannel(L, 1);
  int row = luaL_checkint(L, 2);

  if (row < 0 || row >= imagechannel->image->height)
    luaL_argerror(L, 2, "invalid row, out of bounds");

  imlua_newimagerow(L, imagechannel->image, imagechannel->channel, row);
  return 1;
}

/*****************************************************************************\
 image indexing
\*****************************************************************************/
static int imluaImage_index (lua_State *L)
{
  imImage *image = imlua_checkimage(L, 1);

  if (lua_isnumber(L, 2))
  {
    /* handle numeric indexing */
    int channel = luaL_checkint(L, 2);

    /* create channel */
    int depth = image->has_alpha? image->depth+1: image->depth;
    if (channel < 0 || channel >= depth)
      luaL_argerror(L, 2, "invalid channel, out of bounds");

    imlua_newimagechannel(L, image, channel);
  }
  else if (lua_isstring(L, 2))
  {
    /* get raw method */
    lua_getmetatable(L, 1);
    lua_pushvalue(L, 2);
    lua_rawget(L, -2);
  }
  else
  {
    lua_pushnil(L);
  }

  return 1;
}

static const luaL_reg imimage_lib[] = {
  {"ImageCreate", imluaImageCreate},
  {"ImageCreateFromOpenGLData", imluaImageCreateFromOpenGLData},
  {"ImageDestroy", imluaImageDestroy},
  {"FileImageLoad", imluaFileImageLoad},
  {"FileImageLoadBitmap", imluaFileImageLoadBitmap},
  {"FileImageLoadRegion", imluaFileImageLoadRegion},
  {"FileImageSave", imluaFileImageSave},
  {NULL, NULL}
};

static const luaL_reg imimage_metalib[] = {
  {"Destroy", imluaImageDestroy},
  {"AddAlpha", imluaImageAddAlpha},
  {"SetAlpha", imluaImageSetAlpha},
  {"Reshape", imluaImageReshape},
  {"Copy", imluaImageCopy},
  {"CopyData", imluaImageCopyData},
  {"CopyPlane", imluaImageCopyPlane},
  {"Duplicate", imluaImageDuplicate},
  {"Clone", imluaImageClone},
  {"SetAttribute", imluaImageSetAttribute},
  {"GetAttribute", imluaImageGetAttribute},
  {"GetAttributeList", imluaImageGetAttributeList},
  {"Clear", imluaImageClear},
  {"IsBitmap", imluaImageIsBitmap},
  {"GetOpenGLData", imluaImageGetOpenGLData},
  {"SetPalette", imluaImageSetPalette},
  {"GetPalette", imluaImageGetPalette},
  {"CopyAttributes", imluaImageCopyAttributes},
  {"MatchSize", imluaImageMatchSize},
  {"MatchColor", imluaImageMatchColor},
  {"MatchDataType", imluaImageMatchDataType},
  {"MatchColorSpace", imluaImageMatchColorSpace},
  {"Match", imluaImageMatch},
  {"SetBinary", imluaImageSetBinary},
  {"MakeBinary", imluaImageMakeBinary},
  {"MakeGray", imluaImageMakeGray},
  {"Width", imluaImageWidth},
  {"Height", imluaImageHeight},
  {"Depth", imluaImageDepth},
  {"DataType", imluaImageDataType},
  {"ColorSpace", imluaImageColorSpace},
  {"HasAlpha", imluaImageHasAlpha},
  {"Save", imluaImageSave},

  {"__gc", imluaImage_gc},
  {"__tostring", imluaImage_tostring},
  {"__index", imluaImage_index},

  {NULL, NULL}
};

static void createmeta (lua_State *L) 
{
  luaL_newmetatable(L, "imImageChannel"); /* create new metatable for imImageChannel handles */
  lua_pushliteral(L, "__index");
  lua_pushcfunction(L, imluaImageChannel_index);
  lua_rawset(L, -3);
  lua_pushliteral(L, "__tostring");
  lua_pushcfunction(L, imluaImageChannel_tostring);
  lua_rawset(L, -3);
  lua_pop(L, 1);  /* removes the metatable from the top of the stack */

  luaL_newmetatable(L, "imImageChannelRow"); /* create new metatable for imImageChannelRow handles */
  lua_pushliteral(L, "__index");
  lua_pushcfunction(L, imluaImageRow_index);
  lua_rawset(L, -3);
  lua_pushliteral(L, "__newindex");
  lua_pushcfunction(L, imluaImageRow_newindex);
  lua_rawset(L, -3);
  lua_pushliteral(L, "__tostring");
  lua_pushcfunction(L, imluaImageRow_tostring);
  lua_rawset(L, -3);
  lua_pop(L, 1);   /* removes the metatable from the top of the stack */

  /* Object Oriented Access */
  luaL_newmetatable(L, "imImage");  /* create new metatable for imImage handles */
  lua_pushliteral(L, "__index");    /* dummy code because imluaImage_index will overwrite this behavior */
  lua_pushvalue(L, -2);  /* push metatable */
  lua_rawset(L, -3);  /* metatable.__index = metatable */
  luaL_register(L, NULL, imimage_metalib);  /* register methods */
  lua_pop(L, 1);  /* removes the metatable from the top of the stack */
}

void imlua_open_image (lua_State *L)
{
  /* "im" table is at the top of the stack */
  createmeta(L);
  luaL_register(L, NULL, imimage_lib);

#ifdef IMLUA_USELOH
#include "im_image.loh"
#else
#ifdef IMLUA_USELZH
#include "im_image.lzh"
#else
  luaL_dofile(L, "im_image.lua");
#endif
#endif

}
