PROJNAME = cd
LIBNAME = pdflib
OPT = YES

DEF_FILE = cd_pdflib.def

ifeq ($(TEC_UNAME), ppc)
  DEFINES = WORDS_BIGENDIAN
endif
ifeq ($(TEC_UNAME), mips)
  DEFINES = WORDS_BIGENDIAN
endif
ifeq ($(TEC_UNAME), sparc)
  DEFINES = WORDS_BIGENDIAN
endif

# Changes to PDFlib-Lite-7.0.4p4 (search for CDPDF): 
#     pdflib/pdcore/pc_config.h - added default PDF_PLATFORM  
#     pdflib/pdflib/p_intern.h - removed support for other image formats, leave this to IM */
  
srcdir := pdflib/font
INCLUDES := $(INCLUDES) $(srcdir)
SRCFONT := \
	$(srcdir)/ft_cid.c	\
	$(srcdir)/ft_corefont.c	\
	$(srcdir)/ft_font.c	\
	$(srcdir)/ft_hostfont.c	\
	$(srcdir)/ft_pdffont.c	\
	$(srcdir)/ft_truetype.c \
	$(srcdir)/ft_type1.c
	
srcdir := pdflib/flate
INCLUDES := $(INCLUDES) $(srcdir)
SRCFLATE := \
	$(srcdir)/adler32.c 	\
	$(srcdir)/compress.c	\
	$(srcdir)/crc32.c	\
	$(srcdir)/deflate.c	\
	$(srcdir)/inffast.c	\
	$(srcdir)/inflate.c	\
	$(srcdir)/inftrees.c 	\
	$(srcdir)/trees.c	\
	$(srcdir)/uncompr.c	\
	$(srcdir)/zutil.c
	
srcdir := pdflib/pdcore
INCLUDES := $(INCLUDES) $(srcdir)
SRCPDCORE := \
	$(srcdir)/pc_aes.c     	\
	$(srcdir)/pc_aescbc.c 	\
	$(srcdir)/pc_arc4.c     \
	$(srcdir)/pc_chartabs.c	\
	$(srcdir)/pc_contain.c	\
	$(srcdir)/pc_core.c	\
	$(srcdir)/pc_crypt.c	\
	$(srcdir)/pc_ctype.c	\
	$(srcdir)/pc_digsig.c	\
	$(srcdir)/pc_ebcdic.c	\
	$(srcdir)/pc_encoding.c	\
	$(srcdir)/pc_file.c	\
	$(srcdir)/pc_geom.c	\
	$(srcdir)/pc_md5.c	\
	$(srcdir)/pc_optparse.c	\
	$(srcdir)/pc_output.c	\
	$(srcdir)/pc_resource.c	\
	$(srcdir)/pc_scan.c	\
	$(srcdir)/pc_scope.c	\
	$(srcdir)/pc_string.c	\
	$(srcdir)/pc_unicode.c	\
	$(srcdir)/pc_util.c     \
	$(srcdir)/pc_xmp.c
	
srcdir := pdflib/pdflib
INCLUDES := $(INCLUDES) $(srcdir)
SRCPDFLIB := \
	$(srcdir)/p_3d.c      \
	$(srcdir)/p_actions.c   \
	$(srcdir)/p_afm.c       \
	$(srcdir)/p_annots.c	\
	$(srcdir)/p_block.c     \
	$(srcdir)/p_bmp.c	\
	$(srcdir)/p_ccitt.c	\
	$(srcdir)/p_cid.c	\
	$(srcdir)/p_color.c	\
	$(srcdir)/p_document.c	\
	$(srcdir)/p_draw.c	\
	$(srcdir)/p_encoding.c	\
	$(srcdir)/p_fields.c    \
	$(srcdir)/p_filter.c    \
	$(srcdir)/p_font.c	\
	$(srcdir)/p_gif.c	\
	$(srcdir)/p_gstate.c	\
	$(srcdir)/p_hyper.c	\
	$(srcdir)/p_icc.c	\
	$(srcdir)/p_icclib.c	\
	$(srcdir)/p_image.c	\
	$(srcdir)/p_jpeg.c	\
	$(srcdir)/p_jpx.c	\
	$(srcdir)/p_kerning.c   \
	$(srcdir)/p_layer.c     \
	$(srcdir)/p_mbox.c     \
	$(srcdir)/p_object.c    \
	$(srcdir)/p_opi.c       \
	$(srcdir)/p_page.c	\
	$(srcdir)/p_params.c	\
	$(srcdir)/p_pattern.c	\
	$(srcdir)/p_pdi.c	\
	$(srcdir)/p_pfm.c	\
	$(srcdir)/p_photoshp.c	\
	$(srcdir)/p_png.c	\
	$(srcdir)/p_shading.c	\
	$(srcdir)/p_subsett.c	\
	$(srcdir)/p_table.c	\
	$(srcdir)/p_tagged.c	\
	$(srcdir)/p_template.c	\
	$(srcdir)/p_text.c	\
	$(srcdir)/p_textflow.c	\
	$(srcdir)/p_tiff.c      \
	$(srcdir)/p_truetype.c	\
	$(srcdir)/p_type1.c	\
	$(srcdir)/p_type3.c	\
	$(srcdir)/p_util.c      \
	$(srcdir)/p_xgstate.c	\
	$(srcdir)/p_xmp.c
	
SRC := pdflib/pdflib/pdflib.c $(SRCPDFLIB) $(SRCPDCORE) $(SRCFLATE) $(SRCFONT)

ifneq ($(findstring dll, $(TEC_UNAME)), )
  SRC += cd_pdflib.rc
endif

ifneq ($(findstring MacOS, $(TEC_UNAME)), )
  ifneq ($(TEC_SYSMINOR), 4)
    BUILD_DYLIB=Yes
  endif
endif
