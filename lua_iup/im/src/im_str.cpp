/** \file
 * \brief String Utilities
 *
 * See Copyright Notice in im_lib.h
 * $Id: im_str.cpp,v 1.1 2008/10/17 06:10:16 scuri Exp $
 */


#include <stdlib.h>
#include <memory.h>
#include <assert.h>

#include "im_util.h"

int imStrEqual(const char* str1, const char* str2)
{
  assert(str1);
  assert(str2);

  /* While both strings are equal and not 0 */
  while (*str1 == *str2 && *str1)
  {
    str1++;
    str2++;
  }

  /* Is last char not equal ? */
  if (*str1 != *str2)
    return 0;

  return 1;
}

int imStrNLen(const char* str, int max_len)
{                       
  assert(str);

  const char* start_str = str;

  while(max_len && *str)
  {
    max_len--;
    str++;
  }

  return str - start_str;
}

int imStrCheck(const void* data, int count)
{
  const char* str = (char*)data;

  if (str[count-1] == 0)
    return 1;

  while(count && *str)
  {
    count--;
    str++;
  }

  if (count > 0)
    return 1;

  return 0;
}

