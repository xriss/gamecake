PROJNAME = cd
LIBNAME = cdluapdf3

OPT = YES

DEF_FILE = cdluapdf.def
SRCDIR = lua3
SRC = cdluapdf.c
DEF_FILE = cdluapdf.def

INCLUDES = ../include
LIBS = cdlua3 cdpdf
LDIR = ../lib/$(TEC_UNAME)

USE_LUA = YES
