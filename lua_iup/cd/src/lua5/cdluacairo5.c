/** \file
 * \brief Cairo Lua 5 Binding
 *
 * See Copyright Notice in cd.h
 */

#include <stdlib.h>
#include <stdio.h>

#include "cd.h"
#include "cdcairo.h"

#include <lua.h>
#include <lauxlib.h>

#include "cdlua.h"
#include "cdlua5_private.h"


static void *cdimagergb_checkdata(lua_State* L, int param)
{
  return (void *)luaL_checkstring(L, param);
}

static cdluaContext cdluaimagergbctx = 
{
  0,
  "CAIRO_IMAGERGB",
  cdContextCairoImageRGB,
  cdimagergb_checkdata,
  NULL,
  0
};

static void *cdps_checkdata( lua_State *L, int param)
{
  return (void *)luaL_checkstring(L, param);
}

static cdluaContext cdluapsctx = 
{
  0,
  "CAIRO_PS",
  cdContextCairoPS,
  cdps_checkdata,
  NULL,
  0
};

static void *cdsvg_checkdata( lua_State *L, int param)
{
  return (void *)luaL_checkstring(L, param);
}

static cdluaContext cdluasvgctx = 
{
  0,
  "CAIRO_SVG",
  cdContextCairoSVG,
  cdsvg_checkdata,
  NULL,
  0
};

static void *cdpdf_checkdata(lua_State *L, int param)
{
  return (void *)luaL_checkstring(L, param);
}

static cdluaContext cdluapdfctx = 
{
  0,
  "CAIRO_PDF",
  cdContextCairoPDF,
  cdpdf_checkdata,
  NULL,
  0
};

static int cdlua5_initcairo(lua_State *L)
{
  (void)L;
  cdInitContextPlus();
  return 0;
}

static const struct luaL_reg cdlib[] = {
  {"InitContextPlus", cdlua5_initcairo},
  {NULL, NULL},
};


static int cdluacairo_open (lua_State *L)
{
  cdluaLuaState* cdL = cdlua_getstate(L);
  cdInitContextPlus();
  luaL_register(L, "cd", cdlib);   /* leave "cd" table at the top of the stack */
  cdlua_addcontext(L, cdL, &cdluapdfctx);
  cdlua_addcontext(L, cdL, &cdluapsctx);
  cdlua_addcontext(L, cdL, &cdluasvgctx);
  cdlua_addcontext(L, cdL, &cdluaimagergbctx);
  return 1;
}

int luaopen_cdluacairo(lua_State* L)
{
  return cdluacairo_open(L);
}
