
/*
* list.h
* prototipos das funcoes de manipulacao de lista
* TeCGraf
* 27 Ago 93
*/

#ifndef __LIST_H__
#define __LIST_H__

typedef struct TList_
	{
	 void **h;     /* head */
         int nba;
         int n;     /* Numero de elementos na lista */
	} TList;

#define list_head(l)	    ((l)->h)
#define list_n(l)        ((l)->n)
#define list_nba(l)        ((l)->nba)

TList *cgm_NewList    ( void );
TList *cgm_AppendList ( TList *, void * );
TList *cgm_AddList    ( TList *, int, void * );
TList *cgm_DelList    ( TList *, int );
void  *cgm_GetList    ( TList *, int );
int   cgm_DelEntry    ( TList *, void * );

#endif
