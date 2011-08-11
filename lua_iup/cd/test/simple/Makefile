#-------------------------------------------------------------------------#
#- Tecmake  (POSIX Version)                                              -#
#- Generic Makefile to build applications and libraries at TeCGraf       -#
#- The user makefile usually has the name "config.mak".                  -#
#-------------------------------------------------------------------------#

#---------------------------------#
# Tecmake Version
VERSION = 4.1


#---------------------------------#
# First target 
.PHONY: build
build: tecmake


#---------------------------------#
# System Variables Definitions

ifndef TEC_UNAME
  # Base Defintions
  TEC_SYSNAME:=$(shell uname -s)
  TEC_SYSVERSION:=$(shell uname -r|cut -f1 -d.)
  TEC_SYSMINOR:=$(shell uname -r|cut -f2 -d.)
  TEC_SYSARCH:=$(shell uname -m)

  # Fixes
  ifeq ($(TEC_SYSNAME), SunOS)
    TEC_SYSARCH:=$(shell uname -p)
  endif
  ifeq ($(TEC_SYSNAME), IRIX)
    TEC_SYSARCH:=$(shell uname -p)
  endif
  ifeq ($(TEC_SYSNAME), FreeBSD)
    TEC_SYSMINOR:=$(shell uname -r|cut -f2 -d.|cut -f1 -d-)
  endif
  ifeq ($(TEC_SYSNAME), AIX)
    TEC_SYSVERSION:=$(shell uname -v)
    TEC_SYSMINOR:=$(shell uname -r)
    TEC_SYSARCH:=ppc
  endif
  ifeq ($(TEC_SYSNAME), Darwin)
    TEC_SYSARCH:=$(shell uname -p)
  endif

  ifeq ($(TEC_SYSARCH), i686)
    TEC_SYSARCH:=x86
  endif
  ifeq ($(TEC_SYSARCH), i386)
    TEC_SYSARCH:=x86
  endif
  ifeq ($(TEC_SYSARCH), powerpc)
    TEC_SYSARCH:=ppc
  endif
  ifeq ($(TEC_SYSARCH), x86_64)
    TEC_SYSARCH:=x64
  endif

  # Compose
  TEC_SYSRELEASE:=$(TEC_SYSVERSION).$(TEC_SYSMINOR)
  TEC_UNAME:=$(TEC_SYSNAME)$(TEC_SYSVERSION)$(TEC_SYSMINOR)

  # Linux 2.4 and GCC 3.x
  ifeq ($(TEC_UNAME), Linux24)
    GCCVER:=$(shell gcc -dumpversion|cut -f1 -d.)
    ifeq ($(GCCVER), 3)
      TEC_UNAME:=$(TEC_UNAME)g3
    endif
  endif

  # Linux 2.6 and GCC 4.x
  ifeq ($(TEC_UNAME), Linux26)
    GCCVER:=$(shell gcc -dumpversion|cut -f1 -d.)
    ifeq ($(GCCVER), 4)
      TEC_UNAME:=$(TEC_UNAME)g4
    endif
  endif

  # Linux and PowerPC
  ifeq ($(TEC_SYSNAME), Linux)
    ifeq ($(TEC_SYSARCH), ppc)
      TEC_UNAME:=$(TEC_UNAME)ppc
    endif
  endif

  # 64-bits Linux
  ifeq ($(TEC_SYSARCH), x64)
    BUILD_64=Yes
    TEC_UNAME:=$(TEC_UNAME)_64
  endif

  ifeq ($(TEC_SYSARCH), ia64)
    BUILD_64=Yes
    TEC_UNAME:=$(TEC_UNAME)_ia64
  endif

  # Solaris and Intel
  ifeq ($(TEC_SYSNAME), SunOS)
    ifeq ($(TEC_SYSARCH) , x86)
      TEC_UNAME:=$(TEC_UNAME)x86
    endif
  endif

  # Darwin and Intel
  ifeq ($(TEC_SYSNAME), Darwin)
    ifeq ($(TEC_SYSVERSION), 10)
      TEC_SYSARCH:=x64
    else
      ifeq ($(TEC_SYSARCH), x86)
        TEC_UNAME:=$(TEC_UNAME)x86
      endif
    endif
  endif
endif


#---------------------------------#
# System Info
.PHONY: sysinfo
sysinfo:
	@echo ''; echo 'Tecmake - System Info'
	@echo 'TEC_SYSNAME = $(TEC_SYSNAME)'
	@echo 'TEC_SYSVERSION = $(TEC_SYSVERSION)'
	@echo 'TEC_SYSMINOR = $(TEC_SYSMINOR)'
	@echo 'TEC_SYSARCH = $(TEC_SYSARCH)'
	@echo 'TEC_UNAME = $(TEC_UNAME)'; echo ''

  
#---------------------------------#
# Known Platforms

UNAMES = Linux24 Linux24g3 Linux24g3_64 Linux26 Linux26_64 Linux26g4 Linux26g4_64 Linux26_ia64 FreeBSD54 SunOS57 SunOS58 SunOS510 SunOS510_x86 AIX43 IRIX65 IRIX6465


#---------------------------------#
# Directories Definitions

PROJDIR = ..
SRCDIR  = .
OBJROOT = $(PROJDIR)/obj


#---------------------------------#
# Byte Order and Word Size

ifneq ($(findstring x86, $(TEC_SYSARCH)), )
   TEC_BYTEORDER = TEC_LITTLEENDIAN
else
ifeq ($(TEC_SYSARCH), arm)
   TEC_BYTEORDER = TEC_LITTLEENDIAN
else
   TEC_BYTEORDER = TEC_BIGENDIAN
endif
endif

ifeq ($(TEC_SYSARCH), x64)
  TEC_BYTEORDER = TEC_LITTLEENDIAN
  TEC_WORDSIZE = TEC_64
else
ifdef BUILD_64
  TEC_WORDSIZE = TEC_64
else
  TEC_WORDSIZE = TEC_32
endif
endif

# Itanium Exception
ifeq ($(TEC_SYSARCH), ia64)
  TEC_BYTEORDER = TEC_LITTLEENDIAN
  TEC_WORDSIZE = TEC_64
endif


#---------------------------------#
# Compilation Flags
STDFLAGS := -Wall
STDDEFS  := -DTEC_UNAME=$(TEC_UNAME) -DTEC_SYSNAME=$(TEC_SYSNAME) -D$(TEC_SYSNAME)=$(TEC_SYSRELEASE) -D$(TEC_BYTEORDER) -D$(TEC_WORDSIZE) -DFUNCPROTO=15
STDINCS  := 
OPTFLAGS := -O2
STDLFLAGS  := r
DEBUGFLAGS := -g  
STDLDFLAGS := -shared
DLIBEXT := so
DLIBPRE := lib
APPEXT :=


ifneq ($(findstring cygw, $(TEC_UNAME)), )
  GTK_DEFAULT = Yes
endif  
ifneq ($(findstring Linux, $(TEC_UNAME)), )
  GTK_DEFAULT = Yes
endif  
ifneq ($(findstring Darwin, $(TEC_UNAME)), )
  GTK_DEFAULT = Yes
endif  
ifneq ($(findstring FreeBSD, $(TEC_UNAME)), )
  GTK_DEFAULT = Yes
endif  
ifneq ($(findstring Linux24, $(TEC_UNAME)), )
  GTK_DEFAULT :=
endif  


#---------------------------------#
# Tools

CC       := $(TEC_TOOLCHAIN)gcc
CPPC     := $(TEC_TOOLCHAIN)g++
FF       := $(TEC_TOOLCHAIN)g77
RANLIB   := $(TEC_TOOLCHAIN)ranlib
AR       := $(TEC_TOOLCHAIN)ar
DEBUGGER := $(TEC_TOOLCHAIN)gdb
RCC      := $(TEC_TOOLCHAIN)windres 

# Remote build script
REMOTE  = $(TECMAKE_HOME)/remote

# Packed LOHs script
LUAPRE = $(TECMAKE_HOME)/luapre.lua


#---------------------------------#
# User Configuration File

MAKENAME = config.mak

ifdef MF
  MAKENAME = $(MF).mak
endif

###################
include $(MAKENAME)
###################


#---------------------------------#
# Definitions of public variables 

ifdef LIBNAME
  TARGETNAME = $(LIBNAME)
  MAKETYPE = LIB
else
  TARGETNAME = $(APPNAME)
  MAKETYPE = APP
endif

ifndef TARGETNAME
  $(error LIBNAME nor APPNAME defined in $(MAKENAME)) 
endif

PROJNAME ?= $(TARGETNAME)

DEPEND := $(TARGETNAME).dep

ifdef DEPENDDIR
  DEPEND := $(DEPENDDIR)/$(TARGETNAME).dep.$(TEC_UNAME)
endif

ifeq ($(MAKETYPE), APP)
  TARGETROOT ?= $(PROJDIR)/bin
else
  TARGETROOT ?= $(PROJDIR)/lib
endif

ifneq ($(PROJNAME), $(TARGETNAME))
  OBJROOT := $(OBJROOT)/$(TARGETNAME)
endif

ifdef DBG
  STDFLAGS += $(DEBUGFLAGS)
  STDDEFS += -DDEBUG
else
  STDDEFS += -DNDEBUG
  ifdef OPT
    STDFLAGS += $(OPTFLAGS)
    STRIP ?= Yes
  endif
endif

ifdef BUILD_64
  ifneq ($(findstring SunOS, $(TEC_UNAME)), )
    USE_CC = Yes
    BUILD_64_DIR = Yes   
  endif
  ifneq ($(findstring AIX, $(TEC_UNAME)), )
    USE_CC = Yes
    BUILD_64_DIR = Yes
  endif
  ifneq ($(findstring IRIX, $(TEC_UNAME)), )
    USE_CC = Yes
    BUILD_64_DIR = Yes
  endif
endif
  
ifdef USE_CC
  CC := cc
  CPPC := CC
  STDFLAGS = 
  UNAMES := $(UNAMES_CC)
  ifdef USE_CC_DIR
    TEC_UNAME := $(TEC_UNAME)cc
  endif
endif

ifdef BUILD_64
  ifdef BUILD_64_DIR
    TEC_UNAME := $(TEC_UNAME)_64
  endif
endif

TEC_UNAME_DIR ?= $(TEC_UNAME)
TEC_UNAME_LIB_DIR ?= $(TEC_UNAME)
ifdef DBG
  ifdef DBG_LIB_DIR
    TEC_UNAME_LIB_DIR := $(TEC_UNAME_DIR)d
  endif
  ifdef DBG_DIR
    TEC_UNAME_DIR := $(TEC_UNAME_DIR)d
  endif
endif

OBJDIR := $(OBJROOT)/$(TEC_UNAME_DIR)
TARGETDIR := $(TARGETROOT)/$(TEC_UNAME_DIR)

# Change linker if any C++ source
ifndef LINKER
  ifneq "$(findstring .cpp, $(SRC))" ""
    LINKER := $(CPPC)
    LD := $(CPPC)
  else
    LINKER := $(CC)
    LD := $(CC)
  endif
endif


#---------------------------------#
# LO and LOH Suffix

SRCLUADIR ?= $(SRCDIR)
LOHDIR ?= $(SRCLUADIR)

ifdef USE_LOH_SUBDIR
  ifeq ($(TEC_BYTEORDER), TEC_BIGENDIAN)
    ifeq ($(TEC_WORDSIZE), TEC_64)
      LOH_SUBDIR ?= be64
    else
      LOH_SUBDIR ?= be32
    endif
  else
    ifeq ($(TEC_WORDSIZE), TEC_64)
      LOH_SUBDIR ?= le64
    else
      LOH_SUBDIR ?= le32
    endif
  endif
  LOHDIR := $(LOHDIR)/$(LOH_SUBDIR)
  INCLUDES += $(LOHDIR)
else
  ifeq ($(TEC_BYTEORDER), TEC_BIGENDIAN)
    ifeq ($(TEC_WORDSIZE), TEC_64)
      LO_SUFFIX ?= _be64
    else
      LO_SUFFIX ?= _be32
    endif
  else
    ifeq ($(TEC_WORDSIZE), TEC_64)
      LO_SUFFIX ?= _le64
    else
      LO_SUFFIX ?=
    endif
  endif
endif



#---------------------------------#
#  Platform specific variables

# Definitions for X11
X11_LIBS := Xmu Xt Xext X11
#X11_LIB := 
#X11_INC :=                     #include <X11/X.h>

# Definitions for OpenGL
OPENGL_LIBS := GLU GL
#OPENGL_LIB := 
#OPENGL_INC :=                  #include <GL/gl.h>  and possibly  
MOTIFGL_LIB := GLw              #include <GL/GLwMDrawA.h>

# Definitions for Motif
#MOTIF_LIB := 
#MOTIF_INC :=                   #include <Xm/Xm.h>

# Definitions for GLUT
#GLUT_LIB := 
#GLUT_INC := 

# Definitions for GTK
ifdef GTK_BASE
  GTK := $(GTK_BASE)
else
  ifneq ($(findstring Darwin, $(TEC_UNAME)), )
  # Option 1 - Fink GTK port
    GTK = /sw
  # Option 3 - GTK-OSX Framework
  #   GTK := /Users/cpts/gtk/inst
  else
    GTK = /usr
  endif
endif

ifneq ($(findstring Linux, $(TEC_UNAME)), )
  ifdef BUILD_64
    ifeq ($(TEC_SYSARCH), ia64)
      STDFLAGS += -fPIC
      X11_LIB := /usr/X11R6/lib
    else
      STDFLAGS += -m64 -fPIC
      X11_LIB := /usr/X11R6/lib64
    endif
  else
    X11_LIB := /usr/X11R6/lib
  endif
  X11_INC := /usr/X11R6/include
  MOTIFGL_LIB := 
endif

ifneq ($(findstring IRIX, $(TEC_UNAME)), )
  LD = ld
  STDLDFLAGS := -elf -shared -rdata_shared -soname lib$(TARGETNAME).so
  RANLIB := /bin/true
  X11_LIBS := Xmu Xt X11
  ifdef BUILD_64    
    ifdef USE_CC  
      STDFLAGS += -64 -KPIC
      STDLDFLAGS += -64
      LINKER += -64
    endif
    X11_LIB := /usr/lib64
    MOTIF_LIB := /usr/Motif-2.1/lib64
  else
    X11_LIB := /usr/lib32
    MOTIF_LIB := /usr/Motif-2.1/lib32
  endif
  MOTIF_INC = /usr/Motif-2.1/include
endif

ifneq ($(findstring AIX, $(TEC_UNAME)), ) 
  NO_DYNAMIC ?= Yes
  ifdef BUILD_64
    ifdef USE_CC  
      STDFLAGS += -q64 # to compilers C and C++
      STDLFLAGS := -X64 $(STDLFLAGS) # to librarian
      STDLDFLAGS += -64
      LINKER += -q64 # to linker
    endif
  endif
endif

ifneq ($(findstring HP-UX, $(TEC_UNAME)), )
  NO_DYNAMIC ?= Yes
  MOTIF_INC := /usr/include/Motif2.1
  X11_LIBS := Xt Xext X11
  OPENGL_LIB := /opt/graphics/OpenGL/lib
  OPENGL_INC := /opt/graphics/OpenGL/include
  STDDEFS := -DTEC_UNAME=$(TEC_UNAME) -DTEC_SYSNAME=$(TEC_SYSNAME) -D$(TEC_BYTEORDER) -D$(TEC_WORDSIZE) -DFUNCPROTO=15
  CC := aCC
  CPPC := aCC
  LINKER := aCC
endif

ifneq ($(findstring SunOS, $(TEC_UNAME)), )
  LD = ld
  STDLDFLAGS := -G
  X11_INC := /usr/openwin/share/include
  X11_LIB := /usr/openwin/lib
  MOTIF_INC := /usr/dt/share/include
  MOTIF_LIB := /usr/dt/lib
  OPENGL_INC := /usr/openwin/share/include/X11
  GLUT_LIB := /usr/local/glut-3.7/lib/glut
  GLUT_INC := /usr/local/glut-3.7/include
  ifdef BUILD_64
    ifdef USE_CC  
      STDFLAGS += -xarch=v9 -KPIC
      # have to force these PATHs because of a conflict with standard PATHs
      STDLDFLAGS += -64 -L/usr/lib/64 -L/usr/ucblib/sparcv9  
      LINKER += -xarch=v9
    endif
  endif
endif

ifneq ($(findstring Darwin, $(TEC_UNAME)), )
  X11_LIBS := Xmu Xp Xt Xext X11
  X11_LIB := /usr/X11R6/lib
  X11_INC := /usr/X11R6/include
  MOTIF_INC := /usr/OpenMotif/include
  MOTIF_LIB := /usr/OpenMotif/lib
  ifdef BUILD_DYLIB
    STDLDFLAGS := -dynamiclib -Wl -fno-common -headerpad_max_install_names -undefined dynamic_lookup -install_name lib$(TARGETNAME).dylib
    DLIBEXT := dylib
    STDFLAGS += -fno-common
  else
    STDLDFLAGS := -bundle -undefined dynamic_lookup
  endif
endif

ifneq ($(findstring FreeBSD, $(TEC_UNAME)), )
  X11_LIB := /usr/X11R6/lib
  X11_INC := /usr/X11R6/include
endif

#---------------------------------#
# Allows an extra configuration file.
ifdef EXTRA_CONFIG
include $(EXTRA_CONFIG)
endif


#---------------------------------#
# Tecgraf Libraries Location
TECTOOLS_HOME ?= ../..

IUP   ?= $(TECTOOLS_HOME)/iup
CD    ?= $(TECTOOLS_HOME)/cd
IM    ?= $(TECTOOLS_HOME)/im
LUA   ?= $(TECTOOLS_HOME)/lua
LUA51 ?= $(TECTOOLS_HOME)/lua5.1
LUA52 ?= $(TECTOOLS_HOME)/lua52


#---------------------------------#
#  Pre-defined libraries

# Library order:
#   user + iupcd + cd + iup + motif + X
# Library path order is reversed

ifdef USE_LUA
  LUASUFX :=
  LIBLUASUFX := 3
endif

ifdef USE_LUA4
  LUASUFX := 4
  LIBLUASUFX := 4
  override USE_LUA = Yes
  LUA := $(LUA4)
endif

ifdef USE_LUA5
  LUASUFX := 5
  LIBLUASUFX := 5
  override USE_LUA = Yes
  LUA := $(LUA5)
endif

ifdef USE_LUA50
  LUASUFX := 50
  LIBLUASUFX := 5
  override USE_LUA = Yes
  LUA := $(LUA50)
  NO_LUALIB := Yes
endif

ifdef USE_LUA51
  LUASUFX := 5.1
  LIBLUASUFX := 51
  override USE_LUA = Yes
  LUA := $(LUA51)
  NO_LUALIB := Yes
endif

ifdef USE_LUA52
  LUASUFX := 52
  LIBLUASUFX := 52
  override USE_LUA = Yes
  LUA := $(LUA52)
  NO_LUALIB := Yes
endif

ifdef USE_IUP3
  override USE_IUP = Yes
# Inside Tecgraf only  
  ifndef IUP3_BUILD
#  IUP := $(IUP)3
  endif
endif 

ifdef USE_IUPBETA
  IUP := $(IUP)/beta
endif 

ifdef USE_CDBETA
  CD := $(CD)/beta
endif 

ifdef USE_IMBETA
  IM := $(IM)/beta
endif 

ifdef USE_GLUT
  override USE_OPENGL = Yes
endif

ifdef USE_GDK
  override USE_GTK = Yes
endif

ifdef USE_IUPCONTROLS
  override USE_CD = Yes
  override USE_IUP = Yes
  ifdef USE_IUPLUA
    ifdef USE_STATIC
      SLIB += $(IUP)/lib/$(TEC_UNAME_LIB_DIR)/libiupluacontrols$(LIBLUASUFX).a
    else
      LIBS += iupluacontrols$(LIBLUASUFX)
    endif
    override USE_CDLUA = Yes
  endif
  ifdef USE_STATIC
    SLIB += $(IUP)/lib/$(TEC_UNAME_LIB_DIR)/libiupcontrols.a
  else
    LIBS += iupcontrols
  endif
endif
  
ifdef USE_IMLUA
  override USE_IM = Yes
  ifdef USE_STATIC
    SLIB += $(IM)/lib/$(TEC_UNAME_LIB_DIR)/libimlua$(LIBLUASUFX).a
  else
    LIBS += imlua$(LIBLUASUFX)
  endif
endif

ifdef USE_CDLUA
  override USE_CD = Yes
  ifdef USE_STATIC
    SLIB += $(CD)/lib/$(TEC_UNAME_LIB_DIR)/libcdlua$(LIBLUASUFX).a
  else
    LIBS += cdlua$(LIBLUASUFX)
  endif
endif

ifdef USE_IUPLUA
  override USE_IUP = Yes
  ifdef USE_STATIC
    ifdef USE_CD
      SLIB += $(IUP)/lib/$(TEC_UNAME_LIB_DIR)/libiupluacd$(LIBLUASUFX).a
    endif
    ifdef USE_OPENGL
      SLIB += $(IUP)/lib/$(TEC_UNAME_LIB_DIR)/libiupluagl$(LIBLUASUFX).a
    endif
    SLIB += $(IUP)/lib/$(TEC_UNAME_LIB_DIR)/libiuplua$(LIBLUASUFX).a
  else
    ifdef USE_CD
      LIBS += iupluacd$(LIBLUASUFX)
    endif
    ifdef USE_OPENGL
      LIBS += iupluagl$(LIBLUASUFX)
    endif
    LIBS += iuplua$(LIBLUASUFX)
  endif
endif

ifdef USE_LUA
  LUA_LIB ?= $(LUA)/lib/$(TEC_UNAME_LIB_DIR)
  ifdef USE_STATIC
    ifndef NO_LUALIB
      SLIB += $(LUA_LIB)/liblualib$(LUASUFX).a
    endif
    SLIB += $(LUA_LIB)/liblua$(LUASUFX).a
  else
    ifndef NO_LUALIB
      LIBS += lualib$(LUASUFX)
    endif
    ifndef NO_LUALINK
        LIBS += lua$(LUASUFX)
        LDIR += $(LUA_LIB)
    else
      ifneq ($(findstring cygw, $(TEC_UNAME)), )
        LIBS += lua$(LUASUFX)
        LDIR += $(LUA_LIB)
      endif
    endif
  endif
  
  LUA_INC   ?= $(LUA)/include
  INCLUDES += $(LUA_INC)
  
  LUA_BIN ?= $(LUA)/bin/$(TEC_UNAME)
  BIN2C     := $(LUA_BIN)/bin2c$(LUASUFX)
  LUAC      := $(LUA_BIN)/luac$(LUASUFX)
  LUABIN    := $(LUA_BIN)/lua$(LUASUFX)
endif

ifdef USE_IUP   
  IUPSUFX := 
  ifdef USE_IUP3
    ifdef GTK_DEFAULT
      ifdef USE_MOTIF
        IUPSUFX := mot
      else
        override USE_GTK = Yes
        override USE_GDK = Yes
      endif
    else
      ifdef USE_GTK
        IUPSUFX := gtk
      else
        override USE_MOTIF = Yes
      endif
    endif
  else
    override USE_MOTIF = Yes
  endif
  ifdef USE_STATIC
    ifdef USE_CD
      SLIB += $(IUP)/lib/$(TEC_UNAME_LIB_DIR)/libiupcd.a
    endif
    ifdef USE_OPENGL
      SLIB += $(IUP)/lib/$(TEC_UNAME_LIB_DIR)/libiupgl.a
    endif
    SLIB += $(IUP)/lib/$(TEC_UNAME_LIB_DIR)/libiup$(IUPSUFX).a
  else
    ifdef USE_CD
      LIBS += iupcd
    endif
    ifdef USE_OPENGL
      LIBS += iupgl
    endif
    LIBS += iup$(IUPSUFX)
    LDIR += $(IUP)/lib/$(TEC_UNAME_LIB_DIR)
  endif
  INCLUDES += $(IUP)/include
endif

ifdef USE_CD
  CDSUFX := 
  override USE_X11 = Yes
  ifdef USE_GDK
    ifndef GTK_DEFAULT
      CDSUFX := gdk
    endif
  else
    ifdef GTK_DEFAULT
      CDSUFX := x11
    endif
  endif
  ifdef USE_STATIC
    ifdef USE_XRENDER
      SLIB += $(CD)/lib/$(TEC_UNAME_LIB_DIR)/libcdcontextplus.a
      LIBS += Xrender Xft
    endif
    ifdef USE_CAIRO
      # To use Cairo with X11 base driver (NOT for GDK)
      # Can NOT be used together with XRender
      SLIB += $(CD)/lib/$(TEC_UNAME_LIB_DIR)/libcdcairo.a
      LIBS += pangocairo-1.0 cairo
    endif
    SLIB += $(CD)/lib/$(TEC_UNAME_LIB_DIR)/libcd$(CDSUFX).a
    ifndef USE_GTK
      # Freetype is already included in GTK
      SLIB += $(CD)/lib/$(TEC_UNAME_LIB_DIR)/libfreetype.a
    endif
  else
    ifdef USE_XRENDER
      LIBS += cdcontextplus
      LIBS += Xrender Xft
    endif
    ifdef USE_CAIRO
      # To use Cairo with X11 base driver (NOT for GDK)
      # Can NOT be used together with XRender
      LIBS += cdcairo
      LIBS += pangocairo-1.0 cairo
    endif
    LIBS += cd$(CDSUFX)
    LDIR += $(CD)/lib/$(TEC_UNAME_LIB_DIR)
    ifndef USE_GTK
      # Freetype is already included in GTK
      LIBS += freetype
    endif
  endif
  INCLUDES += $(CD)/include
endif

ifdef USE_IM
  ifdef USE_STATIC
    SLIB += $(IM)/lib/$(TEC_UNAME_LIB_DIR)/libim.a
  else
    LIBS += im
    LDIR += $(IM)/lib/$(TEC_UNAME_LIB_DIR)
  endif
  INCLUDES += $(IM)/include
endif

ifdef USE_GLUT
  LIBS += glut
  LDIR += $(GLUT_LIB)
  STDINCS += $(GLUT_INC)
endif 

ifdef USE_OPENGL
  override USE_X11 = Yes
  ifdef USE_MOTIF
    ifndef USE_IUP3
      LIBS += $(MOTIFGL_LIB)
    endif
  endif
  LIBS += $(OPENGL_LIBS)
  LDIR += $(OPENGL_LIB)
  STDINCS += $(OPENGL_INC)
endif 

ifdef USE_MOTIF
  override USE_X11 = Yes
  LIBS += Xm
  LDIR += $(MOTIF_LIB)
  STDINCS += $(MOTIF_INC)
  ifneq ($(findstring Linux, $(TEC_UNAME)), )
    X11_LIBS := Xpm $(X11_LIBS)
  endif
  ifneq ($(findstring cygw, $(TEC_UNAME)), ) 
    X11_LIBS := Xpm $(X11_LIBS)
  endif
endif

ifdef USE_GTK
  ifneq ($(findstring Darwin, $(TEC_UNAME)), )
# Option 1 - Fink GTK port
    LDIR += $(GTK)/lib
    override USE_X11 = Yes
    LIBS += gtk-x11-2.0 gdk-x11-2.0 pangox-1.0
# Option 2 - Imendio Framework
#   STDINCS += /Library/Frameworks/Gtk.framework/Headers
#   STDINCS += /Library/Frameworks/GLib.framework/Headers
#   STDINCS += /Library/Frameworks/Cairo.framework/Headers
#   LFLAGS += -framework Gtk
# Option 3 - GTK-OSX Framework
#   LDIR += $(GTK)/lib
#   LFLAGS += -framework Carbon
#   LIBS += gtk-quartz-2.0 gdk-quartz-2.0 pangoft2-1.0

    LIBS += freetype
  else
    # if not the default, then include it for linker
    # must be before the default
    ifdef GTK_BASE
      LDIR += $(GTK)/lib
    endif
    override USE_X11 = Yes
    LIBS += gtk-x11-2.0 gdk-x11-2.0 pangox-1.0
  endif
  
  LIBS += gdk_pixbuf-2.0 pango-1.0 gobject-2.0 gmodule-2.0 glib-2.0
  STDINCS += $(GTK)/include/atk-1.0 $(GTK)/include/gtk-2.0 $(GTK)/include/cairo $(GTK)/include/pango-1.0 $(GTK)/include/glib-2.0
  
  ifeq ($(TEC_SYSARCH), x64)
    STDINCS += $(GTK)/lib64/glib-2.0/include $(GTK)/lib64/gtk-2.0/include
    # Add also these to avoid errors in systems that lib64 does not exists
    STDINCS += $(GTK)/lib/glib-2.0/include $(GTK)/lib/gtk-2.0/include
  else
  ifeq ($(TEC_SYSARCH), ia64)
    STDINCS += $(GTK)/lib64/glib-2.0/include $(GTK)/lib64/gtk-2.0/include
  else
    STDINCS += $(GTK)/lib/glib-2.0/include $(GTK)/lib/gtk-2.0/include
  endif
  endif
  ifneq ($(findstring FreeBSD, $(TEC_UNAME)), )
    STDINCS += /lib/X11R6/include/gtk-2.0  
  endif
endif

ifdef USE_QT
  override USE_X11 = Yes
  LIBS += QtGui QtCore
  QT_BASE_INC := /usr/include/qt4
  STDINCS += $(QT_BASE_INC) $(QT_BASE_INC)/QtCore $(QT_BASE_INC)/QtGui
  STDDEFS += -DQT_DLL -DQT_QT3SUPPORT_LIB -DQT3_SUPPORT -DQT_GUI_LIB -DQT_CORE_LIB -DQT_THREAD_SUPPORT
endif

ifdef USE_X11
  LIBS += $(X11_LIBS)
  LDIR += $(X11_LIB)
  STDINCS += $(X11_INC)
endif 

LIBS += m

ifneq ($(findstring cygw, $(TEC_UNAME)), )
  # INCLUDES for dependencies, remove references to "c:" and similars
  DEPINCS := $(patsubst c:%, /cygdrive/c%, $(INCLUDES))
  DEPINCS := $(patsubst d:%, /cygdrive/d%, $(DEPINCS))
  DEPINCS := $(patsubst x:%, /cygdrive/x%, $(DEPINCS))
  DEPINCS := $(patsubst t:%, /cygdrive/t%, $(DEPINCS))
  
  DLIBEXT := dll
  APPEXT := .exe
  # Use the cyg prefix to indicate that it is a Cygwin Posix DLL  
  DLIBPRE := cyg
  
  STDLDFLAGS += -Wl,--out-implib=$(TARGETDIR)/lib$(TARGETNAME).dll.a
endif 

#---------------------------------#
#  Building compilation flags that are sets

DEPINCS ?= $(INCLUDES)
DEPINCS := $(addprefix -I, $(DEPINCS))

INCLUDES := $(addprefix -I, $(INCLUDES))
STDINCS := $(addprefix -I, $(STDINCS))
EXTRAINCS := $(addprefix -I, $(EXTRAINCS))
DEFINES := $(addprefix -D, $(DEFINES))

LIBS := $(addprefix -l, $(LIBS))
ifdef LDIR
  LDIR := $(addprefix -L, $(LDIR))
endif


#---------------------------------#
# Definitions of private variables

# Library flags for application and dynamic library linker
LFLAGS += $(LDIR) $(LIBS)
# C compiler flags
CFLAGS   = $(FLAGS) $(STDFLAGS) $(INCLUDES) $(STDINCS) $(EXTRAINCS) $(DEFINES) $(STDDEFS)
# C++ compiler flags
CXXFLAGS = $(CPPFLAGS) $(STDFLAGS) $(INCLUDES) $(STDINCS) $(EXTRAINCS) $(DEFINES) $(STDDEFS)

# Sources with relative path
SOURCES := $(addprefix $(SRCDIR)/, $(SRC))

TARGETAPPNAME := $(TARGETNAME)$(APPEXT)
TARGETSLIBNAME := lib$(TARGETNAME).a
TARGETDLIBNAME := $(DLIBPRE)$(TARGETNAME).$(DLIBEXT)

# Target for applications or libraries
ifeq ($(MAKETYPE), APP)
  TARGET := $(TARGETDIR)/$(TARGETAPPNAME)
else
  ifeq ($(NO_DYNAMIC), Yes) 
    TARGET := $(TARGETDIR)/$(TARGETSLIBNAME)
  else
    TARGET := $(TARGETDIR)/$(TARGETSLIBNAME) $(TARGETDIR)/$(TARGETDLIBNAME)
  endif
endif

# OBJ: list of .o, without path
# OBJS: list of .o with relative path
OBJ = $(notdir $(SRC))
OBJ := $(OBJ:.c=.o)
OBJ := $(OBJ:.cpp=.o)
OBJ := $(OBJ:.cxx=.o)
OBJ := $(OBJ:.cc=.o)
OBJ := $(OBJ:.f=.o)
OBJ := $(OBJ:.for=.o)
OBJ := $(OBJ:.rc=.ro)
OBJS = $(addprefix $(OBJDIR)/, $(OBJ))

ifdef LOHPACK
  # Package with all LOHs
  LOHS := $(LOHDIR)/$(LOHPACK)
  LOHDIRS :=
else
  # LOH: list of .loh, without path
  # LOHS: list of .loh, with relative path
  LO = $(notdir $(SRCLUA))
  LO := $(LO:.lua=$(LO_SUFFIX).lo)
  LOS = $(addprefix $(OBJROOT)/, $(LO))

  LOH = $(notdir $(SRCLUA))
  LOH := $(LOH:.lua=$(LO_SUFFIX).loh)
  LOHS = $(addprefix $(LOHDIR)/, $(LOH))
endif

# Construct VPATH variable
P-SRC = $(dir $(SRC))
P-SRC += $(dir $(SRCLUA))
VPATH = .:$(foreach dir,$(P-SRC),$(if $(dir)="./",:$(dir)))


#---------------------------------#
# Main Rule - Build Everything that it is necessary

.PHONY: tecmake 
ifeq ($(MAKETYPE), APP)
  tecmake: print-start directories application scripts
else
  ifeq ($(NO_DYNAMIC), Yes) 
    tecmake: print-start directories static-lib
  else
    tecmake: print-start directories static-lib dynamic-lib
  endif
endif

.PHONY: print-start
print-start:
	@echo ''; echo 'Tecmake - Starting [ $(TARGETNAME):$(TEC_UNAME) ]'

  
#---------------------------------#
# Dynamic Library Build

.PHONY: dynamic-lib
dynamic-lib: $(TARGETDIR)/$(TARGETDLIBNAME)

$(TARGETDIR)/$(TARGETDLIBNAME) : $(LOHS) $(OBJS) $(EXTRADEPS)
	$(LD) $(STDLDFLAGS) -o $@ $(OBJS) $(SLIB) $(LFLAGS)
	@echo 'Tecmake - Dynamic Library ($@) Done.'; echo ''

  
#---------------------------------#
# Static Library Build

.PHONY: static-lib
static-lib: $(TARGETDIR)/$(TARGETSLIBNAME)

$(TARGETDIR)/$(TARGETSLIBNAME) : $(LOHS) $(OBJS) $(EXTRADEPS)
	$(AR) $(STDLFLAGS) $@ $(OBJS) $(SLIB) $(LCFLAGS)
	-$(RANLIB) $@
	@echo 'Tecmake - Static Library ($@) Done.'; echo ''

  
#---------------------------------#
# Application Build

.PHONY: application
application: $(TARGETDIR)/$(TARGETAPPNAME)

$(TARGETDIR)/$(TARGETAPPNAME) : $(LOHS) $(OBJS) $(EXTRADEPS)
	$(LINKER) -o $@ $(OBJS) $(SLIB) $(LFLAGS)
	@if [ ! -z "$(STRIP)" ]; then \
	   echo "Striping debug information" ;\
	   strip $@ ;\
	 fi
	@echo 'Tecmake - Application ($@) Done.'; echo ''

  
#---------------------------------#
#  Application Scripts

# Script name
SRELEASE := $(SRCDIR)/$(TARGETNAME)

.PHONY: scripts
ifdef NO_SCRIPTS
  scripts: ;
else
  scripts: $(SRELEASE) ;
endif
  
$(SRELEASE): $(MAKENAME)
	@echo 'Building script $(@F)'
	@echo "#!/bin/csh" > $@
	@echo "# Script generated automatically by tecmake v$(VERSION)" >> $@
	@echo "# Remove the comment bellow to set the LD_LIBRARY_PATH if needed." >> $@
	@echo '#setenv LD_LIBRARY_PATH $(MYLIB1)/lib/$${TEC_UNAME}:$(MYLIB2)/lib/$${TEC_UNAME}:$$LD_LIBRARY_PATH' >> $@
	@echo 'if ( -r app.env ) source app.env' >> $@
	@echo 'exec $(TARGETROOT)/$$TEC_UNAME/$(TARGETNAME) $$*' >> $@
	@chmod a+x $@

  
#---------------------------------#
# Directories Creation

.PHONY: directories
directories: $(OBJDIR) $(TARGETDIR) $(EXTRADIR) $(LOHDIR)

$(OBJDIR) $(TARGETDIR):
	if [ ! -d $@ ] ; then mkdir -p $@ ; fi

ifdef EXTRADIR
  $(EXTRADIR):
	  if [ ! -d $@ ] ; then mkdir -p $@ ; fi
else
  $(EXTRADIR): ;
endif

ifdef LOHDIR
  $(LOHDIR):
	  if [ ! -d $@ ] ; then mkdir -p $@ ; fi
else
  $(LOHDIR): ;
endif


#---------------------------------#
# Compilation Rules

$(OBJDIR)/%.o:  $(SRCDIR)/%.c
	@echo Compiling $(<F)...
	$(CC) -c $(CFLAGS) -o $@ $<

$(OBJDIR)/%.o:  $(SRCDIR)/%.cpp
	@echo Compiling $(<F)...
	$(CPPC) -c $(CXXFLAGS) -o $@ $<

$(OBJDIR)/%.o:  $(SRCDIR)/%.cxx
	@echo Compiling $(<F)...
	$(CPPC) -c $(CXXFLAGS) -o $@ $<

$(OBJDIR)/%.o:  $(SRCDIR)/%.cc
	@echo Compiling $(<F)...
	$(CPPC) -c $(CXXFLAGS) -o $@ $<

$(OBJDIR)/%.o: $(SRCDIR)/%.f
	@echo Compiling $(<F)...
	$(FC) -c $(FFLAGS) -o $@ $<

$(OBJDIR)/%.o: $(SRCDIR)/%.for
	@echo Compiling $(<F)...
	$(FC) -c $(FFLAGS) -o $@ $<

$(OBJDIR)/%.ro:  $(SRCDIR)/%.rc
	@echo Compiling $(<F)...
	$(RCC) $(RCFLAGS) -O coff -o $@ $<

$(LOHDIR)/%.loh:  $(OBJROOT)/%.lo
	@echo Generating $(<F)...
	$(BIN2C) $< > $@

$(OBJROOT)/%$(LO_SUFFIX).lo:  $(SRCLUADIR)/%.lua
	@echo Compiling $(<F)...
	$(LUAC) -o $@ $<

ifdef LOHPACK
$(LOHDIR)/$(LOHPACK):  $(SRCLUA)
	@echo Generating $(<F)...
	$(LUABIN) $(LUAPRE) $(LUAPREFLAGS) -l $(SRCLUADIR) -o $@ $(SRCLUA)
endif

  
#---------------------------------#
# Dependencies

# make depend
#   Build dependencies
.PHONY: depend
depend: $(DEPEND)

$(DEPEND): $(MAKENAME)
  ifdef SRC
	  @echo "" > $(DEPEND)
	  @which $(CPPC) 2> /dev/null 1>&2 ;\
	  if [ $$? -eq 0 ]; then \
	    echo "Building dependencies... (can be slow)" ;\
	    $(CPPC) $(DEPINCS) $(DEFINES) $(STDDEFS) -MM $(SOURCES) | \
	    sed -e '1,$$s/^\([^ ]\)/$$(OBJDIR)\/\1/' > $(DEPEND) ;\
	  else \
	    echo "" ;\
	    echo "$(CPPC) not found. Dependencies can not be built." ;\
	    echo "Must set USE_NODEPEND=Yes." ;\
	    echo "" ;\
	    exit 1 ;\
	  fi
  endif

###################
ifndef USE_NODEPEND
include $(DEPEND)
endif
###################


#---------------------------------#
# Management Rules

# make clean-extra
#   Remove extra files
.PHONY: clean-extra
clean-extra:
	rm -f $(DEPEND) $(SRELEASE) so_locations
	
# make clean-lohs
#   Remove Lua object inclusion files
.PHONY: clean-lohs
clean-lohs:
	rm -f $(LOS) $(LOHS)
	
# make clean-obj
#   Remove object files
.PHONY: clean-obj
clean-obj:
	rm -f $(OBJS)

# make clean-target
#   Remove target
.PHONY: clean-target
clean-target:
	rm -f $(TARGET)

# make clean-dir
.PHONY: clean-dir
clean-dir:
	rm -fr $(OBJROOT) $(TARGETROOT)

# make clean
#   Remove target and object files
.PHONY: clean
clean: clean-target clean-obj

# make strip
#   Remove symbols from executables
.PHONY: strip
strip:
	test -r $(TARGETDIR)/$(TARGETAPPNAME) && strip $(TARGETDIR)/$(TARGETAPPNAME)

# make rebuild
#   Rebuild target and object files 
.PHONY: rebuild
rebuild: clean-extra clean-lohs clean-obj clean-target tecmake

# make relink
#   Rebuild target without rebuilding object files 
.PHONY: relink
relink: clean-target tecmake

# make clean-all-obj
#   Remove target and object files
.PHONY: clean-all-obj
clean-all-obj:
	@for d in $(UNAMES); do \
	  (cd $(OBJROOT)/$$d; echo $(OBJ) | xargs rm -f) ;\
	done

# make clean-all-target
#   Remove libraries and executables for all platforms
.PHONY: clean-all-target
clean-all-target:
	@for d in $(UNAMES); do \
	  (rm -f $(TARGETROOT)/$$d/$(TARGETNAME) $(TARGETROOT)/$$d/$(TARGETSLIBNAME) $(TARGETROOT)/$$d/$(TARGETDLIBNAME)) ;\
	done
 
#---------------------------------#
# Remote build
# There must be aliases in DNS for the known UNAMES
.PHONY: $(UNAMES)
$(UNAMES):
	@cwd=`csh -c "\\pwd"` ; home=`csh -c "cd;\\pwd"` ;\
	 dir=`echo $$cwd | sed -e "s|$$home/||"` ;\
	 xterm -bg black -fg lightblue -T "Tecmake: $@ ($(TARGETNAME))" -e ssh $@ $(REMOTE) $$dir $(TECMAKEFLAGS) $(MAKEFLAGS) & 2> /dev/null


#---------------------------------#

.PHONY: version
version:
	@echo "Tecmake Posix Version $(VERSION)"

#---------------------------------#
