APPNAME := simple

ifdef GTK_DEFAULT
  ifdef USE_MOTIF
    # Build Motif version in Linux,Darwin,FreeBSD
    APPNAME := $(APPNAME)mot
  else
    GDK_CAIRO = Yes
  endif
else  
  ifdef USE_GTK
    # Build GTK version in IRIX,SunOS,AIX,Win32
    APPNAME := $(APPNAME)gtk
    GDK_CAIRO = Yes
  endif
endif

DEFINES = USE_CONTEXTPLUS

SRC = simple.c simple_led.c iupmain.c
            
#DBG = Yes
USE_CD=Yes
USE_IUP3=Yes

ifeq "$(TEC_SYSNAME)" "Win32"
  LEDC = $(IUP)/bin/$(TEC_SYSNAME)/ledc
else  
  LEDC = $(IUP)/bin/$(TEC_UNAME)/ledc
endif

simple_led.c: simple.led
	$(LEDC) -f simple_loadled -o simple_led.c simple.led

USE_STATIC = Yes

#IUP = ../../../iup
#CD = ../..

USE_OPENGL = Yes
ifdef USE_OPENGL
  DEFINES += USE_OPENGL
endif

ifneq ($(findstring Win, $(TEC_SYSNAME)), )
  LIBS = cdpdf pdflib
  ifndef GDK_CAIRO
    LIBS += cdcontextplus gdiplus
  endif
  ifdef USE_OPENGL
    LIBS += ftgl cdgl
  endif
else
  ifdef DBG_DIR
    CDLIB = $(CD)/lib/$(TEC_UNAME)d
  else
    CDLIB = $(CD)/lib/$(TEC_UNAME)
  endif  

  SLIB = $(CDLIB)/libcdpdf.a $(CDLIB)/libpdflib.a 
  ifndef GDK_CAIRO
    SLIB += $(CDLIB)/libcdcontextplus.a
    LIBS = Xrender Xft
  endif
  ifdef USE_OPENGL
    SLIB += $(CDLIB)/libcdgl.a $(CDLIB)/libftgl.a
    #LIBS = ftgl
  endif
endif
