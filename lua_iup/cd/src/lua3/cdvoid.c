/** \file
 * \brief CD Void driver for error checking while there is no active canvas
 *
 * See Copyright Notice in cd.h
 */

#include <stdlib.h>
#include <stdio.h>

#ifndef CD_NO_OLD_INTERFACE
#define CD_NO_OLD_INTERFACE
#endif

#include "cd.h"
#include "cd_private.h"
#include <lua.h>
#include <lauxlib.h>
#include "cdlua3_private.h"


struct _cdCtxCanvas 
{
  cdCanvas* canvas;
};

static void cdcreatecanvas(cdCanvas *canvas, void *data)
{
  cdCtxCanvas *ctxcanvas = (cdCtxCanvas*) malloc(sizeof(cdCtxCanvas));
  ctxcanvas->canvas = canvas;
  canvas->ctxcanvas = ctxcanvas;
  (void)data;
}

static void cdkillcanvas(cdCtxCanvas* ctxcanvas)
{
  free(ctxcanvas);
}

/***************************************************************************\
* Echos an error if called.                                                 *
\***************************************************************************/
static void cdvoid_error(void)
{
  lua_error("cdlua: there is no active canvas!");
}

/***************************************************************************\
* Dummy.                                                                    *
\***************************************************************************/
static int cdvoid_dummy(void)
{
  return CD_OK;
}

/***************************************************************************\
* Driver function table.                                                    *
\***************************************************************************/

void cdinittable(cdCanvas* canvas)
{
  /* attribute functions can not be set, because of default attributes */

  canvas->cxClip = (int (*)(cdCtxCanvas*, int))cdvoid_error;
  canvas->cxClipArea = (void (*)(cdCtxCanvas*, int, int, int, int))cdvoid_error;
  canvas->cxNewRegion = (void (*)(cdCtxCanvas*))cdvoid_error;
  canvas->cxIsPointInRegion = (int (*)(cdCtxCanvas*, int, int))cdvoid_error;
  canvas->cxOffsetRegion = (void (*)(cdCtxCanvas*, int, int))cdvoid_error;
  canvas->cxGetRegionBox = (void (*)(cdCtxCanvas*, int *, int *, int *, int *))cdvoid_error;
  canvas->cxFlush = (void ( *)(cdCtxCanvas*))cdvoid_error;
  canvas->cxClear = (void ( *)(cdCtxCanvas*))cdvoid_error;
  canvas->cxPixel = (void ( *)(cdCtxCanvas*, int ,int ,long ))cdvoid_error;
  canvas->cxLine = (void ( *)(cdCtxCanvas*, int ,int ,int ,int ))cdvoid_error;
  canvas->cxPoly = (void ( *)(cdCtxCanvas*, int ,struct _cdPoint *,int ))cdvoid_error;
  canvas->cxRect = (void ( *)(cdCtxCanvas*, int ,int ,int ,int ))cdvoid_error;
  canvas->cxBox = (void ( *)(cdCtxCanvas*, int ,int ,int ,int ))cdvoid_error;
  canvas->cxArc = (void ( *)(cdCtxCanvas*, int ,int ,int ,int ,double ,double ))cdvoid_error;
  canvas->cxSector = (void ( *)(cdCtxCanvas*, int ,int ,int ,int ,double ,double ))cdvoid_error;
  canvas->cxChord = (void ( *)(cdCtxCanvas*, int ,int ,int ,int ,double ,double ))cdvoid_error;
  canvas->cxText = (void (*)(cdCtxCanvas*, int ,int ,const char *))cdvoid_error;
  canvas->cxGetFontDim = (void (*)(cdCtxCanvas*, int *,int *,int *,int *))cdvoid_error;
  canvas->cxGetTextSize = (void (*)(cdCtxCanvas*, const char *,int *,int *))cdvoid_error;
  canvas->cxPutImageRectRGB = (void (*)(cdCtxCanvas*, int ,int ,const unsigned char *,const unsigned char *,const unsigned char *,int ,int ,int ,int ,int ,int ,int ,int ))cdvoid_error;
  canvas->cxPutImageRectRGBA = (void (*)(cdCtxCanvas*, int ,int ,const unsigned char *,const unsigned char *,const unsigned char *,const unsigned char *,int ,int ,int ,int ,int ,int ,int ,int ))cdvoid_error;
  canvas->cxPutImageRectMap = (void (*)(cdCtxCanvas*, int ,int ,const unsigned char *,const long *,int ,int ,int ,int ,int ,int ,int ,int ))cdvoid_error;
  canvas->cxScrollArea = (void (*)(cdCtxCanvas*, int ,int ,int ,int ,int ,int ))cdvoid_error;
  canvas->cxFLine = (void (*)(cdCtxCanvas*, double ,double ,double ,double ))cdvoid_error;
  canvas->cxFPoly = (void (*)(cdCtxCanvas*, int , cdfPoint*,int ))cdvoid_error;
  canvas->cxFRect = (void (*)(cdCtxCanvas*, double ,double ,double ,double ))cdvoid_error;
  canvas->cxFBox = (void (*)(cdCtxCanvas*, double ,double ,double ,double ))cdvoid_error;
  canvas->cxFArc = (void (*)(cdCtxCanvas*, double ,double ,double ,double ,double ,double ))cdvoid_error;
  canvas->cxFSector = (void (*)(cdCtxCanvas*, double ,double ,double ,double ,double ,double ))cdvoid_error;
  canvas->cxFText = (void (*)(cdCtxCanvas*, double ,double ,const char *))cdvoid_error;
  canvas->cxStipple = (void (*)(cdCtxCanvas*, int ,int ,const unsigned char *))cdvoid_error;
  canvas->cxPattern = (void (*)(cdCtxCanvas*, int ,int , const long *))cdvoid_error;
  canvas->cxNativeFont = (int (*)(cdCtxCanvas*, const char*))cdvoid_error;
  canvas->cxPalette = (void (*)(cdCtxCanvas*, int ,const long *,int ))cdvoid_error;
  canvas->cxGetImageRGB = (void (*)(cdCtxCanvas*, unsigned char *,unsigned char *,unsigned char *,int ,int ,int ,int ))cdvoid_error;
  canvas->cxCreateImage = (cdCtxImage* (*)(cdCtxCanvas*, int ,int ))cdvoid_error;
  canvas->cxGetImage = (void (*)(cdCtxCanvas*, cdCtxImage*, int ,int ))cdvoid_error;
  canvas->cxPutImageRect = (void (*)(cdCtxCanvas*, cdCtxImage*,int ,int ,int ,int ,int ,int ))cdvoid_error;
  canvas->cxKillImage = (void (*)(cdCtxImage*))cdvoid_error;
  canvas->cxFClipArea = (void (*)(cdCtxCanvas*, double,double,double,double))cdvoid_error;

  /* must not be the error callback  */
  canvas->cxActivate = (int (*)(cdCtxCanvas*))cdvoid_dummy;
  canvas->cxDeactivate = (void (*)(cdCtxCanvas*))cdvoid_dummy;
  canvas->cxFont = (int (*)(cdCtxCanvas*, const char *, int, int))cdvoid_dummy;

  canvas->cxKillCanvas = cdkillcanvas;
}

static cdContext cdVoidContext =
{
  0,
  0,
  cdcreatecanvas,
  cdinittable,
  NULL,
  NULL
};

cdContext* cdContextVoid(void)
{
  return &cdVoidContext;
}

