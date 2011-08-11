/** \file
 * \brief IupImgLib
 *
 * See Copyright Notice in iup.h
 */

#include <stdlib.h>

#include "iup.h"

#include "iup_str.h"
#include "iup_image.h"

#include "iup_imglib.h"

/* GTK and Win32 share the same library in Windows */
/* GTK and Motif share the same library in UNIX */

void IupImageLibOpen(void)
{
#ifndef IUP_IMGLIB_DUMMY
  int motif=0, win32=0, gtk=0;
  char* driver;

  if (IupGetGlobal("_IUP_IMAGELIB_OPEN"))
    return;

  driver = IupGetGlobal("DRIVER");
  if (iupStrEqualNoCase(driver, "GTK"))
    gtk = 1;
  else if (iupStrEqualNoCase(driver, "Motif"))
    motif = 1;
  else if (iupStrEqualNoCase(driver, "Win32"))
    win32 = 1;

  IupSetGlobal("_IUP_IMAGELIB_OPEN", "1");

  /**************** Bitmaps *****************/

#ifndef WIN32
  if (motif)
    iupImglibBitmaps8Open();
#endif

#ifdef WIN32
  if (win32)
    iupImglibBitmapsOpen();
#endif  

  if (gtk)
    iupImglibBitmapsGtkOpen();

  /***************** Icons *****************/

#ifndef WIN32
  if (motif)
    iupImglibIcons8Open();
  else
#endif
    iupImglibIconsOpen();

  /***************** Logos *****************/

#ifdef IUP_IMGLIB_LARGE
#ifndef WIN32
  if (motif)
    iupImglibLogos8Open();
  else
#endif
    iupImglibLogosOpen();

#ifdef WIN32
  if (win32)
  {
    iupImglibLogosWin32Open();
    iupImglibLogosWin32MsgOpen();
  }
#endif  

  if (gtk)
    iupImglibLogosGtkOpen();
#endif  
#endif  
}
 
void iupImageLibLoadAll(void)
{
#ifndef IUP_IMGLIB_DUMMY
  /* Bitmaps */
  iupImageStockLoad("IUP_ActionCancel");
  iupImageStockLoad("IUP_ActionOk");
  iupImageStockLoad("IUP_ArrowDown");
  iupImageStockLoad("IUP_ArrowLeft");
  iupImageStockLoad("IUP_ArrowRight");
  iupImageStockLoad("IUP_ArrowUp");
  iupImageStockLoad("IUP_EditCopy");
  iupImageStockLoad("IUP_EditCut");
  iupImageStockLoad("IUP_EditErase");
  iupImageStockLoad("IUP_EditFind");
  iupImageStockLoad("IUP_EditPaste");
  iupImageStockLoad("IUP_EditRedo");
  iupImageStockLoad("IUP_EditUndo");
  iupImageStockLoad("IUP_FileClose");
  iupImageStockLoad("IUP_FileCloseAll");
  iupImageStockLoad("IUP_FileNew");
  iupImageStockLoad("IUP_FileOpen");
  iupImageStockLoad("IUP_FileProperties");
  iupImageStockLoad("IUP_FileSave");
  iupImageStockLoad("IUP_FileSaveAll");
  iupImageStockLoad("IUP_FileText");
  iupImageStockLoad("IUP_FontBold");
  iupImageStockLoad("IUP_FontDialog");
  iupImageStockLoad("IUP_FontItalic");
  iupImageStockLoad("IUP_MediaForward");
  iupImageStockLoad("IUP_MediaGotoBegin");
  iupImageStockLoad("IUP_MediaGoToEnd");
  iupImageStockLoad("IUP_MediaPause");
  iupImageStockLoad("IUP_MediaPlay");
  iupImageStockLoad("IUP_MediaRecord");
  iupImageStockLoad("IUP_MediaReverse");
  iupImageStockLoad("IUP_MediaRewind");
  iupImageStockLoad("IUP_MediaStop");
  iupImageStockLoad("IUP_MessageError");
  iupImageStockLoad("IUP_MessageHelp");
  iupImageStockLoad("IUP_MessageInfo");
  iupImageStockLoad("IUP_NavigateHome");
  iupImageStockLoad("IUP_NavigateRefresh");
  iupImageStockLoad("IUP_Print");
  iupImageStockLoad("IUP_PrintPreview");
  iupImageStockLoad("IUP_ToolsColor");
  iupImageStockLoad("IUP_ToolsSettings");
  iupImageStockLoad("IUP_ToolsSortAscend");
  iupImageStockLoad("IUP_ToolsSortDescend");
  iupImageStockLoad("IUP_ViewFullScreen");
  iupImageStockLoad("IUP_Webcam");
  iupImageStockLoad("IUP_WindowsCascade");
  iupImageStockLoad("IUP_WindowsTile");
  iupImageStockLoad("IUP_Zoom");
  iupImageStockLoad("IUP_ZoomActualSize");
  iupImageStockLoad("IUP_ZoomIn");
  iupImageStockLoad("IUP_ZoomOut");
  iupImageStockLoad("IUP_ZoomSelection");

  /* Icons */
  iupImageStockLoad("IUP_Tecgraf");
  iupImageStockLoad("IUP_PUC-Rio");
  iupImageStockLoad("IUP_BR");
  iupImageStockLoad("IUP_Lua");
  iupImageStockLoad("IUP_TecgrafPUC-Rio");
  iupImageStockLoad("IUP_Petrobras");

  /* Logos */
#ifdef IUP_IMGLIB_LARGE
  iupImageStockLoad("IUP_LogoTecgraf");
  iupImageStockLoad("IUP_LogoPUC-Rio");
  iupImageStockLoad("IUP_LogoBR");
  iupImageStockLoad("IUP_LogoLua");
  iupImageStockLoad("IUP_LogoTecgrafPUC-Rio");
  iupImageStockLoad("IUP_LogoPetrobras");

#ifdef WIN32
  /* Logos - Win32 Only */
  iupImageStockLoad("IUP_DeviceCamera");
  iupImageStockLoad("IUP_DeviceCD");
  iupImageStockLoad("IUP_DeviceCellPhone");
  iupImageStockLoad("IUP_DeviceComputer");
  iupImageStockLoad("IUP_DeviceFax");
  iupImageStockLoad("IUP_DeviceMP3");
  iupImageStockLoad("IUP_DeviceNetwork");
  iupImageStockLoad("IUP_DevicePDA");
  iupImageStockLoad("IUP_DevicePrinter");
  iupImageStockLoad("IUP_DeviceScanner");
  iupImageStockLoad("IUP_DeviceSound");
  iupImageStockLoad("IUP_DeviceVideo");
#endif

  /* Logos - GTK and Win32 Only */
  iupImageStockLoad("IUP_LogoMessageError");
  iupImageStockLoad("IUP_LogoMessageHelp");
  iupImageStockLoad("IUP_LogoMessageInfo");
  iupImageStockLoad("IUP_LogoMessageSecurity");
  iupImageStockLoad("IUP_LogoMessageWarning");
#endif
#endif  
}
