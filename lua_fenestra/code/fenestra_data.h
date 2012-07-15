/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss Daniels 2010 http://XIXs.com
//
/*+-----------------------------------------------------------------------------------------------------------------+*/



// a main window class to hangs everything off of

struct fenestra_data
{
	struct fenestra *fenestra; // up pointer



	bool setup(struct fenestra * _fenestra);
	void clean(void);



};


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// simple data header
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
struct data_header
{
	u32 id;
	u32 version;
	u32 filesize; // not including this header

	void twiddle(void)
	{
	}
	
	void *data(void)
	{
		return ((u8*)(this))+12;
	}
};



#ifdef __cplusplus
extern "C" {
#endif

	LUALIB_API int luaopen_fenestra_core_data (lua_State *l);

#ifdef __cplusplus
};
#endif
