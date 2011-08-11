/** \file
 * \brief Kernel Generators
 * Creates several known kernels
 *
 * See Copyright Notice in im_lib.h
 * $Id: im_kernel.cpp,v 1.1 2008/10/17 06:16:33 scuri Exp $
 */

#include "im.h"
#include "im_util.h"
#include "im_image.h"
#include "im_kernel.h"

#include <stdlib.h>
#include <memory.h>
#include <assert.h>
#include <math.h>


static imImage* iKernelCreate(int w, int h, int* data, char* desc)
{
  imImage* kernel = imImageCreate(w, h, IM_GRAY, IM_INT);
  int* kernel_data = (int*)kernel->data[0];
  memcpy(kernel_data, data, kernel->size);
  imImageSetAttribute(kernel, "Description", IM_BYTE, -1, (void*)desc);
  return kernel;
}

imImage* imKernelSobel(void)
{
  int kernel_data[3*3] = {
    -1, -2, -1,
     0,  0,  0,
     1,  2,  1
  };

  return iKernelCreate(3, 3, kernel_data, "Sobel");
}

imImage* imKernelPrewitt(void)
{
  int kernel_data[3*3] = {
    -1, -1, -1,
     0,  0,  0,
     1,  1,  1
  };

  return iKernelCreate(3, 3, kernel_data, "Prewitt");
}

imImage* imKernelKirsh(void)
{
  int kernel_data[3*3] = {
    -3, -3, -3,
    -3,  0, -3,
     5,  5,  5
  };

  return iKernelCreate(3, 3, kernel_data, "Kirsh");
}

imImage* imKernelLaplacian4(void)
{
  int kernel_data[3*3] = {
     0, -1, 0,
    -1,  4, -1,
     0, -1, 0
  };

  return iKernelCreate(3, 3, kernel_data, "Laplacian4");
}

imImage* imKernelLaplacian8(void)
{
  int kernel_data[3*3] = {
    -1, -1, -1,
    -1,  8, -1,
    -1, -1, -1
  };

  return iKernelCreate(3, 3, kernel_data, "Laplacian8");
}

imImage* imKernelLaplacian5x5(void)
{
  int kernel_data[5*5] = {
     0, -1, -1, -1,  0,
    -1,  0,  1,  0, -1,
    -1,  1,  8,  1, -1,
    -1,  0,  1,  0, -1,
     0, -1, -1, -1,  0
  };

  return iKernelCreate(5, 5, kernel_data, "Laplacian5x5");
}

imImage* imKernelLaplacian7x7(void)
{
  int kernel_data[7*7] = {
    -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, 48, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1
  };

  return iKernelCreate(7, 7, kernel_data, "Laplacian7x7");
}

imImage* imKernelGradian3x3(void)
{
  int kernel_data[3*3] = {
     0, -1, 0,
     0,  1, 0,
     0,  0, 0
  };

  return iKernelCreate(3, 3, kernel_data, "Gradian3x3");
}

imImage* imKernelGradian7x7(void)
{
  int kernel_data[7*7] = {
     0, -1, -1,  0,  1,  1,  0,
    -1, -2, -2,  0,  2,  2,  1,
    -1, -2, -3,  0,  3,  2,  1,
    -1, -2, -3,  0,  3,  2,  1,
    -1, -2, -3,  0,  3,  2,  1,
    -1, -2, -2,  0,  2,  2,  1,
     0, -1, -1,  0,  1,  1,  0
  };

  return iKernelCreate(7, 7, kernel_data, "Gradian7x7");
}

imImage* imKernelSculpt(void)
{
  int kernel_data[3*3] = {
     0, 0, 1,
     0, 0, 0, 
    -1, 0, 0 
  };

  return iKernelCreate(3, 3, kernel_data, "Sculpt");
}

imImage* imKernelMean3x3(void)
{
  int kernel_data[3*3] = {
    1, 1, 1, 
    1, 1, 1, 
    1, 1, 1 
  };

  return iKernelCreate(3, 3, kernel_data, "Mean3x3");
}

imImage* imKernelMean5x5(void)
{
  int kernel_data[5*5] = {
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1
  };

  return iKernelCreate(5, 5, kernel_data, "Mean5x5");
}

imImage* imKernelCircularMean5x5(void)
{
  int kernel_data[5*5] = {
    0, 1, 1, 1, 0,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
    1, 1, 1, 1, 1,
    0, 1, 1, 1, 0
  };

  return iKernelCreate(5, 5, kernel_data, "CircularMean5x5");
}

imImage* imKernelMean7x7(void)
{
  int kernel_data[7*7] = {
    1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1
  };

  return iKernelCreate(7, 7, kernel_data, "Mean7x7");
}

imImage* imKernelCircularMean7x7(void)
{
  int kernel_data[7*7] = {
    0, 0, 1, 1, 1, 0, 0,
    0, 1, 1, 1, 1, 1, 0,
    1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1,
    0, 1, 1, 1, 1, 1, 0,
    0, 0, 1, 1, 1, 0, 0
  };

  return iKernelCreate(7, 7, kernel_data, "CircularMean7x7");
}

imImage* imKernelGaussian3x3(void)
{
  int kernel_data[3*3] = {
    1, 2, 1, 
    2, 4, 2, 
    1, 2, 1 
  };

  return iKernelCreate(3, 3, kernel_data, "Gaussian3x3");
}

imImage* imKernelGaussian5x5(void)
{
  int kernel_data[5*5] = {
    1,  4,  6,  4, 1, 
    4, 16, 24, 16, 4, 
    6, 24, 36, 24, 6, 
    4, 16, 24, 16, 4, 
    1,  4,  6,  4, 1 
  };

  return iKernelCreate(5, 5, kernel_data, "Gaussian5x5");
}

imImage* imKernelBarlett5x5(void)
{
  int kernel_data[5*5] = {
    1, 2, 3, 2, 1, 
    2, 4, 6, 4, 2, 
    3, 6, 9, 6, 3, 
    2, 4, 6, 4, 2, 
    1, 2, 3, 2, 1
  };

  return iKernelCreate(5, 5, kernel_data, "Barlett5x5");
}

imImage* imKernelTopHat5x5(void)
{
  int kernel_data[5*5] = {
     0, -1, -1, -1,  0, 
    -1, -1,  3, -1, -1, 
    -1,  3,  4,  3, -1, 
    -1, -1,  3, -1, -1, 
     0, -1, -1, -1,  0 
  };

  return iKernelCreate(5, 5, kernel_data, "TopHat5x5");
}

imImage* imKernelTopHat7x7(void)
{
  int kernel_data[7*7] = {
     0,  0, -1, -1, -1,  0,  0,
     0, -1, -1, -1, -1, -1,  0, 
    -1, -1,  3,  3,  3, -1, -1, 
    -1, -1,  3,  4,  3, -1, -1, 
    -1, -1,  3,  3,  3, -1, -1, 
     0, -1, -1, -1, -1, -1,  0,
     0,  0, -1, -1, -1,  0,  0 
  };

  return iKernelCreate(7, 7, kernel_data, "TopHat7x7");
}

imImage* imKernelEnhance(void)
{
  int kernel_data[5*5] = {
     0, -1, -2, -1,  0, 
    -1, -4,  0, -4, -1, 
    -2,  0, 40,  0, -2, 
    -1, -4,  0, -4, -1, 
     0, -1, -2, -1,  0 
  };

  return iKernelCreate(5, 5, kernel_data, "Enhance");
}

