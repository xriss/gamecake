/** \file
 * \brief Processing Counter
 *
 * See Copyright Notice in im_lib.h
 * $Id: im_counter.cpp,v 1.1 2008/10/17 06:10:16 scuri Exp $
 */

#include "im_counter.h"

#include <stdlib.h>
#include <memory.h>


static imCounterCallback iCounterFunc = NULL;
static void* iCounterUserData = NULL;

imCounterCallback imCounterSetCallback(void* user_data, imCounterCallback counter_func)
{
  imCounterCallback old_counter_func = iCounterFunc;
  iCounterFunc = counter_func;
  if (user_data)
    iCounterUserData = user_data;
  return old_counter_func;
}

struct iCounter
{
  int total;
  int current;
  int sequence;
  const char* message;
};

#define MAX_COUNTERS 10
static iCounter iCounterList[MAX_COUNTERS];

int imCounterBegin(const char* title)
{
  static int first = 1;
  if (first)
  {
    memset(iCounterList, 0, MAX_COUNTERS*sizeof(iCounter));
    first = 0;
  }

  if (!iCounterFunc) // counter management is useless
    return -1;

  int counter = -1;
  for (int i = 0; i < MAX_COUNTERS; i++)
  {
    if (iCounterList[i].sequence == 0 ||  // the counter is free
        iCounterList[i].current == 0)     // or we are in a sequence
    {
      counter = i;
      break;
    }
  }

  if (counter == -1) return -1; // too many counters

  iCounter *ct = &iCounterList[counter];

  ct->sequence++;

  if (ct->sequence == 1) // top level counter
    iCounterFunc(counter, iCounterUserData, title, -1);

  return counter;
}

void imCounterEnd(int counter)
{
  if (counter == -1 || !iCounterFunc) return;               // invalid counter

  iCounter *ct = &iCounterList[counter];

  if (ct->sequence == 1) // top level counter
  {
    iCounterFunc(counter, iCounterUserData, NULL, 1001);
    memset(ct, 0, sizeof(iCounter));
  }
  else
    ct->sequence--;
}

int imCounterInc(int counter)
{
  if (counter == -1 || !iCounterFunc)                       // invalid counter
    return 1;

  iCounter *ct = &iCounterList[counter];

  if (ct->sequence == 0 || // counter with no begin or no total
      ct->total == 0)
    return 1;

  const char* msg = NULL;
  if (ct->current == 0)
    msg = ct->message;

  ct->current++;

  int progress = (int)((ct->current * 1000.0f)/ct->total);

  if (ct->current == ct->total)
    ct->current = 0;

  return iCounterFunc(counter, iCounterUserData, msg, progress);
}

int imCounterIncTo(int counter, int count)
{
  if (counter == -1 || !iCounterFunc)                       // invalid counter
    return 1;

  iCounter *ct = &iCounterList[counter];

  if (ct->sequence == 0 || // counter with no begin or no total
      ct->total == 0)
    return 1;

  if (count <= 0) count = 0;
  if (count >= ct->total) count = ct->total;

  ct->current = count;

  const char* msg = NULL;
  if (ct->current == 0)
    msg = ct->message;

  int progress = (int)((ct->current * 1000.0f)/ct->total);

  if (ct->current == ct->total)
    ct->current = 0;

  return iCounterFunc(counter, iCounterUserData, msg, progress);
}

void imCounterTotal(int counter, int total, const char* message)
{
  if (counter == -1 || !iCounterFunc) return;               // invalid counter

  iCounter *ct = &iCounterList[counter];

  if (ct->sequence == 0) return; // counter with no begin

  ct->message = message;
  ct->total = total;
  ct->current = 0;
}
