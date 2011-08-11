/** \file
 * \brief Windows DIB Utilities
 *
 * See Copyright Notice in cd.h
 */

#include "cdwin.h"


/*
%F Calcula o tamanho de uma linha. O DIB existe em uma "long boundary", 
ou seja cada linha e' um multiplo de quatro bytes ou 32 bits.
*/
static int cdwDIBLineSize(int width, int bpp)
{
  return ((width * bpp + 31L) / 32L) * 4L;
}


/*
%F Alloca memoria para o DIB com os par^ametros do cdwDIB. 
*/
int cdwCreateDIB(cdwDIB* dib)
{
  int dibSize, pal_size;
  BITMAPINFOHEADER* bmih;        
  
  pal_size = dib->type? 256: 0;  
  dibSize = sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * pal_size + cdwDIBLineSize(dib->w, dib->type? 8: 24) * dib->h;
  
  dib->dib = (BYTE*) calloc(dibSize, 1);
  if (dib->dib == NULL)
    return 0;
  
  dib->bmi = (BITMAPINFO*)dib->dib;
  dib->bmih = (BITMAPINFOHEADER*)dib->dib;
  dib->bmic = (RGBQUAD*)(dib->dib + sizeof(BITMAPINFOHEADER));
  dib->bits = dib->dib + sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * pal_size;
  
  bmih = dib->bmih;
  bmih->biSize = sizeof(BITMAPINFOHEADER);
  bmih->biWidth = dib->w;
  bmih->biHeight = dib->h;
  bmih->biPlanes = 1;
  bmih->biBitCount = dib->type? 8: 24;
  bmih->biCompression = 0;
  bmih->biSizeImage = 0;
  bmih->biXPelsPerMeter = 0;
  bmih->biYPelsPerMeter = 0;
  bmih->biClrUsed = dib->type? 256: 0;
  bmih->biClrImportant = dib->type? 256: 0;
  
  return 1;
}

HANDLE cdwCreateCopyHDIB(BITMAPINFO* bmi, BYTE* bits)
{
  int dibSize, pal_size, headerSize;
  HANDLE hDib; unsigned char* pDib;
  
  if (bmi->bmiHeader.biBitCount > 8)
  {
    pal_size = 0; 
    
    if (bmi->bmiHeader.biCompression == BI_BITFIELDS)
      pal_size = 3; 
  }
  else
  {
    if (bmi->bmiHeader.biClrUsed != 0)
      pal_size = bmi->bmiHeader.biClrUsed;
    else
      pal_size = 1 << bmi->bmiHeader.biBitCount;
  }

  /* calc size */
  headerSize = sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * pal_size;
  dibSize = headerSize + cdwDIBLineSize(bmi->bmiHeader.biWidth, bmi->bmiHeader.biBitCount) * bmi->bmiHeader.biHeight;
  
  hDib = GlobalAlloc(GHND, dibSize);
	if (!hDib)
		return NULL;

	/* Get a pointer to the memory block */
	pDib = (LPBYTE)GlobalLock(hDib);	 

  /* copy struct data */
  CopyMemory(pDib, bmi, headerSize);

  /* copy dib data */
  CopyMemory(pDib + headerSize, bits, dibSize - headerSize);

	GlobalUnlock(hDib);	 
  
  return hDib;
}

int cdwCreateDIBRefBuffer(cdwDIB* dib, unsigned char* *bits, int *size)
{
  int dibSize, pal_size;
  BITMAPINFOHEADER* bmih;        
  
  pal_size = dib->type? 256: 0; 
  dibSize = sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * pal_size + cdwDIBLineSize(dib->w, dib->type? 8: 24) * dib->h;
  
  /* bits may contains an allocated buffer, but no dib */
  if (*bits && *size >= dibSize)
    dib->dib = *bits;
  else
  {
    *size = dibSize;

    if (*bits)
      *bits = realloc(*bits, *size);
    else
      *bits = malloc(*size);

    dib->dib = *bits;
  }

  if (dib->dib == NULL)
    return 0;
  
  dib->bmi = (BITMAPINFO*)dib->dib;
  dib->bmih = (BITMAPINFOHEADER*)dib->dib;
  dib->bmic = (RGBQUAD*)(dib->dib + sizeof(BITMAPINFOHEADER));
  dib->bits = dib->dib + sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * pal_size;
  
  bmih = dib->bmih;
  bmih->biSize = sizeof(BITMAPINFOHEADER);
  bmih->biWidth = dib->w;
  bmih->biHeight = dib->h;
  bmih->biPlanes = 1;
  bmih->biBitCount = dib->type? 8: 24;
  bmih->biCompression = 0;
  bmih->biSizeImage = 0;
  bmih->biXPelsPerMeter = 0;
  bmih->biYPelsPerMeter = 0;
  bmih->biClrUsed = dib->type? 256: 0;
  bmih->biClrImportant = dib->type? 256: 0;
  
  return 1;
}

void cdwCreateDIBRefBits(cdwDIB* dib, unsigned char *bits)
{
  BITMAPINFO* bmi = malloc(sizeof(BITMAPINFO));

  bmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bmi->bmiHeader.biWidth = dib->w;
  bmi->bmiHeader.biHeight = dib->h;
  bmi->bmiHeader.biPlanes = 1;
  bmi->bmiHeader.biBitCount = dib->type==0? 24: 32;
  bmi->bmiHeader.biCompression = BI_RGB;
  bmi->bmiHeader.biXPelsPerMeter = 0;
  bmi->bmiHeader.biYPelsPerMeter = 0;
  bmi->bmiHeader.biSizeImage = 0;
  bmi->bmiHeader.biClrUsed = 0;
  bmi->bmiHeader.biClrImportant = 0;

  cdwDIBReference(dib, (BYTE*)bmi, bits);

    /* restore correct type */
    if (bmi->bmiHeader.biBitCount == 32)
      dib->type = 2;
}

HBITMAP cdwCreateDIBSection(cdwDIB* dib, HDC hDC)
{
  HBITMAP hbitmap;
  BYTE *pvBits;
  BITMAPINFO* bmi = malloc(sizeof(BITMAPINFO));

  bmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bmi->bmiHeader.biWidth = dib->w;
  bmi->bmiHeader.biHeight = dib->h;
  bmi->bmiHeader.biPlanes = 1;
  bmi->bmiHeader.biBitCount = dib->type==0? 24: 32;
  bmi->bmiHeader.biCompression = BI_RGB;
  bmi->bmiHeader.biXPelsPerMeter = (long)(GetDeviceCaps(hDC, LOGPIXELSX) / 0.0254);
  bmi->bmiHeader.biYPelsPerMeter = (long)(GetDeviceCaps(hDC, LOGPIXELSY) / 0.0254);
  bmi->bmiHeader.biSizeImage = 0;
  bmi->bmiHeader.biClrUsed = 0;
  bmi->bmiHeader.biClrImportant = 0;

  hbitmap = CreateDIBSection(hDC, bmi, DIB_RGB_COLORS, &pvBits, NULL, 0x0);

  if (hbitmap)
  {
    cdwDIBReference(dib, (BYTE*)bmi, pvBits);

    /* restore correct type */
    if (bmi->bmiHeader.biBitCount == 32)
      dib->type = 2;
  }
  else
    free(bmi);

  return hbitmap;
}

void cdwDIBReference(cdwDIB* dib, BYTE* bmi, BYTE* bits)
{
  int pal_size;
  
  dib->dib = bmi;
  
  dib->bmi = (BITMAPINFO*)bmi;
  dib->bmih = &dib->bmi->bmiHeader;
  dib->bmic = dib->bmi->bmiColors;
  
  if (dib->bmih->biBitCount > 8)
  {
    dib->type = 0;
    pal_size = 0; 
    
    if (dib->bmih->biCompression == BI_BITFIELDS)
      pal_size = 3; 
  }
  else
  {
    dib->type = 1;
    
    if (dib->bmih->biClrUsed != 0)
      pal_size = dib->bmih->biClrUsed;
    else
      pal_size = 1 << dib->bmih->biBitCount;
  }
  
  if (bits == NULL)
    dib->bits = dib->dib + sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * pal_size;
  else
    dib->bits = bits;
  
  dib->w = dib->bmih->biWidth;
  dib->h = dib->bmih->biHeight;
}


/* %F Libera a memoria alocada para o DIB. */
void cdwKillDIB(cdwDIB* dib)
{
  free(dib->dib);
}

/* %F Converte cor de CD para DIB. */
static RGBQUAD sColorToDIB(long cd_color)
{
  RGBQUAD color;
  color.rgbRed = cdRed(cd_color);
  color.rgbGreen = cdGreen(cd_color);
  color.rgbBlue = cdBlue(cd_color);
  return color;
}

void cdwDIBEncodeRGBRect(cdwDIB* dib, const unsigned char *red, const unsigned char *green, const unsigned char *blue, int xi, int yi, int wi, int hi)
{
  int x,y, resto1, resto2, offset;
  BYTE* bits;
  
  bits = dib->bits;
  resto1 = cdwDIBLineSize(dib->w, 24) - dib->w * 3;
  resto2 = wi - dib->w;

  offset = wi * yi + xi;
  
  red = red + offset;
  green = green + offset;
  blue = blue + offset;

  for (y = 0; y < dib->h; y++)
  {
    for (x = 0; x < dib->w; x++)
    {
      *bits++ = *blue++;
      *bits++ = *green++;
      *bits++ = *red++;
    }
    
    bits += resto1;

    red += resto2;
    green += resto2;
    blue += resto2;
  }
}

/* RGB in RGBA DIBs are pre-multiplied by alpha to AlphaBlend usage. */
#define CD_ALPHAPRE(_src, _alpha) (((_src)*(_alpha))/255)

void cdwDIBEncodeRGBARect(cdwDIB* dib, const unsigned char *red, const unsigned char *green, const unsigned char *blue, const unsigned char *alpha, int xi, int yi, int wi, int hi)
{
  int x,y, resto1, resto2, offset;
  BYTE* bits;
  
  bits = dib->bits;
  resto1 = cdwDIBLineSize(dib->w, 32) - dib->w * 4;
  resto2 = wi - dib->w;

  offset = wi * yi + xi;
  
  red = red + offset;
  green = green + offset;
  blue = blue + offset;
  alpha = alpha + offset;

  for (y = 0; y < dib->h; y++)
  {
    for (x = 0; x < dib->w; x++)
    {
      *bits++ = CD_ALPHAPRE(*blue, *alpha);   blue++;
      *bits++ = CD_ALPHAPRE(*green, *alpha);  green++;
      *bits++ = CD_ALPHAPRE(*red, *alpha);    red++;
      *bits++ = *alpha++;
    }
    
    bits += resto1;

    red += resto2;
    green += resto2;
    blue += resto2;
    alpha += resto2;
  }
}

void cdwDIBEncodeRGBARectMirror(cdwDIB* dib, const unsigned char *red, const unsigned char *green, const unsigned char *blue, const unsigned char *alpha, int xi, int yi, int wi, int hi)
{
  int x,y, resto1, resto2, offset, line_size;
  BYTE* bits;
  
  line_size = cdwDIBLineSize(dib->w, 32);

  bits = dib->bits + line_size*(dib->h-1);
  resto1 = line_size - dib->w * 4;
  resto2 = wi - dib->w;

  offset = wi * yi + xi;
  
  red = red + offset;
  green = green + offset;
  blue = blue + offset;
  alpha = alpha + offset;

  for (y = 0; y < dib->h; y++)
  {
    for (x = 0; x < dib->w; x++)
    {
      *bits++ = CD_ALPHAPRE(*blue, *alpha);   blue++;
      *bits++ = CD_ALPHAPRE(*green, *alpha);  green++;
      *bits++ = CD_ALPHAPRE(*red, *alpha);    red++;
      *bits++ = *alpha++;
    }
    
    bits += resto1;
    bits -= 2*line_size;

    red += resto2;
    green += resto2;
    blue += resto2;
    alpha += resto2;
  }
}

void cdwDIBEncodeAlphaRect(cdwDIB* dib, const unsigned char *alpha, int xi, int yi, int wi, int hi)
{
  int x,y, resto1, resto2, offset;
  BYTE* bits;
  
  bits = dib->bits;
  resto1 = cdwDIBLineSize(dib->w, 32) - dib->w * 4;
  resto2 = wi - dib->w;

  offset = wi * yi + xi;
  
  alpha = alpha + offset;

  for (y = 0; y < dib->h; y++)
  {
    for (x = 0; x < dib->w; x++)
    {
      *bits++ = CD_ALPHAPRE(*bits, *alpha); 
      *bits++ = CD_ALPHAPRE(*bits, *alpha); 
      *bits++ = CD_ALPHAPRE(*bits, *alpha); 
      *bits++ = *alpha++;
    }
    
    bits += resto1;
    alpha += resto2;
  }
}

void cdwDIBEncodeRGBARectZoom(cdwDIB* dib, const unsigned char *red, const unsigned char *green, const unsigned char *blue, const unsigned char *alpha, int w, int h, int xi, int yi, int wi, int hi)
{
  int x,y, resto1, resto2, offset;
  BYTE* bits;
  const unsigned char *_red, *_green, *_blue, *_alpha;
  unsigned char a;
  
  bits = dib->bits;
  resto1 = cdwDIBLineSize(dib->w, 24) - dib->w * 3;

  if (dib->w != wi || dib->h != hi)
  {
    int* XTab = cdGetZoomTable(dib->w, wi, xi);
    int* YTab = cdGetZoomTable(dib->h, hi, yi);
    
    for (y = 0; y < dib->h; y++)
    {
      offset = YTab[y] * w;
      _red = red + offset;
      _green = green + offset;
      _blue = blue + offset;
      _alpha = alpha + offset;
      
      for (x = 0; x < dib->w; x++)
      {
        offset = XTab[x];
        a = _alpha[offset];
        *bits++ = CD_ALPHA_BLEND(_blue[offset], *bits, a);
        *bits++ = CD_ALPHA_BLEND(_green[offset], *bits, a);
        *bits++ = CD_ALPHA_BLEND(_red[offset], *bits, a);
      }
      
      bits += resto1;
    }
    
    free(XTab);
    free(YTab);
  }
  else
  {
    resto2 = w - wi;

    offset = w * yi + xi;
    red = red + offset;
    green = green + offset;
    blue = blue + offset;
    alpha = alpha + offset;
  
    for (y = 0; y < dib->h; y++)
    {
      for (x = 0; x < dib->w; x++)
      {
        a = *alpha++;
        *bits++ = CD_ALPHA_BLEND(*blue++, *bits, a);
        *bits++ = CD_ALPHA_BLEND(*green++, *bits, a);
        *bits++ = CD_ALPHA_BLEND(*red++, *bits, a);
      }
      
      bits += resto1;

      red += resto2;
      green += resto2;
      blue += resto2;
      alpha += resto2;
    }
  }
}

/*
%F Copia os pixels de um DIB em 3 matrizes red, green e 
blue, respectivamente. As matrizes, armazenadas num vetor de bytes devem
ter a mesma dimens~ao da imagem.
*/
void cdwDIBDecodeRGB(cdwDIB* dib, unsigned char *red, unsigned char *green, unsigned char *blue)
{
  int x,y, offset;
  unsigned short color;
  BYTE* bits;
  unsigned long rmask=0, gmask=0, bmask=0, 
    roff = 0, goff = 0, boff = 0; /* pixel bit mask control when reading 16 and 32 bpp images */
  
  bits = dib->bits;
  
  if (dib->bmih->biBitCount == 16)
    offset = cdwDIBLineSize(dib->w, dib->bmih->biBitCount);
  else
    offset = cdwDIBLineSize(dib->w, dib->bmih->biBitCount) - dib->w * (dib->bmih->biBitCount == 24?3:4);
  
  if (dib->bmih->biCompression == BI_BITFIELDS)
  {
    unsigned long Mask;
    unsigned long* palette = (unsigned long*)dib->bmic;
    
    rmask = Mask = palette[0];
    while (!(Mask & 0x01))
    {Mask >>= 1; roff++;}
    
    gmask = Mask = palette[1];
    while (!(Mask & 0x01))
    {Mask >>= 1; goff++;}
    
    bmask = Mask = palette[2];
    while (!(Mask & 0x01))
    {Mask >>= 1; boff++;}
  }
  else if (dib->bmih->biBitCount == 16)
  {
    bmask = 0x001F;
    gmask = 0x03E0;
    rmask = 0x7C00;
    boff = 0;
    goff = 5;
    roff = 10;
  }
  
  for (y = 0; y < dib->h; y++)
  {
    for (x = 0; x < dib->w; x++)
    {
      if (dib->bmih->biBitCount != 16)
      {
        *blue++ = *bits++;
        *green++ = *bits++;
        *red++ = *bits++;
        
        if (dib->bmih->biBitCount == 32)
          bits++;
      }
      else
      {
        color = ((unsigned short*)bits)[x];
        *red++ = (unsigned char)((((rmask & color) >> roff) * 255) / (rmask >> roff));
        *green++ = (unsigned char)((((gmask & color) >> goff) * 255) / (gmask >> goff));
        *blue++ = (unsigned char)((((bmask & color) >> boff) * 255) / (bmask >> boff));
      }
    }
    
    bits += offset;
  }
}

void cdwDIBDecodeMap(cdwDIB* dib, unsigned char *index, long *colors)
{
  int x,y, line_size,c,pal_size;
  BYTE* bits;
  RGBQUAD* bmic;
  
  bmic = dib->bmic;
  bits = dib->bits;
  line_size = cdwDIBLineSize(dib->w, dib->bmih->biBitCount);
  pal_size = dib->bmih->biClrUsed != 0? dib->bmih->biClrUsed: 1 << dib->bmih->biBitCount;
  
  for (y = 0; y < dib->h; y++)
  {
    for (x = 0; x < dib->w; x++)
    {
      switch (dib->bmih->biBitCount)
      {
      case 1:
        *index++ = (unsigned char)((bits[x / 8] >> (7 - x % 8)) & 0x01);
        break;
      case 4:
        *index++ = (unsigned char)((bits[x / 2] >> ((1 - x % 2) * 4)) & 0x0F);
        break;
      case 8:
        *index++ = bits[x];
        break;
      }
    }
    
    bits += line_size;
  }
  
  for (c = 0; c < pal_size; c++)
  {
    colors[c] = cdEncodeColor(bmic->rgbRed, bmic->rgbGreen, bmic->rgbBlue);
    bmic++;
  }
}

/*
%F Cria uma Logical palette a partir da palette do DIB.
*/
HPALETTE cdwDIBLogicalPalette(cdwDIB* dib)
{
  LOGPALETTE* pLogPal;      
  PALETTEENTRY* pPalEntry;
  HPALETTE hPal;
  RGBQUAD* bmic;
  int c;
  
  pLogPal = (LOGPALETTE*)malloc(sizeof(LOGPALETTE) + 256 * sizeof(PALETTEENTRY));
  pLogPal->palVersion    = 0x300; 
  pLogPal->palNumEntries = 256;
  
  bmic = dib->bmic;
  pPalEntry = pLogPal->palPalEntry;
  
  for (c = 0; c < 256; c++) 
  {
    pPalEntry->peRed   = bmic->rgbRed;
    pPalEntry->peGreen = bmic->rgbGreen;
    pPalEntry->peBlue  = bmic->rgbBlue;
    pPalEntry->peFlags = PC_NOCOLLAPSE;
    
    pPalEntry++;
    bmic++;
  }
  
  hPal = CreatePalette(pLogPal);
  free(pLogPal);
  
  return hPal;
}

/*
%F Copia os pixels definidos por uma matriz de indices e palheta de
de cores num DIB de mesma dimens~ao. 
*/
void cdwDIBEncodeMap(cdwDIB* dib, unsigned char *index, long int *colors)
{
  int x,y, pal_size, resto, c;
  BYTE* bits;
  RGBQUAD* bmic;
  
  bits = dib->bits;
  bmic = dib->bmic;
  resto = cdwDIBLineSize(dib->w, 8) - dib->w;
  
  /* Como nao sabemos o tamanho da palette a priori, 
  teremos que ver qual o maior indice usado na imagem. */
  pal_size = *index;
  
  for (y = 0; y < dib->h; y++)
  {
    for (x = 0; x < dib->w; x++)
    {
      if (*index > pal_size)
        pal_size = *index;
      
      *bits++ = *index++;
    }
    
    bits += resto;
  }
  
  pal_size++;
  
  for (c = 0; c < pal_size; c++)
    *bmic++ = sColorToDIB(colors[c]);
}

void cdwDIBEncodeMapRect(cdwDIB* dib, const unsigned char *index, const long int *colors, int xi, int yi, int wi, int hi)
{
  int x,y, pal_size, resto1, resto2, c;
  BYTE* bits;
  RGBQUAD* bmic;
  
  bits = dib->bits;
  bmic = dib->bmic;
  resto1 = cdwDIBLineSize(dib->w, 8) - dib->w;
  resto2 = wi - dib->w;

  index = index + (wi * yi + xi);
  
  /* Como nao sabemos o tamanho da palette a priori, 
  teremos que ver qual o maior indice usado na imagem. */
  pal_size = *index;
  
  for (y = 0; y < dib->h; y++)
  {
    for (x = 0; x < dib->w; x++)
    {
      if (*index > pal_size)
        pal_size = *index;
      
      *bits++ = *index++;
    }
    
    bits += resto1;
    index += resto2;
  }
  
  pal_size++;
  
  for (c = 0; c < pal_size; c++)
    *bmic++ = sColorToDIB(colors[c]);
}

void cdwDIBEncodePattern(cdwDIB* dib, const long int *colors)
{
  int x,y, resto1;
  BYTE* bits;
  
  bits = dib->bits;
  resto1 = cdwDIBLineSize(dib->w, 24) - dib->w * 3;
  
  for (y = 0; y < dib->h; y++)
  {
    for (x = 0; x < dib->w; x++)
    {
      *bits++ = cdBlue(*colors);
      *bits++ = cdGreen(*colors);
      *bits++ = cdRed(*colors);
      colors++;
    }
    bits += resto1;
  }
}
