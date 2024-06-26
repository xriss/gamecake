/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss@XIXs.com 2013
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
#include "all.h"



#define UPVALUE_LIB 1
#define UPVALUE_PTR 2
#define UPVALUE_TAB 3

void lua_grd_tab_openlib (lua_State *l, int upvalues);



/*----------------------------------------------------------------------------------------------------------------------------*/
//
// str def lookups
//
/*----------------------------------------------------------------------------------------------------------------------------*/




struct strenum
{
	const char *str;
	s32 num;
};


const char *strenum_find_string(const strenum *se, s32 num)
{
	while( se->str )
	{
		if(se->num==num)
		{
			return se->str;
		}

		se++;
	}

	return 0;
}


s32 strenum_find_num(const strenum *se, const char *str)
{
	while( se->str )
	{
		if(strcmp(se->str,str)==0)
		{
			return se->num;
		}

		se++;
	}

	return 0;
}




#define STRENUM(s) {#s,s},

const strenum GRD_FMT_STRENUM[]=
{
STRENUM(GRD_FMT_NONE)

STRENUM(GRD_FMT_F32_ARGB)
STRENUM(GRD_FMT_U8_BGRA)
STRENUM(GRD_FMT_U8_INDEXED)
STRENUM(GRD_FMT_U8_LUMINANCE)

STRENUM(GRD_FMT_HINT_NO_ALPHA)
STRENUM(GRD_FMT_HINT_ALPHA_1BIT)
STRENUM(GRD_FMT_HINT_ALPHA)
STRENUM(GRD_FMT_HINT_ONLY_ALPHA)

STRENUM(GRD_FMT_MAX)

{0,0}
};

#undef STRENUM



s32 lua_grd_tofmt(lua_State *l,int idx)
{
	if(lua_isstring(l,idx))
	{
		return strenum_find_num( GRD_FMT_STRENUM , lua_tostring(l,idx) );
	}
	else
	{
		return (s32)lua_tonumber(l,idx);
	}
	return 0;
}

void lua_grd_pushfmt(lua_State *l,s32 fmt)
{
	lua_pushstring(l,strenum_find_string(GRD_FMT_STRENUM,fmt));
}



//
// we can use either this string as a string identifier
// or its address as a light userdata identifier, both unique
//
const char *lua_grd_ptr_name="grd*ptr";


// the data pointer we are using

typedef s32 part_ptr ;


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// check that a table at the given index contains a grd object
// return the part_ptr if it does, otherwise return 0
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
part_ptr lua_grd_check (lua_State *l, int idx)
{
part_ptr p;

	lua_pushstring(l, lua_grd_ptr_name );
	lua_gettable(l,idx);

	p=(part_ptr )(*(void **)luaL_checkudata(l,lua_gettop(l),lua_grd_ptr_name));

	lua_pop(l,1);

	return p;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// get userdata from upvalue, no need to test for type
// just error on null
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
part_ptr lua_grd_get_ptr (lua_State *l)
{
part_ptr p;

	p=(part_ptr )(*(void **)lua_touserdata(l,lua_upvalueindex(UPVALUE_PTR)));


	if (p == NULL)
	{
		luaL_error(l, "null pointer in grd usedata" );
	}

	return p;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// fill a table in with the current settings
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_grd_getinfo (lua_State *l, s32 id, int tab)
{
grd_info grd[1];

	grd_getinfo(id,grd);


	lua_pushliteral(l,"format");	lua_grd_pushfmt(l,grd->fmt);		lua_rawset(l,tab);

	lua_pushliteral(l,"width");		lua_pushnumber(l,grd->w);		lua_rawset(l,tab);
	lua_pushliteral(l,"height");	lua_pushnumber(l,grd->h);		lua_rawset(l,tab);
	lua_pushliteral(l,"depth");		lua_pushnumber(l,grd->d);		lua_rawset(l,tab);


	return 0;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// alloc an item, returns table that you can modify and associate extra data with
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_grd_create (lua_State *l)
{
part_ptr *p;
const char *s;

int idx_ptr;
int idx_tab;

	p = (part_ptr *)lua_newuserdata(l, sizeof(part_ptr));
	
	idx_ptr=lua_gettop(l);

	(*p)=0;

	luaL_getmetatable(l, lua_grd_ptr_name);
	lua_setmetatable(l, -2);

	lua_newtable(l);

	idx_tab=lua_gettop(l);

// main lib and userdata are stored as upvalues in the function calls for easy/fast access

	lua_pushvalue(l, lua_upvalueindex(UPVALUE_LIB) ); // get our base table
	lua_pushvalue(l, idx_ptr ); // get our userdata,
	lua_pushvalue(l, idx_tab ); // get our userdata,

	lua_grd_tab_openlib(l,3);

// remember the userdata in the table as well as the upvalue

	lua_pushstring(l, lua_grd_ptr_name );
	lua_pushvalue(l, idx_ptr ); // get our userdata,
	lua_rawset(l,-3);


	(*p)=0;





	if(lua_istable(l,1))	// duplicate another image
	{
		(*p)=grd_duplicate( lua_grd_check(l,1) );
	}
	else
	if(lua_isnumber(l,2))	// create an image of a given size
	{
	s32 fmt,w,h,d;

		fmt=lua_grd_tofmt(l,1);
		w=(s32)lua_tonumber(l,2);
		h=(s32)lua_tonumber(l,3);
		d=(s32)lua_tonumber(l,4);

		(*p)=grd_create(fmt,w,h,d);
	}
	else
	if(lua_isstring(l,2))	// load an image if a filename is given
	{
	s32 fmt;

		fmt=lua_grd_tofmt(l,1);
		s=lua_tostring(l,2);

		(*p)=grd_load(s,fmt,0);
	}

	lua_grd_getinfo(l,*p,lua_gettop(l));

	lua_remove(l, idx_ptr );
	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// destroy pointer
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_grd_destroy_idx (lua_State *l, int idx)
{
part_ptr *p;
	
	p = (part_ptr *)luaL_checkudata(l, idx, lua_grd_ptr_name);

	if(*p)
	{
		grd_free(*p);
	}
	(*p)=0;

	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// __GC for ptr
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_grd_destroy_ptr (lua_State *l)
{
	return lua_grd_destroy_idx(l,1);
}
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// delete the pointer data and set pointer to 0
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_grd_destroy (lua_State *l)
{
	return lua_grd_destroy_idx(l,lua_upvalueindex(UPVALUE_PTR));
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// reset the image so all data is lost, resizing it if a new format and size is given w,h,d
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_grd_reset (lua_State *l)
{
part_ptr *p;
part_ptr new_p;
	
	new_p=0;

	p = (part_ptr *)luaL_checkudata(l, lua_upvalueindex(UPVALUE_PTR) , lua_grd_ptr_name);


	if(lua_isnil(l,1)) // just clear what we have
	{

	}
	else	// perform a resize
	{
	s32 fmt,w,h,d;

		fmt=lua_grd_tofmt(l,2);
		w=(s32)lua_tonumber(l,3);
		h=(s32)lua_tonumber(l,4);
		d=(s32)lua_tonumber(l,5);

		new_p=grd_create(fmt,w,h,d);
	}

	if(new_p!=0)
	{
		if(*p!=0) // free old
		{
			lua_grd_destroy(l);
		}

		(*p)=new_p;
	}
	lua_grd_getinfo(l,*p,lua_upvalueindex(UPVALUE_TAB));

	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// load an image
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_grd_load (lua_State *l)
{
part_ptr *p;
part_ptr new_p;
	
	new_p=0;

	p = (part_ptr *)luaL_checkudata(l, lua_upvalueindex(UPVALUE_PTR) , lua_grd_ptr_name);

const char *s;
s32 fmt;

	fmt=lua_grd_tofmt(l,2);
	s=lua_tostring(l,3);

	new_p=grd_load(s,fmt,0);


	if(new_p!=0)
	{
		if(*p!=0) // free old
		{
			lua_grd_destroy(l);
		}

		(*p)=new_p;
	}
	lua_grd_getinfo(l,*p,lua_upvalueindex(UPVALUE_TAB));


	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// save an image
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_grd_save (lua_State *l)
{
part_ptr p;
const char *s;

	p=lua_grd_get_ptr(l);

	s=lua_tostring(l,2);

	if(!s)
	{
		luaL_error(l, "no file name specified" );
	}
	grd_save(p,s);


	return 0;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_grd_convert (lua_State *l)
{
part_ptr p;
s32 fmt;

	p=lua_grd_get_ptr(l);

	fmt=lua_grd_tofmt(l,2);

	grd_convert(p,fmt);

	lua_grd_getinfo(l,p,1);

	return 0;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_grd_conscale (lua_State *l)
{
part_ptr p;
f32 base;
f32 scale;

	p=lua_grd_get_ptr(l);

	base=(f32)lua_tonumber(l,2);
	scale=(f32)lua_tonumber(l,3);

	grd_conscale(p,base,scale);

	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_grd_scale (lua_State *l)
{
part_ptr p;
s32 w,h,d;

	p=lua_grd_get_ptr(l);

	w=(s32)lua_tonumber(l,2);
	h=(s32)lua_tonumber(l,3);
	d=(s32)lua_tonumber(l,4);

	grd_scale(p,w,h,d);

	lua_grd_getinfo(l,p,1);

	return 0;
}



/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// used to get/set pixel talbe values
//
// works with palettes of bitmaps
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void lua_grd_pix(lua_State *l , s32 tab_idx , grd_info *grd , s32 x, s32 y, s32 z , s32 w, s32 h, s32 d )
{
s32 xi,yi,zi;

u8* datu8;

s32 idx;

bool read_tab;

	idx=1;

// snap x,y,z w,h,d to available pixels


	if(x<0)			{	x=0;			}
	if(x>grd->w-1)	{	x=grd->w-1;		}
	if(y<0)			{	y=0;			}
	if(y>grd->h-1)	{	y=grd->h-1;		}
	if(z<0)			{	z=0;			}
	if(z>grd->d-1)	{	z=grd->d-1;		}

	if(x+w>grd->w)	{	w=grd->w-x;		}
	if(y+h>grd->h)	{	h=grd->h-y;		}
	if(z+d>grd->d)	{	d=grd->d-z;		}


	if(tab_idx==0) // just fill in, dont read
	{
		lua_newtable(l);
		tab_idx=lua_gettop(l);
		read_tab=false;
	}
	else
	{
		read_tab=true;
	}


	if( (grd->fmt==GRD_FMT_U8_BGRA) )
	{
		for( zi=z ; zi<z+d ; zi++ )
		{
			for( yi=y ; yi<y+h ; yi++ )
			{
				datu8=grd->get_data(x,yi,zi);

				for( xi=x ; xi<x+w ; xi++ )
				{

					if(read_tab)
					{
					float t;

						lua_pushnumber(l,idx+0);
						lua_rawget(l,tab_idx);
						t=(float)lua_tonumber(l,-1);
						if(t>255) { t=255; }	if(t<0)   { t=0; }
						datu8[3]=(u8)t;
						lua_pop(l,1);

						lua_pushnumber(l,idx+1);
						lua_rawget(l,tab_idx);
						t=(float)lua_tonumber(l,-1);
						if(t>255) { t=255; }	if(t<0)   { t=0; }
						datu8[2]=(u8)t;
						lua_pop(l,1);

						lua_pushnumber(l,idx+2);
						lua_rawget(l,tab_idx);
						t=(float)lua_tonumber(l,-1);
						if(t>255) { t=255; }	if(t<0)   { t=0; }
						datu8[1]=(u8)t;
						lua_pop(l,1);

						lua_pushnumber(l,idx+3);
						lua_rawget(l,tab_idx);
						t=(float)lua_tonumber(l,-1);
						if(t>255) { t=255; }	if(t<0)   { t=0; }
						datu8[0]=(u8)t;
						lua_pop(l,1);

					}
					else
					{
						lua_pushnumber(l,idx+0);
						lua_pushnumber(l,datu8[3]);
						lua_rawset(l,tab_idx);

						lua_pushnumber(l,idx+1);
						lua_pushnumber(l,datu8[2]);
						lua_rawset(l,tab_idx);

						lua_pushnumber(l,idx+2);
						lua_pushnumber(l,datu8[1]);
						lua_rawset(l,tab_idx);

						lua_pushnumber(l,idx+3);
						lua_pushnumber(l,datu8[0]);
						lua_rawset(l,tab_idx);
					}

					datu8+=4;
					idx+=4;
				}
			}
		}
	}
	else
	if( (grd->fmt==GRD_FMT_U8_RGB) )
	{
		for( zi=z ; zi<z+d ; zi++ )
		{
			for( yi=y ; yi<y+h ; yi++ )
			{
				datu8=grd->get_data(x,yi,zi);

				for( xi=x ; xi<x+w ; xi++ )
				{

					if(read_tab)
					{
					float t;

						lua_pushnumber(l,idx+1);
						lua_rawget(l,tab_idx);
						t=(float)lua_tonumber(l,-1);
						if(t>255) { t=255; }	if(t<0)   { t=0; }
						datu8[0]=(u8)t;
						lua_pop(l,1);

						lua_pushnumber(l,idx+2);
						lua_rawget(l,tab_idx);
						t=(float)lua_tonumber(l,-1);
						if(t>255) { t=255; }	if(t<0)   { t=0; }
						datu8[1]=(u8)t;
						lua_pop(l,1);

						lua_pushnumber(l,idx+3);
						lua_rawget(l,tab_idx);
						t=(float)lua_tonumber(l,-1);
						if(t>255) { t=255; }	if(t<0)   { t=0; }
						datu8[2]=(u8)t;
						lua_pop(l,1);
					}
					else
					{
						lua_pushnumber(l,idx+0);
						lua_pushnumber(l,255);
						lua_rawset(l,tab_idx);

						lua_pushnumber(l,idx+1);
						lua_pushnumber(l,datu8[0]);
						lua_rawset(l,tab_idx);

						lua_pushnumber(l,idx+2);
						lua_pushnumber(l,datu8[1]);
						lua_rawset(l,tab_idx);

						lua_pushnumber(l,idx+3);
						lua_pushnumber(l,datu8[2]);
						lua_rawset(l,tab_idx);
					}

					datu8+=3;
					idx+=4;
				}
			}
		}
	}
	else
	if( (grd->fmt==GRD_FMT_U8_INDEXED) ||  (grd->fmt==GRD_FMT_U8_LUMINANCE) )
	{
		for( zi=z ; zi<z+d ; zi++ )
		{
			for( yi=y ; yi<y+h ; yi++ )
			{
				datu8=grd->get_data(x,yi,zi);

				for( xi=x ; xi<x+w ; xi++ )
				{
					if(read_tab)
					{
					float t;

						lua_pushnumber(l,idx+0);
						lua_rawget(l,tab_idx);
						t=(float)lua_tonumber(l,-1);
						if(t>255) { t=255; }	if(t<0)   { t=0; }
						datu8[0]=(u8)t;
						lua_pop(l,1);
					}
					else
					{
						lua_pushnumber(l,idx);
						lua_pushnumber(l,datu8[0]);
						lua_rawset(l,tab_idx);
					}
					datu8+=1;
					idx+=1;
				}
			}
		}
	}
	else
	{
		luaL_error(l, "unsuported format" );
	}

}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// return palette in a table
//
// possibly requests are
//
// x,w == start , numof
//
// the palette is represented by a 1d bitmap
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_grd_palette (lua_State *l)
{
part_ptr p;

grd_info grd[1];

s32 x;
s32 w;

	p=lua_grd_get_ptr(l);

	grd_getpalinfo(p,grd);

	x=(s32)lua_tonumber(l,2);
	w= (s32)lua_tonumber(l,3);

	if(x<0)			{	luaL_error(l, "x<0" );		}
	if(x>grd->w-1)	{	luaL_error(l, "x>width" );	}
	if(x+w>grd->w)	{	luaL_error(l, "w>width" );	}

	if(lua_istable(l,4))
	{
		lua_grd_pix(l,4,grd,x,0,0,w,1,1);
		return 0;
	}
	else
	{
		lua_grd_pix(l,0,grd,x,0,0,w,1,1);
		return 1;
	}

}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// return pixels in a table
//
// possibly requests are
//
// x,y,z , w,h,d
// x,y,0 , w,h,1
// x,y,0 , 1,1,1
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_grd_pixels (lua_State *l)
{
part_ptr p;

grd_info grd[1];


s32 x,y,z;
s32 w,h,d;
s32 tab_idx;

	p=lua_grd_get_ptr(l);

	grd_getinfo(p,grd);

	tab_idx=0;

	if( lua_isnumber(l,7) )
	{
		x=(s32)lua_tonumber(l,2);
		y=(s32)lua_tonumber(l,3);
		z=(s32)lua_tonumber(l,4);

		w=(s32)lua_tonumber(l,5);
		h=(s32)lua_tonumber(l,6);
		d=(s32)lua_tonumber(l,7);
		if(lua_istable(l,8)) { tab_idx=8; }
	}
	else
	if( lua_isnumber(l,5) )
	{
		x=(s32)lua_tonumber(l,2);
		y=(s32)lua_tonumber(l,3);
		z=0;

		w=(s32)lua_tonumber(l,4);
		h=(s32)lua_tonumber(l,5);
		d=1;
		if(lua_istable(l,6)) { tab_idx=6; }
	}
	else
	{
		x=(s32)lua_tonumber(l,2);
		y=(s32)lua_tonumber(l,3);
		z=0;

		w=1;
		h=1;
		d=1;
		if(lua_istable(l,4)) { tab_idx=4; }
	}


// error on out of range


	if(x<0)			{	luaL_error(l, "x<0" );		}
	if(x>grd->w-1)	{	luaL_error(l, "x>width" );	}
	if(y<0)			{	luaL_error(l, "y<0" );		}
	if(y>grd->h-1)	{	luaL_error(l, "y>height" );	}
	if(z<0)			{	luaL_error(l, "z<0" );		}
	if(z>grd->d-1)	{	luaL_error(l, "z>depth" );	}

	if(x+w>grd->w)	{	luaL_error(l, "w>width" );	}
	if(y+h>grd->h)	{	luaL_error(l, "h>height" );	}
	if(z+d>grd->d)	{	luaL_error(l, "d>depth" );	}


	lua_grd_pix(l,tab_idx,grd,x,y,z,w,h,d);

	if(tab_idx) { return 0; }
	else		{ return 1; }
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// call open lib with our functions
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void lua_grd_openlib (lua_State *l, int upvalues)
{
	const luaL_reg lib[] =
	{
		{	"create"		,	lua_grd_create	},

		{0,0}
	};
	luaL_openlib(l, NULL, lib, upvalues);
};


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// call open lib with our ptr functions
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void lua_grd_ptr_openlib (lua_State *l, int upvalues)
{
const luaL_reg lib[] =
	{
		{"__gc",			lua_grd_destroy_ptr},

		{0,0}
	};
	luaL_openlib(l, NULL, lib, upvalues);
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// call open lib with our tab functions
//
// all functions expect the self table to be passed in as arg1
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void lua_grd_tab_openlib (lua_State *l, int upvalues)
{
const luaL_reg lib[] =
	{
		{"destroy",			lua_grd_destroy},


		{"reset",			lua_grd_reset},
		{"load",			lua_grd_load},
		{"save",			lua_grd_save},

		{"convert",			lua_grd_convert},

		{"pixels",			lua_grd_pixels},
		{"palette",			lua_grd_palette},

		{"conscale",		lua_grd_conscale},

		{"scale",			lua_grd_scale},
			
//		{	"unref"					,	lua_grd_unref},

		{0,0}
	};
	luaL_openlib(l, NULL, lib, upvalues);
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// open library.
//
/*+-----------------------------------------------------------------------------------------------------------------+*/


int luaopen_grd (lua_State *l)
{


	luaL_newmetatable(l, lua_grd_ptr_name);
	lua_grd_ptr_openlib(l,0);
	lua_pop(l,1);

	lua_pushstring(l, LUA_grd_LIB_NAME );
	lua_newtable(l);
	lua_pushvalue(l, -1); // have this table as the first up value
	lua_grd_openlib(l,1);
	lua_rawset(l, LUA_GLOBALSINDEX);


	return 1;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// close library.
//
/*+-----------------------------------------------------------------------------------------------------------------+*/

int luaclose_grd (lua_State *l)
{
	lua_pushstring(l, LUA_grd_LIB_NAME);
	lua_pushnil(l);
	lua_rawset(l, LUA_GLOBALSINDEX);


	return 0;
}
