/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss@XIXs.com 2013
//
/*+-----------------------------------------------------------------------------------------------------------------+*/

typedef struct swetwin_lua {

	int			fp_dsp;			// display as file
	fd_set		set_dsp;		// display as set
	Display		*dsp;
	int			screen;
	Window		win;
	
	GLXContext context;

	int width;
	int height;
	
	int joy_fd[4];

} wetwin_lua ;

typedef struct swetwin_lua_wrap { // this can be treated as wetwin_lua **

	wetwin_lua	*p;				// p will point to a
	wetwin_lua	a[1];			

} wetwin_lua_wrap ;

extern const char *lua_wetwin_ptr_name;


#ifdef __cplusplus
extern "C" {
#endif


LUALIB_API	int luaopen_wetgenes_win_linux (lua_State *l);


#ifdef __cplusplus
};
#endif
