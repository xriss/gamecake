#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <limits.h>
#include <float.h>

#include <cd.h>

#include "list.h"
#include "types.h"
#include "intcgm.h"
#include "intcgm2.h"
#include "intcgm6.h"

static tpoint trace_start_pos, base_direction, amp_direction, trace_direction;
static double bline_sc_f, amp_sc_f, trc_st_f, VA_bline_offset, pos_clp_lmt,
             neg_clp_lmt, pos_bckfil_bnd, neg_bckfil_bnd;
static int trace_dsp_md, samp_type, n_samp, wig_trc_mod, nul_clr_i, n_trace;
static double *sample;
static long *coind;
static int trace=0;

void cgm_sism4 ( tpoint *pt, char *data_rec )
{
 sscanf ( data_rec, "%lg %lg %lg %lg %lg %lg %d %d %d %lg %lg %*d %d %d %d",
          &bline_sc_f, &amp_sc_f, &trc_st_f, &VA_bline_offset, &pos_clp_lmt,
          &neg_clp_lmt, &trace_dsp_md, &samp_type, &n_samp, &pos_bckfil_bnd, 
          &neg_bckfil_bnd, &wig_trc_mod, &nul_clr_i, &n_trace );

 trace_start_pos = pt[0];
 base_direction = pt[1];
 amp_direction = pt[2];
 trace_direction = pt[3];
 trace = 0;
}

/*     adjust the samples to the amplitude direction vector */
static void dirvet ( double dx )
{
 int i;

 for ( i=0; i<n_samp; i++ )
  sample[i] *= dx/fabs(dx);
}

/*     adust the x offset from the baseline to maximum amplitude */
static void ampscf ( double dx )
{
 double max=1;
 int i;

 if ( samp_type == 0 || samp_type == 4 ) max = (double) SHRT_MAX;
 else if ( samp_type == 1 ) max = (double) INT_MAX;
 else if ( samp_type == 2 ) max = (double) FLT_MAX;
 else if ( samp_type == 3 || samp_type == 5 ) max = (double) SCHAR_MAX;

 for ( i=0; i<n_samp; i++ )
  sample[i] = (sample[i]/max) * amp_sc_f * dx;
}

static double interpl ( int i, double y1, double y2, double x )
{
 return ( y1*(x-sample[i+1]) + y2*(sample[i]-x) ) / ( sample[i]-sample[i+1] );
}

static void vasamp ( int mode, double max, double min, int cordep )
{
 double factx, facty;
 double x[5], y[5];
 double samp1, samp2, mn, mx;
 long int cor;
 int i,j,n;

 facty = base_direction.y * bline_sc_f;
 factx = trace_direction.x * trc_st_f;

 max *= amp_sc_f;
 min *= amp_sc_f;
 mn = min;
 mx = max;

 if ( mode==64 )
  {
   double tmp = min;
   min = max;
   max = tmp;
   mn = min * -1.;
   mx = max * -1.;
  }

 for ( i=0; i<(n_samp-1); i++ )
  {
   double y1 = trace_start_pos.y + facty*(i);
   double y2 = trace_start_pos.y + facty*(i+1);
   double dx = trace_start_pos.x + factx*(trace-1);
   
   samp1 = sample[i];
   samp2 = sample[i+1];

   if ( mode==64 )
    {
     samp1 *= -1.;
     samp2 *= -1.;
    }

   n=0;

   if (samp1<mn && samp2>mn && samp2<mx)
    {
     x[n]   = min + dx;
     y[n++] = interpl(i,y1,y2,min);
     x[n]   = sample[i+1] + dx;
     y[n++] = y2;
     x[n]   = min + dx;
     y[n++] = y2;
    }

   else if (samp1<mn && samp2>mx)
    {
     x[n]   = min + dx;
     y[n++] = interpl(i,y1,y2,min);
     x[n]   = max + dx;
     y[n++] = interpl(i,y1,y2,max);
     x[n]   = max + dx;
     y[n++] = y2;
     x[n]   = min + dx;
     y[n++] = y2;
    }

   else if ( (samp1>mn && samp1<mx) &&
             (samp2>mn && samp2<mx) )
    {
     x[n]   = min + dx;
     y[n++] = y1;
     x[n]   = sample[i] + dx;
     y[n++] = y1;
     x[n]   = sample[i+1] + dx;
     y[n++] = y2;
     x[n]   = min + dx;
     y[n++] = y2;
    }

   else if ( samp1>mn && samp1<mx && samp2>mx )
    {
     x[n]   = min + dx;
     y[n++] = y1;
     x[n]   = sample[i] + dx;
     y[n++] = y1;
     x[n]   = max + dx;
     y[n++] = interpl(i,y1,y2,max);
     x[n]   = max + dx;
     y[n++] = y2;
     x[n]   = min + dx;
     y[n++] = y2;
    }

   else if ( samp1>mx && samp2>mx )
    {
     x[n]   = min + dx;
     y[n++] = y1;
     x[n]   = max + dx;
     y[n++] = y1;
     x[n]   = max + dx;
     y[n++] = y2;
     x[n]   = min + dx;
     y[n++] = y2;
    }

   else if ( samp1>mx && samp2<mx && samp2>mn )
    {
     x[n]   = min + dx;
     y[n++] = y1;
     x[n]   = max + dx;
     y[n++] = y1;
     x[n]   = max + dx;
     y[n++] = interpl(i,y1,y2,max);
     x[n]   = sample[i+1] + dx;
     y[n++] = y2;
     x[n]   = min + dx;
     y[n++] = y2;
    }

   else if ( samp1>mn && samp1<mx && samp2<mn )
    {
     x[n]   = min + dx;
     y[n++] = y1;
     x[n]   = sample[i] + dx;
     y[n++] = y1;
     x[n]   = min + dx;
     y[n++] = interpl(i,y1,y2,min);
    }

   if ( n>0 )
    {
     if ( cordep )
      {
       cor = cdEncodeColor ( (unsigned char)((intcgm_color_table[coind[i]].red*255)/intcgm_cgm.color_ext.white.red),
                             (unsigned char)((intcgm_color_table[coind[i]].green*255)/intcgm_cgm.color_ext.white.green),
                             (unsigned char)((intcgm_color_table[coind[i]].blue*255)/intcgm_cgm.color_ext.white.blue) );
       cdCanvasSetForeground (intcgm_canvas,  cor );
      }

     cdCanvasBegin(intcgm_canvas,  CD_FILL );

     for ( j=0; j<n; j++ )
      cdCanvasVertex (intcgm_canvas,  cgm_vdcx2canvas(x[j]), cgm_vdcy2canvas(y[j]) );

     cdCanvasEnd(intcgm_canvas);
    }
  }
}

static void bgclfl ( int mode )
{
 double factx, facty;
 double posdx, negdx;
 long int cor;
 int i;

 facty = base_direction.y * bline_sc_f;
 factx = trace_direction.x * trc_st_f;

 posdx = pos_bckfil_bnd * amp_sc_f * amp_direction.x;
 negdx = neg_bckfil_bnd * amp_sc_f * amp_direction.x;

 cdCanvasInteriorStyle(intcgm_canvas,  CD_SOLID );

 for ( i=0; i<n_samp; i++ )
  {
   int index = ( coind[i] <= intcgm_cgm.max_cix ) ? coind[i] : 1;

   cor = cdEncodeColor ( (unsigned char)((intcgm_color_table[index].red*255)/intcgm_cgm.color_ext.white.red),
                         (unsigned char)((intcgm_color_table[index].green*255)/intcgm_cgm.color_ext.white.green),
                         (unsigned char)((intcgm_color_table[index].blue*255)/intcgm_cgm.color_ext.white.blue) );
   cdCanvasSetForeground (intcgm_canvas,  cor );

   cdCanvasBegin(intcgm_canvas,  CD_FILL );

   cdCanvasVertex (intcgm_canvas,  cgm_vdcx2canvas(posdx + trace_start_pos.x + factx*(trace-1)),
              cgm_vdcy2canvas(trace_start_pos.y + facty*(i)) );
   cdCanvasVertex (intcgm_canvas,  cgm_vdcx2canvas(posdx + trace_start_pos.x + factx*(trace-1)),
              cgm_vdcy2canvas(trace_start_pos.y + facty*(i+1)) );
   cdCanvasVertex (intcgm_canvas,  cgm_vdcx2canvas(negdx + trace_start_pos.x + factx*(trace-1)),
              cgm_vdcy2canvas(trace_start_pos.y + facty*(i+1)) );
   cdCanvasVertex (intcgm_canvas,  cgm_vdcx2canvas(negdx + trace_start_pos.x + factx*(trace-1)),
              cgm_vdcy2canvas(trace_start_pos.y + facty*(i)) );

   cdCanvasEnd(intcgm_canvas);
  }
}

void wiggle ( double posclp, double negclp )
{
 int i;
 double facty = base_direction.y * bline_sc_f;
 double factx = trace_direction.x * trc_st_f;
 double dx = trace_start_pos.x + factx*(trace-1);

 posclp *= amp_sc_f;
 negclp *= amp_sc_f;

 cdCanvasBegin(intcgm_canvas,  CD_OPEN_LINES );

 for ( i=0; i<n_samp; i++ )
  {
   double y1 = trace_start_pos.y + facty*(i);
   double y2 = trace_start_pos.y + facty*(i+1);

   if ( sample[i]>negclp && sample[i]<posclp && 
        sample[i+1]>negclp && sample[i+1]<posclp )
    cdCanvasVertex (intcgm_canvas,  cgm_vdcx2canvas(sample[i] + dx), cgm_vdcy2canvas(y1) );
   else if ( sample[i]<negclp && sample[i+1]>negclp && sample[i+1]<posclp )
    cdCanvasVertex (intcgm_canvas,  cgm_vdcx2canvas(negclp + dx), cgm_vdcy2canvas(interpl(i,y1,y2,negclp)) );
   else if ( sample[i]<negclp && sample[i+1]>posclp )
    {
     cdCanvasVertex (intcgm_canvas,  cgm_vdcx2canvas(negclp + dx), cgm_vdcy2canvas(interpl(i,y1,y2,negclp)) );
     cdCanvasVertex (intcgm_canvas,  cgm_vdcx2canvas(posclp + dx), cgm_vdcy2canvas(interpl(i,y1,y2,posclp)) );
    }
   else if ( sample[i]>negclp && sample[i]<posclp && sample[i+1]>posclp )
    {
     cdCanvasVertex (intcgm_canvas,  cgm_vdcx2canvas(sample[i] + dx), cgm_vdcy2canvas(y1) );
     cdCanvasVertex (intcgm_canvas,  cgm_vdcx2canvas(posclp + dx), cgm_vdcy2canvas(interpl(i,y1,y2,posclp)) );
    }
   else if ( sample[i]>posclp && sample[i+1]<posclp && sample[i+1]>negclp )
    cdCanvasVertex (intcgm_canvas,  cgm_vdcx2canvas(posclp + dx), cgm_vdcy2canvas(interpl(i,y1,y2,posclp)) );
   else if ( sample[i]>posclp && sample[i+1]<negclp )
    {
     cdCanvasVertex (intcgm_canvas,  cgm_vdcx2canvas(posclp + dx), cgm_vdcy2canvas(interpl(i,y1,y2,posclp)) );
     cdCanvasVertex (intcgm_canvas,  cgm_vdcx2canvas(negclp + dx), cgm_vdcy2canvas(interpl(i,y1,y2,negclp)) );
    }
   else if ( sample[i]>negclp && sample[i]<posclp && sample[i+1]<negclp )
    {
     cdCanvasVertex (intcgm_canvas,  cgm_vdcx2canvas(sample[i] + dx), cgm_vdcy2canvas(y1) );
     cdCanvasVertex (intcgm_canvas,  cgm_vdcx2canvas(negclp + dx), cgm_vdcy2canvas(interpl(i,y1,y2,negclp)) );
    }
  }

 cdCanvasEnd(intcgm_canvas);
}

void cgm_sism5 ( char *data_rec )
{
 int i, mode, trace_mode;
 double pos_clp, neg_clp;

 sample = (double *) malloc ( n_samp*sizeof(double) );

 if ( trace_dsp_md > 7 )
  coind = (long *) malloc ( n_samp*sizeof(long) );
 
 for ( i=0; i<n_samp; i++ )
  if ( trace_dsp_md < 8 )
    sample[i] = strtod ( data_rec, &data_rec );
  else
   {
    sample[i] = (double) strtol ( data_rec, &data_rec, 10 );
    coind[i] = strtol ( data_rec, &data_rec, 10 );
   }

 trace += 1;

 dirvet ( amp_direction.x );

 ampscf ( amp_direction.x );

 trace_mode = trace_dsp_md;

 do
  {

   if ( trace_mode >= 64 )
    mode = 64;
   else if ( trace_mode >= 32 )
    mode = 32;
   else if ( trace_mode >= 16 )
    mode = 16;
   else if ( trace_mode >= 8 )
    mode = 8;
   else if ( trace_mode >= 4 )
    mode = 4;
   else if ( trace_mode >= 2 )
    mode = 2;
   else if ( trace_mode == 1 )
    mode = 1;

   switch ( mode )
    {
     case 64:
      neg_clp = neg_clp_lmt;
      pos_clp = ( pos_clp_lmt < 0 ) ?  pos_clp_lmt : 0;
      vasamp ( mode, pos_clp, neg_clp, 1 );
      trace_mode -= 64;
      break;
     case 32:
      neg_clp = ( neg_clp_lmt > 0 ) ? neg_clp_lmt : 0;
      pos_clp = pos_clp_lmt;
      vasamp ( mode, pos_clp, neg_clp, 1 );
      trace_mode -= 32;
      break;
     case 16:
      bgclfl( mode );
      trace_mode -= 16;
      break;
     case  8:
      bgclfl( mode );
      trace_mode -= 8;
      break;
     case  4:
      neg_clp = ( neg_clp_lmt > 0 ) ? neg_clp_lmt : 0;
      pos_clp = pos_clp_lmt;
      vasamp ( mode, pos_clp, neg_clp, 0 );
      trace_mode -= 4;
      break;
     case  2:
      neg_clp = ( neg_clp_lmt > 0 ) ? neg_clp_lmt : 0;
      pos_clp = pos_clp_lmt;
      vasamp ( mode, pos_clp, neg_clp, 0 );
      trace_mode -= 2;
      break;
     case  1:
      wiggle ( pos_clp_lmt, neg_clp_lmt );
      trace_mode -= 1;
      break;
    }
  } while ( trace_mode != 0 );

 free(sample);

 if ( trace_dsp_md > 7 )
  free(coind);
}
