/******************************************************************************
 * Automatically generated file (iuplua5). Please don't change anything.                *
 *****************************************************************************/

#include <stdlib.h>

#include <lua.h>
#include <lauxlib.h>

#include "iup.h"
#include "iuplua.h"
#include "iuptuio.h"
#include "il.h"


static int tuioclient_touch_cb(Ihandle *self, int p0, int p1, int p2, char * p3)
{
  lua_State *L = iuplua_call_start(self, "touch_cb");
  lua_pushinteger(L, p0);
  lua_pushinteger(L, p1);
  lua_pushinteger(L, p2);
  lua_pushstring(L, p3);
  return iuplua_call(L, 4);
}

static int TuioClient(lua_State *L)
{
  Ihandle *ih = IupTuioClient(luaL_optinteger(L, 1, 0));
  iuplua_plugstate(L, ih);
  iuplua_pushihandle_raw(L, ih);
  return 1;
}

int iuptuioclientlua_open(lua_State * L)
{
  iuplua_register(L, TuioClient, "TuioClient");

  iuplua_register_cb(L, "TOUCH_CB", (lua_CFunction)tuioclient_touch_cb, NULL);

#ifdef IUPLUA_USELOH
#include "tuioclient.loh"
#else
#ifdef IUPLUA_USELZH
#include "tuioclient.lzh"
#else
  iuplua_dofile(L, "tuioclient.lua");
#endif
#endif

  return 0;
}

 
static int tuio_multitouch_cb(Ihandle *ih, int count, int* id, int* px, int* py, int* pstate)
{
  int i;
  lua_State *L = iuplua_call_start(ih, "multitouch_cb");
  lua_pushinteger(L, count);
  
  lua_createtable(L, count, 0);
  for (i = 0; i < count; i++)
  {
    lua_pushinteger(L,i+1);
    lua_pushinteger(L,id[i]);
    lua_settable(L,-3);
  }
  
  lua_createtable(L, count, 0);
  for (i = 0; i < count; i++)
  {
    lua_pushinteger(L,i+1);
    lua_pushinteger(L,px[i]);
    lua_settable(L,-3);
  }
  
  lua_createtable(L, count, 0);
  for (i = 0; i < count; i++)
  {
    lua_pushinteger(L,i+1);
    lua_pushinteger(L,py[i]);
    lua_settable(L,-3);
  }
  
  
  lua_createtable(L, count, 0);
  for (i = 0; i < count; i++)
  {
    lua_pushinteger(L,i+1);
    lua_pushinteger(L,pstate[i]);
    lua_settable(L,-3);
  }
  
  return iuplua_call(L, 5);
}

int iuptuiolua_open(lua_State* L)
{
  if (iuplua_opencall_internal(L))
    IupTuioOpen();
    
  iuplua_get_env(L);
  iuptuioclientlua_open(L);
  
  iuplua_register_cb(L, "MULTITOUCH_CB", (lua_CFunction)tuio_multitouch_cb, NULL);
  
  return 0;
}

/* obligatory to use require"iupluatuio" */
int luaopen_iupluatuio(lua_State* L)
{
  return iuptuiolua_open(L);
}

