/** \file
 * \brief PDF Canvas Lua 3 Binding
 *
 * See Copyright Notice in cd.h
 */

#include <stdlib.h>
#include <stdio.h>

#include "cd.h"
#include "cdpdf.h"

#include <lua.h>
#include <lauxlib.h>

#include "cdlua.h"
#include "cdluapdf.h"
#include "cdlua3_private.h"

static void *cdpdf_checkdata(int param)
{
  lua_Object data = lua_getparam(param);
  if (!lua_isstring(data))
    lua_error("cdCreateCanvas CD_PDF: data should be of type string!");

  return lua_getstring(data);
}

static cdContextLUA cdluapdfctx = 
{
  0,
  "CD_PDF",
  cdContextPDF,
  cdpdf_checkdata,
  NULL,
  0
};

void cdluapdf_open(void)
{
  cdlua_addcontext(&cdluapdfctx);
}

