PROJNAME = cd
LIBNAME = cdlua

OPT = YES

DEFINES = CD_NO_OLD_INTERFACE
SRCDIR = lua5
SRC = cdlua5.c cdvoid5.c cdlua5ctx.c cdlua5_active.c cdlua5_canvas.c
DEF_FILE = cdlua5.def

ifdef USE_LUA52
  LIBNAME := $(LIBNAME)52
else
  USE_LUA51 = Yes
  LIBNAME := $(LIBNAME)51
endif

NO_LUALINK = Yes
USE_CD = YES
CD = ..
