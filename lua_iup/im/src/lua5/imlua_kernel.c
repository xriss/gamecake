/** \file
 * \brief IM Lua 5 Binding
 *
 * See Copyright Notice in im_lib.h
 * $Id: imlua_kernel.c,v 1.1 2008/10/17 06:16:32 scuri Exp $
 */

#include <memory.h>
#include <math.h>
#include <stdlib.h>

#include "im.h"
#include "im_image.h"
#include "im_process.h"
#include "im_util.h"
#include "im_kernel.h"

#include <lua.h>
#include <lauxlib.h>

#include "imlua.h"
#include "imlua_aux.h"
#include "imlua_image.h"


static int imluaKernelSobel(lua_State *L)
{
  imlua_pushimage(L, imKernelSobel());
  return 1;
}

static int imluaKernelPrewitt(lua_State *L)
{
  imlua_pushimage(L, imKernelPrewitt());
  return 1;
}

static int imluaKernelKirsh(lua_State *L)
{
  imlua_pushimage(L, imKernelKirsh());
  return 1;
}

static int imluaKernelLaplacian4(lua_State *L)
{
  imlua_pushimage(L, imKernelLaplacian4());
  return 1;
}

static int imluaKernelLaplacian8(lua_State *L)
{
  imlua_pushimage(L, imKernelLaplacian8());
  return 1;
}

static int imluaKernelLaplacian5x5(lua_State *L)
{
  imlua_pushimage(L, imKernelLaplacian5x5());
  return 1;
}

static int imluaKernelLaplacian7x7(lua_State *L)
{
  imlua_pushimage(L, imKernelLaplacian7x7());
  return 1;
}

static int imluaKernelGradian3x3(lua_State *L)
{
  imlua_pushimage(L, imKernelGradian3x3());
  return 1;
}

static int imluaKernelGradian7x7(lua_State *L)
{
  imlua_pushimage(L, imKernelGradian7x7());
  return 1;
}

static int imluaKernelSculpt(lua_State *L)
{
  imlua_pushimage(L, imKernelSculpt());
  return 1;
}

static int imluaKernelMean3x3(lua_State *L)
{
  imlua_pushimage(L, imKernelMean3x3());
  return 1;
}

static int imluaKernelMean5x5(lua_State *L)
{
  imlua_pushimage(L, imKernelMean5x5());
  return 1;
}

static int imluaKernelCircularMean5x5(lua_State *L)
{
  imlua_pushimage(L, imKernelCircularMean5x5());
  return 1;
}

static int imluaKernelMean7x7(lua_State *L)
{
  imlua_pushimage(L, imKernelMean7x7());
  return 1;
}

static int imluaKernelCircularMean7x7(lua_State *L)
{
  imlua_pushimage(L, imKernelCircularMean7x7());
  return 1;
}

static int imluaKernelGaussian3x3(lua_State *L)
{
  imlua_pushimage(L, imKernelGaussian3x3());
  return 1;
}

static int imluaKernelGaussian5x5(lua_State *L)
{
  imlua_pushimage(L, imKernelGaussian5x5());
  return 1;
}

static int imluaKernelBarlett5x5(lua_State *L)
{
  imlua_pushimage(L, imKernelBarlett5x5());
  return 1;
}

static int imluaKernelTopHat5x5(lua_State *L)
{
  imlua_pushimage(L, imKernelTopHat5x5());
  return 1;
}

static int imluaKernelTopHat7x7(lua_State *L)
{
  imlua_pushimage(L, imKernelTopHat7x7());
  return 1;
}

static int imluaKernelEnhance(lua_State *L)
{
  imlua_pushimage(L, imKernelEnhance());
  return 1;
}


static const luaL_reg imkernel_lib[] = {
  {"KernelSobel",           imluaKernelSobel},
  {"KernelPrewitt",         imluaKernelPrewitt},
  {"KernelKirsh",           imluaKernelKirsh},
  {"KernelLaplacian4",      imluaKernelLaplacian4},
  {"KernelLaplacian8",      imluaKernelLaplacian8},
  {"KernelLaplacian5x5",    imluaKernelLaplacian5x5},
  {"KernelLaplacian7x7",    imluaKernelLaplacian7x7},
  {"KernelGradian3x3",      imluaKernelGradian3x3},
  {"KernelGradian7x7",      imluaKernelGradian7x7},
  {"KernelSculpt",          imluaKernelSculpt},
  {"KernelMean3x3",         imluaKernelMean3x3},
  {"KernelMean5x5",         imluaKernelMean5x5},
  {"KernelCircularMean5x5", imluaKernelCircularMean5x5},
  {"KernelMean7x7",         imluaKernelMean7x7},
  {"KernelCircularMean7x7", imluaKernelCircularMean7x7},
  {"KernelGaussian3x3",     imluaKernelGaussian3x3},
  {"KernelGaussian5x5",     imluaKernelGaussian5x5},
  {"KernelBarlett5x5",      imluaKernelBarlett5x5},
  {"KernelTopHat5x5",       imluaKernelTopHat5x5},
  {"KernelTopHat7x7",       imluaKernelTopHat7x7},
  {"KernelEnhance",         imluaKernelEnhance},
  {NULL, NULL}
};

void imlua_open_kernel (lua_State *L)
{
  /* "im" table is at the top of the stack */
  luaL_register(L, NULL, imkernel_lib);
}
