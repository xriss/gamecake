/** \file
 * \brief Cairo IMAGERGB Driver
 *
 * See Copyright Notice in cd.h
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cd.h"
#include "cdcairo.h"
#include "cdcairoctx.h"


static char* get_stride_attrib(cdCtxCanvas* ctxcanvas)
{
  static char data[100];
  sprintf(data, "%d", cairo_image_surface_get_stride(cairo_get_target(ctxcanvas->cr)));
  return data;
}

static cdAttribute stride_attrib =
{
  "STRIDE",
  NULL,
  get_stride_attrib
}; 

static void set_write2png_attrib(cdCtxCanvas *ctxcanvas, char* data)
{
  if (data)
    cairo_surface_write_to_png(cairo_get_target(ctxcanvas->cr), data);
}

static cdAttribute write2png_attrib =
{
  "WRITE2PNG",
  set_write2png_attrib,
  NULL
}; 

static char* get_data_attrib(cdCtxCanvas* ctxcanvas)
{
  return (char*)ctxcanvas->rgb;
}

static cdAttribute data_attrib =
{
  "RGBDATA",
  NULL,
  get_data_attrib
}; 

static void cdkillcanvas (cdCtxCanvas *ctxcanvas)
{
  if (!ctxcanvas->user_image)
    free(ctxcanvas->rgb);

  cdcairoKillCanvas(ctxcanvas);
}

static void cdcreatecanvas(cdCanvas* canvas, void* data)
{
  cdCtxCanvas* ctxcanvas;
  cairo_surface_t *surface;
  int w = 0, h = 0, use_alpha = 0;
  float res = (float)3.78;
  unsigned char *rgb = NULL;
  char* str_data = (char*)data;
  char* res_ptr = NULL;
  cairo_format_t format = CAIRO_FORMAT_RGB24;

  /* Starting parameters */
  if (str_data == NULL) 
    return;

  if (strstr(str_data, "-a"))
    use_alpha = 1;

  res_ptr = strstr(str_data, "-r");
  if (res_ptr)
    sscanf(res_ptr+2, "%g", &res);

  /* size and rgb */
#ifdef SunOS_OLD
  sscanf(str_data, "%dx%d %d", &w, &h, &rgb);
#else
  sscanf(str_data, "%dx%d %p", &w, &h, &rgb);
#endif

  if (w == 0 || h == 0)
    return;

  canvas->w = w;
  canvas->h = h;
  canvas->yres = res;
  canvas->xres = res;
  canvas->w_mm = ((double)w) / res;
  canvas->h_mm = ((double)h) / res;
  if (use_alpha)
  {
    canvas->bpp = 32;
    format = CAIRO_FORMAT_ARGB32;
  }
  else
    canvas->bpp = 24;  /* fake value, image bpp is always 32 */

  if (rgb)
    surface = cairo_image_surface_create_for_data(rgb, format, w, h, w*32);
  else
  	surface = cairo_image_surface_create(format, canvas->w, canvas->h);

  /* Starting Cairo driver */
  ctxcanvas = cdcairoCreateCanvas(canvas, cairo_create(surface));
  cairo_surface_destroy(surface);

  if (rgb)
  {
    ctxcanvas->user_image = 1;
    ctxcanvas->rgb = rgb;
  }
  else
  {
    ctxcanvas->user_image = 0;
    ctxcanvas->rgb = cairo_image_surface_get_data(cairo_get_target(ctxcanvas->cr));

    /* fill with white */
    /* transparent, this is the normal alpha coding */
    cairo_set_source_rgba(ctxcanvas->cr, 1.0, 1.0, 1.0, 0.0);
    cairo_rectangle(ctxcanvas->cr, 0, 0, canvas->w, canvas->h);
    cairo_fill(ctxcanvas->cr);
  }
                                      
  cdRegisterAttribute(canvas, &stride_attrib);
  cdRegisterAttribute(canvas, &write2png_attrib);
  cdRegisterAttribute(canvas, &data_attrib);
}

static void cdinittable(cdCanvas* canvas)
{
  cdcairoInitTable(canvas);
  canvas->cxKillCanvas = cdkillcanvas;
}

static cdContext cdCairoImageRGBContext =
{
  CD_CAP_ALL & ~(CD_CAP_PLAY | CD_CAP_YAXIS | CD_CAP_REGION | CD_CAP_WRITEMODE | CD_CAP_PALETTE),
  0,
  cdcreatecanvas,  
  cdinittable,
  NULL,                 
  NULL
};

cdContext* cdContextCairoImageRGB(void)
{
  return &cdCairoImageRGBContext;
}
