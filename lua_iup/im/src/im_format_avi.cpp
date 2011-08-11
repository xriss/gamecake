/** \file
 * \brief AVI - Windows Audio-Video Interleaved RIFF
 *
 * See Copyright Notice in im_lib.h
 * $Id: im_format_avi.cpp,v 1.4 2009/06/14 17:02:34 scuri Exp $
 */

#include "im_format.h"
#include "im_format_avi.h"
#include "im_util.h"
#include "im_counter.h"

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <vfw.h> 

#include "im_dib.h"

#include <stdlib.h>
#include <string.h>
#include <memory.h>


static const char* iAVICompTable[15] = 
{
  "NONE",
  "RLE",      // Microsoft RLE
  "CINEPACK", // Cinepak Codec by Radius
  "MSVC",     // Microsoft Video 1
  "M261",     // Microsoft H.261 Video Codec
  "M263",     // Microsoft H.263 Video Codec
  "I420",     // Intel 4:2:0 Video Codec (same as M263)
  "IV32",     // Intel Indeo Video Codec 3.2
  "IV41",     // Intel Indeo Video Codec 4.5
  "IV50",     // Intel Indeo Video 5.1
  "IYUV",     // Intel IYUV Codec
  "MPG4",     // Microsoft MPEG-4 Video Codec V1
  "MP42",     // Microsoft MPEG-4 Video Codec V2
  "DIVX",     // DivX 5.0.4 Codec (must be installed)
  "CUSTOM"    // (show compression dialog)
};

class imFileFormatAVI: public imFileFormatBase
{
  PAVIFILE file;
  PAVISTREAM stream;

  imDib* dib;
  float fps;
  unsigned int rmask, gmask, bmask, 
                roff, goff, boff; /* pixel bit mask control when reading 16 and 32 bpp images */

  PGETFRAME frame;    // used when reading
  int current_frame;

  COMPVARS compvars;  // used when writing
  int use_compressor;

  void ReadPalette(unsigned char* bmp_colors);
  void WritePalette(unsigned char* bmp_colors);
  void FixRGBOrder(int bpp);
  void InitMasks(imDib* dib);

public:
  imFileFormatAVI(const imFormat* _iformat): imFileFormatBase(_iformat) {}
  ~imFileFormatAVI() {}

  int Open(const char* file_name);
  int New(const char* file_name);
  void Close();
  void* Handle(int index);
  int ReadImageInfo(int index);
  int ReadImageData(void* data);
  int WriteImageInfo();
  int WriteImageData(void* data);
};

class imFormatAVI: public imFormat
{
public:
  imFormatAVI()
    :imFormat("AVI", 
              "Windows Audio-Video Interleaved RIFF", 
              "*.avi;", 
              iAVICompTable, 
              15, 
              1)
  {}
  ~imFormatAVI() {}

  imFileFormatBase* Create(void) const { return new imFileFormatAVI(this); }
  int CanWrite(const char* compression, int color_mode, int data_type) const;
};

void imFormatRegisterAVI(void)
{
  imFormatRegister(new imFormatAVI());
}

int imFileFormatAVI::Open(const char* file_name)
{
  /* initializes avi file library, can be called many times */
  AVIFileInit();

  /* open existing file */
  HRESULT hr = AVIFileOpen(&file, file_name, OF_READ, NULL);
  if (hr != 0)
  {
    AVIFileExit();

    if (hr == AVIERR_FILEOPEN)
      return IM_ERR_OPEN;
    else if (hr == AVIERR_BADFORMAT || hr == REGDB_E_CLASSNOTREG)
      return IM_ERR_FORMAT;
    else
      return IM_ERR_ACCESS;
  }

  /* get the video stream */
  hr = AVIFileGetStream(file, &stream, streamtypeVIDEO, 0);
  if (hr != 0)
  {
    AVIFileRelease(this->file);
    AVIFileExit();

    if (hr == AVIERR_NODATA)
      return IM_ERR_DATA;
    else
      return IM_ERR_ACCESS;
  }

  /* get stream info */
  AVISTREAMINFO streaminfo;
  AVIStreamInfo(stream, &streaminfo, sizeof(AVISTREAMINFO));

  this->image_count = streaminfo.dwLength;
  this->fps = (float)streaminfo.dwRate / (float)streaminfo.dwScale;

  if (streaminfo.fccHandler == mmioFOURCC('D','I','B',' '))
    strcpy(this->compression, "NONE");
  else if (streaminfo.fccHandler == mmioFOURCC('M','R','L','E'))
    strcpy(this->compression, "RLE");
  else if (streaminfo.fccHandler == mmioFOURCC('c','v','i','d'))
    strcpy(this->compression, "CINEPACK");    
  else
  {
    DWORD handler = streaminfo.fccHandler;
    this->compression[0] = (char)handler;
    this->compression[1] = (char)(handler >> 8);
    this->compression[2] = (char)(handler >> 16);
    this->compression[3] = (char)(handler >> 24);
    this->compression[4] = 0;
  }

  this->frame = 0;
  this->use_compressor = 0;
  this->dib = 0;
  this->current_frame = 0;

  return IM_ERR_NONE;
}

int imFileFormatAVI::New(const char* file_name)
{
  /* initializes avi file library, can be called many times */
  AVIFileInit();

  /* creates a new file */
  HRESULT hr = AVIFileOpen(&file, file_name, OF_WRITE | OF_CREATE, NULL);
  if (hr != 0)
  {
    AVIFileExit();

    if (hr == AVIERR_FILEOPEN)
      return IM_ERR_OPEN;
    else if (hr == AVIERR_BADFORMAT || hr == REGDB_E_CLASSNOTREG)
      return IM_ERR_FORMAT;
    else
      return IM_ERR_ACCESS;
  }

  this->frame = 0;
  this->stream = 0;
  this->use_compressor = 0;
  this->dib = 0;

  return IM_ERR_NONE;
}

void imFileFormatAVI::Close()
{
  if (this->dib) imDibDestroy(this->dib);

  if (this->use_compressor) 
  {
    ICSeqCompressFrameEnd(&this->compvars);
    ICCompressorFree(&this->compvars);
  }

  if (this->frame) AVIStreamGetFrameClose(this->frame);
  if (this->stream) AVIStreamRelease(this->stream);

  AVIFileRelease(this->file);
  AVIFileExit();    /* called one for each AVIFileInit */
}

void* imFileFormatAVI::Handle(int index)
{
  if (index == 1)
    return (void*)this->file;
  else if (index == 2)
    return (void*)this->stream;
  else
    return NULL;
}

int imFileFormatAVI::ReadImageInfo(int index)
{
  this->current_frame = index;

  if (this->frame)       // frame reading already prepared
    return IM_ERR_NONE;

  /* get stream format */
  LONG formsize;
  AVIStreamReadFormat(stream, 0, NULL, &formsize);
  BITMAPINFO *bmpinfo = (BITMAPINFO*)malloc(formsize);
  HRESULT hr = AVIStreamReadFormat(stream, 0, bmpinfo, &formsize);
  if (hr != 0)
  {
    free(bmpinfo);
    return IM_ERR_ACCESS;
  }

  int top_down = 0;
  if (bmpinfo->bmiHeader.biHeight < 0)
    top_down = 1;

  this->width = bmpinfo->bmiHeader.biWidth;
  this->height = top_down? -bmpinfo->bmiHeader.biHeight: bmpinfo->bmiHeader.biHeight;

  int bpp = bmpinfo->bmiHeader.biBitCount;

  imAttribTable* attrib_table = AttribTable();
  attrib_table->Set("FPS", IM_FLOAT, 1, &fps);

  this->file_data_type = IM_BYTE;

  if (bpp > 8)
  {
    this->file_color_mode = IM_RGB;
    this->file_color_mode |= IM_PACKED;
  }
  else
  {
    this->palette_count = 1 << bpp;
    this->file_color_mode = IM_MAP;
  }

  if (bpp < 8)
    this->convert_bpp = bpp;

  if (bpp == 32)
    this->file_color_mode |= IM_ALPHA;

  if (top_down)
    this->file_color_mode |= IM_TOPDOWN;

  if (bpp <= 8)
  {
    /* updates the palette_count based on the number of colors used */
    if (bmpinfo->bmiHeader.biClrUsed != 0 && 
        (int)bmpinfo->bmiHeader.biClrUsed < this->palette_count)
      this->palette_count = bmpinfo->bmiHeader.biClrUsed;

    ReadPalette((unsigned char*)bmpinfo->bmiColors);
  }

  free(bmpinfo);

  this->line_buffer_extra = 4; // room enough for padding

  /* prepares to read data from the stream */
  if (bpp == 32 || bpp == 16)
  {
    BITMAPINFOHEADER info;
    memset(&info, 0, sizeof(BITMAPINFOHEADER));
    info.biSize = sizeof(BITMAPINFOHEADER);
    info.biWidth = width;
    info.biHeight = height;
    info.biPlanes = 1;
    info.biBitCount = (WORD)bpp;
    frame = AVIStreamGetFrameOpen(stream, &info);
  }
  else
    frame = AVIStreamGetFrameOpen(stream, NULL);

  if (!frame)
    return IM_ERR_ACCESS;

  return IM_ERR_NONE;
}

int imFileFormatAVI::WriteImageInfo()
{
  if (dib)
  {
    if (dib->bmih->biWidth != width || dib->bmih->biHeight != height ||
        imColorModeSpace(file_color_mode) != imColorModeSpace(user_color_mode))
      return IM_ERR_DATA;

    return IM_ERR_NONE;  // parameters can be set only once
  }

  // force bottom up orientation
  this->file_data_type = IM_BYTE;
  this->file_color_mode = imColorModeSpace(this->user_color_mode);

  int bpp;
  if (this->file_color_mode == IM_RGB)
  {
    this->file_color_mode |= IM_PACKED;
    bpp = 24;

    if (imColorModeHasAlpha(this->user_color_mode))
    {
      this->file_color_mode |= IM_ALPHA;
      bpp = 32;

      this->rmask = 0x00FF0000;
      this->roff = 16;

      this->gmask = 0x0000FF00;
      this->goff = 8;

      this->bmask = 0x000000FF;
      this->boff = 0;
    }
  }
  else
    bpp = 8;

  this->line_buffer_extra = 4; // room enough for padding

  imAttribTable* attrib_table = AttribTable();

  const void* attrib_data = attrib_table->Get("FPS");
  if (attrib_data)
    fps = *(float*)attrib_data;
  else
    fps = 15;

  if (this->compression[0] == 0 || imStrEqual(this->compression, "NONE"))
    this->use_compressor = 0;
  else
    this->use_compressor = 1;

  dib = imDibCreate(width, height, bpp);

  if (use_compressor)
  {
    memset(&compvars, 0, sizeof(COMPVARS));
    compvars.cbSize = sizeof(COMPVARS);

    if (imStrEqual(this->compression, "CUSTOM"))
    {
      if (ICCompressorChoose(NULL, ICMF_CHOOSE_DATARATE | ICMF_CHOOSE_KEYFRAME, dib->dib, NULL, &compvars, "Choose Compression") == FALSE)
        return IM_ERR_COMPRESS;
    }
    else
    {
      compvars.dwFlags = ICMF_COMPVARS_VALID;
      compvars.fccType = ICTYPE_VIDEO;

      int* attrib = (int*)attrib_table->Get("KeyFrameRate");
      if (attrib)
        compvars.lKey = *attrib;
      else
        compvars.lKey = 15;        // same defaults of the dialog

      attrib = (int*)attrib_table->Get("DataRate");
      if (attrib)
        compvars.lDataRate = *attrib / 8;
      else
        compvars.lDataRate = 300;  // same defaults of the dialog

      attrib = (int*)attrib_table->Get("AVIQuality");
      if (attrib)
        compvars.lQ = *attrib;
      else
        compvars.lQ = (DWORD)ICQUALITY_DEFAULT;

      if (imStrEqual(this->compression, "RLE"))
        compvars.fccHandler = mmioFOURCC('M','R','L','E');
      else if (imStrEqual(this->compression, "CINEPACK"))
        compvars.fccHandler = mmioFOURCC('c','v','i','d');    
      else
        compvars.fccHandler = mmioFOURCC(compression[0],compression[1],compression[2],compression[3]);

      compvars.hic = ICOpen(ICTYPE_VIDEO, compvars.fccHandler, ICMODE_COMPRESS);
    }

    if (compvars.hic == NULL)
      use_compressor = 0;
  }

  AVISTREAMINFO streaminfo;
  memset(&streaminfo, 0, sizeof(AVISTREAMINFO));
  streaminfo.fccType = streamtypeVIDEO;
  streaminfo.dwScale = 1000;
  streaminfo.dwRate  = (DWORD)(fps*1000);
  SetRect(&streaminfo.rcFrame, 0, 0, width, height);

  if (use_compressor)
  {
    streaminfo.fccHandler = compvars.fccHandler;
    streaminfo.dwQuality = compvars.lQ;
  }
  else
  {
    streaminfo.fccHandler = mmioFOURCC('D','I','B',' ');
    streaminfo.dwQuality = (DWORD)ICQUALITY_DEFAULT;
  }

  /* creates a new stream in the new file */
  HRESULT hr = AVIFileCreateStream(file, &stream, &streaminfo);         
  if (hr != 0)
    return IM_ERR_ACCESS;

  /* set stream format */
  if (use_compressor)
  {
    if (!ICSeqCompressFrameStart(&compvars, dib->bmi))
      return IM_ERR_COMPRESS;

    hr = AVIStreamSetFormat(stream, 0, compvars.lpbiOut, dib->size - dib->bits_size); 
  }
  else
    hr = AVIStreamSetFormat(stream, 0, dib->dib, dib->size - dib->bits_size); 

  if (hr != 0)
    return IM_ERR_ACCESS;

  return IM_ERR_NONE;
}

void imFileFormatAVI::ReadPalette(unsigned char* bmp_colors)
{
  /* convert the color map to the IM format */
  for (int c = 0; c < this->palette_count; c++)
  {
    int i = c * 4;
    this->palette[c] = imColorEncode(bmp_colors[i + 2], 
                                     bmp_colors[i + 1], 
                                     bmp_colors[i]);
  }
}

void imFileFormatAVI::WritePalette(unsigned char* bmp_colors)
{
  /* convert the color map to the IM format */
  for (int c = 0; c < this->palette_count; c++)
  {
    int i = c * 4;                       
    imColorDecode(&bmp_colors[i + 2], &bmp_colors[i + 1], &bmp_colors[i], this->palette[c]);
    bmp_colors[i + 3] = 0;
  }
}

void imFileFormatAVI::InitMasks(imDib* dib)
{
  if (dib->bmih->biCompression == BI_BITFIELDS)
  {
    unsigned int Mask;
    unsigned int *PalMask = (unsigned int*)dib->bmic;

    this->roff = 0;
    this->rmask = Mask = PalMask[0];
    while (!(Mask & 0x01) && (Mask != 0))
      {Mask >>= 1; this->roff++;}

    this->goff = 0;
    this->gmask = Mask = PalMask[1];
    while (!(Mask & 0x01) && (Mask != 0))
      {Mask >>= 1; this->goff++;}

    this->boff = 0;
    this->bmask = Mask = PalMask[2];
    while (!(Mask & 0x01) && (Mask != 0))
      {Mask >>= 1; this->boff++;}
  }
  else
  {
    if (dib->bmih->biBitCount == 16)
    {                   
      this->rmask = 0x7C00;
      this->roff = 10;

      this->gmask = 0x03E0;
      this->goff = 5;

      this->bmask = 0x001F;
      this->boff = 0;
    }
    else
    {
      this->rmask = 0x00FF0000;
      this->roff = 16;

      this->gmask = 0x0000FF00;
      this->goff = 8;

      this->bmask = 0x000000FF;
      this->boff = 0;
    }
  }
}

void imFileFormatAVI::FixRGBOrder(int bpp)
{
  int x;

  switch (bpp)
  {
  case 16:
    {
      /* inverts the WORD values if not intel */
      if (imBinCPUByteOrder() == IM_BIGENDIAN)
        imBinSwapBytes2(this->line_buffer, this->width);

      imushort* word_data = (imushort*)this->line_buffer;
      imbyte* byte_data = (imbyte*)this->line_buffer;

      // from end to start
      for (x = this->width-1; x >= 0; x--)
      {
        imushort word_value = word_data[x];
        int c = x*3;
        byte_data[c]   = (imbyte)((((rmask & word_value) >> roff) * 255) / (rmask >> roff));
        byte_data[c+1] = (imbyte)((((gmask & word_value) >> goff) * 255) / (gmask >> goff));
        byte_data[c+2] = (imbyte)((((bmask & word_value) >> boff) * 255) / (bmask >> boff));
      }
    }
    break;
  case 32:
    {
      /* inverts the DWORD values if not intel */
      if (imBinCPUByteOrder() == IM_BIGENDIAN)
        imBinSwapBytes4(this->line_buffer, this->width);

      unsigned int* dword_data = (unsigned int*)this->line_buffer;
      imbyte* byte_data = (imbyte*)this->line_buffer;

      for (x = 0; x < this->width; x++)
      {
        unsigned int dword_value = dword_data[x];
        int c = x*4;
        byte_data[c]   = (imbyte)((rmask & dword_value) >> roff);
        byte_data[c+1] = (imbyte)((gmask & dword_value) >> goff);
        byte_data[c+2] = (imbyte)((bmask & dword_value) >> boff);
        byte_data[c+3] = (imbyte)((0xFF000000 & dword_value) >> 24);
      }
    }
    break;
  default: // 24
    {
      imbyte* byte_data = (imbyte*)this->line_buffer;
      for (x = 0; x < this->width; x++)
      {
        int c = x*3;
        imbyte temp = byte_data[c];     // swap R and B
        byte_data[c] = byte_data[c+2];
        byte_data[c+2] = temp;
      }
    }
    break;
  }
}

int imFileFormatAVI::ReadImageData(void* data)
{
  imCounterTotal(this->counter, this->height, "Reading AVI Frame...");

  void* packed_dib = AVIStreamGetFrame(this->frame, this->current_frame);
  if (!packed_dib)
    return IM_ERR_ACCESS;

  dib = imDibCreateReference((imbyte*)packed_dib, NULL);

  if (dib->bmih->biBitCount == 16 || dib->bmih->biBitCount == 32)
    InitMasks(dib);
  else if (dib->bmih->biBitCount <= 8)
  {
    this->palette_count = dib->palette_count;
    ReadPalette((unsigned char*)dib->bmic);
  }

  imbyte* bits = dib->bits;
  for (int row = 0; row < this->height; row++)
  {
    CopyMemory(this->line_buffer, bits, dib->line_size);
    bits += dib->line_size;

    if (dib->bmih->biBitCount > 8)
      FixRGBOrder(dib->bmih->biBitCount);

    imFileLineBufferRead(this, data, row, 0);

    if (!imCounterInc(this->counter))
    {
      imDibDestroy(dib);
      dib = NULL;
      return IM_ERR_COUNTER;
    }
  }

  imDibDestroy(dib);
  dib = NULL;
  this->current_frame++;

  return IM_ERR_NONE;
}

int imFileFormatAVI::WriteImageData(void* data)
{
  imCounterTotal(this->counter, this->height, "Writing AVI Frame...");

  if (dib->bmih->biBitCount <= 8)
  {
    WritePalette((unsigned char*)dib->bmic);

    /* this must be called here to update the palette */
    AVIStreamSetFormat(this->stream, 0, dib->dib, dib->size - dib->bits_size);
  }

  imbyte* bits = dib->bits;
  for (int row = 0; row < this->height; row++)
  {
    imFileLineBufferWrite(this, data, row, 0);

    if (dib->bmih->biBitCount > 8)
      FixRGBOrder(dib->bmih->biBitCount);

    CopyMemory(bits, this->line_buffer, dib->line_size);
    bits += dib->line_size;

    if (!imCounterInc(this->counter))
      return IM_ERR_COUNTER;
  }

  bits = dib->bits;
  LONG bits_size = dib->bits_size; 
  DWORD flags = 0;

  if (this->use_compressor)
  {
    BOOL key = FALSE;
    bits = (imbyte*)ICSeqCompressFrame(&this->compvars, 0, bits, &key, &bits_size);
    if (key == TRUE)
      flags = AVIIF_KEYFRAME;

    if (!bits)
    {
      bits = dib->bits;
      bits_size = dib->bits_size; 
    }
  }
                                               
  HRESULT hr = AVIStreamWrite(this->stream, this->image_count, 1, bits, bits_size, flags, NULL, NULL);
  if (hr != 0)
    return IM_ERR_ACCESS;

  this->image_count++;

  return IM_ERR_NONE;
}

int imFormatAVI::CanWrite(const char* compression, int color_mode, int data_type) const
{
  (void)compression;

  int color_space = imColorModeSpace(color_mode);

  if (color_space == IM_YCBCR || color_space == IM_LAB || 
      color_space == IM_LUV || color_space == IM_XYZ ||
      color_space == IM_CMYK)
    return IM_ERR_DATA;                       
                                              
  if (data_type != IM_BYTE)
    return IM_ERR_DATA;

  return IM_ERR_NONE;
}
