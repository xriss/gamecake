PROJNAME = cd
LIBNAME = cd
OPT = YES   

ifdef GTK_DEFAULT
  ifdef USE_X11
    # Build X11 version in Linux and BSD
    LIBNAME := $(LIBNAME)x11
  else
    ifeq ($(findstring Win, $(TEC_SYSNAME)), )
      # Force definition if not in Windows
      USE_GDK = Yes
    endif
  endif
else  
  ifdef USE_GDK
    # Build GDK version in IRIX,SunOS,AIX,Win32
    LIBNAME := $(LIBNAME)gdk
  else
    ifeq ($(findstring Win, $(TEC_SYSNAME)), )
      # Force definition if not in Windows
      USE_X11 = Yes
    endif
  endif
endif

DEFINES = CD_NO_OLD_INTERFACE

SRCSVG = base64.c lodepng.c cdsvg.c
SRCSVG := $(addprefix svg/, $(SRCSVG))

SRCINTCGM = circle.c ellipse.c intcgm1.c \
  intcgm2.c intcgm4.c intcgm6.c list.c \
  sism.c tparse.c bparse.c 
SRCINTCGM  := $(addprefix intcgm/, $(SRCINTCGM))

SRCSIM := cdfontex.c sim.c cd_truetype.c sim_primitives.c sim_text.c sim_linepolyfill.c
SRCSIM  := $(addprefix sim/, $(SRCSIM))

SRCWIN32 = cdwclp.c cdwemf.c cdwimg.c cdwin.c cdwnative.c cdwprn.c cdwwmf.c wmf_emf.c cdwdbuf.c cdwdib.c
SRCWIN32  := $(addprefix win32/, $(SRCWIN32))

SRCX11 = cdx11.c cdxclp.c cdximg.c cdxnative.c cdxdbuf.c xvertex.c
SRCX11  := $(addprefix x11/, $(SRCX11))

SRCGDK = cdgdk.c cdgdkclp.c cdgdkdbuf.c cdgdkimg.c cdgdknative.c
SRCGDK  := $(addprefix gdk/, $(SRCGDK))

SRCCAIRO = cdcairodbuf.c cdcairopdf.c cdcairosvg.c cdcairo.c cdcairoimg.c cdcairoplus.c cdcairoirgb.c cdcairops.c cdcaironative_gdk.c
SRCCAIRO := $(addprefix cairo/, $(SRCCAIRO))

SRCDRV = cddgn.c cdcgm.c cgm.c cddxf.c cdirgb.c cdmf.c cdps.c cdpicture.c cddebug.c
SRCDRV  := $(addprefix drv/, $(SRCDRV))

SRCNULL = cd0prn.c cd0emf.c cd0wmf.c
SRCNULL  := $(addprefix drv/, $(SRCNULL))

SRCCOMM = cd.c wd.c wdhdcpy.c rgb2map.c cd_vectortext.c cd_active.c \
          cd_attributes.c cd_bitmap.c cd_image.c cd_primitives.c cd_text.c cd_util.c
      
SRC = $(SRCCOMM) $(SRCSVG) $(SRCINTCGM) $(SRCDRV) $(SRCSIM)
INCLUDES = . drv x11 win32 intcgm freetype2 sim cairo ../include

ifdef USE_GDK
  SRC += $(SRCGDK) $(SRCNULL) 
  USE_GTK = Yes
  CHECK_GTK = Yes
  # Temporarily to build inside Tecgraf
  ADD_CAIRO = Yes
  ifdef ADD_CAIRO
    LIBS = pangocairo-1.0 cairo
    SRC += $(SRCCAIRO)
  endif
  ifneq ($(findstring Win, $(TEC_SYSNAME)), )
    ifdef ADD_CAIRO
      SRC += cairo/cdcairoprn_win32.c cdcairoemf.c
    endif
    LIBS += freetype6
  else
    ifeq ($(findstring MacOS, $(TEC_UNAME)), )
      ifdef ADD_CAIRO
        SRC += cairo/cdcairoprn_unix.c
      endif
      INCLUDES += $(GTK)/include/gtk-unix-print-2.0
    endif
    LIBS += freetype
  endif
else
  ifdef USE_X11
    SRC += $(SRCX11) $(SRCNULL)
    LIBS = freetype
  else
    SRC += $(SRCWIN32)
    ifneq ($(findstring cygw, $(TEC_UNAME)), )
      LIBS = freetype-6
    else
      LIBS = freetype6
    endif
  endif
endif

ifneq ($(findstring dll, $(TEC_UNAME)), )
  SRC += cd.rc
endif

ifneq ($(findstring MacOS, $(TEC_UNAME)), )
  ifneq ($(TEC_SYSMINOR), 4)
    BUILD_DYLIB=Yes
  endif
endif

LDIR = ../lib/$(TEC_UNAME)
