PROJNAME = iup
LIBNAME = iupluaweb
OPT = YES
DEF_FILE = iupluaweb.def

IUP := ..

DEFINES = IUPLUA_USELOH

USE_IUPLUA = Yes
LIBS = iupweb

ifdef USE_LUA52
  LOHDIR = loh52
  LIBNAME := $(LIBNAME)52
else
  USE_LUA51 = Yes
  LOHDIR = loh51
  LIBNAME := $(LIBNAME)51
endif

NO_LUALINK = Yes
USE_LOH_SUBDIR = Yes
SRCLUA = webbrowser.lua
USE_BIN2C_LUA=Yes

GC = $(addsuffix .c, $(basename $(SRCLUA)))
GC := $(addprefix il_, $(GC))

$(GC) : il_%.c : %.lua generator.lua
	$(LUABIN) generator.lua $<

SRC	= $(GC)
