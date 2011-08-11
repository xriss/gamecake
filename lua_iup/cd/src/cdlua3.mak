PROJNAME = cd
LIBNAME = cdlua3

OPT = YES

DEF_FILE = cdlua.def
SRCDIR = lua3
SRC = cdlua.c toluacd.c toluawd.c cdvoid.c cdluactx.c

USE_LUA = YES
USE_CD = YES
CD = ..
