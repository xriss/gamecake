#include <stdlib.h>
#include <stdio.h>
#include <iup.h>


static int mdi_tilehoriz(Ihandle* ih)
{
  IupSetAttribute(IupGetDialog(ih), "MDIARRANGE", "TILEHORIZONTAL");
  return IUP_DEFAULT;
}

static int mdi_tilevert(Ihandle* ih)
{
  IupSetAttribute(IupGetDialog(ih), "MDIARRANGE", "TILEVERTICAL");
  return IUP_DEFAULT;
}

static int mdi_cascade(Ihandle* ih)
{
  IupSetAttribute(IupGetDialog(ih), "MDIARRANGE", "CASCADE");
  return IUP_DEFAULT;
}

static int mdi_icon(Ihandle* ih)
{
  IupSetAttribute(IupGetDialog(ih), "MDIARRANGE", "ICON");
  return IUP_DEFAULT;
}

static int mdi_next(Ihandle* ih)
{
  IupSetAttribute(IupGetDialog(ih), "MDIACTIVATE", "NEXT");
  return IUP_DEFAULT;
}

static int mdi_previous(Ihandle* ih)
{
  IupSetAttribute(IupGetDialog(ih), "MDIACTIVATE", "PREVIOUS");
  return IUP_DEFAULT;
}

static int mdi_closeall(Ihandle* ih)
{
  IupSetAttribute(IupGetDialog(ih), "MDICLOSEALL", NULL);
  return IUP_DEFAULT;
}

static int mdi_activate(Ihandle* ih)
{
  printf("mdi_activate(%s)\n", IupGetName(ih));
  return IUP_DEFAULT;
}

static int mdi_new(Ihandle* ih)
{
  static int id = 0;
  Ihandle *box, *cnv, *dlg;

  cnv = IupCanvas(NULL);
  IupSetAttribute(cnv,"BGCOLOR","128 255 0");

  box = IupVbox(cnv, NULL);
  IupSetAttribute(box,"MARGIN","5x5");

  dlg = IupDialog(box);
  IupSetfAttribute(dlg,"TITLE","MDI Child (%d)", id); id++;
  IupSetAttribute(dlg,"MDICHILD","YES");
  IupSetAttribute(dlg,"PARENTDIALOG","mdiFrame");
  IupSetCallback(dlg,"MDIACTIVATE_CB",(Icallback)mdi_activate);
  IupSetAttribute(dlg,"RASTERSIZE","300x300");
//  IupSetAttribute(dlg, "PLACEMENT", "MAXIMIZED");

  IupShow(dlg);

  return IUP_DEFAULT;
}

static void createMenu(void)
{
  Ihandle* winmenu;
  Ihandle* mnu = IupMenu(
  IupSubmenu("MDI",IupMenu(
      IupItem("New", "mdi_new"), 
      NULL)),
  IupSubmenu("Window", winmenu = IupMenu(
      IupItem("Tile Horizontal", "mdi_tilehoriz"), 
      IupItem("Tile Vertical", "mdi_tilevert"), 
      IupItem("Cascade", "mdi_cascade"), 
      IupItem("Icon Arrange", "mdi_icon"), 
      IupItem("Close All", "mdi_closeall"), 
      IupSeparator(),
      IupItem("Next", "mdi_next"), 
      IupItem("Previous", "mdi_previous"), 
      NULL)),
    NULL);
  IupSetHandle("mnu",mnu);
  IupSetHandle("mdiMenu",winmenu);

  IupSetFunction("mdi_new", (Icallback)mdi_new);
  IupSetFunction("mdi_tilehoriz", (Icallback)mdi_tilehoriz);
  IupSetFunction("mdi_tilevert", (Icallback)mdi_tilevert);
  IupSetFunction("mdi_cascade", (Icallback)mdi_cascade);
  IupSetFunction("mdi_icon", (Icallback)mdi_icon);
  IupSetFunction("mdi_next", (Icallback)mdi_next);
  IupSetFunction("mdi_previous", (Icallback)mdi_previous);
  IupSetFunction("mdi_closeall", (Icallback)mdi_closeall);
}

static Ihandle* createFrame(void)
{
  Ihandle *dlg, *cnv;
  cnv = IupCanvas( NULL);
  IupSetAttribute(cnv,"MDICLIENT","YES");
  IupSetAttribute(cnv,"MDIMENU","mdiMenu");

  dlg = IupDialog(cnv);
  IupSetAttribute(dlg,"TITLE","MDI Frame");
  IupSetAttribute(dlg,"MDIFRAME","YES");
  IupSetAttribute(dlg,"RASTERSIZE","800x600");
  IupSetAttribute(dlg,"MENU","mnu");
  IupSetHandle("mdiFrame", dlg);

  return dlg;
}

void MdiTest(void)
{
  Ihandle* dlg;

  createMenu();

  dlg = createFrame();
//  IupSetAttribute(dlg, "PLACEMENT", "MAXIMIZED");
  IupShow(dlg);
}

#ifndef BIG_TEST
int main(int argc, char* argv[])
{
  IupOpen(&argc, &argv);

  MdiTest();

  IupMainLoop();

  IupClose();

  return EXIT_SUCCESS;
}
#endif
