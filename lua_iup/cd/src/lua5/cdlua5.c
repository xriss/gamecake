/** \file
 * \brief Lua Binding
 *
 * See Copyright Notice in cd.h
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cd.h"
#include "cdgdiplus.h"
#include "cdnative.h"
#include "cdps.h"

#include <lua.h>
#include <lauxlib.h>

#include "cdlua.h"
#include "cdlua5_private.h"


/***************************************************************************\
* Initialization                                                            *
\***************************************************************************/

static const char* cdlua_key = "cdlua5";

static void cdlua_SetState(lua_State * L, cdluaLuaState* cdL)
{
  lua_pushlightuserdata(L, (void*)cdlua_key);
  lua_pushlightuserdata(L, (void*)cdL);
  lua_settable(L, LUA_REGISTRYINDEX);    /*  registry[address("cdlua5")]=cdL  */
  lua_pop(L, 1);
}

cdluaLuaState* cdlua_getstate(lua_State * L)
{
  cdluaLuaState* cdL;
  lua_pushlightuserdata(L, (void*)cdlua_key);
  lua_gettable(L, LUA_REGISTRYINDEX);
  cdL = (cdluaLuaState*)lua_touserdata(L, -1);
  lua_pop(L, 1);
  return cdL;
}

cdluaContext* cdlua_getcontext(lua_State * L, int param)
{
  cdluaLuaState* cdL = cdlua_getstate(L);

  int driver = luaL_checkint(L, param);
  if ((driver < 0) || (driver >= cdL->numdrivers))
    luaL_argerror(L, param, "unknown driver");

  return cdL->drivers[driver];
}

static lua_State* cdlua5_play_luaState = NULL;

lua_State* cdlua_getplaystate(void)
{
  return cdlua5_play_luaState;
}

void cdlua_setplaystate(lua_State* L)
{
  cdlua5_play_luaState = L;
}

static cdluaPalette* cdlua_rawcheckpalette(lua_State *L, int param)
{
  void *p = lua_touserdata(L, param);
  if (p != NULL) {  /* value is a userdata? */
    if (lua_getmetatable(L, param)) {  /* does it have a metatable? */
      lua_getfield(L, LUA_REGISTRYINDEX, "cdPalette");  /* get correct metatable */
      if (lua_rawequal(L, -1, -2)) {  /* does it have the correct mt? */
        lua_pop(L, 2);  /* remove both metatables */
        return (cdluaPalette*)p;
      }
      lua_pop(L, 1);  /* remove previous metatable */

      /* check also for IM palette */
      lua_getfield(L, LUA_REGISTRYINDEX, "imPalette");  /* get correct metatable */
      if (lua_rawequal(L, -1, -2)) {  /* does it have the correct mt? */
        lua_pop(L, 2);  /* remove both metatables */
        return (cdluaPalette*)p;
      }
    }
  }
  luaL_typeerror(L, param, "cdPalette");  /* else error */
  return NULL;  /* to avoid warnings */
}

cdluaPalette * cdlua_checkpalette(lua_State * L, int param)
{
  cdluaPalette * pal = cdlua_rawcheckpalette(L, param);
  if (!pal->color)
    luaL_argerror(L, param, "killed cdPalette");
  return pal;
}

void cdlua_pushpalette(lua_State* L, long* palette, int size)
{
  cdluaPalette* pal = (cdluaPalette*)lua_newuserdata(L, sizeof(cdluaPalette));
  luaL_getmetatable(L, "cdPalette");
  lua_setmetatable(L, -2);

  pal->count = size;
  pal->color = palette;
}

cdState * cdlua_checkstate(lua_State* L, int param)
{
  cdState** state_p = (cdState**)luaL_checkudata(L, param, "cdState");
  if (!(*state_p))
    luaL_argerror(L, param, "released cdState");
  return *state_p;
}

void cdlua_pushstate(lua_State* L, cdState* state)
{
  cdState** state_p = (cdState**)lua_newuserdata(L, sizeof(cdState*));
  luaL_getmetatable(L, "cdState");
  lua_setmetatable(L, -2);

  *state_p = state;
}

cdluaPattern* cdlua_checkpattern(lua_State* L, int param)
{
  cdluaPattern* pattern_p = (cdluaPattern*) luaL_checkudata(L, param, "cdPattern");
  if (!pattern_p->pattern)
    luaL_argerror(L, param, "killed cdPattern");
  return pattern_p;
}

void cdlua_pushpattern(lua_State* L, long int* pattern, int width, int height)
{
  cdluaPattern* pattern_p = (cdluaPattern*)lua_newuserdata(L, sizeof(cdluaPattern));
  luaL_getmetatable(L, "cdPattern");
  lua_setmetatable(L, -2);

  pattern_p->pattern = pattern;
  pattern_p->width = width;
  pattern_p->height = height;
  pattern_p->size = width * height;
}

cdluaStipple* cdlua_checkstipple(lua_State* L, int param)
{
  cdluaStipple* stipple_p = (cdluaStipple*) luaL_checkudata(L, param, "cdStipple");
  if (!stipple_p->stipple)
    luaL_argerror(L, param, "killed cdStipple");
  return stipple_p;
}

void cdlua_pushstipple(lua_State* L, unsigned char* stipple, int width, int height)
{
  cdluaStipple* stipple_p = (cdluaStipple*)lua_newuserdata(L, sizeof(cdluaStipple));
  luaL_getmetatable(L, "cdStipple");
  lua_setmetatable(L, -2);

  stipple_p->stipple = stipple;
  stipple_p->width = width;
  stipple_p->height = height;
  stipple_p->size = width * height;
}

cdluaImageRGB* cdlua_checkimagergb(lua_State* L, int param)
{
  cdluaImageRGB* imagergb_p = (cdluaImageRGB*) luaL_checkudata(L, param, "cdImageRGB");
  if (!imagergb_p->red)
    luaL_argerror(L, param, "killed cdImageRGB");
  return imagergb_p;
}

void cdlua_pushimagergb(lua_State* L, unsigned char* red, unsigned char* green, unsigned char* blue, int width, int height)
{
  cdluaImageRGB* imagergb_p = (cdluaImageRGB*)lua_newuserdata(L, sizeof(cdluaImageRGB));
  luaL_getmetatable(L, "cdImageRGB");
  lua_setmetatable(L, -2);

  imagergb_p->red = red;
  imagergb_p->green = green;
  imagergb_p->blue = blue;
  imagergb_p->width = width;
  imagergb_p->height = height;
  imagergb_p->size = width * height;
  imagergb_p->free = 1;
}

void cdlua_pushimagergb_ex(lua_State* L, unsigned char* red, unsigned char* green, unsigned char* blue, int width, int height)
{
  cdluaImageRGB* imagergb_p = (cdluaImageRGB*)lua_newuserdata(L, sizeof(cdluaImageRGB));
  luaL_getmetatable(L, "cdImageRGB");
  lua_setmetatable(L, -2);

  imagergb_p->red = red;
  imagergb_p->green = green;
  imagergb_p->blue = blue;
  imagergb_p->width = width;
  imagergb_p->height = height;
  imagergb_p->size = width * height;
  imagergb_p->free = 0;
}

cdluaImageRGBA* cdlua_checkimagergba(lua_State* L, int param)
{
  cdluaImageRGBA* imagergba_p = (cdluaImageRGBA*) luaL_checkudata(L, param, "cdImageRGBA");
  if (!imagergba_p->red)
    luaL_argerror(L, param, "killed cdImageRGBA");
  return imagergba_p;
}

void cdlua_pushimagergba(lua_State* L, unsigned char* red, unsigned char* green, unsigned char* blue, unsigned char* alpha, int width, int height)
{
  cdluaImageRGBA* imagergba_p = (cdluaImageRGBA*)lua_newuserdata(L, sizeof(cdluaImageRGBA));
  luaL_getmetatable(L, "cdImageRGBA");
  lua_setmetatable(L, -2);

  imagergba_p->red = red;
  imagergba_p->green = green;
  imagergba_p->blue = blue;
  imagergba_p->alpha = alpha;
  imagergba_p->width = width;
  imagergba_p->height = height;
  imagergba_p->size = width * height;
  imagergba_p->free = 1;
}

void cdlua_pushimagergba_ex(lua_State* L, unsigned char* red, unsigned char* green, unsigned char* blue, unsigned char* alpha, int width, int height)
{
  cdluaImageRGBA* imagergba_p = (cdluaImageRGBA*)lua_newuserdata(L, sizeof(cdluaImageRGBA));
  luaL_getmetatable(L, "cdImageRGBA");
  lua_setmetatable(L, -2);

  imagergba_p->red = red;
  imagergba_p->green = green;
  imagergba_p->blue = blue;
  imagergba_p->alpha = alpha;
  imagergba_p->width = width;
  imagergba_p->height = height;
  imagergba_p->size = width * height;
  imagergba_p->free = 0;
}

cdluaImageMap* cdlua_checkimagemap(lua_State* L, int param)
{
  cdluaImageMap* imagemap_p = (cdluaImageMap*) luaL_checkudata(L, param, "cdImageMap");
  if (!imagemap_p->index)
    luaL_argerror(L, param, "killed cdImageMap");
  return imagemap_p;
}

void cdlua_pushimagemap(lua_State* L, unsigned char* index, int width, int height)
{
  cdluaImageMap* imagemap_p = (cdluaImageMap*)lua_newuserdata(L, sizeof(cdluaImageMap));
  luaL_getmetatable(L, "cdImageMap");
  lua_setmetatable(L, -2);

  imagemap_p->index = index;
  imagemap_p->width = width;
  imagemap_p->height = height;
  imagemap_p->size = width * height;
}

cdluaImageChannel* cdlua_checkchannel(lua_State* L, int param)
{
  cdluaImageChannel* channel_p = (cdluaImageChannel*) luaL_checkudata(L, param, "cdImageChannel");
  if (!channel_p->channel)
    luaL_argerror(L, param, "killed cdImageChannel");
  return channel_p;
}

void cdlua_pushchannel(lua_State* L, unsigned char* channel, int size)
{
  cdluaImageChannel* channel_p = (cdluaImageChannel*)lua_newuserdata(L, sizeof(cdluaImageChannel));
  luaL_getmetatable(L, "cdImageChannel");
  lua_setmetatable(L, -2);

  channel_p->channel = channel;
  channel_p->size = size;
}

long cdlua_checkcolor(lua_State* L, int param)
{
  if (!lua_islightuserdata(L, param))
  {
    if (lua_isnumber(L, param) && (lua_tointeger(L, param) == CD_QUERY))
      return CD_QUERY;

    luaL_argerror(L, param, "invalid color, must be a light user data");
  }

  return (long int)lua_touserdata(L, param);
}

cdImage* cdlua_checkimage(lua_State* L, int param)
{
  cdImage** image_p = (cdImage**)luaL_checkudata(L, param, "cdImage");
  if (!(*image_p))
    luaL_argerror(L, param, "killed cdImage");
  return *image_p;
}

void cdlua_pushimage(lua_State* L, cdImage* image)
{
  cdImage** image_p = (cdImage**)lua_newuserdata(L, sizeof(cdImage*));
  luaL_getmetatable(L, "cdImage");
  lua_setmetatable(L, -2);

  *image_p = image;
}

cdBitmap * cdlua_checkbitmap(lua_State* L, int param)
{
  cdBitmap** bitmap_p = (cdBitmap**)luaL_checkudata(L, param, "cdBitmap");
  if (!(*bitmap_p))
    luaL_argerror(L, param, "killed cdBitmap");
  return *bitmap_p;
}

void cdlua_pushbitmap(lua_State* L, cdBitmap* bitmap)
{
  cdBitmap** bitmap_p = (cdBitmap**)lua_newuserdata(L, sizeof(cdBitmap*));
  luaL_getmetatable(L, "cdBitmap");
  lua_setmetatable(L, -2);

  *bitmap_p = bitmap;
}

/***************************************************************************\
* cd.ContextCaps(ctx: number) -> (caps: number)                             *
\***************************************************************************/
static int cdlua5_contextcaps(lua_State * L)
{
  cdluaContext* cdlua_ctx = cdlua_getcontext(L, 1);
  lua_pushnumber(L, cdContextCaps(cdlua_ctx->ctx()));
  return 1;
}

static int cdlua5_releasestate(lua_State * L)
{
  cdState* *state_p = (cdState* *) luaL_checkudata(L, 1, "cdState");
  if (*state_p)
  {
    cdReleaseState(*state_p);
    *state_p = NULL;     /* mark as released */
  }

  return 0;
}

static int cdlua5_createstipple(lua_State *L)
{
  int size;
  unsigned char* stipple;

  int width = luaL_checkint(L, 1);
  int height = luaL_checkint(L, 2);

  if (width < 1 || height < 1)
    luaL_argerror(L, 1, "stipple dimensions should be positive integers");

  size = width * height;
  stipple = (unsigned char *) malloc(size);
  memset(stipple, '\0', size);  

  cdlua_pushstipple(L, stipple, width, height);

  return 1;
}

static int cdlua5_killstipple(lua_State *L)
{
  cdluaStipple *stipple_p = (cdluaStipple*)luaL_checkudata(L, 1, "cdStipple");
  if (stipple_p->stipple)
  {
    free(stipple_p->stipple);
    stipple_p->stipple = NULL;  /* mark as killed */
  }

  return 0;
}

/***************************************************************************\
* number = stipple[i]                                                       *
\***************************************************************************/
static int cdlua5_indexstipple(lua_State *L)
{
  cdluaStipple* stipple_p = cdlua_checkstipple(L, 1);

  int index = luaL_checkint(L, 2);
  if (index < 0 || index >= stipple_p->size)
    luaL_argerror(L, 2, "index is out of bounds");

  lua_pushnumber(L, stipple_p->stipple[index]);
  return 1;
}

/***************************************************************************\
* stipple[i] = number        .                                              *
\***************************************************************************/
static int cdlua5_newindexstipple(lua_State *L)
{
  unsigned char value;

  cdluaStipple* stipple_p = cdlua_checkstipple(L, 1);

  int index = luaL_checkint(L, 2);
  if (index < 0 || index >= stipple_p->size)
    luaL_argerror(L, 2, "index is out of bounds");

  value = (unsigned char)luaL_checkint(L, 3);
  if ((value != 0 && value != 1)) 
    luaL_argerror(L, 3, "value must be 0 or 1");

  stipple_p->stipple[index] = value;
  return 0;
}

static int cdlua5_createpattern(lua_State *L)
{
  int size;
  long int *pattern;

  int width = luaL_checkint(L, 1);
  int height = luaL_checkint(L, 2);

  if (width < 1 || height < 1)
    luaL_argerror(L, 1, "pattern dimensions should be positive integers");

  size = width * height;
  pattern = (long int *) malloc(size * sizeof(long int));
  memset(pattern, 255, size * sizeof(long int));

  cdlua_pushpattern(L, pattern, width, height);

  return 1;
}

static int cdlua5_killpattern(lua_State *L)
{
  cdluaPattern *pattern_p = (cdluaPattern *) luaL_checkudata(L, 1, "cdPattern");
  if (pattern_p->pattern)
  {
    free(pattern_p->pattern);
    pattern_p->pattern = NULL;    /* mark as killed */
  }

  return 0;
}

/***************************************************************************\
* color = pattern[i]                                                        *
\***************************************************************************/
static int cdlua5_indexpattern(lua_State *L)
{
  cdluaPattern* pattern_p = cdlua_checkpattern(L, 1);

  int index = luaL_checkint(L, 2);
  if (index < 0 || index >= pattern_p->size)
    luaL_argerror(L, 2, "index is out of bounds");

  lua_pushlightuserdata(L, (void *) pattern_p->pattern[index]);
  return 1;
}

/***************************************************************************\
* pattern[i] = color                                                        *
\***************************************************************************/
static int cdlua5_newindexpattern(lua_State *L)
{
  long int color;

  cdluaPattern* pattern_p = cdlua_checkpattern(L, 1);

  int index = luaL_checkint(L, 2);
  if (index < 0 || index >= pattern_p->size)
    luaL_argerror(L, 2, "index is out of bounds");

  color = cdlua_checkcolor(L, 3);
  
  pattern_p->pattern[index] = color;
  return 0;
}

static int cdlua5_rgb2map(lua_State *L)
{
  cdluaImageRGB* imagergb_p = cdlua_checkimagergb(L, 1);
  cdluaImageMap* imagemap_p = cdlua_checkimagemap(L, 2);
  cdluaPalette* pal = cdlua_checkpalette(L, 3);
  cdRGB2Map(imagergb_p->width, imagergb_p->height, 
            imagergb_p->red, imagergb_p->green, imagergb_p->blue, 
            imagemap_p->index, pal->count, pal->color);
  return 0;
}

static int cdlua5_createbitmap(lua_State *L)
{
  cdBitmap *bitmap;

  int width = luaL_checkint(L, 1);
  int height = luaL_checkint(L, 2);
  int type = luaL_checkint(L, 3);

  if (width < 1 || height < 1)
    luaL_argerror(L, 1, "bitmap dimensions should be positive integers");

  bitmap = cdCreateBitmap(width, height, type);
  if (bitmap) 
    cdlua_pushbitmap(L, bitmap);
  else
    lua_pushnil(L);

  return 1;
}

static int cdlua5_killbitmap(lua_State *L)
{
  cdBitmap* *bitmap_p = (cdBitmap* *) luaL_checkudata(L, 1, "cdBitmap");
  if (*bitmap_p)
  {
    cdKillBitmap(*bitmap_p);
    *bitmap_p = NULL;         /* mark as killed */
  }

  return 0;
}

static int cdlua5_bitmapgetdata(lua_State *L)
{
  cdBitmap* bitmap = cdlua_checkbitmap(L, 1);
  int dataptr = luaL_checkint(L, 2);

  unsigned char *data = cdBitmapGetData(bitmap, dataptr);
  if (data)
    cdlua_pushchannel(L, data, bitmap->w * bitmap->h);
  else
    lua_pushnil(L);

  return 1;
}

static int cdlua5_BitmapWidth(lua_State *L)
{
  cdBitmap* bitmap = cdlua_checkbitmap(L, 1);
  lua_pushinteger(L, bitmap->w);
  return 1;
}

static int cdlua5_BitmapHeight(lua_State *L)
{
  cdBitmap* bitmap = cdlua_checkbitmap(L, 1);
  lua_pushinteger(L, bitmap->h);
  return 1;
}

static int cdlua5_BitmapType(lua_State *L)
{
  cdBitmap* bitmap = cdlua_checkbitmap(L, 1);
  lua_pushinteger(L, bitmap->type);
  return 1;
}

static int cdlua5_bitmapsetrect(lua_State *L)
{
  cdBitmap* bitmap = cdlua_checkbitmap(L, 1);
  int xmin = (int) luaL_checkint(L, 2);
  int xmax = (int) luaL_checkint(L, 3);
  int ymin = (int) luaL_checkint(L, 4);
  int ymax = (int) luaL_checkint(L, 5);

  cdBitmapSetRect(bitmap, xmin, xmax, ymin, ymax);
  return 0;
}

static int cdlua5_bitmaprgb2map(lua_State *L)
{
  cdBitmap* bitmaprgb = cdlua_checkbitmap(L, 1);
  cdBitmap* bitmapmap = cdlua_checkbitmap(L, 2);

  if (bitmaprgb->type != CD_RGB)
    luaL_argerror(L, 1, "invalid bitmap type, must be RGB");
  if (bitmapmap->type != CD_MAP)
    luaL_argerror(L, 2, "invalid bitmap type, must be Map");

  cdBitmapRGB2Map(bitmaprgb, bitmapmap);
  return 0;
}

static int cdlua5_createimagergb(lua_State * L)
{  
  unsigned char *red, *green, *blue;
  int size;
  int width = luaL_checkint(L,1);
  int height = luaL_checkint(L,2);

  if (width < 1 || height < 1)
    luaL_argerror(L, 1, "image dimensions should be positive integers");
 
  size = width*height;
  red = (unsigned char*)malloc(3*size);
  
  if (red)
  {
    memset(red, 255, 3*size);  /* white */
    green = red + size;
    blue = red + 2*size;
    cdlua_pushimagergb(L, red, green, blue, width, height);
  }
  else
    lua_pushnil(L);

  return 1;
}

static int cdlua5_killimagergb(lua_State *L)
{
  cdluaImageRGB* imagergb_p = (cdluaImageRGB*)luaL_checkudata(L, 1, "cdImageRGB");
  if (imagergb_p->red && imagergb_p->free)
  {
    free(imagergb_p->red);
    imagergb_p->red = NULL;     /* mark as killed */
    imagergb_p->green = NULL;
    imagergb_p->blue = NULL;
  }

  return 0;
}

static int cdlua5_createimagergba(lua_State * L)
{  
  unsigned char *red, *green, *blue, *alpha;
  int size;
  int width = luaL_checkint(L,1);
  int height = luaL_checkint(L,2);

  if (width < 1 || height < 1)
    luaL_argerror(L, 1, "image dimensions should be positive integers");
 
  size = width*height;
  red = (unsigned char*)malloc(4*size);
  
  if (red)
  {
    memset(red, 255, 3*size); /* white */
    green = red + size;
    blue = red + 2*size;
    alpha = red + 3*size;
    memset(alpha, 0, size);  /* transparent */
    cdlua_pushimagergba(L, red, green, blue, alpha, width, height);
  }
  else
    lua_pushnil(L);

  return 1;
}

static int cdlua5_killimagergba(lua_State *L)
{
  cdluaImageRGBA* imagergba_p = (cdluaImageRGBA*)luaL_checkudata(L, 1, "cdImageRGBA");
  if (imagergba_p->red && imagergba_p->free)
  {
    free(imagergba_p->red);
    imagergba_p->red = NULL;   /* mark as killed */
    imagergba_p->green = NULL;
    imagergba_p->blue = NULL;
    imagergba_p->alpha = NULL;
  }

  return 0;
}

static int cdlua5_createimagemap(lua_State *L)
{
  int size;
  unsigned char *index;
  int width = luaL_checkint(L,1);
  int height = luaL_checkint(L,2);

  if (width < 1 || height < 1)
    luaL_argerror(L, 1, "imagemap dimensions should be positive integers");

  size = width * height;
  index = (unsigned char *) malloc(size);

  if (index)
  {
    memset(index, 0, size);
    cdlua_pushimagemap(L, index, width, height);
  }
  else
    lua_pushnil(L);

  return 1;
}

static int cdlua5_killimagemap(lua_State *L)
{
  cdluaImageMap *imagemap_p = (cdluaImageMap *) luaL_checkudata(L, 1, "cdImageMap");
  if (imagemap_p->index)
  {
    free(imagemap_p->index);
    imagemap_p->index = NULL;   /* mark as killed */
  }

  return 0;
}

/***************************************************************************\
* number = imagemap[i]                                                      *
\***************************************************************************/
static int cdlua5_indeximagemap(lua_State *L)
{
  cdluaImageMap* imagemap_p = cdlua_checkimagemap(L, 1);

  int index = luaL_checkint(L, 2);
  if (index < 0 || index >= imagemap_p->size)
    luaL_argerror(L, 2, "index is out of bounds");

  lua_pushnumber(L, imagemap_p->index[index]);
  return 1;
}

/***************************************************************************\
* imagemap[i] = number                                                      *
\***************************************************************************/
static int cdlua5_newindeximagemap(lua_State *L)
{
  int value;

  cdluaImageMap* imagemap_p = cdlua_checkimagemap(L, 1);

  int index = luaL_checkint(L, 2);
  if (index < 0 || index >= imagemap_p->size)
    luaL_argerror(L, 2, "index is out of bounds");

  value = luaL_checkint(L, 3);
  if ((value < 0 || value > 255)) 
    luaL_argerror(L, 3, "value should be in range [0, 255]");

  imagemap_p->index[index] = (unsigned char) value;
  return 0;
}

/***************************************************************************\
* channel "gettable" fallback. This fallback is called when a LUA line like *
* "c = imagergb.r[y*w + x]" is executed. The imagergb "gettable" fallback   *
* fills and returns a channel structure with info about the buffer. This    *
* structure is consulted and the appropriate value is returned.             *
\***************************************************************************/
static int cdlua5_indexchannel(lua_State *L)
{
  cdluaImageChannel* channel_p = cdlua_checkchannel(L, 1);

  int index = luaL_checkint(L, 2);
  if (index < 0 || 
      (channel_p->size > 0 && index >= channel_p->size) || 
      (channel_p->size == -1 && index >= 256)) {
    luaL_argerror(L, 2, "index is out of bounds");
  }
  
  if (channel_p->size == -1) /* COLORS */
    lua_pushlightuserdata(L, (void *)((long int*)channel_p->channel)[index]);
  else
    lua_pushnumber(L, channel_p->channel[index]);

  return 1;
}

/***************************************************************************\
* channel "settable" fallback. This fallback is called when a LUA line like *
* "imagergb.r[y*w + x] = c" is executed. The imagergb "gettable" fallback   *
* fills and returns a channel structure with info about the buffer. This    *
* structure is consulted and the value is assigned where it should.         *
\***************************************************************************/
static int cdlua5_newindexchannel(lua_State *L)
{
  int value;

  cdluaImageChannel* channel_p = cdlua_checkchannel(L, 1);

  int index = luaL_checkint(L, 2);
  if (index < 0 || 
      (channel_p->size > 0 && index >= channel_p->size) || 
      (channel_p->size == -1 && index >= 256)) {
    luaL_argerror(L, 2, "index is out of bounds");
  }
  
  if (channel_p->size > 0)
  {
    value = luaL_checkint(L, 3);
    if ((value < 0 || value > 255))
      luaL_argerror(L, 3, "value should be in range [0, 255]");
    channel_p->channel[index] = (unsigned char) value;
  }
  else /* COLORS */
  {
    value = (long int) cdlua_checkcolor(L, 3);
    ((long int*)channel_p->channel)[index] = value;
  }
  return 0;
}

static int cdl_ischar(const char* str, char c)
{
  if ((str[0] == c || str[0] == c-32) && str[1] == 0)
    return 1;
  return 0;
}

/***************************************************************************\
* imagergb "gettable" fallback. This fallback is called when a LUA line     *
* like "c = imagergb.r[y*w + x]" or "imagergb.r[y*w + x] = c" is executed.  *
* The following "gettable" or "settable"                                    *
* then assigns or returns the appropriate value.                            *
\***************************************************************************/
static int cdlua5_indeximagergb(lua_State *L)
{
  unsigned char* channel = NULL;
  cdluaImageRGB* imagergb_p = cdlua_checkimagergb(L, 1);
  const char *index_s = luaL_checkstring(L, 2);

  if (cdl_ischar(index_s, 'r'))
    channel = imagergb_p->red;
  else if (cdl_ischar(index_s, 'g'))
    channel = imagergb_p->green;
  else if (cdl_ischar(index_s, 'b'))
    channel = imagergb_p->blue;

  if (channel)
    cdlua_pushchannel(L, channel, imagergb_p->size);
  else
  {
    /* get raw method */
    lua_getmetatable(L, 1);
    lua_pushvalue(L, 2);
    lua_rawget(L, -2);
  }

  return 1;
}

/***************************************************************************\
* imagergba "gettable" fallback. This fallback is called when a LUA line    *
* like "c = imagergba.r[y*w + x]" or "imagergba.r[y*w + x] = c" is executed.*
* The following "gettable" or "settable"                                    *
* then assigns or returns the appropriate value.                            *
\***************************************************************************/
static int cdlua5_indeximagergba(lua_State *L)
{
  unsigned char* channel = NULL;
  cdluaImageRGBA* imagergba_p = cdlua_checkimagergba(L, 1);
  const char *index_s = luaL_checkstring(L, 2);

  if (cdl_ischar(index_s, 'r'))
    channel = imagergba_p->red;
  else if (cdl_ischar(index_s, 'g'))
    channel = imagergba_p->green;
  else if (cdl_ischar(index_s, 'b'))
    channel = imagergba_p->blue;
  else if (cdl_ischar(index_s, 'a'))
    channel = imagergba_p->alpha;

  if (channel)
    cdlua_pushchannel(L, channel, imagergba_p->size);
  else
  {
    /* get raw method */
    lua_getmetatable(L, 1);
    lua_pushvalue(L, 2);
    lua_rawget(L, -2);
  }

  return 1;
}

/***************************************************************************\
* bitmap "gettable" fallback. This fallback is called when a LUA line       *
* like "c = bitmap.r[y*w + x]" or "bitmap.r[y*w + x] = c" is executed.      *
* The following "gettable" or "settable"                                    *
* then assigns or returns the appropriate value.                            *
\***************************************************************************/
static int cdlua5_indexbitmap(lua_State *L)
{
  unsigned char* channel = NULL;
  cdBitmap* bitmap = cdlua_checkbitmap(L, 1);
  const char *index_s = luaL_checkstring(L, 2);

  int size = bitmap->w * bitmap->h;

  if (cdl_ischar(index_s, 'r'))
    channel = cdBitmapGetData(bitmap, CD_IRED);
  else if (cdl_ischar(index_s, 'g'))
    channel = cdBitmapGetData(bitmap, CD_IGREEN);
  else if (cdl_ischar(index_s, 'b'))
    channel = cdBitmapGetData(bitmap, CD_IBLUE);
  else if (cdl_ischar(index_s, 'a'))
    channel = cdBitmapGetData(bitmap, CD_IALPHA);
  else if (cdl_ischar(index_s, 'i'))
    channel = cdBitmapGetData(bitmap, CD_INDEX);
  else if (cdl_ischar(index_s, 'c')) 
  {
    channel = cdBitmapGetData(bitmap, CD_COLORS);
    size = -1;
  }

  if (channel)
    cdlua_pushchannel(L, channel, size);
  else
  {
    /* get raw method */
    lua_getmetatable(L, 1);
    lua_pushvalue(L, 2);
    lua_rawget(L, -2);
  }

  return 1;
}

static int cdlua5_killimage(lua_State *L)
{
  cdImage* *image_p = (cdImage* *) luaL_checkudata(L, 1, "cdImage");
  if (*image_p)
  {
    cdKillImage(*image_p);
    *image_p = NULL;    /* mark as killed */
  }
  return 0;
}

/***************************************************************************\
* cd.Version() -> (version: string)                                         *
\***************************************************************************/
static int cdlua5_version(lua_State *L)
{
  lua_pushstring(L, cdVersion());
  return 1;
}

/***************************************************************************\
* Register callback functions.                                              *
* cd.ContextRegisterCallback(ctx, cb: number, func: function) -> (status: number)  *
\***************************************************************************/
static int cdlua5_registercallback(lua_State *L)
{
  int cb_i, func_lock, ret = CD_ERROR;
  cdluaCallback* cdCB;
  cdluaContext* cdlua_ctx;

  cdlua_ctx = cdlua_getcontext(L, 1);

  cb_i = luaL_checkint(L, 2);
  if (cb_i >= cdlua_ctx->cb_n)
    luaL_argerror(L, 2, "invalid callback parameter");
 
  if (lua_isnil(L, 3))
    func_lock = -1;
  else if (!lua_isfunction(L, 3))
    luaL_argerror(L, 3, "invalid function parameter");
  else
    lua_pushvalue(L, 3);
  func_lock = lua_ref(L, 1);

  cdCB = &cdlua_ctx->cb_list[cb_i];

  if (cdCB->lock != -1)
  {
    lua_unref(L,cdCB->lock);
    cdCB->lock = func_lock;
    if (func_lock == -1)
    {
      ret = cdContextRegisterCallback(cdlua_ctx->ctx(), cb_i, NULL);
    }
  }
  else
  {
    if (func_lock != -1)
    {
      cdContextRegisterCallback(cdlua_ctx->ctx(), cb_i, (cdCallback)cdCB->func);
      ret = cdCB->lock = func_lock;
    }
  }
  lua_pushnumber(L, ret);
  return 1;
}



/***************************************************************************\
* Color Coding                                                              *
\***************************************************************************/

/***************************************************************************\
* Creates a color as a light userdata. The color value is                   *
* placed in the (void *) value. Not beautiful, but works best.              *
* cd.EncodeColor(r, g, b: number) -> (old_color: color)                     *
\***************************************************************************/
static int cdlua5_encodecolor(lua_State *L)
{
  int red_f, green_f, blue_f;
  unsigned char red_i, green_i, blue_i;
  long int color;

  red_f = luaL_checkint(L, 1);
  green_f = luaL_checkint(L, 2);
  blue_f = luaL_checkint(L, 3);

  if (red_f < 0 || red_f > 255)
    luaL_argerror(L, 1, "color components values should be in range [0, 255]");
  if (green_f < 0 || green_f > 255) 
    luaL_argerror(L, 2, "color components values should be in range [0, 255]");
  if (blue_f < 0 || blue_f > 255)
    luaL_argerror(L, 3, "color components values should be in range [0, 255]");
  
  red_i = (unsigned char) (red_f);
  green_i = (unsigned char) (green_f);
  blue_i = (unsigned char) (blue_f);

  color = cdEncodeColor(red_i, green_i, blue_i);
  lua_pushlightuserdata(L, (void *)color);
  
  return 1;
}

/***************************************************************************\
* Decodes a color previously created.                                       *
* cd.DecodeColor(color: color) -> (r, g, b: number)                         *
\***************************************************************************/
static int cdlua5_decodecolor(lua_State *L)
{
  unsigned char red_i, green_i, blue_i;
  long int color = cdlua_checkcolor(L, 1);
  cdDecodeColor(color, &red_i, &green_i, &blue_i);
  lua_pushnumber(L, red_i);
  lua_pushnumber(L, green_i);
  lua_pushnumber(L, blue_i);

  return 3;
}

/***************************************************************************\
* cd.EncodeAlpha(color: color_tag, alpha: number) -> (color: color)         *
\***************************************************************************/
static int cdlua5_encodealpha(lua_State *L)
{
  float alpha_f;
  unsigned char alpha_i;
  long int color;
  
  color = cdlua_checkcolor(L, 1);

  if (!lua_isnumber(L, 2))
    luaL_argerror(L, 2, "invalid alpha parameter");

  alpha_f = (float) lua_tonumber(L, 2);

  if (alpha_f < 0 || alpha_f > 255)
    luaL_argerror(L, 2, "alpha components values should be in range [0, 255]");
  
  alpha_i = (unsigned char) (alpha_f);

  color = cdEncodeAlpha(color, alpha_i);
  lua_pushlightuserdata(L, (void *) color);
  return 1;
}

/***************************************************************************\
* cd.DecodeAlpha(color: color) -> (a: number)                               *
\***************************************************************************/
static int cdlua5_decodealpha(lua_State* L)
{
  long int color = cdlua_checkcolor(L, 1);
  unsigned char alpha_i = cdDecodeAlpha(color);
  lua_pushnumber(L, alpha_i);
  return 1;
}

/***************************************************************************\
* cd.Alpha(color: color) -> (r: number)                                       *
\***************************************************************************/
static int cdlua5_alpha(lua_State* L)
{
  long int color = cdlua_checkcolor(L, 1);
  lua_pushnumber(L, cdAlpha(color));
  return 1;
}

/***************************************************************************\
* cd.Red(color: color) -> (r: number)                                       *
\***************************************************************************/
static int cdlua5_red(lua_State* L)
{
  long int color = cdlua_checkcolor(L, 1);
  lua_pushnumber(L, cdRed(color));
  return 1;
}

/***************************************************************************\
* cd.Blue(color: color) -> (r: number)                                      *
\***************************************************************************/
static int cdlua5_blue(lua_State *L)
{
  long int color = cdlua_checkcolor(L, 1);
  lua_pushnumber(L, cdBlue(color));
  return 1;
}

/***************************************************************************\
* cd.Green(color: color) -> (r: number)                                     *
\***************************************************************************/
static int cdlua5_green(lua_State *L)
{
  long int color = cdlua_checkcolor(L, 1);
  lua_pushnumber(L, cdGreen(color));
  return 1;
}

static int cdlua5_createpalette(lua_State *L)
{
  int size_i;
  long int *palette;

  size_i = luaL_checkint(L, 1);
  if (size_i < 1)
    luaL_argerror(L, 1, "palette size should be a positive integer");

  palette = (long int *) malloc(256 * sizeof(long int));
  memset(palette, 0, 256 * sizeof(long int));

  cdlua_pushpalette(L, palette, size_i);

  return 1;
}

static int cdlua5_killpalette(lua_State *L)
{
  cdluaPalette* pal = (cdluaPalette *)luaL_checkudata(L, 1, "cdPalette");
  if (pal->color)
  {
    free(pal->color);
    pal->color = NULL;     /* mark as killed */
  }

  return 0;
}

/***************************************************************************\
* color = palette[i]                                                        *
\***************************************************************************/
static int cdlua5_indexpalette(lua_State *L)
{
  cdluaPalette* pal = cdlua_checkpalette(L, 1);

  int index = luaL_checkint(L, 2);
  if (index < 0 || index >= pal->count)
    luaL_argerror(L, 2, "index is out of bounds");

  lua_pushlightuserdata(L, (void*) pal->color[index]);
  return 1;
}

/***************************************************************************\
* palette[i] = color                                                        *
\***************************************************************************/
static int cdlua5_newindexpalette(lua_State *L)
{
  long int color;
  cdluaPalette* pal = cdlua_checkpalette(L, 1);

  int index = luaL_checkint(L, 2);
  if (index < 0 || index >= pal->count)
    luaL_argerror(L, 2, "index is out of bounds");

  color = cdlua_checkcolor(L, 3);

  pal->color[index] = color;
  return 0;
}

/*****************************************************************************\
 len
\*****************************************************************************/
static int cdluaPalette_len(lua_State *L)
{
  cdluaPalette *pal = (cdluaPalette*)lua_touserdata(L, 1);
  lua_pushinteger(L, pal->count);
  return 1;
}

/*****************************************************************************\
 tostring
\*****************************************************************************/
static int cdlua5_tostringpalette (lua_State *L)
{
  cdluaPalette *pal = (cdluaPalette*)lua_touserdata(L, 1);
  lua_pushfstring(L, "cdPalette(%p)%s", pal, (pal->color)? "": "-killed");
  return 1;
}

static int cdlua5_tostringimage (lua_State *L)
{
  cdImage* *image_p = (cdImage**)lua_touserdata(L, 1);
  lua_pushfstring(L, "cdImage(%p)%s", image_p, (*image_p)? "": "-killed");
  return 1;
}

static int cdlua5_tostringbitmap (lua_State *L)
{
  cdBitmap* *bitmap_p = (cdBitmap**)lua_touserdata(L, 1);
  lua_pushfstring(L, "cdBitmap(%p)%s", bitmap_p, (*bitmap_p)? "": "-killed");
  return 1;
}

static int cdlua5_tostringchannel (lua_State *L)
{
  cdluaImageChannel *imagechannel_p = (cdluaImageChannel*)lua_touserdata(L, 1);
  lua_pushfstring(L, "cdImageChannel(%p)%s", imagechannel_p, (imagechannel_p->channel)? "": "-killed");
  return 1;
}

static int cdlua5_tostringstate (lua_State *L)
{
  cdState* *state_p = (cdState**)lua_touserdata(L, 1);
  lua_pushfstring(L, "cdState(%p)%s", state_p, (*state_p)? "": "-released");
  return 1;
}

static int cdlua5_tostringpattern (lua_State *L)
{
  cdluaPattern *pattern_p = (cdluaPattern*)lua_touserdata(L, 1);
  lua_pushfstring(L, "cdPattern(%p)%s", pattern_p, (pattern_p->pattern)? "": "-killed");
  return 1;
}

static int cdlua5_tostringstipple (lua_State *L)
{
  cdluaStipple *stipple_p = (cdluaStipple*)lua_touserdata(L, 1);
  lua_pushfstring(L, "cdStipple(%p)%s", stipple_p, (stipple_p->stipple)? "": "-killed");
  return 1;
}

static int cdlua5_tostringimagergba (lua_State *L)
{
  cdluaImageRGBA *imagergba_p = (cdluaImageRGBA*)lua_touserdata(L, 1);
  lua_pushfstring(L, "cdImageRGBA(%p)%s", imagergba_p, (imagergba_p->red)? "": "-killed");
  return 1;
}

static int cdlua5_tostringimagergb (lua_State *L)
{
  cdluaImageRGB *imagergb_p = (cdluaImageRGB*)lua_touserdata(L, 1);
  lua_pushfstring(L, "cdImageRGB(%p)%s", imagergb_p, (imagergb_p->red)? "": "-killed");
  return 1;
}

static int cdlua5_tostringimagemap (lua_State *L)
{
  cdluaImageMap *imagemap_p = (cdluaImageMap*)lua_touserdata(L, 1);
  lua_pushfstring(L, "cdImageMap(%p)%s", imagemap_p, (imagemap_p->index)? "": "-killed");
  return 1;
}

/***************************************************************************\
* cd.Reserved                                                               *
\***************************************************************************/
static int cdlua5_reserved(lua_State *L)
{
  long int color = cdlua_checkcolor(L, 1);
  lua_pushnumber(L, cdReserved(color));
  return 1;
}

/***************************************************************************\
* cd.GetScreenColorPlanes                                                  *
\***************************************************************************/
static int cdlua5_getscreencolorplanes(lua_State *L)
{
  lua_pushnumber(L, cdGetScreenColorPlanes());
  return 1;
}

/***************************************************************************\
* cd.GetScreenSize                                                         *
\***************************************************************************/
static int cdlua5_getscreensize(lua_State *L)
{
  int width;
  int height;
  double mm_width;
  double mm_height;
  cdGetScreenSize(&width, &height, &mm_width, &mm_height);
  lua_pushnumber(L, width);
  lua_pushnumber(L, height);
  lua_pushnumber(L, mm_width);
  lua_pushnumber(L, mm_height);
  return 4;
}

/***************************************************************************\
* cd.UseContextPlus                                                        *
\***************************************************************************/
static int cdlua5_usecontextplus(lua_State *L)
{
  lua_pushboolean(L, cdUseContextPlus(lua_toboolean(L, 1)));
  return 1;
}


/********************************************************************************\
* CDLua Exported functions                                                       *
\********************************************************************************/
static const struct luaL_reg cdlib[] = {

  /* Initialization */
  {"ContextCaps"   , cdlua5_contextcaps},

  /* Control */
  {"ReleaseState"  , cdlua5_releasestate},

  /* Stipple */
  {"CreateStipple", cdlua5_createstipple},
  {"KillStipple"  , cdlua5_killstipple},
  
  /* Pattern */
  {"CreatePattern", cdlua5_createpattern},
  {"KillPattern"  , cdlua5_killpattern},
  
  /* Client Images */
  {"RGB2Map"          , cdlua5_rgb2map},
  {"CreateBitmap"     , cdlua5_createbitmap},
  {"KillBitmap"       , cdlua5_killbitmap},
  {"BitmapGetData"    , cdlua5_bitmapgetdata},
  {"BitmapSetRect"    , cdlua5_bitmapsetrect},
  {"BitmapRGB2Map"    , cdlua5_bitmaprgb2map},

  {"CreateImageRGB"   , cdlua5_createimagergb},
  {"KillImageRGB"     , cdlua5_killimagergb},
  {"CreateImageRGBA"  , cdlua5_createimagergba},
  {"KillImageRGBA"    , cdlua5_killimagergba},
  {"CreateImageMap"   , cdlua5_createimagemap},
  {"KillImageMap"     , cdlua5_killimagemap},
  
  /* Server Images */
  {"KillImage"        , cdlua5_killimage},

  /* Other */
  {"Version"          , cdlua5_version},
  {"RegisterCallback" , cdlua5_registercallback},
  {"ContextRegisterCallback" , cdlua5_registercallback},

  /* Color Coding */
  {"EncodeColor"    , cdlua5_encodecolor},
  {"DecodeColor"    , cdlua5_decodecolor},
  {"EncodeAlpha"    , cdlua5_encodealpha},
  {"DecodeAlpha"    , cdlua5_decodealpha},
  {"Alpha"          , cdlua5_alpha},
  {"Red"            , cdlua5_red},
  {"Blue"           , cdlua5_blue},
  {"Green"          , cdlua5_green},
  {"Reserved"       , cdlua5_reserved},

  /* Palette */
  {"CreatePalette", cdlua5_createpalette},
  {"KillPalette"  , cdlua5_killpalette},

  /* native window functions */
  {"GetScreenColorPlanes" , cdlua5_getscreencolorplanes},
  {"GetScreenSize" , cdlua5_getscreensize},
  
  /* gdi+ functions */
  {"UseContextPlus"       , cdlua5_usecontextplus},

  {NULL, NULL},
};

void cdlua_addcontext(lua_State *L, cdluaLuaState* cdL, cdluaContext *cdlua_ctx)
{
  int i;
  cdlua_ctx->id = cdL->numdrivers;
  cdL->drivers[cdL->numdrivers] = cdlua_ctx;
  
  lua_pushstring(L, cdlua_ctx->name);
  lua_pushnumber(L, cdL->numdrivers);
  lua_settable(L, -3);

  /* skip CD_SIZECB, register other callbacks */
  for (i=1; i < cdlua_ctx->cb_n; i++)
  {
    lua_pushstring(L, cdlua_ctx->cb_list[i].name);
    lua_pushnumber(L, i);
    lua_settable(L, -3);
  }

  cdL->numdrivers++;
}


/********************************************************************************\
* Exports all CD constants                                                       *
\********************************************************************************/
typedef struct cdlua5_constant {
  const char *name;
  lua_Number value;
} cdlua5_constant;

typedef struct cdlua5_color {
  const char *name;
  long int value;
} cdlua5_color;

static const struct cdlua5_constant cdlibconstant[] = {
  /* query value */
  {"QUERY", CD_QUERY},

  /* these definitions are compatible with the IM library */
  {"RGB" , CD_RGB},
  {"MAP" , CD_MAP},
  {"RGBA", CD_RGBA},

  {"IRED"  , CD_IRED},
  {"IGREEN", CD_IGREEN},
  {"IBLUE" , CD_IBLUE},
  {"IALPHA", CD_IALPHA},
  {"INDEX" , CD_INDEX},
  {"COLORS", CD_COLORS},

  /* status report */
  {"ERROR", CD_ERROR},
  {"OK"   , CD_OK},

  /* clip mode */
  {"CLIPOFF"    , CD_CLIPOFF},
  {"CLIPAREA"   , CD_CLIPAREA},
  {"CLIPPOLYGON", CD_CLIPPOLYGON},
  {"CLIPREGION" , CD_CLIPREGION},

  /* region combine mode */
  {"UNION"       , CD_UNION},
  {"INTERSECT"   , CD_INTERSECT},
  {"DIFFERENCE"  , CD_DIFFERENCE},
  {"NOTINTERSECT", CD_NOTINTERSECT},

  /* polygon mode (begin...end) */
  {"FILL"        , CD_FILL},
  {"OPEN_LINES"  , CD_OPEN_LINES},
  {"CLOSED_LINES", CD_CLOSED_LINES},
  {"CLIP"        , CD_CLIP},
  {"BEZIER"      , CD_BEZIER},
  {"REGION"      , CD_REGION},
  {"PATH"        , CD_PATH},
  {"POLYCUSTOM"  , CD_POLYCUSTOM},

  /* path actions */
  {"PATH_NEW",         CD_PATH_NEW},
  {"PATH_MOVETO",      CD_PATH_MOVETO},
  {"PATH_LINETO",      CD_PATH_LINETO},
  {"PATH_ARC",         CD_PATH_ARC},
  {"PATH_CURVETO",     CD_PATH_CURVETO},
  {"PATH_CLOSE",       CD_PATH_CLOSE},
  {"PATH_FILL",        CD_PATH_FILL},
  {"PATH_STROKE",      CD_PATH_STROKE},
  {"PATH_FILLSTROKE",  CD_PATH_FILLSTROKE},
  {"PATH_CLIP",        CD_PATH_CLIP},

  /* fill mode */
  {"EVENODD", CD_EVENODD},
  {"WINDING", CD_WINDING},

  /* line join  */
  {"MITER", CD_MITER},
  {"BEVEL", CD_BEVEL},
  {"ROUND", CD_ROUND},

  /* line cap  */
  {"CAPFLAT"  , CD_CAPFLAT},
  {"CAPSQUARE", CD_CAPSQUARE},
  {"CAPROUND" , CD_CAPROUND},

  /* background opacity mode */
  {"OPAQUE"     , CD_OPAQUE},
  {"TRANSPARENT", CD_TRANSPARENT},

  /* write mode */
  {"REPLACE", CD_REPLACE},
  {"XOR"    , CD_XOR},
  {"NOT_XOR", CD_NOT_XOR},

  /* color allocation mode (palette) */
  {"POLITE", CD_POLITE},
  {"FORCE" , CD_FORCE},

  /* line style */
  {"CONTINUOUS"  , CD_CONTINUOUS},
  {"DASHED"      , CD_DASHED},
  {"DOTTED"      , CD_DOTTED},
  {"DASH_DOT"    , CD_DASH_DOT},
  {"DASH_DOT_DOT", CD_DASH_DOT_DOT},
  {"CUSTOM"      , CD_CUSTOM},

  /* marker type */
  {"PLUS"          , CD_PLUS},
  {"STAR"          , CD_STAR},
  {"CIRCLE"        , CD_CIRCLE},
  {"X"             , CD_X},
  {"BOX"           , CD_BOX},
  {"DIAMOND"       , CD_DIAMOND},
  {"HOLLOW_CIRCLE" , CD_HOLLOW_CIRCLE},
  {"HOLLOW_BOX"    , CD_HOLLOW_BOX},
  {"HOLLOW_DIAMOND", CD_HOLLOW_DIAMOND},

  /* hatch type */
  {"HORIZONTAL", CD_HORIZONTAL},
  {"VERTICAL"  , CD_VERTICAL},
  {"FDIAGONAL" , CD_FDIAGONAL},
  {"BDIAGONAL" , CD_BDIAGONAL},
  {"CROSS"     , CD_CROSS},
  {"DIAGCROSS" , CD_DIAGCROSS},

  /* interior style */
  {"SOLID"  , CD_SOLID},
  {"HATCH"  , CD_HATCH},
  {"STIPPLE", CD_STIPPLE},
  {"PATTERN", CD_PATTERN},
  {"HOLLOW" , CD_HOLLOW},

  /* text alignment */
  {"NORTH"      , CD_NORTH},
  {"SOUTH"      , CD_SOUTH},
  {"EAST"       , CD_EAST},
  {"WEST"       , CD_WEST},
  {"NORTH_EAST" , CD_NORTH_EAST},
  {"NORTH_WEST" , CD_NORTH_WEST},
  {"SOUTH_EAST" , CD_SOUTH_EAST},
  {"SOUTH_WEST" , CD_SOUTH_WEST},
  {"CENTER"     , CD_CENTER},
  {"BASE_LEFT"  , CD_BASE_LEFT},
  {"BASE_CENTER", CD_BASE_CENTER},
  {"BASE_RIGHT" , CD_BASE_RIGHT},

  /* style */
  {"PLAIN"      , CD_PLAIN},
  {"BOLD"       , CD_BOLD},
  {"ITALIC"     , CD_ITALIC},
  {"BOLD_ITALIC", CD_BOLD_ITALIC},
  {"UNDERLINE"     , CD_UNDERLINE},
  {"STRIKEOUT"     , CD_STRIKEOUT},

  /* font size */
  {"SMALL"   , CD_SMALL},
  {"STANDARD", CD_STANDARD},
  {"LARGE"   , CD_LARGE},

  /* Canvas Capabilities */
  {"CAP_NONE"           , CD_CAP_NONE},
  {"CAP_FLUSH"          , CD_CAP_FLUSH},
  {"CAP_CLEAR"          , CD_CAP_CLEAR},
  {"CAP_PLAY"           , CD_CAP_PLAY},
  {"CAP_YAXIS"          , CD_CAP_YAXIS},
  {"CAP_CLIPAREA"       , CD_CAP_CLIPAREA},
  {"CAP_CLIPPOLY"       , CD_CAP_CLIPPOLY},
  {"CAP_RECT"           , CD_CAP_RECT},
  {"CAP_IMAGERGB"       , CD_CAP_IMAGERGB},
  {"CAP_IMAGERGBA"      , CD_CAP_IMAGERGBA},
  {"CAP_IMAGEMAP"       , CD_CAP_IMAGEMAP},
  {"CAP_GETIMAGERGB"    , CD_CAP_GETIMAGERGB},
  {"CAP_IMAGESRV"       , CD_CAP_IMAGESRV},
  {"CAP_BACKGROUND"     , CD_CAP_BACKGROUND},
  {"CAP_BACKOPACITY"    , CD_CAP_BACKOPACITY},
  {"CAP_WRITEMODE"      , CD_CAP_WRITEMODE},
  {"CAP_LINESTYLE"      , CD_CAP_LINESTYLE},
  {"CAP_LINEWITH"       , CD_CAP_LINEWITH},
  {"CAP_WD"             , CD_CAP_FPRIMTIVES},
  {"CAP_HATCH"          , CD_CAP_HATCH},
  {"CAP_STIPPLE"        , CD_CAP_STIPPLE},
  {"CAP_PATTERN"        , CD_CAP_PATTERN},
  {"CAP_FONT"           , CD_CAP_FONT},
  {"CAP_FONTDIM"        , CD_CAP_FONTDIM},
  {"CAP_TEXTSIZE"       , CD_CAP_TEXTSIZE},
  {"CAP_TEXTORIENTATION", CD_CAP_TEXTORIENTATION},
  {"CAP_PALETTE"        , CD_CAP_PALETTE},
  {"CAP_LINECAP"        , CD_CAP_LINECAP},
  {"CAP_LINEJOIN"       , CD_CAP_LINEJOIN},
  {"CAP_REGION"         , CD_CAP_REGION},
  {"CAP_CHORD"          , CD_CAP_CHORD},
  {"CAP_ALL"            , CD_CAP_ALL},

  /* cdPlay definitions */
  {"SIZECB",   CD_SIZECB},
  {"ABORT",    CD_ABORT},
  {"CONTINUE", CD_CONTINUE},

  /* simulation flags */
  {"SIM_NONE"      , CD_SIM_NONE},
  {"SIM_TEXT"      , CD_SIM_TEXT},
  {"SIM_LINE"      , CD_SIM_LINE},
  {"SIM_RECT"      , CD_SIM_RECT},
  {"SIM_ARC"       , CD_SIM_ARC},
  {"SIM_POLYLINE"  , CD_SIM_POLYLINE},
  {"SIM_BOX"       , CD_SIM_BOX},
  {"SIM_SECTOR"    , CD_SIM_SECTOR},
  {"SIM_POLYGON"   , CD_SIM_POLYGON},
  {"SIM_CHORD"     , CD_SIM_CHORD},
  {"SIM_ALL"       , CD_SIM_ALL},
  {"SIM_LINES"     , CD_SIM_LINES},
  {"SIM_FILLS"     , CD_SIM_FILLS},

  /* some conversion factors */
  {"MM2PT"  , CD_MM2PT},
  {"RAD2DEG", CD_RAD2DEG},

  /* cdcgm.h (the callback names are registered in cdlua_addcontext) */

  /* cdgdiplus.h */
  {"SPLINE"      , CD_SPLINE},
  {"FILLSPLINE"  , CD_FILLSPLINE},
  {"FILLGRADIENT", CD_FILLGRADIENT},

  /* cdps.h */
  {"A0"    , CD_A0},
  {"A1"    , CD_A1},
  {"A2"    , CD_A2},
  {"A3"    , CD_A3},
  {"A4"    , CD_A4},
  {"A5"    , CD_A5},
  {"LETTER", CD_LETTER},
  {"LEGAL" , CD_LEGAL},

  {NULL, -1},
};

static void initconst(lua_State *L)
{
  const cdlua5_constant *l = cdlibconstant;
  for (; l->name; l++) {
    lua_pushstring(L, l->name);
    lua_pushnumber(L, l->value);
    lua_settable(L, -3);
  }
}

/* some predefined colors for convenience */
static const struct cdlua5_color cdlibcolor[] = {
  {"RED"         , CD_RED},
  {"DARK_RED"    , CD_DARK_RED},
  {"GREEN"       , CD_GREEN},
  {"DARK_GREEN"  , CD_DARK_GREEN},
  {"BLUE"        , CD_BLUE},
  {"DARK_BLUE"   , CD_DARK_BLUE},
  {"YELLOW"      , CD_YELLOW},
  {"DARK_YELLOW" , CD_DARK_YELLOW},
  {"MAGENTA"     , CD_MAGENTA},
  {"DARK_MAGENTA", CD_DARK_MAGENTA},
  {"CYAN"        , CD_CYAN},
  {"DARK_CYAN"   , CD_DARK_CYAN},
  {"WHITE"       , CD_WHITE},
  {"BLACK"       , CD_BLACK},
  {"DARK_GRAY"   , CD_DARK_GRAY},
  {"GRAY"        , CD_GRAY},
  {NULL, -1},
};

static void initcolor(lua_State *L)
{
  const cdlua5_color *l = cdlibcolor;
  for (; l->name; l++) 
  {
    lua_pushstring(L, l->name);
    lua_pushlightuserdata(L, (void*) l->value);
    lua_settable(L, -3);
  }
}

static void initmetatables(lua_State *L)
{
  /* there is no object orientation for these metatables, 
     only gc and optionaly array access */

  luaL_newmetatable(L, "cdState");   /* create new metatable for cdState handles */
  lua_pushliteral (L, "__gc");
  lua_pushcfunction (L, cdlua5_releasestate);  /* register the method */
  lua_settable (L, -3);  
  lua_pushliteral(L, "__tostring");
  lua_pushcfunction(L, cdlua5_tostringstate);
  lua_settable(L, -3);
  lua_pop(L, 1);   /* removes the metatable from the top of the stack */

  luaL_newmetatable(L, "cdImage");
  lua_pushliteral (L, "__gc");
  lua_pushcfunction (L, cdlua5_killimage);
  lua_settable (L, -3);  
  lua_pushliteral(L, "__tostring");
  lua_pushcfunction(L, cdlua5_tostringimage);
  lua_settable(L, -3);
  lua_pop(L, 1);

  luaL_newmetatable(L, "cdBitmap");
  lua_pushliteral (L, "__gc");
  lua_pushcfunction (L, cdlua5_killbitmap);
  lua_settable (L, -3);  
  lua_pushliteral(L, "__index");
  lua_pushcfunction(L, cdlua5_indexbitmap);
  lua_settable(L, -3);
  lua_pushliteral(L, "__tostring");
  lua_pushcfunction(L, cdlua5_tostringbitmap);
  lua_settable(L, -3);
  lua_pushliteral(L, "Width");
  lua_pushcfunction(L, cdlua5_BitmapWidth);
  lua_settable(L, -3);
  lua_pushliteral(L, "Height");
  lua_pushcfunction(L, cdlua5_BitmapHeight);
  lua_settable(L, -3);
  lua_pushliteral(L, "Type");
  lua_pushcfunction(L, cdlua5_BitmapType);
  lua_settable(L, -3);
  lua_pop(L, 1);

  luaL_newmetatable(L, "cdImageRGB");
  lua_pushliteral (L, "__gc");
  lua_pushcfunction (L, cdlua5_killimagergb);
  lua_settable (L, -3);
  lua_pushliteral(L, "__index");
  lua_pushcfunction(L, cdlua5_indeximagergb);
  lua_settable(L, -3);
  lua_pushliteral(L, "__tostring");
  lua_pushcfunction(L, cdlua5_tostringimagergb);
  lua_settable(L, -3);
  lua_pop(L, 1);

  luaL_newmetatable(L, "cdImageRGBA");
  lua_pushliteral (L, "__gc");
  lua_pushcfunction (L, cdlua5_killimagergba);
  lua_settable (L, -3);  
  lua_pushliteral(L, "__index");
  lua_pushcfunction(L, cdlua5_indeximagergba);
  lua_settable(L, -3);
  lua_pushliteral(L, "__tostring");
  lua_pushcfunction(L, cdlua5_tostringimagergba);
  lua_settable(L, -3);
  lua_pop(L, 1);

  luaL_newmetatable(L, "cdImageChannel");
  lua_pushliteral(L, "__index");
  lua_pushcfunction(L, cdlua5_indexchannel);
  lua_settable(L, -3);
  lua_pushliteral(L, "__newindex");
  lua_pushcfunction(L, cdlua5_newindexchannel);
  lua_settable(L, -3);
  lua_pushliteral(L, "__tostring");
  lua_pushcfunction(L, cdlua5_tostringchannel);
  lua_settable(L, -3);
  lua_pop(L, 1);

  luaL_newmetatable(L, "cdStipple");
  lua_pushliteral (L, "__gc");
  lua_pushcfunction (L, cdlua5_killstipple);
  lua_settable (L, -3);
  lua_pushliteral(L, "__index");
  lua_pushcfunction(L, cdlua5_indexstipple);
  lua_settable(L, -3);
  lua_pushliteral(L, "__newindex");
  lua_pushcfunction(L, cdlua5_newindexstipple);
  lua_settable(L, -3);
  lua_pushliteral(L, "__tostring");
  lua_pushcfunction(L, cdlua5_tostringstipple);
  lua_settable(L, -3);
  lua_pop(L, 1);

  luaL_newmetatable(L, "cdPattern");
  lua_pushliteral (L, "__gc");
  lua_pushcfunction (L, cdlua5_killpattern);
  lua_settable (L, -3);
  lua_pushliteral(L, "__index");
  lua_pushcfunction(L, cdlua5_indexpattern);
  lua_settable(L, -3);
  lua_pushliteral(L, "__newindex");
  lua_pushcfunction(L, cdlua5_newindexpattern);
  lua_settable(L, -3);
  lua_pushliteral(L, "__tostring");
  lua_pushcfunction(L, cdlua5_tostringpattern);
  lua_settable(L, -3);
  lua_pop(L, 1); 

  luaL_newmetatable(L, "cdPalette");
  lua_pushliteral(L, "__gc");
  lua_pushcfunction(L, cdlua5_killpalette);
  lua_settable(L, -3);
  lua_pushliteral(L, "__index");
  lua_pushcfunction(L, cdlua5_indexpalette);
  lua_settable(L, -3);
  lua_pushliteral(L, "__newindex");
  lua_pushcfunction(L, cdlua5_newindexpalette);
  lua_settable(L, -3);
  lua_pushliteral(L, "__tostring");
  lua_pushcfunction(L, cdlua5_tostringpalette);
  lua_settable(L, -3);
  lua_pushliteral(L, "__len");
  lua_pushcfunction(L, cdluaPalette_len);
  lua_settable(L, -3);
  lua_pop(L, 1);

  luaL_newmetatable(L, "cdImageMap");
  lua_pushliteral(L, "__gc");
  lua_pushcfunction(L, cdlua5_killimagemap);
  lua_settable(L, -3);
  lua_pushliteral(L, "__index");
  lua_pushcfunction(L, cdlua5_indeximagemap);
  lua_settable(L, -3);
  lua_pushliteral(L, "__newindex");
  lua_pushcfunction(L, cdlua5_newindeximagemap);
  lua_settable(L, -3);
  lua_pushliteral(L, "__tostring");
  lua_pushcfunction(L, cdlua5_tostringimagemap);
  lua_settable(L, -3);
  lua_pop(L, 1);
}

static void setinfo (lua_State *L) 
{
	lua_pushliteral (L, "_COPYRIGHT");
	lua_pushliteral (L, CD_COPYRIGHT);
	lua_settable (L, -3);

	lua_pushliteral (L, "_DESCRIPTION");
	lua_pushliteral (L, CD_DESCRIPTION);
	lua_settable (L, -3);

	lua_pushliteral (L, "_NAME");
	lua_pushliteral (L, CD_NAME);
	lua_settable (L, -3);

	lua_pushliteral (L, "_VERSION");
	lua_pushstring (L, cdVersion());
	lua_settable (L, -3);

	lua_pushliteral (L, "_VERSION_DATE");
	lua_pushliteral(L, CD_VERSION_DATE);
	lua_settable (L, -3);

	lua_pushliteral (L, "_VERSION_NUMBER");
	lua_pushinteger(L, cdVersionNumber());
	lua_settable (L, -3);
}


/********************************************************************************\
* CDLua OpenLib                                                                  *
\********************************************************************************/


int cdlua_open (lua_State *L)
{                                  
  cdluaLuaState* cdL = malloc(sizeof(cdluaLuaState));
  memset(cdL, 0, sizeof(cdluaLuaState));
  cdlua_SetState(L, cdL);

  initmetatables(L);

  luaL_register(L, "cd", cdlib);   /* leave "cd" table at the top of the stack */
  setinfo(L);

  cdlua_open_active(L, cdL);
  cdlua_open_canvas(L);

  cdlua_initdrivers(L, cdL);
  initconst(L);
  initcolor(L);

  return 1;
}

int cdlua_close(lua_State *L)
{
  cdluaLuaState* cdL = cdlua_getstate(L);
  if (cdL)
  {
    cdKillCanvas(cdL->void_canvas);
    free(cdL);
  }
  return 0;
}

int luaopen_cdlua(lua_State* L)
{
  return cdlua_open(L);
}
