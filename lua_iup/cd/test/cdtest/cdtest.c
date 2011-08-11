/*=========================================================================*/
/* CDTEST.C - 09/12/95                                                     */
/* Canvas Draw Test.                                                       */
/*=========================================================================*/

/*- Convenccoes Usadas: ---------------------------------------------------*/
/* - Identificadores de funccoes associadas a um callback comeccam por f.  */
/*   Ex: fResize, fRepaint.                                                */
/* - Identificadores de constantes sao escritos em maiusculas.             */
/*   Ex: LINE, ARC, NEWPOINT.                                              */
/* - Identificadores de funcoes internas do programa sao escritas em       */
/*   minusculas.                                                           */
/*   Ex: newpolypoint, dellist.                                            */
/*-------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <iup.h>
#include <cd.h>
#include <wd.h>
#include <cdiup.h>
#include <cdpicture.h>
#include <cddbuf.h>
#include <cdirgb.h>
#include <iupkey.h>

#include "cdtest.h"

/*- Contexto do CD Test: --------------------------------------------------*/
tCTC ctgc;

/*- Parametros geometricos da primitiva sendo desenhada: ------------------*/
tLinePos line_pos;
tBoxPos box_pos;
tPixelPos pixel_pos;
tMarkPos mark_pos;
tArcPos arc_pos;


#ifdef USE_CONTEXTPLUS
static const int use_contextplus = 1;
#else
static const int use_contextplus = 0;
#endif

static const int antialias = 1;


/*-------------------------------------------------------------------------*/
/* Associa os call-backs do IUP.                                           */
/*-------------------------------------------------------------------------*/
void setcallbacks(void)
{
  IupSetFunction("cmdFileExit", (Icallback) fFileExit);
  
  IupSetFunction("cmdEditClear", (Icallback) fEditClear);
  IupSetFunction("cmdEditUndo", (Icallback) fEditUndo);

  IupSetFunction("cmdHelpAbout", (Icallback) fHelpAbout);
  IupSetFunction("cmdCloseAbout", (Icallback) fCloseAbout);

  IupSetFunction("cmdWDCanvas", (Icallback) fWDCanvas);
  IupSetFunction("cmdCloseWD", (Icallback) fCloseWD);
  IupSetFunction("cmdPICCanvas", (Icallback) fPICCanvas);
  IupSetFunction("cmdClosePIC", (Icallback) fClosePIC);

  IupSetFunction("cmdShowDialog", (Icallback) fShowDialog);
  IupSetFunction("cmdLine", (Icallback) fLine);
  IupSetFunction("cmdRect", (Icallback) fRect);
  IupSetFunction("cmdBox", (Icallback) fBox);
  IupSetFunction("cmdArc", (Icallback) fArc);
  IupSetFunction("cmdSector", (Icallback) fSector);
  IupSetFunction("cmdChord", (Icallback) fChord);
  IupSetFunction("cmdPixel", (Icallback) fPixel);
  IupSetFunction("cmdMark", (Icallback) fMark);
  IupSetFunction("cmdText", (Icallback) fText);
  IupSetFunction("cmdPoly", (Icallback) fPoly);

  IupSetFunction("cmdOptions", (Icallback) fOptions);
  IupSetFunction("cmdOptionsHide", (Icallback) fOptionsHide);
  IupSetFunction("cmdAttributes", (Icallback) fAttributes);
  IupSetFunction("cmdAttributesHide", (Icallback) fAttributesHide);
  IupSetFunction("cmdMsgHide", (Icallback) fMsgHide);
  IupSetFunction("cmdSimulate", (Icallback) fSimulate);
  IupSetFunction("cmdStretchPlay", (Icallback) fStretchPlay);

  IupSetFunction("cmdWriteMode",  (Icallback) fWriteMode);
  IupSetFunction("cmdLineStyle", (Icallback) fLineStyle);
  IupSetFunction("cmdLineCap", (Icallback) fLineCap);
  IupSetFunction("cmdLineJoin", (Icallback) fLineJoin);
  IupSetFunction("cmdFillMode", (Icallback) fFillMode);
  IupSetFunction("cmdFontStyle", (Icallback) fFontStyle);
  IupSetFunction("cmdFontTypeFace", (Icallback) fFontTypeFace);
  IupSetFunction("cmdMarkType", (Icallback) fMarkType);
  IupSetFunction("cmdTextAlignment", (Icallback) fTextAlignment);
  IupSetFunction("cmdHatchStyle", (Icallback) fHatchStyle);
  IupSetFunction("cmdOpacity", (Icallback) fOpacity);

  IupSetFunction("cmdNoBuffering", (Icallback) fNoBuffering);
  IupSetFunction("cmdImageBuffer", (Icallback) fImageBuffer);
  IupSetFunction("cmdRGBBuffer", (Icallback) fRGBBuffer);

  IupSetFunction("cmdInteger", (Icallback) fInteger);
  IupSetFunction("cmdReal", (Icallback) fReal);
  IupSetFunction("cmdDraw", (Icallback) fDraw);

  IupSetFunction("cmdSolid", (Icallback)   fSolid);
  IupSetFunction("cmdHatch", (Icallback)   fHatch);
  IupSetFunction("cmdStipple", (Icallback) fStipple);
  IupSetFunction("cmdPattern", (Icallback) fPattern);

  IupSetFunction("cmdOpenLines", (Icallback) fOpenLines);
  IupSetFunction("cmdClosedLines", (Icallback) fClosedLines);
  IupSetFunction("cmdFill", (Icallback) fFill);
  IupSetFunction("cmdPolyClip", (Icallback) fPolyClip);
  IupSetFunction("cmdPolyBezier", (Icallback) fPolyBezier);

  IupSetFunction("cmdClip", (Icallback) fClip);
  IupSetFunction("cmdClipArea", (Icallback) fClipArea);
  IupSetFunction("cmdClipOff", (Icallback) fClipOff);
  IupSetFunction("cmdClipPoly", (Icallback) fClipPoly);

  IupSetFunction("cmdImage", (Icallback) fImage);
  IupSetFunction("cmdImagePut", (Icallback) fImagePut);
  IupSetFunction("cmdImageGet", (Icallback) fImageGet);

  IupSetFunction("cmdImageRGB", (Icallback) fImageRGB);
  IupSetFunction("cmdImageRGBPut", (Icallback) fImageRGBPut);
  IupSetFunction("cmdImageRGBGet", (Icallback) fImageRGBGet);
}

/*-------------------------------------------------------------------------*/
/* Inicializa o stipple e o pattern exemplo.                               */
/*-------------------------------------------------------------------------*/
void initsamples(void)
{
  int i;

  /* zera os vetores */
  for (i=0; i<100; i++) {
    ctgc.stipple[i] = 0;
    ctgc.pattern[i] = 0xffffffl;
  }

  /* especificaccao do stipple */
  ctgc.stipple[11] = 1;  /*------------*/ 
  ctgc.stipple[21] = 1;  /*  0123456789*/  
  ctgc.stipple[31] = 1;  /*            */  
  ctgc.stipple[41] = 1;  /*9 0000000000*/  
  ctgc.stipple[51] = 1;  /*8 0000111110*/  
                         /*7 0001000110*/  
  ctgc.stipple[12] = 1;  /*6 0010001010*/  
  ctgc.stipple[52] = 1;  /*5 0111110010*/  
  ctgc.stipple[62] = 1;  /*4 0100010010*/  
                         /*3 0100010100*/  
  ctgc.stipple[13] = 1;  /*2 0100011000*/  
  ctgc.stipple[53] = 1;  /*1 0111110000*/  
  ctgc.stipple[73] = 1;  /*0 0000000000*/  
                         /*------------*/  
  ctgc.stipple[14] = 1;    
  ctgc.stipple[54] = 1;
  ctgc.stipple[84] = 1;

  ctgc.stipple[15] = 1; ctgc.stipple[26] = 1;
  ctgc.stipple[25] = 1; ctgc.stipple[66] = 1;
  ctgc.stipple[35] = 1; ctgc.stipple[86] = 1;
  ctgc.stipple[45] = 1; 
  ctgc.stipple[55] = 1; ctgc.stipple[48] = 1;
  ctgc.stipple[85] = 1; ctgc.stipple[58] = 1;
                        ctgc.stipple[68] = 1;
  ctgc.stipple[37] = 1; ctgc.stipple[78] = 1;
  ctgc.stipple[77] = 1; ctgc.stipple[88] = 1;
  ctgc.stipple[87] = 1; 
                        

  /* especificaccao do stipple */
  ctgc.pattern[11] = CD_RED;   /*------------*/
  ctgc.pattern[21] = CD_RED;   /*  0123456789*/               
  ctgc.pattern[31] = CD_RED;   /*            */               
  ctgc.pattern[41] = CD_RED;   /*9 WWWWWWWWWW*/               
  ctgc.pattern[51] = CD_RED;   /*8 WWWWGGGGGW*/               
  ctgc.pattern[12] = CD_RED;   /*7 WWWGGGGGBW*/                                         
  ctgc.pattern[22] = CD_RED;   /*6 WWGGGGGBBW*/                                         
  ctgc.pattern[32] = CD_RED;   /*5 WrrrrrBBBW*/                                         
  ctgc.pattern[42] = CD_RED;   /*4 WrrrrrBBBW*/                                         
  ctgc.pattern[52] = CD_RED;   /*3 WrrrrrBBWW*/                                         
  ctgc.pattern[13] = CD_RED;   /*2 WrrrrrBWWW*/                                         
  ctgc.pattern[23] = CD_RED;   /*1 WrrrrrWWWW*/                                         
  ctgc.pattern[33] = CD_RED;   /*0 WWWWWWWWWW*/                                         
  ctgc.pattern[43] = CD_RED;   /*------------*/                                         
  ctgc.pattern[53] = CD_RED;                                            
  ctgc.pattern[14] = CD_RED;   ctgc.pattern[15] = CD_RED;
  ctgc.pattern[24] = CD_RED;   ctgc.pattern[25] = CD_RED;
  ctgc.pattern[34] = CD_RED;   ctgc.pattern[35] = CD_RED;
  ctgc.pattern[44] = CD_RED;   ctgc.pattern[45] = CD_RED;
  ctgc.pattern[54] = CD_RED;   ctgc.pattern[55] = CD_RED;
  
  ctgc.pattern[26] = CD_BLUE;  ctgc.pattern[37] = CD_BLUE;
  ctgc.pattern[36] = CD_BLUE;  ctgc.pattern[47] = CD_BLUE;
  ctgc.pattern[46] = CD_BLUE;  ctgc.pattern[57] = CD_BLUE;
  ctgc.pattern[56] = CD_BLUE;  ctgc.pattern[67] = CD_BLUE;
  
  ctgc.pattern[48] = CD_BLUE;  ctgc.pattern[62] = CD_GREEN;
  ctgc.pattern[58] = CD_BLUE;  ctgc.pattern[63] = CD_GREEN;
  ctgc.pattern[68] = CD_BLUE;  ctgc.pattern[64] = CD_GREEN;
  ctgc.pattern[78] = CD_BLUE;  ctgc.pattern[65] = CD_GREEN;
                               ctgc.pattern[66] = CD_GREEN;

  ctgc.pattern[73] = CD_GREEN; ctgc.pattern[84] = CD_GREEN;
  ctgc.pattern[74] = CD_GREEN; ctgc.pattern[85] = CD_GREEN;
  ctgc.pattern[75] = CD_GREEN; ctgc.pattern[86] = CD_GREEN;
  ctgc.pattern[76] = CD_GREEN; ctgc.pattern[87] = CD_GREEN;
  ctgc.pattern[77] = CD_GREEN; ctgc.pattern[88] = CD_GREEN;

  ctgc.dashes[0] = 10; 
  ctgc.dashes[1] = 2; 
  ctgc.dashes[2] = 5; 
  ctgc.dashes[3] = 2; 
}

/*-------------------------------------------------------------------------*/
/* Inicializa o CD Test.                                                   */
/*-------------------------------------------------------------------------*/
void CDTestInit(void)
{
  memset(&ctgc, 0, sizeof(ctgc));

  if (use_contextplus) 
  {
#ifdef USE_CONTEXTPLUS
    cdInitContextPlus();
#endif
  }

  /* inicializaccao dos drivers */
  DriversInit();
  
  /* mostra o dialogo principal */
  IupShow(IupGetHandle("dlgMain")); 

  /* inicializaccao da barra de cores */
  ColorBarInit(IupGetHandle("dlgMain"), IupGetHandle("cnvColorBar"), &ctgc.foreground, &ctgc.background);
    
  /* cria o canvas do CD associado ao canvas do IUP */
  if (use_contextplus) cdUseContextPlus(1); 
  ctgc.iup_canvas = cdCreateCanvas(CD_IUP, IupGetHandle("cnvMain"));
  if (!antialias) cdCanvasSetAttribute(ctgc.iup_canvas, "ANTIALIAS", "0");
  cdActivate(ctgc.iup_canvas);
  if (use_contextplus) cdUseContextPlus(0); 

  /* associa os call-backs */
  setcallbacks();

  /* os call-backs do canvas devem ser associados depois de sua criacao */
  IupSetFunction("cmdRepaint", (Icallback) fRepaint);
  IupSetFunction("cmdMotionCB", (Icallback) fMotionCB);
  IupSetFunction("cmdButtonCB", (Icallback) fButtonCB);
  IupSetFunction("cmdResizeCB", (Icallback) fResizeCB);
  IupSetFunction("cmdGetFocusCB", (Icallback) fGetFocusCB);

  /* inicializaccao do contexto */
  ctgc.write_mode = CD_REPLACE;
  ctgc.line_style = CD_CONTINUOUS;
  ctgc.line_cap = CD_CAPFLAT;
  ctgc.line_join = CD_MITER;
  ctgc.fill_mode = CD_EVENODD;
  ctgc.line_width = 1;
  ctgc.font_style = CD_PLAIN;
  ctgc.font_typeface = CD_SYSTEM;
  ctgc.font_size = CD_STANDARD;
  ctgc.text_alignment = CD_BASE_LEFT;
  ctgc.text_orientation = 0;
  ctgc.back_opacity =  CD_TRANSPARENT;
  ctgc.mark_type = CD_STAR;
  ctgc.poly_mode = CD_OPEN_LINES;
  ctgc.interior_style = CD_SOLID;
  ctgc.hatch = CD_HORIZONTAL;
  ctgc.following = FALSE;
  ctgc.foreground = CD_BLACK;
  ctgc.background = CD_WHITE;
  ctgc.head = NULL;
  ctgc.test_image = NULL;
  ctgc.sim = 0;
  ctgc.stretch_play = 0;
  ctgc.dlg_x = IUP_CENTER;
  ctgc.dlg_y = IUP_CENTER;
  ctgc.visible = 0;

  /* inicializa os vetores stipple e pattern */
  initsamples();

  /* inicializa o CDTest com a primitiva LINE */
  ctgc.dlg_cur_prim = IupGetHandle("dlgLB");
  ctgc.bt_cur_prim = IupGetHandle("btCurPrim");
  ctgc.cur_prim = LINE;
  
  /* inicializaccao do Canvas do IUP */
  cdActivate(ctgc.iup_canvas);
  cdFont(ctgc.font_typeface,ctgc.font_style,ctgc.font_size);
  cdBackground(ctgc.background);
  cdClear();
  cdGetCanvasSize(&(ctgc.w),&(ctgc.h),NULL,NULL);
  ctgc.bpp = cdGetColorPlanes();

  {
    double mm, xres;  
    cdPixel2MM(1, 0, &mm, NULL);
    xres = 1.0/mm;
    ctgc.res = xres;
  }

  /* inicializa o canvas off-screen de double-bufering */
  ctgc.buffering = NO_BUFFER;
  ctgc.buffer_canvas = NULL;

  /* inicializa o clipping */
  ctgc.clip_mode = CD_CLIPOFF;
  cdGetClipArea(&(ctgc.clip_xmin), &(ctgc.clip_xmax),
                &(ctgc.clip_ymin), &(ctgc.clip_ymax));

  /* zera o buffer temporario de pontos */
  ctgc.num_points = 0;

  /* a lista de primitivas jah estah vazia, nao hah o que apagar */
  IupSetAttribute(IupGetHandle("itEditUndo"), IUP_ACTIVE, IUP_NO);

  /* atualiza o tamanho do canvas em pixels na barra de titulo */
  sprintf(ctgc.title, "CDTest 5.3 (%dx%d - %dbpp)", ctgc.w, ctgc.h, ctgc.bpp);
  IupSetAttribute(IupGetHandle("dlgMain"), IUP_TITLE, ctgc.title);

  /* inicializa a barra de status */
  sprintf(ctgc.status_line, "LEFT click and drag.");
  set_status();

  /* inicializa a posiccao do mouse */
  mouse_pos(0, 0);
  
  /* constroi os dialogos do CDTest sem mostra-los */
  IupMap(IupGetHandle("dlgLB"));
  IupMap(IupGetHandle("dlgAS"));
  IupMap(IupGetHandle("dlgPixel"));
  IupMap(IupGetHandle("dlgImage"));
  IupMap(IupGetHandle("dlgImageRGB"));
  IupMap(IupGetHandle("dlgMark"));
  IupMap(IupGetHandle("dlgText"));
  IupMap(IupGetHandle("dlgClip"));
  IupMap(IupGetHandle("dlgAttributes"));
  IupMap(IupGetHandle("dlgWDCanvas"));
  IupMap(IupGetHandle("dlgPICCanvas"));

  /* cria o canvas WD */
  if (use_contextplus) cdUseContextPlus(1); 
  ctgc.wd_canvas = cdCreateCanvas(CD_IUP, IupGetHandle("cnvWDCanvas"));
  ctgc.pic_canvas = cdCreateCanvas(CD_IUP, IupGetHandle("cnvPICCanvas"));
  ctgc.picture = cdCreateCanvas(CD_PICTURE, "");
  if (use_contextplus) cdUseContextPlus(0); 

  /* CDTEST default values */
  cdActivate(ctgc.picture);
  cdLineStyleDashes(ctgc.dashes, 4);
  cdPattern(10, 10, ctgc.pattern);
  cdStipple(10, 10, ctgc.stipple);
  cdInteriorStyle(CD_SOLID);

  cdActivate(ctgc.wd_canvas);
  cdLineStyleDashes(ctgc.dashes, 4);
  cdPattern(10, 10, ctgc.pattern);
  cdStipple(10, 10, ctgc.stipple);
  cdInteriorStyle(CD_SOLID);

  /* reativa o canvas IUP */
  cdActivate(ctgc.iup_canvas);
  cdLineStyleDashes(ctgc.dashes, 4);
  cdPattern(10, 10, ctgc.pattern);
  cdStipple(10, 10, ctgc.stipple);
  cdInteriorStyle(CD_SOLID);
}

static void CDTestClose(void)
{
  dellist();         

  ColorBarClose();

  if (ctgc.buffer_canvas) cdKillCanvas(ctgc.buffer_canvas);
  if (ctgc.test_image) cdKillImage(ctgc.test_image);
  cdKillCanvas(ctgc.picture);
  cdKillCanvas(ctgc.pic_canvas);
  cdKillCanvas(ctgc.wd_canvas);
  cdKillCanvas(ctgc.iup_canvas);

  memset(&ctgc, 0, sizeof(tCTC));

  IupDestroy(IupGetHandle("dlgLB"));
  IupDestroy(IupGetHandle("dlgAS"));
  IupDestroy(IupGetHandle("dlgPixel"));
  IupDestroy(IupGetHandle("dlgImage"));
  IupDestroy(IupGetHandle("dlgImageRGB"));
  IupDestroy(IupGetHandle("dlgMark"));
  IupDestroy(IupGetHandle("dlgText"));
  IupDestroy(IupGetHandle("dlgClip"));
  IupDestroy(IupGetHandle("dlgAttributes"));
  IupDestroy(IupGetHandle("dlgWDCanvas"));
  IupDestroy(IupGetHandle("dlgPICCanvas"));

  IupDestroy(IupGetHandle("dlgMain"));
}

static int iscurvisible(void)
{
  char* vis = IupGetAttribute(ctgc.dlg_cur_prim, IUP_VISIBLE);
  if (!vis)
    return 0;
  return strcmp(vis, IUP_YES) == 0? 1: 0;
}

/*-------------------------------------------------------------------------*/
/* Copia o conteudo da lista de primitivas para o dispositivo ativo.       */
/*-------------------------------------------------------------------------*/
void putlist(cdCanvas *target)
{
  tList *temp;
  int wdc_w, wdc_h, wd = 0;

  /* ativa o canvas destino */
  cdActivate(target);

  if (target == ctgc.wd_canvas)
  {
    cdGetCanvasSize(&wdc_w, &wdc_h, NULL, NULL);
    wdWindow(0, ctgc.w, 0, ctgc.h);
    wdViewport(0, wdc_w, 0, wdc_h);
    wd = 1;
  }

  /* limpa o canvas com a cor de fundo */
  cdBackground(ctgc.background);
  cdClear();
  cdClipArea(ctgc.clip_xmin, ctgc.clip_xmax, ctgc.clip_ymin, ctgc.clip_ymax);
  cdClip(ctgc.clip_mode);

  /* coloca a lista de primitivas no canvas */
  for (temp = ctgc.head; temp; temp = (tList *) temp->next) 
  {
    switch (temp->type) {
      case LINE:
        cdWriteMode(temp->par.lineboxpar.write_mode);
        cdLineCap(temp->par.lineboxpar.line_cap);
        cdLineJoin(temp->par.lineboxpar.line_join);
        cdLineStyle(temp->par.lineboxpar.line_style);
        cdLineWidth(temp->par.lineboxpar.line_width);
        cdForeground(temp->par.lineboxpar.foreground);
        if (wd)
          wdLine(temp->par.lineboxpar.x1, temp->par.lineboxpar.y1,
            temp->par.lineboxpar.x2, temp->par.lineboxpar.y2);
        else
          cdLine(temp->par.lineboxpar.x1, temp->par.lineboxpar.y1,
            temp->par.lineboxpar.x2, temp->par.lineboxpar.y2);
        break;
      case RECT:
        cdWriteMode(temp->par.lineboxpar.write_mode);
        cdLineCap(temp->par.lineboxpar.line_cap);
        cdLineJoin(temp->par.lineboxpar.line_join);
        cdLineStyle(temp->par.lineboxpar.line_style);
        cdLineWidth(temp->par.lineboxpar.line_width);
        cdForeground(temp->par.lineboxpar.foreground);
        if (wd)
          wdRect(temp->par.lineboxpar.x1, temp->par.lineboxpar.x2,
            temp->par.lineboxpar.y1, temp->par.lineboxpar.y2);
        else
          cdRect(temp->par.lineboxpar.x1, temp->par.lineboxpar.x2,
            temp->par.lineboxpar.y1, temp->par.lineboxpar.y2);
        break;
      case BOX:
        cdWriteMode(temp->par.lineboxpar.write_mode);
        cdLineCap(temp->par.lineboxpar.line_cap);
        cdLineJoin(temp->par.lineboxpar.line_join);
        cdLineStyle(temp->par.lineboxpar.line_style);
        cdLineWidth(temp->par.lineboxpar.line_width);
        cdForeground(temp->par.lineboxpar.foreground);
        cdBackground(temp->par.lineboxpar.background);
        cdBackOpacity(temp->par.lineboxpar.back_opacity);
        cdHatch(temp->par.lineboxpar.hatch);
        cdInteriorStyle(temp->par.lineboxpar.interior_style);
        if (wd)
          wdBox(temp->par.lineboxpar.x1, temp->par.lineboxpar.x2,
            temp->par.lineboxpar.y1, temp->par.lineboxpar.y2);
        else
          cdBox(temp->par.lineboxpar.x1, temp->par.lineboxpar.x2,
            temp->par.lineboxpar.y1, temp->par.lineboxpar.y2);
        break;
      case ARC:
        cdWriteMode(temp->par.arcsectorpar.write_mode);
        cdLineCap(temp->par.arcsectorpar.line_cap);
        cdLineJoin(temp->par.arcsectorpar.line_join);
        cdLineStyle(temp->par.arcsectorpar.line_style);
        cdLineWidth(temp->par.arcsectorpar.line_width);
        cdForeground(temp->par.arcsectorpar.foreground);
        if (wd)
          wdArc(temp->par.arcsectorpar.xc, temp->par.arcsectorpar.yc,
            temp->par.arcsectorpar.w, temp->par.arcsectorpar.h,
            temp->par.arcsectorpar.angle1, temp->par.arcsectorpar.angle2);
        else
          cdArc(temp->par.arcsectorpar.xc, temp->par.arcsectorpar.yc,
            temp->par.arcsectorpar.w, temp->par.arcsectorpar.h,
            temp->par.arcsectorpar.angle1, temp->par.arcsectorpar.angle2);
        break;
      case CHORD:
        cdWriteMode(temp->par.arcsectorpar.write_mode);
        cdLineCap(temp->par.arcsectorpar.line_cap);
        cdLineJoin(temp->par.arcsectorpar.line_join);
        cdLineStyle(temp->par.arcsectorpar.line_style);
        cdLineWidth(temp->par.arcsectorpar.line_width);
        cdForeground(temp->par.arcsectorpar.foreground);
        cdBackground(temp->par.arcsectorpar.background);
        cdBackOpacity(temp->par.arcsectorpar.back_opacity);
        cdHatch(temp->par.arcsectorpar.hatch);
        cdInteriorStyle(temp->par.arcsectorpar.interior_style);
        if (wd)
          wdChord(temp->par.arcsectorpar.xc, temp->par.arcsectorpar.yc,
            temp->par.arcsectorpar.w, temp->par.arcsectorpar.h,
            temp->par.arcsectorpar.angle1, temp->par.arcsectorpar.angle2);
        else
          cdChord(temp->par.arcsectorpar.xc, temp->par.arcsectorpar.yc,
            temp->par.arcsectorpar.w, temp->par.arcsectorpar.h,
            temp->par.arcsectorpar.angle1, temp->par.arcsectorpar.angle2);
        break;
      case SECTOR:
        cdWriteMode(temp->par.arcsectorpar.write_mode);
        cdLineCap(temp->par.arcsectorpar.line_cap);
        cdLineJoin(temp->par.arcsectorpar.line_join);
        cdLineStyle(temp->par.arcsectorpar.line_style);
        cdLineWidth(temp->par.arcsectorpar.line_width);
        cdForeground(temp->par.arcsectorpar.foreground);
        cdBackground(temp->par.arcsectorpar.background);
        cdBackOpacity(temp->par.arcsectorpar.back_opacity);
        cdHatch(temp->par.arcsectorpar.hatch);
        cdInteriorStyle(temp->par.arcsectorpar.interior_style);
        if (wd)
          wdSector(temp->par.arcsectorpar.xc, temp->par.arcsectorpar.yc,
            temp->par.arcsectorpar.w, temp->par.arcsectorpar.h,
            temp->par.arcsectorpar.angle1, temp->par.arcsectorpar.angle2);
        else
          cdSector(temp->par.arcsectorpar.xc, temp->par.arcsectorpar.yc,
            temp->par.arcsectorpar.w, temp->par.arcsectorpar.h,
            temp->par.arcsectorpar.angle1, temp->par.arcsectorpar.angle2);
        break;
      case PIXEL:
        cdWriteMode(temp->par.pixelpar.write_mode);
        cdPixel(temp->par.pixelpar.x, temp->par.pixelpar.y,
          temp->par.pixelpar.foreground);
        break;
      case MARK:
        cdWriteMode(temp->par.markpar.write_mode);
        cdMarkSize(temp->par.markpar.mark_size);
        cdMarkType(temp->par.markpar.mark_type);
        cdForeground(temp->par.markpar.foreground);
        if (wd)
          wdMark(temp->par.markpar.x, temp->par.markpar.y);
        else
          cdMark(temp->par.markpar.x, temp->par.markpar.y);
        break;
      case TEXT:
        cdWriteMode(temp->par.textpar.write_mode);
        cdForeground(temp->par.textpar.foreground);
        cdBackground(temp->par.textpar.background);
        cdBackOpacity(temp->par.textpar.back_opacity);
        cdTextAlignment(temp->par.textpar.text_alignment);
        cdTextOrientation(temp->par.textpar.text_orientation);
        cdFont(temp->par.textpar.font_typeface,
          temp->par.textpar.font_style,
          temp->par.textpar.font_size);
        if (wd)
          wdText(temp->par.textpar.x, temp->par.textpar.y,
            temp->par.textpar.s);
        else
          cdText(temp->par.textpar.x, temp->par.textpar.y,
            temp->par.textpar.s);
        break;
      case POLY: {
        int i;
        cdForeground(temp->par.polypar.foreground);
        cdBackground(temp->par.polypar.background);
        cdWriteMode(temp->par.polypar.write_mode);
        cdLineCap(temp->par.polypar.line_cap);
        cdLineJoin(temp->par.polypar.line_join);
        cdLineStyle(temp->par.polypar.line_style);
        cdLineWidth(temp->par.polypar.line_width);
        cdFillMode(temp->par.polypar.fill_mode);
        cdBackOpacity(temp->par.polypar.back_opacity);
        cdHatch(temp->par.polypar.hatch);
        cdInteriorStyle(temp->par.polypar.interior_style);
        cdBegin(temp->par.polypar.poly_mode);
        if (wd)
          for (i=0; (i<temp->par.polypar.num_points); i++) {
            wdVertex(temp->par.polypar.points[i].x,
              temp->par.polypar.points[i].y);
          }
        else
          for (i=0; (i<temp->par.polypar.num_points); i++) {
            cdVertex(temp->par.polypar.points[i].x,
              temp->par.polypar.points[i].y);
          }
        cdEnd();
        break;
      case META:
        cdWriteMode(CD_REPLACE);
        cdLineStyle(CD_CONTINUOUS);
        cdLineWidth(1);
        cdBackground(CD_WHITE);
        cdBackOpacity(CD_TRANSPARENT);
        cdForeground(CD_BLACK);
        cdInteriorStyle(CD_SOLID);
        if (ctgc.stretch_play)
        {
          if (wd)
            cdPlay(temp->par.metapar.ctx, 0, wdc_w-1, 0, wdc_h-1, temp->par.metapar.filename);
          else
            cdPlay(temp->par.metapar.ctx, 0, ctgc.w-1, 0, ctgc.h-1, temp->par.metapar.filename);
        }
        else
          cdPlay(temp->par.metapar.ctx, 0, 0, 0, 0, temp->par.metapar.filename);
        break;
      default:
        break;
      }
    }
  }

  /* volta o clip-mode para o corrente */
  cdClip(ctgc.clip_mode);
}

/*-------------------------------------------------------------------------*/
/* Copia o conteudo da lista de primitivas no canvas WD.                   */
/*-------------------------------------------------------------------------*/
int fWDRepaint(void)
{
  putlist(ctgc.wd_canvas);

  /* reativa o canvas iup */
  cdActivate(ctgc.iup_canvas);

  return IUP_DEFAULT;
}

int fPICRepaint(void)
{
  int w, h;
  putlist(ctgc.picture);

  cdActivate(ctgc.pic_canvas);
  cdGetCanvasSize(&w, &h, NULL, NULL);
  cdClear();
  if (ctgc.stretch_play)
    cdPlay(CD_PICTURE, 0, w-1, 0, h-1, ctgc.picture);
  else
    cdPlay(CD_PICTURE, 0, 0, 0, 0, ctgc.picture);

  /* reativa o canvas iup */
  cdActivate(ctgc.iup_canvas);

  return IUP_DEFAULT;
}

/*-------------------------------------------------------------------------*/
/* Redesenha o canvas.                                                     */
/*-------------------------------------------------------------------------*/
void updatecanvas(void)
{
  if (ctgc.buffering == NO_BUFFER)
    putlist(ctgc.iup_canvas);
  else
  {
    cdActivate(ctgc.iup_canvas);
    cdClip(CD_CLIPOFF);

    putlist(ctgc.buffer_canvas);
    cdFlush();

    cdActivate(ctgc.iup_canvas);
    cdClip(ctgc.clip_mode);
  }

  if (ctgc.wd_dialog)
    fWDRepaint();

  if (ctgc.pic_dialog)
    fPICRepaint();
}

/*-------------------------------------------------------------------------*/
/* Redesenha o canvas.                                                     */
/*-------------------------------------------------------------------------*/
int fRepaint(void)
{
  int i;

  /* ativa o canvas na tela */
  cdActivate(ctgc.iup_canvas);
  wdViewport(0, ctgc.w, 0, ctgc.h);

  /* desliga o clipping durante o evento Repaint */
  cdClip(CD_CLIPOFF);

  /* double-buffering? */
  cdWriteMode(CD_REPLACE);
  updatecanvas();

  /* se o evento repaint for gerado durante o rubber band...*/
  /* ...eh preciso restaurar o estado anterior */
  if (ctgc.following) {
    cdWriteMode(CD_NOT_XOR);
    cdForeground(CD_BLACK);
    cdLineStyle(CD_CONTINUOUS);
    cdLineWidth(1);
    switch (ctgc.cur_prim) {
      case POLY:
        for (i=0; (i<ctgc.num_points-1); i++) {
          cdLine(ctgc.points[i].x, ctgc.points[i].y,
                 ctgc.points[i+1].x, ctgc.points[i+1].y);
        }
        polygon(REPAINT, 0, 0);
        break;
      case LINE:
        line(REPAINT, 0, 0);
        break;
      case ARC:
      case CHORD:
      case SECTOR:
        arc(REPAINT, 0, 0);
        break;
      case RECT:
      case BOX:
        box(REPAINT, 0, 0);
        break;
      default:
        break;
    }
  }

  /* restaura o estado de clipping anterior ao evento Repaint */
  cdClip(ctgc.clip_mode);

  return IUP_DEFAULT;
}

/*-------------------------------------------------------------------------*/
/* Ativa o canvas WD.                                                      */
/*-------------------------------------------------------------------------*/
int fWDCanvas(void)
{
  IupShow(IupGetHandle("dlgWDCanvas"));
  ctgc.wd_dialog = TRUE;
  IupSetFunction("cmdWDRepaint", (Icallback)fWDRepaint);
  fWDRepaint();
  return IUP_DEFAULT;
}

int fPICCanvas(void)
{
  IupShow(IupGetHandle("dlgPICCanvas"));
  ctgc.pic_dialog = TRUE;
  IupSetFunction("cmdPICRepaint", (Icallback)fPICRepaint);
  fPICRepaint();
  return IUP_DEFAULT;
}

/*-------------------------------------------------------------------------*/
/* Desativa o canvas WD.                                                   */
/*-------------------------------------------------------------------------*/
int fCloseWD(void) 
{
  ctgc.wd_dialog = FALSE;
  return IUP_DEFAULT;
}

int fClosePIC(void) 
{
  ctgc.pic_dialog = FALSE;
  return IUP_DEFAULT;
}

/*-------------------------------------------------------------------------*/
/* Apaga a ultima primitiva desenhada.                                     */
/*-------------------------------------------------------------------------*/
int fEditUndo(void)
{
  /* apaga a ultima primitiva da fila */
  dellast();
  /* se nao ha mais primitivas na fila, desabilita o undo */
  if (ctgc.head == NULL) 
    IupSetAttribute(IupGetHandle("itEditUndo"), IUP_ACTIVE, IUP_NO);

  updatecanvas();

  return IUP_DEFAULT;
}

int fSimulate(Ihandle *self, int v)
{
  ignore(self);

  cdActivate(ctgc.iup_canvas);

  if (v == 1) 
  {
    if (ctgc.sim == 1)
      return IUP_DEFAULT;

    ctgc.sim = 1;
    cdSimulate(CD_SIM_ALL);
    sprintf(ctgc.status_line, "cdSimulate(CD_SIM_ALL)");
  }
  else 
  {
    if (ctgc.sim == 0)
      return IUP_DEFAULT;

    ctgc.sim = 0;
    cdSimulate(CD_SIM_NONE);
    sprintf(ctgc.status_line, "cdSimulate(CD_SIM_NONE)");
  }

  set_status();
  updatecanvas();
  return IUP_DEFAULT;
}

int fStretchPlay(Ihandle *self, int v)
{
  ignore(self);
  ctgc.stretch_play = v;
  updatecanvas();
  return IUP_DEFAULT;
}

/*-------------------------------------------------------------------------*/
/* Tchau.                                                                  */
/*-------------------------------------------------------------------------*/
int fFileExit(void)
{
  IupHide(ctgc.dlg_cur_prim);
  return IUP_CLOSE;
}

/*-------------------------------------------------------------------------*/
/* Funcao associada ao toggle OpenLines.                                   */
/*-------------------------------------------------------------------------*/
int fOpenLines(void)
{
  ctgc.poly_mode = CD_OPEN_LINES;
  sprintf(ctgc.status_line, "cdBegin(CD_OPEN_LINES)");
  set_status();
  return IUP_DEFAULT;
}

/*-------------------------------------------------------------------------*/
/* Funcao associada ao toggle ClosedLines.                                 */
/*-------------------------------------------------------------------------*/
int fClosedLines(void)
{
  ctgc.poly_mode = CD_CLOSED_LINES;
  sprintf(ctgc.status_line, "cdBegin(CD_CLOSED_LINES)");
  set_status();
  return IUP_DEFAULT;
}

int fPolyBezier(void)
{
  ctgc.poly_mode = CD_BEZIER;
  sprintf(ctgc.status_line, "cdBegin(CD_BEZIER)");
  set_status();
  return IUP_DEFAULT;
}

/*-------------------------------------------------------------------------*/
/* Funcao associada ao toggle Fill.                                        */
/*-------------------------------------------------------------------------*/
int fFill(void)
{
  ctgc.poly_mode = CD_FILL;
  sprintf(ctgc.status_line, "cdBegin(CD_FILL)");
  set_status();
  return IUP_DEFAULT;
}

/*-------------------------------------------------------------------------*/
/* Funcao associada ao toggle Clip.                                        */
/*-------------------------------------------------------------------------*/
int fPolyClip(void)
{
  ctgc.poly_mode = CD_CLIP;
  sprintf(ctgc.status_line, "cdBegin(CD_CLIP)");
  set_status();
  return IUP_DEFAULT;
}

/*-------------------------------------------------------------------------*/
/* Funcao associada ao toggle Solid.                                       */
/*-------------------------------------------------------------------------*/
int fSolid(void)
{
  ctgc.interior_style = CD_SOLID;
  sprintf(ctgc.status_line, "cdInteriorStyle(CD_SOLID)");
  set_status();
  cdInteriorStyle(CD_SOLID);
  return IUP_DEFAULT;
}

/*-------------------------------------------------------------------------*/
/* Funcao associada ao toggle Hatch.                                       */
/*-------------------------------------------------------------------------*/
int fHatch(void)
{
  ctgc.interior_style = CD_HATCH;
  cdInteriorStyle(CD_HATCH);
  sprintf(ctgc.status_line, "cdInteriorStyle(CD_HATCH)");
  set_status();
  return IUP_DEFAULT;
}

/*-------------------------------------------------------------------------*/
/* Funcao associada ao toggle Stipple.                                     */
/*-------------------------------------------------------------------------*/
int fStipple(void)
{
  ctgc.interior_style = CD_STIPPLE;
  cdInteriorStyle(CD_STIPPLE);
  sprintf(ctgc.status_line, "cdInteriorStyle(CD_STIPPLE)");
  set_status();
  return IUP_DEFAULT;
}

/*-------------------------------------------------------------------------*/
/* Funcao associada ao toggle Pattern.                                     */
/*-------------------------------------------------------------------------*/
int fPattern(void)
{
  ctgc.interior_style = CD_PATTERN;
  cdInteriorStyle(CD_PATTERN);
  sprintf(ctgc.status_line, "cdInteriorStyle(CD_PATTERN)");
  set_status();
  return IUP_DEFAULT;
}

/*-------------------------------------------------------------------------*/
/* Seleciona o a opacidade do fundo.                                       */
/*-------------------------------------------------------------------------*/
int fOpacity(Ihandle *self, char *t, int o, int v)
{
  ignore(t);
  ignore(self);
  if (v == 0) return IUP_DEFAULT;
  switch (o) {
    case 1:
      ctgc.back_opacity = CD_OPAQUE;
      sprintf(ctgc.status_line, "cdBackOpacity(CD_OPAQUE)");
      break;
    case 2:
      ctgc.back_opacity = CD_TRANSPARENT;
      sprintf(ctgc.status_line, "cdBackOpacity(CD_TRANSPARENT)");
      break;
  }
  set_status();
  return IUP_DEFAULT;
}


/*-------------------------------------------------------------------------*/
/* Seleciona o modo de repaint para imagem do servidor.                    */
/*-------------------------------------------------------------------------*/
int fImageBuffer(Ihandle *self, int v)
{
  ignore(self);

  /* se o toggle foi para o estado ligado... */
  if (v) {
    switch (ctgc.buffering) {
    case IMAGE_BUFFER:
      break;
    case IMAGERGB_BUFFER:
      /* mata o canvas do buffer anterior */
      if (ctgc.buffer_canvas) cdKillCanvas(ctgc.buffer_canvas);
      ctgc.buffer_canvas = NULL;
      /* prosegue como se nao houvesse o buffer anterior */
    case NO_BUFFER:
      /* cria o canvas do buffer */
      if (use_contextplus) cdUseContextPlus(1); 
      ctgc.buffer_canvas = cdCreateCanvas(CD_DBUFFER, ctgc.iup_canvas);
      if (!antialias) cdCanvasSetAttribute(ctgc.buffer_canvas, "ANTIALIAS", "0");
      if (use_contextplus) cdUseContextPlus(0); 
      /* se nao consegui criar o canvas... */
      if (!ctgc.buffer_canvas) {
        /* desabilita o double-buffering */
        ctgc.buffering = NO_BUFFER;
        sprintf(ctgc.status_line, "Error creating CD_DBUFFER canvas.");
        set_status();
      }
      else {
        /* seta o modo de escrita para double-buffering em imagem do servidor */
        ctgc.buffering = IMAGE_BUFFER;
        /* reconstroe o stipple e o pattern no canvas do buffer */
        cdActivate(ctgc.buffer_canvas);
        cdLineStyleDashes(ctgc.dashes, 4);
        cdPattern(10, 10, ctgc.pattern);
        cdStipple(10, 10, ctgc.stipple);
        cdInteriorStyle(CD_SOLID);
        if (ctgc.sim == 1)
          cdSimulate(CD_SIM_ALL);
        else
          cdSimulate(CD_SIM_NONE);
        /* atualiza o buffer */
        updatecanvas();
        /* muda a linha de status */
        sprintf(ctgc.status_line, "CD_DBUFFER buffer active.");
        set_status();
      }

      break;
    }
  }
  return IUP_DEFAULT;
}

/*-------------------------------------------------------------------------*/
/* Seleciona o modo de repaint para imagem RGB.                            */
/*-------------------------------------------------------------------------*/
int fRGBBuffer(Ihandle *self, int v)
{
  ignore(self);

  if (v) {
    switch (ctgc.buffering) {
    case IMAGERGB_BUFFER:
      break;
    case IMAGE_BUFFER:
      /* mata o canvas do buffer anterior */
      if (ctgc.buffer_canvas) cdKillCanvas(ctgc.buffer_canvas);
      ctgc.buffer_canvas = NULL;
      /* prossegue como se nao houvesse o buffer anterior */
    case NO_BUFFER:
      /* cria o canvas do buffer */
      ctgc.buffer_canvas = cdCreateCanvas(CD_DBUFFERRGB, ctgc.iup_canvas);
      if (!antialias) cdCanvasSetAttribute(ctgc.buffer_canvas, "ANTIALIAS", "0");
      /* se nao consegui criar o canvas... */
      if (!ctgc.buffer_canvas) {
        /* mata a imagem alocada */
        /* desabilita o double-buffering */
        ctgc.buffering = NO_BUFFER;
        sprintf(ctgc.status_line, "Error creating CD_DBUFFERRGB canvas.");
        set_status();
      }
      else {
        /* seta o modo de escrita para double-buffering em imagem RGB */
        ctgc.buffering = IMAGERGB_BUFFER;
        /* reconstroe o stipple e o pattern no canvas do buffer */
        cdActivate(ctgc.buffer_canvas);
        cdLineStyleDashes(ctgc.dashes, 4);
        cdPattern(10, 10, ctgc.pattern);
        cdStipple(10, 10, ctgc.stipple);
        cdInteriorStyle(CD_SOLID);
        if (ctgc.sim == 1)
          cdSimulate(CD_SIM_ALL);
        else
          cdSimulate(CD_SIM_NONE);
        /* atualiza o buffer */
        updatecanvas();
        /* muda a linha de status */
        sprintf(ctgc.status_line, "CD_DBUFFERRGB buffer active.");
        set_status();
      }
      break;
    }
  }
  return IUP_DEFAULT;
}

/*-------------------------------------------------------------------------*/
/* Seleciona o modo de repaint para escrita diretamente na tela.           */
/*-------------------------------------------------------------------------*/
int fNoBuffering(Ihandle *self, int v)
{
  ignore(self);

  /* se foi para double-buffering, atualiza o buffer off-screen */
  if (v) {
    switch (ctgc.buffering) {
    case IMAGERGB_BUFFER:
      /* mata o canvas */
      if (ctgc.buffer_canvas) cdKillCanvas(ctgc.buffer_canvas);
      ctgc.buffer_canvas = NULL;
      /* passa a desenhar diretamente na tela */
      ctgc.buffering = NO_BUFFER;
      /* atualiza o buffer */
      updatecanvas();
      break;
    case IMAGE_BUFFER:
      /* mata o canvas */
      if (ctgc.buffer_canvas) cdKillCanvas(ctgc.buffer_canvas);
      ctgc.buffer_canvas = NULL;
      /* passa a desenhar diretamente na tela */
      ctgc.buffering = NO_BUFFER;
      /* atualiza o buffer */
      updatecanvas();
      break;
    case NO_BUFFER:
      break;
    }
  }
  /* muda a linha de status */
  sprintf(ctgc.status_line, "Now drawing on-screen.");
  set_status();
  
  return IUP_DEFAULT;
}

/*-------------------------------------------------------------------------*/
/* Funcoes da barra de estilos..                                           */
/*-------------------------------------------------------------------------*/
int fMarkType(Ihandle *self, char *t, int o, int v)
{
  ignore(t);
  ignore(self);
  if (v == 0) return IUP_DEFAULT;
  switch (o) {
    case 1:
      ctgc.mark_type = CD_PLUS;
      sprintf(ctgc.status_line, "cdMarkType(CD_PLUS)");
      break;
    case 2:
      ctgc.mark_type = CD_STAR;
      sprintf(ctgc.status_line, "cdMarkType(CD_STAR)");
      break;
    case 3:
      ctgc.mark_type = CD_CIRCLE;
      sprintf(ctgc.status_line, "cdMarkType(CD_CIRCLE)");
      break;
    case 4:
      ctgc.mark_type = CD_X;
      sprintf(ctgc.status_line, "cdMarkType(CD_X)");
      break;
    case 5:
      ctgc.mark_type = CD_BOX;
      sprintf(ctgc.status_line, "cdMarkType(CD_BOX)");
      break;
    case 6:
      ctgc.mark_type = CD_DIAMOND;
      sprintf(ctgc.status_line, "cdMarkType(CD_DIAMOND)");
      break;
    case 7:
      ctgc.mark_type = CD_HOLLOW_CIRCLE;
      sprintf(ctgc.status_line, "cdMarkType(CD_HOLLOW_CIRCLE)");
      break;
    case 8:
      ctgc.mark_type = CD_HOLLOW_BOX;
      sprintf(ctgc.status_line, "cdMarkType(CD_HOLLOW_BOX)");
      break;
    case 9:
      ctgc.mark_type = CD_HOLLOW_DIAMOND;
      sprintf(ctgc.status_line, "cdMarkType(CD_HOLLOW_DIAMOND)");
      break;
  }
  set_status();
  return IUP_DEFAULT;
}

int fWriteMode(Ihandle *self, char *t, int o, int v)
{
  ignore(t);
  ignore(self);
  if (v == 0) return IUP_DEFAULT;
  switch (o) {
    case 1:
      ctgc.write_mode = CD_REPLACE;
      sprintf(ctgc.status_line, "cdWriteMode(CD_REPLACE)");
      break;
    case 2:
      ctgc.write_mode = CD_XOR;
      sprintf(ctgc.status_line, "cdWriteMode(CD_XOR)");
      break;
    case 3:
      ctgc.write_mode = CD_NOT_XOR;
      sprintf(ctgc.status_line, "cdWriteMode(CD_NOT_XOR)");
      break;
  }
  set_status();
  return IUP_DEFAULT;
}

int fLineStyle(Ihandle *self, char *t, int o, int v)
{
  ignore(t);
  ignore(self);
  if (v == 0) return IUP_DEFAULT;
  switch (o) {
    case 1:
      ctgc.line_style = CD_CONTINUOUS;
      sprintf(ctgc.status_line, "cdLineStyle(CD_CONTINUOUS)");
      break;
    case 2:
      ctgc.line_style = CD_DASHED;
      sprintf(ctgc.status_line, "cdLineStyle(CD_DASHED)");
      break;
    case 3:
      ctgc.line_style = CD_DOTTED;
      sprintf(ctgc.status_line, "cdLineStyle(CD_DOTTED)");
      break;
    case 4:
      ctgc.line_style = CD_DASH_DOT;
      sprintf(ctgc.status_line, "cdLineStyle(CD_DASH_DOT)");
      break;
    case 5:
      ctgc.line_style = CD_DASH_DOT_DOT;
      sprintf(ctgc.status_line, "cdLineStyle(CD_DASH_DOT_DOT)");
      break;
    case 6:
      ctgc.line_style = CD_CUSTOM;
      sprintf(ctgc.status_line, "cdLineStyle(CD_CUSTOM)");
      break;
  }
  set_status();
  return IUP_DEFAULT;
}

int fLineCap(Ihandle *self, char *t, int o, int v)
{
  ignore(t);
  ignore(self);
  if (v == 0) return IUP_DEFAULT;
  switch (o) {
    case 1:
      ctgc.line_cap = CD_CAPFLAT;
      sprintf(ctgc.status_line, "cdLineCap(CD_CAPFLAT)");
      break;
    case 2:
      ctgc.line_cap = CD_CAPSQUARE;
      sprintf(ctgc.status_line, "cdLineCap(CD_CAPSQUARE)");
      break;
    case 3:
      ctgc.line_cap = CD_CAPROUND;
      sprintf(ctgc.status_line, "cdLineCap(CD_CAPROUND)");
      break;
  }
  set_status();
  return IUP_DEFAULT;
}

int fLineJoin(Ihandle *self, char *t, int o, int v)
{
  ignore(t);
  ignore(self);
  if (v == 0) return IUP_DEFAULT;
  switch (o) {
    case 1:
      ctgc.line_join = CD_MITER;
      sprintf(ctgc.status_line, "cdLineJoin(CD_MITER)");
      break;
    case 2:
      ctgc.line_join = CD_BEVEL;
      sprintf(ctgc.status_line, "cdLineJoin(CD_BEVEL)");
      break;
    case 3:
      ctgc.line_join = CD_ROUND;
      sprintf(ctgc.status_line, "cdLineJoin(CD_ROUND)");
      break;
  }
  set_status();
  return IUP_DEFAULT;
}

int fFillMode(Ihandle *self, char *t, int o, int v)
{
  ignore(t);
  ignore(self);
  if (v == 0) return IUP_DEFAULT;
  switch (o) {
    case 1:
      ctgc.fill_mode = CD_EVENODD;
      sprintf(ctgc.status_line, "cdFillMode(CD_EVENODD)");
      break;
    case 2:
      ctgc.fill_mode = CD_WINDING;
      sprintf(ctgc.status_line, "cdFillMode(CD_WINDING)");
      break;
  }
  set_status();
  return IUP_DEFAULT;
}

char* font_style[4] = {"CD_PLAIN", "CD_BOLD", "CD_ITALIC", "CD_BOLD_ITALIC"};
char* font_face[4] = {"CD_SYSTEM", "CD_COURIER", "CD_TIMES_ROMAN", "CD_HELVETICA"};

int fFontStyle(Ihandle *self, char *t, int o, int v)
{
  ignore(t);
  ignore(self);
  if (v == 0) return IUP_DEFAULT;
  switch (o) {
    case 1:
      ctgc.font_style = CD_PLAIN;
      break;
    case 2:
      ctgc.font_style = CD_BOLD;
      break;
    case 3:
      ctgc.font_style = CD_ITALIC;
      break;
    case 4:
      ctgc.font_style = CD_BOLD_ITALIC;
      break;
  }
  ctgc.font_size = IupGetInt(IupGetHandle("txtFontSize"), IUP_VALUE);
  sprintf(ctgc.status_line, "cdFont(%s, %s, %d)", font_face[ctgc.font_typeface], font_style[ctgc.font_style], ctgc.font_size);
  set_status();
  return IUP_DEFAULT;
}


int fFontTypeFace(Ihandle *self, char *t, int o, int v)
{
  ignore(t);
  ignore(self);
  if (v == 0) return IUP_DEFAULT;
  switch (o) {
    case 1:
      ctgc.font_typeface = CD_SYSTEM;
      break;
    case 2:
      ctgc.font_typeface = CD_COURIER;
      break;
    case 3:
      ctgc.font_typeface = CD_TIMES_ROMAN;
      break;
    case 4:
      ctgc.font_typeface = CD_HELVETICA;
      break;
  }
  ctgc.font_size = IupGetInt(IupGetHandle("txtFontSize"), IUP_VALUE);
  sprintf(ctgc.status_line, "cdFont(%s, %s, %d)", font_face[ctgc.font_typeface], font_style[ctgc.font_style], ctgc.font_size);
  set_status();
  return IUP_DEFAULT;
}

int fTextAlignment(Ihandle *self, char *t, int o, int v)
{
  ignore(t);
  ignore(self);
  if (v == 0) return IUP_DEFAULT;
  switch (o) {
    case 1:
      ctgc.text_alignment = CD_NORTH;
	  sprintf(ctgc.status_line, "cdTextAlignment(CD_NORTH)");
      break;
    case 2 :
      ctgc.text_alignment = CD_SOUTH;
	  sprintf(ctgc.status_line, "cdTextAlignment(CD_SOUTH)");
      break;
    case 3:
      ctgc.text_alignment = CD_EAST;
	  sprintf(ctgc.status_line, "cdTextAlignment(CD_EAST)");
      break;
    case 4:
      ctgc.text_alignment = CD_WEST;
	  sprintf(ctgc.status_line, "cdTextAlignment(CD_WEST)");
      break;
    case 5:
      ctgc.text_alignment = CD_NORTH_EAST;
	  sprintf(ctgc.status_line, "cdTextAlignment(CD_NORTH_EAST)");
      break;
    case 6:
      ctgc.text_alignment = CD_NORTH_WEST;
	  sprintf(ctgc.status_line, "cdTextAlignment(CD_NORTH_WEST)");
      break;
    case 7:
      ctgc.text_alignment = CD_SOUTH_EAST;
	  sprintf(ctgc.status_line, "cdTextAlignment(CD_SOUTH_EAST)");
      break;
    case 8:
      ctgc.text_alignment = CD_SOUTH_WEST;
	  sprintf(ctgc.status_line, "cdTextAlignment(CD_SOUTH_WEST)");
      break;
    case 9:
      ctgc.text_alignment = CD_CENTER;
	  sprintf(ctgc.status_line, "cdTextAlignment(CD_CENTER)");
      break;
    case 10:
      ctgc.text_alignment = CD_BASE_LEFT;
	  sprintf(ctgc.status_line, "cdTextAlignment(CD_BASE_LEFT)");
      break;
    case 11:
      ctgc.text_alignment = CD_BASE_CENTER;
	  sprintf(ctgc.status_line, "cdTextAlignment(CD_BASE_CENTER)");
      break;
    case 12:
      ctgc.text_alignment = CD_BASE_RIGHT;
	  sprintf(ctgc.status_line, "cdTextAlignment(CD_BASE_RIGHT)");
      break;
  }
  set_status();
  return IUP_DEFAULT;
}

int fHatchStyle(Ihandle *self, char *t, int o, int v)
{
  ignore(t);
  ignore(self);
  if (v == 0) return IUP_DEFAULT;
  switch (o) {
    case 1:
      ctgc.hatch = CD_HORIZONTAL;
	  sprintf(ctgc.status_line, "cdHatch(CD_HORIZONTAL)");
      break;
    case 2 :
      ctgc.hatch = CD_VERTICAL;
	  sprintf(ctgc.status_line, "cdHatch(CD_VERTICAL)");
      break;
    case 3:
      ctgc.hatch = CD_FDIAGONAL;
	  sprintf(ctgc.status_line, "cdHatch(CD_FDIAGONAL)");
      break;
    case 4:
      ctgc.hatch = CD_BDIAGONAL;
	  sprintf(ctgc.status_line, "cdHatch(CD_BDIAGONAL)");
      break;
    case 5:
      ctgc.hatch = CD_CROSS;
	  sprintf(ctgc.status_line, "cdHatch(CD_CROSS)");
      break;
    case 6:
      ctgc.hatch = CD_DIAGCROSS;
	  sprintf(ctgc.status_line, "cdHatch(CD_DIAGCROSS)");
      break;
  }
  set_status();
  cdHatch(ctgc.hatch);
  return IUP_DEFAULT;
}

/*-------------------------------------------------------------------------*/
/* Atualiza a linha de estatus do CDTest.                                  */
/*-------------------------------------------------------------------------*/
void set_status(void) 
{
  IupSetAttribute(IupGetHandle("lbStatusLine"), IUP_TITLE, ctgc.status_line);
}

/*-------------------------------------------------------------------------*/
/* Atualiza a posiccao do mouse no dialogo principal.                      */
/*-------------------------------------------------------------------------*/
void mouse_pos(int x, int y) 
{
  /* salva a posiccao do mouse no contexto */
  ctgc.x = x;
  ctgc.y = y;

  /* atualiza a posiccao do mouse no feedback para o usuario */
  sprintf(ctgc.mouse_pos, "(%4d,%4d)", x, y);
  IupSetAttribute(IupGetHandle("lbMousePos"), IUP_TITLE, ctgc.mouse_pos);
}

/*-------------------------------------------------------------------------*/
/* Mostra a caixa de dialogo corrente.                                     */
/*-------------------------------------------------------------------------*/
int fShowDialog(void)
{
  if (ctgc.dlg_cur_prim != NULL) {
    IupShowXY(ctgc.dlg_cur_prim, ctgc.dlg_x, ctgc.dlg_y);
  }
  return IUP_DEFAULT;
}

/*-------------------------------------------------------------------------*/
/* Ativa a seleccao de area de clip.                                       */
/*-------------------------------------------------------------------------*/
int fClip(Ihandle* self)
{
  if (ctgc.cur_prim == POLY) {
    fButtonCB(NULL, IUP_BUTTON3, 0, 0, 0, 0);
  }
  IupSetAttribute(ctgc.bt_cur_prim, IUP_IMAGE, "imgClip");
  ctgc.following = FALSE;
  ctgc.cur_prim = CLIP;
  ctgc.visible = iscurvisible();
  IupHide(ctgc.dlg_cur_prim);
  if (ctgc.visible)
  {
    ctgc.dlg_x = IupGetInt(ctgc.dlg_cur_prim, IUP_X);
    ctgc.dlg_y = IupGetInt(ctgc.dlg_cur_prim, IUP_Y);
  }
  ctgc.dlg_cur_prim = IupGetHandle("dlgClip");
  sprintf(ctgc.status_line, "LEFT click and drag to set. RIGHT clip to disable.");
  set_status();
  if (ctgc.visible || IupGetAttribute(self, "ISMENU"))
    IupShowXY(ctgc.dlg_cur_prim, ctgc.dlg_x, ctgc.dlg_y);
  return IUP_DEFAULT;
}

/*-------------------------------------------------------------------------*/
/* Atualiza a area de clipping.                                            */
/*-------------------------------------------------------------------------*/
int fClipArea(void)
{
  ctgc.clip_mode = CD_CLIPAREA;
  ctgc.clip_xmin = IupGetInt(IupGetHandle("txtClipXmin"), IUP_VALUE);
  ctgc.clip_xmax = IupGetInt(IupGetHandle("txtClipXmax"), IUP_VALUE);
  ctgc.clip_ymin = IupGetInt(IupGetHandle("txtClipYmin"), IUP_VALUE);
  ctgc.clip_ymax = IupGetInt(IupGetHandle("txtClipYmax"), IUP_VALUE);
  cdClipArea(ctgc.clip_xmin,ctgc.clip_xmax,ctgc.clip_ymin,ctgc.clip_ymax);
  cdClip(CD_CLIPAREA);
  sprintf(ctgc.status_line, "cdClipArea( %d, %d, %d, %d)", 
          ctgc.clip_xmin,ctgc.clip_xmax,ctgc.clip_ymin,ctgc.clip_ymax);
  set_status();
  updatecanvas();
  return IUP_DEFAULT;
}

/*-------------------------------------------------------------------------*/
/* Atualiza poligono de clipping.                                          */
/*-------------------------------------------------------------------------*/
int fClipPoly(void)
{
  ctgc.clip_mode=CD_CLIPPOLYGON;
  cdClip(CD_CLIPPOLYGON);
  sprintf(ctgc.status_line, "cdClip(CD_CLIPPOLYGON)");
  set_status();
  updatecanvas();
  return IUP_DEFAULT;
}

/*-------------------------------------------------------------------------*/
/* Desativa o Clipping.                                                    */
/*-------------------------------------------------------------------------*/
int fClipOff(void)
{
  ctgc.clip_mode = CD_CLIPOFF;
  cdClip(CD_CLIPOFF);
  sprintf(ctgc.status_line, "cdClip(CD_CLIPOFF)"); 
  set_status();
  updatecanvas();
  return IUP_DEFAULT;
}

/*-------------------------------------------------------------------------*/
/* Prepara a seleccao de imagens.                                          */
/*-------------------------------------------------------------------------*/
int fImage(Ihandle* self)
{
  if (ctgc.cur_prim == POLY) {
    fButtonCB(NULL, IUP_BUTTON3, 0, 0, 0, 0);
  }
  IupSetAttribute(ctgc.bt_cur_prim, IUP_IMAGE, "imgImage");
  ctgc.cur_prim = IMAGE;
  ctgc.following = FALSE;
  ctgc.visible = iscurvisible();
  IupHide(ctgc.dlg_cur_prim);
  if (ctgc.visible)
  {
    ctgc.dlg_x = IupGetInt(ctgc.dlg_cur_prim, IUP_X);
    ctgc.dlg_y = IupGetInt(ctgc.dlg_cur_prim, IUP_Y);
  }
  ctgc.dlg_cur_prim = IupGetHandle("dlgImage");
  sprintf(ctgc.status_line, "LEFT click and drag to get. RIGHT click to put.");
  set_status();
  if (ctgc.visible || IupGetAttribute(self, "ISMENU"))
    IupShowXY(ctgc.dlg_cur_prim, ctgc.dlg_x, ctgc.dlg_y);
  return IUP_DEFAULT;
}

/*-------------------------------------------------------------------------*/
/* Pega a imagem.                                                          */
/*-------------------------------------------------------------------------*/
int fImageGet(void)
{
  int x, y, width, height;

  /* mata a image */
  if (ctgc.test_image != NULL) {
    cdKillImage(ctgc.test_image);
  }

  x = IupGetInt(IupGetHandle("txtImageX"), IUP_VALUE);
  y = IupGetInt(IupGetHandle("txtImageY"), IUP_VALUE);

  width = IupGetInt(IupGetHandle("txtImageW"), IUP_VALUE);
  height = IupGetInt(IupGetHandle("txtImageH"), IUP_VALUE);

  if (width*height != 0) 
  {
    if (ctgc.buffering) 
      cdActivate(ctgc.buffer_canvas);
    else 
      cdActivate(ctgc.iup_canvas);

    ctgc.test_image = cdCreateImage(width, height);
    if (ctgc.test_image != NULL) 
    {
      cdGetImage(ctgc.test_image, x, y);
      sprintf(ctgc.status_line, "cdGetImage( image, %d, %d)", x, y); 
      set_status();
    }

    cdActivate(ctgc.iup_canvas);
  }
  return IUP_DEFAULT;
}

/*-------------------------------------------------------------------------*/
/* Coloca a imagem.                                                        */
/*-------------------------------------------------------------------------*/
int fImagePut(void)
{
  int x, y;

  x = IupGetInt(IupGetHandle("txtImageX"), IUP_VALUE);
  y = IupGetInt(IupGetHandle("txtImageY"), IUP_VALUE);

  if (ctgc.test_image != NULL) 
  {
    if (ctgc.buffering) 
    {
      cdActivate(ctgc.iup_canvas);
      cdClip(CD_CLIPOFF);

      cdActivate(ctgc.buffer_canvas);
    }
    else 
      cdActivate(ctgc.iup_canvas);

    cdWriteMode(ctgc.write_mode);
    cdPutImage(ctgc.test_image, x, y);
    sprintf(ctgc.status_line, "cdPutImage( image, %d, %d)", x, y); 
    set_status();

    if (ctgc.buffering) 
    {
      cdFlush();

      cdActivate(ctgc.iup_canvas);
      cdClip(ctgc.clip_mode);
    }
  }
  return IUP_DEFAULT;
}

/*-------------------------------------------------------------------------*/
/* Prepara a seleccao de imagens.                                          */
/*-------------------------------------------------------------------------*/
int fImageRGB(Ihandle* self)
{
  if (ctgc.cur_prim == POLY) {
    fButtonCB(NULL, IUP_BUTTON3, 0, 0, 0, 0);
  }
  IupSetAttribute(ctgc.bt_cur_prim, IUP_IMAGE, "imgImageRGB");
  ctgc.cur_prim = RGB;
  ctgc.following = FALSE;
  ctgc.visible = iscurvisible();
  IupHide(ctgc.dlg_cur_prim);
  if (ctgc.visible)
  {
    ctgc.dlg_x = IupGetInt(ctgc.dlg_cur_prim, IUP_X);
    ctgc.dlg_y = IupGetInt(ctgc.dlg_cur_prim, IUP_Y);
  }
  ctgc.dlg_cur_prim = IupGetHandle("dlgImageRGB");
  sprintf(ctgc.status_line, "LEFT click and drag to get. RIGHT click to put.");
  set_status();
  if (ctgc.visible || IupGetAttribute(self, "ISMENU"))
    IupShowXY(ctgc.dlg_cur_prim, ctgc.dlg_x, ctgc.dlg_y);
  return IUP_DEFAULT;
}

/*-------------------------------------------------------------------------*/
/* Pega a imagem.                                                          */
/*-------------------------------------------------------------------------*/
int fImageRGBGet(void)
{
  int x, y;

  x = IupGetInt(IupGetHandle("txtImageRGBX"), IUP_VALUE);
  y = IupGetInt(IupGetHandle("txtImageRGBY"), IUP_VALUE);
  ctgc.rgb_w = IupGetInt(IupGetHandle("txtImageRGBW"), IUP_VALUE);
  ctgc.rgb_h = IupGetInt(IupGetHandle("txtImageRGBH"), IUP_VALUE);

  if (ctgc.red) free(ctgc.red);
  ctgc.red   = (unsigned char *) malloc (ctgc.rgb_w*ctgc.rgb_h*sizeof(unsigned char));
  if (ctgc.green) free(ctgc.green);
  ctgc.green = (unsigned char *) malloc (ctgc.rgb_w*ctgc.rgb_h*sizeof(unsigned char));
  if (ctgc.blue) free(ctgc.blue);
  ctgc.blue  = (unsigned char *) malloc (ctgc.rgb_w*ctgc.rgb_h*sizeof(unsigned char));

  if ((ctgc.red!=NULL)&&(ctgc.green!=NULL)&&(ctgc.blue!=NULL)) {
    sprintf(ctgc.status_line, "cdGetImageRGB( r, g, b, %d, %d, %d, %d)",
      x, y, ctgc.rgb_w, ctgc.rgb_h); 
    set_status();

    if (ctgc.buffering) 
      cdActivate(ctgc.buffer_canvas);
    else 
      cdActivate(ctgc.iup_canvas);

    cdGetImageRGB(ctgc.red, ctgc.green, ctgc.blue, x, y, ctgc.rgb_w, ctgc.rgb_h);

    cdActivate(ctgc.iup_canvas);
  }
  return IUP_DEFAULT;
}

/*-------------------------------------------------------------------------*/
/* Joga a imagem na tela.                                                  */
/*-------------------------------------------------------------------------*/
int fImageRGBPut(void)
{
  int x, y;
  
  x = IupGetInt(IupGetHandle("txtImageRGBX"), IUP_VALUE);
  y = IupGetInt(IupGetHandle("txtImageRGBY"), IUP_VALUE);

  if ((ctgc.red!=NULL)&&(ctgc.green!=NULL)&&(ctgc.blue!=NULL)) 
  {
    if (ctgc.buffering)
    {
      cdActivate(ctgc.iup_canvas);
      cdClip(CD_CLIPOFF);

      cdActivate(ctgc.buffer_canvas);
    }
    else 
      cdActivate(ctgc.iup_canvas);

    cdWriteMode(ctgc.write_mode);
    sprintf(ctgc.status_line, "cdPutImageRGB( %d, %d, r, g, b, %d, %d, %d, %d)",
      ctgc.rgb_w, ctgc.rgb_h, x, y, ctgc.rgb_w, ctgc.rgb_h); 
    set_status();
    cdPutImageRGB(ctgc.rgb_w, ctgc.rgb_h, ctgc.red, ctgc.green, ctgc.blue, x, y, ctgc.rgb_w, ctgc.rgb_h);

    if (ctgc.buffering)
    {
      cdFlush();

      cdActivate(ctgc.iup_canvas);
      cdClip(ctgc.clip_mode);
    }
  }
  return IUP_DEFAULT;
}

/*-------------------------------------------------------------------------*/
/* Muda a primitiva corrente.                                              */
/*-------------------------------------------------------------------------*/
int fLine(Ihandle* self)
{
  if (ctgc.cur_prim != LINE) {
    IupSetAttribute(ctgc.bt_cur_prim, IUP_IMAGE, "imgLine");
    if (ctgc.cur_prim == POLY) { /* termina o poligono em andamento */
      fButtonCB(NULL, IUP_BUTTON3, 0, 0, 0, 0);
    }
    ctgc.cur_prim = LINE;
    ctgc.following = FALSE;
    ctgc.visible = iscurvisible();
    IupHide(ctgc.dlg_cur_prim);      /* esconde o dialogo anterior */
    if (ctgc.visible)
    {
      ctgc.dlg_x = IupGetInt(ctgc.dlg_cur_prim, IUP_X);
      ctgc.dlg_y = IupGetInt(ctgc.dlg_cur_prim, IUP_Y);
    }
    ctgc.dlg_cur_prim = IupGetHandle("dlgLB");
    IupSetAttribute(ctgc.dlg_cur_prim, IUP_TITLE, "Line Parameters");
  }
  sprintf(ctgc.status_line, "LEFT click and drag.");
  set_status();
  if (ctgc.visible || IupGetAttribute(self, "ISMENU"))
    IupShowXY(ctgc.dlg_cur_prim, ctgc.dlg_x, ctgc.dlg_y);
  return IUP_DEFAULT;
}

/*-------------------------------------------------------------------------*/
/* Muda a primitiva corrente.                                              */
/*-------------------------------------------------------------------------*/
int fRect(Ihandle* self)
{
  if (ctgc.cur_prim != RECT) {
    IupSetAttribute(ctgc.bt_cur_prim, IUP_IMAGE, "imgRect");
    if (ctgc.cur_prim == POLY) {
      fButtonCB(NULL, IUP_BUTTON3, 0, 0, 0, 0);
    }
    ctgc.cur_prim = RECT;
    ctgc.following = FALSE;
    ctgc.visible = iscurvisible();
    IupHide(ctgc.dlg_cur_prim);
    if (ctgc.visible)
    {
      ctgc.dlg_x = IupGetInt(ctgc.dlg_cur_prim, IUP_X);
      ctgc.dlg_y = IupGetInt(ctgc.dlg_cur_prim, IUP_Y);
    }
    ctgc.dlg_cur_prim = IupGetHandle("dlgLB");
    IupSetAttribute(ctgc.dlg_cur_prim, IUP_TITLE, "Rect Parameters");
  }
  sprintf(ctgc.status_line, "LEFT click and drag.");
  set_status();
  if (ctgc.visible || IupGetAttribute(self, "ISMENU"))
    IupShowXY(ctgc.dlg_cur_prim, ctgc.dlg_x, ctgc.dlg_y);
  return IUP_DEFAULT;
}

int fBox(Ihandle* self)
{
  if (ctgc.cur_prim != BOX) {
    IupSetAttribute(ctgc.bt_cur_prim, IUP_IMAGE, "imgBox");
    if (ctgc.cur_prim == POLY) {
      fButtonCB(NULL, IUP_BUTTON3, 0, 0, 0, 0);
    }
    ctgc.cur_prim = BOX;
    ctgc.following = FALSE;
    ctgc.visible = iscurvisible();
    IupHide(ctgc.dlg_cur_prim);
    if (ctgc.visible)
    {
      ctgc.dlg_x = IupGetInt(ctgc.dlg_cur_prim, IUP_X);
      ctgc.dlg_y = IupGetInt(ctgc.dlg_cur_prim, IUP_Y);
    }
    ctgc.dlg_cur_prim = IupGetHandle("dlgLB");
    IupSetAttribute(ctgc.dlg_cur_prim, IUP_TITLE, "Box Parameters");
  }
  sprintf(ctgc.status_line, "LEFT click and drag.");
  set_status();
  if (ctgc.visible || IupGetAttribute(self, "ISMENU"))
    IupShowXY(ctgc.dlg_cur_prim, ctgc.dlg_x, ctgc.dlg_y);
  return IUP_DEFAULT;
}

/*-------------------------------------------------------------------------*/
/* Muda a primitiva corrente.                                              */
/*-------------------------------------------------------------------------*/
int fArc(Ihandle* self)
{
  if (ctgc.cur_prim != ARC) {
    IupSetAttribute(ctgc.bt_cur_prim, IUP_IMAGE, "imgArc");
    if (ctgc.cur_prim == POLY) {
      fButtonCB(NULL, IUP_BUTTON3, 0, 0, 0, 0);
    }
    ctgc.cur_prim = ARC;
    ctgc.following = FALSE;
    ctgc.visible = iscurvisible();
    IupHide(ctgc.dlg_cur_prim);
    if (ctgc.visible)
    {
      ctgc.dlg_x = IupGetInt(ctgc.dlg_cur_prim, IUP_X);
      ctgc.dlg_y = IupGetInt(ctgc.dlg_cur_prim, IUP_Y);
    }
    ctgc.dlg_cur_prim = IupGetHandle("dlgAS");
    IupSetAttribute(ctgc.dlg_cur_prim, IUP_TITLE, "Arc Parameters");
  }
  sprintf(ctgc.status_line, "LEFT click at center and drag.");
  set_status();
  if (ctgc.visible || IupGetAttribute(self, "ISMENU"))
    IupShowXY(ctgc.dlg_cur_prim, ctgc.dlg_x, ctgc.dlg_y);
  return IUP_DEFAULT;
}

/*-------------------------------------------------------------------------*/
/* Muda a primitiva corrente.                                              */
/*-------------------------------------------------------------------------*/
int fSector(Ihandle* self)
{
  if (ctgc.cur_prim != SECTOR) {
    IupSetAttribute(ctgc.bt_cur_prim, IUP_IMAGE, "imgSector");
    if (ctgc.cur_prim == POLY) {
      fButtonCB(NULL, IUP_BUTTON3, 0, 0, 0, 0);
    }
    ctgc.cur_prim = SECTOR;
    ctgc.following = FALSE;
    ctgc.visible = iscurvisible();
    IupHide(ctgc.dlg_cur_prim);
    if (ctgc.visible)
    {
      ctgc.dlg_x = IupGetInt(ctgc.dlg_cur_prim, IUP_X);
      ctgc.dlg_y = IupGetInt(ctgc.dlg_cur_prim, IUP_Y);
    }
    ctgc.dlg_cur_prim = IupGetHandle("dlgAS");
    IupSetAttribute(ctgc.dlg_cur_prim, IUP_TITLE, "Sector Parameters");
  }
  sprintf(ctgc.status_line, "LEFT click at center and drag.");
  set_status();
  if (ctgc.visible || IupGetAttribute(self, "ISMENU"))
    IupShowXY(ctgc.dlg_cur_prim, ctgc.dlg_x, ctgc.dlg_y);
  return IUP_DEFAULT;
}
   
int fChord(Ihandle* self)
{
  if (ctgc.cur_prim != CHORD) {
    IupSetAttribute(ctgc.bt_cur_prim, IUP_IMAGE, "imgChord");
    if (ctgc.cur_prim == POLY) {
      fButtonCB(NULL, IUP_BUTTON3, 0, 0, 0, 0);
    }
    ctgc.cur_prim = CHORD;
    ctgc.following = FALSE;
    ctgc.visible = iscurvisible();
    IupHide(ctgc.dlg_cur_prim);
    if (ctgc.visible)
    {
      ctgc.dlg_x = IupGetInt(ctgc.dlg_cur_prim, IUP_X);
      ctgc.dlg_y = IupGetInt(ctgc.dlg_cur_prim, IUP_Y);
    }
    ctgc.dlg_cur_prim = IupGetHandle("dlgAS");
    IupSetAttribute(ctgc.dlg_cur_prim, IUP_TITLE, "Chord Parameters");
  }
  sprintf(ctgc.status_line, "LEFT click at center and drag.");
  set_status();
  if (ctgc.visible || IupGetAttribute(self, "ISMENU"))
    IupShowXY(ctgc.dlg_cur_prim, ctgc.dlg_x, ctgc.dlg_y);
  return IUP_DEFAULT;
}
   
/*-------------------------------------------------------------------------*/
/* Muda a primitiva corrente.                                              */
/*-------------------------------------------------------------------------*/
int fPixel(Ihandle* self)
{
  if (ctgc.cur_prim != PIXEL) {
    IupSetAttribute(ctgc.bt_cur_prim, IUP_IMAGE, "imgPixel");
    if (ctgc.cur_prim == POLY) {
      fButtonCB(NULL, IUP_BUTTON3, 0, 0, 0, 0);
    }
    ctgc.cur_prim = PIXEL;
    ctgc.following = FALSE;
    ctgc.visible = iscurvisible();
    IupHide(ctgc.dlg_cur_prim);
    if (ctgc.visible)
    {
      ctgc.dlg_x = IupGetInt(ctgc.dlg_cur_prim, IUP_X);
      ctgc.dlg_y = IupGetInt(ctgc.dlg_cur_prim, IUP_Y);
    }
    ctgc.dlg_cur_prim = IupGetHandle("dlgPixel");
  }
  sprintf(ctgc.status_line, "LEFT click.");
  set_status();
  if (ctgc.visible || IupGetAttribute(self, "ISMENU"))
    IupShowXY(ctgc.dlg_cur_prim, ctgc.dlg_x, ctgc.dlg_y);
  return IUP_DEFAULT;
}

/*-------------------------------------------------------------------------*/
/* Muda a primitiva corrente.                                              */
/*-------------------------------------------------------------------------*/
int fMark(Ihandle* self)
{
  if (ctgc.cur_prim != MARK) {
    IupSetAttribute(ctgc.bt_cur_prim, IUP_IMAGE, "imgMark");
    if (ctgc.cur_prim == POLY) {
      fButtonCB(NULL, IUP_BUTTON3, 0, 0, 0, 0);
    }
    ctgc.cur_prim = MARK;
    ctgc.following = FALSE;
    ctgc.visible = iscurvisible();
    IupHide(ctgc.dlg_cur_prim);
    if (ctgc.visible)
    {
      ctgc.dlg_x = IupGetInt(ctgc.dlg_cur_prim, IUP_X);
      ctgc.dlg_y = IupGetInt(ctgc.dlg_cur_prim, IUP_Y);
    }
    ctgc.dlg_cur_prim = IupGetHandle("dlgMark");
  }
  sprintf(ctgc.status_line, "LEFT click.");
  set_status();
  if (ctgc.visible || IupGetAttribute(self, "ISMENU"))
    IupShowXY(ctgc.dlg_cur_prim, ctgc.dlg_x, ctgc.dlg_y);
  return IUP_DEFAULT;
}

/*-------------------------------------------------------------------------*/
/* Muda a primitiva corrente.                                              */
/*-------------------------------------------------------------------------*/
int fText(Ihandle* self)
{
  if (ctgc.cur_prim != TEXT) {
    IupSetAttribute(ctgc.bt_cur_prim, IUP_IMAGE, "imgText");
    if (ctgc.cur_prim == POLY) {
      fButtonCB(NULL, IUP_BUTTON3, 0, 0, 0, 0);
    }
    ctgc.cur_prim = TEXT;
    ctgc.following = FALSE;
    ctgc.visible = iscurvisible();
    IupHide(ctgc.dlg_cur_prim);
    if (ctgc.visible)
    {
      ctgc.dlg_x = IupGetInt(ctgc.dlg_cur_prim, IUP_X);
      ctgc.dlg_y = IupGetInt(ctgc.dlg_cur_prim, IUP_Y);
    }
    ctgc.dlg_cur_prim = IupGetHandle("dlgText");
  }
  sprintf(ctgc.status_line, "LEFT click.");
  set_status();
  if (ctgc.visible || IupGetAttribute(self, "ISMENU"))
    IupShowXY(ctgc.dlg_cur_prim, ctgc.dlg_x, ctgc.dlg_y);
  return IUP_DEFAULT;
}

/*-------------------------------------------------------------------------*/
/* Muda a primitiva corrente.                                              */
/*-------------------------------------------------------------------------*/
int fPoly(Ihandle* self)
{
  if (ctgc.cur_prim != POLY) {
    IupSetAttribute(ctgc.bt_cur_prim, IUP_IMAGE, "imgPoly");
    ctgc.cur_prim = POLY;
    ctgc.following = FALSE;
    ctgc.visible = iscurvisible();
    IupHide(ctgc.dlg_cur_prim);
    if (ctgc.visible)
    {
      ctgc.dlg_x = IupGetInt(ctgc.dlg_cur_prim, IUP_X);
      ctgc.dlg_y = IupGetInt(ctgc.dlg_cur_prim, IUP_Y);
    }
    ctgc.dlg_cur_prim = IupGetHandle("dlgPoly");
    /* zera o buffer temporario de pontos */
    ctgc.num_points = 0; 
  }
  sprintf(ctgc.status_line, "LEFT click to add vertex. RIGHT click to end.");
  set_status();
  if (ctgc.visible || IupGetAttribute(self, "ISMENU"))
    IupShowXY(ctgc.dlg_cur_prim, ctgc.dlg_x, ctgc.dlg_y);
  return IUP_DEFAULT;
}

int fOptions(void)
{
  IupShow(IupGetHandle("dlgOptions"));
  return IUP_DEFAULT;
}

int fOptionsHide(void)
{
  IupHide(IupGetHandle("dlgOptions"));
  return IUP_DEFAULT;
}


/*-------------------------------------------------------------------------*/
/* Mostra a caixa de dialogo Attributes.                                   */
/*-------------------------------------------------------------------------*/
int fAttributes(void)
{
  IupShow(IupGetHandle("dlgAttributes"));
  return IUP_DEFAULT;
}

/*-------------------------------------------------------------------------*/
/* Esconde o dialogo de Attributes.                                        */
/*-------------------------------------------------------------------------*/
int fAttributesHide(void)
{
  ctgc.font_size = IupGetInt(IupGetHandle("txtFontSize"), IUP_VALUE);
  cdFont(ctgc.font_typeface,ctgc.font_style,ctgc.font_size);
  IupHide(IupGetHandle("dlgAttributes"));
  return IUP_DEFAULT;
}

/*-------------------------------------------------------------------------*/
/* Esconde o dialogo de primitiva corrente.                                */
/*-------------------------------------------------------------------------*/
int fMsgHide(void)
{
  IupHide(ctgc.dlg_cur_prim);
  ctgc.dlg_x = IupGetInt(ctgc.dlg_cur_prim, IUP_X);
  ctgc.dlg_y = IupGetInt(ctgc.dlg_cur_prim, IUP_Y);
  return IUP_DEFAULT;
}

/*-------------------------------------------------------------------------*/
/* Filtra inteiros.                                                        */
/*-------------------------------------------------------------------------*/
int fInteger(Ihandle *self, int c)
{
  ignore(self);
  if (isdigit(c) || c == '-') 
    return IUP_DEFAULT;
  else if (c) 
    return IUP_IGNORE;
  else 
    return IUP_DEFAULT;
}

/*-------------------------------------------------------------------------*/
/* Filtra reais.                                                           */
/*-------------------------------------------------------------------------*/
int fReal(Ihandle *self, int c)
{
  ignore(self);
  if (isdigit(c)) 
    return IUP_DEFAULT;
  else if ((c=='.') || (c=='e') || (c=='E') || (c=='+') || (c=='-')) 
    return IUP_DEFAULT;
  else if (c) 
    return IUP_IGNORE;
  else 
    return IUP_DEFAULT;
}

/*-------------------------------------------------------------------------*/
/* Desenha a primitiva entrada na caixa de dialogo ativa.                  */
/*-------------------------------------------------------------------------*/
void draw(void)
{
  int a, b;

  IupSetAttribute(IupGetHandle("itEditUndo"), IUP_ACTIVE, IUP_YES);
  ctgc.line_width = IupGetInt(IupGetHandle("txtLineWidth"), IUP_VALUE);
  if (ctgc.line_width < 1) ctgc.line_width = 1;
  ctgc.font_size = IupGetInt(IupGetHandle("txtFontSize"), IUP_VALUE);

  /* escolhe entre o canvas na tela e o off-screen */
  if (ctgc.buffering) 
  {
    cdActivate(ctgc.iup_canvas);
    cdClip(CD_CLIPOFF);

    cdActivate(ctgc.buffer_canvas);
  }
  else 
    cdActivate(ctgc.iup_canvas);

  /* atualiza os atributos */
  cdWriteMode(ctgc.write_mode);
  cdLineStyle(ctgc.line_style);
  cdLineCap(ctgc.line_cap);
  cdLineJoin(ctgc.line_join);
  cdLineWidth(ctgc.line_width);
  cdForeground(ctgc.foreground);
  cdBackground(ctgc.background);
  cdHatch(ctgc.hatch);
  cdFillMode(ctgc.fill_mode);
  cdInteriorStyle(ctgc.interior_style);
  cdBackOpacity(ctgc.back_opacity);

  cdClipArea(ctgc.clip_xmin,ctgc.clip_xmax,ctgc.clip_ymin,ctgc.clip_ymax);
  cdClip(ctgc.clip_mode);

  switch (ctgc.cur_prim) {
    case LINE:
      /* atualiza a linha de status */
      sprintf(ctgc.status_line, "cdLine( %d, %d, %d, %d)", line_pos.x1, 
        line_pos.y1, line_pos.x2, line_pos.y2);
      set_status();
      /* desenha a line na tela */
      cdLine(line_pos.x1, line_pos.y1, line_pos.x2, line_pos.y2);
      /* arquiva a line */
      newline(line_pos.x1, line_pos.y1, line_pos.x2, line_pos.y2); 
      break;
    case RECT:
      /* atualiza a linha de status */
      sprintf(ctgc.status_line,"cdRect( %d, %d, %d, %d)",box_pos.xmin, 
        box_pos.xmax, box_pos.ymin, box_pos.ymax);
      set_status();
      /* desenha a box na tela */
      cdRect(box_pos.xmin, box_pos.xmax, box_pos.ymin, box_pos.ymax);
      /* armazena a box */
      newrect(box_pos.xmin, box_pos.xmax, box_pos.ymin, box_pos.ymax); 
      break;
    case BOX:
      /* atualiza a linha de status */
      sprintf(ctgc.status_line,"cdBox( %d, %d, %d, %d)",box_pos.xmin, 
        box_pos.xmax, box_pos.ymin, box_pos.ymax);
      set_status();
      /* desenha a box na tela */
      cdBox(box_pos.xmin, box_pos.xmax, box_pos.ymin, box_pos.ymax);
      /* armazena a box */
      newbox(box_pos.xmin, box_pos.xmax, box_pos.ymin, box_pos.ymax); 
      break;
    case ARC:
      arc_pos.angle1 = IupGetFloat(IupGetHandle("txtASAngle1"),IUP_VALUE);
      arc_pos.angle2 = IupGetFloat(IupGetHandle("txtASAngle2"),IUP_VALUE);
      /* atualiza a linha de status */
      sprintf(ctgc.status_line,"cdArc( %d, %d, %d, %d, %.5G, %.5G)", arc_pos.xc, 
        arc_pos.yc, arc_pos.w, arc_pos.h, arc_pos.angle1, arc_pos.angle2);
      set_status();
      /* desenha o arc na tela */
      cdArc(arc_pos.xc, arc_pos.yc, arc_pos.w, arc_pos.h, arc_pos.angle1, 
        arc_pos.angle2);
      /* armazena o arc */
      newarc(arc_pos.xc, arc_pos.yc, arc_pos.w, arc_pos.h, arc_pos.angle1, 
        arc_pos.angle2);
      break;
    case SECTOR:
      arc_pos.angle1 = IupGetFloat(IupGetHandle("txtASAngle1"),IUP_VALUE);
      arc_pos.angle2 = IupGetFloat(IupGetHandle("txtASAngle2"),IUP_VALUE);
      /* atualiza a linha de status */
      sprintf(ctgc.status_line,"cdSector( %d, %d, %d, %d, %.5G, %.5G)", arc_pos.xc, 
        arc_pos.yc, arc_pos.w, arc_pos.h, arc_pos.angle1, arc_pos.angle2);
      set_status();
      /* desenha o sector na tela */
      cdSector(arc_pos.xc, arc_pos.yc, arc_pos.w, arc_pos.h, arc_pos.angle1, 
        arc_pos.angle2);
      /* armazena o sector */
      newsector(arc_pos.xc, arc_pos.yc, arc_pos.w, arc_pos.h, arc_pos.angle1, 
        arc_pos.angle2);
      break;
    case CHORD:
      arc_pos.angle1 = IupGetFloat(IupGetHandle("txtASAngle1"),IUP_VALUE);
      arc_pos.angle2 = IupGetFloat(IupGetHandle("txtASAngle2"),IUP_VALUE);
      /* atualiza a linha de status */
      sprintf(ctgc.status_line,"cdChord( %d, %d, %d, %d, %.5G, %.5G)", arc_pos.xc, 
        arc_pos.yc, arc_pos.w, arc_pos.h, arc_pos.angle1, arc_pos.angle2);
      set_status();
      /* desenha o sector na tela */
      cdChord(arc_pos.xc, arc_pos.yc, arc_pos.w, arc_pos.h, arc_pos.angle1, 
        arc_pos.angle2);
      /* armazena o sector */
      newchord(arc_pos.xc, arc_pos.yc, arc_pos.w, arc_pos.h, arc_pos.angle1, 
        arc_pos.angle2);
      break;
    case PIXEL:
      /* atualiza a linha de status */
      sprintf(ctgc.status_line, "cdPixel( %d, %d, 0x%.6lx )",pixel_pos.x, pixel_pos.y,ctgc.foreground);
      set_status();
      /* desenha o pixel na tela */
      cdPixel(pixel_pos.x, pixel_pos.y, ctgc.foreground);
      /* armazena o pixel */
      newpixel(pixel_pos.x, pixel_pos.y);
      break;
    case MARK:
      mark_pos.size = IupGetInt(IupGetHandle("txtMarkSize"),IUP_VALUE);
      /* atualiza a linha de status */
      sprintf(ctgc.status_line,"cdMark( %d, %d)", mark_pos.x, mark_pos.y);
      set_status();
      /* armazena a marca */
      newmark(mark_pos.x, mark_pos.y, mark_pos.size);
      /* desenha a marca na tela */
      cdMarkType(ctgc.mark_type);
      cdMarkSize(mark_pos.size);
      cdMark(mark_pos.x, mark_pos.y);
      break;
    case TEXT:
      if (IupGetAttribute(IupGetHandle("txtTextS"),IUP_VALUE)) {
        a=IupGetInt(IupGetHandle("txtTextX"),IUP_VALUE);
        b=IupGetInt(IupGetHandle("txtTextY"),IUP_VALUE);
        sprintf(ctgc.status_line," cdText( %d, %d, ""%.3s""...)", a, b,
                IupGetAttribute(IupGetHandle("txtTextS"),IUP_VALUE));
        set_status();
        ctgc.text_orientation = IupGetInt(IupGetHandle("txtTextOrientation"),IUP_VALUE);
        newtext(a,b,IupGetAttribute(IupGetHandle("txtTextS"),IUP_VALUE));
        cdFont(ctgc.font_typeface,ctgc.font_style,ctgc.font_size);
        cdTextAlignment(ctgc.text_alignment);
        cdTextOrientation(ctgc.text_orientation);
        cdText(a,b,IupGetAttribute(IupGetHandle("txtTextS"),IUP_VALUE));
      }
    case POLY:
      if (ctgc.num_points > 1) {
        cdBegin(ctgc.poly_mode);
        for (a=0; (a<ctgc.num_points); a++) {
          cdVertex(ctgc.points[a].x,ctgc.points[a].y);
        }
        cdEnd();
        if (ctgc.poly_mode != CD_CLIP)
          newpoly();
        ctgc.num_points = 0;
      }
      break;
    default:
      break;
  }

  if (ctgc.buffering) 
  {
    cdFlush();

    cdActivate(ctgc.iup_canvas);
    cdClip(ctgc.clip_mode);
  }
}

/*-------------------------------------------------------------------------*/
/* Desenha a primitiva entrada na caixa de dialogo corrente.               */
/*-------------------------------------------------------------------------*/
int fDraw(void)
{
  /* atualiza os dados entrados na caixa de dialogo no contexto da */
  /* primitiva corrente */
  switch (ctgc.cur_prim) {
    case LINE:
      line_pos.x1 = IupGetInt(IupGetHandle("txtLBX1"), IUP_VALUE);
      line_pos.x2 = IupGetInt(IupGetHandle("txtLBX2"), IUP_VALUE);
      line_pos.y1 = IupGetInt(IupGetHandle("txtLBY1"), IUP_VALUE);
      line_pos.y2 = IupGetInt(IupGetHandle("txtLBY2"), IUP_VALUE);
      break;
    case RECT:
    case BOX:
      box_pos.xmin = IupGetInt(IupGetHandle("txtLBX1"), IUP_VALUE);
      box_pos.xmax = IupGetInt(IupGetHandle("txtLBX2"), IUP_VALUE);
      box_pos.ymin = IupGetInt(IupGetHandle("txtLBY1"), IUP_VALUE);
      box_pos.ymax = IupGetInt(IupGetHandle("txtLBY2"), IUP_VALUE);
      break;
    case ARC:
    case CHORD:
    case SECTOR:
      arc_pos.xc = IupGetInt(IupGetHandle("txtASXC"), IUP_VALUE);
      arc_pos.yc = IupGetInt(IupGetHandle("txtASYC"), IUP_VALUE);
      arc_pos.w = IupGetInt(IupGetHandle("txtASW"), IUP_VALUE);
      arc_pos.h = IupGetInt(IupGetHandle("txtASH"), IUP_VALUE);
      break;
    case PIXEL:
      pixel_pos.x = IupGetInt(IupGetHandle("txtPixelX"), IUP_VALUE);
      pixel_pos.y = IupGetInt(IupGetHandle("txtPixelY"), IUP_VALUE);
      break;
    case MARK:
      mark_pos.x = IupGetInt(IupGetHandle("txtMarkX"), IUP_VALUE);
      mark_pos.y = IupGetInt(IupGetHandle("txtMarkY"), IUP_VALUE);
      break;
    default:
      break;
  }

  /* efetivamente desenha a primitiva */
  draw();
  
  return IUP_DEFAULT;
}

/*-------------------------------------------------------------------------*/
/* Mostra a janelinha de apresentacao.                                     */
/*-------------------------------------------------------------------------*/
int fHelpAbout(void)
{
  IupSetAttribute(IupGetHandle("lblVersion"), IUP_TITLE, cdVersion());
  IupShow(IupGetHandle("dlgHelpAbout"));
  return IUP_DEFAULT;
}

/*-------------------------------------------------------------------------*/
/* Mata a janelinha de apresentacao.                                       */
/*-------------------------------------------------------------------------*/
int fCloseAbout(void)
{
  IupHide(IupGetHandle("dlgHelpAbout"));
  return IUP_DEFAULT;
}

/*-------------------------------------------------------------------------*/
/* Limpa o canvas e a lista de primitivas.                                 */
/*-------------------------------------------------------------------------*/
int fEditClear(void)
{
  /* mata a lista de primitivas */
  dellist();         

  /* torna inativo a opcap undo */
  IupSetAttribute(IupGetHandle("itEditUndo"), IUP_ACTIVE, IUP_NO);

  updatecanvas();

  sprintf(ctgc.status_line, "cdClear()");
  set_status();

  return IUP_DEFAULT;
}

/*-------------------------------------------------------------------------*/
/* Funcao associada ao ganho de focus do canvas.                           */
/*-------------------------------------------------------------------------*/
int fGetFocusCB(Ihandle *self)
{
  ignore(self);
  /* avisa ao CD que o focus foi recebido, */
  /* para que ele restaure o contexto grafico */
  cdActivate(ctgc.iup_canvas);
  return IUP_DEFAULT;
}

/*-------------------------------------------------------------------------*/
/* Funcao associada ao movimento do mouse.                                 */
/*-------------------------------------------------------------------------*/
int fMotionCB(Ihandle *self, int x, int y, char *r)
{
  ignore(self);
  ignore(r);

  if (!ctgc.iup_canvas)
    return IUP_DEFAULT;

  cdActivate(ctgc.iup_canvas);
  cdUpdateYAxis(&y);
  mouse_pos(x, y);

  if (ctgc.following) {
    switch(ctgc.cur_prim) {
      case LINE:
        line(MOVE, x, y);
        follow(x, y);
        break;
      case RECT:
      case BOX:
        box(MOVE, x, y);
        follow(x, y);
        break;
      case CLIP:
        box(MOVE, x, y);
        follow(x, y);
        break;
      case ARC:    /* ARC e SECTOR... */
      case SECTOR: /* sao equivalentes */
      case CHORD:
        arc(MOVE, x, y);
        follow(x, y);
        break;
      case IMAGE:
        box(MOVE, x, y);
        follow(x, y);
        break;
      case RGB:
        box(MOVE, x, y);
        follow(x, y);
        break;
      case POLY:
        polygon(MOVE, x, y);
        break;
      default:
        break;
    }
  }
  return IUP_DEFAULT;
}

/*-------------------------------------------------------------------------*/
/* Funcao associada ao botao do mouse.                                     */
/*-------------------------------------------------------------------------*/
int fButtonCB(Ihandle *self, int b, int e, int x, int y, char *r)
{
  ignore(self);
  ignore(r);

  cdActivate(ctgc.iup_canvas);
  cdWriteMode(CD_NOT_XOR);
  cdForeground(CD_BLACK);
  cdLineStyle(CD_CONTINUOUS);
  cdLineWidth(1);
  cdClip(CD_CLIPOFF);

  cdUpdateYAxis(&y);
  mouse_pos(x, y);

  if (b == IUP_BUTTON1) {
    if (e) {
      switch(ctgc.cur_prim) {
        case LINE:
          follow(x, y);
          line(NEWPOINT, x, y);
          line_pos.x1 = x;
          line_pos.y1 = y;
          ctgc.following = TRUE;
          break;
        case RECT:
        case BOX:
          follow(x, y);
          box(NEWPOINT, x, y);
          box_pos.x = x;
          box_pos.y = y;
          ctgc.following = TRUE;
          break;
        case CLIP:
          box(NEWPOINT, x, y);
          follow(x, y);
          ctgc.following = TRUE;
          break;
        case ARC:    /* ARC e SECTOR... */
        case SECTOR: /* sao equivalentes */
        case CHORD:
          follow(x, y);
          arc(CENTER, x, y);
          arc_pos.xc = x;
          arc_pos.yc = y;
          ctgc.following = TRUE;
          break;
        case MARK:
        case PIXEL:
        case TEXT:
          follow(x, y);
          draw();
          break;
        case IMAGE:
          follow(x, y);
          box(NEWPOINT, x, y);
          ctgc.following = TRUE;
          break;
        case RGB:
          follow(x, y);
          box(NEWPOINT, x, y);
          ctgc.following = TRUE;
          break;
        default:
          break;
      }
    }
    else {
      switch(ctgc.cur_prim) {
        case LINE:
          if (ctgc.following) {
            ctgc.following = FALSE;
            line(CLOSE, x, y);
            cdClip(ctgc.clip_mode);
            draw();
          }
          break;
        case RECT:
        case BOX:
          if (ctgc.following) {
            ctgc.following = FALSE;
            box(CLOSE, x, y);
            cdClip(ctgc.clip_mode);
            draw();
          }
          break;
        case CLIP:
          if (ctgc.following) {
            ctgc.following = FALSE;
            box(CLOSE, x, y);
            fClipArea();
          }
         break;
        case ARC:    /* ARC e SECTOR... */
        case SECTOR: /* ...sao equivalentes */
        case CHORD:
          if (ctgc.following) {
            ctgc.following = FALSE;
            arc(CLOSE, x, y);
            cdClip(ctgc.clip_mode);
            draw();
            arc_pos.w = 0;
            arc_pos.h = 0;
          }
          break;
        case POLY:
          cdActivate(ctgc.iup_canvas);
          polygon(NEWPOINT, x, y);
          newpolypoint(x, y);
          sprintf(ctgc.status_line, "cdVertex( %d, %d)", x, y);
          set_status();
          ctgc.following = TRUE;
          break;
        case IMAGE:
          if (ctgc.following) {
            ctgc.following = FALSE;
            box(CLOSE, x, y);
            cdClip(ctgc.clip_mode);
            fImageGet();
          }
          break;
        case RGB:
          if (ctgc.following) {
            ctgc.following = FALSE;
            box(CLOSE, x, y);
            cdClip(ctgc.clip_mode);
            fImageRGBGet();
          }
          break;
        default:
          break;
      }
    }
  }
  else if (b == IUP_BUTTON3) {
    if (e) {
      switch (ctgc.cur_prim) {
        case IMAGE:
          cdClip(ctgc.clip_mode);
          follow(x, y);
          fImagePut();
          break;
        case RGB:
          cdClip(ctgc.clip_mode);
          follow(x, y);
          fImageRGBPut();
          break;
        case CLIP:
          fClipOff();
          break;
        default:
          break;
      }
    }
    else {
      switch (ctgc.cur_prim) {
        case POLY:
          cdActivate(ctgc.iup_canvas);
          ctgc.following = FALSE;
          polygon(CLOSE, x, y);
          sprintf(ctgc.status_line, "cdEnd()");
          set_status();
          fDraw();
          break;
        default:
          break;
      }
    }
  }

  return IUP_DEFAULT;
}

/*-------------------------------------------------------------------------*/
/* Funcao associada ao resize do canvas.                                   */
/*-------------------------------------------------------------------------*/
int fResizeCB(Ihandle *self, int width, int height)
{
  ignore(self);

  /* atualiza o contexto do programa */
  ctgc.w = width;
  ctgc.h = height;

  /* atualiza o tamanho do canvas em pixels no titulo */
  sprintf(ctgc.title, "CDTest 5.3 (%dx%d - %dbpp)", ctgc.w, ctgc.h, ctgc.bpp);
  IupSetAttribute(IupGetHandle("dlgMain"), IUP_TITLE, ctgc.title);

  /* reconstroi o buffer off-screen, se necessario */
  if (ctgc.buffering) 
    cdActivate(ctgc.buffer_canvas);
  else
    cdActivate(ctgc.iup_canvas);

  return IUP_DEFAULT;
}

void cdtest_loadled(void);

/*-------------------------------------------------------------------------*/
/* Rotina principal.                                                       */
/*-------------------------------------------------------------------------*/
int main(int argc, char** argv)
{
  char *err = NULL;

  /* inicializa o IUP */
  IupOpen(&argc, &argv);                        

  /* carrega o LED */
#ifdef USE_LED
  err = IupLoad("cdtest.led");
#else
  cdtest_loadled();
#endif

  if (!err)
  {
    /* inicializa o CDTest */
    CDTestInit();                     
  }
  else
  {
    /* imprime uma mensagem de erro */ 
    IupMessage("LED Error:", err);
    IupClose();
    return;
  }

  IupMainLoop();

  CDTestClose();

  IupClose();

  return 0;
}

