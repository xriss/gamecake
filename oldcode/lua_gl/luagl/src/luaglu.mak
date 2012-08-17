LIBNAME = luaglu
OPT=Yes

INCLUDES = ../include .

SRC = luaglu.c

USE_LUA51 = Yes
USE_OPENGL = Yes
NO_LUALINK = Yes

LIBS = luagl
LDIR = ../lib/$(TEC_UNAME)

ifneq ($(findstring MacOS, $(TEC_UNAME)), )
  LIBS=
  ifdef USE_MACOS_OPENGL
    LFLAGS = -framework OpenGL
    USE_OPENGL :=
  endif
endif
