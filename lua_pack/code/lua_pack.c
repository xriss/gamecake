/*
-- Copyright (C) 2012 Kriss Blank < Kriss@XIXs.com >
-- This file is distributed under the terms of the MIT license.
-- http://en.wikipedia.org/wiki/MIT_License
-- Please ping me if you use it for anything cool...
*/

#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"


#include "code/lua_pack.h"

// inyourendo is a flag set to 1 for little endian (the default) or 0 for bigendian
// dataformats can be preceeded by - to force little endian although they are little endian by default
// or with + to force big endian


#include "wet_types.h"

extern unsigned char * lua_toluserdata (lua_State *l, int idx, size_t *len);

extern unsigned char * lua_pack_to_buffer (lua_State *l, int idx, size_t *len)
{

	unsigned char *p;

// we can also use a table that gives us a pointer an offset and a size
// you should also keep any original userdata in this table as a reference to keep the memory alive
	if(lua_istable(l,idx))
	{
		lua_getfield(l,idx,"buffer");
		p=lua_pack_to_buffer(l,lua_gettop(l),len); // possible recursion
		lua_pop(l,1);

		if(len) // we want a size
		{
			lua_getfield(l,idx,"sizeof");
			if(lua_isnumber(l,-1))
			{
				*len=lua_tonumber(l,-1);
			}
			lua_pop(l,1);
		}
		
		lua_getfield(l,idx,"offset");
		if(lua_isnumber(l,-1))
		{
			size_t offset=(size_t)lua_tonumber(l,-1);
			p+=offset; // fix pointer using offset (which must not be negative)
			if(len)
			{
				(*len)-=offset; // length is now smaller
			}
		}
		lua_pop(l,1);

		return p;
	}
	
	return lua_toluserdata(l,idx,len);
}

// same as lua_pack_to_buffer but also allows strings
extern const unsigned char * lua_pack_to_const_buffer (lua_State *l, int idx, size_t *len)
{
const unsigned char *p;

// we can also use a table that gives us a pointer an offset and a size
// you should also keep any original userdata in this table as a reference to keep the memory alive
	if(lua_istable(l,idx))
	{
		lua_getfield(l,idx,"buffer");
		p=lua_pack_to_const_buffer(l,lua_gettop(l),len); // possible recursion
		lua_pop(l,1);

		if(len) // we want a size
		{
			lua_getfield(l,idx,"sizeof");
			if(lua_isnumber(l,-1))
			{
				*len=lua_tonumber(l,-1);
			}
			lua_pop(l,1);
		}
		
		lua_getfield(l,idx,"offset");
		if(lua_isnumber(l,-1))
		{
			size_t offset=(size_t)lua_tonumber(l,-1);
			p+=offset; // fix pointer using offset (which must not be negative)
			if(len)
			{
				(*len)-=offset; // length is now smaller
			}
		}
		lua_pop(l,1);

		return p;
	}
	if(lua_isstring(l,idx))
	{
		if(len)
		{
			return (const unsigned char *) lua_tolstring(l,idx,len);
		}
		else
		{
			return (const unsigned char *) lua_tostring(l,idx);
		}
	}
	return (const unsigned char *) lua_toluserdata(l,idx,len);
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// Turn a short string to a number, 4 chars max.
// This number can then be compared against multichar numbers.
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static u32 string_to_id(const char *s)
{
	const u32 test4='abcd'; // lets play find the ace
	const u32 test1='a';	
	int inyourendo= ( (test4&0xff) == test1 ); // true if first char is at bottom, littleendian
	int len;
	const u8 *p;

	if(!s) { return 0; }
	
	len=strlen(s);
	if(len>4) { len=4; } // only first 4 chars of longer strings
	
	p=(const u8 *)s; // unsigned please
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
// read/write a u64 or s64 or f64
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static u64 lua_pack_read_u64 (const u8 *p, int inyourendo)
{
	if(inyourendo) { 
	return ((u64)p[0]) | ((u64)p[1]<<8) | ((u64)p[2]<<16) | ((u64)p[3]<<24) | ((u64)p[4]<<32) | ((u64)p[5]<<40) | ((u64)p[6]<<48) | ((u64)p[7]<<56); }
	return ((u64)p[7]) | ((u64)p[6]<<8) | ((u64)p[5]<<16) | ((u64)p[4]<<24) | ((u64)p[3]<<32) | ((u64)p[2]<<40) | ((u64)p[1]<<48) | ((u64)p[0]<<56);
}
static s64 lua_pack_read_s64 (const u8 *p, int inyourendo)
{
	u64 r=lua_pack_read_u64 ( p , inyourendo );
	return *((s64*)(&r));
}
static f64 lua_pack_read_f64 (const u8 *p, int inyourendo)
{
	u64 r=lua_pack_read_u64 ( p , inyourendo );
	return *((f64*)(&r));
}
static void lua_pack_write_u64 (u64 d,u8 *p, int inyourendo )
{
	if(inyourendo)
	{
		p[0]=((d)&0xff);
		p[1]=((d>>8)&0xff);
		p[2]=((d>>16)&0xff);
		p[3]=((d>>24)&0xff);
		p[4]=((d>>32)&0xff);
		p[5]=((d>>40)&0xff);
		p[6]=((d>>48)&0xff);
		p[7]=((d>>56)&0xff);
	}
	else
	{
		p[7]=((d)&0xff);
		p[6]=((d>>8)&0xff);
		p[5]=((d>>16)&0xff);
		p[4]=((d>>24)&0xff);
		p[3]=((d>>32)&0xff);
		p[2]=((d>>40)&0xff);
		p[1]=((d>>48)&0xff);
		p[0]=((d>>56)&0xff);
	}
}
static void lua_pack_write_s64 (s64 d,u8 *p, int inyourendo )
{
	lua_pack_write_u64 ( *((u64*)(&d)) ,p,inyourendo );
}
static void lua_pack_write_f64 (f64 d,u8 *p, int inyourendo )
{
	lua_pack_write_u64 ( *((u64*)(&d)) ,p,inyourendo );
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// read/write a u32 or s32 or f32
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static u32 lua_pack_read_u32 (const u8 *p, int inyourendo)
{
	if(inyourendo) { return (p[0]) | (p[1]<<8) | (p[2]<<16) | (p[3]<<24) ; }
	return (p[3]) | (p[2]<<8) | (p[1]<<16) | (p[0]<<24) ;
}
static s32 lua_pack_read_s32 (const u8 *p, int inyourendo)
{
	u32 r=lua_pack_read_u32 ( p , inyourendo );
	return *((s32*)(&r));
}
static f32 lua_pack_read_f32 (const u8 *p, int inyourendo)
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
	lua_pack_write_u32 ( *((u32*)(&d)) ,p,inyourendo );
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// read/write a u16 or s16
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static u16 lua_pack_read_u16 (const u8 *p, int inyourendo )
{
	if(inyourendo) { return (p[0]) | (p[1]<<8) ; }
	return (p[1]) | (p[0]<<8) ;
}
static s16 lua_pack_read_s16 (const u8 *p, int inyourendo )
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
static u8 lua_pack_read_u8 (const u8 *p, int inyourendo )
{
	return *p ;
}
static s8 lua_pack_read_s8 (const u8 *p, int inyourendo )
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
		case '-s8' :
		case '+s8' :
		case 'u8' :
		case '-u8' :
		case '+u8' :
			return 1;
			
		case 's16' :
		case '-s16' :
		case '+s16' :
		case 'u16' :
		case '-u16' :
		case '+u16' :
			return 2;
			
		case 'f32' :
		case '-f32' :
		case '+f32' :
		case 's32' :
		case '-s32' :
		case '+s32' :
		case 'u32' :
		case '-u32' :
		case '+u32' :
			return 4;

		case 'f64' :
		case '-f64' :
		case '+f64' :
		case 's64' :
		case '-s64' :
		case '+s64' :
		case 'u64' :
		case '-u64' :
		case '+u64' :
			return 8;
	}
	
	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// read this field into a double (s32 and u32 will fit with no bitloss)
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static double lua_pack_get_field (u32 def,const u8*p)
{
	switch(def)
	{
		case 's8' :
		case '-s8' : return (double) lua_pack_read_s8 ( p, 1 );
		case '+s8' : return (double) lua_pack_read_s8 ( p, 0 );
		
		case 'u8' :
		case '-u8' : return (double) lua_pack_read_u8 ( p, 1 );
		case '+u8' : return (double) lua_pack_read_u8 ( p, 0 );

		case 's16' :
		case '-s16' : return (double) lua_pack_read_s16 ( p, 1 );
		case '+s16' : return (double) lua_pack_read_s16 ( p, 0 );

		case 'u16' :
		case '-u16' : return (double) lua_pack_read_u16 ( p, 1 );
		case '+u16' : return (double) lua_pack_read_u16 ( p, 0 );

		case 'f32' :
		case '-f32' : return (double) lua_pack_read_f32 ( p, 1 );
		case '+f32' : return (double) lua_pack_read_f32 ( p, 0 );

		case 's32' :
		case '-s32' : return (double) lua_pack_read_s32 ( p, 1 );
		case '+s32' : return (double) lua_pack_read_s32 ( p, 0 );

		case 'u32' :
		case '-u32' : return (double) lua_pack_read_u32 ( p, 1 );
		case '+u32' : return (double) lua_pack_read_u32 ( p, 0 );

		case 'f64' :
		case '-f64' : return (double) lua_pack_read_f64 ( p, 1 );
		case '+f64' : return (double) lua_pack_read_f64 ( p, 0 );

		case 's64' :
		case '-s64' : return (double) lua_pack_read_s64 ( p, 1 ); // note that a double does not have enough precision for s64
		case '+s64' : return (double) lua_pack_read_s64 ( p, 0 );

		case 'u64' :
		case '-u64' : return (double) lua_pack_read_u64 ( p, 1 ); // note that a double does not have enough precision for u64
		case '+u64' : return (double) lua_pack_read_u64 ( p, 0 );
	}
		
	return 0.0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// write this field into some data
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static void lua_pack_set_field (double d,u32 def,u8*p)
{
	switch(def)
	{
		case 's8' :
		case '-s8' : lua_pack_write_s8 ( (s8)d,p, 1 ); break;
		case '+s8' : lua_pack_write_s8 ( (s8)d,p, 0 ); break;

		case 'u8' :
		case '-u8' : lua_pack_write_u8 ( (u8)d,p, 1 ); break;
		case '+u8' : lua_pack_write_u8 ( (u8)d,p, 0 ); break;
			
		case 's16' :
		case '-s16' : lua_pack_write_s16 ( (s16)d,p, 1 ); break;
		case '+s16' : lua_pack_write_s16 ( (s16)d,p, 0 ); break;

		case 'u16' :
		case '-u16' : lua_pack_write_u16 ( (u16)d,p, 1 ); break;
		case '+u16' : lua_pack_write_u16 ( (u16)d,p, 0 ); break;
			
		case 'f32' :
		case '-f32' : lua_pack_write_f32 ( (f32)d,p, 1 ); break;
		case '+f32' : lua_pack_write_f32 ( (f32)d,p, 0 ); break;

		case 's32' :
		case '-s32' : lua_pack_write_s32 ( (s32)d,p, 1 ); break;
		case '+s32' : lua_pack_write_s32 ( (s32)d,p, 0 ); break;

		case 'u32' :
		case '-u32' : lua_pack_write_u32 ( (u32)d,p, 1 ); break;
		case '+u32' : lua_pack_write_u32 ( (u32)d,p, 0 ); break;

		case 'f64' :
		case '-f64' : lua_pack_write_f64 ( (f64)d,p, 1 ); break;
		case '+f64' : lua_pack_write_f64 ( (f64)d,p, 0 ); break;

		case 's64' :
		case '-s64' : lua_pack_write_s64 ( (s64)d,p, 1 ); break; // note that a double does not have enough precision for s64
		case '+s64' : lua_pack_write_s64 ( (s64)d,p, 0 ); break;

		case 'u64' :
		case '-u64' : lua_pack_write_u64 ( (u64)d,p, 1 ); break; // note that a double does not have enough precision for u64
		case '+u64' : lua_pack_write_u64 ( (u64)d,p, 0 ); break;
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
size_t len;
int off=0;
int n;

u32 arr=0;

u32 def;
int def_len;

int data_len=0;
int count;

	ptr=lua_pack_to_const_buffer(l,1,&len);
	if(ptr==0)
	{
		lua_pushstring(l,"need data to pack load from");
		return lua_error(l);
	}
	
	if(lua_isstring(l,2))
	{
		arr=string_to_id(  lua_tostring(l,2) ); // array flag and default
	}
	else
	if(!lua_istable(l,2))
	{
		lua_pushstring(l,"need a table to describe packed data");
		return lua_error(l);
	}
	
	if(lua_isnumber(l,3)) // optional start point
	{
		off=(u32)lua_tonumber(l,3);
	}

	if(lua_isnumber(l,4)) // optional forced length
	{
		len=(size_t)lua_tonumber(l,4);
	}
	
	data_len=0;
	count=0;
	if(arr) // read a full array
	{
		def=arr;
		def_len=lua_pack_field_size(arr);
		count=( len - off )/def_len;
		data_len=count*def_len;
		
	}
	else
	{
		for(n=1;1;n++)
		{
			lua_rawgeti(l,2,n);
			if(lua_isnil(l,-1))// the end
			{
				lua_pop(l,1);
				break;
			}
			
			if(lua_isnumber(l,-1)) // the number is the size in bytes of the string we want
			{
				def_len=(int)lua_tonumber(l,-1);
				def=0;
			}
			else
			{
				def=string_to_id( lua_tostring(l,-1) );
				def_len=lua_pack_field_size(def);
			}
			lua_pop(l,1);
			
			data_len+=def_len;			
			count++;
		}
	}
	
	if(data_len==0) { return 0; } // no data to pack
	
	if(data_len+off>len) // data overflow
	{
//printf("PACKLOAD %d %d %d\n",data_len,off,len);
		lua_pushstring(l,"data pack overflow");
		return lua_error(l);
	}

	
	lua_createtable(l,count,0); // we know size of array so ask for it
	for(n=1;n<=count;n++)
	{
		if(!arr) // all the same, def and def_len already set
		{
			lua_rawgeti(l,2,n);
			if(lua_isnil(l,-1)) // the end
			{
				lua_pop(l,1);
				break;
			}
			
			if(lua_isnumber(l,-1)) // the number is the size in bytes of the string we want
			{
				def_len=(int)lua_tonumber(l,-1);
				def=0;
			}
			else
			{
				def=string_to_id( lua_tostring(l,-1) );
				def_len=lua_pack_field_size(def);
			}
			lua_pop(l,1);
		}
		
		if(def==0)
		{
			lua_pushlstring(l,(const char *)(ptr+off),def_len);
			lua_rawseti(l,-2,n); // save in table
		}
		else
		{
			d=lua_pack_get_field(def,ptr+off);
			lua_pushnumber(l,d);
			lua_rawseti(l,-2,n); // save in table
		}
		
		off+=def_len; // advance ptr
	}
	
	lua_pushnumber(l,data_len);
	return 2;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// write packed data into a string from a table
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_pack_save (lua_State *l)
{
double d;
size_t len;
int off=0;
int n;

int defidx=2;
int defadd=0;
int defmul=1;

int datidx=1;
int datadd=0;
int datmul=1;

u32 arr=0;

u32 def;
int def_len;

int data_len=0;
u8 *data=0;
u8 *ptr=0;
int count;

const char *s;
size_t sl;

	if(!lua_istable(l,1))
	{
		lua_pushstring(l,"need table to pack save from");
		return lua_error(l);
	}
	
	if(lua_isstring(l,2))
	{
		arr=string_to_id( lua_tostring(l,2) ); // array flag and default
	}
	else
	if(!lua_istable(l,2))
	{
		defidx=1;
		defadd=-1;
		defmul=2;
		datidx=1;
		datadd=0;
		datmul=2; // use interleaved data in a single table (very simple structure defines)
	}
	
	if(lua_isnumber(l,3)) // optional start point
	{
		off=(u32)lua_tonumber(l,3);
	}

	ptr=lua_pack_to_buffer(l,4,&len); // optional buffer to write too
	
	data_len=0;
	count=0;
	if(arr)
	{
		def=arr;
		def_len=lua_pack_field_size(arr);
		count=lua_objlen(l,1);
		data_len=def_len*count;
	}
	else
	{
		for(n=1;1;n++)
		{
			lua_rawgeti(l,defidx,defadd+(n*defmul));
			if(lua_isnil(l,-1))// the end
			{
				lua_pop(l,1);
				break;
			}

			if(lua_isnumber(l,-1)) // the number is the size in bytes of the string we want
			{
				def_len=(int)lua_tonumber(l,-1);
				def=0;
			}
			else
			{
				def=string_to_id( lua_tostring(l,-1) );
				def_len=lua_pack_field_size(def);
			}
			lua_pop(l,1);
				
			data_len+=def_len;
			count++;
		}
	}
	
	if(data_len==0) { return 0; } // no data to pack
	
	if(ptr) // got a buffer to write intoo
	{
		if(data_len+off>len) // error buffer is too small
		{
//printf("PACKSAVE %d %d %d\n",data_len,off,len);
			lua_pushstring(l,"data pack overflow");
			return lua_error(l);
		}
		data=ptr;
	}
	else // need to allocate one
	{
		data=calloc(data_len,1);
		if(!data)
		{
			lua_pushstring(l,"failed to allocate pack data buffer");
			return lua_error(l);
		}
	}
	
	for(n=1;n<=count;n++)
	{
		if(!arr)
		{
			lua_rawgeti(l,defidx,defadd+(n*defmul));
			if(lua_isnil(l,-1)) // the end
			{
				lua_pop(l,1);
				break;
			}
			if(lua_isnumber(l,-1)) // the number is the size in bytes of the string we want
			{
				def_len=(int)lua_tonumber(l,-1);
				def=0;
			}
			else
			{
				def=string_to_id( lua_tostring(l,-1) );
				def_len=lua_pack_field_size(def);
			}
			lua_pop(l,1);
		}
		
		if(def==0) // raw string to insert
		{
			lua_rawgeti(l,datidx,datadd+(n*datmul));
			s=lua_tolstring(l,-1,&sl);
			lua_pop(l,1);
			if(sl<def_len) { memcpy(data+off,s,sl); }
			else { memcpy(data+off,s,def_len); }
		}
		else
		{
			lua_rawgeti(l,datidx,datadd+(n*datmul));
			d=lua_tonumber(l,-1);
			lua_pop(l,1);
			
			lua_pack_set_field(d,def,data+off);
		}
				
		off+=def_len; // advance ptr
	}	

	if(ptr) // we are writing into a buffer that was passed in
	{
		lua_pushvalue(l,4);
	}
	else
	{
		lua_pushlstring(l,(char *)data,data_len);
		free(data);
	}
	
	lua_pushnumber(l,data_len);
	return 2;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// just allocate a userdata of the given size and return it
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_pack_alloc (lua_State *l)
{
s32 size=(s32)lua_tonumber(l,1);

	if(size<=0)
	{
		lua_pushstring(l,"alloc size must be > 0");
		return lua_error(l);
	}
	
	lua_newuserdata(l,size);
	
	if( lua_istable(l,2) ) // optionally supply a metatable for the new userdata
	{
		lua_pushvalue(l,2);
		lua_setmetatable(l,-2);
	}

	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// grow a userdata, this returns a new bigger data with the old userdata copied into it
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_pack_grow (lua_State *l)
{
size_t len=0;
const u8 *ptr=0;
u8 *newptr=0;
s32 size=(s32)lua_tonumber(l,2);

	ptr=lua_pack_to_const_buffer(l,1,&len); // also allow input strings
	if(!ptr)
	{
		lua_pushstring(l,"need a userdata to grow");
		return lua_error(l);
	}

// this may also get triggered by a light userdata which will have a very large size	
	if(size<=len)
	{
		lua_pushstring(l,"grow size must be bigger than original");
		return lua_error(l);
	}
	
	newptr=lua_newuserdata(l,size);
	
	memcpy(newptr,ptr,len);

	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// find the size of a userdata or string or buffer table
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_pack_sizeof (lua_State *l)
{
size_t len=0;
const u8 *ptr=0;
	
	ptr=lua_pack_to_const_buffer(l,1,&len);
	if(!ptr) { return 0; }

	lua_pushnumber(l,len);
	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// find the size of a given type
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_pack_typesize (lua_State *l)
{
u32 def;
int def_len;

	def=string_to_id( lua_tostring(l,1) );
	def_len=lua_pack_field_size(def);

	lua_pushnumber(l,def_len);
	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// convert a userdata to a string
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_pack_tostring (lua_State *l)
{
size_t len=0;
const u8 *ptr=0;
	
	ptr=lua_pack_to_const_buffer(l,1,&len);
	if(!ptr) { return 0; }
	
	if(lua_isnumber(l,2)) // force size
	{
		len=(size_t)lua_tonumber(l,2);
	}

	lua_pushlstring(l,(const char *)ptr,len);
	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// convert a userdata into a lightuserdata (with byte offset)
// be careful not to use this pointer after freeing the userdata
// this is mostly intended for temporary use as an alternative to a buffer table
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_pack_tolightuserdata (lua_State *l)
{
size_t len;
size_t offset;
const u8 *ptr=0;

	ptr=lua_pack_to_const_buffer(l,1,&len);
	if(!ptr) { return 0; }

	if(lua_isnumber(l,2)) // add to ptr
	{
		offset=(size_t)lua_tonumber(l,2);
		if(offset>=len)
		{
			lua_pushstring(l,"offset too large");
			return lua_error(l);
		}
		ptr+=offset;
	}

	lua_pushlightuserdata(l,(void *)ptr);
	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// copy a string or userdata into a userdata
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_pack_copy (lua_State *l)
{
const u8 *ptr=0;
size_t len;
size_t newlen;
u8 *newptr=0;

	ptr=lua_pack_to_const_buffer(l,1,&len);

	if(lua_isnumber(l,2)) // copy this much from the start of the buffer
	{
		newlen=lua_tonumber(l,2);
		newptr=lua_newuserdata(l,newlen);
		if(len<newlen)
		{
			lua_pushstring(l,"source too small");
			return lua_error(l);
		}
		memcpy(newptr,ptr,newlen);
	}
	else
	if(lua_isuserdata(l,2)) // copy into this buffer
	{
		newptr=lua_pack_to_buffer(l,2,&newlen);
		if(len>newlen)
		{
			lua_pushstring(l,"destination too small");
			return lua_error(l);
		}
		memcpy(newptr,ptr,len);
		lua_pushvalue(l,2);
	}
	else // create a new buffer
	{
		newptr=lua_newuserdata(l,len);
		memcpy(newptr,ptr,len);
	}
	
	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// open library.
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
LUALIB_API int luaopen_wetgenes_pack_core (lua_State *l)
{
	const luaL_Reg lib[] =
	{
		{"load",			lua_pack_load},
		{"save",			lua_pack_save},
		{"copy",			lua_pack_copy},

		{"alloc",			lua_pack_alloc},
		{"sizeof",			lua_pack_sizeof},
		{"tostring",		lua_pack_tostring},

		{"grow",			lua_pack_grow},
		{"typesize",		lua_pack_typesize},

		{"tolightuserdata",	lua_pack_tolightuserdata},
		
		{0,0}
	};
		
	lua_newtable(l);
	luaL_openlib(l, NULL, lib, 0);
	return 1;
}

