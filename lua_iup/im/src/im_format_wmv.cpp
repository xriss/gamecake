/** \file
 * \brief WMV - Windows Media Video Format
 *
 * See Copyright Notice in im_lib.h
 * $Id: im_format_wmv.cpp,v 1.3 2009/08/23 23:57:52 scuri Exp $
 */

#include "im_format.h"
#include "im_util.h"
#include "im_format_wmv.h"
#include "im_counter.h"

#include <wmsdk.h>

//#include <Dvdmedia.h>
#define AMINTERLACE_1FieldPerSample  0x00000002
#define AMINTERLACE_Field1First      0x00000004

#include "im_dib.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>


#define SAFE_RELEASE( x )   \
    if ( x )                \
    {                       \
        x->Release();       \
        x = NULL;           \
    }

#define SAFE_ARRAYDELETE( x )   \
    if ( x )                    \
    {                           \
        delete[] x;             \
        x = NULL;               \
    }

static HRESULT iConfigCompressedStream( IWMStreamConfig * pStreamConfig,
                                        IWMProfile * pIWMProfile,
                                        BOOL fIsVBR, DWORD dwBitrate, DWORD dwQuality, DWORD dwSecPerKey,
                                        WM_MEDIA_TYPE * pmt )
{
  WORD wFALSE = 0;
  HRESULT hr = S_OK;

  do
  {
    // This is used just to get the stream number, it will be released and
    // NOT added to the profile
    IWMStreamConfig * pStreamConfig2 = NULL;
    hr = pIWMProfile->CreateNewStream( WMMEDIATYPE_Video, &pStreamConfig2 );
    if (FAILED(hr))
      break;

    WORD wStreamNum = 0;
    hr = pStreamConfig2->GetStreamNumber( &wStreamNum );

    SAFE_RELEASE( pStreamConfig2 );

    if (FAILED(hr))
      break;

    // Configure the stream

    hr = pStreamConfig->SetStreamNumber( wStreamNum );
    if (FAILED(hr))
      break;

    hr = pStreamConfig->SetStreamName( L"Video Stream" );
    if (FAILED(hr))
      break;

    // Each stream in the profile has to have a unique connection name.
    // Let's use the stream number to create it.

    WCHAR pwszConnectionName[10];
    swprintf( pwszConnectionName, L"Video%d", (DWORD)wStreamNum );

    hr = pStreamConfig->SetConnectionName( pwszConnectionName );
    if (FAILED(hr))
      break;

    hr = pStreamConfig->SetBitrate( dwBitrate );
    if (FAILED(hr))
      break;

    hr = pStreamConfig->SetBufferWindow( (DWORD)-1 );
    if (FAILED(hr))
      break;

    IWMVideoMediaProps * pIWMMediaProps = NULL;
    hr = pStreamConfig->QueryInterface( IID_IWMVideoMediaProps, (void **) &pIWMMediaProps );
    if (FAILED(hr))
      break;

    hr = pIWMMediaProps->SetQuality( dwQuality );
    hr = pIWMMediaProps->SetMaxKeyFrameSpacing( 10000 * (QWORD)dwSecPerKey );

    hr = pIWMMediaProps->SetMediaType( pmt );

    SAFE_RELEASE( pIWMMediaProps );

    if (FAILED(hr))
      break;

    IWMPropertyVault* pPropertyVault = NULL;
    hr = pStreamConfig->QueryInterface( IID_IWMPropertyVault, (void**)&pPropertyVault ); 
    if (FAILED(hr))
      break;

    hr = pPropertyVault->SetProperty( g_wszVBREnabled, WMT_TYPE_BOOL, (BYTE*)&fIsVBR, sizeof( BOOL ) );
    if ( SUCCEEDED( hr ) && fIsVBR)
      pPropertyVault->SetProperty( g_wszVBRQuality, WMT_TYPE_DWORD, (BYTE*)&dwQuality, sizeof( DWORD ) );

    SAFE_RELEASE( pPropertyVault );

    hr = S_OK;

  } while( wFALSE );

  return( hr );
}

static HRESULT iCreateCompressedStream(IWMProfileManager * pManager,
                                       IWMStreamConfig* *pNewStreamConfig,
                                       WM_MEDIA_TYPE* *pNewMediaType,
                                       WORD biBitCount, GUID subtype)
{
  IWMCodecInfo  * pCodecInfo = NULL;
  IWMMediaProps  * pMediaProps = NULL;

  IWMStreamConfig* pStreamConfig = NULL;
  WM_MEDIA_TYPE* pMediaType = NULL;

  HRESULT hr = S_OK;
  WORD wFALSE = 0;

  do
  {
    hr = pManager->QueryInterface(IID_IWMCodecInfo, (void **) &pCodecInfo);
    if (FAILED(hr))
      break;

    DWORD cCodecs;
    hr = pCodecInfo->GetCodecInfoCount( WMMEDIATYPE_Video, &cCodecs );
    if (FAILED(hr))
      break;

    for( int i = cCodecs-1; i >= 0; i-- )
    {
      DWORD cFormats;
      hr = pCodecInfo->GetCodecFormatCount( WMMEDIATYPE_Video, i, &cFormats );
      if (FAILED(hr))
        break;

      for(DWORD j = 0; j < cFormats; j++ )
      {
        SAFE_RELEASE( pStreamConfig );

        hr = pCodecInfo->GetCodecFormat( WMMEDIATYPE_Video, i, j, &pStreamConfig );
        if (FAILED(hr))
          break;

        SAFE_RELEASE( pMediaProps );

        hr = pStreamConfig->QueryInterface( IID_IWMMediaProps, (void **) &pMediaProps );
        if (FAILED(hr))
          break;

        DWORD cbMT;
        hr = pMediaProps->GetMediaType( NULL, &cbMT );
        if (FAILED(hr))
          break;

        SAFE_ARRAYDELETE( pMediaType );

        pMediaType = (WM_MEDIA_TYPE *) new BYTE[ cbMT ];
        if( !pMediaType )
        {
          hr = E_OUTOFMEMORY;
          break;
        }

        hr = pMediaProps->GetMediaType( pMediaType, &cbMT );
        if (FAILED(hr))
          break;

        if( pMediaType->formattype != WMFORMAT_VideoInfo ||
            pMediaType->subtype != subtype)  // This is our main target
        {
          SAFE_RELEASE( pStreamConfig );
          continue;
        }

        WMVIDEOINFOHEADER* pVIH = (WMVIDEOINFOHEADER*) pMediaType->pbFormat;

        if( pVIH->bmiHeader.biBitCount >= biBitCount )
          break; // SUCCESS !!!!!

        SAFE_RELEASE( pStreamConfig );
      }

      if( FAILED( hr ) || NULL != pStreamConfig )
        break;
    }

    if (FAILED(hr))
      break;

    if( NULL == pStreamConfig )
    {
      hr = NS_E_VIDEO_CODEC_NOT_INSTALLED;
      break;
    }

  } while( wFALSE );

  SAFE_RELEASE( pCodecInfo );
  SAFE_RELEASE( pMediaProps );

  *pNewStreamConfig = pStreamConfig;
  *pNewMediaType = pMediaType;

  return( hr );
}

static HRESULT iAddCompressedVideoStream( IWMProfileManager * pManager, IWMProfile * pIWMProfile,
                                          GUID subtype, BITMAPINFOHEADER * bmiHeader, float fps,
                                          BOOL fIsVBR, DWORD dwBitRate, DWORD dwQuality, DWORD dwSecPerKey)
{
  HRESULT hr = S_OK;
  WORD wFALSE = 0;

  IWMStreamConfig* pStreamConfig = NULL;
  WM_MEDIA_TYPE* pMediaType = NULL;

  do
  {
    hr = iCreateCompressedStream(pManager, &pStreamConfig, &pMediaType,
                                 bmiHeader->biBitCount, subtype);
    if (FAILED(hr))
      break;

    WMVIDEOINFOHEADER * pVIH = (WMVIDEOINFOHEADER *) pMediaType->pbFormat;

    pVIH->dwBitRate = dwBitRate;

    // Video content does not play correctly unless it is encoded 
    // to a size that is a multiple of four for both width and height. 
    pVIH->bmiHeader.biWidth = ((bmiHeader->biWidth + 3) / 4) * 4;
    pVIH->bmiHeader.biHeight = ((bmiHeader->biHeight + 3) / 4) * 4;

    pVIH->rcSource.left = 0;
    pVIH->rcSource.top = 0;
    pVIH->rcSource.bottom = pVIH->bmiHeader.biHeight;
    pVIH->rcSource.right = pVIH->bmiHeader.biWidth;
    pVIH->rcTarget = pVIH->rcSource;
    pVIH->dwBitErrorRate = 0;
    pVIH->AvgTimePerFrame = (LONGLONG)(10000000.0f / fps);

    hr = iConfigCompressedStream( pStreamConfig, pIWMProfile, 
                                  fIsVBR, dwBitRate, dwQuality, 
                                  dwSecPerKey, pMediaType );
    if (FAILED(hr))
      break;

    hr = pIWMProfile->AddStream( pStreamConfig );
    if (FAILED(hr))
      break;
  }
  while( wFALSE );

  SAFE_RELEASE( pStreamConfig );
  SAFE_ARRAYDELETE( pMediaType );

  return( hr );
}

static HRESULT iConfigUncompressedStream( IWMStreamConfig * pStreamConfig,
                                          DWORD dwBitrate,
                                          WM_MEDIA_TYPE * pmt )
{
  WORD wFALSE = 0;
  HRESULT hr = S_OK;

  do
  {
    // Configure the stream

    hr = pStreamConfig->SetStreamName( L"Video Stream" );
    if (FAILED(hr))
      break;

    // Each stream in the profile has to have a unique connection name.
    // Let's use the stream number to create it.

    WORD wStreamNum = 0;
    hr = pStreamConfig->GetStreamNumber( &wStreamNum );
    if (FAILED(hr))
      break;

    WCHAR pwszConnectionName[10];
    swprintf( pwszConnectionName, L"Video%d", (DWORD)wStreamNum );

    hr = pStreamConfig->SetConnectionName( pwszConnectionName );
    if (FAILED(hr))
      break;

    hr = pStreamConfig->SetBitrate( dwBitrate );
    if (FAILED(hr))
      break;

    hr = pStreamConfig->SetBufferWindow( 0 );
    if (FAILED(hr))
      break;

    IWMMediaProps * pIWMMediaProps = NULL;
    hr = pStreamConfig->QueryInterface( IID_IWMMediaProps, (void **) &pIWMMediaProps );
    if (FAILED(hr))
      break;

    hr = pIWMMediaProps->SetMediaType( pmt );

    SAFE_RELEASE( pIWMMediaProps );

    if (FAILED(hr))
      break;

    IWMPropertyVault* pPropertyVault = NULL;
    hr = pStreamConfig->QueryInterface( IID_IWMPropertyVault, (void**)&pPropertyVault ); 
    if (FAILED(hr))
      break;

    BOOL fFalse = FALSE;
    hr = pPropertyVault->SetProperty( g_wszVBREnabled, WMT_TYPE_BOOL, (BYTE*)&fFalse, sizeof( BOOL ) );

    SAFE_RELEASE( pPropertyVault );

    hr = S_OK;

  } while( wFALSE );

  return( hr );
}

static HRESULT iAddUncompressedVideoStream( IWMProfile * pProfile, 
                                            BITMAPINFOHEADER * bmiHeader, 
                                            int BitmapInfoSize, int BitmapDataSize,
                                            float fps)
{
  HRESULT hr = S_OK;
  WORD wFALSE = 0;

  IWMStreamConfig* pStreamConfig = NULL;
  WM_MEDIA_TYPE* pMediaType = NULL;

  do
  {
    hr = pProfile->CreateNewStream( WMMEDIATYPE_Video, &pStreamConfig );
    if ( FAILED( hr ) )
      break;

    DWORD cbVideoInfo = sizeof(WMVIDEOINFOHEADER) - sizeof(BITMAPINFOHEADER) + BitmapInfoSize;

    // Create a new Media Type
    pMediaType = (WM_MEDIA_TYPE*) new BYTE[ sizeof( WM_MEDIA_TYPE ) + cbVideoInfo ];
    if ( !pMediaType)
    {
      hr = E_OUTOFMEMORY;
      break;
    }

    switch (bmiHeader->biBitCount)
    {
    case 32:
        pMediaType->subtype = WMMEDIASUBTYPE_RGB32;
        break;
    case 24:
        pMediaType->subtype = WMMEDIASUBTYPE_RGB24;
        break;
    case 8:
        pMediaType->subtype = WMMEDIASUBTYPE_RGB8;
        break;
    }

    pMediaType->majortype = WMMEDIATYPE_Video;
    pMediaType->bFixedSizeSamples = TRUE;
    pMediaType->bTemporalCompression = FALSE;
    pMediaType->lSampleSize = BitmapDataSize;
    pMediaType->formattype = WMFORMAT_VideoInfo;
    pMediaType->pUnk = NULL;
    pMediaType->cbFormat = cbVideoInfo;
    pMediaType->pbFormat = ( ((BYTE*) pMediaType) + sizeof( WM_MEDIA_TYPE ) ); // Format data is immediately after media type

    WMVIDEOINFOHEADER * pVIH = (WMVIDEOINFOHEADER *) pMediaType->pbFormat;

    pVIH->rcSource.left = 0;
    pVIH->rcSource.top = 0;
    pVIH->rcSource.bottom = bmiHeader->biHeight;
    pVIH->rcSource.right = bmiHeader->biWidth;
    pVIH->rcTarget = pVIH->rcSource;
    pVIH->dwBitRate = (DWORD)(BitmapDataSize * fps);
    pVIH->dwBitErrorRate = 0;
    pVIH->AvgTimePerFrame = (LONGLONG)(10000000.0f / fps);

    CopyMemory(&pVIH->bmiHeader, bmiHeader, BitmapInfoSize);

    hr = iConfigUncompressedStream( pStreamConfig, pVIH->dwBitRate, pMediaType );
    if (FAILED(hr))
      break;

    hr = pProfile->AddStream( pStreamConfig );
    if (FAILED(hr))
      break;
  }
  while( wFALSE );

  SAFE_RELEASE( pStreamConfig );
  SAFE_ARRAYDELETE( pMediaType );

  return( hr );
}

#define WMV_COMPRESS_COUNT 7
#define WMV_UNCOMPRESS_COUNT 9

static GUID iWMVCompSubtypeTable[WMV_COMPRESS_COUNT+WMV_UNCOMPRESS_COUNT];

static void iInitGuid()
{
  iWMVCompSubtypeTable[0] = WMMEDIASUBTYPE_MP43; 
  iWMVCompSubtypeTable[1] = WMMEDIASUBTYPE_MP4S; 
  iWMVCompSubtypeTable[2] = WMMEDIASUBTYPE_WMV1; 
  iWMVCompSubtypeTable[3] = WMMEDIASUBTYPE_MSS1; 
  iWMVCompSubtypeTable[4] = WMMEDIASUBTYPE_WMV2; 
  iWMVCompSubtypeTable[5] = WMMEDIASUBTYPE_MSS2; 
  iWMVCompSubtypeTable[6] = WMMEDIASUBTYPE_WMV3;

  iWMVCompSubtypeTable[WMV_COMPRESS_COUNT+0] = WMMEDIASUBTYPE_RGB555; 
  iWMVCompSubtypeTable[WMV_COMPRESS_COUNT+1] = WMMEDIASUBTYPE_RGB24; 
  iWMVCompSubtypeTable[WMV_COMPRESS_COUNT+2] = WMMEDIASUBTYPE_RGB32; 
  iWMVCompSubtypeTable[WMV_COMPRESS_COUNT+3] = WMMEDIASUBTYPE_I420; 
  iWMVCompSubtypeTable[WMV_COMPRESS_COUNT+4] = WMMEDIASUBTYPE_IYUV; 
  iWMVCompSubtypeTable[WMV_COMPRESS_COUNT+5] = WMMEDIASUBTYPE_YV12; 
  iWMVCompSubtypeTable[WMV_COMPRESS_COUNT+6] = WMMEDIASUBTYPE_YUY2; 
  iWMVCompSubtypeTable[WMV_COMPRESS_COUNT+7] = WMMEDIASUBTYPE_UYVY; 
  iWMVCompSubtypeTable[WMV_COMPRESS_COUNT+8] = WMMEDIASUBTYPE_YVYU; 
}

static const char* iWMVCompTable[WMV_COMPRESS_COUNT+1] = 
{
  "NONE",
  "MPEG-4v3",   
  "MPEG-4v1",   
  "WMV7",       
  "WMV7Screen",  
  "WMV8",       
  "WMV9Screen", 
  "WMV9"
};

static const char* iWMFCompFindName(GUID SubType)
{
  int i;
  for(i = 0; i < WMV_COMPRESS_COUNT; i++)
  {
    if (SubType == iWMVCompSubtypeTable[i])
      return iWMVCompTable[i+1];
  }

  for(; i < WMV_COMPRESS_COUNT+WMV_UNCOMPRESS_COUNT; i++)
  {
    if (SubType == iWMVCompSubtypeTable[i])
      return iWMVCompTable[0];
  }

  return "Unknown";
}

static GUID iWMFCompFindSubType(const char* compression)
{
  if (compression[0] == 0)
    return WMMEDIASUBTYPE_WMV3;

  for(int i = 0; i < WMV_COMPRESS_COUNT; i++)
  {
    if (imStrEqual(compression, iWMVCompTable[i+1]))
      return iWMVCompSubtypeTable[i];
  }

  return WMMEDIASUBTYPE_Base;
}

class imFileFormatWMV: public imFileFormatBase
{
  IWMSyncReader* Reader;        // When reading
  WM_MEDIA_TYPE* MediaType;
  WORD stream_number;
  DWORD seekable;
  int current_frame;

  IWMWriter* Writer;            // When writing
  DWORD input_number;
  DWORD BitmapDataSize;
  DWORD BitmapInfoSize;

  float fps;
  WCHAR wfile_name[4096];
  IWMHeaderInfo* HeaderInfo;
  BITMAPINFOHEADER* bmiHeader;
  unsigned int rmask, gmask, bmask, 
                roff, goff, boff; /* pixel bit mask control when reading 16 and 32 bpp images */

  void ReadPalette(unsigned char* bmp_colors);
  void WritePalette(unsigned char* bmp_colors);
  void FixRGB(int bpp);
  void InitMasks(imDib* dib);
  void iReadAttrib(imAttribTable* attrib_table);
  void iWriteAttrib(imAttribTable* attrib_table);
  void CalcFPS();
  void SetOutputProps();
  int SetInputProps();
  int SetProfile();

public:
  imFileFormatWMV(const imFormat* _iformat): imFileFormatBase(_iformat) {}
  ~imFileFormatWMV() {}

  int Open(const char* file_name);
  int New(const char* file_name);
  void Close();
  void* Handle(int index);
  int ReadImageInfo(int index);
  int ReadImageData(void* data);
  int WriteImageInfo();
  int WriteImageData(void* data);
};

class imFormatWMV: public imFormat
{
public:
  imFormatWMV()
    :imFormat("WMV", 
              "Windows Media Video Format", 
              "*.wmv;*.asf;", 
              iWMVCompTable, 
              WMV_COMPRESS_COUNT+1, 
              1)
    {}
  ~imFormatWMV() {}

  imFileFormatBase* Create(void) const { return new imFileFormatWMV(this); }
  int CanWrite(const char* compression, int color_mode, int data_type) const;
};

void imFormatRegisterWMV(void)
{
  imFormatRegister(new imFormatWMV());
}

int imFileFormatWMV::Open(const char* file_name)
{
  /* initializes COM */
  CoInitialize(NULL);
  iInitGuid();

  HRESULT hr = WMCreateSyncReader(NULL, 0, &Reader);
  if (hr != 0)
  {
    CoUninitialize();
    return IM_ERR_MEM;
  }

  /* open existing file */
  MultiByteToWideChar(CP_ACP, 0, file_name, -1, wfile_name, 4096);
  hr = Reader->Open(wfile_name);
  if (hr != 0)
  {
    Reader->Release();
    CoUninitialize();
            
    if (hr == NS_E_FILE_OPEN_FAILED || 
        hr == NS_E_FILE_NOT_FOUND || 
        hr == NS_E_INVALID_DATA)
      return IM_ERR_OPEN;
    else if (hr == NS_E_UNRECOGNIZED_STREAM_TYPE)
      return IM_ERR_FORMAT;
    else
      return IM_ERR_ACCESS;
  }

  IWMProfile* pProfile = NULL;
  Reader->QueryInterface(IID_IWMProfile, (VOID**)&pProfile);

  DWORD stream_count;
  pProfile->GetStreamCount(&stream_count);

  this->stream_number = (WORD)-1;
  for (int i = 0; i < (int)stream_count; i++)
  {
    IWMStreamConfig* StreamConfig;
    pProfile->GetStream(i, &StreamConfig);

    GUID StreamType;
    StreamConfig->GetStreamType(&StreamType);

    if (StreamType == WMMEDIATYPE_Video ||
        StreamType == WMMEDIATYPE_Image)
    {
      hr = StreamConfig->GetStreamNumber(&this->stream_number);

      IWMMediaProps* Props;
      StreamConfig->QueryInterface(IID_IWMMediaProps, (VOID**)&Props);

      DWORD pcbType;
      Props->GetMediaType(NULL, &pcbType);
      MediaType = (WM_MEDIA_TYPE*)malloc(pcbType);
      Props->GetMediaType(MediaType, &pcbType);

      Props->Release();

      const char* comp_name = iWMFCompFindName(MediaType->subtype);
      strcpy(this->compression, comp_name);
      break;
    }

    StreamConfig->Release();
  }

  if (this->stream_number == (WORD)-1)
  {
    pProfile->Release();
    Reader->Close();
    Reader->Release();
    CoUninitialize();
    return IM_ERR_DATA;
  }

  hr = Reader->QueryInterface(IID_IWMHeaderInfo, (VOID**)&HeaderInfo);

  CalcFPS();

  WMT_ATTR_DATATYPE attrib_type;
  WORD attrib_length;
  WORD StreamNumber = 0;

  seekable = 0;
  attrib_length = 4;
  attrib_type = WMT_TYPE_BOOL;
  hr = HeaderInfo->GetAttributeByName(&StreamNumber, g_wszWMSeekable, 
                                      &attrib_type, (BYTE*)&seekable, &attrib_length);

  QWORD num_frame = 0;
  attrib_length = 8;
  attrib_type = WMT_TYPE_QWORD;
  hr = HeaderInfo->GetAttributeByName(&stream_number, g_wszWMNumberOfFrames, 
                                      &attrib_type, (BYTE*)&num_frame, &attrib_length);

  if (num_frame == 0)
  {
    QWORD duration = 0;
    attrib_length = 8;
    attrib_type = WMT_TYPE_QWORD;
    hr = HeaderInfo->GetAttributeByName(&StreamNumber, g_wszWMDuration, 
                                        &attrib_type, (BYTE*)&duration, &attrib_length);

    num_frame = (int)(((double)(unsigned int)duration * (double)fps) / 10000000.0);
  }

  this->image_count = (int)num_frame;

  SetOutputProps();

  WMT_STREAM_SELECTION wmtSS = WMT_ON;
  hr = Reader->SetStreamsSelected(1, &stream_number, &wmtSS);
  hr = Reader->SetReadStreamSamples(stream_number, FALSE);

  this->bmiHeader = NULL;
  this->current_frame = 0;

  return IM_ERR_NONE;
}

int imFileFormatWMV::New(const char* file_name)
{
  /* initializes COM */
  CoInitialize(NULL);
  iInitGuid();

  HRESULT hr = WMCreateWriter(NULL, &Writer); 
  if (hr != 0)
  {
    CoUninitialize();
    return IM_ERR_MEM;
  }

  MultiByteToWideChar(CP_ACP, 0, file_name, -1, wfile_name, 4096);

  Writer->QueryInterface(IID_IWMHeaderInfo, (VOID**)&HeaderInfo);

  this->bmiHeader = NULL;
  this->current_frame = 0;

  return IM_ERR_NONE;
}

void imFileFormatWMV::Close()
{
  HeaderInfo->Release();

  if (this->is_new)
  {
    free(this->bmiHeader);

    Writer->EndWriting();
    Writer->Release();
  }
  else
  {
    free(MediaType);

    Reader->Close();
    Reader->Release();
  }

  CoUninitialize();
}

void* imFileFormatWMV::Handle(int index)
{
  if (index == 1)
  {
    if (this->is_new)
      return (void*)this->Writer;
    else
      return (void*)this->Reader;
  }
  else
    return NULL;
}

void imFileFormatWMV::iReadAttrib(imAttribTable* attrib_table)
{
  WORD StreamNumber = 0;
  WORD attrib_list_count = 0;
  HeaderInfo->GetAttributeCount(StreamNumber, &attrib_list_count);

  WCHAR* attrib_name = NULL;
  int name_max_size = 0;
  char* name = NULL;
  WORD attrib_name_count;
  WMT_ATTR_DATATYPE attrib_type;
  BYTE* attrib_data = NULL;
  WORD attrib_length;
  int data_max_size = 0;
  HRESULT hr;
  int data_type, data_count;

  for (WORD i = 0; i < attrib_list_count; i++)
  {
    attrib_name_count = 0;
    attrib_length = 0;

    hr = HeaderInfo->GetAttributeByIndex(i, &StreamNumber, NULL, &attrib_name_count, 
                                            &attrib_type, NULL, &attrib_length);

    if (FAILED(hr))
      continue;

    if (attrib_length == 0)
      continue;

    if (name_max_size < attrib_name_count)
    {
      attrib_name = (WCHAR*)realloc(attrib_name, attrib_name_count*2);
      name = (char*)realloc(name, attrib_name_count);
      name_max_size = attrib_name_count;
    }

    if (data_max_size < attrib_length)
    {
      attrib_data = (BYTE*)realloc(attrib_data, attrib_length);
      data_max_size = attrib_length;
    }

    HeaderInfo->GetAttributeByIndex(i, &StreamNumber, attrib_name, &attrib_name_count, 
                                       &attrib_type, attrib_data, &attrib_length);

    WideCharToMultiByte(CP_ACP, 0, attrib_name, attrib_name_count, name, attrib_name_count, NULL, NULL);

    switch (attrib_type)
    {
    case WMT_TYPE_BOOL:
      {
        DWORD* ddata = (DWORD*)attrib_data;
        if (*ddata == 0)
          continue;
      }
    case WMT_TYPE_DWORD:
      data_type = IM_INT;
      data_count = attrib_length/4;
      break;
    case WMT_TYPE_STRING:
      data_type = IM_BYTE;
      data_count = attrib_length/2;
      {
        WCHAR* wdata = (WCHAR*)attrib_data;
        CHAR* sdata = (CHAR*)attrib_data;
        for (int j = 0; j < data_count; j++)
        {
          CHAR cvalue;
          WideCharToMultiByte(CP_ACP, 0, &wdata[j], 1, &cvalue, 1, NULL, NULL);
          sdata[j] = cvalue;
        }
      }
      break;
    case WMT_TYPE_BINARY:
      data_type = IM_BYTE;
      data_count = attrib_length;
      break;
    case WMT_TYPE_QWORD:
      {
        data_type = IM_INT;
        data_count = attrib_length/8;
        // convert to int in-place
        QWORD* qdata = (QWORD*)attrib_data;
        DWORD* ddata = (DWORD*)attrib_data;
        for (int j = 0; j < data_count; j++)
        {
          ddata[j] = (DWORD)qdata[j];
        }
      }
      break;
    case WMT_TYPE_WORD:
      data_type = IM_USHORT;
      data_count = attrib_length/2;
      break;
    default:
      continue;
    }

    attrib_table->Set(name, data_type, data_count, attrib_data);
  }

  if (name) free(name);
  if (attrib_name) free(attrib_name);
  if (attrib_data) free(attrib_data);
}

static int iAttribSet(void* user_data, int index, const char* name, int data_type, int data_count, const void* data)
{
  (void)index;
  WORD StreamNumber = 0;
  IWMHeaderInfo* HeaderInfo = (IWMHeaderInfo*)user_data;

  WCHAR wName[50];
  WMT_ATTR_DATATYPE Type;
  BYTE* Value = NULL;
  WORD ValueSize = 0;

  MultiByteToWideChar(CP_ACP, 0, name, -1, wName, 50);

  switch(data_type)
  {
  case IM_BYTE:
    if (imStrCheck(data, data_count))
      Type = WMT_TYPE_STRING;
    else
      Type = WMT_TYPE_BINARY;
    break;
  case IM_USHORT:
    Type = WMT_TYPE_WORD;
    break;
  case IM_INT:
    Type = WMT_TYPE_DWORD;
    break;
  default:
    return 1;
  }

  switch (Type)
  {
  case WMT_TYPE_BOOL:
  case WMT_TYPE_DWORD:
    ValueSize = (WORD)(data_count*4);
    break;
  case WMT_TYPE_STRING:
    ValueSize = (WORD)(data_count*2);
    Value = (BYTE*)malloc(ValueSize);
    MultiByteToWideChar(CP_ACP, 0, (char*)data, data_count, (WCHAR*)Value, data_count);
    break;
  case WMT_TYPE_BINARY:
    ValueSize = (WORD)data_count;
    break;
  case WMT_TYPE_QWORD:
    {
      ValueSize = (WORD)(data_count*8);
      Value = (BYTE*)malloc(ValueSize);

      QWORD* qdata = (QWORD*)Value;
      int* idata = (int*)data;
      for (int j = 0; j < data_count; j++)
      {
        qdata[j] = (QWORD)idata[j];
      }
    }
    break;
  case WMT_TYPE_WORD:
    ValueSize = (WORD)(data_count*2);
    break;
  }

  if (Value)
  {
    HeaderInfo->SetAttribute(StreamNumber, wName, Type, 
                                           Value, ValueSize);
    free(Value);
  }
  else
    HeaderInfo->SetAttribute(StreamNumber, wName, Type, 
                                           (BYTE*)data, ValueSize);
  return 1;
}

void imFileFormatWMV::iWriteAttrib(imAttribTable* attrib_table)
{
  attrib_table->ForEach((void*)HeaderInfo, iAttribSet);
}

void imFileFormatWMV::CalcFPS()
{
 	LONGLONG AvgTimePerFrame = 0;

  if (MediaType->formattype == WMFORMAT_VideoInfo)
  {
    WMVIDEOINFOHEADER* info_header = (WMVIDEOINFOHEADER*)MediaType->pbFormat;
    bmiHeader = &info_header->bmiHeader;
    AvgTimePerFrame = info_header->AvgTimePerFrame;
  }
  else if (MediaType->formattype == WMFORMAT_MPEG2Video)
  {
    WMVIDEOINFOHEADER2* info_header = (WMVIDEOINFOHEADER2*)MediaType->pbFormat;
    bmiHeader = &info_header->bmiHeader;
    AvgTimePerFrame = info_header->AvgTimePerFrame;
  }

  WMT_ATTR_DATATYPE attrib_type;
  WORD attrib_length;

  DWORD frame_rate = 0;
  attrib_length = 4;
  HeaderInfo->GetAttributeByName(&stream_number, g_wszWMVideoFrameRate,   // V9 Only
                                 &attrib_type, (BYTE*)&frame_rate, &attrib_length);

  fps = (float)frame_rate;
  if (frame_rate == 0)
  {
    if (AvgTimePerFrame == 0)
    {
      fps = 15;   // default value
    }
    else
    {
      fps = 10000000.0f / (float)AvgTimePerFrame;

      int ifps = (int)(fps * 100);
      if (ifps == 2997 || ifps == 2996 || ifps == 2998)
        fps = (30.0f * 1000.0f) / 1001.0f;
      else if (ifps == 2397 || ifps == 2396 || ifps == 2398)
        fps = (24.0f * 1000.0f) / 1001.0f;
      else if (ifps == 2400)
        fps = 24.0f;
      else if (ifps == 3000)
        fps = 30.0f;
    }
  }
}

void imFileFormatWMV::SetOutputProps()
{
  DWORD output_number;
  Reader->GetOutputNumberForStream(stream_number, &output_number);

  DWORD format_count;
  Reader->GetOutputFormatCount(output_number, &format_count);

  for(DWORD f = 0; f < format_count; f++)
  {
    IWMOutputMediaProps* Props;
    Reader->GetOutputFormat(output_number, f, &Props); 

    DWORD pcbType;
    Props->GetMediaType(NULL, &pcbType);
    WM_MEDIA_TYPE* mt = (WM_MEDIA_TYPE*)malloc(pcbType);
    Props->GetMediaType(mt, &pcbType);

    if (mt->subtype == WMMEDIASUBTYPE_RGB24 ||
        mt->subtype == WMMEDIASUBTYPE_RGB8)
    {
      Reader->SetOutputProps(output_number, Props);
      Props->Release();
      free(mt);
      return;
    }

    Props->Release();
    free(mt);
  }
}

int imFileFormatWMV::SetInputProps()
{
  DWORD input_count;
  Writer->GetInputCount(&input_count);

  GUID guidInputType;
  IWMInputMediaProps* Props = NULL;

  input_number = (DWORD)-1;
  for(DWORD i = 0; i < input_count; i++)
  {
    Writer->GetInputProps(i, &Props);

    Props->GetType(&guidInputType);

    if(guidInputType == WMMEDIATYPE_Video)
    {
      input_number = i;
      break;
    }

    Props->Release();
  }

  if (input_number == (DWORD)-1)
    return 0;

  DWORD cbVideoInfo = sizeof(WMVIDEOINFOHEADER) - sizeof(BITMAPINFOHEADER) + this->BitmapInfoSize;
  WMVIDEOINFOHEADER* pVideoInfo = (WMVIDEOINFOHEADER*)new BYTE[cbVideoInfo];

  pVideoInfo->rcSource.left = 0;
  pVideoInfo->rcSource.top = 0;
  pVideoInfo->rcSource.bottom = this->bmiHeader->biHeight;
  pVideoInfo->rcSource.right = this->bmiHeader->biWidth;
  pVideoInfo->rcTarget = pVideoInfo->rcSource;
  pVideoInfo->dwBitRate = (DWORD)(this->BitmapDataSize * fps);
  pVideoInfo->dwBitErrorRate = 0;
  pVideoInfo->AvgTimePerFrame = (LONGLONG)(10000000.0f / fps);

  CopyMemory(&(pVideoInfo->bmiHeader), this->bmiHeader, BitmapInfoSize);

  WM_MEDIA_TYPE mt;
  mt.majortype = WMMEDIATYPE_Video;
  mt.bFixedSizeSamples = TRUE;
  mt.bTemporalCompression = FALSE;
  mt.lSampleSize = BitmapDataSize;
  mt.formattype = WMFORMAT_VideoInfo;
  mt.pUnk = NULL;
  mt.cbFormat = cbVideoInfo;
  mt.pbFormat = (BYTE*)pVideoInfo;

  switch (this->bmiHeader->biBitCount)
  {
  case 32:
      mt.subtype = WMMEDIASUBTYPE_RGB32;
      break;
  case 24:
      mt.subtype = WMMEDIASUBTYPE_RGB24;
      break;
  case 8:
      mt.subtype = WMMEDIASUBTYPE_RGB8;
      break;
  }

  Props->SetMediaType(&mt);

  HRESULT hr = Writer->SetInputProps(input_number, Props);
  Props->Release();
  free(pVideoInfo);

  if (FAILED(hr))
    return 0;

  return 1;
}

int imFileFormatWMV::SetProfile()
{
  HRESULT hr;

  IWMProfileManager* ProfileManager = NULL;
  WMCreateProfileManager(&ProfileManager);

  IWMProfile* Profile = NULL;
  hr = ProfileManager->CreateEmptyProfile(WMT_VER_9_0, &Profile);
  if (FAILED(hr))
  {
    ProfileManager->Release();
    return 0;
  }

  if (imStrEqual(this->compression, "NONE"))
  {
    hr = iAddUncompressedVideoStream(Profile, 
                                    this->bmiHeader, 
                                    this->BitmapInfoSize, this->BitmapDataSize, this->fps);
  }
  else
  {
    DWORD dwBitRate = 2400*1000;
    const void* attrib_data = AttribTable()->Get("DataRate");
    if (attrib_data)
      dwBitRate = (*(int*)attrib_data) * 1000;

    DWORD dwQuality = 50;
    attrib_data = AttribTable()->Get("WMFQuality");
    if (attrib_data)
      dwQuality = *(int*)attrib_data;

    DWORD dwSecPerKey = 5000;
    attrib_data = AttribTable()->Get("MaxKeyFrameTime");
    if (attrib_data)
      dwSecPerKey = *(int*)attrib_data;

    BOOL fIsVBR = FALSE; // CBR is the default
    attrib_data = AttribTable()->Get("VBR");
    if (attrib_data)
      fIsVBR = *(int*)attrib_data;

    GUID subtype = iWMFCompFindSubType(this->compression);
    if (subtype == WMMEDIASUBTYPE_Base)
    {
      Profile->Release();
      ProfileManager->Release();
      return 0;
    }

    hr = iAddCompressedVideoStream(ProfileManager, Profile, subtype,
                                  this->bmiHeader, this->fps,
                                  fIsVBR, dwBitRate, dwQuality, dwSecPerKey);
  }

  hr = Writer->SetProfile(Profile);
  Profile->Release();
  ProfileManager->Release();

  if (FAILED(hr))
    return 0;

  return 1;
}

int imFileFormatWMV::ReadImageInfo(int index)
{
  if (this->seekable && this->current_frame != index)
  {
    HRESULT hr = Reader->SetRangeByFrame(stream_number, index, 0);
    this->current_frame = index;

    if (hr == NS_E_INVALID_REQUEST)
    {
      QWORD start_time = (QWORD)(index * (10000000.0f / fps));
      hr = Reader->SetRange(start_time, 0);
    }

    if (hr != S_OK)
      return IM_ERR_ACCESS;
  }
  
  if (this->bmiHeader != NULL)
    return IM_ERR_NONE;

  imAttribTable* attrib_table = AttribTable();

  /* must clear the attribute list, because it can have multiple images and 
     has many attributes that may exists only for specific images. */
  attrib_table->RemoveAll();
  imFileSetBaseAttributes(this);

  if (MediaType->formattype == WMFORMAT_VideoInfo)
  {
    WMVIDEOINFOHEADER* info_header = (WMVIDEOINFOHEADER*)MediaType->pbFormat;
    bmiHeader = &info_header->bmiHeader;

    if (info_header->dwBitRate)
    {
      int data_rate = info_header->dwBitRate/1000;
      attrib_table->Set("DataRate", IM_INT, 1, &data_rate);
    }
  }
  else if (MediaType->formattype == WMFORMAT_MPEG2Video)
  {
    WMVIDEOINFOHEADER2* info_header = (WMVIDEOINFOHEADER2*)MediaType->pbFormat;
    bmiHeader = &info_header->bmiHeader;

    if (info_header->dwBitRate)
    {
      int data_rate = info_header->dwBitRate/1000;
      attrib_table->Set("DataRate", IM_INT, 1, &data_rate);
    }

    if (info_header->dwInterlaceFlags)
    {
      int int_value = 1;
      attrib_table->Set("Interlaced", IM_INT, 1, &int_value);

      if (info_header->dwInterlaceFlags & AMINTERLACE_1FieldPerSample)
        int_value = 1;
      else
        int_value = 2;

      attrib_table->Set("FieldsPerSample", IM_INT, 1, &int_value);
       
      if (info_header->dwInterlaceFlags & AMINTERLACE_Field1First)
        int_value = 1;
      else
        int_value = 2;

      attrib_table->Set("FirstField", IM_INT, 1, &int_value);

      // OBS: The top field in PAL is field 1, and the top field in NTSC is field 2
    }

    if (info_header->dwPictAspectRatioX)
      attrib_table->Set("XAspectRatio", IM_INT, 1, &info_header->dwPictAspectRatioX);
    if (info_header->dwPictAspectRatioY)
      attrib_table->Set("YAspectRatio", IM_INT, 1, &info_header->dwPictAspectRatioY);
  }
  else
    return IM_ERR_DATA;

  attrib_table->Set("FPS", IM_FLOAT, 1, &fps);

  int top_down = 0;
  if (bmiHeader->biHeight < 0)
    top_down = 1;

  this->width = bmiHeader->biWidth;
  this->height = top_down? -bmiHeader->biHeight: bmiHeader->biHeight;

  int bpp = bmiHeader->biBitCount;

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
    if (bmiHeader->biClrUsed != 0 && 
        (int)bmiHeader->biClrUsed < this->palette_count)
      this->palette_count = bmiHeader->biClrUsed;

    ReadPalette((unsigned char*)(bmiHeader + 1));
  }

  this->line_buffer_extra = 4; // room enough for padding

  iReadAttrib(attrib_table);

  return IM_ERR_NONE;
}

int imFileFormatWMV::WriteImageInfo()
{
  if (this->bmiHeader)
  {
    if (this->bmiHeader->biWidth != width || this->bmiHeader->biHeight != height ||
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

  this->BitmapDataSize = this->height * imFileLineSizeAligned(this->width, bpp, 4);

  DWORD biClrUsed = bpp > 8? 0: this->palette_count;
  this->BitmapInfoSize = sizeof(BITMAPINFOHEADER) + biClrUsed * sizeof(RGBQUAD);

  this->bmiHeader = (BITMAPINFOHEADER*)malloc(this->BitmapInfoSize);
  this->bmiHeader->biSize = sizeof(BITMAPINFOHEADER);
  this->bmiHeader->biWidth = this->width;
  this->bmiHeader->biHeight = this->height;
  this->bmiHeader->biPlanes = 1;
  this->bmiHeader->biBitCount = (WORD)bpp;
  this->bmiHeader->biCompression = BI_RGB;
  this->bmiHeader->biSizeImage = this->BitmapDataSize;
  this->bmiHeader->biXPelsPerMeter = 0;
  this->bmiHeader->biYPelsPerMeter = 0;
  this->bmiHeader->biClrUsed = biClrUsed;
  this->bmiHeader->biClrImportant = 0;

  if (this->bmiHeader->biBitCount <= 8)
    WritePalette((unsigned char*)(this->bmiHeader + 1));

  if (!SetProfile())
    return IM_ERR_COMPRESS;

  if (!SetInputProps())
    return IM_ERR_ACCESS;

  HRESULT hr = Writer->SetOutputFilename(wfile_name);
  if(FAILED(hr))
    return IM_ERR_ACCESS;

  iWriteAttrib(attrib_table);

  hr = Writer->BeginWriting();
  if(FAILED(hr))
    return IM_ERR_ACCESS;

  return IM_ERR_NONE;
}

void imFileFormatWMV::ReadPalette(unsigned char* bmp_colors)
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

void imFileFormatWMV::WritePalette(unsigned char* bmp_colors)
{
  /* convert the color map to the IM format */
  for (int c = 0; c < this->palette_count; c++)
  {
    int i = c * 4;                       
    imColorDecode(&bmp_colors[i + 2], &bmp_colors[i + 1], &bmp_colors[i], this->palette[c]);
    bmp_colors[i + 3] = 0;
  }
}

void imFileFormatWMV::InitMasks(imDib* dib)
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

void imFileFormatWMV::FixRGB(int bpp)
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
      unsigned int* dword_data = (unsigned int*)this->line_buffer;
      imbyte* byte_data = (imbyte*)this->line_buffer;

      for (x = 0; x < this->width; x++)
      {
        unsigned int dword_value = dword_data[x];
        int c = x*3;
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

int imFileFormatWMV::ReadImageData(void* data)
{
  imCounterTotal(this->counter, this->height, "Reading WMV Frame...");

  INSSBuffer* pSample = NULL;

  {
    QWORD cnsSampleTime = 0;  // All will be ignored
    QWORD cnsDuration = 0;
    DWORD dwFlags = 0;
    WORD wStreamNum = 0;
    DWORD dwOutputNum = 0;
    HRESULT hr;

    hr = Reader->GetNextSample(stream_number, &pSample, &cnsSampleTime,
                                              &cnsDuration, &dwFlags,
                                              &dwOutputNum, &wStreamNum);

    if (FAILED(hr))
      return IM_ERR_ACCESS;
  }

  imbyte* dib_bits = NULL;
  pSample->GetBuffer(&dib_bits);
  if (!dib_bits)
  {
    pSample->Release();
    return IM_ERR_MEM;
  }

  imDib* dib = imDibCreateReference((imbyte*)this->bmiHeader, dib_bits);

  if (dib->bmih->biBitCount == 16 || dib->bmih->biBitCount == 32)
    InitMasks(dib);
  else if (dib->bmih->biBitCount <= 8)
  {
    this->palette_count = dib->palette_count;
    ReadPalette((unsigned char*)dib->bmic);
  }

  for (int row = 0; row < this->height; row++)
  {
    CopyMemory(this->line_buffer, dib_bits, dib->line_size);
    dib_bits += dib->line_size;

    if (dib->bmih->biBitCount > 8)
      FixRGB(dib->bmih->biBitCount);

    imFileLineBufferRead(this, data, row, 0);

    if (!imCounterInc(this->counter))
    {
      imDibDestroy(dib);
      dib = NULL;
      pSample->Release();
      return IM_ERR_COUNTER;
    }
  }

  imDibDestroy(dib);
  pSample->Release();
  this->current_frame++;

  return IM_ERR_NONE;
}

int imFileFormatWMV::WriteImageData(void* data)
{
  imCounterTotal(this->counter, this->height, "Writing WMV Frame...");

  INSSBuffer* pSample = NULL;
  Writer->AllocateSample(BitmapDataSize, &pSample);

  imbyte* dib_bits = NULL;
  if (pSample) pSample->GetBuffer(&dib_bits);
  if (!dib_bits || !pSample)
  {
    if (pSample) pSample->Release();
    return IM_ERR_MEM;
  }

  imDib* dib = imDibCreateReference((imbyte*)this->bmiHeader, dib_bits);
  if (dib->bmih->biBitCount <= 8)
    WritePalette((unsigned char*)dib->bmic);

  for (int row = 0; row < this->height; row++)
  {
    imFileLineBufferWrite(this, data, row, 0);

    if (dib->bmih->biBitCount > 8)
      FixRGB(dib->bmih->biBitCount);

    CopyMemory(dib_bits, this->line_buffer, dib->line_size);
    dib_bits += dib->line_size;

    if (!imCounterInc(this->counter))
      return IM_ERR_COUNTER;
  }

  QWORD VideoTime = (QWORD)(this->image_count * (10000000.0f / fps));

  HRESULT hr = Writer->WriteSample(input_number,
                                   VideoTime,
                                   0,
                                   pSample);
  if (hr != 0)
    return IM_ERR_ACCESS;

  imDibDestroy(dib);
  pSample->Release();
  this->image_count++;

  return IM_ERR_NONE;
}

int imFormatWMV::CanWrite(const char* compression, int color_mode, int data_type) const
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
