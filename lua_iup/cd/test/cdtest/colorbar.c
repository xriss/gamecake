/*=========================================================================*/
/* COLORBAR.C - 03/03/96                                                   */
/* Color Bar implementation.                                               */
/*=========================================================================*/

/*- Constantes: -----------------------------------------------------------*/
#define NUMCOLORS 16

/*- Bibliotecas padrao usadas ---------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

/*- Inclusao das bibliotecas IUP e CD: ------------------------------------*/
#include <iup.h>
#include <cd.h>
#include <cdiup.h>
#include <iupkey.h>

/*- Declaraccoes e Prototypes: --------------------------------------------*/
#include "cdtest.h"

#undef isdigit
#include <ctype.h>


/*- Globais: --------------------------------------------------------------*/
static struct {
  cdCanvas *bar_canvas;             /* canvas da colorbar */
  cdCanvas *other_canvas;           /* canvas ativo anteriormente */
  int w, h;                         /* dimensoes do canvas */
  int bgr, bgg, bgb;                /* cor de fundo do dialogo pai */
  int bci, fci;                     /* indice das cores correntes */
  long *p_foreground, *p_background;              /* variaveis do  usuario */
  long colors[NUMCOLORS];       /* palheta interna */
  int bounds[2*(NUMCOLORS+1)];      /* fronteiras dos elementos da palheta */ 
} colorbar;                         /* contexto da colorbar */

/*- Dialogo para mudancca da cor da palheta: ------------------------------*/
static struct {
  Ihandle *dialog;
  Ihandle *red;
  Ihandle *green;
  Ihandle *blue;
  Ihandle *alpha;
  Ihandle *bt_ok;
  Ihandle *bt_cancel;
  int to_change;
} color_change;

/*- Macros: ---------------------------------------------------------------*/
#define ignore(_) (void)(_)

/*-------------------------------------------------------------------------*/
/* Filtra inteiros.                                                        */
/*-------------------------------------------------------------------------*/
static int integer(Ihandle *self, int c)
{
  ignore(self);

  if (isdigit(c)) {
    return IUP_DEFAULT;
  }
  else if ((c==K_TAB) || (c==K_CR) || (c==K_LEFT) ||
           (c==K_RIGHT) || (c==K_DEL) || (c==K_BS) || (c==K_sTAB)) {
    return IUP_DEFAULT;
  }
  else {
    return IUP_IGNORE;
  }
}

/*-------------------------------------------------------------------------*/
/* Retorna a cor de indice i.                                              */
/*-------------------------------------------------------------------------*/
static long getcolor(int i)
{
  if (i<=NUMCOLORS) {
    return colorbar.colors[i-1];
  }
  else return 0;
}

/*-------------------------------------------------------------------------*/
/* Mostra a caixa de dialogo Color Change.                                 */
/*-------------------------------------------------------------------------*/
static void color_change_show(int c)
{
  unsigned char r, g, b, a;
  long color;
  char s[4];

  /* mostra o dialogo Color Change */
  IupShow(color_change.dialog);

  /* mostra a cor atual */
  color = getcolor(c);
  cdDecodeColor(color, &r, &g, &b);
  sprintf(s, "%d", r);
  IupSetAttribute(color_change.red, IUP_VALUE, s);
  sprintf(s, "%d", g);
  IupSetAttribute(color_change.green, IUP_VALUE, s);
  sprintf(s, "%d", b);
  IupSetAttribute(color_change.blue, IUP_VALUE, s);

  a = cdDecodeAlpha(color);
  sprintf(s, "%d", a);
  IupSetAttribute(color_change.alpha, IUP_VALUE, s);

  /* salva cor a ser alterada no contexto */
  color_change.to_change = c;
}


/*-------------------------------------------------------------------------*/
/* Cancela a operaccao de mudancca de cor.                                 */
/*-------------------------------------------------------------------------*/
static int color_change_cancel(void)
{
  IupHide(color_change.dialog);

  return IUP_DEFAULT;
}

/*-------------------------------------------------------------------------*/
/* Determina as fronteiras dos elementos da palheta.                       */
/*-------------------------------------------------------------------------*/
static void bounds(int dx, int p, int *x)
{
  int i;                                /* contador dos intervalos */
  int j;                                /* indice do vetor x */
  int k;                                /* inicio do intervalo */
  int e;                                /* erro da aproximacao inteira */
  int _2p;                              /* dobro do numero de intervalos */
  int q;                                /* quociente da divisao inteira */
  int _2r;                              /* dobro do resto da divisao inteira */

  /* inicializa as variaveis */
  k = 0;                                /* inicio do primeiro intervalo */
  j = 0;                                /* indice do vetor x */
  e = 0;                                /* inicializa o erro */
  _2p = p << 1;                         /* dobro do numero de intervalos */
  _2r = (dx % p) << 1;                  /* dobro do resto da divisao inteira */
  q = dx / p;                           /* quociente da divisao inteira */

  /* gera o vetor de intervalos */
  for (i=0; i<p; i++) {                 /* para p intervalos */
    e += _2r;                           /* incrementa o erro */
    x[j++] = k;                         /* inicio do intervalo */
    if (e >= p) {                       /* estourou? */
      e -= _2p;                         /* ajusta o novo limite */
      k += q + 1;                       /* arredonda para cima */
    } 
    else {
      k += q;                           /* arredonda para baixo */
    }
    x[j++] = k - 2;                     /* fim do intervalo */
  }
}

/*-------------------------------------------------------------------------*/
/* Acha a cor onde o mouse foi clicado por busca binaria. Retorna zero se  */ 
/* o click foi fora de qualquer cor.                                       */
/*-------------------------------------------------------------------------*/
static int findwhere(int x, int y, int c1, int c2)
{
  int mid;
  
  /* so pode ser este ou o click foi fora */
  if (c1 == c2) {
    if ((x > colorbar.bounds[c1<<1]) && (x < colorbar.bounds[(c1<<1) + 1])) {
      if ((y > 0) && (y < (colorbar.h-1))) return c1;
      else return 0;
    }
    else return 0;
  }
  
  /* elemento intermediario */
  mid = (c1 + c2)>>1;
  
  /* se o click estah a direita do elemento intermediario */
  if (x > colorbar.bounds[(mid<<1) + 1]) return findwhere(x, y, mid+1, c2);
  
  /* se estah a esquerda do elemento intermediario */
  else if (x < colorbar.bounds[mid<<1]) return findwhere(x, y, c1, mid-1);
  
  /* estah no meio do intermediario e a vertical estah legal, eh esse */
  else if ((y > 0) && (y < (colorbar.h-1))) return mid;

  /* o programa nunca chega aqui, mas o compilador fica feliz */
  return 0;
}

/*-------------------------------------------------------------------------*/
/* Desenha a moldura do elemento da palheta.                               */
/*-------------------------------------------------------------------------*/
static void hollowbox(int xmin, int xmax, int ymin, int ymax)
{
  cdForeground(cdEncodeColor(255, 255, 255));
  cdLine(xmin, ymin, xmax, ymin);
  cdLine(xmax, ymin+1, xmax, ymax);

  cdForeground(cdEncodeColor(102, 102, 102));
  cdLine(xmin, ymin+1, xmin, ymax);
  cdLine(xmin+1, ymax, xmax-1, ymax);
}


/*-------------------------------------------------------------------------*/
/* Muda a cor de indice i e retorna a anterior.                            */
/*-------------------------------------------------------------------------*/
static long changecolor(int i, long color)
{
  long temp;

  if (i<=NUMCOLORS) {
    temp = colorbar.colors[i-1];
    colorbar.colors[i-1] = color;
    return temp;
  }
  else return 0;
}


/*-------------------------------------------------------------------------*/
/* Inicializa o vetor de cores.                                            */
/*-------------------------------------------------------------------------*/
static void resetcolors(void)
{
  colorbar.colors[ 0] = cdEncodeColor(  0,   0,   0);
  colorbar.colors[ 1] = cdEncodeColor(153, 153, 153);
  colorbar.colors[ 2] = cdEncodeColor(178, 178, 178);
  colorbar.colors[ 3] = cdEncodeColor(255, 255, 255);
  colorbar.colors[ 4] = cdEncodeColor(255, 255,   0);
  colorbar.colors[ 5] = cdEncodeColor(255,   0,   0);
  colorbar.colors[ 6] = cdEncodeColor(255,   0, 255);
  colorbar.colors[ 7] = cdEncodeColor(  0, 255, 255);
  colorbar.colors[ 8] = cdEncodeColor(  0,   0, 255);
  colorbar.colors[ 9] = cdEncodeColor(  0, 255,   0);
  colorbar.colors[10] = cdEncodeColor(128, 128,   0);
  colorbar.colors[11] = cdEncodeColor(128,   0,   0);
  colorbar.colors[12] = cdEncodeColor(128,   0, 128);
  colorbar.colors[13] = cdEncodeColor(  0, 128, 128);
  colorbar.colors[14] = cdEncodeColor(  0,   0, 128);
  colorbar.colors[15] = cdEncodeColor(  0, 128,   0);
}

/*-------------------------------------------------------------------------*/
/* Desenha a ColorBar.                                                     */
/*-------------------------------------------------------------------------*/
static int fColorBarRepaint(void)
{
  int i;
  double dt;

  /* salva o canvas ativo no momento */
  colorbar.other_canvas = cdActiveCanvas();

  /* ativa o canvas da colorbar */
  if (cdActivate(colorbar.bar_canvas) == CD_ERROR) {
    printf("Color Bar Error: Unable to activate canvas.");
    return 1;
  }
  cdClear();

  /* desenha as cores da palheta */
  for (i=2; i<=2*NUMCOLORS; i+=2) {
    cdForeground(getcolor(i>>1));
    cdBox(colorbar.bounds[i], colorbar.bounds[i+1], 1, colorbar.h-1);
    hollowbox(colorbar.bounds[i], colorbar.bounds[i+1], 1, colorbar.h-1);
  }

  /* desenha o fedback */
  dt = (colorbar.w-1)/(NUMCOLORS+1);
  /* desenha a cor de fundo */
  cdForeground(getcolor(colorbar.bci));
  cdBox(1, (int)(2.0*dt/3.0), 1, (int)(2.0*(colorbar.h-1)/3.0));
  hollowbox(1, (int)(2.0*dt/3.0), 1, (int)(2.0*(colorbar.h-1)/3.0));
  /* desenha a cor de frente */
  cdForeground(getcolor(colorbar.fci));
  cdBox((int)(dt/3.0), (int)(dt)-1, (int)((colorbar.h-1)/3.0)+1, colorbar.h-1);
  hollowbox((int)(dt/3.0), (int)(dt)-1, (int)((colorbar.h-1)/3.0)+1, colorbar.h-1);
 
  /* restaura o canvas anteriormente ativo */
  cdActivate(colorbar.other_canvas);

  return IUP_DEFAULT;
}

/*-------------------------------------------------------------------------*/
/* Funcao associada ao botao do mouse.                                     */
/*-------------------------------------------------------------------------*/
static int fColorBarButtonCB(Ihandle *self, int b, int e, int x, int y, char *r)
{
  int c;
 
  ignore(self);
  ignore(r);

  /* salva o canvas ativo no momento */
  colorbar.other_canvas = cdActiveCanvas();

  /* ativa o canvas da colorbar */
  if (cdActivate(colorbar.bar_canvas) == CD_ERROR) {
    printf("Color Bar Error: Unable to activate canvas.");
    return 1;
  }

  /* converte para coordenadas do canvas */
  cdUpdateYAxis(&y);

  /* se o botao foi pressionado */
  if (e) {

    /* acha onde foi o click */
    c = findwhere(x, y, 1, 16);
    
    /* se o click foi dentro de alguma cor... */
    if (c != 0) {

      /* botao da esquerda eh mudancca de cor de foreground */
      if (b == IUP_BUTTON1) {

        /* se for double-click */
        if ((isdouble(r)) && (c == colorbar.fci) ) {

          /* mostra o dialogo */
          color_change_show(c);
        }

        /* muda a cor de frente corrente */
        else {

          /* largura de cada celula */
          double dt;
          unsigned char r, g, b;
  
          /* altera a variavel do usuario */
          *(colorbar.p_foreground) = getcolor(c);
       
          /* altera o indice da cor de frente corrente */
          colorbar.fci = c;

          cdDecodeColor(getcolor(colorbar.fci), &r, &g, &b);
          sprintf(ctgc.status_line, "cdForeground(cdEncodeColor(%d, %d, %d))", (int)r, (int)g, (int)b);
          set_status();

          /* altera o feedback no primeiro elemento da palheta */
          dt = (colorbar.w-1)/(NUMCOLORS+1);
          cdForeground(getcolor(colorbar.fci));
          cdBox((int)(dt/3.0), (int)(dt)-1, (int)((colorbar.h-1)/3.0)+1, colorbar.h-1);
          hollowbox((int)(dt/3.0), (int)(dt)-1, (int)((colorbar.h-1)/3.0)+1, colorbar.h-1);
        }

      }

      else if (b == IUP_BUTTON3) {
          
        /* largura de cada celula */
        double dt;
        unsigned char r, g, b;

        /* altera a variavel do usuario */
        *(colorbar.p_background) = getcolor(c);
      
        /* altera o indice da cor de frente corrente */
        colorbar.bci = c;
     
        cdDecodeColor(getcolor(colorbar.bci), &r, &g, &b);
        sprintf(ctgc.status_line, "cdBackground(cdEncodeColor(%d, %d, %d))", (int)r, (int)g, (int)b);
        set_status();

        /* altera o feedback no primeiro elemento da palheta */
        dt = (colorbar.w-1)/(NUMCOLORS+1);
        cdForeground(getcolor(colorbar.bci));
        cdBox(1, (int)(2.0*dt/3.0), 1, (int)(2.0*(colorbar.h-1)/3.0));
        hollowbox(1, (int)(2.0*dt/3.0), 1, (int)(2.0*(colorbar.h-1)/3.0));
        cdForeground(getcolor(colorbar.fci));
        cdBox((int)(dt/3.0), (int)(dt)-1, (int)((colorbar.h-1)/3.0)+1, colorbar.h-1);
        hollowbox((int)(dt/3.0), (int)(dt)-1, (int)((colorbar.h-1)/3.0)+1, colorbar.h-1);
      }

    }

  }

  /* restaura o canvas anteriormente ativo */
  cdActivate(colorbar.other_canvas);
  
  return IUP_DEFAULT;
}

/*-------------------------------------------------------------------------*/
/* Callback associado ao resize do canvas.                                 */
/*-------------------------------------------------------------------------*/
static int fColorBarResizeCB(Ihandle *self, int w, int h)
{
  ignore(self);

  /* atualiza as dimensoes do canvas */
  colorbar.w = w;
  colorbar.h = h;

  /* atualiza as fronteiras dos elementos da palheta */
  bounds(colorbar.w, NUMCOLORS+1, colorbar.bounds); 

  return IUP_DEFAULT;
}

/*-------------------------------------------------------------------------*/
/* Confirma a mudanca de cores.                                            */
/*-------------------------------------------------------------------------*/
static int color_change_ok(void)
{
  int r, g, b, a;
  long new_color;

  /* pega os novos valores */
  r = IupGetInt(color_change.red, IUP_VALUE);
  g = IupGetInt(color_change.green, IUP_VALUE);
  b = IupGetInt(color_change.blue, IUP_VALUE);
  a = IupGetInt(color_change.alpha, IUP_VALUE);

  /* se todos forem validos */
  if ((r<256)&&(g<256)&&(b<256)) {

    /* esconde a caixa de dialogo */
    IupHide(color_change.dialog);

    /* atualiza a cor no contexto */ 
    new_color = cdEncodeColor((unsigned char)r, (unsigned char) g, (unsigned char) b);
    new_color = cdEncodeAlpha(new_color, (unsigned char)a);
    changecolor(color_change.to_change, new_color);

    colorbar.fci = color_change.to_change;

    /* redesenha a colorbar */
    fColorBarRepaint();

    /* altera a variavel do usuario */
    *(colorbar.p_foreground) = new_color;

  }

  return IUP_DEFAULT;
}

/*-------------------------------------------------------------------------*/
/* Inicializa a ColorBar.                                                  */
/*-------------------------------------------------------------------------*/
void ColorBarClose(void)
{
  cdKillCanvas(colorbar.bar_canvas);
  IupDestroy(color_change.dialog);
}

int ColorBarInit(Ihandle *parent, Ihandle *canvas, long *foreground, long *background)
{
  char *bg_color;

  /* pega a cor de fundo do dialogo parente */
  bg_color = IupGetAttribute(parent, "BGCOLOR");
  if (bg_color == NULL) {
    printf("Color Bar Error: Unable to get bg_color.");
    return 0;
  }
  sscanf(bg_color, "%d %d %d", &colorbar.bgr, &colorbar.bgg, &colorbar.bgb);

  /* inicializa as cores de fundo e de frente defaults */
  colorbar.fci = 1;
  colorbar.bci = 4;

  /* pega o enderecco das variaveis do usuario */
  colorbar.p_foreground = foreground;
  colorbar.p_background = background;

  /* inicializa a palheta interna de cores */
  resetcolors();

  /* cria o canvas do CD */
  colorbar.bar_canvas = cdCreateCanvas(CD_IUP, canvas);
  if (colorbar.bar_canvas == NULL) {
    printf("Color Bar Error: Unable to create canvas.");
    return 0;
  }

  /* salva o canvas ativo no momento */
  colorbar.other_canvas = cdActiveCanvas();

  /* ativa o canvas da colorbar */
  if (cdActivate(colorbar.bar_canvas) == CD_ERROR) {
    printf("Color Bar Error: Unable to activate canvas.");
    return 0;
  }

  /* pega as dimensoes do canvas pela primeira vez */
  cdGetCanvasSize(&colorbar.w, &colorbar.h, NULL, NULL);

  /* restaura o canvas anteriormente ativo */
  if (colorbar.other_canvas != NULL) cdActivate(colorbar.other_canvas);

  /* cria o vetor com as fronteiras dos elementos da palheta */
  bounds(colorbar.w, NUMCOLORS+1, colorbar.bounds); 
  
  /* associa os callbacks */
  IupSetFunction("cmdColorBarButtonCB", (Icallback) fColorBarButtonCB);
  IupSetFunction("cmdColorBarRepaint", (Icallback) fColorBarRepaint);
  IupSetFunction("cmdColorBarResizeCB", (Icallback) fColorBarResizeCB);

  /* desenha a barra de cores pela primeira vez */
  fColorBarRepaint();

  /* inicializa o dialogo de troca de cores da palheta */
  color_change.dialog = IupDialog(
    IupVbox(
      IupHbox(
        IupLabel("R:"),
        color_change.red = IupText("cmdInteger"),
        IupLabel("G:"),
        color_change.green = IupText("cmdInteger"),
        IupLabel("B:"),
        color_change.blue = IupText("cmdInteger"),
        IupLabel("A:"),
        color_change.alpha = IupText("cmdInteger"),
        NULL
      ),
      IupHbox(
        IupFill(),
        color_change.bt_ok = IupButton("OK", "cmdColorChangeOK"),
        color_change.bt_cancel = IupButton("Cancel", "cmdColorChangeCancel"),
        IupFill(),
        NULL
      ),
      NULL
    )
  );

  /* atributos do dialogo */
  IupSetAttribute(color_change.dialog, IUP_TITLE, "Color Change:");
  IupSetAttribute(color_change.dialog, IUP_MARGIN, "5x5");
  IupSetAttribute(color_change.dialog, IUP_GAP, "5");
  IupSetAttribute(color_change.dialog, "MAXBOX", "NO");
  IupSetAttribute(color_change.dialog, "MINBOX", "NO");
  IupSetAttribute(color_change.dialog, "PARENTDIALOG", "dlgMain");

  /* atributos dos texts */
  IupSetFunction("cmdInteger", (Icallback) integer);
  IupSetAttribute(color_change.red, "NC", "3");
  IupSetAttribute(color_change.green, "NC", "3");
  IupSetAttribute(color_change.blue, "NC", "3");
  IupSetAttribute(color_change.alpha, "NC", "3");
  IupSetAttribute(color_change.red, "SIZE", "24");
  IupSetAttribute(color_change.green, "SIZE", "24");
  IupSetAttribute(color_change.blue, "SIZE", "24");
  IupSetAttribute(color_change.alpha, "SIZE", "24");
  
  /* atributos dos botoes */
  IupSetAttribute(color_change.bt_ok, IUP_SIZE, "30");
  IupSetAttribute(color_change.bt_cancel, IUP_SIZE, "30");
  IupSetFunction("cmdColorChangeCancel", (Icallback) color_change_cancel);
  IupSetFunction("cmdColorChangeOK", (Icallback) color_change_ok);

  return 1;
}        
