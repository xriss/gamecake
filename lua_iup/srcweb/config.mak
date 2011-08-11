PROJNAME = iup
LIBNAME = iupweb
OPT = YES

INCLUDES =  ../include ../src
LDIR = ../lib/$(TEC_UNAME)  
LIBS = iup
SRC = iup_webbrowser.c

ifneq ($(findstring Win, $(TEC_SYSNAME)), )
  SRC += iupwin_webbrowser.cpp
  LIBS += iupole comsuppw
  USE_ATL = Yes
  DEFINES = _MBCS
else
  ifdef GTK_DEFAULT
    SRC  += iupgtk_webbrowser.c
    LIBS += webkit-1.0
    USE_GTK = Yes
    INCLUDES += ../src/gtk $(GTK)/include/webkit-1.0 $(GTK)/include/libsoup-2.4
  else
#    SRC = iupmot_webbrowser.c
#    LIBS += XmHTML
  endif
endif
