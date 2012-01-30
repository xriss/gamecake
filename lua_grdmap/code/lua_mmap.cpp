/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss Daniels 2003 http://www.XIXs.com
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
metamap *lua_mmap_to_mmap (lua_State *L, int index)
{
#if 0
metamap *p;
	
	p = (metamap *)luaL_checkudata(L, index, MMAPHANDLE);

	if (p == NULL)
	{
		luaL_argerror(L, index, "bad " MMAPHANDLE );
	}

	return p;
#endif
	return 0;
}

metamap *lua_mmap_to_mmap_from_table (lua_State *l, int index)
{
#if 0

metamap *p;
devilhandle *ip;

	lua_pushliteral(l,MMAPHANDLE); // get our data within the table
	lua_gettable(l, index);

	p = (metamap *)luaL_checkudata(l, lua_gettop(l), MMAPHANDLE);

	if (p == NULL)
	{
		luaL_argerror(l, index, "bad " MMAPHANDLE );
	}

	lua_pop(l,1);

	p->image=0;

	lua_pushliteral(l,"image"); // get our image within the table;
	lua_gettable(l, index);
	if(!lua_isnil(l,-1))
	{
		ip=lua_devil_to_imagep(l,lua_gettop(l));
		p->image=ip->image; // sync to lua image pointer
	}
	lua_pop(l,1);

	if(p->image==0) // try grd instead
	{
		lua_pushliteral(l,"grd"); // get our image within the table;
		lua_gettable(l, index);
		if(!lua_isnil(l,-1))
		{
			p->image=lua_grd_check(l,lua_gettop(l));
		}
		lua_pop(l,1);
	}

	return p;
#endif
	return 0;
}


static int lua_mmap_alloc (lua_State *l)
{
#if 0
metamap *p;


	lua_newtable(l);
	luaL_getmetatable(l, MMAPTHANDLE);
	lua_setmetatable(l, -2);

	lua_pushliteral(l,MMAPHANDLE); // set mmaphandle into the table we return,

	p = (metamap *)lua_newuserdata(l, sizeof(metamap));

	memset(p,0,sizeof(metamap));

	mmap_setup(p);

	luaL_getmetatable(l, MMAPHANDLE);
	lua_setmetatable(l, -2);

	lua_settable(l, -3);

	return 1;
#endif
	return true;
}

static int lua_mmap_free_mmap (lua_State *l)
{
#if 0
metamap *p;
	p=lua_mmap_to_mmap(l,1);

	mmap_clean(p);

	return 0;
#endif
	return 0;
}

static int lua_mmap_free (lua_State *l)
{
#if 0
metamap *p;

	lua_pushliteral(l,MMAPHANDLE); // get our data within the table
	lua_gettable(l, 1);
	p=lua_mmap_to_mmap(l,lua_gettop(l));
	lua_pop(l,1);

	mmap_clean(p);

	return 0;
#endif
	return 0;
}

/*----------------------------------------------------------------------------------------------------------------------------*/
//
// cut image into bits
//
/*----------------------------------------------------------------------------------------------------------------------------*/

//
// chop into base chars
//
static int lua_mmap_cutup (lua_State *l)
{
#if 0
metamap *p;

s32 h,w;

	p=lua_mmap_to_mmap_from_table(l,1);

	w=(s32)lua_tonumber(l,2);
	h=(s32)lua_tonumber(l,3);

	mmap_cutup(p,w,h);

	return 0;
#endif
	return 0;
}


/*----------------------------------------------------------------------------------------------------------------------------*/
//
// merge chars so as to remove dupes
//
/*----------------------------------------------------------------------------------------------------------------------------*/
static int lua_mmap_merge (lua_State *l)
{
#if 0
metamap *p;

	p=lua_mmap_to_mmap_from_table(l,1);

	mmap_merge(p);

	return 0;
#endif
	return 0;
}

/*----------------------------------------------------------------------------------------------------------------------------*/
//
// shrink each char so it only contains non transparent pixels
//
/*----------------------------------------------------------------------------------------------------------------------------*/
static int lua_mmap_shrink (lua_State *l)
{
#if 0
metamap *p;

	p=lua_mmap_to_mmap_from_table(l,1);

	mmap_shrink(p);

	return 0;
#endif
	return 0;
}

/*----------------------------------------------------------------------------------------------------------------------------*/
//
// map tiles in one bitmpa into tiles in another bitmap
//
/*----------------------------------------------------------------------------------------------------------------------------*/
static int lua_mmap_keymap (lua_State *l)
{
#if 0
metamap *pa;
metamap *pb;

	pa=lua_mmap_to_mmap_from_table(l,1);
	pb=lua_mmap_to_mmap_from_table(l,2);

	mmap_keymap(pa,pb);

	return 0;
#endif
	return 0;
}

/*----------------------------------------------------------------------------------------------------------------------------*/
//
// map tiles in one bitmpa into tiles in another bitmap
//
/*----------------------------------------------------------------------------------------------------------------------------*/
static int lua_mmap_layout (lua_State *l)
{
#if 0
metamap *pa;
metamap *pb;
s32 border;

	pa=lua_mmap_to_mmap_from_table(l,1);
	pb=lua_mmap_to_mmap_from_table(l,2);
	border=(s32)luaL_checknumber(l,3);

	mmap_layout(pa,pb,border);

	lua_devil_error_check(l);

	return 0;
#endif
	return 0;
}


/*----------------------------------------------------------------------------------------------------------------------------*/
//
// get/set a tile from lua reference be a single index or a 2d coord
//
/*----------------------------------------------------------------------------------------------------------------------------*/
static int lua_mmap_get_tile (lua_State *l)
{
#if 0
mmap_tile *m;
mmap_tile *t;
metamap *p;
s32 x,y,idx;

	lua_pushliteral(l,MMAPHANDLE); // get our data within the table
	lua_gettable(l, 1);
	p=lua_mmap_to_mmap(l,lua_gettop(l));
	lua_pop(l,1);

// 2 is a table we wish to fill in from tile

	x=(s32)lua_tonumber(l,3);

	y=0;
	if(lua_isnumber(l,4))
	{
		y=(s32)lua_tonumber(l,4);
	}

	idx=x+y*p->tw;
	if( (idx>=p->numof_tiles) || (!p->tiles) )
	{
		luaL_error(l, "requested tile out of range" );
	}

	t=p->tiles+idx;
	for( m=t ; m->master ; m=m->master );

	lua_pushliteral(l,"id");
	lua_pushnumber(l,t-t->base->tiles);
	lua_settable(l,2);

	lua_pushliteral(l,"x");
	lua_pushnumber(l,t->x);
	lua_settable(l,2);

	lua_pushliteral(l,"y");
	lua_pushnumber(l,t->y);
	lua_settable(l,2);
	
	lua_pushliteral(l,"w");
	lua_pushnumber(l,t->w);
	lua_settable(l,2);
	
	lua_pushliteral(l,"h");
	lua_pushnumber(l,t->h);
	lua_settable(l,2);

	lua_pushliteral(l,"hx");
	lua_pushnumber(l,t->hx);
	lua_settable(l,2);

	lua_pushliteral(l,"hy");
	lua_pushnumber(l,t->hy);
	lua_settable(l,2);
	

	lua_pushliteral(l,"master");
	lua_pushnumber(l,t->master?m->base->image:0);
	lua_settable(l,2);

	lua_pushliteral(l,"base");
	lua_pushnumber(l,t->base->image);
	lua_settable(l,2);

	return 0;
#endif
	return 0;
}

static int lua_mmap_get_master_tile (lua_State *l)
{
#if 0

mmap_tile *t;
mmap_tile *m;
metamap *p;
s32 x,y,idx;

	lua_pushliteral(l,MMAPHANDLE); // get our data within the table
	lua_gettable(l, 1);
	p=lua_mmap_to_mmap(l,lua_gettop(l));
	lua_pop(l,1);

// 2 is a table we wish to fill in from tile

	x=(s32)lua_tonumber(l,3);

	y=0;
	if(lua_isnumber(l,4))
	{
		y=(s32)lua_tonumber(l,4);
	}

	idx=x+y*p->tw;
	if( (idx>=p->numof_tiles) || (!p->tiles) )
	{
		luaL_error(l, "requested tile out of range" );
	}

	t=p->tiles+idx;

	for( m=t ; m->master ; m=m->master );

	lua_pushliteral(l,"id");
	lua_pushnumber(l,m-m->base->tiles);
	lua_settable(l,2);

	lua_pushliteral(l,"x");
	lua_pushnumber(l,m->x);
	lua_settable(l,2);

	lua_pushliteral(l,"y");
	lua_pushnumber(l,m->y);
	lua_settable(l,2);
	
	lua_pushliteral(l,"w");
	lua_pushnumber(l,m->w);
	lua_settable(l,2);
	
	lua_pushliteral(l,"h");
	lua_pushnumber(l,m->h);
	lua_settable(l,2);

	lua_pushliteral(l,"hx");
	lua_pushnumber(l,m->hx);
	lua_settable(l,2);

	lua_pushliteral(l,"hy");
	lua_pushnumber(l,m->hy);
	lua_settable(l,2);
	
	lua_pushliteral(l,"master");
	lua_pushnumber(l,t->master?m->base->image:0);
	lua_settable(l,2);

	lua_pushliteral(l,"base");
	lua_pushnumber(l,t->base->image);
	lua_settable(l,2);

	return 0;
#endif
	return 0;
}

static int lua_mmap_set_tile (lua_State *l)
{
#if 0
mmap_tile *t;
metamap *p;
s32 x,y,idx;

	lua_pushliteral(l,MMAPHANDLE); // get our data within the table
	lua_gettable(l, 1);
	p=lua_mmap_to_mmap(l,lua_gettop(l));
	lua_pop(l,1);

// 2 is a table containing data we want to set into the tile

	x=(s32)lua_tonumber(l,3);

	y=0;
	if(lua_isnumber(l,4))
	{
		y=(s32)lua_tonumber(l,4);
	}

	idx=x+y*p->tw;
	if( (idx>=p->numof_tiles) || (!p->tiles) )
	{
		luaL_error(l, "requested tile out of range" );
	}

	t=p->tiles+idx;

	lua_pushliteral(l,"x");
	lua_gettable(l,2);
	t->x=(s32)lua_tonumber(l,-1);
	lua_pop(l,1);

	lua_pushliteral(l,"y");
	lua_gettable(l,2);
	t->y=(s32)lua_tonumber(l,-1);
	lua_pop(l,1);
	
	lua_pushliteral(l,"w");
	lua_gettable(l,2);
	t->w=(s32)lua_tonumber(l,-1);
	lua_pop(l,1);
	
	lua_pushliteral(l,"h");
	lua_gettable(l,2);
	t->h=(s32)lua_tonumber(l,-1);
	lua_pop(l,1);

	lua_pushliteral(l,"hx");
	lua_gettable(l,2);
	t->hx=(s32)lua_tonumber(l,-1);
	lua_pop(l,1);

	lua_pushliteral(l,"hy");
	lua_gettable(l,2);
	t->hy=(s32)lua_tonumber(l,-1);
	lua_pop(l,1);
	

	return 0;
#endif
	return 0;
}

static int lua_mmap_get_numof_tiles (lua_State *l)
{
#if 0
metamap *p;

	lua_pushliteral(l,MMAPHANDLE); // get our data within the table
	lua_gettable(l, 1);
	p=lua_mmap_to_mmap(l,lua_gettop(l));
	lua_pop(l,1);

	lua_pushnumber(l,p->numof_tiles);
	
	return 1;
#endif
	return 0;
}

/*----------------------------------------------------------------------------------------------------------------------------*/
//
// map tiles in one bitmpa into tiles in another bitmap
//
/*----------------------------------------------------------------------------------------------------------------------------*/
static int lua_mmap_saveXTX (lua_State *l)
{
#if 0
metamap *pa;
metamap *pb;
const char *str;

	pa=lua_mmap_to_mmap_from_table(l,1);
	pb=lua_mmap_to_mmap_from_table(l,2);
	str = luaL_checklstring(l, 3 , 0 );

	mmap_save_XTX(pa,pb,str);

	return 0;
#endif
	return 0;
}


/*----------------------------------------------------------------------------------------------------------------------------*/
//
// library defs
//
/*----------------------------------------------------------------------------------------------------------------------------*/

static const luaL_reg lua_mmap_lib[] =
{
	
	{	"alloc",		lua_mmap_alloc	},
	{	"free",			lua_mmap_free	},
	
	{NULL, NULL}
};


// suport for one type of object

static const luaL_reg lua_mmap_meta[] =
{

	{"free",			lua_mmap_free_mmap},
	{"__gc",			lua_mmap_free_mmap},

	{NULL, NULL}
};

// and a table which contains the object
//
// all functions expect the self table to be passed in as arg1

static const luaL_reg lua_mmapt_meta[] =
{
	{"free",				lua_mmap_free},
	{"cutup",				lua_mmap_cutup},
	{"merge",				lua_mmap_merge},
	{"shrink",				lua_mmap_shrink},
	{"keymap",				lua_mmap_keymap},
	{"layout",				lua_mmap_layout},
	{"get_master_tile",		lua_mmap_get_master_tile},
	{"get_tile",			lua_mmap_get_tile},
	{"set_tile",			lua_mmap_set_tile},
	{"get_numof_tiles",		lua_mmap_get_numof_tiles},

	{"saveXTX",				lua_mmap_saveXTX},

	{NULL, NULL}
};


static void lua_mmap_meta_create (lua_State *l)
{
	luaL_newmetatable(l, MMAPHANDLE);  /* create new metatable */

	lua_pushliteral(l, "__index");
	lua_pushvalue(l, -2);  /* push metatable */
	lua_rawset(l, -3);  /* metatable.__index = metatable */

	luaL_openlib(l, NULL, lua_mmap_meta, 0);
}

static void lua_mmapt_meta_create (lua_State *l)
{
	luaL_newmetatable(l, MMAPTHANDLE);  /* create new metatable */

	lua_pushliteral(l, "__index");
	lua_pushvalue(l, -2);  /* push metatable */
	lua_rawset(l, -3);  /* metatable.__index = metatable */

	luaL_openlib(l, NULL, lua_mmapt_meta, 0);
}


/*----------------------------------------------------------------------------------------------------------------------------*/
//
// open library.
//
/*----------------------------------------------------------------------------------------------------------------------------*/

int luaopen_mmap (lua_State *l)
{
	lua_mmap_meta_create(l);
	lua_mmapt_meta_create(l);

	lua_pushstring(l, LUA_MMAPLIBNAME );
	lua_newtable(l);
	luaL_openlib(l, 0, lua_mmap_lib, 0);
	lua_rawset(l, LUA_GLOBALSINDEX);


	return 1;
}


