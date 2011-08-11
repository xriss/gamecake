/* ************************************************************************ */
/* Header file for the `xvertext 5.0' routines.

   Copyright (c) 1993 Alan Richardson (mppa3@uk.ac.sussex.syma) */
/* ************************************************************************ */

#ifndef __XVERTEXT_H 
#define __XVERTEXT_H

#ifdef __cplusplus
extern "C" {
#endif 

#define XV_VERSION      5.0
#define XV_COPYRIGHT \
      "xvertext routines Copyright (c) 1993 Alan Richardson"

/* text alignment */
enum {XR_LEFT, XR_CENTRE, XR_RIGHT, XR_TLEFT, XR_TCENTRE, XR_TRIGHT, XR_MLEFT, XR_MCENTRE, XR_MRIGHT, XR_BLEFT, XR_BCENTRE, XR_BRIGHT};

double   XRotVersion(char* str, int n);
void    XRotSetMagnification(double m);
void    XRotSetBoundingBoxPad(int p);
XPoint *XRotTextExtents(Display* dpy, XFontStruct* font, double angle, int x, int y, const char* text, int len, int align);
int     XRotDrawString(Display* dpy, XFontStruct* font, double angle, Drawable drawable, GC gc, int x, int y, const char* text, int len, int align, int bg);

#ifdef __cplusplus
}
#endif 

#endif /* _XVERTEXT_INCLUDED_ */
