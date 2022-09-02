/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// (C) Kriss@XIXs.com 2013
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
#include "all.h"


#define UPVALUE_LIB 1

//
// we can use either this string as a string identifier
// or its address as a light userdata identifier, both unique
//
const char *lua_grdmap_ptr_name="grdmap*ptr";


// the data pointer we are using

typedef struct grdmap * part_ptr;

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// fill a table in with the current settings
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_grdmap_getinfo (lua_State *l, part_ptr p, int tab)
{
	if(p)
	{
		lua_pushliteral(l,"tw");		lua_pushnumber(l,p->tw);		lua_rawset(l,tab);
		lua_pushliteral(l,"th");		lua_pushnumber(l,p->th);		lua_rawset(l,tab);
		
		lua_pushliteral(l,"pw");		lua_pushnumber(l,p->pw);		lua_rawset(l,tab);
		lua_pushliteral(l,"ph");		lua_pushnumber(l,p->ph);		lua_rawset(l,tab);

		lua_pushliteral(l,"err");
		if(p->err) 	{ lua_pushstring(l,p->err); }
		else		{ lua_pushnil(l); }
		lua_rawset(l,tab);
	}
	else
	{
		lua_pushliteral(l,"err"); lua_pushstring(l,"unbound grdmap"); lua_rawset(l,tab);
	}

	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// fill a table in with the current settings
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_grdmap_tile_getinfo (lua_State *l, struct grdmap_tile *p, int tab)
{
	if(p)
	{
		lua_pushliteral(l,"id");	lua_pushnumber(l,p->id);	lua_rawset(l,tab);
		
		lua_pushliteral(l,"x");		lua_pushnumber(l,p->x);		lua_rawset(l,tab);
		lua_pushliteral(l,"y");		lua_pushnumber(l,p->y);		lua_rawset(l,tab);
		lua_pushliteral(l,"w");		lua_pushnumber(l,p->w);		lua_rawset(l,tab);
		lua_pushliteral(l,"h");		lua_pushnumber(l,p->h);		lua_rawset(l,tab);
		
		lua_pushliteral(l,"hx");	lua_pushnumber(l,p->hx);	lua_rawset(l,tab);
		lua_pushliteral(l,"hy");	lua_pushnumber(l,p->hy);	lua_rawset(l,tab);
		
		if(p->master)
		{
			lua_pushliteral(l,"master");		lua_pushnumber(l,p->master->id);		lua_rawset(l,tab);
		}
		else
		{
			lua_pushliteral(l,"master");		lua_pushnil(l);		lua_rawset(l,tab);
		}
	}
	else
	{
		lua_pushliteral(l,"err"); lua_pushstring(l,"unbound grdmap tile"); lua_rawset(l,tab);
	}

	return 0;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// alloc an item, returns table that you can modify and associate extra data with
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_grdmap_create (lua_State *l)
{
part_ptr *p;
const char *s;
const char *opts=0;

//int idx_ptr;
//int idx_tab;

	p = (part_ptr *)lua_newuserdata(l, sizeof(part_ptr));
//	idx_ptr=lua_gettop(l);
	(*p)=0;
	luaL_getmetatable(l, lua_grdmap_ptr_name);
	lua_setmetatable(l, -2);

//	lua_newtable(l);
//	idx_tab=lua_gettop(l);

//	lua_pushvalue(l, idx_ptr ); // get our userdata,
//	lua_rawseti(l,idx_tab,0); // our userdata lives in tab[0]


	(*p)=grdmap_alloc();
	if(!(*p))
	{
		lua_pop(l,1); // remove userdata
		lua_pushnil(l);
		lua_pushstring(l,"failed to alloc grdmap");
	}

//	lua_grdmap_getinfo(l,*p,idx_tab);

//	lua_remove(l, idx_ptr ); // dont need pointer anymore, so just return the table
	return 1;
}


/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// __GC for ptr
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_grdmap_destroy_ptr (lua_State *l)
{	
part_ptr *p;

	p = (part_ptr *)luaL_checkudata(l, 1 , lua_grdmap_ptr_name);
	
	if(*p)
	{
		grdmap_free(*p);
	}
	(*p)=0;
	
	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// check that a table at the given index contains a grd object
// return the part_ptr if it does, otherwise return 0
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
part_ptr lua_grdmap_check (lua_State *l, int idx)
{
part_ptr p=0;

//	if(lua_istable(l,idx))
//	{
//		lua_rawgeti(l,idx,0);
		p = *((part_ptr *)luaL_checkudata(l, idx , lua_grdmap_ptr_name));
//		lua_pop(l,1);
//	}

	return p;
}
/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// lua_grdmap_check with auto error on bad ptr
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
part_ptr lua_grdmap_get (lua_State *l, int idx)
{
part_ptr p=lua_grdmap_check(l,idx);

	if (p == 0)
	{
		luaL_error(l, "bad grdmap userdata" );
	}

	return p;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// assign a grd into this grdmap, pass in null to remove
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_grdmap_setup (lua_State *l)
{
part_ptr p=lua_grdmap_get(l,1);

struct grd *g=lua_grd_check_ptr(l,2);

	if(g)
	{
		p->g=g;
//		lua_pushstring(l,"g"); // we store this here, and you should not remove it
//		lua_pushvalue(l,2);
//		lua_rawset(l,1);
	}
	else
	{
		p->g=0;
//		lua_pushstring(l,"g"); // except by calling this function again, otherewise you may get GC problems
//		lua_pushnil(l);
//		lua_rawset(l,1);
	}

	lua_pushvalue(l,1);
	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// cutup the grd into chars of w,h in size
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_grdmap_cutup (lua_State *l)
{
part_ptr p=lua_grdmap_get(l,1);
s32 px;
s32 py;

	if(!p->g)
	{
		luaL_error(l, "missing grd in grdmap" );
	}
		
	px=(s32)lua_tonumber(l,2);
	py=(s32)lua_tonumber(l,3);

	grdmap_cutup(p,px,py);
	
	if(p->err)
	{
		luaL_error(l, p->err );
	}
	
//	lua_grdmap_getinfo(l,p,1);
	
	lua_pushvalue(l,1);
	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// get information about the given tile
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_grdmap_tile (lua_State *l)
{
part_ptr p=lua_grdmap_get(l,1);

s32 id;
s32 x;
s32 y;
struct grdmap_tile *t;

	if( lua_isnumber(l,3) ) // x,y ?
	{
		x=(s32)lua_tonumber(l,2);
		y=(s32)lua_tonumber(l,3);
		
		id=x+y*p->tw;
	}
	else
	{
		id=(s32)lua_tonumber(l,2); // or just id
	}
	
	if( (id<0) || (id>=p->numof_tiles) ) { luaL_error(l, "tile out of range" ); }
	
	t=p->tiles+id;
	
	lua_newtable(l);
	lua_grdmap_tile_getinfo(l, t, lua_gettop(l) );
	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// merge similar tiles
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_grdmap_merge (lua_State *l)
{
part_ptr p=lua_grdmap_get(l,1);

	grdmap_merge(p);

	lua_pushvalue(l,1);
	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// remove whitespace from around tiles
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_grdmap_shrink (lua_State *l)
{
part_ptr p=lua_grdmap_get(l,1);

	grdmap_shrink(p);

	lua_pushvalue(l,1);
	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// keymap from one grdmap to another
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_grdmap_keymap (lua_State *l)
{
part_ptr a=lua_grdmap_get(l,1);
part_ptr b=lua_grdmap_get(l,2);

	grdmap_keymap(a,b);

	lua_pushvalue(l,1);
	return 1;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// set info into table
//
/*+-----------------------------------------------------------------------------------------------------------------+*/
int lua_grdmap_info (lua_State *l)
{
part_ptr p=lua_grdmap_get(l,1);

	lua_grdmap_getinfo(l,p,2);
	return 0;
}

/*+-----------------------------------------------------------------------------------------------------------------+*/
//
// open library.
//
/*+-----------------------------------------------------------------------------------------------------------------+*/


int luaopen_wetgenes_grdmap_core (lua_State *l)
{
	
	const luaL_Reg lib[] =
	{
		{	"create",		lua_grdmap_create			},
		{	"setup",		lua_grdmap_setup			},
		{	"info",			lua_grdmap_info				},
		{	"cutup",		lua_grdmap_cutup			},
		{	"tile",			lua_grdmap_tile				},
		{	"merge",		lua_grdmap_merge			},
		{	"shrink",		lua_grdmap_shrink			},
		{	"keymap",		lua_grdmap_keymap			},
		
		{0,0}
	};

	const luaL_Reg meta[] =
	{
		{	"__gc",			lua_grdmap_destroy_ptr		},

		{0,0}
	};


	luaL_newmetatable(l, lua_grdmap_ptr_name);
	luaL_openlib(l,0,meta,0);
	lua_pop(l,1);
	
	lua_newtable(l);
	luaL_openlib(l,0,lib,0);

	return 1;
}





#if 0

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

static const luaL_Reg lua_mmap_lib[] =
{
	
	{	"alloc",		lua_mmap_alloc	},
	{	"free",			lua_mmap_free	},
	
	{NULL, NULL}
};


// suport for one type of object

static const luaL_Reg lua_mmap_meta[] =
{

	{"free",			lua_mmap_free_mmap},
	{"__gc",			lua_mmap_free_mmap},

	{NULL, NULL}
};

// and a table which contains the object
//
// all functions expect the self table to be passed in as arg1

static const luaL_Reg lua_mmapt_meta[] =
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

int luaopen_wetgenes_grdmap_core (lua_State *l)
{
	lua_mmap_meta_create(l);
	lua_mmapt_meta_create(l);

	lua_pushstring(l, LUA_MMAPLIBNAME );
	lua_newtable(l);
	luaL_openlib(l, 0, lua_mmap_lib, 0);
	lua_rawset(l, LUA_GLOBALSINDEX);


	return 1;
}

#endif

