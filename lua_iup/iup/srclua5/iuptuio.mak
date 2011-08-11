PROJNAME = iup
LIBNAME  = iupluatuio
OPT = YES
                     
DEF_FILE = iupluatuio.def
SRCLUA = tuioclient.lua

GC = $(addsuffix .c, $(basename $(SRCLUA)))
GC := $(addprefix il_, $(GC))

$(GC) : il_%.c : %.lua generator.lua
	$(LUABIN) generator.lua $<

SRC	= $(GC)

DEFINES = IUPLUA_USELOH
USE_LOH_SUBDIR = Yes

INCLUDES = ../src
LIBS = iuptuio

IUP := ..

USE_IUP3 = Yes
USE_IUPLUA = Yes
NO_LUALINK = Yes
USE_BIN2C_LUA=Yes

ifdef USE_LUA52
  LOHDIR = loh52
  LIBNAME := $(LIBNAME)52
else
  LOHDIR = loh51
  USE_LUA51 = Yes
  LIBNAME := $(LIBNAME)51
endif

ifneq ($(findstring MacOS, $(TEC_UNAME)), )
  USE_IUPLUA:=
  INCLUDES += ../include
  LDIR = ../lib/$(TEC_UNAME)
endif
