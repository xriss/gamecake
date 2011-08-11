/** \file
 * \brief PPlot component for Iup.
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUPPPLOT_H 
#define __IUPPPLOT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Initialize PPlot widget class */
void IupPPlotOpen(void);

/* Create an PPlot widget instance */
Ihandle* IupPPlot(void);

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

/***********************************************/
/*              Deprecated                    */
void IupPPlotBegin(Ihandle *ih, int strXdata);
void IupPPlotAdd(Ihandle *ih, float x, float y);
void IupPPlotAddStr(Ihandle *ih, const char* x, float y);
int IupPPlotEnd(Ihandle *ih);
void IupPPlotInsertStr(Ihandle *ih, int index, int sample_index, const char* x, float y);
void IupPPlotInsert(Ihandle *ih, int index, int sample_index, float x, float y);
void IupPPlotInsertStrPoints(Ihandle* ih, int index, int sample_index, const char** x, float* y, int count);
void IupPPlotInsertPoints(Ihandle* ih, int index, int sample_index, float *x, float *y, int count);
void IupPPlotAddPoints(Ihandle* ih, int index, float *x, float *y, int count);
void IupPPlotAddStrPoints(Ihandle* ih, int index, const char** x, float* y, int count);
void IupPPlotTransform(Ihandle* ih, float x, float y, int *ix, int *iy);
void IupPPlotPaintTo(Ihandle *ih, void *cnv);
/***********************************************/


#ifdef __cplusplus
}
#endif

#endif
