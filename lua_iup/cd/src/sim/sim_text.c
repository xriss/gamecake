/** \file
 * \brief Text and Font Functions of the Simulation Base Driver
 *
 * See Copyright Notice in cd.h
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <memory.h>
#include <ctype.h>
         
#include "cd.h"
#include "cd_private.h"
#include "cd_truetype.h"
#include "sim.h"
#include FT_GLYPH_H


static int font_name_match(const char* map, const char* name)
{
  while (*map != '=')
  {
    if (tolower(*map) != tolower(*name))
      return 0;

    map++;
    name++;
  }

  return 1;
}

static void cdSimAddFontMap(cdSimulation* simulation, const char* map)
{
  int i;

  if (!strstr(map, "="))
    return;

  for (i = 0; i < simulation->font_map_n; i++)
  {
    if (font_name_match(simulation->font_map[i], map))
    {
      /* replace */
      simulation->font_map[i] = map;
      return;
    }
  }

  /* not found add */
  simulation->font_map[i] = map;
  simulation->font_map_n++;
}

static void set_addfontmap(cdCtxCanvas* ctxcanvas, char* data)
{
  if (data)
  {
    cdCanvas* canvas = ((cdCtxCanvasBase*)ctxcanvas)->canvas;
    cdSimAddFontMap(canvas->simulation, data);
  }
}

static cdAttribute addfontmap_attrib =
{
  "ADDFONTMAP",
  set_addfontmap,
  NULL
}; 

static char* get_version_attrib(cdCtxCanvas* ctxcanvas)
{
  static char version[50];
  FT_Int major, minor, patch;
  cdCanvas* canvas = ((cdCtxCanvasBase*)ctxcanvas)->canvas;
  FT_Library_Version(canvas->simulation->tt_text->library, &major, &minor, &patch);
  sprintf(version, "FreeType %d.%d.%d", major, minor, patch);
  return version;
}

static cdAttribute version_attrib =
{
  "FREETYPEVERSION",
  NULL,
  get_version_attrib
}; 

void cdSimInitText(cdSimulation* simulation)
{
  if (!simulation->tt_text)
    simulation->tt_text = cdTT_create();

  cdRegisterAttribute(simulation->canvas, &addfontmap_attrib);
  cdRegisterAttribute(simulation->canvas, &version_attrib);
}

static const char* find_font_filename(cdSimulation* simulation, const char* name)
{
  int i;
  for (i = 0; i < simulation->font_map_n; i++)
  {
    if (font_name_match(simulation->font_map[i], name))
      return strstr(simulation->font_map[i], "=")+1;
  }
  return NULL;
}

int cdSimFontFT(cdCtxCanvas* ctxcanvas, const char *type_face, int style, int size)
{
  cdCanvas* canvas = ((cdCtxCanvasBase*)ctxcanvas)->canvas;

  /* check for the pre-defined names */
  if (cdStrEqualNoCase(type_face, "System"))
    type_face = "cour";
  else if (cdStrEqualNoCase(type_face, "Courier"))
    type_face = "cour";
  else if (cdStrEqualNoCase(type_face, "Times"))
    type_face = "times";
  else if (cdStrEqualNoCase(type_face, "Helvetica"))
    type_face = "arial";
  else
  {
    /* use the font map */
    const char* filename = find_font_filename(canvas->simulation, type_face);
    if (filename)
      return cdTT_load(canvas->simulation->tt_text, filename, cdGetFontSizePoints(canvas, size), canvas->xres, canvas->yres);
    else 
    {
      /* try the type_face name without change */
      if (cdTT_load(canvas->simulation->tt_text, type_face, cdGetFontSizePoints(canvas, size), canvas->xres, canvas->yres))
        return 1;
    }
  }

  {
    static char * cd_ttf_font_style[4] = {
      "",
      "bd",
      "i",
      "bi"};
    char font[10240]; /* can have a path */
    sprintf(font, "%s%s", type_face, cd_ttf_font_style[style&3]);
    return cdTT_load(canvas->simulation->tt_text, font, cdGetFontSizePoints(canvas, size), canvas->xres, canvas->yres);
  }
}
            
void cdSimGetFontDimFT(cdCtxCanvas* ctxcanvas, int *max_width, int *height, int *ascent, int *descent)
{
  cdCanvas* canvas = ((cdCtxCanvasBase*)ctxcanvas)->canvas;
  cdSimulation* simulation = canvas->simulation;

  if (!simulation->tt_text->face)
    return;

  if(ascent) *ascent = simulation->tt_text->ascent;
  if(descent) *descent= simulation->tt_text->descent;
  if(max_width) *max_width= simulation->tt_text->max_width;
  if(height) *height= simulation->tt_text->max_height;
}

void cdSimGetTextSizeFT(cdCtxCanvas* ctxcanvas, const char *s, int len, int *width, int *height)
{
  cdCanvas* canvas = ((cdCtxCanvasBase*)ctxcanvas)->canvas;
  cdSimulation* simulation = canvas->simulation;
  int i = 0, w = 0;
  FT_Face       face;
  FT_GlyphSlot  slot;
  FT_Error      error;

  if (!simulation->tt_text->face)
    return;

  face = simulation->tt_text->face;
  slot = face->glyph;

  /* set transformation */
  FT_Set_Transform( face, NULL, NULL );

  while(i < len)
  {
    /* load glyph image into the slot (erase previous one) */
    error = FT_Load_Char( face, (unsigned char)s[i], FT_LOAD_DEFAULT );
    if (error) {i++; continue;}  /* ignore errors */

    w += slot->advance.x; 

    i++;
  }

  if (height) *height = simulation->tt_text->max_height;
  if (width)  *width  = w >> 6;
}

static void simDrawTextBitmap(cdSimulation* simulation, FT_Bitmap* bitmap, int x, int y)
{
  unsigned char *red, *green, *blue, *alpha, *bitmap_data;
  int width = bitmap->width;
  int height = bitmap->rows;
  int size = width*height;
  int rgba_data_size = size*4;
  int old_use_matrix = simulation->canvas->use_matrix;

  /* avoid spaces */
  if (width == 0 || height == 0)
    return;

  if (!simulation->tt_text->rgba_data)
    simulation->tt_text->rgba_data = malloc(rgba_data_size);
  else if (rgba_data_size > simulation->tt_text->rgba_data_size)
  {
    simulation->tt_text->rgba_data = realloc(simulation->tt_text->rgba_data, rgba_data_size);
    simulation->tt_text->rgba_data_size = rgba_data_size;
  }

  /* disable image transformation */
  simulation->canvas->use_matrix = 0;

  /* this is the char bitmap, contains an alpha map of the char 
     to be combined with the foreground color */
  bitmap_data = bitmap->buffer + (height-1)*width;  /* bitmap is top down. */

  /* this is the image used to draw the char with the foreground color */ 
  red   = simulation->tt_text->rgba_data;
  green = red   + size;
  blue  = green + size;
  alpha = blue  + size;

  if (!simulation->canvas->cxPutImageRectRGBA && !simulation->canvas->cxGetImageRGB)
  {
    int i, j;
    unsigned char bg_red, bg_green, bg_blue, 
                  fg_red, fg_green, fg_blue, fg_alpha, calpha;
    long int c;

    /* must manually combine using only the background color, ignore canvas contents */

    c = simulation->canvas->background;
    bg_red   = cdRed(c);
    bg_green = cdGreen(c);
    bg_blue  = cdBlue(c);
    c = simulation->canvas->foreground;
    fg_red   = cdRed(c);
    fg_green = cdGreen(c);
    fg_blue  = cdBlue(c);
    fg_alpha = cdAlpha(c);

    for (i = 0; i < height; i++)
    {
      for (j = 0; j < width; j++)
      {
        if (simulation->antialias)
        {
          if (fg_alpha == 255)
            calpha = bitmap_data[j];
          else
            calpha = (fg_alpha*bitmap_data[j])/255;
        }
        else
        {
          if (bitmap_data[j] > 128)  /* behave as 255 */
            calpha = fg_alpha;
          else
            calpha = 0;
        }

        *red++ = CD_ALPHA_BLEND(fg_red, bg_red, calpha);
        *green++ = CD_ALPHA_BLEND(fg_green, bg_green, calpha);
        *blue++ = CD_ALPHA_BLEND(fg_blue, bg_blue, calpha);
      }

      bitmap_data -= width;
    }

    /* reset pointers */
    red   = simulation->tt_text->rgba_data;
    green = red   + size;
    blue  = green + size;

    /* draw the char */
    simulation->canvas->cxPutImageRectRGB(simulation->canvas->ctxcanvas, width,height,red,green,blue,x,y,width,height,0,width-1,0,height-1);
  }
  else
  {
    int i, j;
    long int fg = simulation->canvas->foreground;
    unsigned char fg_alpha = cdAlpha(fg);
    memset(red,   cdRed(fg), size);
    memset(green, cdGreen(fg), size);
    memset(blue,  cdBlue(fg), size);

    /* alpha is the bitmap_data itself 
       if the foreground color does not contains alpha.
       Also must invert since it is top-down. */

    for (i = 0; i < height; i++)
    {
      if (simulation->antialias)
      {
        if (fg_alpha == 255)
        {
          memcpy(alpha,  bitmap_data, width);
          alpha += width;
        }
        else
        {
          for (j = 0; j < width; j++)
          {
            *alpha++ = (fg_alpha*bitmap_data[j])/255;
          }
        }
      }
      else
      {
        for (j = 0; j < width; j++)
        {
          if (bitmap_data[j] > 128)  /* behave as 255 */
            *alpha++ = fg_alpha;
          else
            *alpha++ = 0;
        }
      }

      bitmap_data -= width;
    }

    /* reset alpha pointer */
    alpha = blue + size;

    /* draw the char */
    simulation->canvas->cxPutImageRectRGBA(simulation->canvas->ctxcanvas, width,height,red,green,blue,alpha,x,y,width,height,0,width-1,0,height-1);
  }

  simulation->canvas->use_matrix = old_use_matrix;
}

void simGetPenPos(cdCanvas* canvas, int x, int y, const char* s, int len, FT_Matrix *matrix, FT_Vector *pen)
{
  int ox = x, oy = y;
  int old_invert_yaxis = canvas->invert_yaxis;
  int w, h, ascent, height, baseline;

  cdSimGetTextSizeFT(canvas->ctxcanvas, s, len, &w, &h);
  cdSimGetFontDimFT(canvas->ctxcanvas, NULL, &height, &ascent, NULL);
  baseline = height - ascent;

  /* in this case we are always upwards */

  /* move to bottom left */
  canvas->invert_yaxis = 0;
  cdTextTranslatePoint(canvas, x, y, w, h, baseline, &x, &y);
  canvas->invert_yaxis = old_invert_yaxis;

  /* move to the base line */
  y += baseline;

  /* set up matrix */
  matrix->xx = (FT_Fixed)0x10000L;
  matrix->xy = (FT_Fixed)0;
  matrix->yx = (FT_Fixed)0;
  matrix->yy = (FT_Fixed)0x10000L;

  if (canvas->text_orientation)
  {
    FT_Matrix text_matrix;
    double cos_theta = cos(canvas->text_orientation*CD_DEG2RAD);
    double sin_theta = sin(canvas->text_orientation*CD_DEG2RAD);

    /* manually rotate the initial point */
    canvas->invert_yaxis = 0;
    cdRotatePoint(canvas, x, y, ox, oy, &x, &y, sin_theta, cos_theta);
    canvas->invert_yaxis = old_invert_yaxis;

    text_matrix.xx = (FT_Fixed)( cos_theta*0x10000L);
    text_matrix.xy = (FT_Fixed)(-sin_theta*0x10000L);
    text_matrix.yx = (FT_Fixed)( sin_theta*0x10000L);
    text_matrix.yy = (FT_Fixed)( cos_theta*0x10000L);

    FT_Matrix_Multiply(&text_matrix, matrix);
  }

  if (canvas->use_matrix)
  {
    FT_Matrix trans_matrix;
    trans_matrix.xx = (FT_Fixed)(canvas->matrix[0]*0x10000L);
    trans_matrix.yx = (FT_Fixed)(canvas->matrix[1]*0x10000L);
    trans_matrix.xy = (FT_Fixed)(canvas->matrix[2]*0x10000L);
    trans_matrix.yy = (FT_Fixed)(canvas->matrix[3]*0x10000L);

    FT_Matrix_Multiply(&trans_matrix, matrix);

    /* manually transform the initial point */
    cdMatrixTransformPoint(canvas->matrix, x, y, &x, &y);
  }

  /* the pen position in 26.6 scale */
  pen->x = x * 64;
  pen->y = y * 64;

}

void cdSimTextFT(cdCtxCanvas* ctxcanvas, int x, int y, const char* s, int len)
{
  cdCanvas* canvas = ((cdCtxCanvasBase*)ctxcanvas)->canvas;
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

  /* the pen position is in cartesian space coordinates */
  if (simulation->canvas->invert_yaxis)
    y = _cdInvertYAxis(canvas, y);   /* y is already inverted, invert back to cartesian space */

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

    if (canvas->invert_yaxis)
      y = _cdInvertYAxis(canvas, y);

    /* now, draw to our target surface (convert position) */
    simDrawTextBitmap(simulation, &slot->bitmap, x, y);

    /* increment pen position */
    pen.x += slot->advance.x;
    pen.y += slot->advance.y;

    i++;
  }
}
