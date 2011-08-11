/** \file
 * \brief Plot component for Iup.
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUPPLOT_H 
#define __IUPPLOT_H

#ifdef __cplusplus
extern "C" {
#endif

void IupPLPlotOpen(void);

Ihandle* IupPLPlot(void);

#ifndef IUP_PLOT_API
#define IUP_PLOT_API
void IupPlotBegin(Ihandle *ih, int strXdata);
void IupPlotAdd(Ihandle *ih, float x, float y);
void IupPlotAddStr(Ihandle *ih, const char* x, float y);
int  IupPlotEnd(Ihandle *ih);
void IupPlotInsertStr(Ihandle *ih, int index, int sample_index, const char* x, float y);
void IupPlotInsert(Ihandle *ih, int index, int sample_index, float x, float y);
void IupPlotInsertStrPoints(Ihandle* ih, int index, int sample_index, const char** x, float* y, int count);
void IupPlotInsertPoints(Ihandle* ih, int index, int sample_index, float *x, float *y, int count);
void IupPlotAddPoints(Ihandle* ih, int index, float *x, float *y, int count);
void IupPlotAddStrPoints(Ihandle* ih, int index, const char** x, float* y, int count);
void IupPlotTransform(Ihandle* ih, float x, float y, int *ix, int *iy);
void IupPlotPaintTo(Ihandle *ih, void *cnv);
#endif


#ifdef __cplusplus
}
#endif

#endif
