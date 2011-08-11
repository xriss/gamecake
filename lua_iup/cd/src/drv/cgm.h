/** \file
 * \brief CGM library
 * Extracted from GKS/PUC
 *
 * See Copyright Notice in cd.h
 */

#ifndef __CGM_H
#define __CGM_H

typedef struct _cgmFunc CGMFUNC;

typedef struct {
  FILE       *file;      /* file pointer */

  const CGMFUNC *func;   /* functions */

  int         mode;      /* character, binary, clear text */

  int         vdc_type,  /* integer, real */
              int_prec,  /* 8, 16, 24, 32 */
              real_prec, /* float*32, float*64, fixed*32, fixed*64 */
              ix_prec,   /* 8, 16, 24, 32 */
              cd_prec,   /* 8, 16, 24, 32 */
              cix_prec,  /* 8, 16, 24, 32 */
              max_cix;   /* maximum colour index */

  int         clrsm,     /* indexed, direct */
              lnwsm,     /* absolute, scaled */
              mkssm,     /* absolute, scaled */
              edwsm;     /* absolute, scaled */
  int         vdc_int,   /* X, 16, 24, 32 */
              vdc_real;  /* float*32, float*64, fixed*32, fixed*64 */

  int         vdc_size,  /* 2, 3, 4, 8 */
              int_size,  /* 1, 2, 3, 4 */
              real_size, /* 4, 8 */
              ix_size,   /* 1, 2, 3, 4 */
              cd_size,   /* 1, 2, 3, 4 */
              cix_size,  /* 1, 2, 3, 4 */
              clr_size,  /* 3 * cd_size , cix_size */
              lnw_size,  /* 2, 3, 4, 8 */
              mks_size,  /* 2, 3, 4, 8 */
              edw_size;  /* 2, 3, 4, 8 */

  int         cl;        /* coluna para alinhamento */

  int         op;        /* commands opened */
  int         bc[5];     /* bytes count for command */
  long        po[5];     /* position offset do arquivo */
} CGM;

CGM *cgm_begin_metafile		( char *, int, char * );
int cgm_end_metafile		( CGM * );
int cgm_begin_picture		( CGM *, const char * );
int cgm_begin_picture_body	( CGM * );
int cgm_end_picture		( CGM * );
int cgm_metafile_version	( CGM *, long );
int cgm_metafile_description	( CGM *, const char * );
int cgm_vdc_type		( CGM *, int );
int cgm_integer_precision	( CGM *, int );
int cgm_real_precision		( CGM *, int );
int cgm_index_precision		( CGM *, int );
int cgm_colour_precision	( CGM *, int );
int cgm_colour_index_precision	( CGM *, int );
int cgm_maximum_colour_index	( CGM *, unsigned long );
int cgm_colour_value_extent	( CGM *, const double *, const double * );
int cgm_metafile_element_list	( CGM *, int, const int *, const int * );
int cgm_begin_metafile_defaults	( CGM * );
int cgm_end_metafile_defaults	( CGM * );
int cgm_font_list		( CGM *, const char *l[] );
int cgm_scaling_mode		( CGM *, int, float );
int cgm_colour_selection_mode	( CGM *, int );
int cgm_line_width_specify_mode	( CGM *, int );
int cgm_marker_size_specify_mode( CGM *, int );
int cgm_edge_width_specify_mode	( CGM *, int );
int cgm_vdc_extent		( CGM *, double, double, double, double );
int cgm_backgound_colour	( CGM *, const double * );
int cgm_vdc_integer_precision	( CGM *, int );
int cgm_vdc_real_precision	( CGM *, int );
int cgm_auxiliary_colour	( CGM *, const void * );
int cgm_transparency		( CGM *, int );
int cgm_clip_rectangle		( CGM *, double, double, double, double );
int cgm_clip_indicator		( CGM *, int );
int cgm_polyline		( CGM *, int, const double * );
int cgm_polymarker		( CGM *, int, const double * );
int cgm_text			( CGM *, int, double, double, const char *, int);
int cgm_polygon			( CGM *, int, const double * );
int cgm_cell_array		( CGM *, const double *, long, long, int, const void * );
int cgm_rectangle		( CGM *, const double * );
int cgm_elliptical_arc		( CGM *, const double *, const double *, const double *, double, double, double, double );
int cgm_elliptical_arc_close    ( CGM *, const double *, const double *, const double *, double, double, double, double, int );
int cgm_line_bundle_index	( CGM *, long );
int cgm_line_type		( CGM *, long );
int cgm_line_width		( CGM *, double );
int cgm_line_colour		( CGM *, const void * );
int cgm_marker_bundle_index	( CGM *, long );
int cgm_marker_type		( CGM *, long );
int cgm_marker_size		( CGM *, double );
int cgm_marker_colour		( CGM *, const void * );
int cgm_text_bundle_index	( CGM *, long );
int cgm_text_font_index		( CGM *, long );
int cgm_text_precision		( CGM *, int );
int cgm_char_expansion_factor	( CGM *, double );
int cgm_char_spacing		( CGM *, double );
int cgm_text_colour		( CGM *, const void * );
int cgm_char_height		( CGM *, double );
int cgm_char_orientation	( CGM *, double, double, double, double );
int cgm_text_path		( CGM *, int );
int cgm_text_alignment		( CGM *, int, int, double, double );
int cgm_fill_bundle_index	( CGM *, long );
int cgm_interior_style		( CGM *, int );
int cgm_fill_colour		( CGM *, const void * );
int cgm_hatch_index		( CGM *, long );
int cgm_pattern_index		( CGM *, long );
int cgm_edge_width              ( CGM *, double );
int cgm_edge_colour             ( CGM *, const void * );
int cgm_edge_visibility		( CGM *, int );
int cgm_fill_reference_point	( CGM *, double, double );
int cgm_pattern_table		( CGM *, long, long, long, int, const void * );
int cgm_pattern_size		( CGM *, double, double, double, double );
int cgm_colour_table		( CGM *, long, long, const double * );
int cgm_asfs			( CGM *, int, const int *, const int * );
int cgm_message			( CGM *, int, const char * );

enum {
      LINE_SOLID=1,
      LINE_DASH=2,
      LINE_DOT=3,
      LINE_DASH_DOT=4,
      LINE_DASH_DOT_DOT=5
     };

enum {
      MARKER_DOT=1,
      MARKER_PLUS=2,
      MARKER_ASTERISK=3,
      MARKER_CIRCLE=4,
      MARKER_CROSS=5
     };

enum {
      HOLLOW,
      SOLID,
      PAT,
      HATCH,
      EMPTY
     };

enum {                          /* encoding */
      CD_CHARACTER,
      CD_BIN,
      CD_CLEAR_TEXT
     };

#endif
