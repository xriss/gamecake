/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss@XIXs.com 2013
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
#include "all.h"





/*------------------------------------------------------------------------------------------------------------------------------*/
//
// cut a struct from a list
//
/*------------------------------------------------------------------------------------------------------------------------------*/
dlist* dlist_cut(dlist *me)
{
	if(me->next)
	{
		me->next->prev=me->prev;
	}

	if(me->prev)
	{
		me->prev->next=me->next;
	}

	me->next=0;
	me->prev=0;

	return me;
}


/*------------------------------------------------------------------------------------------------------------------------------*/
//
// paste a struct into a list
//
/*------------------------------------------------------------------------------------------------------------------------------*/
dlist* dlist_paste(dlist *after , dlist *me , dlist *before)
{

	if(after) //insert after after
	{
		me->next=after->next;
		me->prev=after;
	}
	else //insert before before
	{
		me->next=before;
		me->prev=before->prev;
	}
	if(me->next)	me->next->prev=me;
	if(me->prev)	me->prev->next=me;

	return me;
}


/*------------------------------------------------------------------------------------------------------------------------------*/
//
// initalise a dhead struc
//
/*------------------------------------------------------------------------------------------------------------------------------*/
void dhead_init(dhead *head)
{
	head->first=(dlist*)&head->null;
	head->last=(dlist*)&head->first;
	head->null=(dlist*)0;
}



/*------------------------------------------------------------------------------------------------------------------------------*/
//
// build an array into a dlist, all strucs will be cleared to 0
//
/*------------------------------------------------------------------------------------------------------------------------------*/
void dlist_build(dhead *head , u8 *data , s32 size , s32 count)
{
dlist *cura;

	dmem_set(0,data,size*count);

	dhead_init(head);

	cura=head->last;

	while(count)
	{
		cura = dlist_paste(cura,(dlist*)data,0);
		data+=size;
		count--;
	}
}


