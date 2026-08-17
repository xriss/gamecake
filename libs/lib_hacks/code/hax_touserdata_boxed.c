

// get the userdata box, checking for metatable if given
// which may be zero for no check

static void * hax_touserdata_check(lua_State *l, int index, const char *metaname)
{
	void *p = lua_touserdata(l, index);

	if( (p != NULL) && (metaname != NULL ) )
	{
		if (lua_getmetatable(l, index))
		{
			luaL_getmetatable(l, metaname);
			if (!lua_rawequal(l, -1, -2))
			{
				p = NULL; // bad meta table name
			}
			lua_pop(l, 2);
		}
	}
	return p;
}

// lua errors if check fails or not userdata
static void * hax_touserdata_assert(lua_State *l, int index, const char *metaname)
{
	void *p = hax_touserdata_check(l, index, metaname);
	if(!p)
	{
		luaL_error(l, "invalid userdata(%s) at %d",metaname,index);
	}
	return p;
}

// get the casted contents of the checked userdata box
// lua errors if invalid box but result (contents of box) may be null
#define hax_touserdata_boxed(l,index,metaname,cast) \
	*((*(cast))lua_touserdata_assert(L,index,metaname))

