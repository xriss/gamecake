/***************************************************************************\
* $Id: cdlua5ctx.c,v 1.5 2010/10/13 19:19:05 scuri Exp $
*                                                                           *
\***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "cd.h"
#include "wd.h"

#include "cdimage.h"
#include "cdirgb.h"
#include "cddxf.h"
#include "cddgn.h"
#include "cdcgm.h"
#include "cdwmf.h"
#include "cdemf.h"
#include "cdnative.h"
#include "cdprint.h"
#include "cdclipbd.h"
#include "cdmf.h"
#include "cdps.h"
#include "cdsvg.h"
#include "cddbuf.h"
#include "cddebug.h"
#include "cdpicture.h"


#include <lua.h>
#include <lauxlib.h>

#include "cdlua.h"
#include "cdlua5_private.h"


/***************************************************************************\
* CD_CGM.                                                                   *
\***************************************************************************/
static void *cdcgm_checkdata(lua_State * L, int param)
{
  return (void *)luaL_checkstring(L,param);
}

static int cgm_sizecb(cdCanvas *canvas, int w, int h, double mm_w, double mm_h);
static int cgm_countercb(cdCanvas *canvas, double percent);
static int cgm_sclmdecb(cdCanvas *canvas, short scl_mde, short *draw_mode_i, double *factor_f);
static int cgm_vdcextcb(cdCanvas *canvas, short type, void *xmn, void *ymn, void *xmx, void *ymx);
static int cgm_begpictcb(cdCanvas *canvas, char *pict);
static int cgm_begpictbcb(cdCanvas *canvas);
static int cgm_begmtfcb(cdCanvas *canvas, int *xmn, int *ymn, int *xmx, int *ymx);

static cdluaCallback cdluacgmcb[7] = {
{
  -1,
  "SIZECB",
  (cdCallback)cgm_sizecb
},
{
  -1,
  "CGMCOUNTERCB",
  (cdCallback)cgm_countercb
},
{
  -1,
  "CGMSCLMDECB",
  (cdCallback)cgm_sclmdecb
},
{
  -1,
  "CGMVDCEXTCB",
  (cdCallback)cgm_vdcextcb
},
{
  -1,
  "CGMBEGPICTCB",
  (cdCallback)cgm_begpictcb
},
{
  -1,
  "CGMBEGPICTBCB",
  (cdCallback)cgm_begpictbcb
},
{
  -1,
  "CGMBEGMTFCB",
  (cdCallback)cgm_begmtfcb
}
};

static cdluaContext cdluacgmctx = 
{
  0,
  "CGM",
  cdContextCGM,
  cdcgm_checkdata,
  cdluacgmcb,
  7
};

/***************************************************************************\
* CGM CD_COUNTERCB.                                                         *
\***************************************************************************/
static int cgm_countercb(cdCanvas *canvas, double percent)
{
  /* little Wrapper */
  lua_State * L = cdlua_getplaystate();
  
  lua_getref(L, cdluacgmcb[CD_CGMCOUNTERCB].lock);

  cdlua_pushcanvas(L, canvas);
  lua_pushnumber(L, percent);
  if(lua_pcall(L, 2, 1, 0) != 0)
    luaL_error(L, "error running function: %s", lua_tostring(L, -1));

  if (!lua_isnumber(L,-1))
    luaL_error(L, "invalid return value");

  return luaL_checkint(L,-1);
}

/***************************************************************************\
* CGM CD_BEGPICTCB.                                                         *
\***************************************************************************/
static int cgm_begpictcb(cdCanvas *canvas, char *pict)
{
  /* little Wrapper */
  lua_State * L = cdlua_getplaystate();
  
  lua_getref(L, cdluacgmcb[CD_CGMBEGPICTCB].lock);

  cdlua_pushcanvas(L, canvas);
  lua_pushstring(L, pict);
  if(lua_pcall(L, 2, 1, 0) != 0)
    luaL_error(L, "error running function: %s", lua_tostring(L, -1));

  if (!lua_isnumber(L,-1))
    luaL_error(L,"invalid return value");

  return luaL_checkint(L,-1);
}

static int cgm_begmtfcb(cdCanvas *canvas, int *xmn, int *ymn, int *xmx, int *ymx)
{
  int result_i;
  /* little Wrapper */
  lua_State * L = cdlua_getplaystate();

  lua_getref(L, cdluacgmcb[CD_CGMBEGMTFCB].lock);

  cdlua_pushcanvas(L, canvas);
  if(lua_pcall(L, 1, 5, 0) != 0)
    luaL_error(L, "error running function: %s", lua_tostring(L, -1));

  if (!lua_isnumber(L, -5))
    luaL_error(L, "invalid return value");
  
  result_i = luaL_checkint(L, -5);
  if (result_i == 1)
    return 1;

  if (!lua_isnumber(L, -4))
    luaL_error(L, "invalid xmn return value");
  *xmn = luaL_checkint(L, -4);

  if (!lua_isnumber(L, -3))
    luaL_error(L, "invalid ymn return value");
  *ymn = luaL_checkint(L, -3);

  if (!lua_isnumber(L, -2))
    luaL_error(L, "invalid xmx return value");
  *xmx = luaL_checkint(L, -2);

  if (!lua_isnumber(L, -1))
    luaL_error(L, "invalid ymx return value");
  *ymx = luaL_checkint(L, -1);

  return result_i;
}

/***************************************************************************\
* CGM CD_BEGPICTBCB.                                                         *
\***************************************************************************/
static int cgm_begpictbcb(cdCanvas *canvas)
{
  /* little Wrapper */
  lua_State * L = cdlua_getplaystate();

  lua_getref(L, cdluacgmcb[CD_CGMBEGPICTBCB].lock);

  cdlua_pushcanvas(L, canvas);
  if(lua_pcall(L, 1, 1, 0) != 0)
    luaL_error(L, "error running function: %s", lua_tostring(L, -1));

  if (!lua_isnumber(L, -1))
    luaL_error(L, "invalid return value");
    
  return luaL_checkint(L,-1);
}

/***************************************************************************\
* CGM CD_SIZECB.                                                            *
\***************************************************************************/
static int cgm_sizecb(cdCanvas *canvas, int w, int h, double mm_w, double mm_h)
{
  /* little Wrapper */
  lua_State * L = cdlua_getplaystate();

  lua_getref(L,cdluacgmcb[CD_SIZECB].lock);
  
  cdlua_pushcanvas(L, canvas);
  lua_pushnumber(L, w );
  lua_pushnumber(L, h );
  lua_pushnumber(L, mm_w );
  lua_pushnumber(L, mm_h );
  if(lua_pcall(L, 5, 1, 0) != 0)
    luaL_error(L, "error running function: %s", lua_tostring(L, -1));

  if (!lua_isnumber(L, -1))
    luaL_error(L, "invalid return value");
   
  return luaL_checkint(L,-1);
}

/***************************************************************************\
* CGM CD_SCLMDE.                                                            *
\***************************************************************************/
static int cgm_sclmdecb(cdCanvas *canvas, short scl_mde, short *draw_mode_i, double *factor_f)
{
  int result_i;
  /* little Wrapper */
  lua_State * L = cdlua_getplaystate();

  lua_getref(L,cdluacgmcb[CD_CGMSCLMDECB].lock);

  cdlua_pushcanvas(L, canvas);
  lua_pushnumber(L, scl_mde);
  if(lua_pcall(L, 2, 3, 0) != 0)
    luaL_error(L, "error running function: %s", lua_tostring(L, -1));

  if (!lua_isnumber(L, -3))
    luaL_error(L, "invalid return value");

  result_i = luaL_checkint(L, -3);

  if (result_i == 1)
    return 1;
  
  if (!lua_isnumber(L, -2))
    luaL_error(L, "invalid draw mode return value");
  *draw_mode_i = (short) lua_tonumber(L,-2);

  if (!lua_isnumber(L, -1))
    luaL_error(L, "invalid factor return value");

  *factor_f = (double) lua_tonumber(L, -1);

  return result_i;
}

/***************************************************************************\
* CGM CD_VDCEXTCB.                                                          *
\***************************************************************************/
static int cgm_vdcextcb(cdCanvas *canvas, short type, void *xmn, void *ymn, void *xmx, void *ymx)
{  
  int result_i;
  /* little Wrapper */
  lua_State * L = cdlua_getplaystate();

  lua_getref(L, cdluacgmcb[CD_CGMVDCEXTCB].lock);

  cdlua_pushcanvas(L, canvas);
  if(lua_pcall(L, 1, 5, 0) != 0)
    luaL_error(L, "error running function: %s", lua_tostring(L, -1));

  if (!lua_isnumber(L, -5))
    luaL_error(L, "invalid return value");
  result_i = luaL_checkint(L,-5);
  if (result_i == 1)
    return 1;

  if (!lua_isnumber(L, -4))
    luaL_error(L, "invalid xmn return value");
  if (type == 1) *((float *) xmn) = (float) lua_tonumber(L, -4);
  else *((int *) xmn) = luaL_checkint(L, -4);

  if (!lua_isnumber(L, -3))
    luaL_error(L, "invalid ymn return value");
  if (type == 1) *((float *) ymn) = (float) lua_tonumber(L, -3);
  else *((int *) ymn) = luaL_checkint(L, -3);

  if (!lua_isnumber(L, -2))
    luaL_error(L,"invalid xmx return value");
  if (type == 1) *((float *) xmx) = (float) lua_tonumber(L, -2);
  else *((int *) xmx) = luaL_checkint(L, -2);

  if (!lua_isnumber(L, -1))
    luaL_error(L,"invalid ymx return value");
  if (type == 1) *((float *) ymx) = (float) lua_tonumber(L, -1);
  else *((int *) ymx) = (int) luaL_checkint(L, -1);

  return result_i;
}

/***************************************************************************\
* CD_DBUFFER.                                                                 *
\***************************************************************************/
static void *cddbuf_checkdata(lua_State * L, int param)
{
  return cdlua_checkcanvas(L, param);
}

static cdluaContext cdluadbufctx = 
{
  0,
  "DBUFFER",
  cdContextDBuffer,
  cddbuf_checkdata,
  NULL,
  0
};

/***************************************************************************\
* CD_DBUFFERRGB.                                                                 *
\***************************************************************************/
static void *cddbufrgb_checkdata(lua_State * L, int param)
{
  return cdlua_checkcanvas(L, param);
}

static cdluaContext cdluadbufrgbctx = 
{
  0,
  "DBUFFERRGB",
  cdContextDBufferRGB,
  cddbufrgb_checkdata,
  NULL,
  0
};

/***************************************************************************\
* CD_IMAGE.                                                                 *
\***************************************************************************/
static void *cdimage_checkdata(lua_State *L, int param)
{
  return cdlua_checkimage(L, param);
}

static cdluaContext cdluaimagectx = 
{
  0,
  "IMAGE",
  cdContextImage,
  cdimage_checkdata,
  NULL,
  0
};

static int cdlua_rawchecktype(lua_State *L, int param, const char* type)
{
  if (lua_isuserdata(L, param))   /* value is a userdata? */
  {
    if (lua_getmetatable(L, param))   /* does it have a metatable? */
    {
      lua_getfield(L, LUA_REGISTRYINDEX, type);  /* get correct metatable */
      if (lua_rawequal(L, -1, -2))   /* does it have the correct mt? */
      {
        lua_pop(L, 2);  /* remove both metatables */
        return 1;
      }
      else
      {
        lua_pop(L, 2);  /* remove both metatables */
        return -1;  /* test for other metatables */
      }
    }
  }
  return 0;  /* do not continue */
}

/***************************************************************************\
* CD_IMAGERGB.                                                              *
\***************************************************************************/
static void *cdimagergb_checkdata(lua_State* L, int param)
{
  static char data_s[100] = "";

  if (lua_isstring(L, param))
  {
    const char* str = lua_tostring(L, param);
    strcpy(data_s, str);
  }
  else
  {
    int ret = cdlua_rawchecktype(L, param, "cdBitmap");

    if (ret == 0)
      luaL_typeerror(L, param, "cdBitmap");  /* not a user data and not a metatable */

    if (ret == 1)
    {
      cdBitmap* *bitmap_p = (cdBitmap**)luaL_checkudata(L, param, "cdBitmap");
      if (!(*bitmap_p))
        luaL_argerror(L, param, "killed cdBitmap");

      if ((*bitmap_p)->type != CD_RGB && (*bitmap_p)->type != CD_RGBA)
        luaL_argerror(L, param, "bitmap should be of type rgb or rgba");

      if (lua_isnoneornil(L, param+1))
      {
        if ((*bitmap_p)->type == CD_RGBA)
          sprintf(data_s, "%dx%d %p %p %p %p -a", (*bitmap_p)->w, (*bitmap_p)->h,
                                                  cdBitmapGetData(*bitmap_p, CD_IRED), 
                                                  cdBitmapGetData(*bitmap_p, CD_IGREEN), 
                                                  cdBitmapGetData(*bitmap_p, CD_IBLUE), 
                                                  cdBitmapGetData(*bitmap_p, CD_IALPHA));
        else
          sprintf(data_s, "%dx%d %p %p %p", (*bitmap_p)->w, (*bitmap_p)->h,
                                            cdBitmapGetData(*bitmap_p, CD_IRED), 
                                            cdBitmapGetData(*bitmap_p, CD_IGREEN), 
                                            cdBitmapGetData(*bitmap_p, CD_IBLUE));
      }
      else
      {
        double res_f = luaL_checknumber(L, param+1);
        if ((*bitmap_p)->type == CD_RGBA)
          sprintf(data_s, "%dx%d %p %p %p %p -r%g -a", (*bitmap_p)->w, (*bitmap_p)->h,
                                                       cdBitmapGetData(*bitmap_p, CD_IRED), 
                                                       cdBitmapGetData(*bitmap_p, CD_IGREEN), 
                                                       cdBitmapGetData(*bitmap_p, CD_IBLUE), 
                                                       cdBitmapGetData(*bitmap_p, CD_IALPHA), 
                                                       res_f);
        else
          sprintf(data_s, "%dx%d %p %p %p -r%g", (*bitmap_p)->w, (*bitmap_p)->h,
                                                 cdBitmapGetData(*bitmap_p, CD_IRED), 
                                                 cdBitmapGetData(*bitmap_p, CD_IGREEN), 
                                                 cdBitmapGetData(*bitmap_p, CD_IBLUE), 
                                                 res_f);
      }

      return data_s;
    }

    ret = cdlua_rawchecktype(L, param, "cdImageRGB");
    if (ret == 1)
    {
      cdluaImageRGB *imagergb_p = (cdluaImageRGB*) luaL_checkudata(L, param, "cdImageRGB");
      if (!imagergb_p->red)
        luaL_argerror(L, param, "killed cdImageRGB");

      if (lua_isnoneornil(L, param+1))
      {
        sprintf(data_s, "%dx%d %p %p %p", imagergb_p->width, imagergb_p->height,
          imagergb_p->red, imagergb_p->green, imagergb_p->blue);
      }
      else
      {
        double res_f = luaL_checknumber(L, param+1);
        sprintf(data_s, "%dx%d %p %p %p -r%g", imagergb_p->width, imagergb_p->height,
          imagergb_p->red, imagergb_p->green, imagergb_p->blue, res_f);
      }

      return data_s;
    }

    ret = cdlua_rawchecktype(L, param, "cdImageRGBA");
    if (ret == 1)
    {
      cdluaImageRGBA *imagergba_p = (cdluaImageRGBA*) luaL_checkudata(L, param, "cdImageRGBA");
      if (!imagergba_p->red)
        luaL_argerror(L, param, "killed cdImageRGBA");

      if (lua_isnoneornil(L, param+1))
      {
        sprintf(data_s, "%dx%d %p %p %p %p -a", imagergba_p->width, imagergba_p->height,
          imagergba_p->red, imagergba_p->green, imagergba_p->blue, imagergba_p->alpha);
      }
      else
      {
        double res_f = luaL_checknumber(L, param+1);
        sprintf(data_s, "%dx%d %p %p %p %p -r%g -a", imagergba_p->width, imagergba_p->height,
          imagergba_p->red, imagergba_p->green, imagergba_p->blue, imagergba_p->alpha, res_f);
      }

      return data_s;
    }

    luaL_typeerror(L, param, "cdBitmap");  /* is a metatable but it is not one of the accepted */
  }

  return data_s;
}

static cdluaContext cdluaimagergbctx = 
{
  0,
  "IMAGERGB",
  cdContextImageRGB,
  cdimagergb_checkdata,
  NULL,
  0
};

/***************************************************************************\
* CD_DXF.                                                                   *
\***************************************************************************/
static void *cddxf_checkdata(lua_State * L, int param)
{
  return (void *)luaL_checkstring(L,param);
}

static cdluaContext cdluadxfctx = 
{
  0,
  "DXF",
  cdContextDXF,
  cddxf_checkdata
};

/***************************************************************************\
* CD_DGN.                                                                   *
\***************************************************************************/
static void *cddgn_checkdata(lua_State * L, int param)
{
  return  (void *)luaL_checkstring(L,param);
}

static cdluaContext cdluadgnctx = 
{
  0,
  "DGN",
  cdContextDGN,
  cddgn_checkdata,
  NULL,
  0
};

/***************************************************************************\
* CD_WMF.                                                                   *
\***************************************************************************/
static void *cdwmf_checkdata(lua_State * L, int param)
{
  return  (void *)luaL_checkstring(L,param);
}

static int wmf_sizecb(cdCanvas *canvas, int w, int h, double mm_w, double mm_h);

static cdluaCallback cdluawmfcb[1] = 
{{
  -1,
  "SIZECB",
  (cdCallback)wmf_sizecb
}};

static cdluaContext cdluawmfctx = 
{
  0,
  "WMF",
  cdContextWMF,
  cdwmf_checkdata,
  cdluawmfcb,
  1
};

/***************************************************************************\
* WMF CD_SIZECB.                                                            *
\***************************************************************************/
static int wmf_sizecb(cdCanvas *canvas, int w, int h, double mm_w, double mm_h)
{
  /* little Wrapper */
  lua_State * L = cdlua_getplaystate();

  lua_getref(L,cdluawmfcb[CD_SIZECB].lock);

  cdlua_pushcanvas(L, canvas);
  lua_pushnumber(L, w);
  lua_pushnumber(L, h);
  lua_pushnumber(L, mm_w);
  lua_pushnumber(L, mm_h);
  if(lua_pcall(L, 5, 1, 0) != 0)
    luaL_error(L, "error running function: %s", lua_tostring(L, -1));

  if (!lua_isnumber(L, -1))
    luaL_error(L,"invalid return value");
    
  return luaL_checkint(L,-1);
}

/***************************************************************************\
* CD_EMF.                                                                   *
\***************************************************************************/
static void *cdemf_checkdata(lua_State *L, int param)
{
  return (void *)luaL_checkstring(L,param);
}

static int emf_sizecb(cdCanvas *canvas, int w, int h, double mm_w, double mm_h);

static cdluaCallback cdluaemfcb[1] = 
{{
  -1,
  "SIZECB",
  (cdCallback)emf_sizecb
}};

static cdluaContext cdluaemfctx = 
{
  0,
  "EMF",
  cdContextEMF,
  cdemf_checkdata,
  cdluaemfcb,
  1
};

/***************************************************************************\
* EMF CD_SIZECB.                                                            *
\***************************************************************************/
static int emf_sizecb(cdCanvas *canvas, int w, int h, double mm_w, double mm_h)
{
  /* little Wrapper */
  lua_State * L = cdlua_getplaystate();

  lua_getref(L,cdluaemfcb[CD_SIZECB].lock);

  cdlua_pushcanvas(L, canvas);
  lua_pushnumber(L, w);
  lua_pushnumber(L, h);
  lua_pushnumber(L, mm_w);
  lua_pushnumber(L, mm_h);
  if(lua_pcall(L, 5, 1, 0) != 0)
    luaL_error(L, "error running function: %s", lua_tostring(L, -1));

  if (!lua_isnumber(L,-1))
    luaL_error(L, "invalid return value");

  return luaL_checkint(L,-1);
}

/***************************************************************************\
* CD_PICTURE.                                                              *
\***************************************************************************/
static void *cdpicture_checkdata(lua_State *L,int param)
{
  return (void *)luaL_checkstring(L,param);
}

static cdluaContext cdluapicturectx = 
{
  0,
  "PICTURE",
  cdContextPicture,
  cdpicture_checkdata,
  NULL,
  0
};

/***************************************************************************\
* CD_DEBUG.                                                              *
\***************************************************************************/
static void *cddebug_checkdata(lua_State *L,int param)
{
  return (void *)luaL_checkstring(L,param);
}

static cdluaContext cdluadebugctx = 
{
  0,
  "DEBUG",
  cdContextDebug,
  cddebug_checkdata,
  NULL,
  0
};

/***************************************************************************\
* CD_METAFILE.                                                              *
\***************************************************************************/
static void *cdmetafile_checkdata(lua_State *L,int param)
{
  return (void *)luaL_checkstring(L,param);
}

static int metafile_sizecb(cdCanvas *canvas, int w, int h, double mm_w, double mm_h);

static cdluaCallback cdluamfcb[1] = 
{{
  -1,
  "SIZECB",
  (cdCallback)metafile_sizecb
}};

static cdluaContext cdluamfctx = 
{
  0,
  "METAFILE",
  cdContextMetafile,
  cdmetafile_checkdata,
  cdluamfcb,
  1
};

/***************************************************************************\
* METAFILE CD_SIZECB.                                                       *
\***************************************************************************/
static int metafile_sizecb(cdCanvas *canvas, int w, int h, double mm_w, double mm_h)
{
  /* little Wrapper */
  lua_State * L = cdlua_getplaystate();

  lua_getref(L, cdluamfcb[CD_SIZECB].lock);

  cdlua_pushcanvas(L, canvas);
  lua_pushnumber(L, w);
  lua_pushnumber(L, h);
  lua_pushnumber(L, mm_w);
  lua_pushnumber(L, mm_h);
  if(lua_pcall(L, 5, 1, 0) != 0)
    luaL_error(L, "error running function: %s", lua_tostring(L, -1));

  if (!lua_isnumber(L, -1))
    luaL_error(L, "invalid return value");

  return luaL_checkint(L,-1);
}

/***************************************************************************\
* CD_PS.                                                                    *
\***************************************************************************/
static void *cdps_checkdata( lua_State *L, int param)
{
  return (void *)luaL_checkstring(L, param);
}

static cdluaContext cdluapsctx = 
{
  0,
  "PS",
  cdContextPS,
  cdps_checkdata,
  NULL,
  0
};

/***************************************************************************\
* CD_SVG.                                                                    *
\***************************************************************************/
static void *cdsvg_checkdata( lua_State *L, int param)
{
  return (void *)luaL_checkstring(L, param);
}

static cdluaContext cdluasvgctx = 
{
  0,
  "SVG",
  cdContextSVG,
  cdsvg_checkdata,
  NULL,
  0
};

/***************************************************************************\
* CD_PRINTER.                                                               *
\***************************************************************************/
static void *cdprinter_checkdata(lua_State *L, int param)
{
  return (void *)luaL_checkstring(L,param);
}

static cdluaContext cdluaprinterctx = 
{
  0,
  "PRINTER",
  cdContextPrinter,
  cdprinter_checkdata,
  NULL,
  0
};

/***************************************************************************\
* CD_CLIPBOARD.                                                             *
\***************************************************************************/
static void *cdclipboard_checkdata(lua_State *L, int param)
{
  return (void *)luaL_checkstring(L,param);
}

static int clipboard_sizecb(cdCanvas *canvas, int w, int h, double mm_w, double mm_h);

static cdluaCallback cdluaclipboardcb[1] = 
{{
  -1,
  "SIZECB",
  (cdCallback)clipboard_sizecb
}};

static cdluaContext cdluaclipboardctx = 
{
  0,
  "CLIPBOARD",
  cdContextClipboard,
  cdclipboard_checkdata,
  cdluaclipboardcb,
  1
};

/***************************************************************************\
* CLIPBOARD CD_SIZECB.                                                      *
\***************************************************************************/
static int clipboard_sizecb(cdCanvas *canvas, int w, int h, double mm_w, double mm_h)
{
  /* little Wrapper */
  lua_State * L = cdlua_getplaystate();
  lua_getref(L, cdluaclipboardcb[CD_SIZECB].lock);

  cdlua_pushcanvas(L, canvas);
  lua_pushnumber(L, w);
  lua_pushnumber(L, h);
  lua_pushnumber(L, mm_w);
  lua_pushnumber(L, mm_h);
  if(lua_pcall(L, 5, 1, 0) != 0)
    luaL_error(L, "error running function: %s", lua_tostring(L, -1));

  if (!lua_isnumber(L,-1))
    luaL_error(L, "invalid return value");

  return luaL_checkint(L,-1);
}

/***************************************************************************\
* CD_NATIVEWINDOW.                                                          *
\***************************************************************************/
static void *cdnativewindow_checkdata(lua_State *L, int param)
{
  if (!lua_isnil(L,param) && 
     (!lua_isuserdata(L,param) || !lua_isstring(L, param)))
    luaL_argerror(L, param, "data should be of type userdata or a string");

  if (lua_isuserdata(L,param))
    return lua_touserdata(L,param);
  else
    return (void *)luaL_checkstring(L,param);
}

static cdluaContext cdluanativewindowctx = 
{
  0,
  "NATIVEWINDOW",
  cdContextNativeWindow,
  cdnativewindow_checkdata,
  NULL,
  0
};


/*******************************************************************************\
* Init all CD Drivers                                                           *  
*********************************************************************************/
void cdlua_initdrivers(lua_State * L, cdluaLuaState* cdL) 
{
  cdlua_addcontext(L, cdL, &cdluaimagectx);
  cdlua_addcontext(L, cdL, &cdluaimagergbctx);
  cdlua_addcontext(L, cdL, &cdluadxfctx);
  cdlua_addcontext(L, cdL, &cdluadgnctx);
  cdlua_addcontext(L, cdL, &cdluacgmctx);
  cdlua_addcontext(L, cdL, &cdluamfctx);
  cdlua_addcontext(L, cdL, &cdluadebugctx);
  cdlua_addcontext(L, cdL, &cdluapicturectx);
  cdlua_addcontext(L, cdL, &cdluapsctx);
  cdlua_addcontext(L, cdL, &cdluasvgctx);
  cdlua_addcontext(L, cdL, &cdluaclipboardctx);
  cdlua_addcontext(L, cdL, &cdluanativewindowctx);
  cdlua_addcontext(L, cdL, &cdluaprinterctx);
  cdlua_addcontext(L, cdL, &cdluawmfctx);
  cdlua_addcontext(L, cdL, &cdluaemfctx);
  cdlua_addcontext(L, cdL, &cdluadbufctx);
  cdlua_addcontext(L, cdL, &cdluadbufrgbctx);
}
