/** \file
 * \brief PCX - ZSoft Picture
 *
 * See Copyright Notice in im_lib.h
 * $Id: im_format_pcx.cpp,v 1.3 2009/08/19 18:39:43 scuri Exp $
 */

#include "im_format.h"
#include "im_format_all.h"
#include "im_util.h"
#include "im_counter.h"

#include "im_binfile.h"

#include <stdlib.h>
#include <string.h>
#include <memory.h>

#define PCX_ID 0x0A


/* PCX file header */
/*  1   Id;             Manufacturer ID        */
/*  1   Version;        Version                */
/*  1   Encoding;       Encoding Scheme        */
/*  1   BitsPerPixel;   Bits/Pixel/Plane       */
/*  2   Xmin;           X Start (upper left)   */
/*  2   Ymin;           Y Start (top)          */
/*  2   Xmax;           X End (lower right)    */
/*  2   Ymax;           Y End (bottom)         */
/*  2   Hdpi;           Horizontal Resolution  */
/*  2   Vdpi;           Vertical Resolution    */
/*  3*16 Colormap;      16-Color EGA Palette   */
/*  1   Reserved;       Reserved               */
/*  1   NPlanes;        Number of Color Planes */
/*  2   BytesPerLine;   Bytes/Line/Plane       */
/*  2   PaletteInfo;    Palette Interpretation */
/*  2   HScreenSize;    Horizontal Screen Size */
/*  2   VScreenSize;    Vertical Screen Size   */
/*  54  Filler;         Reserved               */
/*  128    */

/* Default 16 color VGA palette */
static unsigned char iPCXDefaultPalette[3*16] = 
{
    0,   0,   0,
    0,   0, 255,
    0, 255,   0,
    0, 255, 255,
  255,   0,   0,
  255,   0, 255,
  255, 255,   0,
  255, 255, 255,
   85,  85, 255,
   85,  85,  85,
    0, 170,   0,
  170,   0,   0,
   85, 255, 255,
  255,  85, 255,
  255, 255,  85,
  255, 255, 255  
};

static int iPCXEncodeScanLine(unsigned char* EncodedBuffer, const unsigned char* DecodedBuffer, int BufferSize)
{
  int index = 0;        /* Index into uncompressed data buffer  */
  int scanindex = 0;    /* Index into compressed data buffer    */
  unsigned char runcount;                  /* Length of encoded pixel run          */
  unsigned char runvalue;                  /* Value of encoded pixel run           */

  while (index < BufferSize)
  {
    /** Get the run count of the next pixel value run.
    ** Pixel value runs are encoded until a different pixel value
    ** is encountered, the end of the scan line is reached, or 63
    ** pixel values have been counted. */
    for (runcount = 1, runvalue = DecodedBuffer[index]; 
         index + runcount < BufferSize && runvalue == DecodedBuffer[index + runcount] && runcount < 63; 
         runcount++);

    /** Encode the run into a one or two-unsigned char code.
    ** Multiple pixel runs are stored in two-unsigned char codes.  If a single
    ** pixel run has a value of less than 64 then it is stored in a
    ** one-unsigned char code.  If a single pixel run has a value of 64 to 255
    ** then it is stored in a two-unsigned char code. */

    if (runcount > 1)                   /* Multiple pixel run */
    {
      EncodedBuffer[scanindex++] = (unsigned char)(runcount | 0xC0);
      EncodedBuffer[scanindex++] = runvalue;
    }
    else                                /* Single pixel run   */
    {
      if (DecodedBuffer[index] < 64)  /* Value is 0 to 63   */
        EncodedBuffer[scanindex++] = runvalue;
      else                            /* Value is 64 to 255 */
      {
        EncodedBuffer[scanindex++] = (unsigned char)(runcount | 0xC0);
        EncodedBuffer[scanindex++] = runvalue;
      }
    }

    index += runcount;  /* Jump ahead to next pixel run value */
  }

  return scanindex;      /* Return the number of unsigned chars written to buffer */
}

static int iPCXDecodeScanLine(imBinFile* handle, unsigned char* DecodedBuffer, int BufferSize)
{
  int  index = 0;    /* Index into compressed scan line buffer */
  unsigned char data;          /* Data byte read from PCX file           */
  unsigned char runcount = 0;   /* Length of decoded pixel run            */
  unsigned char runvalue = 0;   /* Value of decoded pixel run             */

  while (index < BufferSize)    /* Read until the end of the buffer     */
  {
    imBinFileRead(handle, &data, 1, 1);

    if ((data & 0xC0) == 0xC0)              /* Two-unsigned char code    */
    {
      runcount = (unsigned char)(data & 0x3F);             /* Get run count    */
      imBinFileRead(handle, &runvalue, 1, 1);
    }
    else                                    /* One unsigned char code    */
    {
      runcount = 1;                       /* Run count is one */
      runvalue = data;                    /* Pixel value      */
    }

    if (imBinFileError(handle))
      return IM_ERR_ACCESS;

    /* Write the pixel run to the buffer */
    for (;runcount && (index < BufferSize); runcount--, index++)
      DecodedBuffer[index] = runvalue;    /* Assign value to buffer   */
  }

  return IM_ERR_NONE;      
}

static const char* iPCXCompTable[2] = 
{
  "NONE",
  "RLE"
};

class imFileFormatPCX: public imFileFormatBase
{
  imBinFile* handle;           /* the binary file handle */
  int bpp;                     /* number of bits per pixel */
  unsigned char version;       /* format version */
  unsigned char comp_type;      /* PCX compression information */
  int line_raw_size;         /* bytes per line per plane */

  int ReadPalette();
  int WritePalette();
  void Expand4bpp();
  void Pack24bpp();
  void Unpack24bpp();

public:
  imFileFormatPCX(const imFormat* _iformat): imFileFormatBase(_iformat) {}
  ~imFileFormatPCX() {}

  int Open(const char* file_name);
  int New(const char* file_name);
  void Close();
  void* Handle(int index);
  int ReadImageInfo(int index);
  int ReadImageData(void* data);
  int WriteImageInfo();
  int WriteImageData(void* data);
};

class imFormatPCX: public imFormat
{
public:
  imFormatPCX()
    :imFormat("PCX", 
              "ZSoft Picture", 
              "*.pcx;", 
              iPCXCompTable, 
              2, 
              0)
    {}
  ~imFormatPCX() {}

  imFileFormatBase* Create(void) const { return new imFileFormatPCX(this); }
  int CanWrite(const char* compression, int color_mode, int data_type) const;
};


void imFormatRegisterPCX(void)
{
  imFormatRegister(new imFormatPCX());
}

int imFileFormatPCX::Open(const char* file_name)
{
  unsigned char id;

  /* opens the binary file for reading with intel unsigned char order */
  handle = imBinFileOpen(file_name);
  if (!handle)
    return IM_ERR_OPEN;

  imBinFileByteOrder(handle, IM_LITTLEENDIAN); 

  /* reads the PCX format identifier */
  imBinFileRead(handle, &id, 1, 1);
  if (imBinFileError(handle))
  {
    imBinFileClose(handle);
    return IM_ERR_ACCESS;
  }

  if (id != PCX_ID)
  {
    imBinFileClose(handle);
    return IM_ERR_FORMAT;
  }

  /* reads the format version */
  imBinFileRead(handle, &this->version, 1, 1);

  /* reads the compression comp_type */
  imBinFileRead(handle, &this->comp_type, 1, 1);
  if (this->comp_type)
    strcpy(this->compression, "RLE");
  else
    strcpy(this->compression, "NONE");

  this->image_count = 1;

  return IM_ERR_NONE;
}

int imFileFormatPCX::New(const char* file_name)
{
  /* opens the binary file for writing with intel byte order */
  handle = imBinFileNew(file_name);
  if (!handle)
    return IM_ERR_OPEN;

  imBinFileByteOrder(handle, IM_LITTLEENDIAN); 

  return IM_ERR_NONE;
}

void imFileFormatPCX::Close()
{
  imBinFileClose(handle);
}

void* imFileFormatPCX::Handle(int index)
{
  if (index == 0)
    return (void*)this->handle;
  else
    return NULL;
}

int imFileFormatPCX::ReadImageInfo(int index)
{
  unsigned char bppp, planes;
  unsigned short xmin, xmax, ymax, ymin, word, bplp;
  (void)index;

  this->file_data_type = IM_BYTE;

  /* reads the Number of bits/pixel per plane */
  imBinFileRead(handle, &bppp, 1, 1);

  /* reads the image width and height */
  imBinFileRead(handle, &xmin, 1, 2);
  imBinFileRead(handle, &ymin, 1, 2);
  imBinFileRead(handle, &xmax, 1, 2);
  imBinFileRead(handle, &ymax, 1, 2);
  this->width = xmax - xmin + 1;
  this->height = ymax - ymin + 1;

  imAttribTable* attrib_table = AttribTable();

  if (xmin && ymin)
  {
    attrib_table->Set("XScreen", IM_USHORT, 1, &xmin);
    attrib_table->Set("YScreen", IM_USHORT, 1, &ymin);
  }

  /* read the x resolution */
  imBinFileRead(handle, &word, 1, 2);
  float xres = word;

  /* read the y resolution */
  imBinFileRead(handle, &word, 1, 2);
  float yres = word;

  if (xres && yres)
  {
    attrib_table->Set("XResolution", IM_FLOAT, 1, &xres);
    attrib_table->Set("YResolution", IM_FLOAT, 1, &yres);
    attrib_table->Set("ResolutionUnit", IM_BYTE, -1, "DPI");
  }

  /* jump 3*16+1 bytes (colormap + reserved) */
  imBinFileSeekOffset(handle, 3*16+1);

  /* reads the Number of color planes */
  imBinFileRead(handle, &planes, 1, 1);
  this->bpp = bppp * planes;

  /* reads the Number of bytes per scan line per color planes */
  imBinFileRead(handle, &bplp, 1, 2);
  this->line_raw_size = bplp * planes;
  this->line_buffer_extra = 2; // room enough for padding

  if (imBinFileError(handle))
    return IM_ERR_ACCESS;

  // sanity check
  if (this->bpp != 1 && this->bpp != 4 && 
      this->bpp != 8 && this->bpp != 24)
    return IM_ERR_DATA;

  if (this->bpp > 8)
  {
    this->file_color_mode = IM_RGB;
    this->file_color_mode |= IM_PACKED;
    this->line_buffer_extra += 3*this->width; // room for 24 bpp packing
  }
  else
  {
    this->file_color_mode = IM_MAP;
    this->palette_count = 1 << this->bpp;

    if (this->bpp == 1)  // only 1 bpp, 4 bpp will be expanded here
      this->convert_bpp = 1;

    if (this->bpp == 4)
      this->line_buffer_extra += this->width; // room for 4 bpp expansion
  }

  this->file_color_mode |= IM_TOPDOWN;

  if (this->bpp <= 8)
    return ReadPalette();

  return IM_ERR_NONE;
}

int imFileFormatPCX::WriteImageInfo()
{
  unsigned short word_value, bplp;
  unsigned char byte_value, filler[54+3*2];

  this->file_data_type = IM_BYTE;

  this->file_color_mode = imColorModeSpace(this->user_color_mode);

  if (imStrEqual(this->compression, "NONE"))
    this->comp_type = (unsigned char)0;
  else
    this->comp_type = (unsigned char)1;

  if (this->file_color_mode == IM_BINARY)
  {
    this->bpp = 1;
    this->convert_bpp = 1;
  }
  else if (this->file_color_mode == IM_RGB)
  {
    this->bpp = 24;
    this->file_color_mode |= IM_PACKED;
  }
  else
    this->bpp = 8;

  this->file_color_mode |= IM_TOPDOWN;

  int planes = imColorModeDepth(this->file_color_mode);
  bplp = (unsigned short)imFileLineSizeAligned(this->width, this->bpp/planes, 2);
  this->line_raw_size = bplp * planes;
  this->line_buffer_extra = 2; // room enough for padding

  if (this->comp_type || this->bpp == 24)
  {
    // allocates room for 24 bpp packing/unpacking and/or compression
    // allocates more than enough since compression algoritm can be ineficient
    this->line_buffer_extra += 2*this->line_raw_size;
  }

  this->version = 5;

  imAttribTable* attrib_table = AttribTable();
  /* writes the PCX file header */

  unsigned short xmin = 0, ymin = 0;
  const void* attrib_data = attrib_table->Get("XScreen");
  if (attrib_data) xmin = *(unsigned short*)attrib_data;
  attrib_data = attrib_table->Get("YScreen");
  if (attrib_data) ymin = *(unsigned short*)attrib_data;

  byte_value = PCX_ID;
  imBinFileWrite(handle, &byte_value, 1, 1); /* identifier */
  imBinFileWrite(handle, &this->version, 1, 1); /* format version */
  imBinFileWrite(handle, &this->comp_type, 1, 1); /* compression comp_type */
  byte_value = (imbyte)(this->bpp/planes);
  imBinFileWrite(handle, &byte_value, 1, 1); /* bits/pixel/plane */
  word_value = xmin;
  imBinFileWrite(handle, &word_value, 1, 2); /* xmin */
  word_value = ymin;
  imBinFileWrite(handle, &word_value, 1, 2); /* ymin */
  word_value = (unsigned short)(this->width - 1) + xmin;
  imBinFileWrite(handle, &word_value, 1, 2); /* xmax */
  word_value = (unsigned short)(this->height - 1) + ymin;
  imBinFileWrite(handle, &word_value, 1, 2); /* ymax */
  
  unsigned short hdpi = 0, vdpi = 0;
  attrib_data = attrib_table->Get("ResolutionUnit");
  if (attrib_data)
  {
    char* res_unit = (char*)attrib_data;

    float* xres = (float*)attrib_table->Get("XResolution");
    float* yres = (float*)attrib_table->Get("YResolution");

    if (imStrEqual(res_unit, "DPC"))
    {
      hdpi = (unsigned short)(*xres * 2.54);
      vdpi = (unsigned short)(*yres * 2.54);
    }
  }

  /* write the x resolution */
  word_value = hdpi;
  imBinFileWrite(handle, &word_value, 1, 2); /* hdpi */
                                                      
  /* write the y resolution */
  word_value = vdpi;
  imBinFileWrite(handle, &word_value, 1, 2); /* vdpi */

  imBinFileWrite(handle, iPCXDefaultPalette, 3*16, 1); /* 16 colors palette */
  byte_value = 0;
  imBinFileWrite(handle, &byte_value, 1, 1); /* reserved */
  byte_value = (imbyte)planes;
  imBinFileWrite(handle, &byte_value, 1, 1); /* planes */
  word_value = bplp;
  imBinFileWrite(handle, &word_value, 1, 2); /* bytes per line per plane */
  memset(filler, 0, 54+3*2);
  imBinFileWrite(handle, filler, 54+3*2, 1); /* palette info, hscreen size, vscreen size, filler */

  /* tests if everything was ok */
  if (imBinFileError(handle))
    return IM_ERR_ACCESS;

  return IM_ERR_NONE;
}

int imFileFormatPCX::ReadPalette()
{
  unsigned char pcx_colors[256 * 3];

  if (this->version == 5 && this->bpp == 1)
  {
    pcx_colors[0] = 0; pcx_colors[1] = 0; pcx_colors[2] = 0;
    pcx_colors[3] = 255; pcx_colors[4] = 255; pcx_colors[5] = 255;
  }
  else if (this->version == 5 && this->bpp == 8)
  {
    unsigned char ExtPal;

    /* jump to the end of file minus the palette data */
    imBinFileSeekFrom(handle, -769);

    /* reads palette identifier */
    imBinFileRead(handle, &ExtPal, 1, 1);

    if (ExtPal != 12)
      return IM_ERR_ACCESS;

    /* reads palette colors */
    imBinFileRead(handle, pcx_colors, 768, 1);
  }
  else if (this->version == 3)
  {
    memcpy(pcx_colors, iPCXDefaultPalette, this->palette_count * 3);
  }
  else
  {
    /* jump to the begining of the file at the start of the palette data */
    imBinFileSeekTo(handle, 4+6*2);

    /* reads palette colors */
    imBinFileRead(handle, pcx_colors, 3 * 16, 1);
  }

  if (imBinFileError(handle))
    return IM_ERR_ACCESS;

  /* convert the color map to the IM format */
  for (int c = 0; c < this->palette_count; c++)
  {
    int i = c * 3;                       
    this->palette[c] = imColorEncode(pcx_colors[i], pcx_colors[i+1], pcx_colors[i+2]);
  }

  return IM_ERR_NONE;
}

int imFileFormatPCX::WritePalette()
{
  unsigned char ExtPal = (unsigned char)12;
  unsigned char pcx_colors[256 * 3];

  /* convert the color map to the IM format */
  for (int c = 0; c < this->palette_count; c++)
  {
    int i = c * 3;
    imColorDecode(&pcx_colors[i], &pcx_colors[i+1], &pcx_colors[i+2], this->palette[c]);
  }

  /* writes the palette identifier */
  imBinFileWrite(handle, &ExtPal, 1, 1);

  /* writes the color palette */
  imBinFileWrite(handle, pcx_colors, 256 * 3, 1);

  if (imBinFileError(handle))
    return IM_ERR_ACCESS;

  return IM_ERR_NONE;
}

void imFileFormatPCX::Expand4bpp()
{
  int num_bits = 8, WidthDiv8 = (this->width + 7) / 8;

  int line_plane_size = this->line_raw_size / 4;
  imbyte *in_data = (unsigned char*)this->line_buffer;
  imbyte *out_data = in_data + this->line_buffer_size+2;

  for (int x = 0; x < WidthDiv8; x++)
  {
    imbyte b1 = in_data[x];
    imbyte b2 = (in_data + line_plane_size)[x];
    imbyte b3 = (in_data + 2 * line_plane_size)[x];
    imbyte b4 = (in_data + 3 * line_plane_size)[x];

    if (x == WidthDiv8-1)
      num_bits = this->width % 8;

    for (int b = 0; b < num_bits; b++)
    {
      imbyte byte_value = 0;

      /* If the most significant bit is set... */
      /* Set the appropriate bit in the higher order nibble */
      if (b1 & '\x80') byte_value |= 0x01;
      if (b2 & '\x80') byte_value |= 0x02;
      if (b3 & '\x80') byte_value |= 0x04;
      if (b4 & '\x80') byte_value |= 0x08;
      b1<<=1; b2<<=1; b3<<=1; b4<<=1;

      *out_data++ = byte_value;
    }
  }

  memcpy(this->line_buffer, in_data + this->line_buffer_size+2, this->width);
}

void imFileFormatPCX::Pack24bpp()
{
  imbyte *in_data = (unsigned char*)this->line_buffer;
  imbyte *out_data = in_data + this->line_buffer_size+2;

  int line_plane_size = this->line_raw_size / 3;

  imbyte *red   = in_data;
  imbyte *green = in_data + line_plane_size;
  imbyte *blue  = in_data + 2*line_plane_size;

  for (int i = 0; i < this->width; i++)
  {
    *out_data++ = *red++;
    *out_data++ = *green++;
    *out_data++ = *blue++;
  }

  memcpy(in_data, in_data + this->line_buffer_size+2, this->line_raw_size);
}

void imFileFormatPCX::Unpack24bpp()
{
  imbyte *in_data = (unsigned char*)this->line_buffer;
  imbyte *out_data = in_data + this->line_buffer_size+2;

  int line_plane_size = this->line_raw_size / 3;

  imbyte *red   = out_data;
  imbyte *green = out_data + line_plane_size;
  imbyte *blue  = out_data + 2*line_plane_size;

  for (int i = 0; i < this->width; i++)
  {
    *red++ = *in_data++;
    *green++ = *in_data++;
    *blue++ = *in_data++;
  }

  memcpy(out_data - (this->line_buffer_size+2), out_data, this->line_raw_size);
}

int imFileFormatPCX::ReadImageData(void* data)
{
  imCounterTotal(this->counter, this->height, "Reading PCX...");

  imBinFileSeekTo(handle, 128);

  for (int row = 0; row < this->height; row++)
  {
    /* read and decompress the data */
    if (this->comp_type)
    {
      if (iPCXDecodeScanLine(handle, (imbyte*)this->line_buffer, this->line_raw_size) == IM_ERR_ACCESS)
        return IM_ERR_ACCESS;     
    }
    else
    {
      imBinFileRead(handle, this->line_buffer, this->line_raw_size, 1);
      if (imBinFileError(handle))
        return IM_ERR_ACCESS;     
    }

    if (this->bpp == 4)
      Expand4bpp();

    if (this->bpp == 24)
      Pack24bpp();

    imFileLineBufferRead(this, data, row, 0);

    if (!imCounterInc(this->counter))
      return IM_ERR_COUNTER;
  }

  return IM_ERR_NONE;
}

int imFileFormatPCX::WriteImageData(void* data)
{
  imCounterTotal(this->counter, this->height, "Writing PCX...");

  imBinFileSeekTo(handle, 128);

  imbyte* compressed_buffer = NULL;
  if (this->comp_type) // point to the extra buffer
    compressed_buffer = (imbyte*)this->line_buffer + this->line_buffer_size+2;

  for (int row = 0; row < this->height; row++)
  {
    imFileLineBufferWrite(this, data, row, 0);

    if (this->bpp == 24)
      Unpack24bpp();

    /* compress and writes the data */
    /* the compressed buffer size will probably be diferent from the uncompressed buffer size */
    if (this->comp_type)
    {
      int compressed_size = iPCXEncodeScanLine(compressed_buffer, (imbyte*)this->line_buffer, this->line_raw_size);
      imBinFileWrite(handle, compressed_buffer, compressed_size, 1);
    }
    else
    {
      imBinFileWrite(handle, this->line_buffer, this->line_raw_size, 1);
    }

    if (imBinFileError(handle))
      return IM_ERR_ACCESS;     

    if (!imCounterInc(this->counter))
      return IM_ERR_COUNTER;
  }

  if (this->bpp == 8)
    return WritePalette();
  
  return IM_ERR_NONE;
}

int imFormatPCX::CanWrite(const char* compression, int color_mode, int data_type) const
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

  return IM_ERR_NONE;
}
