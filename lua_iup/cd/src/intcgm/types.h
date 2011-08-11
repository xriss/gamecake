#ifndef _CGM_TYPES_H_
#define _CGM_TYPES_H_

typedef struct { 
                double xmin;
                double xmax;
                double ymin;
                double ymax;
               } tlimit;

typedef struct {
                unsigned long red;
                unsigned long green;
                unsigned long blue;
               } trgb;

typedef union {
               int ind;
               trgb rgb;
              } tcolor;

typedef struct {
                double x;
                double y;
               } tpoint;

typedef struct {
                char *dados;
                int size;
               } tdados;

typedef struct {
                FILE *fp;

		int (*cgmf)(void);

		int file_size;
		
		int first;

        int mode;      /* character, binary, clear text */

		int len;

		tdados buff;

                short vdc_type;

                union  {
                        long   b_prec;  /* 8, 16, 24, 32 */
		        struct { long minint; long maxint; } t_prec ;
                       } int_prec;
                union {
                       long   b_prec; /* float*32, float*64, fixed*32, fixed*64 */
		       struct { double minreal; double maxreal; long digits; } t_prec;
                      } real_prec;
                union {
                       long   b_prec;  /* 8, 16, 24, 32 */
		       struct { long minint; long maxint; } t_prec;
                      } ix_prec;
                long cd_prec;
                long cix_prec;
                struct {
                        trgb black;
                        trgb white;
                       } color_ext;
                long max_cix;

                struct {
                        short mode;
                        double scale_factor;
                       } scaling_mode;

                short drawing_mode;

                short clrsm;
                short lnwsm;
                short mkssm;
                short edwsm;

                union {
                       long   b_prec;  /* 8, 16, 24, 32 */
		       struct { long minint; long maxint; } t_prec;
                      } vdc_int;
                union {
                       long   b_prec; /* float*32, float*64, fixed*32, fixed*64 */
		       struct { double minreal; double maxreal; long digits; } t_prec;
                      } vdc_real;

                struct {
                        tpoint first;
                        tpoint second;
                       } vdc_ext;

                trgb back_color;

                tcolor aux_color;

                short transparency;

                struct {
                        tpoint first;
                        tpoint second;
                       } clip_rect;

                short clip_ind;

                int            bc;        /* byte count */
                int            pc;        /* pixel count */

                long           bl;        /* bytes lidos */

                int            cl;        /* coluna para alinhamento */

               } t_cgm;

typedef struct {
                long index;
                long type;
                double width;
                tcolor color;
               } _line_att;

typedef struct {
                long index;
                long type;
                double size;
                tcolor color;
               } _marker_att;

typedef struct {
                long index;
                long font_index;
				TList *font_list;
				int font;
				int style;
				int size;
                short prec;
                double exp_fact;
                double char_spacing;
                tcolor color;
                double height;
                tpoint char_up;
                tpoint char_base;
                short path;
                struct { short hor;
                         short ver;
                         double cont_hor;
                         double cont_ver;
                       } alignment;
               } _text_att;

typedef struct {
                long index;
                short int_style;
                tcolor color;
                long hatch_index;
                long pat_index;
                tpoint ref_pt;
                TList *pat_list;
                struct {
                        tpoint height;
                        tpoint width;
                       } pat_size;
               } _fill_att;

typedef struct {
                long index;
                long type;
                double width;
                tcolor color;
                short visibility;
               } _edge_att;

enum { OFF, ON };

enum { YES, NO };

enum { INTEGER, REAL };

enum { STRING, CHAR, STROKE };

enum { ABSOLUTE, SCALED };

enum { ABSTRACT, METRIC };

enum { INDEXED, DIRECT };

enum { MARK_DOT=1, MARK_PLUS=2, MARK_ASTERISK=3, MARK_CIRCLE=4, MARK_CROSS=5 };

enum { LINE_SOLID=1, LINE_DASH=2, LINE_DOT=3, LINE_DASH_DOT=4,
       LINE_DASH_DOT_DOT=5 };

enum { EDGE_SOLID=1, EDGE_DASH=2, EDGE_DOT=3, EDGE_DASH_DOT=4,
       EDGE_DASH_DOT_DOT=5 };

enum { HOLLOW, SOLID, PATTERN, HATCH, EMPTY };

enum { HORIZONTAL=1, VERTICAL=2, POSITIVE_SLOPE=3, NEGATIVE_SLOPE=4,
       HV_CROSS=5, SLOPE_CROSS=6 };

enum { PATH_RIGHT, PATH_LEFT, PATH_UP, PATH_DOWN };

enum { NORMHORIZ, LEFT, CTR, RIGHT, CONTHORIZ };

enum { NORMVERT, TOP, CAP, HALF, BASE, BOTTOM, CONTVERT };

enum { LINE_TYPE, LINE_WIDTH, LINE_COLOUR, MARKER_TYPE, MARKER_SIZE,
       MARKER_COLOUR, TEXT_FONT_INDEX, TEXT_PRECISION,
       CHARACTER_EXPANSION_FACTOR, CHARACTER_SPACING, TEXT_COLOUR,
       INTERIOR_STYLE, FILL_COLOUR, HATCH_INDEX, PATTERN_INDEX, EDGE_TYPE,
       EDGE_WIDTH, EDGE_COLOUR, ALL, ALL_LINE, ALL_MARKER, ALL_TEXT, ALL_FILL,
       ALL_EDGE };

enum { INDIVIDUAL, BUNDLED };

enum { INVISIBLE, VISIBLE, CLOSE_INVISIBLE, CLOSE_VISIBLE };

enum { PIE, CHORD };

enum { OPEN, CLOSED_PIE, CLOSED_CHORD };

enum { NORM_TEXT, RESTRICTED_TEXT };

#endif
