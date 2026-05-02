/*
-- Copyright (C) 2012 Kriss Blank < Kriss@XIXs.com >
-- This file is distributed under the terms of the MIT license.
-- http://en.wikipedia.org/wiki/MIT_License
-- Please ping me if you use it for anything cool...
--
-- Time And Relative Dimensions In Space
--
-- Best, 3dmathlib, name, ever.
--
*/

#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdint.h>

#include "wet_types.h"
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#include "lua_tardis.h"


// expect luajit CDATA which is just an array of doubles with metatable


// get size of cdata in doubles, or 0 if not cdata
extern int lua_tardis_count (lua_State *l, int idx)
{
int len=0;
char *ts=0;

	ts=lua_typename(l,lua_type(l,idx));
	if(ts && ts[0]=='c' && ts[1]=='d' && ts[2]=='a' && ts[3]=='t' && ts[4]=='a' ) // cdata hax
	{
		lua_getfield(l,idx,"__len"); // get length using metatable
		lua_pushvalue(l,idx);
		lua_call(l,1,1);
		len=(int)lua_tonumber(l,-1);
		lua_pop(l,1);
	}
	return len;
}

// get pointer from cdata at idx or 0 if not cdata
extern double * lua_tardis_cdata (lua_State *l, int idx)
{
double *p=0;
char *ts=0;

	ts=lua_typename(l,lua_type(l,idx));
	if(ts && ts[0]=='c' && ts[1]=='d' && ts[2]=='a' && ts[3]=='t' && ts[4]=='a' ) // cdata hax
	{
		p=(double *)lua_topointer(l,idx); // hax but seems to work
	}

//printf(" tardis %s %p %f,%f,%f,%f\n",ts,p,p[0],p[1],p[2],p[3]);

	return p;
}

// test that our cdata hacks are working
int lua_tardis_test_v4_ptr (lua_State *l)
{
	double *p=lua_tardis_cdata(l,1);
	if(p)
	{
		lua_pushnumber(l,p[0]);
		lua_pushnumber(l,p[1]);
		lua_pushnumber(l,p[2]);
		lua_pushnumber(l,p[3]);
		return 4;
	}
	return 0;
}



/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// open library.
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
LUALIB_API int luaopen_wetgenes_tardis_core (lua_State *l)
{
	const luaL_Reg lib[] =
	{
		{"test_v4_ptr",		lua_tardis_test_v4_ptr},
		
		{0,0}
	};
		
	lua_newtable(l);
	luaL_openlib(l, NULL, lib, 0);
	return 1;
}
