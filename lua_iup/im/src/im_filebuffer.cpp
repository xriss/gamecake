/** \file
 * \brief File Access - Buffer Management
 *
 * See Copyright Notice in im_lib.h
 * $Id: im_filebuffer.cpp,v 1.2 2009/08/13 22:34:25 scuri Exp $
 */

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "im.h"
#include "im_format.h"
#include "im_util.h"
#include "im_complex.h"
#include "im_color.h"


int imFileLineSizeAligned(int width, int bpp, int align)
{
  if (align == 4)
    return ((width * bpp + 31) / 32) * 4;
  else if (align == 2)
    return ((width * bpp + 15) / 16) * 2;
  else
    return (width * bpp + 7) / 8;
}

template <class T> 
static void iDoFillLineBuffer(int width, int height, int line, int plane,  
                              int file_color_mode, T* line_buffer, 
                              int user_color_mode, const T* data)
{
  // (writing) from data to file
  // will handle packing and alpha

  int file_depth = imColorModeDepth(file_color_mode);  
  int data_depth = imColorModeDepth(user_color_mode);
  int data_plane_size = width*height;  // This will be used in UNpacked data

  if (imColorModeIsPacked(user_color_mode))
    data += line*width*data_depth;
  else
    data += line*width;

  for (int x = 0; x < width; x++)
  {
    int x_data_offset = x*data_depth;    // This will be used in packed data

    if (imColorModeIsPacked(file_color_mode))
    {
      int x_file_offset = x*file_depth;  // This will be used in packed data

      // file is packed
      // NO color space conversion, color_space must match
      // Ignore alpha if necessary.
      int depth = IM_MIN(file_depth, data_depth);      
      for (int d = 0; d < depth; d++)
      {
        if (imColorModeIsPacked(user_color_mode))
          line_buffer[x_file_offset + d] = data[x_data_offset + d];
        else
          line_buffer[x_file_offset + d] = data[d*data_plane_size + x];
      }
    }
    else
    {
      // file NOT packed, copy just one plane
      // NO color space conversion, color_space must match

      if (plane >= imColorModeDepth(user_color_mode))
        return;

      if (imColorModeIsPacked(user_color_mode))
        line_buffer[x] = data[x_data_offset + plane];
      else
        line_buffer[x] = data[plane*data_plane_size + x];
    }
  }
}

template <class T> 
static void iDoFillData(int width, int height, int line, int plane,  
                              int file_color_mode, const T* line_buffer, 
                              int user_color_mode, T* data)
{
  // (reading) from file to data
  // will handle packing and alpha

  int file_depth = imColorModeDepth(file_color_mode);
  int data_depth = imColorModeDepth(user_color_mode);
  int data_plane_size = width*height;  // This will be used in UNpacked data

  if (imColorModeIsPacked(user_color_mode))
    data += line*width*data_depth;
  else
    data += line*width;

  for (int x = 0; x < width; x++)
  {
    int x_data_offset = x*data_depth;    // This will be used in packed data

    if (imColorModeIsPacked(file_color_mode))
    {
      int x_file_offset = x*file_depth;  // This will be used in packed data

      // file is packed
      // NO color space conversion, color_space must match
      // ignore alpha if necessary.
      int depth = IM_MIN(file_depth, data_depth);      
      for (int d = 0; d < depth; d++)
      {
        if (imColorModeIsPacked(user_color_mode))
          data[x_data_offset + d] = line_buffer[x_file_offset + d];
        else
          data[d*data_plane_size + x] = line_buffer[x_file_offset + d];
      }
    }
    else
    {
      // file NOT packed, copy just one plane
      // NO color space conversion, color_space must match

      if (plane >= imColorModeDepth(user_color_mode))
        return;

      if (imColorModeIsPacked(user_color_mode))
        data[x_data_offset + plane] = line_buffer[x];
      else
        data[plane*data_plane_size + x] = line_buffer[x];
    }
  }
}

template <class T> 
static inline void iConvertColor2RGB(T* data, int color_space, int data_type)
{
  T zero, max = (T)imColorMax(data_type);

  // These are identical procedures to iDoConvert2RGB in "im_filebuffer.cpp".

  switch (color_space)
  {
  case IM_XYZ: 
    {
      // to increase precision do intermediate conversions in float

      // scale to 0-1
      float c0 = imColorReconstruct(data[0], max);
      float c1 = imColorReconstruct(data[1], max);
      float c2 = imColorReconstruct(data[2], max);

      // result is still 0-1
      imColorXYZ2RGB(c0, c1, c2, 
                     c0, c1, c2, 1.0f);

      // do gamma correction then scale back to 0-max
      data[0] = imColorQuantize(imColorTransfer2Nonlinear(c0), max);
      data[1] = imColorQuantize(imColorTransfer2Nonlinear(c1), max);
      data[2] = imColorQuantize(imColorTransfer2Nonlinear(c2), max);
    }
    break;
  case IM_YCBCR: 
    zero = (T)imColorZero(data_type);
    imColorYCbCr2RGB(data[0], data[1], data[2], 
                      data[0], data[1], data[2], zero, max);
    break;
  case IM_CMYK: 
    imColorCMYK2RGB(data[0], data[1], data[2], data[3], 
                    data[0], data[1], data[2], max);
    break;
  case IM_LUV:
  case IM_LAB:
    {
      // to increase precision do intermediate conversions in float
      // scale to 0-1 and -0.5/+0.5
      float c0 = imColorReconstruct(data[0], max);
      float c1 = imColorReconstruct(data[1], max) - 0.5f;
      float c2 = imColorReconstruct(data[2], max) - 0.5f;

      if (color_space == IM_LUV)
        imColorLuv2XYZ(c0, c1, c2,  // conversion in-place
                       c0, c1, c2);
      else
        imColorLab2XYZ(c0, c1, c2,  // conversion in-place
                       c0, c1, c2);

      imColorXYZ2RGB(c0, c1, c2,    // conversion in-place
                     c0, c1, c2, 1.0f);

      // do gamma correction then scale back to 0-max
      data[0] = imColorQuantize(imColorTransfer2Nonlinear(c0), max);
      data[1] = imColorQuantize(imColorTransfer2Nonlinear(c1), max);
      data[2] = imColorQuantize(imColorTransfer2Nonlinear(c2), max);
    }
    break;
  }
}

// These functions will be always converting RGB -> RGB  (0-max) -> (0-255)

static inline imbyte iConvertType2Byte(const imbyte& data)
  { return data; }

static inline imbyte iConvertType2Byte(const imushort& data)
  { return imColorQuantize(imColorReconstruct(data, (imushort)65535), (imbyte)255); }

static inline imbyte iConvertType2Byte(const int& data)
  { return imColorQuantize(imColorReconstruct(data, 16777215), (imbyte)255); }

static inline imbyte iConvertType2Byte(const float& data)
  { return imColorQuantize(data, (imbyte)255); }

// Fake float to avoid erros in the color conversion template rotines.
// Since the color conversion use the double value, they are invalid,
// so the automatic conversion to bitmap for complex images works only for RGB.
static inline imbyte iConvertType2Byte(const double& data)
{ 
  imcfloat* fdata = (imcfloat*)&data;
  return imColorQuantize(cpxmag(*fdata), (imbyte)255); 
}

template <class T> 
static void iDoFillDataBitmap(int width, int height, int line, int plane, int data_type,
                              int file_color_mode, const T* line_buffer, 
                              int user_color_mode, imbyte* data)
{
  // (reading) from file to data
  // will handle packing, alpha, color space conversion to RGB and data_type to BYTE

  int file_depth = imColorModeDepth(file_color_mode);
  int data_depth = imColorModeDepth(user_color_mode);
  int copy_alpha = imColorModeHasAlpha(file_color_mode) && imColorModeHasAlpha(user_color_mode);
  int data_plane_size = width*height;  // This will be used in UNpacked data

  if (imColorModeIsPacked(user_color_mode))
    data += line*width*data_depth;
  else
    data += line*width;

  for (int x = 0; x < width; x++)
  {
    int x_data_offset = x*data_depth;    // This will be used in packed data

    if (imColorModeIsPacked(file_color_mode))
    {
      int x_file_offset = x*file_depth;  // This will be used in packed data

      if (imColorModeMatch(file_color_mode, user_color_mode))
      {
        // file is packed
        // same color space components   (in this case means RGB)
        // ignore alpha if necessary.
        int depth = IM_MIN(file_depth, data_depth);      
        for (int d = 0; d < depth; d++)
        {
          if (imColorModeIsPacked(user_color_mode))
            data[x_data_offset + d] = iConvertType2Byte(line_buffer[x_file_offset + d]);
          else
            data[d*data_plane_size + x] = iConvertType2Byte(line_buffer[x_file_offset + d]);
        }
      }
      else
      {
        // file is packed
        // but different color space components
        // only to RGB conversions are accepted

        if (imColorModeSpace(user_color_mode) != IM_RGB)
          return;

        T src_data[4];
        src_data[0] = line_buffer[x_file_offset];
        src_data[1] = line_buffer[x_file_offset + 1];
        src_data[2] = line_buffer[x_file_offset + 2];
        if (imColorModeSpace(file_color_mode) == IM_CMYK)
          src_data[3] = line_buffer[x_file_offset + 3];

        // Do conversion in place
        iConvertColor2RGB(src_data, imColorModeSpace(file_color_mode), data_type);

        if (imColorModeIsPacked(user_color_mode))
        {
          data[x_data_offset] = iConvertType2Byte(src_data[0]);
          data[x_data_offset + 1] = iConvertType2Byte(src_data[1]);
          data[x_data_offset + 2] = iConvertType2Byte(src_data[2]);

          if (copy_alpha)
          {
            if (imColorModeSpace(file_color_mode) == IM_CMYK)
              data[x_data_offset + 3] = iConvertType2Byte(line_buffer[x_file_offset + 4]);
            else
              data[x_data_offset + 3] = iConvertType2Byte(line_buffer[x_file_offset + 3]);
          }
        }
        else
        {
          data[x] = iConvertType2Byte(src_data[0]);
          data[data_plane_size + x] = iConvertType2Byte(src_data[1]);
          data[2*data_plane_size + x] = iConvertType2Byte(src_data[2]);

          if (copy_alpha)
          {
            if (imColorModeSpace(file_color_mode) == IM_CMYK)
              data[3*data_plane_size + x] = iConvertType2Byte(line_buffer[x_file_offset + 4]);
            else
              data[3*data_plane_size + x] = iConvertType2Byte(line_buffer[x_file_offset + 3]);
          }
        }
      }
    }
    else
    {
      // file NOT packed, copy just one plane
      // NO color space conversion possible now

      if (plane >= imColorModeDepth(user_color_mode))
        return;

      if (imColorModeIsPacked(user_color_mode))
        data[x_data_offset + plane] = iConvertType2Byte(line_buffer[x]);
      else
        data[plane*data_plane_size + x] = iConvertType2Byte(line_buffer[x]);
    }
  }
}

static void iFileExpandBits(imFile* ifile)
{
  // conversion will be done in place in backward order (from end to start)

  if (abs(ifile->convert_bpp) < 8)
  {
    imbyte* byte_buffer = (imbyte*)ifile->line_buffer;
    imbyte* bit_buffer = (imbyte*)ifile->line_buffer;

    byte_buffer += ifile->width-1; 
    int bpp = ifile->convert_bpp;
    int expand_range = imColorModeSpace(ifile->file_color_mode) == IM_GRAY? 1: 0;

    for (int i=ifile->width-1; i >= 0; i--)
    {
      if (bpp == 1)
        *byte_buffer = (imbyte)((bit_buffer[i / 8] >> (7 - i % 8)) & 0x01);
      else if (bpp == 4)
        *byte_buffer = (imbyte)((bit_buffer[i / 2] >> ((1 - i % 2) * 4)) & 0x0F);
      else if (bpp == 2)
        *byte_buffer = (imbyte)((bit_buffer[i / 4] >> ((3 - i % 4) * 2)) & 0x03);

      if (expand_range)   /* if convert_bpp<0 then only expand its range */
      {
        if (bpp == 4 || bpp == -4)
          *byte_buffer *= 17;
        else if (bpp == 2 || bpp == -2)
          *byte_buffer *= 85;
      }

      byte_buffer--;
    }
  }
  else if (ifile->convert_bpp == 12)
  {
    imushort* ushort_buffer = (imushort*)ifile->line_buffer;
    imbyte* bit_buffer = (imbyte*)ifile->line_buffer;

    for (int i=ifile->width-1; i >= 0; i--)
    {
      int byte_index = (3*i)/2;
      if (i%2)
        ushort_buffer[i] = (bit_buffer[byte_index] << 4) | (bit_buffer[byte_index+1] & 0x0F);
      else
        ushort_buffer[i] = ((bit_buffer[byte_index] & 0x0F) << 8) | (bit_buffer[byte_index+1]);
    }
  }
}

static void iFileCompactBits(imFile* ifile)
{
  // conversion will be done in place
  imbyte* byte_buffer = (imbyte*)ifile->line_buffer;
  imbyte* bit_buffer = (imbyte*)ifile->line_buffer;

  if (ifile->convert_bpp == 1)
  {
    for (int i = 0; i < ifile->width; i++)
    {
      if (*byte_buffer)
        bit_buffer[i / 8] |=  (0x01 << (7 - (i % 8)));
      else
        bit_buffer[i / 8] &= ~(0x01 << (7 - (i % 8)));

      byte_buffer++;
    }
  }
  else  // -1 == expand 1 to 255
  {
    for (int i = 0; i < ifile->width; i++)
    {
      if (*byte_buffer)
        *byte_buffer = 255;

      byte_buffer++;
    }
  }
}

template <class SRC, class DST> 
static void iDoSwitchInt(int count, const SRC* src_data, DST* dst_data, int offset)
{
  for (int i = 0; i < count; i++)
  {
    *dst_data++ = (DST)((int)*src_data++ + offset);
  }
}

template <class SRC, class DST> 
static void iDoSwitchReal(int count, const SRC* src_data, DST* dst_data)
{
  for (int i = 0; i < count; i++)
  {
    *dst_data++ = (DST)(*src_data++);
  }
}

static void iFileSwitchFromType(imFile* ifile)
{
  int line_count = imImageLineCount(ifile->width, ifile->file_color_mode);
  switch(ifile->file_data_type)
  {
  case IM_BYTE:    // Source is char
    iDoSwitchInt(line_count, (const char*)ifile->line_buffer, (imbyte*)ifile->line_buffer, 128);
    break;
  case IM_USHORT:  // Source is short
    iDoSwitchInt(line_count, (const short*)ifile->line_buffer, (imushort*)ifile->line_buffer, 32768);
    break;
  case IM_INT:     // Source is uint
    iDoSwitchInt(line_count, (const unsigned int*)ifile->line_buffer, (int*)ifile->line_buffer, -8388608);
    break;
  case IM_FLOAT:   // Source is double
    iDoSwitchReal(line_count, (const double*)ifile->line_buffer, (float*)ifile->line_buffer);
    break;
  case IM_CFLOAT:  // Source is complex double
    iDoSwitchReal(2*line_count, (const double*)ifile->line_buffer, (float*)ifile->line_buffer);
    break;
  }
}

static void iFileSwitchToType(imFile* ifile)
{
  int line_count = imImageLineCount(ifile->width, ifile->file_color_mode);
  switch(ifile->file_data_type)
  {
  case IM_BYTE:    // Destiny is char
    iDoSwitchInt(line_count, (const imbyte*)ifile->line_buffer, (char*)ifile->line_buffer, -128);
    break;
  case IM_USHORT:  // Destiny is short
    iDoSwitchInt(line_count, (const imushort*)ifile->line_buffer, (short*)ifile->line_buffer, -32768);
    break;
  case IM_INT:     // Destiny is uint
    iDoSwitchInt(line_count, (const int*)ifile->line_buffer, (unsigned int*)ifile->line_buffer, 8388608);
    break;
  case IM_FLOAT:   // Destiny is double
    iDoSwitchReal(line_count, (const float*)ifile->line_buffer, (double*)ifile->line_buffer);
    break;
  case IM_CFLOAT:  // Destiny is complex double
    iDoSwitchReal(2*line_count, (const float*)ifile->line_buffer, (double*)ifile->line_buffer);
    break;
  }
}

void imFileLineBufferWrite(imFile* ifile, const void* data, int line, int plane)
{
  // (writing) from data to file

  if (imColorModeIsTopDown(ifile->file_color_mode) != imColorModeIsTopDown(ifile->user_color_mode))
    line = ifile->height-1 - line;

  if ((ifile->file_color_mode & 0x3FF) == 
      (ifile->user_color_mode & 0x3FF)) // compare only packing, alpha and color space
  {
    int data_offset = line*ifile->line_buffer_size;
    if (plane != 0)
      data_offset += plane*ifile->height*ifile->line_buffer_size;

    memcpy(ifile->line_buffer, (unsigned char*)data + data_offset, ifile->line_buffer_size);
  }
  else
  {
    switch(ifile->file_data_type)
    {
    case IM_BYTE:
      iDoFillLineBuffer(ifile->width, ifile->height, line, plane, 
                        ifile->file_color_mode, (imbyte*)ifile->line_buffer, 
                        ifile->user_color_mode, (const imbyte*)data);
      break;
    case IM_USHORT:
      iDoFillLineBuffer(ifile->width, ifile->height, line, plane,  
                        ifile->file_color_mode, (imushort*)ifile->line_buffer, 
                        ifile->user_color_mode, (const imushort*)data);
      break;
    case IM_INT:
      iDoFillLineBuffer(ifile->width, ifile->height, line, plane,  
                        ifile->file_color_mode, (int*)ifile->line_buffer, 
                        ifile->user_color_mode, (const int*)data);
      break;
    case IM_FLOAT:
      iDoFillLineBuffer(ifile->width, ifile->height, line, plane,  
                        ifile->file_color_mode, (float*)ifile->line_buffer, 
                        ifile->user_color_mode, (const float*)data);
      break;
    case IM_CFLOAT:
      iDoFillLineBuffer(ifile->width, ifile->height, line, plane,  
                        ifile->file_color_mode, (imcfloat*)ifile->line_buffer, 
                        ifile->user_color_mode, (const imcfloat*)data);
      break;
    }
  }

  if (ifile->convert_bpp)
    iFileCompactBits(ifile);

  if (ifile->switch_type)
    iFileSwitchToType(ifile);
}

void imFileLineBufferRead(imFile* ifile, void* data, int line, int plane)
{
  // (reading) from file to data

  if (imColorModeIsTopDown(ifile->file_color_mode) != imColorModeIsTopDown(ifile->user_color_mode))
    line = ifile->height-1 - line;

  if (ifile->convert_bpp)
    iFileExpandBits(ifile);

  if (ifile->switch_type)
    iFileSwitchFromType(ifile);

  if ((ifile->file_color_mode & 0x3FF) == (ifile->user_color_mode & 0x3FF) && // compare only packing, alpha and color space, ignore bottom up.
      ifile->file_data_type == ifile->user_data_type) // compare data type when reading
  {
    int data_offset = line*ifile->line_buffer_size;
    if (plane != 0)
      data_offset += plane*ifile->height*ifile->line_buffer_size;

    memcpy((unsigned char*)data + data_offset, ifile->line_buffer, ifile->line_buffer_size);
  }
  else
  {
    // now we have 2 conversions groups
    // one to convert only packing and alpha
    // and the other to convert packing, alpha, color space and data type
    int convert2bitmap = 0;
    if (imColorModeSpace(ifile->user_color_mode) != imColorModeSpace(ifile->file_color_mode) ||
        ifile->file_data_type != IM_BYTE)
      convert2bitmap = 1;

    switch(ifile->file_data_type)
    {
    case IM_BYTE:
      if (convert2bitmap)
        iDoFillDataBitmap(ifile->width, ifile->height, line, plane, ifile->file_data_type,
                          ifile->file_color_mode, (const imbyte*)ifile->line_buffer, 
                          ifile->user_color_mode, (imbyte*)data);
      else
        iDoFillData(ifile->width, ifile->height, line, plane, 
                    ifile->file_color_mode, (const imbyte*)ifile->line_buffer, 
                    ifile->user_color_mode, (imbyte*)data);
      break;
    case IM_USHORT:
      if (convert2bitmap)
        iDoFillDataBitmap(ifile->width, ifile->height, line, plane, ifile->file_data_type,
                          ifile->file_color_mode, (const imushort*)ifile->line_buffer, 
                          ifile->user_color_mode, (imbyte*)data);
      else
        iDoFillData(ifile->width, ifile->height, line, plane,  
                    ifile->file_color_mode, (const imushort*)ifile->line_buffer, 
                    ifile->user_color_mode, (imushort*)data);
      break;
    case IM_INT:
      if (convert2bitmap)
        iDoFillDataBitmap(ifile->width, ifile->height, line, plane, ifile->file_data_type,
                          ifile->file_color_mode, (const int*)ifile->line_buffer, 
                          ifile->user_color_mode, (imbyte*)data);
      else
        iDoFillData(ifile->width, ifile->height, line, plane,  
                    ifile->file_color_mode, (const int*)ifile->line_buffer, 
                    ifile->user_color_mode, (int*)data);
      break;
    case IM_FLOAT:
      if (convert2bitmap)
        iDoFillDataBitmap(ifile->width, ifile->height, line, plane, ifile->file_data_type,
                          ifile->file_color_mode, (const float*)ifile->line_buffer, 
                          ifile->user_color_mode, (imbyte*)data);
      else
        iDoFillData(ifile->width, ifile->height, line, plane,  
                    ifile->file_color_mode, (const float*)ifile->line_buffer, 
                    ifile->user_color_mode, (float*)data);
      break;
    case IM_CFLOAT:
      if (convert2bitmap)
        iDoFillDataBitmap(ifile->width, ifile->height, line, plane, ifile->file_data_type,
                          ifile->file_color_mode, (const double*)ifile->line_buffer, 
                          ifile->user_color_mode, (imbyte*)data);
      else
        iDoFillData(ifile->width, ifile->height, line, plane,  
                    ifile->file_color_mode, (const imcfloat*)ifile->line_buffer, 
                    ifile->user_color_mode, (imcfloat*)data);
      break;
    }
  }
}
           
void imFileLineBufferInit(imFile* ifile)
{
  ifile->line_buffer_size = imImageLineSize(ifile->width, ifile->file_color_mode, ifile->file_data_type);

  if (ifile->switch_type && (ifile->file_data_type == IM_FLOAT || ifile->file_data_type == IM_CFLOAT))
    ifile->line_buffer_extra += ifile->line_buffer_size; // double the size at least

  if (ifile->line_buffer_size + ifile->line_buffer_extra > ifile->line_buffer_alloc)
  {
    ifile->line_buffer_alloc = ifile->line_buffer_size + ifile->line_buffer_extra;
    ifile->line_buffer = realloc(ifile->line_buffer, ifile->line_buffer_alloc);
  }
}

int imFileLineBufferCount(imFile* ifile)
{
  int count = ifile->height;
  if (!imColorModeIsPacked(ifile->file_color_mode))
  {
    if (imColorModeHasAlpha(ifile->file_color_mode) && imColorModeHasAlpha(ifile->user_color_mode))
      count *= imColorModeDepth(ifile->file_color_mode);
    else
      count *= imColorModeDepth(imColorModeSpace(ifile->file_color_mode));
  }
  return count;
}

void imFileLineBufferInc(imFile* ifile, int *row, int *plane)
{
  if (!imColorModeIsPacked(ifile->file_color_mode))
  {
    if (*row == ifile->height-1)
    {
      *row = 0;
      (*plane)++;
      return;
    }
  }

  (*row)++;
}

int imFileCheckConversion(imFile* ifile)
{
  if ((ifile->file_color_mode & 0x3FF) == (ifile->user_color_mode & 0x3FF) && // compare only packing, alpha and color space
      ifile->file_data_type == ifile->user_data_type)
    return 1;

  int user_color_space = imColorModeSpace(ifile->user_color_mode);
  int file_color_space = imColorModeSpace(ifile->file_color_mode);

  // NO color space conversion if file is not packed.
  if(user_color_space != file_color_space &&
     imColorModeDepth(file_color_space) > 1 &&
     !imColorModeIsPacked(ifile->file_color_mode))
    return 0;

  if (ifile->is_new)
  {
    // (writing) from data to file

    // NO data type conversions when writing.
    if (ifile->file_data_type != ifile->user_data_type)
      return 0;

    // NO color space conversions when writing. 
    // If there is a necessary conversion the format driver will do it.
    if (user_color_space != file_color_space)
      return 0;
   }
  else
  {
    // (reading) from file to data

    // Data type conversions only to byte
    if (ifile->file_data_type != ifile->user_data_type &&
        ifile->user_data_type != IM_BYTE)
      return 0;
  }

  return 1;
}
