/** \file
 * \brief Library Management
 *
 * See Copyright Notice in im_lib.h
 * $Id: im_lib.cpp,v 1.9 2010/11/09 12:18:08 scuri Exp $
 */

#include <stdlib.h>
#include <string.h>

#include "im_lib.h"

/* This appears only here to avoid changing the im_lib.h header for bug fixes */
#define IM_VERSION_FIX ".3"
#define IM_VERSION_FIX_NUMBER 3
#define IM_VERSION_FIX_DATE "2010/11/09"

static char *iVersion = "TECVERID.str:IM:LIB:" IM_VERSION IM_VERSION_FIX;

const char iIdent[] =
  "$IM: " IM_VERSION IM_VERSION_FIX " " IM_COPYRIGHT " $\n"
  "$URL: www.tecgraf.puc-rio.br/im $\n";

const char* imVersion(void)
{                  
	 (void)iVersion;
	 (void)iIdent;
   return IM_VERSION IM_VERSION_FIX;
}

const char* imVersionDate(void)
{
#ifdef IM_VERSION_FIX_DATE
  return IM_VERSION_FIX_DATE;
#else
  return IM_VERSION_DATE;
#endif
}

int imVersionNumber(void)
{
  return IM_VERSION_NUMBER+IM_VERSION_FIX_NUMBER;
}
