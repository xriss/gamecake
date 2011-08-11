/*
* list.c
* Generic List Manager
* TeCGraf
*
*/

#include <stdio.h>
#include <stdlib.h>
#include "list.h"

TList *cgm_NewList ( void )
{
 TList *l = (TList *) malloc ( sizeof (TList) );

 list_nba(l) = 8;
 list_head(l) = (void **)malloc(list_nba(l)*sizeof(void*));
 list_n(l) = 0;

 return l;
}

TList *cgm_AppendList ( TList *l, void *i )
{
 if ( l == NULL )
   return NULL;

 if (list_n(l) == list_nba(l))
 {
  list_nba(l) += 32;
  list_head(l) = (void **)realloc(list_head(l),list_nba(l)*sizeof(void*));
 }

 list_head(l)[list_n(l)]=i;
 list_n(l)++;

 return l;
}

TList *cgm_AddList ( TList *l, int n, void *info )
{
 int i;

 if ( l == NULL)
   return NULL;

 if ( n < 1)
  n=1;

 if ( n > list_n(l) )
   return cgm_AppendList( l, info );

 --n;           /* o usuario ve a lista iniciando em 1 e a */
                /* lista e' implementada iniciando em 0     */

 if (list_n(l) == list_nba(l))
 {
  list_nba(l) *= 2;
  list_head(l) = (void **)realloc(list_head(l),list_nba(l)*sizeof(void*));
 }

 for (i=list_n(l)-1; i>=n; --i)
   list_head(l)[i+1]=list_head(l)[i];

 list_head(l)[n]=info;
 list_n(l)++;
 return l;
}

TList *cgm_DelList ( TList *l, int n )
{
 int i;

 if ( l == NULL || list_n(l) == 0 || n < 0)
   return NULL;

 if ( n < 1)
  n=1;

 if ( n > list_n(l))
  n=list_n(l);

 --n;           /* o usuario ve a lista iniciando em 1 e a */
                /* lista e' implementada iniciando em 0     */
 list_n(l)--;

 for (i=n; i<list_n(l); ++i)
   list_head(l)[i]=list_head(l)[i+1];

 return l;
}

void *cgm_GetList ( TList *l, int n )
{
 if ( l == NULL || n <= 0)
   return NULL;

 return n > list_n(l) ? NULL : list_head(l)[n-1];
}

int cgm_DelEntry ( TList *l, void *entry )
{
 int i=1;

 if ( l == NULL || list_n(l) == 0)
   return 0;

 for (i=0; i< list_n(l); ++i)
  if (list_head(l)[i]==entry)
  {
   cgm_DelList(l,i+1);              /* o usuario ve a lista iniciando em 1 e a */
                                /* lista e' implementada iniciando em 0     */
   return i+1;
  }
 return 0;
}

