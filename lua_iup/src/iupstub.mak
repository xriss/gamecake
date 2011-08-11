PROJNAME = iup
LIBNAME = iupstub
OPT = YES

USE_DLL = Yes

INCLUDES = ../include

SRC = win/iupwindows_main.c 

iupstup-dll:
	@move /y ..\lib\vc6\iupstub.lib ..\lib\dll
