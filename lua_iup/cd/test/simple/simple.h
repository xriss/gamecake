
#ifndef __Simple_H
#define __Simple_H

#include <cd.h>

void SimpleCreateCanvas(char* data);
void SimpleKillCanvas(void);

void SimpleUpdateSize(cdCanvas* cnv);
void SimpleFlush(void);

int SimplePlayClipboard(void);
int SimplePlayCGMBin(void);
int SimplePlayCGMText(void);
int SimplePlayMetafile(void);
int SimplePlayWMF(void);
int SimplePlayEMF(void);

int SimpleDrawDebug(void);
int SimpleDrawWindow(void);
int SimpleDrawCGMText(void);
int SimpleDrawCGMBin(void);
int SimpleDrawDXF(void);
int SimpleDrawDGN(void);
int SimpleDrawEMF(void);
int SimpleDrawMetafile(void);
int SimpleDrawPDF(void);
int SimpleDrawPS(void);
int SimpleDrawEPS(void);
int SimpleDrawSVG(void);
int SimpleDrawWMF(void);
int SimpleDrawPrint(void);
int SimpleDrawPrintDialog(void);
int SimpleDrawClipboardBitmap(void);
int SimpleDrawClipboardMetafile(void);
int SimpleDrawClipboardEMF(void);
int SimpleDrawImage(void);
int SimpleDrawImageRGB(void);
int SimpleDrawSimulate(void);
int SimpleDrawGL(void);

int SimpleNotXor(void);
int SimpleXor(void);
int SimpleReplace(void);
int SimpleClippingOff(void);
int SimpleClippingArea(void);
int SimpleClippingPolygon(void);
int SimpleClippingRegion(void);

int SimpleTransform(void);
int SimpleContextPlus(void);
int SimpleAll(void);
int SimpleTextAlign(void);
int SimpleTextFonts(void);
int SimpleTest(void);
int SimpleRepaint(void);

#endif
