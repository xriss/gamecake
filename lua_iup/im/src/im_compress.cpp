/** \file
 * \brief Data Compression Utilities
 *
 * See Copyright Notice in im_lib.h
 * $Id: im_compress.cpp,v 1.1 2008/10/17 06:10:16 scuri Exp $
 */


#include <math.h>

#include "im_util.h"
#include "zlib.h"

extern "C" {
#include "lzf.h"
}

int imCompressDataZ(const void* src_data, int src_size, void* dst_data, int dst_size, int zip_quality)
{
  uLongf ret_size = (uLongf)dst_size;
	if (compress2((Bytef*)dst_data, &ret_size, (const Bytef*)src_data, src_size, zip_quality) != Z_OK)
    return 0;

  return (int)ret_size;
}

int imCompressDataUnZ(const void* src_data, int src_size, void* dst_data, int dst_size)
{
  uLongf ret_size = (uLongf)dst_size;
	if (uncompress((Bytef*)dst_data, &ret_size, (const Bytef*)src_data, src_size) != Z_OK)
    return 0;

  return (int)ret_size;
}

int imCompressDataLZF(const void* src_data, int src_size, void* dst_data, int dst_size)
{
  return lzf_compress(src_data, src_size, dst_data, dst_size);
}

int imCompressDataUnLZF(const void* src_data, int src_size, void* dst_data, int dst_size)
{
  return lzf_decompress(src_data, src_size, dst_data, dst_size);
}
