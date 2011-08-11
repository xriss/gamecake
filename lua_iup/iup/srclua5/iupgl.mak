PROJNAME = iup
LIBNAME = iupluagl
OPT = YES

DEF_FILE = iupluagl.def
DEFINES = IUPLUA_USELOH

IUP := ..

# Can not use USE_IUPLUA because Tecmake will include "iupluagl51" in linker
USE_IUP3 = Yes
USE_OPENGL = Yes
LIBS = iuplua$(LIBLUASUFX)

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
SRCLUA = glcanvas.lua
USE_BIN2C_LUA=Yes

GC = $(addsuffix .c, $(basename $(SRCLUA)))
GC := $(addprefix il_, $(GC))

$(GC) : il_%.c : %.lua generator.lua
	$(LUABIN) generator.lua $<

SRC	= iuplua_glcanvas.c $(GC)

ifneq ($(findstring MacOS, $(TEC_UNAME)), )
  LIBS:=
endif
