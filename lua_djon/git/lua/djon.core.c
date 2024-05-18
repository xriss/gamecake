
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#define DJON_C 1
#include "djon.h"




/*

We can use this string as a string identifier or its address as a light 
userdata identifier. Both are unique values.

*/
const char *lua_djon_ptr_name="djon*ptr";


/*

check that a userdata at the given index is a djon object
return the djon_state ** if it does, otherwise return 0

*/
static djon_state ** lua_djon_get_ptr (lua_State *l, int idx)
{
djon_state **ptrptr=0;

	ptrptr = lua_touserdata(l, idx);

	if(ptrptr)
	{
		if( lua_getmetatable(l, idx) )
		{
			luaL_getmetatable(l, lua_djon_ptr_name);
			if( !lua_rawequal(l, -1, -2) )
			{
				ptrptr = 0;
			}
			lua_pop(l, 2);
			return ptrptr;
		}
	}

	return ptrptr;
}


/*

call lua_djon_get_ptr and raise an error on null ptr or *ptr then return *ptr

*/
static djon_state * lua_djon_check_ptr (lua_State *l, int idx)
{
djon_state **ptrptr=lua_djon_get_ptr(l,idx);

	if(ptrptr == 0)
	{
		luaL_error(l, "not djon userdata" );
	}

	if(*ptrptr == 0)
	{
		luaL_error(l, "null djon userdata" );
	}

	return *ptrptr;
}

/*

alloc a ptr

*/
static djon_state ** lua_djon_alloc_ptr(lua_State *l)
{
djon_state **ptrptr;
	ptrptr = (djon_state **)lua_newuserdata(l, sizeof(djon_state *));
	(*ptrptr)=0;
	luaL_getmetatable(l, lua_djon_ptr_name);
	lua_setmetatable(l, -2);
	return ptrptr;
}

/*

free pointer at given index

*/
static int lua_djon_free_ptr (lua_State *l, int idx)
{
djon_state **ptrptr=lua_djon_get_ptr(l,idx);

	if(ptrptr)
	{
		if(*ptrptr)
		{
			djon_clean(*ptrptr);
		}
		(*ptrptr)=0;
	}
	return 0;
}




/*

allocate and setup a djon state

*/
static int lua_djon_setup (lua_State *l)
{
djon_state **ptrptr;
const char *s;
const char *opts=0;

	ptrptr=lua_djon_alloc_ptr(l);

	(*ptrptr)=djon_setup();

	return 1;
}

/*

clean and free a djon state

*/
static int lua_djon_clean (lua_State *l)
{
	lua_djon_free_ptr(l, 1);
	return 0;
}

/*

Return the current parse location ( error location )

*/
static int lua_djon_location (lua_State *l)
{
djon_state *ds=lua_djon_check_ptr(l,1);

	lua_pushnumber(l,ds->error_line);
	lua_pushnumber(l,ds->error_char);
	lua_pushnumber(l,ds->error_idx);

	return 3;
}




static void lua_djon_get_value (lua_State *l,djon_state *ds,int i)
{
int vi=0;
int ki=0;
djon_value *v=djon_get(ds,i);
int ci;
int cc;
int cb;
int idx=0;

	if(ds->comment)
	{
			lua_newtable(l);
	}

	switch(v->typ&DJON_TYPEMASK)
	{
		case DJON_ARRAY:
			lua_newtable(l);
			idx=0;
			vi=v->val;
			while( vi )
			{
				idx++;
				lua_djon_get_value(l,ds,vi);
				lua_rawseti(l,-2,idx);
				vi=djon_get(ds,vi)->nxt;
			}
		break;
		case DJON_OBJECT:
			lua_newtable(l);
			ki=v->key;
			while( ki )
			{
				lua_pushlstring(l,djon_get(ds,ki)->str,djon_get(ds,ki)->len);
				lua_djon_get_value(l,ds,djon_get(ds,ki)->val);
				if(ds->comment)
				{
					// replace value comments with key and value comments
					for( ci=djon_get(ds,ki)->com , cc=2 ; ci ; ci=djon_get(ds,ci)->com , cc++ )
					{
						v=djon_get(ds,ci);
						lua_pushlstring(l,v->str,v->len);
						lua_rawseti(l,-2,cc);
					}
					cb=cc;
					for( ci=djon_get(ds,djon_get(ds,ki)->val)->com , cc=2 ; ci ; ci=djon_get(ds,ci)->com , cc++ )
					{
						v=djon_get(ds,ci);
						lua_pushlstring(l,v->str,v->len);
						lua_rawseti(l,-2,cc+cb-2);
					}
				}
				lua_rawset(l,-3);
				ki=djon_get(ds,ki)->nxt;
			}
		break;
		case DJON_STRING:
			lua_pushlstring(l,v->str,v->len);
		break;
		case DJON_NUMBER:
			lua_pushnumber(l,v->num);
		break;
		case DJON_BOOL:
			lua_pushboolean(l,v->num);
		break;
		case DJON_NULL:
			lua_pushnumber(l,NAN); // Lua does not have a null so we use NAN ( important for arrays )
		break;
		default:
			lua_pushnil(l);
		break;
	}

	if(ds->comment)
	{
		lua_rawseti(l,-2,1);
		for( ci=djon_get(ds,i)->com , cc=2 ; ci ; ci=djon_get(ds,ci)->com , cc++ )
		{
			v=djon_get(ds,ci);
			lua_pushlstring(l,v->str,v->len);
			lua_rawseti(l,-2,cc);
		}
	}
}

/*

Convert internal data state into lua tables.

*/
static int lua_djon_get (lua_State *l)
{
djon_state *ds=lua_djon_check_ptr(l,1);
	ds->comment=0;
	for(int i=2;i<=10;i++) // check string flags in args
	{
		const char *s=lua_tostring(l,i);
		if(!s){break;}
		if( strcmp(s,"comment")==0 ) { ds->comment=1; }
	}

	lua_djon_get_value(l,ds,ds->parse_value);

	return 1;
}


static int lua_djon_set_value (lua_State *l , djon_state *ds)
{
size_t slen=0;
int i=0;
djon_value *v=0;
int idx=0;
djon_value *lv=0;
djon_value *lk=0;
djon_value *kv=0;
djon_value *cv=0;
int li=0;
int vi=0;
int ki=0;
int ci;
int cc;
int lc;
	
	if(ds->comment)
	{
		if( ! lua_istable(l,-1) ) {luaL_error(l,"values must be tables"); }
		lua_rawgeti(l,-1,1);
	}
	
	if( lua_istable(l,-1) )
	{
		lua_rawgeti(l,-1,1); // check first element of an array
		if(lua_isnil(l,-1)) // if nil then this is an object
		{
			lua_pop(l,1);

			i=djon_alloc(ds); if(!i) { luaL_error(l, "out of memory" ); }
			v=djon_get(ds,i);
			v->typ=DJON_OBJECT;

			lua_pushnil(l);
			while( lua_next(l, -2) != 0)
			{
				if(!lua_isstring(l,-2)) { luaL_error(l, "object keys must be strings" ); }
				ki=djon_alloc(ds); if(!ki) { luaL_error(l, "out of memory" ); }
				kv=djon_get(ds,ki);
				kv->typ=DJON_STRING;
				kv->str=(char*)lua_tolstring(l,-2,&slen);
				kv->len=slen;
				
				vi=lua_djon_set_value(l,ds);
				kv=djon_get(ds,ki); // realloc safe
				kv->val=vi;

				v=djon_get(ds,i); // realloc safe
				if( v->key==0) // first
				{
					v->key=ki;
				}
				else // chain
				{
					djon_get(ds,li)->nxt=ki;
					djon_get(ds,ki)->prv=li;
				}
				li=ki;

				if(ds->comment)
				{
					// move comments to key
					djon_get(ds,ki)->com=djon_get(ds,vi)->com;
					djon_get(ds,vi)->com=0;
				}

				lua_pop(l, 1);
			}

		}
		else
		{
			lua_pop(l,1);

			i=djon_alloc(ds); if(!i) { luaL_error(l, "out of memory" ); }
			v=djon_get(ds,i);
			v->typ=DJON_ARRAY;
			
			for( idx=1 ; 1 ; idx++)
			{
				lua_rawgeti(l,-1,idx);
				if(lua_isnil(l,-1)) { lua_pop(l,1); break; }

				vi=lua_djon_set_value(l,ds);
				lua_pop(l,1);

				v=djon_get(ds,i);
				if( v->val==0) // first
				{
					v->val=vi;
				}
				else // chain
				{
					djon_get(ds,li)->nxt=vi;
					djon_get(ds,vi)->prv=li;
				}
				li=vi;
			}
		}
	}
	else
	if( lua_isnumber(l,-1) )
	{
		i=djon_alloc(ds); if(!i) { luaL_error(l, "out of memory" ); }
		v=djon_get(ds,i);
		v->typ=DJON_NUMBER;
		v->num=lua_tonumber(l,-1);
	}
	else
	if( lua_isstring(l,-1) )
	{
		i=djon_alloc(ds); if(!i) { luaL_error(l, "out of memory" ); }
		v=djon_get(ds,i);
		v->typ=DJON_STRING;
		v->str=(char*)lua_tolstring(l,-1,&slen);
		v->len=slen;
	}
	else
	if( lua_isboolean(l,-1) )
	{
		i=djon_alloc(ds); if(!i) { luaL_error(l, "out of memory" ); }
		v=djon_get(ds,i);
		v->typ=DJON_BOOL;
		v->num=lua_toboolean(l,-1);
	}

	if(ds->comment)
	{
		lua_pop(l, 1);
		for( cc=2 , lc=i ; 1 ; cc++ )
		{
			lua_rawgeti(l,-1,cc);
			if(lua_isnil(l,-1))
			{
				lua_pop(l, 1);
				break;
			}

			if(!lua_isstring(l,-1)) { luaL_error(l, "comments must be strings" ); }
			ci=djon_alloc(ds); if(!ci) { luaL_error(l, "out of memory" ); }
			cv=djon_get(ds,ci);
			cv->typ=DJON_COMMENT;
			cv->str=(char*)lua_tolstring(l,-1,&slen);
			cv->len=slen;
			djon_get(ds,lc)->com=ci;
			lc=ci;

			lua_pop(l, 1);
		}
	}
	
	return i;
}

/*

Convert lua tables into internal data state.

Going to share string pointers from the lua strings here, so, be 
careful not to free the data before you write it.

*/
static int lua_djon_set (lua_State *l)
{
djon_state *ds=lua_djon_check_ptr(l,1);
	ds->comment=0;
	for(int i=3;i<=10;i++) // check string flags in args
	{
		const char *s=lua_tostring(l,i);
		if(!s){break;}
		if( strcmp(s,"comment")==0 ) { ds->comment=1; }
	}
	
	ds->write_data=0; // prepare to alloc strings

	lua_pushvalue(l,2);
	ds->parse_value=lua_djon_set_value(l,ds);
	lua_pop(l,1);

	return 0;
}


/*

Load internal data state from a json/djon string.

*/
static int lua_djon_load (lua_State *l)
{
djon_state *ds=lua_djon_check_ptr(l,1);
size_t len=0;
const char *str=lua_tolstring(l,2,&len);
char *data;

	ds->strict=0;
	for(int i=3;i<=10;i++) // check string flags in args
	{
		const char *s=lua_tostring(l,i);
		if(!s){break;}
		if( strcmp(s,"strict")==0 ) { ds->strict=1; }
	}

	if(*str == 0)
	{
		luaL_error(l, "djon string required" );
	}
	
	data = DJON_MEM_MALLOC(ds,len+1); if(!data) { luaL_error(l, "out of memory" ); }
	
	memcpy(data,str,len+1); // includes null term

	ds->data=data; // leave the data here and it will get freed when we clean
	ds->data_len=len;
	djon_parse(ds);

	if( ds->error_string ){ luaL_error(l, ds->error_string ); }

	return 0;
}


/*

Save internal data state into a json/djon string.

*/
static int lua_djon_save (lua_State *l)
{
djon_state *ds=lua_djon_check_ptr(l,1);
int write_djon=0;
djon_value *v=0;

	ds->write=&djon_write_data; // we want to write to a string
	ds->write_data=0; //  and reset

	ds->compact=0;
	ds->strict=0;

	for(int i=2;i<=10;i++) // check string flags in args
	{
		const char *s=lua_tostring(l,i);
		if(!s){break;}
		if( strcmp(s,"djon")==0 ) { write_djon=1; }
		if( strcmp(s,"compact")==0 ) { ds->compact=1; }
		if( strcmp(s,"strict")==0 ) { ds->strict=1; }
	}
	
	if(write_djon)
	{
		djon_write_djon(ds,ds->parse_value);
	}
	else
	{
		djon_write_json(ds,ds->parse_value);
	}
	
	lua_pushlstring(l,ds->write_data,ds->write_len); // return string we wrote to
	free(ds->write_data); // and free buffer
	ds->write_data=0;

	return 1;
}

LUALIB_API int luaopen_djon_core (lua_State *l)
{
	const luaL_Reg lib[] =
	{
		{"setup",					lua_djon_setup},
		{"clean",					lua_djon_clean},

		{"location",				lua_djon_location},

		{"get",						lua_djon_get},
		{"set",						lua_djon_set},

		{"load",					lua_djon_load},
		{"save",					lua_djon_save},
		{0,0}
	};

	const luaL_Reg meta[] =
	{
		{"__gc",			lua_djon_clean},
		{0,0}
	};

	luaL_newmetatable(l, lua_djon_ptr_name);
	
#if defined(LUA_VERSION_NUM) && LUA_VERSION_NUM >= 520
	luaL_setfuncs(l, meta, 0);
#else
	luaL_openlib(l, NULL, meta, 0);
#endif

	lua_pop(l,1);


	lua_newtable(l);

#if defined(LUA_VERSION_NUM) && LUA_VERSION_NUM >= 520
	luaL_setfuncs(l, lib, 0);
#else
	luaL_openlib(l, NULL, lib, 0);
#endif

	return 1;
}

