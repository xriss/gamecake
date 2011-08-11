/** \file
 * \brief Color Mode Utilities
 *
 * See Copyright Notice in im_lib.h
 * $Id: im_colormode.cpp,v 1.1 2008/10/17 06:10:16 scuri Exp $
 */


#include <stdlib.h>
#include <memory.h>
#include <string.h>

#include "im.h"
#include "im_util.h"

const char* imColorModeSpaceName(int color_mode)
{
  int color_space = imColorModeSpace(color_mode);
  switch (color_space)
  {
  case IM_RGB:    return "RGB";
  case IM_MAP:    return "Map";
  case IM_GRAY:   return "Gray";
  case IM_BINARY: return "Binary";
  case IM_CMYK:   return "CMYK";
  case IM_YCBCR:  return "Y'CbCr";
  case IM_LAB:    return "CIE L*a*b*";
  case IM_LUV:    return "CIE L*u*v*";
  case IM_XYZ:    return "CIE XYZ";
  }

  return NULL;
}

int imColorModeDepth(int color_mode)
{
  int depth = 0;

  int color_space = imColorModeSpace(color_mode);
  switch (color_space)
  {
  case IM_GRAY:
  case IM_BINARY:
  case IM_MAP:   
    depth = 1; 
    break;
  case IM_CMYK:
    depth = 4; 
    break;
  default:
    depth = 3; 
    break;
  }

  if (imColorModeHasAlpha(color_mode))
    depth++;

  return depth;
}

int imColorModeToBitmap(int color_mode)
{
  int color_space = imColorModeSpace(color_mode);
  switch (color_space)
  {
  case IM_BINARY:
  case IM_GRAY:
  case IM_MAP:
    return color_space;
  default:
    return IM_RGB;
  }
}

int imColorModeIsBitmap(int color_mode, int data_type)
{
  if (imColorModeSpace(color_mode) == IM_BINARY || 
      imColorModeSpace(color_mode) == IM_MAP)
    return 1;

  if ((imColorModeSpace(color_mode) == IM_RGB || 
       imColorModeSpace(color_mode) == IM_GRAY) &&
      (data_type == IM_BYTE))
    return 1;

  return 0;
}
