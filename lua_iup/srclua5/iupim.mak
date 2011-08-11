PROJNAME = iup
LIBNAME  = iupluaim
OPT = YES
                     
DEF_FILE = iupluaim.def
SRC = iuplua_im.c

INCLUDES = ../src
LIBS = iupim

IUP := ..

USE_IUP3 = Yes
USE_IUPLUA = Yes
USE_IMLUA = Yes
NO_LUALINK = Yes

ifdef USE_LUA52
  LIBNAME := $(LIBNAME)52
else
  USE_LUA51 = Yes
  LIBNAME := $(LIBNAME)51
endif

ifneq ($(findstring MacOS, $(TEC_UNAME)), )
  USE_IUPLUA:=
  INCLUDES += ../include
  LDIR = ../lib/$(TEC_UNAME)
endif
