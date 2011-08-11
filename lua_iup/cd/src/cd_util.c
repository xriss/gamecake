/** \file
 * \brief Utilities
 *
 * See Copyright Notice in cd.h
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <memory.h>
#include <math.h>


#include "cd.h"

#include "cd_private.h"


int cdRound(double x)
{
  return _cdRound(x);
}

/* Returns a table to speed up zoom in discrete zoom rotines.
   Adds the min parameter and allocates the table using malloc.
   The table should be used when mapping from destiny coordinates to
   source coordinates (src_pos = tab[dst_pos]).
   dst_len is the full destiny size range.
   src_len is only the zoomed region size, usually max-min+1.
*/
int* cdGetZoomTable(int dst_len, int src_len, int src_min)
{
  int dst_i, src_i;
  double factor;
	int* tab = (int*)malloc(dst_len*sizeof(int));

  factor = (double)(src_len) / (double)(dst_len);

	for(dst_i = 0; dst_i < dst_len; dst_i++)
  {
    src_i = cdRound((factor*(dst_i + 0.5)) - 0.5);
		tab[dst_i] = src_i + src_min;
  }

  return tab;
}

/* funcao usada para calcular os retangulos efetivos de zoom 
   de imagens clientes. Pode ser usada para os eixos X e Y.

   canvas_size - tamanho do canvas (canvas->w, canvas->h)
   cnv_rect_pos - posicao no canvas onde a regiao sera´ desenhada (x, y)
   cnv_rect_size - tamanho da regiao no canvas com zoom (w, h)
   img_rect_pos - posicao na imagem da regiao a ser desenhada (min)
   img_rect_size - tamanho da regiao na imagem (max-min+1)

   calcula o melhor tamanho a ser usado
   retorna 0 se o retangulo resultante e´ nulo
*/
int cdCalcZoom(int canvas_size,
               int cnv_rect_pos, int cnv_rect_size, 
               int *new_cnv_rect_pos, int *new_cnv_rect_size, 
               int img_rect_pos, int img_rect_size, 
               int *new_img_rect_pos, int *new_img_rect_size, 
               int is_horizontal)
{
  int offset;
  float zoom_factor = (float)img_rect_size / (float)cnv_rect_size;

  /* valores default sem otimizacao */
  *new_cnv_rect_size = cnv_rect_size, *new_cnv_rect_pos = cnv_rect_pos;    
  *new_img_rect_size = img_rect_size, *new_img_rect_pos = img_rect_pos;

  if (cnv_rect_size > 0)
  {
    /* se posicao no canvas > tamanho do canvas, fora da janela, nao desenha nada */
    if (cnv_rect_pos >= canvas_size) 
      return 0;

    /* se posicao no canvas + tamanho da regiao no canvas < 0, fora da janela, nao desenha nada */
    if (cnv_rect_pos+cnv_rect_size < 0) 
      return 0;

    /* se posicao no canvas < 0, entao comeca fora do canvas melhor posicao no canvas e' 0
                                 E o tamanho e' reduzido do valor negativo */
    if (cnv_rect_pos < 0) 
    {
      /* valores ajustados para cair numa vizinhanca de um pixel da imagem */
      offset = (int)ceil(cnv_rect_pos*zoom_factor);   /* offset is <0 */
      offset = (int)ceil(offset/zoom_factor);
      *new_cnv_rect_pos -= offset;  
      *new_cnv_rect_size += offset; 
    }

    /* se posicao no canvas + tamanho da regiao no canvas > tamanho do canvas, 
       termina fora do canvas entao 
       o tamanho da regiao no canvas e' o tamanho do canvas reduzido da posicao */
    if (*new_cnv_rect_pos+*new_cnv_rect_size > canvas_size) 
    {
      offset = (int)((*new_cnv_rect_pos+*new_cnv_rect_size - canvas_size)*zoom_factor);
      *new_cnv_rect_size -= (int)(offset/zoom_factor);  /* offset is >0 */
    }
  }
  else
  {
    /* cnv_rect_size tamanho negativo, significa imagem top down */
    /* calculos adicionados pela Paula */

    /* se posicao no canvas + tamanho no canvas (xmin+1) >= tamanho do canvas, fora da janela, nao desenha nada */
    if (cnv_rect_pos+cnv_rect_size >= canvas_size) 
      return 0;

    /* se posicao da imagem com zoom (xmax) < 0, fora da janela, nao desenha nada */
    if (cnv_rect_pos < 0) 
      return 0;

    /* se posicao com zoom (xmax) >= tamanho do canvas, posicao da imagem com zoom e' o tamanho do canvas menos um
       tambem o tamanho e' reduzido do valor negativo */
    if (cnv_rect_pos >= canvas_size) 
    {
      *new_cnv_rect_pos = canvas_size-1; 
      *new_cnv_rect_size = cnv_rect_size + (cnv_rect_pos - *new_cnv_rect_pos);
    }

    /* se posicao + tamanho com zoom (xmin+1) < 0, 
       entao o tamanho com zoom e' a posição + 1 */
    if (cnv_rect_pos+cnv_rect_size < 0) 
      *new_cnv_rect_size = -(*new_cnv_rect_pos + 1);
  }

  /* agora ja' tenho tamanho e posicao da regiao no canvas,
     tenho que obter tamanho e posicao dentro da imagem original,
     baseado nos novos valores */

  /* tamanho da regiao na imagem e' a conversao de zoom para real do tamanho no canvas */
  *new_img_rect_size = (int)(*new_cnv_rect_size * zoom_factor + 0.5);

  if (is_horizontal)
  {
    /* em X, o offset dentro da imagem so' existe se houver diferenca entre a posicao inicial da
       imagem e a posicao otimizada (ambas da imagem com zoom) */
    if (*new_cnv_rect_pos != cnv_rect_pos)
    {
      offset = *new_cnv_rect_pos - cnv_rect_pos; /* offset is >0 */
      *new_img_rect_pos += (int)(offset*zoom_factor);
    }
  }
  else
  {
    /* em Y, o offset dentro da imagem so' existe se houver diferenca entre a posicao 
       final (posição inicial + tamanho) da imagem e a posicao otimizada (ambas da 
       imagem com zoom) */
    if ((cnv_rect_pos + cnv_rect_size) != (*new_cnv_rect_pos + *new_cnv_rect_size))
    {
      /* offset is >0, because Y axis is from top to bottom */
      offset = (cnv_rect_pos + cnv_rect_size) - (*new_cnv_rect_pos + *new_cnv_rect_size);
      *new_img_rect_pos += (int)(offset*zoom_factor);
    }
  }

  return 1;
}

int cdGetFileName(const char* strdata, char* filename)
{
  const char* start = strdata;
  if (!strdata || strdata[0] == 0) return 0;
  
  if (strdata[0] == '\"')
  {   
    strdata++; /* the first " */
    while(*strdata && *strdata != '\"')
      *filename++ = *strdata++;
    strdata++; /* the last " */
  }
  else
  {
    while(*strdata && *strdata != ' ')
      *filename++ = *strdata++;
  }

  if (*strdata == ' ')
    strdata++;

  *filename = 0;
  return (int)(strdata - start);
}

void cdNormalizeLimits(int w, int h, int *xmin, int *xmax, int *ymin, int *ymax)
{
  *xmin = *xmin < 0? 0: *xmin < w? *xmin: (w - 1);
  *ymin = *ymin < 0? 0: *ymin < h? *ymin: (h - 1);
  *xmax = *xmax < 0? 0: *xmax < w? *xmax: (w - 1);
  *ymax = *ymax < 0? 0: *ymax < h? *ymax: (h - 1);
}

int cdCheckBoxSize(int *xmin, int *xmax, int *ymin, int *ymax)
{
  if (*xmin > *xmax) _cdSwapInt(*xmin, *xmax);
  if (*ymin > *ymax) _cdSwapInt(*ymin, *ymax);

  if ((*xmax-*xmin+1) <= 0)
    return 0;

  if ((*ymax-*ymin+1) <= 0)
    return 0;

  return 1;
}

int cdfCheckBoxSize(double *xmin, double *xmax, double *ymin, double *ymax)
{
  if (*xmin > *xmax) _cdSwapDouble(*xmin, *xmax);
  if (*ymin > *ymax) _cdSwapDouble(*ymin, *ymax);

  if ((*xmax-*xmin+1) <= 0)
    return 0;

  if ((*ymax-*ymin+1) <= 0)
    return 0;

  return 1;
}

void cdMovePoint(int *x, int *y, double dx, double dy, double sin_theta, double cos_theta)
{
  double t;
  t = cos_theta*dx - sin_theta*dy;
  *x += _cdRound(t);
  t = sin_theta*dx + cos_theta*dy;
  *y += _cdRound(t);
}

void cdfMovePoint(double *x, double *y, double dx, double dy, double sin_theta, double cos_theta)
{
  *x += cos_theta*dx - sin_theta*dy;
  *y += sin_theta*dx + cos_theta*dy;
}

void cdRotatePoint(cdCanvas* canvas, int x, int y, int cx, int cy, int *rx, int *ry, double sin_theta, double cos_theta)
{
  double t;

  /* translate to (cx,cy) */
  x = x - cx;
  y = y - cy;

  /* rotate */
  if (canvas->invert_yaxis)
  {
    t =  (x * cos_theta) + (y * sin_theta); *rx = _cdRound(t); 
    t = -(x * sin_theta) + (y * cos_theta); *ry = _cdRound(t);
  }
  else
  {
    t = (x * cos_theta) - (y * sin_theta); *rx = _cdRound(t); 
    t = (x * sin_theta) + (y * cos_theta); *ry = _cdRound(t); 
  }

  /* translate back */
  *rx = *rx + cx;
  *ry = *ry + cy;
}

void cdfRotatePoint(cdCanvas* canvas, double x, double y, double cx, double cy, double *rx, double *ry, double sin_theta, double cos_theta)
{
  /* translate to (cx,cy) */
  x = x - cx;
  y = y - cy;

  /* rotate */
  if (canvas->invert_yaxis)
  {
    *rx =  (x * cos_theta) + (y * sin_theta); 
    *ry = -(x * sin_theta) + (y * cos_theta); 
  }
  else
  {
    *rx = (x * cos_theta) - (y * sin_theta); 
    *ry = (x * sin_theta) + (y * cos_theta); 
  }

  /* translate back */
  *rx = *rx + cx;
  *ry = *ry + cy;
}

void cdRotatePointY(cdCanvas* canvas, int x, int y, int cx, int cy, int *ry, double sin_theta, double cos_theta)
{
  double t;

  /* translate to (cx,cy) */
  x = x - cx;
  y = y - cy;

  /* rotate */
  if (canvas->invert_yaxis)
  {
    t = -(x * sin_theta) + (y * cos_theta); *ry = _cdRound(t);
  }
  else
  {
    t = (x * sin_theta) + (y * cos_theta); *ry = _cdRound(t); 
  }

  /* translate back */
  *ry = *ry + cy;
}

/* Copied from IUP3 */

int cdStrEqualNoCase(const char* str1, const char* str2) 
{
  int i = 0;
  if (str1 == str2) return 1;
  if (!str1 || !str2 || tolower(*str1) != tolower(*str2)) return 0;

  while (str1[i] && str2[i] && tolower(str1[i])==tolower(str2[i])) 
    i++;
  if (str1[i] == str2[i]) return 1; 

  return 0;
}

int cdStrEqualNoCasePartial(const char* str1, const char* str2) 
{
  int i = 0;
  if (str1 == str2) return 1;
  if (!str1 || !str2 || tolower(*str1) != tolower(*str2)) return 0;

  while (str1[i] && str2[i] && tolower(str1[i])==tolower(str2[i])) 
    i++;
  if (str1[i] == str2[i]) return 1; 
  if (str2[i] == 0) return 1;

  return 0;
}

/* Copied from IUP3, simply ignore line breaks other than '\n' for CD */

int cdStrLineCount(const char* str)
{
  int num_lin = 1;

  if (!str)
    return num_lin;

  while(*str != 0)
  {
    while(*str!=0 && *str!='\n')
      str++;

    if (*str=='\n')   /* UNIX line end */
    {
      num_lin++;
      str++;
    }
  }

  return num_lin;
}

char* cdStrDup(const char *str)
{
  if (str)
  {
    int size = strlen(str)+1;
    char *newstr = malloc(size);
    if (newstr) memcpy(newstr, str, size);
    return newstr;
  }
  return NULL;
}

char* cdStrDupN(const char *str, int len)
{
  if (str)
  {
    int size = len+1;
    char *newstr = malloc(size);
    if (newstr) 
    {
      memcpy(newstr, str, len);
      newstr[len]=0;
    }
    return newstr;
  }
  return NULL;
}

void cdSetPaperSize(int size, double *w_pt, double *h_pt)
{
  static struct
  {
    int w_pt;
    int h_pt;
  } paper[] =
    {
      { 2393, 3391 },   /*   A0   */
      { 1689, 2393 },   /*   A1   */
      { 1192, 1689 },   /*   A2   */
      {  842, 1192 },   /*   A3   */
      {  595,  842 },   /*   A4   */
      {  420,  595 },   /*   A5   */
      {  612,  792 },   /* LETTER */
      {  612, 1008 }    /*  LEGAL */
    };

  if (size<CD_A0 || size>CD_LEGAL) 
    return;

  *w_pt = (double)paper[size].w_pt;
  *h_pt = (double)paper[size].h_pt;
}

#ifdef WIN32
#include <windows.h>
static int sReadStringKey(HKEY base_key, char* key_name, char* value_name, char* value)
{
	HKEY key;
	DWORD max_size = 512;

	if (RegOpenKeyEx(base_key, key_name, 0, KEY_READ, &key) != ERROR_SUCCESS)
		return 0;

  if (RegQueryValueEx(key, value_name, NULL, NULL, (LPBYTE)value, &max_size) != ERROR_SUCCESS)
  {
    RegCloseKey(key);
		return 0;
  }

	RegCloseKey(key);
	return 1;
}

static char* sGetFontDir(void)
{
  static char font_dir[512];
  if (!sReadStringKey(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders", "Fonts", font_dir))
    return "";
  else
  {
    int i, size = (int)strlen(font_dir);
    for(i = 0; i < size; i++)
    {
      if (font_dir[i] == '\\')
        font_dir[i] = '/';
    }
    return font_dir;
  }
}
#endif

int cdGetFontFileName(const char* font, char* filename)
{
  FILE *file;

  /* current directory */
  sprintf(filename, "%s.ttf", font);
  file = fopen(filename, "r");

  if (file)
    fclose(file);
  else
  {
    /* CD environment */
    char* env = getenv("CDDIR");
    if (env)
    {
      sprintf(filename, "%s/%s.ttf", env, font);
      file = fopen(filename, "r");
    }

    if (file)
      fclose(file);
    else
    {
#ifdef WIN32
      /* Windows Font folder */
      sprintf(filename, "%s/%s.ttf", sGetFontDir(), font);
      file = fopen(filename, "r");

      if (file)
        fclose(file);
      else
        return 0;
#else
      return 0;
#endif
    }
  }

  return 1;
}

int cdStrTmpFileName(char* filename)
{
#ifdef WIN32
  char tmpPath[10240];
  if (GetTempPath(10240, tmpPath)==0)
    return 0;
  if (GetTempFileName(tmpPath, "~cd", 0, filename)==0)
    return 0;
  return 1;
#else
  return tmpnam(filename)!=NULL;
#endif
}
