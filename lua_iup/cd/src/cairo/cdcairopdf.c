/** \file
 * \brief Cairo PDF Driver
 *
 * See Copyright Notice in cd.h
 */

#include <stdlib.h>
#include <stdio.h>

#include "cd.h"
#include "cdcairo.h"
#include "cdcairoctx.h"

#include <cairo-pdf.h>


static void cdkillcanvas (cdCtxCanvas *ctxcanvas)
{
  cdcairoKillCanvas(ctxcanvas);
}

static void cdcreatecanvas(cdCanvas* canvas, void* data)
{
  cdCtxCanvas* ctxcanvas;
  char* strdata = (char*)data;
  char filename[10240] = "";
  cairo_surface_t *surface;
  int res = 300;
  double w_pt;         /* Largura do papel (points) */
  double h_pt;         /* Altura do papel (points) */
  double scale;          /* Fator de conversao de coordenadas (pixel2points) */
  int landscape = 0;         /* page orientation */

  /* Starting parameters */
  if (strdata == NULL) 
    return;

  strdata += cdGetFileName(strdata, filename);
  if (filename[0] == 0)
    return;

  cdSetPaperSize(CD_A4, &w_pt, &h_pt);

  while (*strdata != '\0')
  {
    while (*strdata != '\0' && *strdata != '-') 
      strdata++;

    if (*strdata != '\0')
    {
      float num;
      strdata++;
      switch (*strdata++)
      {
      case 'p':
        {
          int paper;
          sscanf(strdata, "%d", &paper);
          cdSetPaperSize(paper, &w_pt, &h_pt);
          break;
        }
      case 'w':
        sscanf(strdata, "%g", &num);
        w_pt = CD_MM2PT*num;
        break;
      case 'h':
        sscanf(strdata, "%g", &num);
        h_pt = CD_MM2PT*num;
        break;
      case 'o':
        landscape = 1;
        break;
      case 's':
        sscanf(strdata, "%d", &res);
        break;
      }
    }

    while (*strdata != '\0' && *strdata != ' ') 
      strdata++;
  }

  if (landscape)
    _cdSwapDouble(w_pt, h_pt);

  scale = 72.0/res;

  canvas->w = (int)(w_pt/scale + 0.5);   /* Converte p/ unidades do usuario */
  canvas->h = (int)(h_pt/scale + 0.5); /* Converte p/ unidades do usuario */
  canvas->w_mm = w_pt/CD_MM2PT;   /* Converte p/ milimetros */
  canvas->h_mm = h_pt/CD_MM2PT; /* Converte p/ milimetros */
  canvas->bpp = 24;
  canvas->xres = canvas->w / canvas->w_mm;
  canvas->yres = canvas->h / canvas->h_mm;

	surface = cairo_pdf_surface_create(filename, w_pt, h_pt);

  /* Starting Cairo driver */
  ctxcanvas = cdcairoCreateCanvas(canvas, cairo_create(surface));
  cairo_surface_destroy(surface);
}

static void cdinittable(cdCanvas* canvas)
{
  cdcairoInitTable(canvas);
  canvas->cxKillCanvas = cdkillcanvas;
}

static cdContext cdCairoPDFContext =
{
  CD_CAP_ALL & ~(CD_CAP_PLAY | CD_CAP_YAXIS | CD_CAP_REGION | CD_CAP_WRITEMODE | CD_CAP_PALETTE),
  0,
  cdcreatecanvas,  
  cdinittable,
  NULL,                 
  NULL
};

cdContext* cdContextCairoPDF(void)
{
  return &cdCairoPDFContext;
}
