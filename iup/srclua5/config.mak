PROJNAME = iup
LIBNAME  = iuplua
OPT = YES
DEF_FILE = iuplua.def

DEFINES = IUPLUA_USELOH

ifdef USE_LUA52
  LOHDIR = loh52
  LIBNAME := $(LIBNAME)52
else
  USE_LUA51 = Yes
  LOHDIR = loh51
  LIBNAME := $(LIBNAME)51
endif

NO_LUALINK = Yes
USE_LOH_SUBDIR = Yes
USE_BIN2C_LUA=Yes

INCLUDES = ../include ../src
LDIR = ../lib/$(TEC_UNAME)  
LIBS = iup

CTRLUA = button.lua canvas.lua dialog.lua colordlg.lua clipboard.lua \
       filedlg.lua fill.lua frame.lua hbox.lua normalizer.lua \
       item.lua image.lua imagergb.lua imagergba.lua label.lua \
       menu.lua multiline.lua list.lua separator.lua user.lua \
       submenu.lua text.lua toggle.lua vbox.lua zbox.lua timer.lua \
       sbox.lua split.lua spin.lua spinbox.lua cbox.lua messagedlg.lua \
       radio.lua val.lua tabs.lua fontdlg.lua tree.lua progressbar.lua

GC := $(addsuffix .c, $(basename $(CTRLUA)))
GC := $(addprefix il_, $(GC))

SRCLUA = iuplua.lua constants.lua $(CTRLUA)

$(GC) : il_%.c : %.lua generator.lua
	$(LUABIN) generator.lua $<

SRC = iuplua.c iuplua_api.c iuplua_tree_aux.c iuplua_scanf.c iuplua_getparam.c iuplua_getcolor.c $(GC)
