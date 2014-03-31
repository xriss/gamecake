/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss@XIXs.com 2013
//
/*+-----------------------------------------------------------------------------------------------------------------+*/







struct string_history_chunk
{

	struct string_history_chunk	*next;
	struct string_history_chunk	*prev;

	char	data[1];
};



struct string_history
{
	bool setup(s32 size);
	bool reset(void);
	void clean(void);


	s32		data_size; // amount of memory in the data chunk
	void   *data; // data chunk


// pointers to first and last strings in the data chunk
	struct string_history_chunk	*first;
	struct string_history_chunk	*last;


	s32 line_count; //number of lines in buffer
	s32	line_look;
	
	void add(const char *cp);

	void look_add(s32 change);
	const char * look_get(void);

};
