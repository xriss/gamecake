/** \file
 * \brief File Access
 *
 * See Copyright Notice in im_lib.h
 * $Id: im_file.cpp,v 1.6 2009/11/23 17:13:05 scuri Exp $
 */

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "im.h"
#include "im_format.h"
#include "im_util.h"
#include "im_attrib.h"
#include "im_counter.h"
#include "im_plus.h"  // make shure that this file is compiled


void imFileClear(imFile* ifile)
{
  // can not reset compression and image_count

  ifile->is_new = 0;
  ifile->attrib_table = 0;

  ifile->line_buffer = 0;
  ifile->line_buffer_size = 0;
  ifile->line_buffer_extra = 0;
  ifile->line_buffer_alloc = 0;

  ifile->convert_bpp = 0;
  ifile->switch_type = 0;

  ifile->width = 0; 
  ifile->height = 0; 
  ifile->image_index = -1; 
  ifile->user_data_type = 0;
  ifile->user_color_mode = 0; 
  ifile->file_data_type = 0;
  ifile->file_color_mode = 0;

  ifile->palette_count = 256;
  for (int i = 0; i < 256; i++)
    ifile->palette[i] = imColorEncode((imbyte)i, (imbyte)i, (imbyte)i);
}

void imFileSetBaseAttributes(imFile* ifile)
{
  imFileFormatBase* ifileformat = (imFileFormatBase*)ifile;
  imAttribTable* atable = (imAttribTable*)ifileformat->attrib_table;

  atable->Set("FileFormat", IM_BYTE, -1, ifileformat->iformat->format);
  atable->Set("FileCompression", IM_BYTE, -1, ifileformat->compression);
  atable->Set("FileImageCount", IM_INT, 1, &ifileformat->image_count);
}

imFile* imFileOpen(const char* file_name, int *error)
{
  assert(file_name);

  imFileFormatBase* ifileformat = imFileFormatBaseOpen(file_name, error);
  if (!ifileformat) 
    return NULL;

  imFileClear(ifileformat);

  ifileformat->attrib_table = new imAttribTable(599);
  imFileSetBaseAttributes(ifileformat);

  ifileformat->counter = imCounterBegin(file_name);

  return ifileformat;
}

imFile* imFileOpenAs(const char* file_name, const char* format, int *error)
{
  assert(file_name);

  imFileFormatBase* ifileformat = imFileFormatBaseOpenAs(file_name, format, error);
  if (!ifileformat) 
    return NULL;

  imFileClear(ifileformat);

  ifileformat->attrib_table = new imAttribTable(599);
  imFileSetBaseAttributes(ifileformat);

  ifileformat->counter = imCounterBegin(file_name);

  return ifileformat;
}

imFile* imFileNew(const char* file_name, const char* format, int *error)
{
  assert(file_name);

  imFileFormatBase* ifileformat = imFileFormatBaseNew(file_name, format, error);
  if (!ifileformat) 
    return NULL;

  imFileClear(ifileformat);

  ifileformat->is_new = 1;
  ifileformat->image_count = 0;
  ifileformat->compression[0] = 0;

  ifileformat->attrib_table = new imAttribTable(101);

  ifileformat->counter = imCounterBegin(file_name);

  return ifileformat;
}

void imFileClose(imFile* ifile)
{
  assert(ifile);
  imFileFormatBase* ifileformat = (imFileFormatBase*)ifile;
  imAttribTable* attrib_table = (imAttribTable*)ifile->attrib_table;

  imCounterEnd(ifile->counter);

  ifileformat->Close();

  if (ifile->line_buffer) free(ifile->line_buffer);
  
  delete attrib_table;
  delete ifileformat;
}

void* imFileHandle(imFile* ifile, int index)
{
  assert(ifile);
  imFileFormatBase* ifileformat = (imFileFormatBase*)ifile;
  return ifileformat->Handle(index);
}

void imFileSetAttribute(imFile* ifile, const char* attrib, int data_type, int count, const void* data)
{
  assert(ifile);
  assert(attrib);
  imFileFormatBase* ifileformat = (imFileFormatBase*)ifile;
  imAttribTable* atable = (imAttribTable*)ifileformat->attrib_table;
  if (data)
    atable->Set(attrib, data_type, count, data);
  else
    atable->UnSet(attrib);
}

const void* imFileGetAttribute(imFile* ifile, const char* attrib, int *data_type, int *count)
{
  assert(ifile);
  assert(attrib);
  imFileFormatBase* ifileformat = (imFileFormatBase*)ifile;
  imAttribTable* attrib_table = (imAttribTable*)ifileformat->attrib_table;
  return attrib_table->Get(attrib, data_type, count);
}

static int iAttribCB(void* user_data, int index, const char* name, int data_type, int count, const void* data)
{
  (void)data_type;
  (void)data;
  (void)count;
  char** attrib = (char**)user_data;
  attrib[index] = (char*)name;
  return 1;
}

void imFileGetAttributeList(imFile* ifile, char** attrib, int *attrib_count)
{
  assert(ifile);
  assert(attrib_count);

  imAttribTable* attrib_table = (imAttribTable*)ifile->attrib_table;
  *attrib_count = attrib_table->Count();

  if (attrib) attrib_table->ForEach((void*)attrib, iAttribCB);
}

void imFileGetInfo(imFile* ifile, char* format, char* compression, int *image_count)
{
  assert(ifile);
  imFileFormatBase* ifileformat = (imFileFormatBase*)ifile;

  if(compression) strcpy(compression, ifile->compression);
  if(format) strcpy(format, ifileformat->iformat->format);
  if (image_count) *image_count = ifile->image_count;
}

static int iFileCheckPaletteGray(imFile* ifile)
{
  int i;
  imbyte r, g, b;
  imbyte remaped[256];
  memset(remaped, 0, 256);

  for (i = 0; i < ifile->palette_count; i++)
  {
    imColorDecode(&r, &g, &b, ifile->palette[i]);

    /* if there are colors abort */
    if (r != g || g != b)
      return 0;

    /* grays out of order, will be remapped, but must be unique, 
       if there are duplicates maybe they are used for different pourposes */
    if (i != r)
    {
      if (!remaped[r])
        remaped[r] = 1;
      else
        return 0;
    }
  }

  return 1;
}

static int iFileCheckPaletteBinary(imFile* ifile)
{
  if (ifile->palette_count > 2)
    return 0;

  imbyte r, g, b;

  imColorDecode(&r, &g, &b, ifile->palette[0]);
  if ((r != 0 || g != 0 || b != 0) &&
      (r != 1 || g != 1 || b != 1) &&
      (r != 255 || g != 255 || b != 255))
    return 0;

  imColorDecode(&r, &g, &b, ifile->palette[1]);
  if ((r != 0 || g != 0 || b != 0) &&
      (r != 1 || g != 1 || b != 1) &&
      (r != 255 || g != 255 || b != 255))
    return 0;

  return 1;
}

int imFileReadImageInfo(imFile* ifile, int index, int *width, int *height, int *file_color_mode, int *file_data_type)
{
  assert(ifile);
  assert(!ifile->is_new);
  imFileFormatBase* ifileformat = (imFileFormatBase*)ifile;

  if (index >= ifile->image_count)
    return IM_ERR_DATA;

  if (ifile->image_index != -1 &&
      ifile->image_index == index)
  {
    if(width) *width = ifile->width;
    if(height) *height = ifile->height;
    if(file_color_mode) *file_color_mode = ifile->file_color_mode;
    if(file_data_type) *file_data_type = ifile->file_data_type;

    return IM_ERR_NONE;
  }

  ifile->convert_bpp = 0;
  ifile->switch_type = 0;

  int error = ifileformat->ReadImageInfo(index);
  if (error) return error;

  if (!imImageCheckFormat(ifile->file_color_mode, ifile->file_data_type))
    return IM_ERR_DATA;

  if (imColorModeSpace(ifile->file_color_mode) == IM_BINARY)
  {
    ifile->palette_count = 2;
    ifile->palette[0] = imColorEncode(0, 0, 0);
    ifile->palette[1] = imColorEncode(255, 255, 255);
  }

  if (imColorModeSpace(ifile->file_color_mode) == IM_MAP)
  {    
    if (iFileCheckPaletteGray(ifile))
      ifile->file_color_mode = (ifile->file_color_mode & 0xFF00) | IM_GRAY;

    if (iFileCheckPaletteBinary(ifile))
      ifile->file_color_mode = (ifile->file_color_mode & 0xFF00) | IM_BINARY;
  }

  if(width) *width = ifile->width;
  if(height) *height = ifile->height;
  if(file_color_mode) *file_color_mode = ifile->file_color_mode;
  if(file_data_type) *file_data_type = ifile->file_data_type;

  ifile->image_index = index; 

  return IM_ERR_NONE;
}

void imFileGetPalette(imFile* ifile, long* palette, int *palette_count)
{
  assert(ifile);
  assert(palette);

  if (ifile->palette_count != 0 && palette)
    memcpy(palette, ifile->palette, ifile->palette_count*sizeof(long));

 if (palette_count) *palette_count = ifile->palette_count;
}

static void iFileCheckConvertGray(imFile* ifile, imbyte* data)
{
  int i, do_remap = 0;
  imbyte remap[256], r, g, b;

  // enforce the palette to only have grays in the correct order.

  for (i = 0; i < ifile->palette_count; i++)
  {
    imColorDecode(&r, &g, &b, ifile->palette[i]);

    if (r != i)
    {
      ifile->palette[i] = imColorEncode((imbyte)i, (imbyte)i, (imbyte)i);
      do_remap = 1;
    }

    remap[i] = r;
  }

  if (!do_remap)
    return;

  int count = ifile->width*ifile->height;
  for(i = 0; i < count; i++)
  {
    *data = remap[*data];
    data++;
  }

  int transp_count;
  imbyte* transp_map = (imbyte*)imFileGetAttribute(ifile, "TransparencyMap", NULL, &transp_count);
  if (transp_map)
  {
    imbyte new_transp_map[256];
    for (i=0; i<transp_count; i++)
      new_transp_map[i] = transp_map[remap[i]];
    imFileSetAttribute(ifile, "TransparencyMap", IM_BYTE, transp_count, new_transp_map);
  }
}

static void iFileCheckConvertBinary(imFile* ifile, imbyte* data)
{
  int count = ifile->width*ifile->height;
  for(int i = 0; i < count; i++)
  {
    if (*data)
      *data = 1;
    data++;
  }
}

int imFileReadImageData(imFile* ifile, void* data, int convert2bitmap, int color_mode_flags)
{
  assert(ifile);
  assert(!ifile->is_new);
  imFileFormatBase* ifileformat = (imFileFormatBase*)ifile;

  if (ifile->image_index == -1)
    return IM_ERR_DATA;

  ifile->user_color_mode = ifile->file_color_mode;
  ifile->user_data_type = ifile->file_data_type;

  if (convert2bitmap)
  {
    ifile->user_data_type = IM_BYTE;
    ifile->user_color_mode = imColorModeToBitmap(ifile->file_color_mode);
  }

  if (color_mode_flags != -1)
  {
    ifile->user_color_mode = imColorModeSpace(ifile->user_color_mode);
    ifile->user_color_mode |= color_mode_flags;
  }

  if (!imImageCheckFormat(ifile->user_color_mode, ifile->user_data_type))
    return IM_ERR_DATA;

  if (!imFileCheckConversion(ifile))
    return IM_ERR_DATA;

  imFileLineBufferInit(ifile);

  int ret = ifileformat->ReadImageData(data);

  // here we can NOT change the file_color_mode we already returned to the user
  // so just check for gray and binary consistency

  if (imColorModeSpace(ifile->file_color_mode) == IM_GRAY && ifile->file_data_type == IM_BYTE)
    iFileCheckConvertGray(ifile, (imbyte*)data);

  if (imColorModeSpace(ifile->file_color_mode) == IM_BINARY)
    iFileCheckConvertBinary(ifile, (imbyte*)data);

  return ret;
}

void imFileSetInfo(imFile* ifile, const char* compression)
{
  assert(ifile);
  assert(ifile->is_new);

  if (!compression)
    ifile->compression[0] = 0;
  else
    strcpy(ifile->compression, compression);
}

void imFileSetPalette(imFile* ifile, long* palette, int palette_count)
{
  assert(ifile);
  assert(palette);
  assert(palette_count != 0);

  memcpy(ifile->palette, palette, palette_count*sizeof(long));
  ifile->palette_count = palette_count;
}

int imFileWriteImageInfo(imFile* ifile, int width, int height, int user_color_mode, int user_data_type)
{
  assert(ifile);
  assert(ifile->is_new);
  imFileFormatBase* ifileformat = (imFileFormatBase*)ifile;

  if (!imImageCheckFormat(user_color_mode, user_data_type))
    return IM_ERR_DATA;

  int error = ifileformat->iformat->CanWrite(ifile->compression, user_color_mode, user_data_type);
  if (error) return error;

  ifile->width = width;
  ifile->height = height;
  ifile->user_color_mode = user_color_mode;
  ifile->user_data_type = user_data_type;

  if (imColorModeSpace(user_color_mode) == IM_BINARY)
  {
    ifile->palette_count = 2;
    ifile->palette[0] = imColorEncode(0, 0, 0);
    ifile->palette[1] = imColorEncode(255, 255, 255);
  }

  return ifileformat->WriteImageInfo();
}

int imFileWriteImageData(imFile* ifile, void* data)
{
  assert(ifile);
  assert(ifile->is_new);
  assert(data);
  imFileFormatBase* ifileformat = (imFileFormatBase*)ifile;

  if (!imFileCheckConversion(ifile))
    return IM_ERR_DATA;

  imFileLineBufferInit(ifile);

  return ifileformat->WriteImageData(data);
}
