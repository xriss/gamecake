/** \file
 * \brief Simple expandable array
 *
 * See Copyright Notice in "iup.h"
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>

#include "iup_array.h"
#include "iup_assert.h"


struct _Iarray
{
  void* data;
  int count;
  int max_count;
  int elem_size;
  int start_count;
};

Iarray* iupArrayCreate(int start_count, int elem_size)
{
  Iarray* iarray = (Iarray*)malloc(sizeof(Iarray));
  iarray->count = 0;
  iarray->elem_size = elem_size;
  iarray->max_count = start_count;
  iarray->start_count = start_count;
  iarray->data = malloc(elem_size*start_count);
  iupASSERT(iarray->data!=NULL);
  if (!iarray->data)
  {
    free(iarray);
    return NULL;
  }
  memset(iarray->data, 0, elem_size*start_count);
  return iarray;
}

void iupArrayDestroy(Iarray* iarray)
{
  iupASSERT(iarray!=NULL);
  if (!iarray)
    return;
  if (iarray->data) 
  {
    memset(iarray->data, 0, iarray->elem_size*iarray->max_count);
    free(iarray->data);
  }
  free(iarray);
}

void* iupArrayGetData(Iarray* iarray)
{
  iupASSERT(iarray!=NULL);
  if (!iarray)
    return NULL;
  return iarray->data;
}

void* iupArrayInc(Iarray* iarray)
{
  iupASSERT(iarray!=NULL);
  if (!iarray)
    return NULL;
  if (iarray->count >= iarray->max_count)
  {
    int old_count = iarray->max_count;
    iarray->max_count += iarray->start_count;
    iarray->data = realloc(iarray->data, iarray->elem_size*iarray->max_count);
    iupASSERT(iarray->data!=NULL);
    if (!iarray->data)
      return NULL;
    memset((unsigned char*)iarray->data + iarray->elem_size*old_count, 0, iarray->elem_size*(iarray->max_count-old_count));
  }
  iarray->count++;
  return iarray->data;
}

void* iupArrayAdd(Iarray* iarray, int new_count)
{
  iupASSERT(iarray!=NULL);
  if (!iarray)
    return NULL;
  if (iarray->count+new_count > iarray->max_count)
  {
    int old_count = iarray->max_count;
    iarray->max_count += new_count;
    iarray->data = realloc(iarray->data, iarray->elem_size*iarray->max_count);
    iupASSERT(iarray->data!=NULL);
    if (!iarray->data)
      return NULL;
    memset((unsigned char*)iarray->data + iarray->elem_size*old_count, 0, iarray->elem_size*(iarray->max_count-old_count));
  }
  iarray->count += new_count;
  return iarray->data;
}

int iupArrayCount(Iarray* iarray)
{
  iupASSERT(iarray!=NULL);
  if (!iarray)
    return 0;
  return iarray->count;
}
