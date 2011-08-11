/***************************************************************************\
* CDLUA.C, for LUA 3.1                                                      *
* Diego Fernandes Nehab, Antonio Escano Scuri                               *
* 01/99                                                                     *
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
#include "cdgdiplus.h"

#include <lua.h>
#include <lauxlib.h>

#include "cdlua.h"
#include "cdlua3_private.h"


/***************************************************************************\
* CD_CGM.                                                                   *
\***************************************************************************/
static void *cdcgm_checkdata(int param)
{
  lua_Object data = lua_getparam(param);
  if (!lua_isstring(data))
    lua_error("cdCreateCanvas CD_CGM: data should be of type string!");

  return lua_getstring(data);
}

static int cgm_sizecb(cdCanvas *canvas, int w, int h, double mm_w, double mm_h);
static int cgm_countercb(cdCanvas *canvas, double percent);
static int cgm_sclmdecb(cdCanvas *canvas, short scl_mde, short *draw_mode_i, double *factor_f);
static int cgm_vdcextcb(cdCanvas *canvas, short type, void *xmn, void *ymn, void *xmx, void *ymx);
static int cgm_begpictcb(cdCanvas *canvas, char *pict);
static int cgm_begpictbcb(cdCanvas *canvas);
static int cgm_begmtfcb(cdCanvas *canvas, int *xmn, int *ymn, int *xmx, int *ymx);

static cdCallbackLUA cdluacgmcb[7] = {
{
  -1,
  "CD_SIZECB",
  (cdCallback)cgm_sizecb
},
{
  -1,
  "CD_CGMCOUNTERCB",
  (cdCallback)cgm_countercb
},
{
  -1,
  "CD_CGMSCLMDECB",
  (cdCallback)cgm_sclmdecb
},
{
  -1,
  "CD_CGMVDCEXTCB",
  (cdCallback)cgm_vdcextcb
},
{
  -1,
  "CD_CGMBEGPICTCB",
  (cdCallback)cgm_begpictcb
},
{
  -1,
  "CD_CGMBEGPICTBCB",
  (cdCallback)cgm_begpictbcb
},
{
  -1,
  "CD_CGMBEGMTFCB",
  (cdCallback)cgm_begmtfcb
}
};

static cdContextLUA cdluacgmctx = 
{
  0,
  "CD_CGM",
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
  lua_Object func, result;
  int result_i;

  lua_beginblock();
  func = lua_getref(cdluacgmcb[CD_CGMCOUNTERCB].lock);

  cdlua_pushcanvas(canvas);
  lua_pushnumber( percent);
  lua_callfunction(func);

  result = lua_getresult(1);
  if (!lua_isnumber(result))
    lua_error("cdPlay: CD_CGMCOUNTERCB: invalid return value!");
  result_i = (int) lua_getnumber(result);

  lua_endblock();
  
  return result_i;
}

/***************************************************************************\
* CGM CD_BEGPICTCB.                                                         *
\***************************************************************************/
static int cgm_begpictcb(cdCanvas *canvas, char *pict)
{
  lua_Object func, result;
  int result_i;

  lua_beginblock();
  func = lua_getref(cdluacgmcb[CD_CGMBEGPICTCB].lock);

  cdlua_pushcanvas(canvas);
  lua_pushstring(pict);
  lua_callfunction(func);

  result = lua_getresult(1);
  if (!lua_isnumber(result))
    lua_error("cdPlay: CD_CGMBEGPICTCB: invalid return value!");
  result_i = (int) lua_getnumber(result);

  lua_endblock();
  
  return result_i;
}

static int cgm_begmtfcb(cdCanvas *canvas, int *xmn, int *ymn, int *xmx, int *ymx)
{
  lua_Object func, result, xmn_l, ymn_l, xmx_l, ymx_l;
  int result_i;

  lua_beginblock();
  func = lua_getref(cdluacgmcb[CD_CGMBEGMTFCB].lock);

  cdlua_pushcanvas(canvas);
  lua_callfunction(func);

  result = lua_getresult(1);
  if (!lua_isnumber(result))
    lua_error("cdPlay: CD_CGMBEGMTFCB: invalid return value!");
  result_i = (int) lua_getnumber(result);
  if (result_i == 1) {
    lua_endblock();
    return 1;
  }

  xmn_l = lua_getresult(2);
  if (!lua_isnumber(xmn_l))
    lua_error("cdPlay: CD_CGMBEGMTFCB: invalid xmn return value!");
  *xmn = (int) lua_getnumber(xmn_l);

  ymn_l = lua_getresult(3);
  if (!lua_isnumber(ymn_l))
    lua_error("cdPlay: CD_CGMBEGMTFCB: invalid ymn return value!");
  *ymn = (int) lua_getnumber(ymn_l);

  xmx_l = lua_getresult(4);
  if (!lua_isnumber(xmx_l))
    lua_error("cdPlay: CD_CGMBEGMTFCB: invalid xmx return value!");
  *xmx = (int) lua_getnumber(xmx_l);

  ymx_l = lua_getresult(5);
  if (!lua_isnumber(ymx_l))
    lua_error("cdPlay: CD_CGMBEGMTFCB: invalid ymx return value!");
  *ymx = (int) lua_getnumber(ymx_l);

  lua_endblock();
  
  return result_i;
}

/***************************************************************************\
* CGM CD_BEGPICTBCB.                                                         *
\***************************************************************************/
static int cgm_begpictbcb(cdCanvas *canvas)
{
  lua_Object func, result;
  int result_i;

  lua_beginblock();
  func = lua_getref(cdluacgmcb[CD_CGMBEGPICTBCB].lock);

  cdlua_pushcanvas(canvas);
  lua_callfunction(func);

  result = lua_getresult(1);
  if (!lua_isnumber(result))
    lua_error("cdPlay: CD_CGMBEGPICTBCB: invalid return value!");
  result_i = (int) lua_getnumber(result);

  lua_endblock();
  
  return result_i;
}

/***************************************************************************\
* CGM CD_SIZECB.                                                            *
\***************************************************************************/
static int cgm_sizecb(cdCanvas *canvas, int w, int h, double mm_w, double mm_h)
{
  lua_Object func, result;
  int result_i;

  lua_beginblock();
  func = lua_getref(cdluacgmcb[CD_SIZECB].lock);

  cdlua_pushcanvas(canvas);
  lua_pushnumber( w);
  lua_pushnumber( h);
  lua_pushnumber( mm_w);
  lua_pushnumber( mm_h);
  lua_callfunction(func);

  result = lua_getresult(1);
  if (!lua_isnumber(result))
    lua_error("cdPlay: CD_SIZECB: invalid return value!");
  result_i = (int) lua_getnumber(result);

  lua_endblock();
  
  return result_i;
}

/***************************************************************************\
* CGM CD_SCLMDE.                                                            *
\***************************************************************************/
static int cgm_sclmdecb(cdCanvas *canvas, short scl_mde, short *draw_mode_i, double *factor_f)
{
  lua_Object func, result, draw_mode, factor;
  int result_i;

  lua_beginblock();
  func = lua_getref(cdluacgmcb[CD_CGMSCLMDECB].lock);

  cdlua_pushcanvas(canvas);
  lua_pushnumber( scl_mde);
  lua_callfunction(func);

  result = lua_getresult(1);
  if (!lua_isnumber(result))
    lua_error("cdPlay: CD_CGMSCLMDECB: invalid return value!");
  result_i = (int) lua_getnumber(result);
  if (result_i == 1) {
    lua_endblock();
    return 1;
  }

  draw_mode = lua_getresult(2);
  if (!lua_isnumber(draw_mode))
    lua_error("cdPlay: CD_CGMSCLMDECB: invalid draw_mode return value!");
  *draw_mode_i = (short) lua_getnumber(draw_mode);

  factor = lua_getresult(3);
  if (!lua_isnumber(factor))
    lua_error("cdPlay: CD_CGMSCLMDECB: invalid factor return value!");
  *factor_f = (double) lua_getnumber(factor);

  lua_endblock();
  
  return result_i;
}

/***************************************************************************\
* CGM CD_VDCEXTCB.                                                          *
\***************************************************************************/
static int cgm_vdcextcb(cdCanvas *canvas, short type, void *xmn, void *ymn, void *xmx, void *ymx)
{
  lua_Object func, result, xmn_l, ymn_l, xmx_l, ymx_l;
  int result_i;

  lua_beginblock();
  func = lua_getref(cdluacgmcb[CD_CGMVDCEXTCB].lock);

  cdlua_pushcanvas(canvas);
  lua_callfunction(func);

  result = lua_getresult(1);
  if (!lua_isnumber(result))
    lua_error("cdPlay: CD_CGMVDCEXTCB: invalid return value!");
  result_i = (int) lua_getnumber(result);
  if (result_i == 1) {
    lua_endblock();
    return 1;
  }

  xmn_l = lua_getresult(2);
  if (!lua_isnumber(xmn_l))
    lua_error("cdPlay: CD_CGMVDCEXTCB: invalid xmn return value!");
  if (type == 1) *((float *) xmn) = (float) lua_getnumber(xmn_l);
  else *((int *) xmn) = (int) lua_getnumber(xmn_l);

  ymn_l = lua_getresult(3);
  if (!lua_isnumber(ymn_l))
    lua_error("cdPlay: CD_CGMVDCEXTCB: invalid ymn return value!");
  if (type == 1) *((float *) ymn) = (float) lua_getnumber(ymn_l);
  else *((int *) ymn) = (int) lua_getnumber(ymn_l);

  xmx_l = lua_getresult(4);
  if (!lua_isnumber(xmx_l))
    lua_error("cdPlay: CD_CGMVDCEXTCB: invalid xmx return value!");
  if (type == 1) *((float *) xmx) = (float) lua_getnumber(xmx_l);
  else *((int *) xmx) = (int) lua_getnumber(xmx_l);

  ymx_l = lua_getresult(5);
  if (!lua_isnumber(ymx_l))
    lua_error("cdPlay: CD_CGMVDCEXTCB: invalid ymx return value!");
  if (type == 1) *((float *) ymx) = (float) lua_getnumber(ymx_l);
  else *((int *) ymx) = (int) lua_getnumber(ymx_l);

  lua_endblock();
  
  return result_i;
}

/***************************************************************************\
* CD_DBUFFER.                                                                 *
\***************************************************************************/
static void *cddbuf_checkdata(int param)
{
  canvas_t *canvas_p;
  lua_Object canvas;
  int canvas_tag = (int)lua_getnumber(lua_getglobal(CANVAS_TAG));

  canvas = lua_getparam(param);
  if (lua_isnil(canvas))
    lua_error("cdCreateCanvas CD_DBUFFER: data is a NIL canvas!");

  if (lua_tag(canvas) != canvas_tag)
    lua_error("cdCreateCanvas CD_DBUFFER: data should be of type canvas_tag!");

  canvas_p = (canvas_t *) lua_getuserdata(canvas);
  if (!canvas_p->cd_canvas)
    lua_error("cdCreateCanvas CD_DBUFFER: data is a killed canvas!");

  return canvas_p->cd_canvas;
}

static cdContextLUA cdluadbufctx = 
{
  0,
  "CD_DBUFFER",
  cdContextDBuffer,
  cddbuf_checkdata,
  NULL,
  0
};

/***************************************************************************\
* CD_IMAGE.                                                                 *
\***************************************************************************/
static void *cdimage_checkdata(int param)
{                
  int image_tag;
  image_t *image_p;
  lua_Object image = lua_getparam(param);
  if (lua_isnil(image))
    lua_error("cdCreateCanvas CD_IMAGE: data is a NIL image!");

  image_tag = (int)lua_getnumber(lua_getglobal(IMAGE_TAG));
  if (lua_tag(image) != image_tag)
    lua_error("cdCreateCanvas CD_IMAGE: data should be of type image_tag!");

  image_p = (image_t *) lua_getuserdata(image);
  if (!image_p->cd_image)
    lua_error("cdCreateCanvas CD_IMAGE: data is a killed image!");

  return image_p->cd_image;
}

static cdContextLUA cdluaimagectx = 
{
  0,
  "CD_IMAGE",
  cdContextImage,
  cdimage_checkdata,
  NULL,
  0
};

/***************************************************************************\
* CD_IMAGERGB.                                                              *
\***************************************************************************/
static void *cdimagergb_checkdata(int param)
{
  lua_Object imagergb;
  static char data_s[50];

  imagergb = lua_getparam(param);
  if (lua_isnil(imagergb))
    lua_error("cdCreateCanvas CD_IMAGERGB: data is a NIL imagergb!");

  if (lua_isstring(imagergb))
  {
    char* str = lua_getstring(imagergb);
    strcpy(data_s, str);
  }
  else
  {
    lua_Object res;
    int bitmap_tag = (int)lua_getnumber(lua_getglobal(BITMAP_TAG));

    if (lua_tag(imagergb) == bitmap_tag)
    {
      bitmap_t *imagergb_p;

      imagergb_p = (bitmap_t *) lua_getuserdata(imagergb);
      if (!imagergb_p->image)
        lua_error("cdCreateCanvas CD_IMAGERGB: data is a killed imagergb!");

      if (imagergb_p->image->type != CD_RGB && imagergb_p->image->type != CD_RGBA)
        lua_error("cdCreateCanvas CD_IMAGERGB: bitmap should be of type rgb or rgba!");

      res = lua_getparam(param+1);
      if (res == LUA_NOOBJECT || lua_isnil(res))
      {
        if (imagergb_p->image->type == CD_RGBA)
          sprintf(data_s, "%dx%d %p %p %p %p -a", imagergb_p->image->w, imagergb_p->image->h,
                                                  cdBitmapGetData(imagergb_p->image, CD_IRED), 
                                                  cdBitmapGetData(imagergb_p->image, CD_IGREEN), 
                                                  cdBitmapGetData(imagergb_p->image, CD_IBLUE),
                                                  cdBitmapGetData(imagergb_p->image, CD_IALPHA));
        else
          sprintf(data_s, "%dx%d %p %p %p", imagergb_p->image->w, imagergb_p->image->h,
                                            cdBitmapGetData(imagergb_p->image, CD_IRED), 
                                            cdBitmapGetData(imagergb_p->image, CD_IGREEN), 
                                            cdBitmapGetData(imagergb_p->image, CD_IBLUE));
      }
      else
      {
        double res_f = lua_getnumber(res);
        if (imagergb_p->image->type == CD_RGBA)
          sprintf(data_s, "%dx%d %p %p %p %p -r%g -a", imagergb_p->image->w, imagergb_p->image->h,
                                                    cdBitmapGetData(imagergb_p->image, CD_IRED), 
                                                    cdBitmapGetData(imagergb_p->image, CD_IGREEN), 
                                                    cdBitmapGetData(imagergb_p->image, CD_IBLUE), 
                                                    cdBitmapGetData(imagergb_p->image, CD_IALPHA), 
                                                    res_f);
        else
          sprintf(data_s, "%dx%d %p %p %p -r%g", imagergb_p->image->w, imagergb_p->image->h,
                                                cdBitmapGetData(imagergb_p->image, CD_IRED), 
                                                cdBitmapGetData(imagergb_p->image, CD_IGREEN), 
                                                cdBitmapGetData(imagergb_p->image, CD_IBLUE), 
                                                res_f);
      }
    }
    else
    {
      imagergb_t *imagergb_p;
      int imagergb_tag = (int)lua_getnumber(lua_getglobal(IMAGERGB_TAG));

      if (lua_tag(imagergb) != imagergb_tag)
      {
        imagergba_t *imagergba_p;
        int imagergba_tag = (int)lua_getnumber(lua_getglobal(IMAGERGBA_TAG));
        if (lua_tag(imagergb) != imagergba_tag)
          lua_error("cdCreateCanvas CD_IMAGERGB: data should be of type imagergb_tag or imagergba_tag!");

        imagergba_p = (imagergba_t *) lua_getuserdata(imagergb);
        if (!(imagergba_p->red && imagergba_p->green && imagergba_p->blue))
          lua_error("cdCreateCanvas CD_IMAGERGB: data is a killed imagergba!");

        res = lua_getparam(param+1);
        if (res == LUA_NOOBJECT || lua_isnil(res))
        {
          sprintf(data_s, "%dx%d %p %p %p", imagergba_p->width, imagergba_p->height,
            imagergba_p->red, imagergba_p->green, imagergba_p->blue);
        }
        else
        {
          double res_f = lua_getnumber(res);
          sprintf(data_s, "%dx%d %p %p %p -r%g", imagergba_p->width, imagergba_p->height,
            imagergba_p->red, imagergba_p->green, imagergba_p->blue, res_f);
        }

        return data_s;
      }

      imagergb_p = (imagergb_t *) lua_getuserdata(imagergb);
      if (!(imagergb_p->red && imagergb_p->green && imagergb_p->blue))
        lua_error("cdCreateCanvas CD_IMAGERGB: data is a killed imagergb!");

      res = lua_getparam(param+1);
      if (res == LUA_NOOBJECT || lua_isnil(res))
      {
        sprintf(data_s, "%dx%d %p %p %p", imagergb_p->width, imagergb_p->height,
          imagergb_p->red, imagergb_p->green, imagergb_p->blue);
      }
      else
      {
        double res_f = lua_getnumber(res);
        sprintf(data_s, "%dx%d %p %p %p -r%g", imagergb_p->width, imagergb_p->height,
          imagergb_p->red, imagergb_p->green, imagergb_p->blue, res_f);
      }
    }
  }

  return data_s;
}

static cdContextLUA cdluaimagergbctx = 
{
  0,
  "CD_IMAGERGB",
  cdContextImageRGB,
  cdimagergb_checkdata,
  NULL,
  0
};

/***************************************************************************\
* CD_DXF.                                                                   *
\***************************************************************************/
static void *cddxf_checkdata(int param)
{
  lua_Object data = lua_getparam(param);
  if (!lua_isstring(data))
    lua_error("cdCreateCanvas CD_DXF: data should be of type string!");
  
  return lua_getstring(data);
}

static cdContextLUA cdluadxfctx = 
{
  0,
  "CD_DXF",
  cdContextDXF,
  cddxf_checkdata
};

/***************************************************************************\
* CD_DGN.                                                                   *
\***************************************************************************/
static void *cddgn_checkdata(int param)
{
  lua_Object data = lua_getparam(param);
  if (!lua_isstring(data))
    lua_error("cdCreateCanvas CD_DGN: data should be of type string!");
  
  return lua_getstring(data);
}

static cdContextLUA cdluadgnctx = 
{
  0,
  "CD_DGN",
  cdContextDGN,
  cddgn_checkdata,
  NULL,
  0
};

/***************************************************************************\
* CD_WMF.                                                                   *
\***************************************************************************/
static void *cdwmf_checkdata(int param)
{
  lua_Object data = lua_getparam(param);
  if (!lua_isstring(data))
    lua_error("cdCreateCanvas CD_WMF: data should be of type string!");
  
  return lua_getstring(data);
}

static int wmf_sizecb(cdCanvas *canvas, int w, int h, double mm_w, double mm_h);

static cdCallbackLUA cdluawmfcb[1] = 
{{
  -1,
  "CD_SIZECB",
  (cdCallback)wmf_sizecb
}};

static cdContextLUA cdluawmfctx = 
{
  0,
  "CD_WMF",
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
  lua_Object func, result;
  int result_i;

  lua_beginblock();
  func = lua_getref(cdluawmfcb[CD_SIZECB].lock);

  cdlua_pushcanvas(canvas);
  lua_pushnumber( w);
  lua_pushnumber( h);
  lua_pushnumber( mm_w);
  lua_pushnumber( mm_h);
  lua_callfunction(func);

  result = lua_getresult(1);
  if (!lua_isnumber(result))
    lua_error("cdPlay: CD_SIZECB: invalid return value!");
  result_i = (int) lua_getnumber(result);

  lua_endblock();
  
  return result_i;
}

/***************************************************************************\
* CD_EMF.                                                                   *
\***************************************************************************/
static void *cdemf_checkdata(int param)
{
  lua_Object data = lua_getparam(param);
  if (!lua_isstring(data))
    lua_error("cdCreateCanvas CD_EMF: data should be of type string!");
  
  return lua_getstring(data);
}

static int emf_sizecb(cdCanvas *canvas, int w, int h, double mm_w, double mm_h);

static cdCallbackLUA cdluaemfcb[1] = 
{{
  -1,
  "CD_SIZECB",
  (cdCallback)emf_sizecb
}};

static cdContextLUA cdluaemfctx = 
{
  0,
  "CD_EMF",
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
  lua_Object func, result;
  int result_i;

  lua_beginblock();
  func = lua_getref(cdluaemfcb[CD_SIZECB].lock);

  cdlua_pushcanvas(canvas);
  lua_pushnumber( w);
  lua_pushnumber( h);
  lua_pushnumber( mm_w);
  lua_pushnumber( mm_h);
  lua_callfunction(func);

  result = lua_getresult(1);
  if (!lua_isnumber(result))
    lua_error("cdPlay: CD_SIZECB: invalid return value!");
  result_i = (int) lua_getnumber(result);

  lua_endblock();
  
  return result_i;
}

/***************************************************************************\
* CD_METAFILE.                                                              *
\***************************************************************************/
static void *cdmetafile_checkdata(int param)
{
  lua_Object data;

  data = lua_getparam(param);
  if (!lua_isstring(data))
    lua_error("cdCreateCanvas CD_METAFILE: data should be of type string!");

  return lua_getstring(data);
}

static int metafile_sizecb(cdCanvas *canvas, int w, int h, double mm_w, double mm_h);

static cdCallbackLUA cdluamfcb[1] = 
{{
  -1,
  "CD_SIZECB",
  (cdCallback)metafile_sizecb
}};

static cdContextLUA cdluamfctx = 
{
  0,
  "CD_METAFILE",
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
  lua_Object func, result;
  int result_i;

  lua_beginblock();
  func = lua_getref(cdluamfcb[CD_SIZECB].lock);

  cdlua_pushcanvas(canvas);
  lua_pushnumber( w);
  lua_pushnumber( h);
  lua_pushnumber( mm_w);
  lua_pushnumber( mm_h);
  lua_callfunction(func);

  result = lua_getresult(1);
  if (!lua_isnumber(result))
    lua_error("cdPlay: CD_SIZECB: invalid return value!");
  result_i = (int) lua_getnumber(result);

  lua_endblock();
  
  return result_i;
}

/***************************************************************************\
* CD_PS.                                                                    *
\***************************************************************************/
static void *cdps_checkdata(int param)
{
  lua_Object data = lua_getparam(param);
  if (!lua_isstring(data))
    lua_error("cdCreateCanvas CD_PS: data should be of type string!");

  return lua_getstring(data);
}

static cdContextLUA cdluapsctx = 
{
  0,
  "CD_PS",
  cdContextPS,
  cdps_checkdata,
  NULL,
  0
};

/***************************************************************************\
* CD_SVG.                                                                    *
\***************************************************************************/
static void *cdsvg_checkdata(int param)
{
  lua_Object data = lua_getparam(param);
  if (!lua_isstring(data))
    lua_error("cdCreateCanvas CD_SVG: data should be of type string!");

  return lua_getstring(data);
}

static cdContextLUA cdluasvgctx = 
{
  0,
  "CD_SVG",
  cdContextSVG,
  cdsvg_checkdata,
  NULL,
  0
};

/***************************************************************************\
* CD_PRINTER.                                                               *
\***************************************************************************/
static void *cdprinter_checkdata(int param)
{
  lua_Object data = lua_getparam(param);
  if (!lua_isstring(data))
    lua_error("cdCreateCanvas CD_PRINTER: data should be of type string!");

  return lua_getstring(data);
}

static cdContextLUA cdluaprinterctx = 
{
  0,
  "CD_PRINTER",
  cdContextPrinter,
  cdprinter_checkdata,
  NULL,
  0
};

/***************************************************************************\
* CD_CLIPBOARD.                                                             *
\***************************************************************************/
static void *cdclipboard_checkdata(int param)
{
  lua_Object data = lua_getparam(param);
  if (!lua_isstring(data))
    lua_error("cdCreateCanvas CD_CLIPBOARD: data should be of type string!");

  return lua_getstring(data);
}

static int clipboard_sizecb(cdCanvas *canvas, int w, int h, double mm_w, double mm_h);

static cdCallbackLUA cdluaclipboardcb[1] = 
{{
  -1,
  "CD_SIZECB",
  (cdCallback)clipboard_sizecb
}};

static cdContextLUA cdluaclipboardctx = 
{
  0,
  "CD_CLIPBOARD",
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
  lua_Object func, result;
  int result_i;

  lua_beginblock();
  func = lua_getref(cdluaclipboardcb[CD_SIZECB].lock);

  cdlua_pushcanvas(canvas);
  lua_pushnumber( w);
  lua_pushnumber( h);
  lua_pushnumber( mm_w);
  lua_pushnumber( mm_h);
  lua_callfunction(func);

  result = lua_getresult(1);
  if (!lua_isnumber(result))
    lua_error("cdPlay: CD_SIZECB: invalid return value!");
  result_i = (int) lua_getnumber(result);

  lua_endblock();
  
  return result_i;
}

/***************************************************************************\
* CD_NATIVEWINDOW.                                                          *
\***************************************************************************/
static void *cdnativewindow_checkdata(int param)
{
  lua_Object data = lua_getparam(param);

#ifdef WIN32
  if (!lua_isuserdata(data))
    lua_error("cdCreateCanvas CD_NATIVEWINDOW: data should be of type userdata!");

  return lua_getuserdata(data);
#else
  if (!lua_isstring(data))
    lua_error("cdCreateCanvas CD_NATIVEWINDOW: data should be of type string!");

  return lua_getstring(data);
#endif
}

static cdContextLUA cdluanativewindowctx = 
{
  0,
  "CD_NATIVEWINDOW",
  cdContextNativeWindow,
  cdnativewindow_checkdata,
  NULL,
  0
};

static void cdlua_getscreensize(void)
{
  int width;
  int height;
  double mm_width;
  double mm_height;

  if (lua_getparam(1) != LUA_NOOBJECT)
    lua_error("cdGetScreenSize: too many parameters!");

  cdGetScreenSize(&width, &height, &mm_width, &mm_height);
  lua_pushnumber( width);
  lua_pushnumber( height);
  lua_pushnumber( mm_width);
  lua_pushnumber( mm_height);
}

static void cdlua_getscreencolorplanes(void)
{
 int L_result = cdGetScreenColorPlanes();
 lua_pushnumber(L_result);
}

static void cdlua_usecontextplus(void)
{
  int use = (int)luaL_check_number(1);
  int L_result = cdUseContextPlus(use);
  lua_pushnumber(L_result);
}

/*******************************************************************************/

void cdlua_initdrivers(void)
{
  cdlua_register("cdGetScreenColorPlanes",cdlua_getscreencolorplanes);
  cdlua_register("cdGetScreenSize",cdlua_getscreensize);

  cdlua_register("cdUseContextPlus",cdlua_usecontextplus);

  /* from GDI+ addicional polygon modes */
  cdlua_pushnumber(CD_SPLINE, "CD_SPLINE");
  cdlua_pushnumber(CD_FILLSPLINE, "CD_FILLSPLINE");
  cdlua_pushnumber(CD_FILLGRADIENT, "CD_FILLGRADIENT");

  cdlua_addcontext(&cdluaimagectx);
  cdlua_addcontext(&cdluaimagergbctx);
  cdlua_addcontext(&cdluadxfctx);
  cdlua_addcontext(&cdluadgnctx);
  cdlua_addcontext(&cdluacgmctx);
  cdlua_addcontext(&cdluamfctx);
  cdlua_addcontext(&cdluapsctx);
  cdlua_addcontext(&cdluasvgctx);
  cdlua_addcontext(&cdluaclipboardctx);
  cdlua_addcontext(&cdluanativewindowctx);
  cdlua_addcontext(&cdluaprinterctx);
  cdlua_addcontext(&cdluawmfctx);
  cdlua_addcontext(&cdluaemfctx);
  cdlua_addcontext(&cdluadbufctx);
}

