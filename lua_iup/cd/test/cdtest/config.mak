APPNAME = cdtest

ifdef USE_GDK
  APPNAME = cdtestgdk
  USE_GTK = Yes
endif
ifdef USE_CONTEXTPLUS
  APPNAME = cdtestplus
  DEFINES = USE_CONTEXTPLUS
endif

SRC = cdtest.c cdtest_led.c colorbar.c drivers.c list.c rubber.c

ifeq "$(TEC_SYSNAME)" "Win32"
  SRC += cdtest.rc
  LEDC = $(IUP)/bin/$(TEC_SYSNAME)/ledc
else  
  LEDC = $(IUP)/bin/$(TEC_UNAME)/ledc
endif

USE_CD=Yes
USE_IUP3=Yes

cdtest_led.c: cdtest.led
	$(LEDC) -f cdtest_loadled -o cdtest_led.c cdtest.led

USE_STATIC = Yes

#IUP = ../../../iup
#CD = ../..

ifneq ($(findstring Win, $(TEC_SYSNAME)), )
  LIBS = cdpdflib pdflib
else
  SLIB = $(CD)/lib/$(TEC_UNAME)/libcdpdf.a $(CD)/lib/$(TEC_UNAME)/libpdflib.a
endif
