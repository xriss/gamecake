

// get the userdata box, checking for metaname if given
// metaname may be zero for no meta check
static void ** hax_touserdata_check(lua_State *l, int index, const char *metaname)
{
	void **p = lua_touserdata(l, index);

	if( (p) && (metaname) )
	{
		if (lua_getmetatable(l, index))
		{
			luaL_getmetatable(l, metaname);
			if (!lua_rawequal(l, -1, -2))
			{
				p = 0; // bad meta table name
			}
			lua_pop(l, 2);
		}
	}
	return p;
}

// lua errors if check fails or not userdata
// metaname may be zero for no meta check
static void ** hax_touserdata_assert(lua_State *l, int index, const char *metaname)
{
	void **p = hax_touserdata_check(l, index, metaname);
	if(!p)
	{
		luaL_error(l, "invalid userdata(%s) at index %d",metaname,index);
	}
	return p;
}

// lua errors if check fails or not userdata or contents of box is 0
// metaname may be zero for no meta check
static void * hax_touserdata_boxed(lua_State *l, int index, const char *metaname)
{
	void **p = hax_touserdata_assert(l, index, metaname);
	if(!(*p))
	{
		luaL_error(l, "empty boxed userdata(%s) at index %d",metaname,index);
		return 0;
	}
	return *p;
}

// generally the box contains a ptr, but sometimes it contains a handle
// get the casted contents of the checked userdata box (probably not a ptr)
// lua errors if invalid box but result (contents of box) may be null
// so you will still need to check for bad return
// metaname may be zero for no meta check
#define hax_touserdata_cast(l,index,metaname,cast) \
	*((*(cast))lua_touserdata_assert(l,index,metaname))

