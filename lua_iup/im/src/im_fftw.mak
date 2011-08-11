PROJNAME = im
LIBNAME = im_fftw
OPT = YES

DEF_FILE = im_fftw.def

SRC = config.c executor.c fftwnd.c fn_1.c fn_10.c fn_11.c fn_12.c fn_13.c \
  fn_14.c fn_15.c fn_16.c fn_2.c fn_3.c fn_32.c fn_4.c fn_5.c fn_6.c fn_64.c fn_7.c \
  fn_8.c fn_9.c fni_1.c fni_10.c fni_11.c fni_12.c fni_13.c fni_14.c fni_15.c fni_16.c \
  fni_2.c fni_3.c fni_32.c fni_4.c fni_5.c fni_6.c fni_64.c fni_7.c fni_8.c fni_9.c \
  ftw_10.c ftw_16.c ftw_2.c ftw_3.c ftw_32.c ftw_4.c ftw_5.c ftw_6.c ftw_64.c ftw_7.c \
  ftw_8.c ftw_9.c ftwi_10.c ftwi_16.c ftwi_2.c ftwi_3.c ftwi_32.c ftwi_4.c ftwi_5.c \
  ftwi_6.c ftwi_64.c ftwi_7.c ftwi_8.c ftwi_9.c generic.c malloc.c planner.c putils.c \
  rader.c timer.c twiddle.c wisdom.c wisdomio.c
SRC := $(addprefix fftw/, $(SRC))

SRC := process/im_fft.cpp $(SRC)

INCLUDES := fftw

DEFINES = FFTW_ENABLE_FLOAT

USE_IM = Yes
IM = ..
LIBS = im_process
    
ifneq ($(findstring ow, $(TEC_UNAME)), )
  DEFINES += IM_DEFMATHFLOAT
endif   

ifneq ($(findstring bc, $(TEC_UNAME)), )
  DEFINES += IM_DEFMATHFLOAT
endif         

ifneq ($(findstring AIX, $(TEC_UNAME)), )
  DEFINES += IM_DEFMATHFLOAT
endif

ifneq ($(findstring SunOS, $(TEC_UNAME)), )
  DEFINES += IM_DEFMATHFLOAT
endif
      
ifneq ($(findstring HP-UX, $(TEC_UNAME)), )
  DEFINES += IM_DEFMATHFLOAT
endif

ifneq ($(findstring MacOS, $(TEC_UNAME)), )
  ifneq ($(TEC_SYSMINOR), 4)
    BUILD_DYLIB=Yes
  endif
endif
