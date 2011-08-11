/** \file
 * \brief Lua Binding of the OLD API that needs an active canvas
 *
 * See Copyright Notice in cd.h
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef CD_NO_OLD_INTERFACE
#undef CD_NO_OLD_INTERFACE
#endif

#include "cd.h"
#include "wd.h"
#include "cdirgb.h"

#include <lua.h>
#include <lauxlib.h>

#include "cdlua.h"
#include "cdlua5_private.h"
#include "cdvoid5.h"


void cdlua_kill_active(lua_State * L, cdCanvas* canvas)
{
  cdluaLuaState* cdL = cdlua_getstate(L);
  cdCanvas* void_canvas = cdL->void_canvas;

  /* find out about the currently active canvas */
  cdCanvas *current_canvas = cdActiveCanvas();

  /* this should never happen, unless the user did it on purpouse! */
  if (canvas == void_canvas)
    luaL_error(L, "trying to kill the void canvas");

  /* if the user killed the currently active canvas, activate void canvas */
  if (canvas == current_canvas)
    cdActivate(void_canvas);
}

/***************************************************************************\
* Activates a cd canvas.                                                    *
\***************************************************************************/
static int cdlua5_activate(lua_State * L)
{
  cdCanvas* canvas;

  /* if canvas is nil, activate a void canvas */
  if (lua_isnil(L, 1))
  {
    cdluaLuaState* cdL = cdlua_getstate(L);
    cdCanvas* void_canvas = cdL->void_canvas;
    lua_pushnumber(L, cdActivate(void_canvas));
    return 1;
  }

  canvas = cdlua_checkcanvas(L, 1);
  lua_pushnumber(L, cdActivate(canvas));
  return 1;
}

/***************************************************************************\
* Returns the active canvas.                                                *
\***************************************************************************/
static int cdlua5_activecanvas(lua_State* L)
{
  cdCanvas* canvas = cdActiveCanvas();
  if (canvas)
    cdlua_pushcanvas(L, canvas);
  else
    lua_pushnil(L);

  return 1;
}

/***************************************************************************\
* cd.Simulate(mode: number) -> (old_mode: number)                           *
\***************************************************************************/
static int cdlua5_simulate(lua_State *L)
{
  lua_pushnumber(L, cdSimulate(luaL_checkint(L, 1)));
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
  (void)L;
  cdClear();
  return 0;
}

/***************************************************************************\
* cd.Flush()                                                                *
\***************************************************************************/
static int cdlua5_flush(lua_State *L)
{
  (void)L;
  cdFlush();
  return 0;
}

static int cdlua5_savestate(lua_State *L)
{
  cdState* state = cdSaveState();
  if (state)
    cdlua_pushstate(L, state);
  else
    lua_pushnil(L);
  return 1;
}

static int cdlua5_restorestate(lua_State * L)
{
  cdRestoreState(cdlua_checkstate(L, 1));
  return 0;
}

/***************************************************************************\
* cd.SetAttribute(name, data: string)                                       *
\***************************************************************************/

static int cdlua_isuserdata(const char* name)
{
  if (strcmp(name, "HDC")==0) return 1;
  if (strcmp(name, "GC")==0) return 1;
  return 0;
}

static int cdlua5_setattribute(lua_State *L)
{
  const char* name = luaL_checkstring(L, 1);
   
  if (lua_isnil(L, 2))
  {
    cdSetAttribute(name, NULL);
  }
  else
  {
    char* data;
    if (cdlua_isuserdata(name))
      data = (char*) lua_touserdata(L, 2);
    else
      data = (char*) luaL_checkstring(L, 2);
    cdSetAttribute(name, data);
  }
  return 0;
}


/***************************************************************************\
* cd.SetAttribute(name: string) -> (data: string)                           *
\***************************************************************************/
static int cdlua5_getattribute(lua_State *L)
{
  char* name = (char *)luaL_checkstring(L, 1);
  char* data = cdGetAttribute(name);
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

  cdGetCanvasSize(&width, &height, &mm_width, &mm_height);
  
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
  int y = luaL_checkint(L, 1);
  cdUpdateYAxis(&y);
  lua_pushnumber(L, y);
  return 1;
}

/***************************************************************************\
* cd.MM2Pixel(mm_dx, mm_dy: number) -> (dx, dy: number)                     *
\***************************************************************************/
static int cdlua5_mm2pixel(lua_State *L)
{
  double mm_dx_d, mm_dy_d;
  int dx, dy;

  mm_dx_d = luaL_checknumber(L,1);
  mm_dy_d = luaL_checknumber(L,2);

  cdMM2Pixel(mm_dx_d, mm_dy_d, &dx, &dy);
  lua_pushnumber(L, dx);
  lua_pushnumber(L, dy);
  return 2;
}

/***************************************************************************\
* cd.Pixel2MM(dx, dy: number) -> (mm_dx, mm_dy: number)                     *
\***************************************************************************/
static int cdlua5_pixel2mm(lua_State *L)
{
  double mm_dx_d, mm_dy_d;
  int dx, dy;

  dx = luaL_checkint(L,1);
  dy = luaL_checkint(L,2);

  cdPixel2MM(dx, dy, &mm_dx_d, &mm_dy_d);
  lua_pushnumber(L, mm_dx_d);
  lua_pushnumber(L, mm_dy_d);
  return 2;
}

/***************************************************************************\
* cd.Origin(x, y: number)                                                   *
\***************************************************************************/
static int cdlua5_origin(lua_State *L)
{
  cdOrigin(luaL_checkint(L,1), luaL_checkint(L,2));
  return 0;
}



/***************************************************************************\
* World Coordinates                                                         *
\***************************************************************************/

/***************************************************************************\
* wd.Window(xmin, xmax, ymin, ymax: number)                                 *
\***************************************************************************/
static int wdlua5_window(lua_State *L)
{
  double xmin = luaL_checknumber(L, 1);
  double xmax = luaL_checknumber(L, 2);
  double ymin = luaL_checknumber(L, 3);
  double ymax = luaL_checknumber(L, 4);
  wdWindow(xmin, xmax, ymin, ymax);
  return 0;
}

/***************************************************************************\
* wd.GetWindow() -> (xmin, xmax, ymin, ymax: number)                        *
\***************************************************************************/
static int wdlua5_getwindow(lua_State *L)
{
  double xmin, xmax, ymin, ymax;

  wdGetWindow(&xmin, &xmax, &ymin, &ymax);
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
  int xmin = luaL_checkint(L, 1);
  int xmax = luaL_checkint(L, 2);
  int ymin = luaL_checkint(L, 3);
  int ymax = luaL_checkint(L, 4);
  wdViewport(xmin, xmax, ymin, ymax);
  return 0;
}

/***************************************************************************\
* wd.GetViewport() -> (xmin, xmax, ymin, ymax: number                       *
\***************************************************************************/
static int wdlua5_getviewport(lua_State *L)
{
  int xmin, xmax, ymin, ymax;

  wdGetViewport(&xmin, &xmax, &ymin, &ymax);
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

  xw_d = luaL_checknumber(L, 1);
  yw_d = luaL_checknumber(L, 2);

  wdWorld2Canvas(xw_d, yw_d, &xv_i, &yv_i);
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

  xv_i = luaL_checkint(L, 1);
  yv_i = luaL_checkint(L, 2);

  wdCanvas2World(xv_i, yv_i, &xw_d, &yw_d);
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
  long int color_i = cdlua_checkcolor(L, 1);
  color_i = cdForeground(color_i);
  lua_pushlightuserdata(L, (void*)color_i);
  return 1;
}

/***************************************************************************\
* cd.Background(color) -> color                                             *
\***************************************************************************/
static int cdlua5_background(lua_State *L)
{
  long int color_i = cdlua_checkcolor(L, 1);
  color_i = cdBackground(color_i);
  lua_pushlightuserdata(L, (void*) color_i);
  return 1;
}

/***************************************************************************\
* cd.WriteMode(mode: number) -> (old_mode: number)                          *
\***************************************************************************/
static int cdlua5_writemode(lua_State *L)
{
  lua_pushnumber(L, cdWriteMode(luaL_checkint(L, 1)));
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
  lua_pushnumber(L, cdClip(luaL_checkint(L,1)));
  return 1;
}

/***************************************************************************\
* cd.ClipArea(xmin, xmax, ymin, ymax: number)                               *
\***************************************************************************/
static int cdlua5_cliparea(lua_State *L)
{
  int xmin = luaL_checkint(L, 1);
  int xmax = luaL_checkint(L, 2);
  int ymin = luaL_checkint(L, 3);
  int ymax = luaL_checkint(L, 4);

  cdClipArea(xmin, xmax, ymin, ymax);
  return 0;
}

/***************************************************************************\
* cd.wClipArea(xmin, xmax, ymin, ymax: number)                              *
\***************************************************************************/
static int wdlua5_cliparea(lua_State *L)
{
  double xmin = luaL_checknumber(L, 1);
  double xmax = luaL_checknumber(L, 2);
  double ymin = luaL_checknumber(L, 3);
  double ymax = luaL_checknumber(L, 4);

  wdClipArea(xmin, xmax, ymin, ymax);
  return 0;
}

/***************************************************************************\
* cd.GetClipArea() -> (xmin, xmax, ymin, ymax, status: number)              *
\***************************************************************************/
static int cdlua5_getcliparea(lua_State *L)
{
  int xmin, xmax, ymin, ymax;
  int status;

  status = cdGetClipArea(&xmin, &xmax, &ymin, &ymax);
  lua_pushnumber(L, xmin);
  lua_pushnumber(L, xmax);
  lua_pushnumber(L, ymin);
  lua_pushnumber(L, ymax);
  lua_pushnumber(L, status);
  return 5;
}

/***************************************************************************\
* cd.wGetClipArea() -> (xmin, xmax, ymin, ymax, status: number)             *
\***************************************************************************/
static int wdlua5_getcliparea(lua_State *L)
{
  double xmin, xmax, ymin, ymax;
  int status;

  status = wdGetClipArea(&xmin, &xmax, &ymin, &ymax);
  lua_pushnumber(L, xmin);
  lua_pushnumber(L, xmax);
  lua_pushnumber(L, ymin);
  lua_pushnumber(L, ymax);
  lua_pushnumber(L, status);
  return 5;
}

/***************************************************************************\
* cd.GetClipPoly() -> (n: number, points: table)                            *
\***************************************************************************/
static int cdlua5_getclippoly(lua_State *L)
{
  int n, i;
  int *pts;
  
  pts = cdGetClipPoly(&n);
  if (pts)
  {
    lua_pushnumber(L, n);

    lua_createtable(L, 2*n, 0);
    for (i=0; i < 2*n; i++)
    {
      lua_pushnumber(L, i+1);
      lua_pushnumber(L, pts[i]);
      lua_settable(L, -3);
    }

    return 2;
  }
  else
  {
    lua_pushnil(L);
    return 1;
  }
}

/***************************************************************************\
* cd.wGetClipPoly() -> (n: number, points: table)                           *
\***************************************************************************/
static int wdlua5_getclippoly(lua_State *L)
{
  int n, i;
  double *pts;
  
  pts = wdGetClipPoly(&n);
  if (pts)
  {
    lua_pushnumber(L, n);

    lua_createtable(L, 2*n, 0);
    for (i=0; i < 2*n; i++)
    {
      lua_pushnumber(L, i+1);
      lua_pushnumber(L, pts[i]);
      lua_settable(L,-3);
    }

    return 2;
  }
  else
  {
    lua_pushnil(L);
    return 1;
  }
}


/***************************************************************************\
* Regions                                                                   *
\***************************************************************************/

/***************************************************************************\
* cd.RegionCombineMode(mode: number) -> (old_mode: number)                  *
\***************************************************************************/
static int cdlua5_regioncombinemode(lua_State *L)
{
  lua_pushnumber(L, cdRegionCombineMode(luaL_checkint(L, 1)));
  return 1;
}

/***************************************************************************\
* cd.PointInRegion(x, y: number) -> (status: number)                        *
\***************************************************************************/
static int  cdlua5_pointinregion(lua_State *L)
{
  lua_pushnumber(L, cdPointInRegion(luaL_checkint(L, 1),luaL_checkint(L, 2)));
  return 1;
}

/***************************************************************************\
* cd.wPointInRegion(x, y: number) -> (status: number)                       *
\***************************************************************************/
static int wdlua5_pointinregion(lua_State *L)
{
  lua_pushnumber(L, wdPointInRegion(luaL_checknumber(L, 1), luaL_checknumber(L, 2)));
  return 1;
}

/***************************************************************************\
* cd.OffsetRegion(dx, dy: number)                                           *
\***************************************************************************/
static int cdlua5_offsetregion(lua_State *L)
{
  cdOffsetRegion(luaL_checkint(L, 1), luaL_checkint(L, 2));
  return 0;
}

/***************************************************************************\
* cd.wOffsetRegion(dx, dy: number)                                          *
\***************************************************************************/
static int wdlua5_offsetregion(lua_State *L)
{
  wdOffsetRegion(luaL_checknumber(L, 1), luaL_checknumber(L, 2));
  return 0;
}

/***************************************************************************\
* cd.RegionBox() -> (xmin, xmax, ymin, ymax, status: number)                *
\***************************************************************************/
static int cdlua5_regionbox(lua_State *L)
{
  int xmin, xmax, ymin, ymax;
  
  cdRegionBox(&xmin, &xmax, &ymin, &ymax);
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

  wdRegionBox(&xmin, &xmax, &ymin, &ymax);
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
* Marks                                                                     *
\***************************************************************************/

/***************************************************************************\
* cd.Pixel(x, y: number, color)                                             *
\***************************************************************************/
static int cdlua5_pixel (lua_State *L)
{
  cdPixel(luaL_checkint(L, 1), luaL_checkint(L, 2), cdlua_checkcolor(L, 3));
  return 0 ;
}

/***************************************************************************\
* cd.wPixel(x, y: number, color)                                            *
\***************************************************************************/
static int wdlua5_pixel (lua_State *L)
{
  wdPixel(luaL_checknumber(L, 1), luaL_checknumber(L, 2), cdlua_checkcolor(L, 3));
  return 0;
}

/***************************************************************************\
* cd.Mark(x, y: number)                                                     *
\***************************************************************************/
static int cdlua5_mark(lua_State *L)
{
  cdMark(luaL_checkint(L,1), luaL_checkint(L,2));
  return 0;
}

/***************************************************************************\
* cd.wMark(x, y: number)                                                    *
\***************************************************************************/
static int wdlua5_mark(lua_State *L)
{
  wdMark(luaL_checknumber(L, 1), luaL_checknumber(L, 2));
  return 0;
}

/***************************************************************************\
* cd.MarkType(type: number) -> (old_type: number)                           *
\***************************************************************************/
static int cdlua5_marktype(lua_State *L)
{
  lua_pushnumber(L, cdMarkType(luaL_checkint(L, 1)));
  return 1;
}

/***************************************************************************\
* cd.MarkSize(size: number) -> (old_size: number)                           *
\***************************************************************************/
static int cdlua5_marksize(lua_State *L)
{
  lua_pushnumber(L, cdMarkSize(luaL_checkint(L, 1)));
  return 1;
}

/***************************************************************************\
* cd.wMarkSize(size: number) -> (old_size: number)                          *
\***************************************************************************/
static int wdlua5_marksize(lua_State *L)
{
  lua_pushnumber(L, wdMarkSize(luaL_checknumber(L, 1)));
  return 1;
}



/***************************************************************************\
* Lines                                                                     *
\***************************************************************************/

/***************************************************************************\
* cd.Line(x1, y1, x2, y2: number)                                           *
\***************************************************************************/
static int cdlua5_line(lua_State *L)
{
  int x1 = luaL_checkint(L,1);
  int y1 = luaL_checkint(L,2);
  int x2 = luaL_checkint(L,3);
  int y2 = luaL_checkint(L,4);
  cdLine(x1, y1, x2, y2);
  return 0;
}

/***************************************************************************\
* cd.wLine(x1, y1, x2, y2: number)                                          *
\***************************************************************************/
static int wdlua5_line(lua_State *L)
{
  double x1 = luaL_checknumber(L, 1);
  double y1 = luaL_checknumber(L, 2);
  double x2 = luaL_checknumber(L, 3);
  double y2 = luaL_checknumber(L, 4);
  wdLine(x1, y1, x2, y2);
  return 0;
}

/***************************************************************************\
* cd.Rect(xmin, xmax, ymin, ymax: number)                                   *
\***************************************************************************/
static int cdlua5_rect(lua_State *L)
{
  int xmin = luaL_checkint(L,1);
  int xmax = luaL_checkint(L,2);
  int ymin = luaL_checkint(L,3);
  int ymax = luaL_checkint(L,4);
  cdRect(xmin, xmax, ymin, ymax);
  return 0;
}

/***************************************************************************\
* cd.wRect(xmin, xmax, ymin, ymax: number)                                  *
\***************************************************************************/
static int wdlua5_rect(lua_State *L)
{
  double xmin = luaL_checknumber(L, 1);
  double xmax = luaL_checknumber(L, 2);
  double ymin = luaL_checknumber(L, 3);
  double ymax = luaL_checknumber(L, 4);
  wdRect(xmin,xmax,ymin,ymax);
  return 0;
}

/***************************************************************************\
* cd.Arc(xc, yc, w, h, angle1, angle2: number)                              *
\***************************************************************************/
static int cdlua5_arc(lua_State *L)
{
  int xc = luaL_checkint(L,1);
  int yc = luaL_checkint(L,2);
  int w = luaL_checkint(L,3);
  int h = luaL_checkint(L,4);
  double angle1 = luaL_checknumber(L,5);
  double angle2 = luaL_checknumber(L,6);
  cdArc(xc, yc, w, h, angle1, angle2);
  return 0;
}

/***************************************************************************\
* cd.wArc(xc, yc, w, h, angle1, angle2: number)                             *
\***************************************************************************/
static int wdlua5_arc(lua_State *L)
{
  double xc = luaL_checknumber(L, 1);
  double yc = luaL_checknumber(L, 2);
  double w = luaL_checknumber(L, 3);
  double h = luaL_checknumber(L, 4);
  double angle1 = luaL_checknumber(L, 5);
  double angle2 = luaL_checknumber(L, 6);
  wdArc(xc, yc, w, h, angle1, angle2);
  return 0;
}

/***************************************************************************\
* cd.LineStyle(style: number) -> (old_style: number)                        *
\***************************************************************************/
static int cdlua5_linestyle(lua_State *L)
{
  lua_pushnumber(L, cdLineStyle(luaL_checkint(L, 1)));
  return 1;
}

/***************************************************************************\
* cd.LineStyleDashes(dashes: table, count: number)                          *
\***************************************************************************/
static int cdlua5_linestyledashes(lua_State *L)
{
  int *dashes_int, dashes_count, i;

  if (!lua_istable(L, 1))
    luaL_argerror(L, 1, "invalid dashes, must be a table");

  dashes_count = luaL_checkint(L, 2);
  dashes_int = (int*) malloc(dashes_count * sizeof(int));

  for (i=0; i < dashes_count; i++)
  {
    lua_pushnumber(L, i+1);
    lua_gettable(L,1);

    dashes_int[i] = luaL_checkint(L,-1);
  }

  cdLineStyleDashes(dashes_int, dashes_count);
  free(dashes_int);

  return 0;
}

/***************************************************************************\
* cd.LineWidth(width: number) -> (old_width: number)                        *
\***************************************************************************/
static int cdlua5_linewidth(lua_State *L)
{
  lua_pushnumber(L, cdLineWidth(luaL_checkint(L, 1)));
  return 1;
}

/***************************************************************************\
* cd.wLineWidth(width: number) -> (old_width: number)                       *
\***************************************************************************/
static int wdlua5_linewidth(lua_State *L)
{
  lua_pushnumber(L, wdLineWidth(luaL_checknumber(L, 1)));
  return 1;
}

/***************************************************************************\
* cd.LineJoin(style: number) -> (old_style: number)                         *
\***************************************************************************/
static int cdlua5_linejoin(lua_State *L)
{
  lua_pushnumber(L, cdLineJoin(luaL_checkint(L, 1)));
  return 1;
}

/***************************************************************************\
* cd.LineCap(style: number) -> (old_style: number)                          *
\***************************************************************************/
static int cdlua5_linecap(lua_State *L)
{
  lua_pushnumber(L, cdLineCap(luaL_checkint(L, 1)));
  return 1;
}



/***************************************************************************\
* Filled Areas                                                              *
\***************************************************************************/

/***************************************************************************\
* cd.Box(xmin, xmax, ymin, ymax: number)                                    *
\***************************************************************************/
static int cdlua5_box(lua_State *L)
{
  int xmin = luaL_checkint(L, 1);
  int xmax = luaL_checkint(L, 2);
  int ymin = luaL_checkint(L, 3);
  int ymax = luaL_checkint(L, 4);
  cdBox(xmin, xmax, ymin, ymax);
  return 0;
}

/***************************************************************************\
* cd.wBox(xmin, xmax, ymin, ymax: number)                                   *
\***************************************************************************/
static int wdlua5_box(lua_State *L)
{
  double xmin = luaL_checknumber(L, 1);
  double xmax = luaL_checknumber(L, 2);
  double ymin = luaL_checknumber(L, 3);
  double ymax = luaL_checknumber(L, 4);
  wdBox(xmin, xmax, ymin, ymax);
  return 0;
}

/***************************************************************************\
* cd.Sector(xc, yc, w, h, angle1, angle2: number)                           *
\***************************************************************************/
static int cdlua5_sector(lua_State *L)
{
  int xc = luaL_checkint(L,1);
  int yc = luaL_checkint(L,2);
  int w = luaL_checkint(L,3);
  int h = luaL_checkint(L,4);
  double angle1 = luaL_checknumber(L,5);
  double angle2 = luaL_checknumber(L,6);
  cdSector(xc, yc, w, h, angle1, angle2);
  return 0;
}

/***************************************************************************\
* cd.wSector(xc, yc, w, h, angle1, angle2: number)                          *
\***************************************************************************/
static int wdlua5_sector(lua_State *L)
{
  double xc = luaL_checknumber(L, 1);
  double yc = luaL_checknumber(L, 2);
  double w = luaL_checknumber(L, 3);
  double h = luaL_checknumber(L, 4);
  double angle1 = luaL_checknumber(L, 5);
  double angle2 = luaL_checknumber(L, 6);
  wdSector(xc, yc, w, h, angle1, angle2);
  return 0;
}

/***************************************************************************\
* cd.Chord(xc, yc, w, h, angle1, angle2: number)                            *
\***************************************************************************/
static int cdlua5_chord(lua_State *L)
{
  int xc = luaL_checkint(L,1);
  int yc = luaL_checkint(L,2);
  int w = luaL_checkint(L,3);
  int h = luaL_checkint(L,4);
  double angle1 = luaL_checknumber(L,5);
  double angle2 = luaL_checknumber(L,6);
  cdChord(xc, yc, w, h, angle1, angle2);
  return 0;
}

/***************************************************************************\
* cd.wChord(xc, yc, w, h, angle1, angle2: number)                           *
\***************************************************************************/
static int wdlua5_chord(lua_State *L)
{
  double xc = luaL_checknumber(L, 1);
  double yc = luaL_checknumber(L, 2);
  double w = luaL_checknumber(L, 3);
  double h = luaL_checknumber(L, 4);
  double angle1 = luaL_checknumber(L, 5);
  double angle2 = luaL_checknumber(L, 6);
  wdChord(xc, yc, w, h, angle1, angle2);
  return 0;
}

/***************************************************************************\
* cd.BackOpacity(opacity: number) -> (old_opacity: number)                  *
\***************************************************************************/
static int cdlua5_backopacity(lua_State *L)
{
  lua_pushnumber(L, cdBackOpacity(luaL_checkint(L, 1)));
  return 1;
}

/***************************************************************************\
* cd.FillMode(mode: number) -> (old_mode: number)                           *
\***************************************************************************/
static int cdlua5_fillmode(lua_State *L)
{
  lua_pushnumber(L, cdFillMode(luaL_checkint(L, 1)));
  return 1;
}

/***************************************************************************\
* cd.InteriorStyle(style: number) -> (old_style: number)                    *
\***************************************************************************/
static int cdlua5_interiorstyle(lua_State *L)
{
  lua_pushnumber(L, cdInteriorStyle(luaL_checkint(L, 1)));
  return 1;
}

/***************************************************************************\
* cd.Hatch(style: number) -> (old_style: number)                            *
\***************************************************************************/
static int cdlua5_hatch(lua_State *L)
{
  lua_pushnumber(L, cdHatch(luaL_checkint(L, 1)));
  return 1;
}

static int cdlua5_stipple(lua_State *L)
{
  cdluaStipple *stipple_p = cdlua_checkstipple(L, 1);
  cdStipple(stipple_p->width, stipple_p->height, stipple_p->stipple);
  return 0 ;
}

static int wdlua5_stipple(lua_State *L)
{
  cdluaStipple *stipple_p = cdlua_checkstipple(L, 1);
  double w_mm = luaL_checknumber(L, 2);
  double h_mm = luaL_checknumber(L, 3);
  wdStipple(stipple_p->width, stipple_p->height, stipple_p->stipple, w_mm, h_mm);
  return 0;
}

static int cdlua5_getstipple(lua_State *L)
{
  int width, height, size;
  unsigned char *stipple, *new_stipple = NULL;

  stipple = cdGetStipple(&width, &height);

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
  cdluaPattern* pattern_p = cdlua_checkpattern(L, 1);
  cdPattern(pattern_p->width, pattern_p->height, pattern_p->pattern);
  return 0;
}

static int wdlua5_pattern(lua_State *L)
{
  cdluaPattern* pattern_p = cdlua_checkpattern(L, 1);
  double w_mm = luaL_checknumber(L, 2);
  double h_mm = luaL_checknumber(L, 3);
  wdPattern(pattern_p->width, pattern_p->height, pattern_p->pattern, w_mm, h_mm);
  return 0;
}

static int cdlua5_getpattern(lua_State *L)
{
  int width, height, size;
  long int *pattern, *new_pattern = NULL;

  pattern = cdGetPattern(&width, &height);

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

/***************************************************************************\
* cd.Text(x, y: number, text: string)                                       *
\***************************************************************************/
static int cdlua5_text(lua_State *L)
{
  const char* s = luaL_checkstring(L, 3);
  cdText(luaL_checkint(L, 1), luaL_checkint(L, 2), s);
  return 0;
}

/***************************************************************************\
* cd.wText(x, y: number, text: string)                                      *
\***************************************************************************/
static int wdlua5_text(lua_State *L)
{
  const char* s = luaL_checkstring(L, 3);
  wdText(luaL_checknumber(L, 1), luaL_checknumber(L, 2), s);
  return 0;
}

/***************************************************************************\
* cd.Font(typeface, style, size: number)                                    *
\***************************************************************************/
static int cdlua5_font(lua_State *L)
{
  cdFont(luaL_checkint(L, 1), luaL_checkint(L, 2), luaL_checkint(L, 3));
  return 0;
}

/***************************************************************************\
* cd.wFont(typeface, style, size: number)                                   *
\***************************************************************************/
static int wdlua5_font(lua_State *L)
{
  wdFont(luaL_checkint(L, 1), luaL_checkint(L, 2), luaL_checknumber(L, 3));
  return 0;
}


/***************************************************************************\
* cd.GetFont() -> (typeface, style, size: number)                           *
\***************************************************************************/
static int cdlua5_getfont(lua_State *L)
{
  int type_face, style, size;

  cdGetFont(&type_face, &style, &size);
  lua_pushnumber(L, type_face);
  lua_pushnumber(L, style);
  lua_pushnumber(L, size);
  return 3;
}

/***************************************************************************\
* cd.wGetFont() -> (typeface, style, size: number)                          *
\***************************************************************************/
static int wdlua5_getfont(lua_State *L)
{
  int type_face, style;
  double size;

  wdGetFont(&type_face, &style, &size);
  lua_pushnumber(L, type_face);
  lua_pushnumber(L, style);
  lua_pushnumber(L, size);
  return 3;
}

/***************************************************************************\
* cd.NativeFont(font: string)                                               *
\***************************************************************************/
static int cdlua5_nativefont(lua_State *L)
{
  lua_pushstring(L, cdNativeFont(luaL_checkstring(L, 1)));
  return 1;
}

/***************************************************************************\
* cd.TextAlignment(alignment: number) -> (old_alignment: number)            *
\***************************************************************************/
static int cdlua5_textalignment(lua_State *L)
{
  lua_pushnumber(L, cdTextAlignment(luaL_checkint(L, 1)));
  return 1;
}

/***************************************************************************\
* cd.TextOrientation(angle: number) -> (old_angle: number)                  *
\***************************************************************************/
static int cdlua5_textorientation(lua_State *L)
{
  lua_pushnumber(L, cdTextOrientation(luaL_checknumber(L, 1)));
  return 1;
}

/***************************************************************************\
* cd.FontDim() -> (max_width, max_height, ascent, descent: number)          *
\***************************************************************************/
static int cdlua5_fontdim(lua_State *L)
{
  int max_width;
  int height;
  int ascent;
  int descent;

  cdFontDim(&max_width, &height, &ascent, &descent);
  lua_pushnumber(L, max_width);
  lua_pushnumber(L, height);
  lua_pushnumber(L, ascent);
  lua_pushnumber(L, descent);
  return 4;
}

/***************************************************************************\
* cd.wFontDim() -> (max_width, max_height, ascent, descent: number)         *
\***************************************************************************/
static int wdlua5_fontdim(lua_State *L)
{
  double max_width;
  double height;
  double ascent;
  double descent;

  wdFontDim(&max_width, &height, &ascent, &descent);
  lua_pushnumber(L, max_width);
  lua_pushnumber(L, height);
  lua_pushnumber(L, ascent);
  lua_pushnumber(L, descent);
  return 4;
}

/***************************************************************************\
* cd.TextSize(text: string) -> (width, heigth: number)                      *
\***************************************************************************/
static int cdlua5_textsize(lua_State *L)
{
  int width;
  int height;

  const char* text_s = luaL_checkstring(L, 1);

  cdTextSize(text_s, &width, &height);
  lua_pushnumber(L, width);
  lua_pushnumber(L, height);
  return 2;
}

/***************************************************************************\
* cd.wTextSize(text: string) -> (width, heigth: number)                     *
\***************************************************************************/
static int wdlua5_textsize(lua_State *L)
{
  double width;
  double height;
 
  const char* text_s = luaL_checkstring(L, 1);

  wdTextSize(text_s, &width, &height);
  lua_pushnumber(L, width);
  lua_pushnumber(L, height);
  return 2;
}

/****************************************************************************\
* cd.TextBox(x, y: number, text: string) -> (xmin, xmax, ymin, ymax: number) *
\****************************************************************************/
static int cdlua5_textbox(lua_State *L)
{
  int xmin, xmax, ymin, ymax;
  int x = luaL_checkint(L, 1);
  int y = luaL_checkint(L, 2);
  const char* s = luaL_checkstring(L, 3);

  cdTextBox(x, y, s, &xmin, &xmax, &ymin, &ymax);
  lua_pushnumber(L, xmin);
  lua_pushnumber(L, xmax);
  lua_pushnumber(L, ymin);
  lua_pushnumber(L, ymax);
  return 4;
}

/*****************************************************************************\
* cd.wTextBox(x, y: number, text: string) -> (xmin, xmax, ymin, ymax: number) *
\*****************************************************************************/
static int wdlua5_textbox(lua_State *L)
{
  double xmin, xmax, ymin, ymax;
  double x = luaL_checknumber(L, 1);
  double y = luaL_checknumber(L, 2);
  const char* s = luaL_checkstring(L, 3);

  wdTextBox(x, y, s, &xmin, &xmax, &ymin, &ymax);
  lua_pushnumber(L, xmin);
  lua_pushnumber(L, xmax);
  lua_pushnumber(L, ymin);
  lua_pushnumber(L, ymax);
  return 4;
}

/***************************************************************************************************************\
* cd.TextBounds(x, y: number, text: string) -> (rect0, rect1, rect2, rect3, rect4, rect5, rect6, rect7: number) *
\***************************************************************************************************************/
static int cdlua5_textbounds(lua_State *L)
{
  int rect[8];
  int x = luaL_checkint(L, 1);
  int y = luaL_checkint(L, 2);
  const char* s = luaL_checkstring(L, 3);

  cdTextBounds(x, y, s, rect);
  lua_pushnumber(L, rect[0]);
  lua_pushnumber(L, rect[1]);
  lua_pushnumber(L, rect[2]);
  lua_pushnumber(L, rect[3]);
  lua_pushnumber(L, rect[4]);
  lua_pushnumber(L, rect[5]);
  lua_pushnumber(L, rect[6]);
  lua_pushnumber(L, rect[7]);
  return 4;
}

/****************************************************************************************************************\
* cd.wTextBounds(x, y: number, text: string) -> (rect0, rect1, rect2, rect3, rect4, rect5, rect6, rect7: number) *
\****************************************************************************************************************/
static int wdlua5_textbounds(lua_State *L)
{
  double rect[8];
  double x = luaL_checknumber(L, 1);
  double y = luaL_checknumber(L, 2);
  const char* s = luaL_checkstring(L, 3);

  wdTextBounds(x, y, s, rect);
  lua_pushnumber(L, rect[0]);
  lua_pushnumber(L, rect[1]);
  lua_pushnumber(L, rect[2]);
  lua_pushnumber(L, rect[3]);
  lua_pushnumber(L, rect[4]);
  lua_pushnumber(L, rect[5]);
  lua_pushnumber(L, rect[6]);
  lua_pushnumber(L, rect[7]);
  return 4;
}



/***************************************************************************\
* Text                                                                      *
\***************************************************************************/

/***************************************************************************\
* cd.VectorText(x, y: number, text: string)                                 *
\***************************************************************************/
static int cdlua5_vectortext(lua_State *L)
{
  const char* s = luaL_checkstring(L,3);
  cdVectorText(luaL_checkint(L, 1), luaL_checkint(L, 2), s);
  return 0;
}

/***************************************************************************\
* cd.wVectorText(x, y: number, text: string)                                *
\***************************************************************************/
static int wdlua5_vectortext(lua_State *L)
{
  const char* s = luaL_checkstring(L, 3);
  wdVectorText(luaL_checknumber(L, 1), luaL_checknumber(L, 2),s);
  return 0;
}

/***************************************************************************\
* cd.MultiLineVectorText(x, y: number, text: string)                        *
\***************************************************************************/
static int cdlua5_multilinevectortext(lua_State *L)
{
  const char* s = luaL_checkstring(L, 3);
  cdMultiLineVectorText(luaL_checkint(L, 1), luaL_checkint(L, 2), s);
  return 0;
}

/***************************************************************************\
* cd.wMultiLineVectorText(x, y: number, text: string)                       *
\***************************************************************************/
static int wdlua5_multilinevectortext(lua_State *L)
{
  const char* s = luaL_checkstring(L, 3);
  wdMultiLineVectorText(luaL_checknumber(L, 1), luaL_checknumber(L, 2), s);
  return 0;
}

/***************************************************************************\
* cd.VectorTextDirection(x1, y1, x2, y2: number)                            *
\***************************************************************************/
static int cdlua5_vectortextdirection(lua_State *L)
{
  int x1 = luaL_checkint(L,1);
  int y1 = luaL_checkint(L,2);
  int x2 = luaL_checkint(L,3);
  int y2 = luaL_checkint(L,4);
  cdVectorTextDirection(x1, y1, x2, y2);
  return 0;
}

/***************************************************************************\
* cd.wVectorTextDirection(x1, y1, x2, y2: number)                           *
\***************************************************************************/
static int wdlua5_vectortextdirection(lua_State *L)
{
  double x1 = luaL_checknumber(L, 1);
  double y1 = luaL_checknumber(L, 2);
  double x2 = luaL_checknumber(L, 3);
  double y2 = luaL_checknumber(L, 4);
  wdVectorTextDirection(x1, y1, x2, y2);
  return 0;
}


/***************************************************************************\
* cd.VectorTextTransform(matrix: table) -> (old_matrix: table)              *
\***************************************************************************/
static int cdlua5_vectortexttransform(lua_State *L)
{
  double matrix[6], *old_matrix;
  int i;

  if (!lua_istable(L, 1))
    luaL_argerror(L, 1, "invalid matrix, must be a table");

  for (i=0; i < 6; i++)
  {
    lua_rawgeti(L, 1, i+1);

    if (!lua_isnumber(L, -1))
      luaL_argerror(L, 1, "invalid matrix value, must be a number");

    matrix[i] = lua_tonumber(L, -1);
    lua_pop(L, 1);
  }

  old_matrix = cdVectorTextTransform(matrix);
  lua_createtable(L, 6, 0);
  for (i=0; i < 6; i++)
  {
    lua_pushnumber(L, old_matrix[i]);
    lua_rawseti(L, 1, i+1);
  }
  return 1;
}

/***************************************************************************\
* cd.VectorTextSize(w, h: number, text: string)                             *
\***************************************************************************/
static int cdlua5_vectortextsize(lua_State *L)
{
  const char* s = luaL_checkstring(L, 3);
  cdVectorTextSize(luaL_checkint(L,1), luaL_checkint(L,2), s);
  return 0;
}

/***************************************************************************\
* cd.wVectorTextSize(w, h: number, text: string)                            *
\***************************************************************************/
static int wdlua5_vectortextsize(lua_State *L)
{
  const char* s = luaL_checkstring(L, 3);
  wdVectorTextSize(luaL_checknumber(L, 1), luaL_checknumber(L, 2), s);
  return 0;
}

/***************************************************************************\
* cd.VectorTextSize(w, h: number, text: string)                             *
\***************************************************************************/
static int cdlua5_vectorcharsize(lua_State *L)
{
  lua_pushnumber(L, cdVectorCharSize(luaL_checkint(L, 1)));
  return 1;
}

/***************************************************************************\
* cd.wVectorTextSize(w, h: number, text: string)                            *
\***************************************************************************/
static int wdlua5_vectorcharsize(lua_State *L)
{
  lua_pushnumber(L, wdVectorCharSize(luaL_checknumber(L, 1)));
  return 1;
}

/***************************************************************************\
* cd.VectorFont(filename: string) -> (font_name: string)                    *
\***************************************************************************/
static int cdlua5_vectorfont(lua_State *L)
{
  lua_pushstring(L, cdVectorFont(luaL_checkstring(L, 1)));
  return 1;
}

/***************************************************************************\
* cd.GetVectorTextSize(text: string) -> (w, h: number)                      *
\***************************************************************************/
static int cdlua5_getvectortextsize(lua_State *L)
{
  int width;
  int height;

  const char* text_s = luaL_checkstring(L, 1);

  cdGetVectorTextSize(text_s, &width, &height);
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

  const char* text_s = luaL_checkstring(L, 1);

  wdGetVectorTextSize(text_s, &width, &height);
  lua_pushnumber(L, width);
  lua_pushnumber(L, height);
  return 2;
}

/***************************************************************************\
* cd.GetVectorTextBounds(s: string, px,py: number) -> (rect: table)         *
\***************************************************************************/
static int cdlua5_vectortextbounds(lua_State *L)
{
  const char* s = luaL_checkstring(L, 1);
  int x = luaL_checkint(L, 2);
  int y = luaL_checkint(L, 3);
  int rect[8], i;

  cdGetVectorTextBounds(s, x, y, rect);
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
static int wdlua5_vectortextbounds(lua_State *L)
{
  const char* s = luaL_checkstring(L, 1);
  double x = luaL_checknumber(L, 2);
  double y = luaL_checknumber(L, 3);
  double rect[8];
  int i;
  
  wdGetVectorTextBounds(s, x, y, rect);
  lua_createtable(L, 8, 0);
  for (i=0; i < 8; i++)
  {
    lua_pushnumber(L, rect[i]);
    lua_rawseti(L, -2, i+1);
  }
  return 1;
}



/***************************************************************************\
* Client Images.                                                            *
\***************************************************************************/

static int cdlua5_getimagergb(lua_State *L)
{
  cdluaImageRGB* imagergb_p = cdlua_checkimagergb(L, 1);
  int x = luaL_checkint(L, 2);
  int y = luaL_checkint(L, 3);

  cdGetImageRGB(imagergb_p->red, imagergb_p->green, imagergb_p->blue,
                x, y, imagergb_p->width, imagergb_p->height);
  return 0;
}

static int cdlua5_putimagerectrgb(lua_State *L)
{
  cdluaImageRGB* imagergb_p = cdlua_checkimagergb(L, 1);
  int x = luaL_checkint(L, 2);
  int y = luaL_checkint(L, 3);
  int w = luaL_checkint(L, 4);
  int h = luaL_checkint(L, 5);
  int xmin = luaL_checkint(L, 6);
  int xmax = luaL_checkint(L, 7);
  int ymin = luaL_checkint(L, 8);
  int ymax = luaL_checkint(L, 9);

  if (w < 0 || h < 0)
    luaL_argerror(L, 4, "target region dimensions should be positive integers");
  
  cdPutImageRectRGB(imagergb_p->width, imagergb_p->height, imagergb_p->red, 
                    imagergb_p->green, imagergb_p->blue, x, y, w, h, xmin, xmax, ymin, ymax);
  return 0;
}

static int wdlua5_putimagerectrgb(lua_State *L)
{
  cdluaImageRGB* imagergb_p = cdlua_checkimagergb(L, 1);
  double x = luaL_checknumber(L, 2);
  double y = luaL_checknumber(L, 3);
  double w = luaL_checknumber(L, 4);
  double h = luaL_checknumber(L, 5);
  int xmin = luaL_checkint(L, 6);
  int xmax = luaL_checkint(L, 7);
  int ymin = luaL_checkint(L, 8);
  int ymax = luaL_checkint(L, 9);

  if (w < 0 || h < 0)
    luaL_argerror(L, 4, "target region dimensions should be positive integers");
  
  wdPutImageRectRGB(imagergb_p->width, imagergb_p->height, imagergb_p->red, 
                    imagergb_p->green, imagergb_p->blue, x, y, w, h, xmin, xmax, ymin, ymax);
  return 0;
}

static int cdlua5_putimagerectrgba(lua_State *L)
{
  cdluaImageRGBA* imagergba_p = cdlua_checkimagergba(L, 1);
  int x = luaL_checkint(L, 2);
  int y = luaL_checkint(L, 3);
  int w = luaL_checkint(L, 4);
  int h = luaL_checkint(L, 5);
  int xmin = luaL_checkint(L, 6);
  int xmax = luaL_checkint(L, 7);
  int ymin = luaL_checkint(L, 8);
  int ymax = luaL_checkint(L, 9);

  if (w < 0 || h < 0)
    luaL_argerror(L, 4, "target region dimensions should be positive integers");

  cdPutImageRectRGBA(imagergba_p->width, imagergba_p->height, imagergba_p->red, 
                     imagergba_p->green, imagergba_p->blue, imagergba_p->alpha, x, y, w, h, xmin, xmax, ymin, ymax);
  return 0;
}

static int wdlua5_putimagerectrgba(lua_State *L)
{
  cdluaImageRGBA* imagergba_p = cdlua_checkimagergba(L, 1);
  double x = luaL_checknumber(L, 2);
  double y = luaL_checknumber(L, 3);
  double w = luaL_checknumber(L, 4);
  double h = luaL_checknumber(L, 5);
  int xmin = luaL_checkint(L, 6);
  int xmax = luaL_checkint(L, 7);
  int ymin = luaL_checkint(L, 8);
  int ymax = luaL_checkint(L, 9);

  if (w < 0 || h < 0)
    luaL_argerror(L, 4, "target region dimensions should be positive integers");

  wdPutImageRectRGBA(imagergba_p->width, imagergba_p->height, imagergba_p->red, 
                     imagergba_p->green, imagergba_p->blue, imagergba_p->alpha, x, y, w, h, xmin, xmax, ymin, ymax);
  return 0;
}

static int cdlua5_putimagerectmap(lua_State *L)
{
  cdluaImageMap* imagemap_p = cdlua_checkimagemap(L, 1);
  cdluaPalette *pal = cdlua_checkpalette(L, 2);
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
  
  cdPutImageRectMap(imagemap_p->width, imagemap_p->height, imagemap_p->index, 
                    pal->color, x, y, w, h, xmin, xmax, ymin, ymax);
  return 0;
}

static int wdlua5_putimagerectmap(lua_State *L)
{
  cdluaImageMap* imagemap_p = cdlua_checkimagemap(L, 1);
  cdluaPalette *pal = cdlua_checkpalette(L, 2);
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
  
  wdPutImageRectMap(imagemap_p->width, imagemap_p->height, imagemap_p->index, 
                    pal->color, x, y, w, h, xmin, xmax, ymin, ymax);
  return 0;
}

static int cdlua5_putbitmap(lua_State *L)
{
  cdBitmap *bitmap = cdlua_checkbitmap(L, 1);
  int x = luaL_checkint(L, 2);
  int y = luaL_checkint(L, 3);
  int w = luaL_checkint(L, 4);
  int h = luaL_checkint(L, 5);
  
  if (w < 0 || h < 0)
    luaL_argerror(L, 4, "target region dimensions should be positive integers");

  cdPutBitmap(bitmap, x, y, w, h);
  return 0;
}

static int wdlua5_putbitmap(lua_State *L)
{
  cdBitmap *bitmap = cdlua_checkbitmap(L, 1);
  double x = luaL_checknumber(L, 2);
  double y = luaL_checknumber(L, 3);
  double w = luaL_checknumber(L, 4);
  double h = luaL_checknumber(L, 5);

  if (w < 0 || h < 0)
    luaL_argerror(L, 4, "target region dimensions should be positive integers");

  wdPutBitmap(bitmap, x, y, w, h);
  return 0;
}

static int cdlua5_getbitmap(lua_State *L)
{
  cdBitmap *bitmap = cdlua_checkbitmap(L, 1);
  int x = luaL_checkint(L, 2);
  int y = luaL_checkint(L, 3);
  cdGetBitmap(bitmap, x, y);
  return 0;
}

/***************************************************************************\
* Server Images.                                                            *
\***************************************************************************/

static int cdlua5_createimage(lua_State *L)
{
  cdImage *image;
  int width = luaL_checkint(L, 1);
  int height = luaL_checkint(L, 2);

  if (width < 1 || height < 1)
    luaL_argerror(L, 1, "image dimensions should be positive integers");

  image = cdCreateImage(width, height);
  if (image) 
    cdlua_pushimage(L, image);
  else
    lua_pushnil(L);

  return 1;
}

static int cdlua5_getimage(lua_State *L)
{
  cdImage* image = cdlua_checkimage(L, 1);
  int x = luaL_checkint(L, 2);
  int y = luaL_checkint(L, 3);
  cdGetImage(image, x, y);
  return 0;
}

static int cdlua5_putimagerect(lua_State *L)
{
  cdImage* image = cdlua_checkimage(L, 1);
  int x = luaL_checkint(L, 2);
  int y = luaL_checkint(L, 3);
  int xmin = luaL_checkint(L, 4);
  int xmax = luaL_checkint(L, 5);
  int ymin = luaL_checkint(L, 6);
  int ymax = luaL_checkint(L, 7);
  cdPutImageRect(image, x, y, xmin, xmax, ymin, ymax);
  return 0;
}

static int wdlua5_putimagerect(lua_State *L)
{
  cdImage* image = cdlua_checkimage(L, 1);
  double x = luaL_checknumber(L, 2);
  double y = luaL_checknumber(L, 3);
  int xmin = luaL_checkint(L, 4);
  int xmax = luaL_checkint(L, 5);
  int ymin = luaL_checkint(L, 6);
  int ymax = luaL_checkint(L, 7);
  wdPutImageRect(image, x, y, xmin, xmax, ymin, ymax);
  return 0;
}

/***************************************************************************\
* cd.ScrollArea(xmin, xmax, ymin, ymax, dx, dy: number)                     *
\***************************************************************************/
static int cdlua5_scrollarea(lua_State *L)
{
  int xmin = luaL_checkint(L, 1);
  int xmax = luaL_checkint(L, 2);
  int ymin = luaL_checkint(L, 3);
  int ymax = luaL_checkint(L, 4);
  int dx = luaL_checkint(L, 5);
  int dy = luaL_checkint(L, 6);
  cdScrollArea(xmin, xmax, ymin, ymax, dx, dy);
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
  cdluaContext* cdlua_ctx = cdlua_getcontext(L, 1);
  int xmin = luaL_checkint(L,2);
  int xmax = luaL_checkint(L,3);
  int ymin = luaL_checkint(L,4);
  int ymax = luaL_checkint(L,5);
  const char *data_s = luaL_checkstring(L,6);

  cdlua_setplaystate(L);
  cdPlay(cdlua_ctx->ctx(), xmin, xmax, ymin, ymax, (void*)data_s);
  cdlua_setplaystate(NULL);
  return 0;
}

/***************************************************************************\
* cd.GetColorPlanes() -> (bpp: number)                                      *
\***************************************************************************/
static int cdlua5_getcolorplanes(lua_State *L)
{
  lua_pushnumber(L, cdGetColorPlanes());
  return 1;
}

static int cdlua5_palette(lua_State *L)
{
  cdluaPalette *pal = cdlua_checkpalette(L, 1);
  int mode_i = luaL_checkint(L, 2);
  cdPalette(pal->count, pal->color, mode_i);
  return 0;
}

/***************************************************************************\
* cd.ImageRGB                                                               *
\***************************************************************************/
static int cdlua5_imagergb(lua_State *L)
{
  cdCanvas *current_canvas;
  int w, h, type = CD_RGB;

  cdCanvas* canvas = cdlua_checkcanvas(L, 1);

  if (cdCanvasGetContext(canvas) != CD_IMAGERGB)
    luaL_argerror(L, 1, "invalid canvas, must be CD_IMAGERGB");

  if (cdAlphaImage(canvas))
    type = CD_RGBA;

  current_canvas = cdActiveCanvas();
  cdActivate(canvas);
  cdGetCanvasSize(&w, &h, NULL, NULL);
  cdActivate(current_canvas);

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
  cdCanvas *current_canvas;
  int w, h, type = CD_RGB;

  cdCanvas* canvas = cdlua_checkcanvas(L, 1);

  if (cdCanvasGetContext(canvas) != CD_IMAGERGB)
    luaL_argerror(L, 1, "invalid canvas, must be CD_IMAGERGB");

  if (cdAlphaImage(canvas))
    type = CD_RGBA;

  current_canvas = cdActiveCanvas();
  cdActivate(canvas);
  cdGetCanvasSize(&w, &h, NULL, NULL);
  cdActivate(current_canvas);

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

static void wdlua5_hardcopy_func(void) 
{
  lua_State* L = wdlua5_hardcopy_luaState;
  lua_pushvalue(L, 4); /* push the function in the stack */
  if(lua_pcall(L, 0, 0, 0) != 0)
    luaL_error(L, "error running function: %s", lua_tostring(L, -1));
}

static int wdlua5_hardcopy(lua_State *L) 
{
  cdluaContext* cdlua_ctx = cdlua_getcontext(L, 1);
  void *data_p = cdlua_ctx->checkdata(L, 2);
  cdCanvas* canvas = cdlua_checkcanvas(L, 3);
  luaL_argcheck(L, !lua_isfunction(L, 4), 4, "invalid draw function");

  wdlua5_hardcopy_luaState = L;
  wdHardcopy(cdlua_ctx->ctx(), data_p, canvas, wdlua5_hardcopy_func);

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
  cdBegin(luaL_checkint(L, 1));
  return 0;
}

/***************************************************************************\
* cd.Vertex(x, y: number)                                                   *
\***************************************************************************/
static int cdlua5_vertex(lua_State *L)
{
  cdVertex(luaL_checkint(L, 1), luaL_checkint(L, 2));
  return 0;
}

/***************************************************************************\
* cd.wVertex(x, y: number)                                                  *
\***************************************************************************/
static int wdlua5_vertex(lua_State *L)
{
  wdVertex(luaL_checknumber(L, 1), luaL_checknumber(L, 2));
  return 0;
}

/***************************************************************************\
* cd.End()                                                                  *
\***************************************************************************/
static int cdlua5_end(lua_State *L)
{
  (void)L;
  cdEnd();
  return 0;
}


/********************************************************************************\
* CDLua Exported functions                                                       *
\********************************************************************************/
static const struct luaL_reg cdlib_active[] = {

  /* Initialization */
  {"Activate"      , cdlua5_activate},
  {"ActiveCanvas"  , cdlua5_activecanvas},
  {"Simulate"      , cdlua5_simulate},

  /* Control */
  {"Clear"         , cdlua5_clear},
  {"Flush"         , cdlua5_flush},
  {"SaveState"     , cdlua5_savestate},
  {"RestoreState"  , cdlua5_restorestate},
  {"SetAttribute"  , cdlua5_setattribute},
  {"GetAttribute"  , cdlua5_getattribute},

  /* Coordinate System */
  {"GetCanvasSize" , cdlua5_getcanvassize},
  {"UpdateYAxis"   , cdlua5_updateyaxis},
  {"MM2Pixel"      , cdlua5_mm2pixel},
  {"Pixel2MM"      , cdlua5_pixel2mm},
  {"Origin"        , cdlua5_origin},

  /* World Coordinates */
  {"wWindow"        , wdlua5_window},
  {"wGetWindow"     , wdlua5_getwindow},
  {"wViewport"      , wdlua5_viewport},
  {"wGetViewport"   , wdlua5_getviewport},  
  {"wWorld2Canvas"  , wdlua5_world2canvas},
  {"wCanvas2World"  , wdlua5_canvas2world},

  {"wHardcopy"      , wdlua5_hardcopy},

  /* General Attributes */
  {"Foreground"  , cdlua5_foreground},
  {"Background"  , cdlua5_background},  
  {"WriteMode"   , cdlua5_writemode},

  /* Clipping */
  {"Clip"          , cdlua5_clip},
  {"ClipArea"      , cdlua5_cliparea},
  {"wClipArea"     , wdlua5_cliparea},
  {"GetClipArea"   , cdlua5_getcliparea},
  {"wGetClipArea"  , wdlua5_getcliparea},  
  {"GetClipPoly"   , cdlua5_getclippoly},
  {"wGetClipPoly"  , wdlua5_getclippoly},

  /* Regions */
  {"RegionCombineMode" , cdlua5_regioncombinemode},
  {"PointInRegion"     , cdlua5_pointinregion},
  {"wPointInRegion"    , wdlua5_pointinregion},
  {"OffsetRegion"      , cdlua5_offsetregion},
  {"wOffsetRegion"     , wdlua5_offsetregion},
  {"RegionBox"         , cdlua5_regionbox},
  {"wRegionBox"        , wdlua5_regionbox},

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
  {"Rect"            , cdlua5_rect},
  {"wRect"           , wdlua5_rect},
  {"Arc"             , cdlua5_arc},
  {"wArc"            , wdlua5_arc},
  {"LineStyle"       , cdlua5_linestyle},
  {"LineStyleDashes" , cdlua5_linestyledashes},
  {"LineWidth"       , cdlua5_linewidth},
  {"wLineWidth"      , wdlua5_linewidth},
  {"LineJoin"        , cdlua5_linejoin},
  {"LineCap"         , cdlua5_linecap},

  /* Filled Areas */
  {"Box"           , cdlua5_box},
  {"wBox"          , wdlua5_box},
  {"Sector"        , cdlua5_sector},
  {"wSector"       , wdlua5_sector},
  {"Chord"         , cdlua5_chord},  
  {"wChord"        , wdlua5_chord},
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
  {"Font"            , cdlua5_font},
  {"wFont"           , wdlua5_font},
  {"GetFont"         , cdlua5_getfont},
  {"wGetFont"        , wdlua5_getfont},
  {"NativeFont"      , cdlua5_nativefont},
  {"TextAlignment"   , cdlua5_textalignment},
  {"TextOrientation" , cdlua5_textorientation},
  {"FontDim"         , cdlua5_fontdim},
  {"wFontDim"        , wdlua5_fontdim},
  {"TextSize"        , cdlua5_textsize},
  {"wTextSize"       , wdlua5_textsize},
  {"TextBox"         , cdlua5_textbox},
  {"wTextBox"        , wdlua5_textbox},
  {"TextBounds"      , cdlua5_textbounds},
  {"wTextBounds"     , wdlua5_textbounds},

  /* Vector Text */
  {"VectorText"           , cdlua5_vectortext},
  {"wVectorText"          , wdlua5_vectortext},
  {"MultiLineVectorText"  , cdlua5_multilinevectortext},
  {"wMultiLineVectorText" , wdlua5_multilinevectortext},
  {"VectorTextDirection"  , cdlua5_vectortextdirection},
  {"wVectorTextDirection" , wdlua5_vectortextdirection},
  {"VectorTextTransform"  , cdlua5_vectortexttransform},
  {"VectorTextSize"       , cdlua5_vectortextsize},
  {"wVectorTextSize"      , wdlua5_vectortextsize},
  {"VectorCharSize"       , cdlua5_vectorcharsize},
  {"wVectorCharSize"      , wdlua5_vectorcharsize},
  {"VectorFont"           , cdlua5_vectorfont},
  {"GetVectorTextSize"    , cdlua5_getvectortextsize},
  {"wGetVectorTextSize"   , wdlua5_getvectortextsize},
  {"VectorTextBounds"     , cdlua5_vectortextbounds},
  {"wVectorTextBounds"    , wdlua5_vectortextbounds},  
  
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

  {"ImageRGB"         , cdlua5_imagergb},
  {"ImageRGBBitmap"   , cdlua5_imagergbbitmap},
  
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
  {"Vertex"        , cdlua5_vertex},
  {"wVertex"       , wdlua5_vertex},
  {"End"           , cdlua5_end},

  {NULL, NULL},
};

typedef struct cdlua5_constant {
  const char *name;
  lua_Number value;
} cdlua5_constant;

/* old backward compatible constants */
static const struct cdlua5_constant cdlibconstant[] = {
  {"SYSTEM",      CD_SYSTEM},        
  {"COURIER",     CD_COURIER},       
  {"TIMES_ROMAN", CD_TIMES_ROMAN},   
  {"HELVETICA",   CD_HELVETICA},     
  {"NATIVE",      CD_NATIVE},
  {"CLIPON",      CD_CLIPON},      
  {"CENTER_BASE", CD_CENTER_BASE},
  {"LEFT_BASE",   CD_LEFT_BASE},
  {"RIGHT_BASE",  CD_RIGHT_BASE},  
  {"ITALIC_BOLD", CD_ITALIC_BOLD}, 

  {NULL, -1}
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

void cdlua_open_active (lua_State *L, cdluaLuaState* cdL)
{                                  
  /* "cd" table is at the top of the stack */
  luaL_register(L, NULL, cdlib_active);
  initconst(L);

  cdL->void_canvas = cdCreateCanvas(CD_VOID, NULL);
  cdlua_setvoidstate(cdL->void_canvas, L);
  cdActivate(cdL->void_canvas);
}
