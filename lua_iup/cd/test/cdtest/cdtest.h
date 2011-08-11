/*=========================================================================*/
/* CDTEST.H - 05/12/95.                                                    */
/*=========================================================================*/
#ifndef CDTEST_H
#define CDTEST_H

/*- Constantes: -----------------------------------------------------------*/
#define MAXPOINTS 300

/*- Macros: ---------------------------------------------------------------*/
#define isdigit(_) (((_)>='0') && ((_)<='9'))
#define ignore(_) (void)(_)

/*-------------------------------------------------------------------------*/
/* Tipos enumerados.                                                       */
/*-------------------------------------------------------------------------*/
typedef enum {
  LINE,
  RECT,
  BOX,
  ARC,
  SECTOR,
  CHORD,
  PIXEL,
  MARK,
  TEXT,
  POLY,
  CLIP,
  IMAGE,
  RGB,
  META
} tPrim;

enum {
  BACKGROUND,
  FOREGROUND
};

enum {
  NO_BUFFER,
  IMAGE_BUFFER,
  IMAGERGB_BUFFER
};

typedef enum {
  NEWPOINT,
  MOVE,
  CLOSE,
  CENTER,
  REPAINT
} tRubber;

#if ((!defined(FALSE))&&(!defined(TRUE)))
typedef enum {
  FALSE,
  TRUE
} tBoolean;
#else
#define tBoolean int
#endif

/*-------------------------------------------------------------------------*/
/* Definicao das estruturas de dados usadas.                               */
/*-------------------------------------------------------------------------*/
typedef struct {
  int x, y;
} tPoint;

typedef struct {
  cdContext* ctx;
  char *filename;
} tMeta;

typedef struct {
  int x1;
  int y1;
  int x2;
  int y2;
  int write_mode;
  int line_style;
  int line_width;
  int line_cap;
  int line_join;
  int interior_style;
  int back_opacity;
  int hatch;
  long foreground;
  long background;
} tLB;       /* cdLine ou cdBox ou cdRect */

typedef struct {
  int xc;
  int yc;
  int w;
  int h;
  double angle1;
  double angle2;
  int write_mode;
  int line_style;
  int line_width;
  int line_cap;
  int line_join;
  int interior_style;
  int back_opacity;
  int hatch;
  long foreground;
  long background;
} tAS;       /* cdArc e cdSector e Chord */

typedef struct {
  int x;
  int y;
  int write_mode;
  long foreground;
} tPixel;    /* cdPixel */

typedef struct {
  int x;
  int y;
  int write_mode;
  int mark_type;
  int mark_size;
  long foreground;
} tMark;     /* cdMark */

typedef struct {
  int x;
  int y;
  char *s;
  int write_mode;
  int font_size;
  int font_style;
  int font_typeface;
  int back_opacity;
  double text_orientation;
  int text_alignment;
  long foreground;
  long background;
} tText;     /* cdText */

typedef struct {
  int poly_mode;
  int write_mode;
  int line_style;
  int line_width;
  int line_cap;
  int line_join;
  int fill_mode;
  int back_opacity;
  int interior_style;
  int hatch;
  long foreground;
  long background;
  int num_points;
  tPoint *points;
} tPoly;     /* cdBegin, cdVertex e cdEnd */

typedef struct tnode {
  tPrim type;
  union {
    tLB lineboxpar;
    tAS arcsectorpar;
    tPoly polypar;
    tPixel pixelpar;
    tMark markpar;
    tText textpar;
    tMeta metapar;
  } par;
  struct tnode *next;
} tList;

/*-------------------------------------------------------------------------*/
/* Contexto do CD Test.                                                    */
/*-------------------------------------------------------------------------*/
typedef struct {
  cdCanvas *iup_canvas;      /* canvas do iup */
  int w, h;                  /* largura e altura do canvas */
  double res;
  int bpp;

  cdCanvas *wd_canvas;       /* canvas IUP p/ WD */
  int wd_dialog;             /* se o dialogo do canvas WD estah na tela */
                                  
  cdCanvas *pic_canvas;       /* canvas IUP p/ Picture */
  cdCanvas *picture;          /* Picture */
  int pic_dialog;             /* se o dialogo do canvas Picture estah na tela */
                                  
  cdCanvas *buffer_canvas;   /* canvas para double-buffering */
  int buffering;

  Ihandle *dlg_cur_prim;     /* handle do dialogo de primitiva ativo */
  Ihandle *bt_cur_prim;      /* handle do botao da primitiva corente */

  tPrim cur_prim;            /* primitiva corrente */
  tBoolean following;        /* flag de rubber-band */
  int dlg_x;
  int dlg_y;
  int visible;

  int write_mode;            /* atributos do CD */
  int line_cap;
  int line_join;
  int line_style;
  int line_width;
  int fill_mode;
  int font_typeface;
  int font_style;
  int font_size;
  int text_alignment;
  double text_orientation;
  int back_opacity;
  int mark_type;
  int poly_mode;
  long foreground;
  long background;
  int interior_style;        
  int hatch;

  unsigned char stipple[100];/* sample stipple */
  long pattern[100];         /* sample pattern */
  int dashes[4];             /* sample dash */

  int clip_xmin;
  int clip_xmax;
  int clip_ymin;
  int clip_ymax;
  int clip_mode;

  unsigned char *red;        /* imagem RGB */
  unsigned char *green;
  unsigned char *blue;
  int rgb_w, rgb_h;          /* largura e altura da imagem RGB */

  cdImage *test_image;       /* imagem off-screen para testes */

  int num_points;            /* numero de pontos no poligono corrente */
  tPoint points[MAXPOINTS];  /* armazanamento temporario do poligono */

  char status_line[256];     /* linha de status */
  char title[80];            /* barra de titulo do programa */

  int x, y;                  /* posiccao do mouse no canvas */
  char mouse_pos[40];        /* posiccao do mouse em uma string */

  int sim;                   /* flag para simulacao */
  int stretch_play;

  tList *head;               /* lista de primitivas */
} tCTC;                      /* CD Test Context */

extern tCTC ctgc;

/* parametros geometricos das primitivas */
typedef struct {
  int x1, x2, y1, y2;
} tLinePos;

typedef struct {
  int xmin, xmax, ymin, ymax;
  int x, y;
} tBoxPos;

typedef struct {
  int x, y;
} tPixelPos;

typedef struct {
  int x, y;
  int size;
} tMarkPos;

typedef struct {
  int xc, yc;
  int w, h;
  double angle1, angle2;
} tArcPos;

/*-------------------------------------------------------------------------*/
/* Funccoes do modulo CDTEST.C.                                            */
/*-------------------------------------------------------------------------*/
int fEditUndo(void);
int fEditClear(void);

int fRepaint(void);
int fFileExit(void);
int fOK(void);
int fOpenLines(void);
int fClosedLines(void);
int fPolyBezier(void);
int fFill(void);
int fSolid(void);
int fHatch(void);
int fStipple(void);
int fPattern(void);

int fPolyClip(void);
int fClipPoly(void);

int fWDCanvas(void);
int fCloseWD(void);
int fWDRepaint(void);

int fPICCanvas(void);
int fClosePIC(void);
int fPICRepaint(void);

int fOpacity(Ihandle *, char *, int, int);
int fMarkType(Ihandle *, char *, int, int);

int fNoBuffering(Ihandle *, int);
int fImageBuffer(Ihandle *, int);
int fRGBBuffer(Ihandle *, int);

int fWriteMode(Ihandle *, char *, int, int);
int fLineStyle(Ihandle *, char *, int, int);
int fLineCap(Ihandle *, char *, int, int);
int fLineJoin(Ihandle *, char *, int, int);
int fFillMode(Ihandle *, char *, int, int);
int fFontStyle(Ihandle *, char *, int, int);
int fFontTypeFace(Ihandle *, char *, int, int);
int fTextAlignment(Ihandle *, char *, int, int);
int fHatchStyle(Ihandle *, char *, int, int);

int fColor(Ihandle *);

int fClip(Ihandle *);
int fClipArea(void);
int fClipOff(void);

int fImage(Ihandle *);
int fImagePut(void);
int fImageGet(void);

int fImageRGB(Ihandle *);
int fImageRGBPut(void);
int fImageRGBGet(void);

int fLine(Ihandle *);
int fRect(Ihandle *);
int fBox(Ihandle *);
int fArc(Ihandle *);
int fSector(Ihandle *);
int fChord(Ihandle *);
int fPixel(Ihandle *);
int fMark(Ihandle *);
int fText(Ihandle *);
int fPoly(Ihandle *);

int fShowDialog(void);

int fStretchPlay(Ihandle*, int);
int fSimulate(Ihandle *, int);
int fOptionsHide(void);
int fOptions(void);
int fAttributes(void);
int fAttributesHide(void);
int fMsgHide(void);

int fInteger(Ihandle *, int);
int fReal(Ihandle *, int);

int fDraw(void);

int fHelpAbout(void);
int fCloseAbout(void);

int fMotionCB(Ihandle *, int, int, char *);
int fButtonCB(Ihandle *, int, int, int, int, char *);
int fResizeCB(Ihandle *, int, int);
int fGetFocusCB(Ihandle *);

void set_status(void);
void mouse_pos(int, int);
void putlist(cdCanvas *target);
void draw(void);

/*-------------------------------------------------------------------------*/
/* Funccoes do modulo RUBBER.C.                                            */
/*-------------------------------------------------------------------------*/
void follow(int, int);
void line(tRubber, int, int);
void box(tRubber, int, int);
void arc(tRubber, int, int);
void polygon(tRubber, int, int);

/*-------------------------------------------------------------------------*/
/* Funccoes do modulo LIST.C.                                              */
/*-------------------------------------------------------------------------*/
int newpolypoint(int, int);
int newline(int, int, int, int);
int newrect(int, int, int, int);
int newbox(int, int, int, int);
int newarc(int, int, int, int, double, double);
int newsector(int, int, int, int, double, double);
int newchord(int, int, int, int, double, double);
int newpixel(int, int);
int newmark(int, int, int);
int newtext(int, int, char *);
int newmetafile(char *, cdContext* ctx);
int newpoly(void);
void dellist(void);
void dellast(void);

/*-------------------------------------------------------------------------*/
/* Funccoes do modulo DRIVERS.C.                                           */
/*-------------------------------------------------------------------------*/
void DriversInit(void);

/*-------------------------------------------------------------------------*/
/* Funccoes do modulo COLORBAR.C.                                          */
/*-------------------------------------------------------------------------*/
int ColorBarInit(Ihandle *parent, Ihandle *canvas, long *foreground, long *background);
void ColorBarClose(void);

#endif
