/*
-- Copyright (C) 2012 Kriss Blank < Kriss@XIXs.com >
-- This file is distributed under the terms of the MIT license.
-- http://en.wikipedia.org/wiki/MIT_License
-- Please ping me if you use it for anything cool...
*/

#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "../wet/util/wet_types.h"
#include "../lib_lua/src/lua.h"
#include "../lib_lua/src/lauxlib.h"
#include "../lib_lua/src/lualib.h"

#include "code/lua_pack.h"


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// Turn a short string to a number, 4 chars max.
// This number can then be compared agains multichar numbers.
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static u32 string_to_id(const char *s)
{
	const u32 test4='abcd'; // lets play find the ace
	const u32 test1='a';	
	int inyourendo= ( (test4&0xff) == test1 ); // true if first char is at bottom, littleendian
	
	if(!s) { return 0; }
	
	int len=strlen(s);
	if(len>4) { len=4; } // only first 4 chars of longer strings
	
	cu8 *p=(cu8 *)s; // unsigned please
	if(inyourendo)
	{
		switch(len)
		{
			case 1: return (p[0]) ;
			case 2:	return (p[0]) | (p[1]<<8) ;
			case 3:	return (p[0]) | (p[1]<<8) | (p[2]<<16) ;
			case 4:	return (p[0]) | (p[1]<<8) | (p[2]<<16) | (p[3]<<24) ;
		}
	}
	else
	{
		switch(len)
		{
			case 1: return (p[0]) ;
			case 2:	return (p[1]) | (p[0]<<8) ;
			case 3:	return (p[2]) | (p[1]<<8) | (p[0]<<16) ;
			case 4:	return (p[3]) | (p[2]<<8) | (p[1]<<16) | (p[0]<<24) ;
		}
	}
	return 0; // if we get here then the string was probably ""
}



/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// read/write a u32 or s32 or f32
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static u32 lua_pack_read_u32 (cu8 *p, int inyourendo)
{
	if(inyourendo) { return (p[0]) | (p[1]<<8) | (p[2]<<16) | (p[3]<<24) ; }
	return (p[3]) | (p[2]<<8) | (p[1]<<16) | (p[0]<<24) ;
}
static s32 lua_pack_read_s32 (cu8 *p, int inyourendo)
{
	u32 r=lua_pack_read_u32 ( p , inyourendo );
	return *((s32*)(&r));
}
static f32 lua_pack_read_f32 (cu8 *p, int inyourendo)
{
	u32 r=lua_pack_read_u32 ( p , inyourendo );
	return *((f32*)(&r));
}
static void lua_pack_write_u32 (u32 d,u8 *p, int inyourendo )
{
	if(inyourendo)
	{
		p[0]=((d)&0xff);
		p[1]=((d>>8)&0xff);
		p[2]=((d>>16)&0xff);
		p[3]=((d>>24)&0xff);
	}
	else
	{
		p[3]=((d)&0xff);
		p[2]=((d>>8)&0xff);
		p[1]=((d>>16)&0xff);
		p[0]=((d>>24)&0xff);
	}
}
static void lua_pack_write_s32 (s32 d,u8 *p, int inyourendo )
{
	lua_pack_write_u32 ( *((u32*)(&d)) ,p,inyourendo );
}
static void lua_pack_write_f32 (f32 d,u8 *p, int inyourendo )
{
	lua_pack_write_f32 ( *((f32*)(&d)) ,p,inyourendo );
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// read/write a u16 or s16
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static u16 lua_pack_read_u16 (cu8 *p, int inyourendo )
{
	if(inyourendo) { return (p[0]) | (p[1]<<8) ; }
	return (p[1]) | (p[0]<<8) ;
}
static s16 lua_pack_read_s16 (cu8 *p, int inyourendo )
{
	u16 r=lua_pack_read_u16 ( p , inyourendo );
	return *((s16*)(&r));
}
static void lua_pack_write_u16 (u16 d,u8 *p, int inyourendo )
{
	if(inyourendo)
	{
		p[0]=((d)&0xff);
		p[1]=((d>>8)&0xff);
	}
	else
	{
		p[1]=((d)&0xff);
		p[0]=((d>>8)&0xff);
	}
}
static void lua_pack_write_s16 (s16 d,u8 *p, int inyourendo )
{
	lua_pack_write_u16 ( *((u16*)(&d)) ,p,inyourendo );
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// read/write a u8 or s8
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static u8 lua_pack_read_u8 (cu8 *p, int inyourendo )
{
	return *p ;
}
static s8 lua_pack_read_s8 (cu8 *p, int inyourendo )
{
	u8 r=lua_pack_read_u8 ( p , inyourendo );
	return *((s8*)(&r));
}
static void lua_pack_write_u8 (u8 d,u8 *p, int inyourendo )
{
	p[0]=d;
}
static void lua_pack_write_s8 (s8 d,u8 *p, int inyourendo )
{
	lua_pack_write_u8 ( *((u8*)(&d)) ,p,inyourendo );
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// what size is this field, so we can allocate a string in advance
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_pack_field_size (u32 def)
{
	switch(def)
	{
		case 's8' :
		case 'u8' :
			return 1;
			
		case 's16' :
		case 'u16' :
			return 2;
			
		case 'f32' :
		case 's32' :
		case 'u32' :
			return 4;
	}
	
	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// read this field into a double (s32 and u32 will fit with no bitloss)
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static double lua_pack_get_field (u32 def,cu8*p,int inyourendo)
{
	switch(def)
	{
		case 's8' : return (double) lua_pack_read_s8 ( p, inyourendo );
		case 'u8' : return (double) lua_pack_read_u8 ( p, inyourendo );
			
		case 's16' : return (double) lua_pack_read_s16 ( p, inyourendo );
		case 'u16' : return (double) lua_pack_read_u16 ( p, inyourendo );
			
		case 'f32' : return (double) lua_pack_read_f32 ( p, inyourendo );
		case 's32' : return (double) lua_pack_read_s32 ( p, inyourendo );
		case 'u32' : return (double) lua_pack_read_u32 ( p, inyourendo );
	}
	
	return 0.0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// write this field into some data
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static void lua_pack_set_field (double d,u32 def,u8*p,int inyourendo)
{
	switch(def)
	{
		case 's8' : lua_pack_write_s8 ( (s8)d,p, inyourendo ); break;
		case 'u8' : lua_pack_write_u8 ( (u8)d,p, inyourendo ); break;
			
		case 's16' : lua_pack_write_s16 ( (s16)d,p, inyourendo ); break;
		case 'u16' : lua_pack_write_u16 ( (u16)d,p, inyourendo ); break;
			
		case 'f32' : lua_pack_write_f32 ( (f32)d,p, inyourendo ); break;
		case 's32' : lua_pack_write_s32 ( (s32)d,p, inyourendo ); break;
		case 'u32' : lua_pack_write_u32 ( (u32)d,p, inyourendo ); break;
	}
	
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// read packed data from a string or (light)userdata into a table
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_pack_load (lua_State *l)
{
double d;
const u8 *ptr=0;
int len;
int off=0;
int n;
int inyourendo=1;

u32 def;

int data_len=0;
int count;

	if(lua_isstring(l,1))
	{
		ptr=(const u8*)lua_tostring(l,1);
	}
	else
	if(lua_isuserdata(l,1))
	{
		ptr=lua_touserdata(l,1);
	}
	else
	if(lua_islightuserdata(l,1))
	{
		ptr=lua_touserdata(l,1);
	}
	else
	{
		lua_pushstring(l,"need a string to load packed data from");
		lua_error(l);
	}
	
	if(!lua_istable(l,2))
	{
		lua_pushstring(l,"need a table to describe packed data");
		lua_error(l);
	}
	
	lua_pushstring(l,"bigend"); // check big endian flag
	lua_rawget(l,2);
	if(lua_toboolean(l,-1)) { inyourendo=0; } // big endian
	lua_pop(l,1);

	
	if(!lua_isnumber(l,3)) // optional start point
	{
		off=(u32)lua_tonumber(l,3);
	}
	
	data_len=0;
	count=0;
	for(n=1;1;n++)
	{
		lua_rawgeti(l,2,n);
		if(lua_isnil(l,-1))// the end
		{
			lua_pop(l,1);
			break;
		}
		def=string_to_id( (u8*) lua_tostring(l,-1) );
		lua_pop(l,1);
		
		data_len+=lua_pack_field_size(def);
		count++;
	}
	
	if(data_len==0) { return 0; } // no data to pack

	
	lua_createtable(l,count,0); // we know size of array so ask for it
	for(n=1;1;n++)
	{
		lua_rawgeti(l,2,n);
		if(lua_isnil(l,-1)) // the end
		{
			lua_pop(l,1);
			break;
		}
		def=string_to_id( (u8*) lua_tostring(l,-1) );
		lua_pop(l,1);
		
		d=lua_pack_get_field(def,ptr+off,inyourendo);
		lua_pushnumber(l,d);
		lua_rawseti(l,-2,n); // save in table
		
		off+=lua_pack_field_size(def); // advance ptr
	}
	
	
	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// write packed data into a string from a table
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_pack_save (lua_State *l)
{
double d;
int len;
int off=0;
int n;
int inyourendo=1;

u32 def;

int data_len=0;
u8 *data=0;
int count;

	if(!lua_istable(l,1))
	{
		lua_pushstring(l,"need a table to save packed data from");
		lua_error(l);
	}
	
	if(!lua_istable(l,2))
	{
		lua_pushstring(l,"need a table to describe packed data");
		lua_error(l);
	}
	
	lua_pushstring(l,"bigend"); // check big endian flag
	lua_rawget(l,2);
	if(lua_toboolean(l,-1)) { inyourendo=0; } // big endian
	lua_pop(l,1);

	
	if(!lua_isnumber(l,3)) // optional start point
	{
		off=(u32)lua_tonumber(l,3);
	}
	
	data_len=0;
	count=0;
	for(n=1;1;n++)
	{
		lua_rawgeti(l,2,n);
		if(lua_isnil(l,-1))// the end
		{
			lua_pop(l,1);
			break;
		}
		def=string_to_id( (u8*) lua_tostring(l,-1) );
		lua_pop(l,1);
		
		data_len+=lua_pack_field_size(def);
		count++;
	}
	
	if(data_len==0) { return 0; } // no data to pack

	data=calloc(data_len,1);
	if(!data)
	{
		lua_pushstring(l,"failed to allocate pack data buffer");
		lua_error(l);
	}
	
	for(n=1;1;n++)
	{
		lua_rawgeti(l,2,n);
		if(lua_isnil(l,-1)) // the end
		{
			lua_pop(l,1);
			break;
		}
		def=string_to_id( (u8*) lua_tostring(l,-1) );
		lua_pop(l,1);
		
		lua_rawgeti(l,1,n); // read data
		d=lua_tonumber(l,-1);
		lua_pop(l,1);
		
		lua_pack_set_field(d,def,data+off,inyourendo);
				
		off+=lua_pack_field_size(def); // advance ptr
	}	

	lua_pushlstring(l,(char *)data,data_len);
	free(data);
	
	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// open library.
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
LUALIB_API int luaopen_wetgenes_pack_core (lua_State *l)
{
	const luaL_reg lib[] =
	{
		{"load",			lua_pack_load},
		{"save",			lua_pack_save},

		{0,0}
	};
		
	lua_newtable(l);
	luaL_openlib(l, NULL, lib, 0);
	return 1;
}

