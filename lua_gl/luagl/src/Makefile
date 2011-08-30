ifeq ($(OS), Windows_NT)
  TECMAKE_CMD = $(MAKE) --no-print-directory -f ../tecmakewin.mak TEC_UNAME=$(TEC_UNAME)
else
  TECMAKE_CMD = $(MAKE) --no-print-directory -f ../tecmake.mak
endif

.PHONY: do_all luagl luaglu
do_all: luagl luaglu

luagl:
	$(TECMAKE_CMD)
luaglu:
	@$(TECMAKE_CMD) MF=luaglu
