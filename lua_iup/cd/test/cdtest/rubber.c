/*=========================================================================*/
/* RUBBER.C - 10/12/95                                                     */
/* Funcoes para o desenho interativo das primitivas.                       */
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

/*- Parametros para o desenho das primitivas: -----------------------------*/
extern tLinePos line_pos;
extern tBoxPos box_pos;
extern tPixelPos pixel_pos;
extern tMarkPos mark_pos;
extern tArcPos arc_pos;

/*-------------------------------------------------------------------------*/
/* Segue as coordenadas do mouse atualizando o TXT apropriado.             */
/*-------------------------------------------------------------------------*/
void follow(int x, int y)
{
  static char mx[10], my[10];
  static char nx[10], ny[10];

  sprintf(nx, "%d", x);
  sprintf(ny, "%d", y);

  switch(ctgc.cur_prim) {
    case PIXEL:
      /* atualiza os parametros do pixel */
      pixel_pos.x = x;
      pixel_pos.y = y;
      /* atualiza a caixa de dialogo */
      sprintf(mx, "%d", x);
      sprintf(my, "%d", y);
      IupSetAttribute(IupGetHandle("txtPixelX"), IUP_VALUE, mx);
      IupSetAttribute(IupGetHandle("txtPixelY"), IUP_VALUE, my);
      break;
    case MARK:
      /* atualiza os parametros da mark */
      mark_pos.x = x;
      mark_pos.y = y;
      /* atualiza a caixa de dialogo */
      sprintf(mx, "%d", x);
      sprintf(my, "%d", y);
      IupSetAttribute(IupGetHandle("txtMarkX"), IUP_VALUE, mx);
      IupSetAttribute(IupGetHandle("txtMarkY"), IUP_VALUE, my);
      break;
    case RECT:
    case BOX:
      /* atualiza os parametros da box */
      if (ctgc.following) {
        if (x < box_pos.x) {
          box_pos.xmin = x;
          box_pos.xmax = box_pos.x;
        }
        else {
          box_pos.xmax = x;
          box_pos.xmin = box_pos.x;
        }

        if (y < box_pos.y) {
          box_pos.ymin = y;
          box_pos.ymax = box_pos.y;
        }
        else {
          box_pos.ymax = y;
          box_pos.ymin = box_pos.y;
        }
      }
      else {
        box_pos.xmax = box_pos.xmin = x;
        box_pos.ymax = box_pos.ymin = y;
      }  
      /* atualiza a caixa de dialogo */
      sprintf(mx, "%d", box_pos.xmin);
      sprintf(nx, "%d", box_pos.xmax);
      sprintf(my, "%d", box_pos.ymin);
      sprintf(ny, "%d", box_pos.ymax);
      IupSetAttribute(IupGetHandle("txtLBX1"), IUP_VALUE, mx);
      IupSetAttribute(IupGetHandle("txtLBX2"), IUP_VALUE, nx);
      IupSetAttribute(IupGetHandle("txtLBY1"), IUP_VALUE, my);
      IupSetAttribute(IupGetHandle("txtLBY2"), IUP_VALUE, ny);
      break;
    case LINE:
      sprintf(mx, "%d", x);
      sprintf(my, "%d", y);
      line_pos.x2 = x;
      line_pos.y2 = y;
      if (ctgc.following) {
        IupSetAttribute(IupGetHandle("txtLBX2"), IUP_VALUE, mx);
        IupSetAttribute(IupGetHandle("txtLBY2"), IUP_VALUE, my);
      }
      else {
        line_pos.x1 = x;
        line_pos.y1 = y;
        IupSetAttribute(IupGetHandle("txtLBX1"), IUP_VALUE, mx);
        IupSetAttribute(IupGetHandle("txtLBX2"), IUP_VALUE, mx);
        IupSetAttribute(IupGetHandle("txtLBY1"), IUP_VALUE, my);
        IupSetAttribute(IupGetHandle("txtLBY2"), IUP_VALUE, my);
      }
      break;
    case ARC:    /* ARC e SECTOR... */
    case CHORD: 
    case SECTOR: /* ...sao equivalentes */
      if (ctgc.following) {
        /* atualiza os parametros do arc */
        arc_pos.w = 2*abs(arc_pos.xc-x+1);
        arc_pos.h = 2*abs(arc_pos.yc-y+1);
        /* atualiza a caixa de dialogo */
        sprintf(mx, "%d", arc_pos.w);
        sprintf(my, "%d", arc_pos.h);
        IupSetAttribute(IupGetHandle("txtASW"), IUP_VALUE, mx);
        IupSetAttribute(IupGetHandle("txtASH"), IUP_VALUE, my);
      }
      else {
        /* atualiza os parametros do arc */
        arc_pos.xc = x;
        arc_pos.xc = y;
        /* atualiza a caixa de dialogo */
        sprintf(mx, "%d", x);
        sprintf(my, "%d", y);
        IupSetAttribute(IupGetHandle("txtASXC"), IUP_VALUE, mx);
        IupSetAttribute(IupGetHandle("txtASYC"), IUP_VALUE, my);
      }
      break;
    case TEXT:
      IupSetAttribute(IupGetHandle("txtTextX"), IUP_VALUE, nx);
      IupSetAttribute(IupGetHandle("txtTextY"), IUP_VALUE, ny);
      break;
    case CLIP:
      if (ctgc.following) {
        if (atoi(nx) >= atoi(mx)) {
          IupSetAttribute(IupGetHandle("txtClipXmax"), IUP_VALUE, nx);
          IupSetAttribute(IupGetHandle("txtClipXmin"), IUP_VALUE, mx);
        }
        else {
          IupSetAttribute(IupGetHandle("txtClipXmin"), IUP_VALUE, nx);
          IupSetAttribute(IupGetHandle("txtClipXmax"), IUP_VALUE, mx);
        }
        if (atoi(ny) >= atoi(my)) {
          IupSetAttribute(IupGetHandle("txtClipYmax"), IUP_VALUE, ny);
          IupSetAttribute(IupGetHandle("txtClipYmin"), IUP_VALUE, my);
        }
        else {
          IupSetAttribute(IupGetHandle("txtClipYmin"), IUP_VALUE, ny);
          IupSetAttribute(IupGetHandle("txtClipYmax"), IUP_VALUE, my);
        }
      }
      else {
        IupSetAttribute(IupGetHandle("txtClipXmin"), IUP_VALUE, nx);
        IupSetAttribute(IupGetHandle("txtClipYmin"), IUP_VALUE, ny);
        IupSetAttribute(IupGetHandle("txtClipXmax"), IUP_VALUE, nx);
        IupSetAttribute(IupGetHandle("txtClipYmax"), IUP_VALUE, ny);
        strcpy(mx, nx);
        strcpy(my, ny);
      }
      break;
    case IMAGE:
      if (ctgc.following) {
        if (atoi(mx) <= atoi(nx)) {
          sprintf(nx, "%d", abs(atoi(nx)-atoi(mx)));
          IupSetAttribute(IupGetHandle("txtImageW"), IUP_VALUE, nx);
        }
        else {
          IupSetAttribute(IupGetHandle("txtImageX"), IUP_VALUE, nx);
          sprintf(nx, "%d", abs(atoi(nx)-atoi(mx)));
          IupSetAttribute(IupGetHandle("txtImageW"), IUP_VALUE, nx);
        }
        if (atoi(my) <= atoi(ny)) {
          sprintf(ny, "%d", abs(atoi(ny)-atoi(my)));
          IupSetAttribute(IupGetHandle("txtImageH"), IUP_VALUE, ny);
        }
        else {
          IupSetAttribute(IupGetHandle("txtImageY"), IUP_VALUE, ny);
          sprintf(ny, "%d", abs(atoi(ny)-atoi(my)));
          IupSetAttribute(IupGetHandle("txtImageH"), IUP_VALUE, ny);
        }
      }
      else {
        IupSetAttribute(IupGetHandle("txtImageX"), IUP_VALUE, nx);
        IupSetAttribute(IupGetHandle("txtImageY"), IUP_VALUE, ny);
        strcpy(mx, nx);
        strcpy(my, ny);
      }
      break;
    case RGB:
      if (ctgc.following) {
        if (atoi(mx) <= atoi(nx)) {
          sprintf(nx, "%d", abs(atoi(nx)-atoi(mx)));
          IupSetAttribute(IupGetHandle("txtImageRGBW"), IUP_VALUE, nx);
        }
        else {
          IupSetAttribute(IupGetHandle("txtImageRGBX"), IUP_VALUE, nx);
          sprintf(nx, "%d", abs(atoi(nx)-atoi(mx)));
          IupSetAttribute(IupGetHandle("txtImageRGBW"), IUP_VALUE, nx);
        }
        if (atoi(my) <= atoi(ny)) {
          sprintf(ny, "%d", abs(atoi(ny)-atoi(my)));
          IupSetAttribute(IupGetHandle("txtImageRGBH"), IUP_VALUE, ny);
        }
        else {
          IupSetAttribute(IupGetHandle("txtImageRGBY"), IUP_VALUE, ny);
          sprintf(ny, "%d", abs(atoi(ny)-atoi(my)));
          IupSetAttribute(IupGetHandle("txtImageRGBH"), IUP_VALUE, ny);
        }
      }
      else {
        IupSetAttribute(IupGetHandle("txtImageRGBX"), IUP_VALUE, nx);
        IupSetAttribute(IupGetHandle("txtImageRGBY"), IUP_VALUE, ny);
        strcpy(mx, nx);
        strcpy(my, ny);
      }
      break;
    default:
      break;
  }
}


/*-------------------------------------------------------------------------*/
/* Desenha uma linha em rubber band.                                       */
/*-------------------------------------------------------------------------*/
void line(tRubber what, int x, int y)
{
  static int x1, x2, y1, y2;
  static int lastwhat = CLOSE;

  switch (what) {
    case NEWPOINT:
      x1 = x2 = x;                   /* novo segmento comeca no... */
      y1 = y2 = y;                   /* ...fim do primeiro */
      break;
    case MOVE:
      if (lastwhat == MOVE) {
        cdLine(x1, y1, x2, y2);      /* apaga o segmento velho */
      }
      cdLine(x1, y1, x, y);          /* desenha o novo */
      x2 = x;                        /* o novo se... */
      y2 = y;                        /* ...torna velho */
      break;
    case REPAINT:
      cdLine(x1, y1, x2, y2);        /* recupera o segmento perdido */
      return;                        /* nao modifica lastwhat */
    case CLOSE:
      cdLine(x1, y1, x2, y2);        /* apaga o ultimo segmento */
      break;
    default:
      break;
  }
  lastwhat = what;
}

/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
/* Desenha uma caixa vazia (funcao nao exportada).                         */
/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
static void frame(int x1, int y1, int x2, int y2)
{
  cdLine(x1, y1, x1, y2);
  cdLine(x2, y1, x2, y2);
  cdLine(x1, y1, x2, y1);
  cdLine(x1, y2, x2, y2);
}

/*-------------------------------------------------------------------------*/
/* Desenha uma caixa em rubber band.                                       */
/*-------------------------------------------------------------------------*/
void box(tRubber what, int x, int y)
{
  static int x1, x2, y1, y2;
  static int lastwhat = CLOSE;

  switch (what) {
    case NEWPOINT:
      x1 = x2 = x;                   /* novo segmento comeca no... */
      y1 = y2 = y;                   /* fim do primeiro */
      break;
    case MOVE:
      if (lastwhat == MOVE) {
        frame(x1, y1, x2, y2);       /* apaga a caixa anterior */
      }
      frame(x1, y1, x, y);           /* desenha a nova */
      x2 = x;                        /* o novo se... */
      y2 = y;                        /* torna velho */
      break;
    case REPAINT:
      frame(x1, y1, x2, y2);         /* restaura a caixa perdida */
      return;                        /* nao modifica lastwhat */
    case CLOSE:
      frame(x1, y1, x2, y2);         /* apaga a caixa definitiva */
      break;
    default:
      break;
  }
  lastwhat = what;
}

/*-------------------------------------------------------------------------*/
/* Desenha uma caixa centrada, em rubber band.                             */
/*-------------------------------------------------------------------------*/
void arc(tRubber what, int x, int y)
{
  static int xc, yc, y1, x1;
  static int lastwhat = CLOSE;

  switch (what) {
    case CENTER:
      xc = x1 = x;                   /* novo segmento comeca no... */
      yc = y1 = y;                   /* fim do primeiro */
      break;
    case MOVE:
      if (lastwhat == MOVE) {
        cdArc(xc, yc, 2*abs(xc-x1+1), 2*abs(yc-y1+1), 0, 360);
      }
      cdArc(xc, yc, 2*abs(xc-x+1), 2*abs(yc-y+1), 0, 360);
      x1 = x;                        /* o novo se... */
      y1 = y;                        /* torna velho */
      break;
    case REPAINT:
      cdArc(xc, yc, 2*abs(xc-x1+1), 2*abs(yc-y1+1), 0, 360);
      return;                        /* nao modifica lastwhat */
    case CLOSE:
      cdArc(xc, yc, 2*abs(xc-x1+1), 2*abs(yc-y1+1), 0, 360);
      break;
    default:
      break;
  }
  lastwhat = what;
}

/*-------------------------------------------------------------------------*/
/* Desenha o poligono em rubber band.                                      */
/*-------------------------------------------------------------------------*/
void polygon(tRubber what, int x, int y)
{
  static int x1, x2, y1, y2;
  static int lastwhat = CLOSE;

  switch (what) {
    case NEWPOINT:
      if (lastwhat != CLOSE) {
        cdLine(x1, y1, x2, y2);      /* ...apaga a anterior e... */
        cdLine(x1, y1, x, y);        /* desenha a definitiva */
      }
      x1 = x;                        /* novo segmento comeca no... */
      y1 = y;                        /* fim do primeiro */
      break;
    case MOVE:
      if (lastwhat == MOVE) {
        cdLine(x1, y1, x2, y2);      /* apaga o segmento velho */
      }
      cdLine(x1, y1, x, y);          /* desenha o novo */
      x2 = x;                        /* o novo se... */
      y2 = y;                        /* torna velho */
      break;
    case REPAINT:
      cdLine(x1, y1, x2, y2);        /* recupera o segmento perdido */
      return;                        /* nao modifica lastwhat */
    case CLOSE: 
      if (lastwhat != CLOSE) {
        int i;
        cdLine(x1, y1, x2, y2);        /* apaga o ultimo segmento */
        /* apaga o poligono temporario inteiro */
        for (i=0; (i<ctgc.num_points-1); i++) {
          cdLine(ctgc.points[i].x, ctgc.points[i].y,
                 ctgc.points[i+1].x, ctgc.points[i+1].y);
        }
      }
      break;
    default:
      break;
  }
  lastwhat = what;
}
