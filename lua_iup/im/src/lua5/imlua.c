/** \file
 * \brief IM Lua 5 Binding
 *
 * See Copyright Notice in im_lib.h
 * $Id: imlua.c,v 1.4 2010/07/18 03:04:23 scuri Exp $
 */

#include <string.h>
#include <memory.h>
#include <stdlib.h>

#include "im.h"
#include "im_lib.h"
#include "im_image.h"
#include "im_convert.h"

#include <lua.h>
#include <lauxlib.h>

#include "imlua.h"
#include "imlua_aux.h"
#include "imlua_image.h"
#include "imlua_palette.h"

/*****************************************************************************\
 im.Version()
\*****************************************************************************/
static int imluaVersion (lua_State *L)
{
  lua_pushstring(L, imVersion());
  return 1;
}

/*****************************************************************************\
 im.VersionDate()
\*****************************************************************************/
static int imluaVersionDate (lua_State *L)
{
  lua_pushstring(L, imVersionDate());
  return 1;
}

/*****************************************************************************\
 im.VersionNumber()
\*****************************************************************************/
static int imluaVersionNumber (lua_State *L)
{
  lua_pushnumber(L, imVersionNumber());
  return 1;
}

/*****************************************************************************\
 im.FormatList()
\*****************************************************************************/
static int imluaFormatList (lua_State *L)
{
  int i, format_count;
  char *format_list[50];

  imFormatList(format_list, &format_count);

  lua_createtable(L, format_count, 0);
  for (i = 0; i < format_count; i++)
  {
    lua_pushstring(L, format_list[i]);
    lua_settable(L, -2);
  }

  return 1;
}

/*****************************************************************************\
 im.FormatInfo(format)
\*****************************************************************************/
static int imluaFormatInfo (lua_State *L)
{
  char desc[50];
  char ext[50];
  int can_sequence;
  int error;

  error = imFormatInfo(luaL_checkstring(L, 1), desc, ext, &can_sequence);

  imlua_pusherror(L, error);
  if (error)
    return 1;

  lua_pushstring(L, desc);
  lua_pushstring(L, ext);
  lua_pushboolean(L, can_sequence);

  return 4;
}

/*****************************************************************************\
 im.FormatCompressions(format)
\*****************************************************************************/
static int imluaFormatCompressions (lua_State *L)
{
  int i, comp_count;
  int error;
  char *comp[50];

  int color_mode = luaL_optint(L, 2, -1);
  int data_type = luaL_optint(L, 3, -1);

  error = imFormatCompressions(luaL_checkstring(L, 1), comp, &comp_count, color_mode, data_type);

  imlua_pusherror(L, error);
  if (error)
    return 1;

  lua_createtable(L, comp_count, 0);
  for (i = 0; i < comp_count; i++)
  {
    lua_pushstring(L, comp[i]);
    lua_settable(L, -2);
  }

  return 2;
}

/*****************************************************************************\
 im.FormatCanWriteImage(format, compression, color_mode, data_type)
\*****************************************************************************/
static int imluaFormatCanWriteImage (lua_State *L)
{
  const char *format = luaL_checkstring(L, 1);
  const char *compression = luaL_checkstring(L, 2);
  int color_mode = luaL_checkint(L, 3);
  int data_type = luaL_checkint(L, 4);

  lua_pushboolean(L, imFormatCanWriteImage(format, compression, color_mode, data_type));
  return 1;
}

/*****************************************************************************\
 Constants
\*****************************************************************************/
static const imlua_constant im_constants[] = {

  { "BYTE", IM_BYTE, NULL },
  { "USHORT", IM_USHORT, NULL },
  { "INT", IM_INT, NULL },
  { "FLOAT", IM_FLOAT, NULL },
  { "CFLOAT", IM_CFLOAT, NULL },

  { "RGB", IM_RGB, NULL },
  { "MAP", IM_MAP, NULL },
  { "GRAY", IM_GRAY, NULL },
  { "BINARY", IM_BINARY, NULL },
  { "CMYK", IM_CMYK, NULL },
  { "YCBCR", IM_YCBCR, NULL },
  { "LAB", IM_LAB, NULL },
  { "LUV", IM_LUV, NULL },
  { "XYZ", IM_XYZ, NULL },

  { "ALPHA", IM_ALPHA, NULL },
  { "PACKED", IM_PACKED, NULL },
  { "TOPDOWN", IM_TOPDOWN, NULL },

  { "ERR_NONE", IM_ERR_NONE, NULL },
  { "ERR_OPEN", IM_ERR_OPEN, NULL }, 
  { "ERR_ACCESS", IM_ERR_ACCESS, NULL }, 
  { "ERR_FORMAT", IM_ERR_FORMAT, NULL }, 
  { "ERR_DATA", IM_ERR_DATA, NULL }, 
  { "ERR_COMPRESS", IM_ERR_COMPRESS, NULL }, 
  { "ERR_MEM", IM_ERR_MEM, NULL }, 
  { "ERR_COUNTER", IM_ERR_COUNTER, NULL }, 

  { "CPX_REAL", IM_CPX_REAL, NULL },
  { "CPX_IMAG", IM_CPX_IMAG, NULL },
  { "CPX_MAG", IM_CPX_MAG, NULL },
  { "CPX_PHASE", IM_CPX_PHASE, NULL },

  { "GAMMA_LINEAR", IM_GAMMA_LINEAR, NULL },
  { "GAMMA_LOGLITE", IM_GAMMA_LOGLITE, NULL },
  { "GAMMA_LOGHEAVY", IM_GAMMA_LOGHEAVY, NULL },
  { "GAMMA_EXPLITE", IM_GAMMA_EXPLITE, NULL },
  { "GAMMA_EXPHEAVY", IM_GAMMA_EXPHEAVY, NULL },

  { "CAST_MINMAX", IM_CAST_MINMAX, NULL },
  { "CAST_FIXED", IM_CAST_FIXED, NULL },
  { "CAST_DIRECT", IM_CAST_DIRECT, NULL },

  { "_AUTHOR",  0, IM_AUTHOR },
  { "_COPYRIGHT",  0, IM_COPYRIGHT },
  { "_VERSION_DATE",  0, IM_VERSION_DATE },
  { "_DESCRIPTION",  0, IM_DESCRIPTION },
  { "_NAME",  0, IM_NAME },

  { NULL, -1, NULL },
};

void imlua_regconstants (lua_State *L, const imlua_constant *imconst)
{
  const imlua_constant *l = imconst;
  for (; l->name; l++) 
  {
    lua_pushstring(L, l->name);
    if (l->str_value)
      lua_pushstring(L, l->str_value);
    else
      lua_pushnumber(L, l->value);
    lua_settable(L, -3);
  }

  lua_pushstring(L, "_VERSION");
  lua_pushstring(L, imVersion());
  lua_settable(L, -3);

  lua_pushstring(L, "_VERSION_NUMBER");
  lua_pushnumber(L, imVersionNumber());
  lua_settable(L, -3);
}

static const luaL_reg im_lib[] = {
  {"Version", imluaVersion},
  {"VersionDate", imluaVersionDate},
  {"VersionNumber", imluaVersionNumber},

  {"FormatList", imluaFormatList},
  {"FormatInfo", imluaFormatInfo},
  {"FormatCompressions", imluaFormatCompressions},
  {"FormatCanWriteImage", imluaFormatCanWriteImage},

  {NULL, NULL}
};

int imlua_open (lua_State *L)
{
  luaL_register(L, "im", im_lib);   /* leave "im" table at the top of the stack */
  imlua_regconstants(L, im_constants);

  imlua_open_file(L);
  imlua_open_image(L);
  imlua_open_convert(L);
  imlua_open_util(L);
  imlua_open_palette(L);

  return 1;
}

int luaopen_imlua(lua_State *L)
{
  return imlua_open(L);
}
