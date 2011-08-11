PROJNAME = iup
LIBNAME = iupole
OPT = YES

INCLUDES =  ../include ../src
LDIR = ../lib/$(TEC_UNAME)  
LIBS = iup

SRC = iup_olecontrol.cpp \
			tLegacy.cpp \
			tAmbientProperties.cpp \
			tDispatch.cpp \
			tOleClientSite.cpp \
			tOleControlSite.cpp \
			tOleHandler.cpp \
			tOleInPlaceFrame.cpp \
			tOleInPlaceSite.cpp


ifneq ($(findstring cygw, $(TEC_UNAME)), )
  LIBS += uuid ole32 gdi32 oleaut32
endif
