/** \file
 * \brief libJasper I/O
 * I/O uses imBinFile instead of libJasper original handlers.
 *
 * See Copyright Notice in im_lib.h
 * $Id: jas_binfile.c,v 1.1 2008/10/17 06:10:16 scuri Exp $
 */

#include <stdlib.h>

#include "jasper/jas_types.h"
#include "jasper/jas_stream.h"
#include "jasper/jas_malloc.h"
#include "jasper/jas_math.h"

#include "im_binfile.h"

/* These were static in jas_stream.c */
jas_stream_t *jas_stream_create(void);
void jas_stream_initbuf(jas_stream_t *stream, int bufmode, char *buf, int bufsize);

static int file_read(jas_stream_obj_t *obj, char *buf, int cnt)
{
  imBinFile* file_bin = (imBinFile*)obj;
  return imBinFileRead(file_bin, buf, cnt, 1);
}

static int file_write(jas_stream_obj_t *obj, char *buf, int cnt)
{
  imBinFile* file_bin = (imBinFile*)obj;
  return imBinFileWrite(file_bin, buf, cnt, 1);
}

static long file_seek(jas_stream_obj_t *obj, long offset, int origin)
{
  imBinFile* file_bin = (imBinFile*)obj;
  switch (origin)
  {
  case SEEK_SET:
    imBinFileSeekTo(file_bin, offset);
    break;
  case SEEK_CUR:
    imBinFileSeekOffset(file_bin, offset);
    break;
  case SEEK_END: 
    imBinFileSeekFrom(file_bin, offset);
    break;
  }

  return imBinFileError(file_bin);
}

static int file_close(jas_stream_obj_t *obj)
{
  imBinFile* file_bin = (imBinFile*)obj;
  imBinFileClose(file_bin);
  return 0;
}

static jas_stream_ops_t jas_stream_fileops = {
  file_read,
  file_write,
  file_seek,
  file_close
};

jas_stream_t *jas_binfile_open(const char *file_name, int is_new)
{
  void* handle;
  jas_stream_t *stream;

  if (is_new)
    handle = (void*)imBinFileNew(file_name);
  else
    handle = (void*)imBinFileOpen(file_name);

  if (!handle)
    return 0;

  /* Allocate a stream object. */
  stream = jas_stream_create();

  if (is_new)
    stream->openmode_ = JAS_STREAM_WRITE | JAS_STREAM_CREATE | JAS_STREAM_BINARY;
  else
    stream->openmode_ = JAS_STREAM_READ | JAS_STREAM_BINARY;

  /* Select the operations for a file stream object. */
  stream->ops_ = &jas_stream_fileops;

  stream->obj_ = handle;

  /* By default, use full buffering for this type of stream. */
  jas_stream_initbuf(stream, JAS_STREAM_FULLBUF, 0, 0);

  return stream;
}
