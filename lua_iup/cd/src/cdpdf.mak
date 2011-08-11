PROJNAME = cd
LIBNAME = cdpdf
OPT = YES

DEFINES = CD_NO_OLD_INTERFACE
SRC = drv/cdpdf.c

INCLUDES = . sim pdflib/pdflib
LIBS = pdflib

USE_CD = YES
CD = ..

ifneq ($(findstring MacOS, $(TEC_UNAME)), )
  ifneq ($(TEC_SYSMINOR), 4)
    BUILD_DYLIB=Yes
  endif
endif
