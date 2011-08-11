PROJNAME = im
LIBNAME = im_fftw3
OPT = YES

DEF_FILE = im_fftw.def

SRC = process/im_fft.cpp

INCLUDES = ../include

DEFINES = USE_FFTW3

USE_IM = Yes
IM = ..
LIBS = im_process


ifneq ($(findstring Win, $(TEC_SYSNAME)), )
  ifneq ($(findstring _64, $(TEC_UNAME)), )
    FFTW = d:/lng/fftw64
  else
    FFTW = d:/lng/fftw32
  endif
  INCLUDES += $(FFTW)
  LIBS += libfftw3f-3
  LDIR = $(FFTW)
else  
  LIBS += fftw3f
endif


ifneq ($(findstring Win, $(TEC_SYSNAME)), )
  ifneq ($(findstring gcc, $(TEC_UNAME)), )
    DEFINES += HAVE_UINTPTR_T
  endif
  ifneq ($(findstring ow, $(TEC_UNAME)), )
    DEFINES += IM_DEFMATHFLOAT
  endif         
  ifneq ($(findstring bc, $(TEC_UNAME)), )
    DEFINES += IM_DEFMATHFLOAT
  endif
else
  ifneq ($(findstring IRIX, $(TEC_UNAME)), )
    DEFINES += HAVE_UINTPTR_T
  endif
  ifneq ($(findstring MacOS, $(TEC_UNAME)), )
    ifneq ($(TEC_SYSMINOR), 4)
      BUILD_DYLIB=Yes
    endif
    DEFINES += HAVE_UINTPTR_T
  endif
  ifneq ($(findstring FreeBSD, $(TEC_UNAME)), )
    DEFINES += HAVE_UINTPTR_T
  endif
  ifneq ($(findstring AIX, $(TEC_UNAME)), )
    DEFINES += IM_DEFMATHFLOAT HAVE_UINTPTR_T
  endif
  ifneq ($(findstring SunOS, $(TEC_UNAME)), )
    DEFINES += IM_DEFMATHFLOAT
  endif
endif

ifneq ($(findstring MacOS, $(TEC_UNAME)), )
  BUILD_DYLIB=Yes
endif

