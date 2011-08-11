/** \file
 * \brief Windows Base Driver
 *
 * See Copyright Notice in cd.h
 */

#ifndef __CDWIN_H
#define __CDWIN_H

#include <windows.h>
#include "cd.h"
#include "cd_private.h"

#ifdef __cplusplus
extern "C" {
#endif
  

  
/* Contexto de cada imagem no servidor */
struct _cdCtxImage 
{
  HDC      hDC;        /* handle para o contexto de imagem na memoria */
  HBITMAP  hBitmap;    /* handle para o bitmap associado              */
  HBITMAP  hOldBitmap; /* handle para o bitmap associado inicialmente */
  int      w;          /* largura da imagem                           */
  int      h;          /* altura da imagem                            */
  double w_mm, h_mm;   /* size in mm                                  */                  
  double xres, yres;   /* resolution in pixels/mm                     */     
  int bpp;

  void* rgba_dib;       /* used by 32 bpp to set alpha before putimage */
  unsigned char* alpha; /* the alpha values must be stored here */
}; 

/* Contexto de cada canvas (CanvasContext). */
struct _cdCtxCanvas 
{
  cdCanvas* canvas;
  
  HWND     hWnd;        /* handle para janela                           */
  HDC      hDC;         /* contexto gr'afico para janela                */
  int release_dc;
  
  COLORREF fg, bg;      /* foreground, backgound                        */
  
  LOGPEN   logPen;      /* pena logica - struct com tipo, cor,...       */
  HPEN     hPen;        /* handle para a pena corrente                  */
  HPEN     hNullPen;    /* handle da pena que nao desenha nada          */
  HPEN     hOldPen;     /* pena anterior  selecionado                   */
  int      rebuild_pen;
  
  LOGBRUSH logBrush;    /* pincel l'ogico - struct com tipo, cor,...    */ 
  HBRUSH   hBrush;      /* handle para o pincel corrente                */
  HBRUSH   hOldBrush;   /* brush anterior selecionado                   */
  HBRUSH   hBkBrush;    /* handle para o pincel com a cor de fundo      */
  
  HDC      hDCMemPat;
  HBITMAP  hOldBitmapPat,hBitmapPat;
  
  HDC      hDCMemStip;
  HBITMAP  hOldBitmapStip,hBitmapStip;
  
  HFONT    hFont;          /* handle para o fonte corrente              */
  HFONT    hOldFont;
  
  int font_angle;

  float  rotate_angle;
  int    rotate_center_x,
         rotate_center_y;
  
  struct 
  {
    int max_width;     
    int height;        
    int ascent;        
    int descent;  
  } font;
  
  POINT *clip_pnt;   /* coordenadas do pixel no X,Y                  */
  int clip_pnt_n;    /* numero de pontos correntes                   */
  HRGN clip_hrgn;

  HRGN new_rgn;

  HPALETTE hPal, hOldPal;           /* handle para a paleta corrente             */
  LOGPALETTE* pLogPal;     /* paleta logica do canvas                   */
  
  char *filename;          /* Nome do arquivo para WMF */
  int wtype;               /* Flag indicando qual o tipo de superficie */ 
  
  HBITMAP hBitmapClip, hOldBitmapClip;   /* Bitmap para copiar para clipboard */
  BITMAPINFO bmiClip;
  BYTE* bitsClip;
  DWORD RopBlt;         /* Raster Operation for bitmaps */
  int isOwnedDC;        /* usado pelo Native canvas */
  
  BYTE* dib_bits;
  int bits_size;
  
  cdImage* image_dbuffer; /* utilizado pelo driver de Double buffer */
  cdCanvas* canvas_dbuffer;

  HBITMAP img_mask;  /* used by PutImage with mask and rotation and transparency */

  POINT img_points[3];
  int use_img_points;

  char fill_attrib[2];

  int img_format;
  unsigned char* img_alpha;
};

enum{CDW_WIN, CDW_BMP, CDW_WMF, CDW_EMF};

/* Cria um canvas no driver Windows e inicializa valores default */
cdCtxCanvas *cdwCreateCanvas(cdCanvas* canvas, HWND hWnd, HDC hDC, int wtype);
void cdwInitTable(cdCanvas* canvas);
void cdwRestoreDC(cdCtxCanvas *ctxcanvas);

/* Remove valores comuns do driver Windows, deve ser chamado por todos os drivers */
void cdwKillCanvas(cdCtxCanvas* canvas);


/* implemented in the wmfmeta.c module */
void wmfMakePlaceableMetafile(HMETAFILE hmf, const char* filename, int w, int h);
void wmfWritePlacebleFile(HANDLE hFile, unsigned char* buffer, DWORD dwSize, LONG mm, LONG xExt, LONG yExt);

/* implemented in the wmf_emf.c module */
int cdplayWMF(cdCanvas* canvas, int xmin, int xmax, int ymin, int ymax, void *data);
int cdregistercallbackWMF(int cb, cdCallback func);
int cdplayEMF(cdCanvas* canvas, int xmin, int xmax, int ymin, int ymax, void *data);
int cdregistercallbackEMF(int cb, cdCallback func);

/* Estrutura que descreve um DIB. The secondary members are pointers to the main dib pointer. */
typedef struct _cdwDIB 
{
  BYTE*             dib;         /* The DIB as it is defined */
  BITMAPINFO*       bmi;         /* Bitmap Info = Bitmap Info Header + Palette*/
  BITMAPINFOHEADER* bmih;        /* Bitmap Info Header */
  RGBQUAD*          bmic;        /* Palette */
  BYTE*             bits;	       /* Bitmap Bits */
  int               w;
  int               h;    
  int               type;        /* RGB = 0 or MAP = 1 or RGBA = 2 (dib section only) */
} cdwDIB; 

enum {CDW_RGB, CDW_MAP, CDW_RGBA};

int cdwCreateDIB(cdwDIB* dib);
void cdwKillDIB(cdwDIB* dib);

HANDLE cdwCreateCopyHDIB(BITMAPINFO* bmi, BYTE* bits);
void cdwDIBReference(cdwDIB* dib, BYTE* bmi, BYTE* bits);
int cdwCreateDIBRefBuffer(cdwDIB* dib, unsigned char* *bits, int *size);
void cdwCreateDIBRefBits(cdwDIB* dib, unsigned char *bits);
HBITMAP cdwCreateDIBSection(cdwDIB* dib, HDC hDC);

HPALETTE cdwDIBLogicalPalette(cdwDIB* dib);

/* copy from DIB */
void cdwDIBDecodeRGB(cdwDIB* dib, unsigned char *red, unsigned char *green, unsigned char *blue);
void cdwDIBDecodeMap(cdwDIB* dib, unsigned char *index, long *colors);

/* copy to DIB */
void cdwDIBEncodePattern(cdwDIB* dib, const long int *colors);
void cdwDIBEncodeMapRect(cdwDIB* dib, const unsigned char *index, const long int *colors, int xi, int yi, int wi, int hi);
void cdwDIBEncodeRGBRect(cdwDIB* dib, const unsigned char *red, const unsigned char *green, const unsigned char *blue, int xi, int yi, int wi, int hi);
void cdwDIBEncodeRGBARect(cdwDIB* dib, const unsigned char *red, const unsigned char *green, const unsigned char *blue, const unsigned char *alpha, int xi, int yi, int wi, int hi);
void cdwDIBEncodeRGBARectMirror(cdwDIB* dib, const unsigned char *red, const unsigned char *green, const unsigned char *blue, const unsigned char *alpha, int xi, int yi, int wi, int hi);
void cdwDIBEncodeRGBARectZoom(cdwDIB* dib, const unsigned char *red, const unsigned char *green, const unsigned char *blue, const unsigned char *alpha, int w, int h, int xi, int yi, int wi, int hi);
void cdwDIBEncodeAlphaRect(cdwDIB* dib, const unsigned char *alpha, int xi, int yi, int wi, int hi);
  

#ifdef __cplusplus
}
#endif

#endif /* ifndef CDWIN_H */

