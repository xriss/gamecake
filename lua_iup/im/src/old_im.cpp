/** \file
 * \brief Old API
 *
 * See Copyright Notice in im_lib.h
 * $Id: old_im.cpp,v 1.2 2009/08/19 18:39:43 scuri Exp $
 */

#include <stdlib.h>
#include <memory.h>
#include <string.h>

#include "old_im.h"
#include "im.h"
#include "im_util.h"
#include "im_counter.h"

long imEncodeColor(unsigned char Red, unsigned char Green, unsigned char Blue)
{
  return imColorEncode(Red, Green, Blue);
}

void imDecodeColor(unsigned char* Red, unsigned char* Green, unsigned char* Blue, long Color)
{
  imColorDecode(Red, Green, Blue, Color);
}

static int FormatNew2Old(const char* new_format, const char* compression)
{
  int format;

  if (!imStrEqual(new_format, "BMP"))
    format = IM_BMP;
  else if (!imStrEqual(new_format, "GIF"))
    format = IM_GIF;
  else if (!imStrEqual(new_format, "PCX"))
    format = IM_PCX;
  else if (!imStrEqual(new_format, "RAS"))
    format = IM_RAS;
  else if (!imStrEqual(new_format, "SGI"))
    format = IM_SGI;
  else if (!imStrEqual(new_format, "JPEG"))
    format = IM_JPG;
  else if (!imStrEqual(new_format, "LED"))
    format = IM_LED;
  else if (!imStrEqual(new_format, "TIFF"))
    format = IM_TIF;
  else if (!imStrEqual(new_format, "TGA"))
    format = IM_TGA;
  else
    return -1;

  if (!imStrEqual(compression, "NONE"))
    format |= IM_DEFAULT;

  return format;
}

int imFileFormat(char *filename, int* format)
{
  char new_format[10], compression[10];
  int error, image_count;
  
  imFile* ifile = imFileOpen(filename, &error);
  if (!ifile) return error;
  
  imFileGetInfo(ifile, new_format, compression, &image_count);
  imFileClose(ifile);

  *format = FormatNew2Old(new_format, compression);
  if (*format == -1)
    return IM_ERR_FORMAT;

  return IM_ERR_NONE;
}

static int ColorMode2Type(int color_mode)
{
  switch (imColorModeSpace(color_mode))
  {
  case IM_BINARY:
  case IM_GRAY:
  case IM_MAP:
    return IM_MAP;
  default:
    return IM_RGB;
  }
}

int imImageInfo(char *filename, int *width, int *height, int *type, int *palette_count)
{
  int error;
  imFile* ifile = imFileOpen(filename, &error);
  if (!ifile) return error;

  int data_type, color_mode;
  error = imFileReadImageInfo(ifile, 0, width, height, &color_mode, &data_type);
  if (error)
  {
    imFileClose(ifile);
    return error;
  }

  *type = ColorMode2Type(color_mode);
  if (*type == -1)
  {
    imFileClose(ifile);
    return IM_ERR_DATA;
  }

  if (*type == IM_MAP)
  {
    long palette[256];
    imFileGetPalette(ifile, palette, palette_count);
  }

  imFileClose(ifile);
  return IM_ERR_NONE;
}

static imTiffImageDesc      iOldTiffImageDescCB = NULL;
static imGifTranspIndex     iOldGifTranspIndexCB = NULL;
static imResolutionCallback iOldResolutionCB = NULL;
static imFileCounterCallback    iOldCounterCB = NULL;

static int iOldFileCounter(int counter, void* user_data, const char* name, int progress)
{
  (void)counter;
  if (progress == -1 || progress == 1001) return 1;
  return !iOldCounterCB((char*)user_data, progress/10, (name[4] == 'R')? 0: 1);
}

int imRegisterCallback(imCallback cb, int cb_id, int format)
{
  if (format == IM_ALL)
  {
    switch(cb_id)
    {
    case IM_COUNTER_CB:
      iOldCounterCB = (imFileCounterCallback)cb;
      return 1;
    case IM_RESOLUTION_CB:
      iOldResolutionCB = (imResolutionCallback)cb;
      return 1;
    }
  }

  if (format == IM_GIF && cb_id == IM_GIF_TRANSPARENT_COLOR_CB)
  {
    iOldGifTranspIndexCB = (imGifTranspIndex)cb;
    return 1;
  }

  if (format == IM_TIF && cb_id == IM_TIF_IMAGE_DESCRIPTION_CB)
  {
    iOldTiffImageDescCB = (imTiffImageDesc)cb;
    return 1;
  }

  return 0;
}

static void iConvertMapToRGB(const imbyte* src_map, imbyte* red, imbyte* green, imbyte* blue, int count, const long* palette, const int palette_count)
{
  imbyte r[256], g[256], b[256];
  for (int c = 0; c < palette_count; c++)
    imColorDecode(&r[c], &g[c], &b[c], palette[c]);

  for (int i = 0; i < count; i++)
  {
    int index = *src_map++;
    *red++ = r[index];
    *green++ = g[index];
    *blue++ = b[index];
  }
}

int imLoadRGB(char *filename, unsigned char *red, unsigned char *green, unsigned char *blue)
{
  int error;
  imFile* ifile = imFileOpen(filename, &error);
  if (!ifile) return error;
  
  int width, height, color_mode, data_type;
  error = imFileReadImageInfo(ifile, 0, &width, &height, &color_mode, &data_type);
  if (error) 
  {
    imFileClose(ifile);
    return error;
  }

  if (iOldResolutionCB)
  {
    double xres = *(float*)imFileGetAttribute(ifile, "XResolution", NULL, NULL);
    double yres = *(float*)imFileGetAttribute(ifile, "YResolution", NULL, NULL);
    int res_unit = *(int*)imFileGetAttribute(ifile, "ResolutionUnit", NULL, NULL);
    iOldResolutionCB(filename, &xres, &yres, &res_unit);
  }

  if (iOldTiffImageDescCB)
  {
    char* img_desc = (char*)imFileGetAttribute(ifile, "Description", NULL, NULL);
    iOldTiffImageDescCB(filename, img_desc);
  }

  if (iOldGifTranspIndexCB)
  {
    unsigned char transp_index = *(unsigned char*)imFileGetAttribute(ifile, "TransparencyIndex", NULL, NULL);
    iOldGifTranspIndexCB(filename, &transp_index);
  }

  int count = width*height;
  void* data;
  if (green != red + count || blue != green + count)
    data = malloc(imImageDataSize(width, height, IM_RGB, IM_BYTE));
  else
    data = red;
    
  if (!data)
  {
    imFileClose(ifile);
    return IM_ERR_MEM;
  }

  if (iOldCounterCB)
    imCounterSetCallback(filename, iOldFileCounter);
  
  error = imFileReadImageData(ifile, data, 1, 0);
  if (error) 
  {
    if (data != red) free(data);
    imFileClose(ifile);
    return error;
  }

  if (imColorModeToBitmap(color_mode) != IM_RGB)
  {
    long palette[256];
    int palette_count;
    imFileGetPalette(ifile, palette, &palette_count);
    iConvertMapToRGB((imbyte*)data, red, green, blue, count, palette, palette_count);
  }
  else if (data != red)
  {
    memcpy(red, data, count);
    memcpy(green, (unsigned char*)data+count, count);
    memcpy(blue, (unsigned char*)data+2*count, count);
  }

  imFileClose(ifile);

  if (data != red) free(data);
  return IM_ERR_NONE;
}

int imLoadMap(char *filename, unsigned char *map, long *palette)
{
  int error;
  imFile* ifile = imFileOpen(filename, &error);
  if (!ifile) return error;
  
  int width, height, color_mode, data_type;
  error = imFileReadImageInfo(ifile, 0, &width, &height, &color_mode, &data_type);
  if (error)
  {
    imFileClose(ifile);
    return error;
  }

  if (imColorModeSpace(color_mode) != IM_MAP &&
      imColorModeSpace(color_mode) != IM_GRAY &&
      imColorModeSpace(color_mode) != IM_BINARY)
    return IM_ERR_DATA;

  if (iOldResolutionCB)
  {
    double xres = *(float*)imFileGetAttribute(ifile, "XResolution", NULL, NULL);
    double yres = *(float*)imFileGetAttribute(ifile, "YResolution", NULL, NULL);
    int res_unit = *(int*)imFileGetAttribute(ifile, "ResolutionUnit", NULL, NULL);
    iOldResolutionCB(filename, &xres, &yres, &res_unit);
  }

  if (iOldTiffImageDescCB)
  {
    char* img_desc = (char*)imFileGetAttribute(ifile, "Description", NULL, NULL);
    iOldTiffImageDescCB(filename, img_desc);
  }

  if (iOldGifTranspIndexCB)
  {
    unsigned char transp_index = *(unsigned char*)imFileGetAttribute(ifile, "TransparencyIndex", NULL, NULL);
    iOldGifTranspIndexCB(filename, &transp_index);
  }

  if (iOldCounterCB)
    imCounterSetCallback(filename, iOldFileCounter);

  error = imFileReadImageData(ifile, map, 1, 0);
  if (error)
  {
    imFileClose(ifile);
    return error;
  }

  int palette_count;
  imFileGetPalette(ifile, palette, &palette_count);

  imFileClose(ifile);

  return IM_ERR_NONE;
}

static char* i_format_old2new[] = {"BMP", "PCX", "GIF", "TIFF", "RAS", "SGI", "JPEG", "LED", "TGA"};

int imSaveRGB(int width, int height, int format, unsigned char *red, unsigned char *green, unsigned char *blue, char *filename)
{
  int error;
  char* new_format = i_format_old2new[format & 0x00FF];  
  
  imFile* ifile = imFileNew(filename, new_format, &error);
  if (!ifile) return error;
  
  if (format & 0xFF00)
    imFileSetInfo(ifile, NULL);
  else
    imFileSetInfo(ifile, "NONE");

  if (iOldResolutionCB)
  {
    double xres, yres;
    int res_unit;
    iOldResolutionCB(filename, &xres, &yres, &res_unit);
    float fxres=(float)xres, fyres=(float)yres;
    imFileSetAttribute(ifile, "XResolution", IM_FLOAT, 1, (void*)&fxres);
    imFileSetAttribute(ifile, "YResolution", IM_FLOAT, 1, (void*)&fyres);
    imFileSetAttribute(ifile, "ResolutionUnit", IM_INT, 1, (void*)&res_unit);
  }

  if (iOldTiffImageDescCB)
  {
    char img_desc[50];
    iOldTiffImageDescCB(filename, img_desc);
    imFileSetAttribute(ifile, "Description", IM_BYTE, -1, (void*)img_desc);
  }

  if (iOldGifTranspIndexCB)
  {
    unsigned char transp_index;
    iOldGifTranspIndexCB(filename, &transp_index);
    imFileSetAttribute(ifile, "TransparencyIndex", IM_BYTE, 1, (void*)&transp_index);
  }
  
  error = imFileWriteImageInfo(ifile, width, height, IM_RGB, IM_BYTE);
  if (error)
  {
    imFileClose(ifile);
    return error;
  }

  if (iOldCounterCB)
    imCounterSetCallback(filename, iOldFileCounter);
  
  int count = width*height;
  void* data;
  if (green != red + count || blue != green + count)
    data = malloc(imImageDataSize(width, height, IM_RGB, IM_BYTE));
  else
    data = red;

  if (!data)
  {
    imFileClose(ifile);
    return IM_ERR_MEM;
  }
  
  if (data != red)
  {
    memcpy(data, red, count);
    memcpy((unsigned char*)data+count, green, count);
    memcpy((unsigned char*)data+2*count, blue, count);
  }
 
  error = imFileWriteImageData(ifile, data);
  imFileClose(ifile);  
  if (data != red) free(data);
  return error;
}

int imSaveMap(int width, int height, int format, unsigned char *map, int palette_count, long *palette, char *filename)
{
  int error;
  char* new_format = i_format_old2new[format & 0x00FF];
  imFile* ifile = imFileNew(filename, new_format, &error);
  if (!ifile) return error;
  
  if (format & 0xFF00)
    imFileSetInfo(ifile, NULL);
  else
    imFileSetInfo(ifile, "NONE");

  imFileSetPalette(ifile, palette, palette_count);

  if (iOldResolutionCB)
  {
    double xres, yres;
    int res_unit;
    iOldResolutionCB(filename, &xres, &yres, &res_unit);
    float fxres=(float)xres, fyres=(float)yres;
    imFileSetAttribute(ifile, "XResolution", IM_FLOAT, 1, (void*)&fxres);
    imFileSetAttribute(ifile, "YResolution", IM_FLOAT, 1, (void*)&fyres);
    imFileSetAttribute(ifile, "ResolutionUnit", IM_INT, 1, (void*)&res_unit);
  }

  if (iOldTiffImageDescCB)
  {
    char img_desc[50];
    iOldTiffImageDescCB(filename, img_desc);
    imFileSetAttribute(ifile, "Description", IM_BYTE, -1, (void*)img_desc);
  }

  if (iOldGifTranspIndexCB)
  {
    unsigned char transp_index;
    iOldGifTranspIndexCB(filename, &transp_index);
    imFileSetAttribute(ifile, "TransparencyIndex", IM_BYTE, 1, (void*)&transp_index);
  }
  
  error = imFileWriteImageInfo(ifile, width, height, IM_MAP, IM_BYTE);
  if (error)
  {
    imFileClose(ifile);
    return error;
  }

  if (iOldCounterCB)
    imCounterSetCallback(filename, iOldFileCounter);
 
  error = imFileWriteImageData(ifile, map);
  imFileClose(ifile);  
  return error;
}
