/** \file
 * \brief HSI Color Manipulation
 *
 * See Copyright Notice in im_lib.h
 * $Id: im_colorhsi.cpp,v 1.2 2008/11/18 13:15:46 scuri Exp $
 */


#include <math.h>

#include "im_colorhsi.h"
#include "im_color.h"

static const float rad60 =  1.0471975f;
static const float rad120 = 2.0943951f;
static const float rad180 = 3.1415926f;
static const float rad240 = 4.1887902f;
static const float rad300 = 5.2359877f;
static const float rad360 = 6.2831853f;
static const float sqrt3 = 1.7320508f;
static const float rad2deg = 57.2957795131f;


static double iColorNormHue(double H)
{
  while (H < 0.0) 
    H += rad360;

  if (H > rad360)  
    H = fmod(H, (double)rad360);

  return H;
}

/**********************************/               
/*         HSI MAX S              */
/**********************************/               
               
static void iColorSmax01(float h, float hr, float hb, float hg, float *h0, float *h1)
{
  if (h < rad60)
  {
    *h0 = hb;
    *h1 = hr;
  }
  else if (h < rad120)
  {
    *h0 = hb;
    *h1 = hg;
  }
  else if (h < rad180)
  {
    *h0 = hr;
    *h1 = hg;
  }
  else if (h < rad240)
  {
    *h0 = hr;
    *h1 = hb;
  }
  else if (h < rad300)
  {
    *h0 = hg;
    *h1 = hb;
  }
  else
  {
    *h0 = hg;
    *h1 = hr;
  }
}

static float iColorHSI_Smax(float h, double cosH, double sinH, float i)
{
  float hr, hb, hg, imax, h0, h1;

  /* i here is normalized between 0-1 */
  
  if (i == 0 || i == 1)
    return 0.0f;

  /* Making r=0, g=0, b=0, r=1, g=1 or b=1 in the parametric equations and 
     writting s in function of H and I. */

  hr = (float)(cosH / 1.5);
  hg = (float)((-cosH + sinH*sqrt3)/ 3.0);
  hb = (float)((-cosH - sinH*sqrt3)/ 3.0);

  /* at bottom */
  if (i <= 1.0f/3.0f)
  {
    /* face B=0 */
    if (h < rad120)
      return -i/hb;

    /* face R=0 */
    if (h < rad240)
      return -i/hr;

    /* face G=0 */
    return -i/hg;
  }

  /* at top */
  if (i >= 2.0f/3.0f)
  {
    /* face R=1 */
    if (h < rad60 || h > rad300)
      return (1-i)/hr;

    /* face G=1 */
    if (h < rad180)
      return (1-i)/hg;

    /* face B=1 */
    return (1-i)/hb;
  }

  /* in the middle */

  iColorSmax01(h, hr, hb, hg, &h0, &h1);

  if (h == 0 || h == rad120 || h == rad240)
    imax = 1.0f/3.0f;
  else if (h == rad60 || h == rad180 || h == rad300)
    imax = 2.0f/3.0f;
  else
    imax = h0 / (h0 - h1);

  if (i < imax) 
    return -i/h0;
  else
    return (1-i)/h1;
}

float imColorHSI_ImaxS(float h, double cosH, double sinH)
{
  float i, h0, h1;
  float hr, hb, hg;

  if (h == 0 || h == rad120 || h == rad240)
    return 1.0f/3.0f;

  if (h == rad60 || h == rad180 || h == rad300)
    return 2.0f/3.0f;

  hr = (float)(cosH / 1.5f);
  hg = (float)((-cosH + sinH*sqrt3)/ 3.0);
  hb = (float)((-cosH - sinH*sqrt3)/ 3.0);

  iColorSmax01(h, hr, hb, hg, &h0, &h1);

  i = h0 / (h0 - h1);

  return i;
}

/**********************************/               
/*         RGB 2 HSI              */
/**********************************/               

void imColorRGB2HSI(float r, float g, float b, float *h, float *s, float *i)
{            
  float v, u;
  double H;

  /* Parametric equations */
  v = r - (g + b)/2;
  u = (g - b) * (sqrt3/2);

  *i = (r + g + b)/3;   /* already normalized to 0-1 */
  *s = (float)sqrt(v*v + u*u);  /* s is between 0-1, it is linear in the cube and it is in u,v space. */
  
  if (*s == 0)
    *h = 360.0f;  /* by definition */
  else
  {
    H = atan2(u, v);
    H = iColorNormHue(H);
    *h = (float)(H * rad2deg);

    /* must scale S from 0-Smax to 0-1 */
    float Smax = iColorHSI_Smax((float)H, cos(H), sin(H), *i);
    if (Smax == 0.0f)
      *s = 0.0f;
    else
    {
      if (*s > Smax) /* because of round problems when calculating s and Smax */
        *s = Smax;
      *s /= Smax;
    }
  }
}

void imColorRGB2HSIbyte(unsigned char r, unsigned char g, unsigned char b, float *h, float *s, float *i)
{
  float fr = imColorReconstruct(r, (imbyte)255);
  float fg = imColorReconstruct(g, (imbyte)255);
  float fb = imColorReconstruct(b, (imbyte)255);
  
  imColorRGB2HSI(fr, fg, fb, h, s, i);
}

/**********************************/               
/*         HSI 2 RGB              */
/**********************************/               

void imColorHSI2RGB(float h, float s, float i, float *r, float *g, float *b)
{
  double cosH, sinH, H, v, u;

  if (i < 0) i = 0;
  else if (i > 1) i = 1;
  
  if (s < 0) s = 0;
  else if (s > 1) s = 1;

  if (s == 0.0f || i == 1.0f || i == 0.0f || (int)h == 360)
  {
    *r = i;
    *g = i;
    *b = i;
    return;
  }

  H = h/rad2deg;
  H = iColorNormHue(H);

  cosH = cos(H);
  sinH = sin(H);
    
  /* must scale S from 0-1 to 0-Smax */
  float Smax = iColorHSI_Smax((float)H, cosH, sinH, i);
  s *= Smax;
  if (s > 1.0f) /* because of round problems when calculating s and Smax */
    s = 1.0f;

  v = s * cosH;
  u = s * sinH;

  /* Inverse of the Parametric equations, using i normalized to 0-1 */
  *r = (float)(i + v/1.5);
  *g = (float)(i - (v - u*sqrt3)/3.0);
  *b = (float)(i - (v + u*sqrt3)/3.0);

  /* fix round errors */
  if (*r < 0) *r = 0;
  if (*g < 0) *g = 0;
  if (*b < 0) *b = 0;

  if (*r > 1) *r = 1.0f;
  if (*g > 1) *g = 1.0f;
  if (*b > 1) *b = 1.0f;
}

void imColorHSI2RGBbyte(float h, float s, float i, unsigned char *r, unsigned char *g, unsigned char *b)
{
  float fr, fg, fb;
  
  imColorHSI2RGB(h, s, i, &fr, &fg, &fb);
  
  *r = imColorQuantize(fr, (imbyte)255);
  *g = imColorQuantize(fg, (imbyte)255);
  *b = imColorQuantize(fb, (imbyte)255);
}
