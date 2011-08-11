/** \file
 * \brief Video Capture Using Direct Show 9
 *
 * See Copyright Notice in im.h
 * $Id: im_capture_dx.cpp,v 1.2 2010/04/25 21:51:29 scuri Exp $
 */

/*
  The Direct Show Graph is composed by 3 components:
    capture source, sample grabber and null renderer.

  Filters are connected:
    capture_filter(out)->(int)grabber_filter(out)->(in)null_filter

  But when the graph is rendered other transform filters 
  can be inserted to connect the capture and the grabber.

  We do not use MFC, ATL and the Direct Show Base Classes.
  This module only needs the library "strmiids.lib".
  If the extra error functions were used, you will need to link with "quartz.lib" or "dxerr9.lib".

  We use the buffer of the ISampleGrabber. But this can not be done in a user callback,
  so we leave the grab loop for the application, it can also be done in the idle function.

  If you use the idle function for the grab loop, then WDM Source Dialog will interrupt the "live" mode.
  Just because it is a modal dialog and it does not use the application message loop.
  It can be solved if the grab loop is implemented using a timer.

  Since there is no gray format, bpp is always 24bpp.
*/

#define _WIN32_WINNT 0x0500   // Because of TryEnterCriticalSection

#include <dshow.h>
#include <qedit.h>

#include <memory.h>
#include <stdio.h>
#include <assert.h>

#include <im.h>
#include <im_util.h>

#include "im_capture.h"

#define VC_CAMERADELAY   200  // This vary from camera to camera, so we use a reasonable value and hope it will work for all.
#define VC_MAXVIDDEVICES 30   // Maximum number of devices to list

//#define VC_REGISTER_FILTERGRAPH // Use this to allow GraphEdit to spy the graph
//#define VC_INCLUDE_VFW_DEVICES  // Use this to allow old video for windows devices
//#define VC_PRINT_ERROR_MESSAGES // Use this to display a system custom error message

#if defined(_DEBUG) | defined(DEBUG)
#define VC_REGISTER_FILTERGRAPH
#define VC_PRINT_ERROR_MESSAGES
#endif


/**************************************************************************
                       imTrackingGrabberCB
***************************************************************************/

// This is better than using the sample grabber internal buffer 
// because we have a more precise control of the data flow.

class imTrackingGrabberCB: public ISampleGrabberCB
{
public:
  imTrackingGrabberCB();
  ~imTrackingGrabberCB();

  STDMETHODIMP SampleCB(double SampleTime, IMediaSample *pSample);
  STDMETHODIMP BufferCB(double SampleTime, BYTE *pBuffer, long BufferLen) {return E_NOTIMPL;}
  STDMETHODIMP_(ULONG) AddRef() {return 2;}
  STDMETHODIMP_(ULONG) Release() {return 1;}
  STDMETHODIMP QueryInterface(REFIID riid, void ** ppv);

  void SetImageSize(int width, int height);
  int GetImage(unsigned char* data, int color_mode, int timeout);

protected:
  int  m_Width, m_Height;
  bool m_newImageFlag;
  unsigned char *m_ImageData;
  CRITICAL_SECTION m_sect;
  HANDLE m_imageReady;
};

imTrackingGrabberCB::imTrackingGrabberCB()
{
  InitializeCriticalSection(&m_sect);
  m_newImageFlag = 0;
  m_ImageData = NULL;
  m_imageReady = CreateEvent(NULL, FALSE, TRUE, NULL);
}

imTrackingGrabberCB::~imTrackingGrabberCB() 
{
  CloseHandle(m_imageReady);
  EnterCriticalSection(&m_sect);
  DeleteCriticalSection(&m_sect);
  if (m_ImageData) delete m_ImageData;
}

STDMETHODIMP imTrackingGrabberCB::QueryInterface(REFIID riid, void ** ppv) 
{
  if( riid == IID_ISampleGrabberCB || riid == IID_IUnknown ) {
    *ppv = (void *) static_cast<ISampleGrabberCB*> (this);
    return NOERROR;
  }    
  return E_NOINTERFACE;
}

void imTrackingGrabberCB::SetImageSize(int width, int height) 
{
  EnterCriticalSection(&m_sect);

  // This can be done because the capture system always returns 
  // images that are a multiple of 4.
  int new_size = width * height * 3;

  if (!m_ImageData) 
  {
    m_ImageData = (BYTE*)calloc(new_size, 1);
    m_Width = width; 
    m_Height = height;
  }

  if (m_Width*m_Height < new_size)
    m_ImageData = (BYTE*)realloc(m_ImageData, new_size);

  m_Width = width; 
  m_Height = height;

  LeaveCriticalSection(&m_sect);
}

STDMETHODIMP imTrackingGrabberCB::SampleCB(double, IMediaSample *pSample)
{
  if (!m_ImageData) return S_OK;

  EnterCriticalSection(&m_sect);

  int size = pSample->GetSize();
  if (size > m_Width*m_Height*3)
  {
    LeaveCriticalSection(&m_sect);
    return S_OK;
  }

  BYTE *pData;
  pSample->GetPointer(&pData);
  CopyMemory(m_ImageData, pData, size);
  m_newImageFlag = 1;

  LeaveCriticalSection(&m_sect);

  SetEvent(m_imageReady);
  
  return S_OK;
}

int imTrackingGrabberCB::GetImage(unsigned char* data, int color_mode, int timeout)
{
  if (timeout != 0)
  {
    DWORD ret = WaitForSingleObject(m_imageReady, timeout);
    if (ret != WAIT_OBJECT_0)
      return 0;
  }

  if (!TryEnterCriticalSection(&m_sect))
    return 0;

  if (m_newImageFlag == 1)
  {
    int count = m_Width*m_Height;
    unsigned char* src_data = m_ImageData;

    if (imColorModeSpace(color_mode) == IM_RGB)
    {
      if (imColorModeIsPacked(color_mode))
      {
        unsigned char* dst_data = data;
        for (int i = 0; i < count; i++)
        {
          *(dst_data+2) = *src_data++;
          *(dst_data+1) = *src_data++;
          *dst_data = *src_data++;
          dst_data += 3;
        }
      }
      else
      {
        unsigned char* red = data;
        unsigned char* green = data + count;
        unsigned char* blue = data + 2*count;
        for (int i = 0; i < count; i++)
        {
          *blue++ = *src_data++;
          *green++ = *src_data++;
          *red++ = *src_data++;
        }
      }
    }
    else
    {
      unsigned char* map = data;
      for (int i = 0; i < count; i++)
      {
        *map++ = *src_data;
        src_data += 3;
      }
    }

    m_newImageFlag = 0;

    LeaveCriticalSection(&m_sect);
    return 1;
  }

  LeaveCriticalSection(&m_sect);
  return 0;
}


/**************************************************************************
                       Direct Show Only
***************************************************************************/


struct vcDevice
{
  IBaseFilter *filter;
  char vendorinfo[128];
  char desc[128];
  char ex_desc[256];
  char path[512];
};
static vcDevice vc_DeviceList[VC_MAXVIDDEVICES];
static int vc_DeviceCount = 0;

static void vc_AddDevice(IBaseFilter *filter, const char* desc, const char* ex_desc, const char* path, const char* vendorinfo)
{
  int i = vc_DeviceCount;
  vcDevice* device = &vc_DeviceList[i];

  memset(device, 0, sizeof(vcDevice));

  device->filter = filter;

  if (!desc) desc = "device";
  sprintf(device->desc, "%d - %s", i, desc);

  if (ex_desc) strcpy(device->ex_desc, ex_desc);
  if (path) strcpy(device->path, path);
  if (vendorinfo) strcpy(device->vendorinfo, vendorinfo);

  vc_DeviceCount++;
}


#ifdef VC_PRINT_ERROR_MESSAGES
//#include <dxerr9.h>

static int vc_ShowError(HRESULT hr)
{
  if (FAILED(hr))
  {
    TCHAR szErr[MAX_ERROR_TEXT_LEN];
    DWORD res = AMGetErrorText(hr, szErr, MAX_ERROR_TEXT_LEN);  // Must link with quartz.lib
    if (res == 0) wsprintf(szErr, "Unknown Error: 0x%2x", hr);
    MessageBox(0, szErr, "imCapture Error!", MB_OK | MB_ICONERROR);
//    MessageBox(NULL, DXGetErrorDescription9(hr), DXGetErrorString9(hr), MB_OK | MB_ICONERROR);
    return 1;
  }

  return 0;
}

#define VC_HARDFAILED(_x) vc_ShowError(_x)
#else
#define VC_HARDFAILED FAILED
#endif

static char* vc_Wide2Char(WCHAR* wstr)
{
  if (wstr)
  {
    int n = wcslen(wstr)+1;
    char* str = (char*)malloc(n);
    WideCharToMultiByte(CP_ACP, 0, wstr, -1, str, n, NULL, NULL);
    return str;
  }

  return NULL;
}

#ifdef VC_INCLUDE_VFW_DEVICES
#define VC_CATEGORY_FLAG 0
#else
#define VC_CATEGORY_FLAG CDEF_DEVMON_FILTER|CDEF_DEVMON_PNP_DEVICE
#endif

static char* vc_GetDeviceProp(IPropertyBag *pPropBag, const WCHAR* PropName)
{
  VARIANT varProp;
  VariantInit(&varProp);
  HRESULT hr = pPropBag->Read(PropName, &varProp, 0);
  if (SUCCEEDED(hr))
  {
    char* str = vc_Wide2Char(varProp.bstrVal);
    VariantClear(&varProp); 
    return str;
  }
  return NULL;
}

static void vc_EnumerateDevices(void)
{
  // Selecting a Capture Device
  ICreateDevEnum *pDevEnum = NULL;
  IEnumMoniker *pEnum = NULL;

  CoInitialize(NULL);

  // Create the System Device Enumerator.
  HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL,
                                CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, 
                                reinterpret_cast<void**>(&pDevEnum));
  if (FAILED(hr)) return;
                                       
  // Create an enumerator for the video capture category.
  hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnum, VC_CATEGORY_FLAG);
  if (FAILED(hr) || !pEnum)
  {
    pDevEnum->Release();
    return;
  }

  IMoniker *pMoniker = NULL;
  while (pEnum->Next(1, &pMoniker, NULL) == S_OK)
  {
    IBaseFilter *capture_filter = NULL;
    hr = pMoniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&capture_filter);
    if (FAILED(hr))
    {
      pMoniker->Release();
      continue;  // Skip this one, maybe the next one will work.
    } 

    IPropertyBag *pPropBag;
    hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void**)(&pPropBag));
    if (FAILED(hr))
    {
      capture_filter->Release();
      pMoniker->Release();
      continue;  // Skip this one, maybe the next one will work.
    } 

    char* desc = vc_GetDeviceProp(pPropBag, L"FriendlyName");
    char* ex_desc = vc_GetDeviceProp(pPropBag, L"Description");
    char* path = vc_GetDeviceProp(pPropBag, L"DevicePath");

    char* vendorinfo = NULL;
    LPWSTR VendorInfo;
    if (capture_filter->QueryVendorInfo(&VendorInfo) == S_OK)
    {
      vendorinfo = vc_Wide2Char(VendorInfo);
      CoTaskMemFree(VendorInfo);
    }

    vc_AddDevice(capture_filter, desc, ex_desc, path, vendorinfo);

    if (desc) free(desc);
    if (ex_desc) free(ex_desc);
    if (path) free(path);
    if (vendorinfo) free(vendorinfo);

    pPropBag->Release();
    pMoniker->Release();
  }

  pEnum->Release();
  pDevEnum->Release();
}

static IPin* vc_GetPin(IBaseFilter* pFilter, PIN_DIRECTION dir)
{
  IEnumPins*  pEnumPins = NULL;
  IPin*       pPin = NULL;

  pFilter->EnumPins(&pEnumPins);
  if(!pEnumPins)
    return NULL;

  for(;;)
  {
    ULONG  cFetched = 0;
    PIN_DIRECTION pinDir = PIN_DIRECTION(-1); 
    pPin = 0;

    if (FAILED(pEnumPins->Next(1, &pPin, &cFetched)))
    {
      pEnumPins->Release();
      return NULL;
    }

    if(cFetched == 1 && pPin != 0)
    {
      pPin->QueryDirection(&pinDir);
      if(pinDir == dir) break;
      pPin->Release();
    }
  }

  pEnumPins->Release();
  return pPin;
}

static void vc_NukeDownstream(IGraphBuilder* filter_builder, IBaseFilter *filter)
{
  IPin *pPin=0, *pPinTo=0;
  IEnumPins *pEnumPins = NULL;
  PIN_INFO pininfo;

  HRESULT hr = filter->EnumPins(&pEnumPins);
  if (FAILED(hr)) return;

  pEnumPins->Reset();

  while(hr == NOERROR)
  {
    hr = pEnumPins->Next(1, &pPin, NULL);
    if(hr == S_OK && pPin)
    {
      pPin->ConnectedTo(&pPinTo);
      if(pPinTo)
      {
        hr = pPinTo->QueryPinInfo(&pininfo);
        if(hr == NOERROR)
        {
          if(pininfo.dir == PINDIR_INPUT)
          {
            vc_NukeDownstream(filter_builder, pininfo.pFilter);
            filter_builder->Disconnect(pPinTo);
            filter_builder->Disconnect(pPin);
            filter_builder->RemoveFilter(pininfo.pFilter);
          }

          pininfo.pFilter->Release();
        }

        pPinTo->Release();
      }

      pPin->Release();
    }
  }

  pEnumPins->Release();
}

static int vc_DisconnectFilters(IGraphBuilder* filter_builder, IBaseFilter* source, IBaseFilter* destiny)
{
  IPin *pOut = vc_GetPin(source, PINDIR_OUTPUT);
  IPin *pIn = vc_GetPin(destiny, PINDIR_INPUT);
  HRESULT hr = filter_builder->Disconnect(pOut);
  hr = filter_builder->Disconnect(pIn);
  pOut->Release(); 
  pIn->Release();
  if (VC_HARDFAILED(hr))  return 0;
  return 1;
}

static int vc_DisconnectFilterPin(IGraphBuilder* filter_builder, IBaseFilter* filter, PIN_DIRECTION dir)
{
  IPin *pIn = vc_GetPin(filter, dir);
  IPin *pOut;
  pIn->ConnectedTo(&pOut);
  
  HRESULT hr = filter_builder->Disconnect(pIn);
  pIn->Release();
  if (VC_HARDFAILED(hr))
  {
    if (pOut) pOut->Release();
    return 0;
  }

  if (pOut)
  {
    hr = filter_builder->Disconnect(pOut);
    pOut->Release();

    if (VC_HARDFAILED(hr))
      return 0;
  }

  return 1;
}

static int vc_ConnectFilters(IGraphBuilder* filter_builder, IBaseFilter* source, IBaseFilter* destiny, int direct)
{
  HRESULT hr;
  IPin *pOut = vc_GetPin(source, PINDIR_OUTPUT);
  IPin *pIn = vc_GetPin(destiny, PINDIR_INPUT);
  if (direct)
    hr = filter_builder->ConnectDirect(pOut, pIn, NULL);
  else
    hr = filter_builder->Connect(pOut, pIn);
  pOut->Release(); 
  pIn->Release();
  if (VC_HARDFAILED(hr)) return 0;
  return 1;
}

static DWORD vc_AddGraphToRot(IUnknown *pUnkGraph) 
{
  IMoniker * pMoniker;
  IRunningObjectTable *pROT;
  WCHAR wsz[128];
  HRESULT hr;

  if (FAILED(GetRunningObjectTable(0, &pROT)))
    return 0;

  wsprintfW(wsz, L"FilterGraph %08x pid %08x\0", (DWORD_PTR)pUnkGraph, GetCurrentProcessId());

  hr = CreateItemMoniker(L"!", wsz, &pMoniker);
  if (SUCCEEDED(hr)) 
  {
    DWORD dwRegister;
    hr = pROT->Register(ROTFLAGS_REGISTRATIONKEEPSALIVE, pUnkGraph, pMoniker, &dwRegister);
    pROT->Release();

    pMoniker->Release();

    if (SUCCEEDED(hr)) 
      return dwRegister;
  }

  pROT->Release();
  return 0;
}

static void vc_RemoveGraphFromRot(DWORD pdwRegister)
{
  IRunningObjectTable *pROT;

  if (SUCCEEDED(GetRunningObjectTable(0, &pROT))) 
  {
    pROT->Revoke(pdwRegister);
    pROT->Release();
  }
}

/**************************************************************************
                          imVideoCapture
***************************************************************************/

typedef int (*vcDialogFunc)(imVideoCapture* vc, HWND parent);

struct _imVideoCapture
{
  int registered_graph,
      live,
      device;                     /* current connected device. -1 if not connected. */

  char* dialog_desc[6];
  vcDialogFunc dialog_func[6];
  int dialog_count;                /* number of available configuration dialogs for the current connection. */

  IGraphBuilder* filter_builder;  /* The Filter Graph Manager */
  ICaptureGraphBuilder2* capture_graph_builder; /* Helps the Filter Graph Manager */
  IBaseFilter* capture_filter;    /* the capture device (can vary), it's a source filter. */
  IBaseFilter* grabber_filter;    /* returns the capture data, it's a transform filter */
  IBaseFilter* null_filter;       /* does nothing, act as a terminator, it's a rendering filter */
  ISampleGrabber* sample_grabber; /* Used to access the ISampleGrabber interface, since grabber_filter is a generic IBaseFilter interface based on ISampleGrabber. */
  IMediaControl* media_control;   /* Used to Run and Stop the graph flow. */
  IBaseFilter* overlay_renderer;  /* Used when there is a video port without a preview */    
  IBaseFilter *overlay_mixer;

  IAMVideoProcAmp* video_prop;    /* Used to set/get video properties */
  IAMCameraControl* camera_prop;  /* Used to set/get camera properties */
  IAMVideoControl* videoctrl_prop; /* Used to set/get video properties */

  imTrackingGrabberCB* sample_callback; /* Used to intercept the samples. */

  int format_count;   /* number of supported formats */
  int format_current; /* current format */
  int format_map[50]; /* table to map returned formats to direct X formats */
};

int imVideoCaptureDeviceCount(void)
{
  return vc_DeviceCount;
}

void imVideoCaptureReleaseDevices(void)
{
  for (int i = 0; i < vc_DeviceCount; i++)
  {
    vc_DeviceList[i].filter->Release();
  }
  vc_DeviceCount = 0;
}
  
int imVideoCaptureReloadDevices(void)
{
  imVideoCaptureReleaseDevices();

  vc_EnumerateDevices();
  return vc_DeviceCount;
}

static int vc_CheckDeviceList(int device)
{
  // List available Devices once
  if (vc_DeviceCount == 0)
  {
    vc_EnumerateDevices();

    if (vc_DeviceCount == 0)
      return 0;
  }

  if (device < 0 || device >= vc_DeviceCount)
    return 0;

  return 1;
}

const char* imVideoCaptureDeviceDesc(int device)
{
  if (!vc_CheckDeviceList(device))
    return NULL;

  return vc_DeviceList[device].desc;
}

const char* imVideoCaptureDeviceExDesc(int device)
{
  if (!vc_CheckDeviceList(device))
    return NULL;

  return vc_DeviceList[device].ex_desc;
}

const char* imVideoCaptureDevicePath(int device)
{
  if (!vc_CheckDeviceList(device))
    return NULL;

  return vc_DeviceList[device].path;
}

const char* imVideoCaptureDeviceVendorInfo(int device)
{
  if (!vc_CheckDeviceList(device))
    return NULL;

  return vc_DeviceList[device].vendorinfo;
}

#define vc_SafeRelease(_p) { if( (_p) != 0 ) { (_p)->Release(); (_p)= NULL; } }

static void vc_CheckVideoPort(imVideoCapture* vc)
{
/*
  If the video capture card supports the video port pin without a video preview pin this will not work. 
  The DirectShow architecture requires that the video port pin be connected to the Overlay Mixer Filter. 
  If this pin is not connected, data cannot be captured in DirectShow. 
*/
  HRESULT hr;       

  IPin *pPreviewPin = NULL;
  hr = vc->capture_graph_builder->FindPin(
      vc->capture_filter,      // Pointer to the capture filter.
      PINDIR_OUTPUT,           // Look for an output pin.
      &PIN_CATEGORY_PREVIEW,   // Look for a preview pin.
      NULL,                    // Any media type.
      FALSE,                   // Pin can be connected.
      0,                       // Retrieve the first matching pin.
      &pPreviewPin             // Receives a pointer to the pin.
  );
  if (hr == S_OK)
  {
    pPreviewPin->Release();
    return;
  }

  IPin *pVideoPortPin = NULL;
  hr = vc->capture_graph_builder->FindPin(
      vc->capture_filter,      // Pointer to the capture filter.
      PINDIR_OUTPUT,           // Look for an output pin.
      &PIN_CATEGORY_VIDEOPORT, // Look for a video port pin.
      NULL,                    // Any media type.
      FALSE,                   // Pin can be connected.
      0,                       // Retrieve the first matching pin.
      &pVideoPortPin           // Receives a pointer to the pin.
  );
  if (FAILED(hr)) return; 

  // Create the overlay mixer.
  CoCreateInstance(CLSID_OverlayMixer, NULL, CLSCTX_INPROC,
                   IID_IBaseFilter, (void **)&vc->overlay_mixer);

  // Add it to the filter graph.
  vc->filter_builder->AddFilter(vc->overlay_mixer, L"Overlay Mixer");

  IPin *pOverlayPin = NULL;
  vc->capture_graph_builder->FindPin(vc->overlay_mixer, PINDIR_INPUT, NULL, NULL, TRUE, 0, &pOverlayPin);

  vc->filter_builder->Connect(pVideoPortPin, pOverlayPin);
  if (FAILED(hr)) return; 

  vc_SafeRelease(pVideoPortPin); 
  vc_SafeRelease(pOverlayPin);

  CoCreateInstance(CLSID_VideoRenderer, NULL, CLSCTX_INPROC_SERVER, 
                   IID_IBaseFilter, reinterpret_cast<void**>(&vc->overlay_renderer));
  vc->filter_builder->AddFilter(vc->overlay_renderer, L"Overlay Renderer");

  vc_ConnectFilters(vc->filter_builder, vc->overlay_mixer, vc->overlay_renderer, 1);

  IVideoWindow* pVideoWindow = NULL;
  vc->overlay_renderer->QueryInterface(IID_IVideoWindow,(void**)&pVideoWindow);
  pVideoWindow->put_AutoShow(OAFALSE);
  pVideoWindow->Release();
}

static void vc_ReleaseMixer(imVideoCapture* vc)
{
  IPin *pOverlayPin = vc_GetPin(vc->overlay_mixer, PINDIR_INPUT);
  IPin *pVideoPortPin = NULL;
  pOverlayPin->ConnectedTo(&pVideoPortPin);
  vc->filter_builder->Disconnect(pOverlayPin);
  vc->filter_builder->Disconnect(pVideoPortPin);
  vc_SafeRelease(pVideoPortPin); 
  vc_SafeRelease(pOverlayPin);

  vc_DisconnectFilters(vc->filter_builder, vc->overlay_mixer, vc->overlay_renderer);

  vc->filter_builder->RemoveFilter(vc->overlay_renderer);
  vc->filter_builder->RemoveFilter(vc->overlay_mixer);
  vc_SafeRelease(vc->overlay_renderer);
  vc_SafeRelease(vc->overlay_mixer);
}

static int vc_InitCaptureGraphBuilder(imVideoCapture* vc)
{
  HRESULT hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC_SERVER, 
                                IID_ICaptureGraphBuilder2, reinterpret_cast<void**>(&vc->capture_graph_builder));
  if (FAILED(hr)) return 0; 

  hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER,
                        IID_IGraphBuilder, reinterpret_cast<void**>(&vc->filter_builder));
  if (FAILED(hr)) return 0; 

  hr = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER, 
                        IID_IBaseFilter, reinterpret_cast<void**>(&vc->grabber_filter));
  if (FAILED(hr)) return 0; 

  hr = CoCreateInstance(CLSID_NullRenderer, NULL, CLSCTX_INPROC_SERVER, 
                        IID_IBaseFilter, reinterpret_cast<void**>(&vc->null_filter));
  if (FAILED(hr)) return 0; 

  // Initialize the Capture Graph Builder.
  vc->capture_graph_builder->SetFiltergraph(vc->filter_builder);

  hr = vc->filter_builder->QueryInterface(IID_IMediaControl,(void**)&vc->media_control);
  hr = vc->grabber_filter->QueryInterface(IID_ISampleGrabber, (void **)&vc->sample_grabber);

  AM_MEDIA_TYPE mt;
  ZeroMemory(&mt, sizeof(AM_MEDIA_TYPE));
  mt.majortype = MEDIATYPE_Video;
  mt.subtype = MEDIASUBTYPE_RGB24;  // Force 24 bpp
  vc->sample_grabber->SetMediaType(&mt);
  vc->sample_grabber->SetOneShot(FALSE);
  vc->sample_grabber->SetBufferSamples(FALSE);

  vc->sample_callback = new imTrackingGrabberCB();

  hr = vc->filter_builder->AddFilter(vc->grabber_filter, L"imSampleGrabber");
  hr = vc->filter_builder->AddFilter(vc->null_filter, L"imNullRenderer");

  // Remove clock to speed up things
  IMediaFilter* pMediaFilter = NULL;
  vc->filter_builder->QueryInterface(IID_IMediaFilter, (void**)&pMediaFilter);
  pMediaFilter->SetSyncSource(NULL);
  pMediaFilter->Release();

#ifdef VC_REGISTER_FILTERGRAPH
  vc->registered_graph = vc_AddGraphToRot(vc->filter_builder);
#endif

  return 1;
}

imVideoCapture* imVideoCaptureCreate(void)
{
  imVideoCapture* vc = (imVideoCapture*)malloc(sizeof(imVideoCapture));
  memset(vc, 0, sizeof(imVideoCapture));

  // List available Devices once
  if (vc_DeviceCount == 0)
  {
    vc_EnumerateDevices();

    if (vc_DeviceCount == 0)
    {
      free(vc);
      return NULL;
    }
  }

  if (!vc_InitCaptureGraphBuilder(vc))
  {
    vc_SafeRelease(vc->grabber_filter);
    vc_SafeRelease(vc->filter_builder);
    vc_SafeRelease(vc->capture_graph_builder);
    vc_SafeRelease(vc->null_filter);
    free(vc);
    return NULL;
  }

  vc->device = -1;

  return vc;
}

static void vc_CaptureRemove(imVideoCapture* vc)
{
  vc->filter_builder->RemoveFilter(vc->capture_filter);

  vc->capture_filter = NULL; /* do not release here */
  vc_SafeRelease(vc->video_prop);
  vc_SafeRelease(vc->camera_prop);
  vc_SafeRelease(vc->videoctrl_prop);

  vc->dialog_count = 0;
  vc->live = 0;
  vc->device = -1;
}

void imVideoCaptureDestroy(imVideoCapture* vc)
{
  assert(vc);

#ifdef VC_REGISTER_FILTERGRAPH
  if (vc->registered_graph) vc_RemoveGraphFromRot(vc->registered_graph);
#endif

  imVideoCaptureDisconnect(vc);

  delete vc->sample_callback;

  vc_SafeRelease(vc->overlay_mixer);
  vc_SafeRelease(vc->overlay_renderer);
  vc_SafeRelease(vc->media_control);
  vc_SafeRelease(vc->sample_grabber);

  vc->null_filter->Release();
  vc->grabber_filter->Release();
  vc->filter_builder->Release();
  vc->capture_graph_builder->Release();

  free(vc);
}

static void vc_StopLive(imVideoCapture* vc)
{
  if (vc->live)  // If it is live, stop it
  {
    vc->media_control->Stop();
    Sleep(VC_CAMERADELAY);
  }
}

static int vc_StartLive(imVideoCapture* vc)
{
  if (vc->live) // If it should be started, start it
  {
    HRESULT hr = vc->media_control->Run();
    if (VC_HARDFAILED(hr))
    {
      vc->live = 0;
      return 0;
    }

    Sleep(VC_CAMERADELAY);
  }

  return 1;
}

int imVideoCaptureOneFrame(imVideoCapture* vc, unsigned char* data, int color_mode)
{
  assert(vc);
  assert(vc->device != -1);

  vc_StopLive(vc);
  vc->live = 0;

  vc->sample_grabber->SetOneShot(TRUE);

  vc->live = 1;
  if (!vc_StartLive(vc))
  {
    vc->sample_grabber->SetOneShot(FALSE);
    return 0;
  }

  int ret = imVideoCaptureFrame(vc, data, color_mode, -1);

  vc_StopLive(vc);
  vc->live = 0;

  vc->sample_grabber->SetOneShot(FALSE);

  return ret;
}

int imVideoCaptureFrame(imVideoCapture* vc, unsigned char* data, int color_mode, int timeout)
{
  assert(vc);
  assert(vc->device != -1);
  assert(vc->live);
  return vc->sample_callback->GetImage(data, color_mode, timeout);
}

static int vc_CaptureDisconnect(imVideoCapture* vc)
{
  vc->sample_grabber->SetCallback(NULL, 0);

  if (vc->overlay_mixer)
    vc_ReleaseMixer(vc);

  vc_DisconnectFilters(vc->filter_builder, vc->grabber_filter, vc->null_filter);

  // Disconnect the grabber to preserve it
  if (!vc_DisconnectFilterPin(vc->filter_builder, vc->grabber_filter, PINDIR_INPUT))
    return 0;

  // Remove everything downstream the capture filter, except the null renderer
  vc_NukeDownstream(vc->filter_builder, vc->capture_filter);

  return 1;
}

void imVideoCaptureDisconnect(imVideoCapture* vc)
{
  assert(vc);

  if (vc->device == -1)
    return;

  vc_StopLive(vc);
  vc->live = 0;

  vc_CaptureDisconnect(vc);
  vc_CaptureRemove(vc);
}

static void vc_UpdateSize(imVideoCapture* vc)
{
  int width, height;
  imVideoCaptureGetImageSize(vc, &width, &height);
  vc->sample_callback->SetImageSize(width, height);
}

static void vc_UpdateDialogs(imVideoCapture* vc);
static void vc_UpdateFormatList(imVideoCapture* vc);

static int vc_CaptureConnect(imVideoCapture* vc)
{
  vc_CheckVideoPort(vc);

  if (!vc_ConnectFilters(vc->filter_builder, vc->capture_filter, vc->grabber_filter, 0))
  {
    vc_CaptureRemove(vc);
    return 0;
  }

  vc_ConnectFilters(vc->filter_builder, vc->grabber_filter, vc->null_filter, 1);

  vc_UpdateDialogs(vc);
  vc_UpdateFormatList(vc);
  vc_UpdateSize(vc);
  vc->sample_grabber->SetCallback(vc->sample_callback, 0);  // associate the sample_grabber with the sample_callback

  return 1;
}

int imVideoCaptureConnect(imVideoCapture* vc, int device)
{
  assert(vc);

  if (device == -1)
    return vc->device;

  if (device == vc->device)
    return 1;

  if (device < -1 || device > vc_DeviceCount)
    return 0;

  if (vc->device != -1)
    imVideoCaptureDisconnect(vc);

  vc->capture_filter = vc_DeviceList[device].filter;
  if (!vc->capture_filter)
    return 0;

  vc->filter_builder->AddFilter(vc->capture_filter, L"imCaptureSource");
  vc->device = device;

  if (!vc_CaptureConnect(vc))
    return 0;

  return 1;
}

int imVideoCaptureLive(imVideoCapture* vc, int live)
{
  assert(vc);

  if (live == -1)
    return vc->live;

  if (vc->device == -1)
    return 0;

  if (live == vc->live)
    return 1;

  if (live)
  {
    vc->live = 1;
    if (!vc_StartLive(vc))
      return 0;
  }
  else
  {
    vc_StopLive(vc); 
    vc->live = 0;
  }

  return 1;
}


/**************************************************************************
                            Format and Size
***************************************************************************/


void imVideoCaptureGetImageSize(imVideoCapture* vc, int *width, int *height)
{
  assert(vc);
  assert(vc->device != -1);

  AM_MEDIA_TYPE mt;
  ZeroMemory(&mt, sizeof(AM_MEDIA_TYPE));
  HRESULT hr = vc->sample_grabber->GetConnectedMediaType(&mt);

  if ( SUCCEEDED(hr) &&
      (mt.majortype == MEDIATYPE_Video) &&
      (mt.formattype == FORMAT_VideoInfo) &&
      (mt.cbFormat >= sizeof (VIDEOINFOHEADER)) &&
      (mt.pbFormat != NULL))
  {
    VIDEOINFOHEADER *pVih = (VIDEOINFOHEADER*)mt.pbFormat;
    *width = pVih->bmiHeader.biWidth;
    *height = abs(pVih->bmiHeader.biHeight);
    CoTaskMemFree((PVOID)mt.pbFormat);
  }
  else
  {
    *width = 0;
    *height = 0;
  }
}

static IIPDVDec* vc_GetDVDecoder(imVideoCapture* vc)
{
  IIPDVDec *pDV = NULL;
  HRESULT hr = vc->capture_graph_builder->FindInterface(NULL,
            &MEDIATYPE_Video, vc->capture_filter, IID_IIPDVDec, (void **)&pDV);
  if(FAILED(hr))
    return NULL;

  return pDV;
}

static IAMStreamConfig* vc_GetStreamConfig(imVideoCapture* vc)
{
  IAMStreamConfig *pSC = NULL;                           
  if (FAILED(vc->capture_graph_builder->FindInterface(&PIN_CATEGORY_CAPTURE,
        &MEDIATYPE_Video, vc->capture_filter, IID_IAMStreamConfig, (void **)&pSC)))
    return NULL;

  return pSC;
}

static void vc_DeleteMediaType(AM_MEDIA_TYPE *pmt)
{
  CoTaskMemFree((PVOID)pmt->pbFormat);
  CoTaskMemFree(pmt);
}

static int vc_SetStreamSize(imVideoCapture* vc, int width, int height)
{
  IAMStreamConfig *pSC = vc_GetStreamConfig(vc);
  if (!pSC) return 0;

  AM_MEDIA_TYPE *pmt;
  HRESULT hr = pSC->GetFormat(&pmt);
  if (FAILED(hr)) return 0;

  VIDEOINFOHEADER* vih = (VIDEOINFOHEADER*)pmt->pbFormat;
  BITMAPINFOHEADER* bih = &vih->bmiHeader;

  /* dibs are DWORD aligned */
  int data_size = height * ((width * bih->biBitCount + 31) / 32) * 4;   /* 4 bytes boundary */

  bih->biSize = sizeof(BITMAPINFOHEADER);
  bih->biHeight = height;
  bih->biWidth = width;
  bih->biSizeImage = data_size;

  int fps = 30;  // desired frame rate
  vih->dwBitRate = fps * data_size;
  vih->AvgTimePerFrame = 10000000 / fps;

  pmt->cbFormat = sizeof(VIDEOINFOHEADER);
  pmt->lSampleSize = data_size;

  hr = pSC->SetFormat(pmt);
  pSC->Release();

  vc_DeleteMediaType(pmt);

  return SUCCEEDED(hr);
}

static int vc_SetImageSize(imVideoCapture* vc, int width, int height)
{
  IIPDVDec* pDV = vc_GetDVDecoder(vc);
  if (pDV)
  {
    int size = 0;

    switch(width)
    {
    case 720:
      size = DVRESOLUTION_FULL;
      break;
    case 360:
      size = DVRESOLUTION_HALF;
      break;
    case 180:
      size = DVRESOLUTION_QUARTER;
      break;
    case 88:
      size = DVRESOLUTION_DC;
      break;
    }

    if (!size)
      return 0;

    int ret = SUCCEEDED(pDV->put_IPDisplay(size));
    if (ret) 
      vc->sample_callback->SetImageSize(width, height);

    return ret;
  }

  int ret = vc_SetStreamSize(vc, width, height);
  if (ret)
    vc->sample_callback->SetImageSize(width, height);

  return ret;
}

int imVideoCaptureSetImageSize(imVideoCapture* vc, int width, int height)
{
  assert(vc);
  assert(vc->device != -1);

  vc_StopLive(vc);

  // must be disconnected to change size or format
  vc_CaptureDisconnect(vc);

  int ret = vc_SetImageSize(vc, width, height);

  if (!vc_CaptureConnect(vc))
    ret = 0;

  vc_StartLive(vc);

  return ret;
}

static void vc_UpdateFormatList(imVideoCapture* vc)
{
  vc->format_count = 0;
  vc->format_current = -1;

  IAMStreamConfig *pSC = vc_GetStreamConfig(vc);
  if (!pSC) return;

  int iCount = 0, iSize = 0;
  if (FAILED(pSC->GetNumberOfCapabilities(&iCount, &iSize)))
  {
    pSC->Release();
    return;
  }

  AM_MEDIA_TYPE *curr_pmt;
  HRESULT hr = pSC->GetFormat(&curr_pmt);
  if (FAILED(hr)) 
  {
    pSC->Release();
    return;
  }

  for (int iFormat = 0; iFormat < iCount; iFormat++)
  {
    VIDEO_STREAM_CONFIG_CAPS scc;
    AM_MEDIA_TYPE *pmt;
    if (SUCCEEDED(pSC->GetStreamCaps(iFormat, &pmt, (BYTE*)&scc)))
    {
      if (scc.guid == FORMAT_VideoInfo)
      {
        VIDEOINFOHEADER* vih = (VIDEOINFOHEADER*)curr_pmt->pbFormat;
        BITMAPINFOHEADER* bih = &vih->bmiHeader;
        int width = bih->biWidth;
        int height = abs(bih->biHeight);

        if (curr_pmt->subtype == pmt->subtype && 
            width == scc.InputSize.cx && 
            height == scc.InputSize.cy)
        {
          vc->format_current = vc->format_count;
        }

        vc->format_map[vc->format_count] = iFormat;
        vc->format_count++;
      }                                    

      vc_DeleteMediaType(pmt);
    }
  }

  vc_DeleteMediaType(curr_pmt);
  pSC->Release();
}

int imVideoCaptureFormatCount(imVideoCapture* vc)
{
  assert(vc);
  assert(vc->device != -1);

  return vc->format_count;
}

static void vc_GetFormatName(GUID subtype, char* desc)
{
#define VC_NUM_FORMATS 7
  typedef struct _guid2name {
    char* name;
    const GUID* subtype;
  } guid2name;
  static guid2name map_table[VC_NUM_FORMATS] = {
    {"RGB1",&MEDIASUBTYPE_RGB1},
    {"RGB4",&MEDIASUBTYPE_RGB4},
    {"RGB8",&MEDIASUBTYPE_RGB8},
    {"RGB565",&MEDIASUBTYPE_RGB565},
    {"RGB555",&MEDIASUBTYPE_RGB555},
    {"RGB24",&MEDIASUBTYPE_RGB24},
    {"RGB32",&MEDIASUBTYPE_RGB32}
  };

  for (int i = 0; i < VC_NUM_FORMATS; i++)
  {
    if (*(map_table[i].subtype) == subtype)
    {
      strcpy(desc, map_table[i].name);
      return;
    }
  }

  desc[0] = (char)(subtype.Data1);
  desc[1] = (char)(subtype.Data1 >> 8);
  desc[2] = (char)(subtype.Data1 >> 16);
  desc[3] = (char)(subtype.Data1 >> 24);
  desc[4] = 0;      
}

int imVideoCaptureGetFormat(imVideoCapture* vc, int format, int *width, int *height, char* desc)
{
  assert(vc);
  assert(vc->device != -1);
  assert(vc->format_count);

  if (format >= vc->format_count)
    return 0;

  IAMStreamConfig *pSC = vc_GetStreamConfig(vc);
  if (!pSC) return 0;

  VIDEO_STREAM_CONFIG_CAPS scc;
  AM_MEDIA_TYPE *pmt;
  if (SUCCEEDED(pSC->GetStreamCaps(vc->format_map[format], &pmt, (BYTE*)&scc)))
  {
    *width = scc.InputSize.cx;
    *height = scc.InputSize.cy;
    vc_GetFormatName(pmt->subtype, desc);

    pSC->Release();
    vc_DeleteMediaType(pmt);
    return 1;
  }

  pSC->Release();
  return 0;
}

static int vc_SetStreamFormat(imVideoCapture* vc, int format)
{
  IAMStreamConfig *pSC = vc_GetStreamConfig(vc);
  if (!pSC) return 0;

  VIDEO_STREAM_CONFIG_CAPS scc;
  AM_MEDIA_TYPE *pmt;
  if (FAILED(pSC->GetStreamCaps(vc->format_map[format], &pmt, (BYTE*)&scc)))
  {
    pSC->Release();
    return 0;
  }

  pSC->SetFormat(pmt);
  pSC->Release();

  vc->sample_callback->SetImageSize(scc.InputSize.cx, scc.InputSize.cy);

  vc_DeleteMediaType(pmt);

  return 1;
}

int imVideoCaptureSetFormat(imVideoCapture* vc, int format)
{
  assert(vc);
  assert(vc->device != -1);

  if (format == -1)
    return vc->format_current;

  if (format >= vc->format_count)
    return 0;

  vc_StopLive(vc);

  // must be disconnected to change size or format
  vc_CaptureDisconnect(vc);

  int ok = vc_SetStreamFormat(vc, format);

  if (!vc_CaptureConnect(vc))
    ok = 0;

  if (ok)
    vc->format_current = format;

  vc_StartLive(vc);

  return ok;
}


/**************************************************************************
                            Dialogs
***************************************************************************/


static ISpecifyPropertyPages* vc_GetPropertyPages(IUnknown* obj)
{
  ISpecifyPropertyPages *pSpec = NULL;

  HRESULT hr = obj->QueryInterface(IID_ISpecifyPropertyPages,  (void **)&pSpec);
  if (FAILED(hr)) return NULL;

  CAUUID cauuid;
  hr = pSpec->GetPages(&cauuid);
  CoTaskMemFree(cauuid.pElems);

  if (FAILED(hr)) 
  {
    pSpec->Release();
    return NULL;
  }

  return pSpec;
}

static int vc_ShowPropertyPages(HWND parent, IUnknown* obj, WCHAR* title)
{
  ISpecifyPropertyPages *pSpec = vc_GetPropertyPages(obj);

  CAUUID cauuid;
  pSpec->GetPages(&cauuid);

  HRESULT hr = OleCreatePropertyFrame(parent, 30, 30, title, 1,
                        &obj, cauuid.cElems, (GUID *)cauuid.pElems, 0, 0, NULL);

  CoTaskMemFree(cauuid.pElems);
  pSpec->Release();

  if (FAILED(hr)) return 0;
  return 1;
}

static IAMVfwCaptureDialogs* vc_getVfwDialogs(imVideoCapture* vc)
{
  IAMVfwCaptureDialogs* pDlg = NULL;
  HRESULT hr = vc->capture_graph_builder->FindInterface(&PIN_CATEGORY_CAPTURE,
        &MEDIATYPE_Video, vc->capture_filter, IID_IAMVfwCaptureDialogs, (void **)&pDlg);

  if (FAILED(hr))
    return NULL;

  return pDlg;
}

static int vc_ShowVfwDialog(imVideoCapture* vc, HWND parent, VfwCaptureDialogs dialog)
{
  assert(vc);
  assert(vc->device != -1);

  IAMVfwCaptureDialogs *pDlg = vc_getVfwDialogs(vc);
  if(!pDlg) return 0;

  HRESULT hr = pDlg->HasDialog(dialog);
  if (FAILED(hr))
  {
    pDlg->Release();
    return 0;
  }

  int ret = 0;
  vc_StopLive(vc);

  // must be disconnected to change size or format
  vc_CaptureDisconnect(vc);

  hr = pDlg->ShowDialog(dialog, parent);
  if (SUCCEEDED(hr))
    ret = 1;

  if (!vc_CaptureConnect(vc))
    ret = 0;

  vc_StartLive(vc);

  pDlg->Release();
  return ret;
}

static int vc_ShowVfwFormatDialog(imVideoCapture* vc, HWND parent)
{
  return vc_ShowVfwDialog(vc, parent, VfwCaptureDialog_Format);
}

static int vc_ShowVfwSourceDialog(imVideoCapture* vc, HWND parent)
{
  return vc_ShowVfwDialog(vc, parent, VfwCaptureDialog_Source);
}

static int vc_ShowVfwDisplayDialog(imVideoCapture* vc, HWND parent)
{
  return vc_ShowVfwDialog(vc, parent, VfwCaptureDialog_Display);
}

static int vc_ShowFormatDialog(imVideoCapture* vc, HWND parent)
{
  assert(vc);
  assert(vc->device != -1);

  IAMStreamConfig *pSC = vc_GetStreamConfig(vc);
  if (!pSC) return 0;

  vc_StopLive(vc);

  // must be disconnected to change size or format
  vc_CaptureDisconnect(vc);

  int ok = vc_ShowPropertyPages(parent, (IUnknown*)pSC, L"Format");
  pSC->Release();

  if (!vc_CaptureConnect(vc))
    ok = 0;

  vc_StartLive(vc);

  return ok;
}

static int vc_ShowSourceDialog(imVideoCapture* vc, HWND parent)
{
  assert(vc);
  assert(vc->device != -1);

  return vc_ShowPropertyPages(parent, (IUnknown*)vc->capture_filter, L"Source");
}

static IAMTVTuner* vc_GetTVTuner(imVideoCapture* vc)
{
  IAMTVTuner *pTVT = NULL;

  HRESULT hr = vc->capture_graph_builder->FindInterface(&PIN_CATEGORY_CAPTURE,
            &MEDIATYPE_Video, vc->capture_filter, IID_IAMTVTuner, (void **)&pTVT);
  if(FAILED(hr))
    return NULL;

  return pTVT;
}

static int vc_ShowTVTunerDialog(imVideoCapture* vc, HWND parent)
{
  assert(vc);
  assert(vc->device != -1);

  IAMTVTuner* pTVT = vc_GetTVTuner(vc);
  if (!pTVT)
    return 0;

  int ret = vc_ShowPropertyPages(parent, (IUnknown*)pTVT, L"TV Turner");
  pTVT->Release();
  return ret;
}

static IAMCrossbar* vc_GetCrossBar(imVideoCapture* vc)
{
  IAMCrossbar *pX = NULL;
  HRESULT hr = vc->capture_graph_builder->FindInterface(&PIN_CATEGORY_CAPTURE,
            &MEDIATYPE_Video, vc->capture_filter, IID_IAMCrossbar, (void **)&pX);
  if(FAILED(hr))
    return NULL;

  return pX;
}

static int vc_ShowCrossbarDialog(imVideoCapture* vc, HWND parent)
{
  assert(vc);
  assert(vc->device != -1);

  IAMCrossbar* pX = vc_GetCrossBar(vc);
  if (!pX)
    return 0;

  int ret = vc_ShowPropertyPages(parent, (IUnknown*)pX, L"Crossbar");
  pX->Release();
  return ret;
}

static IAMCrossbar* vc_GetSecondCrossBar(imVideoCapture* vc, IAMCrossbar *pX)
{
  IAMCrossbar *pX2 = NULL;
  IBaseFilter *pXF;
  HRESULT hr = pX->QueryInterface(IID_IBaseFilter, (void **)&pXF);
  if(hr != S_OK) return NULL;

  hr = vc->capture_graph_builder->FindInterface(&LOOK_UPSTREAM_ONLY,
                 NULL, pXF, IID_IAMCrossbar, (void **)&pX2);
  pXF->Release();
  if(FAILED(hr)) return NULL;

  return pX2;
}

static int vc_ShowSecondCrossbarDialog(imVideoCapture* vc, HWND parent)
{
  assert(vc);
  assert(vc->device != -1);

  IAMCrossbar* pX = vc_GetCrossBar(vc);
  if (!pX)
    return 0;

  IAMCrossbar* pX2 = vc_GetSecondCrossBar(vc, pX);
  if (!pX2)
  {
    pX->Release();
    return 0;
  }

  int ret = vc_ShowPropertyPages(parent, (IUnknown*)pX2, L"Second Crossbar");
  pX->Release();
  pX2->Release();
  return ret;
}

static int vc_ShowDVDecDialog(imVideoCapture* vc, HWND parent)
{
  assert(vc);
  assert(vc->device != -1);

  IIPDVDec* pDV = vc_GetDVDecoder(vc);
  if (!pDV)
    return 0;

  vc_StopLive(vc);

  int ret = vc_ShowPropertyPages(parent, (IUnknown*)pDV, L"DV Decoder");
  pDV->Release();

  vc_StartLive(vc);

  return ret;
}

static void vc_UpdateDialogs(imVideoCapture* vc)
{
  vc->dialog_count = 0;

  IAMVfwCaptureDialogs *pDlg = vc_getVfwDialogs(vc);
  if(pDlg)
  {
    if(pDlg->HasDialog(VfwCaptureDialog_Format) == S_OK)
    {
      vc->dialog_desc[vc->dialog_count] = "Format... (VFW)";
      vc->dialog_func[vc->dialog_count] = vc_ShowVfwFormatDialog;
      vc->dialog_count++;
    }

    if(pDlg->HasDialog(VfwCaptureDialog_Source) == S_OK)
    {
      vc->dialog_desc[vc->dialog_count] = "Source... (VFW)";
      vc->dialog_func[vc->dialog_count] = vc_ShowVfwSourceDialog;
      vc->dialog_count++;
    }

    if(pDlg->HasDialog(VfwCaptureDialog_Display) == S_OK)
    {
      vc->dialog_desc[vc->dialog_count] = "Display... (VFW)";
      vc->dialog_func[vc->dialog_count] = vc_ShowVfwDisplayDialog;
      vc->dialog_count++;
    }

    return;
  }

  ISpecifyPropertyPages *pSpec;
  IAMStreamConfig *pSC = vc_GetStreamConfig(vc);
  if (pSC)
  {
    pSpec = vc_GetPropertyPages((IUnknown*)pSC);
    if (pSpec)
    {
      vc->dialog_desc[vc->dialog_count] = "Format...";
      vc->dialog_func[vc->dialog_count] = vc_ShowFormatDialog;
      vc->dialog_count++;
      pSpec->Release();  
    }

    pSC->Release();  
  }

  pSpec = vc_GetPropertyPages((IUnknown*)vc->capture_filter);
  if (pSpec)
  {
    vc->dialog_desc[vc->dialog_count] = "Source...";
    vc->dialog_func[vc->dialog_count] = vc_ShowSourceDialog;
    vc->dialog_count++;
    pSpec->Release();  
  }

  IIPDVDec* pDV = vc_GetDVDecoder(vc);
  if (pDV)
  {
    pSpec = vc_GetPropertyPages((IUnknown*)pDV);
    if (pSpec)
    {
      vc->dialog_desc[vc->dialog_count] = "DV Decoder...";
      vc->dialog_func[vc->dialog_count] = vc_ShowDVDecDialog;
      vc->dialog_count++;
      pSpec->Release();  
    }

    pDV->Release();  
  }

  IAMCrossbar* pX = vc_GetCrossBar(vc);
  if (pX)
  {
    pSpec = vc_GetPropertyPages((IUnknown*)pX);
    if (pSpec)
    {
      vc->dialog_desc[vc->dialog_count] = "Crossbar...";
      vc->dialog_func[vc->dialog_count] = vc_ShowCrossbarDialog;
      vc->dialog_count++;
      pSpec->Release();  
    }

    IAMCrossbar* pX2 = vc_GetSecondCrossBar(vc, pX);
    if (pX2)
    {
      pSpec = vc_GetPropertyPages((IUnknown*)pX2);
      if (pSpec)
      {
        vc->dialog_desc[vc->dialog_count] = "Second Crossbar...";
        vc->dialog_func[vc->dialog_count] = vc_ShowSecondCrossbarDialog;
        vc->dialog_count++;
        pSpec->Release();  
      }

      pX2->Release();  
    }

    pX->Release();  
  }

  IAMTVTuner* pTVT = vc_GetTVTuner(vc);
  if (pTVT)
  {
    pSpec = vc_GetPropertyPages((IUnknown*)pTVT);
    if (pSpec)
    {
      vc->dialog_desc[vc->dialog_count] = "TV Tuner...";
      vc->dialog_func[vc->dialog_count] = vc_ShowTVTunerDialog;
      vc->dialog_count++;
      pSpec->Release();  
    }

    pTVT->Release();  
  }
}

int imVideoCaptureDialogCount(imVideoCapture* vc)
{
  assert(vc);
  assert(vc->device != -1);

  return vc->dialog_count;
}

const char* imVideoCaptureDialogDesc(imVideoCapture* vc, int dialog)
{
  assert(vc);
  assert(vc->device != -1);

  if (dialog >= vc->dialog_count)
    return NULL;

  return vc->dialog_desc[dialog];
}

int imVideoCaptureShowDialog(imVideoCapture* vc, int dialog, void* parent)
{
  assert(vc);
  assert(vc->device != -1);

  if (dialog >= vc->dialog_count)
    return 0;

  return vc->dialog_func[dialog](vc, (HWND)parent);
}

int imVideoCaptureSetInOut(imVideoCapture* vc, int input, int output, int cross)
{
  assert(vc);
  assert(vc->device != -1);

  IAMCrossbar* pX = vc_GetCrossBar(vc);
  if (pX)
  {
    HRESULT hr = S_FALSE;

    if (cross == 1)
      hr = pX->Route(output, input);
    else
    {
      IAMCrossbar* pX2 = vc_GetSecondCrossBar(vc, pX);
      if (pX2)
      {
        hr = pX2->Route(output, input);
        pX2->Release();
      }
    }

    pX->Release();
    if (hr == S_OK)
      return 1;
  }

  return 0;
}

/**************************************************************************
                            Attributes
***************************************************************************/


static float vc_Value2Percent(long Min, long Max, long Val)
{
  return ((Val - Min)*100.0f)/((float)(Max - Min));
}

static long vc_Percent2Value(long Min, long Max, long Step, float Per)
{
  long Val = (long)((Per/100.)*(Max - Min) + Min);
  if (Step == 1)
    return Val;

  long num_step = (Val - Min + Step-1) / Step;
  return num_step*Step + Min;
}

static IAMVideoProcAmp* vc_InitVideoProcAmp(IBaseFilter* capture_filter, IAMVideoProcAmp* *video_prop)
{
  if (*video_prop)
    return *video_prop;

  HRESULT hr = capture_filter->QueryInterface(IID_IAMVideoProcAmp, (void**)video_prop);
  if (FAILED(hr))
    return NULL;

  return *video_prop;
}

static int vc_SetVideoProcAmpProperty(IBaseFilter* capture_filter, IAMVideoProcAmp* *video_prop, long property, float percent)
{
  IAMVideoProcAmp *pProp = vc_InitVideoProcAmp(capture_filter, video_prop);
  if (!pProp) return 0;
  HRESULT hr;
  VideoProcAmpProperty prop = (VideoProcAmpProperty)property;
  long Min, Max, Step, Default, Flags;
  hr = pProp->GetRange(prop, &Min, &Max, &Step, &Default, &Flags);
  hr = pProp->Set(prop, vc_Percent2Value(Min, Max, Step, percent), VideoProcAmp_Flags_Manual);
  if (FAILED(hr)) return 0;
  return 1;
}

static int vc_GetVideoProcAmpProperty(IBaseFilter* capture_filter, IAMVideoProcAmp* *video_prop, long property, float *percent)
{
  IAMVideoProcAmp *pProp = vc_InitVideoProcAmp(capture_filter, video_prop);
  if (!pProp) return 0;

  HRESULT hr;
  VideoProcAmpProperty prop = (VideoProcAmpProperty)property;
  long Min, Max, Step, Default, Flags, Val;
  hr = pProp->GetRange(prop, &Min, &Max, &Step, &Default, &Flags);
  hr = pProp->Get(prop, &Val, &Flags);

  if (FAILED(hr)) return 0;
  *percent = vc_Value2Percent(Min, Max, Val);
  return 1;
}

static int vc_ResetVideoProcAmpProperty(IBaseFilter* capture_filter, IAMVideoProcAmp* *video_prop, long property, int fauto)
{
  IAMVideoProcAmp *pProp = vc_InitVideoProcAmp(capture_filter, video_prop);
  if (!pProp) return 0;

  HRESULT hr;
  VideoProcAmpProperty prop = (VideoProcAmpProperty)property;
  long Min, Max, Step, Default, Flags;
  hr = pProp->GetRange(prop, &Min, &Max, &Step, &Default, &Flags);

  if (fauto && (Flags & VideoProcAmp_Flags_Auto))
    hr = pProp->Set(prop, Default, VideoProcAmp_Flags_Auto);
  else
    hr = pProp->Set(prop, Default, VideoProcAmp_Flags_Manual);

  if (FAILED(hr)) return 0;
  return 1;
}

static IAMCameraControl* vc_InitCameraControl(IBaseFilter* capture_filter, IAMCameraControl* *camera_prop)
{
  if (*camera_prop)
    return *camera_prop;

  HRESULT hr = capture_filter->QueryInterface(IID_IAMCameraControl, (void**)camera_prop);
  if (FAILED(hr))
    return NULL;

  return *camera_prop;
}

static int vc_SetCameraControlProperty(IBaseFilter* capture_filter, IAMCameraControl* *camera_prop, long property, float percent)
{
  IAMCameraControl *pProp = vc_InitCameraControl(capture_filter, camera_prop);
  if (!pProp) return 0;

  HRESULT hr;
  CameraControlProperty prop = (CameraControlProperty)property;
  long Min, Max, Step, Default, Flags;
  hr = pProp->GetRange(prop, &Min, &Max, &Step, &Default, &Flags);
  hr = pProp->Set(prop, vc_Percent2Value(Min, Max, Step, percent), CameraControl_Flags_Manual);

  if (FAILED(hr)) return 0;
  return 1;
}

static int vc_GetCameraControlProperty(IBaseFilter* capture_filter, IAMCameraControl* *camera_prop, long property, float *percent)
{
  IAMCameraControl *pProp = vc_InitCameraControl(capture_filter, camera_prop);
  if (!pProp) return 0;

  HRESULT hr;
  CameraControlProperty prop = (CameraControlProperty)property;
  long Min, Max, Step, Default, Flags, Val;
  hr = pProp->GetRange(prop, &Min, &Max, &Step, &Default, &Flags);
  hr = pProp->Get(prop, &Val, &Flags);

  if (FAILED(hr)) return 0;
  *percent = vc_Value2Percent(Min, Max, Val);
  return 1;
}

static int vc_ResetCameraControlProperty(IBaseFilter* capture_filter, IAMCameraControl* *camera_prop, long property, int fauto)
{
  IAMCameraControl *pProp = vc_InitCameraControl(capture_filter, camera_prop);
  if (!pProp) return 0;

  HRESULT hr;
  CameraControlProperty prop = (CameraControlProperty)property;
  long Min, Max, Step, Default, Flags;
  hr = pProp->GetRange(prop, &Min, &Max, &Step, &Default, &Flags);

  if (fauto && (Flags & CameraControl_Flags_Auto))
    hr = pProp->Set(prop, Default, CameraControl_Flags_Auto);
  else
    hr = pProp->Set(prop, Default, CameraControl_Flags_Manual);

  if (FAILED(hr)) return 0;
  return 1;
}

static IAMVideoControl* vc_InitVideoControl(IBaseFilter* capture_filter, IAMVideoControl* *video_prop)
{
  if (*video_prop)
    return *video_prop;

  HRESULT hr = capture_filter->QueryInterface(IID_IAMVideoControl, (void**)video_prop);
  if (FAILED(hr))
    return NULL;

  return *video_prop;
}

static int vc_SetVideoControlProperty(IBaseFilter* capture_filter, IAMVideoControl* *video_prop, long property, float percent)
{
  IAMVideoControl *pProp = vc_InitVideoControl(capture_filter, video_prop);
  if (!pProp) return 0;

  HRESULT hr;
  IPin *pOutPin = vc_GetPin(capture_filter, PINDIR_OUTPUT);
  long Mode;
  hr = pProp->GetMode(pOutPin, &Mode);
  if (percent)
    Mode = Mode | property;
  else
    Mode = Mode & ~property;
  hr = pProp->SetMode(pOutPin, Mode);
  pOutPin->Release();

  if (FAILED(hr)) return 0;
  return 1;
}

static int vc_GetVideoControlProperty(IBaseFilter* capture_filter, IAMVideoControl* *video_prop, long property, float *percent)
{
  IAMVideoControl *pProp = vc_InitVideoControl(capture_filter, video_prop);
  if (!pProp) return 0;

  HRESULT hr;
  long Mode;
  IPin *pOutPin = vc_GetPin(capture_filter, PINDIR_OUTPUT);
  hr = pProp->GetMode(pOutPin, &Mode);
  pOutPin->Release();

  if (FAILED(hr)) return 0;
  if (Mode & property)
    *percent = 100.;
  else
    *percent = 0.;
  return 1;
}

static int vc_ResetVideoControlProperty(IBaseFilter* capture_filter, IAMVideoControl* *video_prop, long property, int fauto)
{
  IAMVideoControl *pProp = vc_InitVideoControl(capture_filter, video_prop);
  if (!pProp) return 0;

  HRESULT hr;
  long Mode;
  IPin *pOutPin = vc_GetPin(capture_filter, PINDIR_OUTPUT);
  hr = pProp->GetMode(pOutPin, &Mode);
  if (Mode & property)
    Mode = Mode & ~property;
  else
    Mode = Mode | property;
  hr = pProp->SetMode(pOutPin, Mode);
  pOutPin->Release();

  if (FAILED(hr)) return 0;
  return 1;
}

static long vc_AnalogFormat[19] =
{
  AnalogVideo_NTSC_M, 
  AnalogVideo_NTSC_M_J,  
  AnalogVideo_NTSC_433,
  AnalogVideo_PAL_B,
  AnalogVideo_PAL_D,
  AnalogVideo_PAL_H,
  AnalogVideo_PAL_I,
  AnalogVideo_PAL_M,
  AnalogVideo_PAL_N,
  AnalogVideo_PAL_60,
  AnalogVideo_SECAM_B,
  AnalogVideo_SECAM_D,
  AnalogVideo_SECAM_G,
  AnalogVideo_SECAM_H,
  AnalogVideo_SECAM_K,
  AnalogVideo_SECAM_K1,
  AnalogVideo_SECAM_L,
  AnalogVideo_SECAM_L1,
  AnalogVideo_PAL_N_COMBO
};

static int vc_SetAnalogFormat(IBaseFilter* capture_filter, float percent)
{
  IAMAnalogVideoDecoder* video_decoder = NULL;
  HRESULT hr = capture_filter->QueryInterface(IID_IAMAnalogVideoDecoder, (void**)video_decoder);
  if (FAILED(hr))
    return 0;

  hr = video_decoder->put_TVFormat(vc_AnalogFormat[(int)percent]);
  video_decoder->Release();

  if (FAILED(hr)) return 0;
  return 1;
}

static int vc_GetAnalogFormat(IBaseFilter* capture_filter, float *percent)
{
  IAMAnalogVideoDecoder* video_decoder = NULL;
  HRESULT hr = capture_filter->QueryInterface(IID_IAMAnalogVideoDecoder, (void**)video_decoder);
  if (FAILED(hr))
    return 0;

  long format;
  hr = video_decoder->get_TVFormat(&format);
  video_decoder->Release();

  if (FAILED(hr)) return 0;
  for (int i = 0; i < 19; i++)
  {
    if (vc_AnalogFormat[i] == format)
    {
      *percent = (float)i;
      return 1;
    }
  }
  return 0;
}

#define VC_HASH_SIZE 101
#define VC_HASH_MULTIPLIER 31

/** Unique Hash index for a key
 * We use the hash function described in "The Pratice of Programming" of Kernighan & Pike. */
static int vc_HashIndex(const char *key, int hash_size)
{
  unsigned short hash = 0;
  const unsigned char *p_key = (const unsigned char*)key;

  for(; *p_key; p_key++)
    hash = hash*VC_HASH_MULTIPLIER + *p_key;

  return hash % hash_size;
}

#define VC_CAMERASHIFT 20
#define VC_VIDEOSHIFT  40
#define VC_ANALOGSHIFT 60

static long vc_Attrib2Property(const char* attrib)
{
  static long prop_table[VC_HASH_SIZE];
  static int first = 1;
  if (first)
  {
    memset(prop_table, 0, VC_HASH_SIZE*sizeof(long));
    prop_table[vc_HashIndex("CameraPanAngle", VC_HASH_SIZE)] = (long)CameraControl_Pan + VC_CAMERASHIFT + 1;
    prop_table[vc_HashIndex("CameraTiltAngle", VC_HASH_SIZE)] = (long)CameraControl_Tilt + VC_CAMERASHIFT + 1;
    prop_table[vc_HashIndex("CameraRollAngle", VC_HASH_SIZE)] = (long)CameraControl_Roll + VC_CAMERASHIFT + 1;
    prop_table[vc_HashIndex("CameraLensZoom", VC_HASH_SIZE)] = (long)CameraControl_Zoom + VC_CAMERASHIFT + 1;
    prop_table[vc_HashIndex("CameraExposure", VC_HASH_SIZE)] = (long)CameraControl_Exposure + VC_CAMERASHIFT + 1;
    prop_table[vc_HashIndex("CameraIris", VC_HASH_SIZE)] = (long)CameraControl_Iris + VC_CAMERASHIFT + 1;
    prop_table[vc_HashIndex("CameraFocus", VC_HASH_SIZE)] = (long)CameraControl_Focus + VC_CAMERASHIFT + 1;
    prop_table[vc_HashIndex("VideoBrightness", VC_HASH_SIZE)] = (long)VideoProcAmp_Brightness + 1;
    prop_table[vc_HashIndex("VideoContrast", VC_HASH_SIZE)] = (long)VideoProcAmp_Contrast + 1;
    prop_table[vc_HashIndex("VideoHue", VC_HASH_SIZE)] = (long)VideoProcAmp_Hue + 1;
    prop_table[vc_HashIndex("VideoSaturation", VC_HASH_SIZE)] = (long)VideoProcAmp_Saturation + 1;
    prop_table[vc_HashIndex("VideoSharpness", VC_HASH_SIZE)] = (long)VideoProcAmp_Sharpness + 1;
    prop_table[vc_HashIndex("VideoGamma", VC_HASH_SIZE)] = (long)VideoProcAmp_Gamma + 1;
    prop_table[vc_HashIndex("VideoColorEnable", VC_HASH_SIZE)] = (long)VideoProcAmp_ColorEnable + 1;
    prop_table[vc_HashIndex("VideoWhiteBalance", VC_HASH_SIZE)] = (long)VideoProcAmp_WhiteBalance + 1;
    prop_table[vc_HashIndex("VideoBacklightCompensation", VC_HASH_SIZE)] = (long)VideoProcAmp_BacklightCompensation + 1;
    prop_table[vc_HashIndex("VideoGain", VC_HASH_SIZE)] = (long)VideoProcAmp_Gain + 1;
    prop_table[vc_HashIndex("FlipHorizontal", VC_HASH_SIZE)] = (long)VideoControlFlag_FlipHorizontal + VC_VIDEOSHIFT + 1;
    prop_table[vc_HashIndex("FlipVertical", VC_HASH_SIZE)] = (long)VideoControlFlag_FlipVertical + VC_VIDEOSHIFT + 1;
    prop_table[vc_HashIndex("AnalogFormat", VC_HASH_SIZE)] = (long)0 + VC_ANALOGSHIFT + 1;
    first = 0;
  }
  long prop = prop_table[vc_HashIndex(attrib, VC_HASH_SIZE)];
  if (!prop)
    return 0;
  return prop-1;
}

int imVideoCaptureSetAttribute(imVideoCapture* vc, const char* attrib, float percent)
{
  assert(vc);
  assert(vc->device != -1);

  long property = vc_Attrib2Property(attrib);
  if (property == -1) return 0;
  if (property < VC_CAMERASHIFT)
    return vc_SetVideoProcAmpProperty(vc->capture_filter, &vc->video_prop, property, percent);
  else if (property < VC_VIDEOSHIFT)
    return vc_SetCameraControlProperty(vc->capture_filter, &vc->camera_prop, property-VC_CAMERASHIFT, percent);
  else if (property < VC_ANALOGSHIFT)
    return vc_SetVideoControlProperty(vc->capture_filter, &vc->videoctrl_prop, property-VC_VIDEOSHIFT, percent);
  else
    return vc_SetAnalogFormat(vc->capture_filter, percent);
}

int imVideoCaptureGetAttribute(imVideoCapture* vc, const char* attrib, float *percent)
{
  assert(vc);
  assert(vc->device != -1);

  long property = vc_Attrib2Property(attrib);
  if (property == -1) return 0;
  if (property < VC_CAMERASHIFT)
    return vc_GetVideoProcAmpProperty(vc->capture_filter, &vc->video_prop, property, percent);
  else if (property < VC_VIDEOSHIFT)
    return vc_GetCameraControlProperty(vc->capture_filter, &vc->camera_prop, property-VC_CAMERASHIFT, percent);
  else if (property < VC_ANALOGSHIFT)
    return vc_GetVideoControlProperty(vc->capture_filter, &vc->videoctrl_prop, property-VC_VIDEOSHIFT, percent);
  else
    return vc_GetAnalogFormat(vc->capture_filter, percent);
}

int imVideoCaptureResetAttribute(imVideoCapture* vc, const char* attrib, int fauto)
{
  assert(vc);
  assert(vc->device != -1);

  long property = vc_Attrib2Property(attrib);
  if (property == -1) return 0;
  if (property < VC_CAMERASHIFT)
    return vc_ResetVideoProcAmpProperty(vc->capture_filter, &vc->video_prop, property, fauto);
  else if (property < VC_VIDEOSHIFT)
    return vc_ResetCameraControlProperty(vc->capture_filter, &vc->camera_prop, property-VC_CAMERASHIFT, fauto);
  else if (property < VC_ANALOGSHIFT)
    return vc_ResetVideoControlProperty(vc->capture_filter, &vc->videoctrl_prop, property-VC_VIDEOSHIFT, fauto);
  return 0;
}

const char** imVideoCaptureGetAttributeList(imVideoCapture* vc, int *num_attrib)
{
#define VC_VIDEOPROC_MAX 10
#define VC_CAMERACONTROL_MAX 7
#define VC_VIDEOCONTROL_MAX 2
#define VC_VIDEODECODER_MAX 1
#define VC_NUM_ATTRIB_MAX (VC_VIDEOPROC_MAX+VC_CAMERACONTROL_MAX+VC_VIDEOCONTROL_MAX+VC_VIDEODECODER_MAX)
  static char* attrib_list[VC_NUM_ATTRIB_MAX];
  static char* all_attrib_list[VC_NUM_ATTRIB_MAX] = 
  {                                  //Pre-calculated Hash Index:
    "VideoBrightness",               //  (97)
    "VideoContrast",                 //  (80)
    "VideoHue",                      //  (98)
    "VideoSaturation",               //  (4)
    "VideoSharpness",                //  (56)
    "VideoGamma",                    //  (67)
    "VideoColorEnable",              //  (91)
    "VideoWhiteBalance",             //  (26)
    "VideoBacklightCompensation",    //  (50)
    "VideoGain",                     //  (36)
    "CameraPanAngle",                //  (64)
    "CameraTiltAngle",               //  (54)
    "CameraRollAngle",               //  (85)
    "CameraLensZoom",                //  (57)
    "CameraExposure",                //  (84)
    "CameraIris",                    //  (20)
    "CameraFocus",                   //  (62)
    "FlipHorizontal",                //  (21)
    "FlipVertical",                  //  (28)
    "AnalogFormat"};                 //  (89)

  int i;
  *num_attrib = 0;

  IAMVideoProcAmp *video_prop;
  HRESULT hr = vc->capture_filter->QueryInterface(IID_IAMVideoProcAmp, (void**)&video_prop);
  if (SUCCEEDED(hr))
  {
    for (i = 0; i < VC_VIDEOPROC_MAX; i++)
      attrib_list[i] = all_attrib_list[i];
    *num_attrib = VC_VIDEOPROC_MAX;
    video_prop->Release();
  }

  IAMCameraControl *camera_prop;
  hr = vc->capture_filter->QueryInterface(IID_IAMCameraControl, (void**)&camera_prop);
  if (SUCCEEDED(hr))
  {
    for (i = 0; i < VC_CAMERACONTROL_MAX; i++)
      attrib_list[i+*num_attrib] = all_attrib_list[i+VC_VIDEOPROC_MAX];
    *num_attrib += VC_CAMERACONTROL_MAX;
    camera_prop->Release();
  }

  IAMVideoControl* video_ctrl;
  hr = vc->capture_filter->QueryInterface(IID_IAMVideoControl, (void**)&video_ctrl);
  if (SUCCEEDED(hr))
  {
    for (i = 0; i < VC_VIDEOCONTROL_MAX; i++)
      attrib_list[i+*num_attrib] = all_attrib_list[i+VC_VIDEOPROC_MAX+VC_CAMERACONTROL_MAX];
    *num_attrib += VC_VIDEOCONTROL_MAX;
    video_ctrl->Release();
  }

  IAMAnalogVideoDecoder* video_decoder = NULL;
  hr = vc->capture_filter->QueryInterface(IID_IAMAnalogVideoDecoder, (void**)&video_decoder);
  if (SUCCEEDED(hr))
  {
    for (i = 0; i < VC_VIDEODECODER_MAX; i++)
      attrib_list[i+*num_attrib] = all_attrib_list[i+VC_VIDEOPROC_MAX+VC_CAMERACONTROL_MAX+VC_VIDEOCONTROL_MAX];
    *num_attrib += VC_VIDEODECODER_MAX;
    video_decoder->Release();
  }

  return (const char**)attrib_list;
}

//VIDEOINFOHEADER
// AvgTimePerFrame
