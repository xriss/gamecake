/** \file
 * \brief ECW - ECW JPEG 2000
 *
 * See Copyright Notice in im_lib.h
 */

#include "im_format.h"
#include "im_util.h"
#include "im_format_ecw.h"
#include "im_counter.h"

#include <NCSECWClient.h>
// #include <NCSEcwCompressClient.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>

static const char* iECWCompTable[2] = 
{
  "ECW",
  "JPEG-2000",
};

class imFileFormatECW: public imFileFormatBase
{
  NCSFileView *pNCSFileView;
//  NCSEcwCompressClient *pClient;

public:
  imFileFormatECW(const imFormat* _iformat): imFileFormatBase(_iformat) {}
  ~imFileFormatECW() {}

  int Open(const char* file_name);
  int New(const char* file_name);
  void Close();
  void* Handle(int index);
  int ReadImageInfo(int index);
  int ReadImageData(void* data);
  int WriteImageInfo(){return 0;} // do nothing for now;
  int WriteImageData(void* data){(void)data; return 0;} // do nothing for now;
};

class imFormatECW: public imFormat
{
public:
  imFormatECW()
    :imFormat("ECW", 
              "ECW JPEG-2000 File Format", 
              "*.ecw;*.jp2;*.j2k;*.jpc;*.j2c;", 
              iECWCompTable, 
              2, 
              0)
    {}
  ~imFormatECW() {}

  imFileFormatBase* Create(void) const { return new imFileFormatECW(this); }
  int CanWrite(const char* compression, int color_mode, int data_type) const;
};

void imFormatRegisterECW(void)
{
  imFormatRegister(new imFormatECW());
}

int imFileFormatECW::Open(const char* file_name)
{
  NCSError eError = NCScbmOpenFileView((char*)file_name, &this->pNCSFileView, NULL);
  if (eError != NCS_SUCCESS) 
  {
    if (eError == NCS_FILE_OPEN_ERROR || 
        eError == NCS_FILE_NOT_FOUND || 
        eError == NCS_FILE_INVALID)
      return IM_ERR_OPEN;
    else if (eError == NCS_FILE_OPEN_FAILED)
      return IM_ERR_FORMAT;
    else
      return IM_ERR_ACCESS;
  }

  NCSFileType fileType = NCScbmGetFileType(this->pNCSFileView);
  if (fileType == NCS_FILE_ECW)
    strcpy(this->compression, "ECW");
  else if (fileType == NCS_FILE_JP2)
    strcpy(this->compression, "JPEG-2000");
  else
    return IM_ERR_COMPRESS;

  this->image_count = 1;

  return IM_ERR_NONE;
}

int imFileFormatECW::New(const char* file_name)
{
  strcpy(this->compression, "JPEG-2000");
  this->image_count = 1;

  (void)file_name;
  return IM_ERR_FORMAT;
}

void imFileFormatECW::Close()
{
  if (this->is_new)
    ;// NCSEcwCompressClose(this->pClient);
  else
    NCScbmCloseFileView(this->pNCSFileView);
}

void* imFileFormatECW::Handle(int index)
{
  (void)index;

  if (this->is_new)
    return NULL; // return (void*)this->pClient;
  else
    return (void*)this->pNCSFileView;
}

int imFileFormatECW::ReadImageInfo(int index)
{
  NCSFileViewFileInfoEx *pNCSFileInfo;
  imAttribTable* attrib_table = AttribTable();
  (void)index;

  if (NCScbmGetViewFileInfoEx(this->pNCSFileView, &pNCSFileInfo) != NCS_SUCCESS)
    return IM_ERR_ACCESS;

  this->width = pNCSFileInfo->nSizeX;
  this->height = pNCSFileInfo->nSizeY;

  switch(pNCSFileInfo->eColorSpace)
  {
  case NCSCS_GREYSCALE:
    this->file_color_mode = IM_GRAY;
    break;
  case NCSCS_YUV:
  case NCSCS_sRGB:
    this->file_color_mode = IM_RGB;
    break;
  case NCSCS_YCbCr:
    this->file_color_mode = IM_YCBCR;
    break;
  case NCSCS_MULTIBAND:
    /* multiband data, we read only one band */
    this->file_color_mode = IM_GRAY;
    attrib_table->Set("MultiBandCount", IM_USHORT, 1, (void*)&pNCSFileInfo->nBands);
    break;
  default: 
    return IM_ERR_DATA;
  }

  switch(pNCSFileInfo->eCellType)
  {
  case NCSCT_INT8:
  case NCSCT_UINT8:
    this->file_data_type = IM_BYTE;
    break;
  case NCSCT_INT16:
  case NCSCT_UINT16:
    this->file_data_type = IM_USHORT;
    break;
  case NCSCT_UINT64:
  case NCSCT_INT64:
  case NCSCT_UINT32:
  case NCSCT_INT32:
    // Should be:  this->file_data_type = IM_INT;  
    // but 32bits ints are not supported by the NCScbmReadViewLineBILEx function
    this->file_data_type = IM_USHORT;
    break;
  case NCSCT_IEEE4:
  case NCSCT_IEEE8:
    this->file_data_type = IM_FLOAT;
    break;
  default: 
    return IM_ERR_DATA;
  }

  int prec = pNCSFileInfo->pBands->nBits;
  if (prec < 8)
    this->convert_bpp = -prec; // just expand to 0-255

  if (prec == 1 && this->file_color_mode == IM_GRAY)
    this->file_color_mode = IM_BINARY;

  if (pNCSFileInfo->nBands > imColorModeDepth(this->file_color_mode))
    this->file_color_mode |= IM_ALPHA;

  if (this->file_color_mode != IM_GRAY)
    this->file_color_mode |= IM_PACKED;

  this->file_color_mode |= IM_TOPDOWN;

  float float_value = (float)pNCSFileInfo->fOriginX;
  attrib_table->Set("OriginX", IM_FLOAT, 1, (void*)&float_value);

  float_value = (float)pNCSFileInfo->fOriginY;
  attrib_table->Set("OriginY", IM_FLOAT, 1, (void*)&float_value);

  float_value = (float)pNCSFileInfo->fCWRotationDegrees;
  attrib_table->Set("Rotation", IM_FLOAT, 1, (void*)&float_value);

  float_value = (float)pNCSFileInfo->fCellIncrementX;
  attrib_table->Set("CellIncrementX", IM_FLOAT, 1, (void*)&float_value);

  float_value = (float)pNCSFileInfo->fCellIncrementY;
  attrib_table->Set("CellIncrementY", IM_FLOAT, 1, (void*)&float_value);

  attrib_table->Set("Datum", IM_BYTE, strlen(pNCSFileInfo->szDatum)+1, pNCSFileInfo->szDatum);
  attrib_table->Set("Projection", IM_BYTE, strlen(pNCSFileInfo->szProjection)+1, pNCSFileInfo->szProjection);

  switch (pNCSFileInfo->eCellSizeUnits)
  {
  case ECW_CELL_UNITS_INVALID:
    attrib_table->Set("CellUnits", IM_BYTE, 8, "INVALID");
    break;
  case ECW_CELL_UNITS_METERS:
    attrib_table->Set("CellUnits", IM_BYTE, 7, "METERS");
    break;
  case ECW_CELL_UNITS_DEGREES:
    attrib_table->Set("CellUnits", IM_BYTE, 7, "DEGREES");
    break;
  case ECW_CELL_UNITS_FEET:
    attrib_table->Set("CellUnits", IM_BYTE, 5, "FEET");
    break;
  case ECW_CELL_UNITS_UNKNOWN:
    attrib_table->Set("CellUnits", IM_BYTE, 8, "UNKNOWN");
    break;
  }

  float_value = (float)pNCSFileInfo->nCompressionRate;
  attrib_table->Set("CompressionRatio", IM_FLOAT, 1, (void*)&float_value);

  return IM_ERR_NONE;
}

static void iCopyDataBuffer(UINT8 **ppOutputLine, imbyte* line_buffer, int nBands, int view_width, int type_size)
{
  if (nBands > 1)
  {
    for(int i = 0; i < view_width; i++)
    {
      for(int j = 0; j < nBands; j++)
      {
        for(int k = 0; k < type_size; k++)
        {
          *line_buffer++ = (ppOutputLine[j])[i*type_size + k];
        }
      }
    }
  }
  else
    memcpy(line_buffer, ppOutputLine[0], nBands*type_size*view_width);
}

int imFileFormatECW::ReadImageData(void* data)
{
  imAttribTable* attrib_table = AttribTable();
  int i, *attrib_data, view_width, view_height,
    nBands = imColorModeDepth(this->file_color_mode);

  // this size is free, can be anything, but we restricted to less than the image size
  attrib_data = (int*)attrib_table->Get("ViewWidth");
  view_width = attrib_data? *attrib_data: this->width; 
  if (view_width > this->width) view_width = this->width;

  attrib_data = (int*)attrib_table->Get("ViewHeight");
  view_height = attrib_data? *attrib_data: this->height; 
  if (view_height > this->height) view_height = this->height;

  imCounterTotal(this->counter, view_height, "Reading ECW...");

  {
    int xmin, xmax, ymin, ymax, band_start;

    // full image if not defined.
    // this size must be inside the image
    attrib_data = (int*)attrib_table->Get("ViewXmin");
    xmin = attrib_data? *attrib_data: 0; 
    if (xmin < 0) xmin = 0;

    attrib_data = (int*)attrib_table->Get("ViewYmin");
    ymin = attrib_data? *attrib_data: 0; 
    if (ymin < 0) ymin = 0;

    attrib_data = (int*)attrib_table->Get("ViewXmax");
    xmax = attrib_data? *attrib_data: this->width-1; 
    if (xmax > this->width-1) xmax = this->width-1;

    attrib_data = (int*)attrib_table->Get("ViewYmax");
    ymax = attrib_data? *attrib_data: this->height-1; 
    if (ymax > this->height-1) ymax = this->height-1;
  
    band_start = 0;
    UINT16* start_plane = (UINT16*)attrib_table->Get("MultiBandSelect");
    if (start_plane)
      band_start = *start_plane;

    UINT32 *pBandList = (UINT32*)malloc(sizeof(UINT32)*nBands);
    for(i = 0; i < nBands; i++)
      pBandList[i] = i+band_start;

    NCSError eError = NCScbmSetFileView(this->pNCSFileView, nBands, pBandList,
                                        xmin, ymin, xmax, ymax,
                                        view_width, view_height);
    free(pBandList);

    if( eError != NCS_SUCCESS) 
      return IM_ERR_DATA;
  }

  // this is necessary to fool line buffer management
  this->width = view_width;
  this->height = view_height;
  this->line_buffer_size = imImageLineSize(this->width, this->file_color_mode, this->file_data_type);

  NCSEcwCellType eType = NCSCT_UINT8;
  int type_size = 1;
  if (this->file_data_type == IM_USHORT)
  {
    eType = NCSCT_UINT16;
    type_size = 2;
  }
  else if (this->file_data_type == IM_FLOAT)
  {
    eType = NCSCT_IEEE4;
    type_size = 4;
  }
  UINT8 **ppOutputLine = (UINT8**)malloc(sizeof(UINT8*)*nBands);
  UINT8 *ppOutputBuffer = (UINT8*)malloc(type_size*view_width*nBands);
  for(i = 0; i < nBands; i++)
    ppOutputLine[i] = ppOutputBuffer + i*type_size*view_width;

  for (int row = 0; row < view_height; row++)
  {
    NCSEcwReadStatus eError = NCScbmReadViewLineBILEx(this->pNCSFileView, eType, (void**)ppOutputLine);
    if( eError != NCS_SUCCESS)
    {
      free(ppOutputLine);
      free(ppOutputBuffer);
      return IM_ERR_DATA;
    }

    iCopyDataBuffer(ppOutputLine, (imbyte*)this->line_buffer, nBands, view_width, type_size);

    imFileLineBufferRead(this, data, row, 0);

    if (!imCounterInc(this->counter))
    {
      free(ppOutputLine);
      free(ppOutputBuffer);
      return IM_ERR_COUNTER;
    }
  }

  free(ppOutputLine);
  free(ppOutputBuffer);
  return IM_ERR_NONE;
}

int imFormatECW::CanWrite(const char* compression, int color_mode, int data_type) const
{
  (void)compression;
  (void)color_mode;
  (void)data_type;
  return IM_ERR_DATA;

  //int color_space = imColorModeSpace(color_mode);

  //if (color_space != IM_GRAY && color_space != IM_RGB)// && color_space != IM_LUV)
  //  return IM_ERR_DATA;                       
  //                                            
  //if (data_type != IM_BYTE && data_type != IM_USHORT && data_type != IM_FLOAT)
  //  return IM_ERR_DATA;

  //if (!compression || compression[0] == 0)
  //  return IM_ERR_NONE;

  //if (!imStrEqual(compression, "JPEG-2000"))
  //  return IM_ERR_COMPRESS;

  //return IM_ERR_NONE;
}
