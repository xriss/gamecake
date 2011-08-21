/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss 2011 http://xixs.com
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
#include "all.h"



#define UPVALUE_LIB 1
#define UPVALUE_PTR 2
#define UPVALUE_TAB 3

void lua_freetype_tab_openlib (lua_State *l, int upvalues);



//
// we can use either this string as a string identifier
// or its address as a light userdata identifier, both unique
//
const char *lua_freetype_ptr_name="freetype*ptr";


// the data pointer we are using

typedef struct lua_freetype_font part_struct ;
typedef part_struct * part_ptr ;


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// check that a table at the given index contains a grd object
// return the part_ptr if it does, otherwise return 0
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
part_ptr lua_freetype_check (lua_State *l, int idx)
{
part_ptr p;

	lua_pushstring(l, lua_freetype_ptr_name );
	lua_gettable(l,idx);

	p=(part_ptr )(*(void **)luaL_checkudata(l,lua_gettop(l),lua_freetype_ptr_name));

	lua_pop(l,1);

	return p;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// get userdata from upvalue, no need to test for type
// just error on null
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
part_ptr lua_freetype_get_ptr (lua_State *l)
{
part_ptr p;

	p=(part_ptr )(*(void **)lua_touserdata(l,lua_upvalueindex(UPVALUE_PTR)));


	if (p == 0)
	{
		luaL_error(l, "null pointer in freetype usedata" );
	}

	return p;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// fill a table in with the current settings
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_freetype_getinfo (lua_State *l, part_ptr p, int tab)
{
	if(p)
	{
/*
		lua_pushliteral(l,"format");	lua_freetype_pushfmt(l,p->bmap->fmt);		lua_rawset(l,tab);

		lua_pushliteral(l,"width");		lua_pushnumber(l,p->bmap->w);		lua_rawset(l,tab);
		lua_pushliteral(l,"height");	lua_pushnumber(l,p->bmap->h);		lua_rawset(l,tab);
		lua_pushliteral(l,"depth");		lua_pushnumber(l,p->bmap->d);		lua_rawset(l,tab);

		lua_pushliteral(l,"err");
		if(p->err) 	{ lua_pushstring(l,p->err); }
		else		{ lua_pushnil(l); }
		lua_rawset(l,tab);
*/
	}
	else
	{
		lua_pushliteral(l,"err"); lua_pushstring(l,"unbound freetype"); lua_rawset(l,tab);
	}

	return 0;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// alloc an item, returns table that you can modify and associate extra data with
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_freetype_create (lua_State *l)
{
part_ptr *p;
const char *s;

int idx_ptr;
int idx_tab;

	p = (part_ptr *)lua_newuserdata(l, sizeof(part_ptr));
	
	idx_ptr=lua_gettop(l);

	(*p)=0;

	luaL_getmetatable(l, lua_freetype_ptr_name);
	lua_setmetatable(l, -2);

	lua_newtable(l);

	idx_tab=lua_gettop(l);

// main lib and userdata are stored as upvalues in the function calls for easy/fast access

	lua_pushvalue(l, lua_upvalueindex(UPVALUE_LIB) ); // get our base table
	lua_pushvalue(l, idx_ptr ); // get our userdata,
	lua_pushvalue(l, idx_tab ); // get our userdata,

	lua_freetype_tab_openlib(l,3);

// remember the userdata in the table as well as the upvalue

	lua_pushstring(l, lua_freetype_ptr_name );
	lua_pushvalue(l, idx_ptr ); // get our userdata,
	lua_rawset(l,-3);


	(*p)=0;


	(*p)=(part_ptr)calloc(sizeof(part_struct),1);
	
	(*p)->error = FT_Init_FreeType( &(*p)->library );
	if( !(*p)->error )
	{
		s=lua_tostring(l,1); // the file name of the font to open
		(*p)->error = FT_New_Face( (*p)->library,
			s,
			0,
			&(*p)->face );
	}

	
	lua_freetype_getinfo(l,*p,lua_gettop(l));

	lua_remove(l, idx_ptr );
	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// destroy pointer
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_freetype_destroy_idx (lua_State *l, int idx)
{
part_ptr *p;
	
	p = (part_ptr *)luaL_checkudata(l, idx, lua_freetype_ptr_name);

	if(*p)
	{
		free(*p);
	}
	(*p)=0;

	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// __GC for ptr
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_freetype_destroy_ptr (lua_State *l)
{
	return lua_freetype_destroy_idx(l,1);
}
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// delete the pointer data and set pointer to 0
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_freetype_destroy (lua_State *l)
{
	return lua_freetype_destroy_idx(l,lua_upvalueindex(UPVALUE_PTR));
}



/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// test function
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_freetype_test (lua_State *l)
{
int error=0;
	FT_Library  library;   /* handle to library     */
	FT_Face     face;      /* handle to face object */
const char * s;

	s=lua_tostring(l,1);

	error = FT_Init_FreeType( &library );
	if( !error )
	{
		error = FT_New_Face( library,
			s,
			0,
			&face );
	}

	lua_pushnumber(l, error);
	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// call open lib with our ptr functions
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void lua_freetype_ptr_openlib (lua_State *l, int upvalues)
{
const luaL_reg lib[] =
	{
		{"__gc",			lua_freetype_destroy_ptr},

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
void lua_freetype_tab_openlib (lua_State *l, int upvalues)
{
const luaL_reg lib[] =
	{
		{"destroy",			lua_freetype_destroy},

		
//		{	"unref"					,	lua_grd_unref},

		{0,0}
	};
	luaL_openlib(l, NULL, lib, upvalues);
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// call open lib with our functions
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
void lua_freetype_openlib (lua_State *l, int upvalues)
{
	const luaL_reg lib[] =
	{
		{	"test"		,	lua_freetype_test	},
		{	"create"	,	lua_freetype_create	},

		{0,0}
	};
	luaL_openlib(l, NULL, lib, upvalues);
};


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// open library.
//
/*+-----------------------------------------------------------------------------------------------------------------+*/


int luaopen_freetype (lua_State *l)
{


	luaL_newmetatable(l, lua_freetype_ptr_name);
	lua_freetype_ptr_openlib(l,0);
	lua_pop(l,1);

	lua_newtable(l);
	lua_pushstring(l, LUA_freetype_LIB_NAME );
	lua_pushvalue(l, -2); // have this table as the first up value?
	lua_pushvalue(l, -1); // and we need to save one to return
	lua_freetype_openlib(l,1);
	lua_rawset(l, LUA_GLOBALSINDEX);

	return 1;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// close library.
//
/*+-----------------------------------------------------------------------------------------------------------------+*/

int luaclose_freetype (lua_State *l)
{
	lua_pushstring(l, LUA_freetype_LIB_NAME);
	lua_pushnil(l);
	lua_rawset(l, LUA_GLOBALSINDEX);

	return 0;
}
