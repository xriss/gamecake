/** \file
 * \brief libTIFF I/O and error handlers.
 * I/O uses imBinFile instead of libTIFF original handlers.
 *
 * See Copyright Notice in im_lib.h
 * $Id: tiff_binfile.c,v 1.1 2008/10/17 06:10:16 scuri Exp $
 */

#include "tiffiop.h"

#include "im_binfile.h"

#include <stdlib.h>
#include <memory.h>

static tsize_t iTIFFReadProc(thandle_t fd, tdata_t buf, tsize_t size)
{
  imBinFile* file_bin = (imBinFile*)fd;
  return imBinFileRead(file_bin, buf, size, 1);
}

static tsize_t iTIFFWriteProc(thandle_t fd, tdata_t buf, tsize_t size)
{
  imBinFile* file_bin = (imBinFile*)fd;
  return imBinFileWrite(file_bin, buf, size, 1);
}

static toff_t iTIFFSeekProc(thandle_t fd, toff_t off, int whence)
{
  imBinFile* file_bin = (imBinFile*)fd;
  switch (whence)
  {
  case SEEK_SET:
    imBinFileSeekTo(file_bin, off);
    break;
  case SEEK_CUR:
    imBinFileSeekOffset(file_bin, off);
    break;
  case SEEK_END: 
    imBinFileSeekFrom(file_bin, off);
    break;
  }

  return imBinFileTell(file_bin);
}

static int iTIFFCloseProc(thandle_t fd)
{
  imBinFile* file_bin = (imBinFile*)fd;
  imBinFileClose(file_bin);
  return 0;
}

static toff_t iTIFFSizeProc(thandle_t fd)
{
  imBinFile* file_bin = (imBinFile*)fd;
  return imBinFileSize(file_bin);
}

static int iTIFFMapProc(thandle_t fd, tdata_t* pbase, toff_t* psize)
{
  (void) fd; (void) pbase; (void) psize;
  return (0);
}

static void iTIFFUnmapProc(thandle_t fd, tdata_t base, toff_t size)
{
  (void) fd; (void) base; (void) size;
}

TIFF* TIFFFdOpen(int fd, const char* name, const char* mode)
{
  TIFF* tif;

  tif = TIFFClientOpen(name, mode, (thandle_t) fd,  iTIFFReadProc, iTIFFWriteProc,
                                                    iTIFFSeekProc, iTIFFCloseProc, 
                                                    iTIFFSizeProc, iTIFFMapProc, 
                                                    iTIFFUnmapProc);
  if (tif)
    tif->tif_fd = fd;

  return (tif);
}

TIFF* TIFFOpen(const char* name, const char* mode)
{
  imBinFile* bin_file;
  TIFF* tiff;

  if (mode[0] == 'r')
    bin_file = imBinFileOpen(name);
  else
    bin_file = imBinFileNew(name);

  if (!bin_file)
    return NULL;
  
  tiff = TIFFClientOpen(name, mode, (thandle_t)bin_file,  iTIFFReadProc, iTIFFWriteProc,
                                                          iTIFFSeekProc, iTIFFCloseProc, 
                                                          iTIFFSizeProc, iTIFFMapProc, 
                                                          iTIFFUnmapProc);
  if (!tiff)
    imBinFileClose(bin_file);

  return tiff;
}

void* _TIFFmalloc(tsize_t s)
{
  return (malloc((size_t) s));
}

void _TIFFfree(tdata_t p)
{
  free(p);
}

void* _TIFFrealloc(tdata_t p, tsize_t s)
{
  return (realloc(p, (size_t) s));
}

void _TIFFmemset(tdata_t p, int v, tsize_t c)
{
  memset(p, v, (size_t) c);
}

void _TIFFmemcpy(tdata_t d, const tdata_t s, tsize_t c)
{
  memcpy(d, s, (size_t) c);
}

int _TIFFmemcmp(const tdata_t p1, const tdata_t p2, tsize_t c)
{
  return (memcmp(p1, p2, (size_t) c));
}

TIFFErrorHandler _TIFFwarningHandler = NULL;
TIFFErrorHandler _TIFFerrorHandler = NULL;
