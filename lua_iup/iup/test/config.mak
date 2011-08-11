PROJNAME = iup
APPNAME = iuptest
APPTYPE = CONSOLE

ifdef GTK_DEFAULT
  ifdef USE_MOTIF
    # Build Motif version in Linux and BSD
    APPNAME := $(APPNAME)mot
  endif
else  
  ifdef USE_GTK
    # Build GTK version in IRIX,SunOS,AIX,Win32
    APPNAME := $(APPNAME)gtk
  endif
endif

INCLUDES = ../include

USE_IUP3 = Yes
USE_STATIC = Yes
IUP = ..

ifdef DBG_DIR
  IUPLIB = $(IUP)/lib/$(TEC_UNAME)d
  CDLIB = $(CD)/lib/$(TEC_UNAME)d
  IMLIB = $(IM)/lib/$(TEC_UNAME)d
else
  IUPLIB = $(IUP)/lib/$(TEC_UNAME)
  CDLIB = $(CD)/lib/$(TEC_UNAME)
  IMLIB = $(IM)/lib/$(TEC_UNAME)
endif  

# Must uncomment all SRC lines
DEFINES = BIG_TEST
SRC += bigtest.c  

SRC += tray.c
SRC += dialog.c
SRC += predialogs.c
SRC += timer.c
SRC += label.c
SRC += canvas.c
SRC += frame.c
SRC += idle.c
SRC += button.c
SRC += toggle.c
SRC += vbox.c
SRC += hbox.c
SRC += progressbar.c
SRC += text.c
SRC += val.c
SRC += tabs.c
SRC += sample.c
SRC += menu.c
SRC += spin.c
SRC += text_spin.c
SRC += list.c
SRC += sysinfo.c
SRC += mdi.c
SRC += getparam.c
SRC += getcolor.c
SRC += class_conf.c
SRC += tree.c
SRC += zbox.c
SRC += scanf.c
SRC += sbox.c
SRC += clipboard.c
SRC += split.c


#ifneq ($(findstring Win, $(TEC_SYSNAME)), )
#  LIBS += iupimglib
#else
#  SLIB += $(IUPLIB)/libiupimglib.a
#endif

USE_CD = Yes
SRC += canvas_scrollbar.c
SRC += canvas_cddbuffer.c
SRC += canvas_cdsimple.c

USE_OPENGL = Yes
DEFINES += USE_OPENGL
SRC += glcanvas.c
SRC += glcanvas_cube.c

USE_IUPCONTROLS = Yes
SRC += colorbrowser.c
SRC += dial.c
SRC += colorbar.c
SRC += cells_numbering.c
SRC += cells_degrade.c
SRC += cells_checkboard.c
SRC += gauge.c
SRC += matrix.c
SRC += matrix_cbs.c
SRC += matrix_cbmode.c

LINKER = g++
DEFINES += PPLOT_TEST
SRC += pplot.c
ifneq ($(findstring Win, $(TEC_SYSNAME)), )
  LIBS += iup_pplot
#  LIBS += cdpdflib
#  LDIR += $(IUP)/lib/$(TEC_UNAME)
else
  SLIB += $(IUPLIB)/libiup_pplot.a
#  SLIB += $(CDLIB)/libcdpdflib.a
endif

ifneq ($(findstring Win, $(TEC_SYSNAME)), )
  SRC += iuptest.rc 
else
  ifneq ($(findstring cygw, $(TEC_UNAME)), )
    SRC += iuptest.rc
  endif
endif
 
#ifneq ($(findstring Win, $(TEC_SYSNAME)), )
#  USE_GDIPLUS=Yes
#else
#  USE_XRENDER=Yes
#endif
