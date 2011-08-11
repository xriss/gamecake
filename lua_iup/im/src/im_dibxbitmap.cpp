/** \file
 * \brief Conversion between imDib and imImage
 *
 * See Copyright Notice in im_lib.h
 * $Id: im_dibxbitmap.cpp,v 1.1 2008/10/17 06:10:16 scuri Exp $
 */


#include <windows.h>
#include <assert.h>

#include "im.h"
#include "im_image.h"
#include "im_dib.h"
#include "im_util.h"


void imDibEncodeFromBitmap(imDib* dib, const unsigned char* data)
{
  int x, y;
  BYTE* bits;
  
  assert(dib);
  assert(dib->bmih->biBitCount > 16);
  assert(data);
  
  if (dib->bmih->biHeight < 0)
    bits = dib->bits + (dib->bits_size - dib->line_size); /* start of last line */
  else
    bits = dib->bits;
  
  for (y = 0; y < abs(dib->bmih->biHeight); y++)
  {
    for (x = 0; x < dib->bmih->biWidth; x++)
    {
      *bits++ = *(data+2);   // R
      *bits++ = *(data+1);   // G
      *bits++ = *(data+0);   // B

      data += 3;

      if (dib->bmih->biBitCount == 32)
        *bits++ = *data++;
    }
    
    bits += dib->pad_size;

    if (dib->bmih->biHeight < 0)
      bits -= 2*dib->line_size;
  }
}

void imDibDecodeToBitmap(const imDib* dib, unsigned char* data)
{
  int x, y, offset;
  unsigned short color;
  BYTE* bits;
  unsigned int rmask = 0, gmask = 0, bmask = 0, 
                roff = 0,  goff = 0,  boff = 0; /* pixel bit mask control when reading 16 and 32 bpp images */
  
  assert(dib);
  assert(dib->bmih->biBitCount > 8);
  assert(data);

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
        *data++ = (unsigned char)((((rmask & color) >> roff) * 255) / (rmask >> roff));
        *data++ = (unsigned char)((((gmask & color) >> goff) * 255) / (gmask >> goff));
        *data++ = (unsigned char)((((bmask & color) >> boff) * 255) / (bmask >> boff));
      }
      else
      {
        *(data+2) = *bits++; // B
        *(data+1) = *bits++; // G
        *(data+0) = *bits++; // R

        data += 3;
        
        if (dib->bmih->biBitCount == 32)
          *data++ = *bits++;
      }
    }
    
    bits += offset;

    if (dib->bmih->biHeight < 0)
      bits -= 2*dib->line_size;
  }
}

imImage* imDibToImage(const imDib* dib)
{
  assert(dib);

  int color_space = IM_RGB;
  if (dib->bmih->biBitCount <= 8)
    color_space = IM_MAP;

  imImage* image = imImageCreate(dib->bmih->biWidth, abs(dib->bmih->biHeight), color_space, IM_BYTE);
  if (!image) 
    return NULL;

  if (image->color_space == IM_MAP)
  {
    image->palette_count = dib->palette_count;
    imDibDecodeToMap(dib, (imbyte*)image->data[0], image->palette);
  }
  else
  {
    imDibDecodeToRGBA(dib, (imbyte*)image->data[0], (imbyte*)image->data[1], (imbyte*)image->data[2], NULL);
  }

  return image;
}

imDib* imDibFromImage(const imImage* image)
{
  assert(image);
  assert(imImageIsBitmap(image));

  if (!imImageIsBitmap(image))
    return NULL;

  int bpp;
  if (image->color_space != IM_RGB)
    bpp = 8;
  else
    bpp = 24;

  imDib* dib = imDibCreate(image->width, image->height, bpp);     
  if (!dib) return NULL;

  if (image->color_space != IM_RGB)
    imDibEncodeFromMap(dib, (const imbyte*)image->data[0], image->palette, image->palette_count);
  else
    imDibEncodeFromRGBA(dib, (const imbyte*)image->data[0], (const imbyte*)image->data[1], (const imbyte*)image->data[2], NULL);

  return dib;
}
