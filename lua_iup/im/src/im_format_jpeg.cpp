/** \file
 * \brief JPEG File Interchange Format
 *
 * See Copyright Notice in im_lib.h
 * See libJPEG Copyright Notice in jpeglib.h
 * $Id: im_format_jpeg.cpp,v 1.3 2009/08/19 18:39:43 scuri Exp $
 */

#include "im_format.h"
#include "im_format_all.h"
#include "im_util.h"
#include "im_counter.h"
#include "im_math.h"

#include "im_binfile.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <setjmp.h>
#include <math.h>

extern "C" {
#include "jpeglib.h"
#include "jinclude.h"
#include "jpeglib.h"
#include "jerror.h"
}
             
#ifdef USE_EXIF
#include "exif-data.h"
#include "exif-entry.h"
#include "exif-utils.h"
extern "C" const char *exif_tag_get_name_index (unsigned int i, ExifTag *tag);
#endif

/* libjpeg error handlers */

struct JPEGerror_mgr 
{
  jpeg_error_mgr pub;  /* "public" fields */
  jmp_buf setjmp_buffer;      /* for return to caller */
};

METHODDEF(void)
JPEGerror_exit (j_common_ptr cinfo)
{
  /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
  JPEGerror_mgr* err_mgr = (JPEGerror_mgr*)cinfo->err;

  /* Return control to the setjmp point */
  longjmp(err_mgr->setjmp_buffer, 1);
}

METHODDEF(void)
JPEGoutput_message (j_common_ptr cinfo)
{
  (void)cinfo;
}

METHODDEF(void)
JPEGemit_message (j_common_ptr cinfo, int msg_level)
{
  (void)cinfo; (void)msg_level;
}

static const char* iJPEGCompTable[1] = 
{
  "JPEG"
};

class imFileFormatJPEG: public imFileFormatBase
{
  jpeg_decompress_struct dinfo;
  jpeg_compress_struct cinfo;
  JPEGerror_mgr jerr;

  imBinFile* handle;
  int fix_adobe;

#ifdef USE_EXIF
  void iReadExifAttrib(unsigned char* data, int data_length, imAttribTable* attrib_table);
  void iWriteExifAttrib(imAttribTable* attrib_table);
#endif

public:
  imFileFormatJPEG(const imFormat* _iformat): imFileFormatBase(_iformat) {}
  ~imFileFormatJPEG() {}

  int Open(const char* file_name);
  int New(const char* file_name);
  void Close();
  void* Handle(int index);
  int ReadImageInfo(int index);
  int ReadImageData(void* data);
  int WriteImageInfo();
  int WriteImageData(void* data);
};

class imFormatJPEG: public imFormat
{
public:
  imFormatJPEG()
    :imFormat("JPEG", 
              "JPEG File Interchange Format", 
              "*.jpg;*.jpeg;*.jpe;*.jfif;*.jif;*.jfi;", 
              iJPEGCompTable, 
              1, 
              0)
    {}
  ~imFormatJPEG() {}

  imFileFormatBase* Create(void) const { return new imFileFormatJPEG(this); }
  int CanWrite(const char* compression, int color_mode, int data_type) const;
};

void imFormatRegisterJPEG(void)
{
  imFormatRegister(new imFormatJPEG());
}

int imFileFormatJPEG::Open(const char* file_name)
{
  this->handle = imBinFileOpen(file_name);
  if (this->handle == NULL)
    return IM_ERR_OPEN;

  unsigned char sig[2];
  if (!imBinFileRead(this->handle, sig, 2, 1))
  {
    imBinFileClose(this->handle);
    return IM_ERR_ACCESS;
  }

  if (sig[0] != 0xFF || sig[1] != 0xD8)
  {
    imBinFileClose(this->handle);
    return IM_ERR_FORMAT;
  }

  imBinFileSeekTo(this->handle, 0);

  strcpy(this->compression, "JPEG");
  this->image_count = 1;

  this->dinfo.err = jpeg_std_error(&this->jerr.pub);
  this->jerr.pub.error_exit = JPEGerror_exit;
  this->jerr.pub.output_message = JPEGoutput_message;
  this->jerr.pub.emit_message = JPEGemit_message;

  /* Establish the setjmp return context for error_exit to use. */
  if (setjmp(this->jerr.setjmp_buffer)) 
  {
    /* If we get here, the JPEG code has signaled an error.
     * We need to clean up the JPEG object, close the input file, and return. */
    jpeg_destroy_decompress(&this->dinfo);
    imBinFileClose(this->handle);
    return IM_ERR_FORMAT;
  }

  /* Now we can initialize the JPEG decompression object. */
  jpeg_create_decompress(&this->dinfo);

  /* Step 2: specify data source (eg, a file) */
  jpeg_stdio_src(&this->dinfo, (FILE*)this->handle);

  return IM_ERR_NONE;
}

int imFileFormatJPEG::New(const char* file_name)
{
  this->handle = imBinFileNew(file_name);
  if (this->handle == NULL)
    return IM_ERR_OPEN;

  this->cinfo.err = jpeg_std_error(&this->jerr.pub);
  this->jerr.pub.error_exit = JPEGerror_exit;
  this->jerr.pub.output_message = JPEGoutput_message;
  this->jerr.pub.emit_message = JPEGemit_message;
  
  /* Establish the setjmp return context for error_exit to use. */
  if (setjmp(this->jerr.setjmp_buffer)) 
  {
    /* If we get here, the JPEG code has signaled an error.
     * We need to clean up the JPEG object, close the input file, and return. */
    jpeg_destroy_compress(&this->cinfo);
    imBinFileClose(this->handle);
    return IM_ERR_ACCESS;
  }
  
  jpeg_create_compress(&this->cinfo);

  /* Step 2: specify data destination (eg, a file) */
  jpeg_stdio_dest(&this->cinfo, (FILE*)this->handle);

  strcpy(this->compression, "JPEG");
  this->image_count = 1;

  return IM_ERR_NONE;
}

void imFileFormatJPEG::Close()
{
  if (this->is_new)
    jpeg_destroy_compress(&this->cinfo);
  else
    jpeg_destroy_decompress(&this->dinfo);

  imBinFileClose(this->handle);
}

void* imFileFormatJPEG::Handle(int index)
{
  if (index == 0)
    return this->handle;
  else if (index == 1)
  {
    if (this->is_new)
      return (void*)&this->cinfo;
    else
      return (void*)&this->dinfo;
  }
  else
    return NULL;
}

#ifdef USE_EXIF
void imFileFormatJPEG::iReadExifAttrib(unsigned char* data, int data_length, imAttribTable* attrib_table)
{
  ExifData* exif = exif_data_new_from_data(data, data_length);
  if (!exif)
    return;

  void* value = NULL;
  int c, value_size = 0;

	ExifByteOrder byte_order = exif_data_get_byte_order(exif);

  for (int i = 0; i < 3; i += 2)  // Only scan for IFD_0 (0) and IFD_EXIF (2)
  {
    ExifContent *content = exif->ifd[i];

    if (content && content->count) 
    {
	    for (int j = 0; j < (int)content->count; j++) 
      {
        ExifEntry *entry = content->entries[j];
        int type = 0;

        const char* name = exif_tag_get_name(entry->tag);
        if (!name)
          continue;

        if (value_size < (int)entry->size)
        {
          value = realloc(value, entry->size);
          value_size = entry->size;
        }

        int format_size = exif_format_get_size(entry->format);

        if (entry->tag == EXIF_TAG_RESOLUTION_UNIT)
        {
          int res_unit = (int)exif_get_short (entry->data, byte_order);

          if (res_unit == 2)
            attrib_table->Set("ResolutionUnit", IM_BYTE, -1, "DPI");
          else if (res_unit == 3)
            attrib_table->Set("ResolutionUnit", IM_BYTE, -1, "DPC");

          continue;
        }

        switch (entry->format) 
        {
        case EXIF_FORMAT_UNDEFINED:
        case EXIF_FORMAT_ASCII:
        case EXIF_FORMAT_SBYTE:
        case EXIF_FORMAT_BYTE:
          {
            type = IM_BYTE;
            imbyte *bvalue = (imbyte*)value;
            for (c = 0; c < (int)entry->components; c++) 
              bvalue[c] = entry->data[c];
          }
          break;
        case EXIF_FORMAT_SSHORT:
        case EXIF_FORMAT_SHORT:
          {
            type = IM_USHORT;
            imushort *usvalue = (imushort*)value;
            for (c = 0; c < (int)entry->components; c++) 
              usvalue[c] = exif_get_short(entry->data + format_size * c, byte_order);
          }
          break;
        case EXIF_FORMAT_LONG:
          {
            type = IM_INT;
            int *ivalue = (int*)value;
            for (c = 0; c < (int)entry->components; c++) 
              ivalue[c] = (int)exif_get_long(entry->data + format_size * c, byte_order);
          }
          break;
        case EXIF_FORMAT_SLONG:
          {
            type = IM_INT;
            int *ivalue = (int*)value;
            for (c = 0; c < (int)entry->components; c++) 
              ivalue[c] = (int)exif_get_slong(entry->data + format_size * c, byte_order);
          }
          break;
        case EXIF_FORMAT_RATIONAL:
          {
	          ExifRational v_rat;
            type = IM_FLOAT;
            float *fvalue = (float*)value;
            for (c = 0; c < (int)entry->components; c++) 
            {
              v_rat = exif_get_rational(entry->data + format_size * c, byte_order);
              fvalue[c] = (float)v_rat.numerator / (float)v_rat.denominator;
            }
          }
          break;
        case EXIF_FORMAT_SRATIONAL:
          {
	          ExifSRational v_srat;
            type = IM_FLOAT;
            float *fvalue = (float*)value;
            for (c = 0; c < (int)entry->components; c++) 
            {
              v_srat = exif_get_srational(entry->data + format_size * c, byte_order);
              fvalue[c] = (float)v_srat.numerator / (float)v_srat.denominator;
            }
          }
          break;
        case EXIF_FORMAT_FLOAT:  // missing from libEXIF
        case EXIF_FORMAT_DOUBLE:
          break;
        }

        attrib_table->Set(name, type, entry->components, value);
      }
    }
  }

  if (value) free(value);

  exif_data_free(exif);
}

static void iGetRational(float fvalue, int *num, int *den, int sign)
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

	if (fvalue < 0) 
  {
		if (sign == 1)
			fvalue = 0;
		else
			fvalue = -fvalue;
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

	*num = sign * imRound(fvalue);
}

void imFileFormatJPEG::iWriteExifAttrib(imAttribTable* attrib_table)
{
  ExifData* exif = exif_data_new();

  ExifByteOrder byte_order;
  if (imBinCPUByteOrder() == IM_LITTLEENDIAN)
	  byte_order = EXIF_BYTE_ORDER_INTEL;
	else
		byte_order = EXIF_BYTE_ORDER_MOTOROLA;
    
  exif_data_set_byte_order(exif, byte_order);

  int c, i = 0;
  while(i>=0)
  {
    ExifTag tag;
    const char * name = exif_tag_get_name_index(i, &tag);
    if (!name)
      break;

    ExifEntry *entry;
    int attrib_count;
    const void* attrib_data = attrib_table->Get(name, NULL, &attrib_count); 
    if (attrib_data)
    {
      entry = exif_entry_new();

      ExifContent *content;
      if (tag > EXIF_TAG_COPYRIGHT)
        content = exif->ifd[2];     // IFD_EXIF (2) contains EXIF tags
      else
        content = exif->ifd[0];     // IFD_0    (0) contains TIFF tags 

      exif_content_add_entry(content, entry);

      exif_entry_initialize(entry, tag);

      if (!entry->format)  // unsupported tag
      {
        i++;
        continue;
      }

      int format_size = exif_format_get_size(entry->format);

      if (tag == EXIF_TAG_RESOLUTION_UNIT)
      {
        int res_unit;
        if (imStrEqual((char*)attrib_data, "DPI"))
          res_unit = 2;
        else
          res_unit = 3;

        exif_set_short (entry->data, byte_order, (imushort)res_unit);

        i++;
        continue;
      }

      if (entry->components == 0)
      {
		    entry->components = attrib_count;
        if (entry->data) free(entry->data);
        entry->size = format_size * entry->components;
        entry->data = (imbyte*)malloc(entry->size);
      }

      switch (entry->format) 
      {
      case EXIF_FORMAT_UNDEFINED:
      case EXIF_FORMAT_ASCII:
      case EXIF_FORMAT_BYTE:
        {
          imbyte *bvalue = (imbyte*)attrib_data;
          for (c = 0; c < (int)entry->components; c++) 
            entry->data[c] = bvalue[c];
        }
        break;
      case EXIF_FORMAT_SHORT:
        {
          imushort *usvalue = (imushort*)attrib_data;
          for (c = 0; c < (int)entry->components; c++) 
            exif_set_short(entry->data + format_size * c, byte_order, usvalue[c]);
        }
        break;
      case EXIF_FORMAT_LONG:
        {
          int *ivalue = (int*)attrib_data;
          for (c = 0; c < (int)entry->components; c++) 
            exif_set_long(entry->data + format_size * c, byte_order, (unsigned int)ivalue[c]);
        }
        break;
      case EXIF_FORMAT_SLONG:
        {
          int *ivalue = (int*)attrib_data;
          for (c = 0; c < (int)entry->components; c++) 
            exif_set_slong(entry->data + format_size * c, byte_order, (int)ivalue[c]);
        }
        break;
      case EXIF_FORMAT_RATIONAL:
        {
	        ExifRational v_rat;
          int num, den;
          float *fvalue = (float*)attrib_data;
          for (c = 0; c < (int)entry->components; c++) 
          {
            iGetRational(fvalue[c], &num, &den, 1);
            v_rat.numerator = num;
            v_rat.denominator = den;
            exif_set_rational(entry->data + format_size * c, byte_order, v_rat);
          }
        }
        break;
      case EXIF_FORMAT_SRATIONAL:
        {
	        ExifSRational v_srat;
          int num, den;
          float *fvalue = (float*)attrib_data;
          for (c = 0; c < (int)entry->components; c++) 
          {
            iGetRational(fvalue[c], &num, &den, 1);
            v_srat.numerator = num;
            v_srat.denominator = den;
            exif_set_srational(entry->data + format_size * c, byte_order, v_srat);
          }
        }
        break;
      }
    }

    i++;
  }

  imbyte* data = NULL;
  unsigned int data_size = 0;

  exif_data_save_data(exif, &data, &data_size);

  if (data)
  {
    jpeg_write_marker(&this->cinfo, JPEG_APP0+1, data, data_size);
    free(data);
  }

  exif_data_free(exif);
}
#endif

int imFileFormatJPEG::ReadImageInfo(int index)
{
  (void)index;
  this->fix_adobe = 0;

  if (setjmp(this->jerr.setjmp_buffer)) 
    return IM_ERR_ACCESS;

  // notify libjpeg to save the COM marker
  jpeg_save_markers(&this->dinfo, JPEG_COM, 0xFFFF);
  jpeg_save_markers(&this->dinfo, JPEG_APP0+1, 0xFFFF);

  /* Step 3: read file parameters with jpeg_read_header() */
  if (jpeg_read_header(&this->dinfo, TRUE) != JPEG_HEADER_OK)
    return IM_ERR_ACCESS;

  this->width = this->dinfo.image_width;
  this->height = this->dinfo.image_height;
  this->file_data_type = IM_BYTE;

  switch(this->dinfo.jpeg_color_space)
  {
  case JCS_GRAYSCALE:
    this->file_color_mode = IM_GRAY;
    break;
  case JCS_RGB:
    this->file_color_mode = IM_RGB;
    break;
  case JCS_YCbCr:
    this->file_color_mode = IM_RGB;
    break;
  case JCS_CMYK:
    this->file_color_mode = IM_CMYK;
    break;
  case JCS_YCCK:
    this->file_color_mode = IM_CMYK; // this is the only supported conversion in libjpeg
    this->dinfo.out_color_space = JCS_CMYK;
    this->fix_adobe = 1;
    break;
  default: /* JCS_UNKNOWN */
    return IM_ERR_DATA;
  }

  imAttribTable* attrib_table = AttribTable();

  int* auto_ycbcr = (int*)attrib_table->Get("AutoYCbCr");
  if (auto_ycbcr && *auto_ycbcr == 0 &&
      this->dinfo.jpeg_color_space == JCS_YCbCr)
  {
    this->file_color_mode = IM_YCBCR;
    this->dinfo.out_color_space = JCS_YCbCr;
  }

  this->file_color_mode |= IM_TOPDOWN;

  if (imColorModeDepth(this->file_color_mode) > 1)
    this->file_color_mode |= IM_PACKED;

  if (this->dinfo.progressive_mode != 0)
  {
    int progressive = 1;
    attrib_table->Set("Interlaced", IM_INT, 1, &progressive);
  }

  if (this->dinfo.density_unit != 0)
  {
    float xres = (float)this->dinfo.X_density, 
          yres = (float)this->dinfo.Y_density;

    if (this->dinfo.density_unit == 1)
      attrib_table->Set("ResolutionUnit", IM_BYTE, -1, "DPI");
    else
      attrib_table->Set("ResolutionUnit", IM_BYTE, -1, "DPC");

    attrib_table->Set("XResolution", IM_FLOAT, 1, (void*)&xres);
    attrib_table->Set("YResolution", IM_FLOAT, 1, (void*)&yres);
  }

  if (this->dinfo.marker_list)
  {
    jpeg_saved_marker_ptr cur_marker = this->dinfo.marker_list;

    // search for COM marker
    while (cur_marker)
    {
      if (cur_marker->marker == JPEG_COM)
      {
        char* desc = new char [cur_marker->data_length+1];
        memcpy(desc, cur_marker->data, cur_marker->data_length);
        desc[cur_marker->data_length] = 0;
        attrib_table->Set("Description", IM_BYTE, cur_marker->data_length+1, desc);
        delete [] desc;
      }
      
#ifdef USE_EXIF
      if (cur_marker->marker == JPEG_APP0+1)
        iReadExifAttrib(cur_marker->data, cur_marker->data_length, attrib_table);
#endif

      cur_marker = cur_marker->next;
    }
  }

  /* Step 5: Start decompressor */
  if (jpeg_start_decompress(&this->dinfo) == FALSE)
    return IM_ERR_ACCESS;

  return IM_ERR_NONE;
}

int imFileFormatJPEG::WriteImageInfo()
{
  this->file_color_mode = imColorModeSpace(this->user_color_mode);
  this->file_color_mode |= IM_TOPDOWN;

  if (imColorModeDepth(this->file_color_mode) > 1)
    this->file_color_mode |= IM_PACKED;

  this->file_data_type = IM_BYTE;

  /* Step 3: set parameters for compression */
  this->cinfo.image_width = this->width;   /* image width and height, in pixels */
  this->cinfo.image_height = this->height;

  this->cinfo.input_components = imColorModeDepth(this->file_color_mode);

  switch (imColorModeSpace(this->user_color_mode))
  {
  case IM_BINARY:
    this->convert_bpp = -1; // expand 1 to 255
  case IM_GRAY:
    this->cinfo.in_color_space = JCS_GRAYSCALE;
    break;
  case IM_RGB:   
    this->cinfo.in_color_space = JCS_RGB;
    break;
  case IM_CMYK:
    this->cinfo.in_color_space = JCS_CMYK;
    break;
  case IM_YCBCR:
    this->cinfo.in_color_space = JCS_YCbCr;
    break;
  default:
    this->cinfo.in_color_space = JCS_UNKNOWN;
    break;
  }

  if (setjmp(this->jerr.setjmp_buffer)) 
    return IM_ERR_ACCESS;

  jpeg_set_defaults(&this->cinfo);

  imAttribTable* attrib_table = AttribTable();

  int* auto_ycbcr = (int*)attrib_table->Get("AutoYCbCr");
  if (auto_ycbcr && *auto_ycbcr == 0 &&
      this->cinfo.in_color_space == JCS_RGB)
  {
    jpeg_set_colorspace(&this->cinfo, JCS_RGB);
  }

  int* interlaced = (int*)attrib_table->Get("Interlaced");
  if (interlaced && *interlaced)
    jpeg_simple_progression(&this->cinfo);

  int* quality = (int*)attrib_table->Get("JPEGQuality");
  if (quality)
    jpeg_set_quality(&this->cinfo, *quality, TRUE);

  char* res_unit = (char*)attrib_table->Get("ResolutionUnit");
  if (res_unit)
  {
    float* xres = (float*)attrib_table->Get("XResolution");
    float* yres = (float*)attrib_table->Get("YResolution");

    if (xres && yres)
    {
      if (imStrEqual(res_unit, "DPI"))
        this->cinfo.density_unit = 1;
      else
        this->cinfo.density_unit = 2;

      this->cinfo.X_density = (UINT16)*xres;
      this->cinfo.Y_density = (UINT16)*yres;
    }
  }

  /* Step 4: Start compressor */
  jpeg_start_compress(&this->cinfo, TRUE);

  int desc_size;
  char* desc = (char*)attrib_table->Get("Description", NULL, &desc_size);
  if (desc)
    jpeg_write_marker(&this->cinfo, JPEG_COM, (JOCTET*)desc, desc_size-1);

#ifdef USE_EXIF
  iWriteExifAttrib(attrib_table);
#endif

  return IM_ERR_NONE;
}

static void iFixAdobe(unsigned char* line_buffer, int width)
{
  width *= 4;
  for (int i = 0; i < width; i++)
  {
    *line_buffer = 255 - *line_buffer;
    line_buffer++;
  }
}

int imFileFormatJPEG::ReadImageData(void* data)
{
  if (setjmp(this->jerr.setjmp_buffer)) 
    return IM_ERR_ACCESS;

  imCounterTotal(this->counter, this->dinfo.output_height, "Reading JPEG...");

  int row = 0, plane = 0;
  while (this->dinfo.output_scanline < this->dinfo.output_height) 
  {
    if (jpeg_read_scanlines(&this->dinfo, (JSAMPARRAY)&this->line_buffer, 1) == 0)
      return IM_ERR_ACCESS;

    if (this->fix_adobe)
      iFixAdobe((unsigned char*)this->line_buffer, this->width);

    imFileLineBufferRead(this, data, row, plane);

    if (!imCounterInc(this->counter))
    {
      jpeg_finish_decompress(&this->dinfo);
      return IM_ERR_COUNTER;
    }

    imFileLineBufferInc(this, &row, &plane);
  }

  jpeg_finish_decompress(&this->dinfo);

  return IM_ERR_NONE;
}

int imFileFormatJPEG::WriteImageData(void* data)
{
  if (setjmp(this->jerr.setjmp_buffer)) 
    return IM_ERR_ACCESS;

  imCounterTotal(this->counter, this->dinfo.output_height, "Writing JPEG...");

  int row = 0, plane = 0;
  while (this->cinfo.next_scanline < this->cinfo.image_height) 
  {
    imFileLineBufferWrite(this, data, row, plane);

    if (jpeg_write_scanlines(&this->cinfo, (JSAMPARRAY)&this->line_buffer, 1) == 0)
      return IM_ERR_ACCESS;

    if (!imCounterInc(this->counter))
    {
      jpeg_finish_compress(&this->cinfo);
      return IM_ERR_COUNTER;
    }

    imFileLineBufferInc(this, &row, &plane);
  }

  jpeg_finish_compress(&this->cinfo);

  return IM_ERR_NONE;
}

int imFormatJPEG::CanWrite(const char* compression, int color_mode, int data_type) const
{
  int color_space = imColorModeSpace(color_mode);

  if (color_space == IM_MAP || color_space == IM_LAB || 
      color_space == IM_LUV || color_space == IM_XYZ)
    return IM_ERR_DATA;                       
                                              
  if (data_type != IM_BYTE)
    return IM_ERR_DATA;

  if (!compression || compression[0] == 0)
    return IM_ERR_NONE;

  if (!imStrEqual(compression, "JPEG"))
    return IM_ERR_COMPRESS;

  return IM_ERR_NONE;
}

