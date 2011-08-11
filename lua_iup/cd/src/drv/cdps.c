/** \file
 * \brief PS driver
 *
 * See Copyright Notice in cd.h
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <locale.h>

#include "cd.h"
#include "cd_private.h"
#include "cdps.h"


#define mm2pt(x) (CD_MM2PT*(x))

#ifndef min
#define min(a, b) ((a)<(b)?(a):(b))
#endif
#ifndef max
#define max(a, b) ((a)>(b)?(a):(b))
#endif

/*
** dada uma cor do CD, obtem uma de suas componentes, na faixa 0-1.
*/
#define get_red(_)   (((double)cdRed(_))/255.)
#define get_green(_) (((double)cdGreen(_))/255.)
#define get_blue(_)  (((double)cdBlue(_))/255.)

/* ATENTION: currentmatrix/setmatrix
   Remeber that there is a tranformation set just after file open, to define margins and pixel scale.
   So use transformations carefully.
*/

static unsigned char HatchBits[6][8] = {            /* [style][y] (8x8) */
     {0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00}, /* CD_HORIZONTAL */
     {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10}, /* CD_VERTICAL   */
     {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80}, /* CD_BDIAGONAL  */
     {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01}, /* CD_FDIAGONAL  */
     {0x10, 0x10, 0x10, 0xFF, 0x10, 0x10, 0x10, 0x10}, /* CD_CROSS      */
     {0x81, 0x42, 0x24, 0x18, 0x18, 0x24, 0x42, 0x81}};/* CD_DIAGCROSS  */


struct _cdCtxCanvas 
{
  cdCanvas* canvas;

  FILE *file;            /* Arquivo PS */
  int res;               /* Resolucao */
  int pages;             /* Numero total de paginas */
  double width_pt;       /* Largura do papel (points) */
  double height_pt;      /* Altura do papel (points) */
  double xmin, ymin;     /* Definem as margens esquerda e inferior (points) */
  double xmax, ymax;     /* Definem as margens direita e superior (points) */
  double bbxmin, bbymin; /* Definem a bounding box */
  double bbxmax, bbymax; /* Definem a bounding box */
  double bbmargin;       /* Define a margem para a bounding box */
  double scale;          /* Fator de conversao de coordenadas (pixel2points) */
  int eps;               /* Postscrip encapsulado? */
  int level1;            /* if true generates level 1 only function calls */
  int landscape;         /* page orientation */
  int debug;             /* print debug strings in the file */
  char* old_locale;

  float  rotate_angle;
  int    rotate_center_x,
         rotate_center_y;

  char *nativefontname[100]; /* Registra as fontes usadas */
  int num_native_font;

  int poly_holes[500];
  int holes;
};

/*
%F Registra os valores default para impressao.
*/
static void setpsdefaultvalues(cdCtxCanvas *ctxcanvas)
{
  /* all the other values are set to 0 */
  cdSetPaperSize(CD_A4, &ctxcanvas->width_pt, &ctxcanvas->height_pt);

  ctxcanvas->xmin = 25.4; /* ainda em mm, sera' convertido para points na init_ps */
  ctxcanvas->xmax = 25.4;
  ctxcanvas->ymin = 25.4;
  ctxcanvas->ymax = 25.4;
  ctxcanvas->res = 300;
}

/*
%F Insere o ponto (x, y) na BoundingBox corrente.
Nao leva em consideracao a espessura das linhas.
*/
static void insertpoint(cdCtxCanvas *ctxcanvas, int x, int y)
{
  double xmin = x*ctxcanvas->scale + ctxcanvas->xmin - ctxcanvas->bbmargin;
  double ymin = y*ctxcanvas->scale + ctxcanvas->ymin - ctxcanvas->bbmargin;

  double xmax = x*ctxcanvas->scale + ctxcanvas->xmin + ctxcanvas->bbmargin;
  double ymax = y*ctxcanvas->scale + ctxcanvas->ymin + ctxcanvas->bbmargin;

  if (!ctxcanvas->bbxmin && !ctxcanvas->bbxmax && !ctxcanvas->bbymin && !ctxcanvas->bbymax)
  {
    ctxcanvas->bbxmin = xmin;
    ctxcanvas->bbymin = ymin;

    ctxcanvas->bbxmax = xmax;
    ctxcanvas->bbymax = ymax;
  }
  else
  {
    if (ctxcanvas->canvas->clip_mode == CD_CLIPAREA)
    {
      ctxcanvas->bbxmin = max(ctxcanvas->canvas->clip_rect.xmin*ctxcanvas->scale + ctxcanvas->xmin, min(ctxcanvas->bbxmin, xmin));
      ctxcanvas->bbymin = max(ctxcanvas->canvas->clip_rect.ymin*ctxcanvas->scale + ctxcanvas->ymin, min(ctxcanvas->bbymin, ymin));

      ctxcanvas->bbxmax = min(ctxcanvas->canvas->clip_rect.xmax*ctxcanvas->scale + ctxcanvas->xmax, max(ctxcanvas->bbxmax, xmax));
      ctxcanvas->bbymax = min(ctxcanvas->canvas->clip_rect.ymax*ctxcanvas->scale + ctxcanvas->ymax, max(ctxcanvas->bbymax, ymax));
    }
    else
    {
      ctxcanvas->bbxmin = max(ctxcanvas->xmin, min(ctxcanvas->bbxmin, xmin));
      ctxcanvas->bbymin = max(ctxcanvas->ymin, min(ctxcanvas->bbymin, ymin));

      ctxcanvas->bbxmax = min(ctxcanvas->xmax, max(ctxcanvas->bbxmax, xmax));
      ctxcanvas->bbymax = min(ctxcanvas->ymax, max(ctxcanvas->bbymax, ymax));
    }
  }
}

/*
%F Ajusta a BoundingBox para conter o ponto (x,y).
Leva em consideracao a espessura das linhas.
*/
static void bbox(cdCtxCanvas *ctxcanvas, int x, int y)
{
  if (ctxcanvas->canvas->line_width > 1)
  {
    insertpoint(ctxcanvas, x-ctxcanvas->canvas->line_width, y-ctxcanvas->canvas->line_width);
    insertpoint(ctxcanvas, x+ctxcanvas->canvas->line_width, y+ctxcanvas->canvas->line_width);
  }
  else 
    insertpoint(ctxcanvas, x, y);
}

static void fbbox(cdCtxCanvas *ctxcanvas, double x, double y)
{
  if (ctxcanvas->canvas->line_width > 1)
  {
    insertpoint(ctxcanvas, (int)(x-ctxcanvas->canvas->line_width), (int)(y-ctxcanvas->canvas->line_width));
    insertpoint(ctxcanvas, (int)(x+ctxcanvas->canvas->line_width), (int)(y+ctxcanvas->canvas->line_width));
  }
  else 
    insertpoint(ctxcanvas, (int)x, (int)y);
}

static char new_codes[] = {"\
/newcodes	% foreign character encodings\n\
[\n\
160/space 161/exclamdown 162/cent 163/sterling 164/currency\n\
165/yen 166/brokenbar 167/section  168/dieresis 169/copyright\n\
170/ordfeminine 171/guillemotleft 172/logicalnot 173/hyphen 174/registered\n\
175/macron 176/degree 177/plusminus 178/twosuperior 179/threesuperior\n\
180/acute 181/mu 182/paragraph  183/periodcentered 184/cedilla\n\
185/onesuperior 186/ordmasculine 187/guillemotright 188/onequarter\n\
189/onehalf 190/threequarters 191/questiondown 192/Agrave 193/Aacute\n\
194/Acircumflex 195/Atilde 196/Adieresis 197/Aring 198/AE 199/Ccedilla\n\
200/Egrave 201/Eacute 202/Ecircumflex 203/Edieresis 204/Igrave  205/Iacute\n\
206/Icircumflex 207/Idieresis 208/Eth 209/Ntilde 210/Ograve 211/Oacute\n\
212/Ocircumflex 213/Otilde  214/Odieresis 215/multiply 216/Oslash\n\
217/Ugrave 218/Uacute 219/Ucircumflex 220/Udieresis 221/Yacute 222/Thorn\n\
223/germandbls 224/agrave 225/aacute 226/acircumflex 227/atilde\n\
228/adieresis 229/aring 230/ae 231/ccedilla  232/egrave 233/eacute\n\
234/ecircumflex 235/edieresis 236/igrave 237/iacute 238/icircumflex\n\
239/idieresis 240/eth 241/ntilde 242/ograve 243/oacute 244/ocircumflex\n\
245/otilde 246/odieresis 247/divide 248/oslash 249/ugrave  250/uacute\n\
251/ucircumflex 252/udieresis 253/yacute 254/thorn 255/ydieresis\n\
] def\n\
"};

static char change_font[] = {"\
% change fonts using ISO Latin1 characters\n\
/ChgFnt		% size psname natname  =>  font\n\
{\n\
    dup FontDirectory exch known	% is re-encoded name known?\n\
    { exch pop }			% yes, get rid of long name\n\
    { dup 3 1 roll ReEncode } ifelse	% no, re-encode it\n\
    findfont exch scalefont setfont\n\
} def\n\
"};

static char re_encode[] = {"\
/ReEncode	%\n\
{\n\
  12 dict begin\n\
	/newname exch def\n\
	/basename exch def\n\
	/basedict basename findfont def\n\
	/newfont basedict maxlength dict def\n\
	basedict\n\
	{ exch dup /FID ne\n\
	    { dup /Encoding eq\n\
		    { exch dup length array copy newfont 3 1 roll put }\n\
		    { exch newfont 3 1 roll put } ifelse\n\
	    }\n\
	    { pop pop } ifelse\n\
	} forall\n\
	newfont /FontName newname put\n\
	newcodes aload pop newcodes length 2 idiv\n\
	{ newfont /Encoding get 3 1 roll put } repeat\n\
	newname newfont definefont pop\n\
    end\n\
} def\n\
"};

static void setcliprect(cdCtxCanvas* ctxcanvas, double xmin, double ymin, double xmax, double ymax)
{
  if (ctxcanvas->eps) /* initclip not allowed in EPS */
    return;

  fprintf(ctxcanvas->file, "initclip\n"); 

  /* cliping geral para a margem */
  if (ctxcanvas->level1)
  {
    fprintf(ctxcanvas->file, "N\n");
    fprintf(ctxcanvas->file, "%g %g M\n", xmin, ymin);
    fprintf(ctxcanvas->file, "%g %g L\n", xmin, ymax);
    fprintf(ctxcanvas->file, "%g %g L\n", xmax, ymax);
    fprintf(ctxcanvas->file, "%g %g L\n", xmax, ymin);
    fprintf(ctxcanvas->file, "C\n");
    fprintf(ctxcanvas->file, "clip\n");
    fprintf(ctxcanvas->file, "N\n");
  }
  else
    fprintf(ctxcanvas->file, "%g %g %g %g rectclip\n", xmin, ymin, xmax-xmin, ymax-ymin);
}

static void set_default_matrix(cdCtxCanvas *ctxcanvas)
{
  if (ctxcanvas->eps)
    fprintf(ctxcanvas->file, "oldmatrix setmatrix\n");          /* reset to current */
  else
  {
    fprintf(ctxcanvas->file, "[0 0 0 0 0 0] defaultmatrix\n");  /* reset to default */
    fprintf(ctxcanvas->file, "setmatrix\n");  
  }

  /* margin */
  fprintf(ctxcanvas->file, "%g %g translate\n", ctxcanvas->xmin, ctxcanvas->ymin);

  /* default coordinate system is in points, change it to pixels. */
  fprintf(ctxcanvas->file, "%g %g scale\n", ctxcanvas->scale, ctxcanvas->scale);
}

/*
%F Inicializa o arquivo PS.
*/
static void init_ps(cdCtxCanvas *ctxcanvas)
{
  double w_pt, h_pt;

  time_t now = time(NULL);

  /* convert margin values to actual limits */
  ctxcanvas->xmin = mm2pt(ctxcanvas->xmin);
  ctxcanvas->xmax = ctxcanvas->width_pt - mm2pt(ctxcanvas->xmax);
  ctxcanvas->ymin = mm2pt(ctxcanvas->ymin);
  ctxcanvas->ymax = ctxcanvas->height_pt - mm2pt(ctxcanvas->ymax);
  ctxcanvas->bbmargin = mm2pt(ctxcanvas->bbmargin);

  fprintf(ctxcanvas->file, "%%!PS-Adobe-3.0 %s\n", ctxcanvas->eps ? "EPSF-3.0":"");
  fprintf(ctxcanvas->file, "%%%%Title: CanvasDraw\n");
  fprintf(ctxcanvas->file, "%%%%Creator: CanvasDraw\n");
  fprintf(ctxcanvas->file, "%%%%CreationDate: %s", asctime(localtime(&now)));
  fprintf(ctxcanvas->file, "%%%%DocumentFonts: (atend)\n"); /* attend means at the end of the file, */
  fprintf(ctxcanvas->file, "%%%%Pages: (atend)\n");         /* see killcanvas */ 
  fprintf(ctxcanvas->file, "%%%%PageOrder: Ascend\n");         
  fprintf(ctxcanvas->file, "%%%%LanguageLevel: %d\n", ctxcanvas->level1 ? 1: 2);
  fprintf(ctxcanvas->file, "%%%%Orientation: %s\n", ctxcanvas->landscape ? "Landscape": "Portrait");

  if (ctxcanvas->eps)
  {
    fprintf(ctxcanvas->file, "%%%%BoundingBox: (atend)\n");
    ctxcanvas->bbxmin = ctxcanvas->bbxmax = ctxcanvas->bbymin = ctxcanvas->bbymax = 0;
    /* BoundingBox==Empty */
  }

  fprintf(ctxcanvas->file, "%%%%EndComments\n");
  fprintf(ctxcanvas->file, "%%%%BeginProlog\n");
  
  fprintf(ctxcanvas->file, "/N {newpath} bind def\n");
  fprintf(ctxcanvas->file, "/C {closepath} bind def\n");
  fprintf(ctxcanvas->file, "/M {moveto} bind def\n");
  fprintf(ctxcanvas->file, "/L {lineto} bind def\n");
  fprintf(ctxcanvas->file, "/B {curveto} bind def\n");
  fprintf(ctxcanvas->file, "/S {stroke} bind def\n");
  fprintf(ctxcanvas->file, "/LL {moveto lineto stroke} bind def\n");
  
  if (!ctxcanvas->level1)
  {
    fprintf(ctxcanvas->file, "/RF {rectfill} bind def\n");
    fprintf(ctxcanvas->file, "/RS {rectstroke} bind def\n");
  }

  fprintf(ctxcanvas->file, "%%%%EndProlog\n");
  fprintf(ctxcanvas->file, "%%%%BeginSetup\n");
  
  if (!ctxcanvas->eps && !ctxcanvas->level1)
  {
    /* setpagedevice not allowed in EPS */
    fprintf(ctxcanvas->file, "%%%%IncludeFeature: *Resolution %d\n", ctxcanvas->res);
    fprintf(ctxcanvas->file, "%%%%BeginFeature: *PageSize\n");
    fprintf(ctxcanvas->file, "<< /PageSize [%g %g] >> setpagedevice\n", ctxcanvas->width_pt, ctxcanvas->height_pt); 
    fprintf(ctxcanvas->file, "%%%%EndFeature\n");
  }

  fprintf(ctxcanvas->file, "%%%%EndSetup\n");

  fputs(new_codes, ctxcanvas->file);
  fputs(change_font, ctxcanvas->file);
  fputs(re_encode, ctxcanvas->file);

  ctxcanvas->scale = 72.0/ctxcanvas->res;
  ctxcanvas->canvas->xres = ctxcanvas->res/25.4;
  ctxcanvas->canvas->yres = ctxcanvas->canvas->xres;

  w_pt = ctxcanvas->xmax - ctxcanvas->xmin;
  h_pt = ctxcanvas->ymax - ctxcanvas->ymin;

  ctxcanvas->canvas->w_mm = w_pt/CD_MM2PT;   /* Converte p/ milimetros */
  ctxcanvas->canvas->h_mm = h_pt/CD_MM2PT; /* Converte p/ milimetros */

  ctxcanvas->canvas->w = cdRound(ctxcanvas->canvas->xres*ctxcanvas->canvas->w_mm);
  ctxcanvas->canvas->h = cdRound(ctxcanvas->canvas->yres*ctxcanvas->canvas->h_mm);

  ctxcanvas->canvas->bpp = 24;

  fprintf(ctxcanvas->file, "%%%%Page: 1 1\n");
  ctxcanvas->pages = 1;

  if (ctxcanvas->debug) fprintf(ctxcanvas->file, "\n%%cdCreateCanvas: Margin Begin\n");

  if (ctxcanvas->eps)
    fprintf(ctxcanvas->file, "/oldmatrix [0 0 0 0 0 0] currentmatrix def\n");  /* save current matrix */

  set_default_matrix(ctxcanvas);

  /* cliping geral para a margem */
  setcliprect(ctxcanvas, 0, 0, ctxcanvas->canvas->w, ctxcanvas->canvas->h);

  if (ctxcanvas->debug) fprintf(ctxcanvas->file, "%%cdCreateCanvas: MarginEnd\n");
}


static void cdkillcanvas(cdCtxCanvas *ctxcanvas)
{
  int i;

  if (ctxcanvas->debug) fprintf(ctxcanvas->file, "\n%%cdKillCanvas\n");

  fprintf(ctxcanvas->file, "showpage\n");
  fprintf(ctxcanvas->file, "%%%%Trailer\n");
  fprintf(ctxcanvas->file, "%%%%Pages: %d 1\n", ctxcanvas->pages);

  if (ctxcanvas->eps)
  {
    int xmin = (int)ctxcanvas->bbxmin;
    int xmax = (int)ctxcanvas->bbxmax;
    int ymin = (int)ctxcanvas->bbymin;
    int ymax = (int)ctxcanvas->bbymax;
    if (xmax < ctxcanvas->bbxmax) xmax++;
    if (ymax < ctxcanvas->bbymax) ymax++;
    fprintf(ctxcanvas->file,"%%%%BoundingBox: %5d %5d %5d %5d\n",xmin,ymin,xmax,ymax);
  }

  fprintf(ctxcanvas->file, "%%%%DocumentFonts:");

  for (i = 0; i < ctxcanvas->num_native_font; i++)
  {
    fprintf(ctxcanvas->file, " %s", ctxcanvas->nativefontname[i]);
    free(ctxcanvas->nativefontname[i]);
  }

  putc('\n', ctxcanvas->file);
  fprintf(ctxcanvas->file,"%%%%EOF");

  fclose(ctxcanvas->file);

  if (ctxcanvas->old_locale)
  {
    setlocale(LC_NUMERIC, ctxcanvas->old_locale);
    free(ctxcanvas->old_locale);
  }

  memset(ctxcanvas, 0, sizeof(cdCtxCanvas));
  free(ctxcanvas);
}

static void cddeactivate(cdCtxCanvas *ctxcanvas)
{
  fflush(ctxcanvas->file);
}

static int cdhatch(cdCtxCanvas *ctxcanvas, int style);
static void cdstipple(cdCtxCanvas *ctxcanvas, int n, int m, const unsigned char *stipple);
static void cdpattern(cdCtxCanvas *ctxcanvas, int n, int m, const long int *pattern);

static void sUpdateFill(cdCtxCanvas *ctxcanvas, int fill)
{
  if (fill == 0)
  {
    /* called before a NON filled primitive */
    if (ctxcanvas->debug) fprintf(ctxcanvas->file, "\n%%cdPsUpdateFill %d Begin\n", fill);

    fprintf(ctxcanvas->file, "%g %g %g setrgbcolor\n", get_red(ctxcanvas->canvas->foreground), 
                                                        get_green(ctxcanvas->canvas->foreground), 
                                                        get_blue(ctxcanvas->canvas->foreground));

    if (ctxcanvas->debug) fprintf(ctxcanvas->file, "%%cdPsUpdateFill %dEnd\n", fill);
  }
  else
  {
    /* called before a filled primitive */
    if (ctxcanvas->debug) fprintf(ctxcanvas->file, "\n%%cdPsUpdateFill %d Begin\n", fill);

    if (ctxcanvas->canvas->interior_style == CD_SOLID)
    {
      fprintf(ctxcanvas->file, "%g %g %g setrgbcolor\n", get_red(ctxcanvas->canvas->foreground), 
                                                          get_green(ctxcanvas->canvas->foreground), 
                                                          get_blue(ctxcanvas->canvas->foreground));
    }
    else if (!ctxcanvas->level1)
    {
      fprintf(ctxcanvas->file, "cd_pattern\n");
      fprintf(ctxcanvas->file, "setpattern\n");
    }

    if (ctxcanvas->debug) fprintf(ctxcanvas->file, "%%cdPsUpdateFill %dEnd\n", fill);
  }
}

/*
%F Comeca uma nova pagina.
*/
static void cdflush(cdCtxCanvas *ctxcanvas)
{
  if (ctxcanvas->debug) fprintf(ctxcanvas->file, "\n%%cdFlush Begin\n");

  fflush(ctxcanvas->file);

  if (!ctxcanvas->eps)
  {
    fprintf(ctxcanvas->file, "gsave\n");

    fprintf(ctxcanvas->file, "showpage\n");
    ctxcanvas->pages++;
    fprintf(ctxcanvas->file, "%%%%Page: %d %d\n", ctxcanvas->pages, ctxcanvas->pages);

    fprintf(ctxcanvas->file, "grestore\n");
  }

  if (ctxcanvas->debug) fprintf(ctxcanvas->file, "%%cdFlushEnd\n");
}


/******************************************************/
/* coordinate transformation                          */
/******************************************************/

static void cdfcliparea(cdCtxCanvas *ctxcanvas, double xmin, double xmax, double ymin, double ymax)
{
  if (ctxcanvas->canvas->clip_mode != CD_CLIPAREA)
    return;

  if (ctxcanvas->debug) fprintf(ctxcanvas->file, "\n%%cdfClipArea Begin\n");

  setcliprect(ctxcanvas, xmin, ymin, xmax, ymax);

  if (ctxcanvas->debug) fprintf(ctxcanvas->file, "%%cdfClipAreaEnd\n");
}

static int cdclip(cdCtxCanvas *ctxcanvas, int mode)
{
  if (ctxcanvas->debug) fprintf(ctxcanvas->file, "\n%%cdClip %d Begin\n", mode);

  if (mode == CD_CLIPAREA)
  {
    ctxcanvas->canvas->clip_mode = CD_CLIPAREA;

    setcliprect(ctxcanvas, (double)ctxcanvas->canvas->clip_rect.xmin, 
                           (double)ctxcanvas->canvas->clip_rect.ymin, 
                           (double)ctxcanvas->canvas->clip_rect.xmax, 
                           (double)ctxcanvas->canvas->clip_rect.ymax);
  }
  else if (mode == CD_CLIPPOLYGON)
  {
    fprintf(ctxcanvas->file, "clip_polygon\n");
  }
  else
  {
    /* margin clipping only */
    setcliprect(ctxcanvas, 0, 0, ctxcanvas->canvas->w, ctxcanvas->canvas->h);
  }

  if (ctxcanvas->debug) fprintf(ctxcanvas->file, "%%cdClip %dEnd\n", mode);

  return mode;
}

/******************************************************/
/* primitives                                         */
/******************************************************/

static void cdline(cdCtxCanvas *ctxcanvas, int x1, int y1, int x2, int y2)
{
  sUpdateFill(ctxcanvas, 0);

  fprintf(ctxcanvas->file, "N %d %d %d %d LL\n", x1, y1, x2, y2);

  if (ctxcanvas->eps)
  {
    bbox(ctxcanvas, x1, y1);
    bbox(ctxcanvas, x2, y2);
  }
}

static void cdfline(cdCtxCanvas *ctxcanvas, double x1, double y1, double x2, double y2)
{
  sUpdateFill(ctxcanvas, 0);

  fprintf(ctxcanvas->file, "N %g %g %g %g LL\n", x1, y1, x2, y2);

  if (ctxcanvas->eps)
  {
    fbbox(ctxcanvas, x1, y1);
    fbbox(ctxcanvas, x2, y2);
  }
}

static void cdrect(cdCtxCanvas *ctxcanvas, int xmin, int xmax, int ymin, int ymax)
{
  sUpdateFill(ctxcanvas, 0);

  if (ctxcanvas->level1)
  {
    fprintf(ctxcanvas->file, "N\n");
    fprintf(ctxcanvas->file, "%d %d M\n", xmin, ymin);
    fprintf(ctxcanvas->file, "%d %d L\n", xmin, ymax);
    fprintf(ctxcanvas->file, "%d %d L\n", xmax, ymax);
    fprintf(ctxcanvas->file, "%d %d L\n", xmax, ymin);
    fprintf(ctxcanvas->file, "C S\n");
  }
  else
    fprintf(ctxcanvas->file, "%d %d %d %d RS\n", xmin, ymin, xmax - xmin, ymax - ymin);

  if (ctxcanvas->eps)
  {
    bbox(ctxcanvas, xmin, ymin);
    bbox(ctxcanvas, xmax, ymax);
  }
}

static void cdfrect(cdCtxCanvas *ctxcanvas, double xmin, double xmax, double ymin, double ymax)
{
  sUpdateFill(ctxcanvas, 0);

  if (ctxcanvas->level1)
  {
    fprintf(ctxcanvas->file, "N\n");
    fprintf(ctxcanvas->file, "%g %g M\n", xmin, ymin);
    fprintf(ctxcanvas->file, "%g %g L\n", xmin, ymax);
    fprintf(ctxcanvas->file, "%g %g L\n", xmax, ymax);
    fprintf(ctxcanvas->file, "%g %g L\n", xmax, ymin);
    fprintf(ctxcanvas->file, "C S\n");
  }
  else
    fprintf(ctxcanvas->file, "%g %g %g %g RS\n", xmin, ymin, xmax - xmin, ymax - ymin);

  if (ctxcanvas->eps)
  {
    fbbox(ctxcanvas, xmin, ymin);
    fbbox(ctxcanvas, xmax, ymax);
  }
}

static void cdbox(cdCtxCanvas *ctxcanvas, int xmin, int xmax, int ymin, int ymax)
{
  sUpdateFill(ctxcanvas, 1);

  if (ctxcanvas->level1)
  {
    fprintf(ctxcanvas->file, "N\n");
    fprintf(ctxcanvas->file, "%d %d M\n", xmin, ymin);
    fprintf(ctxcanvas->file, "%d %d L\n", xmin, ymax);
    fprintf(ctxcanvas->file, "%d %d L\n", xmax, ymax);
    fprintf(ctxcanvas->file, "%d %d L\n", xmax, ymin);
    fprintf(ctxcanvas->file, "C fill\n");
  }
  else
    fprintf(ctxcanvas->file, "%d %d %d %d RF\n", xmin, ymin, xmax - xmin, ymax - ymin);

  if (ctxcanvas->eps)
  {
    bbox(ctxcanvas, xmin, ymin);
    bbox(ctxcanvas, xmax, ymax);
  }
}

static void cdfbox(cdCtxCanvas *ctxcanvas, double xmin, double xmax, double ymin, double ymax)
{
  sUpdateFill(ctxcanvas, 1);

  if (ctxcanvas->level1)
  {
    fprintf(ctxcanvas->file, "N\n");
    fprintf(ctxcanvas->file, "%g %g M\n", xmin, ymin);
    fprintf(ctxcanvas->file, "%g %g L\n", xmin, ymax);
    fprintf(ctxcanvas->file, "%g %g L\n", xmax, ymax);
    fprintf(ctxcanvas->file, "%g %g L\n", xmax, ymin);
    fprintf(ctxcanvas->file, "C fill\n");
  }
  else
    fprintf(ctxcanvas->file, "%g %g %g %g RF\n", xmin, ymin, xmax - xmin, ymax - ymin);

  if (ctxcanvas->eps)
  {
    fbbox(ctxcanvas, xmin, ymin);     
    fbbox(ctxcanvas, xmax, ymax);
  }
}

static void cdarc(cdCtxCanvas *ctxcanvas, int xc, int yc, int w, int h, double a1, double a2)
{
  sUpdateFill(ctxcanvas, 0);

  /* angles in degrees counterclockwise, same as CD */

  if (w==h) /* Circulo: PS implementa direto */
  {
    fprintf(ctxcanvas->file, "N %d %d %g %g %g arc S\n", xc, yc, 0.5*w, a1, a2);
  }
  else /* Elipse: mudar a escala p/ criar a partir do circulo */
  {
    if (ctxcanvas->debug) fprintf(ctxcanvas->file, "\n%%cdArc Ellipse Begin\n");

    fprintf(ctxcanvas->file, "[0 0 0 0 0 0] currentmatrix\n"); /* fill new matrix from CTM */
    fprintf(ctxcanvas->file, "%d %d translate\n", xc, yc);
    fprintf(ctxcanvas->file, "1 %g scale\n", ((double)h)/w);
    fprintf(ctxcanvas->file, "N\n");
    fprintf(ctxcanvas->file, "0 0 %g %g %g arc\n", 0.5*w, a1, a2);
    fprintf(ctxcanvas->file, "S\n");
    fprintf(ctxcanvas->file, "setmatrix\n"); /* back to CTM */

    if (ctxcanvas->debug) fprintf(ctxcanvas->file, "%%cdArc EllipseEnd\n");
  }

  if (ctxcanvas->eps)
  {
    int xmin, xmax, ymin, ymax;
    cdCanvasGetArcBox(xc, yc, w, h, a1, a2, &xmin, &xmax, &ymin, &ymax);
    bbox(ctxcanvas, xmin, ymin);
    bbox(ctxcanvas, xmax, ymax);
  }
}

static void cdfarc(cdCtxCanvas *ctxcanvas, double xc, double yc, double w, double h, double a1, double a2)
{
  sUpdateFill(ctxcanvas, 0);

  if (w==h) /* Circulo: PS implementa direto */
  {
    fprintf(ctxcanvas->file, "N %g %g %g %g %g arc S\n", xc, yc, 0.5*w, a1, a2);
  }
  else /* Elipse: mudar a escala p/ criar a partir do circulo */
  {
    if (ctxcanvas->debug) fprintf(ctxcanvas->file, "\n%%cdfArc Ellipse Begin\n");

    fprintf(ctxcanvas->file, "[0 0 0 0 0 0] currentmatrix\n");
    fprintf(ctxcanvas->file, "%g %g translate\n", xc, yc);
    fprintf(ctxcanvas->file, "1 %g scale\n", h/w);
    fprintf(ctxcanvas->file, "N\n");
    fprintf(ctxcanvas->file, "0 0 %g %g %g arc\n", 0.5*w, a1, a2);
    fprintf(ctxcanvas->file, "S\n");
    fprintf(ctxcanvas->file, "setmatrix\n");

    if (ctxcanvas->debug) fprintf(ctxcanvas->file, "%%cdfArc EllipseEnd\n");
  }

  if (ctxcanvas->eps)
  {
    int xmin, xmax, ymin, ymax;
    cdCanvasGetArcBox(_cdRound(xc), _cdRound(yc), _cdRound(w), _cdRound(h), a1, a2, &xmin, &xmax, &ymin, &ymax);
    bbox(ctxcanvas, xmin, ymin);
    bbox(ctxcanvas, xmax, ymax);
  }
}

static void cdsector(cdCtxCanvas *ctxcanvas, int xc, int yc, int w, int h, double a1, double a2)
{
  sUpdateFill(ctxcanvas, 1);

  if (w==h) /* Circulo: PS implementa direto */
  {
    if (ctxcanvas->debug) fprintf(ctxcanvas->file, "\n%%cdSector Circle Begin\n");

    fprintf(ctxcanvas->file, "N\n");
    fprintf(ctxcanvas->file, "%d %d M\n", xc, yc);
    fprintf(ctxcanvas->file, "%d %d %g %g %g arc\n", xc, yc, 0.5*w, a1, a2);
    fprintf(ctxcanvas->file, "C fill\n");

    if (ctxcanvas->debug) fprintf(ctxcanvas->file, "%%cdSector CircleEnd\n");
  }
  else /* Elipse: mudar a escala p/ criar a partir do circulo */
  {
    if (ctxcanvas->debug) fprintf(ctxcanvas->file, "\n%%cdSector Ellipse Begin\n");

    fprintf(ctxcanvas->file, "[0 0 0 0 0 0] currentmatrix\n");
    fprintf(ctxcanvas->file, "%d %d translate\n", xc, yc);
    fprintf(ctxcanvas->file, "1 %g scale\n", ((double)h)/w);
    fprintf(ctxcanvas->file, "N\n");
    fprintf(ctxcanvas->file, "0 0 M\n");
    fprintf(ctxcanvas->file, "0 0 %g %g %g arc\n", 0.5*w, a1, a2);
    fprintf(ctxcanvas->file, "C fill\n");
    fprintf(ctxcanvas->file, "setmatrix\n");

    if (ctxcanvas->debug) fprintf(ctxcanvas->file, "%%cdSector EllipseEnd\n");
  }

  if (ctxcanvas->eps)
  {
    int xmin, xmax, ymin, ymax;
    cdCanvasGetArcBox(xc, yc, w, h, a1, a2, &xmin, &xmax, &ymin, &ymax);
    bbox(ctxcanvas, xmin, ymin);
    bbox(ctxcanvas, xmax, ymax);
    bbox(ctxcanvas, xc, yc);
  }
}

static void cdfsector(cdCtxCanvas *ctxcanvas, double xc, double yc, double w, double h, double a1, double a2)
{
  sUpdateFill(ctxcanvas, 1);

  if (w==h) /* Circulo: PS implementa direto */
  {
    if (ctxcanvas->debug) fprintf(ctxcanvas->file, "\n%%cdfSector Circle Begin\n");

    fprintf(ctxcanvas->file, "N\n");
    fprintf(ctxcanvas->file, "%g %g M\n", xc, yc);
    fprintf(ctxcanvas->file, "%g %g %g %g %g arc\n", xc, yc, 0.5*w, a1, a2);
    fprintf(ctxcanvas->file, "C fill\n");

    if (ctxcanvas->debug) fprintf(ctxcanvas->file, "%%cdfSector CircleEnd\n");
  }
  else /* Elipse: mudar a escala p/ criar a partir do circulo */
  {
    if (ctxcanvas->debug) fprintf(ctxcanvas->file, "\n%%cdfSector Ellipse Begin\n");

    fprintf(ctxcanvas->file, "[0 0 0 0 0 0] currentmatrix\n");
    fprintf(ctxcanvas->file, "%g %g translate\n", xc, yc);
    fprintf(ctxcanvas->file, "1 %g scale\n", h/w);
    fprintf(ctxcanvas->file, "N\n");
    fprintf(ctxcanvas->file, "0 0 M\n");
    fprintf(ctxcanvas->file, "0 0 %g %g %g arc\n", 0.5*w, a1, a2);
    fprintf(ctxcanvas->file, "C fill\n");
    fprintf(ctxcanvas->file, "setmatrix\n");

    if (ctxcanvas->debug) fprintf(ctxcanvas->file, "%%cdfSector EllipseEnd\n");
  }

  if (ctxcanvas->eps)
  {
    int xmin, xmax, ymin, ymax;
    cdCanvasGetArcBox(_cdRound(xc), _cdRound(yc), _cdRound(w), _cdRound(h), a1, a2, &xmin, &xmax, &ymin, &ymax);
    bbox(ctxcanvas, xmin, ymin);
    bbox(ctxcanvas, xmax, ymax);
    fbbox(ctxcanvas, xc, yc);
  }
}

static void cdchord(cdCtxCanvas *ctxcanvas, int xc, int yc, int w, int h, double a1, double a2)
{
  sUpdateFill(ctxcanvas, 1);

  if (w==h) /* Circulo: PS implementa direto */
  {
    if (ctxcanvas->debug) fprintf(ctxcanvas->file, "\n%%cdChord Circle Begin\n");

    fprintf(ctxcanvas->file, "N\n");
    fprintf(ctxcanvas->file, "%d %d %g %g %g arc\n", xc, yc, 0.5*w, a1, a2);
    fprintf(ctxcanvas->file, "C fill\n");

    if (ctxcanvas->debug) fprintf(ctxcanvas->file, "%%cdChord CircleEnd\n");
  }
  else /* Elipse: mudar a escala p/ criar a partir do circulo */
  {
    if (ctxcanvas->debug) fprintf(ctxcanvas->file, "\n%%cdChord Ellipse Begin\n");

    fprintf(ctxcanvas->file, "[0 0 0 0 0 0] currentmatrix\n");
    fprintf(ctxcanvas->file, "%d %d translate\n", xc, yc);
    fprintf(ctxcanvas->file, "1 %g scale\n", ((double)h)/w);
    fprintf(ctxcanvas->file, "N\n");
    fprintf(ctxcanvas->file, "0 0 %g %g %g arc\n", 0.5*w, a1, a2);
    fprintf(ctxcanvas->file, "C fill\n");
    fprintf(ctxcanvas->file, "setmatrix\n");

    if (ctxcanvas->debug) fprintf(ctxcanvas->file, "%%cdChord EllipseEnd\n");
  }

  if (ctxcanvas->eps)
  {
    int xmin, xmax, ymin, ymax;
    cdCanvasGetArcBox(xc, yc, w, h, a1, a2, &xmin, &xmax, &ymin, &ymax);
    bbox(ctxcanvas, xmin, ymin);
    bbox(ctxcanvas, xmax, ymax);
  }
}

static void cdfchord(cdCtxCanvas *ctxcanvas, double xc, double yc, double w, double h, double a1, double a2)
{
  sUpdateFill(ctxcanvas, 1);

  if (w==h) /* Circulo: PS implementa direto */
  {
    if (ctxcanvas->debug) fprintf(ctxcanvas->file, "\n%%cdfChord Circle Begin\n");

    fprintf(ctxcanvas->file, "N\n");
    fprintf(ctxcanvas->file, "%g %g %g %g %g arc\n", xc, yc, 0.5*w, a1, a2);
    fprintf(ctxcanvas->file, "C fill\n");

    if (ctxcanvas->debug) fprintf(ctxcanvas->file, "%%cdfChord CircleEnd\n");
  }
  else /* Elipse: mudar a escala p/ criar a partir do circulo */
  {
    if (ctxcanvas->debug) fprintf(ctxcanvas->file, "\n%%cdfChord Ellipse Begin\n");

    fprintf(ctxcanvas->file, "[0 0 0 0 0 0] currentmatrix\n");
    fprintf(ctxcanvas->file, "%g %g translate\n", xc, yc);
    fprintf(ctxcanvas->file, "1 %g scale\n", h/w);
    fprintf(ctxcanvas->file, "N\n");
    fprintf(ctxcanvas->file, "0 0 %g %g %g arc\n", 0.5*w, a1, a2);
    fprintf(ctxcanvas->file, "C fill\n");
    fprintf(ctxcanvas->file, "setmatrix\n");

    if (ctxcanvas->debug) fprintf(ctxcanvas->file, "%%cdfChord EllipseEnd\n");
  }

  if (ctxcanvas->eps)
  {
    int xmin, xmax, ymin, ymax;
    cdCanvasGetArcBox(_cdRound(xc), _cdRound(yc), _cdRound(w), _cdRound(h), a1, a2, &xmin, &xmax, &ymin, &ymax);
    bbox(ctxcanvas, xmin, ymin);
    bbox(ctxcanvas, xmax, ymax);
  }
}

static void cdtransform(cdCtxCanvas *ctxcanvas, const double* matrix);

static void cdtext(cdCtxCanvas *ctxcanvas, int x, int y, const char *s, int len)
{
  int i;
  int ascent, height, baseline;
  
  sUpdateFill(ctxcanvas, 0);

  cdCanvasGetFontDim(ctxcanvas->canvas, NULL, &height, &ascent, NULL);
  baseline = height - ascent;

  if (ctxcanvas->debug) fprintf(ctxcanvas->file, "\n%%cdText Begin\n");

  if (ctxcanvas->canvas->use_matrix || ctxcanvas->rotate_angle)
    set_default_matrix(ctxcanvas);

  fprintf(ctxcanvas->file, "N 0 0 M\n");
  putc('(', ctxcanvas->file);

  for (i=0; i<len; i++)
  {
    if (s[i]=='(' || s[i]==')')
      putc('\\', ctxcanvas->file);
    putc(s[i], ctxcanvas->file);
  }

  fprintf(ctxcanvas->file, ")\n");
  fprintf(ctxcanvas->file, "dup true charpath\n");
  fprintf(ctxcanvas->file, "flattenpath\n");
  fprintf(ctxcanvas->file, "pathbbox\n");    /* bbox na pilha: llx lly urx ury */
  fprintf(ctxcanvas->file, "exch\n");        /* troca o topo: llx lly ury urx */
  fprintf(ctxcanvas->file, "4 1 roll\n");    /* roda: urx llx lly ury */
  fprintf(ctxcanvas->file, "exch\n");        /* troca o topo: urx llx ury lly */
  fprintf(ctxcanvas->file, "sub\n");         /* subtrai: urx llx h */
  fprintf(ctxcanvas->file, "3 1 roll\n");    /* roda: h urx llx */
  fprintf(ctxcanvas->file, "sub\n");         /* subtrai: h w */
  fprintf(ctxcanvas->file, "0 0\n");         /* empilha: h w 0 0 */
  fprintf(ctxcanvas->file, "4 -1 roll\n");   /* roda: w 0 0 h */

  if (ctxcanvas->canvas->use_matrix || ctxcanvas->rotate_angle)
    cdtransform(ctxcanvas, ctxcanvas->canvas->use_matrix? ctxcanvas->canvas->matrix: NULL);

  fprintf(ctxcanvas->file, "gsave\n");   /* save to use local transform */
  fprintf(ctxcanvas->file, "%d %d translate\n", x, y);

  if (ctxcanvas->canvas->text_orientation != 0)
    fprintf(ctxcanvas->file, "%g rotate\n", ctxcanvas->canvas->text_orientation);

  switch (ctxcanvas->canvas->text_alignment) /* Operacao em Y. topo da pilha: w x y h */
  {
  case CD_NORTH:
  case CD_NORTH_EAST:
  case CD_NORTH_WEST:
    fprintf(ctxcanvas->file, "%d sub sub\n", baseline);       /* empilha, subtrai, subtrai: w x y-(h-baseline) */
    break;
  case CD_EAST:
  case CD_WEST:
  case CD_CENTER:
    fprintf(ctxcanvas->file, "2 div %d sub sub\n", baseline); /* empilha, divide, empilha, subtrai, subtrai: w x y-(h/2-baseline) */
    break;
  case CD_SOUTH_EAST:
  case CD_SOUTH:
  case CD_SOUTH_WEST:
    fprintf(ctxcanvas->file, "pop %d add\n", baseline); /* desempilha, empilha, adiciona: w x y+baseline */
    break;
  case CD_BASE_RIGHT:
  case CD_BASE_CENTER:
  case CD_BASE_LEFT:
    fprintf(ctxcanvas->file, "pop\n");       /* desempilha h: w x y */
    break;
  }

  fprintf(ctxcanvas->file, "3 1 roll\n");    /* roda: y' w x */
  fprintf(ctxcanvas->file, "exch\n");        /* inverte: y' x w */

  switch (ctxcanvas->canvas->text_alignment) /* Operacao em X, topo da pilha: x w */
  {
  case CD_NORTH:
  case CD_SOUTH:
  case CD_CENTER:
  case CD_BASE_CENTER:
    fprintf(ctxcanvas->file, "2 div sub\n");  /* empilha, divide, subtrai: y' x-w/2 */
    break;
  case CD_NORTH_EAST:
  case CD_EAST:
  case CD_SOUTH_EAST:
  case CD_BASE_RIGHT:
    fprintf(ctxcanvas->file, "sub\n");        /* subtrai: y' x-w */
    break;
  case CD_SOUTH_WEST:
  case CD_WEST:
  case CD_NORTH_WEST:
  case CD_BASE_LEFT:
    fprintf(ctxcanvas->file, "pop\n");        /* desempilha: y' x */
    break;
  }

  fprintf(ctxcanvas->file, "exch\n");         /* inverte: x' y' */
  fprintf(ctxcanvas->file, "M\n");            /* moveto */

  fprintf(ctxcanvas->file, "show\n");

  if (ctxcanvas->eps)
  {
    int xmin, xmax, ymin, ymax;
    s = cdStrDupN(s, len);
    cdCanvasGetTextBox(ctxcanvas->canvas, x, y, s, &xmin, &xmax, &ymin, &ymax);
    free((char*)s);
    bbox(ctxcanvas, xmin, ymin);
    bbox(ctxcanvas, xmax, ymax);
  }

  fprintf(ctxcanvas->file, "grestore\n");

  if (ctxcanvas->debug) fprintf(ctxcanvas->file, "%%cdTextEnd\n");
}

static void cdftext(cdCtxCanvas *ctxcanvas, double x, double y, const char *s, int len)
{
  int i;
  int ascent, height, baseline;
  
  sUpdateFill(ctxcanvas, 0);

  cdCanvasGetFontDim(ctxcanvas->canvas, NULL, &height, &ascent, NULL);
  baseline = height - ascent;

  if (ctxcanvas->debug) fprintf(ctxcanvas->file, "\n%%cdText Begin\n");

  if (ctxcanvas->canvas->use_matrix || ctxcanvas->rotate_angle)
    set_default_matrix(ctxcanvas);

  fprintf(ctxcanvas->file, "N 0 0 M\n");
  putc('(', ctxcanvas->file);

  for (i=0; i<len; i++)
  {
    if (s[i]=='(' || s[i]==')')
      putc('\\', ctxcanvas->file);
    putc(s[i], ctxcanvas->file);
  }

  fprintf(ctxcanvas->file, ")\n");
  fprintf(ctxcanvas->file, "dup true charpath\n");
  fprintf(ctxcanvas->file, "flattenpath\n");
  fprintf(ctxcanvas->file, "pathbbox\n");    /* bbox na pilha: llx lly urx ury */
  fprintf(ctxcanvas->file, "exch\n");        /* troca o topo: llx lly ury urx */
  fprintf(ctxcanvas->file, "4 1 roll\n");    /* roda: urx llx lly ury */
  fprintf(ctxcanvas->file, "exch\n");        /* troca o topo: urx llx ury lly */
  fprintf(ctxcanvas->file, "sub\n");         /* subtrai: urx llx h */
  fprintf(ctxcanvas->file, "3 1 roll\n");    /* roda: h urx llx */
  fprintf(ctxcanvas->file, "sub\n");         /* subtrai: h w */
  fprintf(ctxcanvas->file, "0 0\n");         /* empilha: h w 0 0 */
  fprintf(ctxcanvas->file, "4 -1 roll\n");   /* roda: w 0 0 h */

  if (ctxcanvas->canvas->use_matrix || ctxcanvas->rotate_angle)
    cdtransform(ctxcanvas, ctxcanvas->canvas->use_matrix? ctxcanvas->canvas->matrix: NULL);

  fprintf(ctxcanvas->file, "gsave\n");   /* save to use local transform */
  fprintf(ctxcanvas->file, "%g %g translate\n", x, y);

  if (ctxcanvas->canvas->text_orientation != 0)
    fprintf(ctxcanvas->file, "%g rotate\n", ctxcanvas->canvas->text_orientation);

  switch (ctxcanvas->canvas->text_alignment) /* Operacao em Y. topo da pilha: w x y h */
  {
  case CD_NORTH:
  case CD_NORTH_EAST:
  case CD_NORTH_WEST:
    fprintf(ctxcanvas->file, "%d sub sub\n", baseline);       /* empilha, subtrai, subtrai: w x y-(h-baseline) */
    break;
  case CD_EAST:
  case CD_WEST:
  case CD_CENTER:
    fprintf(ctxcanvas->file, "2 div %d sub sub\n", baseline); /* empilha, divide, empilha, subtrai, subtrai: w x y-(h/2-baseline) */
    break;
  case CD_SOUTH_EAST:
  case CD_SOUTH:
  case CD_SOUTH_WEST:
    fprintf(ctxcanvas->file, "pop %d add\n", baseline); /* desempilha, empilha, adiciona: w x y+baseline */
    break;
  case CD_BASE_RIGHT:
  case CD_BASE_CENTER:
  case CD_BASE_LEFT:
    fprintf(ctxcanvas->file, "pop\n");       /* desempilha h: w x y */
    break;
  }

  fprintf(ctxcanvas->file, "3 1 roll\n");    /* roda: y' w x */
  fprintf(ctxcanvas->file, "exch\n");        /* inverte: y' x w */

  switch (ctxcanvas->canvas->text_alignment) /* Operacao em X, topo da pilha: x w */
  {
  case CD_NORTH:
  case CD_SOUTH:
  case CD_CENTER:
  case CD_BASE_CENTER:
    fprintf(ctxcanvas->file, "2 div sub\n");  /* empilha, divide, subtrai: y' x-w/2 */
    break;
  case CD_NORTH_EAST:
  case CD_EAST:
  case CD_SOUTH_EAST:
  case CD_BASE_RIGHT:
    fprintf(ctxcanvas->file, "sub\n");        /* subtrai: y' x-w */
    break;
  case CD_SOUTH_WEST:
  case CD_WEST:
  case CD_NORTH_WEST:
  case CD_BASE_LEFT:
    fprintf(ctxcanvas->file, "pop\n");        /* desempilha: y' x */
    break;
  }

  fprintf(ctxcanvas->file, "exch\n");         /* inverte: x' y' */
  fprintf(ctxcanvas->file, "M\n");            /* moveto */

  fprintf(ctxcanvas->file, "show\n");

  if (ctxcanvas->eps)
  {
    int xmin, xmax, ymin, ymax;
    s = cdStrDupN(s, len);
    cdCanvasGetTextBox(ctxcanvas->canvas, (int)x, (int)y, s, &xmin, &xmax, &ymin, &ymax);
    free((char*)s);
    fbbox(ctxcanvas, (double)xmin, (double)ymin);
    fbbox(ctxcanvas, (double)xmax, (double)ymax);
  }

  fprintf(ctxcanvas->file, "grestore\n");

  if (ctxcanvas->debug) fprintf(ctxcanvas->file, "%%cdTextEnd\n");
}

static void cdpoly(cdCtxCanvas *ctxcanvas, int mode, cdPoint* poly, int n)
{
  int i;

  if (mode == CD_PATH)
  {
    int p;

    /* if there is any current path, remove it */
    fprintf(ctxcanvas->file, "newpath\n");

    i = 0;
    for (p=0; p<ctxcanvas->canvas->path_n; p++)
    {
      switch(ctxcanvas->canvas->path[p])
      {
      case CD_PATH_NEW:
        fprintf(ctxcanvas->file, "newpath\n");
        break;
      case CD_PATH_MOVETO:
        if (i+1 > n) return;
        fprintf(ctxcanvas->file, "%d %d M\n", poly[i].x, poly[i].y);
        i++;
        break;
      case CD_PATH_LINETO:
        if (i+1 > n) return;
        fprintf(ctxcanvas->file, "%d %d L\n", poly[i].x, poly[i].y);
        i++;
        break;
      case CD_PATH_ARC:
        {
          int xc, yc, w, h;
          double a1, a2;
          char* arc = "arc";

          if (i+3 > n) return;

          if (!cdCanvasGetArcPath(ctxcanvas->canvas, poly+i, &xc, &yc, &w, &h, &a1, &a2))
            return;

          if ((a2-a1)<0)
            arc = "arcn";

          if (w==h) /* Circulo: PS implementa direto */
          {
            fprintf(ctxcanvas->file, "%d %d %g %g %g %s\n", xc, yc, 0.5*w, a1, a2, arc);
          }
          else /* Elipse: mudar a escala p/ criar a partir do circulo */
          {
            fprintf(ctxcanvas->file, "[0 0 0 0 0 0] currentmatrix\n"); /* fill new matrix from CTM */
            fprintf(ctxcanvas->file, "%d %d translate\n", xc, yc);
            fprintf(ctxcanvas->file, "1 %g scale\n", ((double)h)/w);
            fprintf(ctxcanvas->file, "0 0 %g %g %g %s\n", 0.5*w, a1, a2, arc);
            fprintf(ctxcanvas->file, "setmatrix\n"); /* back to CTM */
          }

          i += 3;
        }
        break;
      case CD_PATH_CURVETO:
        if (i+3 > n) return;
        fprintf(ctxcanvas->file, "%d %d %d %d %d %d B\n", poly[i].x,   poly[i].y, 
                                                          poly[i+1].x, poly[i+1].y, 
                                                          poly[i+2].x, poly[i+2].y);
        i += 3;
        break;
      case CD_PATH_CLOSE:
        fprintf(ctxcanvas->file, "closepath\n");
        break;
      case CD_PATH_FILL:
        sUpdateFill(ctxcanvas, 1);
        if (ctxcanvas->holes || ctxcanvas->canvas->fill_mode==CD_EVENODD)
          fprintf(ctxcanvas->file, "eofill\n");
        else
          fprintf(ctxcanvas->file, "fill\n");
        break;
      case CD_PATH_STROKE:
        sUpdateFill(ctxcanvas, 0);
        fprintf(ctxcanvas->file, "stroke\n");
        break;
      case CD_PATH_FILLSTROKE:
        sUpdateFill(ctxcanvas, 1);
        fprintf(ctxcanvas->file, "gsave\n");
        if (ctxcanvas->holes || ctxcanvas->canvas->fill_mode==CD_EVENODD)
          fprintf(ctxcanvas->file, "eofill\n");
        else
          fprintf(ctxcanvas->file, "fill\n");
        fprintf(ctxcanvas->file, "grestore\n");
        sUpdateFill(ctxcanvas, 0);
        fprintf(ctxcanvas->file, "stroke\n");
        break;
      case CD_PATH_CLIP:
        if (ctxcanvas->canvas->fill_mode==CD_EVENODD)
          fprintf(ctxcanvas->file, "closepath eoclip\n");
        else
          fprintf(ctxcanvas->file, "closepath clip\n");
        break;
      }
    }
    return;
  }

  if (mode == CD_CLIP)
  {
    if (ctxcanvas->eps) /* initclip not allowed in EPS */
      return;

    fprintf(ctxcanvas->file, "/clip_polygon {\n");
    fprintf(ctxcanvas->file, "initclip\n");
  }
  else
  {
    if (mode == CD_FILL)
      sUpdateFill(ctxcanvas, 1);
    else
      sUpdateFill(ctxcanvas, 0);
  }

  if (ctxcanvas->debug) fprintf(ctxcanvas->file, "\n%%cdPoly %d Begin\n", mode);

  fprintf(ctxcanvas->file, "N\n");
  fprintf(ctxcanvas->file, "%d %d M\n", poly[0].x, poly[0].y);

  if (ctxcanvas->eps) 
    bbox(ctxcanvas, poly[0].x, poly[0].y);

  if (mode == CD_BEZIER)
  {
    for (i=1; i<n; i+=3)
    {
      fprintf(ctxcanvas->file, "%d %d %d %d %d %d B\n", poly[i].x,   poly[i].y, 
                                                          poly[i+1].x, poly[i+1].y, 
                                                          poly[i+2].x, poly[i+2].y);

      if (ctxcanvas->eps) 
      {
        bbox(ctxcanvas, poly[i].x,   poly[i].y);
        bbox(ctxcanvas, poly[i+2].x, poly[i+2].y);
        bbox(ctxcanvas, poly[i+3].x, poly[i+3].y);
      }
    }
  }
  else
  {
    int hole_index = 0;

    for (i=1; i<n; i++)
    {
      if (ctxcanvas->holes && i == ctxcanvas->poly_holes[hole_index])
      {
        fprintf(ctxcanvas->file, "%d %d M\n", poly[i].x, poly[i].y);
        hole_index++;
      }
      else
        fprintf(ctxcanvas->file, "%d %d L\n", poly[i].x, poly[i].y);

      if (ctxcanvas->eps) 
        bbox(ctxcanvas, poly[i].x, poly[i].y);
    }
  }

  switch (mode)
  {
  case CD_CLOSED_LINES :
    fprintf(ctxcanvas->file, "C S\n");
    break;
  case CD_OPEN_LINES :
    fprintf(ctxcanvas->file, "S\n");
    break;
  case CD_BEZIER :
    fprintf(ctxcanvas->file, "S\n");
    break;
  case CD_FILL :
    if (ctxcanvas->holes || ctxcanvas->canvas->fill_mode==CD_EVENODD)
      fprintf(ctxcanvas->file, "eofill\n");
    else
      fprintf(ctxcanvas->file, "fill\n");
    break;
  case CD_CLIP :
    if (ctxcanvas->canvas->fill_mode==CD_EVENODD)
      fprintf(ctxcanvas->file, "C eoclip\n");
    else
      fprintf(ctxcanvas->file, "C clip\n");
    fprintf(ctxcanvas->file, "N\n");
    fprintf(ctxcanvas->file, "} bind def\n");
    if (ctxcanvas->canvas->clip_mode == CD_CLIPPOLYGON) 
      fprintf(ctxcanvas->file, "clip_polygon\n");
    break;
  }

  if (ctxcanvas->debug) fprintf(ctxcanvas->file, "%%cdPoly %dEnd\n", mode);
}

static void cdfpoly(cdCtxCanvas *ctxcanvas, int mode, cdfPoint* poly, int n)
{
  int i, hole_index = 0;

  if (mode == CD_PATH)
  {
    int p;

    /* if there is any current path, remove it */
    fprintf(ctxcanvas->file, "newpath\n");

    i = 0;
    for (p=0; p<ctxcanvas->canvas->path_n; p++)
    {
      switch(ctxcanvas->canvas->path[p])
      {
      case CD_PATH_NEW:
        fprintf(ctxcanvas->file, "newpath\n");
        break;
      case CD_PATH_MOVETO:
        if (i+1 > n) return;
        fprintf(ctxcanvas->file, "%g %g M\n", poly[i].x, poly[i].y);
        i++;
        break;
      case CD_PATH_LINETO:
        if (i+1 > n) return;
        fprintf(ctxcanvas->file, "%g %g L\n", poly[i].x, poly[i].y);
        i++;
        break;
      case CD_PATH_ARC:
        {
          double xc, yc, w, h, a1, a2;
          char* arc = "arc";

          if (i+3 > n) return;

          if (!cdfCanvasGetArcPath(ctxcanvas->canvas, poly+i, &xc, &yc, &w, &h, &a1, &a2))
            return;

          if ((a2-a1)<0)
            arc = "arcn";

          if (w==h) /* Circulo: PS implementa direto */
          {
            fprintf(ctxcanvas->file, "%g %g %g %g %g %s\n", xc, yc, 0.5*w, a1, a2, arc);
          }
          else /* Elipse: mudar a escala p/ criar a partir do circulo */
          {
            fprintf(ctxcanvas->file, "[0 0 0 0 0 0] currentmatrix\n"); /* fill new matrix from CTM */
            fprintf(ctxcanvas->file, "%g %g translate\n", xc, yc);
            fprintf(ctxcanvas->file, "1 %g scale\n", ((double)h)/w);
            fprintf(ctxcanvas->file, "0 0 %g %g %g %s\n", 0.5*w, a1, a2, arc);
            fprintf(ctxcanvas->file, "setmatrix\n"); /* back to CTM */
          }

          i += 3;
        }
        break;
      case CD_PATH_CURVETO:
        if (i+3 > n) return;
        fprintf(ctxcanvas->file, "%g %g %g %g %g %g B\n", poly[i].x,   poly[i].y, 
                                                          poly[i+1].x, poly[i+1].y, 
                                                          poly[i+2].x, poly[i+2].y);
        i += 3;
        break;
      case CD_PATH_CLOSE:
        fprintf(ctxcanvas->file, "closepath\n");
        break;
      case CD_PATH_FILL:
        sUpdateFill(ctxcanvas, 1);
        if (ctxcanvas->holes || ctxcanvas->canvas->fill_mode==CD_EVENODD)
          fprintf(ctxcanvas->file, "eofill\n");
        else
          fprintf(ctxcanvas->file, "fill\n");
        break;
      case CD_PATH_STROKE:
        sUpdateFill(ctxcanvas, 0);
        fprintf(ctxcanvas->file, "stroke\n");
        break;
      case CD_PATH_FILLSTROKE:
        sUpdateFill(ctxcanvas, 1);
        fprintf(ctxcanvas->file, "gsave\n");
        if (ctxcanvas->holes || ctxcanvas->canvas->fill_mode==CD_EVENODD)
          fprintf(ctxcanvas->file, "eofill\n");
        else
          fprintf(ctxcanvas->file, "fill\n");
        fprintf(ctxcanvas->file, "grestore\n");
        sUpdateFill(ctxcanvas, 0);
        fprintf(ctxcanvas->file, "stroke\n");
        break;
      case CD_PATH_CLIP:
        if (ctxcanvas->canvas->fill_mode==CD_EVENODD)
          fprintf(ctxcanvas->file, "C eoclip\n");
        else
          fprintf(ctxcanvas->file, "C clip\n");
        break;
      }
    }
    return;
  }

  if (mode == CD_CLIP)
  {
    if (ctxcanvas->eps) /* initclip not allowed in EPS */
      return;

    fprintf(ctxcanvas->file, "/clip_polygon {\n");
    fprintf(ctxcanvas->file, "initclip\n");
  }
  else
  {
    if (mode == CD_FILL)
      sUpdateFill(ctxcanvas, 1);
    else
      sUpdateFill(ctxcanvas, 0);
  }

  if (ctxcanvas->debug) fprintf(ctxcanvas->file, "\n%%cdfPoly %d Begin\n", mode);

  fprintf(ctxcanvas->file, "N\n");
  fprintf(ctxcanvas->file, "%g %g M\n", poly[0].x, poly[0].y);

  if (ctxcanvas->eps) 
    fbbox(ctxcanvas, poly[0].x, poly[0].y);

  for (i=1; i<n; i++)
  {
    if (ctxcanvas->holes && i == ctxcanvas->poly_holes[hole_index])
    {
      fprintf(ctxcanvas->file, "%g %g M\n", poly[i].x, poly[i].y);
      hole_index++;
    }
    else
      fprintf(ctxcanvas->file, "%g %g L\n", poly[i].x, poly[i].y);

    if (ctxcanvas->eps) 
      fbbox(ctxcanvas, poly[i].x, poly[i].y);
  }

  switch (mode)
  {
  case CD_CLOSED_LINES :
    fprintf(ctxcanvas->file, "C S\n");
    break;
  case CD_OPEN_LINES :
    fprintf(ctxcanvas->file, "S\n");
    break;
  case CD_FILL :
    if (ctxcanvas->holes || ctxcanvas->canvas->fill_mode==CD_EVENODD)
      fprintf(ctxcanvas->file, "eofill\n");
    else
      fprintf(ctxcanvas->file, "fill\n");
    break;
  case CD_CLIP :
    if (ctxcanvas->canvas->fill_mode==CD_EVENODD)
      fprintf(ctxcanvas->file, "C eoclip\n");
    else
      fprintf(ctxcanvas->file, "C clip\n");
    fprintf(ctxcanvas->file, "N\n");
    fprintf(ctxcanvas->file, "} bind def\n");
    if (ctxcanvas->canvas->clip_mode == CD_CLIPPOLYGON) 
      fprintf(ctxcanvas->file, "clip_polygon\n");
    break;
  }

  if (ctxcanvas->debug) fprintf(ctxcanvas->file, "%%cdfPoly %dEnd\n", mode);
}


/******************************************************/
/* attributes                                         */
/******************************************************/

static int cdlinestyle(cdCtxCanvas *ctxcanvas, int style)
{
  double mm = ctxcanvas->canvas->xres;

  if (ctxcanvas->debug) fprintf(ctxcanvas->file, "\n%%cdLineStyle %d Begin\n", style);

  fprintf(ctxcanvas->file, "[");

  switch (style)
  {
  case CD_CONTINUOUS : /* empty dash */
    fprintf(ctxcanvas->file, " ");
    break;
  case CD_DASHED :
    fprintf(ctxcanvas->file, "%g %g", 3*mm, mm);
    break;
  case CD_DOTTED :
    fprintf(ctxcanvas->file, "%g %g", mm, mm);
    break;
  case CD_DASH_DOT :
    fprintf(ctxcanvas->file, "%g %g %g %g", 3*mm, mm, mm, mm);
    break;
  case CD_DASH_DOT_DOT :
    fprintf(ctxcanvas->file, "%g %g %g %g %g %g", 3*mm, mm, mm, mm, mm, mm);
    break;
  case CD_CUSTOM :
    {
      int i;  /* size here is in pixels, do not use mm */
      for (i = 0; i < ctxcanvas->canvas->line_dashes_count; i++)
        fprintf(ctxcanvas->file, "%g ", (double)ctxcanvas->canvas->line_dashes[i]);
    }
    break;
  }

  fprintf(ctxcanvas->file, "] 0 setdash\n");

  if (ctxcanvas->debug) fprintf(ctxcanvas->file, "%%cdLineStyle %dEnd\n", style);

  return style;
}

static int cdlinewidth(cdCtxCanvas *ctxcanvas, int width)
{
  fprintf(ctxcanvas->file, "%d setlinewidth\n", width);
  return width;
}

static int cdlinejoin(cdCtxCanvas *ctxcanvas, int join)
{
  int cd2ps_join[] = {0, 2, 1};
  fprintf(ctxcanvas->file, "%d setlinejoin\n", cd2ps_join[join]);
  return join;
}

static int cdlinecap(cdCtxCanvas *ctxcanvas, int cap)
{
  int cd2ps_cap[] =  {0, 2, 1};
  fprintf(ctxcanvas->file, "%d setlinecap\n", cd2ps_cap[cap]);
  return cap;
}

static void make_pattern(cdCtxCanvas *ctxcanvas, int n, int m, void* data, void (*data2rgb)(cdCtxCanvas *ctxcanvas, int n, int i, int j, void* data, unsigned char*r, unsigned char*g, unsigned char*b))
{
  int i, j;
  unsigned char r, g, b;

  if (ctxcanvas->debug) fprintf(ctxcanvas->file, "\n%%cdPsMakePattern Begin\n");

  fprintf(ctxcanvas->file, "/cd_pattern\n");
  fprintf(ctxcanvas->file, "currentfile %d string readhexstring\n", n*m*3);

  for (j=0; j<m; j++)
  {
    for (i=0; i<n; i++)
    {
      data2rgb(ctxcanvas, n, i, j, data, &r, &g, &b);
      fprintf(ctxcanvas->file, "%02x%02x%02x", (int)r, (int)g, (int)b);
    }

    fprintf(ctxcanvas->file, "\n");
  }

  fprintf(ctxcanvas->file, "pop\n");
  fprintf(ctxcanvas->file, "/Pat exch def\n");
  fprintf(ctxcanvas->file, "<<\n");
  fprintf(ctxcanvas->file, "  /PatternType 1\n");
  fprintf(ctxcanvas->file, "  /PaintType 1\n");
  fprintf(ctxcanvas->file, "  /TilingType 1\n");
  fprintf(ctxcanvas->file, "  /BBox [0 0 %d %d]\n", n, m);
  fprintf(ctxcanvas->file, "  /XStep %d /YStep %d\n", n, m);
  fprintf(ctxcanvas->file, "  /PaintProc {\n");
  fprintf(ctxcanvas->file, "              pop\n");
  fprintf(ctxcanvas->file, "              %d %d 8\n", n, m);
  fprintf(ctxcanvas->file, "              matrix\n");
  fprintf(ctxcanvas->file, "              Pat\n");
  fprintf(ctxcanvas->file, "              false 3\n");
  fprintf(ctxcanvas->file, "              colorimage\n");
  fprintf(ctxcanvas->file, "             }\n");
  fprintf(ctxcanvas->file, ">>\n");
  fprintf(ctxcanvas->file, "matrix\n");
  fprintf(ctxcanvas->file, "makepattern\n");
  fprintf(ctxcanvas->file, "def\n");

  if (ctxcanvas->debug) fprintf(ctxcanvas->file, "%%cdPsMakePatternEnd\n");
}

static void long2rgb(cdCtxCanvas *ctxcanvas, int n, int i, int j, void* data, unsigned char*r, unsigned char*g, unsigned char*b)
{
  long* long_data = (long*)data;
  (void)ctxcanvas;
  cdDecodeColor(long_data[j*n+i], r, g, b);
}

static void cdpattern(cdCtxCanvas *ctxcanvas, int n, int m, const long int *pattern)
{
  if (ctxcanvas->level1)
    return;

  make_pattern(ctxcanvas, n, m, (void*)pattern, long2rgb);
}

static void uchar2rgb(cdCtxCanvas *ctxcanvas, int n, int i, int j, void* data, unsigned char*r, unsigned char*g, unsigned char*b)
{
  unsigned char* uchar_data = (unsigned char*)data;
  if (uchar_data[j*n+i])
    cdDecodeColor(ctxcanvas->canvas->foreground, r, g, b);
  else
    cdDecodeColor(ctxcanvas->canvas->background, r, g, b);
}

static void cdstipple(cdCtxCanvas *ctxcanvas, int n, int m, const unsigned char *stipple)
{
  if (ctxcanvas->level1)
    return;

  make_pattern(ctxcanvas, n, m, (void*)stipple, uchar2rgb);
}

static void ucharh2rgb(cdCtxCanvas *ctxcanvas, int n, int i, int j, void* data, unsigned char*r, unsigned char*g, unsigned char*b)
{
  unsigned char* uchar_data = (unsigned char*)data;
  static unsigned char hatch;
  (void)n;
  if (i == 0) hatch = uchar_data[j];
  if (hatch & 0x80)
    cdDecodeColor(ctxcanvas->canvas->foreground, r, g, b);
  else
    cdDecodeColor(ctxcanvas->canvas->background, r, g, b);
  _cdRotateHatch(hatch);
}

static int cdhatch(cdCtxCanvas *ctxcanvas, int style)
{
  if (ctxcanvas->level1)
    return ctxcanvas->canvas->hatch_style;

  make_pattern(ctxcanvas, 8, 8, (void*)HatchBits[style], ucharh2rgb);

  return style;
}

static void add_font_name(cdCtxCanvas *ctxcanvas, char *nativefontname)
{
  int size, i;
  for (i = 0; i < ctxcanvas->num_native_font; i++)
  {
    if (cdStrEqualNoCase(ctxcanvas->nativefontname[i], nativefontname))
      return;
  }

  size = strlen(nativefontname)+1;
  ctxcanvas->nativefontname[ctxcanvas->num_native_font] = (char*)malloc(size);
  memcpy(ctxcanvas->nativefontname[ctxcanvas->num_native_font], nativefontname, size);
  ctxcanvas->num_native_font++;
}

static char *findfont(const char* type_face, int style)
{
  static char font[1024];

  static char *type[] = 
  {
    "",              /* CD_PLAIN */
    "-Bold",         /* CD_BOLD */
    "-Oblique",      /* CD_ITALIC */
    "-BoldOblique",  /* CD_BOLD_ITALIC */
    
    "-Roman",        /* Plain p/ Times */
    "-Bold",         /* Bold p/ Times */
    "-Italic",       /* Italic p/ Times */
    "-BoldItalic"    /* BoldItalic p/ Times */
  };

  if (cdStrEqualNoCase(type_face, "System"))
    type_face = "Courier";

  if (cdStrEqualNoCase(type_face, "Times"))
	  style += 4;

  sprintf(font, "%s%s", type_face, type[style]);

  return font;
}

static int cdfont(cdCtxCanvas *ctxcanvas, const char *type_face, int style, int size)
{
  char *nativefontname = findfont(type_face, style&3); /* no underline or strikeout support */
  int size_pixel = cdGetFontSizePixels(ctxcanvas->canvas, size);
  fprintf(ctxcanvas->file, "%d /%s /%s-Latin1 ChgFnt\n", size_pixel, nativefontname, nativefontname);
  add_font_name(ctxcanvas, nativefontname);
  return 1;
}

static void cdtransform(cdCtxCanvas *ctxcanvas, const double* matrix)
{
  /* reset to identity */
  set_default_matrix(ctxcanvas);

  if (matrix)
  {
    fprintf(ctxcanvas->file, "[%g %g %g %g %g %g] concat\n", matrix[0], matrix[1], matrix[2], matrix[3], matrix[4], matrix[5]);
  }
  else
  {
    if (ctxcanvas->rotate_angle)
    {
      /* rotation = translate to point + rotation + translate back */
      fprintf(ctxcanvas->file, "%d %d translate\n", ctxcanvas->rotate_center_x, ctxcanvas->rotate_center_y);
      fprintf(ctxcanvas->file, "%g rotate\n", (double)ctxcanvas->rotate_angle);
      fprintf(ctxcanvas->file, "%d %d translate\n", -ctxcanvas->rotate_center_x, -ctxcanvas->rotate_center_y);
    }
  }
}

/******************************************************/
/* client images                                      */
/******************************************************/

static void cdputimagerectrgb(cdCtxCanvas *ctxcanvas, int iw, int ih, const unsigned char *r, const unsigned char *g, const unsigned char *b, int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax)
{
  int i, j, rw, rh;
  rw = xmax-xmin+1;
  rh = ymax-ymin+1;
  (void)ih;

  if (ctxcanvas->level1)
    return;

  if (ctxcanvas->debug) fprintf(ctxcanvas->file, "%%cdPutImageRectRGB Start\n");

  fprintf(ctxcanvas->file, "[0 0 0 0 0 0] currentmatrix\n");
  fprintf(ctxcanvas->file, "%d %d translate\n", x, y);
  fprintf(ctxcanvas->file, "%d %d scale\n", w, h);

  fprintf(ctxcanvas->file, "%d %d 8\n", rw, rh);
  fprintf(ctxcanvas->file, "[%d 0 0 %d 0 0]\n", rw, rh);
  fprintf(ctxcanvas->file, "{currentfile %d string readhexstring pop}\n", rw);
  fprintf(ctxcanvas->file, "false 3\n");
  fprintf(ctxcanvas->file, "colorimage\n");

  for (j=ymin; j<=ymax; j++)
  {
    for (i=xmin; i<=xmax; i++)
    {
      int pos = j*iw+i;
      fprintf(ctxcanvas->file, "%02x%02x%02x", (int)r[pos], (int)g[pos], (int)b[pos]);
    }

    fprintf(ctxcanvas->file, "\n");
  }

  fprintf(ctxcanvas->file, "setmatrix\n");

  if (ctxcanvas->eps)
  {
    bbox(ctxcanvas, x, y);
    bbox(ctxcanvas, x+rw-1, y+rh-1);
  }

  if (ctxcanvas->debug) fprintf(ctxcanvas->file, "%%cdPutImageRectRGBEnd\n");
}

static int isgray(int size, const unsigned char *index, const long int *colors)
{
  int i, pal_size = 0;
  unsigned char r, g, b;

  for (i = 0; i < size; i++)
  {
    if (index[i] > pal_size)
      pal_size = index[i];
  }

  pal_size++;

  for (i = 0; i < pal_size; i++)
  {
    cdDecodeColor(colors[i], &r, &g, &b);

    if (i != r || r != g || g != b)
      return 0;
  }

  return 1;
}

static void cdputimagerectmap(cdCtxCanvas *ctxcanvas, int iw, int ih, const unsigned char *index, const long int *colors, int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax)
{
  int i, j, rw, rh, is_gray;
  rw = xmax-xmin+1;
  rh = ymax-ymin+1;
  (void)ih;

  is_gray = isgray(iw*ih, index, colors);

  if (!is_gray && ctxcanvas->level1)
    return;

  if (ctxcanvas->debug) fprintf(ctxcanvas->file, "%%cdPutImageRectMap Start\n");

  fprintf(ctxcanvas->file, "[0 0 0 0 0 0] currentmatrix\n");
  fprintf(ctxcanvas->file, "%d %d translate\n", x, y);
  fprintf(ctxcanvas->file, "%d %d scale\n", w, h);

  fprintf(ctxcanvas->file, "%d %d 8\n", rw, rh);
  fprintf(ctxcanvas->file, "[%d 0 0 %d 0 0]\n", rw, rh);
  fprintf(ctxcanvas->file, "{currentfile %d string readhexstring pop}\n", rw);

  if (is_gray)
  {
    fprintf(ctxcanvas->file, "image\n");

    for (j=ymin; j<=ymax; j++)
    {
      for (i=xmin; i<=xmax; i++)
      {
        int pos = j*iw+i;
        fprintf(ctxcanvas->file, "%02x", (int)index[pos]);
      }

      fprintf(ctxcanvas->file, "\n");
    }
  }
  else
  {
    fprintf(ctxcanvas->file, "false 3\n");
    fprintf(ctxcanvas->file, "colorimage\n");

    for (j=ymin; j<=ymax; j++)
    {
      for (i=xmin; i<=xmax; i++)
      {
        int pos = j*iw+i;
        unsigned char r, g, b;
        cdDecodeColor(colors[index[pos]], &r, &g, &b);
        fprintf(ctxcanvas->file, "%02x%02x%02x", (int)r, (int)g, (int)b);
      }

      fprintf(ctxcanvas->file, "\n");
    }
  }

  fprintf(ctxcanvas->file, "setmatrix\n");

  if (ctxcanvas->eps)
  {
    bbox(ctxcanvas, x, y);
    bbox(ctxcanvas, x+rw-1, y+rh-1);
  }

  if (ctxcanvas->debug) fprintf(ctxcanvas->file, "%%cdPutImageRectMapEnd\n");
}

/******************************************************/
/* server images                                      */
/******************************************************/

static void cdpixel(cdCtxCanvas *ctxcanvas, int x, int y, long int color)
{
  if (ctxcanvas->debug) fprintf(ctxcanvas->file, "%%cdPixel Start\n");

  fprintf(ctxcanvas->file, "%g %g %g setrgbcolor\n",
          get_red(color), get_green(color), get_blue(color));

  if (ctxcanvas->level1)
  {
    fprintf(ctxcanvas->file, "N\n");
    fprintf(ctxcanvas->file, "%d %d 1 0 360 arc\n", x, y);
    fprintf(ctxcanvas->file, "C fill\n");
  }
  else
    fprintf(ctxcanvas->file, "%d %d 1 1 RF\n", x, y);

  if (ctxcanvas->eps) 
    bbox(ctxcanvas, x, y);

  if (ctxcanvas->debug) fprintf(ctxcanvas->file, "%%cdPixelEnd\n");
}

/******************************************************/
/* custom attributes                                  */
/******************************************************/

static void set_rotate_attrib(cdCtxCanvas *ctxcanvas, char* data)
{
  /* ignore ROTATE if transform is set, 
     because there is native support for transformations */
  if (ctxcanvas->canvas->use_matrix)
    return;

  if (data)
  {
    sscanf(data, "%g %d %d", &ctxcanvas->rotate_angle,
                             &ctxcanvas->rotate_center_x,
                             &ctxcanvas->rotate_center_y);
  }
  else
  {
    ctxcanvas->rotate_angle = 0;
    ctxcanvas->rotate_center_x = 0;
    ctxcanvas->rotate_center_y = 0;
  }

  cdtransform(ctxcanvas, NULL);
}

static char* get_rotate_attrib(cdCtxCanvas *ctxcanvas)
{
  static char data[100];

  if (!ctxcanvas->rotate_angle)
    return NULL;

  sprintf(data, "%g %d %d", (double)ctxcanvas->rotate_angle,
                            ctxcanvas->rotate_center_x,
                            ctxcanvas->rotate_center_y);

  return data;
}

static cdAttribute rotate_attrib =
{
  "ROTATE",
  set_rotate_attrib,
  get_rotate_attrib
}; 

static void set_cmd_attrib(cdCtxCanvas *ctxcanvas, char* data)
{
  fprintf(ctxcanvas->file, "%s", data);
}

static cdAttribute cmd_attrib =
{
  "CMD",
  set_cmd_attrib,
  NULL
}; 

static void set_poly_attrib(cdCtxCanvas *ctxcanvas, char* data)
{
  int hole;

  if (data == NULL)
  {
    ctxcanvas->holes = 0;
    return;
  }

  sscanf(data, "%d", &hole);
  ctxcanvas->poly_holes[ctxcanvas->holes] = hole;
  ctxcanvas->holes++;
}

static char* get_poly_attrib(cdCtxCanvas *ctxcanvas)
{
  static char holes[10];
  sprintf(holes, "%d", ctxcanvas->holes);
  return holes;
}

static cdAttribute poly_attrib =
{
  "POLYHOLE",
  set_poly_attrib,
  get_poly_attrib
}; 

/*
%F Cria um novo canvas PS
Parametros passados em data:
nome       nome do arquivo de saida <= 255 caracteres
-p[num]    tamanho do papel (A0-5, LETTER, LEGAL)
-w[num]    largura do papel em milimetros
-h[num]    altura do papel em milimetros
-l[num]    margem esquerda em milimetros
-r[num]    margem direita em milimetros
-b[num]    margem inferior em milimetros
-t[num]    margem superior em milimetros
-s[num]    resolucao em dpi
-e         encapsulated postscript
-1         level 1 operators only
-d[num]    margem da bbox em milimetros para eps
*/
static void cdcreatecanvas(cdCanvas* canvas, void *data)
{
  char *line = (char *)data;
  cdCtxCanvas *ctxcanvas;
  char filename[10240] = "";

  ctxcanvas = (cdCtxCanvas *)malloc(sizeof(cdCtxCanvas));
  memset(ctxcanvas, 0, sizeof(cdCtxCanvas));

  /* SVN specification states that number must use dot as decimal separator */
  ctxcanvas->old_locale = cdStrDup(setlocale(LC_NUMERIC, NULL));
  setlocale(LC_NUMERIC, "C");

  line += cdGetFileName(line, filename);
  if (filename[0] == 0)
    return;

  if ((ctxcanvas->file = fopen(filename, "w")) == NULL)
  {
    free(ctxcanvas);
    return;
  }

  ctxcanvas->holes = 0;
  cdRegisterAttribute(canvas, &poly_attrib);
  cdRegisterAttribute(canvas, &cmd_attrib);
  cdRegisterAttribute(canvas, &rotate_attrib);

  setpsdefaultvalues(ctxcanvas);

  while (*line != '\0')
  {
    while (*line != '\0' && *line != '-') 
      line++;

    if (*line != '\0')
    {
      float num;
      line++;
      switch (*line++)
      {
      case 'p':
        {
          int paper;
          sscanf(line, "%d", &paper);
          cdSetPaperSize(paper, &ctxcanvas->width_pt, &ctxcanvas->height_pt);
          break;
        }
      case 'w':
        sscanf(line, "%g", &num);
        ctxcanvas->width_pt = mm2pt(num);
        break;
      case 'h':
        sscanf(line, "%g", &num);
        ctxcanvas->height_pt = mm2pt(num);
        break;
      case 'l':
        sscanf(line, "%g", &num);
        ctxcanvas->xmin = num;
        break;
      case 'r':
        sscanf(line, "%g", &num);
        ctxcanvas->xmax = num;   /* right margin, must be converted to xmax */
        break;
      case 'b':
        sscanf(line, "%g", &num);
        ctxcanvas->ymin = num;  
        break;
      case 't':
        sscanf(line, "%g", &num);
        ctxcanvas->ymax = num;  /* top margin, must be converted to ymax */
        break;
      case 's':
        sscanf(line, "%d", &(ctxcanvas->res));
        break;
      case 'e':
        ctxcanvas->eps = 1;
        break;
      case 'o':
        ctxcanvas->landscape = 1;
        break;
      case '1':
        ctxcanvas->level1 = 1;
        break;
      case 'g':
        ctxcanvas->debug = 1;
        break;
      case 'd':
        sscanf(line, "%g", &num);
        ctxcanvas->bbmargin = num;
        break;
      }
    }

    while (*line != '\0' && *line != ' ') 
      line++;
  }

  /* store the base canvas */
  ctxcanvas->canvas = canvas;

  /* update canvas context */
  canvas->ctxcanvas = ctxcanvas;

  if (ctxcanvas->landscape)
  {
    _cdSwapDouble(ctxcanvas->width_pt, ctxcanvas->height_pt);
    _cdSwapDouble(ctxcanvas->xmin, ctxcanvas->ymin);
    _cdSwapDouble(ctxcanvas->xmax, ctxcanvas->ymax);
  }

  init_ps(ctxcanvas);
}

static void cdinittable(cdCanvas* canvas)
{
  canvas->cxFlush = cdflush;

  canvas->cxPixel = cdpixel;

  canvas->cxLine = cdline;
  canvas->cxPoly = cdpoly;
  canvas->cxRect = cdrect;
  canvas->cxBox = cdbox;
  canvas->cxArc = cdarc;
  canvas->cxSector = cdsector;
  canvas->cxChord = cdchord;
  canvas->cxText = cdtext;

  canvas->cxPutImageRectRGB = cdputimagerectrgb;
  canvas->cxPutImageRectMap = cdputimagerectmap;

  canvas->cxFLine = cdfline;
  canvas->cxFPoly = cdfpoly;
  canvas->cxFRect = cdfrect;
  canvas->cxFBox = cdfbox;
  canvas->cxFArc = cdfarc;
  canvas->cxFSector = cdfsector;
  canvas->cxFChord = cdfchord;
  canvas->cxFText = cdftext;

  canvas->cxClip = cdclip;
  canvas->cxFClipArea = cdfcliparea;
  canvas->cxLineStyle = cdlinestyle;
  canvas->cxLineWidth = cdlinewidth;
  canvas->cxLineCap = cdlinecap;
  canvas->cxLineJoin = cdlinejoin;
  canvas->cxPattern = cdpattern;
  canvas->cxStipple = cdstipple;
  canvas->cxHatch = cdhatch;
  canvas->cxFont = cdfont;
  canvas->cxTransform = cdtransform;
  canvas->cxKillCanvas = cdkillcanvas;
  canvas->cxDeactivate = cddeactivate;
}

static cdContext cdPSContext =
{
  CD_CAP_ALL & ~(CD_CAP_CLEAR | CD_CAP_PLAY | CD_CAP_PALETTE | 
                 CD_CAP_REGION | CD_CAP_IMAGESRV | 
                 CD_CAP_BACKGROUND | CD_CAP_BACKOPACITY | CD_CAP_WRITEMODE | 
                 CD_CAP_FONTDIM | CD_CAP_TEXTSIZE | 
                 CD_CAP_IMAGERGBA | CD_CAP_GETIMAGERGB),
  0,
  cdcreatecanvas,
  cdinittable,
  NULL,
  NULL,
};

cdContext* cdContextPS(void)
{
  return &cdPSContext;
}
