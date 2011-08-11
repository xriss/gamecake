/** \file
 * \brief GL Canvas Lua 3 Binding
 *
 * See Copyright Notice in cd.h
 */

#include <stdlib.h>
#include <stdio.h>

#include "cd.h"
#include "cdgl.h"

#include <lua.h>
#include <lauxlib.h>

#include "cdlua.h"
#include "cdluagl.h"
#include "cdlua3_private.h"

static void *cdgl_checkdata(int param)
{
  lua_Object data = lua_getparam(param);
  if (!lua_isstring(data))
    lua_error("cdCreateCanvas CD_GL: data should be of type string!");

  return lua_getstring(data);
}

static cdContextLUA cdluaglctx = 
{
  0,
  "CD_GL",
  cdContextGL,
  cdgl_checkdata,
  NULL,
  0
};

void cdluagl_open(void)
{
  cdlua_addcontext(&cdluaglctx);
}

