/*=========================================================================*/
/* LIST.C 10/12/95                                                         */
/* Funcoes para a manipulacao da lista de primitivas.                      */
/*=========================================================================*/

/*- Bibliotecas padrao usadas ---------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

/*- Inclusao das bibliotecas IUP e CD: ------------------------------------*/
#include <iup.h>
#include <cd.h>
#include <cdiup.h>

/*- Prototypes e declaracoes do CD Test: ----------------------------------*/
#include "cdtest.h"

/*- Contexto do CD Test (declarado em CDTEST.C): --------------------------*/
extern tCTC ctgc;

/*-------------------------------------------------------------------------*/
/* Adiciona um ponto ao poligono temporario corrente.                      */
/*-------------------------------------------------------------------------*/
int newpolypoint(int x, int y)
{
  if (ctgc.num_points <= MAXPOINTS) {
    ctgc.points[ctgc.num_points].x = x;
    ctgc.points[ctgc.num_points].y = y;
    ctgc.num_points++;
    return TRUE;
  }
  else return FALSE;
}

tList* newNode(void)
{
  tList *newnode = (tList *) malloc(sizeof (tList));
  newnode->next = NULL;

  if (ctgc.head != NULL) 
  {
    tList *temp;
    for(temp = ctgc.head; temp->next; temp = (tList *)temp->next);
    temp->next = newnode;        /* coloca o novo item no final da lista */
  }
  else 
    ctgc.head = newnode;

  return newnode;
}

/*-------------------------------------------------------------------------*/
/* Adiciona uma linha/rect/caixa a lista de primitivas.                    */
/*-------------------------------------------------------------------------*/

static int newtLB(int x1, int y1, int x2, int y2, int type)
{
  tList* newnode = newNode();

  newnode->type = type;
  newnode->par.lineboxpar.x1 = x1;
  newnode->par.lineboxpar.x2 = x2;
  newnode->par.lineboxpar.y1 = y1;
  newnode->par.lineboxpar.y2 = y2;
  newnode->par.lineboxpar.foreground = ctgc.foreground;
  newnode->par.lineboxpar.background = ctgc.background;
  newnode->par.lineboxpar.line_style = ctgc.line_style;
  newnode->par.lineboxpar.line_width = ctgc.line_width;
  newnode->par.lineboxpar.write_mode = ctgc.write_mode;
  newnode->par.lineboxpar.line_cap = ctgc.line_cap;
  newnode->par.lineboxpar.line_join = ctgc.line_join;
  newnode->par.lineboxpar.interior_style = ctgc.interior_style;
  newnode->par.lineboxpar.back_opacity = ctgc.back_opacity;
  newnode->par.lineboxpar.hatch = ctgc.hatch;

  return TRUE;                 
}

int newline(int x1, int y1, int x2, int y2)
{
  return newtLB(x1, y1, x2, y2, LINE);
}

int newrect(int x1, int x2, int y1, int y2)
{
  return newtLB(x1, y1, x2, y2, RECT);
}

int newbox(int x1, int x2, int y1, int y2)
{
  return newtLB(x1, y1, x2, y2, BOX);
}

/*-------------------------------------------------------------------------*/
/* Adiciona um arc/sector/chord a lista de primitivas.                     */
/*-------------------------------------------------------------------------*/
static int newtAS(int xc, int yc, int w, int h, double angle1, double angle2, int type)
{
  tList* newnode = newNode();

  newnode->type = type;
  newnode->par.arcsectorpar.xc = xc;
  newnode->par.arcsectorpar.yc = yc;
  newnode->par.arcsectorpar.w = w;
  newnode->par.arcsectorpar.h = h;
  newnode->par.arcsectorpar.angle1 = angle1;
  newnode->par.arcsectorpar.angle2 = angle2;
  newnode->par.arcsectorpar.foreground = ctgc.foreground;
  newnode->par.arcsectorpar.background = ctgc.background;
  newnode->par.arcsectorpar.line_style = ctgc.line_style;
  newnode->par.arcsectorpar.line_width = ctgc.line_width;
  newnode->par.arcsectorpar.line_cap = ctgc.line_cap;
  newnode->par.arcsectorpar.line_join = ctgc.line_join;
  newnode->par.arcsectorpar.write_mode = ctgc.write_mode;
  newnode->par.arcsectorpar.interior_style = ctgc.interior_style;
  newnode->par.arcsectorpar.back_opacity = ctgc.back_opacity;
  newnode->par.arcsectorpar.hatch = ctgc.hatch;

  return TRUE;
}

int newarc(int xc, int yc, int w, int h, double angle1, double angle2)
{
  return newtAS(xc, yc, w, h, angle1, angle2, ARC);
}

int newsector(int xc, int yc, int w, int h, double angle1, double angle2)
{
  return newtAS(xc, yc, w, h, angle1, angle2, SECTOR);
}

int newchord(int xc, int yc, int w, int h, double angle1, double angle2)
{
  return newtAS(xc, yc, w, h, angle1, angle2, CHORD);
}

/*-------------------------------------------------------------------------*/
/* Adiciona um pixel a lista de primitivas.                                */
/*-------------------------------------------------------------------------*/
int newpixel(int x, int y)
{
  tList* newnode = newNode();

  newnode->type = PIXEL;
  newnode->par.pixelpar.x = x;
  newnode->par.pixelpar.y = y;
  newnode->par.pixelpar.foreground = ctgc.foreground;
  newnode->par.pixelpar.write_mode = ctgc.write_mode;

  return TRUE;
}

/*-------------------------------------------------------------------------*/
/* Adiciona um metafile a lista de primitivas.                             */
/*-------------------------------------------------------------------------*/
int newmetafile(char *filename, cdContext* ctx)
{
  tList* newnode = newNode();

  newnode->type = META;
  newnode->par.metapar.filename = (char *) malloc(strlen(filename) + 1);
  newnode->par.metapar.ctx = ctx;
  strcpy(newnode->par.metapar.filename, filename);

  return TRUE;
}

/*-------------------------------------------------------------------------*/
/* Adiciona uma marca a lista de primitivas.                               */
/*-------------------------------------------------------------------------*/
int newmark(int x, int y, int mark_size)
{
  tList* newnode = newNode();

  newnode->type = MARK;
  newnode->par.markpar.x = x;
  newnode->par.markpar.y = y;
  newnode->par.markpar.mark_size = mark_size;
  newnode->par.markpar.foreground = ctgc.foreground;
  newnode->par.markpar.write_mode = ctgc.write_mode;
  newnode->par.markpar.mark_type = ctgc.mark_type;

  return TRUE;
}

/*-------------------------------------------------------------------------*/
/* Adiciona um texto a lista de primitivas.                                */
/*-------------------------------------------------------------------------*/
int newtext(int x, int y, char *s)
{
  tList* newnode = newNode();

  newnode->type = TEXT;
  newnode->par.textpar.x = x;
  newnode->par.textpar.y = y;
  newnode->par.textpar.foreground = ctgc.foreground;
  newnode->par.textpar.background = ctgc.background;
  newnode->par.textpar.font_style = ctgc.font_style;
  newnode->par.textpar.font_typeface = ctgc.font_typeface;
  newnode->par.textpar.font_size = ctgc.font_size;
  newnode->par.textpar.write_mode = ctgc.write_mode;
  newnode->par.textpar.back_opacity = ctgc.back_opacity;
  newnode->par.textpar.text_alignment = ctgc.text_alignment;
  newnode->par.textpar.text_orientation = ctgc.text_orientation;
  newnode->par.textpar.s = (char *) malloc(strlen(s) + 1);
  strcpy(newnode->par.textpar.s, s);

  return TRUE;
}

/*-------------------------------------------------------------------------*/
/* Adiciona um poligono a lista de primitivas.                             */
/*-------------------------------------------------------------------------*/
int newpoly(void)
{
  tList* newnode = newNode();

  newnode->type = POLY;
  newnode->par.polypar.foreground = ctgc.foreground;
  newnode->par.polypar.background = ctgc.background;
  newnode->par.polypar.line_style = ctgc.line_style;
  newnode->par.polypar.line_width = ctgc.line_width;
  newnode->par.polypar.write_mode = ctgc.write_mode;
  newnode->par.polypar.fill_mode = ctgc.fill_mode;
  newnode->par.polypar.line_cap = ctgc.line_cap;
  newnode->par.polypar.line_join = ctgc.line_join;
  newnode->par.polypar.poly_mode = ctgc.poly_mode;
  newnode->par.polypar.interior_style = ctgc.interior_style;
  newnode->par.polypar.back_opacity = ctgc.back_opacity;
  newnode->par.polypar.hatch = ctgc.hatch;
  newnode->par.polypar.points = (tPoint *) malloc(ctgc.num_points*sizeof(tPoint));
  newnode->par.polypar.num_points = ctgc.num_points;
  memcpy(newnode->par.polypar.points, ctgc.points, ctgc.num_points*sizeof(tPoint));
  newnode->next = NULL;

  return TRUE;
}

/*-------------------------------------------------------------------------*/
/* Mata a lista de primitivas.                                             */
/*-------------------------------------------------------------------------*/
void dellist(void)
{
  tList *killer, *back;

  for (killer = ctgc.head; killer; killer = back) {
    back = (tList *) (killer->next);
    if (killer->type == TEXT) {         /* se for TEXT... */
      free(killer->par.textpar.s);      /* ...mata a string */
    }
    if (killer->type == POLY) {         /* se for POLY... */
      free(killer->par.polypar.points); /* ...mata os pontos */
    }
    free(killer);
  }
  ctgc.head = NULL;
}

/*-------------------------------------------------------------------------*/
/* Mata a ultima primitiva.                                                */
/*-------------------------------------------------------------------------*/
void dellast(void)
{
  tList *killer;

  if (ctgc.head == NULL ) return;
  else if (ctgc.head->next == NULL) {
    free(ctgc.head);
    ctgc.head = NULL;
  }
  else {
    for (killer = ctgc.head; killer->next->next; killer = killer->next);
    free(killer->next);
    killer->next = NULL;
  }
}
