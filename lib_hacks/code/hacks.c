

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"


// hax to be honest, all this to create a lua_pack_toluserdata function
// if lua or luajit change then this will break
// it is however still better than not having any bounds checking

#include "wet_types.h"

// even more evil hack
// we create some userdata of various sizes and find where lua or luajit is keeping the size value.
// then we remember that location
static int userdata_size_offset=0;
static int get_userdata_size_offset( lua_State *l )
{
	int i;
	u32 *p;
	u32 t;
	if(userdata_size_offset==0) // go fish
	{
		for(i=-1;i>-16;i--)
		{
			p=(u32*)lua_newuserdata(l,42);
			t=*(p+i);
			lua_pop(l,1);
			if( t == 42 )
			{
				p=(u32*)lua_newuserdata(l,23);
				t=*(p+i);
				lua_pop(l,1);
				if( t == 23 )
				{
					p=(u32*)lua_newuserdata(l,19);
					t=*(p+i);
					lua_pop(l,1);
					if( t == 19 )
					{
						userdata_size_offset=i;
						break;
					}
				}
			}
		}
	}
	return userdata_size_offset;
}



extern unsigned char * lua_toluserdata (lua_State *l, int idx, size_t *len)
{
	int hax=get_userdata_size_offset(l);
	u32 *t=(u32*)lua_touserdata(l,idx);
	if(!t) { return 0; }
	if(len)
	{
		if(lua_islightuserdata(l,idx))
		{
			*len=0x7fffffff;
		}
		else
		{
			*len=(size_t)(*(t+hax));
		}
	}
	
	return (unsigned char *)t;
}


//#if defined(LUA_JITLIBNAME)
extern void * luaL_wetestudata(lua_State *L, int index, const char *tname)
{
	void *p = lua_touserdata(L, index);

	if (p != NULL) {
		if (lua_getmetatable(L, index)) {
			luaL_getmetatable(L, tname);

			if (!lua_rawequal(L, -1, -2))
				p = NULL;

			lua_pop(L, 2);

			return p;
		}
	}

	return NULL;
}
//#endif

