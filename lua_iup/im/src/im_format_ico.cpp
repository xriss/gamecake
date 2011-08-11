/** \file
 * \brief ICO - Windows Icon
 *
 * See Copyright Notice in im_lib.h
 * $Id: im_format_ico.cpp,v 1.3 2010/04/30 18:06:31 scuri Exp $
 */

#include "im_format.h"
#include "im_format_all.h"
#include "im_util.h"
#include "im_counter.h"

#include "im_binfile.h"

#include <stdlib.h>
#include <string.h>
#include <memory.h>

/*
typedef struct
{
    WORD           idReserved;   // Reserved (must be 0)
    WORD           idType;       // Resource Type (1 for icons)
    WORD           idCount;      // How many images?
    ICONDIRENTRY   idEntries[1]; // An entry for each image (idCount of 'em)
} ICONDIR, *LPICONDIR; // 6
typedef struct
{
    BYTE        bWidth;          // Width, in pixels, of the image
    BYTE        bHeight;         // Height, in pixels, of the image
    BYTE        bColorCount;     // Number of colors in image (0 if >=8bpp)
    BYTE        bReserved;       // Reserved ( must be 0)
    WORD        wPlanes;         // Color Planes
    WORD        wBitCount;       // Bits per pixel
    DWORD       dwBytesInRes;    // How many bytes in this resource?
    DWORD       dwImageOffset;   // Where in the file is this image?
} ICONDIRENTRY, *LPICONDIRENTRY;  // 16
typdef struct
{
   BITMAPINFOHEADER   icHeader;      // DIB header
   RGBQUAD         icColors[1];   // Color table
   BYTE            icXOR[1];      // DIB bits for XOR mask
   BYTE            icAND[1];      // DIB bits for AND mask (1 bpp)
} ICONIMAGE, *LPICONIMAGE;
*/

static const char* iICOCompTable[1] = 
{
  "NONE"
};

#define IMICON_MAX 10

class imFileFormatICO: public imFileFormatBase
{
  imBinFile* handle;          /* the binary file handle */
  unsigned short bpp;         /* number of bits per pixel */
  unsigned int offset[IMICON_MAX],
                next_offset;
  int line_raw_size;              // raw line size

  int ReadPalette();
  int WritePalette();
  void FixRGBOrder();

public:
  imFileFormatICO(const imFormat* _iformat): imFileFormatBase(_iformat) {}
  ~imFileFormatICO() {}

  int Open(const char* file_name);
  int New(const char* file_name);
  void Close();
  void* Handle(int index);
  int ReadImageInfo(int index);
  int ReadImageData(void* data);
  int WriteImageInfo();
  int WriteImageData(void* data);
};

class imFormatICO: public imFormat
{
public:
  imFormatICO()
    :imFormat("ICO", 
              "Windows Icon", 
              "*.ico;", 
              iICOCompTable, 
              1, 
              1)
    {}
  ~imFormatICO() {}

  imFileFormatBase* Create(void) const { return new imFileFormatICO(this); }
  int CanWrite(const char* compression, int color_mode, int data_type) const;
};


void imFormatRegisterICO(void)
{
  imFormatRegister(new imFormatICO());
}

int imFileFormatICO::Open(const char* file_name)
{
  unsigned short word;

  /* opens the binary file for reading with intel byte order */
  handle = imBinFileOpen(file_name);
  if (!handle)
    return IM_ERR_OPEN;

  imBinFileByteOrder(handle, IM_LITTLEENDIAN); 

  /* reads the reserved value */
  imBinFileRead(handle, &word, 1, 2);
  if (imBinFileError(handle))
  {
    imBinFileClose(handle);
    return IM_ERR_ACCESS;
  }

  if (word != 0)
  {
    imBinFileClose(handle);
    return IM_ERR_FORMAT;
  }

  /* reads the resource type */
  imBinFileRead(handle, &word, 1, 2);
  if (word != 1)
  {
    imBinFileClose(handle);
    return IM_ERR_FORMAT;
  }

  /* reads the number of images */
  imBinFileRead(handle, &word, 1, 2);

  this->image_count = word > IMICON_MAX? IMICON_MAX: word;
  strcpy(this->compression, "NONE");

  for (int i = 0; i < this->image_count; i++)
  {
    /* skip ICONDIRENTRY data except image offset */
    imBinFileSeekOffset(handle, 12);

    /* reads the image offset */
    imBinFileRead(handle, &this->offset[i], 1, 4);
   
    if (imBinFileError(handle))
    {
      imBinFileClose(handle);
      return IM_ERR_ACCESS;
    }
  }

  return IM_ERR_NONE;
}

int imFileFormatICO::New(const char* file_name)
{
  /* opens the binary file for writing with intel byte order */
  handle = imBinFileNew(file_name);
  if (!handle)
    return IM_ERR_OPEN;

  imBinFileByteOrder(handle, IM_LITTLEENDIAN); 

  imushort word_value = 0;
  imBinFileWrite(handle, &word_value, 1, 2); /* reserved */
  word_value = 1;
  imBinFileWrite(handle, &word_value, 1, 2); /* resource type */
  imBinFileWrite(handle, &word_value, 1, 2); /* number of images, at least one, must update at close */

  this->next_offset = 6 + IMICON_MAX * 16;  // offset to the first image, room for IMICON_MAX ICONDIRENTRY

  return IM_ERR_NONE;
}

void imFileFormatICO::Close()
{
  if (this->is_new)
  {
    if (this->image_count > 1)
    {
      imBinFileSeekTo(handle, 4);
      imushort word_value = (imushort)this->image_count;
      imBinFileWrite(handle, &word_value, 1, 2); /* number of images */
    }
  }

  imBinFileClose(handle);
}

void* imFileFormatICO::Handle(int index)
{
  if (index == 0)
    return (void*)this->handle;
  else
    return NULL;
}

int imFileFormatICO::ReadImageInfo(int index)
{
  this->file_data_type = IM_BYTE;
  unsigned int dword_value;

  if (index >= image_count)
    return IM_ERR_DATA;

  // offset + size
  imBinFileSeekTo(handle, this->offset[index] + 4);

  /* reads the image width */
  imBinFileRead(handle, &dword_value, 1, 4);
  this->width = (int)dword_value;

  /* reads the image height */
  imBinFileRead(handle, &dword_value, 1, 4);
  this->height = (int)(dword_value / 2);

  /* jump 2 bytes (planes) */
  imBinFileSeekOffset(handle, 2);

  // bpp
  imBinFileRead(handle, &this->bpp, 1, 2);

  if (imBinFileError(handle))
    return IM_ERR_ACCESS;

  // sanity check
  if (this->bpp != 1 && this->bpp != 4 && this->bpp != 8 &&
      this->bpp != 24 && this->bpp != 32)
    return IM_ERR_DATA;

  if (this->bpp > 8)
  {
    this->file_color_mode = IM_RGB;
    this->file_color_mode |= IM_PACKED;
    if (this->bpp == 32)
      this->file_color_mode |= IM_ALPHA;
  }
  else
  {
    this->file_color_mode = IM_MAP;
    this->palette_count = 1 << bpp;
  }

  if (this->bpp < 8)
    this->convert_bpp = this->bpp;

  this->line_raw_size = imFileLineSizeAligned(this->width, this->bpp, 4);
  this->line_buffer_extra = 4; // room enough for padding

  /* jump 8 bytes (compression, image size, resolution) */
  imBinFileSeekOffset(handle, 16);

  if (this->bpp <= 8)
  {
    /* reads the number of colors used */
    imBinFileRead(handle, &dword_value, 1, 4);

    /* updates the palette_count based on the number of colors used */
    if (dword_value != 0 && (int)dword_value < this->palette_count)
      this->palette_count = dword_value;

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

  return IM_ERR_NONE;
}

int imFileFormatICO::WriteImageInfo()
{
  this->file_data_type = IM_BYTE;
  this->file_color_mode = imColorModeSpace(this->user_color_mode);

  if (this->image_count == IMICON_MAX)
    return IM_ERR_DATA;

  if (this->width > 255 || this->height > 255)
    return IM_ERR_DATA;

  if (this->file_color_mode == IM_BINARY)
  {
    this->bpp = 1;
    this->convert_bpp = 1;
  }
  else if (this->file_color_mode == IM_RGB)
  {
    this->file_color_mode |= IM_PACKED;
    if (imColorModeHasAlpha(this->user_color_mode))
    {
      this->file_color_mode |= IM_ALPHA;
      this->bpp = 32;
    }
    else
      this->bpp = 24;
  }
  else
    this->bpp = 8;

  this->line_raw_size = imFileLineSizeAligned(this->width, this->bpp, 4);
  this->line_buffer_extra = 4; // room enough for padding
  int palette_size = (this->bpp > 8)? 0: this->palette_count*4;

  imbyte byte_value;
  imushort word_value;

  /* updates the ICON directory entry */

  imBinFileSeekTo(handle, 6 + this->image_count * 16);  // ICONDIR + i * ICONDIRENTRY

  byte_value = (imbyte)this->width;
  imBinFileWrite(handle, &byte_value, 1, 1); /* width */
  byte_value = (imbyte)this->height;
  imBinFileWrite(handle, &byte_value, 1, 1); /* height */
  byte_value = (imbyte)((this->bpp > 8)? 0: this->palette_count);
  imBinFileWrite(handle, &byte_value, 1, 1); /* color count */
  imBinFileWrite(handle, (void*)"\0", 1, 1);        /* reserved */
  word_value = 1;
  imBinFileWrite(handle, &word_value, 1, 2); /* planes */
  word_value = this->bpp;
  imBinFileWrite(handle, &word_value, 1, 2); /* bit count */
  int and_line_size = imFileLineSizeAligned(this->width, 1, 4);
  int resource_size = 40 + palette_size + (line_raw_size + and_line_size) * this->height;
  unsigned int dword_value = resource_size;
  imBinFileWrite(handle, &dword_value, 1, 4); /* resource size */
  dword_value = this->next_offset;
  imBinFileWrite(handle, &dword_value, 1, 4); /* data offset */

  this->offset[this->image_count] = this->next_offset;
  this->next_offset += resource_size;

  /* writes the image */

  imBinFileSeekTo(handle, this->offset[this->image_count]);

  dword_value = 40;
  imBinFileWrite(handle, &dword_value, 1, 4); /* header size */
  dword_value = this->width;
  imBinFileWrite(handle, &dword_value, 1, 4); /* width */
  dword_value = this->height*2;
  imBinFileWrite(handle, &dword_value, 1, 4); /* height */
  word_value = 1;
  imBinFileWrite(handle, &word_value, 1, 2);  /* planes */
  word_value = this->bpp;
  imBinFileWrite(handle, &word_value, 1, 2);  /* bpp */
  dword_value = 0;
  imBinFileWrite(handle, &dword_value, 1, 4); /* compression */
  dword_value = line_raw_size * this->height;
  imBinFileWrite(handle, &dword_value, 1, 4); /* data size */

  imBinFileWrite(handle, (void*)"\0\0\0\0\0\0\0\0", 8, 1); /* resolution */

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

int imFileFormatICO::ReadPalette()
{
  /* reads the color palette */
  unsigned char bmp_colors[256 * 4];
  imBinFileRead(handle, bmp_colors, this->palette_count * 4, 1);

  if (imBinFileError(handle))
    return IM_ERR_ACCESS;

  /* convert the color map to the IM format */
  for (int c = 0; c < this->palette_count; c++)
  {
    int i = c * 4;
    this->palette[c] = imColorEncode(bmp_colors[i + 2], 
                                     bmp_colors[i + 1], 
                                     bmp_colors[i]);
  }

  return IM_ERR_NONE;
}

int imFileFormatICO::WritePalette()
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

void imFileFormatICO::FixRGBOrder()
{
  if (this->bpp == 24)
  {
    imbyte* byte_data = (imbyte*)this->line_buffer;
    for (int x = 0; x < this->width; x++)
    {
      int c = x*3;
      imbyte temp = byte_data[c];     // swap R and B
      byte_data[c] = byte_data[c+2];
      byte_data[c+2] = temp;
    }
  }
  else /* bpp == 32 */
  {
    /* inverts the DWORD values if not intel */
    if (imBinCPUByteOrder() == IM_BIGENDIAN)
      imBinSwapBytes4(this->line_buffer, this->width);

    unsigned int* dword_data = (unsigned int*)this->line_buffer;
    imbyte* byte_data = (imbyte*)this->line_buffer;

    for (int x = 0; x < this->width; x++)
    {
      unsigned int dword_value = dword_data[x];
      int c = x*4;
      byte_data[c]   = (imbyte)((0x00FF0000 & dword_value) >> 16);
      byte_data[c+1] = (imbyte)((0x0000FF00 & dword_value) >> 8);
      byte_data[c+2] = (imbyte)((0x000000FF & dword_value) >> 0);
      byte_data[c+3] = (imbyte)((0xFF000000 & dword_value) >> 24);
    }
  }
}

static inline int PixelOffset(int is_top_down, int is_packed, int width, int height, int depth, int col, int row, int plane)
{
  if (is_top_down)
    row = height-1 - row;

  if (is_packed) 
    return row*width*depth + col*depth + plane;
  else           
    return plane*width*height + row*width + col;
}

int imFileFormatICO::ReadImageData(void* data)
{
  imCounterTotal(this->counter, this->height, "Reading ICO...");

  for (int row = 0; row < this->height; row++)
  {
    imBinFileRead(handle, this->line_buffer, this->line_raw_size, 1);
    if (imBinFileError(handle))
      return IM_ERR_ACCESS;     

    if (this->bpp > 8)
      FixRGBOrder();

    imFileLineBufferRead(this, data, row, 0);

    if (!imCounterInc(this->counter))
      return IM_ERR_COUNTER;
  }

  if ((imColorModeHasAlpha(this->user_color_mode) && this->bpp!=32) ||  /* user has alpha and file does not have alpha -> alpha came from AND data */
       imColorModeSpace(this->user_color_mode) == IM_MAP)   /* or MAP */
  {
    int line_size = imFileLineSizeAligned(this->width, 1, 4);
    int image_size = this->height*line_size;
    imbyte* and_data = new imbyte[image_size];

    imBinFileRead(handle, and_data, image_size, 1);
    if (imBinFileError(handle))
      return IM_ERR_ACCESS;     

    imbyte* and_data_line = and_data;
    imbyte* user_data = (imbyte*)data;
    unsigned long histo[256];
    int depth = imColorModeDepth(this->user_color_mode);
    int alpha_plane = 0;
    if (imColorModeHasAlpha(this->user_color_mode))
      alpha_plane = depth - 1;
    else
      memset(histo, 0, 256*sizeof(unsigned long));

    for (int j = 0; j < this->height; j++)
    {
      for (int i = 0; i < this->width; i++)
      {
        int offset = PixelOffset(imColorModeIsTopDown(this->user_color_mode), 
                                imColorModeIsPacked(this->user_color_mode), 
                                this->width, this->height, depth, i, j, alpha_plane);

        if (imColorModeHasAlpha(this->user_color_mode))
        {
          if (((and_data_line[i / 8] >> (7 - i % 8)) & 0x01))
            user_data[offset] = 0;
          else
            user_data[offset] = 255;
        }
        else
        {
          /* the most repeated index with transparency will be the transparent index. */
          if (((and_data_line[i / 8] >> (7 - i % 8)) & 0x01))
            histo[user_data[offset]]++;
        }
      }
      and_data_line += line_size;
    }

    if (imColorModeSpace(this->user_color_mode) == IM_MAP)
    {
      imbyte transp_index = 0;
      unsigned long histo_max = histo[0];

      for (int i = 1; i < 256; i++)
      {
        if (histo_max < histo[i])
        {
          histo_max = histo[i];
          transp_index = (imbyte)i;
        }
      }
      AttribTable()->Set("TransparencyIndex", IM_BYTE, 1, &transp_index);
    }

    delete [] and_data;
  }

  return IM_ERR_NONE;
}

int imFileFormatICO::WriteImageData(void* data)
{
  imCounterTotal(this->counter, this->height, "Writing ICO...");

  /* Image Data */

  for (int row = 0; row < this->height; row++)
  {
    imFileLineBufferWrite(this, data, row, 0);

    if (this->bpp > 8)
      FixRGBOrder();

    imBinFileWrite(handle, this->line_buffer, this->line_raw_size, 1);

    if (imBinFileError(handle))
      return IM_ERR_ACCESS;     

    if (!imCounterInc(this->counter))
      return IM_ERR_COUNTER;
  }

  /* AND Data */

  int and_line_size = imFileLineSizeAligned(this->width, 1, 4);
  int and_size = this->height*and_line_size;
  imbyte* and_data = new imbyte[and_size];
  memset(and_data, 0, and_size);  /* zero = opaque */

  if (imColorModeHasAlpha(this->user_color_mode))
  {
    imbyte* and_data_line = and_data;
    imbyte* user_data = (imbyte*)data;
    int depth = imColorModeDepth(this->user_color_mode);
    int alpha_plane = depth - 1;

    for (int j = 0; j < this->height; j++)
    {
      for (int i = 0; i < this->width; i++)
      {
        int offset = PixelOffset(imColorModeIsTopDown(this->user_color_mode), 
                                 imColorModeIsPacked(this->user_color_mode), 
                                 this->width, this->height, depth, i, j, alpha_plane);

        if (user_data[offset] == 0) /* mark only full transparent pixels */
          and_data_line[i / 8] |=  (0x01 << (7 - (i % 8)));
      }
      and_data_line += and_line_size;
    }
  }
  else
  {
    const imbyte* transp_index = (const imbyte*)AttribTable()->Get("TransparencyIndex");
    if (imColorModeSpace(this->user_color_mode) == IM_MAP && transp_index)
    {
      imbyte* and_data_line = and_data;
      imbyte* user_data = (imbyte*)data;
      int depth = imColorModeDepth(this->user_color_mode);

      for (int j = 0; j < this->height; j++)
      {
        for (int i = 0; i < this->width; i++)
        {
          int offset = PixelOffset(imColorModeIsTopDown(this->user_color_mode), 
                                  imColorModeIsPacked(this->user_color_mode), 
                                  this->width, this->height, depth, i, j, 0);

          if (user_data[offset] == *transp_index)
            and_data_line[i / 8] |=  (0x01 << (7 - (i % 8)));
        }
        and_data_line += and_line_size;
      }
    }
  }

  imBinFileWrite(handle, and_data, and_size, 1);
  delete [] and_data;

  if (imBinFileError(handle))
    return IM_ERR_ACCESS;     

  this->image_count++;

  return IM_ERR_NONE;
}

int imFormatICO::CanWrite(const char* compression, int color_mode, int data_type) const
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

  if (!imStrEqual(compression, "NONE"))
    return IM_ERR_COMPRESS;

  return IM_ERR_NONE;
}
