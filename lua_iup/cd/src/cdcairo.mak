PROJNAME = cd
LIBNAME = cdcairo
OPT = YES

DEFINES = CD_NO_OLD_INTERFACE
CHECK_GTK = Yes

INCLUDES = . cairo drv
SRCDIR = cairo
SRC = cdcairodbuf.c cdcairopdf.c cdcairosvg.c cdcairo.c cdcairoimg.c cdcairoplus.c cdcairoirgb.c cdcairops.c

# Unused here, goes inside cdgdk
# cdcaironative_gdk.c  

ifneq ($(findstring Win, $(TEC_SYSNAME)), )
  SRC += cdcaironative_win32.c cdcairoprn_win32.c cdcairoemf.c
else
  SRC += cdcaironative_x11.c   
  DEFINES += CAIRO_X11  
  CHECK_GTK = Yes
  
  USE_X11 = Yes
  
  ifdef GTK_DEFAULT
    CD_SUFFIX := x11
  endif
endif

# Can NOT use USE_GTK because gtk will be included for linking
INCLUDES += $(GTK)/include/cairo $(GTK)/include/pango-1.0 $(GTK)/include/glib-2.0 $(GTK)/lib/glib-2.0/include $(GTK)/lib64/glib-2.0/include
LDIR = $(GTK)/lib
LIBS = pangocairo-1.0 cairo pango-1.0 gobject-2.0 glib-2.0

# Can NOT use USE_CAIRO because cdcairo.lib will be included for linking
USE_CD = YES
CD = ..

ifneq ($(findstring MacOS, $(TEC_UNAME)), )
  ifneq ($(TEC_SYSMINOR), 4)
    BUILD_DYLIB=Yes
  endif
endif
