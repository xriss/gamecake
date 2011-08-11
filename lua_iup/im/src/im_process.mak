PROJNAME = im
LIBNAME = im_process
OPT = YES

SRC = \
    im_arithmetic_bin.cpp  im_morphology_gray.cpp  im_quantize.cpp   \
    im_arithmetic_un.cpp   im_geometric.cpp        im_render.cpp     \
    im_color.cpp           im_histogram.cpp        im_resize.cpp     \
    im_convolve.cpp        im_houghline.cpp        im_statistics.cpp \
    im_convolve_rank.cpp   im_logic.cpp            im_threshold.cpp  \
    im_effects.cpp         im_morphology_bin.cpp   im_tonegamut.cpp  \
    im_canny.cpp           im_distance.cpp         im_analyze.cpp    \
    im_kernel.cpp
SRC  := $(addprefix process/, $(SRC))
                                       
USE_IM = Yes
IM = ..

ifneq ($(findstring Win, $(TEC_SYSNAME)), )
    ifneq ($(findstring ow, $(TEC_UNAME)), )
      DEFINES += IM_DEFMATHFLOAT
    endif  
    ifneq ($(findstring bc, $(TEC_UNAME)), )
      DEFINES += IM_DEFMATHFLOAT
    endif  
else
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
endif
