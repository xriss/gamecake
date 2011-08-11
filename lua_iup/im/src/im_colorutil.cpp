/** \file
 * \brief Color Utilities
 *
 * See Copyright Notice in im_lib.h
 * $Id: im_colorutil.cpp,v 1.1 2008/10/17 06:10:16 scuri Exp $
 */


#include <stdlib.h>
#include <memory.h>
#include <string.h>

#include "im.h"
#include "im_util.h"

long imColorEncode(unsigned char Red, unsigned char Green, unsigned char Blue)
{
	return (((long)Red) << 16) | (((long)Green) << 8) | ((long)Blue);
}

void imColorDecode(unsigned char* Red, unsigned char* Green, unsigned char* Blue, long Color)
{
	if (Red) *Red = (imbyte)(Color >> 16);
	if (Green) *Green = (imbyte)(Color >> 8);
	if (Blue) *Blue = (imbyte)Color;
}

