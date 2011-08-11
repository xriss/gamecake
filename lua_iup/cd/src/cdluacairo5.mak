PROJNAME = cd
LIBNAME = cdluacairo

OPT = YES

DEFINES = CD_NO_OLD_INTERFACE
SRCDIR = lua5
SRC = cdluacairo5.c
DEF_FILE = cdluacairo5.def
CHECK_GTK = Yes

ifneq ($(findstring Win, $(TEC_SYSNAME)), )
  # In Win32 will work only for the Win32 base driver, 
  # it will NOT work for the GDK base driver.
  LIBS = cdcairo
else
  ifdef GTK_DEFAULT
    # In Linux will work only for the GDK base driver, 
    # it will NOT work for the X11 base driver.
    # The main cd library already includes the Cairo driver.
    LIBS =
  else
    # In Other Unices will work only for the X11 base driver, 
    # it will NOT work for the GDK base driver.
    LIBS = cdcairo
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
