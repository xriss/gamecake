/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss Daniels 2003 http://www.XIXs.com
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
#include "all.h"


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// initialize array to this size/info
//
// the initial size passed into this is not modified by the granuality, so if you know how many you really need
// pass it in here for optimum memory use, this works as a hint of initial array size.
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
bool dynarray::setup( s32 _size , s32 _granuallity , s32 _numof )
{
	DMEM_ZERO(this);

	size=_size;
	granuallity=_granuallity;

	if(_numof)
	{
		data=MEM_calloc(size*_numof);
		if(data)
		{
			numof=_numof;
			return true;
		}
		return false;
	}

	return true;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// free the allocated data
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void dynarray::clean( void)
{
	if(data)
	{
		MEM_free(data);
	}

	DMEM_ZERO(this);
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// increase allocated chunks by at least this many elements
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
bool dynarray::grow( s32 by)
{
s32 new_number;
u8* new_data;

	new_number=((numof+by+granuallity-1)/granuallity);

	new_data=MEM_realloc(data,new_number*granuallity*size);

	if(new_data) // it worked, force new memory to 0
	{
		data=new_data;

		dmem_set(0,data+(numof*size),(new_number*granuallity*size)-(numof*size));

		numof=(new_number*granuallity);

		return true;
	}

	return false;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// all chunks that are all 0 data are moved to the front of the array and used updated
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void dynarray::tidy( void)
{
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// tidy then reallocate memory to the used size only
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void dynarray::shrink( void)
{
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// grab some unused chunks, grow array if necessary, returns 0 if fails
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
u8* dynarray::grab( s32 num)
{
s32 need;
u8 *ret;

	need=used+num-numof;

	if(need>0) // grow array
	{
		if(!grow(need))
		{
			return 0;
		}
	}

// there is now enough chunks available after used

	ret=data+used*size;

	used+=num;

	return ret;
}

