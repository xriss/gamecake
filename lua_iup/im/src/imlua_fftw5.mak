PROJNAME = im
LIBNAME = imlua_fftw
DEF_FILE = imlua_fftw.def

OPT = YES
DEFINES = IMLUA_USELOH
USE_LOH_SUBDIR = Yes
SRC = lua5/imlua_fftw.c
DEF_FILE = lua5/imlua_fftw.def

SRCLUA = lua5/im_fftw.lua
SRCLUADIR = lua5

LIBS = im_fftw
INCLUDES = lua5

ifdef USE_LUA52
  LIBNAME := $(LIBNAME)52
  LOHDIR = lua5/loh52
else
  USE_LUA51 = Yes
  LIBNAME := $(LIBNAME)51
  LOHDIR = lua5/loh51
endif

USE_IMLUA = YES
NO_LUALINK = Yes
USE_BIN2C_LUA=Yes
IM = ..

ifneq ($(findstring MacOS, $(TEC_UNAME)), )
  USE_IMLUA:=
  INCLUDES += ../include
  LDIR = ../lib/$(TEC_UNAME)
endif
