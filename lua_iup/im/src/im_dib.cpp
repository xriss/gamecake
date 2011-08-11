/** \file
 * \brief Windows DIB
 *
 * See Copyright Notice in im_lib.h
 * $Id: im_dib.cpp,v 1.2 2010/01/26 18:06:01 scuri Exp $
 */


#include <windows.h>
#include <assert.h>
#include <stdlib.h>

#include "im_dib.h"

/*****************
  Private Funtions
*****************/

/* Long returned in getpixel is an array of 4 bytes, not actually a DWORD */
/* 32 bpp max */
/* Windows use Little Endian always, this means LSB first: 0xF3F2F1F0 = "F0F1F2F3" */

#define iSETDWORD(_vLong, _Line, _Nb)                    \
        {                                                  \
          unsigned char* _pLong = (unsigned char*)&_vLong; \
          int b = _Nb;                                     \
          while(b--)                                       \
            *_pLong++ = *_Line++;                          \
        }

#define iGETDWORD(_vLong, _Line, _Nb)                    \
        {                                                  \
          unsigned char* _pLong = (unsigned char*)&_vLong; \
          int b = _Nb;                                     \
          while(b--)                                       \
            *_Line++ = *_pLong++;                          \
        }

#define iGETDWORDMASK(_vLong, _vMask, _Line, _Nb)        \
        {                                                  \
          unsigned char* _pLong = (unsigned char*)&_vLong; \
          unsigned char* _pMask = (unsigned char*)&_vMask; \
          int b = _Nb;                                     \
          while(b--)                                       \
            *_Line++ = *_pLong++ | (~*_pMask++ & *_Line);  \
        }

static unsigned int iMakeBitMask(int bpp)
{
  unsigned int mask = 1;

  while (bpp > 1)
  {
    mask = (mask << 1) + 1;
    bpp--;
  }

  return mask;
}

static unsigned int iLineGetPixel1(unsigned char* line, int col)
{
  return (line[col / 8] >> (7 - col % 8)) & 0x01;        /* LSB is filled */
}

static void iLineSetPixel1(unsigned char* line, int col, unsigned int pixel)
{
  if (pixel)                                             /* only test 1/0 */
    line[col / 8] |= (0x01 << (7 - (col % 8)));
  else
    line[col / 8] &= (0xFE << (7 - (col % 8)));
}

static unsigned int iLineGetPixel4(unsigned char* line, int col)
{
  return (line[col / 2] >> ((1 - col % 2) * 4)) & 0x0F;  /* LSB is filled */
}

static void iLineSetPixel4(unsigned char* line, int col, unsigned int pixel)
{
  unsigned char mask = (col % 2)? 0xF0: 0x0F;            /* LSB is used */
  line[col/2] = (unsigned char)((mask & (((unsigned char)pixel) << ((1 - col % 2) * 4))) | (~mask & line[col/2]));
}

static unsigned int iLineGetPixel8(unsigned char* line, int col)
{
  return line[col];                                      /* LSB is filled */
}

static void iLineSetPixel8(unsigned char* line, int col, unsigned int pixel)
{
  line[col] = (unsigned char)pixel;                      /* LSB is used */
}

static unsigned int iLineGetPixel16(unsigned char* line, int col)
{
  return ((unsigned short*)line)[col];                   /* 0xF1F0 => "F0F10000" */
}

static void iLineSetPixel16(unsigned char* line, int col, unsigned int pixel)
{
  ((unsigned short*)line)[col] = (unsigned short)pixel;  /* inverse of above */
}

static unsigned int iLineGetPixel24(unsigned char* line, int col)
{
  unsigned int pixel = 0;
  line += col*3;
  iSETDWORD(pixel, line, 3);
  return pixel;
}

static void iLineSetPixel24(unsigned char* line, int col, unsigned int pixel)
{
  line += col*3;
  iGETDWORD(pixel, line, 3);
}

static unsigned int iLineGetPixel32(unsigned char* line, int col)
{
  return ((unsigned int*)line)[col];                    /* direct mapping */
}

static void iLineSetPixel32(unsigned char* line, int col, unsigned int pixel)
{
  ((unsigned int*)line)[col] = pixel;                   /* direct mapping */
}

static int iGetPixelAnyBpp = 0;
static unsigned int iGetPixelAnyMask = 0;

static unsigned int iAnyGet(unsigned char* line, int col, int bpp)
{
  int s_byte = (col*bpp) >> 3;
  int s_bit = (col*bpp) & 0x7;
  unsigned int pixel = 0;
  unsigned int mask = (~0) >> (32-bpp);
  int n_bytes = (bpp + s_bit + 7) >> 3;
  int shift = (n_bytes << 3) - bpp - s_bit;
  line += s_byte;
  while (n_bytes)
  {
    pixel |= *line++;
    if (--n_bytes > 0) pixel <<= 8;
    else break;
  }
  pixel >>= shift;
  return pixel & mask;
}

static void iAnySet(unsigned char* line, int col, int bpp, unsigned int pixel)
{
  int s_byte = (col*bpp) >> 3;
  int s_bit = (col*bpp) & 0x7;
  unsigned int mask = (~0) >> (32-bpp);
  int n_bytes = (bpp + s_bit + 7) >> 3;
  int shift = (n_bytes << 3) - bpp - s_bit;
  unsigned char* p_pixel = (unsigned char*) &pixel, *p_mask = (unsigned char*) &mask;
  line += s_byte + n_bytes - 1;
  pixel <<= shift;
  mask <<= shift;
  while (n_bytes--) {
    *line = (*line & ~(*p_mask)) | (*p_pixel & *p_mask);
    p_mask++; p_pixel++; line--;
  }
}

static unsigned int iLineGetPixelAny(unsigned char* line, int col)
{
  return iAnyGet(line, col, iGetPixelAnyBpp);
#if 0
  unsigned int pixel = 0;
  int rbits  = (col * iGetPixelAnyBpp) % 8;       /* calc remaining bits */
  line      += (col * iGetPixelAnyBpp) / 8;       /* position pointer */

  /* transfer from pixel line to a DWORD in little endian, so it can be shifted */
  {
    int nbytes = (iGetPixelAnyBpp + rbits + 7) / 8; /* bytes used */
    iSETDWORD(pixel, line, nbytes);
  }

  /* shift down pixel remaining bits and mask extra non pixel bits */
  return (pixel >> rbits) & iGetPixelAnyMask;
#endif
}

static int iSetPixelAnyBpp = 0;
static unsigned int iSetPixelAnyMask = 0;

static void iLineSetPixelAny(unsigned char* line, int col, unsigned int pixel)
{
  iAnySet(line, col, iSetPixelAnyBpp, pixel);
#if 0
  int rbits  = (col * iSetPixelAnyBpp) % 8;       /* calc remaining bits */
  line      += (col * iSetPixelAnyBpp) / 8;       /* position pointer */

  pixel = pixel << rbits; /* position bits */

  {
    unsigned int mask = iSetPixelAnyMask << rbits; /* position mask */
    int nbytes = (iGetPixelAnyBpp + rbits + 7) / 8; /* bytes used */
    iGETDWORDMASK(pixel, mask, line, nbytes);
  }                      
#endif
}

static long iQuad2Long(RGBQUAD* quad_color)
{
  return (((unsigned long)quad_color->rgbRed) << 16) |
         (((unsigned long)quad_color->rgbGreen) <<  8) |
         (((unsigned long)quad_color->rgbBlue) <<  0);
}

static RGBQUAD iLong2Quad(long long_color)
{
  RGBQUAD quad_color;
  
  quad_color.rgbRed = (unsigned char)(((long_color) >> 16) & 0xFF);
  quad_color.rgbGreen = (unsigned char)(((long_color) >>  8) & 0xFF);
  quad_color.rgbBlue = (unsigned char)(((long_color) >>  0) & 0xFF);
 
  return quad_color;
}

static int iImageLineSize(int width, int bpp)
{
  return (width * bpp + 7) / 8;            /* 1 byte boundary */
}

static int iLineSize(int width, int bpp)
{
  return ((width * bpp + 31) / 32) * 4;   /* 4 bytes boundary */
}

static void iInitHeadersReference(imDib* dib)
{
  dib->bmi = (BITMAPINFO*)dib->dib;
  dib->bmih = (BITMAPINFOHEADER*)dib->dib;
  dib->bmic = (RGBQUAD*)(dib->dib + sizeof(BITMAPINFOHEADER));
}

static void iInitSizes(imDib* dib, int width, int height, int bpp)
{
  dib->line_size = iLineSize(width, bpp);
  dib->pad_size = dib->line_size - iImageLineSize(width, bpp);
  dib->bits_size = dib->line_size * height;
  dib->size = sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * dib->palette_count + dib->bits_size;
}

static void iInitInfoHeader(BITMAPINFOHEADER* bmih, int width, int height, int bpp, int palette_count)
{
  bmih->biSize = sizeof(BITMAPINFOHEADER);
  bmih->biWidth = width;
  bmih->biHeight = height;
  bmih->biPlanes = 1;
  bmih->biBitCount = (WORD)bpp;
  bmih->biCompression = 0;
  bmih->biSizeImage = 0;
  bmih->biClrUsed = palette_count;
  bmih->biClrImportant = 0;

  {
    HDC ScreenDC = GetDC(NULL);

    bmih->biXPelsPerMeter = (unsigned int)(GetDeviceCaps(ScreenDC, LOGPIXELSX) / 0.0254);
    bmih->biYPelsPerMeter = (unsigned int)(GetDeviceCaps(ScreenDC, LOGPIXELSY) / 0.0254);

    ReleaseDC(NULL, ScreenDC);
  }
}

static void iInitBits(imDib* dib, BYTE* bits)
{
  if (bits == NULL)
    dib->bits = dib->dib + sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * dib->palette_count;
  else
    dib->bits = bits;
}

static int iGetValidBpp(int bpp)
{
  if (bpp == 1) 
    bpp = 1; 
  else if (bpp <= 4) 
    bpp = 4; 
  else if (bpp <= 8) 
    bpp = 8; 
  else if (bpp <= 16) 
    bpp = 16; 
  else if (bpp <= 24) 
    bpp = 24; 
  else if (bpp <= 32)
    bpp = 32; 
  else
    bpp = 0;

  return bpp;
}

static int iCheckHeader(BITMAPINFOHEADER *bmih)
{
  if (bmih->biSize != sizeof(BITMAPINFOHEADER))
    return 0;

  if (bmih->biWidth <= 0)
    return 0;

  if (bmih->biHeight == 0)
    return 0;

  {
    int bpp = iGetValidBpp(bmih->biBitCount);
    if (!bpp)
      return 0;

    if (bmih->biCompression == BI_RLE8 && bpp != 8)
      return 0;

    if (bmih->biCompression == BI_RLE4 && bpp != 4)
      return 0;

    if (bmih->biCompression == BI_BITFIELDS && (bpp != 16 || bpp != 32))
      return 0;

    if (bmih->biHeight < 0 && (bmih->biCompression == BI_RLE8 || bmih->biCompression == BI_RLE4))
      return 0;

/*    if (bmih->biCompression == BI_JPEG || bmih->biCompression == BI_PNG)
      return 0; */
  }

  return 1;
}

/*****************
  Creation
*****************/

static void AllocDib(imDib* dib) 
{
  dib->dib = NULL;
  dib->handle = GlobalAlloc(GMEM_MOVEABLE, dib->size); 
  if (!dib->handle) return;
  dib->dib = (BYTE*)GlobalLock(dib->handle); 
}

imDib* imDibCreate(int width, int height, int bpp)
{
  imDib* dib;
  int obpp = bpp;

  bpp = iGetValidBpp(abs(bpp));
  
  assert(width > 0 && height > 0);
  assert(bpp);

  dib = (imDib*)malloc(sizeof(imDib));

  if (bpp > 8)
  {
    if ((bpp == 16 || bpp == 32) && obpp < 0)
      dib->palette_count = 3;
    else
      dib->palette_count = 0;
  }
  else
    dib->palette_count = 1 << bpp;
  
  iInitSizes(dib, width, height, bpp);
                         
  AllocDib(dib);
  if (dib->dib == NULL)
  {
    free(dib);
    return NULL;
  }

  iInitHeadersReference(dib);

  iInitInfoHeader(dib->bmih, width, height, bpp, dib->palette_count);

  iInitBits(dib, NULL);

  dib->is_reference = 0;
  
  return dib;
}

imDib* imDibCreateSection(HDC hDC, HBITMAP *bitmap, int width, int height, int bpp)
{
  BITMAPINFO* bmi;
  BYTE* bits;
  int palette_count;
  int obpp = bpp;

  bpp = iGetValidBpp(abs(bpp));

  assert(hDC);
  assert(width > 0 && height > 0);
  assert(bpp);

  if (bpp > 8)
  {
    if ((bpp == 16 || bpp == 32) && obpp < 0)
      palette_count = 3;  
    else
      palette_count = 0;
  }
  else
    palette_count = 1 << bpp;

  bmi = (BITMAPINFO*)malloc(sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * palette_count);

  iInitInfoHeader(&bmi->bmiHeader, width, height, bpp, palette_count);

  if (bpp > 8 && palette_count == 3)
  {
    DWORD *masks = (DWORD*)(bmi + sizeof(BITMAPINFOHEADER));
    masks[0] = 0x001F;
    masks[1] = 0x03E0;
    masks[2] = 0x7C00;
  }

  *bitmap = CreateDIBSection(hDC, bmi, DIB_RGB_COLORS, (void**)&bits, NULL, 0);

  {
    imDib* dib;
    dib = imDibCreateReference((BYTE*)bmi, bits);
    dib->is_reference = 0;
    return dib;
  }
}


imDib* imDibCreateCopy(const imDib* src_dib)
{
  imDib* dib;

  assert(src_dib);

  dib = (imDib*)malloc(sizeof(imDib));

  memcpy(dib, src_dib, sizeof(imDib));

  AllocDib(dib);
  if (dib->dib == NULL)
  {
    free(dib);
    return NULL;
  }

  iInitHeadersReference(dib);

  memcpy(dib->dib, src_dib->dib, dib->size - dib->bits_size);

  iInitBits(dib, NULL);

  memcpy(dib->bits, src_dib->bits, dib->bits_size);

  dib->is_reference = 0;

  return dib;
}

imDib* imDibCreateReference(BYTE* bmi, BYTE* bits)
{
  imDib* dib;

  assert(bmi);

  dib = (imDib*)malloc(sizeof(imDib));

  dib->dib = bmi;
  
  iInitHeadersReference(dib);
  
  if (dib->bmih->biBitCount > 8)
  {
    dib->palette_count = 0;
    
    if (dib->bmih->biCompression == BI_BITFIELDS)
      dib->palette_count = 3;
  }
  else
  {
    if (dib->bmih->biClrUsed != 0)
      dib->palette_count = dib->bmih->biClrUsed;
    else
      dib->palette_count = 1 << dib->bmih->biBitCount;
  }
  
  iInitBits(dib, bits);
  
  dib->is_reference = 1;

  iInitSizes(dib, dib->bmih->biWidth, abs(dib->bmih->biHeight), dib->bmih->biBitCount);

  return dib;
}

void imDibDestroy(imDib* dib)
{
  assert(dib);
  if (!dib->is_reference) 
  {
    GlobalUnlock(dib->handle);
    GlobalFree(dib->handle);
  }
  free(dib);
}

/*****************
  Line Acess
*****************/

imDibLineGetPixel imDibLineGetPixelFunc(int bpp)
{
  switch(bpp)
  {
  case 1:
    return &iLineGetPixel1;
  case 4:
    return &iLineGetPixel4;
  case 8:
    return &iLineGetPixel8;
  case 16:
    return &iLineGetPixel16;
  case 24:
    return &iLineGetPixel24;
  case 32:
    return &iLineGetPixel32;
  default:
    if (bpp > 32) return NULL;
    iGetPixelAnyBpp = bpp;
    iGetPixelAnyMask = iMakeBitMask(bpp);
    return &iLineGetPixelAny;
  }
}

imDibLineSetPixel imDibLineSetPixelFunc(int bpp)
{
  switch(bpp)
  {
  case 1:
    return &iLineSetPixel1;
  case 4:
    return &iLineSetPixel4;
  case 8:
    return &iLineSetPixel8;
  case 16:
    return &iLineSetPixel16;
  case 24:
    return &iLineSetPixel24;
  case 32:
    return &iLineSetPixel32;
  default:
    if (bpp > 32) return NULL;
    iSetPixelAnyBpp = bpp;
    iSetPixelAnyMask = iMakeBitMask(bpp);
    return &iLineSetPixelAny;
  }
}

/*****************
  DIB <-> Bitmap
*****************/

imDib* imDibFromHBitmap(const HBITMAP bitmap, const HPALETTE hPalette)
{
  imDib* dib;

  assert(bitmap);

  {
    BITMAP bmp; 
 
    if (!GetObject(bitmap, sizeof(BITMAP), (LPSTR)&bmp)) 
      return NULL;
 
    dib = imDibCreate(bmp.bmWidth, bmp.bmHeight, bmp.bmPlanes * bmp.bmBitsPixel);
  }

  if (!dib)
    return NULL;
  
  {
    HDC ScreenDC = GetDC(NULL);
    HPALETTE hOldPalette = NULL;
    if (hPalette) hOldPalette = SelectPalette(ScreenDC, hPalette, FALSE);
    RealizePalette(ScreenDC);

    GetDIBits(ScreenDC, bitmap, 0, dib->bmih->biHeight, dib->bits, dib->bmi, DIB_RGB_COLORS);	

    if (hOldPalette) SelectPalette(ScreenDC, hOldPalette, FALSE);
    ReleaseDC(NULL, ScreenDC);
  }
  
  return dib;
}

HBITMAP imDibToHBitmap(const imDib* dib)
{
  HBITMAP bitmap;

  assert(dib);

  {
    HDC ScreenDC = GetDC(NULL);
    bitmap = CreateDIBitmap(ScreenDC, dib->bmih, CBM_INIT, dib->bits, dib->bmi, DIB_RGB_COLORS);
    ReleaseDC(NULL, ScreenDC);
  }

/* 
  Another Way
  bitmap = CreateCompatibleBitmap(ScreenDC, dib->bmih->biWidth, dib->bmih->biHeight);
  SetDIBits(ScreenDC, bitmap, 0, dib->bmih->biHeight, dib->bits, dib->bmi, DIB_RGB_COLORS);	
*/

  return bitmap;
}

/*******************
  DIB <-> Clipboard
*******************/

int imDibIsClipboardAvailable(void)
{
  if (IsClipboardFormatAvailable(CF_DIB) ||
      IsClipboardFormatAvailable(CF_BITMAP))
    return 1;

  return 0;
}

imDib* imDibPasteClipboard(void)
{
  int clip_type = 0;
  if (IsClipboardFormatAvailable(CF_DIB)) 
    clip_type = CF_DIB;
  else if (IsClipboardFormatAvailable(CF_BITMAP)) 
    clip_type = CF_BITMAP;

  if (!clip_type)
    return NULL;

  OpenClipboard(NULL);
  HANDLE Handle = GetClipboardData(clip_type);
  if (Handle == NULL)
  {
    CloseClipboard();
    return NULL;
  }
  
  imDib *dib;
  if (clip_type == CF_DIB)
  {
    BYTE* bmi = (BYTE*)GlobalLock(Handle);
    if (!bmi || !iCheckHeader((BITMAPINFOHEADER*)bmi))
    {
      CloseClipboard();
      return NULL;
    }

    {
      imDib* clip_dib = imDibCreateReference(bmi, NULL);
      dib = imDibCreateCopy(clip_dib);
      imDibDestroy(clip_dib);
      GlobalUnlock(Handle);
    }
  }
  else
  {
    HPALETTE hpal = (HPALETTE)GetClipboardData(CF_PALETTE);

    /* If there is a CF_PALETTE object in the clipboard, this is the palette to assume */
    /* the bitmap is realized against.                                                 */
    if (!hpal)
      hpal = (HPALETTE)GetStockObject(DEFAULT_PALETTE);

    dib = imDibFromHBitmap((HBITMAP)Handle, hpal);
  }
  
  CloseClipboard();
  
  return dib;
}  

void imDibCopyClipboard(imDib* dib)
{
  assert(dib);

  if (!OpenClipboard(NULL))
    return;
  EmptyClipboard();
  GlobalUnlock(dib->handle);
  SetClipboardData(CF_DIB, dib->handle);
  CloseClipboard();

  dib->dib = NULL;
  dib->is_reference = 1;
  imDibDestroy(dib);
}

/*******************
  DIB -> Palette
*******************/

HPALETTE imDibLogicalPalette(const imDib* dib)
{
  LOGPALETTE* pLogPal;      
  PALETTEENTRY* pPalEntry;
  HPALETTE hPal;
  RGBQUAD* bmic;
  int c;
  
  assert(dib);
  assert(dib->bmih->biBitCount <= 8);
  
  pLogPal = (LOGPALETTE*)malloc(sizeof(LOGPALETTE) + dib->palette_count * sizeof(PALETTEENTRY));
  pLogPal->palVersion    = 0x300; 
  pLogPal->palNumEntries = (WORD)dib->palette_count;
  
  bmic = dib->bmic;
  pPalEntry = pLogPal->palPalEntry;
  
  for (c = 0; c < dib->palette_count; c++) 
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

/*******************
  DIB <-> RGB Image
*******************/

void imDibEncodeFromRGBA(imDib* dib, const unsigned char* red, const unsigned char* green, const unsigned char* blue, const unsigned char* alpha)
{
  int x, y;
  BYTE* bits;
  
  if (dib->bmih->biHeight < 0)
    bits = dib->bits + (dib->bits_size - dib->line_size); /* start of last line */
  else
    bits = dib->bits;
  
  assert(dib->bmih->biBitCount > 16);
  
  for (y = 0; y < abs(dib->bmih->biHeight); y++)
  {
    for (x = 0; x < dib->bmih->biWidth; x++)
    {
      *bits++ = *blue++;
      *bits++ = *green++;
      *bits++ = *red++;

      if (dib->bmih->biBitCount == 32)
      {
        if (alpha)
          *bits++ = *alpha++;
        else
          *bits++ = 0xFF; /* opaque */
      }
    }
    
    bits += dib->pad_size;

    if (dib->bmih->biHeight < 0)
      bits -= 2*dib->line_size;
  }
}

void imDibDecodeToRGBA(const imDib* dib, unsigned char* red, unsigned char* green, unsigned char* blue, unsigned char* alpha)
{
  int x, y, offset;
  unsigned short color;
  BYTE* bits;
  unsigned int rmask = 0, gmask = 0, bmask = 0, 
                roff = 0, goff = 0, boff = 0; /* pixel bit mask control when reading 16 and 32 bpp images */
  
  assert(dib);
  assert(dib->bmih->biBitCount > 8);
  assert(red && green && blue);

  if (dib->bmih->biHeight < 0)
    bits = dib->bits + (dib->bits_size - dib->line_size); /* start of last line */
  else
    bits = dib->bits;
  
  if (dib->bmih->biBitCount == 16)
    offset = dib->line_size;  /* do not increment for each pixel, jump line */
  else
    offset = dib->pad_size;   /* increment for each pixel, jump pad */
  
  if (dib->bmih->biCompression == BI_BITFIELDS)
  {
    unsigned int Mask;
    unsigned int* palette = (unsigned int*)dib->bmic;
    
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
  
  for (y = 0; y < abs(dib->bmih->biHeight); y++)
  {
    for (x = 0; x < dib->bmih->biWidth; x++)
    {
      if (dib->bmih->biBitCount == 16)
      {
        color = ((unsigned short*)bits)[x];
        *red++ = (unsigned char)((((rmask & color) >> roff) * 255) / (rmask >> roff));
        *green++ = (unsigned char)((((gmask & color) >> goff) * 255) / (gmask >> goff));
        *blue++ = (unsigned char)((((bmask & color) >> boff) * 255) / (bmask >> boff));
      }
      else
      {
        *blue++ = *bits++;
        *green++ = *bits++;
        *red++ = *bits++;
        
        if (dib->bmih->biBitCount == 32)
        {
          if (alpha)
            *alpha++ = *bits++;
          else
            bits++;
        }
      }
    }
    
    bits += offset;

    if (dib->bmih->biHeight < 0)
      bits -= 2*dib->line_size;
  }
}

/*******************
  DIB <-> Map Image
*******************/

void imDibEncodeFromMap(imDib* dib, const unsigned char* map, const long* palette, int palette_count)
{
  assert(dib);
  assert(map && palette);
  assert(dib->bmih->biBitCount <= 8);
  assert(dib->bmih->biCompression != BI_RLE8);

  {
    int x, y;
    BYTE* bits;
    
    if (dib->bmih->biHeight < 0)
      bits = dib->bits + (dib->bits_size - dib->line_size); /* start of last line */
    else
      bits = dib->bits;

    for (y = 0; y < abs(dib->bmih->biHeight); y++)
    {
      for (x = 0; x < dib->bmih->biWidth; x++)
        bits[x] = *map++;
    
      if (dib->bmih->biHeight < 0)
        bits -= dib->line_size;
      else
        bits += dib->line_size;
    }
  }

  {
    int c;
    RGBQUAD* bmic = dib->bmic;

    for (c = 0; c < palette_count; c++)
      *bmic++ = iLong2Quad(palette[c]);
  }

  dib->bmih->biClrUsed = palette_count;
  dib->bmih->biClrImportant = 0;
  dib->palette_count = palette_count;
}

void imDibDecodeToMap(const imDib* dib, unsigned char* map, long* palette)
{
  assert(dib);
  assert(dib->bmih->biBitCount <= 8);
  assert(map && palette);

  {
    int x, y;
    BYTE* bits;
    
    if (dib->bmih->biHeight < 0)
      bits = dib->bits + (dib->bits_size - dib->line_size); /* start of last line */
    else
      bits = dib->bits;
  
    for (y = 0; y < abs(dib->bmih->biHeight); y++)
    {
      for (x = 0; x < dib->bmih->biWidth; x++)
      {
        switch (dib->bmih->biBitCount)
        {
        case 1:
          *map++ = (unsigned char)((bits[x / 8] >> (7 - x % 8)) & 0x01);
          break;
        case 4:
          *map++ = (unsigned char)((bits[x / 2] >> ((1 - x % 2) * 4)) & 0x0F);
          break;
        case 8:
          *map++ = bits[x];
          break;
        }
      }
    
      if (dib->bmih->biHeight < 0)
        bits -= dib->line_size;
      else
        bits += dib->line_size;
    }
  }
  
  {
    int c;
    RGBQUAD* bmic = dib->bmic;

    for (c = 0; c < dib->palette_count; c++)
    {
      palette[c] = iQuad2Long(bmic);
      *bmic++;
    }
  }
}

/*******************
  DIB <-> File
*******************/

int imDibSaveFile(const imDib* dib, const char* filename)
{ 
  DWORD dwTmp; 
  HANDLE hFile;                 /* file handle */ 
  BITMAPFILEHEADER file_header; /* bitmap file-header */ 

  assert(dib);
  assert(filename);

  hFile = CreateFile(filename, GENERIC_WRITE, (DWORD) 0, 
                 (LPSECURITY_ATTRIBUTES)NULL, 
                 CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, (HANDLE) NULL); 

  if (hFile == INVALID_HANDLE_VALUE) 
    return 0;

  /* 0x42 = "B" 0x4d = "M" */ 
  file_header.bfType = 0x4d42;        

  /* Compute the size of the entire file. */ 
  file_header.bfSize = (DWORD) (sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dib->palette_count*sizeof(RGBQUAD) + dib->bits_size); 

  file_header.bfReserved1 = 0; 
  file_header.bfReserved2 = 0; 

  /* Compute the offset to the bits array. */ 
  file_header.bfOffBits = (DWORD) sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dib->palette_count*sizeof(RGBQUAD); 

  /* Copy the BITMAPFILEHEADER into the .BMP file. */ 
  if (!WriteFile(hFile, (LPVOID)&file_header, sizeof(BITMAPFILEHEADER), (LPDWORD)&dwTmp, (LPOVERLAPPED)NULL)) 
    goto save_error;

  /* Copy the BITMAPINFOHEADER into the file. */ 
  if (!WriteFile(hFile, (LPVOID)dib->bmih, sizeof(BITMAPINFOHEADER), (LPDWORD)&dwTmp, (LPOVERLAPPED)NULL)) 
    goto save_error;

  /* Copy the RGBQUAD array into the file. */ 
  if (dib->palette_count > 0)
  {
    if (!WriteFile(hFile, (LPVOID)dib->bmic, dib->palette_count*sizeof(RGBQUAD), (LPDWORD)&dwTmp, (LPOVERLAPPED)NULL)) 
      goto save_error;
  }

  /* Copy the bits array into the .BMP file. */ 
  if (!WriteFile(hFile, dib->bits, dib->bits_size, (LPDWORD)&dwTmp, (LPOVERLAPPED)NULL)) 
    goto save_error;

  /* Close the .BMP file. */ 
  CloseHandle(hFile);

  return 1;

save_error:
  CloseHandle(hFile);
  return 0;
} 
 
imDib* imDibLoadFile(const char* filename)
{ 
  HANDLE hFile;                 /* file handle */ 
  DWORD dwTmp; 
  imDib* dib = NULL;
  BITMAPFILEHEADER file_header; /* bitmap file-header */ 
  BITMAPINFOHEADER bmih;

  assert(filename);

  hFile = CreateFile(filename, GENERIC_READ, (DWORD) 0, 
                 (LPSECURITY_ATTRIBUTES)NULL, 
                 OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, (HANDLE) NULL); 

  if (hFile == INVALID_HANDLE_VALUE) 
    return NULL;

  /* Read the BITMAPFILEHEADER from the .BMP file. */ 
  if (!ReadFile(hFile, (LPVOID)&file_header, sizeof(BITMAPFILEHEADER), (LPDWORD)&dwTmp, (LPOVERLAPPED)NULL)) 
    goto load_error;

  if (file_header.bfType != 0x4d42)
    goto load_error;

  /* Read the BITMAPINFOHEADER from the file. */ 
  if (!ReadFile(hFile, (LPVOID)&bmih, sizeof(BITMAPINFOHEADER), (LPDWORD)&dwTmp, (LPOVERLAPPED)NULL)) 
    goto load_error;

  if(!iCheckHeader(&bmih))
    goto load_error;

  dib = imDibCreate(bmih.biWidth, abs(bmih.biHeight), bmih.biCompression==BI_BITFIELDS? -bmih.biBitCount: bmih.biBitCount);

  memcpy(dib->bmih, &bmih, bmih.biSize);

  if (bmih.biSize != sizeof(BITMAPINFOHEADER))
  {
    /* skip newer BIH definitions */
    SetFilePointer(hFile, bmih.biSize - sizeof(BITMAPINFOHEADER), NULL, FILE_CURRENT);
    dib->bmih->biSize = sizeof(BITMAPINFOHEADER);
  }

  /* Read the RGBQUAD array from the file. */ 
  if (dib->palette_count > 0)
  {
    if (!ReadFile(hFile, (LPVOID)dib->bmic, dib->palette_count*sizeof(RGBQUAD), (LPDWORD)&dwTmp, (LPOVERLAPPED)NULL)) 
      goto load_error;
  }

  /* Read the Bits array from the .BMP file. */ 
  SetFilePointer(hFile, file_header.bfOffBits, NULL, FILE_BEGIN);

  {
    int bits_size = dib->bits_size;

    if (bmih.biBitCount < 16 && bmih.biCompression != BI_RGB)
      bits_size = GetFileSize(hFile, NULL) - file_header.bfOffBits;

    if (bits_size > dib->bits_size)
      goto load_error;

    if (!ReadFile(hFile, dib->bits, bits_size, (LPDWORD)&dwTmp, (LPOVERLAPPED)NULL)) 
      goto load_error;
  }

  /* Close the .BMP file. */ 
  CloseHandle(hFile);

  return dib;

load_error:
  if (dib) imDibDestroy(dib);
  CloseHandle(hFile);
  return NULL;
} 

/*******************
  Screen -> DIB
*******************/

imDib* imDibCaptureScreen(int x, int y, int width, int height)
{
  HBITMAP bitmap;
  HDC ScreenDC = GetDC(NULL);
  HDC hdcCompatible = CreateCompatibleDC(ScreenDC); 

  if (width == 0) width = GetDeviceCaps(ScreenDC, HORZRES);
  if (height == 0) height = GetDeviceCaps(ScreenDC, VERTRES);

  bitmap = CreateCompatibleBitmap(ScreenDC, width, height);

  if (!bitmap) 
  {
    ReleaseDC(NULL, ScreenDC);
    return NULL;
  }

  /* Select the bitmaps into the compatible DC.  */
  SelectObject(hdcCompatible, bitmap);

  /* Copy color data for the entire display into a */
  /* bitmap that is selected into a compatible DC. */
  BitBlt(hdcCompatible, 0, 0, width, height, ScreenDC, x, y, SRCCOPY);

  ReleaseDC(NULL, ScreenDC);
  DeleteDC(hdcCompatible);

  {
    imDib* dib = imDibFromHBitmap(bitmap, NULL);
    DeleteObject(bitmap);
    return dib;
  }
}
