/*
-- Copyright (C) 2013 Kriss Blank < Kriss@XIXs.com >
-- This file is distributed under the terms of the MIT license.
-- http://en.wikipedia.org/wiki/MIT_License
-- Please ping me if you use it for anything cool...
*/
#include "all.h"

#include "hidapi.h"


//
// we can use either this string as a string identifier
// or its address as a light userdata identifier, both will be unique
//
const char *lua_hid_ptr_name="hid*ptr";


// the main data struct/pointer we are using

typedef hid_device part_struct ;
typedef part_struct * part_ptr ;



/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// allocate the pointer (set to 0) and return it, it will also be on the top of the lua stack
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
part_ptr *lua_hid_alloc_ptr (lua_State *l)
{
part_ptr *pp;

	pp = (part_ptr *)lua_newuserdata(l, sizeof(part_ptr));
	(*pp)=0;

	luaL_getmetatable(l, lua_hid_ptr_name);
	lua_setmetatable(l, -2);

	return pp;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// check that a userdata at the given index is valid
// return the part_ptr if it does, otherwise return 0
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
part_ptr *lua_hid_get_ptr (lua_State *l, int idx)
{
part_ptr *pp=0;

	pp = ((part_ptr *)luaL_checkudata(l, idx , lua_hid_ptr_name));

	return pp;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// lua_?_get_ptr but with auto error on a 0 ptr and dereferenced
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
part_ptr lua_hid_check_ptr (lua_State *l, int idx)
{
part_ptr *pp=lua_hid_get_ptr(l,idx);

	if (*pp == 0)
	{
		luaL_error(l, "bad hid userdata" );
	}

	return *pp;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// destroy pointer
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_hid_close_idx (lua_State *l, int idx)
{
part_ptr *pp=lua_hid_get_ptr(l,idx);

	if(*pp)
	{
		hid_close(*pp);
	}
	(*pp)=0;

	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// __GC for ptr
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_hid_close (lua_State *l)
{
	return lua_hid_close_idx(l,1);
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// retarted 16bit string pusher, just chops the high bits for now
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static void Wlua_pushstring(lua_State *l, wchar_t *ss)
{
int size=0;
wchar_t *sw;

unsigned char  *sb;
unsigned char  *s;

	for( size=0 , sw=ss ; *sw ; sw++ , size++ ) {}
	
	s=lua_newuserdata(l,size); // temp buffer

	for( sb=s , sw=ss ; *sw ; sw++ , sb++ ) { *sb=(unsigned char)*sw ; } 
	
	lua_pushlstring(l,s,size); // convert buffer to real string
	
	lua_remove(l,-2); // free temp buffer
	
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// These functions are aparently unnecesary... :)
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_hid_init (lua_State *l)
{
	return 0;
}

static int lua_hid_exit (lua_State *l)
{
	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// Return device info of all available devices
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_hid_enumerate (lua_State *l)
{
int idx=0;
struct hid_device_info *devs=0;
struct hid_device_info *dev=0;

	lua_newtable(l);

	devs=hid_enumerate( (unsigned short)lua_tonumber(l,1) , (unsigned short)lua_tonumber(l,2) );

	for( dev=devs , idx=1 ; dev ; dev=dev->next , idx++)
	{
		lua_pushnumber(l,idx);		
		lua_newtable(l);
		
		lua_pushstring(l,"path");			lua_pushstring(l,dev->path);					lua_rawset(l,-3);

		lua_pushstring(l,"serial");			Wlua_pushstring(l,dev->serial_number);			lua_rawset(l,-3);
		lua_pushstring(l,"manufacturer");	Wlua_pushstring(l,dev->manufacturer_string);	lua_rawset(l,-3);
		lua_pushstring(l,"product");		Wlua_pushstring(l,dev->product_string);			lua_rawset(l,-3);

		lua_pushstring(l,"vendor_id");		lua_pushnumber(l,dev->vendor_id);				lua_rawset(l,-3);
		lua_pushstring(l,"product_id");		lua_pushnumber(l,dev->product_id);				lua_rawset(l,-3);
		lua_pushstring(l,"release");		lua_pushnumber(l,dev->release_number);			lua_rawset(l,-3);
		lua_pushstring(l,"usage_page");		lua_pushnumber(l,dev->usage_page);				lua_rawset(l,-3);
		lua_pushstring(l,"usage");			lua_pushnumber(l,dev->usage);					lua_rawset(l,-3);
		lua_pushstring(l,"interface");		lua_pushnumber(l,dev->interface_number);		lua_rawset(l,-3);
		
		lua_rawset(l,-3);
	}
	
	if(devs)
	{
		hid_free_enumeration(devs);
	}
	
	return 1;
}



/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// open a device
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_hid_open_id (lua_State *l)
{
unsigned short vendor_id=0;
unsigned short product_id=0;
hid_device *dev=0;
part_ptr *pp=0;

	if( lua_isnumber(l,1) ) { vendor_id=(unsigned short)lua_tonumber(l,1); }
	if( lua_isnumber(l,2) ) { product_id=(unsigned short)lua_tonumber(l,2); }
	
	dev=hid_open(vendor_id,product_id,0); // TODO: serial number (stooped fecking wstring)
	
	if(!dev) { return 0; } // failed to open
	
	pp=lua_hid_alloc_ptr(l);
	*pp=dev;
	
	return 1;
}

static int lua_hid_open_path (lua_State *l)
{
hid_device *dev=0;
part_ptr *pp=0;

	dev=hid_open_path(lua_tostring(l,1));
	
	if(!dev) { return 0; } // failed to open
	
	pp=lua_hid_alloc_ptr(l);
	*pp=dev;
	
	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// get some device strings
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_hid_get_string (lua_State *l)
{
size_t len=0;
wchar_t *sw;
int idx;
char *s;
int r=-1;
part_ptr p=lua_hid_check_ptr(l,1);

	if(lua_isnumber(l,3)) { len=lua_tonumber(l,3); }
	if(len<=0) { len=255; }
	
	sw=(wchar_t *)lua_newuserdata(l, (len+1) * sizeof(wchar_t) ); // temp wide buffer

	if( lua_isnumber(l,2) )
	{
		int idx=lua_tonumber(l,2);
		r=hid_get_indexed_string(p,idx,sw,len);
	}
	else
	{
		s=(char *)lua_tostring(l,2);
		if( strcmp(s,"manufacturer") == 0)
		{
			r=hid_get_manufacturer_string(p,sw,len);
		}
		else
		if( strcmp(s,"product") == 0 )
		{
			r=hid_get_product_string(p,sw,len);
		}
		else
		if( strcmp(s,"serial_number") == 0 )
		{
			r=hid_get_serial_number_string(p,sw,len);
		}
		else
		{
			luaL_error(l, "unknown hid get string" );
		}
	}
	
	if(r!=0) // no result
	{
		lua_pop(l,1); // kill userdata
		return 0;
	}
	
	Wlua_pushstring(l, sw);

	lua_remove(l,-2); // free temp buffer

	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// error report string
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_hid_error (lua_State *l)
{
wchar_t *sw;
part_ptr p=lua_hid_check_ptr(l,1);

	sw=(wchar_t *)hid_error(p);

	if(!sw) { return 0; } // no string
	
	Wlua_pushstring(l, sw);
	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// set_non_blocking
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_hid_set_nonblocking (lua_State *l)
{
part_ptr p=lua_hid_check_ptr(l,1);
	hid_set_nonblocking(p,(int)lua_tonumber(l,2));
	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// hid_send_feature_report
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_hid_send_feature_report (lua_State *l)
{
int ret=-1;
size_t len=0;
part_ptr p=lua_hid_check_ptr(l,1);
char *str=(char*)lua_tolstring(l,2,&len);

	ret=hid_send_feature_report(p,str,len);
	lua_pushnumber(l,ret);	
	return 1;
}
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// hid_get_feature_report
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_hid_get_feature_report (lua_State *l)
{
int ret=-1;
size_t len=0;
part_ptr p=lua_hid_check_ptr(l,1);
char *dat;
int id=0;

	id=(int)lua_tonumber(l,2);

	if(lua_isnumber(l,3))
	{
		len=lua_tonumber(l,3);
	}
	if(len<=0) { len=256; }
	
	dat=lua_newuserdata(l,len);
	dat[0]=id;

	ret=hid_get_feature_report(p,dat,len);
	
	if(ret<=0)
	{
		lua_pop(l,1);
		return 0;
	}
	
	lua_pushlstring(l,dat,ret);
	lua_remove(l,-2);

	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// write
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_hid_write (lua_State *l)
{
int ret=-1;
size_t len=0;
part_ptr p=lua_hid_check_ptr(l,1);
char *str=(char*)lua_tolstring(l,2,&len);

	ret=hid_write(p,str,len);
	lua_pushnumber(l,ret);	
	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// read
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
static int lua_hid_read (lua_State *l)
{
int ret=-1;
size_t len=0;
part_ptr p=lua_hid_check_ptr(l,1);
char *dat;

	if(lua_isnumber(l,2))
	{
		len=lua_tonumber(l,2);
	}
	if(len<=0) { len=256; }
	
	dat=lua_newuserdata(l,len);

	if(lua_isnumber(l,3)) // timeout?
	{
		ret=hid_read_timeout(p,dat,len,(int)lua_tonumber(l,3));
	}
	else
	{
		ret=hid_read(p,dat,len);
	}
	
	if(ret<=0)
	{
		lua_pop(l,1);
		return 0;
	}
	
	lua_pushlstring(l,dat,ret);
	lua_remove(l,-2);

	return 1;
}



/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// HID library.
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
LUALIB_API int luaopen_wetgenes_hid_core (lua_State *l)
{
	const luaL_Reg lib[] =
	{

		{"init",			lua_hid_init},
		{"exit",			lua_hid_exit},

		{"enumerate",		lua_hid_enumerate},
	
		{"open_path",		lua_hid_open_path},
		{"open_id",			lua_hid_open_id},
		{"close",			lua_hid_close},

		{"get_string",		lua_hid_get_string},

		{"error",			lua_hid_error},

		{"set_nonblocking",	lua_hid_set_nonblocking},

		{"send_feature_report",	lua_hid_send_feature_report},
		{"get_feature_report",	lua_hid_get_feature_report},

		{"write",	lua_hid_write},
		{"read",	lua_hid_read},

		{0,0}
	};

	const luaL_Reg meta[] =
	{
		{"__gc",			lua_hid_close},

		{0,0}
	};

	luaL_newmetatable(l, lua_hid_ptr_name);
	luaL_openlib(l, NULL, meta, 0);
	lua_pop(l,1);
	
	
	lua_newtable(l);
	luaL_openlib(l, NULL, lib, 0);
	return 1;
}

