/** \file
 * \brief Simulation Base Driver
 *
 * See Copyright Notice in cd.h
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <memory.h>

#include "cd.h"
#include "cd_private.h"
#include "cd_truetype.h"
#include "sim.h"


static unsigned char SimHatchBits[6][8] = {            /* [style][y] (8x8) */
     {0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00}, /* CD_HORIZONTAL */
     {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10}, /* CD_VERTICAL   */
     {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80}, /* CD_BDIAGONAL  */
     {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01}, /* CD_FDIAGONAL  */
     {0x10, 0x10, 0x10, 0xFF, 0x10, 0x10, 0x10, 0x10}, /* CD_CROSS      */
     {0x81, 0x42, 0x24, 0x18, 0x18, 0x24, 0x42, 0x81}};/* CD_DIAGCROSS  */

#define CalcYPat(y, h) (canvas->invert_yaxis? h-1-(y%h): y%h)
#define CalcYHatch(y, h) (canvas->invert_yaxis? h-(y&h): y&h)

void simFillDrawAAPixel(cdCanvas *canvas, int x, int y, unsigned short alpha_weigth)
{
  unsigned char aa_alpha;
  long color, aa_color;

  switch(canvas->interior_style)                                             
  {                                                                                    
  default: /* CD_SOLID */
    {
      color = canvas->foreground;
      break;
    }
  case CD_PATTERN:                                      
    {
      long *pattern = canvas->pattern;
      int yp = CalcYPat(y, canvas->pattern_h);
      int xp = x % canvas->pattern_w;
      color = pattern[canvas->pattern_w*yp + xp];
      break;                                                                           
    }
  case CD_HATCH: 
    {
      unsigned char hatch = SimHatchBits[canvas->hatch_style][CalcYHatch(y, 7)];
      unsigned char n = (unsigned char)(x&7);
      simRotateHatchN(hatch, n);
      if (hatch & 0x80)
        color = canvas->foreground;
      else if (canvas->back_opacity == CD_OPAQUE)
        color = canvas->background;
      else
        return;
      break;                                                                           
    }
  case CD_STIPPLE:                                                                   
    {
      unsigned char *stipple = canvas->stipple;
      int yp = CalcYPat(y, canvas->stipple_h);
      int xp = x % canvas->stipple_w;
      if(stipple[canvas->stipple_w*yp + xp])
        color = canvas->foreground;
      else if (canvas->back_opacity == CD_OPAQUE)
        color = canvas->background;
      else
        return;
      break;                                                                           
    }
  }

  aa_alpha = (unsigned char)((alpha_weigth * cdAlpha(color)) / 255);
  aa_color = cdEncodeAlpha(color, aa_alpha);
  canvas->cxPixel(canvas->ctxcanvas, x, y, aa_color);
}

void simFillHorizLine(cdSimulation* simulation, int xmin, int y, int xmax)
{
  cdCanvas* canvas = simulation->canvas;

  if(xmin > xmax)
    _cdSwapInt(xmin, xmax);

  switch(canvas->interior_style)                                             
  {                                                                                    
  case CD_SOLID:                                                                     
    simulation->SolidLine(canvas, xmin,y,xmax, canvas->foreground);  
    break;
  case CD_PATTERN:                                      
    simulation->PatternLine(canvas, xmin,xmax,y,canvas->pattern_w,
                            canvas->pattern +                                              
                            canvas->pattern_w*CalcYPat(y, canvas->pattern_h));
    break;                                                                           
  case CD_HATCH:                                                                     
    simulation->HatchLine(canvas, xmin,xmax,y,SimHatchBits[canvas->hatch_style][CalcYHatch(y, 7)]);
    break;                                                                           
  case CD_STIPPLE:                                                                   
    simulation->StippleLine(canvas, xmin, xmax, y,                                                               
                            canvas->stipple_w,                                        
                            canvas->stipple +                                    
                            canvas->stipple_w*CalcYPat(y, canvas->stipple_h));
  }
}

void simFillHorizBox(cdSimulation* simulation, int xmin, int xmax, int ymin, int ymax)
{
  int y;
  for(y=ymin;y<=ymax;y++)
    simFillHorizLine(simulation, xmin, y, xmax);
}

static void simSolidLine(cdCanvas* canvas, int xmin, int y, int xmax, long color)
{
  int x;
  for (x = xmin; x <= xmax; x++)
  {
    canvas->cxPixel(canvas->ctxcanvas, x,y,color);
  }
}

static void simPatternLine(cdCanvas* canvas, int xmin, int xmax, int y, int pw, const long *pattern)
{
  cdSimulation* simulation = canvas->simulation;
  int x,i;
  int xb;
  long curColor;
  
  i = xmin % pw;
 
  for (x = xmin; x <= xmax;)
  {
    if (i == pw) 
      i = 0;
    
    curColor=pattern[i];
    xb=x;
    
    while(pattern[i]==curColor && (x <= xmax))
    {
      i++;
      if (i == pw) 
        i = 0;
      x++;
    }
    
    if(xb==x-1)
      canvas->cxPixel(canvas->ctxcanvas, xb,y,curColor);
    else
      simulation->SolidLine(canvas, xb,y,x-1, curColor);  
  }
}

static void simStippleLine(cdCanvas* canvas, int xmin, int xmax, int y, int pw, const unsigned char *stipple)
{
  cdSimulation* simulation = canvas->simulation;
  int x,xb,i;
  long fgColor,bgColor;
  int opacity;
  int curCase;
  
  fgColor = canvas->foreground;
  opacity=canvas->back_opacity;
  
  if(opacity==CD_OPAQUE)
  {
    bgColor=canvas->background;

    for (x = xmin, i=xmin%pw ; x <= xmax;)
    {
      if(i==pw) 
        i=0;
      xb=x;
      curCase=stipple[i];
      while (stipple[i]==curCase && (x<=xmax))
      {
        x++;
        i++;
        if(i==pw) 
          i=0;
      }
      if (curCase) 
      {
        if(xb==x-1)
          canvas->cxPixel(canvas->ctxcanvas, xb,y,fgColor);
        else
          simulation->SolidLine(canvas, xb,y,x-1,fgColor);  
      }
    }

    for (x = xmin, i=xmin%pw ; x <= xmax;)
    {
      if(i==pw) 
        i=0;
      xb=x;
      curCase=stipple[i];
      while (stipple[i]==curCase && (x<=xmax))
      {
        x++;
        i++;
        if(i==pw) 
          i=0;
      }
      if (!curCase)
      {
        if(xb==x-1)
          canvas->cxPixel(canvas->ctxcanvas, xb,y,bgColor);
        else
          simulation->SolidLine(canvas, xb,y,x-1,bgColor);  
      }
    }
  } 
  else
  {
    for (x = xmin,i=xmin%pw; x <= xmax;)
    {        
      xb=x;
      curCase=stipple[i];
      while (stipple[i]==curCase && (x<=xmax))
      {
        i++;
        x++;
        if(i==pw) 
          i=0;
      }
      if(curCase)
      {
        if(xb==x-1)
          canvas->cxPixel(canvas->ctxcanvas, xb,y,fgColor);
        else
          simulation->SolidLine(canvas, xb,y,x-1,fgColor);  
      }
    }
  }
}

static void simHatchLine(cdCanvas* canvas, int xmin, int xmax, int y, unsigned char hatch)
{
  cdSimulation* simulation = canvas->simulation;
  int x,xb;
  int opacity, mask;
  unsigned char startp, n;
  long curColor, fgColor, bgColor;
  
  n = (unsigned char)(xmin&7);
  simRotateHatchN(hatch,n);
  fgColor=canvas->foreground;
  opacity=canvas->back_opacity;
  
  if(opacity==CD_OPAQUE)
  {
    bgColor=canvas->background;
    for (x = xmin; x <= xmax; x++)
    {
      curColor=(hatch&0x80)?fgColor:bgColor;
      
      xb=x;
      startp = hatch&0x80? 1: 0;
      _cdRotateHatch(hatch);
      while (startp == (hatch&0x80? 1: 0) && x <= xmax)
      {
        x++;
        _cdRotateHatch(hatch);
      }

      if(xb==x)
        canvas->cxPixel(canvas->ctxcanvas, xb,y,curColor);
      else
        simulation->SolidLine(canvas, xb,y,x, curColor);  
    }
  }
  else
  {
    for (x = xmin; x <= xmax; x++)
    {
      mask=(hatch&0x80)?1:0;
      
      xb=x;
      startp = hatch&0x80? 1: 0;
      _cdRotateHatch(hatch);
      while (startp == (hatch&0x80? 1: 0) && x <= xmax)
      {
        x++;
        _cdRotateHatch(hatch);
      }
      
      if(mask)
      {
        if(xb==x)
          canvas->cxPixel(canvas->ctxcanvas, xb,y,fgColor);
        else
          simulation->SolidLine(canvas, xb,y,x,fgColor);  
      }
    }
  }
}

cdSimulation* cdCreateSimulation(cdCanvas* canvas)
{
  cdSimulation* simulation = (cdSimulation*)malloc(sizeof(cdSimulation));
  memset(simulation, 0, sizeof(cdSimulation));

  simulation->canvas = canvas;

  simulation->SolidLine   = simSolidLine;             
  simulation->PatternLine = simPatternLine; 
  simulation->StippleLine = simStippleLine; 
  simulation->HatchLine   = simHatchLine;   

  return simulation;
}

void cdKillSimulation(cdSimulation* simulation)
{
  if (simulation->tt_text) cdTT_free(simulation->tt_text);

  memset(simulation, 0, sizeof(cdSimulation));
  free(simulation);
}
