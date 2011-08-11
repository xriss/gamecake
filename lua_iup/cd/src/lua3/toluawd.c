#include <stdlib.h>
#include <stdio.h>

#include <lua.h>
#include <lauxlib.h>

#include "cd.h"
#include "wd.h"

#include "cdlua.h"
#include "cdlua3_private.h"


static void L_wdWindow(void)
{
 double xmin = (double)luaL_check_number(1);
 double xmax = (double)luaL_check_number(2);
 double ymin = (double)luaL_check_number(3);
 double ymax = (double)luaL_check_number(4);
 wdWindow(xmin,xmax,ymin,ymax);
}

static void L_wdViewport(void)
{
 int xmin = (int)luaL_check_number(1);
 int xmax = (int)luaL_check_number(2);
 int ymin = (int)luaL_check_number(3);
 int ymax = (int)luaL_check_number(4);
 wdViewport(xmin,xmax,ymin,ymax);
}

static void L_wdClipArea(void)
{
 double xmin = (double)luaL_check_number(1);
 double xmax = (double)luaL_check_number(2);
 double ymin = (double)luaL_check_number(3);
 double ymax = (double)luaL_check_number(4);
 wdClipArea(xmin,xmax,ymin,ymax);
}

static void L_wdLine(void)
{
 double x1 = (double)luaL_check_number(1);
 double y1 = (double)luaL_check_number(2);
 double x2 = (double)luaL_check_number(3);
 double y2 = (double)luaL_check_number(4);
 wdLine(x1,y1,x2,y2);
}

static void L_wdBox(void)
{
 double xmin = (double)luaL_check_number(1);
 double xmax = (double)luaL_check_number(2);
 double ymin = (double)luaL_check_number(3);
 double ymax = (double)luaL_check_number(4);
 wdBox(xmin,xmax,ymin,ymax);
}

static void L_wdRect(void)
{
 double xmin = (double)luaL_check_number(1);
 double xmax = (double)luaL_check_number(2);
 double ymin = (double)luaL_check_number(3);
 double ymax = (double)luaL_check_number(4);
 wdRect(xmin,xmax,ymin,ymax);
}

static void L_wdArc(void)
{
 double xc = (double)luaL_check_number(1);
 double yc = (double)luaL_check_number(2);
 double w = (double)luaL_check_number(3);
 double h = (double)luaL_check_number(4);
 double angle1 = (double)luaL_check_number(5);
 double angle2 = (double)luaL_check_number(6);
 wdArc(xc,yc,w,h,angle1,angle2);
}

static void L_wdSector(void)
{
 double xc = (double)luaL_check_number(1);
 double yc = (double)luaL_check_number(2);
 double w = (double)luaL_check_number(3);
 double h = (double)luaL_check_number(4);
 double angle1 = (double)luaL_check_number(5);
 double angle2 = (double)luaL_check_number(6);
 wdSector(xc,yc,w,h,angle1,angle2);
}

static void L_wdChord(void)
{
 double xc = (double)luaL_check_number(1);
 double yc = (double)luaL_check_number(2);
 double w = (double)luaL_check_number(3);
 double h = (double)luaL_check_number(4);
 double angle1 = (double)luaL_check_number(5);
 double angle2 = (double)luaL_check_number(6);
 wdChord(xc,yc,w,h,angle1,angle2);
}

static void L_wdText(void)
{
 double x = (double)luaL_check_number(1);
 double y = (double)luaL_check_number(2);
 char* s = (char*)luaL_check_string(3);
 wdText(x,y,s);
}

static void L_wdVertex(void)
{
 double x = (double)luaL_check_number(1);
 double y = (double)luaL_check_number(2);
 wdVertex(x,y);
}

static void L_wdMark(void)
{
 double x = (double)luaL_check_number(1);
 double y = (double)luaL_check_number(2);
 wdMark(x,y);
}

static void L_wdOffsetRegion(void)
{
 double x = (double)luaL_check_number(1);
 double y = (double)luaL_check_number(2);
 wdOffsetRegion(x,y);
}

static void L_wdPointInRegion(void)
{
 double x = (double)luaL_check_number(1);
 double y = (double)luaL_check_number(2);
 int L_result = wdPointInRegion(x,y);
 lua_pushnumber((double)L_result);
}

static void L_wdLineWidth(void)
{
 double width = (double)luaL_check_number(1);
 double L_result = wdLineWidth(width);
 lua_pushnumber(L_result);
}

static void L_wdFont(void)
{
 int type_face = (int)luaL_check_number(1);
 int style = (int)luaL_check_number(2);
 double size = (double)luaL_check_number(3);
 wdFont(type_face,style,size);
}

static void L_wdMarkSize(void)
{
 double size = (double)luaL_check_number(1);
 double L_result = wdMarkSize(size);
 lua_pushnumber(L_result);
}

static void L_wdVectorTextDirection(void)
{
 double x1 = (double)luaL_check_number(1);
 double y1 = (double)luaL_check_number(2);
 double x2 = (double)luaL_check_number(3);
 double y2 = (double)luaL_check_number(4);
 wdVectorTextDirection(x1,y1,x2,y2);
}

static void L_wdVectorTextSize(void)
{
 double size_x = (double)luaL_check_number(1);
 double size_y = (double)luaL_check_number(2);
 char* s = (char*)luaL_check_string(3);
 wdVectorTextSize(size_x,size_y,s);
}

static void L_wdVectorCharSize(void)
{
 double size = (double)luaL_check_number(1);
 double L_result = wdVectorCharSize(size);
 lua_pushnumber(L_result);
}

static void L_wdVectorText(void)
{
 double x = (double)luaL_check_number(1);
 double y = (double)luaL_check_number(2);
 char* s = (char*)luaL_check_string(3);
 wdVectorText(x,y,s);
}

static void L_wdMultiLineVectorText(void)
{
 double x = (double)luaL_check_number(1);
 double y = (double)luaL_check_number(2);
 char* s = (char*)luaL_check_string(3);
 wdMultiLineVectorText(x,y,s);
}


/* ---------------------------------------- public interface */
int luaL_wd_open(void)
{
 cdlua_register("wdWindow",L_wdWindow);
 cdlua_register("wdViewport",L_wdViewport);
 cdlua_register("wdClipArea",L_wdClipArea);
 cdlua_register("wdLine",L_wdLine);
 cdlua_register("wdBox",L_wdBox);
 cdlua_register("wdRect",L_wdRect);
 cdlua_register("wdArc",L_wdArc);
 cdlua_register("wdSector",L_wdSector);
 cdlua_register("wdChord",L_wdChord);
 cdlua_register("wdText",L_wdText);
 cdlua_register("wdVertex",L_wdVertex);
 cdlua_register("wdMark",L_wdMark);
 cdlua_register("wdOffsetRegion",L_wdOffsetRegion);
 cdlua_register("wdPointInRegion",L_wdPointInRegion);
 cdlua_register("wdLineWidth",L_wdLineWidth);
 cdlua_register("wdFont",L_wdFont);
 cdlua_register("wdMarkSize",L_wdMarkSize);
 cdlua_register("wdVectorTextDirection",L_wdVectorTextDirection);
 cdlua_register("wdVectorTextSize",L_wdVectorTextSize);
 cdlua_register("wdVectorCharSize",L_wdVectorCharSize);
 cdlua_register("wdVectorText",L_wdVectorText);
 cdlua_register("wdMultiLineVectorText",L_wdMultiLineVectorText);
 return 1;
}

