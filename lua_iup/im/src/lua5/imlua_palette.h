/** \file
 * \brief IM Lua 5 Binding
 *
 * See Copyright Notice in im_lib.h
 * $Id: imlua_palette.h,v 1.1 2008/10/17 06:16:32 scuri Exp $
 */

#ifndef __IMLUA_PALETTE_H
#define __IMLUA_PALETTE_H

#if	defined(__cplusplus)
extern "C" {
#endif


/* this is the same declaration used in the CD toolkit for cdPalette in Lua */
typedef struct _imPalette {
  long* color;
  int count;
} imluaPalette;

void imlua_pushpalette(lua_State *L, long* color, int count);
imluaPalette* imlua_checkpalette (lua_State *L, int param);

void imlua_open_palette(lua_State *L);


#ifdef __cplusplus
}
#endif

#endif
