/** \file
 * \brief IM Lua 5 Binding
 *
 * See Copyright Notice in im_lib.h
 * $Id: imlua_capture.c,v 1.5 2010/07/18 03:04:23 scuri Exp $
 */

#include <string.h>
#include <memory.h>

#include "im.h"
#include "im_image.h"
#include "im_capture.h"

#include <lua.h>
#include <lauxlib.h>

#include "imlua.h"
#include "imlua_aux.h"
#include "imlua_image.h"



static imVideoCapture** imlua_rawcheckvideocapture (lua_State *L, int param)
{
  return (imVideoCapture**)luaL_checkudata(L, param, "imVideoCapture");
}

static imVideoCapture* imlua_checkvideocapture (lua_State *L, int param)
{
  imVideoCapture** vc_p = imlua_rawcheckvideocapture(L, param);

  if (!(*vc_p))
    luaL_argerror(L, param, "destroyed imVideoCapture");

  return *vc_p;
}

static void imlua_pushvideocapture(lua_State *L, imVideoCapture* vc)
{
  if (!vc)
    lua_pushnil(L);
  else
  {
    imVideoCapture** vc_p = (imVideoCapture**) lua_newuserdata(L, sizeof(imVideoCapture*));
    *vc_p = vc;
    luaL_getmetatable(L, "imVideoCapture");
    lua_setmetatable(L, -2);
  }
}

/*****************************************************************************\
 im.VideoCaptureDeviceCount()
\*****************************************************************************/
static int imluaVideoCaptureDeviceCount (lua_State *L)
{
  lua_pushnumber(L, imVideoCaptureDeviceCount());
  return 1;
}

/*****************************************************************************\
 im.VideoCaptureDeviceDesc(device)
\*****************************************************************************/
static int imluaVideoCaptureDeviceDesc (lua_State *L)
{
  lua_pushstring(L, imVideoCaptureDeviceDesc(luaL_checkint(L, 1)));
  return 1;
}

/*****************************************************************************\
 im.VideoCaptureReloadDevices()
\*****************************************************************************/
static int imluaVideoCaptureReloadDevices (lua_State *L)
{
  lua_pushnumber(L, imVideoCaptureReloadDevices());
  return 1;
}

/*****************************************************************************\
 im.VideoCaptureReleaseDevices()
\*****************************************************************************/
static int imluaVideoCaptureReleaseDevices (lua_State *L)
{
  (void)L;
  imVideoCaptureReleaseDevices();
  return 0;
}

/*****************************************************************************\
 im.VideoCaptureCreate()
\*****************************************************************************/
static int imluaVideoCaptureCreate (lua_State *L)
{
  imlua_pushvideocapture(L, imVideoCaptureCreate());
  return 1;
}

/*****************************************************************************\
 vc:Connect([device])
\*****************************************************************************/
static int imluaVideoCaptureConnect (lua_State *L)
{
  imVideoCapture *vc = imlua_checkvideocapture(L, 1);
  int device = luaL_optint(L, 2, -1);
  if (device == -1)
    lua_pushnumber(L, imVideoCaptureConnect(vc, device));
  else
    lua_pushboolean(L, imVideoCaptureConnect(vc, device));
  return 1;
}

/*****************************************************************************\
 vc:Disconnect()
\*****************************************************************************/
static int imluaVideoCaptureDisconnect (lua_State *L)
{
  imVideoCaptureDisconnect(imlua_checkvideocapture(L, 1));
  return 0;
}

/*****************************************************************************\
 vc:DialogCount()
\*****************************************************************************/
static int imluaVideoCaptureDialogCount (lua_State *L)
{
  lua_pushnumber(L, imVideoCaptureDialogCount(imlua_checkvideocapture(L, 1)));
  return 1;
}

/*****************************************************************************\
 vc:ShowDialog()
\*****************************************************************************/
static int imluaVideoCaptureShowDialog (lua_State *L)
{
  imVideoCapture *vc = imlua_checkvideocapture(L, 1);
  int dialog = luaL_checkint(L, 2);
  void *parent = lua_touserdata(L, 3); 

  lua_pushboolean(L, imVideoCaptureShowDialog(vc, dialog, parent));
  return 1;
}

/*****************************************************************************\
 vc:DialogDesc()
\*****************************************************************************/
static int imluaVideoCaptureDialogDesc (lua_State *L)
{
  imVideoCapture *vc = imlua_checkvideocapture(L, 1);
  int dialog = luaL_checkint(L, 2);

  lua_pushstring(L, imVideoCaptureDialogDesc(vc, dialog));
  return 1;
}

/*****************************************************************************\
 vc:FormatCount()
\*****************************************************************************/
static int imluaVideoCaptureFormatCount (lua_State *L)
{
  lua_pushnumber(L, imVideoCaptureFormatCount(imlua_checkvideocapture(L, 1)));
  return 1;
}

/*****************************************************************************\
 vc:GetFormat()
\*****************************************************************************/
static int imluaVideoCaptureGetFormat (lua_State *L)
{
  imVideoCapture *vc = imlua_checkvideocapture(L, 1);
  int format = luaL_checkint(L, 2);
  int width, height;
  char desc[10];

  lua_pushboolean(L, imVideoCaptureGetFormat(vc, format, &width, &height, desc));
  lua_pushnumber(L, width);
  lua_pushnumber(L, height);
  lua_pushstring(L, desc);

  return 4;
}

/*****************************************************************************\
 vc:GetImageSize()
\*****************************************************************************/
static int imluaVideoCaptureGetImageSize (lua_State *L)
{
  imVideoCapture *vc = imlua_checkvideocapture(L, 1);
  int width, height;

  imVideoCaptureGetImageSize(vc, &width, &height);
  lua_pushnumber(L, width);
  lua_pushnumber(L, height);

  return 2;
}

/*****************************************************************************\
 vc:SetImageSize()
\*****************************************************************************/
static int imluaVideoCaptureSetImageSize (lua_State *L)
{
  imVideoCapture *vc = imlua_checkvideocapture(L, 1);
  int width = luaL_checkint(L, 2);
  int height = luaL_checkint(L, 3);

  lua_pushboolean(L, imVideoCaptureSetImageSize(vc, width, height));

  return 1;
}

/*****************************************************************************\
 vc:SetInOut()
\*****************************************************************************/
static int imluaVideoCaptureSetInOut(lua_State *L)
{
  imVideoCapture *vc = imlua_checkvideocapture(L, 1);
  int input = luaL_checkint(L, 2);
  int output = luaL_checkint(L, 3);
  int cross = luaL_checkint(L, 4);

  lua_pushboolean(L, imVideoCaptureSetInOut(vc, input, output, cross));

  return 1;
}

/*****************************************************************************\
 vc:SetFormat()
\*****************************************************************************/
static int imluaVideoCaptureSetFormat (lua_State *L)
{
  imVideoCapture *vc = imlua_checkvideocapture(L, 1);
  int format = luaL_optint(L, 2, -1);
  if (format == -1)
    lua_pushnumber(L, imVideoCaptureSetFormat(vc, format));
  else
    lua_pushboolean(L, imVideoCaptureSetFormat(vc, format));
  return 1;
}

/*****************************************************************************\
 vc:ResetAttribute(attrib, fauto)
\*****************************************************************************/
static int imluaVideoCaptureResetAttribute (lua_State *L)
{
  imVideoCapture *vc = imlua_checkvideocapture(L, 1);
  const char *attrib = luaL_checkstring(L, 2);
  int fauto = lua_toboolean(L, 3);

  lua_pushboolean(L, imVideoCaptureResetAttribute(vc, attrib, fauto));
  return 1;
}

/*****************************************************************************\
 vc:SetAttribute(attrib, percent)
\*****************************************************************************/
static int imluaVideoCaptureSetAttribute (lua_State *L)
{
  imVideoCapture *vc = imlua_checkvideocapture(L, 1);
  const char *attrib = luaL_checkstring(L, 2);
  float percent = (float) luaL_checknumber(L, 3);

  lua_pushboolean(L, imVideoCaptureSetAttribute(vc, attrib, percent));
  return 1;
}

/*****************************************************************************\
 vc:GetAttribute(attrib)
\*****************************************************************************/
static int imluaVideoCaptureGetAttribute (lua_State *L)
{
  imVideoCapture *vc = imlua_checkvideocapture(L, 1);
  const char *attrib = luaL_checkstring(L, 2);
  float percent;

  lua_pushboolean(L, imVideoCaptureGetAttribute(vc, attrib, &percent));
  lua_pushnumber(L, percent);
  return 2;
}

/*****************************************************************************\
 vc:GetAttributeList()
\*****************************************************************************/
static int imluaVideoCaptureGetAttributeList (lua_State *L)
{
  imVideoCapture *vc = imlua_checkvideocapture(L, 1);
  int num_attrib;
  const char **attribs;
  int i;

  attribs = imVideoCaptureGetAttributeList(vc, &num_attrib);
  lua_createtable(L, num_attrib, 0);
  for (i = 0; i < num_attrib; i++)
  {
    lua_pushstring(L, attribs[i]);
    lua_rawseti(L, -2, i + 1);
  }

  return 1;
}

/*****************************************************************************\
 vc:Frame(image)
\*****************************************************************************/
static int imluaVideoCaptureFrame (lua_State *L)
{
  imVideoCapture *vc = imlua_checkvideocapture(L, 1);
  imImage *image = imlua_checkimage(L, 2);
  int timeout = luaL_checkint(L, 3);

  if (!(image->color_space == IM_RGB || image->color_space == IM_GRAY))
    luaL_argerror(L, 2, "image must be of RGB or Gray color spaces");
  imlua_checkdatatype(L, 2, image, IM_BYTE);

  lua_pushboolean(L, imVideoCaptureFrame(vc, image->data[0], image->color_space, timeout));

  return 1;
}

/*****************************************************************************\
 vc:OneFrame(image)
\*****************************************************************************/
static int imluaVideoCaptureOneFrame (lua_State *L)
{
  imVideoCapture *vc = imlua_checkvideocapture(L, 1);
  imImage *image = imlua_checkimage(L, 2);

  if (!(image->color_space == IM_RGB || image->color_space == IM_GRAY))
    luaL_argerror(L, 2, "image must be of RGB or Gray color spaces");
  imlua_checkdatatype(L, 2, image, IM_BYTE);

  lua_pushboolean(L, imVideoCaptureOneFrame(vc, image->data[0], image->color_space));

  return 1;
}

/*****************************************************************************\
 vc:Live(image)
\*****************************************************************************/
static int imluaVideoCaptureLive (lua_State *L)
{
  imVideoCapture *vc = imlua_checkvideocapture(L, 1);
  int live = luaL_optint(L, 2, -1);
  if (live == -1)
    lua_pushnumber(L, imVideoCaptureLive(vc, live));
  else
    lua_pushboolean(L, imVideoCaptureLive(vc, live));

  return 1;
}

/*****************************************************************************\
 vc:Destroy()
\*****************************************************************************/
static int imluaVideoCaptureDestroy (lua_State *L)
{
  imVideoCapture **vc_p = imlua_rawcheckvideocapture(L, 1);
  if (!(*vc_p))
    luaL_argerror(L, 1, "destroyed imVideoCapture");

  imVideoCaptureDestroy(*vc_p);
  *vc_p = NULL;  /* mark as destroyed */

  return 0;
}

/*****************************************************************************\
 gc
\*****************************************************************************/
static int imluaVideoCapture_gc (lua_State *L)
{
  imVideoCapture **vc_p = (imVideoCapture **)lua_touserdata(L, 1);
  if (*vc_p)
  {
    imVideoCaptureDestroy(*vc_p);
    *vc_p = NULL;  /* mark as destroyed */
  }
  return 0;
}

/*****************************************************************************\
 tostring
\*****************************************************************************/
static int imluaVideoCapture_tostring (lua_State *L)
{
  imVideoCapture **vc_p = (imVideoCapture **)lua_touserdata(L, 1);
  lua_pushfstring(L, "imVideoCapture (%p)%s", vc_p, (*vc_p)? "": "-destroyed");
  return 1;
}

static const luaL_reg imcapture_lib[] = {
  {"VideoCaptureDeviceCount", imluaVideoCaptureDeviceCount},
  {"VideoCaptureDeviceDesc", imluaVideoCaptureDeviceDesc},
  {"VideoCaptureReloadDevices", imluaVideoCaptureReloadDevices},
  {"VideoCaptureReleaseDevices", imluaVideoCaptureReleaseDevices},
  {"VideoCaptureCreate", imluaVideoCaptureCreate},
  {"VideoCaptureDestroy", imluaVideoCaptureDestroy},
  {NULL, NULL}
};

static const luaL_reg imcapture_metalib[] = {
  {"Destroy", imluaVideoCaptureDestroy},
  {"Connect", imluaVideoCaptureConnect},
  {"Disconnect", imluaVideoCaptureDisconnect},
  {"DialogCount", imluaVideoCaptureDialogCount},
  {"ShowDialog", imluaVideoCaptureShowDialog},
  {"DialogDesc", imluaVideoCaptureDialogDesc},
  {"FormatCount", imluaVideoCaptureFormatCount},
  {"GetFormat", imluaVideoCaptureGetFormat},
  {"SetFormat", imluaVideoCaptureSetFormat},
  {"GetImageSize", imluaVideoCaptureGetImageSize},
  {"SetImageSize", imluaVideoCaptureSetImageSize},
  {"SetInOut", imluaVideoCaptureSetInOut},
  {"ResetAttribute", imluaVideoCaptureResetAttribute},
  {"GetAttribute", imluaVideoCaptureGetAttribute},
  {"SetAttribute", imluaVideoCaptureSetAttribute},
  {"GetAttributeList", imluaVideoCaptureGetAttributeList},
  {"OneFrame", imluaVideoCaptureOneFrame},
  {"Frame", imluaVideoCaptureFrame},
  {"Live", imluaVideoCaptureLive},

  {"__gc", imluaVideoCapture_gc},
  {"__tostring", imluaVideoCapture_tostring},

  {NULL, NULL}
};

static void createmeta (lua_State *L) 
{
  /* Object Oriented Access */
  luaL_newmetatable(L, "imVideoCapture");  /* create new metatable for imVideoCapture handle */
  lua_pushliteral(L, "__index");
  lua_pushvalue(L, -2);  /* push metatable */
  lua_rawset(L, -3);  /* metatable.__index = metatable */
  luaL_register(L, NULL, imcapture_metalib);  /* register methods */
  lua_pop(L, 1);  /* removes the metatable from the top of the stack */
}

int imlua_open_capture(lua_State *L)
{
  createmeta(L);
  luaL_register(L, "im", imcapture_lib);  /* leave "im" table at the top of the stack */
  return 1;
}

int luaopen_imlua_capture(lua_State *L)
{
  return imlua_open_capture(L);
}
