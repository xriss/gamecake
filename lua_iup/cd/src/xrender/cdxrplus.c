/** \file
 * \brief X-Render Control
 *
 * See Copyright Notice in cd.h
 */
 
#include "cd.h"
#include "cd_private.h"
#include <stdlib.h>
#include <memory.h>

cdContext* cdContextNativeWindowPlus(void);
cdContext* cdContextImagePlus(void);
cdContext* cdContextDBufferPlus(void);

void cdInitContextPlus(void)
{
  cdContext* ctx_list[NUM_CONTEXTPLUS];
  memset(ctx_list, 0, sizeof(ctx_list));

  ctx_list[CD_CTX_NATIVEWINDOW] = cdContextNativeWindowPlus();
  ctx_list[CD_CTX_IMAGE] = cdContextImagePlus();
  ctx_list[CD_CTX_DBUFFER] = cdContextDBufferPlus();

  cdInitContextPlusList(ctx_list);
}
