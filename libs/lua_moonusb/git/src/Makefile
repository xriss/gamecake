
ifdef MINGW_PREFIX
MINGW=1
else
LINUX=1
endif

# Lua version
LUAVER?=$(shell lua -e 'print(string.match(_VERSION, "%d+%.%d+") or "5.3")')
ifeq ($(LUAVER),)
# lua-interpreter not found
LUAVER=5.3
endif

# Base install directory
ifdef LINUX
PREFIX?=/usr/local
endif
ifdef MINGW
PREFIX?=$(MINGW_PREFIX)
endif

# Directory where to install Lua modules
L_DIR=$(PREFIX)/share/lua/$(LUAVER)
# Directory where to install Lua C modules
C_DIR=$(PREFIX)/lib/lua/$(LUAVER)
# Directory where to install C headers
H_DIR=$(PREFIX)/include
# Directory where to install C libraries
S_DIR=$(PREFIX)/lib

ifeq ($(D),1)
DEBUG=1
endif

ifdef LINUX
LIBS = -lusb-1.0
endif
ifdef MINGW
LIBS = -lusb-1.0 -llua
endif

Tgt	:= moonusb
Src := $(wildcard *.c)
Objs := $(Src:.c=.o)

INCDIR = -I. -I/usr/include/lua$(LUAVER)

COPT	+= -O2
#COPT	+= -O0 -g
#COPT	+= -m32
COPT	+= -Wfatal-errors
COPT	+= -Wall -Wextra -Wpedantic
COPT	+= -DCOMPAT53_PREFIX=moonusb_compat_
COPT    += -std=gnu99
COPT 	+= -DLUAVER=$(LUAVER)
ifdef LINUX
COPT    += -fpic
COPT	+= -DLINUX
endif
ifdef MINGW
COPT	+= -DMINGW
endif
ifdef DEBUG
COPT	+= -DDEBUG
COPT 	+= -Wshadow -Wsign-compare -Wundef -Wwrite-strings
COPT	+= -Wdisabled-optimization -Wdeclaration-after-statement
COPT    += -Wmissing-prototypes -Wstrict-prototypes -Wnested-externs
COPT    += -Wold-style-definition
#COPT    += -Wc++-compat
endif

override CFLAGS = $(COPT) $(INCDIR)

default: build

where:
	@echo "PREFIX="$(PREFIX)
	@echo "LUAVER="$(LUAVER)
	@echo $(L_DIR)
	@echo $(C_DIR)
	@echo $(H_DIR)
	@echo $(S_DIR)

clean:
	@-rm -f *.so *.dll *.o *.err *.map *.S *~ *.log
	@-rm -f $(Tgt).symbols

install:
	@-mkdir -pv $(H_DIR)
	@-mkdir -pv $(C_DIR)
	@-mkdir -pv $(S_DIR)
	@-mkdir -pv $(L_DIR)
	@-cp -fpv $(Tgt).h $(H_DIR)
	@-cp -fpvr ../$(Tgt) $(L_DIR)
ifdef LINUX
	@-cp -fpv $(Tgt).so $(C_DIR)
	@-ln -fsv $(C_DIR)/$(Tgt).so $(S_DIR)/lib$(Tgt).so
endif
ifdef MINGW
	@-cp -fpv $(Tgt).dll $(C_DIR)
endif

uninstall:
	@-rm -f $(H_DIR)/$(Tgt).h
	@-rm -f $(C_DIR)/$(Tgt).so
	@-rm -f $(S_DIR)/lib$(Tgt).so
	@-rm -fr $(L_DIR)/$(Tgt)
	@-rm -f $(C_DIR)/$(Tgt).dll

build:	clean $(Tgt)

symbols: build
	@objdump -T $(Tgt).so > $(Tgt).symbols

$(Tgt):	$(Objs)
ifdef LINUX
	@$(CXX) -shared -o $(Tgt).so $(Objs) $(LIBDIR) $(LIBS)
endif
ifdef MINGW
	@-$(CXX) -shared -o $(Tgt).dll $(Objs) $(LIBDIR) $(LIBS)
endif
	@-rm -f $(Objs)
	@echo

