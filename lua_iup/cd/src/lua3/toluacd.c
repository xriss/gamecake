#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <lua.h>
#include <lauxlib.h>

#include "cd.h"
#include "cdps.h"

#include "cdlua.h"
#include "cdlua3_private.h"


static void L_cdFlush(void)
{
 cdFlush();
}

static void L_cdClear(void)
{
 cdClear();
}

static void L_cdSimulate(void)
{
 int mode = (int)luaL_check_number(1);
 int L_result = cdSimulate(mode);
 lua_pushnumber(L_result);
}

static void L_cdOrigin(void)
{
 int x = (int)luaL_check_number(1);
 int y = (int)luaL_check_number(2);
 cdOrigin(x,y);
}

static void L_cdClip(void)
{
 int mode = (int)luaL_check_number(1);
 int L_result = cdClip(mode);
 lua_pushnumber(L_result);
}

static void L_cdClipArea(void)
{
 int xmin = (int)luaL_check_number(1);
 int xmax = (int)luaL_check_number(2);
 int ymin = (int)luaL_check_number(3);
 int ymax = (int)luaL_check_number(4);
 cdClipArea(xmin,xmax,ymin,ymax);
}

static void L_cdLine(void)
{
 int x1 = (int)luaL_check_number(1);
 int y1 = (int)luaL_check_number(2);
 int x2 = (int)luaL_check_number(3);
 int y2 = (int)luaL_check_number(4);
 cdLine(x1,y1,x2,y2);
}

static void L_cdBox(void)
{
 int xmin = (int)luaL_check_number(1);
 int xmax = (int)luaL_check_number(2);
 int ymin = (int)luaL_check_number(3);
 int ymax = (int)luaL_check_number(4);
 cdBox(xmin,xmax,ymin,ymax);
}

static void L_cdRect(void)
{
 int xmin = (int)luaL_check_number(1);
 int xmax = (int)luaL_check_number(2);
 int ymin = (int)luaL_check_number(3);
 int ymax = (int)luaL_check_number(4);
 cdRect(xmin,xmax,ymin,ymax);
}

static void L_cdArc(void)
{
 int xc = (int)luaL_check_number(1);
 int yc = (int)luaL_check_number(2);
 int w = (int)luaL_check_number(3);
 int h = (int)luaL_check_number(4);
 double angle1 = (double)luaL_check_number(5);
 double angle2 = (double)luaL_check_number(6);
 cdArc(xc,yc,w,h,angle1,angle2);
}

static void L_cdSector(void)
{
 int xc = (int)luaL_check_number(1);
 int yc = (int)luaL_check_number(2);
 int w = (int)luaL_check_number(3);
 int h = (int)luaL_check_number(4);
 double angle1 = (double)luaL_check_number(5);
 double angle2 = (double)luaL_check_number(6);
 cdSector(xc,yc,w,h,angle1,angle2);
}

static void L_cdChord(void)
{
 int xc = (int)luaL_check_number(1);
 int yc = (int)luaL_check_number(2);
 int w = (int)luaL_check_number(3);
 int h = (int)luaL_check_number(4);
 double angle1 = (double)luaL_check_number(5);
 double angle2 = (double)luaL_check_number(6);
 cdChord(xc,yc,w,h,angle1,angle2);
}

static void L_cdText(void)
{
 int x = (int)luaL_check_number(1);
 int y = (int)luaL_check_number(2);
 char* s = (char*)luaL_check_string(3);
 cdText(x,y,s);
}

static void L_cdBegin(void)
{
 int mode = (int)luaL_check_number(1);
 cdBegin(mode);
}

static void L_cdVertex(void)
{
 int x = (int)luaL_check_number(1);
 int y = (int)luaL_check_number(2);
 cdVertex(x,y);
}

static void L_cdEnd(void)
{
 cdEnd();
}

static void L_cdMark(void)
{
 int x = (int)luaL_check_number(1);
 int y = (int)luaL_check_number(2);
 cdMark(x,y);
}

static void L_cdOffsetRegion(void)
{
 int x = (int)luaL_check_number(1);
 int y = (int)luaL_check_number(2);
 cdOffsetRegion(x,y);
}

static void L_cdPointInRegion(void)
{
 int x = (int)luaL_check_number(1);
 int y = (int)luaL_check_number(2);
 int L_result = cdPointInRegion(x,y);
 lua_pushnumber((int)L_result);
}

static void L_cdBackOpacity(void)
{
 int opacity = (int)luaL_check_number(1);
 int L_result = cdBackOpacity(opacity);
 lua_pushnumber(L_result);
}

static void L_cdWriteMode(void)
{
 int mode = (int)luaL_check_number(1);
 int L_result = cdWriteMode(mode);
 lua_pushnumber(L_result);
}

static void L_cdLineStyle(void)
{
 int style = (int)luaL_check_number(1);
 int L_result = cdLineStyle(style);
 lua_pushnumber(L_result);
}

static void L_cdLineWidth(void)
{
 int width = (int)luaL_check_number(1);
 int L_result = cdLineWidth(width);
 lua_pushnumber(L_result);
}

static void L_cdRegionCombineMode(void)
{
 int v = (int)luaL_check_number(1);
 int L_result = cdRegionCombineMode(v);
 lua_pushnumber(L_result);
}

static void L_cdLineJoin(void)
{
 int v = (int)luaL_check_number(1);
 int L_result = cdLineJoin(v);
 lua_pushnumber(L_result);
}

static void L_cdLineCap(void)
{
 int v = (int)luaL_check_number(1);
 int L_result = cdLineCap(v);
 lua_pushnumber(L_result);
}

static void L_cdFillMode(void)
{
 int v = (int)luaL_check_number(1);
 int L_result = cdFillMode(v);
 lua_pushnumber(L_result);
}

static void L_cdInteriorStyle(void)
{
 int style = (int)luaL_check_number(1);
 int L_result = cdInteriorStyle(style);
 lua_pushnumber(L_result);
}

static void L_cdHatch(void)
{
 int style = (int)luaL_check_number(1);
 int L_result = cdHatch(style);
 lua_pushnumber(L_result);
}

static void L_cdFont(void)
{
 int type_face = (int)luaL_check_number(1);
 int style = (int)luaL_check_number(2);
 int size = (int)luaL_check_number(3);
 cdFont(type_face,style,size);
}

static void L_cdNativeFont(void)
{
 char* font = (char*)luaL_check_string(1);
 lua_pushstring(cdNativeFont(font));
}

static int cdlua_isuserdata(char* name)
{
  if (strcmp(name, "HDC")==0) return 1;
  if (strcmp(name, "GC")==0) return 1;
  return 0;
}

static void L_cdSetAttribute(void)
{
  char* name = (char*)luaL_check_string(1);
  lua_Object p2 = lua_getparam(2);

  if (p2 == LUA_NOOBJECT)
    lua_error("cdSetAttribute: value parameter missing!");

  /* if p2 is nil */
  if (lua_isnil(p2)) 
  {
   cdSetAttribute(name, NULL);
  }
  else
  {
   char* data;
    if (cdlua_isuserdata(name))
      data = (char*) lua_getuserdata(p2);
    else
      data = (char*)luaL_check_string(2);
   cdSetAttribute(name, data);
  }
}

static void L_cdGetAttribute(void)
{
 char* name = (char*)luaL_check_string(1);
 char* data = cdGetAttribute(name);
 if (data)
 {
   if (cdlua_isuserdata(name))
    lua_pushuserdata(data);
   else
    lua_pushstring(data);
 }
 else
   lua_pushnil();
}

static void L_cdTextAlignment(void)
{
 int alignment = (int)luaL_check_number(1);
 int L_result = cdTextAlignment(alignment);
 lua_pushnumber(L_result);
}

static void L_cdTextOrientation(void)
{
 double angle = luaL_check_number(1);
 double L_result = cdTextOrientation(angle);
 lua_pushnumber(L_result);
}

static void L_cdMarkType(void)
{
 int type = (int)luaL_check_number(1);
 int L_result = cdMarkType(type);
 lua_pushnumber(L_result);
}

static void L_cdMarkSize(void)
{
 int size = (int)luaL_check_number(1);
 int L_result = cdMarkSize(size);
 lua_pushnumber(L_result);
}

static void L_cdGetColorPlanes(void)
{
 int L_result = cdGetColorPlanes();
 lua_pushnumber(L_result);
}

static void L_cdScrollArea(void)
{
 int xmin = (int)luaL_check_number(1);
 int xmax = (int)luaL_check_number(2);
 int ymin = (int)luaL_check_number(3);
 int ymax = (int)luaL_check_number(4);
 int dx = (int)luaL_check_number(5);
 int dy = (int)luaL_check_number(6);
 cdScrollArea(xmin,xmax,ymin,ymax,dx,dy);
}

static void L_cdVectorFont(void)
{
 char* file = (char*)luaL_check_string(1);
 char* L_result = cdVectorFont(file);
 lua_pushstring(L_result);
}

static void L_cdVectorTextDirection(void)
{
 int x1 = (int)luaL_check_number(1);
 int y1 = (int)luaL_check_number(2);
 int x2 = (int)luaL_check_number(3);
 int y2 = (int)luaL_check_number(4);
 cdVectorTextDirection(x1,y1,x2,y2);
}

static void L_cdVectorTextSize(void)
{
 int size_x = (int)luaL_check_number(1);
 int size_y = (int)luaL_check_number(2);
 char* s = (char*)luaL_check_string(3);
 cdVectorTextSize(size_x,size_y,s);
}

static void L_cdVectorCharSize(void)
{
 int size = (int)luaL_check_number(1);
 int L_result = cdVectorCharSize(size);
 lua_pushnumber(L_result);
}

static void L_cdVectorText(void)
{
 int x = (int)luaL_check_number(1);
 int y = (int)luaL_check_number(2);
 char* s = (char*)luaL_check_string(3);
 cdVectorText(x,y,s);
}

static void L_cdMultiLineVectorText(void)
{
 int x = (int)luaL_check_number(1);
 int y = (int)luaL_check_number(2);
 char* s = (char*)luaL_check_string(3);
 cdMultiLineVectorText(x,y,s);
}


/* ---------------------------------------- public interface */
int luaL_cd_open(void)
{
 cdlua_pushnumber(CD_CAP_NONE, "CD_CAP_NONE"); 
 cdlua_pushnumber(CD_CAP_FLUSH, "CD_CAP_FLUSH"); 
 cdlua_pushnumber(CD_CAP_CLEAR, "CD_CAP_CLEAR"); 
 cdlua_pushnumber(CD_CAP_PLAY, "CD_CAP_PLAY"); 
 cdlua_pushnumber(CD_CAP_YAXIS, "CD_CAP_YAXIS"); 
 cdlua_pushnumber(CD_CAP_CLIPAREA, "CD_CAP_CLIPAREA"); 
 cdlua_pushnumber(CD_CAP_CLIPPOLY, "CD_CAP_CLIPPOLY"); 
 cdlua_pushnumber(CD_CAP_RECT, "CD_CAP_RECT"); 
 cdlua_pushnumber(CD_CAP_IMAGERGB, "CD_CAP_IMAGERGB"); 
 cdlua_pushnumber(CD_CAP_IMAGERGBA, "CD_CAP_IMAGERGBA"); 
 cdlua_pushnumber(CD_CAP_IMAGEMAP, "CD_CAP_IMAGEMAP"); 
 cdlua_pushnumber(CD_CAP_GETIMAGERGB, "CD_CAP_GETIMAGERGB"); 
 cdlua_pushnumber(CD_CAP_IMAGESRV, "CD_CAP_IMAGESRV"); 
 cdlua_pushnumber(CD_CAP_BACKGROUND, "CD_CAP_BACKGROUND"); 
 cdlua_pushnumber(CD_CAP_BACKOPACITY, "CD_CAP_BACKOPACITY"); 
 cdlua_pushnumber(CD_CAP_WRITEMODE, "CD_CAP_WRITEMODE"); 
 cdlua_pushnumber(CD_CAP_LINESTYLE, "CD_CAP_LINESTYLE"); 
 cdlua_pushnumber(CD_CAP_LINEWITH, "CD_CAP_LINEWITH"); 
 cdlua_pushnumber(CD_CAP_FPRIMTIVES, "CD_CAP_FPRIMTIVES"); 
 cdlua_pushnumber(CD_CAP_HATCH, "CD_CAP_HATCH"); 
 cdlua_pushnumber(CD_CAP_STIPPLE, "CD_CAP_STIPPLE"); 
 cdlua_pushnumber(CD_CAP_PATTERN, "CD_CAP_PATTERN"); 
 cdlua_pushnumber(CD_CAP_FONT, "CD_CAP_FONT"); 
 cdlua_pushnumber(CD_CAP_FONTDIM, "CD_CAP_FONTDIM"); 
 cdlua_pushnumber(CD_CAP_TEXTSIZE, "CD_CAP_TEXTSIZE"); 
 cdlua_pushnumber(CD_CAP_TEXTORIENTATION, "CD_CAP_TEXTORIENTATION"); 
 cdlua_pushnumber(CD_CAP_PALETTE, "CD_CAP_PALETTE"); 
 cdlua_pushnumber(CD_CAP_ALL, "CD_CAP_ALL"); 
 cdlua_pushnumber(CD_CAP_LINECAP, "CD_CAP_LINECAP"); 
 cdlua_pushnumber(CD_CAP_LINEJOIN, "CD_CAP_LINEJOIN"); 
 cdlua_pushnumber(CD_CAP_REGION, "CD_CAP_REGION"); 
 cdlua_pushnumber(CD_CAP_CHORD, "CD_CAP_CHORD"); 
 cdlua_pushnumber(CD_SIM_FILLS, "CD_SIM_FILLS");
 cdlua_pushnumber(CD_SIM_LINES, "CD_SIM_LINES");
 cdlua_pushnumber(CD_SIM_ALL, "CD_SIM_ALL");
 cdlua_pushnumber(CD_SIM_POLYGON, "CD_SIM_POLYGON");
 cdlua_pushnumber(CD_SIM_SECTOR, "CD_SIM_SECTOR");
 cdlua_pushnumber(CD_SIM_POLYLINE, "CD_SIM_POLYLINE");
 cdlua_pushnumber(CD_SIM_BOX, "CD_SIM_BOX");
 cdlua_pushnumber(CD_SIM_ARC, "CD_SIM_ARC");
 cdlua_pushnumber(CD_SIM_RECT, "CD_SIM_RECT");
 cdlua_pushnumber(CD_SIM_LINE, "CD_SIM_LINE");
 cdlua_pushnumber(CD_SIM_NONE, "CD_SIM_NONE");
 cdlua_pushnumber(CD_SIM_CHORD, "CD_SIM_CHORD");
 cdlua_pushnumber(CD_QUERY, "CD_QUERY");
 cdlua_pushnumber(CD_ERROR, "CD_ERROR");
 cdlua_pushnumber(CD_OK, "CD_OK");
 cdlua_pushnumber(CD_CLIPOFF, "CD_CLIPOFF");
 cdlua_pushnumber(CD_CLIPAREA, "CD_CLIPAREA");
 cdlua_pushnumber(CD_CLIPPOLYGON, "CD_CLIPPOLYGON");
 cdlua_pushnumber(CD_CLIPREGION, "CD_CLIPREGION");
 cdlua_pushnumber(CD_FILL, "CD_FILL");
 cdlua_pushnumber(CD_OPEN_LINES, "CD_OPEN_LINES");
 cdlua_pushnumber(CD_CLOSED_LINES, "CD_CLOSED_LINES");
 cdlua_pushnumber(CD_CLIP, "CD_CLIP");
 cdlua_pushnumber(CD_BEZIER, "CD_BEZIER");
 cdlua_pushnumber(CD_OPAQUE, "CD_OPAQUE");
 cdlua_pushnumber(CD_TRANSPARENT, "CD_TRANSPARENT");
 cdlua_pushnumber(CD_REPLACE, "CD_REPLACE");
 cdlua_pushnumber(CD_XOR, "CD_XOR");
 cdlua_pushnumber(CD_NOT_XOR, "CD_NOT_XOR");
 cdlua_pushnumber(CD_POLITE, "CD_POLITE");
 cdlua_pushnumber(CD_FORCE, "CD_FORCE");
 cdlua_pushnumber(CD_CONTINUOUS, "CD_CONTINUOUS");
 cdlua_pushnumber(CD_DASHED, "CD_DASHED");
 cdlua_pushnumber(CD_DOTTED, "CD_DOTTED");
 cdlua_pushnumber(CD_DASH_DOT, "CD_DASH_DOT");
 cdlua_pushnumber(CD_DASH_DOT_DOT, "CD_DASH_DOT_DOT");
 cdlua_pushnumber(CD_PLUS, "CD_PLUS");
 cdlua_pushnumber(CD_STAR, "CD_STAR");
 cdlua_pushnumber(CD_CIRCLE, "CD_CIRCLE");
 cdlua_pushnumber(CD_X, "CD_X");
 cdlua_pushnumber(CD_BOX, "CD_BOX");
 cdlua_pushnumber(CD_DIAMOND, "CD_DIAMOND");
 cdlua_pushnumber(CD_HOLLOW_CIRCLE, "CD_HOLLOW_CIRCLE");
 cdlua_pushnumber(CD_HOLLOW_BOX, "CD_HOLLOW_BOX");
 cdlua_pushnumber(CD_HOLLOW_DIAMOND, "CD_HOLLOW_DIAMOND");
 cdlua_pushnumber(CD_HORIZONTAL, "CD_HORIZONTAL");
 cdlua_pushnumber(CD_VERTICAL, "CD_VERTICAL");
 cdlua_pushnumber(CD_FDIAGONAL, "CD_FDIAGONAL");
 cdlua_pushnumber(CD_BDIAGONAL, "CD_BDIAGONAL");
 cdlua_pushnumber(CD_CROSS, "CD_CROSS");
 cdlua_pushnumber(CD_DIAGCROSS, "CD_DIAGCROSS");
 cdlua_pushnumber(CD_SOLID, "CD_SOLID");
 cdlua_pushnumber(CD_HATCH, "CD_HATCH");
 cdlua_pushnumber(CD_STIPPLE, "CD_STIPPLE");
 cdlua_pushnumber(CD_PATTERN, "CD_PATTERN");
 cdlua_pushnumber(CD_HOLLOW, "CD_HOLLOW");
 cdlua_pushnumber(CD_NORTH, "CD_NORTH");
 cdlua_pushnumber(CD_SOUTH, "CD_SOUTH");
 cdlua_pushnumber(CD_EAST, "CD_EAST");
 cdlua_pushnumber(CD_WEST, "CD_WEST");
 cdlua_pushnumber(CD_NORTH_EAST, "CD_NORTH_EAST");
 cdlua_pushnumber(CD_NORTH_WEST, "CD_NORTH_WEST");
 cdlua_pushnumber(CD_SOUTH_EAST, "CD_SOUTH_EAST");
 cdlua_pushnumber(CD_SOUTH_WEST, "CD_SOUTH_WEST");
 cdlua_pushnumber(CD_CENTER, "CD_CENTER");
 cdlua_pushnumber(CD_BASE_LEFT, "CD_BASE_LEFT");
 cdlua_pushnumber(CD_BASE_CENTER, "CD_BASE_CENTER");
 cdlua_pushnumber(CD_BASE_RIGHT, "CD_BASE_RIGHT");
 cdlua_pushnumber(CD_SYSTEM, "CD_SYSTEM");
 cdlua_pushnumber(CD_COURIER, "CD_COURIER");
 cdlua_pushnumber(CD_TIMES_ROMAN, "CD_TIMES_ROMAN");
 cdlua_pushnumber(CD_HELVETICA, "CD_HELVETICA");
 cdlua_pushnumber(CD_PLAIN, "CD_PLAIN");
 cdlua_pushnumber(CD_BOLD, "CD_BOLD");
 cdlua_pushnumber(CD_ITALIC, "CD_ITALIC");
 cdlua_pushnumber(CD_BOLD_ITALIC, "CD_BOLD_ITALIC");
 cdlua_pushnumber(CD_SMALL, "CD_SMALL");
 cdlua_pushnumber(CD_STANDARD, "CD_STANDARD");
 cdlua_pushnumber(CD_LARGE, "CD_LARGE");
 cdlua_pushnumber(CD_MM2PT, "CD_MM2PT");
 cdlua_pushnumber(CD_RAD2DEG, "CD_RAD2DEG");
 cdlua_pushnumber(CD_DEG2RAD, "CD_DEG2RAD");
 cdlua_pushnumber(CD_RGBA, "CD_RGBA");
 cdlua_pushnumber(CD_RGB, "CD_RGB");
 cdlua_pushnumber(CD_MAP, "CD_MAP");
 cdlua_pushnumber(CD_IRED, "CD_IRED");
 cdlua_pushnumber(CD_IGREEN, "CD_IGREEN");
 cdlua_pushnumber(CD_IBLUE, "CD_IBLUE");
 cdlua_pushnumber(CD_IALPHA, "CD_IALPHA");
 cdlua_pushnumber(CD_INDEX, "CD_INDEX");
 cdlua_pushnumber(CD_COLORS, "CD_COLORS");
 cdlua_pushnumber(CD_MAP, "CD_MAP");
 cdlua_pushnumber(CD_A0, "CD_A0");
 cdlua_pushnumber(CD_A2, "CD_A2");
 cdlua_pushnumber(CD_A3, "CD_A3");
 cdlua_pushnumber(CD_A1, "CD_A1");
 cdlua_pushnumber(CD_A4, "CD_A4");
 cdlua_pushnumber(CD_A5, "CD_A5");
 cdlua_pushnumber(CD_LETTER, "CD_LETTER");
 cdlua_pushnumber(CD_LEGAL, "CD_LEGAL");
 cdlua_pushnumber(CD_UNION, "CD_UNION");
 cdlua_pushnumber(CD_INTERSECT, "CD_INTERSECT");
 cdlua_pushnumber(CD_DIFFERENCE, "CD_DIFFERENCE");
 cdlua_pushnumber(CD_NOTINTERSECT, "CD_NOTINTERSECT");
 cdlua_pushnumber(CD_REGION, "CD_REGION");
 cdlua_pushnumber(CD_EVENODD, "CD_EVENODD");
 cdlua_pushnumber(CD_WINDING, "CD_WINDING");
 cdlua_pushnumber(CD_BEVEL, "CD_BEVEL");
 cdlua_pushnumber(CD_MITER, "CD_MITER");
 cdlua_pushnumber(CD_ROUND, "CD_ROUND");
 cdlua_pushnumber(CD_CAPROUND, "CD_CAPROUND");
 cdlua_pushnumber(CD_CAPSQUARE, "CD_CAPSQUARE");
 cdlua_pushnumber(CD_CAPFLAT, "CD_CAPFLAT");
 cdlua_pushnumber(CD_CUSTOM, "CD_CUSTOM");
 cdlua_pushnumber(CD_ABORT, "CD_ABORT");
 cdlua_pushnumber(CD_CONTINUE, "CD_CONTINUE");

 cdlua_register("cdFlush",L_cdFlush);
 cdlua_register("cdSimulate",L_cdSimulate);
 cdlua_register("cdOrigin",L_cdOrigin);
 cdlua_register("cdClear",L_cdClear);
 cdlua_register("cdClip",L_cdClip);
 cdlua_register("cdClipArea",L_cdClipArea);
 cdlua_register("cdLine",L_cdLine);
 cdlua_register("cdBox",L_cdBox);
 cdlua_register("cdRect",L_cdRect);
 cdlua_register("cdArc",L_cdArc);
 cdlua_register("cdSector",L_cdSector);
 cdlua_register("cdChord",L_cdChord);
 cdlua_register("cdText",L_cdText);
 cdlua_register("cdBegin",L_cdBegin);
 cdlua_register("cdVertex",L_cdVertex);
 cdlua_register("cdEnd",L_cdEnd);
 cdlua_register("cdOffsetRegion",L_cdOffsetRegion);
 cdlua_register("cdPointInRegion",L_cdPointInRegion);
 cdlua_register("cdMark",L_cdMark);
 cdlua_register("cdBackOpacity",L_cdBackOpacity);
 cdlua_register("cdWriteMode",L_cdWriteMode);
 cdlua_register("cdRegionCombineMode",L_cdRegionCombineMode);
 cdlua_register("cdLineJoin",L_cdLineJoin);
 cdlua_register("cdLineCap",L_cdLineCap);
 cdlua_register("cdFillMode",L_cdFillMode);
 cdlua_register("cdLineStyle",L_cdLineStyle);
 cdlua_register("cdLineWidth",L_cdLineWidth);
 cdlua_register("cdInteriorStyle",L_cdInteriorStyle);
 cdlua_register("cdHatch",L_cdHatch);
 cdlua_register("cdFont",L_cdFont);
 cdlua_register("cdNativeFont",L_cdNativeFont);
 cdlua_register("cdTextAlignment",L_cdTextAlignment);
 cdlua_register("cdTextOrientation",L_cdTextOrientation);
 cdlua_register("cdMarkType",L_cdMarkType);
 cdlua_register("cdMarkSize",L_cdMarkSize);
 cdlua_register("cdGetColorPlanes",L_cdGetColorPlanes);
 cdlua_register("cdScrollArea",L_cdScrollArea);
 cdlua_register("cdVectorFont",L_cdVectorFont);
 cdlua_register("cdVectorTextDirection",L_cdVectorTextDirection);
 cdlua_register("cdVectorTextSize",L_cdVectorTextSize);
 cdlua_register("cdVectorCharSize",L_cdVectorCharSize);
 cdlua_register("cdVectorText",L_cdVectorText);
 cdlua_register("cdMultiLineVectorText",L_cdMultiLineVectorText);
 cdlua_register("cdSetAttribute",L_cdSetAttribute);
 cdlua_register("cdGetAttribute",L_cdGetAttribute);
 return 1;
}

