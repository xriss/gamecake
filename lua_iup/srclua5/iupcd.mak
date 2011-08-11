PROJNAME = iup
LIBNAME = iupluacd
OPT = YES

DEFINES = CD_NO_OLD_INTERFACE
SRC = iuplua_cd.c
DEF_FILE = iupluacd.def

# Can not use USE_IUPLUA because Tecmake will include "iupluacd51" in linker

INCLUDES = ../include
LIBS = iuplua$(LIBLUASUFX)
LDIR = ../lib/$(TEC_UNAME)

IUP := ..

USE_CD = YES
USE_IUP3 = YES
NO_LUALINK = Yes
USE_CDLUA = YES

ifdef USE_LUA52
  LIBNAME := $(LIBNAME)52
else
  USE_LUA51 = Yes
  LIBNAME := $(LIBNAME)51
endif

ifneq ($(findstring MacOS, $(TEC_UNAME)), )
  LIBS:=
  USE_CDLUA:=
endif
