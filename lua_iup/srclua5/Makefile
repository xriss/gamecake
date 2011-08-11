ifeq ($(OS), Windows_NT)
  TECMAKE_CMD = $(MAKE) --no-print-directory -f ../tecmakewin.mak TEC_UNAME=$(TEC_UNAME)
  WINLIBS = iupole
else
  TECMAKE_CMD = $(MAKE) --no-print-directory -f ../tecmake.mak
endif

.PHONY: do_all iuplua iupluacd iupluacontrols iuplua_pplot iupluagl iupluaim iupluaimglib iupluatuio iupluaole iupluaweb
do_all: iuplua iupluacd iupluacontrols iuplua_pplot iupluagl iupluaim iupluaimglib iupluatuio $(WINLIBS)

iuplua:
	@$(TECMAKE_CMD) 

iupluacd:
	@$(TECMAKE_CMD) MF=iupcd

iupluacontrols:
	@$(TECMAKE_CMD) MF=iupcontrols

iuplua_pplot:
	@$(TECMAKE_CMD) MF=iup_pplot

iupluagl:
	@$(TECMAKE_CMD) MF=iupgl

iupluaim:
	@$(TECMAKE_CMD) MF=iupim

iupluaimglib:
	@$(TECMAKE_CMD) MF=iupimglib
  
iupluatuio:
	@$(TECMAKE_CMD) MF=iuptuio
  
iupluaole:
	@$(TECMAKE_CMD) MF=iupole
  
iupluaweb:
	@$(TECMAKE_CMD) MF=iupweb
