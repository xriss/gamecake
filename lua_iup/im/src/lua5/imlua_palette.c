/** \file
 * \brief IM Lua 5 Binding
 *
 * See Copyright Notice in im_lib.h
 * $Id: imlua_palette.c,v 1.3 2010/10/25 18:29:07 scuri Exp $
 */

#include <string.h>
#include <memory.h>
#include <stdlib.h>

#include "im.h"
#include "im_image.h"
#include "im_util.h"
#include "im_palette.h"

#include <lua.h>
#include <lauxlib.h>

#include "imlua.h"
#include "imlua_aux.h"
#include "imlua_palette.h"


static imluaPalette* imlua_rawcheckpalette(lua_State *L, int param)
{
  void *p = lua_touserdata(L, param);
  if (p != NULL) {  /* value is a userdata? */
    if (lua_getmetatable(L, param)) {  /* does it have a metatable? */
      lua_getfield(L, LUA_REGISTRYINDEX, "imPalette");  /* get correct metatable */
      if (lua_rawequal(L, -1, -2)) {  /* does it have the correct mt? */
        lua_pop(L, 2);  /* remove both metatables */
        return (imluaPalette*)p;
      }
      lua_pop(L, 1);  /* remove previous metatable */

      /* check also for CD palette */
      lua_getfield(L, LUA_REGISTRYINDEX, "cdPalette");  /* get correct metatable */
      if (lua_rawequal(L, -1, -2)) {  /* does it have the correct mt? */
        lua_pop(L, 2);  /* remove both metatables */
        return (imluaPalette*)p;
      }
    }
  }
  luaL_typeerror(L, param, "imPalette");  /* else error */
  return NULL;  /* to avoid warnings */
}

imluaPalette* imlua_checkpalette (lua_State *L, int param)
{
  imluaPalette* pal = imlua_rawcheckpalette(L, param);
  if (!pal->color)
    luaL_argerror(L, param, "destroyed imPalette");

  return pal;
}

void imlua_pushpalette(lua_State *L, long* color, int count)
{
  imluaPalette *pal = (imluaPalette*) lua_newuserdata(L, sizeof(imluaPalette));
  pal->count = count;
  pal->color = color;
  luaL_getmetatable(L, "imPalette");
  lua_setmetatable(L, -2);
}

/***************************************************************************\
* Creates a palette as a "imPalette" userdata. A palette can be          *
* considered and treated as a color table.                                 *
* im.PaletteCreate(count: number) -> (palette: "imPalette")               *
\***************************************************************************/
static int imluaPaletteCreate(lua_State *L)
{
  long* color;

  int count = luaL_optint(L, 1, 256);
  if (count < 1 || count > 256)
    luaL_argerror(L, 1, "palette count should be a positive integer and less then 256");

  color = (long*)malloc(256*sizeof(long));
  memset(color, 0, 256*sizeof(long));

  imlua_pushpalette(L, color, count);
  return 1;
}


/*****************************************************************************\
 im.PaletteFindNearest
\*****************************************************************************/
static int imluaPaletteFindNearest (lua_State *L)
{
  imluaPalette *pal = imlua_checkpalette(L, 1);
  long color = (long)lua_touserdata(L, 1);

  lua_pushnumber(L, imPaletteFindNearest(pal->color, pal->count, color));
  return 1;
}

/*****************************************************************************\
 im.PaletteFindColor
\*****************************************************************************/
static int imluaPaletteFindColor (lua_State *L)
{
  imluaPalette *pal = imlua_checkpalette(L, 1);
  long color = (long)lua_touserdata(L, 2);
  unsigned char tol = (unsigned char)luaL_checkint(L, 3);

  lua_pushnumber(L, imPaletteFindColor(pal->color, pal->count, color, tol));
  return 1;
}

/*****************************************************************************\
 im.PaletteGray
\*****************************************************************************/
static int imluaPaletteGray (lua_State *L)
{
  imlua_pushpalette(L, imPaletteGray(), 256);
  return 1;
}

/*****************************************************************************\
 im.PaletteRed
\*****************************************************************************/
static int imluaPaletteRed (lua_State *L)
{
  imlua_pushpalette(L, imPaletteRed(), 256);
  return 1;
}

/*****************************************************************************\
 im.PaletteGreen
\*****************************************************************************/
static int imluaPaletteGreen (lua_State *L)
{
  imlua_pushpalette(L, imPaletteGreen(), 256);
  return 1;
}

/*****************************************************************************\
 im.PaletteBlue
\*****************************************************************************/
static int imluaPaletteBlue (lua_State *L)
{
  imlua_pushpalette(L, imPaletteBlue(), 256);
  return 1;
}

/*****************************************************************************\
 im.PaletteYellow
\*****************************************************************************/
static int imluaPaletteYellow (lua_State *L)
{
  imlua_pushpalette(L, imPaletteYellow(), 256);
  return 1;
}

/*****************************************************************************\
 im.PaletteMagenta
\*****************************************************************************/
static int imluaPaletteMagenta (lua_State *L)
{
  imlua_pushpalette(L, imPaletteMagenta(), 256);
  return 1;
}

/*****************************************************************************\
 im.PaletteCian
\*****************************************************************************/
static int imluaPaletteCian (lua_State *L)
{
  imlua_pushpalette(L, imPaletteCian(), 256);
  return 1;
}

/*****************************************************************************\
 im.PaletteRainbow
\*****************************************************************************/
static int imluaPaletteRainbow (lua_State *L)
{
  imlua_pushpalette(L, imPaletteRainbow(), 256);
  return 1;
}

/*****************************************************************************\
 im.PaletteHues
\*****************************************************************************/
static int imluaPaletteHues (lua_State *L)
{
  imlua_pushpalette(L, imPaletteHues(), 256);
  return 1;
}

/*****************************************************************************\
 im.PaletteBlueIce
\*****************************************************************************/
static int imluaPaletteBlueIce (lua_State *L)
{
  imlua_pushpalette(L, imPaletteBlueIce(), 256);
  return 1;
}

/*****************************************************************************\
 im.PaletteHotIron
\*****************************************************************************/
static int imluaPaletteHotIron (lua_State *L)
{
  imlua_pushpalette(L, imPaletteHotIron(), 256);
  return 1;
}

/*****************************************************************************\
 im.PaletteBlackBody
\*****************************************************************************/
static int imluaPaletteBlackBody (lua_State *L)
{
  imlua_pushpalette(L, imPaletteBlackBody(), 256);
  return 1;
}

/*****************************************************************************\
 im.PaletteHighContrast
\*****************************************************************************/
static int imluaPaletteHighContrast (lua_State *L)
{
  imlua_pushpalette(L, imPaletteHighContrast(), 256);
  return 1;
}

/*****************************************************************************\
 im.PaletteUniform
\*****************************************************************************/
static int imluaPaletteUniform (lua_State *L)
{
  imlua_pushpalette(L, imPaletteUniform(), 256);
  return 1;
}

/*****************************************************************************\
 im.PaletteUniformIndex
\*****************************************************************************/
static int imluaPaletteUniformIndex (lua_State *L)
{
  lua_pushnumber(L, imPaletteUniformIndex((long)lua_touserdata(L, 1)));
  return 1;
}

/*****************************************************************************\
 im.PaletteUniformIndexHalftoned
\*****************************************************************************/
static int imluaPaletteUniformIndexHalftoned (lua_State *L)
{
  long color = (long)lua_touserdata(L, 1);
  int x = luaL_checkint(L, 2);
  int y = luaL_checkint(L, 3);

  lua_pushnumber(L, imPaletteUniformIndexHalftoned(color, x, y));
  return 1;
}

/***************************************************************************\
* Frees a previously allocated palette                                      *
* im.PaletteDestroy(palette: "imPalette")                                      *
\***************************************************************************/
static int imluaPaletteDestroy (lua_State *L)
{
  imluaPalette *pal = imlua_rawcheckpalette(L, 1);
  if (!pal->color)
    luaL_argerror(L, 1, "destroyed imPalette");

  free(pal->color);
  pal->color = NULL;  /* mark as destroyed */
  pal->count = 0;

  return 0;
}

/*****************************************************************************\
 gc
\*****************************************************************************/
static int imluaPalette_gc(lua_State *L)
{
  imluaPalette *pal = (imluaPalette*)lua_touserdata(L, 1);
  if (pal && pal->color)
  {
    free(pal->color);
    pal->color = NULL;  /* mark as destroyed */
    pal->count = 0;
  }

  return 0;
}

/***************************************************************************\
* color = palette[i]                                                        *
\***************************************************************************/
static int imluaPalette_index(lua_State *L)
{
  imluaPalette *pal = imlua_checkpalette(L, 1);
  int index_i = luaL_checkint(L, 2);

  if (index_i < 0 || index_i >= pal->count)
    luaL_argerror(L, 2, "index is out of bounds");

  lua_pushlightuserdata(L, (void*) pal->color[index_i]);
  return 1;
}

/***************************************************************************\
* palette[i] = color                                                        *
\***************************************************************************/
static int imluaPalette_newindex(lua_State *L)
{
  long color_i;
  imluaPalette *pal = imlua_checkpalette(L, 1);
  int index_i = luaL_checkint(L, 2);

  if (index_i < 0 || index_i >= pal->count)
    luaL_argerror(L, 2, "index is out of bounds");

  if (!lua_islightuserdata(L, 3))
    luaL_argerror(L, 3, "color must be a light user data");

  color_i = (long)lua_touserdata(L, 3);

  pal->color[index_i] = color_i;
  return 0;
}

/*****************************************************************************\
 len
\*****************************************************************************/
static int imluaPalette_len(lua_State *L)
{
  imluaPalette *pal = (imluaPalette*)lua_touserdata(L, 1);
  lua_pushinteger(L, pal->count);
  return 1;
}

/*****************************************************************************\
 tostring
\*****************************************************************************/
static int imluaPalette_tostring (lua_State *L)
{
  imluaPalette *pal = (imluaPalette*)lua_touserdata(L, 1);
  lua_pushfstring(L, "imPalette(%p)%s", pal, (pal->color)? "": "-destroyed");
  return 1;
}

static const luaL_reg impalette_lib[] = {
  {"PaletteFindNearest", imluaPaletteFindNearest},
  {"PaletteFindColor", imluaPaletteFindColor},
  {"PaletteGray", imluaPaletteGray },
  {"PaletteRed", imluaPaletteRed },
  {"PaletteGreen", imluaPaletteGreen },
  {"PaletteBlue", imluaPaletteBlue },
  {"PaletteYellow", imluaPaletteYellow },
  {"PaletteMagenta", imluaPaletteMagenta },
  {"PaletteCian", imluaPaletteCian },
  {"PaletteRainbow", imluaPaletteRainbow },
  {"PaletteHues", imluaPaletteHues },
  {"PaletteBlueIce", imluaPaletteBlueIce },
  {"PaletteHotIron", imluaPaletteHotIron },
  {"PaletteBlackBody", imluaPaletteBlackBody },
  {"PaletteHighContrast", imluaPaletteHighContrast },
  {"PaletteUniform", imluaPaletteUniform },
  {"PaletteUniformIndex", imluaPaletteUniformIndex },
  {"PaletteUniformIndexHalftoned", imluaPaletteUniformIndexHalftoned },

  {"PaletteDestroy", imluaPaletteDestroy},
  {"PaletteCreate", imluaPaletteCreate},

  {NULL, NULL}
};

static const luaL_reg impalette_metalib[] = {
  {"__gc", imluaPalette_gc},
  {"__tostring", imluaPalette_tostring},
  {"__index", imluaPalette_index},
  {"__newindex", imluaPalette_newindex},
  {"__len", imluaPalette_len},

  {NULL, NULL}
};

static void createmeta (lua_State *L) 
{
  /* there is no object orientation for imPalette, only array access */
  luaL_newmetatable(L, "imPalette");  /* create new metatable for imPalette handles */
  luaL_register(L, NULL, impalette_metalib);     /* register methods */
  lua_pop(L, 1);   /* removes the metatable from the top of the stack */
}

void imlua_open_palette (lua_State *L)
{
  /* "im" table is at the top of the stack */
  createmeta(L);
  luaL_register(L, NULL, impalette_lib);
}
