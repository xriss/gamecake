/** \file
 * \brief Image Conversion
 *
 * See Copyright Notice in im_lib.h
 */

#ifndef __IM_CONVERT_H
#define __IM_CONVERT_H

#include "im_image.h"

#if	defined(__cplusplus)
extern "C" {
#endif

                      
/** \defgroup convert Image Conversion
 * \par
 * Converts one type of image into another. Can convert between color modes
 * and between data types.
 * \par
 * See \ref im_convert.h
 * \ingroup imgclass */


/** Complex to real conversions
 * \ingroup convert */
enum imComplex2Real 
{
  IM_CPX_REAL, 
  IM_CPX_IMAG, 
  IM_CPX_MAG, 
  IM_CPX_PHASE
};

/** Predefined Gamma factors. Gamma can be any real number.
 * When gamma<0 use logarithmic, when gamma>0 use exponential.
 * gamma(x,g) = ((e^(g*x))-1)/(exp(g)-1)
 * gamma(x,g) = (log((g*x)+1))/(log(g+1))
 * \ingroup convert */
enum imGammaFactor
{
  IM_GAMMA_LINEAR   = 0,
  IM_GAMMA_LOGLITE  = -10,
  IM_GAMMA_LOGHEAVY = -1000,
  IM_GAMMA_EXPLITE  = 2,
  IM_GAMMA_EXPHEAVY = 7
};

/** Predefined Cast Modes
 * \ingroup convert */
enum imCastMode
{
  IM_CAST_MINMAX, /**< scan for min and max values */
  IM_CAST_FIXED,  /**< use predefied 0-max values, see \ref color Color Manipulation. */
  IM_CAST_DIRECT  /**< direct type cast the value. Only byte and ushort will be cropped. */
};

/** Changes the image data type, using a complex2real conversion, 
 * a gamma factor, and an abssolute mode (modulus). \n
 * When demoting the data type the function will scan for min/max values or use fixed values (cast_mode)
 * to scale the result according to the destiny range. \n
 * Except complex to real that will use only the complex2real conversion. \n
 * Images must be of the same size and color mode. \n
 * Returns IM_ERR_NONE, IM_ERR_DATA or IM_ERR_COUNTER, see also \ref imErrorCodes.
 * See also \ref imComplex2Real, \ref imGammaFactor and \ref imCastMode.
 *
 * \verbatim im.ConvertDataType(src_image: imImage, dst_image: imImage, cpx2real: number, gamma: number, abssolute: boolean, cast_mode: number) -> error: number [in Lua 5] \endverbatim
 * \verbatim im.ConvertDataTypeNew(image: imImage, data_type: number, cpx2real: number, gamma: number, abssolute: boolean, cast_mode: number) -> error: number, new_image: imImage  [in Lua 5] \endverbatim
 * \ingroup convert */
int imConvertDataType(const imImage* src_image, imImage* dst_image, int cpx2real, float gamma, int abssolute, int cast_mode);

/** Converts one color space to another. Images must be of the same size and data type. \n
 * CMYK can be converted to RGB only, and it is a very simple conversion. \n
 * All colors can be converted to Binary, the non zero gray values are converted to 1. \n
 * RGB to Map uses the median cut implementation from the free IJG JPEG software, copyright Thomas G. Lane. \n
 * Alpha channel is considered and Transparency* attributes are converted to alpha channel. \n
 * All other color space conversions assume sRGB and CIE definitions. \n
 * Returns IM_ERR_NONE, IM_ERR_DATA or IM_ERR_COUNTER, see also \ref imErrorCodes.
 *
 * \verbatim im.ConvertColorSpace(src_image: imImage, dst_image: imImage) -> error: number [in Lua 5] \endverbatim
 * \verbatim im.ConvertColorSpaceNew(image: imImage, color_space: number, has_alpha: boolean) -> error: number, new_image: imImage [in Lua 5] \endverbatim
 * \ingroup convert */
int imConvertColorSpace(const imImage* src_image, imImage* dst_image);

/** Converts the image to its bitmap equivalent, 
 * uses \ref imConvertColorSpace and \ref imConvertDataType. \n
 * Returns IM_ERR_NONE, IM_ERR_DATA or IM_ERR_COUNTER, see also \ref imErrorCodes.
 * See also \ref imComplex2Real, \ref imGammaFactor and \ref imCastMode. \n
 * The function im.ConvertToBitmapNew uses the default convertion result from \ref imColorModeToBitmap if color_space is nil.
 *
 * \verbatim im.ConvertToBitmap(src_image: imImage, dst_image: imImage, cpx2real: number, gamma: number, abssolute: boolean, cast_mode: number) -> error: number [in Lua 5] \endverbatim
 * \verbatim im.ConvertToBitmapNew(image: imImage, color_space: number, has_alpha: boolean, cpx2real: number, gamma: number, abssolute: boolean, cast_mode: number) -> error: number, new_image: imImage [in Lua 5] \endverbatim
 * \ingroup convert */
int imConvertToBitmap(const imImage* src_image, imImage* dst_image, int cpx2real, float gamma, int abssolute, int cast_mode);

/** Returns an OpenGL compatible data buffer. Also returns the correspondant pixel format. \n
 * The memory allocated is stored in the attribute "GLDATA" with BYTE type. And it will exists while the image exists. \n
 * It can be cleared by setting the attribute to NULL. \n
 * MAP images are converted to RGB, and BINARY images are converted to GRAY.
 * Alpha channel is considered and Transparency* attributes are converted to alpha channel.
 * So calculate depth from glformat, not from image depth.
 *
 * \verbatim image:GetOpenGLData() -> gldata: userdata, glformat: number [in Lua 5] \endverbatim
 * \ingroup convert */
void* imImageGetOpenGLData(const imImage* image, int *glformat);

/** Creates an image from an OpenGL data.
 *
 * \verbatim im.ImageCreateFromOpenGLData(width, height, glformat: number, gldata: userdata) -> image: imImage [in Lua 5] \endverbatim
 * \ingroup convert */
imImage* imImageCreateFromOpenGLData(int width, int height, int glformat, const void* gldata);


/** \defgroup cnvutil Raw Data Conversion Utilities
 * \par
 * Utilities for raw data buffers.
 * \par
 * See \ref im_convert.h
 * \ingroup imagerep */


/** Changes the packing of the data buffer. Both must have the same width, height and data_type. \n
 * It can be used to copy data even if depth=1.
 * \ingroup cnvutil */
void imConvertPacking(const void* src_data, void* dst_data, int width, int height, int src_depth, int dst_depth, int data_type, int src_is_packed);

/** Changes in-place a MAP data into a RGB data. The data must have room for the RGB image. \n
 * depth can be 3 or 4. count=width*height. \n
 * \ingroup cnvutil */
void imConvertMapToRGB(unsigned char* data, int count, int depth, int packed, long* palette, int palette_count);
                       


/* Converts a RGB bitmap into a map bitmap using the median cut algorithm.
 * Used only "im_convertcolor.cpp" implemented in "im_rgb2map.cpp". 
 * Internal function kept here because of the compatibility module. 
 * Will not be at the documentation. */
int imConvertRGB2Map(int width, int height, 
              unsigned char *red, unsigned char *green, unsigned char *blue, 
              unsigned char *map, long *palette, int *palette_count);


#if defined(__cplusplus)
}
#endif

#endif
