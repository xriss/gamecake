PROJNAME = im
LIBNAME = im_jp2
OPT = YES

SRCJP2 =  \
    base/jas_cm.c      base/jas_icc.c      base/jas_init.c    base/jas_stream.c  base/jas_version.c \
    base/jas_debug.c   base/jas_iccdata.c  base/jas_malloc.c  base/jas_string.c  base/jas_tmr.c \
    base/jas_getopt.c  base/jas_image.c    base/jas_seq.c     base/jas_tvp.c            \
    jp2/jp2_cod.c  jp2/jp2_dec.c  jp2/jp2_enc.c                                         \
    jpc/jpc_bs.c   jpc/jpc_math.c   jpc/jpc_mqenc.c  jpc/jpc_t1enc.c  jpc/jpc_tagtree.c \
    jpc/jpc_cs.c   jpc/jpc_mct.c    jpc/jpc_qmfb.c   jpc/jpc_t2cod.c  jpc/jpc_tsfb.c    \
    jpc/jpc_dec.c  jpc/jpc_mqcod.c  jpc/jpc_t1cod.c  jpc/jpc_t2dec.c  jpc/jpc_util.c    \
    jpc/jpc_enc.c  jpc/jpc_mqdec.c  jpc/jpc_t1dec.c  jpc/jpc_t2enc.c
SRCJP2  := $(addprefix libjasper/, $(SRCJP2))

SRC = jas_binfile.c im_format_jp2.cpp $(SRCJP2)
                                       
INCLUDES = libjasper

DEFINES  = EXCLUDE_JPG_SUPPORT EXCLUDE_MIF_SUPPORT EXCLUDE_PNM_SUPPORT \
           EXCLUDE_BMP_SUPPORT EXCLUDE_PGX_SUPPORT EXCLUDE_RAS_SUPPORT \
           EXCLUDE_TIFF_SUPPORT JAS_GEO_OMIT_PRINTING_CODE

ifneq ($(findstring Win, $(TEC_SYSNAME)), )
  ifneq ($(findstring owc1, $(TEC_UNAME)), )
    DEFINES += JAS_TYPES
  endif         
  ifneq ($(findstring dll, $(TEC_UNAME)), )
    DEFINES += JAS_WIN_MSVC_BUILD JAS_TYPES
  endif         
  ifneq ($(findstring vc, $(TEC_UNAME)), )
    DEFINES += JAS_WIN_MSVC_BUILD JAS_TYPES
  endif         
  ifneq ($(findstring bc, $(TEC_UNAME)), )
    DEFINES += JAS_TYPES
  endif         
  ifneq ($(findstring gcc, $(TEC_UNAME)), )
    DEFINES += HAVE_UNISTD_H JAS_TYPES
  endif         
  ifneq ($(findstring mingw, $(TEC_UNAME)), )
    DEFINES += HAVE_UNISTD_H HAVE_STDINT_H JAS_TYPES
  endif         
else
  DEFINES += HAVE_UNISTD_H JAS_TYPES
endif

ifneq ($(findstring MacOS, $(TEC_UNAME)), )
  ifneq ($(TEC_SYSMINOR), 4)
    BUILD_DYLIB=Yes
  endif
endif

USE_IM=Yes
IM = ..
