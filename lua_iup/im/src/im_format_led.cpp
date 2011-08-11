/** \file
 * \brief LED - IUP image in LED
 *
 * See Copyright Notice in im_lib.h
 * $Id: im_format_led.cpp,v 1.3 2009/10/01 14:15:47 scuri Exp $
 */

#include "im_format.h"
#include "im_format_all.h"
#include "im_util.h"
#include "im_counter.h"

#include "im_binfile.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>

/* Sample LED Image
LEDImage = IMAGE[
0 = "0 0 0",
1 = "192 192 192",
2 = "0 0 128",
3 = "255 255 255"]
(20, 19
,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
,1,1,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1
,1,1,0,3,3,3,3,0,0,1,1,1,1,1,1,1,1,1,1,1
,1,1,0,3,3,3,3,0,3,0,1,1,1,1,1,1,1,1,1,1
,1,1,0,3,0,0,3,0,2,2,2,2,2,2,1,1,1,1,1,1
,1,1,0,3,3,3,3,3,2,3,3,3,3,2,2,1,1,1,1,1
,1,1,0,3,0,0,0,0,2,3,3,3,3,2,3,2,1,1,1,1
,1,1,0,3,3,3,3,3,2,3,0,0,3,2,2,2,2,1,1,1
,1,1,0,3,0,0,0,0,2,3,3,3,3,3,3,3,2,1,1,1
,1,1,0,3,3,3,3,3,2,3,0,0,0,0,0,3,2,1,1,1
,1,1,0,0,0,0,0,0,2,3,3,3,3,3,3,3,2,1,1,1
,1,1,1,1,1,1,1,1,2,3,0,0,0,0,0,3,2,1,1,1
,1,1,1,1,1,1,1,1,2,3,3,3,3,3,3,3,2,1,1,1
,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,1,1,1
,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
)
*/

static const char* iLEDCompTable[1] = 
{
  "NONE"
};

class imFileFormatLED: public imFileFormatBase
{
  imBinFile* handle;          /* the binary file handle */
  int pal_count;

  int ReadPalette();
  int WritePalette();

public:
  imFileFormatLED(const imFormat* _iformat): imFileFormatBase(_iformat) {}
  ~imFileFormatLED() {}

  int Open(const char* file_name);
  int New(const char* file_name);
  void Close();
  void* Handle(int index);
  int ReadImageInfo(int index);
  int ReadImageData(void* data);
  int WriteImageInfo();
  int WriteImageData(void* data);
};

class imFormatLED: public imFormat
{
public:
  imFormatLED()
    :imFormat("LED", 
              "IUP image in LED special format", 
              "*.led;", 
              iLEDCompTable, 
              1, 
              0)
    {}
  ~imFormatLED() {}

  imFileFormatBase* Create(void) const { return new imFileFormatLED(this); }
  int CanWrite(const char* compression, int color_mode, int data_type) const;
};


void imFormatRegisterLED(void)
{
  imFormatRegister(new imFormatLED());
}

int imFileFormatLED::Open(const char* file_name)
{
  char sig[4];
  unsigned char byte_value;
  int found = 0;

  /* opens the binary file for reading */
  handle = imBinFileOpen(file_name);
  if (!handle)
    return IM_ERR_OPEN;

  this->image_count = 1;
  strcpy(this->compression, "NONE");

  imBinFileRead(handle, sig, 3, 1);
  sig[3] = 0;

  if (imBinFileError(handle))
  {
    imBinFileClose(handle);
    return IM_ERR_ACCESS;
  }

  if (!imStrEqual(sig, "LED"))
  {
    imBinFileClose(handle);
    return IM_ERR_FORMAT;
  }

  unsigned long offset = imBinFileTell(handle);

  /* count the number of colors */
  this->pal_count = -1; // will count the first '=' that is not a color
  while (!found)
  {
    imBinFileRead(handle, &byte_value, 1, 1);

    if (byte_value == '(')
      found = 1;

    if (byte_value == '=')
      this->pal_count++;

    if (imBinFileError(handle))
    {
      imBinFileClose(handle);
      return IM_ERR_ACCESS;
    }
  } 

  imBinFileSeekTo(handle, offset);

  return IM_ERR_NONE;
}

int imFileFormatLED::New(const char* file_name)
{
  /* opens the binary file for writing */
  handle = imBinFileNew(file_name);
  if (!handle)
    return IM_ERR_OPEN;

  imBinFileWrite(handle, (void*)"LEDImage = IMAGE", 16, 1);

  /* tests if everything was ok */
  if (imBinFileError(handle))
  {
    imBinFileClose(handle);
    return IM_ERR_ACCESS;
  }

  return IM_ERR_NONE;
}

void imFileFormatLED::Close()
{
  imBinFileClose(handle);
}

void* imFileFormatLED::Handle(int index)
{
  if (index == 0)
    return (void*)this->handle;
  else
    return NULL;
}

int imFileFormatLED::ReadImageInfo(int index)
{
  (void)index;

  this->palette_count = this->pal_count;

  if (ReadPalette() != IM_ERR_NONE)
    return IM_ERR_ACCESS;

  imBinFileReadInteger(handle, &this->width);
  imBinFileReadInteger(handle, &this->height);
 
  if (imBinFileError(handle))
    return IM_ERR_ACCESS;

  this->file_data_type = IM_BYTE;
  this->file_color_mode = IM_MAP;
  this->file_color_mode |= IM_TOPDOWN;

  return IM_ERR_NONE;
}

int imFileFormatLED::WriteImageInfo()
{
  this->file_data_type = IM_BYTE;
  this->file_color_mode = imColorModeSpace(this->user_color_mode);
  this->file_color_mode |= IM_TOPDOWN;

  if (WritePalette() != IM_ERR_NONE)
    return IM_ERR_ACCESS;

  imBinFilePrintf(handle, "(%d, %d\n", this->width, this->height);

  return IM_ERR_NONE;
}

int imFileFormatLED::ReadPalette()
{
  int c, r, g, b, i;

  /* convert the color map to the IM format */
  for (c = 0; c < this->palette_count; c++)
  {
    imBinFileReadInteger(handle, &i);
    imBinFileReadInteger(handle, &r);
    imBinFileReadInteger(handle, &g);
    imBinFileReadInteger(handle, &b);

    this->palette[i] = imColorEncode((unsigned char)r, (unsigned char)g, (unsigned char)b);

    if (imBinFileError(handle))
      return IM_ERR_ACCESS;
  }

  return IM_ERR_NONE;
}

int imFileFormatLED::WritePalette()
{
  int c;
  unsigned char r, g, b;

  imBinFileWrite(handle, (void*)"[\n", 2, 1);

  /* convert the color map from the IM format */
  for (c = 0; c < this->palette_count; c++)
  {
    imColorDecode(&r, &g, &b, this->palette[c]);
    imBinFilePrintf(handle, "%d = \"%d %d %d\"", c, (int)r, (int)g, (int)b);

    if (c != this->palette_count - 1)
      imBinFileWrite(handle, (void*)",\n", 2, 1);
  }

  imBinFileWrite(handle, (void*)"]\n", 2, 1);

  if (imBinFileError(handle))
    return IM_ERR_ACCESS;

  return IM_ERR_NONE;
}

int imFileFormatLED::ReadImageData(void* data)
{
  int value;

  imCounterTotal(this->counter, this->height, "Reading LED...");

  for (int row = 0; row < this->height; row++)
  {
    for (int col = 0; col < this->width; col++)
    {
      if (!imBinFileReadInteger(handle, &value))
        return IM_ERR_ACCESS;

      ((imbyte*)this->line_buffer)[col] = (unsigned char)value;
    }

    imFileLineBufferRead(this, data, row, 0);

    if (!imCounterInc(this->counter))
      return IM_ERR_COUNTER;
  } 

  return IM_ERR_NONE;
}

int imFileFormatLED::WriteImageData(void* data)
{
  imCounterTotal(this->counter, this->height, "Writing LED...");

  for (int row = 0; row < this->height; row++)
  {
    imFileLineBufferWrite(this, data, row, 0);

    for (int col = 0; col < this->width; col++)
    {
      if (!imBinFilePrintf(handle, ",%d", (int)((imbyte*)this->line_buffer)[col]))
        return IM_ERR_ACCESS;
    }
  
    imBinFileWrite(handle, (void*)"\n", 1, 1);
    if (imBinFileError(handle))
      return IM_ERR_ACCESS;     

    if (!imCounterInc(this->counter))
      return IM_ERR_COUNTER;
  }

  imBinFileWrite(handle, (void*)")", 1, 1);
  if (imBinFileError(handle))
    return IM_ERR_ACCESS;     

  return IM_ERR_NONE;
}

int imFormatLED::CanWrite(const char* compression, int color_mode, int data_type) const
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
