PROJNAME = cd
LIBNAME = cdgl
OPT = YES

DEFINES = CD_NO_OLD_INTERFACE
SRC = drv/cdgl.c

INCLUDES = . sim ftgl freetype2
LIBS = ftgl

USE_OPENGL = YES
USE_CD = YES
CD = ..

ifneq ($(findstring Win, $(TEC_SYSNAME)), )
  ifeq ($(findstring dll, $(TEC_UNAME)), )
    DEFINES += FTGL_LIBRARY_STATIC
  endif
else
#  LIBS += iconv
endif

ifneq ($(findstring AIX, $(TEC_UNAME)), )
  DEFINES += NO_FONTCONFIG
endif

ifneq ($(findstring IRIX, $(TEC_UNAME)), )
  DEFINES += NO_FONTCONFIG
endif

ifneq ($(findstring SunOS, $(TEC_UNAME)), )
  DEFINES += NO_FONTCONFIG
endif

ifneq ($(findstring MacOS, $(TEC_UNAME)), )
  ifneq ($(TEC_SYSMINOR), 4)
    BUILD_DYLIB=Yes
  endif
endif

ifneq ($(findstring cygw, $(TEC_UNAME)), )
  LIBS += iconv fontconfig
endif
