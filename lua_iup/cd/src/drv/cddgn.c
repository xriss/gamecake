/** \file
 * \brief DGN driver
 *
 * See Copyright Notice in cd.h
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>

#include "cd.h"
#include "cd_private.h"
#include "cddgn.h"

/* defines */

#define MAX_NUM_VERTEX 101
#define MAX_NUM_VERTEX_PER_POLYLINE 15000

#ifndef PI
#define PI 3.14159265358979323846
#endif

#define END_OF_DGN_FILE 0xffff
#define DGN_FILE_BLOCK  512

#define NOFILL 0    /* tipos de fill que o driver faz */
#define CONVEX 1
#define NORMAL 2 

/* macros */

#define SIZE_LINE_STRING(x)  (19+4*(x))
#define SIZE_FILLED_SHAPE(x) (27+4*(x))
#define SIZE_ARC             40
#define SIZE_LINE            26

#define IGNORE(x) ((void) x)


/* estruturas similares as que o MicroStation usa */

typedef struct
{
  union 
  {
    struct 
    {  
      unsigned level:6;
      unsigned :1;
      unsigned complex:1;
      unsigned type:7;
      unsigned :1;
    } flags;
    short type_as_word;
  } type;

  unsigned short words;
  unsigned long  xmin;
  unsigned long  ymin;
  unsigned long  xmax;
  unsigned long  ymax;
} Elm_hdr; 

typedef struct
{
  short attindx;
  union
  {
    short s;
    struct
    {
      unsigned /*class*/ :4;
      unsigned /*res*/   :4;
      unsigned /*l*/     :1;
      unsigned /*n*/     :1;
      unsigned /*m*/     :1;
      unsigned attributes:1;
      unsigned /*r*/     :1;
      unsigned /*p*/     :1;
      unsigned /*s*/     :1;
      unsigned /*hole*/  :1;
    } flags;
  } props;

  union
  {
    short s;
    struct
    {
      unsigned style:3;
      unsigned weight:5;
      unsigned color:8;
    } b;
  } symb;

} Disp_hdr;

/* tipo de poligono/setor */
enum
{
  FILLED,
  OPEN 
};

/* grupos de tamanhos de caracter 
   (usado por gettextwidth e cdgetfontdim) */

static long fontsizes[4][8]=
{
  {1,2,4,5,6,7,0},
  {5,3,2,1,0},
  {8,6,4,3,2,1,0},
  {5,3,2,1,0}
};


/**********************
 * contexto do driver *
 **********************/

struct _cdCtxCanvas
{
  cdCanvas* canvas;

  FILE *file;                             /* arquivo dgn */
  long int bytes;                            /* tamanho do arquivo */
  char level;
  
  short color, style;                 

  short alignment;
  short typeface_index;
  short symbology;
  long tl;
  short is_base;                               /* setado se texto e' do tipo CD_BASE_...   */

  short fill_type;                   /* como o driver faz fill:
                                           NOFILL -> nao faz fill
                                           CONVEX -> so faz fill de poligonos convexos
                                           NORMAL -> faz fill normalmente */

  long colortable[256];              /* palette */
  short num_colors;

  short is_complex;
};

/* prototipos de funcao */
static void startComplexShape(cdCtxCanvas*, unsigned short,short,unsigned short,
                            unsigned long,unsigned long,
                            unsigned long,unsigned long);
static void endComplexElement(cdCtxCanvas*);

/******************************
 *                            *
 * funcoes internas do driver *
 *                            *
 ******************************/

/*********************************************************
 * Obtem o descent do texto (para letras como q,p,g etc) * 
 *********************************************************/

static long get_descent(const char *text, int len, int size_pixel)
{
 char *descent="jgyqp";
 long  a=0; 

 while(a < len)
 {
   if(strchr(descent, text[a]))
     return size_pixel/2;

   a++;
 }
 return 0;
}

/***********************************************
 * Calcula a largura da string no MicroStation *
 ***********************************************/

static long gettextwidth(cdCtxCanvas* ctxcanvas, const char *s, int len, int size_pixel)
{
  long a=0,
       width=0;

  short default_size=0;

  static char *fontchars[4][8] =
  {
    {         /* CD_SYSTEM */
      "Ww",
      "jshyut#*-=<>",
      "iIl[]",
      ";:.,'|!()`{}",
      "","","",""
    }, 

    { /* CD_COURIER */
      "Iflrit!();.'",
      "1|[]\"/`",
      "BCDEKPRSUVXYbdgkpq&-_", 
      "w#%", 
      "Wm^+=<>~",
      "@","",""
    }, 

    { /* CD_TIMES_ROMAN */
      "m",
      "HMUWw",
      "CSTZLbhknpuvxy23567890e",
      "fstz1#$-=<>", 
      "Iijl*[]", 
      ";:.,'|!()`{}",
      "",""
    },
    
    { /* CD_HELVETICA */
      "Ww",
      "jshyut#*-=<>",
      "iIl[]", 
      ";:.,'|!()`{}",
      "","","",""
    }  
  };

  if (ctxcanvas->typeface_index == 1)
    default_size=2;
  else if (ctxcanvas->typeface_index == 2)
    default_size=5;
  else if (ctxcanvas->typeface_index == 3)
    default_size=4;
  else
    default_size=4;

  for(a=0,width=0;a < len; a++)
  { 
    static short size_number;
    static char letter;
   
    if(s[a] == ' ')
      letter = s[a-1];
    else
      letter = s[a];
      
    for(size_number=0;size_number < 8;size_number++)
    {
      if(strchr(fontchars[ctxcanvas->typeface_index][size_number], letter))
      {
        width+=(ctxcanvas->tl*fontsizes[ctxcanvas->typeface_index][size_number])/6;
        break;
      }
    }
    
    if(size_number == 8)
      width+=(ctxcanvas->tl*default_size)/6;

    width+=ctxcanvas->tl/3;
  }
  
  width-=ctxcanvas->tl/3;

  if (ctxcanvas->canvas->font_style & CD_ITALIC)
    width+= (long) ((double)size_pixel*tan(4*atan(1)/8)); /* angulo de 15 graus */

  return width;
}

/****************************
 * Salva um byte no arquivo *
 ****************************/

static void put_byte(cdCtxCanvas* ctxcanvas, unsigned char byte)
{
  fputc(byte, ctxcanvas->file);
}

/************************************
 * Salva um sequencia de caracteres *
 ************************************/

static void writec (cdCtxCanvas* ctxcanvas, const char *t, short tam )
{
 short i;

 ctxcanvas->bytes += tam;
 for ( i = 0; i < tam; i++ )
     fputc ( t[i], ctxcanvas->file );
}

/******************
 * Salva uma word *
 ******************/

static void put_word(cdCtxCanvas* ctxcanvas, unsigned short w)
{
  char c;

  c = (char) (w & 0xff);
  fputc (c, ctxcanvas->file);
  c = (char) ((w >> 8) & 0xff);
  fputc (c, ctxcanvas->file);
  ctxcanvas->bytes += 2;
}

/****************************
 * Salva um long no arquivo *
 ****************************/

static void put_long (cdCtxCanvas* ctxcanvas, unsigned long i)
{
  put_word(ctxcanvas, (short) (i >> 16));
  put_word(ctxcanvas, (short) i);
}

/*******************
 * Salva um double *
 *******************/

static void put_as_double(cdCtxCanvas* ctxcanvas, long i)
{ 
  float dfloat=(float) 4*i;
 
  put_long(ctxcanvas, *((long *) &dfloat)); 
  put_long(ctxcanvas, 0);

  ctxcanvas->bytes+=sizeof(float);
}

/****************************
 * Salva uma UOR no arquivo *
 ****************************/

static void put_uor(cdCtxCanvas* ctxcanvas, long i)
{
  put_word(ctxcanvas, (unsigned short)((i >> 16) ^ (1 << 15))); /* troca o bit 31 
                                             para transformar em uor */
  put_word(ctxcanvas, (unsigned short)i);
}

/*************************************** 
 * Salva a bounding box de um elemento *
 ***************************************/

static void put_bound(cdCtxCanvas* ctxcanvas, long xmin, long xmax, long ymin, long ymax)
{
  put_uor(ctxcanvas, xmin);
  put_uor(ctxcanvas, ymin);
  put_uor(ctxcanvas, 0L);
  put_uor(ctxcanvas, xmax);
  put_uor(ctxcanvas, ymax);
  put_uor(ctxcanvas, 0L);
}

/******************************************
 * Calcula a bounding box de uma polyline *
 ******************************************/

static void line_string_bound(cdPoint *buffer, short num_vertex,
                     unsigned long *xmin, unsigned long *ymin, 
                     unsigned long *xmax, unsigned long *ymax)
{
  short i;
  unsigned long v;
  
  *xmin = *xmax = buffer[0].x;
  *ymin = *ymax = buffer[0].y;

  for (i = 1; i < num_vertex; i++)
  {
	  v = buffer[i].x;
    if (v < *xmin)
      *xmin = v;
    else if (v > *xmax)
      *xmax = v;

   v = (long) buffer[i].y;
	 if (v < *ymin)
      *ymin = v;
    else if (v > *ymax)
      *ymax = v;
  }
}

/************************************
 * Retorna symbology de um elemento *
 ************************************/

static short symbology(cdCtxCanvas* ctxcanvas)
{
  return (short)((ctxcanvas->color << 8) | (ctxcanvas->canvas->line_width << 3) | ctxcanvas->style);
} 

/*****************************************
 * Salva um Element Header no arquivo *
 *****************************************/

static void putElementHeader(cdCtxCanvas* ctxcanvas, Elm_hdr *ehdr)
{
  ehdr->type.flags.complex = ctxcanvas->is_complex;
  
  put_word(ctxcanvas, (short)(ehdr->type.flags.type << 8 |
    ehdr->type.flags.complex << 7 | ehdr->type.flags.level));
  

  put_word(ctxcanvas, ehdr->words);  
  put_bound(ctxcanvas, ehdr->xmin, ehdr->xmax, ehdr->ymin, ehdr->ymax);
}

/**************************************
 * Salva um display header no arquivo *
 **************************************/

static void putDisplayHeader(cdCtxCanvas* ctxcanvas, Disp_hdr *dhdr)
{
  put_word(ctxcanvas, 0);                      /* graphics group */
  put_word(ctxcanvas, dhdr->attindx);          /* index to attributes */
  put_word(ctxcanvas, dhdr->props.flags.attributes << 11); /* properties */
  put_word(ctxcanvas, dhdr->symb.s);           /* display symbology */
}


/***************************************
 * completa o arquivo com zeros para   *
 * que o numero de bytes seja multiplo *
 * de 512                              *
 ***************************************/

static void complete_file(cdCtxCanvas* ctxcanvas)
{
  long resto, i;

  put_word(ctxcanvas, END_OF_DGN_FILE);

  resto = DGN_FILE_BLOCK - ctxcanvas->bytes % DGN_FILE_BLOCK;

  /* checa validade do tamanho do arquivo */
  if (resto%2 != 0) return;

  for (i = 0; i < resto; i+=2)
    put_word(ctxcanvas, 0);
}

/*************************************
 * Salva um elemento arco no arquivo *
 *************************************/

static void arc (cdCtxCanvas* ctxcanvas, long xc, long yc, long w, long h, double a1, double a2)
{
  Elm_hdr ehdr;
  Disp_hdr dhdr; 

    /* raster header element */
  ehdr.type.flags.level=ctxcanvas->level;
  ehdr.type.flags.type=16;
  ehdr.words=SIZE_ARC-2;
  ehdr.xmin=xc - w/2;
  ehdr.xmax=xc + w/2;
  ehdr.ymin=yc - h/2;
  ehdr.ymax=yc + h/2;

  putElementHeader(ctxcanvas, &ehdr);

    /* Display Header */
  dhdr.attindx = ehdr.words - 14;
  dhdr.props.flags.attributes = 0;
  dhdr.symb.s = symbology(ctxcanvas);
  putDisplayHeader(ctxcanvas, &dhdr);

  put_long(ctxcanvas, (long) a1*360000);       /* start angle */
  put_long(ctxcanvas, (long) (a2-a1)*360000);  /* sweep angle */
  put_as_double(ctxcanvas, w/2);           /* primary axis */
  put_as_double(ctxcanvas, h/2);           /* secondary axis */
  put_long(ctxcanvas, 0);                      /* rotation angle (sempre 0) */
  put_as_double(ctxcanvas, xc);          /* x origin */
  put_as_double(ctxcanvas, yc);          /* y origin */
}

/***************************************
 * Salva um elemento elipse no arquivo *
 ***************************************/

static void ellipse(cdCtxCanvas* ctxcanvas, long xc, long yc, long w, long h, short type)
{
  Elm_hdr ehdr;
  Disp_hdr dhdr; 

    /* raster header element */
  ehdr.type.flags.level=ctxcanvas->level;
  ehdr.type.flags.type=15;
  ehdr.words=34+((type==FILLED) ? 8 : 0);
  ehdr.xmin=xc - w/2;
  ehdr.xmax=xc + w/2;
  ehdr.ymin=yc - h/2;
  ehdr.ymax=yc + h/2;

  putElementHeader(ctxcanvas, &ehdr);

    /* Display Header */
  dhdr.attindx=20;
  dhdr.props.flags.attributes=(type == FILLED) ? 1 : 0;
  dhdr.symb.s=symbology(ctxcanvas);
  putDisplayHeader(ctxcanvas, &dhdr);

  put_as_double(ctxcanvas, w/2);           /* primary axis */
  put_as_double(ctxcanvas, h/2);           /* secondary axis */
  put_long(ctxcanvas, 50);               /* rotation angle (sempre 0) */
  put_as_double(ctxcanvas, xc);          /* x origin */
  put_as_double(ctxcanvas, yc);          /* y origin */
  
    /* salva atributo de fill */
  if(type == FILLED)
  {
    put_word(ctxcanvas, 0x1007);
    put_word(ctxcanvas, 65);
    put_word(ctxcanvas, 0x802);
    put_word(ctxcanvas, 0x0001);
    put_word(ctxcanvas, ctxcanvas->color);
    put_word(ctxcanvas, 0);
    put_word(ctxcanvas, 0);
    put_word(ctxcanvas, 0);
  }
}

static short getclosestColor(cdCtxCanvas* ctxcanvas, long color)
{
  short count=0, closest=0;
  long diff=0;
  unsigned char r = cdRed(color),
                g = cdGreen(color),
                b = cdBlue(color);
  short rd, gd, bd;
  long newdiff;

  /* procura a cor mais proxima */

  diff = 3*65536;  /* inicializa com maior diferenca possivel */

  for(count=0; count < ctxcanvas->num_colors; count++)
  {
    rd = r - cdRed(ctxcanvas->colortable[count]);
    gd = g - cdGreen(ctxcanvas->colortable[count]);
    bd = b - cdBlue(ctxcanvas->colortable[count]);

    newdiff = rd*rd + gd*gd + bd*bd;

    if(newdiff <= diff)
    {
      /* verifica se encontrou a cor */
      if(newdiff == 0)
        return count-1;

      diff = newdiff;
      closest=count-1;
    }
  }

  /* nao encontrou a cor, tenta adicionar na palette, ou retorna a mais proxima */
  if(ctxcanvas->num_colors < 254)
  {
    ctxcanvas->colortable[ctxcanvas->num_colors+1] = color;
      return ctxcanvas->num_colors++;
  }
  else
    return closest;
}

static void saveColorTable(cdCtxCanvas* ctxcanvas)
{
  unsigned char r,g,b;
  short i;
  
  put_word(ctxcanvas, (0x05 << 8) | 1);  /* colortable */
  put_word(ctxcanvas, 434);

  put_long(ctxcanvas, 0);
  put_long(ctxcanvas, 0);
  put_long(ctxcanvas, 0);
  put_long(ctxcanvas, 0xffffffff);
  put_long(ctxcanvas, 0xffffffff);
  put_long(ctxcanvas, 0xffffffff);

  put_word(ctxcanvas, 0);
  put_word(ctxcanvas, 420);
  put_word(ctxcanvas, 0x0400);
  put_word(ctxcanvas, 0x100);
  put_word(ctxcanvas, 0);

  for(i=0;i<256;i++)
  {
	 cdDecodeColor(ctxcanvas->colortable[i], &r, &g, &b);
	 put_byte(ctxcanvas, r);
	 put_byte(ctxcanvas, g);
	 put_byte(ctxcanvas, b);
  }

  put_word(ctxcanvas, 25);
  for(i=0;i<32;i++)
	 put_word(ctxcanvas, 0x2020);
}


/*****************************
 * Le uma word de um arquivo *
 *****************************/

static short file_get_word(FILE *fp)
{
  short word=0;

  word = (short)fgetc(fp);
  word |= ((short)fgetc(fp) << 8) & 0xff00;    

  return word;
}

/********************************
 * Salva uma word em um arquivo *
 ********************************/

static void file_put_word (short word, FILE *fp)
{
  fputc ((char) (word & 0xff), fp);
  fputc ((char) ((word >> 8) & 0xff), fp);
}

/*******************************************
 * Le elementos de um arquivo DGN e os     *
 * coloca no inicio do arquivo aberto pelo *
 * driver                                  *
 *******************************************/

static void dgn_copy (FILE *file, cdCtxCanvas *ctxcanvas)
{
  short word=0;

  while ((word = file_get_word(file)) != END_OF_DGN_FILE) 
  {
    file_put_word(word, ctxcanvas->file); /* type e level do elemento */
    ctxcanvas->bytes+=2;

    word = file_get_word(file); /* words to follow */
    file_put_word(word, ctxcanvas->file);
    ctxcanvas->bytes+=2;

    while (word)       /* copia resto do elemento */
    {
      file_put_word(file_get_word(file), ctxcanvas->file);
      word--;
      ctxcanvas->bytes+=2;
    }
  }
}


/*
 *  Funcoes do driver
 */

static void cdkillcanvas(cdCtxCanvas* ctxcanvas)
{
  saveColorTable(ctxcanvas);
  complete_file(ctxcanvas);
  fclose (ctxcanvas->file);

  memset(ctxcanvas, 0, sizeof(cdCtxCanvas));
  free(ctxcanvas);
}

static void cddeactivate (cdCtxCanvas* ctxcanvas)
{
  fflush(ctxcanvas->file);
}

static void cdflush (cdCtxCanvas* ctxcanvas)
{
  fflush(ctxcanvas->file);
}


/******************************************************/
/* primitives                                         */
/******************************************************/

static void cdline (cdCtxCanvas* ctxcanvas, int x1, int y1, int x2, int y2)
{
  Elm_hdr ehdr;
  Disp_hdr dhdr;
  cdPoint buffer[2];

  buffer[0].x=x1;
  buffer[0].y=y1;
  buffer[1].x=x2;
  buffer[1].y=y2;
  
  ehdr.type.flags.level=ctxcanvas->level;
  ehdr.type.flags.type=3;
  
  ehdr.words=SIZE_LINE-2;

  line_string_bound(buffer, 2, &ehdr.xmin,
                    &ehdr.ymin,&ehdr.xmax,&ehdr.ymax);
  
  putElementHeader(ctxcanvas, &ehdr);

    /* Display Header */
  dhdr.attindx = ehdr.words - 14;
  dhdr.props.flags.attributes = 0;
  dhdr.symb.s=symbology(ctxcanvas);

  putDisplayHeader(ctxcanvas, &dhdr);

    /* pontos inicial e final da linha */

  put_long(ctxcanvas, (long) x1);     
  put_long(ctxcanvas, (long) y1);
  put_long(ctxcanvas, (long) x2);
  put_long(ctxcanvas, (long) y2);
}

static void cdbox (cdCtxCanvas* ctxcanvas, int xmin, int xmax, int ymin, int ymax)
{
  Elm_hdr ehdr;
  Disp_hdr dhdr; 

    /* raster header element */
  ehdr.type.flags.level=ctxcanvas->level;
  ehdr.type.flags.type=6;
  ehdr.words=17+4*5+8;
  ehdr.xmin=xmin;
  ehdr.xmax=xmax;
  ehdr.ymin=ymin;
  ehdr.ymax=ymax;

  putElementHeader(ctxcanvas, &ehdr);

    /* Display Header */
  dhdr.attindx=3+4*5;
  dhdr.props.flags.attributes=1;
  dhdr.symb.s=symbology(ctxcanvas);
  putDisplayHeader(ctxcanvas, &dhdr);

  put_word(ctxcanvas, 5);                       /* numero de vertices */

   /* vertices */
  put_long(ctxcanvas, (long) xmin);
  put_long(ctxcanvas, (long) ymin);
  
  put_long(ctxcanvas, (long) xmax);
  put_long(ctxcanvas, (long) ymin);
 
  put_long(ctxcanvas, (long) xmax);
  put_long(ctxcanvas, (long) ymax);

  put_long(ctxcanvas, (long) xmin);
  put_long(ctxcanvas, (long) ymax);

  put_long(ctxcanvas, (long) xmin);
  put_long(ctxcanvas, (long) ymin);
 
    /* atributos de fill */

  put_word(ctxcanvas, 0x1007);
  put_word(ctxcanvas, 65);
  put_word(ctxcanvas, 0x802);
  put_word(ctxcanvas, 0x0001);
  put_word(ctxcanvas, ctxcanvas->color);
  put_word(ctxcanvas, 0);
  put_word(ctxcanvas, 0);
  put_word(ctxcanvas, 0);
}

static void cdarc (cdCtxCanvas* ctxcanvas, int xc, int yc, int w, int h, double a1, double a2)
{
  if (a2 == a1 + 360)
    ellipse(ctxcanvas, xc,yc,w,h,OPEN);
  else
    arc(ctxcanvas, xc, yc, w, h, a1, a2);
}

static void cdsector (cdCtxCanvas* ctxcanvas, int xc, int yc, int w, int h, double a1, double a2)
{
  if (a2 == a1 + 360)
  {
    ellipse(ctxcanvas, xc,yc,w,h,FILLED);
    return;
  }

  startComplexShape(ctxcanvas, 3, 1, SIZE_ARC+SIZE_LINE*2,
    (unsigned long) xc-w/2, (unsigned long) yc-h/2,
    (unsigned long) xc+h/2, (unsigned long) yc+h/2);

  arc(ctxcanvas, xc, yc, w, h, a1, a2);
  
  cdline(ctxcanvas, xc, yc, (int)
    (((double)w*cos(a1*CD_DEG2RAD)/2.+.5)), (int) (((double)h*sin(a1*CD_DEG2RAD))/2.+.5));
  cdline(ctxcanvas, xc, yc, (int) 
    (((double)w*cos(a2*CD_DEG2RAD)/2.+.5)), (int) (((double)h*sin(a2*CD_DEG2RAD))/2.+.5));

  endComplexElement(ctxcanvas);
}

static void cdtext (cdCtxCanvas* ctxcanvas, int x, int y, const char *s, int len)
{
  long descent=0;
  short w=0;
  long hc=0,wc=0;
  int size_pixel;
  short italic = (ctxcanvas->canvas->font_style&CD_ITALIC);
 
  Elm_hdr ehdr;
  Disp_hdr dhdr;

  if(len > 255)
    len=255;

  w = (short)((len/2)+(len%2));
  size_pixel = cdGetFontSizePixels(ctxcanvas->canvas, ctxcanvas->canvas->font_size);
  descent=get_descent(s, len, size_pixel);
  hc = size_pixel+descent;
  wc = gettextwidth(ctxcanvas, s, len, size_pixel);
 
  y+=descent;

  switch (ctxcanvas->alignment)
  {
  case 12: x = x;                   y = y;                   break;
  case 13: x = x;                   y = y - (int) (hc/2.0);  break;
  case 14: x = x;                   y = y - (int) hc;        break;
  case  6: x = x - (int) (wc/2.0);  y = y;                   break;
  case  7: x = x - (int) (wc/2.0);  y = y - (int) (hc/2.0);  break;
  case  8: x = x - (int) (wc/2.0);  y = y - (int) hc;        break;
  case  0: x = x - (int) wc;        y = y;                   break;
  case  1: x = x - (int) wc;        y = y - (int) (hc/2.0);  break;
  case  2: x = x - (int) wc;        y = y - (int) hc;        break;
  }

  if(ctxcanvas->is_base)
    y -= (int) (hc/4.0);

   /* raster header element */
  ehdr.type.flags.level=ctxcanvas->level;
  ehdr.type.flags.type=17;
  ehdr.words=28+w+((italic) ? 8 : 0);
  ehdr.xmin=x;
  ehdr.xmax=x+wc;
  ehdr.ymin=y-descent;
  ehdr.ymax=y+hc;

  putElementHeader(ctxcanvas, &ehdr);

    /* Display Header */
  dhdr.attindx=14+w;
  dhdr.props.flags.attributes=(italic) ? 1 : 0;

  if (ctxcanvas->canvas->font_style&CD_BOLD)
    dhdr.symb.s=ctxcanvas->color << 8 | (3 << 3);
  else
    dhdr.symb.s=ctxcanvas->color << 8;

  putDisplayHeader(ctxcanvas, &dhdr);

  put_word(ctxcanvas, (ctxcanvas->alignment << 8) | ctxcanvas->typeface_index);

  put_long(ctxcanvas, (long)((1000 * ctxcanvas->tl) / 6) | (1 << 7));
  put_long(ctxcanvas, (long)((1000 * size_pixel) / 6) | (1 << 7));
 
  put_long(ctxcanvas, 0);
  put_long(ctxcanvas, x);
  put_long(ctxcanvas, y);
  put_word(ctxcanvas, (unsigned short)len);
  writec(ctxcanvas, s, (short)(len+(len%2))); /* deve escrever sempre um numero par de caracteres */

  if(italic)
  {
    put_word(ctxcanvas, 0x1007);  /* atributos e words to follow */
    put_word(ctxcanvas, 0x80d4);  /* tipo de atributo */
    put_long(ctxcanvas, 0x000865c0);
    put_long(ctxcanvas, 0x00520000);
    put_long(ctxcanvas, 0x00000000);
  }
}

static void startComplexShape(cdCtxCanvas* ctxcanvas, 
                              unsigned short num_elements,
                              short is_fill,
                              unsigned short size,
                              unsigned long xmin,
                              unsigned long ymin,
                              unsigned long xmax,
                              unsigned long ymax)
{
  Elm_hdr ehdr;
  Disp_hdr dhdr;

  ehdr.type.flags.level=ctxcanvas->level;
  ehdr.type.flags.type=14;
  ehdr.words=22 + ((is_fill) ? 8 : 0);
  ehdr.xmax = xmax; 
  ehdr.xmin = xmin; 
  ehdr.ymax = ymax; 
  ehdr.ymin = ymin; 

  putElementHeader(ctxcanvas, &ehdr);

    /* Display Header */
  dhdr.attindx=4;
  dhdr.props.flags.attributes = (is_fill) ? 1 : 0;
  dhdr.symb.s=symbology(ctxcanvas);

  putDisplayHeader(ctxcanvas, &dhdr);

  put_word(ctxcanvas, size + 5 + ((is_fill) ? 8 : 0));
  put_word(ctxcanvas, num_elements);

  put_long(ctxcanvas, 0);  /* atributo nulo */
  put_long(ctxcanvas, 0);
 
    /* salva atributo de fill */
  if(is_fill)
  {
    put_word(ctxcanvas, 0x1007);
    put_word(ctxcanvas, 65);
    put_word(ctxcanvas, 0x802);
    put_word(ctxcanvas, 0x0001);
    put_word(ctxcanvas, ctxcanvas->color);
    put_word(ctxcanvas, 0);
    put_word(ctxcanvas, 0);
    put_word(ctxcanvas, 0);
  }

  /* marca inicio de elemento complexo */

  ctxcanvas->is_complex = 1;
}


static void startComplexChain(cdCtxCanvas* ctxcanvas, 
                              unsigned short num_elements,
                              unsigned short size,
                              unsigned long xmin,
                              unsigned long ymin,
                              unsigned long xmax,
                              unsigned long ymax)
                            
{
  Elm_hdr ehdr;
  Disp_hdr dhdr;

  ehdr.type.flags.level=ctxcanvas->level;
  ehdr.type.flags.type=12;

  ehdr.words=22;
  ehdr.xmax = xmax; 
  ehdr.xmin = xmin; 
  ehdr.ymax = ymax; 
  ehdr.ymin = ymin; 


  putElementHeader(ctxcanvas, &ehdr);

    /* Display Header */
  dhdr.attindx=4;
  dhdr.props.flags.attributes = 1;
  dhdr.symb.s=symbology(ctxcanvas);

  putDisplayHeader(ctxcanvas, &dhdr);

  put_word(ctxcanvas, size+5);
  put_word(ctxcanvas, num_elements);

  put_long(ctxcanvas, 0);  /* atributo nulo */
  put_long(ctxcanvas, 0);

  
  /* marca inicio de elemento complexo */

  ctxcanvas->is_complex = 1;
}

static void endComplexElement(cdCtxCanvas* ctxcanvas)
{
  ctxcanvas->is_complex = 0;
}

static void putLineString(cdCtxCanvas* ctxcanvas, cdPoint *buffer, short num_vertex)
{
  Elm_hdr ehdr;
  Disp_hdr dhdr;
  short i=0;

  ehdr.type.flags.level=ctxcanvas->level;

  ehdr.type.flags.type=4;
  
  ehdr.words=SIZE_LINE_STRING(num_vertex)-2;

  line_string_bound(buffer, num_vertex, &ehdr.xmin,
                    &ehdr.ymin,&ehdr.xmax,&ehdr.ymax);
  
  putElementHeader(ctxcanvas, &ehdr);

    /* Display Header */
  dhdr.attindx=ehdr.words-14;
  dhdr.props.flags.attributes = 0;
  dhdr.symb.s=symbology(ctxcanvas);

  putDisplayHeader(ctxcanvas, &dhdr);

  put_word(ctxcanvas, num_vertex);

  for (i = 0; i < num_vertex; i++)
  {
    put_long(ctxcanvas, (long) buffer[i].x);
    put_long(ctxcanvas, (long) buffer[i].y);
  }
}

static void putShape(cdCtxCanvas* ctxcanvas, cdPoint *buffer, short num_vertex)
{
  Elm_hdr ehdr;
  Disp_hdr dhdr;
  short i=0;

  ehdr.type.flags.level=ctxcanvas->level;
  ehdr.type.flags.type=6;

  ehdr.words=SIZE_FILLED_SHAPE(num_vertex)-2;

  line_string_bound(buffer, num_vertex, &ehdr.xmin,
                    &ehdr.ymin,&ehdr.xmax,&ehdr.ymax);
  

  putElementHeader(ctxcanvas, &ehdr);

    /* Display Header */
  dhdr.attindx=ehdr.words - 14 - 8; /* 8 -> size of attributes */
  dhdr.props.flags.attributes = 1;
  dhdr.symb.s=symbology(ctxcanvas);

  putDisplayHeader(ctxcanvas, &dhdr);

  put_word(ctxcanvas, num_vertex);

  for (i = 0; i < num_vertex; i++)
  {
    put_long(ctxcanvas, (long) buffer[i].x);
    put_long(ctxcanvas, (long) buffer[i].y);
  }

  put_word(ctxcanvas, 0x1007);
  put_word(ctxcanvas, 65);
  put_word(ctxcanvas, 0x802);
  put_word(ctxcanvas, 0x0001);
  put_word(ctxcanvas, ctxcanvas->color);
  put_word(ctxcanvas, 0);
  put_word(ctxcanvas, 0);
  put_word(ctxcanvas, 0);
}

static void cdpoly(cdCtxCanvas* ctxcanvas, int mode, cdPoint* poly, int n)
{
  short is_fill=0;

  if (mode == CD_BEZIER)
  {
    cdSimPolyBezier(ctxcanvas->canvas, poly, n);
    return;
  }
  if (mode == CD_PATH)
  {
    cdSimPolyPath(ctxcanvas->canvas, poly, n);
    return;
  }

  if(mode == CD_FILL && ctxcanvas->fill_type == NOFILL)
    mode = CD_CLOSED_LINES;

  if(n > MAX_NUM_VERTEX_PER_POLYLINE)
    n = MAX_NUM_VERTEX_PER_POLYLINE;

  /* acerta buffer de pontos */
  if(mode == CD_FILL || mode == CD_CLOSED_LINES)
  {
    poly[n].x = poly[0].x;
    poly[n].y = poly[0].y;
    n++;
  }

  /* se fill_type for CONVEX, testa se poligono e' convexo ou concavo */
  if((ctxcanvas->fill_type == CONVEX) && (n > 3) && (mode == CD_FILL))
  {
    short signal=0;
    short count=0;
    long vect=0;

    /* calcula sinal do vetorial entre o primeiro lado e o segundo */
    vect = (poly[1].x - poly[0].x) * 
           (poly[2].y - poly[1].y) -
           (poly[1].y - poly[0].y) * 
           (poly[2].x - poly[1].x);

    if(vect == 0)
      mode = CD_CLOSED_LINES;  /* ver se precisa mudar */
    else
    {
      signal = (short)(vect/abs(vect));

      for(count=1 ; count< (n-2); count++)
      { 
        vect = (poly[count+1].x - poly[count].x) * 
               (poly[count+2].y - poly[count+1].y) -
               (poly[count+1].y - poly[count].y) * 
               (poly[count+2].x - poly[count+1].x);
      
        if(vect == 0)
        {
          mode=CD_CLOSED_LINES;
          break;
        }

        if((vect/abs(vect)) != signal)
        {
          mode=CD_CLOSED_LINES;
          break;
        }
      }
    }
  }

  /* se tiver fill */

  if(mode == CD_FILL)
    is_fill=1;

  if(n > MAX_NUM_VERTEX)  /* tem que usar complex shape ou chain */
  {
    short count=0;
    short num_whole_elements = n / MAX_NUM_VERTEX;
    short num_whole_vertex = num_whole_elements * MAX_NUM_VERTEX;
    short rest = n % MAX_NUM_VERTEX;
    short is_there_rest = (rest > 0) ? 1 : 0;
    unsigned long xmax, xmin, ymax, ymin;
    unsigned short size =
         SIZE_LINE_STRING(MAX_NUM_VERTEX)*num_whole_elements+ 
         SIZE_LINE_STRING(rest)*is_there_rest;

    line_string_bound(poly, n, &xmin, &ymin, &xmax, &ymax);

    if(mode == CD_OPEN_LINES)
      startComplexChain(ctxcanvas, (unsigned short) (num_whole_elements+((rest > 0) ? 1 : 0)),
                         size, xmin, ymin, xmax, ymax);
    else
      startComplexShape(ctxcanvas, (unsigned short) (num_whole_elements+((rest > 0) ? 1 : 0)),
                        is_fill, size, xmin, ymin, xmax, ymax);

    for(count=0;count < num_whole_vertex; count+=MAX_NUM_VERTEX, n-=MAX_NUM_VERTEX)
      putLineString(ctxcanvas, &poly[count], MAX_NUM_VERTEX);

    if(rest)
      putLineString(ctxcanvas, &poly[num_whole_vertex],n);

    endComplexElement(ctxcanvas);
  }
  else
  {
    if(is_fill)
      putShape(ctxcanvas, poly, n);
    else
      putLineString(ctxcanvas, poly, n);
  }
}

/**************
 * attributes *
 **************/

static int cdlinestyle (cdCtxCanvas* ctxcanvas, int style)
{
  switch(style)
  {
  case CD_CONTINUOUS:
    ctxcanvas->style = 0;
    break;

  case CD_DASHED:
    ctxcanvas->style = 3;
    break;

  case CD_DOTTED:
    ctxcanvas->style = 1;
    break;

  case CD_DASH_DOT:
    ctxcanvas->style = 4;
    break;

  case CD_DASH_DOT_DOT:
    ctxcanvas->style = 6;
    break;
  }

  return style;
}

static int cdlinewidth (cdCtxCanvas* ctxcanvas, int width)
{
  (void)ctxcanvas;
  width = width & 31;
  return width;
}

static int cdfont(cdCtxCanvas* ctxcanvas, const char *type_face, int style, int size)
{
  (void)style;
  ctxcanvas->tl = (long)(cdGetFontSizePoints(ctxcanvas->canvas, size)/4)*3;

  if (cdStrEqualNoCase(type_face, "Courier"))
    ctxcanvas->typeface_index=1;
  else if (cdStrEqualNoCase(type_face, "Times"))
    ctxcanvas->typeface_index=2;
  else if (cdStrEqualNoCase(type_face, "Helvetica"))
    ctxcanvas->typeface_index=3;
  else if (cdStrEqualNoCase(type_face, "System"))
    ctxcanvas->typeface_index=0;
  else
    return 0;

  return 1;
}

static void cdgetfontdim (cdCtxCanvas* ctxcanvas, int *max_width, int *height, int *ascent, int *descent)
{
  int size_pixel;

  if(max_width)
  {
    int a=0;
    *max_width=0;

    while(fontsizes[ctxcanvas->typeface_index][a])
    {
      if(fontsizes[ctxcanvas->typeface_index][a] > *max_width)
        *max_width = fontsizes[ctxcanvas->typeface_index][a];
      a++;
    }
  }

  size_pixel = cdGetFontSizePixels(ctxcanvas->canvas, ctxcanvas->canvas->font_size);

  if(height)  *height  = (size_pixel*3)/2;
  if(ascent)  *ascent  = size_pixel;
  if(descent) *descent = size_pixel/2;
}

static void cdgettextsize (cdCtxCanvas* ctxcanvas, const char *s, int len, int *width, int *height)
{
 int size_pixel = cdGetFontSizePixels(ctxcanvas->canvas, ctxcanvas->canvas->font_size);
 if(height) *height = size_pixel + get_descent(s, len, size_pixel);
 if(width)  *width  = gettextwidth(ctxcanvas, s, len, size_pixel);
}

static int cdtextalignment (cdCtxCanvas* ctxcanvas, int alignment)
{
  ctxcanvas->is_base = 0;

  /* DGN guarda posicao do texto em relacao ao ponto */
    
  switch(alignment)
  {
  case CD_NORTH:
    ctxcanvas->alignment = 8; /* center-bottom */
    break;

  case CD_SOUTH:
    ctxcanvas->alignment = 6; /* center-top */
    break;

  case CD_EAST:
    ctxcanvas->alignment = 1; /* left-center */
    break;

  case CD_WEST:
    ctxcanvas->alignment = 13; /* right-center */
    break;

  case CD_NORTH_EAST:
    ctxcanvas->alignment = 2; /* left-bottom */
    break;

  case CD_NORTH_WEST:
    ctxcanvas->alignment = 14; /* right-bottom */
    break;

  case CD_SOUTH_EAST:
    ctxcanvas->alignment = 0; /* left-top */
    break;

  case CD_SOUTH_WEST:
    ctxcanvas->alignment = 12; /* right-top */
    break;

  case CD_CENTER:
    ctxcanvas->alignment = 7; /* center-center */
    break;

  case CD_BASE_LEFT:
    ctxcanvas->alignment = 13; /* right-center */
    ctxcanvas->is_base=1;
    break;

  case CD_BASE_CENTER:
    ctxcanvas->alignment = 7; /* center-center */
    ctxcanvas->is_base=1;
    break;

  case CD_BASE_RIGHT:
    ctxcanvas->alignment = 1; /* left-center */
    ctxcanvas->is_base=1;
    break;
  }

  return alignment;
}

/******************************************************/
/* color                                              */
/******************************************************/

static void cdpalette (cdCtxCanvas* ctxcanvas, int n, const long int *palette, int mode)
{
  int c=0;

  IGNORE(mode);
  
  for(c=0; c < n; c++)
    ctxcanvas->colortable[c] = *palette++;

  ctxcanvas->num_colors = n;
}

static long int cdforeground (cdCtxCanvas* ctxcanvas, long int color)
{
  ctxcanvas->color = getclosestColor(ctxcanvas, color);
  return color;
}

/******************************************************/
/* client images                                      */
/******************************************************/

static void cdputimagerectmap(cdCtxCanvas* ctxcanvas, int iw, int ih, const unsigned char *index,
			 const long int *colors, int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax)
{
  int i=0,j=0, remainder=iw%2, total_colors;
  int *ix=NULL,*iy=NULL;
  Elm_hdr ehdr;
  Disp_hdr dhdr; 
  unsigned char map_colors[256];


  /* raster header element */
  ehdr.type.flags.level=ctxcanvas->level;
  ehdr.type.flags.type=87;
  ehdr.words=39;
  ehdr.xmin=x;
  ehdr.xmax=x+w;
  ehdr.ymin=y;
  ehdr.ymax=y+h;

  putElementHeader(ctxcanvas, &ehdr);

    /* Display Header */
  dhdr.attindx=25;
  dhdr.symb.s=0;

  putDisplayHeader(ctxcanvas, &dhdr);

    /* description of the element */
  put_long(ctxcanvas, 21+(23+w/2+w%2)*h);  /* total length of cell */
  put_word(ctxcanvas, 0x0714);         /* raster flags */
  put_word(ctxcanvas, 0x0100);         /* background e foreground colors
                               (nao usados) */
  put_word(ctxcanvas, w);             /* largura da imagem em pixels */
  put_word(ctxcanvas, h);             /* altura da imagem em pixel */ 
  put_long(ctxcanvas, 0);              /* resevado */
  put_as_double(ctxcanvas, 0);            /* resolution (nao usado) */

  put_word(ctxcanvas, 0x4080);         /* scale */
  put_long(ctxcanvas, 0);
  put_word(ctxcanvas, 0);

  put_long(ctxcanvas, x);              /* origem */ 
  put_long(ctxcanvas, y+h);
  put_long(ctxcanvas, 0);

  put_word(ctxcanvas, 0);              /* no de vertices */ 

  ctxcanvas->is_complex = 1; /* elemento complexo */

  /* obtem o maior indice usado na imagem */

  total_colors = 0;
  for (i = 0; i < iw*ih; i++)
  {        
    if (index[i] > total_colors)
      total_colors = index[i];
  }
  total_colors++;

  /* cria tabela para acelerar match de cor na palette */

  for (i = 0; i < total_colors; i++)
  {        
    map_colors[i] = (unsigned char)getclosestColor(ctxcanvas, colors[i]);
  }

  /** salva dados da imagem **/

  /* calcula stretch */

  ix = cdGetZoomTable(w, xmax-xmin+1, xmin);
  iy = cdGetZoomTable(h, ymax-ymin+1, ymin);
 
  for(i=h-1; i >= 0; i--)
  {
      /* raster header element */
    ehdr.type.flags.level=ctxcanvas->level;
    ehdr.type.flags.type=88;
    ehdr.words=21+w/2+remainder;

    putElementHeader(ctxcanvas, &ehdr);

      /* Display Header */
    dhdr.attindx=7+w/2+remainder;
    dhdr.symb.s=0;
    putDisplayHeader(ctxcanvas, &dhdr);

    put_word(ctxcanvas, 0x0714);   /* raster flags */
    put_word(ctxcanvas, 0x0100);   /* background e foreground 
                           colors (nao usados) */

    put_word(ctxcanvas, 0);    /* x offset da origem */
    put_word(ctxcanvas, i);    /* y offset */
    put_word(ctxcanvas, w);    /* numero de pixels neste elemento */
    
    for(j=0; j < w; j++)   
      put_byte(ctxcanvas, map_colors[index[(iy[i])*iw + ix[j]]]);

    if(remainder) put_byte(ctxcanvas, 0);
  }

  ctxcanvas->is_complex = 0;

  free(ix);  /* libera memoria alocada */
  free(iy);
}

/******************************************************/
/* server images                                      */
/******************************************************/

static void cdpixel (cdCtxCanvas* ctxcanvas, int x, int y, long int color)
{
  long  old_color = cdforeground(ctxcanvas, color);
  int old_linestyle = cdlinestyle(ctxcanvas, CD_CONTINUOUS);
  int old_linewidth = cdlinewidth(ctxcanvas, 1);

  cdline(ctxcanvas, x,y,x,y);

  cdforeground(ctxcanvas, old_color);
  cdlinestyle(ctxcanvas, old_linestyle);
  cdlinewidth(ctxcanvas, old_linewidth);
}

static void cdcreatecanvas(cdCanvas* canvas, void *data)
{
  cdCtxCanvas *ctxcanvas;
  char* strdata = (char*)data;
  char words[4][256];
  char filename[10240] = "";
  char seedfile[10240] = "";
  int count  = 0;
  double res = 0;

  if (!data) return;
    
  /* separa palavras da expressao, que e' na forma
     "filename [mm_wxmm_h] [res] [-f] [-sseedfile]" */

  strdata += cdGetFileName(strdata, filename);
  if (filename[0] == 0)
    return;

  sscanf(strdata, "%s %s %s %s", words[0], words[1], words[2], words[3]);
  
  if(!strlen(filename)) /* se nao pegou filename */
    return;
   
  ctxcanvas = (cdCtxCanvas *) malloc(sizeof(cdCtxCanvas));

  /* tenta criar arquivo DGN */

  if((ctxcanvas->file = fopen (filename, "wb"))==NULL)
  {
    free(ctxcanvas);
    return;
  }

  /* verifica se foi passado tamanho do canvas em mm. Se foi,
     extrai-o da string */

  if(sscanf(words[0], "%lgx%lg",
     &canvas->w_mm, &canvas->h_mm) == 2)
  {
    count++; /* incrementa contador de palavras */

    if(canvas->w_mm == 0 || canvas->h_mm == 0)
    { 
      fclose(ctxcanvas->file);
      free(ctxcanvas);
      return;
    }
  }
  else
    canvas->w_mm = canvas->h_mm = 0;
  
  /* Verifica se foi passada resolucao */

  if(sscanf(words[count], "%lg", &res) == 1)
  {
    count++; /* incrementa contador de palavras */

    if(res <= 0)  /* verifica validade da resolucao */
    { 
      fclose(ctxcanvas->file);
      free(ctxcanvas);
      return;
    }
  }
  else
    res = 3.78;

  /* se tamanho em milimetros nao tiver sido inicializado,
     usa como default o tamanho maximo em pixels para fazer as
     contas
   */

  if (canvas->w_mm == 0 || canvas->h_mm == 0)
  {
    canvas->w = INT_MAX;
    canvas->h = INT_MAX;

    canvas->w_mm = canvas->w / res;
    canvas->h_mm = canvas->h / res;
  }
  else
  {
    canvas->w = (long) (canvas->w_mm * res);
    canvas->h = (long) (canvas->h_mm * res);
  }

  canvas->xres = res;
  canvas->yres = res;
  canvas->bpp = 8;
  
  /* verifica se usuario que alterar metodo de fill */

  if (strcmp(words[count], "-f")==0)
  {
    ctxcanvas->fill_type = CONVEX;
    count++;
  }
  else
    ctxcanvas->fill_type = NORMAL;
  
  /* se tiver passado seedfile como argumento */
  if(sscanf(words[count], "-s%s", seedfile) == 1)
  {
    FILE *seed=NULL;
    char *cd_dir = getenv("CDDIR");
    static char newfilename[10240];

    if(cd_dir == NULL)
      cd_dir = ".";

    sprintf(newfilename, "%s/%s", cd_dir, seedfile);
        
    count++;

    /* testa concatenando com variavel de ambiente */

    if((seed = fopen (newfilename, "rb"))==NULL)
    { 
      /* tenta abrir usando string passada pelo usuario
         diretamente */

      if((seed = fopen (seedfile, "rb"))==NULL)
      { 
        fclose(ctxcanvas->file);
        free(ctxcanvas);
        return;
      }
    }

    /* concatena seed */  

    fseek(seed, 0, SEEK_SET);
    fseek(ctxcanvas->file, 0, SEEK_SET);

    ctxcanvas->bytes=0;
    dgn_copy(seed, ctxcanvas);
    fclose(seed);
  }
  
  ctxcanvas->canvas = canvas;
  canvas->ctxcanvas = ctxcanvas;

  /* config */

  ctxcanvas->level = 1;

  /** valores default do contexto sao setados **/

  /* texto */

  ctxcanvas->alignment = 12; 
  ctxcanvas->is_base = 1;
  ctxcanvas->typeface_index = 0;
  ctxcanvas->tl=12;

  /* cores */

  memset(ctxcanvas->colortable, 0, 1024); 
  ctxcanvas->colortable[0] = CD_BLACK;
  ctxcanvas->num_colors = 1;

  /* atributos */

  ctxcanvas->color = 1;
  ctxcanvas->style = 0;

  /* DGN */

  ctxcanvas->is_complex=0;
}

static void cdinittable(cdCanvas* canvas)
{
  canvas->cxFlush = cdflush;
  canvas->cxPixel = cdpixel;
  canvas->cxLine = cdline;
  canvas->cxPoly = cdpoly;
  canvas->cxBox = cdbox;
  canvas->cxArc = cdarc;
  canvas->cxSector = cdsector;
  canvas->cxText = cdtext;
  canvas->cxGetFontDim = cdgetfontdim;
  canvas->cxGetTextSize = cdgettextsize;
  canvas->cxPutImageRectMap = cdputimagerectmap;

  canvas->cxLineStyle = cdlinestyle;
  canvas->cxLineWidth = cdlinewidth;
  canvas->cxFont = cdfont;
  canvas->cxTextAlignment = cdtextalignment;
  canvas->cxPalette = cdpalette;
  canvas->cxForeground = cdforeground;

  canvas->cxKillCanvas = cdkillcanvas;
  canvas->cxDeactivate = cddeactivate;
}

/******************************************************/

static cdContext cdDGNContext =
{
  CD_CAP_ALL & ~(CD_CAP_CLEAR | CD_CAP_PLAY | CD_CAP_PATH | CD_CAP_BEZIER | 
                 CD_CAP_IMAGERGBA | CD_CAP_GETIMAGERGB | 
                 CD_CAP_CLIPAREA | CD_CAP_CLIPPOLY |  CD_CAP_RECT | 
                 CD_CAP_LINECAP | CD_CAP_LINEJOIN | CD_CAP_REGION | CD_CAP_CHORD |
                 CD_CAP_IMAGERGB | CD_CAP_IMAGESRV | 
                 CD_CAP_BACKGROUND | CD_CAP_BACKOPACITY | CD_CAP_WRITEMODE | 
                 CD_CAP_HATCH | CD_CAP_STIPPLE | CD_CAP_PATTERN | 
                 CD_CAP_IMAGERGBA | CD_CAP_GETIMAGERGB | 
                 CD_CAP_FPRIMTIVES  | CD_CAP_TEXTORIENTATION),
  0,
  cdcreatecanvas,
  cdinittable,
  NULL,
  NULL,
};

cdContext* cdContextDGN(void)
{
  return &cdDGNContext;
}


