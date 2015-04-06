/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss@XIXs.com 2013
//
/*+-----------------------------------------------------------------------------------------------------------------+*/





//
// A dynamic array, the data pointer may not be cached externally between calls
// nor is it safe to remember even an index nito the array as the data may be compressed with null values removed
// a chunk containing no valid data all 0's is considered removable
// sizeof_chunk must be a multiple of 4 for 32bit niceness
//
struct dynarray
{
	s32 size;			// the size of each chunk in U8s

	s32 granuallity;	// when growing the array keep numof_chunks a multiple of this

	s32 used;			// number of filled chunks, call tidy to update

	s32 numof;			// the number of chunks currently allocated and available in the data

	u8 *data;			// pointer to allocated memory, of size numof_chunks*sizeof_chunk


	bool setup( s32 _size , s32 _granuallity , s32 _numof ); // initialize array to this size/info
	void clean( void); // free the allocated data

	inline bool setup( s32 _size , s32 _granuallity ) // initialize options, don't allocate data
	{
		return setup(_size,_granuallity,0);
	}

	bool grow( s32 by); // increase allocated chunks by at least this many elements

	void tidy( void); // all chunks that are all 0 data are moved to the front of the array and used updated

	void shrink( void); // tidy then reallocate memory to the used size only

	u8* grab( s32 num); // grab some unused chunks, grow array if necessary, returns 0 if cant
};

