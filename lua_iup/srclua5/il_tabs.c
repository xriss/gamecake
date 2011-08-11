/******************************************************************************
 * Automatically generated file (iuplua5). Please don't change anything.                *
 *****************************************************************************/

#include <stdlib.h>

#include <lua.h>
#include <lauxlib.h>

#include "iup.h"
#include "iuplua.h"
#include "il.h"


static int tabs_tabchangepos_cb(Ihandle *self, int p0, int p1)
{
  lua_State *L = iuplua_call_start(self, "tabchangepos_cb");
  lua_pushinteger(L, p0);
  lua_pushinteger(L, p1);
  return iuplua_call(L, 2);
}

static int tabs_tabchange_cb(Ihandle *self, Ihandle * p0, Ihandle * p1)
{
  lua_State *L = iuplua_call_start(self, "tabchange_cb");
  iuplua_pushihandle(L, p0);
  iuplua_pushihandle(L, p1);
  return iuplua_call(L, 2);
}

static int Tabsv(lua_State *L)
{
  Ihandle **hlist = iuplua_checkihandle_array(L, 1, 0);
  Ihandle *h = IupTabsv(hlist);
  iuplua_plugstate(L, h);
  iuplua_pushihandle_raw(L, h);
  free(hlist);
  return 1;
}

int iuptabslua_open(lua_State * L)
{
  iuplua_register(L, Tabsv, "Tabsv");

  iuplua_register_cb(L, "TABCHANGEPOS_CB", (lua_CFunction)tabs_tabchangepos_cb, NULL);
  iuplua_register_cb(L, "TABCHANGE_CB", (lua_CFunction)tabs_tabchange_cb, NULL);

#ifdef IUPLUA_USELOH
#include "tabs.loh"
#else
#ifdef IUPLUA_USELZH
#include "tabs.lzh"
#else
  iuplua_dofile(L, "tabs.lua");
#endif
#endif

  return 0;
}

