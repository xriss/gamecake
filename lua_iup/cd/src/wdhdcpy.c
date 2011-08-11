/** \file
 * \brief WD Hardcopy Client function
 *
 * See Copyright Notice in cd.h
 */

#include <stdlib.h>
#include <stdio.h>

#include "cd.h"
#include "wd.h"

/* from cd_private.h */
int cdRound(double x);

/* re-declared here to ignore CD_NO_OLD_INTERFACE definition */ 
int cdActivate(cdCanvas* canvas);
cdCanvas* cdActiveCanvas(void);

/*
**   ---------------------------------------------------------------
**   Private functions:
*/

static void _wdHdcpyDoit(cdCanvas *canvas, cdCanvas *canvas_copy, void (*draw_func)(cdCanvas *canvas_copy))
{
  cdCanvas *old_active;
  double  left, right, bottom, top;   /* canvas visualization window       */
  int     canvas_hsize, canvas_vsize; /* canvas sizes in pixels            */
  int     hdcpy_hsize, hdcpy_vsize;   /* paper sizes in points             */
  double  canvas_vpr;                 /* canvas viewport distortion ratio  */
  double  hdcpy_vpr;                  /* paper viewport distortion ratio   */
  int     xc, yc;                     /* paper center in pixels            */
  int     xmin, xmax, ymin, ymax;     /* paper viewport                    */

  /* Activate canvas visualization surface. */
  if (cdCanvasActivate(canvas) != CD_OK) return;

  /* Get current canvas window parameters and sizes. */
  wdCanvasGetWindow(canvas, &left, &right, &bottom, &top);
  cdCanvasGetSize(canvas, &canvas_hsize, &canvas_vsize, 0L, 0L);

  /* Activate hardcopy visualization surface. */
  if (cdCanvasActivate(canvas_copy) != CD_OK) return;

  /* Set window parameters on hardcopy surface. */
  wdCanvasWindow(canvas_copy, left, right, bottom, top);

  /* Adjust paper viewport, centralized, matching canvas viewport. */
  canvas_vpr = (double)canvas_vsize / (double)canvas_hsize;
  cdCanvasGetSize(canvas_copy, &hdcpy_hsize, &hdcpy_vsize, 0L, 0L);
  hdcpy_vpr = (double)hdcpy_vsize / (double)hdcpy_hsize;
  xc = (int)((double)hdcpy_hsize/2.0);
  yc = (int)((double)hdcpy_vsize/2.0);

  if (canvas_vpr < hdcpy_vpr)
  {
    xmin = 0;
    xmax = hdcpy_hsize;
    ymin = yc - (int)((double)hdcpy_hsize*(double)canvas_vpr/2.0);
    ymax = yc + (int)((double)hdcpy_hsize*(double)canvas_vpr/2.0);
  }
  else
  {
    xmin = xc - (int)((double)hdcpy_vsize/(double)canvas_vpr/2.0);
    xmax = xc + (int)((double)hdcpy_vsize/(double)canvas_vpr/2.0);
    ymin = 0;
    ymax = hdcpy_vsize;
  }

  cdCanvasClipArea(canvas_copy, xmin, xmax, ymin, ymax);
  cdCanvasClip(canvas_copy, CD_CLIPAREA);
  wdCanvasViewport(canvas_copy, xmin, xmax, ymin, ymax);

  /* for backward compatibility */
  old_active = cdActiveCanvas();
  cdActivate(canvas_copy);

  /* Draw on hardcopy surface.  */
  draw_func(canvas_copy);

  if (old_active) cdActivate(old_active);
}

/*
**   ---------------------------------------------------------------
**   Entry points begin here:
*/

void wdCanvasHardcopy(cdCanvas *canvas, cdContext* ctx, void *data, void(*draw_func)(cdCanvas *canvas_copy))
{
  /* Create a visualization surface. */
  cdCanvas *canvas_copy = cdCreateCanvas(ctx, data);
  if (!canvas_copy) return;

  /* Do hardcopy. */
  _wdHdcpyDoit(canvas, canvas_copy, draw_func);

  /* Destroy visualization surface. */
  cdKillCanvas(canvas_copy);
}
