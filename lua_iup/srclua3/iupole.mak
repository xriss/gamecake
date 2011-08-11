PROJNAME = iup
LIBNAME  = iupluaole3
OPT = YES

USE_LUA  = Yes
USE_OPENGL = Yes
                     
SRC    = iuplua_olecontrol.c

INCLUDES = ../include
LDIR = ../lib/$(TEC_UNAME)  
LIBS = iup iuplua3 iupole

