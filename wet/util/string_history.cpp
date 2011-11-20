/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss Daniels 2003 http://www.XIXs.com
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
#include "all.h"




/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// Setup junk
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
bool string_history::setup(s32 chunk_size)
{
struct string_history_chunk *chunk;


	if(!(data=MEM_calloc(chunk_size)))
	{ goto bogus; }

	data_size=chunk_size;


// setup first chunk as a blank string
	chunk=(struct string_history_chunk *)data;

	chunk->next=0;
	chunk->prev=0;

	first=chunk;
	last=chunk;

	line_count=0;

	return(true);
bogus:
	clean();
	return(false);
}



/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// Reset junk
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
bool string_history::reset(void)
{
	clean();
	return setup(data_size);
}



/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// Clean junk
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void string_history::clean(void)
{

	if(data) { free(data); data=0; }

	data_size=0;

}



/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// add a new line and update all pointers etc
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void string_history::add(const char *cp)
{
s32 len;
s32 used;
s32 need;
s32 sizeof_chunk_header;

struct string_history_chunk *chunk;


	if(!data) return;


	sizeof_chunk_header=((u8*)((first->data)))-((u8*)(first));

	len=strlen(cp);

	used=(u8*)last-(u8*)data;

	need=((sizeof_chunk_header*2)+len+1+3)&~3;

	if((used+need)>=data_size) // not enough space so loop round
	{
		chunk=(struct string_history_chunk *)data;

// shift first along to make space for new terminator
		while(first!=(struct string_history_chunk *)data) // loop to first position
		{
			first=first->next;
			first->prev=0;
			line_count--;
		}
		first=first->next; // then move forward one
		first->prev=0;
		line_count--;

// move the terminator chunk from the end to the first position

		last->prev->next=chunk;
		chunk->prev=last->prev;
		chunk->next=0;
		last=chunk;
	}


//if we are wrapping arround remove chunks till there is enough space

	while( (last<first) && ((((u8*)first)-((u8*)last))<need) )
	{
		first=first->next;
		first->prev=0;
		line_count--;
	}


// turn the terminator chunk into new data

	strcpy(last->data,cp);


// add new terminating chunk

	chunk=(struct string_history_chunk *)(((u8*)last)+need-sizeof_chunk_header);

	last->next=chunk;
	chunk->prev=last;
	chunk->next=0;

	last=chunk;

	line_count++;

	line_look=0;

}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// change look value
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void string_history::look_add(s32 change)
{
	line_look+=change;

	if(line_look<0) line_look=0;
	if(line_look>line_count) line_look=line_count;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// get string we are looking at
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
const char * string_history::look_get(void)
{

struct string_history_chunk *chunk;
s32 i;

	if(line_look<=0) { return ""; }


	for( chunk=last , i=line_look; chunk ; chunk=chunk->prev , i--)
	{
		if(i==0) return chunk->data;
	}

	return "";

}
