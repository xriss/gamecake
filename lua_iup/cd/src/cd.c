/** \file
 * \brief External API
 *
 * See Copyright Notice in cd.h
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <memory.h>
#include <stdarg.h>


#include "cd.h"
#include "wd.h"
#include "cd_private.h"
#include "cdirgb.h"

/* This appears only here to avoid changing the cd.h header fo bug fixes */
#define CD_VERSION_FIX ".1"
#define CD_VERSION_FIX_NUMBER 1
#define CD_VERSION_FIX_DATE "2010/11/09"

const char cd_ident[] =
  "$CD: " CD_VERSION CD_VERSION_FIX " " CD_COPYRIGHT " $\n"
  "$URL: www.tecgraf.puc-rio.br/cd $\n";

static char *tecver = "TECVERID.str:CD:LIB:" CD_VERSION CD_VERSION_FIX;

char* cdVersion(void)
{              
  (void)cd_ident;
	(void)tecver;
  return CD_VERSION CD_VERSION_FIX;
}

char* cdVersionDate(void)
{
#ifdef CD_VERSION_FIX_DATE
  return CD_VERSION_FIX_DATE;
#else
  return CD_VERSION_DATE;
#endif
}
 
int cdVersionNumber(void)
{
  return CD_VERSION_NUMBER+CD_VERSION_FIX_NUMBER;
}

static void cd_setdefaultfunc(cdCanvas* canvas)
{
  canvas->cxGetTextSize = cdgettextsizeEX;
  canvas->cxGetFontDim = cdgetfontdimEX;
  canvas->cxRect = cdSimRect;
}

static void cd_setdefaultattrib(cdCanvas* canvas)
{
  /* clipping attributes */
  canvas->clip_mode = CD_CLIPOFF;

  /* color attributes */
  canvas->foreground = CD_BLACK;
  canvas->background = CD_WHITE;

  canvas->back_opacity = CD_TRANSPARENT;
  canvas->write_mode = CD_REPLACE;

  /* primitive attributes */
  canvas->mark_type = CD_STAR;
  canvas->mark_size = 10;

  canvas->line_width = 1;
  canvas->line_style = CD_CONTINUOUS;
  canvas->line_cap = CD_CAPFLAT;
  canvas->line_join = CD_MITER;

  canvas->hatch_style = CD_HORIZONTAL;
  canvas->interior_style = CD_SOLID;
  canvas->fill_mode = CD_EVENODD;

  strcpy(canvas->font_type_face, "System");
  canvas->font_style     = CD_PLAIN;
  canvas->font_size      = CD_STANDARD;

  canvas->text_alignment = CD_BASE_LEFT;

  canvas->matrix[0] = 1;  /* identity */
  canvas->matrix[3] = 1;

  /* o resto recebeu zero no memset */
}

void cdUpdateAttributes(cdCanvas* canvas)
{
  cdCtxCanvas* ctxcanvas = canvas->ctxcanvas;

  if (canvas->cxBackground) canvas->cxBackground(ctxcanvas, canvas->background);
  if (canvas->cxForeground) canvas->cxForeground(ctxcanvas, canvas->foreground);
  if (canvas->cxBackOpacity) canvas->cxBackOpacity(ctxcanvas, canvas->back_opacity);
  if (canvas->cxWriteMode) canvas->cxWriteMode(ctxcanvas, canvas->write_mode);

  if (canvas->cxLineStyle) canvas->cxLineStyle(ctxcanvas, canvas->line_style);
  if (canvas->cxLineWidth) canvas->cxLineWidth(ctxcanvas, canvas->line_width);
  if (canvas->cxLineCap) canvas->cxLineCap(ctxcanvas, canvas->line_cap);
  if (canvas->cxLineJoin) canvas->cxLineJoin(ctxcanvas, canvas->line_join);

  if (canvas->cxHatch) canvas->cxHatch(ctxcanvas, canvas->hatch_style);
  if (canvas->stipple && canvas->cxStipple) canvas->cxStipple(ctxcanvas, canvas->stipple_w, canvas->stipple_h, canvas->stipple);
  if (canvas->pattern && canvas->cxPattern) canvas->cxPattern(ctxcanvas, canvas->pattern_w, canvas->pattern_h, canvas->pattern);
  if (canvas->cxInteriorStyle) canvas->cxInteriorStyle(ctxcanvas, canvas->interior_style);

  if (canvas->native_font[0] && canvas->cxNativeFont)
    canvas->cxNativeFont(ctxcanvas, canvas->native_font);
  else if (canvas->cxFont) 
    canvas->cxFont(ctxcanvas, canvas->font_type_face, canvas->font_style, canvas->font_size);
  if (canvas->cxTextAlignment) canvas->cxTextAlignment(ctxcanvas, canvas->text_alignment);
  if (canvas->cxTextOrientation) canvas->cxTextOrientation(ctxcanvas, canvas->text_orientation);

  if (canvas->use_matrix && canvas->cxTransform) canvas->cxTransform(ctxcanvas, canvas->matrix);

  if (canvas->clip_mode == CD_CLIPAREA && canvas->cxClipArea) canvas->cxClipArea(ctxcanvas, canvas->clip_rect.xmin, canvas->clip_rect.xmax, canvas->clip_rect.ymin, canvas->clip_rect.ymax);
  if (canvas->clip_mode == CD_CLIPAREA && canvas->cxFClipArea) canvas->cxFClipArea(ctxcanvas, canvas->clip_frect.xmin, canvas->clip_frect.xmax, canvas->clip_frect.ymin, canvas->clip_frect.ymax);
  if (canvas->clip_mode == CD_CLIPPOLYGON && canvas->clip_poly) canvas->cxPoly(ctxcanvas, CD_CLIP, canvas->clip_poly, canvas->clip_poly_n);
  if (canvas->clip_mode == CD_CLIPPOLYGON && canvas->clip_fpoly) canvas->cxFPoly(ctxcanvas, CD_CLIP, canvas->clip_fpoly, canvas->clip_poly_n);
  if (canvas->clip_mode != CD_CLIPOFF && canvas->cxClip) canvas->cxClip(ctxcanvas, canvas->clip_mode);
}

cdCanvas* cdCreateCanvasf(cdContext *context, const char* format, ...)
{
  char data[1024];
  va_list arglist;
  va_start(arglist, format);
  vsprintf(data, format, arglist);

  return cdCreateCanvas(context, data);
}

cdCanvas *cdCreateCanvas(cdContext* context, void *data_str)
{
  cdCanvas *canvas;

  /* usefull for NULL drivers, that do nothing and exist only for portability */
  if (!context)
    return NULL;

  {
    static int first = 1;
    char* env = getenv("CD_QUIET");
    if (first && env && strcmp(env, "NO")==0)
    {
      printf("CD  " CD_VERSION CD_VERSION_FIX " " CD_COPYRIGHT "\n");
      first = 0;
    }
  }

  /* alocates and initialize everything with 0s */
  canvas = (cdCanvas*)malloc(sizeof(cdCanvas));
  memset(canvas, 0, sizeof(cdCanvas));

  canvas->signature[0] = 'C';
  canvas->signature[1] = 'D';

  canvas->vector_font = cdCreateVectorFont(canvas);
  canvas->simulation = cdCreateSimulation(canvas);

  canvas->context = context;

  /* initialize default attributes, must be before creating the canvas */
  cd_setdefaultattrib(canvas);

  context->cxCreateCanvas(canvas, data_str);
  if (!canvas->ctxcanvas)
  {
    cdKillVectorFont(canvas->vector_font);
    cdKillSimulation(canvas->simulation);
    memset(canvas, 0, sizeof(cdCanvas));
    free(canvas);
    return NULL;
  }

  /* default simulation functions */
  cd_setdefaultfunc(canvas);

  /* initialize canvas table */
  context->cxInitTable(canvas);

  /* update the default atributes, must be after InitTable */
  cdCanvasActivate(canvas);
  cdUpdateAttributes(canvas);

  /* must be after creating the canvas, so that we know canvas width and height */
  canvas->clip_rect.xmax = canvas->w-1;
  canvas->clip_rect.ymax = canvas->h-1;

  wdSetDefaults(canvas);

  return canvas;
}

/* re-declared here to ignore CD_NO_OLD_INTERFACE definition */ 
int       cdActivate(cdCanvas* canvas);
cdCanvas* cdActiveCanvas(void);

void cdKillCanvas(cdCanvas *canvas)
{
  assert(canvas);
  if (!_cdCheckCanvas(canvas)) return;

  if (canvas == cdActiveCanvas())
    cdActivate(NULL);
  else
    cdCanvasDeactivate(canvas);
  
  canvas->cxKillCanvas(canvas->ctxcanvas);

  if (canvas->pattern) free(canvas->pattern);
  if (canvas->stipple) free(canvas->stipple);
  if (canvas->poly) free(canvas->poly);
  if (canvas->clip_poly) free(canvas->clip_poly);
  if (canvas->fpoly) free(canvas->fpoly);
  if (canvas->clip_fpoly) free(canvas->clip_fpoly);
  if (canvas->line_dashes) free(canvas->line_dashes);

  cdKillVectorFont(canvas->vector_font);
  cdKillSimulation(canvas->simulation);

  memset(canvas, 0, sizeof(cdCanvas));
  free(canvas);
}

cdContext* cdCanvasGetContext(cdCanvas *canvas)
{
  assert(canvas);
  if (!_cdCheckCanvas(canvas)) return NULL;
  return canvas->context;
}

int cdCanvasActivate(cdCanvas *canvas)
{
  assert(canvas);
  if (!_cdCheckCanvas(canvas)) return CD_ERROR;
  if (!canvas->cxActivate) return CD_OK;

  if (canvas->cxActivate(canvas->ctxcanvas) == CD_ERROR)
    return CD_ERROR;
  
  return CD_OK;
}

void cdCanvasDeactivate(cdCanvas *canvas)
{
  assert(canvas);
  if (!_cdCheckCanvas(canvas) || !canvas->cxDeactivate) return;
  canvas->cxDeactivate(canvas->ctxcanvas);
}

unsigned long cdContextCaps(cdContext *context)
{
  if (!context)
    return (unsigned long)CD_ERROR;
  return context->caps;
}

int cdCanvasSimulate(cdCanvas* canvas, int mode)
{
  int sim_mode;
  cdContext* context;

  assert(canvas);
  if (!_cdCheckCanvas(canvas)) return CD_ERROR;

  context = canvas->context;

  sim_mode = canvas->sim_mode;
  if (mode == CD_QUERY || cdCanvasGetContext(canvas) == CD_IMAGERGB)
    return sim_mode;

  /* default simulation functions */
  cd_setdefaultfunc(canvas);

  /* initialize canvas table */
  context->cxInitTable(canvas);

  canvas->sim_mode = mode;
  if (mode == CD_SIM_NONE)
    return sim_mode;

  /* when simulation is active must not set driver transform */
  canvas->cxTransform = NULL;

  if (mode & CD_SIM_LINE)
  {
    canvas->cxLine = cdSimLine;
    canvas->cxFLine = NULL;
  }

  if (mode & CD_SIM_RECT)
  {
    canvas->cxRect = cdSimRect;
    canvas->cxFRect = NULL;
  }

  if (mode & CD_SIM_BOX)
  {
    canvas->cxBox = cdSimBox;
    canvas->cxFBox = NULL;
  }

  if (mode & CD_SIM_ARC)
  {
    canvas->cxArc = cdSimArc;
    canvas->cxFArc = NULL;
  }

  if (mode & CD_SIM_SECTOR)
  {
    canvas->cxSector = cdSimSector;
    canvas->cxFSector = NULL;
  }

  if (mode & CD_SIM_CHORD)
  {
    canvas->cxChord = cdSimChord;
    canvas->cxFChord = NULL;
  }

  if (mode & CD_SIM_TEXT)
  {
    canvas->cxText = cdSimTextFT;
    canvas->cxFText = NULL;
    canvas->cxNativeFont = NULL;
    canvas->cxFont = cdSimFontFT;
    canvas->cxGetFontDim = cdSimGetFontDimFT;
    canvas->cxGetTextSize = cdSimGetTextSizeFT;
    canvas->cxTextOrientation = NULL;

    cdSimInitText(canvas->simulation);
    canvas->cxFont(canvas->ctxcanvas, canvas->font_type_face, canvas->font_style, canvas->font_size);
  }
  else
    canvas->cxFont(canvas->ctxcanvas, canvas->font_type_face, canvas->font_style, canvas->font_size);

  if (mode & CD_SIM_POLYLINE || mode & CD_SIM_POLYGON)
  {
    /* can NOT replace canvas->cxPoly because it will be used by the simulation,
       handle polygon simulation in Begin/End */
    canvas->cxFPoly = NULL;
  }

  return sim_mode;
}

cdState* cdCanvasSaveState(cdCanvas* canvas)
{
  cdState* state;

  assert(canvas);
  if (!_cdCheckCanvas(canvas)) return NULL;

  state = (cdState*)malloc(sizeof(cdState));
  memcpy(state, canvas, sizeof(cdCanvas));

  if (state->pattern) 
  {
    int size = state->pattern_w*state->pattern_h*sizeof(long);
    state->pattern = (long*)malloc(size);
    memcpy(state->pattern, canvas->pattern, size);
  }

  if (state->stipple) 
  {
    int size = state->stipple_w*state->stipple_h;
    state->stipple = (unsigned char*)malloc(size);
    memcpy(state->stipple, canvas->stipple, size);
  }

  if (state->clip_poly) 
  {
    int size = state->clip_poly_n*sizeof(cdPoint);
    state->clip_poly = (cdPoint*)malloc(size);
    memcpy(state->clip_poly, canvas->clip_poly, size);
  }

  if (state->clip_fpoly) 
  {
    int size = state->clip_poly_n*sizeof(cdfPoint);
    state->clip_fpoly = (cdfPoint*)malloc(size);
    memcpy(state->clip_fpoly, canvas->clip_fpoly, size);
  }

  if (state->line_dashes) 
  {
    int size = state->line_dashes_count*sizeof(int);
    state->line_dashes = (int*)malloc(size);
    memcpy(state->line_dashes, canvas->line_dashes, size);
  }

  return state;
}

void cdReleaseState(cdState* state)
{
  assert(state);
  if (!state) return;

  if (state->stipple) 
    free(state->stipple);

  if (state->pattern) 
    free(state->pattern);

  if (state->clip_poly) 
    free(state->clip_poly);

  if (state->clip_fpoly) 
    free(state->clip_fpoly);

  if (state->line_dashes) 
    free(state->line_dashes);

  free(state);
  free(state);
}

void cdCanvasRestoreState(cdCanvas* canvas, cdState* state)
{
  assert(canvas);
  assert(state);
  if (!state || !_cdCheckCanvas(canvas)) return;

  /* clippling must be done in low level because origin and invert y axis */
  canvas->clip_poly_n = state->clip_poly_n;

  if (canvas->clip_poly) 
  {
    free(canvas->clip_poly);
    canvas->clip_poly = NULL;
  }

  if (canvas->clip_fpoly) 
  {
    free(canvas->clip_fpoly);
    canvas->clip_fpoly = NULL;
  }

  if (state->clip_poly) 
  {
    int size = state->clip_poly_n*sizeof(cdPoint);
    canvas->clip_poly = (cdPoint*)malloc(size);
    memcpy(canvas->clip_poly, state->clip_poly, size);
  }

  if (state->clip_fpoly) 
  {
    int size = state->clip_poly_n*sizeof(cdfPoint);
    canvas->clip_fpoly = (cdfPoint*)malloc(size);
    memcpy(canvas->clip_fpoly, state->clip_fpoly, size);
  }

  cdCanvasClip(canvas, CD_CLIPOFF);
  if (canvas->clip_fpoly)
    canvas->cxFPoly(canvas->ctxcanvas, CD_CLIP, state->clip_fpoly, state->clip_poly_n);
  else if (canvas->clip_poly)
    canvas->cxPoly(canvas->ctxcanvas, CD_CLIP, state->clip_poly, state->clip_poly_n);
  cdCanvasClipArea(canvas, state->clip_rect.xmin, state->clip_rect.xmax, state->clip_rect.ymin, state->clip_rect.ymax);
  if (canvas->cxFClipArea)
    canvas->cxFClipArea(canvas->ctxcanvas, state->clip_frect.xmin, state->clip_frect.xmax, state->clip_frect.ymin, state->clip_frect.ymax);
  else if (canvas->cxClipArea)
    canvas->cxClipArea(canvas->ctxcanvas, state->clip_rect.xmin, state->clip_rect.xmax, state->clip_rect.ymin, state->clip_rect.ymax);
  cdCanvasClip(canvas, state->clip_mode);

  /* regular attributes */
  cdCanvasSetBackground(canvas, state->background);
  cdCanvasSetForeground(canvas, state->foreground);
  cdCanvasBackOpacity(canvas, state->back_opacity);
  cdCanvasWriteMode(canvas, state->write_mode);
  cdCanvasLineStyle(canvas, state->line_style);
  cdCanvasLineWidth(canvas, state->line_width);
  cdCanvasLineCap(canvas, state->line_cap);
  cdCanvasLineJoin(canvas, state->line_join);
  cdCanvasFillMode(canvas, state->fill_mode);
  cdCanvasLineStyleDashes(canvas, state->line_dashes, state->line_dashes_count);
  cdCanvasHatch(canvas, state->hatch_style);
  if (state->stipple) cdCanvasStipple(canvas, state->stipple_w, state->stipple_h, state->stipple);
  if (state->pattern) cdCanvasPattern(canvas, state->pattern_w, state->pattern_h, state->pattern);
  cdCanvasInteriorStyle(canvas, state->interior_style);
  if (state->native_font[0])
    cdCanvasNativeFont(canvas, state->native_font);
  else
    cdCanvasFont(canvas, state->font_type_face, state->font_style, state->font_size);
  cdCanvasTextAlignment(canvas, state->text_alignment);
  cdCanvasTextOrientation(canvas, state->text_orientation);
  cdCanvasMarkType(canvas, state->mark_type);
  cdCanvasMarkSize(canvas, state->mark_size);
  cdCanvasOrigin(canvas, state->origin.x, state->origin.y);
  if (state->use_matrix)
    cdCanvasTransform(canvas, state->matrix);
  wdCanvasWindow(canvas, state->window.xmin, state->window.xmax, state->window.ymin, state->window.ymax);
  wdCanvasViewport(canvas, state->viewport.xmin, state->viewport.xmax, state->viewport.ymin, state->viewport.ymax);
  cdCanvasSimulate(canvas, state->sim_mode);

  /* complex clipping regions are not saved */
  /* driver internal attributes are not saved */
}

static cdAttribute* cd_findattrib(cdCanvas *canvas, const char* name, int *a)
{
  int i;

  for (i=0; i < canvas->attrib_n; i++)
  {
    if (strcmp(name, canvas->attrib_list[i]->name) == 0)
    {
      if (a) *a = i;
      return canvas->attrib_list[i];
    }
  }

  return NULL;
}

void cdRegisterAttribute(cdCanvas *canvas, cdAttribute* attrib)
{
  cdAttribute* old_attrib;
  int a;

  assert(canvas);
  assert(attrib);
  if (!attrib || !_cdCheckCanvas(canvas)) return;

  old_attrib = cd_findattrib(canvas, attrib->name, &a);

  if (old_attrib)
    canvas->attrib_list[a] = attrib;
  else
  {
    canvas->attrib_list[canvas->attrib_n] = attrib;
    canvas->attrib_n++;
  }
}

void cdCanvasSetAttribute(cdCanvas* canvas, const char* name, char *data)
{
  cdAttribute* attrib;

  assert(canvas);
  if (!_cdCheckCanvas(canvas)) return;

  attrib = cd_findattrib(canvas, name, NULL);
  if (attrib && attrib->set)
    attrib->set(canvas->ctxcanvas, data);
}

void cdCanvasSetfAttribute(cdCanvas* canvas, const char* name, const char* format, ...)
{
  char data[1024];
  va_list arglist;
  va_start(arglist, format);
  vsprintf(data, format, arglist);

  cdCanvasSetAttribute(canvas, name, data);
}

char* cdCanvasGetAttribute(cdCanvas* canvas, const char* name)
{
  cdAttribute* attrib;

  assert(canvas);
  if (!_cdCheckCanvas(canvas)) return NULL;

  attrib = cd_findattrib(canvas, name, NULL);
  if (attrib && attrib->get)
    return attrib->get(canvas->ctxcanvas);

  return NULL;
}

int cdCanvasPlay(cdCanvas* canvas, cdContext* context, int xmin, int xmax, int ymin, int ymax, void *data)
{
  assert(context);
  assert(canvas);
  if (!_cdCheckCanvas(canvas) || !context || !context->cxPlay) return CD_ERROR;

  /* the all can be 0 here, do not use cdCheckBoxSize */
  if (xmin > xmax) _cdSwapInt(xmin, xmax);
  if (ymin > ymax) _cdSwapInt(ymin, ymax);

  return context->cxPlay(canvas, xmin, xmax, ymin, ymax, data);
}

int cdContextRegisterCallback(cdContext *context, int cb, cdCallback func)
{
  assert(context);
  if (!context || !context->cxRegisterCallback) return CD_ERROR;
  return context->cxRegisterCallback(cb, func);
}

void cdCanvasFlush(cdCanvas* canvas)
{
  assert(canvas);
  if (!_cdCheckCanvas(canvas) || !canvas->cxFlush) return;
  canvas->cxFlush(canvas->ctxcanvas);
}

void cdCanvasClear(cdCanvas* canvas)
{
  assert(canvas);
  if (!_cdCheckCanvas(canvas) || !canvas->cxClear) return;
  canvas->cxClear(canvas->ctxcanvas);
}

int cdCanvasYAxisMode(cdCanvas* canvas, int invert)
{
  int old_invert_yaxis;
  assert(canvas);
  if (!_cdCheckCanvas(canvas)) return CD_ERROR;

  if (invert == CD_QUERY)
    return canvas->invert_yaxis;

  old_invert_yaxis = canvas->invert_yaxis;
  canvas->invert_yaxis = invert;
  return old_invert_yaxis;
}

int cdCanvasUpdateYAxis(cdCanvas* canvas, int* y)
{
  assert(canvas);
  assert(y);
  if (!_cdCheckCanvas(canvas)) return CD_ERROR;

  if(canvas->invert_yaxis)
  {
    *y = _cdInvertYAxis(canvas, *y);

    if (canvas->use_origin)
      *y -= 2*canvas->origin.y;
  }

  return *y;
}

double cdfCanvasUpdateYAxis(cdCanvas* canvas, double* y)
{
  assert(canvas);
  assert(y);
  if (!_cdCheckCanvas(canvas)) return CD_ERROR;

  if (canvas->invert_yaxis)
  {
    *y = _cdInvertYAxis(canvas, *y);

    if (canvas->use_origin)
      *y -= 2*canvas->origin.y;
  }

  return *y;
}

int cdCanvasInvertYAxis(cdCanvas* canvas, int y)
{
  int yi;
  assert(canvas);
  if (!_cdCheckCanvas(canvas)) return CD_ERROR;

  yi = _cdInvertYAxis(canvas, y);

  if (canvas->use_origin)
    yi -= 2*canvas->origin.y;

  return yi;
}

double cdfCanvasInvertYAxis(cdCanvas* canvas, double y)
{
  double yi;
  assert(canvas);
  if (!_cdCheckCanvas(canvas)) return CD_ERROR;

  yi = _cdInvertYAxis(canvas, y);

  if (canvas->use_origin)
    yi -= 2*canvas->origin.y;

  return yi;
}

void cdCanvasGetSize(cdCanvas* canvas, int *width, int *height, double *width_mm, double *height_mm)
{
  assert(canvas);
  if (!_cdCheckCanvas(canvas)) return;

  if (width) *width = canvas->w;
  if (height) *height = canvas->h;
  if (width_mm) *width_mm = canvas->w_mm;
  if (height_mm) *height_mm = canvas->h_mm;
}

void cdCanvasMM2Pixel(cdCanvas* canvas, double mm_dx, double mm_dy, int *dx, int *dy)
{
  assert(canvas);
  if (!_cdCheckCanvas(canvas)) return;

  if (dx) *dx = cdRound(mm_dx*canvas->xres);
  if (dy) *dy = cdRound(mm_dy*canvas->yres);
}

void cdCanvasPixel2MM(cdCanvas* canvas, int dx, int dy, double *mm_dx, double *mm_dy)
{
  assert(canvas);
  if (!_cdCheckCanvas(canvas)) return;

  if (mm_dx) *mm_dx = ((double)dx)/canvas->xres;
  if (mm_dy) *mm_dy = ((double)dy)/canvas->yres;
}

void cdfCanvasMM2Pixel(cdCanvas* canvas, double mm_dx, double mm_dy, double *dx, double *dy)
{
  assert(canvas);
  if (!_cdCheckCanvas(canvas)) return;

  if (dx) *dx = mm_dx*canvas->xres;
  if (dy) *dy = mm_dy*canvas->yres;
}

void cdfCanvasPixel2MM(cdCanvas* canvas, double dx, double dy, double *mm_dx, double *mm_dy)
{
  assert(canvas);
  if (!_cdCheckCanvas(canvas)) return;

  if (mm_dx) *mm_dx = dx/canvas->xres;
  if (mm_dy) *mm_dy = dy/canvas->yres;
}

/***** Context Plus Functions ********/

static int use_context_plus = 0;
static cdContext* context_plus[NUM_CONTEXTPLUS] = {NULL, NULL, NULL, NULL, NULL, NULL};

int cdUseContextPlus(int use)
{
  int old_use_context_plus = use_context_plus;

  if (use == CD_QUERY)
    return use_context_plus;

  use_context_plus = use;
  return old_use_context_plus;
}

void cdInitContextPlusList(cdContext* ctx_list[])
{
  int ctx;
  for (ctx = 0; ctx < NUM_CONTEXTPLUS; ctx++)
    if (ctx_list[ctx] != NULL)
      context_plus[ctx] = ctx_list[ctx];
}

cdContext* cdGetContextPlus(int ctx)
{
  if (ctx < 0 || ctx >= NUM_CONTEXTPLUS)
    return NULL;

  return context_plus[ctx];
}

/***** OLD Compatibility Functions ********/

int cdRegisterCallback(cdContext *context, int cb, cdCallback func)
{
  return cdContextRegisterCallback(context, cb, func);
}

cdContext* cdGetContext(cdCanvas* canvas)
{
  return cdCanvasGetContext(canvas);
}

int * cdGetClipPoly(int *n)
{
  if (n) *n = 0;
  return NULL;
}

double* wdGetClipPoly(int *n)
{
  if (n) *n = 0;
  return NULL;
}
