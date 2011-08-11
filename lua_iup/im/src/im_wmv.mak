PROJNAME = im
LIBNAME = im_wmv
OPT = YES

SRC = im_format_wmv.cpp

ifneq ($(findstring vc9, $(TEC_UNAME)), )
  USE_WIN_SDK = Yes
endif
ifneq ($(findstring vc10, $(TEC_UNAME)), )
  USE_WIN_SDK = Yes
endif
ifneq ($(findstring dll9, $(TEC_UNAME)), )
  USE_WIN_SDK = Yes
endif
ifneq ($(findstring dll10, $(TEC_UNAME)), )
  USE_WIN_SDK = Yes
endif

ifndef USE_WIN_SDK
  #vc6-vc8 needs an external SDK
  ifneq ($(findstring _64, $(TEC_UNAME)), )
    WMFSDK = d:/lng/wmfsdk95
  else
  #  WMFSDK = d:/lng/wmfsdk11
    WMFSDK = d:/lng/wmfsdk9
  endif
  INCLUDES = $(WMFSDK)/include
  LDIR = $(WMFSDK)/lib
else
  #vc9-vc10, wmf sdk is inside Windows SDK
endif
  
DEFINES = _CRT_NON_CONFORMING_SWPRINTFS                                     

LIBS = wmvcore

USE_IM = Yes
IM = ..
