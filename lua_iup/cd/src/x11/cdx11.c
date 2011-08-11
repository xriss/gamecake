/** \file
 * \brief X-Windows Base Driver
 *
 * See Copyright Notice in cd.h
 */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <math.h>

#include "cdx11.h"
#include "xvertex.h"

#include <X11/Xproto.h>

unsigned long (*cdxGetPixel)(cdCtxCanvas *ctxcanvas, unsigned long rgb); /* acesso a tabela de cores */
void (*cdxGetRGB)(cdCtxCanvas *ctxcanvas, unsigned long pixel, 
                                          unsigned char* red, 
                                          unsigned char* green, 
                                          unsigned char* blue); /* acesso a tabela de cores */
static XGCValues gcval;

static int cdxDirectColorTable[256];    /* used with directColor visuals */

#define NUM_HATCHES  6
#define HATCH_WIDTH  8
#define HATCH_HEIGHT 8
/* 
** 6 padroes pre-definidos a serem acessados atraves de cdHatch(
   CD_HORIZONTAL | CD_VERTICAL | CD_FDIAGONAL | CD_BDIAGONAL |
   CD_CROSS      | CD_DIAGCROSS)

*/
static char hatches[NUM_HATCHES][8] = {
  {0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0x00},  /* HORIZONTAL */
  {0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22},  /* VERTICAL */
  {0x08,0x10,0x20,0x40,0x80,0x01,0x02,0x04},  /* FDIAGONAL */
  {0x10,0x08,0x04,0x02,0x01,0x80,0x40,0x20},  /* BDIAGONAL */
  {0x22,0x22,0xFF,0x22,0x22,0x22,0xFF,0x22},  /* CROSS */
  {0x18,0x18,0x24,0x42,0x81,0x81,0x42,0x24}   /* DIAGCROSS */
};

/******************************************************/

static int cdxErrorHandler(Display* dpy, XErrorEvent *err)
{
  char msg[80];

  /* Se for erro de BadMatch em XGetImage, tudo bem */
  if (err->request_code==X_GetImage && err->error_code==BadMatch)
    return 0;

  /* Se for erro de BadAcess em XFreeColors, tudo bem */
  if (err->request_code==X_FreeColors && err->error_code==BadAccess)
    return 0;

  XGetErrorText(dpy, err->error_code, msg, 80);
  fprintf(stderr,"X Error of failed request %d: %s\n", err->request_code, msg);

  return 0; /* ignore always */
}
                                
static void update_colors(cdCtxCanvas *ctxcanvas)
{
  XQueryColors(ctxcanvas->dpy, ctxcanvas->colormap, ctxcanvas->color_table, ctxcanvas->num_colors);
}

static int find_color(cdCtxCanvas *ctxcanvas, XColor* xc1)
{
  int pos = 0, i;
  unsigned long min_dist = ULONG_MAX, /* just a very big value */
    this_dist;
  int dr, dg, db;
  XColor* xc2;

  for (i=0; i<ctxcanvas->num_colors; i++)
  {
    xc2 = &(ctxcanvas->color_table[i]);
    
    dr = (xc1->red   - xc2->red) / 850;       /* 0.30 / 255 */
    dg = (xc1->green - xc2->green) / 432;     /* 0.59 / 255 */
    db = (xc1->blue  - xc2->blue) /  2318;    /* 0.11 / 255 */
    
    this_dist = dr*dr + dg*dg + db*db;
    
    if (this_dist < min_dist)
    {
      min_dist = this_dist;            
      pos = i;                          
    }
  }

  return pos;
}

/* Busca o RGB mais proximo na tabela de cores */
static unsigned long nearest_rgb(cdCtxCanvas *ctxcanvas, XColor* xc)
{
  static int nearest_try = 0;

  int pos = find_color(ctxcanvas, xc);

  /* verifico se a cor ainda esta alocada */
  /* Try to allocate the closest match color.
     This should fail only if the cell is read/write.
     Otherwise, we're incrementing the cell's reference count.
     (comentario extraido da biblioteca Mesa) */
  if (!XAllocColor(ctxcanvas->dpy, ctxcanvas->colormap, &(ctxcanvas->color_table[pos])))
  {
    /* nao esta, preciso atualizar a tabela e procurar novamente */
    /* isto acontece porque a cor encontrada pode ter sido de uma aplicacao que nao existe mais */
    /* uma vez atualizada, o problema nao ocorrera' na nova procura */
    /* ou a celula e' read write */

    if (nearest_try == 1)
    {
      nearest_try = 0;
      return ctxcanvas->color_table[pos].pixel;
    }

    /* o que e' mais lento? 
       Dar um query colors em todo o nearest,                 --> Isso deve ser mais lento
       ou fazer a busca acima antes e arriscar uma repeticao? */

    update_colors(ctxcanvas);

    nearest_try = 1; /* garante que so' vai tentar isso uma vez */
    return nearest_rgb(ctxcanvas, xc);
  }

  return ctxcanvas->color_table[pos].pixel;
}

/* Funcao get_pixel usando tabela de conversao. \
   Usada quando nao estamos em TrueColor. */
static unsigned long not_truecolor_get_pixel(cdCtxCanvas *ctxcanvas, unsigned long rgb)
{
  unsigned long pixel;
  XColor xc;
  xc.red = cdCOLOR8TO16(cdRed(rgb));
  xc.green = cdCOLOR8TO16(cdGreen(rgb));
  xc.blue = cdCOLOR8TO16(cdBlue(rgb));
  xc.flags = DoRed | DoGreen | DoBlue;

  /* verificamos se a nova cor ja' esta' disponivel */
  if (!XAllocColor(ctxcanvas->dpy, ctxcanvas->colormap, &xc))
  {
    /* nao estava disponivel, procuro pela mais proxima na tabela de cores */
    pixel = nearest_rgb(ctxcanvas, &xc); 
  }
  else
  {
    /* ja' estava disponivel */
    /* atualizo a tabela de cores */
    ctxcanvas->color_table[xc.pixel] = xc;
    pixel = xc.pixel;
  }
  
  return pixel;
}

/*
%F Funcao  usando tabela de conversao. \
   Usada quando nao estamos em TrueColor.
*/
static void not_truecolor_get_rgb(cdCtxCanvas *ctxcanvas, unsigned long pixel, unsigned char* red, unsigned char* green, unsigned char* blue)
{
  XColor xc;
  xc.pixel = pixel;
  XQueryColor(ctxcanvas->dpy, ctxcanvas->colormap, &xc);
  *red = cdCOLOR16TO8(xc.red);
  *green = cdCOLOR16TO8(xc.green);
  *blue = cdCOLOR16TO8(xc.blue);
}

/*
%F Funcao get_rgb usada quando estamos em TrueColor.
*/
static void truecolor_get_rgb(cdCtxCanvas *ctxcanvas, unsigned long pixel, unsigned char* red, unsigned char* green, unsigned char* blue)
{
  unsigned long r = pixel & ctxcanvas->vis->red_mask;
  unsigned long g = pixel & ctxcanvas->vis->green_mask;
  unsigned long b = pixel & ctxcanvas->vis->blue_mask;
  if (ctxcanvas->rshift<0) r = r >> (-ctxcanvas->rshift);
  else r = r << ctxcanvas->rshift;
  if (ctxcanvas->gshift<0) g = g >> (-ctxcanvas->gshift);
  else g = g << ctxcanvas->gshift;
  if (ctxcanvas->bshift<0) b = b >> (-ctxcanvas->bshift);
  else b = b << ctxcanvas->bshift;
  *red = cdCOLOR16TO8(r);
  *green = cdCOLOR16TO8(g);
  *blue = cdCOLOR16TO8(b);
}

/*
%F Funcao get_pixel usada quando estamos em TrueColor.
*/
static unsigned long truecolor_get_pixel(cdCtxCanvas *ctxcanvas, unsigned long rgb)
{
  unsigned long r = cdCOLOR8TO16(cdRed(rgb));
  unsigned long g = cdCOLOR8TO16(cdGreen(rgb));
  unsigned long b = cdCOLOR8TO16(cdBlue(rgb));
  
  if (ctxcanvas->rshift<0) 
    r = r << (-ctxcanvas->rshift);
  else 
    r = r >> ctxcanvas->rshift;
    
  if (ctxcanvas->gshift<0) 
    g = g << (-ctxcanvas->gshift);
  else 
    g = g >> ctxcanvas->gshift;
    
  if (ctxcanvas->bshift<0) 
    b = b << (-ctxcanvas->bshift);
  else 
    b = b >> ctxcanvas->bshift;
    
  r = r & ctxcanvas->vis->red_mask;
  g = g & ctxcanvas->vis->green_mask;
  b = b & ctxcanvas->vis->blue_mask;
  
  return r | g | b;
}

static int highbit(unsigned long ul)
{
/* returns position of highest set bit in 'ul' as an integer (0-31),
  or -1 if none */
  int i;  unsigned long hb;
  
  hb = 0x80;  hb = hb << 24;   /* hb = 0x80000000UL */
  for (i=31; ((ul & hb) == 0) && i>=0;  i--, ul<<=1);
  return i;
}

static void makeDirectCmap(cdCtxCanvas *ctxcanvas, Colormap cmap)
{
  int    i, cmaplen, numgot;
  unsigned char   origgot[256];
  XColor c;
  unsigned long rmask, gmask, bmask;
  int    rshift, gshift, bshift;
  
  rmask = ctxcanvas->vis->red_mask;
  gmask = ctxcanvas->vis->green_mask;
  bmask = ctxcanvas->vis->blue_mask;
  
  rshift = highbit(rmask) - 15;
  gshift = highbit(gmask) - 15;
  bshift = highbit(bmask) - 15;
  
  if (rshift<0) rmask = rmask << (-rshift);
  else rmask = rmask >> rshift;
  
  if (gshift<0) gmask = gmask << (-gshift);
  else gmask = gmask >> gshift;
  
  if (bshift<0) bmask = bmask << (-bshift);
  else bmask = bmask >> bshift;
  
  cmaplen = ctxcanvas->vis->map_entries;
  if (cmaplen>256) cmaplen=256;
  
  /* try to alloc a 'cmaplen' long grayscale colormap.  May not get all
  entries for whatever reason.  Build table 'cdxDirectColorTable[]' that
  maps range [0..(cmaplen-1)] into set of colors we did get */
  
  for (i=0; i<256; i++) {  origgot[i] = 0;  cdxDirectColorTable[i] = i; }
  
  for (i=numgot=0; i<cmaplen; i++) 
  {
    c.red = c.green = c.blue = (unsigned short)((i * 0xffff) / (cmaplen - 1));
    c.red   = (unsigned short)(c.red   & rmask);
    c.green = (unsigned short)(c.green & gmask);
    c.blue  = (unsigned short)(c.blue  & bmask);
    c.flags = DoRed | DoGreen | DoBlue;
    
    if (XAllocColor(ctxcanvas->dpy, cmap, &c)) 
    {
      origgot[i] = 1;
      numgot++;
    }
  }
  
  if (numgot == 0) 
    return;
  
  /* cdxDirectColorTable may or may not have holes in it. */
  for (i=0; i<cmaplen; i++) 
  {
    if (!origgot[i]) 
    {
      int numbak, numfwd;
      numbak = numfwd = 0;
      while ((i - numbak) >= 0       && !origgot[i-numbak]) numbak++;
      while ((i + numfwd) <  cmaplen && !origgot[i+numfwd]) numfwd++;
      
      if (i-numbak<0        || !origgot[i-numbak]) numbak = 999;
      if (i+numfwd>=cmaplen || !origgot[i+numfwd]) numfwd = 999;
      
      if      (numbak<numfwd) cdxDirectColorTable[i] = cdxDirectColorTable[i-numbak];
      else if (numfwd<999)    cdxDirectColorTable[i] = cdxDirectColorTable[i+numfwd];
    }
  }
}

/******************************************************/

void cdxKillCanvas(cdCtxCanvas *ctxcanvas)
{
  if (ctxcanvas->canvas->bpp <= 8)
  {
    unsigned long pixels[256];
    int i;

    /* libera todas as cores usadas na palette */
    for(i = 0; i < ctxcanvas->num_colors; i++)
      pixels[i] = ctxcanvas->color_table[i].pixel;

    if (ctxcanvas->colormap != DefaultColormap(ctxcanvas->dpy, ctxcanvas->scr))
      XFreeColormap(ctxcanvas->dpy, ctxcanvas->colormap);
  }
 
  if (ctxcanvas->xidata) free(ctxcanvas->xidata);
  if (ctxcanvas->font) XFreeFont(ctxcanvas->dpy, ctxcanvas->font);
  if (ctxcanvas->last_hatch) XFreePixmap(ctxcanvas->dpy, ctxcanvas->last_hatch);
  if (ctxcanvas->clip_polygon) XFreePixmap(ctxcanvas->dpy, ctxcanvas->clip_polygon);

  if (ctxcanvas->new_region) 
  {
    XFreeGC(ctxcanvas->dpy, ctxcanvas->region_aux_gc); 
    XFreePixmap(ctxcanvas->dpy, ctxcanvas->region_aux);
    XFreePixmap(ctxcanvas->dpy, ctxcanvas->new_region);
  }

  if (ctxcanvas->last_pattern)
  {
    XFreeGC(ctxcanvas->dpy, ctxcanvas->last_pattern_gc); 
    XFreePixmap(ctxcanvas->dpy, ctxcanvas->last_pattern);
  }

  if (ctxcanvas->last_stipple)
  {
    XFreeGC(ctxcanvas->dpy, ctxcanvas->last_stipple_gc); 
    XFreePixmap(ctxcanvas->dpy, ctxcanvas->last_stipple);
  }

  XFreeGC(ctxcanvas->dpy, ctxcanvas->gc); 

  free(ctxcanvas);
}

/******************************************************/

static void cdflush(cdCtxCanvas *ctxcanvas)
{
  XFlush(ctxcanvas->dpy);
}

/******************************************************/

static Pixmap build_clip_polygon(cdCtxCanvas *ctxcanvas, XPoint* pnt, int n)
{
  Pixmap pix = XCreatePixmap(ctxcanvas->dpy, ctxcanvas->wnd, ctxcanvas->canvas->w, ctxcanvas->canvas->h, 1);
  GC gc = XCreateGC(ctxcanvas->dpy, pix, 0, NULL);

  XSetForeground(ctxcanvas->dpy, gc, 0);
  XFillRectangle(ctxcanvas->dpy, pix, gc, 0, 0, ctxcanvas->canvas->w, ctxcanvas->canvas->h);

  XSetForeground(ctxcanvas->dpy, gc, 1);
  XSetFillRule(ctxcanvas->dpy, gc, ctxcanvas->canvas->fill_mode==CD_EVENODD?EvenOddRule:WindingRule);
  XFillPolygon(ctxcanvas->dpy, pix, gc, pnt, n, Complex, CoordModeOrigin);

  XFreeGC(ctxcanvas->dpy, gc);
  return pix;
}

static void xsetclip_area(cdCtxCanvas *ctxcanvas)
{
  cdRect* clip_rect = &ctxcanvas->canvas->clip_rect;
  if (ctxcanvas->canvas->use_matrix)
  {
    cdPoint poly[4];
    poly[0].x = clip_rect->xmin; poly[0].y = clip_rect->ymin;
    poly[1].x = clip_rect->xmin; poly[1].y = clip_rect->ymax;
    poly[2].x = clip_rect->xmax; poly[2].y = clip_rect->ymax;
    poly[3].x = clip_rect->xmax; poly[3].y = clip_rect->ymin;
    ctxcanvas->canvas->cxPoly(ctxcanvas, CD_CLIP, poly, 4);
  }
  else
  {
    XRectangle rect;
    rect.x      = (short)clip_rect->xmin;
    rect.y      = (short)clip_rect->ymin;
    rect.width  = (unsigned short)(clip_rect->xmax - clip_rect->xmin + 1);
    rect.height = (unsigned short)(clip_rect->ymax - clip_rect->ymin + 1);
    XSetClipRectangles(ctxcanvas->dpy, ctxcanvas->gc, 0, 0, &rect, 1, Unsorted);
  }
}

int cdxClip(cdCtxCanvas *ctxcanvas, int clip_mode)
{
  switch (clip_mode)
  {
  case CD_CLIPOFF:
    XSetClipMask(ctxcanvas->dpy, ctxcanvas->gc, None);
    break;
  case CD_CLIPAREA:
    xsetclip_area(ctxcanvas);
    break;
  case CD_CLIPPOLYGON:
    if (ctxcanvas->clip_polygon)
      XSetClipMask(ctxcanvas->dpy, ctxcanvas->gc, ctxcanvas->clip_polygon);
    break;
  case CD_CLIPREGION:
    if (ctxcanvas->new_region)
      XSetClipMask(ctxcanvas->dpy, ctxcanvas->gc, ctxcanvas->new_region);
    break;
  }
  return clip_mode;
}

static void cdcliparea(cdCtxCanvas *ctxcanvas, int xmin, int xmax, int ymin, int ymax)
{
  if (ctxcanvas->canvas->clip_mode == CD_CLIPAREA) 
  {
    ctxcanvas->canvas->clip_rect.xmin = xmin;
    ctxcanvas->canvas->clip_rect.ymin = ymin;
    ctxcanvas->canvas->clip_rect.xmax = xmax;
    ctxcanvas->canvas->clip_rect.ymax = ymax;
    cdxClip(ctxcanvas, CD_CLIPAREA);
  }
}

static void cdnewregion(cdCtxCanvas *ctxcanvas)
{
  if (ctxcanvas->new_region) 
  {
    XFreeGC(ctxcanvas->dpy, ctxcanvas->region_aux_gc); 
    XFreePixmap(ctxcanvas->dpy, ctxcanvas->region_aux);
    XFreePixmap(ctxcanvas->dpy, ctxcanvas->new_region);
  }
   
  ctxcanvas->new_region = XCreatePixmap(ctxcanvas->dpy, ctxcanvas->wnd, ctxcanvas->canvas->w, ctxcanvas->canvas->h, 1);

  {
    GC gc = XCreateGC(ctxcanvas->dpy, ctxcanvas->new_region, 0, NULL);
    XSetForeground(ctxcanvas->dpy, gc, 0);
    XFillRectangle(ctxcanvas->dpy, ctxcanvas->new_region, gc, 0, 0, ctxcanvas->canvas->w, ctxcanvas->canvas->h);
    XFreeGC(ctxcanvas->dpy, gc);
  }
    
  ctxcanvas->region_aux = XCreatePixmap(ctxcanvas->dpy, ctxcanvas->wnd, ctxcanvas->canvas->w, ctxcanvas->canvas->h, 1);
  ctxcanvas->region_aux_gc = XCreateGC(ctxcanvas->dpy, ctxcanvas->region_aux, 0, NULL);
  XSetBackground(ctxcanvas->dpy, ctxcanvas->region_aux_gc, 0);
}

static int cdispointinregion(cdCtxCanvas *ctxcanvas, int x, int y)
{
  if (!ctxcanvas->new_region)
    return 0;

  if (x >= 0  && y >= 0 && x < ctxcanvas->canvas->w && y < ctxcanvas->canvas->h)
  {
    long p;
    XImage* img = XGetImage(ctxcanvas->dpy, ctxcanvas->new_region, 0, 0, ctxcanvas->canvas->w, ctxcanvas->canvas->h, 1, XYPixmap); 
    p = XGetPixel(img, x, y);
    XDestroyImage(img);
    
    if (p) return 1;
  }

  return 0;
}

static void cdgetregionbox(cdCtxCanvas *ctxcanvas, int *xmin, int *xmax, int *ymin, int *ymax)
{
  if (!ctxcanvas->new_region)
    return;
    
  *xmin = ctxcanvas->canvas->w-1;
  *xmax = 0;
  *ymin = ctxcanvas->canvas->h-1;
  *ymax = 0;

  {
    int x, y;
    long p;
    XImage* img = XGetImage(ctxcanvas->dpy, ctxcanvas->new_region, 0, 0, ctxcanvas->canvas->w, ctxcanvas->canvas->h, 1, XYPixmap); 
  
    for (y = 0; y < ctxcanvas->canvas->h; y++)
    {
      for (x = 0; x < ctxcanvas->canvas->w; x++)
      {
        p = XGetPixel(img, x, y);
        
        if (p)
        {
          if (x < *xmin) *xmin = x; 
          if (x > *xmax) *xmax = x; 
          if (y < *ymin) *ymin = y; 
          if (y > *ymax) *ymax = y; 
          break;
        }
      }
      
      if (x != ctxcanvas->canvas->w-1)
      {
        for (x = ctxcanvas->canvas->w-1; x >= 0; x--)
        {
          p = XGetPixel(img, x, y);
        
          if (p)
          {
            if (x < *xmin) *xmin = x; 
            if (x > *xmax) *xmax = x; 
            if (y < *ymin) *ymin = y; 
            if (y > *ymax) *ymax = y; 
            break;
          }
        }
      }
    }

    XDestroyImage(img);
  }
}

static void sPrepareRegion(cdCtxCanvas *ctxcanvas)
{
  if (!ctxcanvas->new_region)
    return;

  XSetFunction(ctxcanvas->dpy, ctxcanvas->region_aux_gc, GXcopy);
  XSetForeground(ctxcanvas->dpy, ctxcanvas->region_aux_gc, 0);
  XFillRectangle(ctxcanvas->dpy, ctxcanvas->region_aux, ctxcanvas->region_aux_gc, 0, 0, ctxcanvas->canvas->w, ctxcanvas->canvas->h);
  XSetForeground(ctxcanvas->dpy, ctxcanvas->region_aux_gc, 1);
}

static void sCombineRegion(cdCtxCanvas *ctxcanvas)
{
  switch(ctxcanvas->canvas->combine_mode)
  {                          
  case CD_UNION:
    XSetFunction(ctxcanvas->dpy, ctxcanvas->region_aux_gc, GXor);
    break;
  case CD_INTERSECT:   
    XSetFunction(ctxcanvas->dpy, ctxcanvas->region_aux_gc, GXand);
    break;
  case CD_DIFFERENCE:           
    XSetFunction(ctxcanvas->dpy, ctxcanvas->region_aux_gc, GXandInverted);
    break;
  case CD_NOTINTERSECT:
    XSetFunction(ctxcanvas->dpy, ctxcanvas->region_aux_gc, GXxor);
    break;
  }
  
  XCopyArea(ctxcanvas->dpy, ctxcanvas->region_aux, ctxcanvas->new_region, ctxcanvas->region_aux_gc,
            0, 0,
            ctxcanvas->canvas->w, ctxcanvas->canvas->h,
            0, 0);  
}

static void cdoffsetregion(cdCtxCanvas *ctxcanvas, int x, int y)
{
  if (!ctxcanvas->new_region)
    return;
    
  sPrepareRegion(ctxcanvas);    

  XCopyArea(ctxcanvas->dpy, ctxcanvas->new_region, ctxcanvas->region_aux, ctxcanvas->region_aux_gc,
            0, 0,
            ctxcanvas->canvas->w-x, ctxcanvas->canvas->h-y,
            x, y);  
  
  XCopyArea(ctxcanvas->dpy, ctxcanvas->region_aux, ctxcanvas->new_region, ctxcanvas->region_aux_gc,
            0, 0,
            ctxcanvas->canvas->w, ctxcanvas->canvas->h,
            0, 0);  
}

/******************************************************/

static int cdwritemode(cdCtxCanvas *ctxcanvas, int write_mode)
{
  switch (write_mode)
  {
  case CD_REPLACE:
    XSetFunction(ctxcanvas->dpy, ctxcanvas->gc, GXcopy);
    break;
  case CD_XOR:
    XSetFunction(ctxcanvas->dpy, ctxcanvas->gc, GXxor);
    break;
  case CD_NOT_XOR:
    XSetFunction(ctxcanvas->dpy, ctxcanvas->gc, GXequiv);
    break;
  }

  return write_mode;
}

static int cdinteriorstyle(cdCtxCanvas *ctxcanvas, int style)
{
  int sty = FillSolid;

  switch (style)
  {
    case CD_SOLID:
      sty = FillSolid;
      break;
    case CD_HATCH :
      if (!ctxcanvas->last_hatch) 
        return ctxcanvas->canvas->interior_style;

      XSetStipple(ctxcanvas->dpy, ctxcanvas->gc, ctxcanvas->last_hatch);

      if (ctxcanvas->canvas->back_opacity == CD_OPAQUE)
        sty = FillOpaqueStippled;
      else
        sty = FillStippled;
      break;
    case CD_STIPPLE:
      XSetStipple(ctxcanvas->dpy, ctxcanvas->gc, ctxcanvas->last_stipple);

      if (ctxcanvas->canvas->back_opacity == CD_OPAQUE)
        sty = FillOpaqueStippled;
      else
        sty = FillStippled;
      break;
    case CD_PATTERN:
      XSetTile(ctxcanvas->dpy, ctxcanvas->gc, ctxcanvas->last_pattern);
      sty = FillTiled;
      break;
  }

  XSetFillStyle(ctxcanvas->dpy, ctxcanvas->gc, sty);

  return style;
}

static int cdhatch(cdCtxCanvas *ctxcanvas, int hatch_style)
{
  if (ctxcanvas->last_hatch)
    XFreePixmap(ctxcanvas->dpy, ctxcanvas->last_hatch);

  ctxcanvas->last_hatch = XCreatePixmapFromBitmapData(ctxcanvas->dpy,
                          ctxcanvas->wnd, hatches[hatch_style],
                          HATCH_WIDTH, HATCH_HEIGHT, 1, 0, 1);

  cdinteriorstyle(ctxcanvas, CD_HATCH);

  return hatch_style;
}

static void cdstipple(cdCtxCanvas *ctxcanvas, int w, int h, const unsigned char *data)
{
  int x, y;

  if (ctxcanvas->last_stipple == 0 || (ctxcanvas->last_stipple_w != w || ctxcanvas->last_stipple_h != h))
  {
    if (ctxcanvas->last_stipple != 0)
    {
      XFreePixmap(ctxcanvas->dpy, ctxcanvas->last_stipple);
      XFreeGC(ctxcanvas->dpy, ctxcanvas->last_stipple_gc); 
    }

    ctxcanvas->last_stipple = XCreatePixmap(ctxcanvas->dpy,ctxcanvas->wnd,w,h,1);
    if (!ctxcanvas->last_stipple) return;
    ctxcanvas->last_stipple_gc = XCreateGC(ctxcanvas->dpy, ctxcanvas->last_stipple, 0, 0);
    ctxcanvas->last_stipple_w = w;
    ctxcanvas->last_stipple_h = h;
  }

  for (y=0; y<h; y++)
  {
    for (x=0; x<w; x++)
    {
      XSetForeground(ctxcanvas->dpy, ctxcanvas->last_stipple_gc, data[y*w+x]? 1: 0);
      XDrawPoint(ctxcanvas->dpy, ctxcanvas->last_stipple, ctxcanvas->last_stipple_gc, x, h-y-1);
    }
  }

  cdinteriorstyle(ctxcanvas, CD_STIPPLE);
}

static int find_match(unsigned long* palette, int pal_size, unsigned long color, unsigned char *match)
{
  int i;

  for (i=0;i<pal_size;i++)
  {
    if (palette[i] == color)
    {
      *match = (unsigned char)i;
      return 1;
    }
  }

  return 0;
}

static void cdpattern(cdCtxCanvas *ctxcanvas, int w, int h, const long int *colors)
{
  int x, y, i;
  int size = w*h;
  unsigned long *pixels;

  if (ctxcanvas->last_pattern == 0 || (ctxcanvas->last_pattern_w != w || ctxcanvas->last_pattern_h != h))
  {
    if (ctxcanvas->last_pattern != 0)
    {
      XFreePixmap(ctxcanvas->dpy, ctxcanvas->last_pattern);
      XFreeGC(ctxcanvas->dpy, ctxcanvas->last_pattern_gc); 
    }

    ctxcanvas->last_pattern = XCreatePixmap(ctxcanvas->dpy,ctxcanvas->wnd,w,h,ctxcanvas->depth);
    if (!ctxcanvas->last_pattern) return;
    ctxcanvas->last_pattern_gc = XCreateGC(ctxcanvas->dpy, ctxcanvas->last_pattern, 0, 0);
    ctxcanvas->last_pattern_w = w;
    ctxcanvas->last_pattern_h = h;
  }

  pixels = (unsigned long*)malloc(w*h*sizeof(long));

  if (ctxcanvas->canvas->bpp <= 8)
  {
    long int match_table[256];    /* X  colors */
    unsigned long palette[256];   /* CD colors */
    unsigned char *index = (unsigned char*)malloc(size), match;
    int pal_size = 1;
    palette[0] = colors[0];

    /* encontra as n primeiras cores diferentes da imagem (ate 256) */
    for(i=0;i<size;i++)
    {
      if (!find_match(palette, pal_size, colors[i], &match))
      {
         palette[pal_size] = colors[i];
         index[i] = (unsigned char)pal_size;
         pal_size++;

         if (pal_size == 256)
           break;
      }
      else
        index[i] = match;
    }

    /* de cores do CD para cores do X */
    for (i = 0; i < pal_size; i++)
      match_table[i] = cdxGetPixel(ctxcanvas, palette[i]);

    /* de imagem do CD para imagem do X */
    for(i=0;i<size;i++)
      pixels[i] = match_table[index[i]];

    free(index);
  }
  else
  {
    for(i=0;i<size;i++)
      pixels[i] = cdxGetPixel(ctxcanvas, colors[i]);
  }

  for (y=0; y<h; y++)
  {
    for (x=0; x<w; x++)
    {
      XSetForeground(ctxcanvas->dpy, ctxcanvas->last_pattern_gc, pixels[y*w+x]);
      XDrawPoint(ctxcanvas->dpy, ctxcanvas->last_pattern, ctxcanvas->last_pattern_gc, x, h-y-1);
    }
  }

  cdinteriorstyle(ctxcanvas, CD_PATTERN);

  free(pixels);
}

static int cdlinestyle(cdCtxCanvas *ctxcanvas, int style)
{
  switch (style)
  {
  case CD_CONTINUOUS:
    gcval.line_style = LineSolid;
    break;
  case CD_DASHED:
  case CD_DOTTED:
  case CD_DASH_DOT:
  case CD_DASH_DOT_DOT:
    {
      static struct {
        int size;
        char list[6];
      } dashes[4] = {
        { 2, { 6, 2 } },
        { 2, { 2, 2 } },
        { 4, { 6, 2, 2, 2 } },
        { 6, { 6, 2, 2, 2, 2, 2 } }
      };

      if (ctxcanvas->canvas->back_opacity == CD_OPAQUE)
        gcval.line_style = LineDoubleDash;
      else
        gcval.line_style = LineOnOffDash;
        
      XSetDashes(ctxcanvas->dpy, ctxcanvas->gc, 0, dashes[style-CD_DASHED].list,
                 dashes[style-CD_DASHED].size);
      break;
    }
  case CD_CUSTOM:        
    {
      int i;
      char* dash_style = (char*)malloc(ctxcanvas->canvas->line_dashes_count);
      for (i = 0; i < ctxcanvas->canvas->line_dashes_count; i++)
        dash_style[i] = (char)ctxcanvas->canvas->line_dashes[i];

      if (ctxcanvas->canvas->back_opacity == CD_OPAQUE)
        gcval.line_style = LineDoubleDash;
      else
        gcval.line_style = LineOnOffDash;

      XSetDashes(ctxcanvas->dpy, ctxcanvas->gc, 0, dash_style,
                 ctxcanvas->canvas->line_dashes_count);
      free(dash_style);
      break;
    }
  }
  XChangeGC(ctxcanvas->dpy, ctxcanvas->gc, GCLineStyle, &gcval);
  return style;
}

static int cdlinewidth(cdCtxCanvas *ctxcanvas, int width)
{
  if (width == 1) 
    gcval.line_width = 0;
  else
    gcval.line_width = width;

  XChangeGC(ctxcanvas->dpy, ctxcanvas->gc, GCLineWidth, &gcval);

  return width;
}

static int cdlinecap(cdCtxCanvas *ctxcanvas, int cap)
{
  int cd2x_cap[] =  {CapButt, CapProjecting, CapRound};
  
  gcval.cap_style = cd2x_cap[cap];
  XChangeGC(ctxcanvas->dpy, ctxcanvas->gc, GCCapStyle, &gcval);

  return cap;
}

static int cdlinejoin(cdCtxCanvas *ctxcanvas, int join)
{
  int cd2x_join[] = {JoinMiter, JoinBevel, JoinRound};
  
  gcval.join_style = cd2x_join[join];
  XChangeGC(ctxcanvas->dpy, ctxcanvas->gc, GCJoinStyle, &gcval);

  return join;
}

static int cdbackopacity(cdCtxCanvas *ctxcanvas, int opaque)
{
  ctxcanvas->canvas->back_opacity = opaque;
  cdinteriorstyle(ctxcanvas, ctxcanvas->canvas->interior_style);
  cdlinestyle(ctxcanvas, ctxcanvas->canvas->line_style);
  return opaque;
}

static int cdxGetFontSize(char* font_name)
{
  int i = 0;
  while (i < 8)
  {
    font_name = strchr(font_name, '-')+1;
    i++;
  }

  *(strchr(font_name, '-')) = 0;
  return atoi(font_name);
}

static int cdfont(cdCtxCanvas *ctxcanvas, const char *type_face, int style, int size)
{
  XFontStruct *font;
  char **font_names_list;
  char font_name[1024];
  char* foundry = "*";
  int i, num_fonts, font_size, near_size, change_italic = 0;

  /* no underline or strikeout support */

  static char * type[] = 
  {
    "medium-r",  /* CD_PLAIN */
    "bold-r",    /* CD_BOLD */
    "medium-i",  /* CD_ITALIC */
    "bold-i"     /* CD_BOLD_ITALIC */
  };

  if (cdStrEqualNoCase(type_face, "System"))
    type_face = "fixed";
  else if (cdStrEqualNoCase(type_face, "Monospace") || cdStrEqualNoCase(type_face, "Courier New"))
    type_face = "courier";
  else if (cdStrEqualNoCase(type_face, "Serif") || cdStrEqualNoCase(type_face, "Times New Roman"))
    type_face = "times";
  else if (cdStrEqualNoCase(type_face, "Sans") || cdStrEqualNoCase(type_face, "Arial"))
    type_face = "helvetica";

  if (cdStrEqualNoCase(type_face, "Fixed"))
    foundry = "misc";

  sprintf(font_name,"-%s-%s-%s-*-*-*-*-*-*-*-*-*-*", foundry, type_face, type[style&3]);

  font_names_list = XListFonts(ctxcanvas->dpy, font_name, 32767, &num_fonts);
  if (!num_fonts)
  {
    /* try changing 'i' to 'o', for italic */
    if (style&CD_ITALIC)
    {
      change_italic = 1;
      strstr(font_name, "-i-")[1] = 'o';
      font_names_list = XListFonts(ctxcanvas->dpy, font_name, 32767, &num_fonts);
    }

    if (!num_fonts)
      return 0;
  }

  size = cdGetFontSizePoints(ctxcanvas->canvas, size);

  size *= 10; /* convert to deci-points */

  near_size = -1000;
  for (i=0; i<num_fonts; i++)
  {
    font_size = cdxGetFontSize(font_names_list[i]);

    if (font_size == size)
    {
      near_size = font_size;
      break;
    }

    if (abs(font_size-size) < abs(near_size-size))
      near_size = font_size;
  }

  XFreeFontNames(font_names_list);

  sprintf(font_name,"-%s-%s-%s-*-*-*-%d-*-*-*-*-*-*", foundry, type_face, type[style&3], near_size);
  if (change_italic) strstr(font_name, "-i-")[1] = 'o';

  font = XLoadQueryFont(ctxcanvas->dpy, font_name);
  if (!font)
    return 0;

  if (ctxcanvas->font) 
    XFreeFont(ctxcanvas->dpy, ctxcanvas->font);

  ctxcanvas->font = font;
  XSetFont(ctxcanvas->dpy, ctxcanvas->gc, ctxcanvas->font->fid);
  return 1;
}

static int cdnativefont(cdCtxCanvas *ctxcanvas, const char* nativefont)
{
  int size = 12, style = CD_PLAIN;
  char type_face[1024];

  if (nativefont[0] == '-')
  {
    XFontStruct *font = XLoadQueryFont(ctxcanvas->dpy, nativefont);
    if (!font)
      return 0;

    if (!cdParseXWinFont(nativefont, type_face, &style, &size))
    {
      XFreeFont(ctxcanvas->dpy, font);
      return 0;
    }

    if (ctxcanvas->font) XFreeFont(ctxcanvas->dpy, ctxcanvas->font);
    ctxcanvas->font = font;
    XSetFont(ctxcanvas->dpy, ctxcanvas->gc, ctxcanvas->font->fid);
  }
  else
  {
    if (!cdParsePangoFont(nativefont, type_face, &style, &size))
      return 0;

    if (!cdfont(ctxcanvas, type_face, style, size))
      return 0;
  }

  /* update cdfont parameters */
  ctxcanvas->canvas->font_style = style;
  ctxcanvas->canvas->font_size = size;
  strcpy(ctxcanvas->canvas->font_type_face, type_face);

  return 1;
}

static void cdgetfontdim(cdCtxCanvas *ctxcanvas, int *max_width, int *height, int *ascent, int *descent)
{
  if (!ctxcanvas->font) return;
  if (max_width) *max_width = ctxcanvas->font->max_bounds.width;
  if (height)    *height    = ctxcanvas->font->ascent + ctxcanvas->font->descent;
  if (ascent)    *ascent    = ctxcanvas->font->ascent;
  if (descent)   *descent   = ctxcanvas->font->descent;
}

static long int cdbackground(cdCtxCanvas *ctxcanvas, long int color)
{
  XSetBackground(ctxcanvas->dpy, ctxcanvas->gc, cdxGetPixel(ctxcanvas, color));
  return color;
}

static long int cdforeground(cdCtxCanvas *ctxcanvas, long int color)
{          
  ctxcanvas->fg = cdxGetPixel(ctxcanvas, color);
  XSetForeground(ctxcanvas->dpy, ctxcanvas->gc, ctxcanvas->fg);
  return color;
}

static void cdpalette(cdCtxCanvas *ctxcanvas, int n, const long int *palette, int mode)
{
  unsigned long pixels[256];
  int i;

  for(i = 0; i < ctxcanvas->num_colors; i++)
    pixels[i] = ctxcanvas->color_table[i].pixel;

  XFreeColors(ctxcanvas->dpy, ctxcanvas->colormap, pixels, ctxcanvas->num_colors, 0);

  if (mode == CD_FORCE)
  {
    XColor xc;
    int tokeep;

    /* se antes era POLITE aloca palette propria */
    if (ctxcanvas->colormap == DefaultColormap(ctxcanvas->dpy, ctxcanvas->scr))
      ctxcanvas->colormap = XCreateColormap(ctxcanvas->dpy, ctxcanvas->wnd, ctxcanvas->vis, AllocNone);

    /* se for FORCE ira' alocar todas as cores, 
       mas se o numero de cores desejado e' menor que o maximo
       entao uso a diferenca para preservar as primeiras cores alocadas no colormap default. */
    tokeep = ctxcanvas->num_colors - n;
    if (tokeep)
    {
      for (i=0; i<tokeep; i++) 
        ctxcanvas->color_table[i].pixel=i;

      XQueryColors(ctxcanvas->dpy, DefaultColormap(ctxcanvas->dpy, ctxcanvas->scr), ctxcanvas->color_table, tokeep);

      /* reservo estas cores para o CD tambem */
      for (i=0; i<tokeep; i++)
        XAllocColor(ctxcanvas->dpy, ctxcanvas->colormap, &(ctxcanvas->color_table[i])); 
    }

    /*aloco todas as cores da palette para o CD */
    for (i=0; i<n; i++)
    {
      xc.red = cdCOLOR8TO16(cdRed(palette[i]));
      xc.green = cdCOLOR8TO16(cdGreen(palette[i]));
      xc.blue = cdCOLOR8TO16(cdBlue(palette[i]));
      xc.flags = DoRed | DoGreen | DoBlue;
      XAllocColor(ctxcanvas->dpy, ctxcanvas->colormap, &xc); 
    }

    /* atualizo toda a tabela de cores */
    XSetWindowColormap(ctxcanvas->dpy, ctxcanvas->wnd, ctxcanvas->colormap);
    update_colors(ctxcanvas);
  }
  else
  {
    /* se antes era FORCE, remove palette propria */
    if (ctxcanvas->colormap != DefaultColormap(ctxcanvas->dpy, ctxcanvas->scr))
    {
      XFreeColormap(ctxcanvas->dpy, ctxcanvas->colormap);
      ctxcanvas->colormap = DefaultColormap(ctxcanvas->dpy, ctxcanvas->scr);
    }

    /* atualizo a tabela antes de acrescentar novas cores afinal liberamos todas as que podiamos antes disso */
    update_colors(ctxcanvas);

    /* se for POLITE apenas tento alocar todas as cores da palette */
    for (i=0; i<n; i++)
      cdxGetPixel(ctxcanvas, palette[i]);
  }
}

/******************************************************/

static void cdxCheckSolidStyle(cdCtxCanvas *ctxcanvas, int set)
{
  if (ctxcanvas->canvas->interior_style == CD_SOLID)
    return;

  if (set)
    XSetFillStyle(ctxcanvas->dpy, ctxcanvas->gc, FillSolid);
  else
    cdinteriorstyle(ctxcanvas, ctxcanvas->canvas->interior_style);
}

static int cdwritemode(cdCtxCanvas *ctxcanvas, int write_mode);

static void cdclear(cdCtxCanvas* ctxcanvas)
{
  if (ctxcanvas->canvas->write_mode!= CD_REPLACE) cdwritemode(ctxcanvas, CD_REPLACE);
  cdxCheckSolidStyle(ctxcanvas, 1);
  XSetForeground(ctxcanvas->dpy, ctxcanvas->gc, cdxGetPixel(ctxcanvas, ctxcanvas->canvas->background));
  XFillRectangle(ctxcanvas->dpy, ctxcanvas->wnd, ctxcanvas->gc, 0, 0, ctxcanvas->canvas->w, ctxcanvas->canvas->h);
  XSetForeground(ctxcanvas->dpy, ctxcanvas->gc, cdxGetPixel(ctxcanvas, ctxcanvas->canvas->foreground));
  cdxCheckSolidStyle(ctxcanvas, 0);
  if (ctxcanvas->canvas->write_mode!= CD_REPLACE) cdwritemode(ctxcanvas, ctxcanvas->canvas->write_mode);
}

static void cdline(cdCtxCanvas *ctxcanvas, int x1, int y1, int x2, int y2)
{ 
  if (ctxcanvas->canvas->use_matrix)
  {
    cdMatrixTransformPoint(ctxcanvas->xmatrix, x1, y1, &x1, &y1);
    cdMatrixTransformPoint(ctxcanvas->xmatrix, x2, y2, &x2, &y2);
  }

  cdxCheckSolidStyle(ctxcanvas, 1);
  XDrawLine(ctxcanvas->dpy, ctxcanvas->wnd, ctxcanvas->gc, x1, y1, x2, y2);
  cdxCheckSolidStyle(ctxcanvas, 0);
}

static void cdarc(cdCtxCanvas *ctxcanvas, int xc, int yc, int w, int h, double a1, double a2)
{
  if (ctxcanvas->canvas->use_matrix)
  {
    cdSimArc(ctxcanvas, xc, yc, w, h, a1, a2);
    return;
  }

  /* angles in 1/64ths of degrees counterclockwise, similar to CD */

  cdxCheckSolidStyle(ctxcanvas, 1);
  XDrawArc(ctxcanvas->dpy, ctxcanvas->wnd, ctxcanvas->gc, xc-w/2, yc-h/2, w, h, cdRound(a1*64), cdRound((a2 - a1)*64));
  cdxCheckSolidStyle(ctxcanvas, 0);
}

static void cdsector(cdCtxCanvas *ctxcanvas, int xc, int yc, int w, int h, double a1, double a2)
{
  if (ctxcanvas->canvas->use_matrix)
  {
    cdSimSector(ctxcanvas, xc, yc, w, h, a1, a2);
    return;
  }

  if (ctxcanvas->canvas->new_region)
  {
    sPrepareRegion(ctxcanvas);
    XSetArcMode(ctxcanvas->dpy, ctxcanvas->region_aux_gc, ArcPieSlice);
    XFillArc(ctxcanvas->dpy, ctxcanvas->region_aux, ctxcanvas->region_aux_gc, xc-w/2, yc-h/2, w, h, cdRound(a1*64), cdRound((a2 - a1)*64));
    sCombineRegion(ctxcanvas);
  }
  else
  {
    XSetArcMode(ctxcanvas->dpy, ctxcanvas->gc, ArcPieSlice);
    XFillArc(ctxcanvas->dpy, ctxcanvas->wnd, ctxcanvas->gc, xc-w/2, yc-h/2, w, h, cdRound(a1*64), cdRound((a2 - a1)*64));
  }
}

static void cdchord(cdCtxCanvas *ctxcanvas, int xc, int yc, int w, int h, double a1, double a2)
{
  if (ctxcanvas->canvas->use_matrix)
  {
    cdSimChord(ctxcanvas, xc, yc, w, h, a1, a2);
    return;
  }

  if (ctxcanvas->canvas->new_region)
  {
    sPrepareRegion(ctxcanvas);
    XSetArcMode(ctxcanvas->dpy, ctxcanvas->region_aux_gc, ArcChord);
    XFillArc(ctxcanvas->dpy, ctxcanvas->region_aux, ctxcanvas->region_aux_gc, xc-w/2, yc-h/2, w, h, cdRound(a1*64), cdRound((a2 - a1)*64));
    sCombineRegion(ctxcanvas);
  }
  else
  {
    XSetArcMode(ctxcanvas->dpy, ctxcanvas->gc, ArcChord);
    XFillArc(ctxcanvas->dpy, ctxcanvas->wnd, ctxcanvas->gc, xc-w/2, yc-h/2, w, h, cdRound(a1*64), cdRound((a2 - a1)*64));
  }
}

static void cdrect(cdCtxCanvas *ctxcanvas, int xmin, int xmax, int ymin, int ymax)
{
  if (ctxcanvas->canvas->use_matrix)
  {
    cdSimRect(ctxcanvas, xmin, xmax, ymin, ymax);
    return;
  }

  cdxCheckSolidStyle(ctxcanvas, 1);
  XDrawRectangle(ctxcanvas->dpy, ctxcanvas->wnd, ctxcanvas->gc, xmin, ymin, xmax-xmin, ymax-ymin);
  cdxCheckSolidStyle(ctxcanvas, 0);
}

static void cdbox(cdCtxCanvas *ctxcanvas, int xmin, int xmax, int ymin, int ymax)
{
  if (ctxcanvas->canvas->use_matrix)
  {
    cdSimBox(ctxcanvas, xmin, xmax, ymin, ymax);
    return;
  }

  if (ctxcanvas->canvas->new_region)
  {
    sPrepareRegion(ctxcanvas);
    XFillRectangle(ctxcanvas->dpy, ctxcanvas->region_aux, ctxcanvas->region_aux_gc, xmin, ymin, xmax-xmin+1, ymax-ymin+1);
    sCombineRegion(ctxcanvas);
  }
  else
    XFillRectangle(ctxcanvas->dpy, ctxcanvas->wnd, ctxcanvas->gc, xmin, ymin, xmax-xmin+1, ymax-ymin+1);
}

static int cd2xvertex [12] = {XR_TCENTRE, XR_BCENTRE, 
                              XR_MRIGHT,  XR_MLEFT, 
                              XR_TRIGHT,  XR_TLEFT, 
                              XR_BRIGHT,  XR_BLEFT, 
                              XR_MCENTRE, XR_LEFT, 
                              XR_CENTRE,  XR_RIGHT};

static void cdtext(cdCtxCanvas *ctxcanvas, int x, int y, const char *s, int len)
{
  int w, h, dir = -1;

  if (ctxcanvas->canvas->text_orientation != 0)
  {
    cdxCheckSolidStyle(ctxcanvas, 1);
  
    if (ctxcanvas->canvas->use_matrix)
      cdMatrixTransformPoint(ctxcanvas->xmatrix, x, y, &x, &y);

    if (ctxcanvas->canvas->new_region)
    {
      sPrepareRegion(ctxcanvas);
      XRotDrawString(ctxcanvas->dpy, ctxcanvas->font, ctxcanvas->canvas->text_orientation, 
                     ctxcanvas->region_aux, ctxcanvas->region_aux_gc, x, y, s, len,
                     cd2xvertex[ctxcanvas->canvas->text_alignment], 0);
      sCombineRegion(ctxcanvas);
    }
    else
      XRotDrawString(ctxcanvas->dpy, ctxcanvas->font, ctxcanvas->canvas->text_orientation, 
                     ctxcanvas->wnd, ctxcanvas->gc, x, y, s, len,
                     cd2xvertex[ctxcanvas->canvas->text_alignment], 0);

    cdxCheckSolidStyle(ctxcanvas, 0);
      
    return;
  }

  w = XTextWidth(ctxcanvas->font, s, len);
  h = ctxcanvas->font->ascent + ctxcanvas->font->descent;

  switch (ctxcanvas->canvas->text_alignment)
  {
  case CD_BASE_RIGHT:
  case CD_NORTH_EAST:
  case CD_EAST:
  case CD_SOUTH_EAST:
    x = x - w;    
    break;
  case CD_BASE_CENTER:
  case CD_CENTER:
  case CD_NORTH:
  case CD_SOUTH:
    x = x - w/2;  
    break;
  case CD_BASE_LEFT:
  case CD_NORTH_WEST:
  case CD_WEST:
  case CD_SOUTH_WEST:
    x = x;         
    break;
  }

  if (ctxcanvas->canvas->invert_yaxis)
    dir = 1;

  switch (ctxcanvas->canvas->text_alignment)
  {
  case CD_BASE_LEFT:
  case CD_BASE_CENTER:
  case CD_BASE_RIGHT:
    y = y;
    break;
  case CD_SOUTH_EAST:
  case CD_SOUTH_WEST:
  case CD_SOUTH:
    y = y - dir*ctxcanvas->font->descent;
    break;
  case CD_NORTH_EAST:
  case CD_NORTH:
  case CD_NORTH_WEST:
    y = y + dir*(h - ctxcanvas->font->descent);
    break;
  case CD_CENTER:
  case CD_EAST:
  case CD_WEST:
    y = y + dir*(h/2 - ctxcanvas->font->descent);
    break;
  }

  cdxCheckSolidStyle(ctxcanvas, 1);

  if (ctxcanvas->canvas->use_matrix)
    cdMatrixTransformPoint(ctxcanvas->xmatrix, x, y, &x, &y);

  if (ctxcanvas->canvas->new_region)
  {
    sPrepareRegion(ctxcanvas);
    XSetFont(ctxcanvas->dpy, ctxcanvas->region_aux_gc, ctxcanvas->font->fid);
    XDrawString(ctxcanvas->dpy, ctxcanvas->region_aux, ctxcanvas->region_aux_gc, x, y+1, s, len);
    sCombineRegion(ctxcanvas);
  }
  else
    XDrawString(ctxcanvas->dpy, ctxcanvas->wnd, ctxcanvas->gc, x, y+1, s, len);

  cdxCheckSolidStyle(ctxcanvas, 0);
}

static void cdgettextsize(cdCtxCanvas *ctxcanvas, const char *s, int len, int *width, int *height)
{
  if (!ctxcanvas->font) return;
  if (width)  *width  = XTextWidth(ctxcanvas->font, s, len);
  if (height) *height = ctxcanvas->font->ascent + ctxcanvas->font->descent;
}

void cdxPoly(cdCtxCanvas *ctxcanvas, int mode, cdPoint* poly, int n)
{
  int i;
  XPoint* pnt = NULL;
  
  if (mode != CD_BEZIER)
  {
    pnt = (XPoint*)malloc((n+1) * sizeof(XPoint)); /* XPoint uses short for coordinates */
    
    for (i = 0; i < n; i++)
    {
      int x = poly[i].x, 
          y = poly[i].y;

      if (ctxcanvas->canvas->use_matrix)
        cdMatrixTransformPoint(ctxcanvas->xmatrix, x, y, &x, &y);

      pnt[i].x = (short)x;
      pnt[i].y = (short)y;
    }
  }

  switch( mode )
  {
  case CD_FILL:
    if (ctxcanvas->canvas->new_region)
    {
      sPrepareRegion(ctxcanvas);
      XSetFillRule(ctxcanvas->dpy, ctxcanvas->region_aux_gc, ctxcanvas->canvas->fill_mode==CD_EVENODD?EvenOddRule:WindingRule);
      XFillPolygon(ctxcanvas->dpy, ctxcanvas->region_aux, ctxcanvas->region_aux_gc,
                   pnt, n, Complex, CoordModeOrigin);
      sCombineRegion(ctxcanvas);
    }
    else
    {
      XSetFillRule(ctxcanvas->dpy, ctxcanvas->gc, ctxcanvas->canvas->fill_mode==CD_EVENODD?EvenOddRule:WindingRule);
      XFillPolygon(ctxcanvas->dpy, ctxcanvas->wnd, ctxcanvas->gc,
                   pnt, n, Complex, CoordModeOrigin);
    }
    break;
  case CD_CLOSED_LINES:
    pnt[n].x = pnt[0].x;
    pnt[n].y = pnt[0].y;
    n++;
    /* continua */
  case CD_OPEN_LINES:
    {
      cdxCheckSolidStyle(ctxcanvas, 1);
      XDrawLines(ctxcanvas->dpy, ctxcanvas->wnd, ctxcanvas->gc, pnt, n, CoordModeOrigin);
      cdxCheckSolidStyle(ctxcanvas, 0);
      break;
    }
  case CD_CLIP:
    if (ctxcanvas->clip_polygon) XFreePixmap(ctxcanvas->dpy, ctxcanvas->clip_polygon);
    ctxcanvas->clip_polygon = build_clip_polygon(ctxcanvas, pnt, n);
    if (ctxcanvas->canvas->clip_mode == CD_CLIPPOLYGON) cdxClip(ctxcanvas, CD_CLIPPOLYGON);
    break;
  case CD_BEZIER:
    cdSimPolyBezier(ctxcanvas->canvas, poly, n);
    break;
  case CD_PATH:
    cdSimPolyPath(ctxcanvas->canvas, poly, n);
    break;
  }

  if (pnt) free(pnt);
}

/******************************************************/

static int byte_order(void)
{
  unsigned short us = 0xFF00;
  unsigned char *uc = (unsigned char *)&us;
  return (uc[0]==0xFF) ? MSBFirst : LSBFirst;
}

static void cdgetimagergb(cdCtxCanvas *ctxcanvas, unsigned char *r, unsigned char *g, unsigned char *b, int x, int y, int w, int h)
{
  int col, lin, pos;
  XImage *xi = XGetImage(ctxcanvas->dpy, ctxcanvas->wnd, x, y-h+1, w, h, ULONG_MAX, ZPixmap);
  if (!xi)
  {
    fprintf(stderr, "CanvasDraw: error getting image\n");
    return;
  }
  
  for (lin=0; lin<h; lin++)
  {
    for (col=0; col<w; col++)
    {
      pos = (h-lin-1)*w+col;
      cdxGetRGB(ctxcanvas, XGetPixel(xi, col, lin), r+pos, g+pos, b+pos);
    }
  }
  
  XDestroyImage(xi);
}

static long int* get_data_buffer(cdCtxCanvas *ctxcanvas, int size)
{
  if (!ctxcanvas->xidata)
  {
    ctxcanvas->xisize = size;
    ctxcanvas->xidata = (long int *)malloc(ctxcanvas->xisize);
  }
  else if (ctxcanvas->xisize < size)
  {
    ctxcanvas->xisize = size;
    ctxcanvas->xidata = (long int *)realloc(ctxcanvas->xidata, ctxcanvas->xisize);
  }

  if (!ctxcanvas->xidata)
    ctxcanvas->xisize = 0;

  return ctxcanvas->xidata;
}
  
static XImage *map2ximage(cdCtxCanvas *ctxcanvas, int ew, int eh, const unsigned char *index, const long int * colors, int by, int bx, int bw, int bh, int iw)
{
  long int match_table[256];
  int i, j, pal_size;
  unsigned long xcol;
  XImage *xim;
  int *fx, *fy, src, dst;
  unsigned char idx;
  
  xim = (XImage *) NULL;
  
  /* Como nao sabemos o tamanho da palette a priori, 
  teremos que ver qual o maior indice usado na imagem. */
  pal_size = 0;
  
  for (i=0; i<bh; i++) 
  {
    for (j=0; j<bw; j++) 
    {
      src = (i+by)*iw + j+bx;
      idx = index[src];
      if (idx > pal_size)
        pal_size = idx;
    }
  }
  
  pal_size++;

  for (i = 0; i < pal_size; i++)
    match_table[i] = cdxGetPixel(ctxcanvas, colors[i]);

  fx = cdGetZoomTable(ew, bw, bx);
  fy = cdGetZoomTable(eh, bh, by);

  switch (ctxcanvas->depth) 
  {
  case 8: 
    {
      unsigned char  *imagedata, *ip;
      int imew, nullCount;
    
      nullCount = (4 - (ew % 4)) & 0x03;  /* # of padding bytes per line */
      imew = ew + nullCount;
    
      /* Now get the image data - pad each scanline as necessary */
      imagedata = (unsigned char*)get_data_buffer(ctxcanvas, eh * imew);
      if (!imagedata) 
      {
        fprintf(stderr, "CanvasDraw: not enough memory putting image\n");
        return NULL;
      }
    
      for (i=0; i<eh; i++) 
      {
        ip = imagedata + (eh-1-i)*imew;

        for (j=0; j<ew; j++, ip++) 
        {
          src = (fy[i])*iw + fx[j];
          *ip = (unsigned char) match_table[index[src]];
        }
      }
    
      xim = XCreateImage(ctxcanvas->dpy,ctxcanvas->vis,ctxcanvas->depth,ZPixmap,0, (char *) imagedata,  ew,  eh, 32, imew);
      if (!xim) 
      {
        fprintf(stderr, "CanvasDraw: not enough memory putting image\n");
        return NULL;
      }
    }
    break;

  case 12:
  case 15:
  case 16: 
    {
      unsigned char *imagedata;
      unsigned short *ip, *tip;
    
      /* Now get the image data - pad each scanline as necessary */
      imagedata = (unsigned char*)get_data_buffer(ctxcanvas, 2*ew*eh);
      if (!imagedata) 
      {
        fprintf(stderr, "CanvasDraw: not enough memory putting image\n");
        return NULL;
      }
    
      xim = XCreateImage(ctxcanvas->dpy,ctxcanvas->vis,ctxcanvas->depth,ZPixmap,0, (char *) imagedata,  ew,  eh, 16, 0);
      if (!xim) 
      {
        fprintf(stderr, "CanvasDraw: not enough memory putting image\n");
        return NULL;
      }
    
      if (ctxcanvas->depth == 12 && xim->bits_per_pixel != 16) 
      {
        xim->data = NULL;
        XDestroyImage(xim);
        fprintf(stderr,"No code for this type of display (depth=%d, bperpix=%d)", ctxcanvas->depth, xim->bits_per_pixel);
        return NULL;
      }
    
      ip = (unsigned short*)(imagedata + (eh-1)*xim->bytes_per_line);

      for (i=0; i<eh; i++) 
      {
        for (j=0, tip=ip; j<ew; j++) 
        {
          src = (fy[i])*iw + fx[j];
          xcol = match_table[index[src]];
          
          if (xim->byte_order == MSBFirst) 
          {
            *tip++ = (unsigned short)(xcol & 0xffff);
          }
          else
          {
            /*  WAS *tip++ = ((xcol>>8) & 0xff) | ((xcol&0xff) << 8);  */
            *tip++ = (unsigned short)(xcol);
          }
        }

        ip -= ew;
      }
    }
    break;

  case 24:
  case 32: 
    {
      unsigned char  *imagedata, *ip, *tip;
      int do32;
    
      /* Now get the image data - pad each scanline as necessary */
      imagedata = (unsigned char*)get_data_buffer(ctxcanvas, 4*ew*eh);
      if (!imagedata) 
      {
        fprintf(stderr, "CanvasDraw: not enough memory putting image\n");
        return NULL;
      }
    
      xim = XCreateImage(ctxcanvas->dpy,ctxcanvas->vis,ctxcanvas->depth,ZPixmap,0, (char *) imagedata,  ew,  eh, 32, 0);
      if (!xim) 
      {
        fprintf(stderr, "CanvasDraw: not enough memory putting image\n");
        return NULL;
      }
    
      do32 = (xim->bits_per_pixel == 32? 1: 0);
    
      ip = imagedata + (eh-1)*xim->bytes_per_line;

      for (i=0; i<eh; i++) 
      {
        for (j=0, tip=ip; j<ew; j++) 
        {
          src = (fy[i])*iw + fx[j];
          xcol = match_table[index[src]];
        
          if (xim->byte_order == MSBFirst) 
          {
            if (do32) *tip++ = 0;
            *tip++ = (unsigned char)((xcol>>16) & 0xff);
            *tip++ = (unsigned char)((xcol>>8)  & 0xff);
            *tip++ = (unsigned char)( xcol      & 0xff);
          }
          else 
          {  /* LSBFirst */
            *tip++ = (unsigned char)( xcol      & 0xff);
            *tip++ = (unsigned char)((xcol>>8)  & 0xff);
            *tip++ = (unsigned char)((xcol>>16) & 0xff);
            if (do32) *tip++ = 0;
          }
        }

        ip -= xim->bytes_per_line;
      }
    }
    break;
  default: 
    {
      /* Now get the image data - pad each scanline as necessary */
      unsigned long* imagedata = (unsigned long*)get_data_buffer(ctxcanvas, 4*ew*eh);
      if (!imagedata) 
      {
        fprintf(stderr, "CanvasDraw: not enough memory putting image\n");
        return NULL;
      }
    
      xim = XCreateImage(ctxcanvas->dpy,ctxcanvas->vis,ctxcanvas->depth,ZPixmap,0, (char *) imagedata,  ew,  eh, 32, ew*4);
      if (!xim) 
      {
        fprintf(stderr, "CanvasDraw: not enough memory putting image\n");
        return NULL;
      }
    
      xim->bits_per_pixel = 32;
      xim->bytes_per_line = 4 * iw;
      xim->byte_order = byte_order();
      xim->bitmap_bit_order = MSBFirst;

      for (i=0; i<eh; i++) 
      {
        for (j=0; j<ew; j++) 
        {
          src = (fy[i])*iw + fx[j];
          dst = (eh-1 - i)*ew + j;
          imagedata[dst] = match_table[index[src]];
        }
      }
    }
    break;
  }
  
  free(fx);
  free(fy);

  return(xim);
}

static XImage *rgb2ximage(cdCtxCanvas *ctxcanvas, int ew, int eh, 
                          const unsigned char *red, const unsigned char *green, const unsigned char *blue, 
                          const unsigned char *alpha, XImage *oxi, 
                          int by, int bx, int bw, int bh, int iw)
{
/*
* if we're displaying on a TrueColor
* or DirectColor display, we've got all the colors we're going to need,
* and 'all we have to do' is convert 24-bit RGB pixels into whatever
* variation of RGB the X device in question wants.  No color allocation
* is involved.
*/
  int     i,j;
  XImage *xim;
  unsigned long r, g, b, rmask, gmask, bmask, xcol;
  int           rshift, gshift, bshift, bperpix, bperline, byte_order, cshift;
  int           maplen, src;
  unsigned char *lip, *ip, *imagedata, or, ob, og, al;
  int *fx, *fy;
  
  /* compute various shifting constants that we'll need... */
  rmask = ctxcanvas->vis->red_mask;
  gmask = ctxcanvas->vis->green_mask;
  bmask = ctxcanvas->vis->blue_mask;
  rshift = 7 - highbit(rmask);
  gshift = 7 - highbit(gmask);
  bshift = 7 - highbit(bmask);
  
  maplen = ctxcanvas->vis->map_entries;
  if (maplen>256) maplen=256;
  cshift = 7 - highbit((unsigned long) (maplen-1));
  
  xim = XCreateImage(ctxcanvas->dpy, ctxcanvas->vis, ctxcanvas->depth, ZPixmap, 0, NULL, ew,  eh, 32, 0);
  if (!xim) 
  {
    fprintf(stderr, "CanvasDraw: not enough memory putting image\n");
    return NULL;
  }
  
  bperline = xim->bytes_per_line;
  bperpix  = xim->bits_per_pixel;
  byte_order   = xim->byte_order;
  
  if (bperpix != 8 && bperpix != 16 && bperpix != 24 && bperpix != 32) 
  {
    XDestroyImage(xim);
    fprintf(stderr, "CanvasDraw: bpp=%d not supported!\n", bperpix);
    return NULL;
  }
  
  imagedata = (unsigned char*)get_data_buffer(ctxcanvas, eh * bperline);
  if (!imagedata)
  {
    XDestroyImage(xim);
    fprintf(stderr, "CanvasDraw: not enough memory putting image\n");
    return NULL;
  }

  fx = cdGetZoomTable(ew, bw, bx);
  fy = cdGetZoomTable(eh, bh, by);

  xim->data = (char *) imagedata;
  
  lip = imagedata + (eh-1)*bperline;

  for (i=0; i<eh; i++, lip -= bperline) 
  {
    for (j=0, ip=lip; j<ew; j++) 
    {
      src = fy[i]*iw + fx[j];

      if (alpha)
      {
        cdxGetRGB(ctxcanvas, XGetPixel(oxi, j, eh-i-1), &or, &og, &ob);
        al = alpha[src];
        r = CD_ALPHA_BLEND(red[src], or, al);
        g = CD_ALPHA_BLEND(green[src], og, al);
        b = CD_ALPHA_BLEND(blue[src], ob, al);
      }
      else
      {
        r = red[src];  
        g = green[src];  
        b = blue[src];
      }
      
      /* shift r,g,b so that high bit of 8-bit color specification is 
      * aligned with high bit of r,g,b-mask in visual, 
      * AND each component with its mask,
      * and OR the three components together
      */

#ifdef __cplusplus
      if (ctxcanvas->vis->c_class == DirectColor) 
#else
      if (ctxcanvas->vis->class == DirectColor) 
#endif
      {
        r = (unsigned long) cdxDirectColorTable[(r>>cshift) & 0xff] << cshift;
        g = (unsigned long) cdxDirectColorTable[(g>>cshift) & 0xff] << cshift;
        b = (unsigned long) cdxDirectColorTable[(b>>cshift) & 0xff] << cshift;
      }
      
      /* shift the bits around */
      if (rshift<0) r = r << (-rshift);
      else r = r >> rshift;
      
      if (gshift<0) g = g << (-gshift);
      else g = g >> gshift;
      
      if (bshift<0) b = b << (-bshift);
      else b = b >> bshift;
      
      r = r & rmask;
      g = g & gmask;
      b = b & bmask;
      
      xcol = r | g | b;
      
      if (bperpix == 32) 
      {
        if (byte_order == MSBFirst) {
          *ip++ = (unsigned char)((xcol>>24) & 0xff);
          *ip++ = (unsigned char)((xcol>>16) & 0xff);
          *ip++ = (unsigned char)((xcol>>8)  & 0xff);
          *ip++ = (unsigned char)( xcol      & 0xff);
        }
        else 
        {  /* LSBFirst */
          *ip++ = (unsigned char)( xcol      & 0xff);
          *ip++ = (unsigned char)((xcol>>8)  & 0xff);
          *ip++ = (unsigned char)((xcol>>16) & 0xff);
          *ip++ = (unsigned char)((xcol>>24) & 0xff);
        }
      }
      else if (bperpix == 24) 
      {
        if (byte_order == MSBFirst) 
        {
          *ip++ = (unsigned char)((xcol>>16) & 0xff);
          *ip++ = (unsigned char)((xcol>>8)  & 0xff);
          *ip++ = (unsigned char)( xcol      & 0xff);
        }
        else 
        {  /* LSBFirst */
          *ip++ = (unsigned char)( xcol      & 0xff);
          *ip++ = (unsigned char)((xcol>>8)  & 0xff);
          *ip++ = (unsigned char)((xcol>>16) & 0xff);
        }
      }
      else if (bperpix == 16) 
      {
        if (byte_order == MSBFirst) 
        {
          *ip++ = (unsigned char)((xcol>>8)  & 0xff);
          *ip++ = (unsigned char)( xcol      & 0xff);
        }
        else {  /* LSBFirst */
          *ip++ = (unsigned char)( xcol      & 0xff);
          *ip++ = (unsigned char)((xcol>>8)  & 0xff);
        }
      }
      else if (bperpix == 8) 
      {
        *ip++ =  (unsigned char)(xcol      & 0xff);
      }
    }
  }
  
  free(fx);
  free(fy);

  return xim;
}

static void cdputimagerectrgba_matrix(cdCtxCanvas* ctxcanvas, int iw, int ih, const unsigned char *r, const unsigned char *g, const unsigned char *b, const unsigned char *a, int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax)
{
  int t_xmin, t_xmax, t_ymin, t_ymax, ew, eh,
      t_x, t_y, dst_offset, size, nc, doff, rect[8];
  float i_x, i_y, xfactor, yfactor;
  unsigned char *dst_r, *dst_g, *dst_b, *dst_a = NULL;
  double inv_matrix[6];

  /* calculate the destination limits */
  cdImageRGBCalcDstLimits(ctxcanvas->canvas, x, y, w, h, &t_xmin, &t_xmax, &t_ymin, &t_ymax, rect);

  /* Setup inverse transform (use the original transform here, NOT ctxcanvas->xmatrix) */
  cdImageRGBInitInverseTransform(w, h, xmin, xmax, ymin, ymax, &xfactor, &yfactor, ctxcanvas->canvas->matrix, inv_matrix);

  /* create an image for the destination area */
  ew = (t_xmax-t_xmin+1);
  eh = (t_ymax-t_ymin+1); 
  size = ew*eh;
  nc = 3;
  if (a) nc = 4;
  dst_r = malloc(nc*size);
  if (!dst_r)
  {
    fprintf(stderr, "CanvasDraw: no enough memory\n");
    return;
  }
  dst_g = dst_r + size;
  dst_b = dst_g + size;
  if (a) dst_a = dst_b + size;
  memset(dst_r, 0, nc*size);

  /* for all pixels in the destiny area */
  for(t_y = t_ymin; t_y <= t_ymax; t_y++)
  {
    dst_offset = (t_y-t_ymin) * ew;

    for(t_x = t_xmin; t_x <= t_xmax; t_x++)
    {
      cdImageRGBInverseTransform(t_x, t_y, &i_x, &i_y, xfactor, yfactor, xmin, ymin, x, y, inv_matrix);

      if (i_x > xmin && i_y > ymin && i_x < xmax+1 && i_y < ymax+1)
      {
        doff = (t_x-t_xmin) + dst_offset;
        *(dst_r+doff) = cdBilinearInterpolation(iw, ih, r, i_x, i_y);
        *(dst_g+doff) = cdBilinearInterpolation(iw, ih, g, i_x, i_y);
        *(dst_b+doff) = cdBilinearInterpolation(iw, ih, b, i_x, i_y);
        if (a) *(dst_a+doff) = cdBilinearInterpolation(iw, ih, a, i_x, i_y);
      }
    }
  }

  {
    int ex = t_xmin, 
        ey = t_ymin + eh-1;  /* XImage origin is at top-left */
    XImage *xi, *oxi = NULL;
    Pixmap clip_polygon, clip_mask = 0;
    XPoint pnt[4];

    /* Since the transformation used was the original transformation, */
    /* must invert the Y axis here. */
    ey = _cdInvertYAxis(ctxcanvas->canvas, ey);

    /* use clipping to select only the transformed rectangle */
    pnt[0].x = (short)rect[0]; pnt[0].y = (short)_cdInvertYAxis(ctxcanvas->canvas, rect[1]);
    pnt[1].x = (short)rect[2]; pnt[1].y = (short)_cdInvertYAxis(ctxcanvas->canvas, rect[3]);
    pnt[2].x = (short)rect[4]; pnt[2].y = (short)_cdInvertYAxis(ctxcanvas->canvas, rect[5]);
    pnt[3].x = (short)rect[6]; pnt[3].y = (short)_cdInvertYAxis(ctxcanvas->canvas, rect[7]);
    clip_polygon = build_clip_polygon(ctxcanvas, pnt, 4);

    /* combine with the existing clipping */
    if (ctxcanvas->canvas->clip_mode == CD_CLIPAREA || ctxcanvas->canvas->clip_mode == CD_CLIPPOLYGON)
      clip_mask = ctxcanvas->clip_polygon;
    else if (ctxcanvas->canvas->clip_mode == CD_CLIPREGION)
      clip_mask = ctxcanvas->new_region;
    XSetFunction(ctxcanvas->dpy, ctxcanvas->gc, GXand);
    XCopyArea(ctxcanvas->dpy, clip_mask, clip_polygon, ctxcanvas->gc,
              0, 0, ctxcanvas->canvas->w, ctxcanvas->canvas->h, 0, 0);  
    XSetClipMask(ctxcanvas->dpy, ctxcanvas->gc, clip_polygon);
    cdwritemode(ctxcanvas, ctxcanvas->canvas->write_mode); /* reset XSetFunction */

    if (a)
    {
      oxi = XGetImage(ctxcanvas->dpy, ctxcanvas->wnd, ex, ey, ew, eh, ULONG_MAX, ZPixmap);
      if (!oxi)
      {
        fprintf(stderr, "CanvasDraw: error getting image\n");
        free(dst_r);
        return;
      }
    }

    xi = rgb2ximage(ctxcanvas, ew, eh, dst_r, dst_g, dst_b, dst_a, oxi, 0, 0, ew, eh, ew);
    if (!xi)
      return;

    XPutImage(ctxcanvas->dpy, ctxcanvas->wnd, ctxcanvas->gc, xi, 0, 0, ex, ey, ew, eh);

    /* reset cliping */
    XFreePixmap(ctxcanvas->dpy, clip_polygon);
    cdxClip(ctxcanvas, ctxcanvas->canvas->clip_mode);

    xi->data = NULL;
    XDestroyImage(xi);
    if (oxi) XDestroyImage(oxi);
  }

  free(dst_r);
}

static void cdputimagerectmap_matrix(cdCtxCanvas* ctxcanvas, int iw, int ih, const unsigned char *index, const long int *colors, int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax)
{
  int t_xmin, t_xmax, t_ymin, t_ymax, ew, eh,
      t_x, t_y, dst_offset, size, doff, rect[8];
  float i_x, i_y, xfactor, yfactor;
  unsigned char *dst_index;
  double inv_matrix[6];

  /* calculate the destination limits */
  cdImageRGBCalcDstLimits(ctxcanvas->canvas, x, y, w, h, &t_xmin, &t_xmax, &t_ymin, &t_ymax, rect);

  /* Setup inverse transform (use the original transform here, NOT ctxcanvas->xmatrix) */
  cdImageRGBInitInverseTransform(w, h, xmin, xmax, ymin, ymax, &xfactor, &yfactor, ctxcanvas->canvas->matrix, inv_matrix);

  /* create an image for the destination area */
  ew = (t_xmax-t_xmin+1);
  eh = (t_ymax-t_ymin+1); 
  size = ew*eh;
  dst_index = malloc(size);
  if (!dst_index)
  {
    fprintf(stderr, "CanvasDraw: no enough memory\n");
    return;
  }
  memset(dst_index, 0, size);

  /* for all pixels in the destiny area */
  for(t_y = t_ymin; t_y <= t_ymax; t_y++)
  {
    dst_offset = (t_y-t_ymin) * ew;

    for(t_x = t_xmin; t_x <= t_xmax; t_x++)
    {
      cdImageRGBInverseTransform(t_x, t_y, &i_x, &i_y, xfactor, yfactor, xmin, ymin, x, y, inv_matrix);

      if (i_x > xmin && i_y > ymin && i_x < xmax+1 && i_y < ymax+1)
      {
        doff = (t_x-t_xmin) + dst_offset;
        *(dst_index+doff) = cdZeroOrderInterpolation(iw, ih, index, i_x, i_y);
      }
    }
  }

  {
    int ex = t_xmin, 
        ey = t_ymin + eh-1;  /* XImage origin is at top-left */
    XImage *xi;
    Pixmap clip_polygon, clip_mask = 0;
    XPoint pnt[4];

    /* Since the transformation used was the original transformation, */
    /* must invert the Y axis here. */
    ey = _cdInvertYAxis(ctxcanvas->canvas, ey);

    /* use clipping to select only the transformed rectangle */
    pnt[0].x = (short)rect[0]; pnt[0].y = (short)_cdInvertYAxis(ctxcanvas->canvas, rect[1]);
    pnt[1].x = (short)rect[2]; pnt[1].y = (short)_cdInvertYAxis(ctxcanvas->canvas, rect[3]);
    pnt[2].x = (short)rect[4]; pnt[2].y = (short)_cdInvertYAxis(ctxcanvas->canvas, rect[5]);
    pnt[3].x = (short)rect[6]; pnt[3].y = (short)_cdInvertYAxis(ctxcanvas->canvas, rect[7]);
    clip_polygon = build_clip_polygon(ctxcanvas, pnt, 4);

    /* combine with the existing clipping */
    if (ctxcanvas->canvas->clip_mode == CD_CLIPAREA || ctxcanvas->canvas->clip_mode == CD_CLIPPOLYGON)
      clip_mask = ctxcanvas->clip_polygon;
    else if (ctxcanvas->canvas->clip_mode == CD_CLIPREGION)
      clip_mask = ctxcanvas->new_region;
    XSetFunction(ctxcanvas->dpy, ctxcanvas->gc, GXand);
    XCopyArea(ctxcanvas->dpy, clip_mask, clip_polygon, ctxcanvas->gc,
              0, 0, ctxcanvas->canvas->w, ctxcanvas->canvas->h, 0, 0);  
    XSetClipMask(ctxcanvas->dpy, ctxcanvas->gc, clip_polygon);
    cdwritemode(ctxcanvas, ctxcanvas->canvas->write_mode); /* reset XSetFunction */

    xi = map2ximage(ctxcanvas, ew, eh, dst_index, colors, 0, 0, ew, eh, ew);
    if (!xi)
      return;

    XPutImage(ctxcanvas->dpy, ctxcanvas->wnd, ctxcanvas->gc, xi, 0, 0, ex, ey, ew, eh);

    /* reset cliping */
    XFreePixmap(ctxcanvas->dpy, clip_polygon);
    cdxClip(ctxcanvas, ctxcanvas->canvas->clip_mode);

    xi->data = NULL;
    XDestroyImage(xi);
  }

  free(dst_index);
}

static void cdputimagerectrgb(cdCtxCanvas *ctxcanvas, int iw, int ih, const unsigned char *r, const unsigned char *g, const unsigned char *b, int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax)
{
  int ew = w, eh = h, ex = x, ey = y;
  int bw = iw, bh = ih, bx = 0, by = 0;
  int rw, rh;
  XImage *xi;

  if (ctxcanvas->canvas->use_matrix)
  {
    cdputimagerectrgba_matrix(ctxcanvas, iw, ih, r, g, b, NULL, x, y, w, h, xmin, xmax, ymin, ymax);
    return;
  }

  rw = xmax-xmin+1;
  rh = ymax-ymin+1;
  y -= (h - 1);        /* XImage origin is at top-left */

  if (!cdCalcZoom(ctxcanvas->canvas->w, x, w, &ex, &ew, xmin, rw, &bx, &bw, 1))
    return;
  
  if (!cdCalcZoom(ctxcanvas->canvas->h, y, h, &ey, &eh, ymin, rh, &by, &bh, 0))
    return;

  xi = rgb2ximage(ctxcanvas, ew, eh, r, g, b, NULL, NULL, by, bx, bw, bh, iw);
  if (!xi)
    return;

  XPutImage(ctxcanvas->dpy, ctxcanvas->wnd, ctxcanvas->gc, xi, 0, 0, ex, ey, ew, eh);

  xi->data = NULL;
  XDestroyImage(xi);
}

static void cdputimagerectrgba(cdCtxCanvas *ctxcanvas, int iw, int ih, const unsigned char *r, const unsigned char *g, const unsigned char *b, const unsigned char *a, int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax)
{
  XImage *xi, *oxi;
  int ew = w, eh = h, ex = x, ey = y;
  int bw = iw, bh = ih, bx = 0, by = 0;
  int rw, rh;

  if (ctxcanvas->canvas->use_matrix)
  {
    cdputimagerectrgba_matrix(ctxcanvas, iw, ih, r, g, b, a, x, y, w, h, xmin, xmax, ymin, ymax);
    return;
  }

  rw = xmax-xmin+1;
  rh = ymax-ymin+1;
  y -= (h - 1);        /* XImage origin is at top-left */

  if (!cdCalcZoom(ctxcanvas->canvas->w, x, w, &ex, &ew, xmin, rw, &bx, &bw, 1))
    return;
  
  if (!cdCalcZoom(ctxcanvas->canvas->h, y, h, &ey, &eh, ymin, rh, &by, &bh, 0))
    return;

  oxi = XGetImage(ctxcanvas->dpy, ctxcanvas->wnd, ex, ey, ew, eh, ULONG_MAX, ZPixmap);
  if (!oxi)
  {
    fprintf(stderr, "CanvasDraw: error getting image\n");
    return;
  }

  xi = rgb2ximage(ctxcanvas, ew, eh, r, g, b, a, oxi, by, bx, bw, bh, iw);
  if (!xi)
    return;

  XPutImage(ctxcanvas->dpy, ctxcanvas->wnd, ctxcanvas->gc, xi, 0, 0, ex, ey, ew, eh);

  xi->data = NULL;
  XDestroyImage(xi);
  XDestroyImage(oxi);
}

static void cdputimagerectmap(cdCtxCanvas *ctxcanvas, int iw, int ih, const unsigned char *index, const long int *colors, int x, int y, int w, int h, int xmin, int xmax, int ymin, int ymax)
{
  int ew = w, eh = h, ex = x, ey = y;
  int bw = iw, bh = ih, bx = 0, by = 0;
  int rw, rh;
  XImage *xi;

  if (ctxcanvas->canvas->use_matrix)
  {
    cdputimagerectmap_matrix(ctxcanvas, iw, ih, index, colors, x, y, w, h, xmin, xmax, ymin, ymax);
    return;
  }

  rw = xmax-xmin+1;
  rh = ymax-ymin+1;
  y -= (h - 1);        /* XImage origin is at top-left */

  if (!cdCalcZoom(ctxcanvas->canvas->w, x, w, &ex, &ew, xmin, rw, &bx, &bw, 1))
    return;
  
  if (!cdCalcZoom(ctxcanvas->canvas->h, y, h, &ey, &eh, ymin, rh, &by, &bh, 0))
    return;

  xi = map2ximage(ctxcanvas, ew, eh, index, colors, by, bx, bw, bh, iw);
  if (!xi)
    return;

  XPutImage(ctxcanvas->dpy, ctxcanvas->wnd, ctxcanvas->gc, xi, 0, 0, ex, ey, ew, eh);

  xi->data = NULL;
  XDestroyImage(xi);
}

static void cdpixel(cdCtxCanvas *ctxcanvas, int x, int y, long int color)
{
  if (ctxcanvas->canvas->foreground != color)
    XSetForeground(ctxcanvas->dpy, ctxcanvas->gc, cdxGetPixel(ctxcanvas, color));

  if (ctxcanvas->canvas->use_matrix)
    cdMatrixTransformPoint(ctxcanvas->xmatrix, x, y, &x, &y);

  XDrawPoint(ctxcanvas->dpy, ctxcanvas->wnd, ctxcanvas->gc, x, y);

  if (ctxcanvas->canvas->foreground != color)
    XSetForeground(ctxcanvas->dpy, ctxcanvas->gc, ctxcanvas->fg);
}

static cdCtxImage *cdcreateimage (cdCtxCanvas *ctxcanvas, int w, int h)
{
  GC gc;
  cdCtxImage *ctximage = (cdCtxImage *)malloc(sizeof(cdCtxImage));

  ctximage->w = w;
  ctximage->h = h;
  ctximage->depth = ctxcanvas->depth;
  ctximage->dpy = ctxcanvas->dpy;
  ctximage->scr = ctxcanvas->scr;
  ctximage->vis = ctxcanvas->vis;

  ctximage->img = XCreatePixmap(ctxcanvas->dpy, ctxcanvas->wnd, w, h, ctxcanvas->depth);
  if (!ctximage->img)
  {
    free(ctximage);
    return (void *)0;
  }

  gc = XCreateGC(ctximage->dpy, ctximage->img, 0, NULL);
  XSetForeground(ctximage->dpy, gc, cdxGetPixel(ctxcanvas, CD_WHITE));
  XFillRectangle(ctximage->dpy, ctximage->img, gc, 0, 0, ctximage->w, ctxcanvas->canvas->h);
  XFreeGC(ctximage->dpy, gc);

  return (void *)ctximage;
}

static void cdgetimage (cdCtxCanvas *ctxcanvas, cdCtxImage *ctximage, int x, int y)
{
  /* y is the bottom-left of the image in CD, must be at upper-left */
  y -= ctximage->h-1;

  XCopyArea(ctxcanvas->dpy, ctxcanvas->wnd, ctximage->img, ctxcanvas->gc,
            x, y, ctximage->w, ctximage->h, 0, 0);
}

static void cdputimagerect (cdCtxCanvas *ctxcanvas, cdCtxImage *ctximage, int x, int y, int xmin, int xmax, int ymin, int ymax)
{
  XCopyArea(ctxcanvas->dpy, ctximage->img, ctxcanvas->wnd, ctxcanvas->gc,
            xmin, ctximage->h-ymax-1, xmax-xmin+1, ymax-ymin+1, x, y-(ymax-ymin+1)+1);
}

static void cdkillimage (cdCtxImage *ctximage)
{
  XFreePixmap(ctximage->dpy, ctximage->img);
  free(ctximage);
}

static void cdscrollarea (cdCtxCanvas *ctxcanvas, int xmin, int xmax, int ymin, int ymax, int dx, int dy)
{
  XCopyArea(ctxcanvas->dpy, ctxcanvas->wnd, ctxcanvas->wnd, ctxcanvas->gc,
            xmin, ymin,
            xmax-xmin+1, ymax-ymin+1,
            xmin+dx, ymin+dy);
}

static void cdtransform(cdCtxCanvas *ctxcanvas, const double* matrix)
{
  if (matrix)
  {
    /* configure a bottom-up coordinate system */
    ctxcanvas->xmatrix[0] = 1; 
    ctxcanvas->xmatrix[1] = 0;
    ctxcanvas->xmatrix[2] = 0; 
    ctxcanvas->xmatrix[3] = -1; 
    ctxcanvas->xmatrix[4] = 0; 
    ctxcanvas->xmatrix[5] = (ctxcanvas->canvas->h-1); 
    cdMatrixMultiply(matrix, ctxcanvas->xmatrix);

    ctxcanvas->canvas->invert_yaxis = 0;
  }
  else
  {
    ctxcanvas->canvas->invert_yaxis = 1;
  }
}

/******************************************************************/

static void set_rotate_attrib(cdCtxCanvas* ctxcanvas, char* data)
{
  if (data)
  {
    /* use this configuration when there is NO native tranformation support */
    sscanf(data, "%g %d %d", &ctxcanvas->rotate_angle,
                             &ctxcanvas->rotate_center_x,
                             &ctxcanvas->rotate_center_y);

    cdCanvasTransformTranslate(ctxcanvas->canvas, ctxcanvas->rotate_center_x, ctxcanvas->rotate_center_y);
    cdCanvasTransformRotate(ctxcanvas->canvas, ctxcanvas->rotate_angle);
    cdCanvasTransformTranslate(ctxcanvas->canvas, -ctxcanvas->rotate_center_x, -ctxcanvas->rotate_center_y);
  }
  else
  {
    ctxcanvas->rotate_angle = 0;
    ctxcanvas->rotate_center_x = 0;
    ctxcanvas->rotate_center_y = 0;

    cdCanvasTransform(ctxcanvas->canvas, NULL);
  }
}

static char* get_rotate_attrib(cdCtxCanvas* ctxcanvas)
{
  static char data[100];

  if (!ctxcanvas->rotate_angle)
    return NULL;

  sprintf(data, "%g %d %d", (double)ctxcanvas->rotate_angle,
                            ctxcanvas->rotate_center_x,
                            ctxcanvas->rotate_center_y);

  return data;
}

static cdAttribute rotate_attrib =
{
  "ROTATE",
  set_rotate_attrib,
  get_rotate_attrib
}; 

static char* get_gc_attrib(cdCtxCanvas *ctxcanvas)
{
  return (char*)ctxcanvas->gc;
}

static cdAttribute gc_attrib =
{
  "GC",
  NULL,
  get_gc_attrib
}; 

static void get_geometry(Display *dpy, Drawable wnd, cdCtxCanvas *ctxcanvas)
{
  Window root;
  int x, y;
  unsigned int w, h, b, d;
  XGetGeometry(dpy, wnd, &root, &x, &y, &w, &h, &b, &d);
  ctxcanvas->canvas->w = w;
  ctxcanvas->canvas->h = h;
  ctxcanvas->depth = d;
}

cdCtxCanvas *cdxCreateCanvas(cdCanvas* canvas, Display *dpy, int scr, Drawable wnd, Visual *vis)
{
  static int first = 1;
  cdCtxCanvas *ctxcanvas = (cdCtxCanvas *)malloc(sizeof(cdCtxCanvas));
  memset(ctxcanvas, 0, sizeof(cdCtxCanvas));

  ctxcanvas->dpy = dpy;
  ctxcanvas->scr = scr;
  ctxcanvas->wnd = wnd;
  ctxcanvas->vis = vis;
  ctxcanvas->gc = XCreateGC(dpy, wnd, 0, NULL);
  if (ctxcanvas->gc == 0) 
  {
    free(canvas);
    return NULL;
  }

  ctxcanvas->canvas = canvas;
  canvas->ctxcanvas = ctxcanvas;
  
  get_geometry(dpy, wnd, ctxcanvas);

  canvas->bpp = ctxcanvas->depth;
  canvas->xres = ((double)DisplayWidth(dpy, scr) / (double)DisplayWidthMM(dpy, scr));
  canvas->yres = ((double)DisplayHeight(dpy, scr) / (double)DisplayHeightMM(dpy, scr));
  canvas->w_mm = ((double)canvas->w) / canvas->xres;
  canvas->h_mm = ((double)canvas->h) / canvas->yres;
  canvas->invert_yaxis = 1;

  if (first)
  {
    if (canvas->bpp > 8)
    {
      cdxGetRGB = truecolor_get_rgb;
      cdxGetPixel = truecolor_get_pixel;

       /* make linear colormap for DirectColor visual */
#ifdef __cplusplus
      if (ctxcanvas->vis->c_class == DirectColor) 
#else
      if (ctxcanvas->vis->class == DirectColor) 
#endif
        makeDirectCmap(ctxcanvas, DefaultColormap(ctxcanvas->dpy, ctxcanvas->scr));
    }
    else
    {
      cdxGetRGB = not_truecolor_get_rgb;
      cdxGetPixel = not_truecolor_get_pixel;
    }
  }
  
  if (canvas->bpp > 8)
  {
    ctxcanvas->rshift = 15 - highbit(ctxcanvas->vis->red_mask);
    ctxcanvas->gshift = 15 - highbit(ctxcanvas->vis->green_mask);
    ctxcanvas->bshift = 15 - highbit(ctxcanvas->vis->blue_mask);
  
    ctxcanvas->num_colors = 0;
    ctxcanvas->colormap = (Colormap)0;

    /* para canvas bpp <= 8 RGBA e' simulado com cdGetImageRGB */
    canvas->cxPutImageRectRGBA = cdputimagerectrgba;
  }
  else
  {
    int i;

    ctxcanvas->colormap = DefaultColormap(dpy, scr);
    ctxcanvas->num_colors = 1L << canvas->bpp;

    for (i=0; i<ctxcanvas->num_colors; i++) 
      ctxcanvas->color_table[i].pixel = i;

    update_colors(ctxcanvas);
  }

  if (first)
  {
    if (!getenv("CD_XERROR"))
      XSetErrorHandler(cdxErrorHandler);
  }

  cdRegisterAttribute(canvas, &gc_attrib);
  cdRegisterAttribute(canvas, &rotate_attrib);

  first = 0;

  return ctxcanvas;
}

void cdxInitTable(cdCanvas* canvas)
{
  canvas->cxFlush = cdflush;
  canvas->cxClear = cdclear;
  canvas->cxPixel = cdpixel;
  canvas->cxLine = cdline;
  canvas->cxPoly = cdxPoly;
  canvas->cxRect = cdrect;
  canvas->cxBox = cdbox;
  canvas->cxArc = cdarc;
  canvas->cxSector = cdsector;
  canvas->cxChord = cdchord;
  canvas->cxText = cdtext;

  canvas->cxNewRegion = cdnewregion;
  canvas->cxIsPointInRegion = cdispointinregion;
  canvas->cxOffsetRegion = cdoffsetregion;
  canvas->cxGetRegionBox = cdgetregionbox;
  canvas->cxClip = cdxClip;
  canvas->cxClipArea = cdcliparea;
  canvas->cxWriteMode = cdwritemode;
  canvas->cxLineStyle = cdlinestyle;
  canvas->cxLineWidth = cdlinewidth;
  canvas->cxLineCap = cdlinecap;
  canvas->cxLineJoin = cdlinejoin;
  canvas->cxBackOpacity = cdbackopacity;
  canvas->cxInteriorStyle = cdinteriorstyle;
  canvas->cxHatch = cdhatch;
  canvas->cxStipple = cdstipple;
  canvas->cxPattern = cdpattern;
  canvas->cxFont = cdfont;
  canvas->cxNativeFont = cdnativefont;
  canvas->cxGetFontDim = cdgetfontdim;
  canvas->cxGetTextSize = cdgettextsize;
  canvas->cxPalette = cdpalette;
  canvas->cxBackground = cdbackground;
  canvas->cxForeground = cdforeground;
  canvas->cxTransform = cdtransform;

  canvas->cxGetImageRGB = cdgetimagergb;
  canvas->cxScrollArea = cdscrollarea;

  canvas->cxCreateImage = cdcreateimage;
  canvas->cxGetImage = cdgetimage;
  canvas->cxPutImageRect = cdputimagerect;
  canvas->cxKillImage = cdkillimage;

  canvas->cxPutImageRectRGB = cdputimagerectrgb;
  canvas->cxPutImageRectMap = cdputimagerectmap;

  if (canvas->bpp > 8)
    canvas->cxPutImageRectRGBA = cdputimagerectrgba;
}

int cdBaseDriver(void)
{
  return CD_BASE_X;
}
