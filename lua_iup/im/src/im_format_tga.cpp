/** \file
 * \brief TGA - Truevision Graphics Adapter File
 *
 * See Copyright Notice in im_lib.h
 * $Id: im_format_tga.cpp,v 1.3 2010/08/13 14:19:49 scuri Exp $
 */

#include "im_format.h"
#include "im_util.h"
#include "im_format_all.h"
#include "im_counter.h"
#include "im_math.h"

#include "im_binfile.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <time.h>
#include <math.h>


/*
|--------|--------|------------------------------------------------------------|
|    0   |     1  |  Number of Characters in Identification Field.             |
|        |        |  This field is a one-byte unsigned integer, specifying     |
|        |        |  the length of the Image Identification Field.  Its range  |
|        |        |  is 0 to 255.  A value of 0 means that no Image            |
|        |        |  Identification Field is included.                         |
|--------|--------|------------------------------------------------------------|
|    1   |     1  |  Color Map Type.                                           |
|--------|--------|------------------------------------------------------------|
|    2   |     1  |  Image Type Code.                                          |
|--------|--------|------------------------------------------------------------|
|    3   |     5  |  Color Map Specification.                                  |
|    3   |     2  |  Color Map Origin.                                         |
|        |        |  Integer ( lo-hi ) index of first color map entry.         |
|    5   |     2  |  Color Map Length.                                         |
|        |        |  Integer ( lo-hi ) count of color map entries.             |
|    7   |     1  |  Color Map Entry Size.                                     |
|        |        |  Number of bits in each color map entry.  16 for           |
|        |        |  the Targa 16, 24 for the Targa 24, 32 for the Targa 32.   |
|--------|--------|------------------------------------------------------------|
|    8   |    10  |  Image Specification.                                      |
|    8   |     2  |  X Origin of Image.                                        |
|        |        |  Integer ( lo-hi ) X coordinate of the lower left corner   |
|        |        |  of the image.                                             |
|   10   |     2  |  Y Origin of Image.                                        |
|        |        |  Integer ( lo-hi ) Y coordinate of the lower left corner   |
|        |        |  of the image.                                             |
|   12   |     2  |  Width of Image.                                           |
|        |        |  Integer ( lo-hi ) width of the image in pixels.           |
|   14   |     2  |  Height of Image.                                          |
|        |        |  Integer ( lo-hi ) height of the image in pixels.          |
|   16   |     1  |  Image Pixel Size.                                         |
|        |        |  Number of bits in a stored pixel index.                   |
|   17   |     1  |  Image Descriptor Byte.                                    |
|        |        |  Bits 3-0 - number of attribute bits associated with each  |
|        |        |             pixel.                                         |
|        |        |  Bit 4    - reserved.  Must be set to 0.                   |
|        |        |  Bit 5    - screen origin bit.                             |
|        |        |             0 = Origin in lower left-hand corner.          |
|        |        |             1 = Origin in upper left-hand corner.          |
|        |        |             Must be 0 for Truevision images.               |
|        |        |  Bits 7-6 - Data storage interleaving flag.                |
|        |        |             00 = non-interleaved.                          |
|        |        |             01 = two-way (even/odd) interleaving.          |
|        |        |             10 = four way interleaving.                    |
|        |        |             11 = reserved.                                 |
|        |        |  This entire byte should be set to 0.  Don't ask me.       |
|--------|--------|------------------------------------------------------------|
|   18   | varies |  Image Identification Field.                               |
|        |        |  Contains a free-form identification field of the length   |
|        |        |  specified in byte 1 of the image record.  It's usually    |
|        |        |  omitted ( length in byte 1 = 0 ), but can be up to 255    |
|        |        |  characters.  If more identification information is        |
|        |        |  required, it can be stored after the image data.          |
|--------|--------|------------------------------------------------------------|

Extension Area:

* The inclusion of a scaled-down “postage stamp” copy of the image
* Date and Time of image file creation
* Author Name
* Author Comments
* Job Name
* Job Accumulated Time
* Gamma Value
* Correct Color LUT
* Pixel Aspect Ratio
* Scan Line Offset Table
* Key Color
* Software Package Name and Version Number
* Developer Definable Areas
* Attribute (Alpha) channel Type
* The ability for simple expansion
*/

static int iTGADecodeScanLine(imBinFile* handle, imbyte *DecodedBuffer, int width, int pixel_size)
{
  int i=0;
  unsigned char runcount; /* repetition count field */
  imbyte pixel_buffer[4];
  
  while (i < width) 
  { 
    imBinFileRead(handle, &runcount, 1, 1);  

    if (runcount & 0x80)
    { 
      imBinFileRead(handle, pixel_buffer, pixel_size, 1); 
      runcount &= 0x7F;

      if (imBinFileError(handle))
        return IM_ERR_ACCESS;     

      runcount++;      
      while (runcount-- && i < width)
      {
        memcpy(DecodedBuffer, pixel_buffer, pixel_size);
        i++;
        DecodedBuffer += pixel_size;
      }
    } 
    else
    { 
      runcount++;
      while (runcount-- && i < width)
      {
        imBinFileRead(handle, pixel_buffer, pixel_size, 1);
        memcpy(DecodedBuffer, pixel_buffer, pixel_size);
        i++;
        DecodedBuffer += pixel_size;
      }

      if (imBinFileError(handle))
        return IM_ERR_ACCESS;     
    }
  }

  return IM_ERR_NONE;
}

static inline int iTGAEqualPixel(const imbyte* Buffer1, const imbyte* Buffer2, int pixel_size)
{
  while(pixel_size--)
  {
    if (*Buffer1++ != *Buffer2++)
      return 0;
  }
  return 1;
}

static int iTGAEncodeScanLine(imbyte* EncodedBuffer, const imbyte* DecodedBuffer, int width, int pixel_size)
{
  imbyte pixel_buffer[4];
  unsigned char runcount; /* Length of encoded pixel run          */
  int x = 0;              /* Index into uncompressed data buffer  */
  imbyte* StartBuffer = EncodedBuffer;

  while (x < width)
  {
    runcount = 1;
    memcpy(pixel_buffer, &DecodedBuffer[x*pixel_size], pixel_size);

    // count equal pixels
    while (x+runcount < width && runcount < 128 && 
           iTGAEqualPixel(pixel_buffer, &DecodedBuffer[(x+runcount)*pixel_size], pixel_size))
      runcount++; 

    if (runcount == 1)
    {
      // count different pixels
      while (x+runcount+1 < width && runcount < 128)
      {
        memcpy(pixel_buffer, &DecodedBuffer[(x+runcount)*pixel_size], pixel_size);

        if (!iTGAEqualPixel(pixel_buffer, &DecodedBuffer[(x+runcount+1)*pixel_size], pixel_size))
          runcount++; 
        else
          break;
      }

      *EncodedBuffer++ = (imbyte)(runcount-1);

      memcpy(EncodedBuffer, &DecodedBuffer[x*pixel_size], runcount*pixel_size);
      EncodedBuffer += runcount*pixel_size;
    }
    else
    {
      *EncodedBuffer++ = (imbyte)(0x80 | (runcount-1));

      memcpy(EncodedBuffer, pixel_buffer, pixel_size);
      EncodedBuffer += pixel_size;
    }

    x += runcount;
  }

  return EncodedBuffer-StartBuffer;      /* Return the number of unsigned chars written to buffer */
}

static const char* iTGACompTable[2] = 
{
  "NONE",
  "RLE"
};

class imFileFormatTGA: public imFileFormatBase
{
  imBinFile* handle;          /* the binary file handle */
  unsigned char id_lenght;
  unsigned char map_type, image_type, map_bpp, bpp;

  int ReadPalette();
  int WritePalette();
  void FixRGB();
  int LoadExtensionArea();
  int SaveExtensionArea();

public:
  imFileFormatTGA(const imFormat* _iformat): imFileFormatBase(_iformat) {}
  ~imFileFormatTGA() {}

  int Open(const char* file_name);
  int New(const char* file_name);
  void Close();
  void* Handle(int index);
  int ReadImageInfo(int index);
  int ReadImageData(void* data);
  int WriteImageInfo();
  int WriteImageData(void* data);
};

class imFormatTGA: public imFormat
{
public:
  imFormatTGA()
    :imFormat("TGA", 
              "Truevision Graphics Adapter File", 
              "*.tga;*.icb;*.vst;*.tpic;", 
              iTGACompTable, 
              2, 
              0)
    {}
  ~imFormatTGA() {}

  imFileFormatBase* Create(void) const { return new imFileFormatTGA(this); }
  int CanWrite(const char* compression, int color_mode, int data_type) const;
};

void imFormatRegisterTGA(void)
{
  imFormatRegister(new imFormatTGA());
}

int imFileFormatTGA::Open(const char* file_name)
{
  /* opens the binary file for reading with intel byte order */
  handle = imBinFileOpen(file_name);
  if (!handle)
    return IM_ERR_OPEN;

  imBinFileByteOrder(handle, IM_LITTLEENDIAN); 
  
  imBinFileRead(handle, &this->id_lenght, 1, 1);
  imBinFileRead(handle, &this->map_type, 1, 1);
  imBinFileRead(handle, &this->image_type, 1, 1);

  if (imBinFileError(handle))
  {
    imBinFileClose(handle);
    return IM_ERR_ACCESS;
  }
  
  if (this->image_type != 1 && this->image_type != 2 && this->image_type != 3 && 
      this->image_type != 9 && this->image_type != 10 && this->image_type != 11)
  {
    imBinFileClose(handle);
    return IM_ERR_FORMAT;
  }
  
  if (this->map_type != 0 && this->map_type != 1)
  {
    imBinFileClose(handle);
    return IM_ERR_FORMAT;
  }
  
  if (this->map_type == 0 && (this->image_type == 1 || this->image_type == 9))
  {
    imBinFileClose(handle);
    return IM_ERR_FORMAT;
  }
  
  if (this->image_type == 9 || this->image_type == 10 || this->image_type == 11)
    strcpy(this->compression, "RLE");
  else
    strcpy(this->compression, "NONE");

  this->image_count = 1;

  return IM_ERR_NONE;
}

int imFileFormatTGA::New(const char* file_name)
{
  /* opens the binary file for writing with intel byte order */
  handle = imBinFileNew(file_name);
  if (!handle)
    return IM_ERR_OPEN;

  imBinFileByteOrder(handle, IM_LITTLEENDIAN); 

  return IM_ERR_NONE;
}

void imFileFormatTGA::Close()
{
  imBinFileClose(handle);
}

void* imFileFormatTGA::Handle(int index)
{
  if (index == 0)
    return (void*)this->handle;
  else
    return NULL;
}

int imFileFormatTGA::ReadImageInfo(int index)
{
  (void)index;
  unsigned char byte_value;
  unsigned short word_value;

  this->file_data_type = IM_BYTE;
  
  if (this->image_type == 1 || this->image_type == 9)
    this->file_color_mode = IM_MAP;
  else if (this->image_type == 2 || this->image_type == 10)
  {
    this->file_color_mode = IM_RGB;
    this->file_color_mode |= IM_PACKED;
  }
  else if (this->image_type == 3 || this->image_type == 11)
    this->file_color_mode = IM_GRAY;
  else
    return IM_ERR_DATA;

  if (this->map_type == 0)
    imBinFileSeekOffset(handle, 5);  // jump color map information
  else
  {
    /* jump 2 bytes (first entry index) */
    imBinFileSeekOffset(handle, 2);
    
    imBinFileRead(handle, &word_value, 1, 2);
    this->palette_count = word_value;
    
    imBinFileRead(handle, &this->map_bpp, 1, 1);

    if (this->map_bpp == 15) this->map_bpp = 16;

	  if (this->map_bpp != 16 && this->map_bpp != 24 && this->map_bpp != 32)
      return IM_ERR_DATA;
  }
  
  /* jump 4 bytes (X-Origin, Y-Origin) */
  unsigned short xmin, ymin;
  imBinFileRead(handle, &xmin, 1, 2);
  imBinFileRead(handle, &ymin, 1, 2);

  if (imBinFileError(handle))
    return IM_ERR_ACCESS;

  imAttribTable* attrib_table = AttribTable();

  if (xmin && ymin)
  {
    attrib_table->Set("XScreen", IM_USHORT, 1, &xmin);
    attrib_table->Set("YScreen", IM_USHORT, 1, &ymin);
  }
  
  /* reads the image width */
  imBinFileRead(handle, &word_value, 1, 2);
  this->width = word_value;
  
  /* reads the image height */
  imBinFileRead(handle, &word_value, 1, 2);
  this->height = word_value;

  if (imBinFileError(handle))
    return IM_ERR_ACCESS;
  
  imBinFileRead(handle, &this->bpp, 1, 1);
  
  if (this->bpp > 8 && imColorModeSpace(this->file_color_mode) != IM_RGB)
    return IM_ERR_DATA;
  
  if (this->bpp == 15) this->bpp = 16;

	if (this->bpp != 8 && this->bpp != 16 && 
      this->bpp != 24 && this->bpp != 32)
    return IM_ERR_DATA;

  if (this->bpp == 32)
    this->file_color_mode |= IM_ALPHA;

  // image descriptor
  imBinFileRead(handle, &byte_value, 1, 1);
  
  if (byte_value & 0x20)
    this->file_color_mode |= IM_TOPDOWN;
  
  // image ID
  if (this->id_lenght)
  {
    char desc[256];
    imBinFileRead(handle, desc, this->id_lenght, 1);
    desc[this->id_lenght] = 0;
    attrib_table->Set("Title", IM_BYTE, this->id_lenght+1, desc);
  }
  
  if (imBinFileError(handle))
    return IM_ERR_ACCESS;
  
  if (this->map_type)
  {
    if (!ReadPalette())
      return IM_ERR_ACCESS;
  }

  long cur_offset = imBinFileTell(handle);
  imBinFileSeekFrom(handle, -18);  
  char ext_sig[18];
  imBinFileRead(handle, ext_sig, 18, 1);
  if (ext_sig[17] == 0 && imStrEqual(ext_sig, "TRUEVISION-XFILE."))
  {
    if (!LoadExtensionArea())
      return IM_ERR_ACCESS;
  }
  imBinFileSeekTo(handle, cur_offset);  
  
  return IM_ERR_NONE;
}

int imFileFormatTGA::WriteImageInfo()
{
  unsigned char byte_value;
  unsigned short word_value;

  this->map_bpp = 0;
  this->map_type = 0;

  this->file_color_mode = imColorModeSpace(this->user_color_mode);
  switch (this->file_color_mode)
  {
  case IM_BINARY:
    this->convert_bpp = -1; // expand 1 to 255
  case IM_GRAY:
    this->bpp = 8;
    if (imStrEqual(this->compression, "RLE"))
      this->image_type = 11;
    else
      this->image_type = 3;
    break;
  case IM_MAP:
    this->bpp = 8;
    this->map_bpp = 24;
    this->map_type = 1;
    if (imStrEqual(this->compression, "RLE"))
      this->image_type = 9;
    else
      this->image_type = 1;
    break;
  case IM_RGB:
    if (imColorModeHasAlpha(this->user_color_mode))
    {
      this->bpp = 32;
      this->file_color_mode |= IM_ALPHA;
    }
    else
      this->bpp = 24;

    this->file_color_mode |= IM_PACKED;
    if (imStrEqual(this->compression, "RLE"))
      this->image_type = 10;
    else
      this->image_type = 2;
    break;
  }

  if (this->image_type > 3)
  {
    // allocates more than enough since compression algoritm can be ineficient
    this->line_buffer_extra += 2*this->width*imColorModeDepth(this->file_color_mode);
  }
  
  imAttribTable* attrib_table = AttribTable();

  /* writes the TGA file header */

  int length = 0;
  const char* desc_attrib = (const char*)attrib_table->Get("Title", NULL, &length);
  if (desc_attrib)
  {
    if (length > 255)
      this->id_lenght = 255;
    else
      this->id_lenght = (imbyte)length;
  }
  else
    this->id_lenght = 0;
  
  /* IDLength */
  imBinFileWrite(handle, &this->id_lenght, 1, 1); 
  
  /* Color Map Type */
  imBinFileWrite(handle, &this->map_type, 1, 1);
  
  /* Image Type */
  imBinFileWrite(handle, &this->image_type, 1, 1);
  
  /* Color Map Specification - 1st entry index */
  word_value = 0;
  imBinFileWrite(handle, &word_value, 1, 2); 
  
  /* Color map length */
  word_value = (unsigned short) this->palette_count;
  imBinFileWrite(handle, &word_value, 1, 2); 
  
  /* Color Map Entry size */
  byte_value = this->map_type? this->map_bpp: (imbyte)0;
  imBinFileWrite(handle, &byte_value, 1, 1); 

  if (imBinFileError(handle))
    return IM_ERR_ACCESS;
  
  unsigned short xmin = 0, ymin = 0;
  const void* attrib_data = attrib_table->Get("XScreen");
  if (attrib_data) xmin = *(unsigned short*)attrib_data;
  attrib_data = attrib_table->Get("YScreen");
  if (attrib_data) ymin = *(unsigned short*)attrib_data;

  /* X-orign of image */
  word_value = xmin;
  imBinFileWrite(handle, &word_value, 1, 2); 
  
  /* Y-orign of image */
  word_value = ymin;
  imBinFileWrite(handle, &word_value, 1, 2); 
  
  /* Image Width */
  word_value = (imushort)this->width;
  imBinFileWrite(handle, &word_value, 1, 2); 
  
  /* Image Height */
  word_value = (imushort)this->height;
  imBinFileWrite(handle, &word_value, 1, 2); 
  
  /* Pixel Depth */
  imBinFileWrite(handle, &this->bpp, 1, 1);  
  
  /* Image Descriptor */
  byte_value = 0x00;
  imBinFileWrite(handle, &byte_value, 1, 1);  
  
  /* image ID */
  if (this->id_lenght)
  {
    if (length > 255)
    {
      imBinFileWrite(handle, (void*)desc_attrib, 254, 1);  
      byte_value = 0x00;
      imBinFileWrite(handle, &byte_value, 1, 1);  
    }
    else
      imBinFileWrite(handle, (void*)desc_attrib, this->id_lenght, 1);  
  }
  
  /* tests if everything was ok */
  if (imBinFileError(handle))
    return IM_ERR_ACCESS;

  if (this->map_type)
  {
    if (!WritePalette())
      return IM_ERR_ACCESS;
  }
  
  return IM_ERR_NONE;
}

static long iTGARGB2Color(int c, unsigned char *colors, int map_bpp)
{
  unsigned char r,g,b;
  
  if (map_bpp == 16)
  {
    unsigned short word_value = ((unsigned short*)colors)[c];
    
    r = (imbyte)(((word_value & 0x7C00) >> 10)*8);
    g = (imbyte)(((word_value & 0x03E0) >>  5)*8);
    b = (imbyte)( (word_value & 0x001F)       *8);
  }
  else // 24 or 32
  {
    int i = c * (map_bpp / 8);
    
    r = colors[i+2];
    g = colors[i+1];
    b = colors[i];
  }
  
  return imColorEncode(r, g, b);
}

int imFileFormatTGA::ReadPalette()
{
  int map_size = imFileLineSizeAligned(this->palette_count, this->map_bpp, 1);
  unsigned char* tga_colors = (unsigned char*) malloc(map_size);
  
  /* reads the color palette */
  imBinFileRead(handle, tga_colors, map_size, 1);
  if (imBinFileError(handle))
    return 0;

  if (imBinCPUByteOrder() == IM_BIGENDIAN && this->map_bpp == 16)
	  imBinSwapBytes2(tga_colors, map_size/2);
  
  /* convert the color map to the IM format */
  for (int c = 0; c < this->palette_count; c++)
    this->palette[c] = iTGARGB2Color(c, tga_colors, this->map_bpp);
  
  free(tga_colors);
  
  return 1;
}

int imFileFormatTGA::WritePalette()
{
  unsigned char tga_color[256*3];
  
  /* convert the color map from the IM format */
  for (int c = 0; c < this->palette_count; c++)
  {
    int i = 3*c;
    imColorDecode(&tga_color[i+2], &tga_color[i+1], &tga_color[i], this->palette[c]);
  }
  
  /* writes the color palette */
  imBinFileWrite(handle, tga_color, this->palette_count * 3, 1);

  if (imBinFileError(handle))
    return 0;
  
  return 1;
}

int imFileFormatTGA::LoadExtensionArea()
{
  unsigned int dword_value;
  imBinFileSeekFrom(handle, -26);  

  // extension offset
  imBinFileRead(handle, &dword_value, 1, 4);
  if (imBinFileError(handle))
    return 0;

  imBinFileSeekTo(handle, dword_value);  
  if (imBinFileError(handle))
    return 0;

  unsigned short word_value;
  imbyte buffer[512];
  imAttribTable* attrib_table = AttribTable();

  // extension size
  imBinFileSeekOffset(handle, 2);  

  // author name
  imBinFileRead(handle, buffer, 41, 1);
  if (buffer[0] != 0)
    attrib_table->Set("Author", IM_BYTE, imStrNLen((char*)buffer, 41)+1, buffer);

  // author comments
  imBinFileRead(handle, buffer, 324, 1);
  if (buffer[0] != 0)
  {
    int size1 = imStrNLen((char*)buffer, 81);
    for (int i = 1; i < 4; i++)
    {
      int sizei = imStrNLen((char*)buffer + i*81, 81);
      if (sizei) 
      {
        memcpy(buffer + size1, buffer + i*81, sizei);
        size1 += sizei;
      }
    }
    buffer[size1] = 0;

    attrib_table->Set("Description", IM_BYTE, size1+1, buffer);
  }

  if (imBinFileError(handle))
    return 0;

  {
    tm ttm;
    ttm.tm_wday = 0;
    ttm.tm_yday = 0;
    ttm.tm_isdst = -1;

    int valid = 0;
    imBinFileRead(handle, &word_value, 1, 2); // moth
    ttm.tm_mon = word_value-1;
    if (word_value) valid = 1;
    imBinFileRead(handle, &word_value, 1, 2); // day
    ttm.tm_mday = word_value;
    if (word_value) valid = 1;
    imBinFileRead(handle, &word_value, 1, 2); // year
    ttm.tm_year = word_value-1900;
    if (word_value) valid = 1;
    imBinFileRead(handle, &word_value, 1, 2); // hour
    ttm.tm_hour = word_value;
    imBinFileRead(handle, &word_value, 1, 2); // minute
    ttm.tm_min = word_value;
    imBinFileRead(handle, &word_value, 1, 2); // seconds
    ttm.tm_sec = word_value;

    if (imBinFileError(handle))
      return 0;

    if (valid)
    {
      time_t tt = mktime(&ttm);
      char* str = ctime(&tt);
      if (str) 
      {
        int size = strlen(str);
        str[size-1] = 0;   // remove "\n"
        attrib_table->Set("DateTimeModified", IM_BYTE, size, str);
      }
    }
  }

  // job name
  imBinFileRead(handle, buffer, 41, 1);
  if (buffer[0] != 0)
    attrib_table->Set("JobName", IM_BYTE, imStrNLen((char*)buffer, 41)+1, buffer);

  // job time
  imBinFileSeekOffset(handle, 6);  

  // Software
  imBinFileRead(handle, buffer, 41, 1);
  if (buffer[0] != 0)
    attrib_table->Set("Software", IM_BYTE, imStrNLen((char*)buffer, 41)+1, buffer);

  if (imBinFileError(handle))
    return 0;

  // Software Version
  imBinFileRead(handle, &word_value, 1, 2);
  if (word_value)
  {
    int size = sprintf((char*)buffer, "%f", (double)word_value / 100.0);
    imBinFileRead(handle, &buffer[size], 1, 1);
    buffer[size+1] = 0;
    attrib_table->Set("SoftwareVersion", IM_BYTE, size+1, buffer);
  }

  // key color, aspect ratio
  imBinFileSeekOffset(handle, 8); 

  // gamma
  imBinFileRead(handle, &word_value, 1, 2); // num
  if (word_value)
  {
    float gamma = (float)word_value;
    imBinFileRead(handle, &word_value, 1, 2); // den
    if (word_value)
    {
      gamma /= (float)word_value;
      attrib_table->Set("Gamma", IM_FLOAT, 1, &gamma);
    }
  }

  if (imBinFileError(handle))
    return 0;

  return 1;
}

static void iGetRational(float fvalue, int *num, int *den)
{
  if (floorf(fvalue) == fvalue)
  {
    *num = (int)floorf(fvalue);
    *den = 1;
    return;
  }

  float ivalue = 1.0f/fvalue;
  if (floorf(ivalue) == ivalue)
  {
    *den = (int)floorf(ivalue);
    *num = 1;
    return;
  }

	*den = 1;
	if (fvalue > 0) 
  {
		while (fvalue < 1L<<(31-3) && *den < 1L<<(31-3))
    {
			fvalue *= 1<<3;
      *den *= 1<<3;
    }
	}

	*num = imRound(fvalue);
}

int imFileFormatTGA::SaveExtensionArea()
{
  unsigned int dword_value;
  unsigned short word_value;

  // get offset before write
  long ext_offset = imBinFileTell(handle);

  imbyte buffer[512];
  memset(buffer, 0, 512);

  imAttribTable* attrib_table = AttribTable();

  // extension size
  word_value = 495;
  imBinFileWrite(handle, &word_value, 1, 2);

  // author name
  int attrib_size;
  const void* attrib_data = attrib_table->Get("Author", NULL, &attrib_size);
  if (attrib_data)
  {
    int size = attrib_size > 41? 40: attrib_size;
    imBinFileWrite(handle, (void*)attrib_data, size, 1);
    if (size < 41)
      imBinFileWrite(handle, buffer, 41-size, 1);
  }
  else
    imBinFileWrite(handle, buffer, 41, 1);

  // author comments
  attrib_data = attrib_table->Get("Description", NULL, &attrib_size);
  if (attrib_data)
  {
    int size = 0, size2 = 0, i = 0;
    while(attrib_size && i < 4)
    {
      int line_size;
      if (attrib_size > 81)
        line_size = 80;
      else
        line_size = attrib_size;

      memcpy(buffer + size, (imbyte*)attrib_data + size2, line_size);

      attrib_size -= line_size;
      size2 += line_size;
      size += line_size;
      i++;

      int remain = 81-line_size;
      if (remain)
      {
        memset(buffer + size, 0, remain);
        size += remain;
      }
    }

    imBinFileWrite(handle, buffer, 324, 1);
    memset(buffer, 0, 512);
  }
  else
    imBinFileWrite(handle, buffer, 324, 1);

  if (imBinFileError(handle))
    return 0;

  attrib_data = attrib_table->Get("DateTimeModified");
  if (attrib_data)
  {
    time_t cur_time;
    time(&cur_time);
    tm* ttm = localtime(&cur_time);

    word_value = (imushort)ttm->tm_mon+1;
    imBinFileWrite(handle, &word_value, 1, 2); // moth
    word_value = (imushort)ttm->tm_mday;
    imBinFileWrite(handle, &word_value, 1, 2); // day
    word_value = (imushort)ttm->tm_year+1900;
    imBinFileWrite(handle, &word_value, 1, 2); // year
    word_value = (imushort)ttm->tm_hour;
    imBinFileWrite(handle, &word_value, 1, 2); // hour
    word_value = (imushort)ttm->tm_min;
    imBinFileWrite(handle, &word_value, 1, 2); // minute
    word_value = (imushort)ttm->tm_sec;
    imBinFileWrite(handle, &word_value, 1, 2); // seconds

    if (imBinFileError(handle))
      return 0;
  }
  else
    imBinFileWrite(handle, buffer, 12, 1);

  // job name
  attrib_data = attrib_table->Get("JobName", NULL, &attrib_size);
  if (attrib_data)
  {
    int size = attrib_size > 41? 40: attrib_size;
    imBinFileWrite(handle, (void*)attrib_data, size, 1);
    if (size < 41)
      imBinFileWrite(handle, buffer, 41-size, 1);
  }
  else
    imBinFileWrite(handle, buffer, 41, 1);

  // job time
  imBinFileWrite(handle, buffer, 6, 1);

  // Software
  attrib_data = attrib_table->Get("Software", NULL, &attrib_size);
  if (attrib_data)
  {
    int size = attrib_size > 41? 40: attrib_size;
    imBinFileWrite(handle, (void*)attrib_data, size, 1);
    if (size < 41)
      imBinFileWrite(handle, buffer, 41-size, 1);
  }
  else
    imBinFileWrite(handle, buffer, 41, 1);

  if (imBinFileError(handle))
    return 0;

  // Software Version, key color, aspect ratio
  imBinFileWrite(handle, buffer, 11, 1);

  // gamma
  attrib_data = attrib_table->Get("Gamma");
  if (attrib_data)
  {
    float gamma = *(float*)attrib_data;

    int num, den;
    iGetRational(gamma, &num, &den);

    word_value = (imushort)num;
    imBinFileWrite(handle, &word_value, 1, 2); // num
    word_value = (imushort)den;
    imBinFileWrite(handle, &word_value, 1, 2); // den
  }
  else
    imBinFileWrite(handle, buffer, 4, 1);

  // Color Correction, Postage Stamp, Scanline Offset, Attributes Type
  imBinFileWrite(handle, buffer, 13, 1);

  // FOOTER

  // extension offset
  dword_value = ext_offset;
  imBinFileWrite(handle, &dword_value, 1, 4);

  // Developer Directory Offset
  imBinFileWrite(handle, buffer, 4, 1);

  // signature, reserved, zero string terminator
  imBinFileWrite(handle, (void*)"TRUEVISION-XFILE.\0", 18, 1);

  if (imBinFileError(handle))
    return 0;

  return 1;
}

void imFileFormatTGA::FixRGB()
{
  int x;
  imbyte* byte_data = (imbyte*)this->line_buffer;

  if (this->bpp == 16)
  {
    /* inverts the WORD values if not intel */
    if (imBinCPUByteOrder() == IM_BIGENDIAN)
      imBinSwapBytes2(this->line_buffer, this->width);

    imushort* word_data = (imushort*)this->line_buffer;

    // from end to start
    for (x = this->width-1; x >= 0; x--)
    {
      imushort word_value = word_data[x];
      int c = x*3;
      byte_data[c]   = (imbyte)(((word_value & 0x7C00) >> 10)*8);
      byte_data[c+1] = (imbyte)(((word_value & 0x03E0) >>  5)*8);
      byte_data[c+2] = (imbyte)( (word_value & 0x001F)       *8);
    }
  }
  else  // 24 and 32
  {
    // convert BGR <-> RGB
    // convert BGRA <-> RGBA
    imbyte* byte_data = (imbyte*)this->line_buffer;
    int planes = this->bpp/8;
    for (x = 0; x < this->width; x++)
    {
      int c = x*planes;
      imbyte temp = byte_data[c];     // swap R and B
      byte_data[c] = byte_data[c+2];
      byte_data[c+2] = temp;
    }
  }
}

int imFileFormatTGA::ReadImageData(void* data)
{
  imCounterTotal(this->counter, this->height, "Reading TGA...");

  int line_size = this->line_buffer_size;
  if (this->bpp == 16)
    line_size = this->width*2;

  for (int row = 0; row < this->height; row++)
  {
    if (this->image_type > 3)
    {
      if (iTGADecodeScanLine(handle, (imbyte*)this->line_buffer, this->width, this->bpp/8) == IM_ERR_ACCESS)
        return IM_ERR_ACCESS;     
    }
    else
    {
      imBinFileRead(handle, this->line_buffer, line_size, 1);
      if (imBinFileError(handle))
        return IM_ERR_ACCESS;     
    }

    if (this->bpp > 8)
      FixRGB();
  
    imFileLineBufferRead(this, data, row, 0);

    if (!imCounterInc(this->counter))
      return IM_ERR_COUNTER;
  }
  
  return IM_ERR_NONE;
}

int imFileFormatTGA::WriteImageData(void* data)
{
  imCounterTotal(this->counter, this->height, "Writing TGA...");

  imbyte* compressed_buffer = NULL;
  if (this->image_type > 3)  // point to the extra buffer
    compressed_buffer = (imbyte*)this->line_buffer + this->line_buffer_size;

  for (int row = 0; row < this->height; row++)
  {
    imFileLineBufferWrite(this, data, row, 0);

    if (this->bpp > 8)
      FixRGB();

    if (this->image_type > 3)
    {
      int compressed_size = iTGAEncodeScanLine(compressed_buffer, (imbyte*)this->line_buffer, this->width, this->bpp/8);
      imBinFileWrite(handle, compressed_buffer, compressed_size, 1);
    }
    else
    {
      imBinFileWrite(handle, this->line_buffer, this->line_buffer_size, 1);
    }

    if (imBinFileError(handle))
      return IM_ERR_ACCESS;     

    if (!imCounterInc(this->counter))
      return IM_ERR_COUNTER;
  }

  if (!SaveExtensionArea())
    return IM_ERR_ACCESS;     

  return IM_ERR_NONE;
}

int imFormatTGA::CanWrite(const char* compression, int color_mode, int data_type) const
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
