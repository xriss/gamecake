/** \file
 * \brief Image RGB Driver
 *
 * See Copyright Notice in cd.h
 */

#include <stdlib.h> 
#include <memory.h> 
#include <math.h> 
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "cd.h"
#include "cd_private.h"
#include "cd_truetype.h"
#include "sim.h"
#include "cdirgb.h"


struct _cdCtxImage 
{
  int w, h;
  unsigned char* red;     /* red color buffer */
  unsigned char* green;   /* green color buffer */
  unsigned char* blue;    /* blue color buffer */
  unsigned char* alpha;   /* alpha color buffer */
};


struct _cdCtxCanvas 
{
  cdCanvas* canvas;

  int user_image;         /* can not free an user image */

  unsigned char* red;     /* red color buffer */
  unsigned char* green;   /* green color buffer */
  unsigned char* blue;    /* blue color buffer */
  unsigned char* alpha;   /* alpha color buffer */
  unsigned char* clip;    /* clipping buffer */

  unsigned char* clip_region;  /* clipping region used during NewRegion */

  float  rotate_angle;
  int    rotate_center_x,
         rotate_center_y;

  cdCanvas* canvas_dbuffer; /* used by the CD_DBUFFERRGB driver */
};

/*******************/
/* Local functions */
/*******************/

#define _sNormX(_ctxcanvas, _x) (_x < 0? 0: _x < _ctxcanvas->canvas->w? _x: _ctxcanvas->canvas->w-1)
#define _sNormY(_ctxcanvas, _y) (_y < 0? 0: _y < _ctxcanvas->canvas->h? _y: _ctxcanvas->canvas->h-1)

#define RGB_COMPOSE_OVER(_SRC, _SRC_ALPHA, _DST, _TMP_MULTI, _TMP_ALPHA) (unsigned char)(((_SRC_ALPHA)*(_SRC) + (_TMP_MULTI)*(_DST)) / (_TMP_ALPHA))

#define RGBA_WRITE_MODE(_write_mode, _pdst_red, _pdst_green, _pdst_blue, _tmp_red, _tmp_green, _tmp_blue) \
{                                                                                                         \
  switch (_write_mode)                                                                                    \
  {                                                                                                       \
  case CD_REPLACE:                                                                                        \
    *_pdst_red = _tmp_red;                                                                                \
    *_pdst_green = _tmp_green;                                                                            \
    *_pdst_blue = _tmp_blue;                                                                              \
    break;                                                                                                \
  case CD_XOR:                                                                                            \
    *_pdst_red ^= _tmp_red;                                                                               \
    *_pdst_green ^= _tmp_green;                                                                           \
    *_pdst_blue ^= _tmp_blue;                                                                             \
    break;                                                                                                \
  case CD_NOT_XOR:                                                                                        \
    *_pdst_red = (unsigned char)~(_tmp_red ^ *_pdst_red);                                                 \
    *_pdst_green = (unsigned char)~(_tmp_green ^ *_pdst_green);                                           \
    *_pdst_blue = (unsigned char)~(_tmp_blue ^ *_pdst_blue);                                              \
    break;                                                                                                \
  }                                                                                                       \
}

#define RGBA_COLOR_COMBINE(_ctxcanvas, _pdst_red, _pdst_green, _pdst_blue, _pdst_alpha, _src_red, _src_green, _src_blue, _src_alpha) \
{                                                                                                                        \
  unsigned char _tmp_red = 0, _tmp_green = 0, _tmp_blue = 0;                                                             \
                                                                                                                         \
  if (_pdst_alpha)   /* destiny has alpha */                                                                             \
  {                                                                                                                      \
    if (_src_alpha != 255)   /* some transparency */                                                                     \
    {                                                                                                                    \
      if (_src_alpha != 0) /* source not full transparent */                                                             \
      {                                                                                                                  \
        if (*_pdst_alpha == 0) /* destiny full transparent */                                                            \
        {                                                                                                                \
          _tmp_red = _src_red;                                                                                           \
          _tmp_green = _src_green;                                                                                       \
          _tmp_blue = _src_blue;                                                                                         \
          *_pdst_alpha = _src_alpha;                                                                                     \
          RGBA_WRITE_MODE(CD_REPLACE, _pdst_red, _pdst_green, _pdst_blue,                                                \
                                      _tmp_red, _tmp_green, _tmp_blue);                                                  \
        }                                                                                                                \
        else if (*_pdst_alpha == 255) /* destiny opaque */                                                               \
        {                                                                                                                \
          _tmp_red = CD_ALPHA_BLEND(_src_red, *_pdst_red, _src_alpha);                                                   \
          _tmp_green = CD_ALPHA_BLEND(_src_green, *_pdst_green, _src_alpha);                                             \
          _tmp_blue = CD_ALPHA_BLEND(_src_blue, *_pdst_blue, _src_alpha);                                                \
          /* *_pdst_alpha is not changed */                                                                              \
          RGBA_WRITE_MODE(CD_REPLACE, _pdst_red, _pdst_green, _pdst_blue,                                                \
                                      _tmp_red, _tmp_green, _tmp_blue);                                                  \
        }                                                                                                                \
        else /* (0<*_pdst_alpha<255 && 0<_src_alpha<255) destiny and source are semi-transparent */                      \
        {                                                                                                                \
          /* Closed Compositing SRC over DST  (see smith95a.pdf)        */                                               \
          /* Colors NOT Premultiplied by Alpha                          */                                               \
          /* DST = SRC * SRC_ALPHA + DST * DST_ALPHA * (1 - SRC_ALPHA)  */                                               \
          /* DST_ALPHA = SRC_ALPHA + DST_ALPHA * (1 - SRC_ALPHA)        */                                               \
          /* DST /= DST_ALPHA */                                                                                         \
          int _tmp_multi = *_pdst_alpha * (255 - _src_alpha);                                                            \
          int _tmp_src_alpha = _src_alpha*255;                                                                           \
          int _tmp_alpha = _tmp_src_alpha + _tmp_multi;                                                                  \
          _tmp_red = RGB_COMPOSE_OVER(_src_red, _tmp_src_alpha, *_pdst_red, _tmp_multi, _tmp_alpha);                     \
          _tmp_green = RGB_COMPOSE_OVER(_src_green, _tmp_src_alpha, *_pdst_green, _tmp_multi, _tmp_alpha);               \
          _tmp_blue = RGB_COMPOSE_OVER(_src_blue, _tmp_src_alpha, *_pdst_blue, _tmp_multi, _tmp_alpha);                  \
          *_pdst_alpha = (unsigned char)(_tmp_alpha / 255);                                                              \
          RGBA_WRITE_MODE(CD_REPLACE, _pdst_red, _pdst_green, _pdst_blue,                                                \
                                      _tmp_red, _tmp_green, _tmp_blue);                                                  \
        }                                                                                                                \
      }                                                                                                                  \
      else  /* (_src_alpha == 0) source full transparent */                                                              \
      {                                                                                                                  \
        _tmp_red = *_pdst_red;                                                                                           \
        _tmp_green = *_pdst_green;                                                                                       \
        _tmp_blue = *_pdst_blue;                                                                                         \
        RGBA_WRITE_MODE(CD_REPLACE, _pdst_red, _pdst_green, _pdst_blue,                                                  \
                                    _tmp_red, _tmp_green, _tmp_blue);                                                    \
        /* *_pdst_alpha is not changed */                                                                                \
      }                                                                                                                  \
    }                                                                                                                    \
    else  /* (_src_alpha == 255) source has no alpha = opaque */                                                         \
    {                                                                                                                    \
      _tmp_red = _src_red;                                                                                               \
      _tmp_green = _src_green;                                                                                           \
      _tmp_blue = _src_blue;                                                                                             \
      *_pdst_alpha = (unsigned char)255;   /* set destiny as opaque */                                                   \
      RGBA_WRITE_MODE(_ctxcanvas->canvas->write_mode, _pdst_red, _pdst_green, _pdst_blue,                                \
                                                      _tmp_red, _tmp_green, _tmp_blue);                                  \
    }                                                                                                                    \
  }                                                                                                                      \
  else /* destiny does NOT have alpha */                                                                                 \
  {                                                                                                                      \
    if (_src_alpha != 255) /* source has some transparency */                                                            \
    {                                                                                                                    \
      if (_src_alpha != 0) /* source semi-transparent */                                                                 \
      {                                                                                                                  \
        _tmp_red = CD_ALPHA_BLEND(_src_red, *_pdst_red, _src_alpha);                                                     \
        _tmp_green = CD_ALPHA_BLEND(_src_green, *_pdst_green, _src_alpha);                                               \
        _tmp_blue = CD_ALPHA_BLEND(_src_blue, *_pdst_blue, _src_alpha);                                                  \
        RGBA_WRITE_MODE(CD_REPLACE, _pdst_red, _pdst_green, _pdst_blue,                                                  \
                                    _tmp_red, _tmp_green, _tmp_blue);                                                    \
      }                                                                                                                  \
      else  /* (_src_alpha == 0) source full transparent */                                                              \
      {                                                                                                                  \
        _tmp_red = *_pdst_red;                                                                                           \
        _tmp_green = *_pdst_green;                                                                                       \
        _tmp_blue = *_pdst_blue;                                                                                         \
        RGBA_WRITE_MODE(CD_REPLACE, _pdst_red, _pdst_green, _pdst_blue,                                                  \
                                    _tmp_red, _tmp_green, _tmp_blue);                                                    \
      }                                                                                                                  \
    }                                                                                                                    \
    else  /* (_src_alpha == 255) source has no alpha = opaque */                                                         \
    {                                                                                                                    \
      _tmp_red = _src_red;                                                                                               \
      _tmp_green = _src_green;                                                                                           \
      _tmp_blue = _src_blue;                                                                                             \
      RGBA_WRITE_MODE(_ctxcanvas->canvas->write_mode, _pdst_red, _pdst_green, _pdst_blue,                                \
                                                      _tmp_red, _tmp_green, _tmp_blue);                                  \
    }                                                                                                                    \
  }                                                                                                                      \
}

static void sCombineRGBColor(cdCtxCanvas* ctxcanvas, int offset, long color)
{
  unsigned char *dr = ctxcanvas->red + offset;
  unsigned char *dg = ctxcanvas->green + offset;
  unsigned char *db = ctxcanvas->blue + offset;
  unsigned char *da = ctxcanvas->alpha? ctxcanvas->alpha + offset: NULL;
  unsigned char *clip = ctxcanvas->clip + offset;

  unsigned char sr = cdRed(color);
  unsigned char sg = cdGreen(color);
  unsigned char sb = cdBlue(color); 
  unsigned char sa = cdAlpha(color);

  if (*clip)
    RGBA_COLOR_COMBINE(ctxcanvas, dr, dg, db, da, sr, sg, sb, sa);
}

static void sCombineRGB(cdCtxCanvas* ctxcanvas, int offset, unsigned char sr, unsigned char sg, unsigned char sb, unsigned char sa)
{
  unsigned char *dr = ctxcanvas->red + offset;
  unsigned char *dg = ctxcanvas->green + offset;
  unsigned char *db = ctxcanvas->blue + offset;
  unsigned char *da = ctxcanvas->alpha? ctxcanvas->alpha + offset: NULL;
  unsigned char *clip = ctxcanvas->clip + offset;

  if (*clip)
    RGBA_COLOR_COMBINE(ctxcanvas, dr, dg, db, da, sr, sg, sb, sa);
}

static void sCombineRGBLine(cdCtxCanvas* ctxcanvas, int offset, const unsigned char *sr, const unsigned char *sg, const unsigned char *sb, int size)
{
  int c;
  unsigned char *dr = ctxcanvas->red + offset;
  unsigned char *dg = ctxcanvas->green + offset;
  unsigned char *db = ctxcanvas->blue + offset;
  unsigned char *da = ctxcanvas->alpha? ctxcanvas->alpha + offset: NULL;
  unsigned char *clip = ctxcanvas->clip + offset;
  unsigned char src_a = 255;

  if (size > 0)
  {
    for (c = 0; c < size; c++)
    {
      if (*clip)
        RGBA_COLOR_COMBINE(ctxcanvas, dr, dg, db, da, *sr, *sg, *sb, src_a);
      dr++; dg++; db++; clip++;
      sr++; sg++; sb++;
      if (da) da++;
    }
  }
  else
  {
    size *= -1;
    for (c = 0; c < size; c++)
    {
      if (*clip)
        RGBA_COLOR_COMBINE(ctxcanvas, dr, dg, db, da, *sr, *sg, *sb, src_a);
      dr--; dg--; db--; clip--;
      sr--; sg--; sb--;
      if (da) da--;
    }
  }
}

static void sCombineRGBALine(cdCtxCanvas* ctxcanvas, int offset, const unsigned char *sr, const unsigned char *sg, const unsigned char *sb, const unsigned char *sa, int size)
{
  int c;
  unsigned char *dr = ctxcanvas->red + offset;
  unsigned char *dg = ctxcanvas->green + offset;
  unsigned char *db = ctxcanvas->blue + offset;
  unsigned char *da = ctxcanvas->alpha? ctxcanvas->alpha + offset: NULL;
  unsigned char *clip = ctxcanvas->clip + offset;

  if (size > 0)
  {
    for (c = 0; c < size; c++)
    {
      if (*clip)
        RGBA_COLOR_COMBINE(ctxcanvas, dr, dg, db, da, *sr, *sg, *sb, *sa);
      dr++; dg++; db++; clip++;
      sr++; sg++; sb++; sa++; 
      if (da) da++;
    }
  }
  else
  {
    size *= -1;
    for (c = 0; c < size; c++)
    {
      if (*clip)
        RGBA_COLOR_COMBINE(ctxcanvas, dr, dg, db, da, *sr, *sg, *sb, *sa);
      dr--; dg--; db--; clip--;
      sr--; sg--; sb--; sa--; 
      if (da) da--;
    }
  }
}

static void irgbSolidLine(cdCanvas* canvas, int xmin, int y, int xmax, long color)
{
  int x;
  unsigned long offset = y * canvas->w;

  if (y < 0)
    return;

  if (y > (canvas->h-1))
    return;

  if (xmin < 0)  /* Arruma limites de acordo com o retangulo de clip */
    xmin = 0;    /* so clipa em x                                    */
  if (xmax > (canvas->w-1))
    xmax = (canvas->w-1);

  for (x = xmin; x <= xmax; x++)
    sCombineRGBColor(canvas->ctxcanvas, offset + x, color);
}

static void irgbPatternLine(cdCanvas* canvas, int xmin, int xmax, int y, int pw, const long *pattern)
{
  int x, i;
  unsigned long offset = y * canvas->w;

  if (y < 0 || y > (canvas->h-1))
    return;
  
  if (xmin < 0)  /* Arruma limites de acordo com o retangulo de clip */
    xmin = 0;    /* so clipa em x                                    */
  if (xmax > (canvas->w-1))
    xmax = (canvas->w-1);

  i = xmin % pw;

  for (x = xmin; x <= xmax; x++,i++)
  {
    if (i == pw) 
      i = 0;

    sCombineRGBColor(canvas->ctxcanvas, offset + x, pattern[i]);
  }
}

static void irgbStippleLine(cdCanvas* canvas, int xmin, int xmax, int y, int pw, const unsigned char *stipple)
{
  int x,i;
  unsigned long offset = y * canvas->w;

  if (y < 0 || y > (canvas->h-1))
    return;

  if (xmin < 0)  /* Arruma limites de acordo com o retangulo de clip */
    xmin = 0;    /* so clipa em x                                    */
  if (xmax > (canvas->w-1))
    xmax = (canvas->w-1);

  i = xmin % pw;

  for (x = xmin; x <= xmax; x++,i++)
  {
    if (i == pw) 
      i = 0;
    if(stipple[i])
      sCombineRGBColor(canvas->ctxcanvas, offset + x, canvas->foreground);
    else if (canvas->back_opacity == CD_OPAQUE)
      sCombineRGBColor(canvas->ctxcanvas, offset + x, canvas->background);
  }
}

static void irgbHatchLine(cdCanvas* canvas, int xmin, int xmax, int y, unsigned char hatch)
{
  int x;
  unsigned long offset = y * canvas->w;
  unsigned char n;
  
  if (y < 0 || y > (canvas->h-1))
    return;

  if (xmin < 0)  /* Arruma limites de acordo com o retangulo de clip */
    xmin = 0;    /* so clipa em x                                    */
  if (xmax > (canvas->w-1))
    xmax = (canvas->w-1);

  n = (unsigned char)(xmin&7);
  simRotateHatchN(hatch, n);

  for (x = xmin; x <= xmax; x++)
  {
    if (hatch & 0x80)
      sCombineRGBColor(canvas->ctxcanvas, offset + x, canvas->foreground);
    else if (canvas->back_opacity == CD_OPAQUE)
      sCombineRGBColor(canvas->ctxcanvas, offset + x, canvas->background);

    _cdRotateHatch(hatch);
  }
}

/********************/
/* driver functions */
/********************/

static void cdkillcanvas(cdCtxCanvas* ctxcanvas)
{
  if (!ctxcanvas->user_image)
    free(ctxcanvas->red);

  if (ctxcanvas->clip_region)
    free(ctxcanvas->clip_region);

  free(ctxcanvas->clip);

  memset(ctxcanvas, 0, sizeof(cdCtxCanvas));
  free(ctxcanvas);
}

unsigned char* cdAlphaImage(cdCanvas* canvas)
{
  cdCtxCanvas* ctxcanvas;
  assert(canvas);
  ctxcanvas = (cdCtxCanvas*)canvas->ctxcanvas;
  return ctxcanvas->alpha;
}

unsigned char* cdRedImage(cdCanvas* canvas)
{
  cdCtxCanvas* ctxcanvas;
  assert(canvas);
  ctxcanvas = (cdCtxCanvas*)canvas->ctxcanvas;
  return ctxcanvas->red;
}

unsigned char* cdGreenImage(cdCanvas* canvas)
{
  cdCtxCanvas* ctxcanvas;
  assert(canvas);
  ctxcanvas = (cdCtxCanvas*)canvas->ctxcanvas;
  return ctxcanvas->green;
}

unsigned char* cdBlueImage(cdCanvas* canvas)
{
  cdCtxCanvas* ctxcanvas;
  assert(canvas);
  ctxcanvas = (cdCtxCanvas*)canvas->ctxcanvas;
  return ctxcanvas->blue;
}

static void cdclear(cdCtxCanvas* ctxcanvas)
{
  int size = ctxcanvas->canvas->w * ctxcanvas->canvas->h; 
  memset(ctxcanvas->red, cdRed(ctxcanvas->canvas->background), size);
  memset(ctxcanvas->green, cdGreen(ctxcanvas->canvas->background), size);
  memset(ctxcanvas->blue, cdBlue(ctxcanvas->canvas->background), size);
  if (ctxcanvas->alpha) 
    memset(ctxcanvas->alpha, cdAlpha(ctxcanvas->canvas->background), size);  /* here is the normal alpha coding */
}

static void irgPostProcessIntersect(unsigned char* clip, int size)
{
  int i;
  for(i = 0; i < size; i++)
  {
    if (*clip == 2)
      *clip = 1;
    else
      *clip = 0;

    clip++;
  }
}

#define _irgSetClipPixel(_clip, _combine_mode)             \
{                                                          \
  switch (_combine_mode)                                   \
  {                                                        \
  case CD_INTERSECT:                                       \
    if (_clip)                                             \
      _clip = 2;  /* fills the intersection                \
                   with a value to be post-processed */    \
    break;                                                 \
  case CD_DIFFERENCE:                                      \
    if (_clip)                                             \
      _clip = 0;  /* clears the intersection */            \
    break;                                                 \
  case CD_NOTINTERSECT: /* XOR */                          \
    if (_clip)                                             \
      _clip = 0;  /* clears the intersection */            \
    else                                                   \
      _clip = 1;  /* fills the region */                   \
    break;                                                 \
  default: /* CD_UNION */                                  \
    _clip = 1;    /* fills the region */                   \
    break;                                                 \
  }                                                        \
}

static void irgbClipTextBitmap(FT_Bitmap* bitmap, int x, int y, int w, unsigned char* clip, int combine_mode)
{
  unsigned char *bitmap_data;
  int width = bitmap->width;
  int height = bitmap->rows;
  int i, j;

  /* avoid spaces */
  if (width == 0 || height == 0)
    return;

  bitmap_data = bitmap->buffer + (height-1)*width;  /* bitmap is top down. */

  clip += y * w + x;

  for (i = 0; i < height; i++)
  {
    for (j = 0; j < width; j++)
    {
      if (bitmap_data[j] == 255)
        _irgSetClipPixel(clip[j], combine_mode);
    }
    clip += w;
    bitmap_data -= width;
  }
}

static void irgbClipText(cdCtxCanvas *ctxcanvas, int x, int y, const char *s, int len)
{
  cdCanvas* canvas = ctxcanvas->canvas;
  cdSimulation* simulation = canvas->simulation;
  FT_Face       face;
  FT_GlyphSlot  slot;
  FT_Matrix     matrix;                 /* transformation matrix */
  FT_Vector     pen;                    /* untransformed origin  */
  FT_Error      error;
  int i = 0;

  if (!simulation->tt_text->face)
    return;

  face = simulation->tt_text->face;
  slot = face->glyph;

  /* move the reference point to the baseline-left */
  simGetPenPos(simulation->canvas, x, y, s, len, &matrix, &pen);

  while(i<len)
  {
    /* set transformation */
    FT_Set_Transform(face, &matrix, &pen);

    /* load glyph image into the slot (erase previous one) */
    error = FT_Load_Char(face, (unsigned char)s[i], FT_LOAD_RENDER);
    if (error) {i++; continue;}  /* ignore errors */

    x = slot->bitmap_left;
    y = slot->bitmap_top-slot->bitmap.rows; /* CD image reference point is at bottom-left */

    /* now, draw to our target surface (convert position) */
    irgbClipTextBitmap(&slot->bitmap, x, y, canvas->w, ctxcanvas->clip_region, canvas->combine_mode);

    /* increment pen position */
    pen.x += slot->advance.x;
    pen.y += slot->advance.y;

    i++;
  }

  if (canvas->combine_mode == CD_INTERSECT)
    irgPostProcessIntersect(ctxcanvas->clip_region, ctxcanvas->canvas->w * ctxcanvas->canvas->h);
}

static void irgbClipFillLine(unsigned char* clip_line, int combine_mode, int x1, int x2, int width)
{
  int x;
  if (x1 < 0) x1 = 0;
  if (x2 > width-1) x2 = width-1;
  for (x = x1; x <= x2; x++)
  {
    _irgSetClipPixel(clip_line[x], combine_mode);
  }
}

static int compare_int(const int* xx1, const int* xx2)
{
  return *xx1 - *xx2;
}

static void irgbClipPoly(cdCtxCanvas* ctxcanvas, unsigned char* clip_region, cdPoint* poly, int n, int combine_mode) 
{
  /***********IMPORTANT: the reference for this function is simPolyFill in "sim_linepolyfill.c",
     if a change is made here, must be reflected there, and vice-versa */
  cdCanvas* canvas = ctxcanvas->canvas;
  unsigned char* clip_line;
  cdPoint* t_poly = NULL;
  int y_max, y_min, i, y, fill_mode, num_lines, 
      xx_count, width, height, *xx, *hh, max_hh, n_seg;
  
  /* alloc maximum number of segments */
  simLineSegment *segments = (simLineSegment *)malloc(n*sizeof(simLineSegment));

  if (canvas->use_matrix)
  {
    t_poly = malloc(sizeof(cdPoint)*n);
    memcpy(t_poly, poly, sizeof(cdPoint)*n);
    poly = t_poly;

    for(i = 0; i < n; i++)   /* must duplicate because clip poly is stored */
      cdMatrixTransformPoint(canvas->matrix, poly[i].x, poly[i].y, &poly[i].x, &poly[i].y);
  }

  width = canvas->w;
  height = canvas->h;
  fill_mode = canvas->fill_mode;
  
  simPolyMakeSegments(segments, &n_seg, poly, n, &max_hh, &y_max, &y_min);
  
  if (y_min > height-1 || y_max < 0)
  {
    free(segments);
    return;
  }
  
  if (y_min < 0) 
    y_min = 0;

  /* number of horizontal lines */
  if (y_max > height-1)
    num_lines = height-y_min;
  else
    num_lines = y_max-y_min+1;

  /* buffer to store the current horizontal intervals during the fill of an horizontal line */
  xx = (int*)malloc((n+1)*sizeof(int));    /* allocated to the maximum number of possible intervals in one line */
  hh = (int*)malloc((2*max_hh)*sizeof(int));

  /* for all horizontal lines between y_max and y_min */
  for(y = y_max; y >= y_min; y--)
  {
    xx_count = simPolyFindHorizontalIntervals(segments, n_seg, xx, hh, y, height);
    if (xx_count < 2)
      continue;
    
    clip_line = clip_region + y*width;

    /* for all intervals, fill the interval */
    for(i = 0; i < xx_count; i += 2)  /* process only pairs */
    {
      /* fills only pairs of intervals, */          
      irgbClipFillLine(clip_line, combine_mode, xx[i], xx[i+1], width);

      if ((fill_mode == CD_WINDING) &&                     /* NOT EVENODD */
          ((i+2 < xx_count) && (xx[i+1] < xx[i+2])) && /* avoid point intervals */
           simIsPointInPolyWind(poly, n, (xx[i+1]+xx[i+2])/2, y)) /* the next interval is inside the polygon */
      {
        irgbClipFillLine(clip_line, combine_mode, xx[i+1], xx[i+2], width);
      }
    }
  }

  if (t_poly) free(t_poly);
  free(xx);
  free(hh);
  free(segments);

  if (combine_mode == CD_INTERSECT)
    irgPostProcessIntersect(ctxcanvas->clip_region, ctxcanvas->canvas->w * ctxcanvas->canvas->h);
}

static void irgbClipBox(cdCtxCanvas *ctxcanvas, int xmin, int xmax, int ymin, int ymax)
{
  int combine_mode = ctxcanvas->canvas->combine_mode;
  unsigned char* clip_line;
  int x, y, width;

  if (ctxcanvas->canvas->use_matrix)
  {
    cdPoint poly[4];
    poly[0].x = xmin; poly[0].y = ymin;
    poly[1].x = xmin; poly[1].y = ymax;
    poly[2].x = xmax; poly[2].y = ymax;
    poly[3].x = xmax; poly[3].y = ymin;
    irgbClipPoly(ctxcanvas, ctxcanvas->clip_region, poly, 4, ctxcanvas->canvas->combine_mode);
    return;
  }

  xmin = _sNormX(ctxcanvas, xmin);
  ymin = _sNormY(ctxcanvas, ymin);
  xmax = _sNormX(ctxcanvas, xmax);
  ymax = _sNormY(ctxcanvas, ymax);
  width = ctxcanvas->canvas->w;

  for(y = ymin; y <= ymax; y++)
  {
    clip_line = ctxcanvas->clip_region + y*width;
    for(x = xmin; x <= xmax; x++)
    {
      _irgSetClipPixel(clip_line[x], combine_mode);
    }
  }

  if (combine_mode == CD_INTERSECT)
    irgPostProcessIntersect(ctxcanvas->clip_region, ctxcanvas->canvas->w * ctxcanvas->canvas->h);
}

static void irgbClipArea(cdCtxCanvas* ctxcanvas, int xmin, int xmax, int ymin, int ymax)
{
  unsigned char* clip_line = ctxcanvas->clip; /* set directly to clip */
  int y, xsize, ysize, height, width, xrigth;

  if (ctxcanvas->canvas->use_matrix)
  {
    cdPoint poly[4];
    poly[0].x = xmin; poly[0].y = ymin;
    poly[1].x = xmin; poly[1].y = ymax;
    poly[2].x = xmax; poly[2].y = ymax;
    poly[3].x = xmax; poly[3].y = ymin;
    memset(ctxcanvas->clip, 0, ctxcanvas->canvas->w * ctxcanvas->canvas->h);
    irgbClipPoly(ctxcanvas, ctxcanvas->clip, poly, 4, CD_UNION);
    return;
  }

  xmin = _sNormX(ctxcanvas, xmin);
  ymin = _sNormY(ctxcanvas, ymin);
  xmax = _sNormX(ctxcanvas, xmax);
  ymax = _sNormY(ctxcanvas, ymax);
  xsize = xmax-xmin+1;
  ysize = ymax-ymin+1;
  height = ctxcanvas->canvas->h;
  width = ctxcanvas->canvas->w;
  xrigth = width-(xmax+1);

  for(y = 0; y < ymin; y++)
  {
    memset(clip_line, 0, width);
    clip_line += width;
  }

  for(y = ymin; y <= ymax; y++)
  {
    if (xmin)
      memset(clip_line, 0, xmin);

    memset(clip_line+xmin, 1, xsize);

    if (xrigth)
      memset(clip_line+xmax+1, 0, xrigth);

    clip_line += width;
  }

  for(y = ymax+1; y < height; y++)
  {
    memset(clip_line, 0, width);
    clip_line += width;
  }
}

static int cdclip(cdCtxCanvas* ctxcanvas, int mode)
{
  switch(mode)
  {
  case CD_CLIPAREA: 
    irgbClipArea(ctxcanvas, ctxcanvas->canvas->clip_rect.xmin, 
                            ctxcanvas->canvas->clip_rect.xmax, 
                            ctxcanvas->canvas->clip_rect.ymin, 
                            ctxcanvas->canvas->clip_rect.ymax);
    break;
  case CD_CLIPPOLYGON:
    memset(ctxcanvas->clip, 0, ctxcanvas->canvas->w * ctxcanvas->canvas->h);
    irgbClipPoly(ctxcanvas, ctxcanvas->clip, ctxcanvas->canvas->clip_poly, ctxcanvas->canvas->clip_poly_n, CD_UNION);
    break;
  case CD_CLIPREGION:
    if (ctxcanvas->clip_region)
      memcpy(ctxcanvas->clip, ctxcanvas->clip_region, ctxcanvas->canvas->w * ctxcanvas->canvas->h);
    break;
  default:
    memset(ctxcanvas->clip, 1, ctxcanvas->canvas->w * ctxcanvas->canvas->h);  /* CD_CLIPOFF */
    break;
  }

  return mode;
}

static void cdcliparea(cdCtxCanvas* ctxcanvas, int xmin, int xmax, int ymin, int ymax)
{                
  if (ctxcanvas->canvas->clip_mode == CD_CLIPAREA)
    irgbClipArea(ctxcanvas, xmin, xmax, ymin, ymax);
}

static void cdnewregion(cdCtxCanvas* ctxcanvas)
{
  if (ctxcanvas->clip_region)
    free(ctxcanvas->clip_region);
  ctxcanvas->clip_region = malloc(ctxcanvas->canvas->w * ctxcanvas->canvas->h);
  memset(ctxcanvas->clip_region, 0, ctxcanvas->canvas->w * ctxcanvas->canvas->h);
}

static int cdispointinregion(cdCtxCanvas* ctxcanvas, int x, int y)
{
  if (!ctxcanvas->clip_region)
    return 0;

  if (x >= 0  && y >= 0 && x < ctxcanvas->canvas->w && y < ctxcanvas->canvas->h)
  {
    if (ctxcanvas->clip_region[y*ctxcanvas->canvas->w + x])
      return 1;
  }

  return 0;
}

static void cdoffsetregion(cdCtxCanvas* ctxcanvas, int dx, int dy)
{
  unsigned char* clip_region = ctxcanvas->clip_region;
  int x, y, X, Y, old_X, old_Y, width, height;

  if (!ctxcanvas->clip_region)
    return;

  height = ctxcanvas->canvas->h;
  width = ctxcanvas->canvas->w;

  for (y = 0; y < height; y++)
  {
    if (dy > 0)
      Y = height-1 - y;
    else
      Y = y;
    old_Y = Y - dy;
    for(x = 0; x < width; x++)
    {
      if (dx > 0)
        X = width-1 - x;
      else
        X = x;
      old_X = X - dx;

      if (old_X >= 0  && old_Y >= 0 && old_Y < height && old_X < width)
        clip_region[Y*width + X] = clip_region[old_Y*width + old_X];
      else
        clip_region[Y*width + X] = 0;
    }
  }
}

static void cdgetregionbox(cdCtxCanvas* ctxcanvas, int *xmin, int *xmax, int *ymin, int *ymax)
{
  unsigned char* clip_line = ctxcanvas->clip_region;
  int x, y, width, height;

  if (!ctxcanvas->clip_region)
    return;

  *xmin = ctxcanvas->canvas->w-1;
  *xmax = 0;
  *ymin = ctxcanvas->canvas->h-1;
  *ymax = 0;
  height = ctxcanvas->canvas->h;
  width = ctxcanvas->canvas->w;

  for (y = 0; y < height; y++)
  {
    for(x = 0; x < width; x++)
    {
      if (*clip_line)
      {
        if (x < *xmin)
          *xmin = x;
        if (y < *ymin)
          *ymin = y;
        if (x > *xmax)
          *xmax = x;
        if (y > *ymax)
          *ymax = y;
      }

      clip_line++;
    }
  }
}

static void cdbox(cdCtxCanvas *ctxcanvas, int xmin, int xmax, int ymin, int ymax)
{
  if (ctxcanvas->canvas->new_region)
  {
    /* matrix transformation is done inside irgbClip* if necessary */
    irgbClipBox(ctxcanvas, xmin, xmax, ymin, ymax);
    return;
  }

  cdSimBox(ctxcanvas, xmin, xmax, ymin, ymax);
}

static void cdfbox(cdCtxCanvas *ctxcanvas, double xmin, double xmax, double ymin, double ymax)
{
  if (ctxcanvas->canvas->new_region)
  {
    /* matrix transformation is done inside irgbClip* if necessary */
    irgbClipBox(ctxcanvas, _cdRound(xmin), _cdRound(xmax), _cdRound(ymin), _cdRound(ymax));
    return;
  }

  cdfSimBox(ctxcanvas, xmin, xmax, ymin, ymax);
}

static void cdtext(cdCtxCanvas *ctxcanvas, int x, int y, const char *s, int len)
{
  if (ctxcanvas->canvas->new_region)
  {
    /* matrix transformation is done inside irgbClip* if necessary */
    irgbClipText(ctxcanvas, x, y, s, len);
    return;
  }

  cdSimTextFT(ctxcanvas, x, y, s, len);
}

static void cdpoly(cdCtxCanvas* ctxcanvas, int mode, cdPoint* poly, int n)
{
  if (ctxcanvas->canvas->new_region)
  {
    /* matrix transformation is done inside irgbClip* if necessary */
    irgbClipPoly(ctxcanvas, ctxcanvas->clip_region, poly, n, ctxcanvas->canvas->combine_mode);
    return;
  }

  if (mode == CD_CLIP)
  {
    /* set directly to clip */

    /* CD_CLIPOFF */
    memset(ctxcanvas->clip, 1, ctxcanvas->canvas->w * ctxcanvas->canvas->h);  

    /* matrix transformation is done inside irgbClip* if necessary */
    irgbClipPoly(ctxcanvas, ctxcanvas->clip, poly, n, CD_UNION);
  }
  else
    cdSimPoly(ctxcanvas, mode, poly, n);
}

static void cdgetimagergb(cdCtxCanvas* ctxcanvas, unsigned char *r, unsigned char *g, unsigned char *b, int x, int y, int w, int h)
{
  int dst_offset, src_offset, l, xsize, ysize, xpos, ypos;
  unsigned char *src_red, *src_green, *src_blue;

  if (x >= ctxcanvas->canvas->w || y >= ctxcanvas->canvas->h || 
      x + w < 0 || y + h < 0)
    return;

  /* ajusta parametros de entrada */
  xpos = _sNormX(ctxcanvas, x);
  ypos = _sNormY(ctxcanvas, y);

  xsize = w < (ctxcanvas->canvas->w - xpos)? w: ctxcanvas->canvas->w - xpos;
  ysize = h < (ctxcanvas->canvas->h - ypos)? h: ctxcanvas->canvas->h - ypos;

  /* ajusta posicao inicial em source */
  src_offset = xpos + ypos * ctxcanvas->canvas->w;
  src_red = ctxcanvas->red + src_offset;
  src_green = ctxcanvas->green + src_offset;
  src_blue = ctxcanvas->blue + src_offset;

  /* offset para source */
  src_offset = ctxcanvas->canvas->w;

  /* ajusta posicao inicial em destine */
  dst_offset = (xpos - x) + (ypos - y) * w;
  r += dst_offset;
  g += dst_offset;
  b += dst_offset;

  for (l = 0; l < ysize; l++)
  {
    memcpy(r, src_red, xsize);
    memcpy(g, src_green, xsize);
    memcpy(b, src_blue, xsize);

    src_red += src_offset;
    src_green += src_offset;
    src_blue += src_offset;

    r += w;
    g += w;
    b += w;
  }
}

static void sFixImageY(int *topdown, int *y, int *h)
{
  if (*h < 0)
  {
    *h = -(*h);
    *y -= (*h - 1);    /* y is at top-left, move it to bottom-left */
    *topdown = 1; /* image pointer will start at top-left     */
  }
  else
    *topdown = 0;
}

static void cdputimagerectrgba_matrix(cdCtxCanvas* ctxcanvas, int iw, int ih, const unsigned char *r, const unsigned char *g, const unsigned char *b, const unsigned char *a, int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax)
{
  int t_xmin, t_xmax, t_ymin, t_ymax, 
      t_x, t_y, topdown, dst_offset;
  float i_x, i_y, xfactor, yfactor;
  unsigned char sr, sg, sb, sa = 255;
  double inv_matrix[6];

  sFixImageY(&topdown, &y, &h);

  /* calculate the destination limits */
  cdImageRGBCalcDstLimits(ctxcanvas->canvas, x, y, w, h, &t_xmin, &t_xmax, &t_ymin, &t_ymax, NULL);

  /* Setup inverse transform */
  cdImageRGBInitInverseTransform(w, h, xmin, xmax, ymin, ymax, &xfactor, &yfactor, ctxcanvas->canvas->matrix, inv_matrix);

  /* for all pixels in the destiny area */
  for(t_y = t_ymin; t_y <= t_ymax; t_y++)
  {
    dst_offset = t_y * ctxcanvas->canvas->w;

    for(t_x = t_xmin; t_x <= t_xmax; t_x++)
    {
      cdImageRGBInverseTransform(t_x, t_y, &i_x, &i_y, xfactor, yfactor, xmin, ymin, x, y, inv_matrix);

      if (i_x > xmin && i_y > ymin && i_x < xmax+1 && i_y < ymax+1)
      {
        if (topdown)  /* image is top-bottom */
          i_y = ih-1 - i_y;

        if (t_x == 350 && t_y == 383)
          t_x = 350;

        sr = cdBilinearInterpolation(iw, ih, r, i_x, i_y);
        sg = cdBilinearInterpolation(iw, ih, g, i_x, i_y);
        sb = cdBilinearInterpolation(iw, ih, b, i_x, i_y);
        if (a) sa = cdBilinearInterpolation(iw, ih, a, i_x, i_y);

        if (sr > 210 && sg > 210 && sb > 210)
          sr = sr;

        sCombineRGB(ctxcanvas, t_x + dst_offset, sr, sg, sb, sa);
      }
    }
  }
}

static void cdputimagerectmap_matrix(cdCtxCanvas* ctxcanvas, int iw, int ih, const unsigned char *index, const long int *colors, int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax)
{
  int t_xmin, t_xmax, t_ymin, t_ymax, 
      t_x, t_y, topdown, dst_offset;
  float i_x, i_y, xfactor, yfactor;
  unsigned char si;
  double inv_matrix[6];

  sFixImageY(&topdown, &y, &h);

  /* calculate the destination limits */
  cdImageRGBCalcDstLimits(ctxcanvas->canvas, x, y, w, h, &t_xmin, &t_xmax, &t_ymin, &t_ymax, NULL);

  /* Setup inverse transform */
  cdImageRGBInitInverseTransform(w, h, xmin, xmax, ymin, ymax, &xfactor, &yfactor, ctxcanvas->canvas->matrix, inv_matrix);

  /* for all pixels in the destiny area */
  for(t_y = t_ymin; t_y <= t_ymax; t_y++)
  {
    dst_offset = t_y * ctxcanvas->canvas->w;

    for(t_x = t_xmin; t_x <= t_xmax; t_x++)
    {
      cdImageRGBInverseTransform(t_x, t_y, &i_x, &i_y, xfactor, yfactor, xmin, ymin, x, y, inv_matrix);

      if (i_x > xmin && i_y > ymin && i_x < xmax+1 && i_y < ymax+1)
      {
        if (topdown)  /* image is top-bottom */
          i_y = ih-1 - i_y;

        si = cdZeroOrderInterpolation(iw, ih, index, i_x, i_y);
        sCombineRGBColor(ctxcanvas, t_x + dst_offset, colors[si]);
      }
    }
  }
}

static void cdputimagerectrgb(cdCtxCanvas* ctxcanvas, int iw, int ih, const unsigned char *r, const unsigned char *g, const unsigned char *b, int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax)
{
  int l, c, xsize, ysize, xpos, ypos, src_offset, dst_offset, rh, rw, topdown;
  const unsigned char *src_red, *src_green, *src_blue;

  if (ctxcanvas->canvas->use_matrix)
  {
    cdputimagerectrgba_matrix(ctxcanvas, iw, ih, r, g, b, NULL, x, y, w, h, xmin, xmax, ymin, ymax);
    return;
  }

  sFixImageY(&topdown, &y, &h);

  /* verifica se esta dentro da area de desenho */
  if (x > (ctxcanvas->canvas->w-1) || y > (ctxcanvas->canvas->h-1) || 
      (x+w) < 0 || (y+h) < 0)
    return;

  xpos = x < 0? 0: x;
  ypos = y < 0? 0: y;

  xsize = (x+w) < (ctxcanvas->canvas->w-1)+1? (x+w) - xpos: (ctxcanvas->canvas->w-1) - xpos + 1;
  ysize = (y+h) < (ctxcanvas->canvas->h-1)+1? (y+h) - ypos: (ctxcanvas->canvas->h-1) - ypos + 1;

  rw = xmax-xmin+1;
  rh = ymax-ymin+1;

  /* testa se tem que fazer zoom */
  if (rw != w || rh != h)
  {
    int* XTab = cdGetZoomTable(w, rw, xmin);
    int* YTab = cdGetZoomTable(h, rh, ymin);

    /* ajusta posicao inicial em destine */
    dst_offset = xpos + ypos * ctxcanvas->canvas->w;

    for(l = 0; l < ysize; l++)
    {
      /* ajusta posicao inicial em source */
      if (topdown)
        src_offset = YTab[(ih - 1) - (l + (ypos - y))] * iw;
      else
        src_offset = YTab[l + (ypos - y)] * iw;

      src_red = r + src_offset;
      src_green = g + src_offset;
      src_blue = b + src_offset;

      for(c = 0; c < xsize; c++)
      {
        src_offset = XTab[c + (xpos - x)];
        sCombineRGB(ctxcanvas, c + dst_offset, src_red[src_offset], src_green[src_offset], src_blue[src_offset], 255);
      }

      dst_offset += ctxcanvas->canvas->w;
    }

    free(XTab);
    free(YTab);
  }
  else
  {
    /* ajusta posicao inicial em destine */
    dst_offset = xpos + ypos * ctxcanvas->canvas->w;

    /* ajusta posicao inicial em source */
    if (topdown)
      src_offset = (xpos - x + xmin) + ((ih - 1) - (ypos - y + ymin)) * iw;
    else
      src_offset = (xpos - x + xmin) + (ypos - y + ymin) * iw;

    r += src_offset;
    g += src_offset;
    b += src_offset;

    for (l = 0; l < ysize; l++)
    {
      sCombineRGBLine(ctxcanvas, dst_offset, r, g, b, xsize);

      dst_offset += ctxcanvas->canvas->w;

      if (topdown)
      {
        r -= iw;
        g -= iw;
        b -= iw;
      }
      else
      {
        r += iw;
        g += iw;
        b += iw;
      }
    }
  }
}

static void cdputimagerectrgba(cdCtxCanvas* ctxcanvas, int iw, int ih, const unsigned char *r, const unsigned char *g, const unsigned char *b, const unsigned char *a, int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax)
{
  int l, c, xsize, ysize, xpos, ypos, src_offset, dst_offset, rw, rh, topdown;
  const unsigned char *src_red, *src_green, *src_blue, *src_alpha;

  if (ctxcanvas->canvas->use_matrix)
  {
    cdputimagerectrgba_matrix(ctxcanvas, iw, ih, r, g, b, a, x, y, w, h, xmin, xmax, ymin, ymax);
    return;
  }

  sFixImageY(&topdown, &y, &h);

  /* verifica se esta dentro da area de desenho */
  if (x > (ctxcanvas->canvas->w-1) || y > (ctxcanvas->canvas->h-1) || 
       (x+w) < 0 || (y+h) < 0)
    return;

  xpos = x < 0? 0: x;
  ypos = y < 0? 0: y;

  xsize = (x+w) < (ctxcanvas->canvas->w-1)+1? (x+w) - xpos: (ctxcanvas->canvas->w-1) - xpos + 1;
  ysize = (y+h) < (ctxcanvas->canvas->h-1)+1? (y+h) - ypos: (ctxcanvas->canvas->h-1) - ypos + 1;

  rw = xmax-xmin+1;
  rh = ymax-ymin+1;

  /* testa se tem que fazer zoom */
  if (rw != w || rh != h)
  {
    int* XTab = cdGetZoomTable(w, rw, xmin);
    int* YTab = cdGetZoomTable(h, rh, ymin);

    /* ajusta posicao inicial em destine */
    dst_offset = xpos + ypos * ctxcanvas->canvas->w;

    for(l = 0; l < ysize; l++)
    {
      /* ajusta posicao inicial em source */
      if (topdown)
        src_offset = YTab[(ih - 1) - (l + (ypos - y))] * iw;
      else
        src_offset = YTab[l + (ypos - y)] * iw;

      src_red = r + src_offset;
      src_green = g + src_offset;
      src_blue = b + src_offset;
      src_alpha = a + src_offset;

      for(c = 0; c < xsize; c++)
      {
        src_offset = XTab[c + (xpos - x)];
        sCombineRGB(ctxcanvas, c + dst_offset, src_red[src_offset], src_green[src_offset], src_blue[src_offset], src_alpha[src_offset]);
      }

      dst_offset += ctxcanvas->canvas->w;
    }

    free(XTab);
    free(YTab);
  }
  else
  {
    /* ajusta posicao inicial em destine */
    dst_offset = xpos + ypos * ctxcanvas->canvas->w;

    /* ajusta posicao inicial em source */
    if (topdown)
      src_offset = (xpos - x + xmin) + ((ih - 1) - (ypos - y + ymin)) * iw;
    else
      src_offset = (xpos - x + xmin) + (ypos - y + ymin) * iw;

    r += src_offset;
    g += src_offset;
    b += src_offset;
    a += src_offset;

    for (l = 0; l < ysize; l++)
    {
      sCombineRGBALine(ctxcanvas, dst_offset, r, g, b, a, xsize);

      dst_offset += ctxcanvas->canvas->w;

      if (topdown)
      {
        r -= iw;
        g -= iw;
        b -= iw;
        a -= iw;
      }
      else
      {
        r += iw;
        g += iw;
        b += iw;
        a += iw;
      }
    }
  }
}

static void cdputimagerectmap(cdCtxCanvas* ctxcanvas, int iw, int ih, const unsigned char *index, const long int *colors, int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax)
{
  int l, c, xsize, ysize, xpos, ypos, src_offset, dst_offset, rw, rh, idx, topdown;
  const unsigned char *src_index;

  if (ctxcanvas->canvas->use_matrix)
  {
    cdputimagerectmap_matrix(ctxcanvas, iw, ih, index, colors, x, y, w, h, xmin, xmax, ymin, ymax);
    return;
  }

  sFixImageY(&topdown, &y, &h);

  /* verifica se esta dentro da area de desenho */
  if (x > (ctxcanvas->canvas->w-1) || y > (ctxcanvas->canvas->h-1) || 
       (x+w) < 0 || (y+h) < 0)
    return;

  xpos = x < 0? 0: x;
  ypos = y < 0? 0: y;

  xsize = (x+w) < (ctxcanvas->canvas->w-1)+1? (x+w) - xpos: (ctxcanvas->canvas->w-1) - xpos + 1;
  ysize = (y+h) < (ctxcanvas->canvas->h-1)+1? (y+h) - ypos: (ctxcanvas->canvas->h-1) - ypos + 1;

  rw = xmax-xmin+1;
  rh = ymax-ymin+1;

  /* testa se tem que fazer zoom */
  if (rw != w || rh != h)
  {
    int* XTab = cdGetZoomTable(w, rw, xmin);
    int* YTab = cdGetZoomTable(h, rh, ymin);

    /* ajusta posicao inicial em destine */
    dst_offset = xpos + ypos * ctxcanvas->canvas->w;

    for(l = 0; l < ysize; l++)
    {
      /* ajusta posicao inicial em source */
      if (topdown)
        src_offset = YTab[(ih - 1) - (l + (ypos - y))] * iw;
      else
        src_offset = YTab[l + (ypos - y)] * iw;

      src_index = index + src_offset;

      for(c = 0; c < xsize; c++)
      {
        src_offset = XTab[c + (xpos - x)];
        idx = src_index[src_offset];
        sCombineRGBColor(ctxcanvas, c + dst_offset, colors[idx]);
      }

      dst_offset += ctxcanvas->canvas->w;
    }

    free(XTab);
    free(YTab);
  }
  else
  {
    /* ajusta posicao inicial em destine */
    dst_offset = xpos + ypos * ctxcanvas->canvas->w;

    /* ajusta posicao inicial em source */
    if (topdown)
      src_offset = (xpos - x + xmin) + ((ih - 1) - (ypos - y + ymin)) * iw;
    else
      src_offset = (xpos - x + xmin) + (ypos - y + ymin) * iw;

    index += src_offset;

    for (l = 0; l < ysize; l++)
    {
      for(c = 0; c < xsize; c++)
      {
        idx = index[c];
        sCombineRGBColor(ctxcanvas, c + dst_offset, colors[idx]);
      }

      dst_offset += ctxcanvas->canvas->w;

      if (topdown)
        index -= iw;
      else
        index += iw;
    }
  }
}

static void cdpixel(cdCtxCanvas* ctxcanvas, int x, int y, long int color)
{
  int offset;

  if (ctxcanvas->canvas->use_matrix)
    cdMatrixTransformPoint(ctxcanvas->canvas->matrix, x, y, &x, &y);

  offset = ctxcanvas->canvas->w * y + x;

  /* verifica se esta dentro da area de desenho */
  if (x < 0 ||
      x > (ctxcanvas->canvas->w-1) ||
      y < 0 ||
      y > (ctxcanvas->canvas->h-1))
  return;

  sCombineRGBColor(ctxcanvas, offset, color);
}

static cdCtxImage* cdcreateimage(cdCtxCanvas* ctxcanvas, int w, int h)
{
  cdCtxImage* ctximage;
  int size = w * h;
  int num_c = ctxcanvas->alpha? 4: 3;

  ctximage = (cdCtxImage*)malloc(sizeof(cdCtxImage));
  memset(ctximage, 0, sizeof(cdCtxImage));

  ctximage->w = w;
  ctximage->h = h;

  ctximage->red = (unsigned char*) malloc(num_c*size);
  if (!ctximage->red)
  {
    free(ctximage);
    return NULL;
  }

  ctximage->green = ctximage->red + size;
  ctximage->blue = ctximage->red + 2*size;
  if (ctxcanvas->alpha)
    ctximage->alpha = ctximage->red + 3*size;

  memset(ctximage->red, 0xFF, 3*size);  /* white */
  if (ctximage->alpha) memset(ctximage->alpha, 0, size);  /* transparent, this is the normal alpha coding */

  return ctximage;
}

static void cdgetimage(cdCtxCanvas* ctxcanvas, cdCtxImage* ctximage, int x, int y)
{
  unsigned char *r, *g, *b, *a = NULL;
  int w, h, dst_offset, src_offset, l, xsize, ysize, xpos, ypos, do_alpha = 0;
  unsigned char *src_red, *src_green, *src_blue, *src_alpha = NULL;

  w = ctximage->w;
  h = ctximage->h;

  if (x >= ctxcanvas->canvas->w || y >= ctxcanvas->canvas->h || 
      x + w < 0 || y + h < 0)
    return;

  if (ctximage->alpha && ctxcanvas->alpha)
    do_alpha = 1;

  r = ctximage->red;
  g = ctximage->green;
  b = ctximage->blue;
  if (do_alpha) a = ctximage->alpha;

  /* ajusta parametros de entrada */
  xpos = _sNormX(ctxcanvas, x);
  ypos = _sNormY(ctxcanvas, y);

  xsize = w < (ctxcanvas->canvas->w - xpos)? w: ctxcanvas->canvas->w - xpos;
  ysize = h < (ctxcanvas->canvas->h - ypos)? h: ctxcanvas->canvas->h - ypos;

  /* ajusta posicao inicial em source */
  src_offset = xpos + ypos * ctxcanvas->canvas->w;
  src_red = ctxcanvas->red + src_offset;
  src_green = ctxcanvas->green + src_offset;
  src_blue = ctxcanvas->blue + src_offset;
  if (do_alpha) src_alpha = ctxcanvas->alpha + src_offset;

  /* offset para source */
  src_offset = ctxcanvas->canvas->w;

  /* ajusta posicao inicial em destine */
  dst_offset = (xpos - x) + (ypos - y) * w;
  r += dst_offset;
  g += dst_offset;
  b += dst_offset;
  if (do_alpha) a += dst_offset;

  for (l = 0; l < ysize; l++)
  {
    memcpy(r, src_red, xsize);
    memcpy(g, src_green, xsize);
    memcpy(b, src_blue, xsize);
    if (do_alpha) memcpy(a, src_alpha, xsize);

    src_red += src_offset;
    src_green += src_offset;
    src_blue += src_offset;
    if (do_alpha) src_alpha += src_offset;

    r += w;
    g += w;
    b += w;
    if (do_alpha) a += w;
  }
}

static void cdputimagerect(cdCtxCanvas* ctxcanvas, cdCtxImage* ctximage, int x, int y, int xmin, int xmax, int ymin, int ymax)
{
  int iw, ih, w, h;
  unsigned char *r, *g, *b, *a;
  int l, xsize, ysize, xpos, ypos, src_offset, dst_offset;

  iw = ctximage->w;
  ih = ctximage->h;

  r = ctximage->red;
  g = ctximage->green;
  b = ctximage->blue;
  a = ctximage->alpha;

  w = xmax-xmin+1;
  h = ymax-ymin+1;

  /* verifica se esta dentro da area de desenho */
  if (x > (ctxcanvas->canvas->w-1) || y > (ctxcanvas->canvas->h-1) || 
      x + w < 0 || y + h < 0)
    return;

  xpos = x;
  ypos = y;

  if (ypos < 0) ypos = 0;
  if (xpos < 0) xpos = 0;

  xsize = w < ((ctxcanvas->canvas->w-1)+1 - xpos)? w: ((ctxcanvas->canvas->w-1)+1 - xpos);
  ysize = h < ((ctxcanvas->canvas->h-1)+1 - ypos)? h: ((ctxcanvas->canvas->h-1)+1 - ypos);

  /* ajusta posicao inicial em destine */
  dst_offset = xpos + ypos * ctxcanvas->canvas->w;

  /* ajusta posicao inicial em source */
  src_offset = ((xpos - x) + xmin) + ((ypos - y) + ymin) * iw;
  r += src_offset;
  g += src_offset;
  b += src_offset;
  if (a) a += src_offset;

  for (l = 0; l < ysize; l++)
  {
    if (a)
      sCombineRGBALine(ctxcanvas, dst_offset, r, g, b, a, xsize);
    else
      sCombineRGBLine(ctxcanvas, dst_offset, r, g, b, xsize);

    dst_offset += ctxcanvas->canvas->w;

    r += iw;
    g += iw;
    b += iw;
    if (a) a += iw;
  }
}

static void cdkillimage(cdCtxImage* ctximage)
{
  free(ctximage->red);
  memset(ctximage, 0, sizeof(cdCtxImage));
  free(ctximage);
}

static void cdscrollarea(cdCtxCanvas* ctxcanvas, int xmin, int xmax, int ymin, int ymax, int dx, int dy)
{
  int l;
  long src_offset, dst_offset;
  int incx,incy, xsize, ysize;
  int dst_xmin, dst_xmax, dst_ymin, dst_ymax;

  /* corrige valores de entrada */

  xmin = _sNormX(ctxcanvas, xmin);
  ymin = _sNormY(ctxcanvas, ymin);
  xmax = _sNormX(ctxcanvas, xmax);
  ymax = _sNormY(ctxcanvas, ymax);

  dst_xmin = xmin + dx;
  dst_ymin = ymin + dy;
  dst_xmax = xmax + dx;
  dst_ymax = ymax + dy;

  /* verifica se esta dentro da area de desenho */
  if (dst_xmin > (ctxcanvas->canvas->w-1) || dst_ymin > (ctxcanvas->canvas->h-1) || 
      dst_xmax < 0 || dst_ymax < 0)
    return;

  if (dst_ymin < 0) dst_ymin = 0;
  if (dst_xmin < 0) dst_xmin = 0;

  if (dst_ymax > (ctxcanvas->canvas->h-1)) dst_ymax = (ctxcanvas->canvas->h-1);
  if (dst_xmax > (ctxcanvas->canvas->w-1)) dst_xmax = (ctxcanvas->canvas->w-1);

  if (dst_xmin > dst_xmax || dst_ymin > dst_ymax)
    return;

  /* Decide de onde vai comecar a copiar, isto e' necessario pois pode haver 
     uma intersecao entre a imagem original e a nova imagem, assim devo 
     garantir que nao estou colocando um ponto, em cima de um ponto ainda nao
     lido da imagem antiga. */

  xsize = dst_xmax - dst_xmin + 1;
  ysize = dst_ymax - dst_ymin + 1;

  /* sentido de copia da direita para a esquerda ou ao contrario. */
  if (dx < 0)
  {
    incx = 1;
    dst_offset = dst_xmin;
    src_offset = xmin;
  }
  else
  {
    incx = -1;
    dst_offset = dst_xmax;
    src_offset = xmax;
  }

  /* sentido de copia de cima para baixo ou ao contrario. */
  if (dy < 0)
  {
    incy = ctxcanvas->canvas->w;
    dst_offset += dst_ymin * ctxcanvas->canvas->w;
    src_offset += ymin * ctxcanvas->canvas->w;
  }
  else
  {
    incy = -(ctxcanvas->canvas->w);
    dst_offset += dst_ymax * ctxcanvas->canvas->w;
    src_offset += ymax * ctxcanvas->canvas->w;
  }

  xsize *= incx;

  for (l = 0; l < ysize; l++)
  {
    sCombineRGBLine(ctxcanvas, dst_offset, ctxcanvas->red + src_offset, ctxcanvas->green + src_offset, ctxcanvas->blue + src_offset, xsize);
    dst_offset += incy;
    src_offset += incy;
  }
}

static char* get_green_attrib(cdCtxCanvas* ctxcanvas)
{
  return (char*)ctxcanvas->green;
}

static cdAttribute green_attrib =
{
  "GREENIMAGE",
  NULL,
  get_green_attrib
}; 

static char* get_blue_attrib(cdCtxCanvas* ctxcanvas)
{
  return (char*)ctxcanvas->blue;
}

static cdAttribute blue_attrib =
{
  "BLUEIMAGE",
  NULL,
  get_blue_attrib
}; 

static char* get_red_attrib(cdCtxCanvas* ctxcanvas)
{
  return (char*)ctxcanvas->red;
}

static cdAttribute red_attrib =
{
  "REDIMAGE",
  NULL,
  get_red_attrib
}; 

static char* get_alpha_attrib(cdCtxCanvas* ctxcanvas)
{
  return (char*)ctxcanvas->alpha;
}

static cdAttribute alpha_attrib =
{
  "ALPHAIMAGE",
  NULL,
  get_alpha_attrib
}; 

static void set_aa_attrib(cdCtxCanvas* ctxcanvas, char* data)
{
  if (!data || data[0] == '0')
    ctxcanvas->canvas->simulation->antialias = 0;
  else
    ctxcanvas->canvas->simulation->antialias = 1;
}

static char* get_aa_attrib(cdCtxCanvas* ctxcanvas)
{
  if (ctxcanvas->canvas->simulation->antialias)
    return "0";
  else
    return "1";
}

static cdAttribute aa_attrib =
{
  "ANTIALIAS",
  set_aa_attrib,
  get_aa_attrib
}; 

static void set_rotate_attrib(cdCtxCanvas* ctxcanvas, char* data)
{
  if (data)
  {
    /* use this configuration when there is NO native tranformation support */
    sscanf(data, "%g %d %d", &ctxcanvas->rotate_angle,
                             &ctxcanvas->rotate_center_x,
                             &ctxcanvas->rotate_center_y);

    cdCanvasTransformTranslate(ctxcanvas->canvas, ctxcanvas->rotate_center_x, ctxcanvas->rotate_center_y);
    cdCanvasTransformRotate(ctxcanvas->canvas, ctxcanvas->rotate_angle);
    cdCanvasTransformTranslate(ctxcanvas->canvas, -ctxcanvas->rotate_center_x, -ctxcanvas->rotate_center_y);
  }
  else
  {
    ctxcanvas->rotate_angle = 0;
    ctxcanvas->rotate_center_x = 0;
    ctxcanvas->rotate_center_y = 0;

    cdCanvasTransform(ctxcanvas->canvas, NULL);
  }
}

static char* get_rotate_attrib(cdCtxCanvas* ctxcanvas)
{
  static char data[100];

  if (!ctxcanvas->rotate_angle)
    return NULL;

  sprintf(data, "%g %d %d", (double)ctxcanvas->rotate_angle,
                            ctxcanvas->rotate_center_x,
                            ctxcanvas->rotate_center_y);

  return data;
}

static cdAttribute rotate_attrib =
{
  "ROTATE",
  set_rotate_attrib,
  get_rotate_attrib
}; 

static void cdcreatecanvas(cdCanvas* canvas, void *data)
{
  cdCtxCanvas* ctxcanvas;
  int w = 0, h = 0, use_alpha = 0;
  float res = (float)3.78;
  unsigned char *r = NULL, *g = NULL, *b = NULL, *a = NULL;
  char* str_data = (char*)data;
  char* res_ptr = NULL;

  if (strstr(str_data, "-a"))
    use_alpha = 1;

  res_ptr = strstr(str_data, "-r");
  if (res_ptr)
    sscanf(res_ptr+2, "%g", &res);

  /* size and rgb */
#ifdef SunOS_OLD
  if (use_alpha)
    sscanf(str_data, "%dx%d %d %d %d %d", &w, &h, &r, &g, &b, &a);
  else
    sscanf(str_data, "%dx%d %d %d %d", &w, &h, &r, &g, &b);
#else
  if (use_alpha)
    sscanf(str_data, "%dx%d %p %p %p %p", &w, &h, &r, &g, &b, &a);
  else
    sscanf(str_data, "%dx%d %p %p %p", &w, &h, &r, &g, &b);
#endif

  if (w == 0 || h == 0)
    return;

  ctxcanvas = (cdCtxCanvas *)malloc(sizeof(cdCtxCanvas));
  memset(ctxcanvas, 0, sizeof(cdCtxCanvas));

  canvas->w = w;
  canvas->h = h;
  canvas->yres = res;
  canvas->xres = res;
  canvas->w_mm = ((double)w) / res;
  canvas->h_mm = ((double)h) / res;
  if (use_alpha)
    canvas->bpp = 32;
  else
    canvas->bpp = 24;

  if (r && g && b)
  {
    ctxcanvas->user_image = 1;

    ctxcanvas->red = r;
    ctxcanvas->green = g;
    ctxcanvas->blue = b;
    ctxcanvas->alpha = a;
  }
  else
  {
    int size = w * h;
    int num_c = use_alpha? 4: 3;

    ctxcanvas->user_image = 0;

    ctxcanvas->red = (unsigned char*)malloc(num_c*size);
    if (!ctxcanvas->red)
    {
      free(ctxcanvas);
      return;
    }

    ctxcanvas->green = ctxcanvas->red + size;
    ctxcanvas->blue = ctxcanvas->red + 2*size;
    if (use_alpha) 
      ctxcanvas->alpha = ctxcanvas->red + 3*size;

    memset(ctxcanvas->red, 0xFF, 3*size);  /* white */
    if (ctxcanvas->alpha) memset(ctxcanvas->alpha, 0, size);  /* transparent, this is the normal alpha coding */
  }

  ctxcanvas->clip = (unsigned char*)malloc(w*h);
  memset(ctxcanvas->clip, 1, w*h);  /* CD_CLIPOFF */

  canvas->ctxcanvas = ctxcanvas;
  ctxcanvas->canvas = canvas;

  cdSimInitText(canvas->simulation); 
  /* nao preciso inicializar a fonte,
     pois isso sera' feito na inicializacao dos atributos default do driver */

  canvas->simulation->antialias = 1;

  cdRegisterAttribute(canvas, &red_attrib);
  cdRegisterAttribute(canvas, &green_attrib);
  cdRegisterAttribute(canvas, &blue_attrib);
  cdRegisterAttribute(canvas, &alpha_attrib);
  cdRegisterAttribute(canvas, &aa_attrib);
  cdRegisterAttribute(canvas, &rotate_attrib);
}

static void cdinittable(cdCanvas* canvas)
{
  cdSimulation* sim;

  /* initialize function table*/
  canvas->cxClip = cdclip;
  canvas->cxClipArea = cdcliparea;
  canvas->cxNewRegion = cdnewregion;
  canvas->cxIsPointInRegion = cdispointinregion;
  canvas->cxOffsetRegion = cdoffsetregion;
  canvas->cxGetRegionBox = cdgetregionbox;

  canvas->cxPutImageRectRGB = cdputimagerectrgb;
  canvas->cxPutImageRectRGBA = cdputimagerectrgba;
  canvas->cxPutImageRectMap = cdputimagerectmap;
  canvas->cxGetImageRGB = cdgetimagergb;

  canvas->cxCreateImage = cdcreateimage;
  canvas->cxGetImage = cdgetimage; 
  canvas->cxPutImageRect = cdputimagerect; 
  canvas->cxKillImage = cdkillimage;
  canvas->cxScrollArea = cdscrollarea;

  canvas->cxClear = cdclear;
  canvas->cxPixel = cdpixel;

  canvas->cxLine = cdSimLine;
  canvas->cxRect = cdSimRect;
  canvas->cxBox = cdbox;
  canvas->cxArc = cdSimArc;
  canvas->cxSector = cdSimSector;
  canvas->cxChord = cdSimChord;
  canvas->cxPoly = cdpoly;
  canvas->cxText = cdtext;

  canvas->cxFLine = cdfSimLine;
  canvas->cxFRect = cdfSimRect;
  canvas->cxFBox = cdfbox;
  canvas->cxFArc = cdfSimArc;
  canvas->cxFSector = cdfSimSector;
  canvas->cxFChord = cdfSimChord;
  canvas->cxFPoly = cdfSimPoly;

  canvas->cxKillCanvas = cdkillcanvas;

  /* use simulation */
  canvas->cxFont = cdSimFontFT;
  canvas->cxGetFontDim = cdSimGetFontDimFT;
  canvas->cxGetTextSize = cdSimGetTextSizeFT;

  sim = canvas->simulation;

  sim->SolidLine   = irgbSolidLine;
  sim->PatternLine = irgbPatternLine; 
  sim->StippleLine = irgbStippleLine; 
  sim->HatchLine   = irgbHatchLine;   
}

static cdContext cdImageRGBContext =
{
  CD_CAP_ALL & ~(CD_CAP_FLUSH | CD_CAP_PLAY | 
                 CD_CAP_LINECAP | CD_CAP_LINEJOIN | 
                 CD_CAP_PALETTE ),
  0,
  cdcreatecanvas,
  cdinittable,
  NULL,
  NULL,
};
 
cdContext* cdContextImageRGB(void)
{
  return &cdImageRGBContext;
}

static void cdflushDB(cdCtxCanvas *ctxcanvas)
{
  int old_writemode;
  cdCanvas* canvas_dbuffer = ctxcanvas->canvas_dbuffer;

  /* this is done in the canvas_dbuffer context */

  /* Flush can be affected by Origin and Clipping, but not WriteMode */

  old_writemode = cdCanvasWriteMode(canvas_dbuffer, CD_REPLACE);
  cdCanvasPutImageRectRGB(canvas_dbuffer, ctxcanvas->canvas->w, ctxcanvas->canvas->h, ctxcanvas->red, ctxcanvas->green, ctxcanvas->blue, 0, 0, ctxcanvas->canvas->w, ctxcanvas->canvas->h, 0, 0, 0, 0);
  cdCanvasWriteMode(canvas_dbuffer, old_writemode);
}

static void cdcreatecanvasDB(cdCanvas* canvas, cdCanvas* canvas_dbuffer)
{
  char rgbdata[100];
  sprintf(rgbdata, "%dx%d -r%g", canvas_dbuffer->w, canvas_dbuffer->h, canvas_dbuffer->xres);
  cdcreatecanvas(canvas, rgbdata);  /* the double buffer image will be internally allocated as the canvas RGB image itself */
  if (canvas->ctxcanvas)
    canvas->ctxcanvas->canvas_dbuffer = canvas_dbuffer;
}

static int cdactivateDB(cdCtxCanvas *ctxcanvas)
{
  cdCanvas* canvas_dbuffer = ctxcanvas->canvas_dbuffer;

  /* this is done in the canvas_dbuffer context */
  /* this will update canvas size */
  cdCanvasActivate(canvas_dbuffer);

  /* check if the size changed */
  if (canvas_dbuffer->w != ctxcanvas->canvas->w ||
      canvas_dbuffer->h != ctxcanvas->canvas->h)
  {
    cdCanvas* canvas = ctxcanvas->canvas;
    /* save the current, if the rebuild fail */
    cdCtxCanvas* old_ctxcanvas = ctxcanvas;

    /* if the image is rebuild, the canvas that uses the image must be also rebuild */

    /* rebuild the image and the canvas */
    canvas->ctxcanvas = NULL;
    cdcreatecanvasDB(canvas, canvas_dbuffer);
    if (!canvas->ctxcanvas)
    {
      canvas->ctxcanvas = old_ctxcanvas;
      return CD_ERROR;
    }

    /* remove the old image and canvas */
    cdkillcanvas(old_ctxcanvas);  /* the double buffer image is the canvas itself */

    ctxcanvas = canvas->ctxcanvas;

    /* update canvas attributes */
    cdUpdateAttributes(canvas);
  }

  return CD_OK;
}

static void cddeactivateDB(cdCtxCanvas *ctxcanvas)
{
  cdCanvas* canvas_dbuffer = ctxcanvas->canvas_dbuffer;
  /* this is done in the canvas_dbuffer context */
  cdCanvasDeactivate(canvas_dbuffer);
}

static void cdinittableDB(cdCanvas* canvas)
{
  cdinittable(canvas);

  canvas->cxActivate = cdactivateDB;
  canvas->cxDeactivate = cddeactivateDB;

  canvas->cxFlush = cdflushDB;
}

static cdContext cdDBufferRGBContext =
{
  CD_CAP_ALL & ~(CD_CAP_PLAY |  
                 CD_CAP_LINECAP | CD_CAP_LINEJOIN | 
                 CD_CAP_PALETTE ),
  0,
  cdcreatecanvasDB,  
  cdinittableDB,
  NULL,             
  NULL, 
};

cdContext* cdContextDBufferRGB(void)
{
  return &cdDBufferRGBContext;
}
