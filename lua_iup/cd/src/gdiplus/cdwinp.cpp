/** \file
 * \brief Windows GDI+ Base Driver
 *
 * See Copyright Notice in cd.h
 */

#include "cdwinp.h"
#include "cdgdiplus.h"
#include "wd.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

static void cdstipple(cdCtxCanvas* ctxcanvas, int w, int h, const unsigned char *index);

void cdwpShowStatus(const char* title, Status status)
{
  char* status_str = "";
  switch(status)
  {
  case Ok: status_str = "Ok"; break;
  case GenericError: status_str = "GenericError"; break;
  case InvalidParameter: status_str = "InvalidParameter"; break;
  case OutOfMemory: status_str = "OutOfMemory"; break;
  case ObjectBusy: status_str = "ObjectBusy"; break;
  case InsufficientBuffer: status_str = "InsufficientBuffer"; break;
  case NotImplemented: status_str = "NotImplemented"; break;
  case Win32Error: status_str = "Win32Error"; break;
  case WrongState: status_str = "WrongState"; break;
  case Aborted: status_str = "Aborted"; break;
  case FileNotFound: status_str = "FileNotFound"; break;
  case ValueOverflow: status_str = "ValueOverflow"; break;
  case AccessDenied: status_str = "AccessDenied"; break;
  case UnknownImageFormat: status_str = "UnknownImageFormat"; break;
  case FontFamilyNotFound: status_str = "FontFamilyNotFound"; break;
  case FontStyleNotFound: status_str = "FontStyleNotFound"; break;
  case NotTrueTypeFont: status_str = "NotTrueTypeFont"; break;
  case UnsupportedGdiplusVersion: status_str = "UnsupportedGdiplusVersion"; break;
  case GdiplusNotInitialized: status_str = "GdiplusNotInitialized"; break;
  case PropertyNotFound: status_str = "PropertyNotFound"; break;
  case PropertyNotSupported: status_str = "PropertyNotSupported"; break;
  }

  if (status != Ok)
    MessageBox(NULL, status_str, title, MB_OK);
}

/*
%F Libera memoria e handles alocados pelo driver Windows.
*/
void cdwpKillCanvas(cdCtxCanvas* ctxcanvas)
{
  if (ctxcanvas->clip_poly) delete[] ctxcanvas->clip_poly;
  if (ctxcanvas->clip_fpoly) delete[] ctxcanvas->clip_fpoly;
  if (ctxcanvas->clip_region) delete ctxcanvas->clip_region;
  if (ctxcanvas->new_region) delete ctxcanvas->new_region;
  if (ctxcanvas->font) delete ctxcanvas->font;

  delete ctxcanvas->fillBrush;
  delete ctxcanvas->lineBrush;
  delete ctxcanvas->linePen;

  delete ctxcanvas->graphics;

  /* ctxcanvas e´ liberado em cada driver */
}

static int sAddTransform(cdCtxCanvas* ctxcanvas, Matrix &transformMatrix, const double* matrix)
{
  if (matrix)
  {
    // configure a bottom-up coordinate system
    Matrix Matrix1((REAL)1, (REAL)0, (REAL)0, (REAL)-1, (REAL)0, (REAL)(ctxcanvas->canvas->h-1));
    transformMatrix.Multiply(&Matrix1);

    // add the global transform
    Matrix Matrix2((REAL)matrix[0], (REAL)matrix[1], (REAL)matrix[2], (REAL)matrix[3], (REAL)matrix[4], (REAL)matrix[5]);
    transformMatrix.Multiply(&Matrix2);
    return 1;
  }
  else if (ctxcanvas->rotate_angle)
  {
    /* the rotation must be corrected because of the Y axis orientation */
    transformMatrix.Translate((REAL)ctxcanvas->rotate_center_x, (REAL)_cdInvertYAxis(ctxcanvas->canvas, ctxcanvas->rotate_center_y));
    transformMatrix.Rotate((REAL)-ctxcanvas->rotate_angle);
    transformMatrix.Translate((REAL)-ctxcanvas->rotate_center_x, (REAL)-_cdInvertYAxis(ctxcanvas->canvas, ctxcanvas->rotate_center_y));
    return 1;
  }

  return 0;
}

static void sUpdateTransform(cdCtxCanvas* ctxcanvas)
{
  Matrix transformMatrix;
  ctxcanvas->graphics->ResetTransform(); // reset to the identity.
  if (sAddTransform(ctxcanvas, transformMatrix, ctxcanvas->canvas->use_matrix? ctxcanvas->canvas->matrix: NULL))
    ctxcanvas->graphics->SetTransform(&transformMatrix);
}

/*********************************************************************/
/*
%S                            Cor                                    
*/
/*********************************************************************/

static Color sColor2Windows(long int cd_color)
{
  return Color(cdAlpha(cd_color),cdRed(cd_color),cdGreen(cd_color),cdBlue(cd_color));
}

static void sUpdateFillBrush(cdCtxCanvas* ctxcanvas)
{
  // must update the fill brush that is dependent from the Foreground and Background Color.
  BrushType type = ctxcanvas->fillBrush->GetType();
  switch(type)
  {
  case BrushTypeSolidColor:
    {
      SolidBrush* solidbrush = (SolidBrush*)ctxcanvas->fillBrush;
      solidbrush->SetColor(ctxcanvas->fg);
      break;
    }
  case BrushTypeHatchFill:
    {
      HatchBrush* hatchbrush = (HatchBrush*)ctxcanvas->fillBrush;
      HatchStyle hatchStyle = hatchbrush->GetHatchStyle();
      delete ctxcanvas->fillBrush;
      ctxcanvas->fillBrush = new HatchBrush(hatchStyle, ctxcanvas->fg, ctxcanvas->bg); 
      break;
    }
  case BrushTypeLinearGradient:
    {
      LinearGradientBrush* gradientbrush = (LinearGradientBrush*)ctxcanvas->fillBrush;
      gradientbrush->SetLinearColors(ctxcanvas->fg, ctxcanvas->bg);
      break;
    }
  case BrushTypeTextureFill:
    {
      // only stipple depends on Foreground and Background Color.
      if (ctxcanvas->canvas->interior_style == CD_STIPPLE)
        cdstipple(ctxcanvas, ctxcanvas->canvas->stipple_w, ctxcanvas->canvas->stipple_h, ctxcanvas->canvas->stipple);
      break;
    }
  }
}

static long int cdforeground(cdCtxCanvas* ctxcanvas, long int color)
{
  ctxcanvas->fg = sColor2Windows(color);
  ctxcanvas->linePen->SetColor(ctxcanvas->fg);
  ctxcanvas->lineBrush->SetColor(ctxcanvas->fg);

  sUpdateFillBrush(ctxcanvas);

  return color;
}

static Color sTranspAlpha(const Color& c)
{
  return Color(0, c.GetRed(), c.GetGreen(), c.GetBlue());
}

static long int cdbackground(cdCtxCanvas* ctxcanvas, long int color)
{
  ctxcanvas->bg = sColor2Windows(color);

  if (ctxcanvas->canvas->back_opacity == CD_TRANSPARENT) 
    ctxcanvas->bg = sTranspAlpha(ctxcanvas->bg);  /* set background as full transparent */

  sUpdateFillBrush(ctxcanvas);

  return color;
}

static int cdbackopacity(cdCtxCanvas* ctxcanvas, int opacity)
{
  switch (opacity)
  {
  case CD_TRANSPARENT:
    ctxcanvas->bg = sTranspAlpha(ctxcanvas->bg);  /* set background as full transparent */
    break;
  case CD_OPAQUE:
    ctxcanvas->bg = sColor2Windows(ctxcanvas->canvas->background);
    break;
  }

  sUpdateFillBrush(ctxcanvas);

  return opacity;
}

static void cdpalette(cdCtxCanvas* ctxcanvas, int pal_size, const long int *colors, int mode)
{
  (void)mode;

  if (ctxcanvas->wtype == CDW_BMP && 
      (ctxcanvas->bitmap->GetPixelFormat() == PixelFormat1bppIndexed ||
       ctxcanvas->bitmap->GetPixelFormat() == PixelFormat4bppIndexed ||
       ctxcanvas->bitmap->GetPixelFormat() == PixelFormat8bppIndexed))
  {
    UINT size = ctxcanvas->bitmap->GetPaletteSize();
    ColorPalette* palette = new ColorPalette [size];

    palette->Count = pal_size;
    palette->Flags = 0;

    for (int c = 0; c < pal_size; c++)
    {
      palette->Entries[c] = sColor2Windows(colors[c]).GetValue();
    }

    ctxcanvas->bitmap->SetPalette(palette);
    delete palette;
  }
}

/*********************************************************************/
/*
%S                 Canvas e clipping
*/
/*********************************************************************/

static void sClipRect(cdCtxCanvas* ctxcanvas)
{
  if (ctxcanvas->clip_region)
    delete ctxcanvas->clip_region;

  Rect rect(ctxcanvas->canvas->clip_rect.xmin, 
            ctxcanvas->canvas->clip_rect.ymin,
            ctxcanvas->canvas->clip_rect.xmax-ctxcanvas->canvas->clip_rect.xmin+1, 
            ctxcanvas->canvas->clip_rect.ymax-ctxcanvas->canvas->clip_rect.ymin+1);  
                                                 
  ctxcanvas->clip_region = new Region(rect);

  ctxcanvas->graphics->SetClip(ctxcanvas->clip_region);
}

static void sClipPoly(cdCtxCanvas* ctxcanvas)
{
  if (ctxcanvas->clip_region)
    delete ctxcanvas->clip_region;

  GraphicsPath path;
  path.SetFillMode(ctxcanvas->canvas->fill_mode==CD_EVENODD?FillModeAlternate:FillModeWinding);
  if (ctxcanvas->clip_fpoly)
    path.AddPolygon(ctxcanvas->clip_fpoly, ctxcanvas->clip_poly_n);
  else
    path.AddPolygon(ctxcanvas->clip_poly, ctxcanvas->clip_poly_n);
  ctxcanvas->clip_region = new Region(&path);

  ctxcanvas->graphics->SetClip(ctxcanvas->clip_region);
}

static int cdclip(cdCtxCanvas* ctxcanvas, int clip_mode)
{
  switch (clip_mode) 
  {
  case CD_CLIPOFF:
    ctxcanvas->graphics->ResetClip();
    if (ctxcanvas->clip_region) 
      delete ctxcanvas->clip_region;
    ctxcanvas->clip_region = NULL;
    break;
  case CD_CLIPAREA:
    sClipRect(ctxcanvas);
    break;
  case CD_CLIPPOLYGON:
    sClipPoly(ctxcanvas);
    break;
  case CD_CLIPREGION:
    if (ctxcanvas->clip_region)
      delete ctxcanvas->clip_region;
    ctxcanvas->clip_region = ctxcanvas->new_region->Clone();
    ctxcanvas->graphics->SetClip(ctxcanvas->clip_region);
    break;
  }

  return clip_mode;
}

static void cdcliparea(cdCtxCanvas* ctxcanvas, int xmin, int xmax, int ymin, int ymax)
{
  if (ctxcanvas->canvas->clip_mode == CD_CLIPAREA) 
  {
    ctxcanvas->canvas->clip_rect.xmin = xmin;
    ctxcanvas->canvas->clip_rect.xmax = xmax;
    ctxcanvas->canvas->clip_rect.ymin = ymin;
    ctxcanvas->canvas->clip_rect.ymax = ymax;

    sClipRect(ctxcanvas);
  }
}

static void cdnewregion(cdCtxCanvas* ctxcanvas)
{
  if (ctxcanvas->new_region)
    delete ctxcanvas->new_region;
  ctxcanvas->new_region = new Region(); 
  ctxcanvas->new_region->MakeEmpty();
}

static int cdispointinregion(cdCtxCanvas* ctxcanvas, int x, int y)
{
  if (!ctxcanvas->new_region)
    return 0;

  if (ctxcanvas->new_region->IsVisible(x, y))
    return 1;

  return 0;
}

static void cdoffsetregion(cdCtxCanvas* ctxcanvas, int x, int y)
{
  if (!ctxcanvas->new_region)
    return;

  ctxcanvas->new_region->Translate(x, y);
}

static void cdgetregionbox(cdCtxCanvas* ctxcanvas, int *xmin, int *xmax, int *ymin, int *ymax)
{
  Rect rect;

  if (!ctxcanvas->new_region)
    return;

  ctxcanvas->new_region->GetBounds(&rect, ctxcanvas->graphics);

  *xmin = rect.X;
  *xmax = rect.X+rect.Width-1;
  *ymin = rect.Y;
  *ymax = rect.Y+rect.Height-1;
}

static void sCombineRegion(cdCtxCanvas* ctxcanvas, Region& region)
{
  switch(ctxcanvas->canvas->combine_mode)
  {
  case CD_UNION:
    ctxcanvas->new_region->Union(&region);
    break;
  case CD_INTERSECT:
    ctxcanvas->new_region->Intersect(&region);
    break;
  case CD_DIFFERENCE:
    ctxcanvas->new_region->Exclude(&region);
    break;
  case CD_NOTINTERSECT:
    ctxcanvas->new_region->Xor(&region);
    break;
  }
}

/******************************************************************/
/*
%S                  Primitivas e seus atributos                        
*/
/******************************************************************/

static int cdlinestyle(cdCtxCanvas* ctxcanvas, int style)
{
  switch (style)
  {
  case (CD_CONTINUOUS):
    {
      ctxcanvas->linePen->SetDashStyle(DashStyleSolid);
      break;
    }
  case CD_DASHED:                
    {
      if (ctxcanvas->canvas->line_width == 1)
      {
        REAL dashed[2] = {18.0f, 6.0f};
        ctxcanvas->linePen->SetDashPattern(dashed, 2);
      }
      else
        ctxcanvas->linePen->SetDashStyle(DashStyleDash);
      break;
    }
  case CD_DOTTED:                  
    {
      if (ctxcanvas->canvas->line_width == 1)
      {
        REAL dotted[2] = {3.0f, 3.0f};
        ctxcanvas->linePen->SetDashPattern(dotted, 2);
      }
      else
        ctxcanvas->linePen->SetDashStyle(DashStyleDot);
      break;
    }
  case CD_DASH_DOT:               
    {
      if (ctxcanvas->canvas->line_width == 1)
      {
        REAL dash_dot[6] = {9.0f, 6.0f, 3.0f, 6.0f};
        ctxcanvas->linePen->SetDashPattern(dash_dot, 4);
      }
      else
        ctxcanvas->linePen->SetDashStyle(DashStyleDashDot);
      break;
    }
  case CD_DASH_DOT_DOT:        
    {
      if (ctxcanvas->canvas->line_width == 1)
      {
        REAL dash_dot_dot[6] = {9.0f, 3.0f, 3.0f, 3.0f, 3.0f, 3.0f};
        ctxcanvas->linePen->SetDashPattern(dash_dot_dot, 6);
      }
      else
        ctxcanvas->linePen->SetDashStyle(DashStyleDashDotDot);
      break;
    }
  case CD_CUSTOM:        
    {
      REAL* dash_style = new REAL [ctxcanvas->canvas->line_dashes_count];
      for (int i = 0; i < ctxcanvas->canvas->line_dashes_count; i++)
        dash_style[i] = (REAL)ctxcanvas->canvas->line_dashes[i]/(REAL)ctxcanvas->canvas->line_width;
      ctxcanvas->linePen->SetDashPattern(dash_style, ctxcanvas->canvas->line_dashes_count);
      delete [] dash_style;
      break;
    }
  }
 
  return style;
}

static int cdlinecap(cdCtxCanvas* ctxcanvas, int cap)
{
  LineCap cd2win_lcap[] = {LineCapFlat, LineCapSquare, LineCapRound};
  DashCap cd2win_dcap[] = {DashCapFlat, DashCapFlat, DashCapRound};

  ctxcanvas->linePen->SetLineCap(cd2win_lcap[cap], cd2win_lcap[cap], cd2win_dcap[cap]);

  return cap;
}

static int cdlinejoin(cdCtxCanvas* ctxcanvas, int join)
{
  LineJoin cd2win_join[] = {LineJoinMiter, LineJoinBevel, LineJoinRound};

  ctxcanvas->linePen->SetLineJoin(cd2win_join[join]);

  return join;
}

static int cdlinewidth(cdCtxCanvas* ctxcanvas, int width)
{
  ctxcanvas->linePen->SetWidth((REAL)width);
  cdlinestyle(ctxcanvas, ctxcanvas->canvas->line_style);
  return width;
}

static int cdhatch(cdCtxCanvas* ctxcanvas, int hatch_style)
{
  HatchStyle hatchStyle;

  switch (hatch_style)
  {
  default: // CD_HORIZONTAL:
    hatchStyle = HatchStyleHorizontal;
    break;
  case CD_VERTICAL:
    hatchStyle = HatchStyleVertical;
    break;
  case CD_FDIAGONAL:
    hatchStyle = HatchStyleForwardDiagonal;
    break;
  case CD_BDIAGONAL:
    hatchStyle = HatchStyleBackwardDiagonal;
    break;
  case CD_CROSS:
    hatchStyle = HatchStyleCross;
    break;
  case CD_DIAGCROSS:
    hatchStyle = HatchStyleDiagonalCross;
    break;                 
  }
  
  delete ctxcanvas->fillBrush;
  ctxcanvas->fillBrush = new HatchBrush(hatchStyle, ctxcanvas->fg, ctxcanvas->bg); 
  
  return hatch_style;
}

static void cdstipple(cdCtxCanvas* ctxcanvas, int w, int h, const unsigned char *index)
{
  ULONG* bitmap_data = new ULONG [w*h];

  for(int j = 0; j < h; j++)
  {
    int line_offset_index;
    int line_offset = j*w;
    if (ctxcanvas->canvas->invert_yaxis)
      line_offset_index = ((h - 1) - j)*w;  // Fix up side down
    else
      line_offset_index = line_offset;

    for(int i = 0; i < w; i++) 
    {
      if (index[line_offset_index+i] != 0)
        bitmap_data[line_offset+i] = ctxcanvas->fg.GetValue();
      else
        bitmap_data[line_offset+i] = ctxcanvas->bg.GetValue();
    }
  }

  Bitmap StippleImage(w, h, 4*w, PixelFormat32bppARGB, (BYTE*)bitmap_data);
  StippleImage.SetResolution(ctxcanvas->graphics->GetDpiX(), ctxcanvas->graphics->GetDpiX());

  delete ctxcanvas->fillBrush;
  ctxcanvas->fillBrush = new TextureBrush(&StippleImage); 

  delete bitmap_data;
}

static void cdpattern(cdCtxCanvas* ctxcanvas, int w, int h, const long int *colors)
{
  int stride = 4*w;
  BYTE* bitmap_data = new BYTE [stride*h];

  for(int j = 0; j < h; j++)
  {
    int line_offset_colors;
    int line_offset = j*stride;
    /* internal transform, affects also pattern orientation */
    if (ctxcanvas->canvas->invert_yaxis)
      line_offset_colors = ((h - 1) - j)*w;  // Fix up side down
    else
      line_offset_colors = j*w;
    memcpy(bitmap_data + line_offset, colors + line_offset_colors, stride);

    // Fix alpha values
    for(int i = 3; i < stride; i+=4) 
      bitmap_data[line_offset+i] = ~bitmap_data[line_offset+i];
  }

  Bitmap PatternImage(w, h, stride, PixelFormat32bppARGB, bitmap_data);
  PatternImage.SetResolution(ctxcanvas->graphics->GetDpiX(), ctxcanvas->graphics->GetDpiX());

  delete ctxcanvas->fillBrush;
  ctxcanvas->fillBrush = new TextureBrush(&PatternImage); 

  delete bitmap_data;
}

static int cdinteriorstyle(cdCtxCanvas* ctxcanvas, int style)
{
  switch (style)
  {
  case CD_SOLID:
    delete ctxcanvas->fillBrush;
    ctxcanvas->fillBrush = new SolidBrush(ctxcanvas->fg);
    break;
    /* the remaining styles must recreate the current brush */
  case CD_HATCH:
    cdhatch(ctxcanvas, ctxcanvas->canvas->hatch_style);
    break;
  case CD_STIPPLE:
    cdstipple(ctxcanvas, ctxcanvas->canvas->stipple_w, ctxcanvas->canvas->stipple_h, ctxcanvas->canvas->stipple);
    break;
  case CD_PATTERN:
    cdpattern(ctxcanvas, ctxcanvas->canvas->pattern_w, ctxcanvas->canvas->pattern_h, ctxcanvas->canvas->pattern);
    break;
  }
  
  return style;
}

static void cdline(cdCtxCanvas* ctxcanvas, int x1, int y1, int x2, int y2)
{
  ctxcanvas->graphics->DrawLine(ctxcanvas->linePen, x1, y1, x2, y2);
  ctxcanvas->dirty = 1;
}

static void cdfline(cdCtxCanvas* ctxcanvas, double x1, double y1, double x2, double y2)
{
  ctxcanvas->graphics->DrawLine(ctxcanvas->linePen, (REAL)x1, (REAL)y1, (REAL)x2, (REAL)y2);
  ctxcanvas->dirty = 1;
}

static void cdrect(cdCtxCanvas* ctxcanvas, int xmin, int xmax, int ymin, int ymax)
{
  Rect rect(xmin, ymin, xmax-xmin, ymax-ymin);  // in this case Size = Max - Min 
  ctxcanvas->graphics->DrawRectangle(ctxcanvas->linePen, rect); 
  ctxcanvas->dirty = 1;
}

static void cdfrect(cdCtxCanvas* ctxcanvas, double xmin, double xmax, double ymin, double ymax)
{
  RectF rect((REAL)xmin, (REAL)ymin, (REAL)(xmax-xmin), (REAL)(ymax-ymin));  // in this case Size = Max - Min 
  ctxcanvas->graphics->DrawRectangle(ctxcanvas->linePen, rect); 
  ctxcanvas->dirty = 1;
}

static void cdbox(cdCtxCanvas* ctxcanvas, int xmin, int xmax, int ymin, int ymax)
{
  Rect rect(xmin, ymin, xmax-xmin+1, ymax-ymin+1); 
  if (ctxcanvas->canvas->new_region)
  {
    Region region(rect);
    sCombineRegion(ctxcanvas, region);
  }
  else
  {
    ctxcanvas->graphics->FillRectangle(ctxcanvas->fillBrush, rect);
    ctxcanvas->dirty = 1;
  }
}

static void cdfbox(cdCtxCanvas* ctxcanvas, double xmin, double xmax, double ymin, double ymax)
{
  RectF rect((REAL)xmin, (REAL)ymin, (REAL)(xmax-xmin+1), (REAL)(ymax-ymin+1));
  if (ctxcanvas->canvas->new_region)
  {
    Region region(rect);
    sCombineRegion(ctxcanvas, region);
  }
  else
  {
    ctxcanvas->graphics->FillRectangle(ctxcanvas->fillBrush, rect);
    ctxcanvas->dirty = 1;
  }
}

static void sFixAngles(cdCanvas* canvas, double *a1, double *a2)
{
  // GDI+ angles are clock-wise by default, in degrees

  /* if NOT inverted means a transformation is set, 
     so the angle will follow the transformation that includes the axis invertion,
     then it is already counter-clockwise */

  if (canvas->invert_yaxis)
  {
    /* change orientation */
    *a1 *= -1;
    *a2 *= -1;

    /* no need to swap, because we will use (angle2-angle1) */
  }
}

static void cdarc(cdCtxCanvas* ctxcanvas, int xc, int yc, int w, int h, double angle1, double angle2)
{
  Rect rect(xc - w/2, yc - h/2, w, h);
  if (angle1 == 0 && angle2 == 360)
    ctxcanvas->graphics->DrawEllipse(ctxcanvas->linePen, rect);
  else
  {
    sFixAngles(ctxcanvas->canvas, &angle1, &angle2);
    ctxcanvas->graphics->DrawArc(ctxcanvas->linePen, rect, (REAL)angle1, (REAL)(angle2-angle1));
  }
  ctxcanvas->dirty = 1;
}

static void cdfarc(cdCtxCanvas* ctxcanvas, double xc, double yc, double w, double h, double angle1, double angle2)
{
  RectF rect((REAL)(xc - w/2.0), (REAL)(yc - h/2.0), (REAL)w, (REAL)h);
  if (angle1 == 0 && angle2 == 360)
    ctxcanvas->graphics->DrawEllipse(ctxcanvas->linePen, rect);
  else
  {
    sFixAngles(ctxcanvas->canvas, &angle1, &angle2);
    ctxcanvas->graphics->DrawArc(ctxcanvas->linePen, rect, (REAL)angle1, (REAL)(angle2-angle1));
  }
  ctxcanvas->dirty = 1;
}

static void cdsector(cdCtxCanvas* ctxcanvas, int xc, int yc, int w, int h, double angle1, double angle2)
{
  Rect rect(xc - w/2, yc - h/2, w, h);
  if (ctxcanvas->canvas->new_region)
  {
    GraphicsPath path;
    if (angle1==0 && angle2==360)
      path.AddEllipse(rect);
    else
    {
      sFixAngles(ctxcanvas->canvas, &angle1, &angle2);
      path.AddPie(rect, (REAL)angle1, (REAL)(angle2-angle1));
    }
    Region region(&path);
    sCombineRegion(ctxcanvas, region);
  }
  else
  {
    // complete the remaining pixels using an Arc
    Pen pen(ctxcanvas->fillBrush); 

    if (angle1==0 && angle2==360)
    {
      ctxcanvas->graphics->FillEllipse(ctxcanvas->fillBrush, rect);
      ctxcanvas->graphics->DrawEllipse(&pen, rect);
    }
    else
    {
      sFixAngles(ctxcanvas->canvas, &angle1, &angle2);
      ctxcanvas->graphics->FillPie(ctxcanvas->fillBrush, rect, (REAL)angle1, (REAL)(angle2-angle1));
      ctxcanvas->graphics->DrawArc(&pen, rect, (REAL)angle1, (REAL)(angle2-angle1));
    }
    ctxcanvas->dirty = 1;
  }
}

static void cdfsector(cdCtxCanvas* ctxcanvas, double xc, double yc, double w, double h, double angle1, double angle2)
{
  RectF rect((REAL)(xc - w/2.0), (REAL)(yc - h/2.0), (REAL)w, (REAL)h);
  if (ctxcanvas->canvas->new_region)
  {
    GraphicsPath path;
    if (angle1==0 && angle2==360)
      path.AddEllipse(rect);
    else
    {
      sFixAngles(ctxcanvas->canvas, &angle1, &angle2);
      path.AddPie(rect, (REAL)angle1, (REAL)(angle2-angle1));
    }
    Region region(&path);
    sCombineRegion(ctxcanvas, region);
  }
  else
  {
    // complete the remaining pixels using an Arc
    Pen pen(ctxcanvas->fillBrush); 

    if (angle1==0 && angle2==360)
    {
      ctxcanvas->graphics->FillEllipse(ctxcanvas->fillBrush, rect);
      ctxcanvas->graphics->DrawEllipse(&pen, rect);
    }
    else
    {
      sFixAngles(ctxcanvas->canvas, &angle1, &angle2);
      ctxcanvas->graphics->FillPie(ctxcanvas->fillBrush, rect, (REAL)angle1, (REAL)(angle2-angle1));
      ctxcanvas->graphics->DrawArc(&pen, rect, (REAL)angle1, (REAL)(angle2-angle1));
    }
    ctxcanvas->dirty = 1;
  }
}

static void cdchord(cdCtxCanvas* ctxcanvas, int xc, int yc, int w, int h, double angle1, double angle2)
{
  Rect rect(xc - w/2, yc - h/2, w, h);
  if (ctxcanvas->canvas->new_region)
  {
    GraphicsPath path;
    if (angle1==0 && angle2==360)
      path.AddEllipse(rect);
    else
    {
      sFixAngles(ctxcanvas->canvas, &angle1, &angle2);
      path.AddArc(rect, (REAL)angle1, (REAL)(angle2-angle1));
      path.CloseFigure();
    }
    Region region(&path);
    sCombineRegion(ctxcanvas, region);
  }
  else
  {
    if (angle1==0 && angle2==360)
      ctxcanvas->graphics->FillEllipse(ctxcanvas->fillBrush, rect);
    else
    {
      GraphicsPath path;
      sFixAngles(ctxcanvas->canvas, &angle1, &angle2);
      path.AddArc(rect, (REAL)angle1, (REAL)(angle2-angle1));
      ctxcanvas->graphics->FillPath(ctxcanvas->fillBrush, &path);
    }
    Pen pen(ctxcanvas->fillBrush); // complete the remaining pixels using an Arc
    ctxcanvas->graphics->DrawArc(&pen, rect, (REAL)angle1, (REAL)(angle2-angle1));
    ctxcanvas->dirty = 1;
  }
}

static void cdfchord(cdCtxCanvas* ctxcanvas, double xc, double yc, double w, double h, double angle1, double angle2)
{
  RectF rect((REAL)(xc - w/2.0), (REAL)(yc - h/2.0), (REAL)w, (REAL)h);
  if (ctxcanvas->canvas->new_region)
  {
    GraphicsPath path;
    if (angle1==0 && angle2==360)
      path.AddEllipse(rect);
    else
    {
      sFixAngles(ctxcanvas->canvas, &angle1, &angle2);
      path.AddArc(rect, (REAL)angle1, (REAL)(angle2-angle1));
      path.CloseFigure();
    }
    Region region(&path);
    sCombineRegion(ctxcanvas, region);
  }
  else
  {
    if (angle1==0 && angle2==360)
      ctxcanvas->graphics->FillEllipse(ctxcanvas->fillBrush, rect);
    else
    {
      GraphicsPath path;
      sFixAngles(ctxcanvas->canvas, &angle1, &angle2);
      path.AddArc(rect, (REAL)angle1, (REAL)(angle2-angle1));
      ctxcanvas->graphics->FillPath(ctxcanvas->fillBrush, &path);
    }
    Pen pen(ctxcanvas->fillBrush); // complete the remaining pixels using an Arc
    ctxcanvas->graphics->DrawArc(&pen, rect, (REAL)angle1, (REAL)(angle2-angle1));
    ctxcanvas->dirty = 1;
  }
}

static void cdpoly(cdCtxCanvas* ctxcanvas, int mode, cdPoint* poly, int n)
{
  switch (mode)
  {
  case CD_PATH:
    {
      int p, i, current_x = 0, current_y = 0, current_set = 0;
      GraphicsPath* graphics_path;
      PointF lastPoint;

      /* starts a new path */
      graphics_path = new GraphicsPath(ctxcanvas->canvas->fill_mode==CD_EVENODD?FillModeAlternate:FillModeWinding);

      i = 0;
      for (p=0; p<ctxcanvas->canvas->path_n; p++)
      {
        switch(ctxcanvas->canvas->path[p])
        {
        case CD_PATH_NEW:
          graphics_path->Reset();
          graphics_path->SetFillMode(ctxcanvas->canvas->fill_mode==CD_EVENODD?FillModeAlternate:FillModeWinding);
          current_set = 0;
          break;
        case CD_PATH_MOVETO:
          if (i+1 > n) break;
          current_x = poly[i].x;
          current_y = poly[i].y;
          current_set = 1;
          i++;
          break;
        case CD_PATH_LINETO:
          if (i+1 > n) break;
          if (current_set)
            graphics_path->AddLine(current_x, current_y, poly[i].x, poly[i].y);
          current_x = poly[i].x;
          current_y = poly[i].y;
          current_set = 1;
          i++;
          break;
        case CD_PATH_ARC:
          {
            int xc, yc, w, h;
            double a1, a2;

            if (i+3 > n) break;

            if (!cdCanvasGetArcPath(ctxcanvas->canvas, poly+i, &xc, &yc, &w, &h, &a1, &a2))
              return;

            if (current_set)
            {
              int StartX, StartY;

              if (ctxcanvas->canvas->invert_yaxis)
                cdCanvasGetArcStartEnd(xc, yc, w, h, -a1, -a2, &StartX, &StartY, NULL, NULL);
              else
                cdCanvasGetArcStartEnd(xc, yc, w, h, a1, a2, &StartX, &StartY, NULL, NULL);

              graphics_path->AddLine(current_x, current_y, StartX, StartY);
            }

            Rect rect(xc - w/2, yc - h/2, w, h);
            if (a1 == 0 && a2 == 360)
              graphics_path->AddEllipse(rect);
            else
            {
              sFixAngles(ctxcanvas->canvas, &a1, &a2);
              graphics_path->AddArc(rect, (REAL)a1, (REAL)(a2-a1));
            }

            graphics_path->GetLastPoint(&lastPoint);
            current_x = (int)lastPoint.X;
            current_y = (int)lastPoint.Y;
            current_set = 1;

            i += 3;
          }
          break;
        case CD_PATH_CURVETO:
          if (i+3 > n) break;
          if (!current_set)
          {
            current_x = poly[i].x;
            current_y = poly[i].y;
          }
          graphics_path->AddBezier(current_x, current_y, poly[i].x, poly[i].y, poly[i+1].x, poly[i+1].y, poly[i+2].x, poly[i+2].y);
          graphics_path->GetLastPoint(&lastPoint);
          current_x = (int)lastPoint.X;
          current_y = (int)lastPoint.Y;
          current_set = 1;
          i += 3;
          break;
        case CD_PATH_CLOSE:
          graphics_path->CloseFigure();
          break;
        case CD_PATH_FILL:
          ctxcanvas->graphics->FillPath(ctxcanvas->fillBrush, graphics_path);
          break;
        case CD_PATH_STROKE:
          ctxcanvas->graphics->DrawPath(ctxcanvas->linePen, graphics_path);
          break;
        case CD_PATH_FILLSTROKE:
          ctxcanvas->graphics->FillPath(ctxcanvas->fillBrush, graphics_path);
          ctxcanvas->graphics->DrawPath(ctxcanvas->linePen, graphics_path);
          break;
        case CD_PATH_CLIP:
          ctxcanvas->graphics->SetClip(graphics_path, CombineModeIntersect);
          break;
        }
      }

      delete graphics_path;
      break;
    }
  case CD_BEZIER:
    if (n < 4) return;
    ctxcanvas->graphics->DrawBeziers(ctxcanvas->linePen, (Point*)poly, n);
    break;
  case CD_FILLSPLINE:
    if (n < 4) return;
    if (ctxcanvas->canvas->new_region)
    {
      GraphicsPath path(ctxcanvas->canvas->fill_mode==CD_EVENODD?FillModeAlternate:FillModeWinding);
      path.AddClosedCurve((Point*)poly, n);
      Region region(&path);
      sCombineRegion(ctxcanvas, region);
    }
    else
      ctxcanvas->graphics->FillClosedCurve(ctxcanvas->fillBrush, (Point*)poly, n);
    break;
  case CD_SPLINE:
    if (n < 4) return;
    ctxcanvas->graphics->DrawClosedCurve(ctxcanvas->linePen, (Point*)poly, n);
    break;
  case CD_CLOSED_LINES:
    poly[n].x = poly[0].x;
    poly[n].y = poly[0].y;
    n++;
    /* continue */
  case CD_OPEN_LINES:
    ctxcanvas->graphics->DrawLines(ctxcanvas->linePen, (Point*)poly, n);
    break;
  case CD_FILLGRADIENT:
    {
      int count = n;
      PathGradientBrush* brush = new PathGradientBrush((Point*)poly, n);
      brush->SetSurroundColors(ctxcanvas->pathGradient, &count);
      brush->SetCenterColor(ctxcanvas->pathGradient[n]);
      ctxcanvas->graphics->FillPolygon(brush, (Point*)poly, n, ctxcanvas->canvas->fill_mode==CD_EVENODD?FillModeAlternate:FillModeWinding);
    }
    break;
  case CD_FILL:
    poly[n].x = poly[0].x;
    poly[n].y = poly[0].y;
    n++;
    if (ctxcanvas->canvas->new_region)
    {
      GraphicsPath path(ctxcanvas->canvas->fill_mode==CD_EVENODD?FillModeAlternate:FillModeWinding);
      path.AddPolygon((Point*)poly, n);
      Region region(&path);
      sCombineRegion(ctxcanvas, region);
    }
    else
      ctxcanvas->graphics->FillPolygon(ctxcanvas->fillBrush, (Point*)poly, n, ctxcanvas->canvas->fill_mode==CD_EVENODD?FillModeAlternate:FillModeWinding);
    break;
  case CD_CLIP:
    poly[n].x = poly[0].x;
    poly[n].y = poly[0].y;
    n++;
    
    if (ctxcanvas->clip_poly)
      delete[] ctxcanvas->clip_poly;
    if (ctxcanvas->clip_fpoly)
    {
      delete[] ctxcanvas->clip_fpoly;
      ctxcanvas->clip_fpoly = NULL;
    }
    
    ctxcanvas->clip_poly = new Point [n];

    Point* pnt = (Point*)poly;
    int t = n;
    int nc = 1;

    ctxcanvas->clip_poly[0] = *pnt;
    pnt++;

    for (int i = 1; i < t-1; i++, pnt++)
    {
      if (!((pnt->X == ctxcanvas->clip_poly[nc-1].X && pnt->X == (pnt + 1)->X) || 
            (pnt->Y == ctxcanvas->clip_poly[nc-1].Y && pnt->Y == (pnt + 1)->Y)))
      {
        ctxcanvas->clip_poly[nc] = *pnt;
        nc++;
      }
    }

    ctxcanvas->clip_poly_n = nc;
    
    if (ctxcanvas->canvas->clip_mode == CD_CLIPPOLYGON) 
      sClipPoly(ctxcanvas);
    
    break;
  }

  ctxcanvas->dirty = 1;
}

static PointF* sPolyToFloat(cdfPoint* poly, int n)
{
  PointF* fpoly = new PointF[n+1];

  for (int i = 0; i < n; i++)
  {
    fpoly[i].X = (REAL)poly[i].x;
    fpoly[i].Y = (REAL)poly[i].y;
  }

  return fpoly;
}

static void cdfpoly(cdCtxCanvas* ctxcanvas, int mode, cdfPoint* poly, int n)
{
  PointF* fpoly = NULL;

  switch (mode)
  {
  case CD_PATH:
    {
      int p, i, current_set = 0;
      double current_x = 0, current_y = 0;
      GraphicsPath* graphics_path;
      PointF lastPoint;

      /* starts a new path */
      graphics_path = new GraphicsPath(ctxcanvas->canvas->fill_mode==CD_EVENODD?FillModeAlternate:FillModeWinding);

      i = 0;
      for (p=0; p<ctxcanvas->canvas->path_n; p++)
      {
        switch(ctxcanvas->canvas->path[p])
        {
        case CD_PATH_NEW:
          graphics_path->Reset();
          graphics_path->SetFillMode(ctxcanvas->canvas->fill_mode==CD_EVENODD?FillModeAlternate:FillModeWinding);
          current_set = 0;
          break;
        case CD_PATH_MOVETO:
          if (i+1 > n) break;
          current_x = poly[i].x;
          current_y = poly[i].y;
          current_set = 1;
          i++;
          break;
        case CD_PATH_LINETO:
          if (i+1 > n) break;
          if (current_set)
            graphics_path->AddLine((REAL)current_x, (REAL)current_y, (REAL)poly[i].x, (REAL)poly[i].y);
          current_x = poly[i].x;
          current_y = poly[i].y;
          current_set = 1;
          i++;
          break;
        case CD_PATH_ARC:
          {
            double xc, yc, w, h;
            double a1, a2;

            if (i+3 > n) break;

            if (!cdfCanvasGetArcPath(ctxcanvas->canvas, poly+i, &xc, &yc, &w, &h, &a1, &a2))
              return;

            if (current_set)
            {
              double StartX, StartY;

              if (ctxcanvas->canvas->invert_yaxis)
                cdfCanvasGetArcStartEnd(xc, yc, w, h, -a1, -a2, &StartX, &StartY, NULL, NULL);
              else
                cdfCanvasGetArcStartEnd(xc, yc, w, h, a1, a2, &StartX, &StartY, NULL, NULL);

              graphics_path->AddLine((REAL)current_x, (REAL)current_y, (REAL)StartX, (REAL)StartY);
            }

            RectF rect((REAL)(xc - w/2.0), (REAL)(yc - h/2.0), (REAL)w, (REAL)h);
            if (a1 == 0 && a2 == 360)
              graphics_path->AddEllipse(rect);
            else
            {
              sFixAngles(ctxcanvas->canvas, &a1, &a2);
              graphics_path->AddArc(rect, (REAL)a1, (REAL)(a2-a1));
            }

            graphics_path->GetLastPoint(&lastPoint);
            current_x = lastPoint.X;
            current_y = lastPoint.Y;
            current_set = 1;

            i += 3;
          }
          break;
        case CD_PATH_CURVETO:
          if (i+3 > n) break;
          if (!current_set)
          {
            current_x = poly[i].x;
            current_y = poly[i].y;
          }
          graphics_path->AddBezier((REAL)current_x, (REAL)current_y, (REAL)poly[i].x, (REAL)poly[i].y, (REAL)poly[i+1].x, (REAL)poly[i+1].y, (REAL)poly[i+2].x, (REAL)poly[i+2].y);
          graphics_path->GetLastPoint(&lastPoint);
          current_x = lastPoint.X;
          current_y = lastPoint.Y;
          current_set = 1;
          i += 3;
          break;
        case CD_PATH_CLOSE:
          graphics_path->CloseFigure();
          break;
        case CD_PATH_FILL:
          ctxcanvas->graphics->FillPath(ctxcanvas->fillBrush, graphics_path);
          break;
        case CD_PATH_STROKE:
          ctxcanvas->graphics->DrawPath(ctxcanvas->linePen, graphics_path);
          break;
        case CD_PATH_FILLSTROKE:
          ctxcanvas->graphics->FillPath(ctxcanvas->fillBrush, graphics_path);
          ctxcanvas->graphics->DrawPath(ctxcanvas->linePen, graphics_path);
          break;
        case CD_PATH_CLIP:
          ctxcanvas->graphics->SetClip(graphics_path, CombineModeIntersect);
          break;
        }
      }

      delete graphics_path;
      break;
    }
  case CD_BEZIER:
    if (n < 4) return;
    fpoly = sPolyToFloat(poly, n);
    ctxcanvas->graphics->DrawBeziers(ctxcanvas->linePen, (PointF*)fpoly, n);
    break;
  case CD_FILLSPLINE:
    if (n < 4) return;
    fpoly = sPolyToFloat(poly, n);
    if (ctxcanvas->canvas->new_region)
    {
      GraphicsPath path(ctxcanvas->canvas->fill_mode==CD_EVENODD?FillModeAlternate:FillModeWinding);
      path.AddClosedCurve((PointF*)fpoly, n);
      Region region(&path);
      sCombineRegion(ctxcanvas, region);
    }
    else
      ctxcanvas->graphics->FillClosedCurve(ctxcanvas->fillBrush, (PointF*)fpoly, n);
    break;
  case CD_SPLINE:
    if (n < 4) return;
    fpoly = sPolyToFloat(poly, n);
    ctxcanvas->graphics->DrawClosedCurve(ctxcanvas->linePen, (PointF*)fpoly, n);
    break;
  case CD_CLOSED_LINES:
    poly[n].x = poly[0].x;
    poly[n].y = poly[0].y;
    n++;
    /* continue */
  case CD_OPEN_LINES:
    fpoly = sPolyToFloat(poly, n);
    ctxcanvas->graphics->DrawLines(ctxcanvas->linePen, (PointF*)fpoly, n);
    break;
  case CD_FILLGRADIENT:
    {
      int count = n;
      PathGradientBrush* brush = new PathGradientBrush((PointF*)fpoly, n);
      fpoly = sPolyToFloat(poly, n);
      brush->SetSurroundColors(ctxcanvas->pathGradient, &count);
      brush->SetCenterColor(ctxcanvas->pathGradient[n]);
      ctxcanvas->graphics->FillPolygon(brush, (PointF*)fpoly, n, ctxcanvas->canvas->fill_mode==CD_EVENODD?FillModeAlternate:FillModeWinding);
      delete brush;
    }
    break;
  case CD_FILL:
    poly[n].x = poly[0].x;
    poly[n].y = poly[0].y;
    n++;
    fpoly = sPolyToFloat(poly, n);
    if (ctxcanvas->canvas->new_region)
    {
      GraphicsPath path(ctxcanvas->canvas->fill_mode==CD_EVENODD?FillModeAlternate:FillModeWinding);
      path.AddPolygon((PointF*)fpoly, n);
      Region region(&path);
      sCombineRegion(ctxcanvas, region);
    }
    else
      ctxcanvas->graphics->FillPolygon(ctxcanvas->fillBrush, (PointF*)fpoly, n, ctxcanvas->canvas->fill_mode==CD_EVENODD?FillModeAlternate:FillModeWinding);
    break;
  case CD_CLIP:
    poly[n].x = poly[0].x;
    poly[n].y = poly[0].y;
    n++;
    
    if (ctxcanvas->clip_fpoly)
      delete[] ctxcanvas->clip_fpoly;
    if (ctxcanvas->clip_poly)
    {
      delete[] ctxcanvas->clip_poly;
      ctxcanvas->clip_poly = NULL;
    }
    
    ctxcanvas->clip_fpoly = new PointF [n];

    cdfPoint* pnt = poly;
    int t = n;
    int nc = 1;

    ctxcanvas->clip_fpoly[0].X = (REAL)pnt->x;
    ctxcanvas->clip_fpoly[0].Y = (REAL)pnt->y;
    pnt++;

    for (int i = 1; i < t-1; i++, pnt++)
    {
      if (!(((REAL)pnt->x == ctxcanvas->clip_fpoly[nc-1].X && pnt->x == (pnt + 1)->x) || 
            ((REAL)pnt->y == ctxcanvas->clip_fpoly[nc-1].Y && pnt->y == (pnt + 1)->y)))
      {
        ctxcanvas->clip_fpoly[nc].X = (REAL)pnt->x;
        ctxcanvas->clip_fpoly[nc].Y = (REAL)pnt->y;
        nc++;
      }
    }

    ctxcanvas->clip_poly_n = nc;
    
    if (ctxcanvas->canvas->clip_mode == CD_CLIPPOLYGON) 
      sClipPoly(ctxcanvas);
    
    break;
  }

  if (fpoly)
    delete[] fpoly;

  ctxcanvas->dirty = 1;
}

WCHAR* cdwpString2Unicode(const char* s, int len)
{
  static WCHAR wstr[10240] = L"";
  if (len >= 10240) len = 10239;
  MultiByteToWideChar(CP_ACP, 0, s, -1, wstr, len+1);
  return wstr;
}

static int cdwpCompensateHeight(int height)
{
  return (int)floor(height/10. + 0.5);  /* 10% */
}

static void cdgettextsize(cdCtxCanvas* ctxcanvas, const char *s, int len, int *width, int *height)
{
  RectF boundingBox;

  ctxcanvas->graphics->MeasureString(cdwpString2Unicode(s, len), len, 
                                            ctxcanvas->font, PointF(0,0),
                                            &boundingBox);
  if (width)  
    *width  = (int)boundingBox.Width;
  
  if (height) 
    *height = (int)boundingBox.Height - cdwpCompensateHeight(ctxcanvas->fontinfo.height);
}

static void sTextBox(cdCtxCanvas* ctxcanvas, WCHAR *ws, int len, int x, int y, int *w, int *h, int *xmin, int *ymin)
{
  int ydir = 1;
  if (ctxcanvas->canvas->invert_yaxis)
    ydir = -1;

  RectF boundingBox;
  ctxcanvas->graphics->MeasureString(ws, len, 
                                     ctxcanvas->font, PointF(0,0),
                                     &boundingBox);
  *w = (int)boundingBox.Width;
  *h = (int)boundingBox.Height - cdwpCompensateHeight(ctxcanvas->fontinfo.height);

  // distance from bottom to baseline
  int baseline = ctxcanvas->fontinfo.height - ctxcanvas->fontinfo.ascent; 

  // move to bottom-left
  cdTextTranslatePoint(ctxcanvas->canvas, x, y, *w, *h, baseline, xmin, ymin);

  // move to top-left
  *ymin += ydir * (*h);
}

static void sGetTransformTextHeight(cdCanvas* canvas, int x, int y, int w, int h, int *hbox)
{
  int xmin, xmax, ymin, ymax;

  // distance from bottom to baseline
  int baseline = canvas->ctxcanvas->fontinfo.height - canvas->ctxcanvas->fontinfo.ascent; 

  /* move to bottom-left */
  cdTextTranslatePoint(canvas, x, y, w, h, baseline, &xmin, &ymin);

  xmax = xmin + w-1;
  ymax = ymin + h-1;

  if (canvas->text_orientation)
  {
    double cos_theta = cos(canvas->text_orientation*CD_DEG2RAD);
    double sin_theta = sin(canvas->text_orientation*CD_DEG2RAD);
    int rectY[4];

    cdRotatePointY(canvas, xmin, ymin, x, y, &rectY[0], sin_theta, cos_theta);
    cdRotatePointY(canvas, xmax, ymin, x, y, &rectY[1], sin_theta, cos_theta);
    cdRotatePointY(canvas, xmax, ymax, x, y, &rectY[2], sin_theta, cos_theta);
    cdRotatePointY(canvas, xmin, ymax, x, y, &rectY[3], sin_theta, cos_theta);

    ymin = ymax = rectY[0];
    if (rectY[1] < ymin) ymin = rectY[1];
    if (rectY[2] < ymin) ymin = rectY[2];
    if (rectY[3] < ymin) ymin = rectY[3];
    if (rectY[1] > ymax) ymax = rectY[1];
    if (rectY[2] > ymax) ymax = rectY[2];
    if (rectY[3] > ymax) ymax = rectY[3];
  }

  *hbox = ymax-ymin+1;
}

static void sAddTextTransform(cdCtxCanvas* ctxcanvas, int *x, int *y, int w, int h, Matrix &transformMatrix)
{
  int hbox;
  Matrix m1;

  sGetTransformTextHeight(ctxcanvas->canvas, *x, *y, w, h, &hbox);

  // move to (x,y) and remove a vertical offset since text reference point is top-left
  m1.SetElements((REAL)1, (REAL)0, (REAL)0, (REAL)1, (REAL)*x, (REAL)(*y - (hbox-1)));
  transformMatrix.Multiply(&m1);

  // invert the text vertical orientation, relative to itself
  m1.SetElements((REAL)1, (REAL)0, (REAL)0, (REAL)-1, (REAL)0, (REAL)(hbox-1));
  transformMatrix.Multiply(&m1);

  *x = 0;
  *y = 0;
}

static void cdtext(cdCtxCanvas* ctxcanvas, int x, int y, const char *s, int len)
{
  Matrix transformMatrix;
  int use_transform = 0, w, h;
  WCHAR* ws = cdwpString2Unicode(s, len);

  if (ctxcanvas->canvas->text_orientation)
  {
    transformMatrix.Translate((REAL)x, (REAL)y);
    transformMatrix.Rotate((REAL)-ctxcanvas->canvas->text_orientation);
    transformMatrix.Translate((REAL)-x, (REAL)-y);
    use_transform = 1;
  }

  // Move (x,y) to top-left
  sTextBox(ctxcanvas, ws, len, x, y, &w, &h, &x, &y);

  if (ctxcanvas->canvas->use_matrix)
  {
    double* matrix = ctxcanvas->canvas->matrix;
    sAddTransform(ctxcanvas, transformMatrix, matrix);
    sAddTextTransform(ctxcanvas, &x, &y, w, h, transformMatrix);
    use_transform = 1;
  }
  else if (sAddTransform(ctxcanvas, transformMatrix, NULL))
    use_transform = 1;

  if (ctxcanvas->canvas->new_region)
  {
    GraphicsPath path(ctxcanvas->canvas->fill_mode==CD_EVENODD?FillModeAlternate:FillModeWinding);

    if (use_transform)
      path.Transform(&transformMatrix);

    FontFamily family;
    ctxcanvas->font->GetFamily(&family);
    path.AddString(ws, len, &family, ctxcanvas->font->GetStyle(), 
                                     ctxcanvas->font->GetSize(), 
                                     Point(x, y), NULL);

    Region region(&path);
    sCombineRegion(ctxcanvas, region);
    return;
  }

  if (use_transform)
    ctxcanvas->graphics->SetTransform(&transformMatrix);

  ctxcanvas->graphics->DrawString(ws, len,
                                  ctxcanvas->font, PointF((REAL)x, (REAL)y),  
                                  ctxcanvas->lineBrush);

  if (use_transform)
    sUpdateTransform(ctxcanvas); // reset transform

  ctxcanvas->dirty = 1;
}

static int sDesign2Pixel(int x, REAL size, int height)
{
  return (int)((x * size) / height);
}

static int cdfont(cdCtxCanvas* ctxcanvas, const char *type_face, int style, int size)
{
  FontFamily* fontFamily;
    
  if (cdStrEqualNoCase(type_face, "Courier") || cdStrEqualNoCase(type_face, "Monospace"))
    fontFamily = new FontFamily(L"Courier New");
  else if (cdStrEqualNoCase(type_face, "Times") || cdStrEqualNoCase(type_face, "Serif"))
    fontFamily = new FontFamily(L"Times New Roman");
  else if (cdStrEqualNoCase(type_face, "Helvetica") || cdStrEqualNoCase(type_face, "Sans"))
    fontFamily = new FontFamily(L"Arial");
  else if (cdStrEqualNoCase(type_face, "System"))
    fontFamily = FontFamily::GenericSansSerif()->Clone(); 
  else
    fontFamily = new FontFamily(cdwpString2Unicode(type_face, strlen(type_face)));
  
  REAL emSize = (REAL)(cdGetFontSizePixels(ctxcanvas->canvas, size));

  INT fontStyle = FontStyleRegular;
  if (style&CD_BOLD)      fontStyle |= FontStyleBold;
  if (style&CD_ITALIC)    fontStyle |= FontStyleItalic;
  if (style&CD_UNDERLINE) fontStyle |= FontStyleUnderline;
  if (style&CD_STRIKEOUT) fontStyle |= FontStyleStrikeout;

  if (ctxcanvas->font) delete ctxcanvas->font;
  ctxcanvas->font = new Font(fontFamily, emSize, fontStyle, UnitPixel);

  REAL fontSize = ctxcanvas->font->GetSize();
  int emHeight = fontFamily->GetEmHeight(fontStyle);
  ctxcanvas->fontinfo.height = sDesign2Pixel(fontFamily->GetLineSpacing(fontStyle), fontSize, emHeight);
  ctxcanvas->fontinfo.ascent = sDesign2Pixel(fontFamily->GetCellAscent(fontStyle), fontSize, emHeight);
  ctxcanvas->fontinfo.descent = sDesign2Pixel(fontFamily->GetCellDescent(fontStyle), fontSize, emHeight);

  delete fontFamily;

  return 1;
}

static int cdnativefont(cdCtxCanvas* ctxcanvas, const char* nativefont)
{
  int size = 12, bold = 0, italic = 0, underline = 0, strikeout = 0, style = CD_PLAIN;
  char type_face[1024];
  
  if (nativefont[0] == '-' && nativefont[1] == 'd')
  {              
    COLORREF rgbColors;
    LOGFONT lf;
    ctxcanvas->font->GetLogFontA(ctxcanvas->graphics, &lf);
    
    CHOOSEFONT cf;
    ZeroMemory(&cf, sizeof(CHOOSEFONT));

    cf.lStructSize = sizeof(CHOOSEFONT);
    cf.hwndOwner = GetForegroundWindow();
    cf.lpLogFont = &lf; 
    cf.Flags = CF_SCREENFONTS | CF_EFFECTS | CF_INITTOLOGFONTSTRUCT;
    rgbColors = cf.rgbColors = ctxcanvas->fg.ToCOLORREF();
    
    if (!ChooseFont(&cf))
      return 0;
                
    if (rgbColors != cf.rgbColors)
      cdCanvasSetForeground(ctxcanvas->canvas, cdEncodeColor(GetRValue(cf.rgbColors),GetGValue(cf.rgbColors),GetBValue(cf.rgbColors)));

    bold = lf.lfWeight>FW_NORMAL? 1: 0;
    italic = lf.lfItalic;
    size = lf.lfHeight;
    strcpy(type_face, lf.lfFaceName);
    underline = lf.lfUnderline;
    strikeout = lf.lfStrikeOut;

    if (bold) style |= CD_BOLD;
    if (italic) style |= CD_ITALIC;
    if (underline) style |= CD_UNDERLINE;
    if (strikeout) style |= CD_STRIKEOUT;
  }                                     
  else
  {
    if (!cdParseIupWinFont(nativefont, type_face, &style, &size))
    {
      if (!cdParsePangoFont(nativefont, type_face, &style, &size))
        return 0;
    }
      
    if (style&CD_BOLD)
      bold = 1;
    if (style&CD_ITALIC)
      italic = 1;
    if (style&CD_UNDERLINE)
      underline = 1;
    if (style&CD_STRIKEOUT)
      strikeout = 1;
  }

  REAL emSize = (REAL)(cdGetFontSizePixels(ctxcanvas->canvas, size));

  INT fontStyle = FontStyleRegular;
  if (bold) fontStyle |= FontStyleBold;
  if (italic) fontStyle |= FontStyleItalic;
  if (underline) fontStyle |= FontStyleUnderline;
  if (strikeout) fontStyle |= FontStyleStrikeout;

  const char* new_type_face = type_face;
  if (strstr(type_face, "Times") != NULL)
    new_type_face = "Times";

  if (strstr(type_face, "Courier") != NULL)
    new_type_face = "Courier";

  if (strstr(type_face, "Arial") != NULL)
    new_type_face = "Helvetica";

  if (strstr(type_face, "Helv") != NULL)
    new_type_face = "Helvetica";

  FontFamily *fontFamily;
  if (strstr(type_face, "System") != NULL)
  {
    new_type_face = "System";
    fontFamily = FontFamily::GenericSansSerif()->Clone();
  }
  else
    fontFamily = new FontFamily(cdwpString2Unicode(type_face, strlen(type_face)));

  if (!fontFamily->IsAvailable())
  {
    delete fontFamily;
    return 0;
  }
    
  Font* font = new Font(fontFamily, emSize, fontStyle, UnitPixel);
  if (!font->IsAvailable())
  {
    delete fontFamily;
    delete font;
    return 0;
  }

  if (ctxcanvas->font) delete ctxcanvas->font;
  ctxcanvas->font = font;

  REAL fontSize = ctxcanvas->font->GetSize();
  int emHeight = fontFamily->GetEmHeight(fontStyle);
  ctxcanvas->fontinfo.height  = sDesign2Pixel(fontFamily->GetLineSpacing(fontStyle), fontSize, emHeight);
  ctxcanvas->fontinfo.ascent  = sDesign2Pixel(fontFamily->GetCellAscent(fontStyle), fontSize, emHeight);
  ctxcanvas->fontinfo.descent = sDesign2Pixel(fontFamily->GetCellDescent(fontStyle), fontSize, emHeight);

  delete fontFamily;

  /* update cdfont parameters */
  ctxcanvas->canvas->font_style = style;
  ctxcanvas->canvas->font_size = size;
  strcpy(ctxcanvas->canvas->font_type_face, new_type_face);

  return 1;
}

static void cdgetfontdim(cdCtxCanvas* ctxcanvas, int *max_width, int *line_height, int *ascent, int *descent)
{
  if (max_width)  
  {
    RectF boundingBox;
    ctxcanvas->graphics->MeasureString(L"W", 2, 
                                       ctxcanvas->font, PointF(0,0),
                                       &boundingBox);
    *max_width = (int)(boundingBox.Width/2);
  }
  
  if (line_height) 
    *line_height = ctxcanvas->fontinfo.height;
  
  if (ascent)      
    *ascent = ctxcanvas->fontinfo.ascent;
  
  if (descent)     
    *descent = ctxcanvas->fontinfo.descent;
}

static void cdtransform(cdCtxCanvas *ctxcanvas, const double* matrix)
{
  Matrix transformMatrix;
  ctxcanvas->graphics->ResetTransform(); // reset to the identity.

  if (matrix)
    ctxcanvas->canvas->invert_yaxis = 0;
  else
    ctxcanvas->canvas->invert_yaxis = 1;

  if (sAddTransform(ctxcanvas, transformMatrix, matrix))
    ctxcanvas->graphics->SetTransform(&transformMatrix);
}

static void cdclear(cdCtxCanvas* ctxcanvas)
{
  if (ctxcanvas->canvas->clip_mode != CD_CLIPOFF) 
    ctxcanvas->graphics->ResetClip();
  
  /* do NOT use "ctxcanvas->bg" here, because it depends on backopacity */
  ctxcanvas->graphics->Clear(sColor2Windows(ctxcanvas->canvas->background));
  
  if (ctxcanvas->canvas->clip_mode != CD_CLIPOFF) 
    cdclip(ctxcanvas, ctxcanvas->canvas->clip_mode);

  ctxcanvas->dirty = 1;
}

/******************************************************************/
/*
%S             Funcoes de imagens do cliente                      
*/
/******************************************************************/

static void sRGB2Bitmap(Bitmap& image, int width, int height, const unsigned char *red, 
                                                              const unsigned char *green, 
                                                              const unsigned char *blue, 
                                       int xmin, int xmax, int ymin, int ymax, int topdown)
{
  BitmapData bitmapData;
  Rect rect(0,0,image.GetWidth(),image.GetHeight());
  image.LockBits(&rect, ImageLockModeWrite, PixelFormat24bppRGB, &bitmapData); 

  /* ymin and xmax unused */
  (void)ymin;
  (void)xmax;

  int line_offset;
  for(int j = 0; j < rect.Height; j++)
  {
    if (topdown)
      line_offset = (height - (ymax - j) - 1)*width;
    else
      line_offset = (ymax - j)*width;

    BYTE* line_data = (BYTE*)bitmapData.Scan0 + j*bitmapData.Stride;

    for(int i = 0; i < rect.Width; i++) 
    {
      int offset = line_offset + i + xmin;
      int offset_data = i*3;
      line_data[offset_data+0] = blue[offset];
      line_data[offset_data+1] = green[offset];
      line_data[offset_data+2] = red[offset];
    }
  }

  image.UnlockBits(&bitmapData);
}

static void sRGBA2Bitmap(Bitmap& image, int width, int height, const unsigned char *red, 
                                                               const unsigned char *green, 
                                                               const unsigned char *blue, 
                                                               const unsigned char *alpha, 
                                       int xmin, int xmax, int ymin, int ymax, int topdown)
{
  BitmapData bitmapData;
  Rect rect(0,0,image.GetWidth(),image.GetHeight());
  image.LockBits(&rect, ImageLockModeWrite, PixelFormat32bppARGB, &bitmapData); 

  /* ymin and xmax unused */
  (void)ymin;
  (void)xmax;

  int line_offset;
  for(int j = 0; j < rect.Height; j++)
  {
    if (topdown)
      line_offset = (height - (ymax - j) - 1)*width;
    else
      line_offset = (ymax - j)*width;

    BYTE* line_data = (BYTE*)bitmapData.Scan0 + j*bitmapData.Stride;

    for(int i = 0; i < rect.Width; i++) 
    {
      int offset = line_offset + i + xmin;
      int offset_data = i*4;
      line_data[offset_data+0] = blue? blue[offset]: 0;
      line_data[offset_data+1] = green? green[offset]: 0;
      line_data[offset_data+2] = red? red[offset]: 0;
      line_data[offset_data+3] = alpha[offset];
    }
  }

  image.UnlockBits(&bitmapData);
}

static void sAlpha2Bitmap(Bitmap& image, int width, int height, const unsigned char *alpha, 
                                         int xmin, int xmax, int ymin, int ymax, int topdown)
{
  BitmapData bitmapData;
  Rect rect(0,0,image.GetWidth(),image.GetHeight());
  image.LockBits(&rect, ImageLockModeWrite, PixelFormat32bppARGB, &bitmapData); 

  /* ymin and xmax unused */
  (void)ymin;
  (void)xmax;

  int line_offset;
  for(int j = 0; j < rect.Height; j++)
  {
    if (topdown)
      line_offset = (height - (ymax - j) - 1)*width;
    else
      line_offset = (ymax - j)*width;

    BYTE* line_data = (BYTE*)bitmapData.Scan0 + j*bitmapData.Stride;

    for(int i = 0; i < rect.Width; i++) 
    {
      int offset = line_offset + i + xmin;
      int offset_data = i*4;
      line_data[offset_data+3] = alpha[offset];
    }
  }

  image.UnlockBits(&bitmapData);
}

/*
GDI+ does not natively work with palettized images.

GDI+ will not try to draw on a palettized image as it would
require color reducing the drawing result.

GDI+ will work natively with 32bpp image data behind the scenes anyway so
there is no economy in trying to work with 8bpp images.

Full support for palettized images was on the feature list but it did not
make the cut for version 1 of GDI+.
*/

static void sMap2Bitmap(Bitmap& image, int width, int height, const unsigned char *index, 
                                                              const long int *colors, 
                                       int xmin, int xmax, int ymin, int ymax, int topdown)
{
  BitmapData bitmapData;
  Rect rect(0,0,image.GetWidth(),image.GetHeight());
  image.LockBits(&rect, ImageLockModeWrite, PixelFormat24bppRGB, &bitmapData); 

  /* ymin and xmax unused */
  (void)ymin;
  (void)xmax;

  int line_offset;
  for(int j = 0; j < rect.Height; j++)
  {
    if (topdown)
      line_offset = (height - (ymax - j) - 1)*width;
    else
      line_offset = (ymax - j)*width;

    BYTE* line_data = (BYTE*)bitmapData.Scan0 + j*bitmapData.Stride;

    for(int i = 0; i < rect.Width; i++) 
    {
      int map_index = index[line_offset + i + xmin];
      long color = colors[map_index];

      int offset_data = i*3;
      line_data[offset_data+0] = cdBlue(color);
      line_data[offset_data+1] = cdGreen(color);
      line_data[offset_data+2] = cdRed(color);
    }
  }

  image.UnlockBits(&bitmapData);
}

static void sBitmap2RGB(Bitmap& image, unsigned char *red, unsigned char *green, unsigned char *blue)
{
  BitmapData bitmapData;
  Rect rect(0,0,image.GetWidth(),image.GetHeight());
  image.LockBits(&rect, ImageLockModeRead, PixelFormat24bppRGB, &bitmapData); 

  for(int j = 0; j < rect.Height; j++)
  {
    int line_offset = ((rect.Height-1) - j)*rect.Width;

    BYTE* line_data = (BYTE*)bitmapData.Scan0 + j*bitmapData.Stride;

    for(int i = 0; i < rect.Width; i++) 
    {
      int offset = line_offset + i;
      int offset_data = i*3;
      blue[offset]  = line_data[offset_data+0];
      green[offset] = line_data[offset_data+1];
      red[offset]   = line_data[offset_data+2];
    }
  }

  image.UnlockBits(&bitmapData);
}

static void cdgetimagergb(cdCtxCanvas* ctxcanvas, unsigned char *red, unsigned char *green, unsigned char *blue, 
                           int x, int y, int w, int h)
{
  Matrix transformMatrix;
  ctxcanvas->graphics->GetTransform(&transformMatrix);
  if (!transformMatrix.IsIdentity())
    ctxcanvas->graphics->ResetTransform(); // reset to the identity.

  if (ctxcanvas->canvas->invert_yaxis==0) // if 0, invert because the transform was reset here
    y = _cdInvertYAxis(ctxcanvas->canvas, y);

  int yr = y - (h - 1);  /* y starts at the bottom of the image */
  Bitmap image(w, h, PixelFormat24bppRGB);
  image.SetResolution(ctxcanvas->graphics->GetDpiX(), ctxcanvas->graphics->GetDpiX());

  Graphics* imggraphics = new Graphics(&image);

  if (ctxcanvas->wtype == CDW_BMP)
  {
    imggraphics->DrawImage(ctxcanvas->bitmap, 
                           Rect(0, 0, w, h), 
                           x, yr, w, h, UnitPixel,
                           NULL, NULL, NULL);
  }
  else
  {
    HDC hdc = ctxcanvas->graphics->GetHDC();
    HDC img_hdc = imggraphics->GetHDC();

    BitBlt(img_hdc,0,0,w,h,hdc,x,yr,SRCCOPY);

    imggraphics->ReleaseHDC(img_hdc);
    ctxcanvas->graphics->ReleaseHDC(hdc);
  }

  delete imggraphics;

  sBitmap2RGB(image, red, green, blue);

  if (!transformMatrix.IsIdentity())
    ctxcanvas->graphics->SetTransform(&transformMatrix);
}

static void sFixImageY(cdCanvas* canvas, int *topdown, int *y, int *h)
{
  if (canvas->invert_yaxis)
  {
    if (*h < 0)
      *topdown = 1;
    else
      *topdown = 0;
  }
  else
  {
    if (*h < 0)
      *topdown = 0;
    else
      *topdown = 1;
  }

  if (*h < 0)
    *h = -(*h);        // y is at top-left corner (UNDOCUMENTED FEATURE)
  
  if (!(*topdown))
    *y -= ((*h) - 1);  // move Y to top-left corner, since it was at the bottom of the image
}

static void cdputimagerectmap(cdCtxCanvas* ctxcanvas, int width, int height, const unsigned char *index, 
                                                      const long int *colors, 
                               int x, int y, int w, int h, 
                               int xmin, int xmax, int ymin, int ymax)
{
  int topdown;
  Bitmap image(xmax-xmin+1, ymax-ymin+1, PixelFormat24bppRGB);
  image.SetResolution(ctxcanvas->graphics->GetDpiX(), ctxcanvas->graphics->GetDpiX());

  sFixImageY(ctxcanvas->canvas, &topdown, &y, &h);
  sMap2Bitmap(image, width, height, index, colors, xmin, xmax, ymin, ymax, topdown);

  ImageAttributes imageAttributes;
  if (ctxcanvas->use_img_transp)
    imageAttributes.SetColorKey(ctxcanvas->img_transp[0], ctxcanvas->img_transp[1], ColorAdjustTypeBitmap);

  if(ctxcanvas->use_img_points)
  {
    Point pts[3];
    pts[0] = ctxcanvas->img_points[0];
    pts[1] = ctxcanvas->img_points[1];
    pts[2] = ctxcanvas->img_points[2];
    if (ctxcanvas->canvas->invert_yaxis)
    {
      pts[0].Y = _cdInvertYAxis(ctxcanvas->canvas, pts[0].Y);
      pts[1].Y = _cdInvertYAxis(ctxcanvas->canvas, pts[1].Y);
      pts[2].Y = _cdInvertYAxis(ctxcanvas->canvas, pts[2].Y);
    }
    ctxcanvas->graphics->DrawImage(&image, pts, 3, 
                                   0, 0, image.GetWidth(), image.GetHeight(), UnitPixel,
                                   &imageAttributes, NULL, NULL);
  }
  else
  {
    REAL bw = (REAL)image.GetWidth();
    REAL bh = (REAL)image.GetHeight();
    if (w > bw) bw-=0.5;  // Fix the problem of not using PixelOffsetModeHalf
    if (h > bh) bh-=0.5;
    ctxcanvas->graphics->DrawImage(&image, RectF((REAL)x, (REAL)y, (REAL)w, (REAL)h), 
                                   0, 0, bw, bh, UnitPixel,
                                   &imageAttributes, NULL, NULL);
  }

  ctxcanvas->dirty = 1;
}

static void cdputimagerectrgb(cdCtxCanvas* ctxcanvas, int width, int height, const unsigned char *red, 
                                                      const unsigned char *green, 
                                                      const unsigned char *blue, 
                               int x, int y, int w, int h, 
                               int xmin, int xmax, int ymin, int ymax)
{
  int topdown;
  Bitmap image(xmax-xmin+1, ymax-ymin+1, PixelFormat24bppRGB);
  image.SetResolution(ctxcanvas->graphics->GetDpiX(), ctxcanvas->graphics->GetDpiX());

  sFixImageY(ctxcanvas->canvas, &topdown, &y, &h);
  sRGB2Bitmap(image, width, height, red, green, blue, xmin, xmax, ymin, ymax, topdown);

  ImageAttributes imageAttributes;
  if (ctxcanvas->use_img_transp)
    imageAttributes.SetColorKey(ctxcanvas->img_transp[0], ctxcanvas->img_transp[1], ColorAdjustTypeBitmap);

  if(ctxcanvas->use_img_points)
  {
    Point pts[3];
    pts[0] = ctxcanvas->img_points[0];
    pts[1] = ctxcanvas->img_points[1];
    pts[2] = ctxcanvas->img_points[2];
    if (ctxcanvas->canvas->invert_yaxis)
    {
      pts[0].Y = _cdInvertYAxis(ctxcanvas->canvas, pts[0].Y);
      pts[1].Y = _cdInvertYAxis(ctxcanvas->canvas, pts[1].Y);
      pts[2].Y = _cdInvertYAxis(ctxcanvas->canvas, pts[2].Y);
    }
    ctxcanvas->graphics->DrawImage(&image, pts, 3, 
                                   0, 0, image.GetWidth(), image.GetHeight(), UnitPixel,
                                   &imageAttributes, NULL, NULL);
  }
  else
  {
    REAL bw = (REAL)image.GetWidth();
    REAL bh = (REAL)image.GetHeight();
    if (w > bw) bw-=0.5;  // Fix the problem of not using PixelOffsetModeHalf
    if (h > bh) bh-=0.5;
    ctxcanvas->graphics->DrawImage(&image, RectF((REAL)x, (REAL)y, (REAL)w, (REAL)h), 
                                   0, 0, bw, bh, UnitPixel,
                                   &imageAttributes, NULL, NULL);
  }

  ctxcanvas->dirty = 1;
}

static void cdputimagerectrgba(cdCtxCanvas* ctxcanvas, int width, int height, const unsigned char *red, 
                                                       const unsigned char *green, 
                                                       const unsigned char *blue, 
                                                       const unsigned char *alpha, 
                                int x, int y, int w, int h, 
                                int xmin, int xmax, int ymin, int ymax)
{
  int topdown;
  Bitmap image(xmax-xmin+1, ymax-ymin+1, PixelFormat32bppARGB);
  image.SetResolution(ctxcanvas->graphics->GetDpiX(), ctxcanvas->graphics->GetDpiX());

  sFixImageY(ctxcanvas->canvas, &topdown, &y, &h);
  sRGBA2Bitmap(image, width, height, red, green, blue, alpha, xmin, xmax, ymin, ymax, topdown);

  ImageAttributes imageAttributes;
  if(ctxcanvas->use_img_points)
  {
    Point pts[3];
    pts[0] = ctxcanvas->img_points[0];
    pts[1] = ctxcanvas->img_points[1];
    pts[2] = ctxcanvas->img_points[2];
    if (ctxcanvas->canvas->invert_yaxis)
    {
      pts[0].Y = _cdInvertYAxis(ctxcanvas->canvas, pts[0].Y);
      pts[1].Y = _cdInvertYAxis(ctxcanvas->canvas, pts[1].Y);
      pts[2].Y = _cdInvertYAxis(ctxcanvas->canvas, pts[2].Y);
    }
    ctxcanvas->graphics->DrawImage(&image, pts, 3, 
                                   0, 0, image.GetWidth(), image.GetHeight(), UnitPixel,
                                   &imageAttributes, NULL, NULL);
  }
  else
  {
    REAL bw = (REAL)image.GetWidth();
    REAL bh = (REAL)image.GetHeight();
    if (w > bw) bw-=0.5;  // Fix the problem of not using PixelOffsetModeHalf
    if (h > bh) bh-=0.5;
    ctxcanvas->graphics->DrawImage(&image, RectF((REAL)x, (REAL)y, (REAL)w, (REAL)h), 
                                   0, 0, bw, bh, UnitPixel,
                                   &imageAttributes, NULL, NULL);
  }

  ctxcanvas->dirty = 1;
}

/********************************************************************/
/*
%S        Funcoes de imagens do servidor                          
*/
/********************************************************************/

static void cdpixel(cdCtxCanvas* ctxcanvas, int x, int y, long int cd_color)
{
  if (!ctxcanvas->graphics->IsVisible(x, y))
    return;

  if (ctxcanvas->wtype == CDW_BMP)
  {
    ctxcanvas->bitmap->SetPixel(x, y, sColor2Windows(cd_color));
  }
  else
  {
    if (ctxcanvas->canvas->use_matrix)
    {
      Point p = Point(x, y);
      ctxcanvas->graphics->TransformPoints(CoordinateSpacePage, CoordinateSpaceWorld, &p, 1);
      x = p.X;
      y = p.Y;
    }
    HDC hdc = ctxcanvas->graphics->GetHDC();
    SetPixelV(hdc, x, y, sColor2Windows(cd_color).ToCOLORREF());
    ctxcanvas->graphics->ReleaseHDC(hdc);
  }

  ctxcanvas->dirty = 1;
}

static int sFormat2Bpp(PixelFormat pixelFormat)
{
  switch(pixelFormat)
  {
  case PixelFormat1bppIndexed:
    return 1;
  case PixelFormat4bppIndexed: 
    return 4;
  case PixelFormat8bppIndexed: 
    return 8;
  case PixelFormat16bppARGB1555: 
  case PixelFormat16bppGrayScale: 
  case PixelFormat16bppRGB555: 
  case PixelFormat16bppRGB565: 
    return 16;
  case PixelFormat24bppRGB: 
    return 24;
  case PixelFormat32bppARGB: 
  case PixelFormat32bppPARGB: 
  case PixelFormat32bppRGB: 
    return 32;
  case PixelFormat48bppRGB: 
    return 48;
  case PixelFormat64bppARGB: 
  case PixelFormat64bppPARGB:
    return 64;
  }

  return 0;
}

static cdCtxImage *cdcreateimage(cdCtxCanvas* ctxcanvas, int width, int height)
{
  cdCtxImage *ctximage = new cdCtxImage;
  ctximage->alpha = NULL;
  
  if (ctxcanvas->img_format)
  {
    ctximage->bitmap = new Bitmap(width, height, ctxcanvas->img_format==24? PixelFormat24bppRGB: PixelFormat32bppARGB);
    if (!ctximage->bitmap)
    {
      delete ctximage;
      return NULL;
    }

    if (ctxcanvas->img_format==32 && ctxcanvas->img_alpha)
      ctximage->alpha = ctxcanvas->img_alpha;

    ctximage->bitmap->SetResolution(ctxcanvas->graphics->GetDpiX(), ctxcanvas->graphics->GetDpiX());
  }
  else
  {
    ctximage->bitmap = new Bitmap(width, height, ctxcanvas->graphics);
    if (!ctximage->bitmap)
    {
      delete ctximage;
      return NULL;
    }
  }

  ctximage->w = width;
  ctximage->h = height;

  ctximage->bpp = sFormat2Bpp(ctximage->bitmap->GetPixelFormat());
  ctximage->xres = ctximage->bitmap->GetHorizontalResolution() / 25.4;
  ctximage->yres = ctximage->bitmap->GetVerticalResolution() / 25.4;

  ctximage->w_mm = ctximage->w / ctximage->xres;
  ctximage->h_mm = ctximage->h / ctximage->yres;

  Graphics imggraphics(ctximage->bitmap);
  imggraphics.Clear(Color((ARGB)Color::White));
  
  return ctximage;
}

static void cdgetimage(cdCtxCanvas* ctxcanvas, cdCtxImage *ctximage, int x, int y)
{
  Matrix transformMatrix;
  ctxcanvas->graphics->GetTransform(&transformMatrix);
  if (!transformMatrix.IsIdentity())
    ctxcanvas->graphics->ResetTransform(); // reset to the identity.

  if (ctxcanvas->canvas->invert_yaxis==0)  // if 0, invert because the transform was reset here
    y = _cdInvertYAxis(ctxcanvas->canvas, y);

  /* y is the bottom-left of the image in CD, must be at upper-left */
  y -= ctximage->h-1;

  if (ctxcanvas->wtype == CDW_BMP)
  {
    Graphics imggraphics(ctximage->bitmap);

    imggraphics.DrawImage(ctxcanvas->bitmap, 
                          Rect(0, 0, ctximage->w,ctximage->h), 
                          x, y, ctximage->w, ctximage->h, UnitPixel,
                          NULL, NULL, NULL);
  }
  else
  {
    Graphics imggraphics(ctximage->bitmap);

    HDC hdc = ctxcanvas->graphics->GetHDC();
    HDC img_hdc = imggraphics.GetHDC();

    BitBlt(img_hdc,0,0,ctximage->w,ctximage->h,hdc,x,y,SRCCOPY);

    imggraphics.ReleaseHDC(img_hdc);
    ctxcanvas->graphics->ReleaseHDC(hdc);
  }

  if (!transformMatrix.IsIdentity())
    ctxcanvas->graphics->SetTransform(&transformMatrix);
}

static void cdputimagerect(cdCtxCanvas* ctxcanvas, cdCtxImage *ctximage, int x0, int y0, int xmin, int xmax, int ymin, int ymax)
{
  int yr = y0 - (ymax-ymin+1)+1;    /* y0 starts at the bottom of the image */

  INT srcx = xmin;
  INT srcy = (ctximage->h-1)-ymax;
  INT srcwidth = xmax-xmin+1;
  INT srcheight = ymax-ymin+1;
  Rect destRect(x0, yr, srcwidth, srcheight);

  ImageAttributes imageAttributes;

  if (ctxcanvas->use_img_transp)
    imageAttributes.SetColorKey(ctxcanvas->img_transp[0], ctxcanvas->img_transp[1], ColorAdjustTypeBitmap);

  if (ctximage->bpp == 32 && ctximage->alpha)
  {
    sAlpha2Bitmap(*ctximage->bitmap, ctximage->w, ctximage->h, ctximage->alpha, 
                  0, ctximage->w-1, 0, ctximage->h-1, 0);
  }

  if(ctxcanvas->use_img_points)
  {
    Point pts[3];
    pts[0] = ctxcanvas->img_points[0];
    pts[1] = ctxcanvas->img_points[1];
    pts[2] = ctxcanvas->img_points[2];
    if (ctxcanvas->canvas->invert_yaxis)
    {
      pts[0].Y = _cdInvertYAxis(ctxcanvas->canvas, pts[0].Y);
      pts[1].Y = _cdInvertYAxis(ctxcanvas->canvas, pts[1].Y);
      pts[2].Y = _cdInvertYAxis(ctxcanvas->canvas, pts[2].Y);
    }
    ctxcanvas->graphics->DrawImage(ctximage->bitmap, pts, 3, 
                                   srcx, srcy, srcwidth, srcheight, UnitPixel,
                                   &imageAttributes, NULL, NULL);
  }
  else
  {
    ctxcanvas->graphics->DrawImage(ctximage->bitmap, destRect, 
                                   srcx, srcy, srcwidth, srcheight, UnitPixel,
                                   &imageAttributes, NULL, NULL);
  }

  ctxcanvas->dirty = 1;
}

static void  cdkillimage(cdCtxImage *ctximage)
{
  delete ctximage->bitmap;
  delete ctximage;
}

static void cdscrollarea(cdCtxCanvas* ctxcanvas, int xmin, int xmax, int ymin, int ymax, int dx, int dy)
{
  Matrix transformMatrix;
  ctxcanvas->graphics->GetTransform(&transformMatrix);
  if (!transformMatrix.IsIdentity())
    ctxcanvas->graphics->ResetTransform(); // reset to the identity.

  if (ctxcanvas->canvas->invert_yaxis==0)  // if 0, invert because the transform was reset here
  {
    dy = -dy;
    ymin = _cdInvertYAxis(ctxcanvas->canvas, ymin);
    ymax = _cdInvertYAxis(ctxcanvas->canvas, ymax);
    _cdSwapInt(ymin, ymax);
  }

  if (ctxcanvas->wtype == CDW_BMP)
  {
    Rect rect(xmin, ymin, xmax-xmin+1, ymax-ymin+1);

    Bitmap* bitmap = ctxcanvas->bitmap->Clone(rect, ctxcanvas->bitmap->GetPixelFormat());

    rect.Offset(dx, dy);

    ctxcanvas->graphics->DrawImage(bitmap, rect, 
                                   0, 0, rect.Width, rect.Height, UnitPixel,
                                   NULL, NULL, NULL);
    delete bitmap;
  }
  else
  {
    RECT rect;
    rect.left   = xmin;          
    rect.right  = xmax+1;
    rect.top    = ymin;
    rect.bottom = ymax+1; 

    HDC hdc = ctxcanvas->graphics->GetHDC();
    ScrollDC(hdc, dx, dy, &rect, NULL, NULL, NULL);
    ctxcanvas->graphics->ReleaseHDC(hdc);
  }

  ctxcanvas->dirty = 1;

  if (!transformMatrix.IsIdentity())
    ctxcanvas->graphics->SetTransform(&transformMatrix);
}

static void cdflush(cdCtxCanvas* ctxcanvas)
{
  ctxcanvas->graphics->Flush(FlushIntentionSync);
}

/********************************************************************/
/*
%S        Atributos personalizados                          
*/
/********************************************************************/

static void set_img_format_attrib(cdCtxCanvas* ctxcanvas, char* data)
{
  if (!data)
    ctxcanvas->img_format = 0;
  else
  {
    int bpp = 0;
    sscanf(data, "%d", &bpp);
    if (bpp == 0)
      return;

    if (bpp == 32)
      ctxcanvas->img_format = 32;
    else
      ctxcanvas->img_format = 24;
  }
}

static char* get_img_format_attrib(cdCtxCanvas* ctxcanvas)
{
  if (!ctxcanvas->img_format)
    return NULL;

  if (ctxcanvas->img_format == 32)
    return "32";
  else
    return "24";
}

static cdAttribute img_format_attrib =
{
  "IMAGEFORMAT",
  set_img_format_attrib,
  get_img_format_attrib
}; 

static void set_img_alpha_attrib(cdCtxCanvas* ctxcanvas, char* data)
{
  if (!data)
    ctxcanvas->img_alpha = NULL;
  else
    ctxcanvas->img_alpha = (unsigned char*)data;
}

static char* get_img_alpha_attrib(cdCtxCanvas* ctxcanvas)
{
  return (char*)ctxcanvas->img_alpha;
}

static cdAttribute img_alpha_attrib =
{
  "IMAGEALPHA",
  set_img_alpha_attrib,
  get_img_alpha_attrib
}; 

static void set_img_transp_attrib(cdCtxCanvas* ctxcanvas, char* data)
{
  if (!data)
    ctxcanvas->use_img_transp = 0;
  else
  {
    int r1, g1, b1, r2, g2, b2;
    ctxcanvas->use_img_transp = 1;
    sscanf(data, "%d %d %d %d %d %d", &r1, &g1, &b1, &r2, &g2, &b2);
    ctxcanvas->img_transp[0] = Color((BYTE)r1, (BYTE)g1, (BYTE)b1);
    ctxcanvas->img_transp[1] = Color((BYTE)r2, (BYTE)g2, (BYTE)b2);
  }
}


static char* get_img_transp_attrib(cdCtxCanvas* ctxcanvas)
{
  if (!ctxcanvas->use_img_transp)
    return NULL;

  int r1 = ctxcanvas->img_transp[0].GetRed();
  int g1 = ctxcanvas->img_transp[0].GetGreen();
  int b1 = ctxcanvas->img_transp[0].GetBlue();
  int r2 = ctxcanvas->img_transp[1].GetRed();
  int g2 = ctxcanvas->img_transp[1].GetGreen();
  int b2 = ctxcanvas->img_transp[1].GetBlue();

  static char data[50];
  sprintf(data, "%d %d %d %d %d %d", r1, g1, b1, r2, g2, b2);
  return data;
}

static cdAttribute img_transp_attrib =
{
  "IMAGETRANSP",
  set_img_transp_attrib,
  get_img_transp_attrib
}; 

static void set_gradientcolor_attrib(cdCtxCanvas* ctxcanvas, char* data)
{
  // used by CD_FILLGRADIENT
  if (data)
  {
    int r, g, b, cur_vertex;
    sscanf(data, "%d %d %d", &r, &g, &b);

    cur_vertex = ctxcanvas->canvas->poly_n;
    if (cur_vertex < 500)
      ctxcanvas->pathGradient[cur_vertex] = Color((BYTE)r, (BYTE)g, (BYTE)b);
  }
}

static cdAttribute gradientcolor_attrib =
{
  "GRADIENTCOLOR",
  set_gradientcolor_attrib,
  NULL
}; 

static void set_linegradient_attrib(cdCtxCanvas* ctxcanvas, char* data)
{
  if (data)
  {
    int x1, y1, x2, y2;
    sscanf(data, "%d %d %d %d", &x1, &y1, &x2, &y2);

    Point p1 = Point(x1, y1);
    Point p2 = Point(x2, y2);
    ctxcanvas->gradient[0] = p1; 
    ctxcanvas->gradient[1] = p2; 
    if (ctxcanvas->canvas->invert_yaxis)
    {
      p1.Y = _cdInvertYAxis(ctxcanvas->canvas, p1.Y);
      p2.Y = _cdInvertYAxis(ctxcanvas->canvas, p2.Y);
    }
    delete ctxcanvas->fillBrush;
    ctxcanvas->fillBrush = new LinearGradientBrush(p1, p2, ctxcanvas->fg, ctxcanvas->bg);
  }
}

static char* get_linegradient_attrib(cdCtxCanvas* ctxcanvas)
{
  static char data[100];

  sprintf(data, "%d %d %d %d", ctxcanvas->gradient[0].X,
                               ctxcanvas->gradient[0].Y,
                               ctxcanvas->gradient[1].X,
                               ctxcanvas->gradient[1].Y);

  return data;
}

static cdAttribute linegradient_attrib =
{
  "LINEGRADIENT",
  set_linegradient_attrib,
  get_linegradient_attrib
}; 

static void set_linecap_attrib(cdCtxCanvas* ctxcanvas, char* data)
{
  if (data)
  {
    LineCap cap = LineCapFlat;

    switch(data[0])
    {
    case 'T':
      cap = LineCapTriangle;
      ctxcanvas->linePen->SetDashCap(DashCapTriangle);
      break;
    case 'N':
      cap = LineCapNoAnchor;
      break;
    case 'S':
      cap = LineCapSquareAnchor;
      break;
    case 'R':
      cap = LineCapRoundAnchor;
      break;
    case 'D':
      cap = LineCapDiamondAnchor;
      break;
    case 'A':
      cap = LineCapArrowAnchor;
      break;
    default:
      return;
    }

    ctxcanvas->linePen->SetStartCap(cap);
    ctxcanvas->linePen->SetEndCap(cap);
  }
}

static cdAttribute linecap_attrib =
{
  "LINECAP",
  set_linecap_attrib,
  NULL
}; 

static void set_rotate_attrib(cdCtxCanvas* ctxcanvas, char* data)
{
  /* ignore ROTATE if transform is set, 
     because there is native support for transformations */
  if (ctxcanvas->canvas->use_matrix)
    return;

  if (data)
  {
    sscanf(data, "%g %d %d", &ctxcanvas->rotate_angle,
                             &ctxcanvas->rotate_center_x,
                             &ctxcanvas->rotate_center_y);
  }
  else
  {
    ctxcanvas->rotate_angle = 0;
    ctxcanvas->rotate_center_x = 0;
    ctxcanvas->rotate_center_y = 0;
  }

  cdtransform(ctxcanvas, NULL);
}

static char* get_rotate_attrib(cdCtxCanvas* ctxcanvas)
{
  static char data[100];

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

static void set_img_points_attrib(cdCtxCanvas* ctxcanvas, char* data)
{
  int p[6];

  if (!data)
  {
    ctxcanvas->use_img_points = 0;
    return;
  }

  sscanf(data, "%d %d %d %d %d %d", &p[0], &p[1], &p[2], &p[3], &p[4], &p[5]);

  ctxcanvas->img_points[0] = Point(p[0], p[1]);
  ctxcanvas->img_points[1] = Point(p[2], p[3]);
  ctxcanvas->img_points[2] = Point(p[4], p[5]);

  ctxcanvas->use_img_points = 1;
}

static char* get_img_points_attrib(cdCtxCanvas* ctxcanvas)
{
  static char data[100];

  if (!ctxcanvas->use_img_points)
    return NULL;

  sprintf(data, "%d %d %d %d %d %d", ctxcanvas->img_points[0].X,
                                     ctxcanvas->img_points[0].Y,
                                     ctxcanvas->img_points[1].X,
                                     ctxcanvas->img_points[1].Y,
                                     ctxcanvas->img_points[2].X,
                                     ctxcanvas->img_points[2].Y);

  return data;
}

static cdAttribute img_points_attrib =
{
  "IMAGEPOINTS",
  set_img_points_attrib,
  get_img_points_attrib
}; 

static BOOL Is_WinXP_or_WinSrv03(void) 
{
  OSVERSIONINFO osvi;
  ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
  osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
  GetVersionEx (&osvi);

  BOOL bIsWindowsXP = 
    (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT) &&
    ((osvi.dwMajorVersion == 5) && (osvi.dwMinorVersion == 1));

  BOOL bIsWindowsServer2003 = 
    (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT) &&
    ((osvi.dwMajorVersion == 5) && (osvi.dwMinorVersion == 2));

  return bIsWindowsXP || bIsWindowsServer2003;
}

static void set_aa_attrib(cdCtxCanvas* ctxcanvas, char* data)
{
  if (!data || data[0] == '0')
  {
    ctxcanvas->graphics->SetInterpolationMode(InterpolationModeNearestNeighbor);
    ctxcanvas->graphics->SetSmoothingMode(SmoothingModeNone);
    ctxcanvas->graphics->SetTextRenderingHint(TextRenderingHintSingleBitPerPixelGridFit);
    ctxcanvas->antialias = 0;
  }
  else
  {
    ctxcanvas->graphics->SetInterpolationMode(InterpolationModeBilinear);
    ctxcanvas->graphics->SetSmoothingMode(SmoothingModeAntiAlias);
    if (Is_WinXP_or_WinSrv03())
      /* Microsoft Windows XP and Windows Server 2003 only: 
         ClearType rendering is supported only on Windows XP and Windows Server 2003. 
         Therefore, TextRenderingHintClearTypeGridFit is ignored on other operating systems. */
      ctxcanvas->graphics->SetTextRenderingHint(TextRenderingHintClearTypeGridFit);
    else
      ctxcanvas->graphics->SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);

    /* Do NOT set PixelOffsetMode because some graphic objects move their position and size.
       ctxcanvas->graphics->SetPixelOffsetMode(PixelOffsetModeHalf); */

    ctxcanvas->antialias = 1;
  }
}

static char* get_aa_attrib(cdCtxCanvas* ctxcanvas)
{
  if (ctxcanvas->antialias)
    return "1";
  else
    return "0";
}

static cdAttribute aa_attrib =
{
  "ANTIALIAS",
  set_aa_attrib,
  get_aa_attrib
}; 

static void set_window_rgn(cdCtxCanvas* ctxcanvas, char* data)
{
  if (data)
  {
    HRGN hrgn = ctxcanvas->new_region->GetHRGN(ctxcanvas->graphics);
    SetWindowRgn(ctxcanvas->hWnd, hrgn, TRUE);
  }
  else
    SetWindowRgn(ctxcanvas->hWnd, NULL, TRUE);
}

static cdAttribute window_rgn_attrib =
{
  "WINDOWRGN",
  set_window_rgn,
  NULL
}; 

static char* get_hdc_attrib(cdCtxCanvas* ctxcanvas)
{
  return (char*)ctxcanvas->hDC;
}

static cdAttribute hdc_attrib =
{
  "HDC",
  NULL,
  get_hdc_attrib
}; 

static char* get_gdiplus_attrib(cdCtxCanvas* ctxcanvas)
{
  (void)ctxcanvas;
  return "1";
}

static cdAttribute gdiplus_attrib =
{
  "GDI+",
  NULL,
  get_gdiplus_attrib
}; 

void cdwpUpdateCanvas(cdCtxCanvas* ctxcanvas)
{
  if (ctxcanvas->wtype == CDW_EMF && ctxcanvas->metafile) // first update metafile is NULL.
  {
    MetafileHeader metaHeader;
    ctxcanvas->metafile->GetMetafileHeader(&metaHeader);
    ctxcanvas->canvas->xres = metaHeader.GetDpiX() / 25.4;
    ctxcanvas->canvas->yres = metaHeader.GetDpiY() / 25.4;
  }
  else
  {
    ctxcanvas->canvas->xres = ctxcanvas->graphics->GetDpiX() / 25.4;
    ctxcanvas->canvas->yres = ctxcanvas->graphics->GetDpiY() / 25.4;
  }

  ctxcanvas->canvas->w_mm = ((double)ctxcanvas->canvas->w) / ctxcanvas->canvas->xres;
  ctxcanvas->canvas->h_mm = ((double)ctxcanvas->canvas->h) / ctxcanvas->canvas->yres;
  
  ctxcanvas->graphics->SetPageScale(1);
  ctxcanvas->graphics->SetPageUnit(UnitPixel);
  ctxcanvas->graphics->SetCompositingMode(CompositingModeSourceOver);
  ctxcanvas->graphics->SetCompositingQuality(CompositingQualityDefault);

  if (ctxcanvas->antialias)
    set_aa_attrib(ctxcanvas, "1");
  else
    set_aa_attrib(ctxcanvas, NULL);

  sUpdateTransform(ctxcanvas);
}

/* 
%F Cria o canvas para o driver Windows. 
*/
cdCtxCanvas *cdwpCreateCanvas(cdCanvas* canvas, Graphics* graphics, int wtype)
{
  cdCtxCanvas* ctxcanvas = new cdCtxCanvas;
  memset(ctxcanvas, 0, sizeof(cdCtxCanvas));

  /* update canvas context */
  ctxcanvas->canvas = canvas;
  canvas->ctxcanvas = ctxcanvas;

  ctxcanvas->wtype = wtype;
  ctxcanvas->graphics = graphics;

  ctxcanvas->linePen = new Pen(Color());
  ctxcanvas->lineBrush = new SolidBrush(Color());
  ctxcanvas->fillBrush = new SolidBrush(Color());
  ctxcanvas->font = NULL; // will be created in the update default attrib

  set_aa_attrib(ctxcanvas, "1");  // default is ANTIALIAS=1

  ctxcanvas->fg = Color(); // black,opaque
  ctxcanvas->bg = Color(255, 255, 255); // white,opaque => used only for fill

  canvas->invert_yaxis = 1;

  ctxcanvas->clip_poly = NULL;
  ctxcanvas->clip_fpoly = NULL;
  ctxcanvas->clip_poly_n = 0;
  ctxcanvas->clip_region = NULL;
  ctxcanvas->new_region = NULL;
  
  cdRegisterAttribute(canvas, &img_points_attrib);
  cdRegisterAttribute(canvas, &img_transp_attrib);
  cdRegisterAttribute(canvas, &img_format_attrib);
  cdRegisterAttribute(canvas, &img_alpha_attrib);
  cdRegisterAttribute(canvas, &aa_attrib);
  cdRegisterAttribute(canvas, &gradientcolor_attrib);
  cdRegisterAttribute(canvas, &linegradient_attrib);
  cdRegisterAttribute(canvas, &rotate_attrib);
  cdRegisterAttribute(canvas, &linecap_attrib);
  cdRegisterAttribute(canvas, &hdc_attrib);
  cdRegisterAttribute(canvas, &window_rgn_attrib);
  cdRegisterAttribute(canvas, &gdiplus_attrib);

  cdwpUpdateCanvas(ctxcanvas);

  return ctxcanvas;
}

static ULONG_PTR cd_gdiplusToken = NULL;

static void __stdcall DebugEvent(DebugEventLevel level, char* msg)
{
  (void)level;
  MessageBox(NULL, msg, "GDI+ Debug", 0);
}

void cdwpGdiPlusStartup(int debug)
{
  if (cd_gdiplusToken == NULL)
  {
    GdiplusStartupInput input;
    if (debug)
      input.DebugEventCallback = DebugEvent;

    // Initialize GDI+.
    GdiplusStartup(&cd_gdiplusToken, &input, NULL);
  }
}

void cdwpGdiPlusShutdown(void)
{
	if (cd_gdiplusToken)
    GdiplusShutdown(cd_gdiplusToken);
}

void cdwpInitTable(cdCanvas* canvas)
{
  cdCtxCanvas* ctxcanvas = canvas->ctxcanvas;

  canvas->cxFlush = cdflush;
  canvas->cxPixel = cdpixel;

  canvas->cxLine = cdline;
  canvas->cxPoly = cdpoly;
  canvas->cxRect = cdrect;
  canvas->cxBox = cdbox;
  canvas->cxArc = cdarc;
  canvas->cxSector = cdsector;
  canvas->cxChord = cdchord;
  canvas->cxText = cdtext;

  canvas->cxFLine = cdfline;
  canvas->cxFPoly = cdfpoly;
  canvas->cxFRect = cdfrect;
  canvas->cxFBox = cdfbox;
  canvas->cxFArc = cdfarc;
  canvas->cxFSector = cdfsector;
  canvas->cxFChord = cdfchord;

  canvas->cxNewRegion = cdnewregion;
  canvas->cxIsPointInRegion = cdispointinregion;
  canvas->cxOffsetRegion = cdoffsetregion;
  canvas->cxGetRegionBox = cdgetregionbox;

  canvas->cxGetFontDim = cdgetfontdim;
  canvas->cxGetTextSize = cdgettextsize; 
  canvas->cxPutImageRectRGB = cdputimagerectrgb;
  canvas->cxPutImageRectMap = cdputimagerectmap;
  canvas->cxPutImageRectRGBA = cdputimagerectrgba; 

  canvas->cxClip = cdclip;
  canvas->cxClipArea = cdcliparea; 
  canvas->cxBackOpacity = cdbackopacity;
  canvas->cxLineStyle = cdlinestyle;
  canvas->cxLineWidth = cdlinewidth;
  canvas->cxLineCap = cdlinecap;
  canvas->cxLineJoin = cdlinejoin;
  canvas->cxInteriorStyle = cdinteriorstyle;
  canvas->cxHatch = cdhatch;
  canvas->cxStipple = cdstipple; 
  canvas->cxPattern = cdpattern;
  canvas->cxFont = cdfont;
  canvas->cxNativeFont = cdnativefont;
  canvas->cxBackground = cdbackground; 
  canvas->cxForeground = cdforeground;
  canvas->cxPalette = cdpalette; 
  canvas->cxTransform = cdtransform;

  if (ctxcanvas->wtype == CDW_WIN || ctxcanvas->wtype == CDW_BMP)
  {
    canvas->cxClear = cdclear; 

    canvas->cxGetImageRGB = cdgetimagergb; 

    canvas->cxCreateImage = cdcreateimage; 
    canvas->cxGetImage = cdgetimage; 
    canvas->cxPutImageRect = cdputimagerect; 
    canvas->cxKillImage = cdkillimage; 

    canvas->cxScrollArea = cdscrollarea; 
  }
}
