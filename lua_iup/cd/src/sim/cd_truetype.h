/** \file
 * \brief Text and Font Simulation using FreeType library.
 *
 * See Copyright Notice in cd.h
 */

#ifndef __TRUETYPE_H
#define __TRUETYPE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ft2build.h"
#include FT_FREETYPE_H

/*
   In CD version 4.4 we start to use FreeType 2.
   Only TrueType font support is enabled.
*/

typedef struct _cdTT_Text
{
  FT_Library library;
  FT_Face face;          

  unsigned char* rgba_data;   /* the image where one character is drawn with the foreground color during text output */
  int rgba_data_size;

  int max_height;
  int max_width;
  int descent;
  int ascent;

}cdTT_Text;

cdTT_Text* cdTT_create(void);
void cdTT_free(cdTT_Text * tt_text);
int cdTT_load(cdTT_Text * tt_text, const char *font,int size, double xres, double yres);

#ifdef __cplusplus
}
#endif

#endif  /* ifndef _CD_TRUETYPE_ */

