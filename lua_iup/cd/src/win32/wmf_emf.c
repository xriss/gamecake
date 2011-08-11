/** \file
 * \brief EMF and WMF Play
 *
 * See Copyright Notice in cd.h
 */

#include <stdlib.h>
#include <stdio.h>
#include <io.h>    
#include <fcntl.h>
#include <math.h>

#include "cdwin.h"
#include "cdwmf.h"
#include "cdemf.h"

/* placeable metafile data definitions */
#define ALDUSKEY 0x9AC6CDD7

#ifndef PS_JOIN_MASK
#define PS_JOIN_MASK        0x0000F000
#endif

/*
%F Definicao do header do APM. Ver comentario no final deste arquivo.
*/
typedef struct _APMFILEHEADER
{
  WORD  key1,
        key2,
        hmf,
        bleft, btop, bright, bbottom,
        inch,
        reserved1,
        reserved2,
        checksum;
} APMFILEHEADER;


/* coordinates convertion */

static double wmf_xfactor = 1;
static double wmf_yfactor = -1;   /* negative because top-down orientation */
static int wmf_xmin = 0;
static int wmf_ymin = 0;
static int wmf_left = 0;
static int wmf_bottom = 0;  /* bottom and right are not included */
static int wmf_top = 0;
static int wmf_right = 0;

static int sScaleX(int x)
{
  return cdRound((x - wmf_left) * wmf_xfactor + wmf_xmin);
}

static int sScaleY(int y)
{
  return cdRound((y - (wmf_bottom-1)) * wmf_yfactor + wmf_ymin);
}

static int sScaleW(int w)
{
  int s = (int)(w * fabs(wmf_xfactor) + 0.5);
  return s > 0? s: 1;
}

static int sScaleH(int h)
{
  int s = (int)(h * fabs(wmf_yfactor) + 0.5);
  return s > 0? s: 1;
}

static void sCalcSizeX(int x)
{
  if (x < wmf_left)
  {
    wmf_left = x;
    return;
  }

  if (x+1 > wmf_right)
  {
    wmf_right = x+1;
    return;
  }
}

static void sCalcSizeY(int y)
{
  if (y < wmf_top)
  {
    wmf_top = y;
    return;
  }

  if (y+1 > wmf_bottom)
  {
    wmf_bottom = y+1;
    return;
  }
}

static int CALLBACK CalcSizeEMFEnumProc(HDC hDC, HANDLETABLE *lpHTable,	const ENHMETARECORD *lpEMFR, int nObj, LPARAM  lpData)
{
  switch (lpEMFR->iType)
  {
  case EMR_POLYGON:
    {
      EMRPOLYGON* data = (EMRPOLYGON*)lpEMFR;
      int i;
      for (i = 0; i < (int)data->cptl; i++)
      {
        sCalcSizeX(data->aptl[i].x);
        sCalcSizeY(data->aptl[i].y);
      }

      break;
    }
  case EMR_POLYLINE:
    {
      EMRPOLYLINE* data = (EMRPOLYLINE*)lpEMFR;
      int i;
      for (i = 0; i < (int)data->cptl; i++)
      {
        sCalcSizeX(data->aptl[i].x);
        sCalcSizeY(data->aptl[i].y);
      }

      break;
    }
  case EMR_POLYLINETO:
    {
      EMRPOLYLINETO* data = (EMRPOLYLINETO*)lpEMFR;
      int i;
      for (i = 0; i < (int)data->cptl; i++)
      {
        sCalcSizeX(data->aptl[i].x);
        sCalcSizeY(data->aptl[i].y);
      }
      
      break;
    }
  case EMR_POLYPOLYLINE:
    {
      EMRPOLYPOLYLINE* data = (EMRPOLYPOLYLINE*)lpEMFR;
      POINTL* aptl = (POINTL*)(data->aPolyCounts + data->nPolys); /* skip the array of counts */
      int c, i;                                                   /* data->aptl can not be used if count greater than 1 */
      for (c = 0; c < (int)data->nPolys; c++)
      {
        for (i = 0; i < (int)data->aPolyCounts[c]; i++, aptl++)
        {
          sCalcSizeX(aptl->x);
          sCalcSizeY(aptl->y);
        }
      }
      break;
    }
  case EMR_POLYPOLYGON:
    {
      EMRPOLYPOLYGON* data = (EMRPOLYPOLYGON*)lpEMFR;
      POINTL* aptl = (POINTL*)(data->aPolyCounts + data->nPolys); /* skip the array of counts */
      int c, i;                                                   /* data->aptl can not be used if count greater than 1 */
      for (c = 0; c < (int)data->nPolys; c++)
      {
        for (i = 0; i < (int)data->aPolyCounts[c]; i++, aptl++)
        {
          sCalcSizeX(aptl->x);
          sCalcSizeY(aptl->y);
        }
      }
      break;
    }
  case EMR_SETPIXELV:
    {
      EMRSETPIXELV* data = (EMRSETPIXELV*)lpEMFR;
      sCalcSizeX(data->ptlPixel.x);
      sCalcSizeY(data->ptlPixel.y);
      break;
    }
  case EMR_MOVETOEX:
    {
      EMRMOVETOEX* data = (EMRMOVETOEX*)lpEMFR;
      sCalcSizeX(data->ptl.x);
      sCalcSizeY(data->ptl.y);
      break;
    }
  case EMR_ANGLEARC:
    {
      EMRANGLEARC* data = (EMRANGLEARC*)lpEMFR;
      int x, y;
      
      x = (int)(data->nRadius * cos(CD_DEG2RAD * data->eStartAngle + data->eSweepAngle));
      y = (int)(data->nRadius * sin(CD_DEG2RAD * data->eStartAngle + data->eSweepAngle));
      
      sCalcSizeX(data->ptlCenter.x);
      sCalcSizeY(data->ptlCenter.y);
      sCalcSizeX(x);
      sCalcSizeY(y);
      break;
    }
  case EMR_ELLIPSE:
    {
      EMRELLIPSE* data = (EMRELLIPSE*)lpEMFR;
      sCalcSizeX(data->rclBox.left);
      sCalcSizeX(data->rclBox.right);
      sCalcSizeY(data->rclBox.top);
      sCalcSizeY(data->rclBox.bottom);
      break;
    }
  case EMR_RECTANGLE:
    {
      EMRRECTANGLE* data = (EMRRECTANGLE*)lpEMFR;
      sCalcSizeX(data->rclBox.left);
      sCalcSizeX(data->rclBox.right);
      sCalcSizeY(data->rclBox.bottom);
      sCalcSizeY(data->rclBox.top);
      break;
    }
  case EMR_ROUNDRECT:
    {
      EMRROUNDRECT* data = (EMRROUNDRECT*)lpEMFR;
      sCalcSizeX(data->rclBox.left);
      sCalcSizeX(data->rclBox.right);
      sCalcSizeY(data->rclBox.bottom);
      sCalcSizeY(data->rclBox.top);
      break;
    }
  case EMR_ARC:
    {
      EMRARC* data = (EMRARC*)lpEMFR;
      sCalcSizeX(data->rclBox.left);
      sCalcSizeX(data->rclBox.right);
      sCalcSizeY(data->rclBox.top);
      sCalcSizeY(data->rclBox.bottom);
      break;
    }
  case EMR_CHORD:
    {
      EMRCHORD* data = (EMRCHORD*)lpEMFR;
      sCalcSizeX(data->rclBox.left);
      sCalcSizeX(data->rclBox.right);
      sCalcSizeY(data->rclBox.top);
      sCalcSizeY(data->rclBox.bottom);
      break;
    }
  case EMR_PIE:
    {
      EMRPIE* data = (EMRPIE*)lpEMFR;
      sCalcSizeX(data->rclBox.left);
      sCalcSizeX(data->rclBox.right);
      sCalcSizeY(data->rclBox.top);
      sCalcSizeY(data->rclBox.bottom);
      break;
    }
  case EMR_LINETO:
    {
      EMRLINETO* data = (EMRLINETO*)lpEMFR;
      sCalcSizeX(data->ptl.x);
      sCalcSizeY(data->ptl.y);
      break;
    }
  case EMR_ARCTO:
    {
      EMRARCTO* data = (EMRARCTO*)lpEMFR;
      sCalcSizeX(data->rclBox.left);
      sCalcSizeX(data->rclBox.right);
      sCalcSizeY(data->rclBox.top);
      sCalcSizeY(data->rclBox.bottom);
      break;
    }
  case EMR_POLYDRAW:
    {
      int p;
      EMRPOLYDRAW* data = (EMRPOLYDRAW*)lpEMFR;
      
      for(p = 0; p < (int)data->cptl; p++)
      {
        switch (data->abTypes[p])
        {
        case PT_MOVETO:
          {
            sCalcSizeX(data->aptl[p].x);
            sCalcSizeY(data->aptl[p].y);
            break;
          }
        case PT_LINETO:
          {
            sCalcSizeX(data->aptl[p].x);
            sCalcSizeY(data->aptl[p].y);
            break;
          }
        }
      }
      break;
    }
  case EMR_BITBLT:
    {
      EMRBITBLT* data = (EMRBITBLT*)lpEMFR;
      cdwDIB dib;
      
      cdwDIBReference(&dib, ((BYTE*)data) + data->offBmiSrc, ((BYTE*)data) + data->offBitsSrc);
      
      if (dib.type == -1)
        break;
 
      sCalcSizeX(data->xDest);
      sCalcSizeY(data->yDest);
      sCalcSizeX(data->xDest + dib.w);
      sCalcSizeY(data->yDest + abs(dib.h));
      break;
    }
  case EMR_STRETCHBLT:
    {
      EMRSTRETCHBLT* data = (EMRSTRETCHBLT*)lpEMFR;
      cdwDIB dib;
      
      cdwDIBReference(&dib, ((BYTE*)data) + data->offBmiSrc, ((BYTE*)data) + data->offBitsSrc);
      
      if (dib.type == -1)
        break;
      
      sCalcSizeX(data->xDest);
      sCalcSizeY(data->yDest);
      sCalcSizeX(data->xDest + data->cxDest);
      sCalcSizeY(data->yDest + data->cyDest);
      break;
    }
  case EMR_MASKBLT:
    {
      EMRMASKBLT* data = (EMRMASKBLT*)lpEMFR;
      cdwDIB dib;
      
      cdwDIBReference(&dib, ((BYTE*)data) + data->offBmiSrc, ((BYTE*)data) + data->offBitsSrc);
      
      if (dib.type == -1)
        break;
      
      sCalcSizeX(data->xDest);
      sCalcSizeY(data->yDest);
      sCalcSizeX(data->xDest + dib.w);
      sCalcSizeY(data->yDest + abs(dib.h));
      break;
    }
  case EMR_SETDIBITSTODEVICE:
    {
      EMRSETDIBITSTODEVICE* data = (EMRSETDIBITSTODEVICE*)lpEMFR;
      cdwDIB dib;
      
      cdwDIBReference(&dib, ((BYTE*)data) + data->offBmiSrc, ((BYTE*)data) + data->offBitsSrc);
      
      if (dib.type == -1)
        break;
      
      sCalcSizeX(data->xDest);
      sCalcSizeY(data->yDest);
      sCalcSizeX(data->xDest + dib.w);
      sCalcSizeY(data->yDest + abs(dib.h));
      break;
    }
  case EMR_STRETCHDIBITS:
    {
      EMRSTRETCHDIBITS* data = (EMRSTRETCHDIBITS*)lpEMFR;
      cdwDIB dib;
      
      cdwDIBReference(&dib, ((BYTE*)data) + data->offBmiSrc, ((BYTE*)data) + data->offBitsSrc);
      
      if (dib.type == -1)
        break;
      
      sCalcSizeX(data->xDest);
      sCalcSizeY(data->yDest);
      sCalcSizeX(data->xDest + data->cxDest);
      sCalcSizeY(data->yDest + data->cyDest);
      break;
    }
  case EMR_EXTTEXTOUTA:
    {
      EMREXTTEXTOUTA* data = (EMREXTTEXTOUTA*)lpEMFR;
      sCalcSizeX(data->emrtext.ptlReference.x);
      sCalcSizeY(data->emrtext.ptlReference.y);
      break;
    }
  case EMR_EXTTEXTOUTW:
    {
      EMREXTTEXTOUTW* data = (EMREXTTEXTOUTW*)lpEMFR;
      sCalcSizeX(data->emrtext.ptlReference.x);
      sCalcSizeY(data->emrtext.ptlReference.y);
      break;
    }
  case EMR_POLYGON16:
    {
      EMRPOLYGON16* data = (EMRPOLYGON16*)lpEMFR;
      int i;
      for (i = 0; i < (int)data->cpts; i++)
      {
        sCalcSizeX(data->apts[i].x);
        sCalcSizeY(data->apts[i].y);
      }
      break;
    }
  case EMR_POLYLINE16:
    {
      EMRPOLYLINE16* data = (EMRPOLYLINE16*)lpEMFR;
      int i;
      for (i = 0; i < (int)data->cpts; i++)
      {
        sCalcSizeX(data->apts[i].x);
        sCalcSizeY(data->apts[i].y);
      }
      break;
    }
  case EMR_POLYLINETO16:
    {
      EMRPOLYLINETO16* data = (EMRPOLYLINETO16*)lpEMFR;
      int i;
      for (i = 0; i < (int)data->cpts; i++)
      {
        sCalcSizeX(data->apts[i].x);
        sCalcSizeY(data->apts[i].y);
      }
      break;
    }
  case EMR_POLYPOLYLINE16:
    {
      EMRPOLYPOLYLINE16* data = (EMRPOLYPOLYLINE16*)lpEMFR;
      POINTS* apts = (POINTS*)(data->aPolyCounts + data->nPolys); /* skip the array of counts */
      int c, i;                                                   /* data->apts can not be used if count greater than 1 */
      for (c = 0; c < (int)data->nPolys; c++)
      {
        for (i = 0; i < (int)data->aPolyCounts[c]; i++, apts++)
        {
          sCalcSizeX(apts->x);
          sCalcSizeY(apts->y);
        }
      }
      break;
    }
  case EMR_POLYPOLYGON16:
    {
      EMRPOLYPOLYGON16* data = (EMRPOLYPOLYGON16*)lpEMFR;
      POINTS* apts = (POINTS*)(data->aPolyCounts + data->nPolys); /* skip the array of counts */
      int c, i;                                                   /* data->apts can not be used if count greater than 1 */
      for (c = 0; c < (int)data->nPolys; c++)
      {
        for (i = 0; i < (int)data->aPolyCounts[c]; i++, apts++)
        {
          sCalcSizeX(apts->x);
          sCalcSizeY(apts->y);
        }
      }
      break;
    }
  case EMR_POLYDRAW16:
    {
      EMRPOLYDRAW16* data = (EMRPOLYDRAW16*)lpEMFR;
      int p;
      for(p = 0; p < (int)data->cpts; p++)
      {
        switch (data->abTypes[p])
        {
        case PT_MOVETO:
          {
            sCalcSizeX(data->apts[p].x);
            sCalcSizeY(data->apts[p].y);
            break;
          }
        case PT_LINETO:
          {
            sCalcSizeX(data->apts[p].x);
            sCalcSizeY(data->apts[p].y);
            break;
          }
        }
      }
      break;
    }
  case EMR_POLYTEXTOUTA:
    {
      EMRPOLYTEXTOUTA* data = (EMRPOLYTEXTOUTA*)lpEMFR;
      int t;
      for (t = 0; t < data->cStrings; t++)
      {
        sCalcSizeX(data->aemrtext[t].ptlReference.x);
        sCalcSizeY(data->aemrtext[t].ptlReference.y);
      }
      break;
    }
  case EMR_POLYTEXTOUTW:
    {
      EMRPOLYTEXTOUTW* data = (EMRPOLYTEXTOUTW*)lpEMFR;
      int t;
      for (t = 0; t < data->cStrings; t++)
      {
        sCalcSizeX(data->aemrtext[t].ptlReference.x);
        sCalcSizeY(data->aemrtext[t].ptlReference.y);
      }
      break;
    }
  }

  return 1;         
}

static int CALLBACK EMFEnumProc(HDC hDC, HANDLETABLE *lpHTable,	const ENHMETARECORD *lpEMFR, int nObj, LPARAM  lpData)
{
  static int curx = 0, cury = 0;
  static int upd_xy = 0;
  cdCanvas* canvas = (cdCanvas*)lpData;
  
  switch (lpEMFR->iType)
  {
  case EMR_SETWORLDTRANSFORM:
    {
      double matrix[6];
      EMRSETWORLDTRANSFORM* data = (EMRSETWORLDTRANSFORM*)lpEMFR;
      matrix[0] = data->xform.eM11;
      matrix[1] = data->xform.eM12;
      matrix[2] = data->xform.eM21;
      matrix[3] = data->xform.eM22;
      matrix[4] = data->xform.eDx;
      matrix[5] = data->xform.eDy;
      cdCanvasTransform(canvas, matrix);
      break;
    }
  case EMR_MODIFYWORLDTRANSFORM:
    {
      EMRMODIFYWORLDTRANSFORM* data = (EMRMODIFYWORLDTRANSFORM*)lpEMFR;
      if (data->iMode == MWT_IDENTITY)
        cdCanvasTransform(canvas, NULL);
      else if (data->iMode == MWT_LEFTMULTIPLY)
      {
        double matrix[6];
        matrix[0] = data->xform.eM11;
        matrix[1] = data->xform.eM12;
        matrix[2] = data->xform.eM21;
        matrix[3] = data->xform.eM22;
        matrix[4] = data->xform.eDx;
        matrix[5] = data->xform.eDy;
        cdCanvasTransformMultiply(canvas, matrix);
      }
      break;
    }
  case EMR_POLYGON:
    {
      EMRPOLYGON* data = (EMRPOLYGON*)lpEMFR;
      int i;
      
      cdCanvasBegin(canvas, CD_FILL);
      
      for (i = 0; i < (int)data->cptl; i++)
        cdCanvasVertex(canvas, sScaleX(data->aptl[i].x), sScaleY(data->aptl[i].y));
      
      cdCanvasEnd(canvas);
      break;
    }
  case EMR_POLYLINE:
    {
      EMRPOLYLINE* data = (EMRPOLYLINE*)lpEMFR;
      int i;
      
      cdCanvasBegin(canvas, CD_OPEN_LINES);
      
      for (i = 0; i < (int)data->cptl; i++)
        cdCanvasVertex(canvas, sScaleX(data->aptl[i].x), sScaleY(data->aptl[i].y));
      
      cdCanvasEnd(canvas);
      break;
    }
  case EMR_POLYBEZIER:
    {
      EMRPOLYBEZIER* data = (EMRPOLYBEZIER*)lpEMFR;
      int i;
      
      cdCanvasBegin(canvas, CD_BEZIER);
      
      for (i = 0; i < (int)data->cptl; i++)
        cdCanvasVertex(canvas, sScaleX(data->aptl[i].x), sScaleY(data->aptl[i].y));
      
      cdCanvasEnd(canvas);
      break;
    }
  case EMR_POLYLINETO:
    {
      EMRPOLYLINETO* data = (EMRPOLYLINETO*)lpEMFR;
      int i;
      
      cdCanvasBegin(canvas, CD_OPEN_LINES);
      
      for (i = 0; i < (int)data->cptl; i++)
        cdCanvasVertex(canvas, sScaleX(data->aptl[i].x), sScaleY(data->aptl[i].y));
      
      curx = data->aptl[data->cptl - 1].x;
      cury = data->aptl[data->cptl - 1].y;
      
      cdCanvasEnd(canvas);
      break;
    }
  case EMR_POLYBEZIERTO:
    {
      EMRPOLYBEZIERTO* data = (EMRPOLYBEZIERTO*)lpEMFR;
      int i;
      
      cdCanvasBegin(canvas, CD_BEZIER);
      
      for (i = 0; i < (int)data->cptl; i++)
        cdCanvasVertex(canvas, sScaleX(data->aptl[i].x), sScaleY(data->aptl[i].y));
      
      curx = data->aptl[data->cptl - 1].x;
      cury = data->aptl[data->cptl - 1].y;
      
      cdCanvasEnd(canvas);
      break;
    }
  case EMR_POLYPOLYLINE:
    {
      EMRPOLYPOLYLINE* data = (EMRPOLYPOLYLINE*)lpEMFR;
      POINTL* aptl = (POINTL*)(data->aPolyCounts + data->nPolys); /* skip the array of counts */
      int c, i;                                                   /* data->aptl can not be used if count greater than 1 */
      
      for (c = 0; c < (int)data->nPolys; c++)
      {
        cdCanvasBegin(canvas, CD_OPEN_LINES);
        
        for (i = 0; i < (int)data->aPolyCounts[c]; i++, aptl++)
          cdCanvasVertex(canvas, sScaleX(aptl->x), sScaleY(aptl->y));
        
        cdCanvasEnd(canvas);
      }
      break;
    }
  case EMR_POLYPOLYGON:
    {
      EMRPOLYPOLYGON* data = (EMRPOLYPOLYGON*)lpEMFR;
      POINTL* aptl = (POINTL*)(data->aPolyCounts + data->nPolys); /* skip the array of counts */
      int c, i;                                                   /* data->aptl can not be used if count greater than 1 */
      
      for (c = 0; c < (int)data->nPolys; c++)
      {
        cdCanvasBegin(canvas, CD_FILL);
        
        for (i = 0; i < (int)data->aPolyCounts[c]; i++, aptl++)
          cdCanvasVertex(canvas, sScaleX(aptl->x), sScaleY(aptl->y));
        
        cdCanvasEnd(canvas);
      }
      break;
    }
  case EMR_SETPIXELV:
    {
      EMRSETPIXELV* data = (EMRSETPIXELV*)lpEMFR;
      cdCanvasPixel(canvas, sScaleX(data->ptlPixel.x), sScaleY(data->ptlPixel.y), cdEncodeColor(GetRValue(data->crColor),GetGValue(data->crColor),GetBValue(data->crColor)));
      break;
    }
  case EMR_SETBKMODE:
    {
      EMRSETBKMODE* data = (EMRSETBKMODE*)lpEMFR;
      cdCanvasBackOpacity(canvas, data->iMode == TRANSPARENT? CD_TRANSPARENT: CD_OPAQUE);
      break;
    }
  case EMR_SETROP2:
    {
      EMRSETROP2* data = (EMRSETROP2*)lpEMFR;
      cdCanvasWriteMode(canvas, data->iMode == R2_NOTXORPEN? CD_NOT_XOR: (data->iMode == R2_XORPEN? CD_XOR: CD_REPLACE));
      break;
    }
  case EMR_SETTEXTALIGN:
    {
      EMRSETTEXTALIGN* data = (EMRSETTEXTALIGN*)lpEMFR;
      upd_xy = 0;

      if (data->iMode & TA_UPDATECP)
      {
        upd_xy = 1;
        data->iMode &= ~TA_UPDATECP;
      }

      switch (data->iMode)
      {
      case 0: /* top-left */
        cdCanvasTextAlignment(canvas, CD_NORTH_WEST);
        break;
      case 2: /* top-right */
        cdCanvasTextAlignment(canvas, CD_NORTH_EAST);
        break;
      case 6: /* top-center */
        cdCanvasTextAlignment(canvas, CD_NORTH);
        break;
      case 8: /* bottom-left */
        cdCanvasTextAlignment(canvas, CD_SOUTH_WEST);
        break;
      case 10: /* bottom-right */
        cdCanvasTextAlignment(canvas, CD_SOUTH_EAST);
        break;
      case 14: /* bottom-center */
        cdCanvasTextAlignment(canvas, CD_SOUTH);
        break;
      case 24: /* baseline-left */
        cdCanvasTextAlignment(canvas, CD_BASE_LEFT);
        break;
      case 26: /* baseline-right */
        cdCanvasTextAlignment(canvas, CD_BASE_RIGHT);
        break;
      case 30: /* baseline-center */
        cdCanvasTextAlignment(canvas, CD_BASE_CENTER);
        break;
      }
     
      break;
    }
  case EMR_SETTEXTCOLOR:
    {
      EMRSETTEXTCOLOR* data = (EMRSETTEXTCOLOR*)lpEMFR;
      cdCanvasSetForeground(canvas, cdEncodeColor(GetRValue(data->crColor),GetGValue(data->crColor),GetBValue(data->crColor)));
      break;
    }
  case EMR_SETBKCOLOR:
    {
      EMRSETBKCOLOR* data = (EMRSETBKCOLOR*)lpEMFR;
      cdCanvasSetBackground(canvas, cdEncodeColor(GetRValue(data->crColor),GetGValue(data->crColor),GetBValue(data->crColor)));
      break;
    }
  case EMR_MOVETOEX:
    {
      EMRMOVETOEX* data = (EMRMOVETOEX*)lpEMFR;
      curx = data->ptl.x;
      cury = data->ptl.y;
      break;
    }
  case EMR_CREATEPEN:
    {
      EMRCREATEPEN* data = (EMRCREATEPEN*)lpEMFR;
      int style;
      
      switch (data->lopn.lopnStyle)
      {
      case PS_SOLID:
        style = CD_CONTINUOUS;
        break;
      case PS_DASH:                
        style = CD_DASHED;
        break;
      case PS_DOT:                  
        style = CD_DOTTED;
        break;
      case PS_DASHDOT:               
        style = CD_DASH_DOT;
        break;
      case PS_DASHDOTDOT:
        style = CD_DASH_DOT_DOT;
        break;
      case PS_NULL:
        style = -1;
        break;
      default:
        style = CD_CONTINUOUS;
        break;
      }
      
      if (style != -1)
      {
        cdCanvasLineStyle(canvas, style);
        cdCanvasLineWidth(canvas, sScaleW(data->lopn.lopnWidth.x == 0? 1: data->lopn.lopnWidth.x));
        cdCanvasSetForeground(canvas, cdEncodeColor(GetRValue(data->lopn.lopnColor),GetGValue(data->lopn.lopnColor),GetBValue(data->lopn.lopnColor)));
      }
      break;
    }
  case EMR_EXTCREATEPEN:
    {
      EMREXTCREATEPEN* data = (EMREXTCREATEPEN*)lpEMFR;
      int style;
      
      switch (data->elp.elpPenStyle & PS_STYLE_MASK)
      {
      case PS_SOLID:
        style = CD_CONTINUOUS;
        break;
      case PS_DASH:                
        style = CD_DASHED;
        break;
      case PS_DOT:                  
        style = CD_DOTTED;
        break;
      case PS_DASHDOT:               
        style = CD_DASH_DOT;
        break;
      case PS_DASHDOTDOT:
        style = CD_DASH_DOT_DOT;
        break;
      case PS_NULL:
        style = -1;
        break;
      case PS_USERSTYLE:
        style = CD_CUSTOM;
        cdCanvasLineStyleDashes(canvas, (int*)data->elp.elpStyleEntry, data->elp.elpNumEntries);
        break;
      default:
        style = CD_CONTINUOUS;
        break;
      }
      
      if (style != -1)
      {
        switch (data->elp.elpPenStyle & PS_ENDCAP_MASK)
        {
        case PS_ENDCAP_FLAT:
          cdCanvasLineCap(canvas, CD_CAPFLAT);
          break;
        case PS_ENDCAP_ROUND:
          cdCanvasLineCap(canvas, CD_CAPROUND);
          break;
        case PS_ENDCAP_SQUARE: 
          cdCanvasLineCap(canvas, CD_CAPSQUARE);
          break;
        }

        switch (data->elp.elpPenStyle & PS_JOIN_MASK)
        {
        case PS_JOIN_MITER:
          cdCanvasLineJoin(canvas, CD_MITER);
          break;
        case PS_JOIN_BEVEL:
          cdCanvasLineJoin(canvas, CD_BEVEL);
          break;
        case PS_JOIN_ROUND: 
          cdCanvasLineJoin(canvas, CD_ROUND);
          break;
        }

        cdCanvasLineStyle(canvas, style);
        cdCanvasLineWidth(canvas, sScaleW(data->elp.elpWidth == 0? 1: data->elp.elpWidth));
        cdCanvasSetForeground(canvas, cdEncodeColor(GetRValue(data->elp.elpColor),GetGValue(data->elp.elpColor),GetBValue(data->elp.elpColor)));
      }
      break;
    }
  case EMR_CREATEBRUSHINDIRECT:
    {
      EMRCREATEBRUSHINDIRECT* data = (EMRCREATEBRUSHINDIRECT*)lpEMFR;
      cdCanvasSetForeground(canvas, cdEncodeColor(GetRValue(data->lb.lbColor),GetGValue(data->lb.lbColor),GetBValue(data->lb.lbColor)));
      
      switch (data->lb.lbStyle)
      {
      case BS_HATCHED:                
        {
          int hatch = 0;
          
          switch (data->lb.lbHatch)
          {
          case HS_BDIAGONAL:
            hatch = CD_BDIAGONAL;
            break;
          case HS_CROSS:
            hatch = CD_CROSS;
            break;
          case HS_DIAGCROSS:
            hatch = CD_DIAGCROSS;
            break;
          case HS_FDIAGONAL:
            hatch = CD_FDIAGONAL;
            break;
          case HS_HORIZONTAL:
            hatch = CD_HORIZONTAL;
            break;
          case HS_VERTICAL:
            hatch = CD_VERTICAL;
            break;
          }
          
          cdCanvasHatch(canvas, hatch);
          cdCanvasInteriorStyle(canvas, CD_HATCH);
          break;
        }
      case BS_SOLID:
        cdCanvasInteriorStyle(canvas, CD_SOLID);
        break;
      case BS_NULL: 
        cdCanvasInteriorStyle(canvas, CD_HOLLOW);
        break;
      default:
        break;
      }
      break;
    }
  case EMR_ANGLEARC:
    {
      EMRANGLEARC* data = (EMRANGLEARC*)lpEMFR;
      int x, y;
      
      x = (int)(data->nRadius * cos(CD_DEG2RAD * data->eStartAngle + data->eSweepAngle));
      y = (int)(data->nRadius * sin(CD_DEG2RAD * data->eStartAngle + data->eSweepAngle));
      
      cdCanvasLine(canvas, sScaleX(data->ptlCenter.x), sScaleY(data->ptlCenter.y), sScaleX(x), sScaleY(y));
      cdCanvasArc(canvas, data->ptlCenter.x, data->ptlCenter.y, 2 * data->nRadius, 2 * data->nRadius, data->eStartAngle, data->eStartAngle + data->eSweepAngle);
      break;
    }
  case EMR_ELLIPSE:
    {
      EMRELLIPSE* data = (EMRELLIPSE*)lpEMFR;
      int xc, yc, w, h;
      
      xc = sScaleX((data->rclBox.left + data->rclBox.right - 1) / 2);
      w = sScaleW(data->rclBox.right - data->rclBox.left - 1);
      yc = sScaleY((data->rclBox.top + data->rclBox.bottom - 1) / 2);
      h = sScaleH(data->rclBox.bottom - data->rclBox.top - 1);
      
      cdCanvasSector(canvas, xc, yc, w, h, 0, 360);
      break;
    }
  case EMR_RECTANGLE:
    {
      EMRRECTANGLE* data = (EMRRECTANGLE*)lpEMFR;
      cdCanvasBox (canvas, sScaleX(data->rclBox.left), sScaleX(data->rclBox.right-2), sScaleY(data->rclBox.bottom-2), sScaleY(data->rclBox.top));
      break;
    }
  case EMR_ROUNDRECT:
    {
      EMRROUNDRECT* data = (EMRROUNDRECT*)lpEMFR;
      cdCanvasBox (canvas, sScaleX(data->rclBox.left), sScaleX(data->rclBox.right-2), sScaleY(data->rclBox.bottom-2), sScaleY(data->rclBox.top));
      break;
    }
  case EMR_ARC:
    {
      EMRARC* data = (EMRARC*)lpEMFR;
      int xc, yc, w, h;
      double angle1, angle2;
      
      xc = (data->rclBox.left + data->rclBox.right - 1) / 2;
      yc = (data->rclBox.top + data->rclBox.bottom - 1) / 2;
      
      w = sScaleW(data->rclBox.right - data->rclBox.left - 1);
      h = sScaleH(data->rclBox.bottom - data->rclBox.top - 1);
      
      angle1 = atan2((yc - data->ptlStart.y)*(data->rclBox.right - data->rclBox.left - 1), (data->ptlStart.x - xc)*(data->rclBox.bottom - data->rclBox.top - 1)) / CD_DEG2RAD;
      angle2 = atan2((yc - data->ptlEnd.y)*(data->rclBox.right - data->rclBox.left - 1), (data->ptlEnd.x - xc)*(data->rclBox.bottom - data->rclBox.top - 1)) / CD_DEG2RAD;

      if (angle1 == angle2)
        angle2+=360;
      
      xc = sScaleX(xc);
      yc = sScaleY(yc);
      
      cdCanvasArc(canvas, xc, yc, w, h, angle1, angle2);
      break;
    }
  case EMR_CHORD:
    {
      EMRCHORD* data = (EMRCHORD*)lpEMFR;
      int xc, yc, w, h;
      double angle1, angle2;
      
      xc = (data->rclBox.left + data->rclBox.right - 1) / 2;
      yc = (data->rclBox.top + data->rclBox.bottom - 1) / 2;
      
      w = sScaleW(data->rclBox.right - data->rclBox.left - 1);
      h = sScaleH(data->rclBox.bottom - data->rclBox.top - 1);
      
      angle1 = atan2((yc - data->ptlStart.y)*(data->rclBox.right - data->rclBox.left - 1), (data->ptlStart.x - xc)*(data->rclBox.bottom - data->rclBox.top - 1)) / CD_DEG2RAD;
      angle2 = atan2((yc - data->ptlEnd.y)*(data->rclBox.right - data->rclBox.left - 1), (data->ptlEnd.x - xc)*(data->rclBox.bottom - data->rclBox.top - 1)) / CD_DEG2RAD;

      if (angle1 == angle2)
        angle2+=360;
      
      xc = sScaleX(xc);
      yc = sScaleY(yc);
      
      cdCanvasChord(canvas, xc, yc, w, h, angle1, angle2);
      break;
    }
  case EMR_PIE:
    {
      EMRPIE* data = (EMRPIE*)lpEMFR;
      int xc, yc, w, h;
      double angle1, angle2;
      
      xc = (data->rclBox.left + data->rclBox.right - 1) / 2;
      yc = (data->rclBox.top + data->rclBox.bottom - 1) / 2;
      
      w = sScaleW(data->rclBox.right - data->rclBox.left - 1);
      h = sScaleH(data->rclBox.bottom - data->rclBox.top - 1);
      
      angle1 = atan2((yc - data->ptlStart.y)*(data->rclBox.right - data->rclBox.left - 1), (data->ptlStart.x - xc)*(data->rclBox.bottom - data->rclBox.top - 1)) / CD_DEG2RAD;
      angle2 = atan2((yc - data->ptlEnd.y)*(data->rclBox.right - data->rclBox.left - 1), (data->ptlEnd.x - xc)*(data->rclBox.bottom - data->rclBox.top - 1)) / CD_DEG2RAD;

      if (angle1 == angle2)
        angle2+=360;
      
      xc = sScaleX(xc);
      yc = sScaleY(yc);
      
      cdCanvasSector(canvas, xc, yc, w, h, angle1, angle2);
      break;
    }
  case EMR_CREATEPALETTE:
    {
      int k;
      EMRCREATEPALETTE* data = (EMRCREATEPALETTE*)lpEMFR;
      long palette[256];
      
      for (k=0; k < data->lgpl.palNumEntries; k++) 
        palette[k] = cdEncodeColor(data->lgpl.palPalEntry[k].peRed, data->lgpl.palPalEntry[k].peGreen, data->lgpl.palPalEntry[k].peBlue);
      
      cdCanvasPalette(canvas, data->lgpl.palNumEntries, palette, CD_POLITE);
      break;
    }
  case EMR_LINETO:
    {
      EMRLINETO* data = (EMRLINETO*)lpEMFR;
      cdCanvasLine(canvas, sScaleX(curx), sScaleY(cury), sScaleX(data->ptl.x), sScaleY(data->ptl.y));
      curx = data->ptl.x;
      cury = data->ptl.y;
      break;
    }
  case EMR_ARCTO:
    {
      EMRARCTO* data = (EMRARCTO*)lpEMFR;
      int xc, yc, w, h;
      double angle1, angle2;
      
      xc = (data->rclBox.left + data->rclBox.right - 1) / 2;
      yc = (data->rclBox.top + data->rclBox.bottom - 1) / 2;
      
      w = sScaleW(data->rclBox.right - data->rclBox.left - 1);
      h = sScaleH(data->rclBox.bottom - data->rclBox.top - 1);
      
      angle1 = atan2((yc - data->ptlStart.y)*(data->rclBox.right - data->rclBox.left - 1), (data->ptlStart.x - xc)*(data->rclBox.bottom - data->rclBox.top - 1)) / CD_DEG2RAD;
      angle2 = atan2((yc - data->ptlEnd.y)*(data->rclBox.right - data->rclBox.left - 1), (data->ptlEnd.x - xc)*(data->rclBox.bottom - data->rclBox.top - 1)) / CD_DEG2RAD;

      if (angle1 == angle2)
        angle2+=360;
      
      xc = sScaleX(xc);
      yc = sScaleY(yc);
      
      cdCanvasArc(canvas, xc, yc, w, h, angle1, angle2);
      
      curx = data->ptlEnd.x; /* isto nao esta' certo mas e' a minha melhor aproximacao */
      cury = data->ptlEnd.y;
      break;
    }
  case EMR_POLYDRAW:
    {
      int p;
      EMRPOLYDRAW* data = (EMRPOLYDRAW*)lpEMFR;
      
      for(p = 0; p < (int)data->cptl; p++)
      {
        switch (data->abTypes[p])
        {
        case PT_MOVETO:
          {
            curx = data->aptl[p].x;
            cury = data->aptl[p].y;
            break;
          }
        case PT_LINETO:
          {
            cdCanvasLine(canvas, sScaleX(curx), sScaleY(cury), sScaleX(data->aptl[p].x), sScaleY(data->aptl[p].y));
            curx = data->aptl[p].x;
            cury = data->aptl[p].y;
            break;
          }
        }
      }
      break;
    }
  case EMR_BITBLT:
    {
      EMRBITBLT* data = (EMRBITBLT*)lpEMFR;
      int size;
      cdwDIB dib;
      int old_write_mode;
      
      if (data->dwRop == SRCINVERT)
        old_write_mode = cdCanvasWriteMode(canvas, CD_XOR);
      else
        old_write_mode = cdCanvasWriteMode(canvas, CD_REPLACE);
      
      cdwDIBReference(&dib, ((BYTE*)data) + data->offBmiSrc, ((BYTE*)data) + data->offBitsSrc);
      
      if (dib.type == -1)
        break;
      
      size = dib.w*abs(dib.h);
      
      if (dib.type == 0)
      {
        unsigned char *r, *g, *b;
        
        r = (unsigned char*)malloc(size);
        g = (unsigned char*)malloc(size);
        b = (unsigned char*)malloc(size);
        
        cdwDIBDecodeRGB(&dib, r, g, b);
        
        cdCanvasPutImageRectRGB(canvas, dib.w, abs(dib.h), r, g, b, sScaleX(data->xDest), sScaleY(data->yDest + abs(dib.h)), dib.w, dib.h, 0, 0, 0, 0);
        
        free(r);
        free(g);
        free(b);
      }
      else
      {
        unsigned char *index;
        long *colors;
        
        index = (unsigned char*)malloc(size);
        colors = (long*)malloc(256*sizeof(long));
        
        cdwDIBDecodeMap(&dib, index, colors);
        
        cdCanvasPutImageRectMap(canvas, dib.w, abs(dib.h), index, colors, sScaleX(data->xDest), sScaleY(data->yDest + abs(dib.h)), dib.w, dib.h, 0, 0, 0, 0);
        
        free(index);
        free(colors);
      }
      
      cdCanvasWriteMode(canvas, old_write_mode);
      break;
    }
  case EMR_STRETCHBLT:
    {
      EMRSTRETCHBLT* data = (EMRSTRETCHBLT*)lpEMFR;
      int size;
      cdwDIB dib;
      int old_write_mode;
      
      if (data->dwRop == SRCINVERT)
        old_write_mode = cdCanvasWriteMode(canvas, CD_XOR);
      else
        old_write_mode = cdCanvasWriteMode(canvas, CD_REPLACE);
      
      cdwDIBReference(&dib, ((BYTE*)data) + data->offBmiSrc, ((BYTE*)data) + data->offBitsSrc);
      
      if (dib.type == -1)
        break;
      
      size = dib.w*abs(dib.h);
      
      if (dib.type == 0)
      {
        unsigned char *r, *g, *b;
        
        r = (unsigned char*)malloc(size);
        g = (unsigned char*)malloc(size);
        b = (unsigned char*)malloc(size);
        
        cdwDIBDecodeRGB(&dib, r, g, b);
        
        cdCanvasPutImageRectRGB(canvas, dib.w, abs(dib.h), r, g, b, sScaleX(data->xDest), sScaleY(data->yDest + abs(dib.h)), sScaleW(data->cxDest), sScaleH(data->cyDest), 0, 0, 0, 0);
        
        free(r);
        free(g);
        free(b);
      }
      else
      {
        unsigned char *index;
        long *colors;
        
        index = (unsigned char*)malloc(size);
        colors = (long*)malloc(256*sizeof(long));
        
        cdwDIBDecodeMap(&dib, index, colors);
        
        cdCanvasPutImageRectMap(canvas, dib.w, abs(dib.h), index, colors, sScaleX(data->xDest), sScaleY(data->yDest + abs(dib.h)), sScaleW(data->cxDest), sScaleH(data->cyDest), 0, 0, 0, 0);
        
        free(index);
        free(colors);
      }
      
      cdCanvasWriteMode(canvas, old_write_mode);
      break;
    }
  case EMR_MASKBLT:
    {
      EMRMASKBLT* data = (EMRMASKBLT*)lpEMFR;
      int size;
      cdwDIB dib;
      int old_write_mode;
      
      if (data->dwRop == SRCINVERT)
        old_write_mode = cdCanvasWriteMode(canvas, CD_XOR);
      else
        old_write_mode = cdCanvasWriteMode(canvas, CD_REPLACE);
      
      cdwDIBReference(&dib, ((BYTE*)data) + data->offBmiSrc, ((BYTE*)data) + data->offBitsSrc);
      
      if (dib.type == -1)
        break;
      
      size = dib.w*abs(dib.h);
      
      if (dib.type == 0)
      {
        unsigned char *r, *g, *b;
        
        r = (unsigned char*)malloc(size);
        g = (unsigned char*)malloc(size);
        b = (unsigned char*)malloc(size);
        
        cdwDIBDecodeRGB(&dib, r, g, b);
        
        cdCanvasPutImageRectRGB(canvas, dib.w, abs(dib.h), r, g, b, sScaleX(data->xDest), sScaleY(data->yDest + abs(dib.h)), dib.w, dib.h, 0, 0, 0, 0);
        
        free(r);
        free(g);
        free(b);
      }
      else
      {
        unsigned char *index;
        long *colors;
        
        index = (unsigned char*)malloc(size);
        colors = (long*)malloc(256*sizeof(long));
        
        cdwDIBDecodeMap(&dib, index, colors);
        
        cdCanvasPutImageRectMap(canvas, dib.w, abs(dib.h), index, colors, sScaleX(data->xDest), sScaleY(data->yDest + abs(dib.h)), dib.w, dib.h, 0, 0, 0, 0);
        
        free(index);
        free(colors);
      }
      
      cdCanvasWriteMode(canvas, old_write_mode);
      break;
    }
  case EMR_SETDIBITSTODEVICE:
    {
      EMRSETDIBITSTODEVICE* data = (EMRSETDIBITSTODEVICE*)lpEMFR;
      int size;
      cdwDIB dib;
      
      cdwDIBReference(&dib, ((BYTE*)data) + data->offBmiSrc, ((BYTE*)data) + data->offBitsSrc);
      
      if (dib.type == -1)
        break;
      
      size = dib.w*abs(dib.h);
      
      if (dib.type == 0)
      {
        unsigned char *r, *g, *b;
        
        r = (unsigned char*)malloc(size);
        g = (unsigned char*)malloc(size);
        b = (unsigned char*)malloc(size);
        
        cdwDIBDecodeRGB(&dib, r, g, b);
        
        cdCanvasPutImageRectRGB(canvas, dib.w, abs(dib.h), r, g, b, sScaleX(data->xDest), sScaleY(data->yDest + abs(dib.h)), dib.w, dib.h, 0, 0, 0, 0);
        
        free(r);
        free(g);
        free(b);
      }
      else
      {
        unsigned char *index;
        long *colors;
        
        index = (unsigned char*)malloc(size);
        colors = (long*)malloc(256*sizeof(long));
        
        cdwDIBDecodeMap(&dib, index, colors);
        
        cdCanvasPutImageRectMap(canvas, dib.w, abs(dib.h), index, colors, sScaleX(data->xDest), sScaleY(data->yDest + abs(dib.h)), dib.w, dib.h, 0, 0, 0, 0);
        
        free(index);
        free(colors);
      }
      break;
    }
  case EMR_STRETCHDIBITS:
    {
      EMRSTRETCHDIBITS* data = (EMRSTRETCHDIBITS*)lpEMFR;
      int size;
      cdwDIB dib;
      
      cdwDIBReference(&dib, ((BYTE*)data) + data->offBmiSrc, ((BYTE*)data) + data->offBitsSrc);
      
      if (dib.type == -1)
        break;
      
      size = dib.w*abs(dib.h);
      
      if (dib.type == 0)
      {
        unsigned char *r, *g, *b;
        
        r = (unsigned char*)malloc(size);
        g = (unsigned char*)malloc(size);
        b = (unsigned char*)malloc(size);
        
        cdwDIBDecodeRGB(&dib, r, g, b);
        
        cdCanvasPutImageRectRGB(canvas, dib.w, abs(dib.h), r, g, b, sScaleX(data->xDest), sScaleY(data->yDest + abs(dib.h)), sScaleW(data->cxDest), sScaleH(data->cyDest), 0, 0, 0, 0);
        
        free(r);
        free(g);
        free(b);
      }
      else
      {
        unsigned char *index;
        long *colors;
        
        index = (unsigned char*)malloc(size);
        colors = (long*)malloc(256*sizeof(long));
        
        cdwDIBDecodeMap(&dib, index, colors);
        
        cdCanvasPutImageRectMap(canvas, dib.w, abs(dib.h), index, colors, sScaleX(data->xDest), sScaleY(data->yDest + abs(dib.h)), sScaleW(data->cxDest), sScaleH(data->cyDest), 0, 0, 0, 0);
        
        free(index);
        free(colors);
      }
      break;
    }
  case EMR_EXTCREATEFONTINDIRECTW:
    {
      EMREXTCREATEFONTINDIRECTW* data = (EMREXTCREATEFONTINDIRECTW*)lpEMFR;
      int style, size;
      char type_face[256];
      
      style = CD_PLAIN;
      
      if (data->elfw.elfLogFont.lfWeight >= FW_BOLD)
        style = CD_BOLD;
      
      if (data->elfw.elfLogFont.lfItalic == 1)
        style = CD_ITALIC;
      
      if (data->elfw.elfLogFont.lfWeight >= FW_BOLD && data->elfw.elfLogFont.lfItalic == 1)
        style = CD_BOLD_ITALIC;

      if (data->elfw.elfLogFont.lfUnderline)
        style |= CD_UNDERLINE;
      
      if (data->elfw.elfLogFont.lfStrikeOut)
        style |= CD_STRIKEOUT;
      
      WideCharToMultiByte(CP_ACP, 0, data->elfw.elfLogFont.lfFaceName, LF_FACESIZE, type_face, 256, NULL, NULL);
      
      size = sScaleH(abs(data->elfw.elfLogFont.lfHeight));
      if (size < 5) size = 5;

      if (data->elfw.elfLogFont.lfOrientation)
        cdCanvasTextOrientation(canvas, data->elfw.elfLogFont.lfOrientation/10);
      
      cdCanvasFont(canvas, type_face, style, -size);
      break;
    }
  case EMR_EXTTEXTOUTA:
    {
      EMREXTTEXTOUTA* data = (EMREXTTEXTOUTA*)lpEMFR;
      
      char* str = malloc(data->emrtext.nChars + 1);
      memcpy(str, ((unsigned char*)data) + data->emrtext.offString, data->emrtext.nChars + 1);
      str[data->emrtext.nChars] = 0;
      
      cdCanvasText(canvas, sScaleX(data->emrtext.ptlReference.x), sScaleY(data->emrtext.ptlReference.y), str);
      
      if (upd_xy == 1)
      {
        curx = data->emrtext.ptlReference.x; 
        cury = data->emrtext.ptlReference.y;
      }
      
      free(str);
      break;
    }
  case EMR_EXTTEXTOUTW:
    {
      EMREXTTEXTOUTW* data = (EMREXTTEXTOUTW*)lpEMFR;
      char str[256];
      
      WideCharToMultiByte(CP_ACP, 0, (unsigned short*)(((unsigned char*)data) + data->emrtext.offString), data->emrtext.nChars, str, 256, NULL, NULL);
      
      str[data->emrtext.nChars] = 0;
      
      cdCanvasText(canvas, sScaleX(data->emrtext.ptlReference.x), sScaleY(data->emrtext.ptlReference.y), str);
      
      if (upd_xy == 1)
      {
        curx = data->emrtext.ptlReference.x; 
        cury = data->emrtext.ptlReference.y;
      }
      break;
    }
  case EMR_SETPOLYFILLMODE:
    {
      EMRSETPOLYFILLMODE* data = (EMRSETPOLYFILLMODE*)lpEMFR;
      if (data->iMode == ALTERNATE)
        cdCanvasFillMode(canvas, CD_EVENODD);
      else
        cdCanvasFillMode(canvas, CD_WINDING);
      break;
    }
  case EMR_POLYGON16:
    {
      EMRPOLYGON16* data = (EMRPOLYGON16*)lpEMFR;
      int i;
      
      cdCanvasBegin(canvas, CD_FILL);
      
      for (i = 0; i < (int)data->cpts; i++)
        cdCanvasVertex(canvas, sScaleX(data->apts[i].x), sScaleY(data->apts[i].y));
      
      cdCanvasEnd(canvas);
      break;
    }
  case EMR_POLYLINE16:
    {
      EMRPOLYLINE16* data = (EMRPOLYLINE16*)lpEMFR;
      int i;
      
      cdCanvasBegin(canvas, CD_OPEN_LINES);
      
      for (i = 0; i < (int)data->cpts; i++)
        cdCanvasVertex(canvas, sScaleX(data->apts[i].x), sScaleY(data->apts[i].y));
      
      cdCanvasEnd(canvas);
      break;
    }
  case EMR_POLYLINETO16:
    {
      EMRPOLYLINETO16* data = (EMRPOLYLINETO16*)lpEMFR;
      int i;
      
      cdCanvasBegin(canvas, CD_OPEN_LINES);
      
      for (i = 0; i < (int)data->cpts; i++)
        cdCanvasVertex(canvas, sScaleX(data->apts[i].x), sScaleY(data->apts[i].y));
      
      curx = data->apts[data->cpts - 1].x;
      cury = data->apts[data->cpts - 1].y;
      
      cdCanvasEnd(canvas);
      break;
    }
  case EMR_POLYBEZIER16:
    {
      EMRPOLYBEZIER16* data = (EMRPOLYBEZIER16*)lpEMFR;
      int i;
      
      cdCanvasBegin(canvas, CD_BEZIER);
      
      for (i = 0; i < (int)data->cpts; i++)
        cdCanvasVertex(canvas, sScaleX(data->apts[i].x), sScaleY(data->apts[i].y));
      
      cdCanvasEnd(canvas);
      break;
    }
  case EMR_POLYBEZIERTO16:
    {
      EMRPOLYBEZIERTO16* data = (EMRPOLYBEZIERTO16*)lpEMFR;
      int i;
      
      cdCanvasBegin(canvas, CD_BEZIER);
      
      for (i = 0; i < (int)data->cpts; i++)
        cdCanvasVertex(canvas, sScaleX(data->apts[i].x), sScaleY(data->apts[i].y));
      
      curx = data->apts[data->cpts - 1].x;
      cury = data->apts[data->cpts - 1].y;
      
      cdCanvasEnd(canvas);
      break;
    }
  case EMR_POLYPOLYLINE16:
    {
      EMRPOLYPOLYLINE16* data = (EMRPOLYPOLYLINE16*)lpEMFR;
      POINTS* apts = (POINTS*)(data->aPolyCounts + data->nPolys); /* skip the array of counts */
      int c, i;                                                   /* data->apts can not be used if count greater than 1 */
      
      for (c = 0; c < (int)data->nPolys; c++)
      {
        cdCanvasBegin(canvas, CD_OPEN_LINES);
        
        for (i = 0; i < (int)data->aPolyCounts[c]; i++, apts++)
          cdCanvasVertex(canvas, sScaleX(apts->x), sScaleY(apts->y));
        
        cdCanvasEnd(canvas);
      }
      break;
    }
  case EMR_POLYPOLYGON16:
    {
      EMRPOLYPOLYGON16* data = (EMRPOLYPOLYGON16*)lpEMFR;
      POINTS* apts = (POINTS*)(data->aPolyCounts + data->nPolys); /* skip the array of counts */
      int c, i;                                                   /* data->apts can not be used if count greater than 1 */
      
      for (c = 0; c < (int)data->nPolys; c++)
      {
        cdCanvasBegin(canvas, CD_FILL);
        
        for (i = 0; i < (int)data->aPolyCounts[c]; i++, apts++)
          cdCanvasVertex(canvas, sScaleX(apts->x), sScaleY(apts->y));
        
        cdCanvasEnd(canvas);
      }
      break;
    }
  case EMR_POLYDRAW16:
    {
      int p;
      EMRPOLYDRAW16* data = (EMRPOLYDRAW16*)lpEMFR;
      
      for(p = 0; p < (int)data->cpts; p++)
      {
        switch (data->abTypes[p])
        {
        case PT_MOVETO:
          {
            curx = data->apts[p].x;
            cury = data->apts[p].y;
            break;
          }
        case PT_LINETO:
          {
            cdCanvasLine(canvas, sScaleX(curx), sScaleY(cury), sScaleX(data->apts[p].x), sScaleY(data->apts[p].y));
            curx = data->apts[p].x;
            cury = data->apts[p].y;
            break;
          }
        }
      }
      break;
    }
  case EMR_CREATEMONOBRUSH:
    {
      EMRCREATEMONOBRUSH* data = (EMRCREATEMONOBRUSH*)lpEMFR;
      int size, i;
      cdwDIB dib;
      long *pattern;
      unsigned char *stipple;
      
      cdwDIBReference(&dib, ((BYTE*)data) + data->offBmi, ((BYTE*)data) + data->offBits);
      
      if (dib.type == -1)
        break;
      
      size = dib.w*abs(dib.h);
      
      if (dib.bmih->biBitCount == 1)
        stipple = malloc(size);
      else
        pattern = (long*)malloc(size*sizeof(long));
      
      if (dib.type == 0)
      {
        unsigned char *r, *g, *b;
        
        r = (unsigned char*)malloc(size);
        g = (unsigned char*)malloc(size);
        b = (unsigned char*)malloc(size);
        
        cdwDIBDecodeRGB(&dib, r, g, b);
        
        for (i = 0; i < size; i++)
          pattern[i] = cdEncodeColor(r[i], g[i], b[i]);
        
        free(r);
        free(g);
        free(b);
      }
      else
      {
        unsigned char *index;
        long *colors;
        
        index = (unsigned char*)malloc(size);
        colors = (long*)malloc(256*sizeof(long));
        
        cdwDIBDecodeMap(&dib, index, colors);
        
        if (dib.bmih->biBitCount == 1)
        {
          for (i = 0; i < size; i++)
            stipple[i] = index[i]? 0: 1;
        }
        else
        {
          for (i = 0; i < size; i++)
            pattern[i] = colors[index[i]];
        }
        
        free(index);
        free(colors);
      }
      
      if (dib.bmih->biBitCount == 1)
      {
        cdCanvasStipple(canvas, dib.w, abs(dib.h), stipple);
        free(stipple);
      }
      else
      {
        cdCanvasPattern(canvas, dib.w, abs(dib.h), pattern);
        free(pattern);
      }
      break;
    }
  case EMR_CREATEDIBPATTERNBRUSHPT:
    {
      EMRCREATEDIBPATTERNBRUSHPT* data = (EMRCREATEDIBPATTERNBRUSHPT*)lpEMFR;
      int size, i;
      cdwDIB dib;
      long *pattern;
      unsigned char *stipple;
      
      cdwDIBReference(&dib, ((BYTE*)data) + data->offBmi, ((BYTE*)data) + data->offBits);
      
      if (dib.type == -1)
        break;
      
      size = dib.w*abs(dib.h);
      
      if (dib.bmih->biBitCount == 1)
        stipple = malloc(size);
      else
        pattern = malloc(size*sizeof(long));
      
      if (dib.type == 0)
      {
        unsigned char *r, *g, *b;
        
        r = (unsigned char*)malloc(size);
        g = (unsigned char*)malloc(size);
        b = (unsigned char*)malloc(size);
        
        cdwDIBDecodeRGB(&dib, r, g, b);
        
        for (i = 0; i < size; i++)
          pattern[i] = cdEncodeColor(r[i], g[i], b[i]);
        
        free(r);
        free(g);
        free(b);
      }
      else
      {
        unsigned char *index;
        long *colors;
        
        index = (unsigned char*)malloc(size);
        colors = (long*)malloc(256*sizeof(long));
        
        cdwDIBDecodeMap(&dib, index, colors);

        if (dib.bmih->biBitCount == 1)
        {
          for (i = 0; i < size; i++)
            stipple[i] = index[i]? 0: 1;
        }
        else
        {
          for (i = 0; i < size; i++)
            pattern[i] = colors[index[i]];
        }
        
        free(index);
        free(colors);
      }
      
      if (dib.bmih->biBitCount == 1)
      {
        cdCanvasStipple(canvas, dib.w, abs(dib.h), stipple);
        free(stipple);
      }
      else
      {
        cdCanvasPattern(canvas, dib.w, abs(dib.h), pattern);
        free(pattern);
      }
      break;
    }
  case EMR_POLYTEXTOUTA:
    {
      int t;
      EMRPOLYTEXTOUTA* data = (EMRPOLYTEXTOUTA*)lpEMFR;
      
      for (t = 0; t < data->cStrings; t++)
      {
        char* str = malloc(data->aemrtext[t].nChars + 1);
        memcpy(str, ((unsigned char*)data) + data->aemrtext[t].offString, data->aemrtext[t].nChars + 1);
        str[data->aemrtext[t].nChars] = 0;
        
        cdCanvasText(canvas, sScaleX(data->aemrtext[t].ptlReference.x), sScaleY(data->aemrtext[t].ptlReference.y), str);
        
        free(str);
      }
      break;
    }
  case EMR_POLYTEXTOUTW:
    {
      int t;
      EMRPOLYTEXTOUTW* data = (EMRPOLYTEXTOUTW*)lpEMFR;
      char str[256];
      
      for (t = 0; t < data->cStrings; t++)
      {
        WideCharToMultiByte(CP_ACP, 0, (unsigned short*)(((unsigned char*)data) + data->aemrtext[t].offString), data->aemrtext[t].nChars, str, 256, NULL, NULL);
        str[data->aemrtext[t].nChars] = 0;
        cdCanvasText(canvas, sScaleX(data->aemrtext[t].ptlReference.x), sScaleY(data->aemrtext[t].ptlReference.y), str);
      }
      break;
    }
  }

  return 1;         
}


static cdSizeCB cdsizecbWMF = NULL;

int cdregistercallbackWMF(int cb, cdCallback func)
{
  switch (cb)
  {
  case CD_SIZECB:
    cdsizecbWMF = (cdSizeCB)func;
    return CD_OK;
  }
  
  return CD_ERROR;
}


/***********************************************************************
Read the metafile bits, metafile header and placeable
metafile header of a placeable metafile.
************************************************************************/

static HANDLE GetPlaceableMetaFile(int fh)
{
  HANDLE hMF;
  HANDLE   hMem;
  LPSTR    lpMem;
  int	   wBytesRead;
  APMFILEHEADER aldusMFHeader;
  METAHEADER mfHeader;
  
  /* seek to beginning of file and read aldus header */
  lseek(fh, 0, 0);
  
  /* read the placeable header */
  wBytesRead = read(fh, (LPSTR)&aldusMFHeader, sizeof(APMFILEHEADER));
  
  /* if there is an error, return */
  if(wBytesRead == -1 || wBytesRead < sizeof(APMFILEHEADER))	
    return NULL;
  
  /* read the metafile header */
  wBytesRead = read(fh, (LPSTR)&mfHeader, sizeof(METAHEADER));
  
  /* if there is an error return */
  if( wBytesRead == -1 || wBytesRead < sizeof(METAHEADER) )  
    return NULL;
  
  /* allocate memory for the metafile bits */
  if (!(hMem = GlobalAlloc(GHND, mfHeader.mtSize * 2L)))  
    return NULL;
  
  /* lock the memory */
  if (!(lpMem = GlobalLock(hMem)))
  {
    GlobalFree(hMem);
    return NULL;
  }
  
  /* seek to the metafile bits */
  lseek(fh, sizeof(APMFILEHEADER), 0);
  
  /* read metafile bits */
  wBytesRead = read(fh, lpMem, (WORD)(mfHeader.mtSize * 2L));
  
  /* if there was an error */
  if( wBytesRead == -1 )  
  {
    GlobalUnlock(hMem);
    GlobalFree(hMem);
    return NULL;
  }
  
  if (!(hMF = SetMetaFileBitsEx(mfHeader.mtSize * 2L, lpMem)))
    return NULL;
  
  GlobalUnlock(hMem);
  GlobalFree(hMem);
  
  return hMF;
}


/* 
%F cdPlay para WMF.
Interpreta os dados do WMF.
*/
int cdplayWMF(cdCanvas* canvas, int xmin, int xmax, int ymin, int ymax, void *data)
{
  char* filename = (char*)data;
  int		 fh;
  int		 wBytesRead;
  DWORD  dwIsAldus;
  HANDLE hMF;
  HENHMETAFILE hEMF;
  ENHMETAHEADER emh;
  BYTE* buffer;
  int size;
  
  /* try to open the file. */
  fh = open(filename, O_BINARY | O_RDONLY);
  
  /* if opened failed */
  if (fh == -1)  
    return CD_ERROR;
  
  /* read the first dword of the file to see if it is a placeable wmf */
  wBytesRead = read(fh,(LPSTR)&dwIsAldus, sizeof(dwIsAldus));
  if (wBytesRead == -1 || wBytesRead < sizeof(dwIsAldus))  
  {
    close(fh);
    return CD_ERROR;
  }
  
  /* if this is windows metafile, not a placeable wmf */
  if (dwIsAldus != ALDUSKEY)  
  {
    METAHEADER mfHeader;
    
    /* seek to the beginning of the file */
    lseek(fh, 0, 0);
    
    /* read the wmf header */
    wBytesRead = read(fh, (LPSTR)&mfHeader, sizeof(METAHEADER));
    
    /* done with file so close it */
    close(fh);
    
    /* if read failed */
    if (wBytesRead == -1 || wBytesRead < sizeof(METAHEADER))  
      return CD_ERROR;
    
    hMF = GetMetaFile(filename);
  }
  else /* this is a placeable metafile */
  {
  /* convert the placeable format into something that can
    be used with GDI metafile functions */
    hMF = GetPlaceableMetaFile(fh);
    
    /* close the file */
    close(fh);
  }
  
  if (!hMF) 
    return CD_ERROR;
  
  size = GetMetaFileBitsEx(hMF, 0, NULL);
  
  buffer = malloc(size);
  
  GetMetaFileBitsEx(hMF, size, buffer);
  
  hEMF = SetWinMetaFileBits(size, buffer, NULL, NULL);
  
  GetEnhMetaFileHeader(hEMF, sizeof(ENHMETAHEADER), &emh);
  
  /* when converted from WMF, only rclBounds is available */
  wmf_bottom = emh.rclBounds.bottom;
  wmf_left   = emh.rclBounds.left;
  wmf_top    = emh.rclBounds.top;
  wmf_right  = emh.rclBounds.right;
  
  if ((xmax-xmin+1)>1 && (ymax-ymin+1)>1) /* always update wmf_rect when scaling */
    EnumEnhMetaFile(NULL, hEMF, CalcSizeEMFEnumProc, NULL, NULL);

  if ((wmf_bottom-wmf_top)>1 && 
      (wmf_right-wmf_left)>1 && 
      (xmax-xmin+1)>1 && 
      (ymax-ymin+1)>1)
  {
    wmf_yfactor = ((double)(ymax-ymin+1)) / (double)(wmf_top-wmf_bottom);   /* negative because top-down orientation */
    wmf_xfactor = ((double)(xmax-xmin+1)) / (double)(wmf_right-wmf_left);
    wmf_xmin = xmin;
    wmf_ymin = ymin;
  }
  else
  {
    wmf_yfactor = -1;   /* negative because top-down orientation */
    wmf_xfactor = 1;
    wmf_xmin = 0;
    wmf_ymin = 0;
  }
  
  free(buffer);
  
  if (cdsizecbWMF && dwIsAldus == ALDUSKEY)
  {
    int err;
    err = cdsizecbWMF(canvas, wmf_right-wmf_left, wmf_bottom-wmf_top, 0, 0);
    if (err)
    {
      DeleteEnhMetaFile (hEMF);
      return CD_ERROR;
    }
  }
  
  EnumEnhMetaFile(NULL, hEMF, EMFEnumProc, canvas, NULL);
  
  DeleteEnhMetaFile (hEMF);
  
  DeleteMetaFile (hMF);
  
  return CD_OK;
}


static cdSizeCB cdsizecbEMF = NULL;

int cdregistercallbackEMF(int cb, cdCallback func)
{
  switch (cb)
  {
  case CD_SIZECB:
    cdsizecbEMF = (cdSizeCB)func;
    return CD_OK;
  }
  
  return CD_ERROR;
}


int cdplayEMF(cdCanvas* canvas, int xmin, int xmax, int ymin, int ymax, void *data)
{
  char* filename = (char*)data;
  HENHMETAFILE hEMF;
  ENHMETAHEADER emh;
  double xres, yres;
  
  hEMF = GetEnhMetaFile(filename);
  
  GetEnhMetaFileHeader(hEMF, sizeof(ENHMETAHEADER), &emh);

  /* this is obtained from the hdcRef of CreateEnhMetaFile */
  xres = ((double)emh.szlDevice.cx) / emh.szlMillimeters.cx;
  yres = ((double)emh.szlDevice.cy) / emh.szlMillimeters.cy;
  
  /* this is the same as used in RECT of CreateEnhMetaFile */
  wmf_bottom = (int)((emh.rclFrame.bottom * yres) / 100);
  wmf_left   = (int)((emh.rclFrame.left * xres) / 100);
  wmf_top    = (int)((emh.rclFrame.top * yres) / 100);
  wmf_right  = (int)((emh.rclFrame.right * xres) / 100);

  if ((xmax-xmin+1)>1 && (ymax-ymin+1)>1) /* always update wmf_rect when scaling */
    EnumEnhMetaFile(NULL, hEMF, CalcSizeEMFEnumProc, NULL, NULL);

  if ((wmf_bottom-wmf_top)>1 && 
      (wmf_right-wmf_left)>1 && 
      (xmax-xmin+1)>1 && 
      (ymax-ymin+1)>1)
  {
    wmf_yfactor = ((double)(ymax-ymin+1)) / (double)(wmf_top-wmf_bottom);   /* negative because top-down orientation */
    wmf_xfactor = ((double)(xmax-xmin+1)) / (double)(wmf_right-wmf_left);
    wmf_xmin = xmin;
    wmf_ymin = ymin;
  }
  else
  {
    wmf_yfactor = -1;   /* negative because top-down orientation */
    wmf_xfactor = 1;
    wmf_xmin = 0;
    wmf_ymin = 0;
  }
  
  if (cdsizecbEMF)
  {
    int err;
    err = cdsizecbEMF(canvas, wmf_right-wmf_left, wmf_bottom-wmf_top, 0, 0);
    if (err)
    {
      DeleteEnhMetaFile (hEMF);
      return CD_ERROR;
    }
  }
  
  EnumEnhMetaFile(NULL, hEMF, EMFEnumProc, canvas, NULL);
  
  DeleteEnhMetaFile (hEMF);
  
  return CD_OK;
}

/*
%F Calculo do CheckSum.
*/
static WORD sAPMChecksum(APMFILEHEADER* papm)
{
  WORD* pw = (WORD*)papm;
  WORD  wSum = 0;
  int   i;
  
  /* The checksum in a Placeable Metafile header is calculated */
  /* by XOR-ing the first 10 words of the header.              */
  
  for (i = 0; i < 10; i++)
    wSum ^= *pw++;
  
  return wSum;
}



/*
Aldus placeable metafile format

 DWORD    key;
 HANDLE   hmf;
 RECT     bbox;
 WORD     inch;
 DWORD    reserved;
 WORD     checksum;
 char     metafileData[];
 
  These fields have the following  meanings:
  
  Field         Definition

  key           Binary key that uniquely identifies this
                file type.  This must be 0x9AC6CDD7L.

  hmf           Unused;  must be zero.

  bbox          The coordinates of a rectangle that tightly
                bounds the picture. These coordinates are in
                metafile units as defined below.

  inch          The number of metafile units to the inch.  To
                avoid numeric overflow in PageMaker, this value
                should be less than 1440.

  reserved      A reserved double word.  Must be zero.

  checksum      A checksum of the 10 words that precede it,
                calculated by XORing zero with these 10 words
                and putting the result in the checksum field.

  metafileData  The actual content of the Windows metafile
                retrieved by copying the data returned by
                GetMetafileBits to the file.  The number of
                bytes should be equal to the MS-DOS file length
                minus 22.  The content of a PageMaker placeable
                metafile  cannot currently exceed 64K (this may
                have changed in 4.0).
*/

/* 
%F Cria um APM em arquivo a partir de um WMF em memoria.
*/
void wmfMakePlaceableMetafile(HMETAFILE hmf, const char* filename, int w, int h)
{
  int fh, nSize;
  LPSTR lpData;
  APMFILEHEADER APMHeader;
  
  fh = open(filename, O_CREAT | O_BINARY | O_WRONLY);
  
  APMHeader.key1 = 0xCDD7;
  APMHeader.key2 = 0x9AC6;
  APMHeader.hmf = 0;
  APMHeader.bleft = 0;
  APMHeader.btop = 0;
  APMHeader.bright = (short)w;
  APMHeader.bbottom = (short)h;
  APMHeader.inch = 100;  /* this number works fine in Word, etc.. */
  APMHeader.reserved1 = 0;
  APMHeader.reserved2 = 0;
  APMHeader.checksum = sAPMChecksum(&APMHeader);
  
  write(fh, (LPSTR)&APMHeader, sizeof(APMFILEHEADER));
  
  nSize = GetMetaFileBitsEx(hmf, 0, NULL);
  lpData = malloc(nSize);
  GetMetaFileBitsEx(hmf, nSize, lpData);
  
  write(fh, lpData, nSize);
  free(lpData);
  
  close(fh);
}

void wmfWritePlacebleFile(HANDLE hFile, unsigned char* buffer, DWORD dwSize, LONG mm, LONG xExt, LONG yExt)
{
  DWORD nBytesWrite;
  APMFILEHEADER APMHeader;
  int w = xExt, h = yExt;
  
  if (mm == MM_ANISOTROPIC || mm == MM_ISOTROPIC)
  {
    int res = 30;
    w = xExt / res;
    h = yExt / res;
  }
  
  APMHeader.key1 = 0xCDD7;
  APMHeader.key2 = 0x9AC6;
  APMHeader.hmf = 0;
  APMHeader.bleft = 0;
  APMHeader.btop = 0;
  APMHeader.bright = (short)w;
  APMHeader.bbottom = (short)h;
  APMHeader.inch = 100;  /* this number works fine in Word, etc.. */
  APMHeader.reserved1 = 0;
  APMHeader.reserved2 = 0;
  APMHeader.checksum = sAPMChecksum(&APMHeader);
  
  WriteFile(hFile, (LPSTR)&APMHeader, sizeof(APMFILEHEADER), &nBytesWrite, NULL);
  WriteFile(hFile, buffer, dwSize, &nBytesWrite, NULL);
}

