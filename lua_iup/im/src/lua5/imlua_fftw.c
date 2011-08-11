/** \file
 * \brief IM Lua 5 Binding
 *
 * See Copyright Notice in im_lib.h
 * $Id: imlua_fftw.c,v 1.3 2010/06/07 20:59:32 scuri Exp $
 */

#include <memory.h>

#include "im.h"
#include "im_image.h"
#include "im_process.h"
#include "im_util.h"

#include <lua.h>
#include <lauxlib.h>

#include "imlua.h"
#include "imlua_aux.h"
#include "imlua_image.h"


/*****************************************************************************\
 Domain Transform Operations
\*****************************************************************************/

/*****************************************************************************\
 im.ProcessFFT(src_image, dst_image)
\*****************************************************************************/
static int imluaProcessFFT (lua_State *L)
{
  imImage* src_image = imlua_checkimage(L, 1);
  imImage* dst_image = imlua_checkimage(L, 2);

  imlua_matchsize(L, src_image, dst_image);
  imlua_checkdatatype(L, 2, dst_image, IM_CFLOAT);

  imProcessFFT(src_image, dst_image);
  return 0;
}

/*****************************************************************************\
 im.ProcessIFFT(src_image, dst_image)
\*****************************************************************************/
static int imluaProcessIFFT (lua_State *L)
{
  imImage* src_image = imlua_checkimage(L, 1);
  imImage* dst_image = imlua_checkimage(L, 2);

  imlua_matchsize(L, src_image, dst_image);
  imlua_checkdatatype(L, 1, src_image, IM_CFLOAT);
  imlua_checkdatatype(L, 2, dst_image, IM_CFLOAT);

  imProcessIFFT(src_image, dst_image);
  return 0;
}

/*****************************************************************************\
 im.ProcessFFTRaw(src_image, inverse, center, normalize)
\*****************************************************************************/
static int imluaProcessFFTraw (lua_State *L)
{
  imImage* src_image = imlua_checkimage(L, 1);
  int inverse = luaL_checkint(L, 2);
  int center = luaL_checkint(L, 3);
  int normalize = luaL_checkint(L, 4);

  imlua_checkdatatype(L, 1, src_image, IM_CFLOAT);

  imProcessFFTraw(src_image, inverse, center, normalize);
  return 0;
}

/*****************************************************************************\
 im.ProcessSwapQuadrants(src_image, inverse, center, normalize)
\*****************************************************************************/
static int imluaProcessSwapQuadrants (lua_State *L)
{
  imImage* src_image = imlua_checkimage(L, 1);
  int center2origin = luaL_checkint(L, 2);

  imlua_checkdatatype(L, 1, src_image, IM_CFLOAT);

  imProcessSwapQuadrants(src_image, center2origin);
  return 0;
}

/*****************************************************************************\
 im.ProcessCrossCorrelation(image1, image2, dst_image)
\*****************************************************************************/
static int imluaProcessCrossCorrelation (lua_State *L)
{
  imImage* image1 = imlua_checkimage(L, 1);
  imImage* image2 = imlua_checkimage(L, 2);
  imImage* dst_image = imlua_checkimage(L, 3);

  imlua_matchsize(L, image1, dst_image);
  imlua_matchsize(L, image2, dst_image);
  imlua_checkdatatype(L, 3, dst_image, IM_CFLOAT);

  imProcessCrossCorrelation(image1, image2, dst_image);
  return 0;
}

/*****************************************************************************\
 im.ProcessAutoCorrelation(src_image, dst_image)
\*****************************************************************************/
static int imluaProcessAutoCorrelation (lua_State *L)
{
  imImage* src_image = imlua_checkimage(L, 1);
  imImage* dst_image = imlua_checkimage(L, 2);

  imlua_matchsize(L, src_image, dst_image);
  imlua_checkdatatype(L, 2, dst_image, IM_CFLOAT);

  imProcessAutoCorrelation(src_image, dst_image);
  return 0;
}

static const luaL_reg imfftw_lib[] = {
  {"ProcessFFT", imluaProcessFFT},
  {"ProcessIFFT", imluaProcessIFFT},
  {"ProcessFFTraw", imluaProcessFFTraw},
  {"ProcessSwapQuadrants", imluaProcessSwapQuadrants},
  {"ProcessCrossCorrelation", imluaProcessCrossCorrelation},
  {"ProcessAutoCorrelation", imluaProcessAutoCorrelation},

  {NULL, NULL}
};

int imlua_open_fftw (lua_State *L)
{
  luaL_register(L, "im", imfftw_lib);  /* leave "im" table at the top of the stack */

#ifdef IMLUA_USELOH
#include "im_fftw.loh"
#else
#ifdef IMLUA_USELZH
#include "im_fftw.lzh"
#else
  luaL_dofile(L, "im_fftw.lua");
#endif
#endif

  return 1;
}

int luaopen_imlua_fftw(lua_State *L)
{
  return imlua_open_fftw(L);
}
