/** \file
 * \brief Lua Binding of the Canvas dependent API
 *
 * See Copyright Notice in cd.h
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cd.h"
#include "wd.h"
#include "cdirgb.h"

#include <lua.h>
#include <lauxlib.h>

#include "cdlua.h"
#include "cdlua5_private.h"

#define _cdCheckCanvas(_canvas) (_canvas!=NULL && ((unsigned char*)_canvas)[0] == 'C' && ((unsigned char*)_canvas)[1] == 'D')


void cdlua_pushcanvas(lua_State * L, cdCanvas* canvas)
{
  cdCanvas* *canvas_p = (cdCanvas* *) lua_newuserdata(L, sizeof(cdCanvas*));
  *canvas_p = canvas;
  luaL_getmetatable(L, "cdCanvas");
  lua_setmetatable(L, -2);
}

cdCanvas* cdlua_checkcanvas(lua_State * L, int pos)
{
  cdCanvas* *canvas_p = (cdCanvas**)luaL_checkudata(L, pos, "cdCanvas");
  if (!(*canvas_p))
    luaL_argerror(L, pos, "killed cdCanvas");
  if (!_cdCheckCanvas(*canvas_p))
    luaL_argerror(L, pos, "invalid Lua object, killed cdCanvas in C but not in Lua");
  return *canvas_p;
}

cdCanvas* cdlua_getcanvas(lua_State * L)
{
  return cdlua_checkcanvas(L, 1);
}

static int cdlua5_createcanvas(lua_State * L) 
{
  cdluaContext* cdlua_ctx = cdlua_getcontext(L, 1);
  void *data_p = cdlua_ctx->checkdata(L, 2);

  cdCanvas* canvas = cdCreateCanvas(cdlua_ctx->ctx(), data_p);
  
  /* if creation failed, return nil so that the user can compare */
  /* the result with nil and know that it failed */
  if (canvas) 
    cdlua_pushcanvas(L, canvas);
  else
    lua_pushnil(L);

  return 1;
}

static int cdlua5_killcanvas(lua_State *L)
{
  cdCanvas* *canvas_p = (cdCanvas**) luaL_checkudata(L, 1, "cdCanvas");
  if (!(*canvas_p))
    luaL_argerror(L, 1, "killed cdCanvas");
  if (!_cdCheckCanvas(*canvas_p))
    luaL_argerror(L, 1, "invalid Lua object, killed cdCanvas in C but not in Lua");

  cdlua_kill_active(L, *canvas_p);

  cdKillCanvas(*canvas_p);
  *canvas_p = NULL;   /* mark as killed */

  return 0;
}

static int cdluaCanvas_eq (lua_State *L)
{
  cdCanvas* canvas1 = cdlua_checkcanvas(L, 1);
  cdCanvas* canvas2 = cdlua_checkcanvas(L, 2);
  lua_pushboolean(L, canvas1 == canvas2);
  return 1;
}

static int cdluaCanvas_tostring (lua_State *L)
{
  cdCanvas* *canvas_p = (cdCanvas**) luaL_checkudata(L, 1, "cdCanvas");
  if (!(*canvas_p))
    lua_pushfstring(L, "cdCanvas(%p - NULL)-killed", canvas_p);
  else if (!_cdCheckCanvas(*canvas_p))
    lua_pushfstring(L, "cdCanvas(%p - INVALID)-killed in C but not in Lua", canvas_p);
  else
    lua_pushfstring(L, "cdCanvas(%p - %p)", canvas_p, *canvas_p);
  return 1;
}

static int cdlua5_getcontext(lua_State * L)
{
  cdluaLuaState* cdL;
  cdContext* ctx;
  int i;
  int driver = -1;
  cdCanvas* canvas = cdlua_checkcanvas(L, 1);

  ctx = cdCanvasGetContext(canvas);
  cdL = cdlua_getstate(L);

  for (i=0; i < cdL->numdrivers; i++)
  {
    if (ctx == cdL->drivers[i]->ctx())
    {
      driver = i;
      break;
    }
  }

  if (i == cdL->numdrivers)
    luaL_argerror(L, 1, "unknown driver");

  lua_pushnumber(L, driver);
  return 1;
}

/***************************************************************************\
* Activates a cd canvas.                                                    *
\***************************************************************************/
static int cdlua5_activate(lua_State * L)
{
  lua_pushnumber(L, cdCanvasActivate(cdlua_checkcanvas(L, 1)));
  return 1;
}

static int cdlua5_deactivate(lua_State * L)
{
  cdCanvasDeactivate(cdlua_checkcanvas(L, 1));
  return 0;
}

/***************************************************************************\
* cd.Simulate(mode: number) -> (old_mode: number)                           *
\***************************************************************************/
static int cdlua5_simulate(lua_State *L)
{
  lua_pushnumber(L, cdCanvasSimulate(cdlua_checkcanvas(L, 1), luaL_checkint(L, 2)));
  return 1;
}

/***************************************************************************\
* Control                                                                   *
\***************************************************************************/

/***************************************************************************\
* cd.Clear()                                                                *
\***************************************************************************/
static int cdlua5_clear(lua_State *L)
{
  cdCanvasClear(cdlua_checkcanvas(L, 1));
  return 0;
}

/***************************************************************************\
* cd.Flush()                                                                *
\***************************************************************************/
static int cdlua5_flush(lua_State *L)
{
  cdCanvasFlush(cdlua_checkcanvas(L, 1));
  return 0;
}

static int cdlua5_savestate(lua_State *L)
{
  cdState* state = cdCanvasSaveState(cdlua_checkcanvas(L, 1));
  if (state)
    cdlua_pushstate(L, state);
  else
    lua_pushnil(L);
  return 1;
}

static int cdlua5_restorestate(lua_State * L)
{
  cdCanvasRestoreState(cdlua_checkcanvas(L, 1), cdlua_checkstate(L, 2));
  return 0;
}

static int cdlua_isuserdata(const char* name)
{
  if (strcmp(name, "HDC")==0) return 1;
  if (strcmp(name, "GC")==0) return 1;
  return 0;
}

/***************************************************************************\
* cd.SetAttribute(name, data: string)                                       *
\***************************************************************************/
static int cdlua5_setattribute(lua_State *L)
{
  const char* name = luaL_checkstring(L, 2);
   
  if (lua_isnil(L, 2))
  {
    cdCanvasSetAttribute(cdlua_checkcanvas(L, 1), name, NULL);
  }
  else
  {
    char* data;
    if (cdlua_isuserdata(name))
      data = (char*) lua_touserdata(L, 3);
    else
      data = (char*) luaL_checkstring(L, 3);
    cdCanvasSetAttribute(cdlua_checkcanvas(L, 1), name, data);
  }
  return 0;
}


/***************************************************************************\
* cd.SetAttribute(name: string) -> (data: string)                           *
\***************************************************************************/
static int cdlua5_getattribute(lua_State *L)
{
  const char* name = luaL_checkstring(L, 2);
  char* data = cdCanvasGetAttribute(cdlua_checkcanvas(L, 1), name);
  if (data)
  {
    if (cdlua_isuserdata(name))
      lua_pushlightuserdata(L, data);
    else
      lua_pushstring(L, data);
  }
  else
    lua_pushnil(L);
  return 1;
}



/***************************************************************************\
* Coordinate System                                                         *
\***************************************************************************/

/***************************************************************************\
* cd.GetCanvasSize() -> (width, heigth, mm_width, mm_height: number)        *
\***************************************************************************/
static int cdlua5_getcanvassize(lua_State *L)
{
  int width;
  int height;
  double mm_width;
  double mm_height;

  cdCanvasGetSize(cdlua_checkcanvas(L, 1), &width, &height, &mm_width, &mm_height);
  
  lua_pushnumber(L, width);
  lua_pushnumber(L, height);
  lua_pushnumber(L, mm_width);
  lua_pushnumber(L, mm_height);
  return 4;
}

/***************************************************************************\
* cd.UpdateYAxis(yc: number) -> (yr: number)                                *
\***************************************************************************/
static int cdlua5_updateyaxis(lua_State *L)
{
  double y = luaL_checknumber(L, 2);
  cdfCanvasUpdateYAxis(cdlua_checkcanvas(L, 1), &y);
  lua_pushnumber(L, y);
  return 1;
}

static int cdlua5_yaxismode(lua_State *L)
{
  lua_pushnumber(L, cdCanvasYAxisMode(cdlua_checkcanvas(L, 1), luaL_checkint(L, 2)));
  return 1;
}

static int cdlua5_invertyaxis(lua_State *L)
{
  lua_pushnumber(L, cdfCanvasInvertYAxis(cdlua_checkcanvas(L, 1), luaL_checknumber(L, 2)));
  return 1;
}

static int cdlua5_mm2pixel(lua_State *L)
{
  double mm_dx, mm_dy;
  int dx, dy;

  mm_dx = luaL_checknumber(L,2);
  mm_dy =  luaL_checknumber(L,3);

  cdCanvasMM2Pixel(cdlua_checkcanvas(L, 1), mm_dx, mm_dy, &dx, &dy);
  lua_pushnumber(L, dx);
  lua_pushnumber(L, dy);
  return 2;
}

static int cdlua5_pixel2mm(lua_State *L)
{
  double mm_dx_d, mm_dy_d;
  int dx, dy;

  dx = luaL_checkint(L,2);
  dy = luaL_checkint(L,3);

  cdCanvasPixel2MM(cdlua_checkcanvas(L, 1), dx, dy, &mm_dx_d, &mm_dy_d);
  lua_pushnumber(L, mm_dx_d);
  lua_pushnumber(L, mm_dy_d);
  return 2;
}

static int cdlua5_fmm2pixel(lua_State *L)
{
  double mm_dx, mm_dy;
  double dx, dy;

  mm_dx = luaL_checknumber(L,2);
  mm_dy =  luaL_checknumber(L,3);

  cdfCanvasMM2Pixel(cdlua_checkcanvas(L, 1), mm_dx, mm_dy, &dx, &dy);
  lua_pushnumber(L, dx);
  lua_pushnumber(L, dy);
  return 2;
}

static int cdlua5_fpixel2mm(lua_State *L)
{
  double mm_dx_d, mm_dy_d;
  double dx, dy;

  dx = luaL_checknumber(L,2);
  dy = luaL_checknumber(L,3);

  cdfCanvasPixel2MM(cdlua_checkcanvas(L, 1), dx, dy, &mm_dx_d, &mm_dy_d);
  lua_pushnumber(L, mm_dx_d);
  lua_pushnumber(L, mm_dy_d);
  return 2;
}

static int cdlua5_origin(lua_State *L)
{
  cdCanvasOrigin(cdlua_checkcanvas(L, 1), luaL_checkint(L,2), luaL_checkint(L,3));
  return 0;
}

static int cdlua5_getorigin(lua_State *L)
{
  int x, y;
  cdCanvasGetOrigin(cdlua_checkcanvas(L, 1), &x, &y);
  lua_pushnumber(L, x);
  lua_pushnumber(L, y);
  return 2;
}

static int cdlua5_forigin(lua_State *L)
{
  cdfCanvasOrigin(cdlua_checkcanvas(L, 1), luaL_checknumber(L,2), luaL_checknumber(L,3));
  return 0;
}

static int cdlua5_fgetorigin(lua_State *L)
{
  double x, y;
  cdfCanvasGetOrigin(cdlua_checkcanvas(L, 1), &x, &y);
  lua_pushnumber(L, x);
  lua_pushnumber(L, y);
  return 2;
}

static int cdlua5_transform(lua_State *L)
{
  double matrix[6];
  int i;

  if (lua_isnil(L, 2))
  {
    cdCanvasTransform(cdlua_checkcanvas(L, 1), NULL);
    return 0;
  }

  if (!lua_istable(L, 2))
    luaL_argerror(L, 2, "invalid matrix, must be a table");

  for (i=0; i < 6; i++)
  {
    lua_rawgeti(L, 2, i+1);

    if (!lua_isnumber(L, -1))
      luaL_argerror(L, 2, "invalid matrix value, must be a number");

    matrix[i] = lua_tonumber(L, -1);
    lua_pop(L, 1);
  }

  cdCanvasTransform(cdlua_checkcanvas(L, 1), matrix);
  return 0;
}

static int cdlua5_gettransform(lua_State *L)
{
  int i;
  double* matrix = cdCanvasGetTransform(cdlua_checkcanvas(L, 1));
  lua_createtable(L, 6, 0);
  for (i=0; i < 6; i++)
  {
    lua_pushnumber(L, matrix[i]);
    lua_rawseti(L, 1, i+1);
  }
  return 1;
}

static int cdlua5_transformmultiply(lua_State *L)
{
  double matrix[6];
  int i;

  if (!lua_istable(L, 2))
    luaL_argerror(L, 2, "invalid matrix, must be a table");

  for (i=0; i < 6; i++)
  {
    lua_rawgeti(L, 2, i+1);

    if (!lua_isnumber(L, -1))
      luaL_argerror(L, 1, "invalid matrix value, must be a number");

    matrix[i] = lua_tonumber(L, -1);
    lua_pop(L, 1);
  }

  cdCanvasTransformMultiply(cdlua_checkcanvas(L, 1), matrix);
  return 0;
}

static int cdlua5_transformrotate(lua_State *L)
{
  cdCanvasTransformRotate(cdlua_checkcanvas(L, 1), luaL_checknumber(L, 2));
  return 0;
}

static int cdlua5_transformscale(lua_State *L)
{
  cdCanvasTransformScale(cdlua_checkcanvas(L, 1), luaL_checknumber(L, 2), luaL_checknumber(L, 3));
  return 0;
}

static int cdlua5_transformtranslate(lua_State *L)
{
  cdCanvasTransformTranslate(cdlua_checkcanvas(L, 1), luaL_checknumber(L, 2), luaL_checknumber(L, 3));
  return 0;
}

static int cdlua5_transformpoint(lua_State *L)
{
  int x, y;
  cdCanvasTransformPoint(cdlua_checkcanvas(L, 1), luaL_checkint(L, 2), luaL_checkint(L, 3), &x, &y);
  lua_pushnumber(L, x);
  lua_pushnumber(L, y);
  return 2;
}

static int cdlua5_ftransformpoint(lua_State *L)
{
  double x, y;
  cdfCanvasTransformPoint(cdlua_checkcanvas(L, 1), luaL_checknumber(L, 2), luaL_checknumber(L, 3), &x, &y);
  lua_pushnumber(L, x);
  lua_pushnumber(L, y);
  return 2;
}

/***************************************************************************\
* World Coordinates                                                         *
\***************************************************************************/

/***************************************************************************\
* wd.GetTransform() -> (sx, sy, tx, ty: number)                        *
\***************************************************************************/
static int wdlua5_gettransform(lua_State *L)
{
  double sx, sy, tx, ty;

  wdCanvasGetTransform(cdlua_checkcanvas(L, 1), &sx, &sy, &tx, &ty);
  lua_pushnumber(L, sx);
  lua_pushnumber(L, sy);
  lua_pushnumber(L, tx);
  lua_pushnumber(L, ty);
  return 4;
}

/***************************************************************************\
* wd.SetTransform(sx, sy, tx, ty: number)                                 *
\***************************************************************************/
static int wdlua5_settransform(lua_State *L)
{
  double sx = luaL_checknumber(L, 2);
  double sy = luaL_checknumber(L, 3);
  double tx = luaL_checknumber(L, 4);
  double ty = luaL_checknumber(L, 5);
  wdCanvasSetTransform(cdlua_checkcanvas(L, 1), sx, sy, tx, ty);
  return 0;
}

/***************************************************************************\
* wd.Translate(tx, ty: number)                                 *
\***************************************************************************/
static int wdlua5_translate(lua_State *L)
{
  double tx = luaL_checknumber(L, 2);
  double ty = luaL_checknumber(L, 3);
  wdCanvasTranslate(cdlua_checkcanvas(L, 1), tx, ty);
  return 0;
}

/***************************************************************************\
* wd.Scale(sx, sy: number)                                 *
\***************************************************************************/
static int wdlua5_scale(lua_State *L)
{
  double sx = luaL_checknumber(L, 2);
  double sy = luaL_checknumber(L, 3);
  wdCanvasScale(cdlua_checkcanvas(L, 1), sx, sy);
  return 0;
}

/***************************************************************************\
* wd.Window(xmin, xmax, ymin, ymax: number)                                 *
\***************************************************************************/
static int wdlua5_window(lua_State *L)
{
  double xmin = luaL_checknumber(L, 2);
  double xmax = luaL_checknumber(L, 3);
  double ymin = luaL_checknumber(L, 4);
  double ymax = luaL_checknumber(L, 5);
  wdCanvasWindow(cdlua_checkcanvas(L, 1), xmin, xmax, ymin, ymax);
  return 0;
}

/***************************************************************************\
* wd.GetWindow() -> (xmin, xmax, ymin, ymax: number)                        *
\***************************************************************************/
static int wdlua5_getwindow(lua_State *L)
{
  double xmin, xmax, ymin, ymax;

  wdCanvasGetWindow(cdlua_checkcanvas(L, 1), &xmin, &xmax, &ymin, &ymax);
  lua_pushnumber(L, xmin);
  lua_pushnumber(L, xmax);
  lua_pushnumber(L, ymin);
  lua_pushnumber(L, ymax);
  return 4;
}

/***************************************************************************\
* wd.Viewport(xmin, xmax, ymin, ymax: number)                               *
\***************************************************************************/
static int wdlua5_viewport(lua_State *L)
{
  int xmin = luaL_checkint(L, 2);
  int xmax = luaL_checkint(L, 3);
  int ymin = luaL_checkint(L, 4);
  int ymax = luaL_checkint(L, 5);
  wdCanvasViewport(cdlua_checkcanvas(L, 1), xmin, xmax, ymin, ymax);
  return 0;
}

/***************************************************************************\
* wd.GetViewport() -> (xmin, xmax, ymin, ymax: number                       *
\***************************************************************************/
static int wdlua5_getviewport(lua_State *L)
{
  int xmin, xmax, ymin, ymax;

  wdCanvasGetViewport(cdlua_checkcanvas(L, 1), &xmin, &xmax, &ymin, &ymax);
  lua_pushnumber(L, xmin);
  lua_pushnumber(L, xmax);
  lua_pushnumber(L, ymin);
  lua_pushnumber(L, ymax);
  return 4;
}

/***************************************************************************\
* wd.World2Canvas(xw, yw: number) -> (xv, yv: number)                       *
\***************************************************************************/
static int wdlua5_world2canvas(lua_State *L)
{
  double xw_d, yw_d;
  int xv_i, yv_i;

  xw_d = luaL_checknumber(L, 2);
  yw_d = luaL_checknumber(L, 3);

  wdCanvasWorld2Canvas(cdlua_checkcanvas(L, 1), xw_d, yw_d, &xv_i, &yv_i);
  lua_pushnumber(L, xv_i);
  lua_pushnumber(L, yv_i);
  return 2;
}

/***************************************************************************\
* wd.Canvas2World(xv, yv: number) -> (xw, yw: number)                       *
\***************************************************************************/
static int wdlua5_canvas2world(lua_State *L)
{
  int xv_i, yv_i;
  double xw_d, yw_d;

  xv_i = luaL_checkint(L, 2);
  yv_i = luaL_checkint(L, 3);

  wdCanvasCanvas2World(cdlua_checkcanvas(L, 1), xv_i, yv_i, &xw_d, &yw_d);
  lua_pushnumber(L, xw_d);
  lua_pushnumber(L, yw_d);
  return 2;
}



/***************************************************************************\
* General Attributes                                                        *
\***************************************************************************/

/***************************************************************************\
* cd.Foreground(color) -> color                                             *
\***************************************************************************/
static int cdlua5_foreground(lua_State *L)
{
  long int color_i = cdlua_checkcolor(L, 2);
  color_i = cdCanvasForeground(cdlua_checkcanvas(L, 1), color_i);
  lua_pushlightuserdata(L, (void*) color_i);
  return 1;
}

static int cdlua5_setforeground(lua_State *L)
{
  long int color_i = cdlua_checkcolor(L, 2);
  cdCanvasSetForeground(cdlua_checkcanvas(L, 1), color_i);
  return 0;
}

/***************************************************************************\
* cd.Background(color) -> color                                             *
\***************************************************************************/
static int cdlua5_background(lua_State *L)
{
  long int color_i = cdlua_checkcolor(L, 2);
  color_i = cdCanvasBackground(cdlua_checkcanvas(L, 1), color_i);
  lua_pushlightuserdata(L, (void*) color_i);
  return 1;
}

static int cdlua5_setbackground(lua_State *L)
{
  long int color_i = cdlua_checkcolor(L, 2);
  cdCanvasSetBackground(cdlua_checkcanvas(L, 1), color_i);
  return 0;
}
/***************************************************************************\
* cd.WriteMode(mode: number) -> (old_mode: number)                          *
\***************************************************************************/
static int cdlua5_writemode(lua_State *L)
{
  lua_pushnumber(L, cdCanvasWriteMode(cdlua_checkcanvas(L, 1), luaL_checkint(L, 2)));
  return 1;
}



/***************************************************************************\
* Clipping                                                                  *
\***************************************************************************/

/***************************************************************************\
* cd.Clip(mode: number) -> (old_mode: number)                               *
\***************************************************************************/
static int cdlua5_clip(lua_State *L)
{
  lua_pushnumber(L, cdCanvasClip(cdlua_checkcanvas(L, 1), luaL_checkint(L,2)));
  return 1;
}

static int cdlua5_cliparea(lua_State *L)
{
  int xmin = luaL_checkint(L, 2);
  int xmax = luaL_checkint(L, 3);
  int ymin = luaL_checkint(L, 4);
  int ymax = luaL_checkint(L, 5);

  cdCanvasClipArea(cdlua_checkcanvas(L, 1), xmin, xmax, ymin, ymax);
  return 0;
}

static int wdlua5_cliparea(lua_State *L)
{
  double xmin = luaL_checknumber(L, 2);
  double xmax = luaL_checknumber(L, 3);
  double ymin = luaL_checknumber(L, 4);
  double ymax = luaL_checknumber(L, 5);

  wdCanvasClipArea(cdlua_checkcanvas(L, 1), xmin, xmax, ymin, ymax);
  return 0;
}

static int cdlua5_fcliparea(lua_State *L)
{
  double xmin = luaL_checknumber(L, 2);
  double xmax = luaL_checknumber(L, 3);
  double ymin = luaL_checknumber(L, 4);
  double ymax = luaL_checknumber(L, 5);

  cdfCanvasClipArea(cdlua_checkcanvas(L, 1), xmin, xmax, ymin, ymax);
  return 0;
}

static int cdlua5_getcliparea(lua_State *L)
{
  int xmin, xmax, ymin, ymax;
  int status;

  status = cdCanvasGetClipArea(cdlua_checkcanvas(L, 1), &xmin, &xmax, &ymin, &ymax);
  lua_pushnumber(L, xmin);
  lua_pushnumber(L, xmax);
  lua_pushnumber(L, ymin);
  lua_pushnumber(L, ymax);
  lua_pushnumber(L, status);
  return 5;
}

static int wdlua5_getcliparea(lua_State *L)
{
  double xmin, xmax, ymin, ymax;
  int status;

  status = wdCanvasGetClipArea(cdlua_checkcanvas(L, 1), &xmin, &xmax, &ymin, &ymax);
  lua_pushnumber(L, xmin);
  lua_pushnumber(L, xmax);
  lua_pushnumber(L, ymin);
  lua_pushnumber(L, ymax);
  lua_pushnumber(L, status);
  return 5;
}

static int cdlua5_fgetcliparea(lua_State *L)
{
  double xmin, xmax, ymin, ymax;
  int status;

  status = cdfCanvasGetClipArea(cdlua_checkcanvas(L, 1), &xmin, &xmax, &ymin, &ymax);
  lua_pushnumber(L, xmin);
  lua_pushnumber(L, xmax);
  lua_pushnumber(L, ymin);
  lua_pushnumber(L, ymax);
  lua_pushnumber(L, status);
  return 5;
}

/***************************************************************************\
* Regions                                                                   *
\***************************************************************************/

/***************************************************************************\
* cd.RegionCombineMode(mode: number) -> (old_mode: number)                  *
\***************************************************************************/
static int cdlua5_regioncombinemode(lua_State *L)
{
  lua_pushnumber(L, cdCanvasRegionCombineMode(cdlua_checkcanvas(L, 1), luaL_checkint(L, 2)));
  return 1;
}

/***************************************************************************\
* cd.PointInRegion(x, y: number) -> (status: number)                        *
\***************************************************************************/
static int  cdlua5_pointinregion(lua_State *L)
{
  lua_pushboolean(L, cdCanvasIsPointInRegion(cdlua_checkcanvas(L, 1), luaL_checkint(L, 2), luaL_checkint(L, 3)));
  return 1;
}

/***************************************************************************\
* cd.wPointInRegion(x, y: number) -> (status: number)                       *
\***************************************************************************/
static int wdlua5_pointinregion(lua_State *L)
{
  lua_pushboolean(L, wdCanvasIsPointInRegion(cdlua_checkcanvas(L, 1), luaL_checknumber(L, 2), luaL_checknumber(L, 3)));
  return 1;
}

/***************************************************************************\
* cd.OffsetRegion(dx, dy: number)                                           *
\***************************************************************************/
static int cdlua5_offsetregion(lua_State *L)
{
  cdCanvasOffsetRegion(cdlua_checkcanvas(L, 1), luaL_checkint(L, 2), luaL_checkint(L, 3));
  return 0;
}

/***************************************************************************\
* cd.wOffsetRegion(dx, dy: number)                                          *
\***************************************************************************/
static int wdlua5_offsetregion(lua_State *L)
{
  wdCanvasOffsetRegion(cdlua_checkcanvas(L, 1), luaL_checknumber(L, 2), luaL_checknumber(L, 3));
  return 0;
}

/***************************************************************************\
* cd.RegionBox() -> (xmin, xmax, ymin, ymax, status: number)                *
\***************************************************************************/
static int cdlua5_regionbox(lua_State *L)
{
  int xmin, xmax, ymin, ymax;
  
  cdCanvasGetRegionBox(cdlua_checkcanvas(L, 1), &xmin, &xmax, &ymin, &ymax);
  lua_pushnumber(L, xmin);
  lua_pushnumber(L, xmax);
  lua_pushnumber(L, ymin);
  lua_pushnumber(L, ymax);
  return 4;
}

/***************************************************************************\
* cd.wRegionBox() -> (xmin, xmax, ymin, ymax, status: number)               *
\***************************************************************************/
static int wdlua5_regionbox(lua_State *L)
{
  double xmin, xmax, ymin, ymax;

  wdCanvasGetRegionBox(cdlua_checkcanvas(L, 1), &xmin, &xmax, &ymin, &ymax);
  lua_pushnumber(L, xmin);
  lua_pushnumber(L, xmax);
  lua_pushnumber(L, ymin);
  lua_pushnumber(L, ymax);
  return 4;
}


/***************************************************************************\
* Primitives                                                                *
\***************************************************************************/

/***************************************************************************\
* cd.Pixel(x, y: number, color)                                             *
\***************************************************************************/
static int cdlua5_pixel (lua_State *L)
{
  cdCanvasPixel(cdlua_checkcanvas(L, 1), luaL_checkint(L, 2), luaL_checkint(L, 3), cdlua_checkcolor(L, 4));
  return 0 ;
}

/***************************************************************************\
* cd.wPixel(x, y: number, color)                                            *
\***************************************************************************/
static int wdlua5_pixel (lua_State *L)
{
  wdCanvasPixel(cdlua_checkcanvas(L, 1), luaL_checknumber(L, 2), luaL_checknumber(L, 3), cdlua_checkcolor(L, 4));
  return 0;
}

/***************************************************************************\
* cd.Mark(x, y: number)                                                     *
\***************************************************************************/
static int cdlua5_mark(lua_State *L)
{
  cdCanvasMark(cdlua_checkcanvas(L, 1), luaL_checkint(L, 2), luaL_checkint(L, 3));
  return 0;
}

/***************************************************************************\
* cd.wMark(x, y: number)                                                    *
\***************************************************************************/
static int wdlua5_mark(lua_State *L)
{
  wdCanvasMark(cdlua_checkcanvas(L, 1), luaL_checknumber(L, 2), luaL_checknumber(L, 3));
  return 0;
}

/***************************************************************************\
* cd.MarkType(type: number) -> (old_type: number)                           *
\***************************************************************************/
static int cdlua5_marktype(lua_State *L)
{
  lua_pushnumber(L, cdCanvasMarkType(cdlua_checkcanvas(L, 1), luaL_checkint(L, 2)));
  return 1;
}

/***************************************************************************\
* cd.MarkSize(size: number) -> (old_size: number)                           *
\***************************************************************************/
static int cdlua5_marksize(lua_State *L)
{
  lua_pushnumber(L, cdCanvasMarkSize(cdlua_checkcanvas(L, 1), luaL_checkint(L, 2)));
  return 1;
}

/***************************************************************************\
* cd.wMarkSize(size: number) -> (old_size: number)                          *
\***************************************************************************/
static int wdlua5_marksize(lua_State *L)
{
  lua_pushnumber(L, wdCanvasMarkSize(cdlua_checkcanvas(L, 1), luaL_checknumber(L, 2)));
  return 1;
}



/***************************************************************************\
* Lines                                                                     *
\***************************************************************************/

static int cdlua5_line(lua_State *L)
{
  int x1 = luaL_checkint(L,2);
  int y1 = luaL_checkint(L,3);
  int x2 = luaL_checkint(L,4);
  int y2 = luaL_checkint(L,5);
  cdCanvasLine(cdlua_checkcanvas(L, 1), x1, y1, x2, y2);
  return 0;
}

static int wdlua5_line(lua_State *L)
{
  double x1 = luaL_checknumber(L, 2);
  double y1 = luaL_checknumber(L, 3);
  double x2 = luaL_checknumber(L, 4);
  double y2 = luaL_checknumber(L, 5);
  wdCanvasLine(cdlua_checkcanvas(L, 1), x1, y1, x2, y2);
  return 0;
}

static int cdlua5_fline(lua_State *L)
{
  double x1 = luaL_checknumber(L, 2);
  double y1 = luaL_checknumber(L, 3);
  double x2 = luaL_checknumber(L, 4);
  double y2 = luaL_checknumber(L, 5);
  cdfCanvasLine(cdlua_checkcanvas(L, 1), x1, y1, x2, y2);
  return 0;
}

static int cdlua5_rect(lua_State *L)
{
  int xmin = luaL_checkint(L,2);
  int xmax = luaL_checkint(L,3);
  int ymin = luaL_checkint(L,4);
  int ymax = luaL_checkint(L,5);
  cdCanvasRect(cdlua_checkcanvas(L, 1), xmin, xmax, ymin, ymax);
  return 0;
}

static int wdlua5_rect(lua_State *L)
{
  double xmin = luaL_checknumber(L, 2);
  double xmax = luaL_checknumber(L, 3);
  double ymin = luaL_checknumber(L, 4);
  double ymax = luaL_checknumber(L, 5);
  wdCanvasRect(cdlua_checkcanvas(L, 1), xmin,xmax,ymin,ymax);
  return 0;
}

static int cdlua5_frect(lua_State *L)
{
  double xmin = luaL_checknumber(L, 2);
  double xmax = luaL_checknumber(L, 3);
  double ymin = luaL_checknumber(L, 4);
  double ymax = luaL_checknumber(L, 5);
  cdfCanvasRect(cdlua_checkcanvas(L, 1), xmin,xmax,ymin,ymax);
  return 0;
}

static int cdlua5_arc(lua_State *L)
{
  int xc = luaL_checkint(L,2);
  int yc = luaL_checkint(L,3);
  int w = luaL_checkint(L,4);
  int h = luaL_checkint(L,5);
  double angle1 = luaL_checknumber(L,6);
  double angle2 = luaL_checknumber(L,7);
  cdCanvasArc(cdlua_checkcanvas(L, 1), xc, yc, w, h, angle1, angle2);
  return 0;
}

static int wdlua5_arc(lua_State *L)
{
  double xc = luaL_checknumber(L, 2);
  double yc = luaL_checknumber(L, 3);
  double w = luaL_checknumber(L, 4);
  double h = luaL_checknumber(L, 5);
  double angle1 = luaL_checknumber(L, 6);
  double angle2 = luaL_checknumber(L, 7);
  wdCanvasArc(cdlua_checkcanvas(L, 1), xc, yc, w, h, angle1, angle2);
  return 0;
}

static int cdlua5_farc(lua_State *L)
{
  double xc = luaL_checknumber(L, 2);
  double yc = luaL_checknumber(L, 3);
  double w = luaL_checknumber(L, 4);
  double h = luaL_checknumber(L, 5);
  double angle1 = luaL_checknumber(L, 6);
  double angle2 = luaL_checknumber(L, 7);
  cdfCanvasArc(cdlua_checkcanvas(L, 1), xc, yc, w, h, angle1, angle2);
  return 0;
}

/***************************************************************************\
* cd.LineStyle(style: number) -> (old_style: number)                        *
\***************************************************************************/
static int cdlua5_linestyle(lua_State *L)
{
  lua_pushnumber(L, cdCanvasLineStyle(cdlua_checkcanvas(L, 1), luaL_checkint(L, 2)));
  return 1;
}

/***************************************************************************\
* cd.LineStyleDashes(dashes: table, count: number)                          *
\***************************************************************************/
static int cdlua5_linestyledashes(lua_State *L)
{
  int *dashes_int, dashes_count, i;

  if (!lua_istable(L, 2))
    luaL_argerror(L, 2, "invalid dashes, must be a table");

  dashes_count = luaL_checkint(L, 3);
  dashes_int = (int*) malloc(dashes_count * sizeof(int));

  for (i=0; i < dashes_count; i++)
  {
    lua_pushnumber(L, i+1);
    lua_gettable(L, 2);

    dashes_int[i] = luaL_checkint(L,-1);
  }

  cdCanvasLineStyleDashes(cdlua_checkcanvas(L, 1), dashes_int, dashes_count);
  free(dashes_int);

  return 0;
}

/***************************************************************************\
* cd.LineWidth(width: number) -> (old_width: number)                        *
\***************************************************************************/
static int cdlua5_linewidth(lua_State *L)
{
  lua_pushnumber(L, cdCanvasLineWidth(cdlua_checkcanvas(L, 1), luaL_checkint(L, 2)));
  return 1;
}

/***************************************************************************\
* cd.wLineWidth(width: number) -> (old_width: number)                       *
\***************************************************************************/
static int wdlua5_linewidth(lua_State *L)
{
  lua_pushnumber(L, wdCanvasLineWidth(cdlua_checkcanvas(L, 1), luaL_checknumber(L, 2)));
  return 1;
}

/***************************************************************************\
* cd.LineJoin(style: number) -> (old_style: number)                         *
\***************************************************************************/
static int cdlua5_linejoin(lua_State *L)
{
  lua_pushnumber(L, cdCanvasLineJoin(cdlua_checkcanvas(L, 1), luaL_checkint(L, 2)));
  return 1;
}

/***************************************************************************\
* cd.LineCap(style: number) -> (old_style: number)                          *
\***************************************************************************/
static int cdlua5_linecap(lua_State *L)
{
  lua_pushnumber(L, cdCanvasLineCap(cdlua_checkcanvas(L, 1), luaL_checkint(L, 2)));
  return 1;
}



/***************************************************************************\
* Filled Areas                                                              *
\***************************************************************************/

static int cdlua5_box(lua_State *L)
{
  int xmin = luaL_checkint(L, 2);
  int xmax = luaL_checkint(L, 3);
  int ymin = luaL_checkint(L, 4);
  int ymax = luaL_checkint(L, 5);
  cdCanvasBox(cdlua_checkcanvas(L, 1), xmin, xmax, ymin, ymax);
  return 0;
}

static int wdlua5_box(lua_State *L)
{
  double xmin = luaL_checknumber(L, 2);
  double xmax = luaL_checknumber(L, 3);
  double ymin = luaL_checknumber(L, 4);
  double ymax = luaL_checknumber(L, 5);
  wdCanvasBox(cdlua_checkcanvas(L, 1), xmin, xmax, ymin, ymax);
  return 0;
}

static int cdlua5_fbox(lua_State *L)
{
  double xmin = luaL_checknumber(L, 2);
  double xmax = luaL_checknumber(L, 3);
  double ymin = luaL_checknumber(L, 4);
  double ymax = luaL_checknumber(L, 5);
  cdfCanvasBox(cdlua_checkcanvas(L, 1), xmin, xmax, ymin, ymax);
  return 0;
}

static int cdlua5_sector(lua_State *L)
{
  int xc = luaL_checkint(L,2);
  int yc = luaL_checkint(L,3);
  int w = luaL_checkint(L,4);
  int h = luaL_checkint(L,5);
  double angle1 = luaL_checknumber(L,6);
  double angle2 = luaL_checknumber(L,7);
  cdCanvasSector(cdlua_checkcanvas(L, 1), xc, yc, w, h, angle1, angle2);
  return 0;
}

static int wdlua5_sector(lua_State *L)
{
  double xc = luaL_checknumber(L, 2);
  double yc = luaL_checknumber(L, 3);
  double w = luaL_checknumber(L, 4);
  double h = luaL_checknumber(L, 5);
  double angle1 = luaL_checknumber(L, 6);
  double angle2 = luaL_checknumber(L, 7);
  wdCanvasSector(cdlua_checkcanvas(L, 1), xc, yc, w, h, angle1, angle2);
  return 0;
}

static int cdlua5_fsector(lua_State *L)
{
  double xc = luaL_checknumber(L, 2);
  double yc = luaL_checknumber(L, 3);
  double w = luaL_checknumber(L, 4);
  double h = luaL_checknumber(L, 5);
  double angle1 = luaL_checknumber(L, 6);
  double angle2 = luaL_checknumber(L, 7);
  cdfCanvasSector(cdlua_checkcanvas(L, 1), xc, yc, w, h, angle1, angle2);
  return 0;
}

static int cdlua5_chord(lua_State *L)
{
  int xc = luaL_checkint(L,2);
  int yc = luaL_checkint(L,3);
  int w = luaL_checkint(L,4);
  int h = luaL_checkint(L,5);
  double angle1 = luaL_checknumber(L,6);
  double angle2 = luaL_checknumber(L,7);
  cdCanvasChord(cdlua_checkcanvas(L, 1), xc, yc, w, h, angle1, angle2);
  return 0;
}

static int wdlua5_chord(lua_State *L)
{
  double xc = luaL_checknumber(L, 2);
  double yc = luaL_checknumber(L, 3);
  double w = luaL_checknumber(L, 4);
  double h = luaL_checknumber(L, 5);
  double angle1 = luaL_checknumber(L, 6);
  double angle2 = luaL_checknumber(L, 7);
  wdCanvasChord(cdlua_checkcanvas(L, 1), xc, yc, w, h, angle1, angle2);
  return 0;
}

static int cdlua5_fchord(lua_State *L)
{
  double xc = luaL_checknumber(L, 2);
  double yc = luaL_checknumber(L, 3);
  double w = luaL_checknumber(L, 4);
  double h = luaL_checknumber(L, 5);
  double angle1 = luaL_checknumber(L, 6);
  double angle2 = luaL_checknumber(L, 7);
  cdfCanvasChord(cdlua_checkcanvas(L, 1), xc, yc, w, h, angle1, angle2);
  return 0;
}

/***************************************************************************\
* cd.BackOpacity(opacity: number) -> (old_opacity: number)                  *
\***************************************************************************/
static int cdlua5_backopacity(lua_State *L)
{
  lua_pushnumber(L, cdCanvasBackOpacity(cdlua_checkcanvas(L, 1), luaL_checkint(L, 2)));
  return 1;
}

/***************************************************************************\
* cd.FillMode(mode: number) -> (old_mode: number)                           *
\***************************************************************************/
static int cdlua5_fillmode(lua_State *L)
{
  lua_pushnumber(L, cdCanvasFillMode(cdlua_checkcanvas(L, 1), luaL_checkint(L, 2)));
  return 1;
}

/***************************************************************************\
* cd.InteriorStyle(style: number) -> (old_style: number)                    *
\***************************************************************************/
static int cdlua5_interiorstyle(lua_State *L)
{
  lua_pushnumber(L, cdCanvasInteriorStyle(cdlua_checkcanvas(L, 1), luaL_checkint(L, 2)));
  return 1;
}

/***************************************************************************\
* cd.Hatch(style: number) -> (old_style: number)                            *
\***************************************************************************/
static int cdlua5_hatch(lua_State *L)
{
  lua_pushnumber(L, cdCanvasHatch(cdlua_checkcanvas(L, 1), luaL_checkint(L, 2)));
  return 1;
}

static int cdlua5_stipple(lua_State *L)
{
  cdluaStipple *stipple_p = cdlua_checkstipple(L, 2);
  cdCanvasStipple(cdlua_checkcanvas(L, 1), stipple_p->width, stipple_p->height, stipple_p->stipple);
  return 0 ;
}

static int wdlua5_stipple(lua_State *L)
{
  cdluaStipple *stipple_p = cdlua_checkstipple(L, 2);
  double w_mm = luaL_checknumber(L, 3);
  double h_mm = luaL_checknumber(L, 4);
  wdCanvasStipple(cdlua_checkcanvas(L, 1), stipple_p->width, stipple_p->height, stipple_p->stipple, w_mm, h_mm);
  return 0;
}

static int cdlua5_getstipple(lua_State *L)
{
  int width, height, size;
  unsigned char *stipple, *new_stipple = NULL;

  stipple = cdCanvasGetStipple(cdlua_checkcanvas(L, 1), &width, &height);

  size = width * height;

  if (stipple)
    new_stipple = (unsigned char *)malloc(size);

  if (new_stipple) 
  {
    memcpy(new_stipple, stipple, size);  
    cdlua_pushstipple(L, new_stipple, width, height);
  }
  else
    lua_pushnil(L);

  return 1;
}

static int cdlua5_pattern(lua_State *L)
{
  cdluaPattern* pattern_p = cdlua_checkpattern(L, 2);
  cdCanvasPattern(cdlua_checkcanvas(L, 1), pattern_p->width, pattern_p->height, pattern_p->pattern);
  return 0;
}

static int wdlua5_pattern(lua_State *L)
{
  cdluaPattern* pattern_p = cdlua_checkpattern(L, 2);
  double w_mm = luaL_checknumber(L, 3);
  double h_mm = luaL_checknumber(L, 4);
  wdCanvasPattern(cdlua_checkcanvas(L, 1), pattern_p->width, pattern_p->height, pattern_p->pattern, w_mm, h_mm);
  return 0;
}

static int cdlua5_getpattern(lua_State *L)
{
  int width, height, size;
  long int *pattern, *new_pattern = NULL;

  pattern = cdCanvasGetPattern(cdlua_checkcanvas(L, 1), &width, &height);

  size = width * height;

  if (pattern)
    new_pattern = (long int *) malloc(size * sizeof(long int));

  if (new_pattern)
  {
    memcpy(new_pattern, pattern, size * sizeof(long int));
    cdlua_pushpattern(L, new_pattern, width, height);
  }
  else
    lua_pushnil(L);

  return 1;
}

/***************************************************************************\
* Text                                                                      *
\***************************************************************************/

static int cdlua5_text(lua_State *L)
{
  const char* s = luaL_checkstring(L, 4);
  cdCanvasText(cdlua_checkcanvas(L, 1), luaL_checkint(L, 2), luaL_checkint(L, 3), s);
  return 0;
}

static int wdlua5_text(lua_State *L)
{
  const char* s = luaL_checkstring(L, 4);
  wdCanvasText(cdlua_checkcanvas(L, 1), luaL_checknumber(L, 2), luaL_checknumber(L, 3), s);
  return 0;
}

static int cdlua5_ftext(lua_State *L)
{
  const char* s = luaL_checkstring(L, 4);
  cdfCanvasText(cdlua_checkcanvas(L, 1), luaL_checknumber(L, 2), luaL_checknumber(L, 3), s);
  return 0;
}

/***************************************************************************\
* cd.Font(typeface, style, size: number)                                    *
\***************************************************************************/
static int cdlua5_font(lua_State *L)
{
  lua_pushnumber(L, cdCanvasFont(cdlua_checkcanvas(L, 1), luaL_checkstring(L, 2), luaL_checkint(L, 3), luaL_checkint(L, 4)));
  return 1;
}

/***************************************************************************\
* cd.wFont(typeface, style, size: number)                                   *
\***************************************************************************/
static int wdlua5_font(lua_State *L)
{
  lua_pushnumber(L, wdCanvasFont(cdlua_checkcanvas(L, 1), luaL_checkstring(L, 2), luaL_checkint(L, 3), luaL_checknumber(L, 4)));
  return 1;
}


/***************************************************************************\
* cd.GetFont() -> (typeface, style, size: number)                           *
\***************************************************************************/
static int cdlua5_getfont(lua_State *L)
{
  char type_face[1024];
  int style, size;

  cdCanvasGetFont(cdlua_checkcanvas(L, 1), type_face, &style, &size);
  lua_pushstring(L, type_face);
  lua_pushnumber(L, style);
  lua_pushnumber(L, size);
  return 3;
}

/***************************************************************************\
* cd.wGetFont() -> (typeface, style, size: number)                          *
\***************************************************************************/
static int wdlua5_getfont(lua_State *L)
{
  char type_face[1024];
  int style;
  double size;

  wdCanvasGetFont(cdlua_checkcanvas(L, 1), type_face, &style, &size);
  lua_pushstring(L, type_face);
  lua_pushnumber(L, style);
  lua_pushnumber(L, size);
  return 3;
}

/***************************************************************************\
* cd.NativeFont(font: string)                                               *
\***************************************************************************/
static int cdlua5_nativefont(lua_State *L)
{
  lua_pushstring(L, cdCanvasNativeFont(cdlua_checkcanvas(L, 1), luaL_checkstring(L, 2)));
  return 1;
}

/***************************************************************************\
* cd.TextAlignment(alignment: number) -> (old_alignment: number)            *
\***************************************************************************/
static int cdlua5_textalignment(lua_State *L)
{
  lua_pushnumber(L, cdCanvasTextAlignment(cdlua_checkcanvas(L, 1), luaL_checkint(L, 2)));
  return 1;
}

/***************************************************************************\
* cd.TextOrientation(angle: number) -> (old_angle: number)                  *
\***************************************************************************/
static int cdlua5_textorientation(lua_State *L)
{
  lua_pushnumber(L, cdCanvasTextOrientation(cdlua_checkcanvas(L, 1), luaL_checknumber(L, 2)));
  return 1;
}

/***************************************************************************\
* cd.GetFontDim() -> (max_width, max_height, ascent, descent: number)          *
\***************************************************************************/
static int cdlua5_getfontdim(lua_State *L)
{
  int max_width;
  int height;
  int ascent;
  int descent;

  cdCanvasGetFontDim(cdlua_checkcanvas(L, 1), &max_width, &height, &ascent, &descent);
  lua_pushnumber(L, max_width);
  lua_pushnumber(L, height);
  lua_pushnumber(L, ascent);
  lua_pushnumber(L, descent);
  return 4;
}

/***************************************************************************\
* cd.wGetFontDim() -> (max_width, max_height, ascent, descent: number)         *
\***************************************************************************/
static int wdlua5_getfontdim(lua_State *L)
{
  double max_width;
  double height;
  double ascent;
  double descent;

  wdCanvasGetFontDim(cdlua_checkcanvas(L, 1), &max_width, &height, &ascent, &descent);
  lua_pushnumber(L, max_width);
  lua_pushnumber(L, height);
  lua_pushnumber(L, ascent);
  lua_pushnumber(L, descent);
  return 4;
}

/***************************************************************************\
* cd.GetTextSize(text: string) -> (width, heigth: number)                      *
\***************************************************************************/
static int cdlua5_gettextsize(lua_State *L)
{
  int width;
  int height;
  cdCanvasGetTextSize(cdlua_checkcanvas(L, 1), luaL_checkstring(L, 2), &width, &height);
  lua_pushnumber(L, width);
  lua_pushnumber(L, height);
  return 2;
}

/***************************************************************************\
* cd.wGetTextSize(text: string) -> (width, heigth: number)                     *
\***************************************************************************/
static int wdlua5_gettextsize(lua_State *L)
{
  double width;
  double height;
  wdCanvasGetTextSize(cdlua_checkcanvas(L, 1), luaL_checkstring(L, 2), &width, &height);
  lua_pushnumber(L, width);
  lua_pushnumber(L, height);
  return 2;
}

/****************************************************************************\
* cd.GetTextBox(x, y: number, text: string) -> (xmin, xmax, ymin, ymax: number) *
\****************************************************************************/
static int cdlua5_gettextbox(lua_State *L)
{
  int xmin, xmax, ymin, ymax;
  int x = luaL_checkint(L, 2);
  int y = luaL_checkint(L, 3);
  const char* s = luaL_checkstring(L, 4);

  cdCanvasGetTextBox(cdlua_checkcanvas(L, 1), x, y, s, &xmin, &xmax, &ymin, &ymax);
  lua_pushnumber(L, xmin);
  lua_pushnumber(L, xmax);
  lua_pushnumber(L, ymin);
  lua_pushnumber(L, ymax);
  return 4;
}

/*****************************************************************************\
* cd.wGetTextBox(x, y: number, text: string) -> (xmin, xmax, ymin, ymax: number) *
\*****************************************************************************/
static int wdlua5_gettextbox(lua_State *L)
{
  double xmin, xmax, ymin, ymax;
  double x = luaL_checknumber(L, 2);
  double y =  luaL_checknumber(L, 3);
  const char* s = luaL_checkstring(L, 4);

  wdCanvasGetTextBox(cdlua_checkcanvas(L, 1), x, y, s, &xmin, &xmax, &ymin, &ymax);
  lua_pushnumber(L, xmin);
  lua_pushnumber(L, xmax);
  lua_pushnumber(L, ymin);
  lua_pushnumber(L, ymax);
  return 4;
}

/***************************************************************************************************************\
* cd.GetTextBounds(x, y: number, text: string) -> (rect: table) *
\***************************************************************************************************************/
static int cdlua5_gettextbounds(lua_State *L)
{
  int rect[8];
  int x = luaL_checkint(L, 2);
  int y = luaL_checkint(L, 3);
  const char* s = luaL_checkstring(L, 4);
  int i;

  cdCanvasGetTextBounds(cdlua_checkcanvas(L, 1), x, y, s, rect);
  lua_createtable(L, 8, 0);
  for (i=0; i < 8; i++)
  {
    lua_pushnumber(L, rect[i]);
    lua_rawseti(L, -2, i+1);
  }
  return 1;
}

/****************************************************************************************************************\
* cd.wGetTextBounds(x, y: number, text: string) -> (rect: table) *
\****************************************************************************************************************/
static int wdlua5_gettextbounds(lua_State *L)
{
  double rect[8];
  double x = luaL_checknumber(L, 2);
  double y = luaL_checknumber(L, 3);
  const char* s = luaL_checkstring(L, 4);
  int i;

  wdCanvasGetTextBounds(cdlua_checkcanvas(L, 1), x, y, s, rect);
  lua_createtable(L, 8, 0);
  for (i=0; i < 8; i++)
  {
    lua_pushnumber(L, rect[i]);
    lua_rawseti(L, -2, i+1);
  }
  return 1;
}



/***************************************************************************\
* Text                                                                      *
\***************************************************************************/

/***************************************************************************\
* cd.VectorText(x, y: number, text: string)                                 *
\***************************************************************************/
static int cdlua5_vectortext(lua_State *L)
{
  const char* s = luaL_checkstring(L,4);
  cdCanvasVectorText(cdlua_checkcanvas(L, 1), luaL_checkint(L, 2), luaL_checkint(L, 3), s);
  return 0;
}

/***************************************************************************\
* cd.wVectorText(x, y: number, text: string)                                *
\***************************************************************************/
static int wdlua5_vectortext(lua_State *L)
{
  const char* s = luaL_checkstring(L, 4);
  wdCanvasVectorText(cdlua_checkcanvas(L, 1), luaL_checknumber(L, 2), luaL_checknumber(L, 3),s);
  return 0;
}

/***************************************************************************\
* cd.MultiLineVectorText(x, y: number, text: string)                        *
\***************************************************************************/
static int cdlua5_multilinevectortext(lua_State *L)
{
  const char* s = luaL_checkstring(L, 4);
  cdCanvasMultiLineVectorText(cdlua_checkcanvas(L, 1), luaL_checkint(L, 2), luaL_checkint(L, 3), s);
  return 0;
}

/***************************************************************************\
* cd.wMultiLineVectorText(x, y: number, text: string)                       *
\***************************************************************************/
static int wdlua5_multilinevectortext(lua_State *L)
{
  const char* s = luaL_checkstring(L, 4);
  wdCanvasMultiLineVectorText(cdlua_checkcanvas(L, 1), luaL_checknumber(L, 2), luaL_checknumber(L, 3), s);
  return 0;
}

/***************************************************************************\
* cd.VectorTextDirection(x1, y1, x2, y2: number)                            *
\***************************************************************************/
static int cdlua5_vectortextdirection(lua_State *L)
{
  int x1 = luaL_checkint(L,2);
  int y1 = luaL_checkint(L,3);
  int x2 = luaL_checkint(L,4);
  int y2 = luaL_checkint(L,5);
  cdCanvasVectorTextDirection(cdlua_checkcanvas(L, 1), x1, y1, x2, y2);
  return 0;
}

/***************************************************************************\
* cd.wVectorTextDirection(x1, y1, x2, y2: number)                           *
\***************************************************************************/
static int wdlua5_vectortextdirection(lua_State *L)
{
  double x1 = luaL_checknumber(L, 2);
  double y1 = luaL_checknumber(L, 3);
  double x2 = luaL_checknumber(L, 4);
  double y2 = luaL_checknumber(L, 5);
  wdCanvasVectorTextDirection(cdlua_checkcanvas(L, 1), x1, y1, x2, y2);
  return 0;
}


/***************************************************************************\
* cd.VectorTextTransform(matrix: table) -> (old_matrix: table)              *
\***************************************************************************/
static int cdlua5_vectortexttransform(lua_State *L)
{
  double matrix[6], *old_matrix;
  int i;

  if (!lua_istable(L, 2))
    luaL_argerror(L, 2, "invalid matrix, must be a table");

  for (i=0; i < 6; i++)
  {
    lua_rawgeti(L, 2, i+1);

    if (!lua_isnumber(L, -1))
      luaL_argerror(L, 2, "invalid matrix value, must be a number");

    matrix[i] = lua_tonumber(L, -1);
    lua_pop(L, 1);
  }

  old_matrix = cdCanvasVectorTextTransform(cdlua_checkcanvas(L, 1), matrix);
  lua_createtable(L, 6, 0);
  for (i=0; i < 6; i++)
  {
    lua_pushnumber(L, old_matrix[i]);
    lua_rawseti(L, 1, i+1);
  }
  return 1;
}

/***************************************************************************\
* cd.VectorFontSize(w, h: number, text: string)                             *
\***************************************************************************/
static int cdlua5_vectorfontsize(lua_State *L)
{
  cdCanvasVectorFontSize(cdlua_checkcanvas(L, 1), luaL_checknumber(L, 2), luaL_checknumber(L, 3));
  return 0;
}

/***************************************************************************\
* cd.GetVectorFontSize(text: string) -> (w, h: number)                     *
\***************************************************************************/
static int cdlua5_getvectorfontsize(lua_State *L)
{
  double width;
  double height;
  cdCanvasGetVectorFontSize(cdlua_checkcanvas(L, 1), &width, &height);
  lua_pushnumber(L, width);
  lua_pushnumber(L, height);
  return 2;
}

/***************************************************************************\
* cd.VectorTextSize(w, h: number, text: string)                             *
\***************************************************************************/
static int cdlua5_vectortextsize(lua_State *L)
{
  const char* s = luaL_checkstring(L, 4);
  cdCanvasVectorTextSize(cdlua_checkcanvas(L, 1), luaL_checkint(L, 2), luaL_checkint(L, 3), s);
  return 0;
}

/***************************************************************************\
* cd.wVectorTextSize(w, h: number, text: string)                            *
\***************************************************************************/
static int wdlua5_vectortextsize(lua_State *L)
{
  const char* s = luaL_checkstring(L, 4);
  wdCanvasVectorTextSize(cdlua_checkcanvas(L, 1), luaL_checknumber(L, 2), luaL_checknumber(L, 3), s);
  return 0;
}

/***************************************************************************\
* cd.VectorTextSize(w, h: number, text: string)                             *
\***************************************************************************/
static int cdlua5_vectorcharsize(lua_State *L)
{
  lua_pushnumber(L, cdCanvasVectorCharSize(cdlua_checkcanvas(L, 1), luaL_checkint(L, 2)));
  return 1;
}

/***************************************************************************\
* cd.wVectorTextSize(w, h: number, text: string)                            *
\***************************************************************************/
static int wdlua5_vectorcharsize(lua_State *L)
{
  lua_pushnumber(L, wdCanvasVectorCharSize(cdlua_checkcanvas(L, 1), luaL_checknumber(L, 2)));
  return 1;
}

/***************************************************************************\
* cd.VectorFont(filename: string) -> (font_name: string)                    *
\***************************************************************************/
static int cdlua5_vectorfont(lua_State *L)
{
  lua_pushstring(L, cdCanvasVectorFont(cdlua_checkcanvas(L, 1), luaL_checkstring(L, 2)));
  return 1;
}

/***************************************************************************\
* cd.GetVectorTextSize(text: string) -> (w, h: number)                      *
\***************************************************************************/
static int cdlua5_getvectortextsize(lua_State *L)
{
  int width;
  int height;
  cdCanvasGetVectorTextSize(cdlua_checkcanvas(L, 1), luaL_checkstring(L, 2), &width, &height);
  lua_pushnumber(L, width);
  lua_pushnumber(L, height);   
  return 2;
}

/***************************************************************************\
* cd.wGetVectorTextSize(text: string) -> (w, h: number)                     *
\***************************************************************************/
static int wdlua5_getvectortextsize(lua_State *L)
{
  double width;
  double height;
  wdCanvasGetVectorTextSize(cdlua_checkcanvas(L, 1), luaL_checkstring(L, 2), &width, &height);
  lua_pushnumber(L, width);
  lua_pushnumber(L, height);
  return 2;
}

/***************************************************************************\
* cd.GetVectorTextBounds(s: string, px,py: number) -> (rect: table)         *
\***************************************************************************/
static int cdlua5_getvectortextbounds(lua_State *L)
{
  const char* s = luaL_checkstring(L, 2);
  int x = luaL_checkint(L, 3);
  int y = luaL_checkint(L, 4);
  int rect[8], i;

  cdCanvasGetVectorTextBounds(cdlua_checkcanvas(L, 1), s, x, y, rect);
  lua_createtable(L, 8, 0);
  for (i=0; i < 8; i++)
  {
    lua_pushnumber(L, rect[i]);
    lua_rawseti(L, -2, i+1);
  }
  return 1;
}

/***************************************************************************\
* cd.wGetVectorTextBounds(s: string, px,py: number) -> (rect: table)        *
\***************************************************************************/
static int wdlua5_getvectortextbounds(lua_State *L)
{
  const char* s = luaL_checkstring(L, 2);
  double x = luaL_checknumber(L, 3);
  double y = luaL_checknumber(L, 4);
  double rect[8];
  int i;
  
  wdCanvasGetVectorTextBounds(cdlua_checkcanvas(L, 1), s, x, y, rect);
  lua_createtable(L, 8, 0);
  for (i=0; i < 8; i++)
  {
    lua_pushnumber(L, rect[i]);
    lua_rawseti(L, -2, i+1);
  }
  return 1;
}

/****************************************************************************\
* cd.GetVectorTextBox(x, y: number, text: string) -> (xmin, xmax, ymin, ymax: number) *
\****************************************************************************/
static int cdlua5_getvectortextbox(lua_State *L)
{
  int xmin, xmax, ymin, ymax;
  int x = luaL_checkint(L, 2);
  int y = luaL_checkint(L, 3);
  const char* s = luaL_checkstring(L, 4);

  cdCanvasGetVectorTextBox(cdlua_checkcanvas(L, 1), x, y, s, &xmin, &xmax, &ymin, &ymax);
  lua_pushnumber(L, xmin);
  lua_pushnumber(L, xmax);
  lua_pushnumber(L, ymin);
  lua_pushnumber(L, ymax);
  return 4;
}

/*****************************************************************************\
* cd.wGetVectorTextBox(x, y: number, text: string) -> (xmin, xmax, ymin, ymax: number) *
\*****************************************************************************/
static int wdlua5_getvectortextbox(lua_State *L)
{
  double xmin, xmax, ymin, ymax;
  double x = luaL_checknumber(L, 2);
  double y =  luaL_checknumber(L, 3);
  const char* s = luaL_checkstring(L, 4);

  wdCanvasGetVectorTextBox(cdlua_checkcanvas(L, 1), x, y, s, &xmin, &xmax, &ymin, &ymax);
  lua_pushnumber(L, xmin);
  lua_pushnumber(L, xmax);
  lua_pushnumber(L, ymin);
  lua_pushnumber(L, ymax);
  return 4;
}



/***************************************************************************\
* Client Images.                                                            *
\***************************************************************************/

static int cdlua5_getimagergb(lua_State *L)
{
  cdluaImageRGB* imagergb_p = cdlua_checkimagergb(L, 2);
  int x = luaL_checkint(L, 3);
  int y = luaL_checkint(L, 4);
  cdCanvasGetImageRGB(cdlua_checkcanvas(L, 1), imagergb_p->red, imagergb_p->green, imagergb_p->blue,
                      x, y, imagergb_p->width, imagergb_p->height);
  return 0;
}

static int cdlua5_putimagerectrgb(lua_State *L)
{
  cdluaImageRGB* imagergb_p = cdlua_checkimagergb(L, 2);
  int x = luaL_checkint(L, 3);
  int y = luaL_checkint(L, 4);
  int w = luaL_checkint(L, 5);
  int h = luaL_checkint(L, 6);
  int xmin = luaL_checkint(L, 7);
  int xmax = luaL_checkint(L, 8);
  int ymin = luaL_checkint(L, 9);
  int ymax = luaL_checkint(L, 10);

  if (w < 0 || h < 0)
    luaL_argerror(L, 5, "target region dimensions should be positive integers");
  
  cdCanvasPutImageRectRGB(cdlua_checkcanvas(L, 1), imagergb_p->width, imagergb_p->height, imagergb_p->red, 
    imagergb_p->green, imagergb_p->blue, x, y, w, h, xmin, xmax, ymin, ymax);
  return 0;
}

static int wdlua5_putimagerectrgb(lua_State *L)
{
  cdluaImageRGB* imagergb_p = cdlua_checkimagergb(L, 2);
  double x = luaL_checknumber(L, 3);
  double y = luaL_checknumber(L, 4);
  double w = luaL_checknumber(L, 5);
  double h = luaL_checknumber(L, 6);
  int xmin = luaL_checkint(L, 7);
  int xmax = luaL_checkint(L, 8);
  int ymin = luaL_checkint(L, 9);
  int ymax = luaL_checkint(L, 10);

  if (w < 0 || h < 0)
    luaL_argerror(L, 5, "target region dimensions should be positive integers");
  
  wdCanvasPutImageRectRGB(cdlua_checkcanvas(L, 1), imagergb_p->width, imagergb_p->height, imagergb_p->red, 
                          imagergb_p->green, imagergb_p->blue, x, y, w, h, xmin, xmax, ymin, ymax);
  return 0;
}

static int cdlua5_putimagerectrgba(lua_State *L)
{
  cdluaImageRGBA* imagergba_p = cdlua_checkimagergba(L, 2);
  int x = luaL_checkint(L, 3);
  int y = luaL_checkint(L, 4);
  int w = luaL_checkint(L, 5);
  int h = luaL_checkint(L, 6);
  int xmin = luaL_checkint(L, 7);
  int xmax = luaL_checkint(L, 8);
  int ymin = luaL_checkint(L, 9);
  int ymax = luaL_checkint(L, 10);

  if (w < 0 || h < 0)
    luaL_argerror(L, 5, "target region dimensions should be positive integers");

  cdCanvasPutImageRectRGBA(cdlua_checkcanvas(L, 1), imagergba_p->width, imagergba_p->height, imagergba_p->red, 
                           imagergba_p->green, imagergba_p->blue, imagergba_p->alpha, x, y, w, h, xmin, xmax, ymin, ymax);
  return 0;
}

static int wdlua5_putimagerectrgba(lua_State *L)
{
  cdluaImageRGBA* imagergba_p = cdlua_checkimagergba(L, 2);
  double x = luaL_checknumber(L, 3);
  double y = luaL_checknumber(L, 4);
  double w = luaL_checknumber(L, 5);
  double h = luaL_checknumber(L, 6);
  int xmin = luaL_checkint(L, 7);
  int xmax = luaL_checkint(L, 8);
  int ymin = luaL_checkint(L, 9);
  int ymax = luaL_checkint(L, 10);

  if (w < 0 || h < 0)
    luaL_argerror(L, 5, "target region dimensions should be positive integers");

  wdCanvasPutImageRectRGBA(cdlua_checkcanvas(L, 1), imagergba_p->width, imagergba_p->height, imagergba_p->red, 
                           imagergba_p->green, imagergba_p->blue, imagergba_p->alpha, x, y, w, h, xmin, xmax, ymin, ymax);
  return 0;
}

static int cdlua5_putimagerectmap(lua_State *L)
{
  cdluaImageMap* imagemap_p = cdlua_checkimagemap(L, 2);
  cdluaPalette *pal = cdlua_checkpalette(L, 3);
  int x = luaL_checkint(L, 4);
  int y = luaL_checkint(L, 5);
  int w = luaL_checkint(L, 6);
  int h = luaL_checkint(L, 7);
  int xmin = luaL_checkint(L, 8);
  int xmax = luaL_checkint(L, 9);
  int ymin = luaL_checkint(L, 10);
  int ymax = luaL_checkint(L, 11);

  if (w < 0 || h < 0)
    luaL_argerror(L, 6, "target region dimensions should be positive integers");
  
  cdCanvasPutImageRectMap(cdlua_checkcanvas(L, 1), imagemap_p->width, imagemap_p->height, imagemap_p->index, 
                          pal->color, x, y, w, h, xmin, xmax, ymin, ymax);
  return 0;
}

static int wdlua5_putimagerectmap(lua_State *L)
{
  cdluaImageMap* imagemap_p = cdlua_checkimagemap(L, 2);
  cdluaPalette *pal = cdlua_checkpalette(L, 3);
  double x = luaL_checknumber(L, 4);
  double y = luaL_checknumber(L, 5);
  double w = luaL_checknumber(L, 6);
  double h = luaL_checknumber(L, 7);
  int xmin = luaL_checkint(L, 8);
  int xmax = luaL_checkint(L, 9);
  int ymin = luaL_checkint(L, 10);
  int ymax = luaL_checkint(L, 11);

  if (w < 0 || h < 0)
    luaL_argerror(L, 6, "target region dimensions should be positive integers");
  
  wdCanvasPutImageRectMap(cdlua_checkcanvas(L, 1), imagemap_p->width, imagemap_p->height, imagemap_p->index, 
                          pal->color, x, y, w, h, xmin, xmax, ymin, ymax);
  return 0;
}

static int cdlua5_putbitmap(lua_State *L)
{
  cdBitmap *bitmap = cdlua_checkbitmap(L, 2);
  int x = luaL_checkint(L, 3);
  int y = luaL_checkint(L, 4);
  int w = luaL_checkint(L, 5);
  int h = luaL_checkint(L, 6);
  
  if (w < 0 || h < 0)
    luaL_argerror(L, 5, "target region dimensions should be positive integers");

  cdCanvasPutBitmap(cdlua_checkcanvas(L, 1), bitmap, x, y, w, h);
  return 0;
}

static int wdlua5_putbitmap(lua_State *L)
{
  cdBitmap *bitmap = cdlua_checkbitmap(L, 2);
  double x = luaL_checknumber(L, 3);
  double y = luaL_checknumber(L, 4);
  double w = luaL_checknumber(L, 5);
  double h = luaL_checknumber(L, 6);

  if (w < 0 || h < 0)
    luaL_argerror(L, 5, "target region dimensions should be positive integers");

  wdCanvasPutBitmap(cdlua_checkcanvas(L, 1), bitmap, x, y, w, h);
  return 0;
}

static int cdlua5_getbitmap(lua_State *L)
{
  cdBitmap *bitmap = cdlua_checkbitmap(L, 2);
  int x = luaL_checkint(L, 3);
  int y = luaL_checkint(L, 4);
  cdCanvasGetBitmap(cdlua_checkcanvas(L, 1), bitmap, x, y);
  return 0;
}

/***************************************************************************\
* Server Images.                                                            *
\***************************************************************************/

static int cdlua5_createimage(lua_State *L)
{
  cdImage *image;
  int width = luaL_checkint(L, 2);
  int height = luaL_checkint(L, 3);

  if (width < 1 || height < 1)
    luaL_argerror(L, 2, "image dimensions should be positive integers");

  image = cdCanvasCreateImage(cdlua_checkcanvas(L, 1), width, height);
  if (image) 
    cdlua_pushimage(L, image);
  else
    lua_pushnil(L);

  return 1;
}

static int cdlua5_getimage(lua_State *L)
{
  cdImage* image = cdlua_checkimage(L, 2);
  int x = luaL_checkint(L, 3);
  int y = luaL_checkint(L, 4);
  cdCanvasGetImage(cdlua_checkcanvas(L, 1), image, x, y);
  return 0;
}

static int cdlua5_putimagerect(lua_State *L)
{
  cdImage* image = cdlua_checkimage(L, 2);
  int x = luaL_checkint(L, 3);
  int y = luaL_checkint(L, 4);
  int xmin = luaL_checkint(L, 5);
  int xmax = luaL_checkint(L, 6);
  int ymin = luaL_checkint(L, 7);
  int ymax = luaL_checkint(L, 8);
  cdCanvasPutImageRect(cdlua_checkcanvas(L, 1), image, x, y, xmin, xmax, ymin, ymax);
  return 0;
}

static int wdlua5_putimagerect(lua_State *L)
{
  cdImage* image = cdlua_checkimage(L, 2);
  double x = luaL_checknumber(L, 3);
  double y = luaL_checknumber(L, 4);
  int xmin = luaL_checkint(L, 5);
  int xmax = luaL_checkint(L, 6);
  int ymin = luaL_checkint(L, 7);
  int ymax = luaL_checkint(L, 8);
  wdCanvasPutImageRect(cdlua_checkcanvas(L, 1), image, x, y, xmin, xmax, ymin, ymax);
  return 0;
}

/***************************************************************************\
* cd.ScrollArea(xmin, xmax, ymin, ymax, dx, dy: number)                     *
\***************************************************************************/
static int cdlua5_scrollarea(lua_State *L)
{
  int xmin = luaL_checkint(L, 2);
  int xmax = luaL_checkint(L, 3);
  int ymin = luaL_checkint(L, 4);
  int ymax = luaL_checkint(L, 5);
  int dx = luaL_checkint(L, 6);
  int dy = luaL_checkint(L, 7);
  cdCanvasScrollArea(cdlua_checkcanvas(L, 1), xmin, xmax, ymin, ymax, dx, dy);
  return 0;
}



/***************************************************************************\
* Other                                                                     *
\***************************************************************************/

/********************************************************************************\
* cd.Play(ctx, xmin, xmax, ymin, ymax: number, data: string) -> (status: number) *
\********************************************************************************/

static int cdlua5_play(lua_State *L)
{
  cdluaContext* cdlua_ctx = cdlua_getcontext(L, 2);
  int xmin = luaL_checkint(L,3);
  int xmax = luaL_checkint(L,4);
  int ymin = luaL_checkint(L,5);
  int ymax = luaL_checkint(L,6);
  const char *data_s = luaL_checkstring(L,7);

  cdlua_setplaystate(L);
  cdCanvasPlay(cdlua_checkcanvas(L, 1), cdlua_ctx->ctx(), xmin, xmax, ymin, ymax, (void*)data_s);
  cdlua_setplaystate(NULL);
  return 0;
}

/***************************************************************************\
* cd.GetColorPlanes() -> (bpp: number)                                      *
\***************************************************************************/
static int cdlua5_getcolorplanes(lua_State *L)
{
  lua_pushnumber(L, cdCanvasGetColorPlanes(cdlua_checkcanvas(L, 1)));
  return 1;
}

static int cdlua5_palette(lua_State *L)
{
  cdluaPalette *pal = cdlua_checkpalette(L, 2);
  int mode_i = luaL_checkint(L, 3);
  cdCanvasPalette(cdlua_checkcanvas(L, 1), pal->count, pal->color, mode_i);
  return 0;
}

/***************************************************************************\
* cd.ImageRGB                                                               *
\***************************************************************************/
static int cdlua5_imagergb(lua_State *L)
{
  int w, h, type = CD_RGB;

  cdCanvas* canvas = cdlua_checkcanvas(L, 1);

  if (cdCanvasGetContext(canvas) != CD_IMAGERGB)
    luaL_argerror(L, 1, "invalid canvas, must be CD_IMAGERGB");

  if (cdAlphaImage(canvas))
    type = CD_RGBA;

  cdCanvasGetSize(canvas, &w, &h, NULL, NULL);

  /* mark the image NOT to be freed */
  if (type == CD_RGBA)
    cdlua_pushimagergba_ex(L, cdRedImage(canvas), cdGreenImage(canvas), cdBlueImage(canvas), cdAlphaImage(canvas), w, h);
  else
    cdlua_pushimagergb_ex(L, cdRedImage(canvas), cdGreenImage(canvas), cdBlueImage(canvas), w, h);

  return 1;
}

/***************************************************************************\
* cd.ImageRGBBitmap                                                        *
\***************************************************************************/
static int cdlua5_imagergbbitmap(lua_State *L)
{
  int w, h, type = CD_RGB;

  cdCanvas* canvas = cdlua_checkcanvas(L, 1);

  if (cdCanvasGetContext(canvas) != CD_IMAGERGB)
    luaL_argerror(L, 1, "invalid canvas, must be CD_IMAGERGB");

  if (cdAlphaImage(canvas))
    type = CD_RGBA;

  cdCanvasGetSize(canvas, &w, &h, NULL, NULL);

  cdlua_pushbitmap(L, cdInitBitmap(w, h, type, 
                                   cdRedImage(canvas),
                                   cdGreenImage(canvas),
                                   cdBlueImage(canvas),
                                   cdAlphaImage(canvas)));

  return 1;
}

/***************************************************************************\
* Hardcopy                                                                  *
\***************************************************************************/
static lua_State* wdlua5_hardcopy_luaState = NULL;

static void wdlua5_hardcopy_func(cdCanvas* canvas) 
{
  lua_State* L = wdlua5_hardcopy_luaState;
  lua_pushvalue(L, 4);   /* push the function in the stack */
  cdlua_pushcanvas(L, canvas);
  if(lua_pcall(L, 1, 0, 0) != 0)
    luaL_error(L, "error running function: %s", lua_tostring(L, -1));
}

static int wdlua5_hardcopy(lua_State *L) 
{
  cdCanvas* canvas = cdlua_checkcanvas(L, 1);
  cdluaContext* cdlua_ctx = cdlua_getcontext(L, 2);
  void *data_p = cdlua_ctx->checkdata(L,3);
  luaL_argcheck(L, !lua_isfunction(L, 4), 4, "invalid draw function");

  wdlua5_hardcopy_luaState = L;
  wdCanvasHardcopy(canvas, cdlua_ctx->ctx(), data_p, wdlua5_hardcopy_func);
  wdlua5_hardcopy_luaState = NULL;

  return 0;
}


/***************************************************************************\
* Polygon functions                                                         *
\***************************************************************************/

/***************************************************************************\
* cd.Begin(mode: number)                                                    *
\***************************************************************************/
static int cdlua5_begin(lua_State *L)
{ 
  cdCanvasBegin(cdlua_checkcanvas(L, 1), luaL_checkint(L, 2));
  return 0;
}

static int cdlua5_vertex(lua_State *L)
{
  cdCanvasVertex(cdlua_checkcanvas(L, 1), luaL_checkint(L, 2), luaL_checkint(L, 3));
  return 0;
}

static int cdlua5_pathset(lua_State *L)
{
  cdCanvasPathSet(cdlua_checkcanvas(L, 1), luaL_checkint(L, 2));
  return 0;
}

static int wdlua5_vertex(lua_State *L)
{
  wdCanvasVertex(cdlua_checkcanvas(L, 1), luaL_checknumber(L, 2), luaL_checknumber(L, 3));
  return 0;
}

static int cdlua5_fvertex(lua_State *L)
{
  cdfCanvasVertex(cdlua_checkcanvas(L, 1), luaL_checknumber(L, 2), luaL_checknumber(L, 3));
  return 0;
}

static int cdlua5_end(lua_State *L)
{
  cdCanvasEnd(cdlua_checkcanvas(L, 1));
  return 0;
}


/********************************************************************************\
* Lua Exported functions                                                       *
\********************************************************************************/

static const struct luaL_reg cdlib_canvas_meta[] = {

  /* Initialization */
  {"GetContext"    , cdlua5_getcontext},
  {"Kill"          , cdlua5_killcanvas},
  {"Activate"      , cdlua5_activate},
  {"Deactivate"    , cdlua5_deactivate},
  {"Simulate"      , cdlua5_simulate},

  /* Control */
  {"Clear"         , cdlua5_clear},
  {"Flush"         , cdlua5_flush},
  {"SaveState"     , cdlua5_savestate},
  {"RestoreState"  , cdlua5_restorestate},
  {"SetAttribute"  , cdlua5_setattribute},
  {"GetAttribute"  , cdlua5_getattribute},

  /* Coordinate System */
  {"GetSize"       , cdlua5_getcanvassize},
  {"UpdateYAxis"   , cdlua5_updateyaxis},
  {"YAxisMode"     , cdlua5_yaxismode},
  {"InvertYAxis"   , cdlua5_invertyaxis},
  {"MM2Pixel"      , cdlua5_mm2pixel},
  {"Pixel2MM"      , cdlua5_pixel2mm},
  {"Origin"        , cdlua5_origin},
  {"GetOrigin"     , cdlua5_getorigin},
  {"fMM2Pixel"      , cdlua5_fmm2pixel},
  {"fPixel2MM"      , cdlua5_fpixel2mm},
  {"fOrigin"        , cdlua5_forigin},
  {"fGetOrigin"     , cdlua5_fgetorigin},
  {"Transform"     , cdlua5_transform},
  {"GetTransform"     , cdlua5_gettransform},
  {"TransformMultiply"     , cdlua5_transformmultiply},
  {"TransformRotate"     , cdlua5_transformrotate},
  {"TransformScale"     , cdlua5_transformscale},
  {"TransformTranslate"     , cdlua5_transformtranslate},
  {"TransformPoint"     , cdlua5_transformpoint},
  {"fTransformPoint"     , cdlua5_ftransformpoint},

  /* World Coordinates */
  {"wWindow"        , wdlua5_window},
  {"wGetWindow"     , wdlua5_getwindow},
  {"wViewport"      , wdlua5_viewport},
  {"wGetViewport"   , wdlua5_getviewport},  
  {"wWorld2Canvas"  , wdlua5_world2canvas},
  {"wCanvas2World"  , wdlua5_canvas2world},
  {"wGetTransform"  , wdlua5_gettransform},
  {"wSetTransform"  , wdlua5_settransform},
  {"wScale"         , wdlua5_scale},
  {"wTranslate"     , wdlua5_translate},

  {"wHardcopy"      , wdlua5_hardcopy},

  /* General Attributes */
  {"Foreground"  , cdlua5_foreground},
  {"Background"  , cdlua5_background},  
  {"SetForeground"  , cdlua5_setforeground},
  {"SetBackground"  , cdlua5_setbackground},  
  {"WriteMode"   , cdlua5_writemode},

  /* Clipping */
  {"Clip"          , cdlua5_clip},
  {"ClipArea"      , cdlua5_cliparea},
  {"GetClipArea"   , cdlua5_getcliparea},
  {"wClipArea"     , wdlua5_cliparea},
  {"wGetClipArea"  , wdlua5_getcliparea},  
  {"fClipArea"      , cdlua5_fcliparea},
  {"fGetClipArea"   , cdlua5_fgetcliparea},

  /* Regions */
  {"RegionCombineMode" , cdlua5_regioncombinemode},
  {"IsPointInRegion"     , cdlua5_pointinregion},
  {"wIsPointInRegion"    , wdlua5_pointinregion},
  {"OffsetRegion"      , cdlua5_offsetregion},
  {"wOffsetRegion"     , wdlua5_offsetregion},
  {"GetRegionBox"         , cdlua5_regionbox},
  {"wGetRegionBox"        , wdlua5_regionbox},

  /* Marks */
  {"Pixel"     , cdlua5_pixel},
  {"wPixel"    , wdlua5_pixel},
  {"Mark"      , cdlua5_mark},
  {"wMark"     , wdlua5_mark},
  {"MarkType"  , cdlua5_marktype},
  {"MarkSize"  , cdlua5_marksize},
  {"wMarkSize" , wdlua5_marksize},

  /* Line */
  {"Line"            , cdlua5_line},
  {"wLine"           , wdlua5_line},
  {"fLine"           , cdlua5_fline},
  {"Rect"            , cdlua5_rect},
  {"wRect"           , wdlua5_rect},
  {"fRect"            , cdlua5_frect},
  {"Arc"             , cdlua5_arc},
  {"wArc"            , wdlua5_arc},
  {"fArc"             , cdlua5_farc},
  {"LineStyle"       , cdlua5_linestyle},
  {"LineStyleDashes" , cdlua5_linestyledashes},
  {"LineWidth"       , cdlua5_linewidth},
  {"wLineWidth"      , wdlua5_linewidth},
  {"LineJoin"        , cdlua5_linejoin},
  {"LineCap"         , cdlua5_linecap},

  /* Filled Areas */
  {"Box"           , cdlua5_box},
  {"wBox"          , wdlua5_box},
  {"fBox"           , cdlua5_fbox},
  {"Sector"        , cdlua5_sector},
  {"wSector"       , wdlua5_sector},
  {"fSector"        , cdlua5_fsector},
  {"Chord"         , cdlua5_chord},  
  {"wChord"        , wdlua5_chord},
  {"fChord"         , cdlua5_fchord},  
  {"BackOpacity"   , cdlua5_backopacity},
  {"FillMode"      , cdlua5_fillmode},  
  {"InteriorStyle" , cdlua5_interiorstyle},
  {"Hatch"         , cdlua5_hatch},

  /* Stipple */
  {"Stipple"      , cdlua5_stipple},
  {"wStipple"     , wdlua5_stipple},
  {"GetStipple"   , cdlua5_getstipple},
  
  /* Pattern */
  {"Pattern"      , cdlua5_pattern},
  {"wPattern"     , wdlua5_pattern},
  {"GetPattern"   , cdlua5_getpattern},

  /* Text */
  {"Text"            , cdlua5_text},
  {"wText"           , wdlua5_text},
  {"fText"            , cdlua5_ftext},
  {"Font"            , cdlua5_font},
  {"wFont"           , wdlua5_font},
  {"GetFont"         , cdlua5_getfont},
  {"wGetFont"        , wdlua5_getfont},
  {"NativeFont"      , cdlua5_nativefont},
  {"TextAlignment"   , cdlua5_textalignment},
  {"TextOrientation" , cdlua5_textorientation},
  {"GetFontDim"         , cdlua5_getfontdim},
  {"wGetFontDim"        , wdlua5_getfontdim},
  {"GetTextSize"        , cdlua5_gettextsize},
  {"wGetTextSize"       , wdlua5_gettextsize},
  {"GetTextBox"         , cdlua5_gettextbox},
  {"wGetTextBox"        , wdlua5_gettextbox},
  {"GetTextBounds"      , cdlua5_gettextbounds},
  {"wGetTextBounds"     , wdlua5_gettextbounds},

  /* Vector Text */
  {"VectorText"           , cdlua5_vectortext},
  {"wVectorText"          , wdlua5_vectortext},
  {"MultiLineVectorText"  , cdlua5_multilinevectortext},
  {"wMultiLineVectorText" , wdlua5_multilinevectortext},
  {"VectorTextDirection"  , cdlua5_vectortextdirection},
  {"wVectorTextDirection" , wdlua5_vectortextdirection},
  {"VectorTextTransform"  , cdlua5_vectortexttransform},
  {"VectorFontSize"       , cdlua5_vectorfontsize},
  {"GetVectorFontSize"    , cdlua5_getvectorfontsize},
  {"VectorTextSize"       , cdlua5_vectortextsize},
  {"wVectorTextSize"      , wdlua5_vectortextsize},
  {"VectorCharSize"       , cdlua5_vectorcharsize},
  {"wVectorCharSize"      , wdlua5_vectorcharsize},
  {"VectorFont"           , cdlua5_vectorfont},
  {"GetVectorTextSize"    , cdlua5_getvectortextsize},
  {"wGetVectorTextSize"   , wdlua5_getvectortextsize},
  {"GetVectorTextBounds"  , cdlua5_getvectortextbounds},
  {"wGetVectorTextBounds" , wdlua5_getvectortextbounds},  
  {"GetVectorTextBox"     , cdlua5_getvectortextbox},
  {"wGetVectorTextBox"    , wdlua5_getvectortextbox},  
  
  /* Client Images */
  {"GetImageRGB"      , cdlua5_getimagergb},
  {"PutImageRectRGB"  , cdlua5_putimagerectrgb},
  {"wPutImageRectRGB" , wdlua5_putimagerectrgb},
  {"PutImageRectRGBA" , cdlua5_putimagerectrgba},
  {"wPutImageRectRGBA", wdlua5_putimagerectrgba},
  {"PutImageRectMap"  , cdlua5_putimagerectmap},
  {"wPutImageRectMap" , wdlua5_putimagerectmap},
  {"GetBitmap"        , cdlua5_getbitmap},
  {"PutBitmap"        , cdlua5_putbitmap},
  {"wPutBitmap"       , wdlua5_putbitmap},
  
  /* Server Images */
  {"CreateImage"      , cdlua5_createimage},
  {"GetImage"         , cdlua5_getimage},
  {"PutImageRect"     , cdlua5_putimagerect},
  {"wPutImageRect"    , wdlua5_putimagerect},
  {"ScrollArea"       , cdlua5_scrollarea},

  /* Other */
  {"Play"             , cdlua5_play},

  /* Color Coding */
  {"GetColorPlanes" , cdlua5_getcolorplanes},

  /* Palette */
  {"Palette"      , cdlua5_palette},

  /* Polygon */
  {"Begin"         , cdlua5_begin},
  {"PathSet"        , cdlua5_pathset},
  {"Vertex"        , cdlua5_vertex},
  {"wVertex"       , wdlua5_vertex},
  {"fVertex"        , cdlua5_fvertex},
  {"End"           , cdlua5_end},

  {"__eq", cdluaCanvas_eq},
  {"__tostring", cdluaCanvas_tostring},

  {NULL, NULL},
};

static const struct luaL_reg cdlib_canvas[] = {
  /* Initialization */
  {"CreateCanvas"  , cdlua5_createcanvas},
  {"KillCanvas"    , cdlua5_killcanvas},
  {"GetContext"    , cdlua5_getcontext},    /* compatibility name */
  {"ImageRGB"         , cdlua5_imagergb},
  {"ImageRGBBitmap"   , cdlua5_imagergbbitmap},
  {NULL, NULL},
};

void cdlua_open_canvas (lua_State *L)
{                                  
  /* "cd" table is at the top of the stack */

  /* Object Oriented Access */
  luaL_newmetatable(L, "cdCanvas");    /* create new metatable for cdCanvas handles */
  lua_pushliteral(L, "__index");
  lua_pushvalue(L, -2);  /* push metatable */
  lua_rawset(L, -3);     /* metatable.__index = metatable */
  luaL_register(L, NULL, cdlib_canvas_meta);  /* register methods */
  lua_pop(L, 1); /* removes the metatable from the top of the stack */

  luaL_register(L, NULL, cdlib_canvas);
}
