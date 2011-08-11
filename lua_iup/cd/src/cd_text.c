/** \file
 * \brief External API - Text
 *
 * See Copyright Notice in cd.h
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <memory.h>
#include <math.h>

#include "cd.h"
#include "cd_private.h"


void cdCanvasText(cdCanvas* canvas, int x, int y, const char *s)
{
  int num_line;

  assert(canvas);
  assert(s);
  if (!_cdCheckCanvas(canvas)) return;

  if (s[0] == 0)
    return;

  if (canvas->use_origin)
  {
    x += canvas->origin.x;
    y += canvas->origin.y;
  }

  num_line = cdStrLineCount(s);
  if (num_line == 1)
  {
    if (canvas->invert_yaxis)
      y = _cdInvertYAxis(canvas, y);

    canvas->cxText(canvas->ctxcanvas, x, y, s, strlen(s));
  }
  else
  {
    int yr, i, line_height, len;
    const char *p, *q;
    double cos_theta = 0, sin_theta = 0;

    canvas->cxGetFontDim(canvas->ctxcanvas, NULL, &line_height, NULL, NULL);

    if (canvas->text_orientation)
    {
      int align = canvas->text_alignment;
      cos_theta = cos(canvas->text_orientation*CD_DEG2RAD);
      sin_theta = sin(canvas->text_orientation*CD_DEG2RAD);

      /* position vertically at the first line */
      if (align == CD_NORTH || align == CD_NORTH_EAST || align == CD_NORTH_WEST ||     /* it is relative to the full text */
          align == CD_BASE_LEFT || align == CD_BASE_CENTER || align == CD_BASE_RIGHT)  /* it is relative to the first line already */
      {
        /* Already at position */
      }
      else if (align == CD_SOUTH || align == CD_SOUTH_EAST || align == CD_SOUTH_WEST)  /* it is relative to the full text */
      {
        cdMovePoint(&x, &y, 0, (num_line-1)*line_height, sin_theta, cos_theta);
      }
      else  /* CD_CENTER || CD_EAST || CD_WEST */                                      /* it is relative to the full text */
        cdMovePoint(&x, &y, 0, (num_line-1)*line_height/2.0, sin_theta, cos_theta);
    }
    else
    {
      int align = canvas->text_alignment;

      /* position vertically at the first line */
      if (align == CD_NORTH || align == CD_NORTH_EAST || align == CD_NORTH_WEST ||     /* it is relative to the full text */
          align == CD_BASE_LEFT || align == CD_BASE_CENTER || align == CD_BASE_RIGHT)  /* it is relative to the first line already */
      {
        /* Already at position */
      }
      else if (align == CD_SOUTH || align == CD_SOUTH_EAST || align == CD_SOUTH_WEST)  /* it is relative to the full text */
      {
        y += (num_line-1)*line_height;
      }
      else  /* CD_CENTER || CD_EAST || CD_WEST */                                      /* it is relative to the full text */
        y += ((num_line-1)*line_height)/2;
    }

    p = s;
    for(i = 0; i < num_line; i++)
    {
      q = strchr(p, '\n');
      if (q) len = (int)(q-p);  /* Cut the string to contain only one line */
      else len = strlen(p);

      /* Draw the line */
      if (canvas->invert_yaxis)
        yr = _cdInvertYAxis(canvas, y);
      else
        yr = y;
      canvas->cxText(canvas->ctxcanvas, x, yr, p, len);

      /* Advance the string */
      if (q) p = q + 1;

      /* Advance a line */
      if (canvas->text_orientation)
        cdMovePoint(&x, &y, 0, -line_height, sin_theta, cos_theta);
      else
        y -= line_height;
    }
  }
}

void cdfCanvasText(cdCanvas* canvas, double x, double y, const char *s)
{
  int num_line;

  assert(canvas);
  assert(s);
  if (!_cdCheckCanvas(canvas)) return;

  if (s[0] == 0)
    return;

  if (canvas->use_origin)
  {
    x += canvas->forigin.x;
    y += canvas->forigin.y;
  }

  num_line = cdStrLineCount(s);
  if (num_line == 1)
  {
    if (canvas->invert_yaxis)
      y = _cdInvertYAxis(canvas, y);

    if (canvas->cxFText)
      canvas->cxFText(canvas->ctxcanvas, x, y, s, strlen(s));
    else
      canvas->cxText(canvas->ctxcanvas, _cdRound(x), _cdRound(y), s, strlen(s));
  }
  else
  {
    int i, line_height, len;
    const char *p, *q;
    double yr, cos_theta = 0, sin_theta = 0;

    canvas->cxGetFontDim(canvas->ctxcanvas, NULL, &line_height, NULL, NULL);

    if (canvas->text_orientation)
    {
      int align = canvas->text_alignment;
      cos_theta = cos(canvas->text_orientation*CD_DEG2RAD);
      sin_theta = sin(canvas->text_orientation*CD_DEG2RAD);

      /* position vertically at the first line */
      if (align == CD_NORTH || align == CD_NORTH_EAST || align == CD_NORTH_WEST ||     /* it is relative to the full text */
          align == CD_BASE_LEFT || align == CD_BASE_CENTER || align == CD_BASE_RIGHT)  /* it is relative to the first line already */
      {
        /* Already at position */
      }
      else if (align == CD_SOUTH || align == CD_SOUTH_EAST || align == CD_SOUTH_WEST)  /* it is relative to the full text */
      {
        cdfMovePoint(&x, &y, 0, (num_line-1)*line_height, sin_theta, cos_theta);
      }
      else  /* CD_CENTER || CD_EAST || CD_WEST */                                      /* it is relative to the full text */
        cdfMovePoint(&x, &y, 0, (num_line-1)*line_height/2.0, sin_theta, cos_theta);
    }
    else
    {
      int align = canvas->text_alignment;

      /* position vertically at the first line */
      if (align == CD_NORTH || align == CD_NORTH_EAST || align == CD_NORTH_WEST ||     /* it is relative to the full text */
          align == CD_BASE_LEFT || align == CD_BASE_CENTER || align == CD_BASE_RIGHT)  /* it is relative to the first line already */
      {
        /* Already at position */
      }
      else if (align == CD_SOUTH || align == CD_SOUTH_EAST || align == CD_SOUTH_WEST)  /* it is relative to the full text */
      {
        y += (num_line-1)*line_height;
      }
      else  /* CD_CENTER || CD_EAST || CD_WEST */                                      /* it is relative to the full text */
        y += ((num_line-1)*line_height)/2.0;
    }

    p = s;
    for(i = 0; i < num_line; i++)
    {
      q = strchr(p, '\n');
      if (q) len = (int)(q-p);  /* Cut the string to contain only one line */
      else len = strlen(p);

      /* Draw the line */
      if (canvas->invert_yaxis)
        yr = _cdInvertYAxis(canvas, y);
      else
        yr = y;
      if (canvas->cxFText)
        canvas->cxFText(canvas->ctxcanvas, x, yr, p, len);
      else
        canvas->cxText(canvas->ctxcanvas, _cdRound(x), _cdRound(yr), p, len);

      /* Advance the string */
      if (q) p = q + 1;

      /* Advance a line */
      if (canvas->text_orientation)
        cdfMovePoint(&x, &y, 0, -line_height, sin_theta, cos_theta);
      else
        y -= line_height;
    }
  }
}

int cdGetFontSizePixels(cdCanvas* canvas, int size)
{
  if (size < 0)
    size = -size;
  else
  {
    double size_mm = (double)size/CD_MM2PT;
    size = cdRound(size_mm*canvas->xres);
  }

  if (size == 0)
    size = 1;

  return size;
}

int cdGetFontSizePoints(cdCanvas* canvas, int size)
{
  if (size < 0)
  {
    double size_mm = ((double)-size)/canvas->xres;
    size = cdRound(size_mm * CD_MM2PT);
  }

  if (size == 0)
    size = 1;

  return size;
}

int cdCanvasFont(cdCanvas* canvas, const char* type_face, int style, int size)
{
  assert(canvas);
  assert(style>=-1 && style<=CD_STRIKEOUT);
  if (!_cdCheckCanvas(canvas)) return CD_ERROR;

  if (!type_face || type_face[0]==0)
    type_face = canvas->font_type_face;
  if (style==-1)
    style = canvas->font_style;
  if (size==0)
    size = canvas->font_size;

  if (strcmp(type_face, canvas->font_type_face)==0 && 
      style == canvas->font_style && 
      size == canvas->font_size)
    return 1;

  if (canvas->cxFont(canvas->ctxcanvas, type_face, style, size))
  {
    strcpy(canvas->font_type_face, type_face);
    canvas->font_style = style;
    canvas->font_size = size;
    canvas->native_font[0] = 0;
    return 1;
  }

  return 0;
}

void cdCanvasGetFont(cdCanvas* canvas, char *type_face, int *style, int *size)
{
  assert(canvas);
  if (!_cdCheckCanvas(canvas)) return;

  if (type_face) strcpy(type_face, canvas->font_type_face);
  if (style) *style = canvas->font_style;
  if (size) *size = canvas->font_size;
}

char* cdCanvasNativeFont(cdCanvas* canvas, const char* font)
{
  static char native_font[1024] = "";

  assert(canvas);
  if (!_cdCheckCanvas(canvas)) return NULL;

  strcpy(native_font, canvas->native_font);

  if (font == (char*)CD_QUERY)
  {
    char style[200] = " ";
    if (canvas->font_style&CD_BOLD)
      strcat(style, "Bold ");
    if (canvas->font_style&CD_ITALIC)
      strcat(style, "Italic ");
    if (canvas->font_style&CD_UNDERLINE)
      strcat(style, "Underline ");
    if (canvas->font_style&CD_STRIKEOUT)
      strcat(style, "Strikeout ");

    sprintf(native_font, "%s,%s %d", canvas->font_type_face, style, canvas->font_size);
    return native_font;
  }

  if (!font || font[0] == 0)
    return native_font;

  if (canvas->cxNativeFont)
  {
    if (canvas->cxNativeFont(canvas->ctxcanvas, font))
      strcpy(canvas->native_font, font);
  }
  else
  {
    char type_face[1024];
    int size = 12, style = CD_PLAIN;

    if (!cdParseIupWinFont(font, type_face, &style, &size))
    {
      if (!cdParseXWinFont(font, type_face, &style, &size))
      {
        if (!cdParsePangoFont(font, type_face, &style, &size))
          return native_font;
      }
    }

    if (cdCanvasFont(canvas, type_face, style, size))
      strcpy(canvas->native_font, font);
  }

  return native_font;
}

int cdCanvasTextAlignment(cdCanvas* canvas, int alignment)
{
  int text_alignment;

  assert(canvas);
  assert(alignment==CD_QUERY || (alignment>=CD_NORTH && alignment<=CD_BASE_RIGHT));
  if (!_cdCheckCanvas(canvas)) return CD_ERROR;
  if (alignment<CD_QUERY || alignment>CD_BASE_RIGHT);

  text_alignment = canvas->text_alignment;

  if (alignment == CD_QUERY || alignment == text_alignment)
    return text_alignment;

  if (canvas->cxTextAlignment)
    canvas->text_alignment = canvas->cxTextAlignment(canvas->ctxcanvas, alignment);
  else
    canvas->text_alignment = alignment;

  return text_alignment;
}

double cdCanvasTextOrientation(cdCanvas* canvas, double angle)
{
  double text_orientation;

  assert(canvas);
  if (!_cdCheckCanvas(canvas)) return CD_ERROR;

  text_orientation = canvas->text_orientation;

  if (angle == CD_QUERY || angle == text_orientation)
    return text_orientation;

  if (canvas->cxTextOrientation)
    canvas->text_orientation = canvas->cxTextOrientation(canvas->ctxcanvas, angle);
  else
    canvas->text_orientation = angle;

  return text_orientation;
}

void cdCanvasGetFontDim(cdCanvas* canvas, int *max_width, int *height, int *ascent, int *descent)
{
  assert(canvas);
  if (!_cdCheckCanvas(canvas)) return;
  canvas->cxGetFontDim(canvas->ctxcanvas, max_width, height, ascent, descent);
}

void cdCanvasGetTextSize(cdCanvas* canvas, const char *s, int *width, int *height)
{
  int num_line;

  assert(canvas);
  assert(s);
  if (!_cdCheckCanvas(canvas)) return;

  num_line = cdStrLineCount(s);
  if (num_line == 1)
    canvas->cxGetTextSize(canvas->ctxcanvas, s, strlen(s), width, height);
  else
  {
    int i, line_height, max_w = 0, w, len;
    const char *p, *q;

    p = s;

    canvas->cxGetFontDim(canvas->ctxcanvas, NULL, &line_height, NULL, NULL);

    for(i = 0; i < num_line; i++)
    {
      q = strchr(p, '\n');
      if (q) len = (int)(q-p);  /* Cut the string to contain only one line */
      else len = strlen(p);

      /* Calculate line width */
      canvas->cxGetTextSize(canvas->ctxcanvas, p, len, &w, NULL);
      if (w > max_w) max_w = w;

      /* Advance the string */
      if (q) p = q + 1; /* skip line break */
    }

    if (width) *width = max_w;
    if (height) *height = num_line*line_height;
  }
}

void cdTextTranslatePoint(cdCanvas* canvas, int x, int y, int w, int h, int baseline, int *rx, int *ry)
{
  /* move to left */
  switch (canvas->text_alignment)
  {
  case CD_BASE_RIGHT:
  case CD_NORTH_EAST:
  case CD_EAST:
  case CD_SOUTH_EAST:
    *rx = x - w;    
    break;
  case CD_BASE_CENTER:
  case CD_CENTER:
  case CD_NORTH:
  case CD_SOUTH:
    *rx = x - w/2;  
    break;
  case CD_BASE_LEFT:
  case CD_NORTH_WEST:
  case CD_WEST:
  case CD_SOUTH_WEST:
    *rx = x;         
    break;
  }

  /* move to bottom */
  switch (canvas->text_alignment)
  {
  case CD_BASE_LEFT:
  case CD_BASE_CENTER:
  case CD_BASE_RIGHT:
    if (canvas->invert_yaxis)
      *ry = y + baseline;
    else
      *ry = y - baseline;
    break;
  case CD_SOUTH_EAST:
  case CD_SOUTH_WEST:
  case CD_SOUTH:
    *ry = y;
    break;
  case CD_NORTH_EAST:
  case CD_NORTH:
  case CD_NORTH_WEST:
    if (canvas->invert_yaxis)
      *ry = y + h;
    else
      *ry = y - h;
    break;
  case CD_CENTER:
  case CD_EAST:
  case CD_WEST:
    if (canvas->invert_yaxis)
      *ry = y + h/2;
    else
      *ry = y - h/2;
    break;
  }
}

void cdCanvasGetTextBounds(cdCanvas* canvas, int x, int y, const char *s, int *rect)
{
  int w, h, ascent, line_height, baseline;
  int xmin, xmax, ymin, ymax;
  int old_invert_yaxis, num_lin;

  assert(canvas);
  assert(s);
  if (!_cdCheckCanvas(canvas)) return;

  if (s[0] == 0)
    return;
  
  cdCanvasGetTextSize(canvas, s, &w, &h);
  cdCanvasGetFontDim(canvas, NULL, &line_height, &ascent, NULL);
  baseline = line_height - ascent;
  num_lin = h/line_height;
  if (num_lin > 1)
    baseline += (num_lin-1)*line_height;

  /* from here we are always upwards */
  old_invert_yaxis = canvas->invert_yaxis;
  canvas->invert_yaxis = 0;

  /* move to bottom-left */
  cdTextTranslatePoint(canvas, x, y, w, h, baseline, &xmin, &ymin);

  xmax = xmin + w-1;
  ymax = ymin + h-1;

  if (canvas->text_orientation)
  {
    double cos_theta = cos(canvas->text_orientation*CD_DEG2RAD);
    double sin_theta = sin(canvas->text_orientation*CD_DEG2RAD);

    cdRotatePoint(canvas, xmin, ymin, x, y, &rect[0], &rect[1], sin_theta, cos_theta);
    cdRotatePoint(canvas, xmax, ymin, x, y, &rect[2], &rect[3], sin_theta, cos_theta);
    cdRotatePoint(canvas, xmax, ymax, x, y, &rect[4], &rect[5], sin_theta, cos_theta);
    cdRotatePoint(canvas, xmin, ymax, x, y, &rect[6], &rect[7], sin_theta, cos_theta);
  }
  else
  {
    rect[0] = xmin; rect[1] = ymin;
    rect[2] = xmax; rect[3] = ymin;
    rect[4] = xmax; rect[5] = ymax;
    rect[6] = xmin; rect[7] = ymax;
  }

  canvas->invert_yaxis = old_invert_yaxis;
}

void cdCanvasGetTextBox(cdCanvas* canvas, int x, int y, const char *s, int *xmin, int *xmax, int *ymin, int *ymax)
{
  int rect[8];
  int _xmin, _xmax, _ymin, _ymax;

  cdCanvasGetTextBounds(canvas, x, y, s, rect);

  _xmin = rect[0];
  _ymin = rect[1];
  _xmax = rect[0];
  _ymax = rect[1];

  if(rect[2] < _xmin) _xmin = rect[2];
  if(rect[4] < _xmin) _xmin = rect[4];
  if(rect[6] < _xmin) _xmin = rect[6];

  if(rect[3] < _ymin) _ymin = rect[3];
  if(rect[5] < _ymin) _ymin = rect[5];
  if(rect[7] < _ymin) _ymin = rect[7];

  if(rect[2] > _xmax) _xmax = rect[2];
  if(rect[4] > _xmax) _xmax = rect[4];
  if(rect[6] > _xmax) _xmax = rect[6];

  if(rect[3] > _ymax) _ymax = rect[3];
  if(rect[5] > _ymax) _ymax = rect[5];
  if(rect[7] > _ymax) _ymax = rect[7];

  if (xmin) *xmin = _xmin;
  if (xmax) *xmax = _xmax;
  if (ymin) *ymin = _ymin;
  if (ymax) *ymax = _ymax;
}

/**************************************************************/
/* Native Font Format, compatible with Pango Font Description */
/**************************************************************/

/*
The string contains the font name, the style and the size. 
Style can be a free combination of some names separated by spaces.
Font name can be a list of font family names separated by comma.
*/

#define isspace(_x) (_x == ' ')

static int cd_find_style_name(const char *name, int len, int *style)
{
#define CD_STYLE_NUM_NAMES 21
  static struct { const char* name; int style; } cd_style_names[CD_STYLE_NUM_NAMES] = {
    {"Normal",         0},
    {"Oblique",        CD_ITALIC},
    {"Italic",         CD_ITALIC},
    {"Small-Caps",     0},
    {"Ultra-Light",    0},
    {"Light",          0},
    {"Medium",         0},
    {"Semi-Bold",      CD_BOLD},
    {"Bold",           CD_BOLD},
    {"Ultra-Bold",     CD_BOLD},
    {"Heavy",          0},
    {"Ultra-Condensed",0},
    {"Extra-Condensed",0},
    {"Condensed",      0},
    {"Semi-Condensed", 0},
    {"Semi-Expanded",  0},
    {"Expanded",       0},
    {"Extra-Expanded", 0},
    {"Ultra-Expanded", 0},
    {"Underline", CD_UNDERLINE},
    {"Strikeout", CD_STRIKEOUT}
  };

  int i;
  for (i = 0; i < CD_STYLE_NUM_NAMES; i++)
  {
    if (strncmp(cd_style_names[i].name, name, len)==0)
    {
      *style = cd_style_names[i].style;
      return 1;
    }
  }

  return 0;
}

static const char * cd_getword(const char *str, const char *last, int *wordlen)
{
  const char *result;
  
  while (last > str && isspace(*(last - 1)))
    last--;

  result = last;
  while (result > str && !isspace (*(result - 1)))
    result--;

  *wordlen = last - result;
  
  return result;
}

int cdParsePangoFont(const char *nativefont, char *type_face, int *style, int *size)
{
  const char *p, *last;
  int len, wordlen;

  len = (int)strlen(nativefont);
  last = nativefont + len;
  p = cd_getword(nativefont, last, &wordlen);

  /* Look for a size at the end of the string */
  if (wordlen != 0)
  {
    int new_size = atoi(p);
    if (new_size != 0)
    {
      *size = new_size;
      last = p;
    }
  }

  /* Now parse style words */
  p = cd_getword(nativefont, last, &wordlen);
  while (wordlen != 0)
  {
    int new_style = 0;

    if (!cd_find_style_name(p, wordlen, &new_style))
      break;
    else
    {
      *style |= new_style;

      last = p;
      p = cd_getword(nativefont, last, &wordlen);
    }
  }

  /* Remainder is font family list. */

  /* Trim off trailing white space */
  while (last > nativefont && isspace(*(last - 1)))
    last--;

  /* Trim off trailing commas */
  if (last > nativefont && *(last - 1) == ',')
    last--;

  /* Again, trim off trailing white space */
  while (last > nativefont && isspace(*(last - 1)))
    last--;

  /* Trim off leading white space */
  while (last > nativefont && isspace(*nativefont))
    nativefont++;

  if (nativefont != last)
  {
    len = (last - nativefont);
    strncpy(type_face, nativefont, len);
    type_face[len] = 0;
    return 1;
  }
  else
    return 0;
}

int cdParseIupWinFont(const char *nativefont, char *type_face, int *style, int *size)
{
  int c;

  if (strstr(nativefont, ":") == NULL)
    return 0;

  c = strcspn(nativefont, ":");      /* extract type_face */
  if (c == 0) return 0;
  strncpy(type_face, nativefont, c);
  type_face[c]='\0';
  nativefont += c+1;

  if(nativefont[0] == ':')  /* check for attributes */
    nativefont++;
  else
  {
    *style = 0;
    while(strlen(nativefont)) /* extract style (bold/italic etc) */
    {
      char style_str[20];

      c = strcspn(nativefont, ":,");
      if (c == 0)
        break;

      strncpy(style_str, nativefont, c);
      style_str[c] = '\0';

      if(!strcmp(style_str, "BOLD"))
        *style |= CD_BOLD;
      else if(!strcmp(style_str,"ITALIC"))
        *style |= CD_ITALIC;
      else if(!strcmp(style_str,"UNDERLINE"))
        *style |= CD_UNDERLINE;
      else if(!strcmp(style_str,"STRIKEOUT"))
        *style |= CD_STRIKEOUT;

      nativefont += c;

      if(nativefont[0] == ':')  /* end attribute list */
      {
        nativefont++;
        break;
      }

      nativefont++;   /* skip separator */
    }
  }

  /* extract size in points */
  if (sscanf(nativefont, "%d", size) != 1)
    return 0;

  if (*size == 0)
    return 0;

  return 1;
}

int cdParseXWinFont(const char *nativefont, char *type_face, int *style, int *size)
{
  char style1[10], style2[10];
  char* token;
  char font[1024];

  if (nativefont[0] != '-')
    return 0;

  strcpy(font, nativefont+1);  /* skip first '-' */

  *style = 0;

  /* fndry */
  token = strtok(font, "-");
  if (!token) return 0;

  /* fmly */
  token = strtok(NULL, "-");
  if (!token) return 0;
  strcpy(type_face, token);

  /* wght */
  token = strtok(NULL, "-");
  if (!token) return 0;
  strcpy(style1, token);
  if (strstr("bold", style1))
    *style |= CD_BOLD;

  /* slant */
  token = strtok(NULL, "-");
  if (!token) return 0;
  strcpy(style2, token);
  if (*style2 == 'i' || *style2 == 'o')
    *style |= CD_ITALIC;

  /* sWdth */
  token = strtok(NULL, "-");
  if (!token) return 0;
  /* adstyl */
  token = strtok(NULL, "-");
  if (!token) return 0;

  /* pxlsz */
  token = strtok(NULL, "-");
  if (!token) return 0;
  *size = -atoi(token); /* size in pixels */

  if (*size < 0)
    return 1;

  /* ptSz */
  token = strtok(NULL, "-");
  if (!token) return 0;
  *size = atoi(token)/10; /* size in deci-points */

  if (*size > 0)
    return 1;

  return 0;
}
