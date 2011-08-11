/** \file
 * \brief PNG - Portable Network Graphic Format
 *
 * See Copyright Notice in im_lib.h
 * See libPNG Copyright Notice in png.h
 * $Id: im_format_png.cpp,v 1.4 2009/08/19 18:39:43 scuri Exp $
 */

#include "im_format.h"
#include "im_format_all.h"
#include "im_util.h"
#include "im_counter.h"

#include "im_binfile.h"

#include <stdlib.h>
#include <string.h>

#include "png.h"

static void png_user_read_fn(png_structp png_ptr, png_bytep buffer, png_size_t size)
{
  imBinFileRead((imBinFile*)png_ptr->io_ptr, buffer, size, 1);
  if (imBinFileError((imBinFile*)png_ptr->io_ptr))
    png_error(png_ptr, "Read Error");
}

static void png_user_write_fn(png_structp png_ptr, png_bytep buffer, png_size_t size)
{
  imBinFileWrite((imBinFile*)png_ptr->io_ptr, buffer, size, 1);
  if (imBinFileError((imBinFile*)png_ptr->io_ptr))
    png_error(png_ptr, "Write Error");
}

static void png_user_flush_fn(png_structp png_ptr)
{
  (void)png_ptr;
}

static const char* iPNGCompTable[1] = 
{
  "DEFLATE"
};

class imFileFormatPNG: public imFileFormatBase
{
  png_structp png_ptr;
  png_infop info_ptr;

  imBinFile* handle;
  int interlace_steps, fixbits;

  void iReadAttrib(imAttribTable* attrib_table);
  void iWriteAttrib(imAttribTable* attrib_table);

public:
  imFileFormatPNG(const imFormat* _iformat): imFileFormatBase(_iformat) {}
  ~imFileFormatPNG() {}

  int Open(const char* file_name);
  int New(const char* file_name);
  void Close();
  void* Handle(int index);
  int ReadImageInfo(int index);
  int ReadImageData(void* data);
  int WriteImageInfo();
  int WriteImageData(void* data);
};

class imFormatPNG: public imFormat
{
public:
  imFormatPNG()
    :imFormat("PNG", 
              "Portable Network Graphic Format", 
              "*.png;", 
              iPNGCompTable, 
              1, 
              0)
    {}
  ~imFormatPNG() {}

  imFileFormatBase* Create(void) const { return new imFileFormatPNG(this); }
  int CanWrite(const char* compression, int color_mode, int data_type) const;
};

void imFormatRegisterPNG(void)
{
  imFormatRegister(new imFormatPNG());
}

int imFileFormatPNG::Open(const char* file_name)
{
  this->handle = imBinFileOpen(file_name);
  if (this->handle == NULL)
    return IM_ERR_OPEN;

  unsigned char sig[8];
  if (!imBinFileRead(this->handle, sig, 8, 1))
  {
    imBinFileClose(this->handle);
    return IM_ERR_ACCESS;
  }

  if (png_sig_cmp(sig, 0, 8) != 0)
  {
    imBinFileClose(this->handle);
    return IM_ERR_FORMAT;
  }

  imBinFileSeekTo(this->handle, 0);

  strcpy(this->compression, "DEFLATE");
  this->image_count = 1;

  /* Create and initialize the png_struct with the default error handler functions. */
  this->png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, (png_error_ptr)NULL, (png_error_ptr)NULL);
  if (!this->png_ptr)
  {
    imBinFileClose(this->handle);
    return IM_ERR_FORMAT;
  }

  return IM_ERR_NONE;
}

int imFileFormatPNG::New(const char* file_name)
{
  this->handle = imBinFileNew(file_name);
  if (this->handle == NULL)
    return IM_ERR_OPEN;

  this->png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (this->png_ptr == NULL)
  {
    imBinFileClose(this->handle);
    return IM_ERR_ACCESS;
  }

  strcpy(this->compression, "DEFLATE");
  this->image_count = 1;
  
  return IM_ERR_NONE;
}

void imFileFormatPNG::Close()
{
  if (this->is_new)
    png_destroy_write_struct(&this->png_ptr,  &this->info_ptr);
  else
    png_destroy_read_struct(&this->png_ptr, &this->info_ptr, (png_infopp)NULL);

  imBinFileClose(this->handle);
}

void* imFileFormatPNG::Handle(int index)
{
  if (index == 0)
    return (void*)this->handle;
  else if (index == 1)
    return (void*)this->png_ptr;
  else
    return 0;
}

void imFileFormatPNG::iReadAttrib(imAttribTable* attrib_table)
{
  double gamma;
  if (png_get_gAMA(png_ptr, info_ptr, &gamma))
  {
    float fvalue = (float)gamma;
    attrib_table->Set("Gamma", IM_FLOAT, 1, &fvalue);
  }

  png_uint_32 xr, yr;
  int res_unit_type = PNG_RESOLUTION_UNKNOWN;
  if (png_get_pHYs(png_ptr, info_ptr, &xr, &yr, &res_unit_type))
  {
    if (res_unit_type == PNG_RESOLUTION_METER)
    {
      float xres = xr / 100.0f;
      float yres = yr / 100.0f;
      attrib_table->Set("XResolution", IM_FLOAT, 1, &xres);
      attrib_table->Set("YResolution", IM_FLOAT, 1, &yres);
      attrib_table->Set("ResolutionUnit", IM_BYTE, -1, "DPC");
    }
  }

  png_int_32 x, y;
  int unit_type;
  if (png_get_oFFs(png_ptr, info_ptr, &x, &y, &unit_type))
  {
    float xpos, ypos;

    if (res_unit_type == PNG_RESOLUTION_UNKNOWN)
    {
      if (unit_type == PNG_OFFSET_PIXEL)
      {
        xpos = (float)x;
        ypos = (float)y;
      }
      else
      {
        xpos = 0;  // can not calculate position
        ypos = 0;
      }
    }
    else 
    {
      if (unit_type == PNG_OFFSET_PIXEL)
      {
        // pixels to centimeters
        xpos = ((float)x / (float)xr) * 100.0f;
        ypos = ((float)y / (float)yr) * 100.0f;
      }
      else
      {
        // micrometers to centimeters
        xpos = (float)x / 100.0f;
        ypos = (float)y / 100.0f;
      }
    }

    if (xpos && ypos)
    {
      // Position is in ResolutionUnits
      attrib_table->Set("YPosition", IM_FLOAT, 1, &ypos);
      attrib_table->Set("XPosition", IM_FLOAT, 1, &xpos);
    }
  }
  
  int intent;
  if (png_get_sRGB(png_ptr, info_ptr, &intent))
  {
    if (intent)
      attrib_table->Set("sRGBIntent", IM_INT, 1, &intent);
  }

  double chroma[8];
  if (png_get_cHRM(png_ptr,info_ptr, &chroma[0], &chroma[1], &chroma[2], &chroma[3], &chroma[4], &chroma[5], &chroma[6], &chroma[7]))
  {
    float white[2] = {(float)chroma[0], (float)chroma[1]};
    float primchroma[6] = {(float)chroma[2], (float)chroma[3], 
                           (float)chroma[4], (float)chroma[5], 
                           (float)chroma[6], (float)chroma[7]};
    attrib_table->Set("WhitePoint", IM_FLOAT, 2, white);
    attrib_table->Set("PrimaryChromaticities", IM_FLOAT, 6, primchroma);
  }

  png_charp pcal_purpose;  
  int pcal_type, pcal_nparams;
  png_int_32 pcal_limits[2];
  png_charp pcal_units;    
  png_charpp pcal_params;  
  if (png_get_pCAL(png_ptr, info_ptr, &pcal_purpose, &pcal_limits[0], &pcal_limits[1], &pcal_type, &pcal_nparams, &pcal_units, &pcal_params))
  {
    char param_buf[255*100], *param_ptr;
    int p, size, total_size = 0;

    attrib_table->Set("CalibrationName", IM_BYTE, -1, pcal_purpose);
    attrib_table->Set("CalibrationLimits", IM_INT, 2, pcal_limits);
    attrib_table->Set("CalibrationUnits", IM_BYTE, -1, pcal_units);
    attrib_table->Set("CalibrationEquation", IM_BYTE, 1, &pcal_type);

    param_ptr = &param_buf[0];
    for (p = 0; p < pcal_nparams; p++)
    {
      size = strlen(pcal_params[p]);
      memcpy(param_ptr, pcal_params[p], size);
      param_ptr += size;
      *param_ptr = '\n';
      param_ptr++;
      total_size += size+1;
    }
    *param_ptr = '0';

    attrib_table->Set("CalibrationParam", IM_BYTE, total_size+1, param_buf);
  }

  int num_trans = 0;
  png_bytep trans = NULL;
  png_color_16p trans_values = NULL;
  if (png_get_tRNS(png_ptr, info_ptr, &trans, &num_trans, &trans_values))
  {
    if (imColorModeSpace(file_color_mode) == IM_MAP)
    {
      int i, min_alpha = 256;
      imbyte transp_index = 0;
      attrib_table->Set("TransparencyMap", IM_BYTE, num_trans, trans);
      for (i=0; i<num_trans; i++)
      {
        if (trans[i] < min_alpha)
        {
          min_alpha = trans[i];
          transp_index = (imbyte)i;
        }
      }
      attrib_table->Set("TransparencyIndex", IM_BYTE, 1, &transp_index);
    }
    else if (imColorModeSpace(file_color_mode) == IM_RGB)
    {                              
      imbyte transp_color[3];
      transp_color[0] = (imbyte)(trans_values->red >> 8);
      transp_color[1] = (imbyte)(trans_values->green >> 8);
      transp_color[2] = (imbyte)(trans_values->blue >> 8);
      attrib_table->Set("TransparencyColor", IM_BYTE, 3, transp_color);
    }
    else if (imColorModeSpace(file_color_mode) == IM_GRAY)
    {
      imbyte transp_index = (imbyte)(trans_values->gray >> 8);
      attrib_table->Set("TransparencyIndex", IM_BYTE, 1, &transp_index);
    }
  }

  int num_text;
  png_textp text_ptr;
  if (png_get_text(png_ptr, info_ptr, &text_ptr, &num_text))
  {
    int t;
    for (t = 0; t < num_text; t++)
    {
      png_textp png_text = &text_ptr[t];
      if (png_text->text_length)
      {
        if (imStrEqual(png_text->key, "Creation Time"))
          attrib_table->Set("DateTime", IM_BYTE, png_text->text_length+1, png_text->text);
        else
          attrib_table->Set(png_text->key, IM_BYTE, png_text->text_length+1, png_text->text);
      }
    }
  }

  png_timep time;
  if (png_get_tIME(png_ptr, info_ptr, &time))
  {
    char* stime = png_convert_to_rfc1123(png_ptr, time);
    attrib_table->Set("DateTimeModified", IM_BYTE, -1, stime);
  }

  png_charp name;
  int compression_type;
  png_charp profile;
  png_uint_32 proflen;
  if (png_get_iCCP(png_ptr, info_ptr, &name, &compression_type, &profile, &proflen))
    attrib_table->Set("ICCProfile", IM_BYTE, proflen, profile);

  int scale_unit;
  double scale_width, scale_height;
  if (png_get_sCAL(png_ptr, info_ptr, &scale_unit, &scale_width, &scale_height))
  {
    if (scale_unit == PNG_SCALE_METER || scale_unit == PNG_SCALE_RADIAN)
    {
      float xscale = (float)scale_width;
      float yscale = (float)scale_height;
      attrib_table->Set("XScale", IM_FLOAT, 1, &xscale);
      attrib_table->Set("YScale", IM_FLOAT, 1, &yscale);
      if (scale_unit == PNG_SCALE_METER)
        attrib_table->Set("ScaleUnit", IM_BYTE, -1, "meters");
      else
        attrib_table->Set("ScaleUnit", IM_BYTE, -1, "radians");
    }
  }
}

static int iAttribStringCount = 0;

static int iFindAttribString(void* user_data, int index, const char* name, int data_type, int count, const void* data)
{
  png_textp text_ptr = (png_textp)user_data;
  (void)index;

  if (data_type == IM_BYTE && count > 3 && ((imbyte*)data)[count-1] == 0)
  {                                                   
    if (imStrEqual(name, "ResolutionUnit") ||
        imStrEqual(name, "InkNames") ||
        imStrEqual(name, "CalibrationUnits") ||
        imStrEqual(name, "CalibrationName") ||
        imStrEqual(name, "CalibrationParam") ||
        imStrEqual(name, "ICCProfile") ||
        imStrEqual(name, "ScaleUnit"))
      return 1;
    
    png_textp png_text = &text_ptr[iAttribStringCount];

    png_text->key = (char*)name;
    png_text->text = (char*)data;
    png_text->text_length = count-1;

    if (count < 1000)
      png_text->compression = PNG_TEXT_COMPRESSION_NONE;
    else
      png_text->compression = PNG_TEXT_COMPRESSION_zTXt;

    iAttribStringCount++;
  }

  return 1;
}

void imFileFormatPNG::iWriteAttrib(imAttribTable* attrib_table)
{
  const void* attrib_data = attrib_table->Get("Gamma");
  if (attrib_data)
    png_set_gAMA(png_ptr, info_ptr, *(float*)attrib_data);

  int offset_res = PNG_OFFSET_PIXEL;
  attrib_data = attrib_table->Get("ResolutionUnit");
  if (attrib_data)
  {
    char* res_unit = (char*)attrib_data;

    float* xres = (float*)attrib_table->Get("XResolution");
    float* yres = (float*)attrib_table->Get("YResolution");

    if (xres && yres)
    {
      png_uint_32 ixres, iyres;

      if (imStrEqual(res_unit, "DPI"))
      {
        ixres = (png_uint_32)(*xres * 100. / 2.54);
        iyres = (png_uint_32)(*yres * 100. / 2.54);
        offset_res = -1;
      }
      else
      {
        ixres = (png_uint_32)(*xres * 100.);
        iyres = (png_uint_32)(*yres * 100.);
        offset_res = PNG_OFFSET_MICROMETER;
      }

      png_set_pHYs(png_ptr, info_ptr, ixres, iyres, PNG_RESOLUTION_METER);
    }
  }

  attrib_data = attrib_table->Get("XPosition");
  if (attrib_data)
  {
    float xpos = *(float*)attrib_data;

    attrib_data = attrib_table->Get("YPosition");
    if (attrib_data)
    {
      float ypos = *(float*)attrib_data;

      if (offset_res == -1)
      {
        // inches to micrometer
        offset_res = PNG_OFFSET_MICROMETER;
        xpos *= 25400.0f;
        ypos *= 25400.0f;
      }
      else if (offset_res == PNG_OFFSET_MICROMETER)
      {
        // centimeter to micrometer
        xpos *= 100.0f;
        ypos *= 100.0f;
      }

      png_set_oFFs(png_ptr, info_ptr, (png_int_32)xpos, (png_int_32)ypos, offset_res);
    }
  }

  attrib_data = attrib_table->Get("sRGBIntent");
  if (attrib_data)
    png_set_sRGB(png_ptr, info_ptr, *(int*)attrib_data);

  attrib_data = attrib_table->Get("PrimaryChromaticities");
  if (attrib_data)
  {
    float *primchroma = (float*)attrib_data;

    attrib_data = attrib_table->Get("WhitePoint");
    if (attrib_data)
    {
      float* white = (float*)attrib_data;

      png_set_cHRM(png_ptr,info_ptr, white[0], white[1], 
                                     primchroma[0], primchroma[1], primchroma[2], 
                                     primchroma[3], primchroma[4], primchroma[5]);
    }
  }

  attrib_data = attrib_table->Get("CalibrationName");
  if (attrib_data)
  {
    char params[255][100], *pparams[255], *new_param_ptr;
    int nparams = 0, size;

    char* name = (char*)attrib_data;
    int* limits = (int*)attrib_table->Get("CalibrationLimits");
    char* units = (char*)attrib_table->Get("CalibrationUnits");
    char* equation = (char*)attrib_table->Get("CalibrationEquation");
    char* param_ptr = (char*)attrib_table->Get("CalibrationParam");

    do
    {
      new_param_ptr = (char*)strstr(param_ptr, "\n");
      if (new_param_ptr)
      {
        size = new_param_ptr - param_ptr;
        memcpy(params[nparams], param_ptr, size);
        params[nparams][size] = 0;
        param_ptr = new_param_ptr+1;
        pparams[nparams] = params[nparams];
        nparams++;
      }
    } while (new_param_ptr && *param_ptr != 0);

    png_set_pCAL(png_ptr, info_ptr, name, limits[0], limits[1], *equation, nparams, units, pparams);
  }

  attrib_data = attrib_table->Get("TransparencyIndex", NULL, NULL);
  if (attrib_data)
  {
    if (imColorModeSpace(file_color_mode) == IM_MAP)
    {
      int i;
      imbyte transp_index = *(imbyte*)attrib_data;
      imbyte transp_map[256];
      for (i=0; i<256; i++)
        transp_map[i] = 255;
      transp_map[transp_index] = 0;
      png_set_tRNS(png_ptr, info_ptr, transp_map, 256, NULL);
    }
    else if (imColorModeSpace(file_color_mode) == IM_GRAY)
    {
      png_color_16 trans_values;
      imbyte *transp_color = (imbyte*)attrib_data;
      trans_values.gray = (png_uint_16)(transp_color[0] << 8);
      png_set_tRNS(png_ptr, info_ptr, NULL, 1, &trans_values);
    }
  }

  int transp_count;
  attrib_data = attrib_table->Get("TransparencyMap", NULL, &transp_count);
  if (attrib_data)
  {
    if (imColorModeSpace(file_color_mode) == IM_MAP)
      png_set_tRNS(png_ptr, info_ptr, (imbyte*)attrib_data, transp_count, NULL);
  }

  attrib_data = attrib_table->Get("TransparencyColor");
  if (attrib_data)
  {
    if (imColorModeSpace(file_color_mode) == IM_RGB)
    {
      png_color_16 trans_values;
      imbyte *transp_color = (imbyte*)attrib_data;
      trans_values.red = (png_uint_16)(transp_color[0] << 8);
      trans_values.green = (png_uint_16)(transp_color[1] << 8);
      trans_values.blue = (png_uint_16)(transp_color[2] << 8);
      png_set_tRNS(png_ptr, info_ptr, NULL, 1, &trans_values);
    }
  }
  
  iAttribStringCount = 0;
  png_text text_ptr[512];
  attrib_table->ForEach(text_ptr, iFindAttribString);
  if (iAttribStringCount)
    png_set_text(png_ptr, info_ptr, text_ptr, iAttribStringCount);

  attrib_data = attrib_table->Get("DateTimeModified");
  if (attrib_data)
  {
    png_time ptime;
    time_t cur_time;
    time(&cur_time);
    png_convert_from_time_t(&ptime, cur_time);
    png_set_tIME(png_ptr, info_ptr, &ptime);
  }

  int proflen;
  attrib_data = attrib_table->Get("ICCProfile", NULL, &proflen);
  if (attrib_data)
  {
    png_charp profile = (png_charp)attrib_data;
    png_set_iCCP(png_ptr, info_ptr, "ICC Profile", 0, profile, proflen);
  }

  attrib_data = attrib_table->Get("ScaleUnit");
  if (attrib_data)
  {
    char* scale_unit = (char*)attrib_data;

    float* xscale = (float*)attrib_table->Get("XScale");
    float* yscale = (float*)attrib_table->Get("YScale");

    if (xscale && yscale)
    {
      if (imStrEqual(scale_unit, "meters"))
        png_set_sCAL(png_ptr, info_ptr, PNG_SCALE_METER, *xscale, *yscale);
      else if (imStrEqual(scale_unit, "radians"))
        png_set_sCAL(png_ptr, info_ptr, PNG_SCALE_RADIAN, *xscale, *yscale);
    }
  }
}

int imFileFormatPNG::ReadImageInfo(int index)
{
  (void)index;

  /* Allocate/initialize the memory for image information.  REQUIRED. */
  info_ptr = png_create_info_struct(png_ptr);
  if (info_ptr == NULL)
    return IM_ERR_ACCESS;

  /* Set error handling */
  if (setjmp(png_ptr->jmpbuf))
    return IM_ERR_ACCESS;

  png_set_read_fn(png_ptr, (void*)this->handle, (png_rw_ptr)png_user_read_fn);

  png_read_info(png_ptr, info_ptr);

  png_uint_32 Width, Height;
  int bit_depth, color_type, interlace_type;
  png_get_IHDR(png_ptr, info_ptr, &Width, &Height, &bit_depth, &color_type, &interlace_type, NULL, NULL);

  this->width = Width;
  this->height = Height;

  switch(color_type)
  {
  case PNG_COLOR_TYPE_GRAY:
    this->file_color_mode = IM_GRAY;
    break;
  case PNG_COLOR_TYPE_GRAY_ALPHA:
    this->file_color_mode = IM_GRAY | IM_ALPHA;
    break;
  case PNG_COLOR_TYPE_RGB:
    this->file_color_mode = IM_RGB;
    break;
  case PNG_COLOR_TYPE_RGB_ALPHA:
    this->file_color_mode = IM_RGB | IM_ALPHA;
    break;
  case PNG_COLOR_TYPE_PALETTE:
    this->file_color_mode = IM_MAP;
    break;
  default: 
    return IM_ERR_DATA;
  }

  if (bit_depth == 16)
  {
    this->file_data_type = IM_USHORT;

    if (imBinCPUByteOrder() == IM_LITTLEENDIAN) // Intel
      png_set_swap(png_ptr);
  }
  else if (bit_depth == 1)
  {
    if (this->file_color_mode == IM_RGB)
      return IM_ERR_DATA;

    this->file_color_mode = IM_BINARY;
    this->file_data_type = IM_BYTE;
  }
  else
    this->file_data_type = IM_BYTE;

  this->file_color_mode |= IM_TOPDOWN;

  if (imColorModeDepth(this->file_color_mode) > 1)
    this->file_color_mode |= IM_PACKED;

  this->fixbits = 0;
  if (bit_depth < 8)
  {
    png_set_packing(png_ptr);
    if (bit_depth > 1 && 
        (imColorModeSpace(this->file_color_mode) == IM_GRAY || imColorModeSpace(this->file_color_mode) == IM_RGB))
      this->fixbits = bit_depth;
  }

  if (imColorModeSpace(this->file_color_mode) == IM_MAP)
  {
    png_colorp pal;
    int count;
    if (png_get_PLTE(png_ptr, info_ptr, &pal, &count))
    {
      long palette[256];

      for (int c = 0; c < count; c++)
      {
        palette[c] = imColorEncode(pal[c].red,
                                   pal[c].green,
                                   pal[c].blue);
      }

      imFileSetPalette(this, palette, count);
    }
    else
      return IM_ERR_FORMAT;
  }

  imAttribTable* attrib_table = AttribTable();

  this->interlace_steps = 1; // Not interlaced.
  if (interlace_type)
  {
    attrib_table->Set("Interlaced", IM_INT, 1, &interlace_type);
    /* Turn on interlace handling. */
    this->interlace_steps = png_set_interlace_handling(png_ptr);
  }

  png_read_update_info(png_ptr, info_ptr);

  iReadAttrib(attrib_table);

  return IM_ERR_NONE;
}

int imFileFormatPNG::WriteImageInfo()
{
  this->file_color_mode = imColorModeSpace(this->user_color_mode);
  this->file_color_mode |= IM_TOPDOWN;

  this->file_data_type = this->user_data_type;

  int bit_depth = 8;
  if (this->file_data_type == IM_USHORT)
    bit_depth = 16;

  int color_type;
  switch (imColorModeSpace(this->user_color_mode))
  {
  case IM_BINARY:
    bit_depth = 1;
    this->convert_bpp = 1;
  case IM_GRAY:
    color_type = PNG_COLOR_TYPE_GRAY;
    break;
  case IM_RGB:   
    color_type = PNG_COLOR_TYPE_RGB;
    break;
  case IM_MAP:
    color_type = PNG_COLOR_TYPE_PALETTE;
    break;
  default:
    return IM_ERR_DATA;
  }

  if (imColorModeHasAlpha(this->user_color_mode))
  {
    color_type |= PNG_COLOR_MASK_ALPHA;
    this->file_color_mode |= IM_ALPHA;
  }

  if (imColorModeDepth(this->file_color_mode) > 1)
    this->file_color_mode |= IM_PACKED;

  /* Allocate/initialize the image information data.  REQUIRED */
  info_ptr = png_create_info_struct(png_ptr);
  if (info_ptr == NULL)
    return IM_ERR_ACCESS;

  /* Set error handling.  REQUIRED if you aren't supplying your own
  * error hadnling functions in the png_create_write_struct() call. */
  if (setjmp(png_ptr->jmpbuf))
    return IM_ERR_ACCESS;

  png_set_write_fn(png_ptr, this->handle, (png_rw_ptr)png_user_write_fn, (png_flush_ptr)png_user_flush_fn);

  imAttribTable* attrib_table = AttribTable();

  int interlace = 0;
  int* interlaced = (int*)attrib_table->Get("Interlaced");
  if (interlaced && *interlaced)
    interlace = 1;

  /* write image header */
  png_set_IHDR(png_ptr, info_ptr, this->width, this->height, bit_depth, color_type, interlace, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

  if (imColorModeSpace(this->user_color_mode) == IM_MAP)
  {
    png_color pal[256];
    unsigned char r, g, b;
    for (int c = 0; c < this->palette_count; c++)
    {
      imColorDecode(&r, &g, &b, this->palette[c]);
      pal[c].red = r;
      pal[c].green = g;
      pal[c].blue = b;
    }

    png_set_PLTE(png_ptr, info_ptr, pal, this->palette_count);
  }

  int* quality = (int*)attrib_table->Get("ZIPQuality");
  if (quality)
    png_set_compression_level(png_ptr, *quality);

  iWriteAttrib(attrib_table);

  /* write image attribs */
  png_write_info(png_ptr, info_ptr);

  if (this->file_data_type == IM_USHORT)
  {
    if (imBinCPUByteOrder() == IM_LITTLEENDIAN) // Intel
      png_set_swap(png_ptr);
  }

  this->interlace_steps = 1;
  if (interlace)
    this->interlace_steps = png_set_interlace_handling(png_ptr);

  return IM_ERR_NONE;
}

static int iInterlaceRowCheck(int row_step, int pass)
{
  switch(row_step)
  {
  case 0:
    if (pass == 1 || pass == 2 || pass == 4 || pass == 6)
      return 1;
    break;
  case 4:
    if (pass == 3 || pass == 4 || pass == 6)
      return 1;
    break;
  case 2:
  case 6:
    if (pass == 5 || pass == 6)
      return 1;
    break;
  case 1:
  case 3:
  case 5:
  case 7:
    if (pass == 7)
      return 1;
    break;
  }

  return 0;
}

int imFileFormatPNG::ReadImageData(void* data)
{
  if (setjmp(this->png_ptr->jmpbuf))
    return IM_ERR_ACCESS;

  int count = this->height*this->interlace_steps;
  imCounterTotal(this->counter, count, "Reading PNG...");

  int row = 0;
  for (int i = 0; i < count; i++)
  {
    if (this->interlace_steps > 1 && ((row % 8) % 2 == 0)) // only when interlaced and in the 2,4,6 row steps.
      imFileLineBufferWrite(this, data, row, 0);

    png_read_row(this->png_ptr, (imbyte*)this->line_buffer, NULL);

    if (this->interlace_steps == 1 || iInterlaceRowCheck(row % 8, png_ptr->pass+1))
    {
      if (this->fixbits)
      {
        unsigned char* buf = (unsigned char*)this->line_buffer;
        for (int b = 0; b < this->line_buffer_size; b++)
        {
          if (this->fixbits == 4)
            *buf *= 17;
          else
            *buf *= 85;

          buf++;
        }
      }

      imFileLineBufferRead(this, data, row, 0);
    }

    if (!imCounterInc(this->counter))
    {
      png_read_end(this->png_ptr, NULL);
      return IM_ERR_COUNTER;
    }

   row++;
   if (row == this->height)
     row = 0;
  }

  png_read_end(this->png_ptr, NULL);

  return IM_ERR_NONE;
}

int imFileFormatPNG::WriteImageData(void* data)
{
  if (setjmp(this->png_ptr->jmpbuf))
    return IM_ERR_ACCESS;

  int count = this->height*this->interlace_steps;
  imCounterTotal(this->counter, count, "Writing PNG...");

  int row = 0;
  for (int i = 0; i < count; i++)
  {
    imFileLineBufferWrite(this, data, row, 0);

    png_write_row(this->png_ptr, (imbyte*)this->line_buffer);

    if (!imCounterInc(this->counter))
    {
      png_write_end(this->png_ptr, this->info_ptr);
      return IM_ERR_COUNTER;
    }

   row++;
   if (row == this->height)
     row = 0;
  }

  png_write_end(this->png_ptr, this->info_ptr);

  return IM_ERR_NONE;
}

int imFormatPNG::CanWrite(const char* compression, int color_mode, int data_type) const
{
  int color_space = imColorModeSpace(color_mode);

  if (color_space == IM_YCBCR || color_space == IM_LAB || 
      color_space == IM_LUV || color_space == IM_XYZ ||
      color_space == IM_CMYK)
    return IM_ERR_DATA;                       
                                              
  if (data_type != IM_BYTE && data_type != IM_USHORT)
    return IM_ERR_DATA;

  if (!compression || compression[0] == 0)
    return IM_ERR_NONE;

  if (!imStrEqual(compression, "DEFLATE"))
    return IM_ERR_COMPRESS;

  return IM_ERR_NONE;
}

