/** \file
 * \brief BMP - Windows Device Independent Bitmap
 *
 * See Copyright Notice in im_lib.h
 * $Id: im_format_bmp.cpp,v 1.4 2009/10/01 14:15:47 scuri Exp $
 */

#include "im_format.h"
#include "im_format_all.h"
#include "im_util.h"
#include "im_counter.h"

#include "im_binfile.h"

#include <stdlib.h>
#include <string.h>
#include <memory.h>


#define BMP_ID              0x4d42  /* BMP "magic" number           */

#define BMP_COMPRESS_RGB        0L      /* No compression               */
#define BMP_COMPRESS_RLE8       1L      /* 8 bits per pixel compression */
#define BMP_COMPRESS_RLE4       2L      /* 4 bits per pixel compression */
#define BMP_BITFIELDS           3L      /* no compression, palette is mask for 16 and 32 bits images */

/* State-machine definitions */
#define BMP_READING 0         /* General READING mode */
#define BMP_ENCODING 1        /* Encoding same-color pixel runs */
#define BMP_ABSMODE 2         /* Absolute-mode encoding */
#define BMP_SINGLE 3          /* Encoding short absolute-mode runs */
#define BMP_ENDOFLINE 4       /* End of scan line detected */

#define BMP_LSN(value)  (unsigned char)((value) & 0x0f)      /* Least-significant nibble */
#define BMP_MSN(value)  (unsigned char)(((value) & 0xf0) >> 4)  /* Most-significant nibble  */


/*  File Header Structure.
 *  2   Type;       File Type Identifier        
 *  4   FileSize;   Size of File                
 *  2   Reserved1;  Reserved (should be 0)      
 *  2   Reserved2;  Reserved (should be 0)      
 *  4   Offset;     Offset to bitmap data       
 *  14    TOTAL */

/*  Information Header Structure. 
 *  4  Size;            Size of Remaining Header         
 *  4  Width;           Width of Bitmap in Pixels        
 *  4  Height;          Height of Bitmap in Pixels       
 *  2  Planes;          Number of Planes                 
 *  2  BitCount;        Bits Per Pixel                   
 *  4  Compression;     Compression Scheme      
 *  4  SizeImage;       Size of bitmap         
 *  4  XPelsPerMeter;   Horz. Resolution in Pixels/Meter 
 *  4  YPelsPerMeter;   Vert. Resolution in Pixels/Meter 
 *  4  ClrUsed;         Number of Colors in Color Table  
 *  4  ClrImportant;    Number of Important Colors       
 *  40   TOTAL V3 
 *  4  RedMask; 
 *  4  GreenMask; 
 *  4  BlueMask; 
 *  4  AlphaMask; 
 *  4  CSType; 
 *  12 ciexyzRed(x, y, z);     [3*FXPT2DOT30]
 *  12 ciexyzGreen(x, y, z);         "
 *  12 ciexyzBlue(x, y, z);          "
 *  4  GammaRed; 
 *  4  GammaGreen; 
 *  4  GammaBlue; 
 *  108  TOTAL V4   (not supported here)
 *  4  Intent; 
 *  4  ProfileData; 
 *  4  ProfileSize; 
 *  4  Reserved; 
 *  120  TOTAL V5 (not supported here)
 */

/*  RGB Color Quadruple Structure. */
/*  1   rgbBlue;      Blue Intensity Value   */
/*  1   rgbGreen;     Green Intensity Value  */
/*  1   rgbRed;       Red Intensity Value    */
/*  1   rgbReserved;  Reserved (should be 0) */
/*  4  */

static int iBMPDecodeScanLine(imBinFile* handle, unsigned char* DecodedBuffer, int Width)
{
  unsigned char runCount;   /* Number of pixels in the run  */
  unsigned char runValue;   /* Value of pixels in the run   */
  int Index = 0;            /* The index of DecodedBuffer   */
  int cont = 1, remain;

  while (cont)
  {
    imBinFileRead(handle, &runCount, 1, 1);  /* Number of pixels in the run */
    imBinFileRead(handle, &runValue, 1, 1);  /* Value of pixels in the run  */

    if (imBinFileError(handle))
      return IM_ERR_ACCESS;

    if (runCount)
    {
      while (runCount-- && Index < Width)
        DecodedBuffer[Index++] = runValue;
    }
    else  /* Abssolute Mode or Escape Code */
    {
      switch(runValue)
      {
      case 0:             /* End of Scan Line Escape Code */
      case 1:             /* End of Bitmap Escape Code */
        cont = 0;
        break;
      case 2:             /* Delta Escape Code (ignored) */
        imBinFileRead(handle, &runCount, 1, 1);
        imBinFileRead(handle, &runCount, 1, 1);  
        break;
      default:            /* Abssolute Mode */
        remain = runValue % 2;
        runValue = (unsigned char)(Index + runValue < (Width + 1)? runValue: (Width - 1) - Index);
        imBinFileRead(handle, DecodedBuffer + Index, runValue, 1);
        if (remain) 
          imBinFileSeekOffset(handle, 1);
        Index += runValue;
      }
    }

    if (imBinFileError(handle) || Index > Width)
      return IM_ERR_ACCESS;
  }

  return IM_ERR_NONE;
}

static int iBMPEncodeScanLine(unsigned char* EncodedBuffer, unsigned char* sl, int np)
{
  int slx = 0;             /* Scan line index */
  int state = BMP_READING; /* State machine control variable */
  int count = 0;           /* Used by various states */
  unsigned char pixel;     /* Holds single pixels from sl */
  int done = 0;            /* Ends while loop when true */
  int oldcount, oldslx;    /* Copies of count and slx */
  int BufSize = 0;

  while (!done) 
  {
    switch (state) 
    {
    case BMP_READING:
      /* Input: */
      /* np == number of pixels in scan line */
      /* sl == scan line */
      /* sl[slx] == next pixel to process */

      if (slx >= np)                      /* No pixels left */
        state = BMP_ENDOFLINE;
      else if (slx == np - 1)             /* One pixel left */
      {
        count = 1;
        state = BMP_SINGLE;
      } 
      else if (sl[slx] == sl[slx + 1])    /* Next 2 pixels equal */
        state = BMP_ENCODING;
      else                                /* Next 2 pixels differ */
        state = BMP_ABSMODE;

      break;
    case BMP_ENCODING:
      /* Input: */
      /* slx <= np - 2 (at least 2 pixels in run) */
      /* sl[slx] == first pixel of run */
      /* sl[slx] == sl[slx + 1] */

      count = 2;
      pixel = sl[slx];
      slx += 2;

      while ((slx < np) && (pixel == sl[slx]) && (count < 255)) 
      {
        count++;
        slx++;
      }

      *EncodedBuffer++ = (unsigned char)count; 
      BufSize++;
      *EncodedBuffer++ = pixel; 
      BufSize++;
      state = BMP_READING;
      
      break;
    case BMP_ABSMODE:
      /* Input: */
      /* slx <= np - 2 (at least 2 pixels in run) */
      /* sl[slx] == first pixel of run */
      /* sl[slx] != sl[slx + 1] */

      oldslx = slx;
      count = 2;
      slx += 2;

      /* Compute number of bytes in run */
      while ((slx < np) && (sl[slx] != sl[slx - 1]) && (count < 255)) 
      {
        count++;
        slx++;
      }

      /* If same-color run found, back up one byte */
      if ((slx < np) && (sl[slx] == sl[slx - 1]))
        if (count > 1)
          count--;

      slx = oldslx;  /* Restore scan-line index */

      /* Output short absolute runs of less than 3 pixels */
      if (count < 3 )
        state = BMP_SINGLE;
      else 
      {
        /* Output absolute-mode run */
        *EncodedBuffer++ = 0; 
        BufSize++;
        *EncodedBuffer++ = (unsigned char)count; 
        BufSize++;
        oldcount = count;

        while (count > 0) 
        {
          *EncodedBuffer++ = sl[slx]; 
          BufSize++;
          slx++;
          count--;
        }

        if (oldcount % 2) 
        {
          *EncodedBuffer++ = 0; 
          BufSize++;
        }

       state = BMP_READING;
      }
      break;

    case BMP_SINGLE:
      /* Input: */
      /* count == number of pixels to output */
      /* slx < np */
      /* sl[slx] == first pixel of run */
      /* sl[slx] != sl[slx + 1] */

      while (count > 0) 
      {
        *EncodedBuffer++ = (unsigned char)1; 
        BufSize++;
        *EncodedBuffer++ = sl[slx]; 
        BufSize++;
        slx++;
        count--;
      }

      state = BMP_READING;

      break;
    case BMP_ENDOFLINE:
      *EncodedBuffer++ = (unsigned char)0; 
      BufSize++;
      *EncodedBuffer++ = (unsigned char)0; 
      BufSize++;
      done = 1;

      break;
    default:
      break;
    }
  }

  return BufSize;
}

static const char* iBMPCompTable[2] = 
{
  "NONE",
  "RLE"
};

class imFileFormatBMP: public imFileFormatBase
{
  imBinFile* handle;          /* the binary file handle */
  unsigned short bpp;         /* number of bits per pixel */
  unsigned int offset,        /* image data offset, used only when reading */
               comp_type;     /* bmp compression information */
  int is_os2,                 /* indicates an os2 1.x BMP */
      line_raw_size;              // raw line size
  unsigned int rmask, gmask, bmask, 
                roff, goff, boff; /* pixel bit mask control when reading 16 and 32 bpp images */

  int ReadPalette();
  int WritePalette();
  void FixRGBOrder();

public:
  imFileFormatBMP(const imFormat* _iformat): imFileFormatBase(_iformat) {}
  ~imFileFormatBMP() {}

  int Open(const char* file_name);
  int New(const char* file_name);
  void Close();
  void* Handle(int index);
  int ReadImageInfo(int index);
  int ReadImageData(void* data);
  int WriteImageInfo();
  int WriteImageData(void* data);
};

class imFormatBMP: public imFormat
{
public:
  imFormatBMP()
    :imFormat("BMP", 
              "Windows Device Independent Bitmap", 
              "*.bmp;*.dib;", 
              iBMPCompTable, 
              2, 
              0)
    {}
  ~imFormatBMP() {}

  imFileFormatBase* Create(void) const { return new imFileFormatBMP(this); }
  int CanWrite(const char* compression, int color_mode, int data_type) const;
};


void imFormatRegisterBMP(void)
{
  imFormatRegister(new imFormatBMP());
}

int imFileFormatBMP::Open(const char* file_name)
{
  unsigned short id;
  unsigned int dword;

  /* opens the binary file for reading with intel byte order */
  handle = imBinFileOpen(file_name);
  if (!handle)
    return IM_ERR_OPEN;

  imBinFileByteOrder(handle, IM_LITTLEENDIAN); 

  /* reads the BMP format identifier */
  imBinFileRead(handle, &id, 1, 2);
  if (imBinFileError(handle))
  {
    imBinFileClose(handle);
    return IM_ERR_ACCESS;
  }

  if (id != BMP_ID)
  {
    imBinFileClose(handle);
    return IM_ERR_FORMAT;
  }

  /* jump 8 bytes (file size,reserved) */
  imBinFileSeekOffset(handle, 8);

  /* reads the image offset */
  imBinFileRead(handle, &this->offset, 1, 4);

  /* reads the header size */
  imBinFileRead(handle, &dword, 1, 4);

  if (dword == 40)
    this->is_os2 = 0;
  else if (dword == 12)
    this->is_os2 = 1;
  else
  {
    imBinFileClose(handle);
    return IM_ERR_FORMAT;
  }

  this->image_count = 1;

  /* reads the compression information */
  if (this->is_os2)
  {
    this->comp_type = BMP_COMPRESS_RGB;
    strcpy(this->compression, "NONE");
  }
  else
  {
    imBinFileSeekOffset(handle, 12);

    imBinFileRead(handle, &this->comp_type, 1, 4);

    switch (this->comp_type)
    {
    case BMP_COMPRESS_RGB:
      strcpy(this->compression, "NONE");
      break;
    case BMP_COMPRESS_RLE8:
      strcpy(this->compression, "RLE");
      break;
    case BMP_COMPRESS_RLE4:
    default:
      imBinFileClose(handle);
      return IM_ERR_COMPRESS;
    }

    imBinFileSeekOffset(handle, -16);
  }

  return IM_ERR_NONE;
}

int imFileFormatBMP::New(const char* file_name)
{
  /* opens the binary file for writing with intel byte order */
  handle = imBinFileNew(file_name);
  if (!handle)
    return IM_ERR_OPEN;

  imBinFileByteOrder(handle, IM_LITTLEENDIAN); 

  this->image_count = 1;

  return IM_ERR_NONE;
}

void imFileFormatBMP::Close()
{
  imBinFileClose(handle);
}

void* imFileFormatBMP::Handle(int index)
{
  if (index == 0)
    return (void*)this->handle;
  else
    return NULL;
}

int imFileFormatBMP::ReadImageInfo(int index)
{
  (void)index;
  unsigned int dword;

  this->file_data_type = IM_BYTE;

  if (this->is_os2)
  {
    short word;

    /* reads the image width */
    imBinFileRead(handle, &word, 1, 2);
    this->width = (int)word;

    /* reads the image height */
    imBinFileRead(handle, &word, 1, 2);
    this->height = (int)((word < 0)? -word: word);

    dword = word; // it will be used later
  }
  else
  {
    /* reads the image width */
    imBinFileRead(handle, &dword, 1, 4);
    this->width = (int)dword;

    /* reads the image height */
    imBinFileRead(handle, &dword, 1, 4);
    this->height = (int)dword;
    if (this->height < 0)
      this->height = -this->height;
  }

  /* jump 2 bytes (planes) */
  imBinFileSeekOffset(handle, 2);

  /* reads the number of bits per pixel */
  imBinFileRead(handle, &this->bpp, 1, 2);

  if (imBinFileError(handle))
    return IM_ERR_ACCESS;

  // sanity check
  if (this->bpp != 1 && this->bpp != 4 && this->bpp != 8 && 
      this->bpp != 16 && this->bpp != 24 && this->bpp != 32)
    return IM_ERR_DATA;

  // another sanity check
  if (this->comp_type == BMP_BITFIELDS && this->bpp != 16 && this->bpp != 32)
    return IM_ERR_DATA;

  if (this->bpp > 8)
  {
    this->file_color_mode = IM_RGB;
    this->file_color_mode |= IM_PACKED;
  }
  else
  {
    this->palette_count = 1 << bpp;
    this->file_color_mode = IM_MAP;
  }

  if (this->bpp < 8)
    this->convert_bpp = this->bpp;

  if (this->bpp == 32)
    this->file_color_mode |= IM_ALPHA;

  if (dword < 0)
    this->file_color_mode |= IM_TOPDOWN;

  this->line_raw_size = imFileLineSizeAligned(this->width, this->bpp, 4);
  this->line_buffer_extra = 4; // room enough for padding

  if (this->is_os2)
  {
    if (this->bpp < 24)
      return ReadPalette();

    return IM_ERR_NONE;
  }

  /* we already read the compression information */
  /* jump 8 bytes (compression, image size) */
  imBinFileSeekOffset(handle, 8);

  /* read the x resolution */
  imBinFileRead(handle, &dword, 1, 4);
  float xres = (float)dword / 100.0f;

  /* read the y resolution */
  imBinFileRead(handle, &dword, 1, 4);
  float yres = (float)dword / 100.0f;

  if (xres && yres)
  {
    imAttribTable* attrib_table = AttribTable();
    attrib_table->Set("XResolution", IM_FLOAT, 1, &xres);
    attrib_table->Set("YResolution", IM_FLOAT, 1, &yres);
    attrib_table->Set("ResolutionUnit", IM_BYTE, -1, "DPC");
  }

  if (this->bpp <= 8)
  {
    /* reads the number of colors used */
    imBinFileRead(handle, &dword, 1, 4);

    /* updates the palette_count based on the number of colors used */
    if (dword != 0 && dword < (unsigned int)this->palette_count)
      this->palette_count = dword;

    /* jump 4 bytes (important colors) */
    imBinFileSeekOffset(handle, 4);
  }
  else
  {
    /* jump 8 bytes (used colors, important colors) */
    imBinFileSeekOffset(handle, 8);
  }

  if (imBinFileError(handle))
    return IM_ERR_ACCESS;

  if (this->bpp <= 8)
    return ReadPalette();

  if (this->bpp == 16 || this->bpp == 32)
  {
    if (this->comp_type == BMP_BITFIELDS)
    {
      unsigned int Mask;
      unsigned int PalMask[3];

      imBinFileRead(handle, PalMask, 3, 4);
      if (imBinFileError(handle))
        return IM_ERR_ACCESS;

      this->roff = 0;
      this->rmask = Mask = PalMask[0];
      while (!(Mask & 0x01) && (Mask != 0))
        {Mask >>= 1; this->roff++;}

      this->goff = 0;
      this->gmask = Mask = PalMask[1];
      while (!(Mask & 0x01) && (Mask != 0))
        {Mask >>= 1; this->goff++;}

      this->boff = 0;
      this->bmask = Mask = PalMask[2];
      while (!(Mask & 0x01) && (Mask != 0))
        {Mask >>= 1; this->boff++;}
    }
    else
    {
      if (this->bpp == 16)
      {                   
        this->rmask = 0x7C00;
        this->roff = 10;

        this->gmask = 0x03E0;
        this->goff = 5;

        this->bmask = 0x001F;
        this->boff = 0;
      }
      else
      {
        this->rmask = 0x00FF0000;
        this->roff = 16;

        this->gmask = 0x0000FF00;
        this->goff = 8;

        this->bmask = 0x000000FF;
        this->boff = 0;
      }
    }
  }

  return IM_ERR_NONE;
}

int imFileFormatBMP::WriteImageInfo()
{
  // force bottom up orientation
  this->file_data_type = IM_BYTE;
  this->file_color_mode = imColorModeSpace(this->user_color_mode);

  if (imStrEqual(this->compression, "RLE"))
    this->comp_type = BMP_COMPRESS_RLE8;
  else
    this->comp_type = BMP_COMPRESS_RGB;

  if (this->file_color_mode == IM_BINARY)
  {
    this->bpp = 1;
    this->convert_bpp = 1;
  }
  else if (this->file_color_mode == IM_RGB)
  {
    this->file_color_mode |= IM_PACKED;
    this->bpp = 24;

    if (imColorModeHasAlpha(this->user_color_mode))
    {
      this->file_color_mode |= IM_ALPHA;
      this->bpp = 32;

      this->rmask = 0x00FF0000;
      this->roff = 16;

      this->gmask = 0x0000FF00;
      this->goff = 8;

      this->bmask = 0x000000FF;
      this->boff = 0;
    }
  }
  else
    this->bpp = 8;

  this->line_raw_size = imFileLineSizeAligned(this->width, this->bpp, 4);
  this->line_buffer_extra = 4; // room enough for padding

  if (this->comp_type == BMP_COMPRESS_RLE8)
  {
    // allocates more than enough since compression algoritm can be ineficient
    this->line_buffer_extra += 2*this->line_raw_size;
  }

  /* writes the BMP file header */
  int palette_size = (this->bpp > 8)? 0: palette_count*4;
  short word_value = BMP_ID;
  imBinFileWrite(handle, &word_value, 1, 2); /* identifier */
  unsigned int dword_value = 14 + 40 + palette_size + line_raw_size * this->height;
  imBinFileWrite(handle, &dword_value, 1, 4); /* file size for uncompressed images */
  word_value = 0;
  imBinFileWrite(handle, &word_value, 1, 2); /* reserved 1 */
  imBinFileWrite(handle, &word_value, 1, 2); /* reserved 2 */
  dword_value = 14 + 40 + palette_size;
  imBinFileWrite(handle, &dword_value, 1, 4); /* data offset */

  /* writes the BMP info header */

  dword_value = 40;
  imBinFileWrite(handle, &dword_value, 1, 4); /* header size */
  dword_value = this->width;
  imBinFileWrite(handle, &dword_value, 1, 4); /* width */
  dword_value = this->height;
  imBinFileWrite(handle, &dword_value, 1, 4); /* height */
  word_value = 1;
  imBinFileWrite(handle, &word_value, 1, 2);  /* planes */
  word_value = this->bpp;
  imBinFileWrite(handle, &word_value, 1, 2);  /* bpp */
  dword_value = this->comp_type;
  imBinFileWrite(handle, &dword_value, 1, 4); /* compression */
  dword_value = line_raw_size * this->height;
  imBinFileWrite(handle, &dword_value, 1, 4); /* image size */
  
  imAttribTable* attrib_table = AttribTable();
  unsigned int xppm = 0, yppm = 0;

  const void* attrib_data = attrib_table->Get("ResolutionUnit");
  if (attrib_data)
  {
    char* res_unit = (char*)attrib_data;

    float* xres = (float*)attrib_table->Get("XResolution");
    float* yres = (float*)attrib_table->Get("YResolution");

    if (xres && yres)
    {
      if (imStrEqual(res_unit, "DPI"))
      {
        xppm = (unsigned int)(*xres * 100. / 2.54);
        yppm = (unsigned int)(*yres * 100. / 2.54);
      }
      else
      {
        xppm = (unsigned int)(*xres * 100.);
        yppm = (unsigned int)(*yres * 100.);
      }
    }
  }

  imBinFileWrite(handle, &xppm, 1, 4); /* x dpm */
  imBinFileWrite(handle, &yppm, 1, 4); /* y dpm */

  dword_value = (this->bpp > 8)? 0: this->palette_count;
  imBinFileWrite(handle, &dword_value, 1, 4); /* colors used */
  dword_value = 0;
  imBinFileWrite(handle, &dword_value, 1, 4); /* colors important (all) */

  /* tests if everything was ok */
  if (imBinFileError(handle))
    return IM_ERR_ACCESS;

  if (this->bpp < 24)
    return WritePalette();

  return IM_ERR_NONE;
}

int imFileFormatBMP::ReadPalette()
{
  int nc;
  if (this->is_os2)
    nc = 3;
  else
    nc = 4;

  /* reads the color palette */
  unsigned char bmp_colors[256 * 4];
  imBinFileRead(handle, bmp_colors, this->palette_count * nc, 1);

  if (imBinFileError(handle))
    return IM_ERR_ACCESS;

  /* convert the color map to the IM format */
  for (int c = 0; c < this->palette_count; c++)
  {
    int i = c * nc;
    this->palette[c] = imColorEncode(bmp_colors[i + 2], 
                                     bmp_colors[i + 1], 
                                     bmp_colors[i]);
  }

  return IM_ERR_NONE;
}

int imFileFormatBMP::WritePalette()
{
  unsigned char bmp_colors[256 * 4];

  /* convert the color map to the IM format */
  for (int c = 0; c < this->palette_count; c++)
  {
    int i = c * 4;                       
    imColorDecode(&bmp_colors[i + 2], &bmp_colors[i + 1], &bmp_colors[i], this->palette[c]);
    bmp_colors[i + 3] = 0;
  }

  /* writes the color palette */
  imBinFileWrite(handle, bmp_colors, this->palette_count * 4, 1);

  if (imBinFileError(handle))
    return IM_ERR_ACCESS;

  return IM_ERR_NONE;
}

void imFileFormatBMP::FixRGBOrder()
{
  int x;

  switch (this->bpp)
  {
  case 16:
    {
      /* inverts the WORD values if not intel */
      if (imBinCPUByteOrder() == IM_BIGENDIAN)
        imBinSwapBytes2(this->line_buffer, this->width);

      imushort* word_data = (imushort*)this->line_buffer;
      imbyte* byte_data = (imbyte*)this->line_buffer;

      // from end to start
      for (x = this->width-1; x >= 0; x--)
      {
        imushort word_value = word_data[x];
        int c = x*3;
        byte_data[c]   = (imbyte)((((this->rmask & word_value) >> this->roff) * 255) / (this->rmask >> this->roff));
        byte_data[c+1] = (imbyte)((((this->gmask & word_value) >> this->goff) * 255) / (this->gmask >> this->goff));
        byte_data[c+2] = (imbyte)((((this->bmask & word_value) >> this->boff) * 255) / (this->bmask >> this->boff));
      }
    }
    break;
  case 32:
    {
      /* inverts the DWORD values if not intel */
      if (imBinCPUByteOrder() == IM_BIGENDIAN)
        imBinSwapBytes4(this->line_buffer, this->width);

      unsigned int* dword_data = (unsigned int*)this->line_buffer;
      imbyte* byte_data = (imbyte*)this->line_buffer;

      for (x = 0; x < this->width; x++)
      {
        unsigned int dword_value = dword_data[x];
        int c = x*4;
        byte_data[c]   = (imbyte)((this->rmask & dword_value) >> this->roff);
        byte_data[c+1] = (imbyte)((this->gmask & dword_value) >> this->goff);
        byte_data[c+2] = (imbyte)((this->bmask & dword_value) >> this->boff);
        byte_data[c+3] = (imbyte)((0xFF000000 & dword_value) >> 24);
      }
    }
    break;
  default: // 24
    {
      imbyte* byte_data = (imbyte*)this->line_buffer;
      for (x = 0; x < this->width; x++)
      {
        int c = x*3;
        imbyte temp = byte_data[c];     // swap R and B
        byte_data[c] = byte_data[c+2];
        byte_data[c+2] = temp;
      }
    }
    break;
  }
}

int imFileFormatBMP::ReadImageData(void* data)
{
  imCounterTotal(this->counter, this->height, "Reading BMP...");

  /* jump to the begin of image data */
  imBinFileSeekTo(handle, this->offset);

  for (int row = 0; row < this->height; row++)
  {
    /* read and decompress the data */
    if (this->comp_type == BMP_COMPRESS_RGB)
    {
      imBinFileRead(handle, this->line_buffer, this->line_raw_size, 1);

      if (imBinFileError(handle))
        return IM_ERR_ACCESS;     
    }
    else
    {
      if (iBMPDecodeScanLine(handle, (imbyte*)this->line_buffer, this->width) == IM_ERR_ACCESS)
        return IM_ERR_ACCESS;     
    }

    if (this->bpp > 8)
      FixRGBOrder();

    imFileLineBufferRead(this, data, row, 0);

    if (!imCounterInc(this->counter))
      return IM_ERR_COUNTER;
  }

  return IM_ERR_NONE;
}

int imFileFormatBMP::WriteImageData(void* data)
{
  imCounterTotal(this->counter, this->height, "Writing BMP...");

  imbyte* compressed_buffer = NULL;
  if (this->comp_type == BMP_COMPRESS_RLE8) // point to the extra buffer
    compressed_buffer = (imbyte*)this->line_buffer + this->line_buffer_size+4;

  for (int row = 0; row < this->height; row++)
  {
    imFileLineBufferWrite(this, data, row, 0);

    if (this->bpp > 8)
      FixRGBOrder();

    if (this->comp_type == BMP_COMPRESS_RGB)
    {
      imBinFileWrite(handle, this->line_buffer, this->line_raw_size, 1);
    }
    else
    {
      int compressed_size = iBMPEncodeScanLine(compressed_buffer, (imbyte*)this->line_buffer, this->width);
      imBinFileWrite(handle, compressed_buffer, compressed_size, 1);
    }

    if (imBinFileError(handle))
      return IM_ERR_ACCESS;     

    if (!imCounterInc(this->counter))
      return IM_ERR_COUNTER;
  }

  return IM_ERR_NONE;
}

int imFormatBMP::CanWrite(const char* compression, int color_mode, int data_type) const
{
  int color_space = imColorModeSpace(color_mode);

  if (color_space == IM_YCBCR || color_space == IM_LAB || 
      color_space == IM_LUV || color_space == IM_XYZ ||
      color_space == IM_CMYK)
    return IM_ERR_DATA;                       
                                              
  if (data_type != IM_BYTE)
    return IM_ERR_DATA;

  if (!compression || compression[0] == 0)
    return IM_ERR_NONE;

  if (!imStrEqual(compression, "NONE") && !imStrEqual(compression, "RLE"))
    return IM_ERR_COMPRESS;

  if (imStrEqual(compression, "RLE") && (color_space == IM_RGB || color_space == IM_BINARY))
    return IM_ERR_COMPRESS;

  return IM_ERR_NONE;
}
