/** \file
 * \brief SGI - Silicon Graphics Image File Format
 *
 * See Copyright Notice in im_lib.h
 * $Id: im_format_sgi.cpp,v 1.3 2011/01/19 16:28:59 scuri Exp $
 */

#include "im_format.h"
#include "im_util.h"
#include "im_format_all.h"
#include "im_counter.h"

#include "im_binfile.h"

#include <stdlib.h>
#include <string.h>
#include <memory.h>

/*  File Header Structure. */
/*  2   Magic;          474 */
/*  1   Storage;        0 ou 1 Compression */
/*  1   BPC;            1 ou 2 Bytes Per Pixel Component */
/*  2   Dimension;      1 ou 2 ou 3 */
/*  2   XSize;          Width */
/*  2   YSize;          Height */
/*  2   ZSize;          Number of Channels. B/W=1, RGB=3, RGBA=4 */
/*  4   PixMin;         Minimum Pixel Value */
/*  4   PixMax;         Maximum Pixel Value */
/*  4   Dummy1; */
/*  80  ImageName;*/
/*  4   ColorMap;       0 ou 1 ou 2 ou 3 */
/*  404 Dummy2;*/
/*  512  */

#define SGI_ID  474

/* Compression */
#define SGI_VERBATIM 0
#define SGI_RLE      1

/*  ColorMap Ids */
#define SGI_NORMAL    0
#define SGI_DITHERED  1
#define SGI_SCREEN    2
#define SGI_COLORMAP  3

template <class T> 
static int iSGIDecodeScanLine(T *optr, const T *iptr, int width)
{
  T pixel;
  int c = 0, count;

  while (c < width)
  {
    pixel = *iptr++;

    count = pixel & 0x7f;
    if (!count)
      break;

    c += count;
    if (c > width)
      return IM_ERR_ACCESS;

    if (pixel & 0x80)
    {
      while (count--)
        *optr++ = *iptr++;
    }
    else
    {
      pixel = *iptr++;
      while (count--)
        *optr++ = pixel;
    }
  }

  if (c < width)
    return IM_ERR_ACCESS;

  return IM_ERR_NONE;
}

template <class T> 
static int iSGIEncodeScanLine(T *optr, const T *iptr, int width)
{
  const T *ibufend = iptr + width,
          *sptr;
  T *start_optr = optr;
  int todo, cc, count;

  while(iptr < ibufend) 
  {
    sptr = iptr;
    iptr += 2;
    while ((iptr < ibufend) && 
           ((iptr[-2] != iptr[-1]) || (iptr[-1] != iptr[0])))
      iptr++;
    iptr -= 2;
    count = iptr-sptr;

    while (count) 
    {
      todo = (count > 126) ? 126: count;
      count -= todo;
      *optr++ = (T)(0x80 | todo);
      while(todo--)
        *optr++ = *sptr++;
    }
    sptr = iptr;
    cc = *iptr++;

    while((iptr < ibufend) && (*iptr == cc))
      iptr++;
    count = iptr-sptr;

    while(count) 
    {
      todo = (count > 126)? 126: count;
      count -= todo;
      *optr++ = (T)todo;
      *optr++ = (T)cc;
    }
  }
  *optr++ = 0;

  return optr-start_optr;
}

static const char* iSGICompTable[2] = 
{
  "NONE",
  "RLE"
};

class imFileFormatSGI: public imFileFormatBase
{
  imBinFile* handle;          /* the binary file handle */
  unsigned char comp_type,    /* sgi compression information */
                bpc;          /* bytes per channels */
  unsigned int *starttab,     /* compression control buffer */
               *lengthtab;    /* compression control buffer */

public:
  imFileFormatSGI(const imFormat* _iformat): imFileFormatBase(_iformat) {}
  ~imFileFormatSGI() {}

  int Open(const char* file_name);
  int New(const char* file_name);
  void Close();
  void* Handle(int index);
  int ReadImageInfo(int index);
  int ReadImageData(void* data);
  int WriteImageInfo();
  int WriteImageData(void* data);
};

class imFormatSGI: public imFormat
{
public:
  imFormatSGI()
    :imFormat("SGI", 
              "Silicon Graphics Image File Format", 
              "*.rgb;*.rgba;*.bw;*.sgi;", 
              iSGICompTable, 
              2, 
              0)
    {}
  ~imFormatSGI() {}

  imFileFormatBase* Create(void) const { return new imFileFormatSGI(this); }
  int CanWrite(const char* compression, int color_mode, int data_type) const;
};

void imFormatRegisterSGI(void)
{
  imFormatRegister(new imFormatSGI());
}

int imFileFormatSGI::Open(const char* file_name)
{
  unsigned short word_value;

  /* opens the binary file for reading with motorola byte order */
  handle = imBinFileOpen(file_name);
  if (!handle)
    return IM_ERR_OPEN;

  imBinFileByteOrder(handle, IM_BIGENDIAN); 

  /* reads the SGI format identifier */
  imBinFileRead(handle, &word_value, 1, 2);
  if (imBinFileError(handle))
  {
    imBinFileClose(handle);
    return IM_ERR_ACCESS;
  }

  if (word_value != SGI_ID)
  {
    imBinFileClose(handle);
    return IM_ERR_FORMAT;
  }

  /* reads the compression information */
  imBinFileRead(handle, &this->comp_type, 1, 1);
  if (this->comp_type == SGI_RLE)
    strcpy(this->compression, "RLE");
  else if (this->comp_type == SGI_VERBATIM)
    strcpy(this->compression, "NONE");
  else
  {
    imBinFileClose(handle);
    return IM_ERR_COMPRESS;
  }

  this->starttab = NULL;
  this->lengthtab = NULL;

  this->image_count = 1;

  return IM_ERR_NONE;
}

int imFileFormatSGI::New(const char* file_name)
{
  /* opens the binary file for writing with motorola byte order */
  handle = imBinFileNew(file_name);
  if (!handle)
    return IM_ERR_OPEN;

  imBinFileByteOrder(handle, IM_BIGENDIAN); 

  this->starttab = NULL;
  this->lengthtab = NULL;

  this->image_count = 1;

  return IM_ERR_NONE;
}

void imFileFormatSGI::Close()
{
  if (this->starttab) free(this->starttab);
  if (this->lengthtab) free(this->lengthtab);
  imBinFileClose(handle);
}

void* imFileFormatSGI::Handle(int index)
{
  if (index == 0)
    return (void*)this->handle;
  else
    return NULL;
}

int imFileFormatSGI::ReadImageInfo(int index)
{
  (void)index;
  unsigned short word_value, dimension, depth;

  /* reads the number of bits per channel */
  imBinFileRead(handle, &this->bpc, 1, 1);

  /* reads the number of dimensions */
  imBinFileRead(handle, &dimension, 1, 2);

  /* reads the image width */
  imBinFileRead(handle, &word_value, 1, 2);
  this->width = word_value;

  /* reads the image height */
  imBinFileRead(handle, &word_value, 1, 2);
  this->height = word_value;

  /* reads the number of channels */
  imBinFileRead(handle, &depth, 1, 2);

  /* jump 12 bytes (min, max, dummy) */
  imBinFileSeekOffset(handle, 12);

  /* reads the image name */
  char image_name[80];
  imBinFileRead(handle, image_name, 80, 1);

  if (image_name[0] != 0)
    AttribTable()->Set("Description", IM_BYTE, imStrNLen(image_name, 80)+1, image_name);

  /* reads the color map information */
  unsigned int color_map_id; 
  imBinFileRead(handle, &color_map_id, 1, 4);

  if (imBinFileError(handle))
    return IM_ERR_ACCESS;

  this->file_data_type = IM_BYTE;
  if (this->bpc == 2)
    this->file_data_type = IM_USHORT;

  switch (dimension)
  {
  case 1:
    // If this value is 1, the image file consists of only 1 channel and only 1 scanline (row).
    // Only width is valid.
    this->height = 1;
    depth = 1;
  case 2:
    // If this value is 2, the file consists of a single channel with a number of scanlines. 
    // Only width and height are valid.
    depth = 1;
    break;
  case 3:
    break;
  default:
    return IM_ERR_DATA;
  }

  switch (color_map_id)
  {
  case SGI_NORMAL:
    switch(depth)
    {
    case 1:
      this->file_color_mode = IM_GRAY;
      break;
    case 2:
      // This is NOT mentioned by the specification, but it is used
      this->file_color_mode = IM_GRAY | IM_ALPHA;
      break;
    case 3:
      this->file_color_mode = IM_RGB;
      break;
    case 4:
      this->file_color_mode = IM_RGB | IM_ALPHA;
      break;
    default:
      return IM_ERR_DATA;
    }
    break;
  case SGI_DITHERED:
    this->file_color_mode = IM_MAP;
    break;
  case SGI_COLORMAP:
    this->file_color_mode = IM_RGB;
    break;
  case SGI_SCREEN:
    this->file_color_mode = IM_GRAY;
    break;
  default:
    return IM_ERR_DATA;
  }

  /* jump 404 bytes (dummy) */
  imBinFileSeekOffset(handle, 404);

  if (this->comp_type == SGI_RLE)
  {
    int tablen = this->height * depth;
    this->starttab = (unsigned int *)malloc(tablen * sizeof(int));
    this->lengthtab = (unsigned int *)malloc(tablen * sizeof(int));

    /* reads the compression control information */
    imBinFileRead(handle, this->starttab, tablen, 4);
    imBinFileRead(handle, this->lengthtab, tablen, 4);

    // allocates more than enough since compression algoritm can be ineficient
    this->line_buffer_extra = 2*imImageLineSize(this->width, this->file_color_mode, this->file_data_type);
  }

  if (imBinFileError(handle))
    return IM_ERR_ACCESS;

  if (color_map_id == SGI_DITHERED)
  {
    static int red[8] = {0, 36, 73, 109, 146, 182, 218, 255};
    static int green[8] = {0, 36, 73, 109, 146, 182, 218, 255};
    static int blue[4] = {0, 85, 170, 255};

    int c = 0;
    for (int b = 0; b < 4; b++)
    {
      for (int g = 0; g < 8; g++)
      {
        for (int r = 0; r < 8; r++)
        {
          this->palette[c] = imColorEncode((imbyte)red[r], 
                                           (imbyte)green[g], 
                                           (imbyte)blue[b]);
          c++;
        }
      }
    }
  }

  return IM_ERR_NONE;
}

int imFileFormatSGI::WriteImageInfo()
{
  unsigned int dword_value;
  unsigned short word_value;
  unsigned char dummy[404];
  memset(dummy, 0, 404);

  this->comp_type = SGI_VERBATIM;
  if (imStrEqual(this->compression, "RLE"))
    this->comp_type = SGI_RLE;

  unsigned int color_map_id = SGI_NORMAL;

  this->file_color_mode = imColorModeSpace(this->user_color_mode);

  int dimension = 2;
  if (this->file_color_mode == IM_BINARY)
    this->convert_bpp = -1; // expand 1 to 255
  else if (this->file_color_mode == IM_RGB)
  {
    dimension = 3;
    if (imColorModeHasAlpha(this->user_color_mode))
      this->file_color_mode |= IM_ALPHA;
  }
  else if (this->file_color_mode == IM_GRAY &&
           imColorModeHasAlpha(this->user_color_mode))
  {
    // This is NOT mentioned by the specification, but it is used
    dimension = 3;
    this->file_color_mode |= IM_ALPHA;
  }

  this->file_data_type = this->user_data_type;

  this->bpc = 1;
  int max = 255;
  if (this->file_data_type == IM_USHORT)
  {
    max = 65535;
    this->bpc = 2;
  }

  this->starttab = NULL;
  this->lengthtab = NULL;

  /* writes the SGI file header */
  word_value = SGI_ID;
  imBinFileWrite(handle, &word_value, 1, 2); /* identifier */
  imBinFileWrite(handle, &this->comp_type, 1, 1); /* storage */
  imBinFileWrite(handle, &this->bpc, 1, 1); /* bpc */
  word_value = (imushort)dimension;
  imBinFileWrite(handle, &word_value, 1, 2); /* dimension */
  word_value = (unsigned short)this->width;
  imBinFileWrite(handle, &word_value, 1, 2); /* image width */
  word_value = (unsigned short)this->height;
  imBinFileWrite(handle, &word_value, 1, 2); /* image height */
  word_value = (imushort)imColorModeDepth(this->file_color_mode);
  imBinFileWrite(handle, &word_value, 1, 2); /* depth */
  dword_value = 0;
  imBinFileWrite(handle, &dword_value, 1, 4); /* min */
  dword_value = max;
  imBinFileWrite(handle, &dword_value, 1, 4); /* max */
  imBinFileWrite(handle, dummy, 4, 1); /* dummy */

  /* tests if everything was ok */
  if (imBinFileError(handle))
    return IM_ERR_ACCESS;

  int size;
  char* image_name = (char*)AttribTable()->Get("Description", NULL, &size);
  if (image_name)
  {
    if (size < 80)
    {
      imBinFileWrite(handle, image_name, size, 1); 
      imBinFileWrite(handle, dummy, 80-size, 1); 
    }
    else
    {
      imBinFileWrite(handle, image_name, 79, 1); 
      imBinFileWrite(handle, (void*)"\0", 1, 1); 
    }
  }
  else
    imBinFileWrite(handle, dummy, 80, 1); /* empty image name */

  dword_value = color_map_id;
  imBinFileWrite(handle, &dword_value, 1, 4); /* color_map_id */
  imBinFileWrite(handle, dummy, 404, 1); /* dummy */

  /* tests if everything was ok */
  if (imBinFileError(handle))
    return IM_ERR_ACCESS;

  if (this->comp_type == SGI_RLE)
  {
    int tablen = this->height * imColorModeDepth(this->file_color_mode);
    this->starttab = (unsigned int *)malloc(tablen*4);
    this->lengthtab = (unsigned int *)malloc(tablen*4);

    /* writes the empty compression control information */
    /* we will write again at the end */
    imBinFileWrite(handle, this->starttab, tablen*4, 1);
    imBinFileWrite(handle, this->lengthtab, tablen*4, 1);

    // allocates more than enough since compression algoritm can be ineficient
    this->line_buffer_extra = 2*imImageLineSize(this->width, this->file_color_mode, this->file_data_type);
  }

  /* tests if everything was ok */
  if (imBinFileError(handle))
    return IM_ERR_ACCESS;

  return IM_ERR_NONE;
}

int imFileFormatSGI::ReadImageData(void* data)
{
  int count = imFileLineBufferCount(this);

  imCounterTotal(this->counter, count, "Reading SGI...");

  imbyte* compressed_buffer = NULL;
  if (this->comp_type == SGI_RLE)  // point to the extra buffer
    compressed_buffer = (imbyte*)this->line_buffer + this->line_buffer_size;

  int row = 0, plane = 0;
  for (int i = 0; i < count; i++)
  {
    if (this->comp_type == SGI_VERBATIM)
    {
      imBinFileRead(handle, this->line_buffer, this->line_buffer_size/this->bpc, this->bpc);

      if (imBinFileError(handle))
        return IM_ERR_ACCESS;     
    }
    else
    {
      int row_index = row + plane*this->height;
      imBinFileSeekTo(handle, this->starttab[row_index]);
      imBinFileRead(handle, compressed_buffer, this->lengthtab[row_index] / this->bpc, this->bpc);

      if (imBinFileError(handle))
        return IM_ERR_ACCESS;     

      if (this->bpc == 1)
        iSGIDecodeScanLine((imbyte*)this->line_buffer, compressed_buffer, this->width);
      else
        iSGIDecodeScanLine((imushort*)this->line_buffer, (imushort*)compressed_buffer, this->width);
    }

    imFileLineBufferRead(this, data, row, plane);

    if (!imCounterInc(this->counter))
      return IM_ERR_COUNTER;

    imFileLineBufferInc(this, &row, &plane);
  }

  return IM_ERR_NONE;
}

int imFileFormatSGI::WriteImageData(void* data)
{
  int count = imFileLineBufferCount(this);

  imCounterTotal(this->counter, count, "Writing SGI...");

  imbyte* compressed_buffer = NULL;
  if (this->comp_type == SGI_RLE)  // point to the extra buffer
    compressed_buffer = (imbyte*)this->line_buffer + this->line_buffer_size;

  int row = 0, plane = 0;
  for (int i = 0; i < count; i++)
  {
    imFileLineBufferWrite(this, data, row, plane);

    if (this->comp_type == SGI_VERBATIM)
      imBinFileWrite(handle, this->line_buffer, this->line_buffer_size/this->bpc, this->bpc);
    else
    {
      int length;
      if (this->bpc == 1)
        length = iSGIEncodeScanLine(compressed_buffer, (imbyte*)this->line_buffer, this->width);
      else
        length = iSGIEncodeScanLine((imushort*)compressed_buffer, (imushort*)this->line_buffer, this->width);

      int row_index = row + plane*this->height;
      this->starttab[row_index] = imBinFileTell(handle);
      this->lengthtab[row_index] = length*this->bpc;

      imBinFileWrite(handle, compressed_buffer, length, this->bpc);
    }

    if (imBinFileError(handle))
      return IM_ERR_ACCESS;     

    if (!imCounterInc(this->counter))
      return IM_ERR_COUNTER;

    imFileLineBufferInc(this, &row, &plane);
  }

  if (this->comp_type == SGI_RLE)
  {
    imBinFileSeekTo(this->handle, 512);
    int tablen = this->height * imColorModeDepth(this->file_color_mode);
    imBinFileWrite(handle, this->starttab, tablen, 4);
    imBinFileWrite(handle, this->lengthtab, tablen, 4);
  }

  return IM_ERR_NONE;
}

int imFormatSGI::CanWrite(const char* compression, int color_mode, int data_type) const
{
  int color_space = imColorModeSpace(color_mode);

  if (color_space == IM_YCBCR || color_space == IM_LAB || 
      color_space == IM_LUV || color_space == IM_XYZ ||
      color_space == IM_CMYK || color_space == IM_MAP)
    return IM_ERR_DATA;                       
                                              
  if (data_type != IM_BYTE && data_type != IM_USHORT)
    return IM_ERR_DATA;

  if (!compression || compression[0] == 0)
    return IM_ERR_NONE;

  if (!imStrEqual(compression, "NONE") && !imStrEqual(compression, "RLE"))
    return IM_ERR_COMPRESS;

  return IM_ERR_NONE;
}
