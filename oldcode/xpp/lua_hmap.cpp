/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss@XIXs.com 2013
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
#include "all.h"


/*----------------------------------------------------------------------------------------------------------------------------*/
//
// allocate and free mmaps
//
/*----------------------------------------------------------------------------------------------------------------------------*/

//
// check meta table is valid and get data
//
heightmap *lua_hmap_to_hmap (lua_State *L, int index)
{
heightmap *p;
	
	p = (heightmap *)luaL_checkudata(L, index, HMAPHANDLE);

	if (p == NULL)
	{
		luaL_argerror(L, index, "bad " HMAPHANDLE );
	}

	return p;
}


static int lua_hmap_alloc (lua_State *l)
{
heightmap *p;
	
	p = (heightmap *)lua_newuserdata(l, sizeof(heightmap));

	memset(p,0,sizeof(heightmap));

	hmap_setup(p);

	luaL_getmetatable(l, HMAPHANDLE);
	lua_setmetatable(l, -2);

	return 1;
}

static int lua_hmap_free (lua_State *l)
{
heightmap *p;
	p=lua_hmap_to_hmap(l,1);

	hmap_clean(p);

	return 0;
}


/*----------------------------------------------------------------------------------------------------------------------------*/
//
// build a flat map of given size
//
/*----------------------------------------------------------------------------------------------------------------------------*/
static int lua_hmap_build_from_mmap (lua_State *l)
{
heightmap *h;
metamap *m;
f32 z;

	h=lua_hmap_to_hmap(l,1);
	m=lua_mmap_to_mmap_from_table(l,2);
	z=(f32)lua_tonumber(l,3);

	hmap_build_from_mmap(h,m,z);

	return 0;
}

/*----------------------------------------------------------------------------------------------------------------------------*/
//
// build a flat map of given size
//
/*----------------------------------------------------------------------------------------------------------------------------*/
static int lua_hmap_build (lua_State *l)
{
heightmap *p;
s32 w,h;

	p=lua_hmap_to_hmap(l,1);
	w=(s32)lua_tonumber(l,2);
	h=(s32)lua_tonumber(l,3);

	hmap_build(p,w,h);

	return 0;
}

/*----------------------------------------------------------------------------------------------------------------------------*/
//
// get / set height at given x,y position
//
/*----------------------------------------------------------------------------------------------------------------------------*/
static int lua_hmap_get (lua_State *l)
{
heightmap *p;
s32 x,y,idx;

	p=lua_hmap_to_hmap(l,1);
	x=(s32)lua_tonumber(l,2);
	y=(s32)lua_tonumber(l,3);

	idx=x+y*p->tw;
	if( (idx>=p->numof_tiles) || (!p->tiles) )
	{
		luaL_error(l, "requested height out of range" );
	}

	lua_pushnumber(l,p->tiles[idx].height);

	return 1;
}

static int lua_hmap_set (lua_State *l)
{
heightmap *p;
s32 x,y,idx;
	p=lua_hmap_to_hmap(l,1);
	x=(s32)lua_tonumber(l,2);
	y=(s32)lua_tonumber(l,3);

	idx=x+y*p->tw;
	if( (idx>=p->numof_tiles) || (!p->tiles) )
	{
		luaL_error(l, "requested height out of range" );
	}

	p->tiles[idx].height=(f32)lua_tonumber(l,4);

	return 0;
}

/*----------------------------------------------------------------------------------------------------------------------------*/
//
// draw into devil image
//
/*----------------------------------------------------------------------------------------------------------------------------*/
static int lua_hmap_draw (lua_State *l)
{
heightmap *p;
devilhandle *d;

	p=lua_hmap_to_hmap(l,1);
	d=lua_devil_to_image(l,2);

	lua_devil_bind_imagep(d);
	lua_devil_error_check(l);

	if( ! hmap_draw(p) ) 
	{
		luaL_error(l, "couldn't build heightmap" );
	}

	lua_devil_error_check(l);

	return 0;
}

/*----------------------------------------------------------------------------------------------------------------------------*/
//
// library defs
//
/*----------------------------------------------------------------------------------------------------------------------------*/

static const luaL_reg lua_hmap_lib[] =
{
	
	{	"alloc",		lua_hmap_alloc	},
	{	"free",			lua_hmap_free	},
	
	{NULL, NULL}
};

// suport for one type of object

static const luaL_reg lua_hmap_meta[] =
{

	{"build",				lua_hmap_build},
	{"build_from_mmap",		lua_hmap_build_from_mmap},

	{"get",					lua_hmap_get},
	{"set",					lua_hmap_set},

	{"draw",				lua_hmap_draw},

	{"free",				lua_hmap_free},
	{"__gc",				lua_hmap_free},

	{NULL, NULL}
};

static void lua_hmap_meta_create (lua_State *l)
{
	luaL_newmetatable(l, HMAPHANDLE);  /* create new metatable */

	lua_pushliteral(l, "__index");
	lua_pushvalue(l, -2);  /* push metatable */
	lua_rawset(l, -3);  /* metatable.__index = metatable */

	luaL_openlib(l, NULL, lua_hmap_meta, 0);
}


/*----------------------------------------------------------------------------------------------------------------------------*/
//
// open library.
//
/*----------------------------------------------------------------------------------------------------------------------------*/

int luaopen_hmap (lua_State *l)
{
	lua_hmap_meta_create(l);

	lua_pushstring(l, LUA_HMAPLIBNAME );
	lua_newtable(l);
	luaL_openlib(l, 0, lua_hmap_lib, 0);
	lua_rawset(l, LUA_GLOBALSINDEX);

	return 1;
}


