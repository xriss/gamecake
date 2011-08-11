PROJNAME = im
LIBNAME = im_ecw
OPT = YES

SRC = im_format_ecw.cpp
                                       
ECWSDKINC = d:/lng/ecw_sdk/include
ECWSDKLIB = d:/lng/ecw_sdk/lib/$(TEC_UNAME)
                                       
INCLUDES = ../include $(ECWSDKINC)

LDIR = $(ECWSDKLIB)
LIBS = NCSEcw

IM = ..
USE_IM = Yes
