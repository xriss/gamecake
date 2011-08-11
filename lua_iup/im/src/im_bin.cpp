/** \file
 * \brief Binary Data Utilities
 *
 * See Copyright Notice in im_lib.h
 * $Id: im_bin.cpp,v 1.1 2008/10/17 06:10:16 scuri Exp $
 */

#include <assert.h>

#include "im_util.h"


int imBinCPUByteOrder(void)
{
  static int CPUByteOrder = -1;

  if (CPUByteOrder == -1)
  {
	  unsigned short w = 0x0001;
	  unsigned char* b = (unsigned char*)&w;
	  CPUByteOrder = (b[0] == 0x01)? IM_LITTLEENDIAN: IM_BIGENDIAN;
  }

  return CPUByteOrder;
}

void imBinSwapBytes(void *data, int count, int size)
{
  switch(size)
  {
  case 2:
    imBinSwapBytes2(data, count);
    break;
  case 4:
    imBinSwapBytes4(data, count);
    break;
  case 8:
    imBinSwapBytes8(data, count);
    break;
  }
}

void imBinSwapBytes2(void *data, int count)
{
	assert(data);

	unsigned char lTemp;
  unsigned char *values = (unsigned char *)data;

	while (count-- != 0)
	{
		lTemp = values[1];
		values[1] = values[0];
		values[0] = lTemp;

		values += 2;
	}
}

void imBinSwapBytes4(void *data, int count)
{
	assert(data);

	unsigned char lTemp;
  unsigned char *values = (unsigned char *)data;

	while (count-- != 0)
	{
		lTemp = values[3];
		values[3] = values[0];
		values[0] = lTemp;

		lTemp = values[2];
		values[2] = values[1];
		values[1] = lTemp;

		values += 4;
	}
}

void imBinSwapBytes8(void *data, int count)
{
  assert(data);

	unsigned char lTemp;
  unsigned char *values = (unsigned char *)data;
	
	assert(values);

	while (count-- != 0)
	{
		lTemp = values[7];
		values[7] = values[0];
		values[0] = lTemp;

		lTemp = values[6];
		values[6] = values[1];
		values[1] = lTemp;

		lTemp = values[5];
		values[5] = values[2];
		values[2] = lTemp;

		lTemp = values[4];
		values[4] = values[3];
		values[3] = lTemp;

		values += 8;
	}
}

