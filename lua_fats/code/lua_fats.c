/*------------------------------------------------------------------------------

C version of the luajit code as a fall back for wasm build.


*/



#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdint.h>

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"




/*------------------------------------------------------------------------------



*/
static int lua_fats_table_to_floats (lua_State *l)
{
	int i;
	int s=(int)lua_tonumber(l,2);
	int e=(int)lua_tonumber(l,3);
	int len=1+e-s;
	if(len<=0) { return 0; } // no result
	
	float *n=malloc(len*4);
	if(!n) { return 0; } // memory panic

	for(i=s;i<=e;i++)
	{
		lua_rawgeti(l,1,i);
		n[i-s]=(float)lua_tonumber(l,-1);
		lua_pop(l,1);
	}
	
	lua_pushlstring(l,(char*)n,len*4);
	
	free(n);

	return 1;
}


/*------------------------------------------------------------------------------



*/
static int lua_fats_floats_to_table (lua_State *l)
{
	int i;
	int s=(int)lua_tonumber(l,2);
	int e=(int)lua_tonumber(l,3);
	int len=1+e-s;
	if(len<=0) { return 0; }
	
	float *n=(float*)lua_tostring(l,1);

	lua_newtable(l);
	for(i=s;i<=e;i++)
	{
		lua_pushnumber(l,n[i-1]);
		lua_rawseti(l,-2,1+i-s);
	}

	return 1;
}


/*------------------------------------------------------------------------------



*/
static int lua_fats_table_to_doubles (lua_State *l)
{
	int i;
	int s=(int)lua_tonumber(l,2);
	int e=(int)lua_tonumber(l,3);
	int len=1+e-s;
	if(len<=0) { return 0; } // no result
	
	double *n=malloc(len*8);
	if(!n) { return 0; } // memory panic

	for(i=s;i<=e;i++)
	{
		lua_rawgeti(l,1,i);
		n[i-s]=(double)lua_tonumber(l,-1);
		lua_pop(l,1);
	}
	
	lua_pushlstring(l,(char*)n,len*8);
	
	free(n);

	return 1;
}


/*------------------------------------------------------------------------------



*/
static int lua_fats_doubles_to_table (lua_State *l)
{
	int i;
	int s=(int)lua_tonumber(l,2);
	int e=(int)lua_tonumber(l,3);
	int len=1+e-s;
	if(len<=0) { return 0; }
	
	double *n=(double*)lua_tostring(l,1);

	lua_newtable(l);
	for(i=s;i<=e;i++)
	{
		lua_pushnumber(l,n[i-1]);
		lua_rawseti(l,-2,1+i-s);
	}

	return 1;
}




/*------------------------------------------------------------------------------



*/
static int lua_fats_table_to_uint32s (lua_State *l)
{
	int i;
	int s=(int)lua_tonumber(l,2);
	int e=(int)lua_tonumber(l,3);
	int len=1+e-s;
	if(len<=0) { return 0; } // no result
	
	uint32_t *n=malloc(len*4);
	if(!n) { return 0; } // memory panic

	for(i=s;i<=e;i++)
	{
		lua_rawgeti(l,1,i);
		n[i-s]=(uint32_t)lua_tonumber(l,-1);
		lua_pop(l,1);
	}
	
	lua_pushlstring(l,(char*)n,len*4);
	
	free(n);

	return 1;
}


/*------------------------------------------------------------------------------



*/
static int lua_fats_uint32s_to_table (lua_State *l)
{
	int i;
	int s=(int)lua_tonumber(l,2);
	int e=(int)lua_tonumber(l,3);
	int len=1+e-s;
	if(len<=0) { return 0; }
	
	uint32_t *n=(uint32_t*)lua_tostring(l,1);

	lua_newtable(l);
	for(i=s;i<=e;i++)
	{
		lua_pushnumber(l,n[i-1]);
		lua_rawseti(l,-2,1+i-s);
	}

	return 1;
}

/*------------------------------------------------------------------------------



*/
static int lua_fats_table_to_int32s (lua_State *l)
{
	int i;
	int s=(int)lua_tonumber(l,2);
	int e=(int)lua_tonumber(l,3);
	int len=1+e-s;
	if(len<=0) { return 0; } // no result
	
	int32_t *n=malloc(len*4);
	if(!n) { return 0; } // memory panic

	for(i=s;i<=e;i++)
	{
		lua_rawgeti(l,1,i);
		n[i-s]=(int32_t)lua_tonumber(l,-1);
		lua_pop(l,1);
	}
	
	lua_pushlstring(l,(char*)n,len*4);
	
	free(n);

	return 1;
}


/*------------------------------------------------------------------------------



*/
static int lua_fats_int32s_to_table (lua_State *l)
{
	int i;
	int s=(int)lua_tonumber(l,2);
	int e=(int)lua_tonumber(l,3);
	int len=1+e-s;
	if(len<=0) { return 0; }
	
	int32_t *n=(int32_t*)lua_tostring(l,1);

	lua_newtable(l);
	for(i=s;i<=e;i++)
	{
		lua_pushnumber(l,n[i-1]);
		lua_rawseti(l,-2,1+i-s);
	}

	return 1;
}




/*------------------------------------------------------------------------------



*/
static int lua_fats_table_to_uint16s (lua_State *l)
{
	int i;
	int s=(int)lua_tonumber(l,2);
	int e=(int)lua_tonumber(l,3);
	int len=1+e-s;
	if(len<=0) { return 0; } // no result
	
	uint16_t *n=malloc(len*2);
	if(!n) { return 0; } // memory panic

	for(i=s;i<=e;i++)
	{
		lua_rawgeti(l,1,i);
		n[i-s]=(uint16_t)lua_tonumber(l,-1);
		lua_pop(l,1);
	}
	
	lua_pushlstring(l,(char*)n,len*2);
	
	free(n);

	return 1;
}


/*------------------------------------------------------------------------------



*/
static int lua_fats_uint16s_to_table (lua_State *l)
{
	int i;
	int s=(int)lua_tonumber(l,2);
	int e=(int)lua_tonumber(l,3);
	int len=1+e-s;
	if(len<=0) { return 0; }
	
	uint16_t *n=(uint16_t*)lua_tostring(l,1);

	lua_newtable(l);
	for(i=s;i<=e;i++)
	{
		lua_pushnumber(l,n[i-1]);
		lua_rawseti(l,-2,1+i-s);
	}

	return 1;
}

/*------------------------------------------------------------------------------



*/
static int lua_fats_table_to_int16s (lua_State *l)
{
	int i;
	int s=(int)lua_tonumber(l,2);
	int e=(int)lua_tonumber(l,3);
	int len=1+e-s;
	if(len<=0) { return 0; } // no result
	
	int16_t *n=malloc(len*2);
	if(!n) { return 0; } // memory panic

	for(i=s;i<=e;i++)
	{
		lua_rawgeti(l,1,i);
		n[i-s]=(int16_t)lua_tonumber(l,-1);
		lua_pop(l,1);
	}
	
	lua_pushlstring(l,(char*)n,len*2);
	
	free(n);

	return 1;
}


/*------------------------------------------------------------------------------



*/
static int lua_fats_int16s_to_table (lua_State *l)
{
	int i;
	int s=(int)lua_tonumber(l,2);
	int e=(int)lua_tonumber(l,3);
	int len=1+e-s;
	if(len<=0) { return 0; }
	
	int16_t *n=(int16_t*)lua_tostring(l,1);

	lua_newtable(l);
	for(i=s;i<=e;i++)
	{
		lua_pushnumber(l,n[i-1]);
		lua_rawseti(l,-2,1+i-s);
	}

	return 1;
}




/*------------------------------------------------------------------------------



*/
static int lua_fats_table_to_uint8s (lua_State *l)
{
	int i;
	int s=(int)lua_tonumber(l,2);
	int e=(int)lua_tonumber(l,3);
	int len=1+e-s;
	if(len<=0) { return 0; } // no result
	
	uint8_t *n=malloc(len*1);
	if(!n) { return 0; } // memory panic

	for(i=s;i<=e;i++)
	{
		lua_rawgeti(l,1,i);
		n[i-s]=(uint8_t)lua_tonumber(l,-1);
		lua_pop(l,1);
	}
	
	lua_pushlstring(l,(char*)n,len*1);
	
	free(n);

	return 1;
}


/*------------------------------------------------------------------------------



*/
static int lua_fats_uint8s_to_table (lua_State *l)
{
	int i;
	int s=(int)lua_tonumber(l,2);
	int e=(int)lua_tonumber(l,3);
	int len=1+e-s;
	if(len<=0) { return 0; }
	
	uint8_t *n=(uint8_t*)lua_tostring(l,1);

	lua_newtable(l);
	for(i=s;i<=e;i++)
	{
		lua_pushnumber(l,n[i-1]);
		lua_rawseti(l,-2,1+i-s);
	}

	return 1;
}

/*------------------------------------------------------------------------------



*/
static int lua_fats_table_to_int8s (lua_State *l)
{
	int i;
	int s=(int)lua_tonumber(l,2);
	int e=(int)lua_tonumber(l,3);
	int len=1+e-s;
	if(len<=0) { return 0; } // no result
	
	int8_t *n=malloc(len*1);
	if(!n) { return 0; } // memory panic

	for(i=s;i<=e;i++)
	{
		lua_rawgeti(l,1,i);
		n[i-s]=(int8_t)lua_tonumber(l,-1);
		lua_pop(l,1);
	}
	
	lua_pushlstring(l,(char*)n,len*1);
	
	free(n);

	return 1;
}


/*------------------------------------------------------------------------------



*/
static int lua_fats_int8s_to_table (lua_State *l)
{
	int i;
	int s=(int)lua_tonumber(l,2);
	int e=(int)lua_tonumber(l,3);
	int len=1+e-s;
	if(len<=0) { return 0; }
	
	int8_t *n=(int8_t*)lua_tostring(l,1);

	lua_newtable(l);
	for(i=s;i<=e;i++)
	{
		lua_pushnumber(l,n[i-1]);
		lua_rawseti(l,-2,1+i-s);
	}

	return 1;
}




/*------------------------------------------------------------------------------



*/
LUALIB_API int luaopen_wetgenes_fats_core (lua_State *l)
{
	const luaL_Reg lib[] =
	{
		{"table_to_floats",					lua_fats_table_to_floats},
		{"floats_to_table",					lua_fats_floats_to_table},

		{"table_to_doubles",				lua_fats_table_to_doubles},
		{"doubles_to_table",				lua_fats_doubles_to_table},

		{"table_to_uint32s",				lua_fats_table_to_uint32s},
		{"uint32s_to_table",				lua_fats_uint32s_to_table},
		{"table_to_int32s",					lua_fats_table_to_int32s},
		{"int32s_to_table",					lua_fats_int32s_to_table},

		{"table_to_uint16s",				lua_fats_table_to_uint16s},
		{"uint16s_to_table",				lua_fats_uint16s_to_table},
		{"table_to_int16s",					lua_fats_table_to_int16s},
		{"int16s_to_table",					lua_fats_int16s_to_table},

		{"table_to_uint8s",					lua_fats_table_to_uint8s},
		{"uint8s_to_table",					lua_fats_uint8s_to_table},
		{"table_to_int8s",					lua_fats_table_to_int8s},
		{"int8s_to_table",					lua_fats_int8s_to_table},

		{0,0}
	};
		
	lua_newtable(l);
	luaL_openlib(l, NULL, lib, 0);
	return 1;
}

