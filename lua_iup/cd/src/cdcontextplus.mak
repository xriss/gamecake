PROJNAME = cd
LIBNAME = cdcontextplus
OPT = YES

DEFINES = CD_NO_OLD_INTERFACE


ifneq ($(findstring Win, $(TEC_SYSNAME)), )
  SRCDIR = gdiplus
  SRC = cdwemfp.cpp cdwimgp.cpp cdwinp.cpp cdwnativep.cpp cdwprnp.cpp cdwdbufp.cpp cdwclpp.cpp cdwgdiplus.c

  INCLUDES = . gdiplus drv
  LIBS = gdiplus
  CHECK_GDIPLUS = Yes
else
  SRC = xrender/cdxrender.c xrender/cdxrplus.c

  LIBS = Xrender Xft
  USE_X11 = Yes
  CHECK_XRENDER = Yes
  
  ifdef GTK_DEFAULT
    CD_SUFFIX := x11
  endif

  INCLUDES = . sim drv freetype2 x11
endif

ifneq ($(findstring MacOS, $(TEC_UNAME)), )
  ifneq ($(TEC_SYSMINOR), 4)
    BUILD_DYLIB=Yes
  endif
endif

USE_CD = YES
CD = ..
