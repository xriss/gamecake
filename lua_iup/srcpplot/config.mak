PROJNAME = iup
LIBNAME  = iup_pplot
OPT = YES

USE_CD = Yes

ifdef DBG
  DEFINES += IUP_ASSERT
endif  

INCLUDES = ../include ../src
LDIR = ../lib/$(TEC_UNAME)  
LIBS = iup iupcd

DEFINES = _IUP_PPLOT_ CD_NO_OLD_INTERFACE

SRC = iupPPlot.cpp  iupPPlotInteraction.cpp  iup_pplot.cpp

ifneq ($(findstring owc, $(TEC_UNAME)), )
  CPPFLAGS = -xr -xst
endif

ifeq "$(TEC_UNAME)" "vc6"
  INCLUDES += D:\LNG\STLport\include
endif

ifeq "$(TEC_UNAME)" "dll"
  INCLUDES += D:\LNG\STLport\include
endif

ifneq ($(findstring MacOS, $(TEC_UNAME)), )
  ifneq ($(TEC_SYSMINOR), 4)
    BUILD_DYLIB=Yes
  endif
endif
