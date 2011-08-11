/** \file
 * \brief HWND to ihandle table
 *
 * See Copyright Notice in "iup.h"
 */
 
#ifndef __IUPWIN_HANDLE_H 
#define __IUPWIN_HANDLE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Returns the IUP handle given the Windows handle. */

Ihandle* iupwinHandleGet(void* handle);
void iupwinHandleSet(Ihandle *ih);
void iupwinHandleAdd(Ihandle *ih, InativeHandle* hWnd);
void iupwinHandleRemove(Ihandle *ih);
void iupwinHandleInit(void);
void iupwinHandleFinish(void);


#ifdef __cplusplus
}
#endif

#endif
