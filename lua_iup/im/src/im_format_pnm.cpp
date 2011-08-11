/** \file
 * \brief PNM - Netpbm Portable Image Map
 *
 * See Copyright Notice in im_lib.h
 * $Id: im_format_pnm.cpp,v 1.3 2009/10/01 14:15:47 scuri Exp $
 */

#include "im_format.h"
#include "im_format_all.h"
#include "im_util.h"
#include "im_counter.h"

#include "im_binfile.h"

#include <stdlib.h>
#include <string.h>
#include <memory.h>


/* comments start with '#' after the first \n */
static int iPNMReadComment(imBinFile* handle, char* comment, int *size)
{
  imbyte byte_value = 0;

  // find the first \n
  while(byte_value != '\n')
  {
    imBinFileRead(handle, &byte_value, 1, 1);
    if (imBinFileError(handle))
      return 0;
  }

  *size = 0;

  imBinFileRead(handle, &byte_value, 1, 1);
  if (imBinFileError(handle))
    return 0;

  if (byte_value == '#')
  {
    while(byte_value != '\n')
    {
      imBinFileRead(handle, &byte_value, 1, 1);
      if (imBinFileError(handle))
        return 0;

      if (byte_value != '\r')
      {
        comment[*size] = byte_value;
        (*size)++;
      }
    }
  }
  else
    imBinFileSeekOffset(handle, -1);

  if (*size != 0)
  {
    comment[*size] = 0;
    (*size)++;
  }

  return 1;
}

static const char* iPNMCompTable[2] = 
{
  "NONE",
  "ASCII"
};

class imFileFormatPNM: public imFileFormatBase
{
  imBinFile* handle;          /* the binary file handle */
  unsigned char image_type;

  void FixBinary();

public:
  imFileFormatPNM(const imFormat* _iformat): imFileFormatBase(_iformat) {}
  ~imFileFormatPNM() {}

  int Open(const char* file_name);
  int New(const char* file_name);
  void Close();
  void* Handle(int index);
  int ReadImageInfo(int index);
  int ReadImageData(void* data);
  int WriteImageInfo();
  int WriteImageData(void* data);
};

class imFormatPNM: public imFormat
{
public:
  imFormatPNM()
    :imFormat("PNM", 
              "Netpbm Portable Image Map", 
              "*.pnm;*.pbm;*.ppm;*.pgm;", 
              iPNMCompTable, 
              2, 
              1)
    {}
  ~imFormatPNM() {}

  imFileFormatBase* Create(void) const { return new imFileFormatPNM(this); }
  int CanWrite(const char* compression, int color_mode, int data_type) const;
};


void imFormatRegisterPNM(void)
{
  imFormatRegister(new imFormatPNM());
}

int imFileFormatPNM::Open(const char* file_name)
{
  unsigned char sig[2];

  /* opens the binary file for reading */
  handle = imBinFileOpen(file_name);
  if (!handle)
    return IM_ERR_OPEN;

  /* reads the PNM format identifier */
  imBinFileRead(handle, sig, 2, 1);
  if (imBinFileError(handle))
  {
    imBinFileClose(handle);
    return IM_ERR_ACCESS;
  }

  if (sig[0] != 'P' || (sig[1] != '1' && sig[1] != '2' &&
                        sig[1] != '3' && sig[1] != '4' &&
                        sig[1] != '5' && sig[1] != '6'))
  {
    imBinFileClose(handle);
    return IM_ERR_FORMAT;
  }

  this->image_type = sig[1];
  this->image_count = 1;     // increment this if found image after data

  if (this->image_type == '1' || this->image_type == '2' || this->image_type == '3')
    strcpy(this->compression, "ASCII");
  else
    strcpy(this->compression, "NONE");

  return IM_ERR_NONE;
}

int imFileFormatPNM::New(const char* file_name)
{
  /* opens the binary file for writing */
  handle = imBinFileNew(file_name);
  if (!handle)
    return IM_ERR_OPEN;

  this->image_count = 1;  

  return IM_ERR_NONE;
}

void imFileFormatPNM::Close()
{
  imBinFileClose(handle);
}

void* imFileFormatPNM::Handle(int index)
{
  if (index == 0)
    return (void*)this->handle;
  else
    return NULL;
}

int imFileFormatPNM::ReadImageInfo(int index)
{
  (void)index;

  switch (this->image_type)
  {
  case '4':
    this->convert_bpp = 1;
  case '1':
    this->file_color_mode = IM_BINARY;
    break;
  case '2':
  case '5':
    this->file_color_mode = IM_GRAY;
    break;
  case '3':
  case '6':
    this->file_color_mode = IM_RGB | IM_PACKED;
    break;
  }

  this->file_color_mode |= IM_TOPDOWN;

  imAttribTable* attrib_table = AttribTable();

  char comment[4096];
  int size;
  if (!iPNMReadComment(handle, comment, &size))
    return IM_ERR_ACCESS;

  if (size)
    attrib_table->Set("Description", IM_BYTE, size, comment);

  if (!imBinFileReadInteger(handle, &this->width))
    return IM_ERR_ACCESS;

  if (!imBinFileReadInteger(handle, &this->height))
    return IM_ERR_ACCESS;

  if (this->height <= 0 || this->width <= 0)
    return IM_ERR_DATA;

  int max_val = 255;
  if (this->image_type != '4' && this->image_type != '1')
  {
    if (!imBinFileReadInteger(handle, &max_val))
      return IM_ERR_ACCESS;
  }

  this->file_data_type = IM_BYTE;
  if (max_val > 255)
    this->file_data_type = IM_USHORT;

  return IM_ERR_NONE;
}

int imFileFormatPNM::WriteImageInfo()
{
  this->file_data_type = this->user_data_type;
  this->file_color_mode = imColorModeSpace(this->user_color_mode);

  int ascii;
  if (imStrEqual(this->compression, "ASCII"))
    ascii = 1;
  else
    ascii = 0;

  switch (this->file_color_mode)
  {
  case IM_BINARY:
    if (ascii)
      this->image_type = '1';
    else
    {
      this->image_type = '4';
      this->convert_bpp = 1;
    }
    break;
  case IM_GRAY:
    if (ascii)
      this->image_type = '2';
    else
      this->image_type = '5';
    break;
  case IM_RGB:
    if (ascii)
      this->image_type = '3';
    else
      this->image_type = '6';
    this->file_color_mode |= IM_PACKED;
    break;
  }

  this->file_color_mode |= IM_TOPDOWN;

  imBinFilePrintf(handle, "P%c\n", (int)this->image_type);

  if (imBinFileError(handle))
    return IM_ERR_ACCESS;

  imAttribTable* attrib_table = AttribTable();

  int attrib_size;
  const void* attrib_data = attrib_table->Get("Description", NULL, &attrib_size);
  if (attrib_data)
  {
    char* desc = (char*)attrib_data;
    int size = 0;
    while(size < (attrib_size-1) && (desc[size] != '\r' && desc[size] != '\n'))
      size++;

    imBinFileWrite(handle, (void*)"#", 1, 1);
    imBinFileWrite(handle, desc, size, 1);
    imBinFileWrite(handle, (void*)"\n", 1, 1);
  }

  imBinFilePrintf(handle, "%d\n", this->width);
  imBinFilePrintf(handle, "%d\n", this->height);

  if (this->image_type != '4' && this->image_type != '1')
  {
    int max_val = 255;
    if (this->file_data_type == IM_USHORT)
      max_val = 65535;

    imBinFilePrintf(handle, "%d\n", max_val);
  }
  
  /* tests if everything was ok */
  if (imBinFileError(handle))
    return IM_ERR_ACCESS;

  return IM_ERR_NONE;
}

void imFileFormatPNM::FixBinary()
{
  unsigned char* buf = (unsigned char*)this->line_buffer;
  for (int b = 0; b < this->line_buffer_size; b++)
  {
    *buf = ~(*buf);
    buf++;
  }
}

int imFileFormatPNM::ReadImageData(void* data)
{
  imCounterTotal(this->counter, this->height, "Reading PNM...");

  int line_count = imImageLineCount(this->width, this->file_color_mode);

  int line_raw_size;
  if (this->image_type == '4')
    line_raw_size = imFileLineSizeAligned(this->width, 1, 1);
  else
    line_raw_size = imImageLineSize(this->width, this->file_color_mode, this->file_data_type);

  int ascii = 0;
  if (this->image_type == '1' || this->image_type == '2' || this->image_type == '3')
    ascii = 1;

  for (int row = 0; row < this->height; row++)
  {
    if (ascii)
    {
      int value;
      for (int col = 0; col < line_count; col++)
      {
        if (!imBinFileReadInteger(handle, &value))
          return IM_ERR_ACCESS;

        if (this->image_type == '1' && value < 2)
          value = 1 - value;

        if (this->file_data_type == IM_USHORT)
          ((imushort*)this->line_buffer)[col] = (imushort)value;
        else
          ((imbyte*)this->line_buffer)[col] = (unsigned char)value;
      }
    }
    else
    {
      imBinFileRead(handle, this->line_buffer, line_raw_size, 1);

      if (imBinFileError(handle))
        return IM_ERR_ACCESS;     

      if (this->image_type == '4')
        FixBinary();
    }

    imFileLineBufferRead(this, data, row, 0);

    if (!imCounterInc(this->counter))
      return IM_ERR_COUNTER;
  }

  // try to find another image, ignore errors from here

  /* reads the PNM format identifier */
  unsigned char sig[2];
  imBinFileRead(handle, sig, 2, 1);
  if (imBinFileError(handle))
    return IM_ERR_NONE;

  if (sig[0] != 'P' || (sig[1] != '1' && sig[1] != '2' &&
                        sig[1] != '3' && sig[1] != '4' &&
                        sig[1] != '5' && sig[1] != '6'))
    return IM_ERR_NONE;

  this->image_type = sig[1];
  this->image_count++;

  if (this->image_type == '1' || this->image_type == '2' || this->image_type == '3')
    strcpy(this->compression, "ASCII");
  else
    strcpy(this->compression, "NONE");

  return IM_ERR_NONE;
}

int imFileFormatPNM::WriteImageData(void* data)
{
  imCounterTotal(this->counter, this->height, "Writing PNM...");

  int line_count = imImageLineCount(this->width, this->file_color_mode);

  int line_raw_size;
  if (this->image_type == '4')
    line_raw_size = imFileLineSizeAligned(this->width, 1, 1);
  else
    line_raw_size = imImageLineSize(this->width, this->file_color_mode, this->file_data_type);

  int ascii = 0;
  if (this->image_type == '1' || this->image_type == '2' || this->image_type == '3')
    ascii = 1;

  for (int row = 0; row < this->height; row++)
  {
    imFileLineBufferWrite(this, data, row, 0);

    if (ascii)
    {
      int line_size = 0;
      for (int col = 0; col < line_count; col++)
      {
        int value;
        if (this->file_data_type == IM_USHORT)
          value = ((imushort*)this->line_buffer)[col];
        else
          value = ((imbyte*)this->line_buffer)[col];

        if (this->image_type == '1' && value < 2)
          value = 1 - value;

        int write_size = imBinFilePrintf(handle, "%d ", value);
        if (!write_size)
          return IM_ERR_ACCESS;

        line_size += write_size;

        // No line should be longer than 70 characters. 
        if (line_size > 60 || col == line_count-1)
        {
          line_size = 0;
          imBinFileWrite(handle, (void*)"\n", 1, 1);
        }
      }
    }
    else
    {
      if (this->image_type == '4')
        FixBinary();

      imBinFileWrite(handle, this->line_buffer, line_raw_size, 1);
    }

    if (imBinFileError(handle))
      return IM_ERR_ACCESS;     

    if (!imCounterInc(this->counter))
      return IM_ERR_COUNTER;
  }

  return IM_ERR_NONE;
}

int imFormatPNM::CanWrite(const char* compression, int color_mode, int data_type) const
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

  if (!imStrEqual(compression, "NONE") && !imStrEqual(compression, "ASCII"))
    return IM_ERR_COMPRESS;

  return IM_ERR_NONE;
}
