LIBNAME = luagl
OPT=Yes

INCLUDES = ../include .

SRC = luagl.c luagl_util.c

USE_LUA51 = Yes
USE_OPENGL = Yes
NO_LUALINK = Yes

ifneq ($(findstring MacOS, $(TEC_UNAME)), )
  ifdef USE_MACOS_OPENGL
    LFLAGS = -framework OpenGL
    USE_OPENGL :=
  endif
endif
