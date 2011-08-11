PROJNAME = im
LIBNAME = im
OPT = YES

INCLUDES = . ../include
                     
# WORDS_BIGENDIAN used by libTIFF
ifeq ($(TEC_SYSARCH), ppc)
  DEFINES = WORDS_BIGENDIAN
endif
ifeq ($(TEC_SYSARCH), mips)
  DEFINES = WORDS_BIGENDIAN
endif
ifeq ($(TEC_SYSARCH), sparc)
  DEFINES = WORDS_BIGENDIAN
endif

SRCTIFF = \
    tif_aux.c       tif_dirwrite.c   tif_jpeg.c      tif_print.c    \
    tif_close.c     tif_dumpmode.c   tif_luv.c       tif_read.c     \
    tif_codec.c     tif_error.c      tif_lzw.c       tif_strip.c    \
    tif_color.c     tif_extension.c  tif_next.c      tif_swab.c     \
    tif_compress.c  tif_fax3.c       tif_open.c      tif_thunder.c  \
    tif_dir.c       tif_fax3sm.c     tif_packbits.c  tif_tile.c     \
    tif_dirinfo.c   tif_flush.c      tif_pixarlog.c  tif_zip.c      \
    tif_dirread.c   tif_getimage.c   tif_predict.c   tif_version.c  \
    tif_write.c     tif_warning.c    tif_ojpeg.c
SRCTIFF  := $(addprefix libtiff/, $(SRCTIFF)) im_format_tiff.cpp
INCLUDES += libtiff 

SRCJPEG = \
    jcapimin.c  jcmarker.c  jdapimin.c  jdinput.c   jdtrans.c   \
    jcapistd.c  jcmaster.c  jdapistd.c  jdmainct.c  jerror.c    jmemmgr.c  \
    jccoefct.c  jcomapi.c   jdatadst.c  jdmarker.c  jfdctflt.c  jmemnobs.c \
    jccolor.c   jcparam.c   jdatasrc.c  jdmaster.c  jfdctfst.c  jquant1.c  \
    jcdctmgr.c  jdcoefct.c  jdmerge.c   jfdctint.c  jquant2.c  \
    jchuff.c    jcprepct.c  jdcolor.c   jidctflt.c  jutils.c    jdarith.c \
    jcinit.c    jcsample.c  jddctmgr.c  jdpostct.c  jidctfst.c  jaricom.c  \
    jcmainct.c  jctrans.c   jdhuff.c    jdsample.c  jidctint.c  jcarith.c
SRCJPEG  := $(addprefix libjpeg/, $(SRCJPEG)) im_format_jpeg.cpp
INCLUDES += libjpeg 

SRCPNG = \
    png.c       pngget.c    pngread.c   pngrutil.c     pngwtran.c  \
    pngerror.c  pngmem.c    pngrio.c    pngset.c    pngwio.c    pngwutil.c  \
    pngpread.c  pngrtran.c  pngtrans.c  pngwrite.c
SRCPNG  := $(addprefix libpng/, $(SRCPNG)) im_format_png.cpp
INCLUDES += libpng 
DEFINES += PNG_NO_STDIO PNG_TIME_RFC1123_SUPPORTED

SRCZLIB = \
    adler32.c   crc32.c    gzio.c     inffast.c  inftrees.c  uncompr.c \
    compress.c  deflate.c  infback.c  inflate.c  trees.c     zutil.c
SRCZLIB  := $(addprefix zlib/, $(SRCZLIB))
INCLUDES += zlib

SRCEXIF = \
    fuji/exif-mnote-data-fuji.c  fuji/mnote-fuji-entry.c  fuji/mnote-fuji-tag.c              \
    canon/exif-mnote-data-canon.c  canon/mnote-canon-entry.c  canon/mnote-canon-tag.c              \
    olympus/exif-mnote-data-olympus.c  olympus/mnote-olympus-entry.c  olympus/mnote-olympus-tag.c  \
    pentax/exif-mnote-data-pentax.c  pentax/mnote-pentax-entry.c  pentax/mnote-pentax-tag.c        \
    exif-byte-order.c  exif-entry.c  exif-utils.c    exif-format.c  exif-mnote-data.c              \
    exif-content.c  exif-ifd.c  exif-tag.c exif-data.c  exif-loader.c exif-log.c exif-mem.c
SRCEXIF  := $(addprefix libexif/, $(SRCEXIF))
INCLUDES += libexif

SRCLZF = \
    lzf_c.c lzf_d.c
SRCLZF  := $(addprefix liblzf/, $(SRCLZF))
INCLUDES += liblzf

SRC = \
    old_imcolor.c         old_imresize.c      tiff_binfile.c       im_converttype.cpp \
    im_attrib.cpp         im_format.cpp       im_format_tga.cpp    im_filebuffer.cpp \
    im_bin.cpp            im_format_all.cpp   im_format_raw.cpp \
    im_binfile.cpp        im_format_sgi.cpp   im_datatype.cpp      im_format_pcx.cpp \
    im_colorhsi.cpp       im_format_bmp.cpp   im_image.cpp         im_rgb2map.cpp    \
    im_colormode.cpp      im_format_gif.cpp   im_lib.cpp           im_format_pnm.cpp \
    im_colorutil.cpp      im_format_ico.cpp   im_palette.cpp    \
    im_convertbitmap.cpp  im_format_led.cpp   im_counter.cpp       im_str.cpp        \
    im_convertcolor.cpp   im_fileraw.cpp      im_format_krn.cpp \
    im_file.cpp           im_format_ras.cpp   old_im.cpp           im_compress.cpp   \
    $(SRCJPEG) $(SRCTIFF) $(SRCPNG) $(SRCZLIB) $(SRCLZF)

    
ifneq ($(findstring Win, $(TEC_SYSNAME)), )
    SRC += im_sysfile_win32.cpp im_dib.cpp im_dibxbitmap.cpp
    
    ifneq ($(findstring dll, $(TEC_UNAME)), )
      SRC += im.rc
    endif
    
    ifeq ($(findstring _64, $(TEC_UNAME)), )
      # optimize PNG lib for VC
      ifneq ($(findstring vc, $(TEC_UNAME)), )
        SRC += libpng/pngvcrd.c
        DEFINES += PNG_USE_PNGVCRD
      endif
      ifneq ($(findstring dll, $(TEC_UNAME)), )
        SRC += libpng/pngvcrd.c
        DEFINES += PNG_USE_PNGVCRD
      endif         
    endif         
    
    # force the definition of math functions using float
    # Watcom does not define them
    ifneq ($(findstring ow, $(TEC_UNAME)), )
      DEFINES += IM_DEFMATHFLOAT
    endif         
    
    ifneq ($(findstring bc, $(TEC_UNAME)), )
      DEFINES += IM_DEFMATHFLOAT
    else
      USE_EXIF = Yes
    endif
else
  SRC += im_sysfile_unix.cpp
endif

ifdef USE_EXIF
  SRC += $(SRCEXIF)    
  DEFINES += USE_EXIF
endif  

ifneq ($(findstring Linux, $(TEC_UNAME)), )
    # optimize PNG lib for Linux in x86
    ifeq "$(TEC_SYSARCH)" "x86"
      SRC += libpng/pnggccrd.c
      DEFINES += PNG_USE_PNGGCCRD
    endif
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
