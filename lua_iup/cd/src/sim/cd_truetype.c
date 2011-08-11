/** \file
 * \brief Text and Font Simulation using FreeType library.
 *
 * See Copyright Notice in cd.h
 */

#include <string.h>
#include <stdlib.h>
#include <memory.h>
#include <stdio.h>

#include "cd.h"
#include "cd_private.h"
#include "cd_truetype.h"

/*******************************************
        Inicializa o Rasterizador
********************************************/

int cdTT_load(cdTT_Text * tt_text, const char *font, int size, double xres, double yres)
{
  char filename[10240];
  FT_Error error;
  FT_Face face;          

  if (!cdGetFontFileName(font, filename))
    return 0;

  error = FT_New_Face(tt_text->library, filename, 0, &face );
  if (error) 
    return 0;

  /* char_height is 1/64th of points */
  error = FT_Set_Char_Size(face, 0, size*64, (int)(xres*25.4), (int)(yres*25.4));  
  if (error) 
  {
    FT_Done_Face(face);
    return 0;
  }

  if (tt_text->face && tt_text->face != face)
    FT_Done_Face(tt_text->face);

  tt_text->face = face;

  tt_text->ascent     =  face->size->metrics.ascender >> 6;
  tt_text->descent    =  abs(face->size->metrics.descender >> 6);
  tt_text->max_height =  face->size->metrics.height >> 6;
  tt_text->max_width  =  face->size->metrics.max_advance >> 6;

  if (!face->charmap)
    FT_Set_Charmap(face, face->charmaps[0]);

  return 1;
}

static void cdTT_checkversion(cdTT_Text* tt_text)
{
  FT_Int major, minor, patch;
  FT_Library_Version(tt_text->library, &major, &minor, &patch);
  if (major != FREETYPE_MAJOR ||
      minor != FREETYPE_MINOR)
  {
    printf("CD - Canvas Draw: Warning - Different FreeType library used!\n"
           "    Compiled = %d.%d.%d\n"
           "    RunTime  = %d.%d.%d\n",
           FREETYPE_MAJOR, FREETYPE_MINOR, FREETYPE_PATCH, major, minor, patch);
  }
}

/*******************************************
              Inicializaccao 
********************************************/
cdTT_Text* cdTT_create(void)
{
  static int first = 1;
  cdTT_Text * tt_text = malloc(sizeof(cdTT_Text));
  memset(tt_text, 0, sizeof(cdTT_Text));
  
  FT_Init_FreeType(&tt_text->library);

  if (first)
  {
    cdTT_checkversion(tt_text);
    first = 0;
  }

  return tt_text;
}

/*******************************************
            Desaloca Recursos
********************************************/
void cdTT_free(cdTT_Text * tt_text)
{
  if (tt_text->rgba_data)
    free(tt_text->rgba_data);

  if (tt_text->face)
    FT_Done_Face(tt_text->face);

  FT_Done_FreeType(tt_text->library);

  free(tt_text);
}

#ifdef SunOS_OLD
void *memmove( void *dest, const void *src, size_t count )
{
  return memcpy(dest, src, count);
}
#endif
