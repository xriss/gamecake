/** \file
 * \brief Dummy Printer
 *
 * See Copyright Notice in cd.h
 */

#include <stdlib.h>
#include "cd.h"
#include "cd_private.h"
#include "cdprint.h"



cdContext* cdContextPrinter(void)
{
  if (cdUseContextPlus(CD_QUERY))
  {
    cdContext* ctx = cdGetContextPlus(CD_CTX_PRINTER);
    if (ctx != NULL)
      return ctx;
  }

  return NULL;
}

