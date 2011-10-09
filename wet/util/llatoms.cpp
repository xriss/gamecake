/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss Daniels 2003 http://www.XIXs.com
//
/*+-----------------------------------------------------------------------------------------------------------------+*/

// Linked list funcs

//
// This code was originally written in 68k asm... When I where a lad etc etc etc
//

#include "all.h"


#if defined(_DEBUG)
#endif







/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// Initiate a previously allocated linked list list struc
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
bool llatoms::setup(s32 size , s32 numof)
{
	DHEAD_INIT(puddles);
	DHEAD_INIT(atoms);
	DHEAD_INIT(freeatoms);
	sizeofatoms=size;
	numberofatoms=numof;
	
	return true;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// reeset
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
bool llatoms::reset(void)
{
s32 size,numof;

	size=sizeofatoms;
	numof=numberofatoms;

	clean();

	return setup(size,numof);
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// Free a pool struc
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void llatoms::clean(void)
{
llpuddle *puddle;
	
		puddle=(struct llpuddle *)puddles->last;	// free any allocated puddles in reverse order
		if(puddle)
		while(puddle->prev)
		{
			puddle=puddle->prev;
			MEM_free(puddle->next);																//free
		}
}



/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// Allocate and add a new puddle to a pool
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
llpuddle * llatoms::add_puddle(void)
{
struct llpuddle *ret;
struct llatom *atom;
long i;
	
	ret=(llpuddle *)MEM_calloc(sizeof(llpuddle)+(sizeofatoms*numberofatoms));					//allocation

	if(ret)
	{
		DLIST_PASTE(puddles->last,ret,0);
		atom=(llatom *)(ret+1);

		for(i=0;i<numberofatoms;i++)
		{
			DLIST_PASTE(freeatoms->last,atom,0);
			atom=(llatom *)(((u8*)(atom))+sizeofatoms);
		}
	}
	
	return(ret);
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// Allocate a new atom cleared to 0 except for links, using the free atom list adding new pools if necesary.
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
llatom *	llatoms::alloc(void)
{
llatom *ret;
	
// if no free atoms alocate another pool

	if( freeatoms->first->next==0 ) 
	{
		if(add_puddle()==0) return 0; // couldn't allocate an atom
	}
	
// Return first free atom If I get to here there will be some

	ret=(llatom*)freeatoms->first;
	DLIST_CUT(ret);
	
// Set the data in the atom to zero

	dmem_set(0,(u8 *)(ret),sizeofatoms);
	
	return(ret);
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// Return an atom to the free list
// You should have removed it from any list it was in before calling this.
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void		llatoms::free(llatom *atom)
{
// add this to the front of the free atom list
	DLIST_PASTE(0,atom,freeatoms->first);

}


