/** \file
 * \brief JP2 File Format
 *
 * See Copyright Notice in im_lib.h
 * See libJaper Copyright Notice in jasper.h
 * $Id: im_format_jp2.cpp,v 1.3 2010/04/08 13:19:00 scuri Exp $
 */

#include "im_format.h"
#include "im_format_jp2.h"
#include "im_util.h"
#include "im_counter.h"

#include <stdlib.h>
#include <string.h>

#include "jasper/jasper.h"
#include "jpc/jpc_enc.h"
#include "jp2/jp2_cod.h"

extern "C" 
{
  /* implemented in jas_binfile.c */
  jas_stream_t *jas_binfile_open(const char *file_name, int is_new);
}

jas_seqent_t iJP2Bits2Int(jas_seqent_t v, int prec, int sgnd)
{
  v &= JAS_ONES(prec);
  return  (sgnd && (v & (1 << (prec - 1)))) ? (v - (1 << prec)) : v;
}

/* this is based on jas_image_readcmpt */
template <class T> 
int iJP2ReadLine(jas_image_t *image, int row, int cmpno, T *data)
{
  jas_image_cmpt_t *cmpt = image->cmpts_[cmpno];

  if (jas_stream_seek(cmpt->stream_, (cmpt->width_ * row) * cmpt->cps_, SEEK_SET) < 0) 
    return 0;

  // this offset will convert from signed to unsigned
  int offset = 0;
  if (cmpt->sgnd_ && cmpt->prec_ > 1)
    offset = 1 << (cmpt->prec_-1);

  for (int j = 0; j < cmpt->width_; j++) 
  {
    jas_seqent_t v = 0;

    for (int k = 0; k < cmpt->cps_; k++) 
    {
      int c = jas_stream_getc(cmpt->stream_);
      if (c == EOF) 
        return 0;

      v = (v << 8) | (c & 0xff);
    }

    v = iJP2Bits2Int(v, cmpt->prec_, cmpt->sgnd_);

    *data++ = (T)(v + offset);
  }

  return 1;
}

uint_fast32_t iJP2Int2Bits(jas_seqent_t v, int prec, int sgnd)
{
  uint_fast32_t ret;
  ret = ((sgnd && v < 0) ? ((1 << prec) + v) : v) & JAS_ONES(prec);
  return ret;
}

/* this is based on jas_image_writecmpt */
template <class T> 
int iJP2WriteLine(jas_image_t *image, int row, int cmpno, T *data)
{
  jas_image_cmpt_t *cmpt = image->cmpts_[cmpno];

  if (jas_stream_seek(cmpt->stream_, (cmpt->width_ * row) * cmpt->cps_, SEEK_SET) < 0) 
    return 0;

  for (int j = 0; j < cmpt->width_; j++) 
  {
    jas_seqent_t v = iJP2Int2Bits(*data++, cmpt->prec_, cmpt->sgnd_);

    for (int k = 0; k < cmpt->cps_; k++) 
    {
      int c = (v >> (8 * (cmpt->cps_ - 1))) & 0xff;
      if (jas_stream_putc(cmpt->stream_, (imbyte)c) == EOF) 
        return 0;

      v <<= 8;
    }
  }

  return 1;
}

static const char* iJP2CompTable[1] = 
{
  "JPEG-2000",
};

class imFileFormatJP2: public imFileFormatBase
{
  int fmtid;
  jas_stream_t *stream;
  jas_image_t *image;

public:
  imFileFormatJP2(const imFormat* _iformat): imFileFormatBase(_iformat), image(0) {}
  ~imFileFormatJP2() {}

  int Open(const char* file_name);
  int New(const char* file_name);
  void Close();
  void* Handle(int index);
  int ReadImageInfo(int index);
  int ReadImageData(void* data);
  int WriteImageInfo();
  int WriteImageData(void* data);
};

class imFormatJP2: public imFormat
{
  int fmtid;
  jas_stream_t *stream;
  jas_image_t *image;

public:
  imFormatJP2()
    :imFormat("JP2", 
              "JPEG-2000 JP2 File Format", 
              "*.jp2;*.jpc;*.j2c;*.j2k;", 
              iJP2CompTable, 
              1, 
              0)
    {
    }
  ~imFormatJP2() {}

  imFileFormatBase* Create(void) const { return new imFileFormatJP2(this); }
  int CanWrite(const char* compression, int color_mode, int data_type) const;
};

static char* ijp2_message = NULL;
static int ijp2_abort = 0;
static int ijp2_counter = -1;

static void iJP2ProgressProc(int done, int total, char *descr)
{
  (void)descr;
  if (done == 0)
  {
    imCounterTotal(ijp2_counter, total, ijp2_message);
    ijp2_message = NULL;
  }

  if (!imCounterIncTo(ijp2_counter, done))
    ijp2_abort = 1;
}

static int iJP2AbortProc(void)
{
  return ijp2_abort;
}

void imFormatRegisterJP2(void)
{
  // Jasper library initialization
  jas_init();

  jas_set_progress_proc((jas_progress_proc_t)iJP2ProgressProc);
  jas_set_test_abort_proc((jas_test_abort_proc_t)iJP2AbortProc);
  
  imFormatRegister(new imFormatJP2());
}

int imFileFormatJP2::Open(const char* file_name)
{
  this->stream = jas_binfile_open(file_name, 0);
  if (this->stream == NULL)
    return IM_ERR_OPEN;

  this->fmtid = jas_image_getfmt(this->stream);
  if (this->fmtid < 0)
  {
    jas_stream_close(this->stream);
    return IM_ERR_FORMAT;
  }

  strcpy(this->compression, "JPEG-2000");
  this->image_count = 1;

  return IM_ERR_NONE;
}

int imFileFormatJP2::New(const char* file_name)
{
  this->stream = jas_binfile_open(file_name, 1);
  if (this->stream == NULL)
    return IM_ERR_OPEN;

  strcpy(this->compression, "JPEG-2000");
  this->image_count = 1;

  return IM_ERR_NONE;
}

void imFileFormatJP2::Close()
{
  if (this->image)
    jas_image_destroy(this->image);

  jas_stream_close(this->stream);
}

void* imFileFormatJP2::Handle(int index)
{
  if (index == 0)
    return (void*)this->stream->obj_;
  else if (index == 1)
    return (void*)this->image;
  else if (index == 2)
    return (void*)this->stream;
  else
    return NULL;
}

int imFileFormatJP2::ReadImageInfo(int index)
{
  (void)index;

  // The counter is started because in Jasper all image reading is done here. BAD!
  ijp2_counter = this->counter;
  ijp2_abort = 0;
  ijp2_message = "Reading JP2...";
  this->image = jas_image_decode(this->stream, this->fmtid, 0);
  ijp2_counter = -1;
  if (!this->image)
    return IM_ERR_ACCESS;

  this->width = jas_image_width(this->image);
  this->height = jas_image_height(this->image);

  int clrspc_fam = jas_clrspc_fam(jas_image_clrspc(image));
  switch(clrspc_fam)
  {
  case JAS_CLRSPC_FAM_GRAY:
    this->file_color_mode = IM_GRAY;
    break;
  case JAS_CLRSPC_FAM_XYZ:
    this->file_color_mode = IM_XYZ;
    break;
  case JAS_CLRSPC_FAM_RGB:
    this->file_color_mode = IM_RGB;
    break;
  case JAS_CLRSPC_FAM_YCBCR:
    this->file_color_mode = IM_YCBCR;
    break;
  case JAS_CLRSPC_FAM_LAB:
    this->file_color_mode = IM_LAB;
    break;
  default: 
    return IM_ERR_DATA;
  }

  this->file_data_type = IM_BYTE;
  int prec = jas_image_cmptprec(image, 0);
  if (prec > 8)
    this->file_data_type = IM_USHORT;

  if (prec < 8)
    this->convert_bpp = -prec; // just expand to 0-255

  if (prec == 1 && this->file_color_mode == IM_GRAY)
    this->file_color_mode = IM_BINARY;

  int cmpno = jas_image_getcmptbytype(this->image, JAS_IMAGE_CT_OPACITY);
  if (cmpno != -1)
    this->file_color_mode |= IM_ALPHA;

  this->file_color_mode |= IM_TOPDOWN;

  if (image->metadata.count > 0) 
  {
    imAttribTable* attrib_table = AttribTable();
    
    // First write GeoTIFF data
    jas_metadata_box_t *metabox = &image->metadata.boxes[JAS_IMAGE_BOX_GEO]; 
    if (metabox->size>0 && metabox->buf) 
      attrib_table->Set("GeoTIFFBox", IM_BYTE, metabox->size, metabox->buf);

    // Check if XMP is there
    metabox = &image->metadata.boxes[JAS_IMAGE_BOX_XMP]; 
    if (metabox->size>0 && metabox->buf) 
      attrib_table->Set("XMLPacket", IM_BYTE, metabox->size, metabox->buf);
  }

  return IM_ERR_NONE;
}

int imFileFormatJP2::WriteImageInfo()
{
  this->file_data_type = this->user_data_type;
  this->file_color_mode = imColorModeSpace(this->user_color_mode);
  this->file_color_mode |= IM_TOPDOWN;

  int prec = 8;
  if (this->file_data_type == IM_USHORT)
    prec = 16;

  jas_clrspc_t clrspc;
  switch (imColorModeSpace(this->user_color_mode))
  {
  case IM_BINARY:
    prec = 1;    
  case IM_GRAY:
    clrspc = JAS_CLRSPC_SGRAY;
    break;
  case IM_RGB:   
    clrspc = JAS_CLRSPC_SRGB;
    break;
  case IM_XYZ:
    clrspc = JAS_CLRSPC_CIEXYZ;
    break;
  case IM_LAB:
    clrspc = JAS_CLRSPC_CIELAB;
    break;
  case IM_YCBCR:
    clrspc = JAS_CLRSPC_SYCBCR;
    break;
  default:
    return IM_ERR_DATA;
  }

  if (imColorModeHasAlpha(this->user_color_mode))
    this->file_color_mode |= IM_ALPHA;

  int numcmpts = imColorModeDepth(this->file_color_mode);
  
  jas_image_cmptparm_t cmptparms[4];
  for (int i = 0; i < numcmpts; i++) 
  {
    jas_image_cmptparm_t* cmptparm = &cmptparms[i];

    cmptparm->tlx = 0;
    cmptparm->tly = 0;
    cmptparm->hstep = 1;
    cmptparm->vstep = 1;
    cmptparm->width = this->width;
    cmptparm->height = this->height;
    cmptparm->prec = prec;
    cmptparm->sgnd = 0;
  }

  this->image = jas_image_create(numcmpts, cmptparms, clrspc);
  if (!this->image)
    return IM_ERR_DATA;

  if (this->image->metadata.count > 0) 
  {
    const void* data;
    int size;
    imAttribTable* attrib_table = AttribTable();

    // GeoTIFF first
    data = attrib_table->Get("GeoTIFFBox", NULL, &size);
    if (data)
    {
      jas_metadata_box_t *metabox = &image->metadata.boxes[JAS_IMAGE_BOX_GEO]; 
      jas_box_alloc(metabox, size);
      memcpy(metabox->buf, data, size);
      memcpy(metabox->id, msi_uuid, sizeof(msi_uuid));
    }
   
    // Adobe XMP
    data = attrib_table->Get("XMLPacket", NULL, &size);
    if (data)
    {
      jas_metadata_box_t *metabox = &image->metadata.boxes[JAS_IMAGE_BOX_XMP]; 
      jas_box_alloc(metabox, size);
      memcpy(metabox->buf, data, size);
      memcpy(metabox->id, xmp_uuid, sizeof(xmp_uuid));
    }
  }

  return IM_ERR_NONE;
}

int imFileFormatJP2::ReadImageData(void* data)
{
  int count = imFileLineBufferCount(this);

  imCounterTotal(this->counter, count, NULL);

  int alpha_plane = -1;
  if (imColorModeHasAlpha(this->user_color_mode) && imColorModeHasAlpha(this->file_color_mode))
    alpha_plane = imColorModeDepth(this->file_color_mode) - 1;

  int row = 0, plane = 0;
  for (int i = 0; i < count; i++)
  {
    int cmpno;
    if (plane == alpha_plane)
      cmpno = jas_image_getcmptbytype(image, JAS_IMAGE_CT_OPACITY);
    else
      cmpno = jas_image_getcmptbytype(image, JAS_IMAGE_CT_COLOR(plane));

    if (cmpno == -1)
      return IM_ERR_DATA;

    int ret = 1;
    if (this->file_data_type == IM_BYTE)
      ret = iJP2ReadLine(image, row, cmpno, (imbyte*)this->line_buffer);
    else
      ret = iJP2ReadLine(image, row, cmpno, (imushort*)this->line_buffer);

    if (!ret)
      return IM_ERR_ACCESS;

    imFileLineBufferRead(this, data, row, plane);

    if (!imCounterInc(this->counter))
      return IM_ERR_COUNTER;

    imFileLineBufferInc(this, &row, &plane);
  }

  return IM_ERR_NONE;
}

int imFileFormatJP2::WriteImageData(void* data)
{
  int count = imFileLineBufferCount(this);
  imCounterTotal(this->counter, count, "Writing JP2...");  /* first time count */

  int depth = imColorModeDepth(this->file_color_mode);
  if (imColorModeHasAlpha(this->user_color_mode) && imColorModeHasAlpha(this->file_color_mode))
  {
    jas_image_setcmpttype(image, depth-1, JAS_IMAGE_CT_OPACITY);
    depth--;
  }

  for (int d = 0; d < depth; d++)
    jas_image_setcmpttype(image, d, JAS_IMAGE_CT_COLOR(d));

  int row = 0, plane = 0;
  for (int i = 0; i < count; i++)
  {
    imFileLineBufferWrite(this, data, row, plane);

    int ret = 1;
    if (this->file_data_type == IM_BYTE)
      ret = iJP2WriteLine(image, row, plane, (imbyte*)this->line_buffer);
    else
      ret = iJP2WriteLine(image, row, plane, (imushort*)this->line_buffer);

    if (!imCounterInc(this->counter))
      return IM_ERR_COUNTER;

    imFileLineBufferInc(this, &row, &plane);
  }

  char outopts[512] = "";
  imAttribTable* attrib_table = AttribTable();

  float* ratio = (float*)attrib_table->Get("CompressionRatio");
  if (ratio)
    sprintf(outopts, "rate=%g", (double)(1.0 / *ratio));

  // The counter continuous because in Jasper all image writing is done here. BAD!
  ijp2_counter = this->counter;
  ijp2_abort = 0;
  ijp2_message = NULL;  /* other counts */
  int err = jas_image_encode(image, stream, 0 /*JP2 format always */, outopts);
  ijp2_counter = -1;
  if (err)
    return IM_ERR_ACCESS;

  jas_stream_flush(stream);

  return IM_ERR_NONE;
}

int imFormatJP2::CanWrite(const char* compression, int color_mode, int data_type) const
{
  int color_space = imColorModeSpace(color_mode);

  if (color_space == IM_MAP || color_space == IM_CMYK || 
      color_space == IM_LUV)
    return IM_ERR_DATA;                       
                                              
  if (data_type != IM_BYTE && data_type != IM_USHORT)
    return IM_ERR_DATA;

  if (!compression || compression[0] == 0)
    return IM_ERR_NONE;

  if (!imStrEqual(compression, "JPEG-2000"))
    return IM_ERR_COMPRESS;

  return IM_ERR_NONE;
}
