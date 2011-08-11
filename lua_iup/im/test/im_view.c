/* IM 3 sample that shows an image.

  Needs "im.lib", "iup.lib", "cd.lib" and "cdiup.lib".

  Usage: im_view <file_name>

    Example: im_view test.tif

    
  Click on image to open another file.
*/

#include <iup.h>
#include <cd.h>
#include <cdiup.h>
#include <im.h>
#include <im_image.h>

#include <stdio.h>
#include <string.h>


static int disable_repaint = 0; /* used to optimize repaint, while opening a new file */

static void PrintError(int error)
{
  switch (error)
  {
  case IM_ERR_OPEN:
    IupMessage("IM", "Error Opening File.");
    break;
  case IM_ERR_MEM:
    IupMessage("IM", "Insuficient memory.");
    break;
  case IM_ERR_ACCESS:
    IupMessage("IM", "Error Accessing File.");
    break;
  case IM_ERR_DATA:
    IupMessage("IM", "Image type not Suported.");
    break;
  case IM_ERR_FORMAT:
    IupMessage("IM", "Invalid Format.");
    break;
  case IM_ERR_COMPRESS:
    IupMessage("IM", "Invalid or unsupported compression.");
    break;
  default:
    IupMessage("IM", "Unknown Error.");
  }
}

static int cbCanvasRepaint(Ihandle* iup_canvas)
{
  cdCanvas* cd_canvas = (cdCanvas*)IupGetAttribute(iup_canvas, "cdCanvas");
  imImage* image = (imImage*)IupGetAttribute(iup_canvas, "imImage");

  if (!cd_canvas || disable_repaint)
    return IUP_DEFAULT;

  cdCanvasActivate(cd_canvas);
  cdCanvasClear(cd_canvas);

  if (!image)
    return IUP_DEFAULT;

  imcdCanvasPutImage(cd_canvas, image, 0, 0, image->width, image->height, 0, 0, 0, 0);
  
  cdCanvasFlush(cd_canvas);
  
  return IUP_DEFAULT;
}

static void ShowImage(char* file_name, Ihandle* iup_dialog)
{
  int error;
  imImage* image = (imImage*)IupGetAttribute(iup_dialog, "imImage");
  if (image) imImageDestroy(image);
  IupSetAttribute(iup_dialog, "imImage", NULL);

  image = imFileImageLoadBitmap(file_name, 0, &error);
  if (error) PrintError(error);
  if (!image) return;

  IupSetAttribute(iup_dialog, "imImage", (char*)image);
  IupStoreAttribute(iup_dialog, "TITLE", file_name);

  cbCanvasRepaint(iup_dialog); /* we can do this because canvas inherit attributes from the dialog */
}

static int cbCanvasButton(Ihandle* iup_canvas, int but, int pressed)
{
  char file_name[200] = "*.*";

  if (but != IUP_BUTTON1 || !pressed)
    return IUP_DEFAULT;
  
  disable_repaint = 1;
  if (IupGetFile(file_name) != 0)
  {
    disable_repaint = 0;
    return IUP_DEFAULT;
  }

  disable_repaint = 0;
  ShowImage(file_name, IupGetDialog(iup_canvas));
  
  return IUP_DEFAULT;
}

static int cbCanvasMap(Ihandle* iup_canvas)
{
  cdCanvas* cd_canvas = cdCreateCanvas(CD_IUP, iup_canvas);
  IupSetAttribute(IupGetDialog(iup_canvas), "cdCanvas", (char*)cd_canvas);
  return IUP_DEFAULT;
}

static int cbDialogClose(Ihandle* iup_dialog)
{
  cdCanvas* cd_canvas = (cdCanvas*)IupGetAttribute(iup_dialog, "cdCanvas");
  imImage* image = (imImage*)IupGetAttribute(iup_dialog, "imImage");

  if (cd_canvas) cdKillCanvas(cd_canvas);
  if (image) imImageDestroy(image);

  IupSetAttribute(iup_dialog, "cdCanvas", NULL);
  IupSetAttribute(iup_dialog, "imImage", NULL);

  return IUP_CLOSE;
}

static Ihandle* CreateDialog(void)
{
  Ihandle *iup_dialog, *iup_canvas;

  iup_canvas = IupCanvas(NULL);
  IupSetCallback(iup_canvas, "BUTTON_CB", (Icallback)cbCanvasButton);
  IupSetCallback(iup_canvas, "ACTION", (Icallback)cbCanvasRepaint);
  IupSetCallback(iup_canvas, "MAP_CB", (Icallback)cbCanvasMap);
  
  iup_dialog = IupDialog(iup_canvas);
  IupSetCallback(iup_dialog, "CLOSE_CB", (Icallback)cbDialogClose);
  IupSetAttribute(iup_dialog, "SIZE", "HALFxHALF");  /* initial size */

  return iup_dialog;
}

int main(int argc, char* argv[])
{
  Ihandle* dlg;

  IupOpen(&argc, &argv);

  dlg = CreateDialog();

  IupShow(dlg);
  
  /* Try to get a file name from the command line. */
  if (argc > 1)
    ShowImage(argv[1], dlg);
  else   
  {
    char file_name[1024] = "*.*";
    if (IupGetFile(file_name) == 0)
      ShowImage(file_name, dlg);
  }
                                   
  IupMainLoop();
  IupDestroy(dlg);
  IupClose();

  return 0;
}
