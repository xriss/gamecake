/** \file
 * \brief GDI+ Control
 *
 * See Copyright Notice in cd.h
 */
 
#include "cd.h"
#include "cd_private.h"
#include "cdgdiplus.h"
#include <stdlib.h>
#include <memory.h>

cdContext* cdContextNativeWindowPlus(void);
cdContext* cdContextImagePlus(void);
cdContext* cdContextDBufferPlus(void);
cdContext* cdContextPrinterPlus(void);
cdContext* cdContextEMFPlus(void);
cdContext* cdContextClipboardPlus(void);
void cdwpGdiPlusStartup(int debug);

void cdInitGdiPlus(void)
{
  cdInitContextPlus();
}

void cdInitContextPlus(void)
{
  cdContext* ctx_list[NUM_CONTEXTPLUS];
  memset(ctx_list, 0, sizeof(ctx_list));

  ctx_list[CD_CTX_NATIVEWINDOW] = cdContextNativeWindowPlus();
  ctx_list[CD_CTX_IMAGE] = cdContextImagePlus();
  ctx_list[CD_CTX_DBUFFER] = cdContextDBufferPlus();
  ctx_list[CD_CTX_PRINTER] = cdContextPrinterPlus();
  ctx_list[CD_CTX_EMF] = cdContextEMFPlus();
  ctx_list[CD_CTX_CLIPBOARD] = cdContextClipboardPlus();

  cdInitContextPlusList(ctx_list);

  cdwpGdiPlusStartup(0);
}

