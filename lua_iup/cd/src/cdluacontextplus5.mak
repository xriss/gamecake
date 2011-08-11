PROJNAME = cd
LIBNAME = cdluacontextplus

OPT = YES

DEFINES = CD_NO_OLD_INTERFACE
SRCDIR = lua5
SRC = cdluacontextplus5.c
DEF_FILE = cdluacontextplus5.def

ifneq ($(findstring Win, $(TEC_SYSNAME)), )
  CHECK_GDIPLUS = Yes
  LIBS = cdcontextplus
else
  ifdef GTK_DEFAULT
    CHECK_GTK = Yes
  else
    CHECK_XRENDER = Yes
    LIBS = cdcontextplus
  endif
endif

ifdef USE_LUA52
  LIBNAME := $(LIBNAME)52
else
  USE_LUA51 = Yes
  LIBNAME := $(LIBNAME)51
endif

NO_LUALINK = Yes
USE_CDLUA = YES
CD = ..

ifneq ($(findstring MacOS, $(TEC_UNAME)), )
  USE_CD = YES
  USE_CDLUA:=
  INCLUDES += ../include
  LDIR = ../lib/$(TEC_UNAME)
endif
