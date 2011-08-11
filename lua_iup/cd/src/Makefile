ifeq ($(OS), Windows_NT)
  TECMAKE_CMD = $(MAKE) --no-print-directory -f ../tecmakewin.mak TEC_UNAME=$(TEC_UNAME)
else
  TECMAKE_CMD = $(MAKE) --no-print-directory -f ../tecmake.mak
endif

.PHONY: do_all cd_freetype cd_ftgl cd cdx11 cdgdk cd_pdflib cdpdf cdgl cdcontextplus cdcairo cdlua3 cdluapdf3 cdluagl3 cdlua5 cdluapdf5 cdluagl5 cdluacontextplus5 cdluacairo5 cdluaim5
do_all: cd_freetype cd_ftgl cd cd_pdflib cdpdf cdgl cdlua5 cdluapdf5 cdluagl5 cdluaim5

cd_freetype:
	@$(TECMAKE_CMD) MF=cd_freetype
cd_ftgl:
	@$(TECMAKE_CMD) MF=cd_ftgl
cd:
	@$(TECMAKE_CMD)
cdx11:
	@$(TECMAKE_CMD) USE_X11=Yes
cdgdk:
	@$(TECMAKE_CMD) USE_GDK=Yes
cd_pdflib:
	@$(TECMAKE_CMD) MF=cd_pdflib
cdpdf:
	@$(TECMAKE_CMD) MF=cdpdf
cdgl:
	@$(TECMAKE_CMD) MF=cdgl
cdcontextplus:
	@$(TECMAKE_CMD) MF=cdcontextplus
cdcairo:
	@$(TECMAKE_CMD) MF=cdcairo
cdlua3:
	@$(TECMAKE_CMD) MF=cdlua3
cdluapdf3:
	@$(TECMAKE_CMD) MF=cdluapdf3
cdluagl3:
	@$(TECMAKE_CMD) MF=cdluagl3
cdlua5:
	@$(TECMAKE_CMD) MF=cdlua5
cdluapdf5:
	@$(TECMAKE_CMD) MF=cdluapdf5
cdluagl5:
	@$(TECMAKE_CMD) MF=cdluagl5
cdluacontextplus5:
	@$(TECMAKE_CMD) MF=cdluacontextplus5
cdluacairo5:
	@$(TECMAKE_CMD) MF=cdluacairo5
cdluaim5:
	@$(TECMAKE_CMD) MF=cdluaim5
