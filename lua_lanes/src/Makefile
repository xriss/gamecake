#
# Lanes/src/Makefile
#

_SO=.so
MODULE=lanes

SRC=lanes.c threading.c tools.c

OBJ=$(SRC:.c=.o)

# Autodetect LUA_FLAGS and/or LUA_LIBS
#
ifeq "$(shell pkg-config --exists lua5.1 && echo 1)" "1"
  LUA_FLAGS:=$(shell pkg-config --cflags lua5.1)
  LUA_LIBS:=$(shell pkg-config --libs lua5.1)
      #
      # Ubuntu: -I/usr/include/lua5.1 
      #         -llua5.1
else
  ifeq "$(shell pkg-config --exists lua && echo 1)" "1"
    LUA_FLAGS:=$(shell pkg-config --cflags lua)
    LUA_LIBS:=$(shell pkg-config --libs lua)
        #
        # OS X fink (with 'pkg-config'):
        #            -I/sw/include 
        #            -L/sw/lib -llua -lm
  else
    ifeq "$(shell uname -s)" "Darwin"
      # OS X fink (no 'pkg-config')
      LUA_FLAGS:=-I/sw/include
      LUA_LIBS:=-L/sw/lib -llua
    else
      LUA_FLAGS:=
      LUA_LIBS:=-llua
    endif
  endif
endif

# Autodetect LUA & LUAC
#
LUA=$(word 1,$(shell which lua5.1) $(shell which lua51) lua)
LUAC=$(word 1,$(shell which luac5.1) $(shell which luac51) luac)

# Compilation flags (add '-g' for debugging)
#
CFLAGS=-Wall -Werror -O2 $(LUA_FLAGS)
#CFLAGS=-Wall -Werror -O0 -g $(LUA_FLAGS)

SO_FLAGS=

ifeq "$(shell uname -s)" "Darwin"
  SO_FLAGS += -bundle -undefined dynamic_lookup
endif
ifeq "$(shell uname -s)" "Linux"
  # -D_GNU_SOURCE needed for 'pthread_mutexattr_settype'
  CFLAGS += -D_GNU_SOURCE -fPIC
  SO_FLAGS += -shared -lpthread
endif
ifeq "$(shell uname -s)" "BSD"
  SO_FLAGS += -lpthread
endif

#---
all: lua51-$(MODULE)$(_SO)

%.o: %.c *.h Makefile

lua51-$(MODULE)$(_SO): $(OBJ)
	$(CC) $(SO_FLAGS) $(LUA_LIBS) -o $@ $^

clean:
	-rm -rf lua51-$(MODULE)$(_SO) *.lch *.o

lanes.o: keeper.lch

# Note: 'luac -o -' DOES direct output to stdout; it's just not mentioned
#       by 'luac -h' ;)  (v.5.1.3)
#
#       LHF says it's not mentioned, since it won't work on Windows (stdout
#       is a text stream there). True.
#
%.lch: %.lua
	$(LUAC) -o - $< | $(LUA) ../tools/bin2c.lua -o $@

#---
# NSLU2 "slug" Linux ARM
#
nslu2:
	$(MAKE) all CFLAGS="$(CFLAGS) -I/opt/include -L/opt/lib -D_GNU_SOURCE -lpthread"

#---
# Cross compiling to Win32 (MinGW on OS X)
#
# Native MinGW on Windows XP (expecting MinGW to be installed, and
# its bin/ subdir being on the path). 
#
# Point WIN32_LUA51 to an extraction of LuaBinaries dll8 and dev packages.
#
MINGW_GCC=mingw32-gcc
    # i686-pc-mingw32-gcc

win32: $(WIN32_LUA51)/include/lua.h
	$(MAKE) build CC=$(MINGW_GCC) \
            LUA_FLAGS=-I$(WIN32_LUA51)/include \
            LUA_LIBS="-L$(WIN32_LUA51) -llua51" \
            _SO=.dll \
            SO_FLAGS=-shared \
            LUA=lua51

$(WIN32_LUA51)/include/lua.h:
	@echo "Usage: make win32 WIN32_LUA51=<path of extracted LuaBinaries dll8 and dev packages>"
	@echo "                  [MINGW_GCC=...mingw32-gcc]"
	@false

.PROXY:	all clean nslu2 win32

