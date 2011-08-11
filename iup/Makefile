ifeq ($(OS), Windows_NT)
  WINLIBS = iupole
endif

.PHONY: do_all iup iupgtk iupmot iupcd iupcontrols iupgl iup_pplot iupim iupimglib ledc iupview iuplua3 iuplua5 iupconsole iupole iupweb iuptuio
do_all: iup iupcd iupcontrols iupgl iup_pplot iupim iupimglib iupweb iuptuio $(WINLIBS) ledc iupview iuplua5 iupconsole

iup iupgtk iupmot:
	@$(MAKE) --no-print-directory -C ./src/ $@
iupcd:
	@$(MAKE) --no-print-directory -C ./srccd/
iupcontrols:
	@$(MAKE) --no-print-directory -C ./srccontrols/
iup_pplot:
	@$(MAKE) --no-print-directory -C ./srcpplot/
iupgl:
	@$(MAKE) --no-print-directory -C ./srcgl/
iupim:
	@$(MAKE) --no-print-directory -C ./srcim/
iupole:
	@$(MAKE) --no-print-directory -C ./srcole/
iupweb:
	@$(MAKE) --no-print-directory -C ./srcweb/
iuptuio:
	@$(MAKE) --no-print-directory -C ./srctuio/
iupimglib:
	@$(MAKE) --no-print-directory -C ./srcimglib/
iuplua3:
	@$(MAKE) --no-print-directory -C ./srclua3/
iuplua5:
	@$(MAKE) --no-print-directory -C ./srclua5/
iupconsole:
	@$(MAKE) --no-print-directory -C ./srcconsole/
ledc:
	@$(MAKE) --no-print-directory -C ./srcledc/
iupview: iupcontrols iup
	@$(MAKE) --no-print-directory -C ./srcview/
