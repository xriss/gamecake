#include "all.h"


//
// we can use either this string as a string identifier
// or its address as a light userdata identifier, both will be unique
//
const char *lua_wetwin_ptr_name="wetwin*ptr";

	
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// check that a userdata at the given index is a wetwin object
// return the userdata if it does, otherwise return 0
// this userdata will be a pointer to the real data
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
wetwin_lua **lua_wetwin_ptr_ptr (lua_State *l, int idx)
{
wetwin_lua **pp=0;

	pp = ((wetwin_lua **)luaL_checkudata(l, idx , lua_wetwin_ptr_name));

	return pp;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// *lua_wetwin_check with auto error on 0 ptr
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
wetwin_lua *lua_wetwin_check_ptr (lua_State *l, int idx)
{
wetwin_lua **pp=lua_wetwin_ptr_ptr(l,idx);

	if (*pp == 0)
	{
		luaL_error(l, "bad wetwin userdata" );
	}

	return *pp;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// fill a table in with the current settings
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_wetwin_getinfo (lua_State *l, wetwin_lua *p, int tab)
{
	if(p)
	{
		lua_pushnumber(l,p->width);		lua_setfield(l,tab,"width");
		lua_pushnumber(l,p->height);	lua_setfield(l,tab,"height");
	}
	else
	{
		lua_pushstring(l,"unbound wetwin"); lua_setfield(l,tab,"err");
	}

	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// set info into the given table
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_wetwin_info (lua_State *l)
{
wetwin_lua *p=lua_wetwin_check_ptr(l,1);
	lua_wetwin_getinfo(l,p,2);
	return 0;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// get info about the screen size
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_wetwin_screen (lua_State *l)
{
	
	return 0;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// create and return the data
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_wetwin_create (lua_State *l)
{

	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// destroy data if it exists
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_wetwin_destroy (lua_State *l)
{
wetwin_lua **pp=lua_wetwin_ptr_ptr(l,1);

	if(*pp)
	{
	}
	(*pp)=0;

	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// prepare a gl surface in the window
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_wetwin_context (lua_State *l)
{
wetwin_lua *p=lua_wetwin_check_ptr(l,1);

	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// swap a gl surface 
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_wetwin_swap (lua_State *l)
{
wetwin_lua *p=lua_wetwin_check_ptr(l,1);

	return 0;
}



/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// are there msgs waiting to be delt with?
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_wetwin_peek (lua_State *l)
{
wetwin_lua *p=lua_wetwin_check_ptr(l,1);

	return 0;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// wait a while to see if any msgs turn up
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_wetwin_wait (lua_State *l)
{
wetwin_lua *p=lua_wetwin_check_ptr(l,1);

	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// grab the next msg, and return info about it
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_wetwin_msg (lua_State *l)
{
wetwin_lua *p=lua_wetwin_check_ptr(l,1);

	return 0; // no more msgs
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// wait a while to see if any msgs turn up
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_wetwin_sleep (lua_State *l)
{
    return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// what time is it, with sub second resolution
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_wetwin_time (lua_State *l)
{
s64 rez;
s64 tim;

	QueryPerformanceFrequency( (LARGE_INTEGER *) (&rez) );
	QueryPerformanceCounter( (LARGE_INTEGER *) (&(tim)) );
	lua_pushnumber(l, ((double)tim)/((double)rez) );
	return 1;
	
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// open library.
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
LUALIB_API int luaopen_wetgenes_win_windows(lua_State *l)
{
	const luaL_reg lib[] =
	{
		{"screen",			lua_wetwin_screen},
		
		{"create",			lua_wetwin_create},
		{"destroy",			lua_wetwin_destroy},
		{"info",			lua_wetwin_info},

		{"context",			lua_wetwin_context},
		{"swap",			lua_wetwin_swap},

		{"peek",			lua_wetwin_peek},
		{"wait",			lua_wetwin_wait},
		{"msg",				lua_wetwin_msg},

		{"sleep",			lua_wetwin_sleep},
		{"time",			lua_wetwin_time},
		
		{0,0}
	};

 	const luaL_reg meta[] =
	{
		{"__gc",			lua_wetwin_destroy},
		{0,0}
	};
	
	luaL_newmetatable(l, lua_wetwin_ptr_name);
	luaL_openlib(l, NULL, meta, 0);
	lua_pop(l,1);

	lua_newtable(l);
	luaL_openlib(l, NULL, lib, 0);
	
	return 1;
}

