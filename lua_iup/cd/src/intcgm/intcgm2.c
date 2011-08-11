#include <stdio.h>      /* FILE, ftell, fseek, fputc, fopen, fclose, fputs, fprintf */
#include <string.h>     /* strlen */
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include "cd.h"
#include "cdcgm.h"
#include "list.h"
#include "types.h"
#include "bparse.h"
#include "tparse.h"
#include "intcgm.h"
#include "intcgm6.h"
#include "tparse.h"

typedef struct {
                 const char *nome;
				 int (*func) (void);
               } comando;

/**************************************************
***************************************************
**                                               **
**                  FUNCOES CGM                  **
**                                               **
***************************************************
**************************************************/

/************************************************
*                                               *
*            Dados para nao-binario             *
*                                               *
************************************************/

/* delimiter elements */

comando _cgmt_NULL             = { "", NULL };
comando _cgmt_BEGMF            = { "begmf", &cgmt_begmtf };
comando _cgmt_ENDMF            = { "endmf", &cgmt_endmtf };
comando _cgmt_BEG_PIC          = { "begpic", &cgmt_begpic };
comando _cgmt_BEG_PIC_BODY     = { "begpicbody", &cgmt_begpib };
comando _cgmt_END_PIC          = { "endpic", &cgmt_endpic };

/* metafile descriptor elements */

comando _cgmt_MF_VERSION       = { "mfversion", cgmt_mtfver };
comando _cgmt_MF_DESC          = { "mfdesc", cgmt_mtfdsc };
comando _cgmt_VDC_TYPE         = { "vdctype", cgmt_vdctyp };
comando _cgmt_INTEGER_PREC     = { "integerprec", cgmt_intpre };
comando _cgmt_REAL_PREC        = { "realprec", cgmt_realpr };
comando _cgmt_INDEX_PREC       = { "indexprec", cgmt_indpre };
comando _cgmt_COLR_PREC        = { "colrprec", cgmt_colpre };
comando _cgmt_COLR_INDEX_PREC  = { "colrindexprec", cgmt_colipr };
comando _cgmt_MAX_COLR_INDEX   = { "maxcolrindex", cgmt_maxcoi };
comando _cgmt_COLR_VALUE_EXT   = { "colrvalueext", cgmt_covaex };
comando _cgmt_MF_ELEM_LIST     = { "mfelemlist", cgmt_mtfell };
comando _cgmt_BEG_MF_DEFAULTS  = { "begmfdefaults", cgmt_bmtfdf };
comando _cgmt_END_MF_DEFAULTS  = { "endmfdefaults", cgmt_emtfdf };
comando _cgmt_FONT_LIST        = { "fontlist", cgmt_fntlst };
comando _cgmt_CHAR_SET_LIST    = { "charsetlist", cgmt_chslst };
comando _cgmt_CHAR_CODING      = { "charcoding", cgmt_chcdac };

/* picture descriptor elements */

comando _cgmt_SCALE_MODE       = { "scalemode", cgmt_sclmde };
comando _cgmt_COLR_MODE        = { "colrmode", cgmt_clslmd };
comando _cgmt_LINE_WIDTH_MODE  = { "linewidthmode", cgmt_lnwdmd };
comando _cgmt_MARKER_SIZE_MODE = { "markersizemode", cgmt_mkszmd };
comando _cgmt_EDGE_WIDTH_MODE  = { "edgewidthmode", cgmt_edwdmd };
comando _cgmt_VDC_EXTENT       = { "vdcext", cgmt_vdcext };
comando _cgmt_BACK_COLR        = { "backcolr", cgmt_bckcol };

/* control elements */

comando _cgmt_VDC_INTEGER_PREC = { "vdcintegerprec", cgmt_vdcipr };
comando _cgmt_VDC_REAL_PREC    = { "vdcrealprec", cgmt_vdcrpr };
comando _cgmt_AUX_COLR         = { "auxcolr", cgmt_auxcol };
comando _cgmt_TRANSPARENCY     = { "transparency", cgmt_transp };
comando _cgmt_CLIP_RECT        = { "cliprect", cgmt_clprec };
comando _cgmt_CLIP             = { "clip", cgmt_clpind };

/* primitive elements */

comando _cgmt_LINE             = { "line", cgmt_polyln };
comando _cgmt_INCR_LINE        = { "incrline", cgmt_incply };
comando _cgmt_DISJT_LINE       = { "disjtline", cgmt_djtply };
comando _cgmt_INCR_DISJT_LINE  = { "incrdisjt_line", cgmt_indjpl };
comando _cgmt_MARKER           = { "marker", cgmt_polymk };
comando _cgmt_INCR_MARKER      = { "incrmarker", cgmt_incplm };
comando _cgmt_TEXT             = { "text", cgmt_text };
comando _cgmt_RESTR_TEXT       = { "restrtext", cgmt_rsttxt };
comando _cgmt_APND_TEXT        = { "apndtext", cgmt_apdtxt };
comando _cgmt_POLYGON          = { "polygon", cgmt_polygn };
comando _cgmt_INCR_POLYGON     = { "incrpolygon", cgmt_incplg };
comando _cgmt_POLYGON_SET      = { "polygonset", cgmt_plgset };
comando _cgmt_INCR_POLYGON_SET = { "incrpolygonset", cgmt_inpgst };
comando _cgmt_CELL_ARRAY       = { "cellarray", cgmt_cellar };
comando _cgmt_GDP              = { "gdp", cgmt_gdp };
comando _cgmt_RECT             = { "rect", cgmt_rect };
comando _cgmt_CIRCLE           = { "circle", cgmt_circle };
comando _cgmt_ARC_3_PT         = { "arc3pt", cgmt_circ3p };
comando _cgmt_ARC_3_PT_CLOSE   = { "arc3ptclose", cgmt_cir3pc };
comando _cgmt_ARC_CTR          = { "arcctr", cgmt_circnt };
comando _cgmt_ARC_CTR_CLOSE    = { "arcctr_close", cgmt_ccntcl };
comando _cgmt_ELLIPSE          = { "ellipse", cgmt_ellips };
comando _cgmt_ELLIP_ARC        = { "elliparc", cgmt_ellarc };
comando _cgmt_ELLIP_ARC_CLOSE  = { "elliparcclose", cgmt_ellacl };

/* attribute elements */

comando _cgmt_LINE_INDEX       = { "lineindex", cgmt_lnbdin };
comando _cgmt_LINE_TYPE        = { "linetype", cgmt_lntype };
comando _cgmt_LINE_WIDTH       = { "linewidth", cgmt_lnwidt };
comando _cgmt_LINE_COLR        = { "linecolr", cgmt_lncolr };
comando _cgmt_MARKER_INDEX     = { "markerindex", cgmt_mkbdin };
comando _cgmt_MARKER_TYPE      = { "markertype", cgmt_mktype };
comando _cgmt_MARKER_WIDTH     = { "markersize", cgmt_mksize };
comando _cgmt_MARKER_COLR      = { "markercolr", cgmt_mkcolr };
comando _cgmt_TEXT_INDEX       = { "textindex", cgmt_txbdin };
comando _cgmt_TEXT_FONT_INDEX  = { "textfontindex", cgmt_txftin };
comando _cgmt_TEXT_PREC        = { "textprec", cgmt_txtprc };
comando _cgmt_CHAR_EXPAN       = { "charexpan", cgmt_chrexp };
comando _cgmt_CHAR_SPACE       = { "charspace", cgmt_chrspc };
comando _cgmt_TEXT_COLR        = { "textcolr", cgmt_txtclr };
comando _cgmt_CHAR_HEIGHT      = { "charheight", cgmt_chrhgt };
comando _cgmt_CHAR_ORI         = { "charori", cgmt_chrori };
comando _cgmt_TEXT_PATH        = { "textpath", cgmt_txtpat };
comando _cgmt_TEXT_ALIGN       = { "textalign", cgmt_txtali };
comando _cgmt_CHAR_SET_INDEX   = { "charsetindex", cgmt_chseti };
comando _cgmt_ALT_CHAR_SET     = { "altcharsetindex", cgmt_achsti };
comando _cgmt_FILL_INDEX       = { "fillindex", cgmt_fillin };
comando _cgmt_INT_STYLE        = { "intstyle", cgmt_intsty };
comando _cgmt_FILL_COLR        = { "fillcolr", cgmt_fillco };
comando _cgmt_HATCH_INDEX      = { "hatchindex", cgmt_hatind };
comando _cgmt_PAT_INDEX        = { "patindex", cgmt_patind };
comando _cgmt_EDGE_INDEX       = { "edgeindex", cgmt_edgind };
comando _cgmt_EDGE_TYPE        = { "edgetype", cgmt_edgtyp };
comando _cgmt_EDGE_WIDTH       = { "edgewidth", cgmt_edgwid };
comando _cgmt_EDGE_COLR        = { "edgecolr", cgmt_edgcol };
comando _cgmt_EDGE_VIS         = { "edgevis", cgmt_edgvis };
comando _cgmt_FILL_REF_PT      = { "fillrefpt", cgmt_fillrf };
comando _cgmt_PAT_TABLE        = { "pattable", cgmt_pattab };
comando _cgmt_PAT_SIZE         = { "patsize", cgmt_patsiz };
comando _cgmt_COLR_TABLE       = { "colrtable", cgmt_coltab };
comando _cgmt_ASF              = { "asf", cgmt_asf };

/* escape elements */

comando _cgmt_ESCAPE           = { "escape", cgmt_escape };
comando _cgmt_DOMAIN_RING      = { "domainring", NULL };

/* external elements */

comando _cgmt_MESSAGE          = { "message", cgmt_messag };
comando _cgmt_APPL_DATA        = { "appldata", cgmt_appdta };

comando *_cgmt_delimiter[] = {
      &_cgmt_NULL,
      &_cgmt_BEGMF,
      &_cgmt_ENDMF,
      &_cgmt_BEG_PIC,
      &_cgmt_BEG_PIC_BODY,
      &_cgmt_END_PIC,
      NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
      NULL, NULL, NULL, NULL, NULL, NULL, NULL };

comando *_cgmt_metafile[] = {
      &_cgmt_END_MF_DEFAULTS,
      &_cgmt_MF_VERSION,
      &_cgmt_MF_DESC,
      &_cgmt_VDC_TYPE,
      &_cgmt_INTEGER_PREC,
      &_cgmt_REAL_PREC,
      &_cgmt_INDEX_PREC,
      &_cgmt_COLR_PREC,
      &_cgmt_COLR_INDEX_PREC,
      &_cgmt_MAX_COLR_INDEX,
      &_cgmt_COLR_VALUE_EXT,
      &_cgmt_MF_ELEM_LIST,
      &_cgmt_BEG_MF_DEFAULTS,
      &_cgmt_FONT_LIST,
      &_cgmt_CHAR_SET_LIST,
      &_cgmt_CHAR_CODING,
      NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };

comando *_cgmt_picture[] = {
      &_cgmt_NULL,
      &_cgmt_SCALE_MODE,
      &_cgmt_COLR_MODE,
      &_cgmt_LINE_WIDTH_MODE,
      &_cgmt_MARKER_SIZE_MODE,
      &_cgmt_EDGE_WIDTH_MODE,
      &_cgmt_VDC_EXTENT,
      &_cgmt_BACK_COLR,
      NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
      NULL, NULL, NULL, NULL };

comando *_cgmt_control[] = {
      &_cgmt_NULL,
      &_cgmt_VDC_INTEGER_PREC,
      &_cgmt_VDC_REAL_PREC,
      &_cgmt_AUX_COLR,
      &_cgmt_TRANSPARENCY,
      &_cgmt_CLIP_RECT,
      &_cgmt_CLIP,
      NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
      NULL, NULL, NULL, NULL, NULL, NULL };

comando *_cgmt_primitive[] = {
      &_cgmt_NULL,
      &_cgmt_LINE,
      &_cgmt_INCR_LINE,
      &_cgmt_DISJT_LINE,
      &_cgmt_INCR_DISJT_LINE,
      &_cgmt_MARKER,
      &_cgmt_INCR_MARKER,
      &_cgmt_TEXT,
      &_cgmt_RESTR_TEXT,
      &_cgmt_APND_TEXT,
      &_cgmt_POLYGON,
      &_cgmt_INCR_POLYGON,
      &_cgmt_POLYGON_SET,
      &_cgmt_INCR_POLYGON_SET,
      &_cgmt_CELL_ARRAY,
      &_cgmt_GDP,
      &_cgmt_RECT,
      &_cgmt_CIRCLE,
      &_cgmt_ARC_3_PT,
      &_cgmt_ARC_3_PT_CLOSE,
      &_cgmt_ARC_CTR,
      &_cgmt_ARC_CTR_CLOSE,
      &_cgmt_ELLIPSE,
      &_cgmt_ELLIP_ARC,
      &_cgmt_ELLIP_ARC_CLOSE,
      NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };

comando *_cgmt_attributes[] = {
      &_cgmt_NULL,
      &_cgmt_LINE_INDEX,
      &_cgmt_LINE_TYPE,
      &_cgmt_LINE_WIDTH,
      &_cgmt_LINE_COLR,
      &_cgmt_MARKER_INDEX,
      &_cgmt_MARKER_TYPE,
      &_cgmt_MARKER_WIDTH,
      &_cgmt_MARKER_COLR,
      &_cgmt_TEXT_INDEX,
      &_cgmt_TEXT_FONT_INDEX,
      &_cgmt_TEXT_PREC,
      &_cgmt_CHAR_EXPAN,
      &_cgmt_CHAR_SPACE,
      &_cgmt_TEXT_COLR,
      &_cgmt_CHAR_HEIGHT,
      &_cgmt_CHAR_ORI,
      &_cgmt_TEXT_PATH,
      &_cgmt_TEXT_ALIGN,
      &_cgmt_CHAR_SET_INDEX,
      &_cgmt_ALT_CHAR_SET,
      &_cgmt_FILL_INDEX,
      &_cgmt_INT_STYLE,
      &_cgmt_FILL_COLR,
      &_cgmt_HATCH_INDEX,
      &_cgmt_PAT_INDEX,
      &_cgmt_EDGE_INDEX,
      &_cgmt_EDGE_TYPE,
      &_cgmt_EDGE_WIDTH,
      &_cgmt_EDGE_COLR,
      &_cgmt_EDGE_VIS,
      &_cgmt_FILL_REF_PT,
      &_cgmt_PAT_TABLE,
      &_cgmt_PAT_SIZE,
      &_cgmt_COLR_TABLE,
      &_cgmt_ASF,
      NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
      NULL, NULL, NULL, NULL };

comando *_cgmt_escape[] = {
      &_cgmt_NULL,
      &_cgmt_ESCAPE,
      &_cgmt_DOMAIN_RING,
      NULL};

comando *_cgmt_external[] = {
      &_cgmt_NULL,
      &_cgmt_MESSAGE,
      &_cgmt_APPL_DATA,
      NULL };

comando *_cgmt_segment[] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };

comando *_cgmt_NULL_NULL[] = {
      &_cgmt_NULL,
	  NULL };
		
comando **_cgmt_comandos[] = {
       _cgmt_NULL_NULL,
       _cgmt_delimiter,
       _cgmt_metafile,
       _cgmt_picture,
       _cgmt_control,
       _cgmt_primitive,
       _cgmt_attributes,
       _cgmt_escape,
       _cgmt_external,
       _cgmt_segment,
      NULL };

/************************************************
*                                               *
*            Dados para binario                 *
*                                               *
************************************************/

/* delimiter elements */

CGM_FUNC _cgmb_NULL            = NULL;

CGM_FUNC _cgmb_NOOP            = &cgmb_noop;
CGM_FUNC _cgmb_BEGMF           = &cgmb_begmtf;
CGM_FUNC _cgmb_ENDMF           = &cgmb_endmtf;
CGM_FUNC _cgmb_BEG_PIC         = &cgmb_begpic;
CGM_FUNC _cgmb_BEG_PIC_BODY    = &cgmb_begpib;
CGM_FUNC _cgmb_END_PIC         = &cgmb_endpic;

/* metafile descriptor elements */

CGM_FUNC _cgmb_MF_VERSION       = &cgmb_mtfver;
CGM_FUNC _cgmb_MF_DESC          = &cgmb_mtfdsc;
CGM_FUNC _cgmb_VDC_TYPE         = &cgmb_vdctyp;
CGM_FUNC _cgmb_INTEGER_PREC     = &cgmb_intpre;
CGM_FUNC _cgmb_REAL_PREC        = &cgmb_realpr;
CGM_FUNC _cgmb_INDEX_PREC       = &cgmb_indpre;
CGM_FUNC _cgmb_COLR_PREC        = &cgmb_colpre;
CGM_FUNC _cgmb_COLR_INDEX_PREC  = &cgmb_colipr;
CGM_FUNC _cgmb_MAX_COLR_INDEX   = &cgmb_maxcoi;
CGM_FUNC _cgmb_COLR_VALUE_EXT   = &cgmb_covaex;
CGM_FUNC _cgmb_MF_ELEM_LIST     = &cgmb_mtfell;
CGM_FUNC _cgmb_MF_DEFAULTS_RPL  = &cgmb_bmtfdf;
CGM_FUNC _cgmb_FONT_LIST        = &cgmb_fntlst;
CGM_FUNC _cgmb_CHAR_SET_LIST    = &cgmb_chslst;
CGM_FUNC _cgmb_CHAR_CODING      = &cgmb_chcdac;

/* picture descriptor elements */

CGM_FUNC _cgmb_SCALE_MODE       = &cgmb_sclmde;
CGM_FUNC _cgmb_COLR_MODE        = &cgmb_clslmd;
CGM_FUNC _cgmb_LINE_WIDTH_MODE  = &cgmb_lnwdmd;
CGM_FUNC _cgmb_MARKER_SIZE_MODE = &cgmb_mkszmd;
CGM_FUNC _cgmb_EDGE_WIDTH_MODE  = &cgmb_edwdmd;
CGM_FUNC _cgmb_VDC_EXTENT       = &cgmb_vdcext;
CGM_FUNC _cgmb_BACK_COLR        = &cgmb_bckcol;

/* control elements */

CGM_FUNC _cgmb_VDC_INTEGER_PREC = &cgmb_vdcipr;
CGM_FUNC _cgmb_VDC_REAL_PREC    = &cgmb_vdcrpr;
CGM_FUNC _cgmb_AUX_COLR         = &cgmb_auxcol;
CGM_FUNC _cgmb_TRANSPARENCY     = &cgmb_transp;
CGM_FUNC _cgmb_CLIP_RECT        = &cgmb_clprec;
CGM_FUNC _cgmb_CLIP             = &cgmb_clpind;

/* primitive elements */

CGM_FUNC _cgmb_LINE             = &cgmb_polyln;
CGM_FUNC _cgmb_DISJT_LINE       = &cgmb_djtply;
CGM_FUNC _cgmb_MARKER           = &cgmb_polymk;
CGM_FUNC _cgmb_TEXT             = &cgmb_text;
CGM_FUNC _cgmb_RESTR_TEXT       = &cgmb_rsttxt;
CGM_FUNC _cgmb_APND_TEXT        = &cgmb_apdtxt;
CGM_FUNC _cgmb_POLYGON          = &cgmb_polygn;
CGM_FUNC _cgmb_POLYGON_SET      = &cgmb_plgset;
CGM_FUNC _cgmb_CELL_ARRAY       = &cgmb_cellar;
CGM_FUNC _cgmb_GDP              = &cgmb_gdp;
CGM_FUNC _cgmb_RECT             = &cgmb_rect;
CGM_FUNC _cgmb_CIRCLE           = &cgmb_circle;
CGM_FUNC _cgmb_ARC_3_PT         = &cgmb_circ3p;
CGM_FUNC _cgmb_ARC_3_PT_CLOSE   = &cgmb_cir3pc;
CGM_FUNC _cgmb_ARC_CTR          = &cgmb_circnt;
CGM_FUNC _cgmb_ARC_CTR_CLOSE    = &cgmb_ccntcl;
CGM_FUNC _cgmb_ELLIPSE          = &cgmb_ellips;
CGM_FUNC _cgmb_ELLIP_ARC        = &cgmb_ellarc;
CGM_FUNC _cgmb_ELLIP_ARC_CLOSE  = &cgmb_ellacl;

/* attribute elements */

CGM_FUNC _cgmb_LINE_INDEX       = &cgmb_lnbdin;
CGM_FUNC _cgmb_LINE_TYPE        = &cgmb_lntype;
CGM_FUNC _cgmb_LINE_WIDTH       = &cgmb_lnwidt;
CGM_FUNC _cgmb_LINE_COLR        = &cgmb_lncolr;
CGM_FUNC _cgmb_MARKER_INDEX     = &cgmb_mkbdin;
CGM_FUNC _cgmb_MARKER_TYPE      = &cgmb_mktype;
CGM_FUNC _cgmb_MARKER_WIDTH     = &cgmb_mksize;
CGM_FUNC _cgmb_MARKER_COLR      = &cgmb_mkcolr;
CGM_FUNC _cgmb_TEXT_INDEX       = &cgmb_txbdin;
CGM_FUNC _cgmb_TEXT_FONT_INDEX  = &cgmb_txftin;
CGM_FUNC _cgmb_TEXT_PREC        = &cgmb_txtprc;
CGM_FUNC _cgmb_CHAR_EXPAN       = &cgmb_chrexp;
CGM_FUNC _cgmb_CHAR_SPACE       = &cgmb_chrspc;
CGM_FUNC _cgmb_TEXT_COLR        = &cgmb_txtclr;
CGM_FUNC _cgmb_CHAR_HEIGHT      = &cgmb_chrhgt;
CGM_FUNC _cgmb_CHAR_ORI         = &cgmb_chrori;
CGM_FUNC _cgmb_TEXT_PATH        = &cgmb_txtpat;
CGM_FUNC _cgmb_TEXT_ALIGN       = &cgmb_txtali;
CGM_FUNC _cgmb_CHAR_SET_INDEX   = &cgmb_chseti;
CGM_FUNC _cgmb_ALT_CHAR_SET     = &cgmb_achsti;
CGM_FUNC _cgmb_FILL_INDEX       = &cgmb_fillin;
CGM_FUNC _cgmb_INT_STYLE        = &cgmb_intsty;
CGM_FUNC _cgmb_FILL_COLR        = &cgmb_fillco;
CGM_FUNC _cgmb_HATCH_INDEX      = &cgmb_hatind;
CGM_FUNC _cgmb_PAT_INDEX        = &cgmb_patind;
CGM_FUNC _cgmb_EDGE_INDEX       = &cgmb_edgind;
CGM_FUNC _cgmb_EDGE_TYPE        = &cgmb_edgtyp;
CGM_FUNC _cgmb_EDGE_WIDTH       = &cgmb_edgwid;
CGM_FUNC _cgmb_EDGE_COLR        = &cgmb_edgcol;
CGM_FUNC _cgmb_EDGE_VIS         = &cgmb_edgvis;
CGM_FUNC _cgmb_FILL_REF_PT      = &cgmb_fillrf;
CGM_FUNC _cgmb_PAT_TABLE        = &cgmb_pattab;
CGM_FUNC _cgmb_PAT_SIZE         = &cgmb_patsiz;
CGM_FUNC _cgmb_COLR_TABLE       = &cgmb_coltab;
CGM_FUNC _cgmb_ASF              = &cgmb_asf;

/* escape elements */

CGM_FUNC _cgmb_ESCAPE           = &cgmb_escape;

/* external elements */

CGM_FUNC _cgmb_MESSAGE          = &cgmb_messag;
CGM_FUNC _cgmb_APPL_DATA        = &cgmb_appdta;

CGM_FUNC *_cgmb_delimiter[] = {
      &_cgmb_NOOP,
      &_cgmb_BEGMF,
      &_cgmb_ENDMF,
      &_cgmb_BEG_PIC,
      &_cgmb_BEG_PIC_BODY,
      &_cgmb_END_PIC,
      NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
      NULL, NULL, NULL, NULL, NULL, NULL, NULL };

CGM_FUNC *_cgmb_metafile[] = {
      &_cgmb_NULL,
      &_cgmb_MF_VERSION,
      &_cgmb_MF_DESC,
      &_cgmb_VDC_TYPE,
      &_cgmb_INTEGER_PREC,
      &_cgmb_REAL_PREC,
      &_cgmb_INDEX_PREC,
      &_cgmb_COLR_PREC,
      &_cgmb_COLR_INDEX_PREC,
      &_cgmb_MAX_COLR_INDEX,
      &_cgmb_COLR_VALUE_EXT,
      &_cgmb_MF_ELEM_LIST,
      &_cgmb_MF_DEFAULTS_RPL,
      &_cgmb_FONT_LIST,
      &_cgmb_CHAR_SET_LIST,
      &_cgmb_CHAR_CODING,
      NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };

CGM_FUNC *_cgmb_picture[] = {
      &_cgmb_NULL,
      &_cgmb_SCALE_MODE,
      &_cgmb_COLR_MODE,
      &_cgmb_LINE_WIDTH_MODE,
      &_cgmb_MARKER_SIZE_MODE,
      &_cgmb_EDGE_WIDTH_MODE,
      &_cgmb_VDC_EXTENT,
      &_cgmb_BACK_COLR,
      NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
      NULL, NULL, NULL, NULL };

CGM_FUNC *_cgmb_control[] = {
      &_cgmb_NULL,
      &_cgmb_VDC_INTEGER_PREC,
      &_cgmb_VDC_REAL_PREC,
      &_cgmb_AUX_COLR,
      &_cgmb_TRANSPARENCY,
      &_cgmb_CLIP_RECT,
      &_cgmb_CLIP,
      NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
      NULL, NULL, NULL, NULL, NULL, NULL };

CGM_FUNC *_cgmb_primitive[] = {
      &_cgmb_NULL,
      &_cgmb_LINE,
      &_cgmb_DISJT_LINE,
      &_cgmb_MARKER,
      &_cgmb_TEXT,
      &_cgmb_RESTR_TEXT,
      &_cgmb_APND_TEXT,
      &_cgmb_POLYGON,
      &_cgmb_POLYGON_SET,
      &_cgmb_CELL_ARRAY,
      &_cgmb_GDP,
      &_cgmb_RECT,
      &_cgmb_CIRCLE,
      &_cgmb_ARC_3_PT,
      &_cgmb_ARC_3_PT_CLOSE,
      &_cgmb_ARC_CTR,
      &_cgmb_ARC_CTR_CLOSE,
      &_cgmb_ELLIPSE,
      &_cgmb_ELLIP_ARC,
      &_cgmb_ELLIP_ARC_CLOSE,
      NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };

CGM_FUNC *_cgmb_attributes[] = {
      &_cgmb_NULL,
      &_cgmb_LINE_INDEX,
      &_cgmb_LINE_TYPE,
      &_cgmb_LINE_WIDTH,
      &_cgmb_LINE_COLR,
      &_cgmb_MARKER_INDEX,
      &_cgmb_MARKER_TYPE,
      &_cgmb_MARKER_WIDTH,
      &_cgmb_MARKER_COLR,
      &_cgmb_TEXT_INDEX,
      &_cgmb_TEXT_FONT_INDEX,
      &_cgmb_TEXT_PREC,
      &_cgmb_CHAR_EXPAN,
      &_cgmb_CHAR_SPACE,
      &_cgmb_TEXT_COLR,
      &_cgmb_CHAR_HEIGHT,
      &_cgmb_CHAR_ORI,
      &_cgmb_TEXT_PATH,
      &_cgmb_TEXT_ALIGN,
      &_cgmb_CHAR_SET_INDEX,
      &_cgmb_ALT_CHAR_SET,
      &_cgmb_FILL_INDEX,
      &_cgmb_INT_STYLE,
      &_cgmb_FILL_COLR,
      &_cgmb_HATCH_INDEX,
      &_cgmb_PAT_INDEX,
      &_cgmb_EDGE_INDEX,
      &_cgmb_EDGE_TYPE,
      &_cgmb_EDGE_WIDTH,
      &_cgmb_EDGE_COLR,
      &_cgmb_EDGE_VIS,
      &_cgmb_FILL_REF_PT,
      &_cgmb_PAT_TABLE,
      &_cgmb_PAT_SIZE,
      &_cgmb_COLR_TABLE,
      &_cgmb_ASF,
      NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
      NULL, NULL, NULL, NULL };

CGM_FUNC *_cgmb_escape[] = {
      &_cgmb_NULL,
      &_cgmb_ESCAPE,
      NULL};

CGM_FUNC *_cgmb_external[] = {
      &_cgmb_NULL,
      &_cgmb_MESSAGE,
      &_cgmb_APPL_DATA,
      NULL };

CGM_FUNC *_cgmb_segment[] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };

CGM_FUNC **_cgmb_comandos[] = {
       _cgmb_delimiter,
       _cgmb_metafile,
       _cgmb_picture,
       _cgmb_control,
       _cgmb_primitive,
       _cgmb_attributes,
       _cgmb_escape,
       _cgmb_external,
       _cgmb_segment,
       NULL };

/************************************************
*                                               *
*             Funcoes para binario              *
*                                               *
************************************************/

int cgm_getfilepos ( void )
{
 if (cdcgmcountercb)
  return cdcgmcountercb ( intcgm_canvas, (ftell ( intcgm_cgm.fp )*100.)/intcgm_cgm.file_size );
  
 return 0; 
}

#define cgmb_getw cgmb_getu16
#define cgmb_getb(b) cgmb_getc((unsigned char *)(b))

static void cgmb_donothing ( void )
{
 intcgm_cgm.bc=intcgm_cgm.len;
}

int cgmb_exec_comand ( int classe, int id )
{
 int err;

 if ( _cgmb_comandos[classe][id]==NULL )
  {
   cgmb_donothing();
   return 0;
  }

 err = (*_cgmb_comandos[classe][id])();

 if ( err == -1 )
  return 1;
 else if ( err )
  {
   cgmb_donothing();
   return 0;
  }

 return 0;
}

int cgmb_getbit ( unsigned char *b )
{
 static unsigned char b1;

 if ( intcgm_cgm.pc==0 || intcgm_cgm.pc==8 )
  {
   b1 = intcgm_cgm.buff.dados[intcgm_cgm.bc];
   intcgm_cgm.bc++;

   if ( intcgm_cgm.bc > intcgm_cgm.len )
   {
    return 1;
   }

   intcgm_cgm.pc=0;
  }

 *b = b1;

 switch ( intcgm_cgm.pc )
  {
   case 0:
    *b = ( *b | 0x0080 ) >> 7;
    break;
   case 1:
    *b = ( *b | 0x0040 ) >> 6;
    break;
   case 2:
    *b = ( *b | 0x0020 ) >> 5;
    break;
   case 3:
    *b = ( *b | 0x0010 ) >> 4;
    break;
   case 4:
    *b = ( *b | 0x0008 ) >> 3;
    break;
   case 5:
    *b = ( *b | 0x0004 ) >> 2;
    break;
   case 6:
    *b = ( *b | 0x0002 ) >> 1;
    break;
   case 7:
    *b = ( *b | 0x0001 );
    break;
  }

 intcgm_cgm.pc++;

 return 0;
}

int cgmb_get2bit ( unsigned char *b )
{
 static unsigned char b1;

 if ( intcgm_cgm.pc==0 || intcgm_cgm.pc==8 )
  {
   b1 = intcgm_cgm.buff.dados[intcgm_cgm.bc];
   intcgm_cgm.bc++;

   if ( intcgm_cgm.bc > intcgm_cgm.len ) return 1;

   intcgm_cgm.pc=0;
  }

 *b = b1;

 switch ( intcgm_cgm.pc )
  {
   case 0:
    *b = ( *b | 0x00C0 ) >> 6;
    break;
   case 2:
    *b = ( *b | 0x0030 ) >> 4;
    break;
   case 4:
    *b = ( *b | 0x000C ) >> 2;
    break;
   case 6:
    *b = ( *b | 0x0003 );
    break;
  }

 intcgm_cgm.pc += 2;

 return 0;
}


int cgmb_get4bit ( unsigned char *b )
{
 static unsigned char b1;

 if ( intcgm_cgm.pc==0 || intcgm_cgm.pc==8 )
  {
   b1 = intcgm_cgm.buff.dados[intcgm_cgm.bc];
   intcgm_cgm.bc++;

   if ( intcgm_cgm.bc > intcgm_cgm.len ) return 1;

   intcgm_cgm.pc=0;
  }

 *b = b1;

 switch ( intcgm_cgm.pc )
  {
   case 0:
    *b = ( *b | 0x00F0 ) >> 4;
    break;
   case 4:
    *b = ( *b | 0x000F );
    break;
  }

 intcgm_cgm.pc += 4;

 return 0;
}

int cgmb_getw ( unsigned short * );

int cgmb_getc ( unsigned char *b )
{
 *b = intcgm_cgm.buff.dados[intcgm_cgm.bc];
 intcgm_cgm.bc++;

 if ( intcgm_cgm.bc > intcgm_cgm.len ) return 1;
 
 return 0;   
}

int cgmb_geti8  ( signed char *b )
{
 unsigned char b1;

 if ( cgmb_getb ( &b1 ) ) return 1;

 *b = (signed char) b1;
 
 return 0;
}

int cgmb_geti16 ( short *b )
{
 unsigned char b1, b2;

 if ( cgmb_getb ( &b1 ) ) return 1;
 if ( cgmb_getb ( &b2 ) ) return 1;

 *b = ( b1<<8 ) | b2;

 return 0;
}

int cgmb_geti24 ( long *b )
{
 unsigned char b1, b2, b3;

 if ( cgmb_getb ( &b1 ) ) return 1;
 if ( cgmb_getb ( &b2 ) ) return 1;
 if ( cgmb_getb ( &b3 ) ) return 1;

 *b = ( b1<<16 ) | ( b2<<8 ) | b3;

 return 0;
}

int cgmb_geti32 ( long *b )
{
 unsigned char b1, b2, b3, b4;

 if ( cgmb_getb ( &b1 ) ) return 1;
 if ( cgmb_getb ( &b2 ) ) return 1;
 if ( cgmb_getb ( &b3 ) ) return 1;
 if ( cgmb_getb ( &b4 ) ) return 1;

 *b = ( b1<<24 ) | ( b2<<16 ) | ( b3<<8 ) | b4;

 return 0;
}

int cgmb_getu8  ( unsigned char *b )
{
 if ( cgmb_getb ( b ) ) return 1;

 return 0;
}

int cgmb_getu16 ( unsigned short *b )
{
 unsigned char b1, b2;

 if ( cgmb_getb ( &b1 ) ) return 1;
 if ( cgmb_getb ( &b2 ) ) return 1;

 *b = ( b1<<8 ) | b2;

 return 0;
}

int cgmb_getu24 ( unsigned long *b )
{
 unsigned char b1, b2, b3;

 if ( cgmb_getb ( &b1 ) ) return 1;
 if ( cgmb_getb ( &b2 ) ) return 1;
 if ( cgmb_getb ( &b3 ) ) return 1;

 *b = ( b1<<16 ) | ( b2<<8 ) | b3;

 return 0;
}

int cgmb_getu32 ( unsigned long *b )
{
 unsigned char b1, b2, b3, b4;

 if ( cgmb_getb ( &b1 ) ) return 1;
 if ( cgmb_getb ( &b2 ) ) return 1;
 if ( cgmb_getb ( &b3 ) ) return 1;
 if ( cgmb_getb ( &b4 ) ) return 1;

 *b = ( b1<<24 ) | ( b2<<16 ) | ( b3<<8 ) | b4;

 return 0;
}

int cgmb_getfl32 ( float *b )
{
 unsigned char b1, b2, b3, b4;
 union {
        float f;
        long  l;
       } r;

 if ( cgmb_getb ( &b1 ) ) return 1;
 if ( cgmb_getb ( &b2 ) ) return 1;
 if ( cgmb_getb ( &b3 ) ) return 1;
 if ( cgmb_getb ( &b4 ) ) return 1;

 r.l = ( b1<<24 ) | ( b2<<16 ) | ( b3<<8 ) | b4;
 *b = r.f;

 return 0;
}

int cgmb_getfl64 ( double *b )
{
 unsigned char b1, b2, b3, b4, b5, b6, b7, b8;
 union {
        double d;
        long   l[2];
       } r;

 if ( cgmb_getb ( &b1 ) ) return 1;
 if ( cgmb_getb ( &b2 ) ) return 1;
 if ( cgmb_getb ( &b3 ) ) return 1;
 if ( cgmb_getb ( &b4 ) ) return 1;
 if ( cgmb_getb ( &b5 ) ) return 1;
 if ( cgmb_getb ( &b6 ) ) return 1;
 if ( cgmb_getb ( &b7 ) ) return 1;
 if ( cgmb_getb ( &b8 ) ) return 1;

 r.l[1] = ( b1<<24 ) | ( b2<<16 ) | ( b3<<8 ) | b4;
 r.l[0] = ( b5<<24 ) | ( b6<<16 ) | ( b7<<8 ) | b8;
 *b = r.d;

 return 0;
}

int cgmb_getfx32 ( float *b )
{
 short si;
 unsigned short ui;

 if ( cgmb_geti16 ( &si ) ) return 1;
 if ( cgmb_getu16 ( &ui ) ) return 1;

 *b = (float) ( si + ( ui / 65536.0 ) );

 return 0;
}

int cgmb_getfx64 ( double *b )
{
 long si, ui;

 if ( cgmb_geti32 ( &si ) ) return 1;
 if ( cgmb_geti32 ( &ui ) ) return 1;

 *b = si + ( (unsigned short) ui / ( 65536.0 * 65536.0 ) );

 return 0;
}

int cgmb_ter ( void )
{
 return 0;
}

int cgmb_rch ( void )
{
 int c, id, len, cont, i; 
 unsigned char ch[2], dummy;
 unsigned short b;
 int erro;

 if ( intcgm_cgm.bc!=intcgm_cgm.len )
  {
   return 1;
  }

 intcgm_cgm.bc = 0;

 erro = fread ( ch, 1, 2, intcgm_cgm.fp );
 if ( erro<2 ) return 1;

 intcgm_cgm.bl += 2;

 b = (ch[0] << 8) + ch[1];

 len = b & 0x001F;

 id = ( b & 0x0FE0 ) >> 5;

 c = ( b & 0xF000 ) >> 12;

 cont = 0;

 if ( len > 30 )
  {
   erro = fread ( ch, 1, 2, intcgm_cgm.fp );
   if ( erro<2 ) return 1;

   intcgm_cgm.bl += 2;

   b = (ch[0] << 8) + ch[1];

   len = b & 0x7FFF;
   cont = ( b & 0x8000 );
  }

 intcgm_cgm.len = len;

 if ( intcgm_cgm.len )
  {
   if ( intcgm_cgm.len>intcgm_cgm.buff.size )
     intcgm_cgm.buff.dados = (char *) realloc ( intcgm_cgm.buff.dados, sizeof(char) * intcgm_cgm.len );

   erro = fread ( intcgm_cgm.buff.dados, 1, intcgm_cgm.len, intcgm_cgm.fp );
   if ( erro<intcgm_cgm.len ) return 1;

   intcgm_cgm.bl += intcgm_cgm.len;

   if ( len & 1 )
    {
     erro = fread ( &dummy, 1, 1, intcgm_cgm.fp );
     if ( erro<1 ) return 1;

     intcgm_cgm.bl += 1;
    }

   while ( cont )
    {
     unsigned char ch[2];
     unsigned short b;
     int old_len = intcgm_cgm.len;

     erro = fread ( ch, 1, 2, intcgm_cgm.fp );
     if ( erro<2 ) return 1;

     intcgm_cgm.bl += 2;

     b = (ch[0] << 8) + ch[1];

     cont = ( b & 0x8000 );

     len = b & 0x7fff;

     intcgm_cgm.len += len;

     if ( intcgm_cgm.len>intcgm_cgm.buff.size )
       intcgm_cgm.buff.dados = (char *) realloc ( (char *)intcgm_cgm.buff.dados, sizeof(char) * intcgm_cgm.len );

     erro = fread ( &intcgm_cgm.buff.dados[old_len], 1, len, intcgm_cgm.fp );
     if ( erro<len ) return 1;

     if ( len & 1 )
      {
       erro = fread ( &dummy, 1, 1, intcgm_cgm.fp );
       if ( erro<1 ) return 1;
       intcgm_cgm.bl += 1;
      }
    }
  }

 cgm_getfilepos ();

 if ( cgmb_exec_comand ( c, id ) ) return 1;

 for ( i=0; i<intcgm_cgm.len-intcgm_cgm.bc; i++ )
  {
   unsigned char dummy;
   if ( cgmb_getb ( &dummy ) ) return 1;
  }

 return 0;
}

int cgmb_ci ( unsigned long *ci )
{
 unsigned char c;
 unsigned short i;

 switch ( intcgm_cgm.cix_prec )
  {
   case 0: if ( cgmb_getu8  ( &c ) ) return 1; 
           *ci = (unsigned long) c;
           break;
   case 1: if ( cgmb_getu16 ( &i ) ) return 1;
           *ci = (unsigned long) i;
           break;
   case 2: if ( cgmb_getu24 ( ci ) ) return 1;
           break;
   case 3: if ( cgmb_getu32 ( ci ) ) return 1;
           break;
  }

 return 0;
}

int cgmb_cd ( unsigned long *cd )
{
 unsigned char c;
 unsigned short i;

 switch ( intcgm_cgm.cd_prec )
  {
   case 0: if ( cgmb_getu8  ( &c ) ) return 1;
           *cd = (unsigned long) c;
           break;
   case 1: if ( cgmb_getu16 ( &i ) ) return 1;
           *cd = (unsigned long) i;
           break;
   case 2: if ( cgmb_getu24 ( cd ) ) return 1;
           break;
   case 3: if ( cgmb_getu32 ( cd ) ) return 1;
           break;
  }

 return 0;
}

int cgmb_rgb ( unsigned long *r, unsigned long *g, unsigned long *b )
{
 if ( cgmb_cd ( r ) ) return 1;
 if ( cgmb_cd ( g ) ) return 1;
 if ( cgmb_cd ( b ) ) return 1;

 return 0;
}

int cgmb_ix ( long *ix )
{
 signed char c;
 short i;

 switch ( intcgm_cgm.ix_prec.b_prec )
  {
   case 0: if ( cgmb_geti8  ( &c ) ) return 1;
           *ix = (long) c;
           break;
   case 1: if ( cgmb_geti16 ( &i ) ) return 1;
           *ix = (long) i;
           break;
   case 2: if ( cgmb_geti24 ( ix ) ) return 1;
           break;
   case 3: if ( cgmb_geti32 ( ix ) ) return 1;
           break;
  }

 return 0;
}

int cgmb_e ( short *e )
{
 return cgmb_geti16 ( e );
}

int cgmb_i ( long *li )
{
 signed char c;
 short i;

 switch ( intcgm_cgm.int_prec.b_prec )
  {
   case 0: if ( cgmb_geti8  ( &c ) ) return 1;
           *li = (long) c;
           break;
   case 1: if ( cgmb_geti16 ( &i ) ) return 1;
           *li = (long) i;
           break;
   case 2: if ( cgmb_geti24 ( li ) ) return 1;
           break;
   case 3: if ( cgmb_geti32 ( li ) ) return 1;
           break;
  }

 return 0;
}

int cgmb_u ( unsigned long *ui )
{
 unsigned char c;
 unsigned short i;

 switch ( intcgm_cgm.int_prec.b_prec )
  {
   case 0: if ( cgmb_getu8  ( &c ) ) return 1;
           *ui = (unsigned long) c;
           break;
   case 1: if ( cgmb_getu16 ( &i ) ) return 1;
           *ui = (unsigned long) i;
           break;
   case 2: if ( cgmb_getu24 ( ui ) ) return 1;
           break;
   case 3: if ( cgmb_getu32 ( ui ) ) return 1;
           break;
  }

 return 0;
}

int cgmb_r ( double *d )
{
 float f;

 switch ( intcgm_cgm.real_prec.b_prec )
  {
   case 0: if ( cgmb_getfl32 ( &f ) ) return 1;
           *d = (double) f;
           break;
   case 1: if ( cgmb_getfl64 ( d ) ) return 1;
           break;
   case 2: if ( cgmb_getfx32 ( &f ) ) return 1;
           *d = (double) f;
           break;
   case 3: if ( cgmb_getfx64 ( d ) ) return 1;
           break;
  }

 return 0;
}

int cgmb_s ( char **str )
{
 register unsigned i = 0;
 unsigned char l;
 unsigned short l1;
 unsigned short cont;
 char *s = NULL;

 cont = 1;

 if ( cgmb_getu8 ( &l ) ) return 1;

 l1 = l;

 while ( cont )
  {
   if ( l > 254 )
    {
     if ( cgmb_getu16 ( &l1 ) ) return 1;
     cont = ( l1 & 0x8000);
     l1 &= 0x7fff;
    }
   else
    cont = 0;

   s = (char *)realloc ( (unsigned char *)s, (sizeof ( char ) * l1) + 1 );
 
   for ( i=0; i<l1; i++ )
    {
     unsigned char k;
     if ( cgmb_getb ( &k ) ) return 1;
     s[i] = (char) k;
    }

  }
 s[i] = '\0';

 *str = s;

 return 0;
}

int cgmb_vdc ( double *vdc )
{
 signed char c;
 short i;
 long l;
 float f;

 if ( intcgm_cgm.vdc_type == 0 )
  switch ( intcgm_cgm.vdc_int.b_prec )
   {
    case 0: if ( cgmb_geti8  ( &c ) ) return 1;
            *vdc = (double) c;
            break;
    case 1: if ( cgmb_geti16 (  &i ) ) return 1;
            *vdc = (double) i;
            break;
    case 2: if ( cgmb_geti24 ( &l ) ) return 1;
            *vdc = (double) l;
            break;
    case 3: if ( cgmb_geti32 ( &l ) ) return 1;
            *vdc = (double) l;
            break;
   }
 else
   switch ( intcgm_cgm.vdc_real.b_prec )
    {
     case 0: if ( cgmb_getfl32 ( &f ) ) return 1;
             *vdc = (double) f;
             break;
     case 1: if ( cgmb_getfl64 ( vdc ) ) return 1;
             break;
     case 2: if ( cgmb_getfx32 ( &f ) ) return 1;
             *vdc = (double) f;
             break;
     case 3: if ( cgmb_getfx64 ( vdc ) ) return 1;
             break;
    }

 return 0;
}

int cgmb_p ( double *x, double *y )
{
 if ( cgmb_vdc ( x ) ) return 1;
 if ( cgmb_vdc ( y ) ) return 1;
 
 return 0;
}

int cgmb_co ( void *co )
{
 if ( intcgm_cgm.clrsm == 0 ) /* indexed */
  {
   unsigned long *ci = (unsigned long *) co;
   if ( cgmb_ci ( ci ) ) return 1;
  }
 else
  {
   unsigned long *rgb = (unsigned long *) co;
   if ( cgmb_rgb ( &rgb[0], &rgb[1], &rgb[2] ) ) return 1;
  }

 return 0;
}

int cgmb_pixeli ( unsigned long *ci, int localp )
{
 unsigned char c;
 unsigned short i;

 if ( localp==0 )
  {
   if ( intcgm_cgm.cix_prec==0 ) localp = 8;
   else if ( intcgm_cgm.cix_prec==1 ) localp = 16;
   else if ( intcgm_cgm.cix_prec==2 ) localp = 24;
   else if ( intcgm_cgm.cix_prec==3 ) localp = 32;
  }

 switch ( localp )
  {
   case 1: if ( cgmb_getbit ( &c ) ) return 1;
           *ci = (unsigned long) c;
           break;
   case 2: if ( cgmb_get2bit ( &c ) )
           *ci = (unsigned long) c;
           return 1;
           break;
   case 4: if ( cgmb_get4bit ( &c ) )
           *ci = (unsigned long) c;
           return 1;
           break;
   case 8: if ( cgmb_getu8  ( &c ) ) return 1; 
           *ci = (unsigned long) c;
           break;
   case 16: if ( cgmb_getu16 ( &i ) ) return 1;
           *ci = (unsigned long) i;
           break;
   case 24: if ( cgmb_getu24 ( ci ) ) return 1;
           break;
   case 32: if ( cgmb_getu32 ( ci ) ) return 1;
           break;
  }

 return 0;
}

int cgmb_pixeld ( unsigned long *cd, int localp )
{
 unsigned char c;
 unsigned short i;

 if ( localp==0 )
  {
   if ( intcgm_cgm.cd_prec==0 ) localp = 8;
   else if ( intcgm_cgm.cd_prec==1 ) localp = 16;
   else if ( intcgm_cgm.cd_prec==2 ) localp = 24;
   else if ( intcgm_cgm.cd_prec==3 ) localp = 32;
  }

 switch ( localp )
  {
   case  1: if ( cgmb_getbit  ( &c ) ) return 1;
            *cd = (unsigned long) c;
            break;
   case  2: if ( cgmb_get2bit ( &c ) ) return 1;
            *cd = (unsigned long) c;
            break;
   case  4: if ( cgmb_get4bit ( &c ) )
            *cd = (unsigned long) c;
            return 1;
            break;
   case  8: if ( cgmb_getu8  ( &c ) ) return 1;
            *cd = (unsigned long) c;
            break;
   case 16: if ( cgmb_getu16 ( &i ) ) return 1;
            *cd = (unsigned long) i;
            break;
   case 24: if ( cgmb_getu24 ( cd ) ) return 1;
            break;
   case 32: if ( cgmb_getu32 ( cd ) ) return 1;
            break;
  }

 return 0;
}

int cgmb_pixelrgb ( unsigned long *r, unsigned long *g, unsigned long *b, int localp )
{
 if ( cgmb_pixeld ( r, localp ) ) return 1;
 if ( cgmb_pixeld ( g, localp ) ) return 1;
 if ( cgmb_pixeld ( b, localp ) ) return 1;

 return 0;
}

int cgmb_getpixel ( void *co, int localp )
{
 if ( intcgm_cgm.clrsm == 0 ) /* indexed */
  {
   unsigned long *ci = (unsigned long *) co;
   if ( cgmb_pixeli ( ci, localp ) ) return 1;
  }
 else
  {
   unsigned long *rgb = (unsigned long *) co;
   if ( cgmb_pixelrgb ( &rgb[0], &rgb[1], &rgb[2], localp ) ) return 1;
  }

 return 0;
}

/************************************************
*                                               *
*            Funcoes para clear text            *
*                                               *
************************************************/

void strlower ( char *string )
{
 int i;
 for ( i=0; string[i]!='\0'; i++ )
  string[i] = tolower ( string[i] );
}

char *cgmt_getsep (void)
{
 static char ch[256];

 fscanf ( intcgm_cgm.fp, "%[ \r\n\t\v\f,]", ch );

 return ch;
}

void cgmt_getcom (void)
{
 char chr[256], c;

 while ( (c = fgetc( intcgm_cgm.fp )) == '%' )
  {
   fscanf ( intcgm_cgm.fp, "%[^%]%%", chr );

   cgmt_getsep();
  }

 ungetc( c, intcgm_cgm.fp );
}

char *cgmt_getparentheses (void)
{
 static char ch[256];

 cgmt_getsep();

 fscanf ( intcgm_cgm.fp, "%[()]", ch );

 return ch;
}

int cgmt_ter ( void )
{
 char c;

 cgmt_getcom();

 cgmt_getsep();

 fscanf ( intcgm_cgm.fp, "%c", &c );

 if ( c=='/' || c==';' ) return 0;

 ungetc ( c, intcgm_cgm.fp );

 return 1;
}

int cgmt_rch ( void )
{
 char chr[256];
 char *pt;
 int i, j;

 cgmt_getsep();

 cgmt_getcom();

/* addcounter();*/

 fscanf ( intcgm_cgm.fp, "%[^ \r\n\t\v\f,/;%\"()]", chr );

 pt = strtok(chr,"_$");

 while ( (pt = strtok ( NULL, "_$" )) )
  strcat ( chr, pt );

 strlower(chr);

 for ( i=0; _cgmt_comandos[i]!=NULL; i++ )
  {
   for ( j=0; _cgmt_comandos[i][j]!=NULL; j++ )
    {
     if ( strcmp( chr, _cgmt_comandos[i][j]->nome )==0 )
      {
       int r = (*_cgmt_comandos[i][j]->func)();
	   cgm_getfilepos ();
	   return r;
	  }
    }
  }


 return 0;
}

int cgmt_i ( long *i )
{
 cgmt_getsep();

 cgmt_getcom();

 if ( fscanf( intcgm_cgm.fp, "%ld", i ) ) return 0;

 return 1;
}

int cgmt_ci ( unsigned long *ci )
{
 return cgmt_i ( (long*)ci );
}

int cgmt_cd ( unsigned long *cd )
{
 return cgmt_i ( (long *) cd );
}

int cgmt_rgb ( unsigned long *r, unsigned long *g, unsigned long *b )
{
 if ( cgmt_cd ( r ) ) return 1;

 if ( cgmt_cd ( g ) ) return 1;

 if ( cgmt_cd ( b ) ) return 1;
 
 return 0;
}

int cgmt_ix ( long *ix )
{
 return cgmt_i ( (long *) ix );
}

int cgmt_e ( short *e, const char **el )
{
 char chr[256];
 int i;
 char *pt;

 cgmt_getsep();

 cgmt_getcom();

 fscanf ( intcgm_cgm.fp, "%[^ \r\n\t\v\f,/;%\"\']", chr );

 strlower(chr);

 pt = strtok(chr,"_$");

 while ( (pt = strtok ( NULL, "_$" )) )
  strcat ( chr, pt );

 for ( i=0; el[i]!=NULL; i++ )
  if ( strcmp( chr, el[i] ) == 0 )
   {
    *e = i;
    return 0;
   };
   
 return 1;
}

int cgmt_r ( double *f )
{
 cgmt_getsep();

 cgmt_getcom();

 if ( fscanf( intcgm_cgm.fp, "%lg", f ) ) return 0;

 return 1;
}

int cgmt_s ( char **str )
{
 char c, delim;
 int intcgm_block = 80;
 int i = 0;

 *str = (char *) malloc ( intcgm_block*sizeof(char) );

 strcpy ( *str, "" );

 cgmt_getsep();

 cgmt_getcom();

 delim = fgetc ( intcgm_cgm.fp );

 if ( delim != '"' && delim != '\'' ) return 1;

 do
  {
   if ( (c=fgetc(intcgm_cgm.fp))==delim )
    if ( (c=fgetc(intcgm_cgm.fp))==delim )
     (*str)[i++] = c;
    else 
     {
      ungetc(c,intcgm_cgm.fp);
      break;
     }
   else
    (*str)[i++] = c;

   if ( (i+1)==intcgm_block )
    {
     intcgm_block *= 2;

     *str = (char *) realloc ( *str, intcgm_block*sizeof(char) );
    }
  } while ( 1 );

 (*str)[i] = '\0';

 /* addcounter();*/

 return 0;
}

int cgmt_vdc ( double *vdc )
{
 long l;

 if ( intcgm_cgm.vdc_type==0 )
  {
   if ( cgmt_i ( &l ) ) return 1;
   *vdc = (double) l;
   return 0;
  }
 else
  return cgmt_r ( vdc  );
}

int cgmt_p ( double *x, double *y )
{
 cgmt_getparentheses();

 if ( cgmt_vdc ( x ) ) return 1;

 if ( cgmt_vdc ( y ) ) return 1;

 cgmt_getparentheses();

 return 0;
}

int cgmt_co ( void *co )
{
 if ( intcgm_cgm.clrsm == 0 ) /* indexed */
  {
   unsigned long *ci = (unsigned long*)co;
   if ( cgmt_ci ( ci ) ) return 1;
  }
 else
  {
   unsigned long *cb = (unsigned long *) co;
   if ( cgmt_rgb ( &cb[0], &cb[1], &cb[2] ) ) return 1;
  }
  
 return 0; 
}
