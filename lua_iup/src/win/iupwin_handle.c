/** \file
 * \brief HWND to ihandle table
 *
 * See Copyright Notice in "iup.h"
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>  

#include "iup.h"

#include "iup_object.h"
#include "iup_table.h" 

#include "iupwin_handle.h"  


static Itable* winhandle_table; /* table indexed by HWND containing Ihandle* address */

Ihandle* iupwinHandleGet(void* handle)
{
  Ihandle* ih;
  if (!handle)
    return NULL;
  ih = (Ihandle*)iupTableGet(winhandle_table, (char*)handle);
  if (ih && !iupObjectCheck(ih))
    return NULL;
  return ih;
}

void iupwinHandleSet(Ihandle *ih)
{
  iupTableSet(winhandle_table, (char*)ih->handle, ih, IUPTABLE_POINTER);
}

void iupwinHandleAdd(Ihandle *ih, InativeHandle* hWnd)
{
  iupTableSet(winhandle_table, (char*)hWnd, ih, IUPTABLE_POINTER);
}

void iupwinHandleRemove(Ihandle *ih)
{
  iupTableRemove(winhandle_table, (char*)ih->handle);
}

void iupwinHandleInit(void)
{
  winhandle_table = iupTableCreate(IUPTABLE_POINTERINDEXED);
}

void iupwinHandleFinish(void)
{
  iupTableDestroy(winhandle_table);
  winhandle_table = NULL;
}
