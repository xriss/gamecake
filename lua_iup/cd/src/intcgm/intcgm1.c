#define _INTCGM1_C_

#include <stdio.h>      /* FILE, ftell, fseek, fputc, fopen, fclose, fputs, fprintf */
#include <stdlib.h>     /* malloc, free */
#include <string.h>     /* strlen */
#include <math.h>       /* floor */

#ifdef SunOS
#include <unistd.h>       /* SEEK_SET, SEEK_END */
#endif

#include <float.h>      /* FLT_MIN, FLT_MAX */
#include <limits.h>     /* INT_MIN, INT_MAX */

#include "cd.h"
#include "cdcgm.h"

#include "list.h"
#include "types.h"
#include "intcgm2.h"
#include "intcgm.h"
#include "bparse.h"

static int isitbin ( char *fname )
{
 unsigned char ch[2];
 int erro, c, id;
 unsigned short b;
 FILE *f = fopen ( fname, "rb" );
 if (!f)
   return 0;

 erro = fread ( ch, 1, 2, f );
  
 b = (ch[0] << 8) + ch[1];

 id = ( b & 0x0FE0 ) >> 5;

 c = ( b & 0xF000 ) >> 12;
 
 fclose(f);
 
 if ( c==0 && id==1 )
  return 1;
 else
  return 0; 
}

static int cgmplay ( char* filename, int xmn, int xmx, int ymn, int ymx )
{
 intcgm_view_xmin = xmn;
 intcgm_view_xmax = xmx;
 intcgm_view_ymin = ymn;
 intcgm_view_ymax = ymx;

 intcgm_scale_factor_x = 1;
 intcgm_scale_factor_y = 1;
 intcgm_scale_factor   = 1;

 intcgm_block = 500;
 intcgm_npoints = 500;
 intcgm_cgm.buff.size = 1024;

 intcgm_point_list = (tpoint *) malloc ( sizeof(tpoint)*intcgm_npoints);
 intcgm_cgm.buff.dados = (char *) malloc ( sizeof(char) * intcgm_cgm.buff.size );

 if ( isitbin(filename) )
 {
   intcgm_cgm.fp = fopen ( filename, "rb" );
   if (!intcgm_cgm.fp)
   {
     free(intcgm_point_list);
     free(intcgm_cgm.buff.dados);
     return CD_ERROR;
   }

   fseek ( intcgm_cgm.fp, 0, SEEK_END );
   intcgm_cgm.file_size = ftell ( intcgm_cgm.fp );
   fseek ( intcgm_cgm.fp, 0, SEEK_SET );

   intcgm_cgm.mode = 1;
   intcgm_cgm.cgmf = intcgm_funcs[intcgm_cgm.mode];

   intcgm_cgm.int_prec.b_prec  = 1;
   intcgm_cgm.real_prec.b_prec = 2;
   intcgm_cgm.ix_prec.b_prec   = 1;
   intcgm_cgm.cd_prec   = 0;
   intcgm_cgm.cix_prec  = 0;
   intcgm_cgm.vdc_int.b_prec  = 1;
   intcgm_cgm.vdc_real.b_prec = 2;
 }
 else
 {
   intcgm_cgm.fp = fopen ( filename, "r" );
   if (!intcgm_cgm.fp)
   {
     free(intcgm_point_list);
     free(intcgm_cgm.buff.dados);
     return CD_ERROR;
   }

   fseek ( intcgm_cgm.fp, 0, SEEK_END );
   intcgm_cgm.file_size = ftell ( intcgm_cgm.fp );
   fseek ( intcgm_cgm.fp, 0, SEEK_SET );

   intcgm_cgm.mode = 2;
   intcgm_cgm.cgmf = intcgm_funcs[intcgm_cgm.mode];

   intcgm_cgm.int_prec.t_prec.minint  = -32767;
   intcgm_cgm.int_prec.t_prec.maxint  = 32767;
   intcgm_cgm.real_prec.t_prec.minreal = -32767;
   intcgm_cgm.real_prec.t_prec.maxreal = 32767;
   intcgm_cgm.real_prec.t_prec.digits = 4;
   intcgm_cgm.ix_prec.t_prec.minint   = 0;
   intcgm_cgm.ix_prec.t_prec.maxint   = 127;
   intcgm_cgm.cd_prec   = 127;
   intcgm_cgm.vdc_int.t_prec.minint  = -32767;
   intcgm_cgm.vdc_int.t_prec.maxint  = 32767;
   intcgm_cgm.vdc_real.t_prec.minreal = 0;
   intcgm_cgm.vdc_real.t_prec.maxreal = 1;
   intcgm_cgm.vdc_real.t_prec.digits = 4;
 }

 intcgm_cgm.first = 1;
 intcgm_cgm.len = 0;
 intcgm_cgm.vdc_type = INTEGER;
 intcgm_cgm.max_cix = 63;
 intcgm_cgm.scaling_mode.mode = ABSTRACT;
 intcgm_cgm.scaling_mode.scale_factor = 1.;
 intcgm_cgm.drawing_mode = ABSTRACT;
 intcgm_cgm.clrsm = INDEXED;
 intcgm_cgm.lnwsm = SCALED;
 intcgm_cgm.mkssm = SCALED;
 intcgm_cgm.edwsm = SCALED;
 intcgm_cgm.vdc_ext.first.x = 0;
 intcgm_cgm.vdc_ext.first.y = 0;
 intcgm_cgm.vdc_ext.second.x = 32767;
 intcgm_cgm.vdc_ext.second.y = 32767;
 intcgm_cgm.back_color.red = 0;
 intcgm_cgm.back_color.green = 0;
 intcgm_cgm.back_color.blue = 0;
 intcgm_cgm.aux_color.rgb.red = 0;
 intcgm_cgm.aux_color.rgb.green = 0;
 intcgm_cgm.aux_color.rgb.blue = 0;
 intcgm_cgm.color_ext.black.red = 0;
 intcgm_cgm.color_ext.black.green = 0;
 intcgm_cgm.color_ext.black.blue = 0;
 intcgm_cgm.color_ext.white.red = 255;
 intcgm_cgm.color_ext.white.green = 255;
 intcgm_cgm.color_ext.white.blue = 255;
 intcgm_cgm.transparency = ON;
 intcgm_cgm.clip_rect.first.x = 0;
 intcgm_cgm.clip_rect.first.y = 0;
 intcgm_cgm.clip_rect.second.x = 32767;
 intcgm_cgm.clip_rect.second.y = 32767;
 intcgm_cgm.clip_ind = ON;
 intcgm_cgm.bc = 0;
 intcgm_cgm.bl= 0;
 intcgm_cgm.cl = 0;

 intcgm_line_att.index = 1;
 intcgm_line_att.type = LINE_SOLID;
 intcgm_line_att.width = 1;
 intcgm_line_att.color.ind = 1;

 intcgm_marker_att.index = 1;
 intcgm_marker_att.type = 1;
 intcgm_marker_att.size = 1;
 intcgm_marker_att.color.ind = 1;

 intcgm_text_att.index = 1;
 intcgm_text_att.font_index = 1;
 intcgm_text_att.font_list = NULL;
 intcgm_text_att.font = 0;
 intcgm_text_att.style = CD_PLAIN;
 intcgm_text_att.size = 8;
 intcgm_text_att.prec = STRING;
 intcgm_text_att.exp_fact = 1;
 intcgm_text_att.char_spacing = 0;
 intcgm_text_att.color.ind = 1;
 intcgm_text_att.height = 1;
 intcgm_text_att.char_up.x = 0;
 intcgm_text_att.char_up.y = 1;
 intcgm_text_att.char_base.x = 1;
 intcgm_text_att.char_base.y = 0;
 intcgm_text_att.path = PATH_RIGHT;
 intcgm_text_att.alignment.hor = NORMHORIZ;
 intcgm_text_att.alignment.ver  = NORMVERT;
 intcgm_text_att.alignment.cont_hor = 0;
 intcgm_text_att.alignment.cont_ver = 0;

 intcgm_fill_att.index = 1;
 intcgm_fill_att.int_style = HOLLOW;
 intcgm_fill_att.color.ind = 1;
 intcgm_fill_att.hatch_index = 1;
 intcgm_fill_att.pat_index = 1;
 intcgm_fill_att.ref_pt.x = 0;
 intcgm_fill_att.ref_pt.y = 0;
 intcgm_fill_att.pat_list = NULL;
 intcgm_fill_att.pat_size.height.x = 0;
 intcgm_fill_att.pat_size.height.y = 0;
 intcgm_fill_att.pat_size.height.x = 0;
 intcgm_fill_att.pat_size.width.y = 0;

 intcgm_edge_att.index = 1;
 intcgm_edge_att.type  = EDGE_SOLID;
 intcgm_edge_att.width = 1;
 intcgm_edge_att.color.ind = 1;
 intcgm_edge_att.visibility = OFF;

 cdCanvasLineWidth(intcgm_canvas, 1);

 intcgm_color_table = (trgb *) malloc ( sizeof(trgb)*intcgm_cgm.max_cix);
 intcgm_color_table[0].red   = 255;
 intcgm_color_table[0].green = 255;
 intcgm_color_table[0].blue  = 255;
 intcgm_color_table[1].red   = 0;
 intcgm_color_table[1].green = 0;
 intcgm_color_table[1].blue  = 0;

 while ( !(*intcgm_cgm.cgmf)() ){};

 if ( intcgm_point_list!=NULL )
 {
   free(intcgm_point_list);
   intcgm_point_list = NULL;
 }

 if ( intcgm_cgm.buff.dados!=NULL )
 {
   free(intcgm_cgm.buff.dados);
   intcgm_cgm.buff.dados = NULL;
 }

 if ( intcgm_color_table!=NULL )
 {
   free(intcgm_color_table);
   intcgm_color_table = NULL;
 }

 fclose(intcgm_cgm.fp);

 return CD_OK;
}

_cdcgmsizecb cdcgmsizecb = NULL;
_cdcgmbegmtfcb cdcgmbegmtfcb = NULL;
_cdcgmcountercb cdcgmcountercb = NULL;
_cdcgmsclmdecb cdcgmsclmdecb = NULL;
_cdcgmvdcextcb cdcgmvdcextcb = NULL;
_cdcgmbegpictcb cdcgmbegpictcb = NULL;
_cdcgmbegpictbcb cdcgmbegpictbcb = NULL;

int cdRegisterCallbackCGM(int cb, cdCallback func)
{
  switch (cb)
  {
  case CD_SIZECB:
    cdcgmsizecb = (_cdcgmsizecb)func;
    return CD_OK;
  case CD_CGMBEGMTFCB:
    cdcgmbegmtfcb = (_cdcgmbegmtfcb)func;
    return CD_OK;
  case CD_CGMCOUNTERCB:
    cdcgmcountercb = (_cdcgmcountercb)func;
    return CD_OK;
  case CD_CGMSCLMDECB:
    cdcgmsclmdecb = (_cdcgmsclmdecb)func;
    return CD_OK;
  case CD_CGMVDCEXTCB:
    cdcgmvdcextcb = (_cdcgmvdcextcb)func;
    return CD_OK;
  case CD_CGMBEGPICTCB:
    cdcgmbegpictcb = (_cdcgmbegpictcb)func;
    return CD_OK;
  case CD_CGMBEGPICTBCB:
    cdcgmbegpictbcb = (_cdcgmbegpictbcb)func;
    return CD_OK;
  }
  
  return CD_ERROR;
}

int cdplayCGM(cdCanvas* _canvas, int xmin, int xmax, int ymin, int ymax, void *data)
{
  int ret;
  intcgm_canvas = _canvas;
  ret = cgmplay((char*)data, xmin, xmax, ymin, ymax);
  _canvas = NULL;
  return ret;
}
