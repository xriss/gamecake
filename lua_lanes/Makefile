#
# Lanes/Makefile
#
#   make
#   make test
#   make basic|fifo|keeper|...
#
#   make perftest[-odd|-even|-plain]
#   make launchtest
#
#   make tgz VERSION=x.x
#   make clean
#
# Defines:
#   CFLAGS  ...
#
MODULE = lanes

LUA_FLAGS=
LUA=

N=1000

# Autodetect LUA & LUAC
#
LUA=$(word 1,$(shell which lua5.1) $(shell which lua51) lua)
LUAC=$(word 1,$(shell which luac5.1) $(shell which luac51) luac)

PREFIX=LUA_CPATH=./src/?.so LUA_PATH="src/?.lua;./tests/?.lua"

TIME=time

#---
all: src/lua51-lanes.so

src/lua51-lanes.so: src/*.lua src/*.c src/*.h src/Makefile
	cd src && make

clean:
	cd src && make clean

debug:
	cd src && make
	gdb $(LUA) tests/basic.lua

#--- Testing ---
#
test:
	$(MAKE) basic
	$(MAKE) fifo
	$(MAKE) keeper
	$(MAKE) fibonacci
	$(MAKE) timer
	$(MAKE) atomic
	$(MAKE) cyclic
	$(MAKE) recursive

basic: tests/basic.lua src/lua51-lanes.so
	$(PREFIX) $(LUA) $<

fifo: tests/fifo.lua src/lua51-lanes.so
	$(PREFIX) $(LUA) $<

keeper: tests/keeper.lua src/lua51-lanes.so
	$(PREFIX) $(LUA) $<

fibonacci: tests/fibonacci.lua src/lua51-lanes.so
	$(PREFIX) $(LUA) $<

timer: tests/timer.lua src/lua51-lanes.so
	$(PREFIX) $(LUA) $<

atomic: tests/atomic.lua src/lua51-lanes.so
	$(PREFIX) $(LUA) $<

cyclic: tests/cyclic.lua src/lua51-lanes.so
	$(PREFIX) $(LUA) $<

recursive: tests/recursive.lua src/lua51-lanes.so
	$(PREFIX) $(LUA) $<

#---
perftest-plain: tests/perftest.lua src/lua51-lanes.so
	$(MAKE) _perftest ARGS="$< $(N) -plain"

perftest: tests/perftest.lua src/lua51-lanes.so
	$(MAKE) _perftest ARGS="$< $(N)"

perftest-odd: tests/perftest.lua src/lua51-lanes.so
	$(MAKE) _perftest ARGS="$< $(N) -prio=+2"

perftest-even: tests/perftest.lua src/lua51-lanes.so
	$(MAKE) _perftest ARGS="$< $(N) -prio=-2"

#---
launchtest: tests/launchtest.lua src/lua51-lanes.so
	$(MAKE) _perftest ARGS="$< $(N)"

_perftest:
	$(PREFIX) $(TIME) $(LUA) $(ARGS)


#--- Packaging ---
#
# Make a folder of the same name as tgz, good manners (for the manual
# expander)
#
# "make tgz VERSION=yyyymmdd"
#
VERSION=

tgz:
ifeq "$(VERSION)" ""
	echo "Usage: make tar VERSION=x.x"; false
else
	$(MAKE) clean 
	-rm -rf $(MODULE)-$(VERSION)
	mkdir $(MODULE)-$(VERSION)
	tar c --exclude=.svn --exclude=.DS_Store --exclude="_*" \
	        --exclude="*.tgz" --exclude="*.rockspec" \
	        --exclude=lanes.dev --exclude="$(MODULE)-*" * \
	   | (cd $(MODULE)-$(VERSION) && tar x)
	tar czvf $(MODULE)-$(VERSION).tgz $(MODULE)-$(VERSION)
	md5sum $(MODULE)-$(VERSION).tgz
endif
	
	
#--- Undocumented ---
#
run: src/lua51-lanes.so
	$(PREFIX) $(LUA) -e "require '$(MODULE)'" -i

require: src/lua51-lanes.so
	$(PREFIX) $(LUA) -e "require '$(MODULE)'"

echo:
	@echo aaa

.PROXY:	all clean test require debug _nodemo _notest

