typedef int (*CGM_FUNC) (void);

#ifdef _INTCGM1_C_

t_cgm intcgm_cgm;
cdCanvas* intcgm_canvas = NULL;

tlimit intcgm_vdc_ext;
double intcgm_scale_factor_x;
double intcgm_scale_factor_y;
double intcgm_scale_factor_mm_x;
double intcgm_scale_factor_mm_y;
double intcgm_scale_factor;
int intcgm_view_xmin, intcgm_view_ymin, intcgm_view_xmax, intcgm_view_ymax;
double intcgm_clip_xmin, intcgm_clip_ymin, intcgm_clip_xmax, intcgm_clip_ymax;

_line_att intcgm_line_att = { 1, LINE_SOLID, 1., {1} };

_marker_att intcgm_marker_att = { 3, MARK_ASTERISK, 1., {1} };

_text_att intcgm_text_att = { 1, 1, NULL, 0, CD_PLAIN, 8,  STRING, 1., 0., {1}, .1,
                       {0,1}, {1,0}, PATH_RIGHT, {NORMHORIZ,NORMVERT,0.,0.} };

_fill_att intcgm_fill_att = { 1, HOLLOW, {1}, 1, 1, {0,0}, NULL, {{0.,0.},{0.,0.}} };

_edge_att intcgm_edge_att = { 1, EDGE_SOLID, 1., {1}, OFF };

trgb *intcgm_color_table;
int intcgm_block;

TList *intcgm_asf_list;

tpoint *intcgm_point_list;
int intcgm_npoints;

CGM_FUNC intcgm_funcs[] = { NULL, &cgmb_rch, &cgmt_rch };

#else

extern t_cgm intcgm_cgm;
extern cdCanvas* intcgm_canvas;

extern tlimit intcgm_vdc_ext;
extern double intcgm_scale_factor_x;
extern double intcgm_scale_factor_y;
extern double intcgm_scale_factor_mm_x;
extern double intcgm_scale_factor_mm_y;
extern double intcgm_scale_factor;
extern int intcgm_view_xmin, intcgm_view_ymin, intcgm_view_xmax, intcgm_view_ymax;
extern double intcgm_clip_xmin, intcgm_clip_ymin, intcgm_clip_xmax, intcgm_clip_ymax;

extern _line_att intcgm_line_att;

extern _marker_att intcgm_marker_att;

extern _text_att intcgm_text_att;

extern _fill_att intcgm_fill_att;

extern _edge_att intcgm_edge_att;

extern trgb *intcgm_color_table;
extern int intcgm_block;

extern TList *intcgm_asf_list;

extern tpoint *intcgm_point_list;
extern int intcgm_npoints;

extern CGM_FUNC *intcgm_funcs;

#endif

typedef struct _tasf {
                     short type;
                     short value;
                    } tasf;

typedef struct _pat_table {
                           long index;
                           long nx, ny;
                           tcolor *pattern;
                          } pat_table;

