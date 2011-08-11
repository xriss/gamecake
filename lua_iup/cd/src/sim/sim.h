/** \file
 * \brief Simulation Base Driver
 *
 * See Copyright Notice in cd.h
 */

#ifndef __SIM_H
#define __SIM_H


struct _cdSimulation
{
  cdTT_Text* tt_text; /* TrueType Font Simulation using FreeType library */

  int antialias;

  cdCanvas *canvas;

  const char* font_map[100];
  int font_map_n;

  /* horizontal line draw functions */
  void (*SolidLine)(cdCanvas* canvas, int xmin, int y, int xmax, long color);
  void (*PatternLine)(cdCanvas* canvas, int xmin, int xmax, int y, int pw, const long *pattern);
  void (*StippleLine)(cdCanvas* canvas, int xmin, int xmax, int y, int pw, const unsigned char *stipple);
  void (*HatchLine)(cdCanvas* canvas, int xmin, int xmax, int y, unsigned char hatch);
};

#define simRotateHatchN(_x,_n) ((_x) = ((_x) << (_n)) | ((_x) >> (8-(_n))))

void simFillDrawAAPixel(cdCanvas *canvas, int x, int y, unsigned short alpha_weigth);
void simFillHorizLine(cdSimulation* simulation, int xmin, int y, int xmax);
void simFillHorizBox(cdSimulation* simulation, int xmin, int xmax, int ymin, int ymax);
void simGetPenPos(cdCanvas* canvas, int x, int y, const char* s, int len, FT_Matrix *matrix, FT_Vector *pen);
int simIsPointInPolyWind(cdPoint* poly, int n, int x, int y);

/* list of non-horizontal line segments */
typedef struct _simLineSegment
{
  int x1, y1;   /* always y1 < y2 */
  int x2, y2;   /* (x2,y2) is not included in the segment to avoid duplicated intersections */
  int x;        /* incremental x from x2 to x1 */
  int DeltaX, DeltaY, XDir, Swap;
  unsigned short ErrorInc, ErrorAcc;
} simLineSegment;

int simAddSegment(simLineSegment* segment, int x1, int y1, int x2, int y2, int *y_max, int *y_min);
int simSegmentInc(simLineSegment* segment);

int simPolyFindHorizontalIntervals(simLineSegment *segments, int n_seg, int* xx, int *hh, int y, int height);
void simPolyMakeSegments(simLineSegment *segments, int *n_seg, cdPoint* poly, int n, int *max_hh, int *y_max, int *y_min);

void simPolyFill(cdSimulation* simulation, cdPoint* poly, int n);
void simLineThin(cdCanvas* canvas, int x1, int y1, int x2, int y2);
void simLineThick(cdCanvas* canvas, int x1, int y1, int x2, int y2);
void simfLineThick(cdCanvas* canvas, double x1, double y1, double x2, double y2);
void simfLineThin(cdCanvas* canvas, double x1, double y1, double x2, double y2, int *last_xi_a, int *last_yi_a, int *last_xi_b, int *last_yi_b);
extern int simLineStyleNoReset;

#endif

