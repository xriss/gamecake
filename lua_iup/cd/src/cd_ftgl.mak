PROJNAME = cd
LIBNAME = ftgl
OPT = YES

DEF_FILE = cd_ftgl.def

ftglyph_sources = \
    FTGlyph/FTGlyph.cpp \
    FTGlyph/FTGlyphGlue.cpp \
    FTGlyph/FTBitmapGlyph.cpp \
    FTGlyph/FTBufferGlyph.cpp \
    FTGlyph/FTExtrudeGlyph.cpp \
    FTGlyph/FTOutlineGlyph.cpp \
    FTGlyph/FTPixmapGlyph.cpp \
    FTGlyph/FTPolygonGlyph.cpp \
    FTGlyph/FTTextureGlyph.cpp

ftfont_sources = \
    FTFont/FTFont.cpp \
    FTFont/FTFontGlue.cpp \
    FTFont/FTBitmapFont.cpp \
    FTFont/FTBufferFont.cpp \
    FTFont/FTExtrudeFont.cpp \
    FTFont/FTOutlineFont.cpp \
    FTFont/FTPixmapFont.cpp \
    FTFont/FTPolygonFont.cpp \
    FTFont/FTTextureFont.cpp

ftlayout_sources = \
    FTLayout/FTLayout.cpp \
    FTLayout/FTLayoutGlue.cpp \
    FTLayout/FTSimpleLayout.cpp
    
libftgl_la_SOURCES = \
    FTBuffer.cpp \
    FTCharmap.cpp \
    FTContour.cpp \
    FTFace.cpp \
    FTGlyphContainer.cpp \
    FTLibrary.cpp \
    FTPoint.cpp \
    FTSize.cpp \
    FTVectoriser.cpp \
    $(ftglyph_sources) \
    $(ftfont_sources) \
    $(ftlayout_sources)
SRC := $(addprefix ftgl/, $(libftgl_la_SOURCES))
    
INCLUDES := ftgl freetype2
DEFINES = FTGL_LIBRARY_STATIC
USE_OPENGL = Yes
LIBS = freetype
LDIR = ../lib/$(TEC_UNAME)

ifneq ($(findstring dll, $(TEC_UNAME)), )
  DEFINES = FTGL_LIBRARY
endif

ifneq ($(findstring Win, $(TEC_SYSNAME)), )
  LIBS = freetype6
endif

ifneq ($(findstring cygw, $(TEC_UNAME)), )
  LIBS = freetype-6
endif

ifneq ($(findstring MacOS, $(TEC_UNAME)), )
  ifneq ($(TEC_SYSMINOR), 4)
    BUILD_DYLIB=Yes
  endif
endif
