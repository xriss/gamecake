/** \file
 * \brief Dummy EMF
 *
 * See Copyright Notice in cd.h
 */

#include <stdlib.h>
#include "cd.h"
#include "cd_private.h"
#include "cdemf.h"


cdContext* cdContextEMF(void)
{
  if (cdUseContextPlus(CD_QUERY))
  {
    cdContext* ctx = cdGetContextPlus(CD_CTX_EMF);
    if (ctx != NULL)
      return ctx;
  }

  return NULL;
}


