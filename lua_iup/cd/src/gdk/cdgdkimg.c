/** \file
 * \brief Gdk Image Driver
 *
 * See Copyright Notice in cd.h
 */

#include <stdlib.h>

#include "cdgdk.h"
#include "cdimage.h"


static void cdkillcanvas(cdCtxCanvas* ctxcanvas)
{
  cdgdkKillCanvas(ctxcanvas);
}

static void cdcreatecanvas(cdCanvas* canvas, void *data)
{
  cdCtxImage *ctximage = ((cdImage*)data)->ctximage;
  cdgdkCreateCanvas(canvas, (GdkDrawable*)ctximage->img, ctximage->scr, ctximage->vis);
}

static void cdinittable(cdCanvas* canvas)
{
  cdgdkInitTable(canvas);

  canvas->cxKillCanvas = cdkillcanvas;
}

static cdContext cdImageContext =
{
  CD_CAP_ALL & ~(CD_CAP_PLAY | CD_CAP_YAXIS | CD_CAP_FPRIMTIVES | CD_CAP_PATH | CD_CAP_BEZIER ),
  0,
  cdcreatecanvas,
  cdinittable,
  NULL,
  NULL
};


cdContext* cdContextImage(void)
{
  if (cdUseContextPlus(CD_QUERY))
  {
    cdContext* ctx = cdGetContextPlus(CD_CTX_IMAGE);
    if (ctx != NULL)
      return ctx;
  }

  return &cdImageContext;
}
