PROJNAME = cd
LIBNAME = cdluagl3

OPT = YES

DEF_FILE = cdluagl.def
SRCDIR = lua3
SRC = cdluagl.c
DEF_FILE = cdluagl.def

INCLUDES = ../include
LIBS = cdlua3 cdgl
LDIR = ../lib/$(TEC_UNAME)

USE_LUA = YES
