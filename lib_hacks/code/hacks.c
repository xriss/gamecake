

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"


// hax to be honest, all this to create a lua_pack_toluserdata function
// if lua or luajit change then this will break
// it is however still better than not having any bounds checking

#include "wet_types.h"

// we need to keep this in sync with the lua version
// so we can extract a length from a structure we mostly do not care about
// this is an evil hack but there is no exposed way to do this apart from building your own lua

#if defined(LUA_JIT_USED)

// luajit has 64bit pointers now so use void*
typedef struct wetgenes_pack_user_data {
  struct {
    void *next;
    u8 tt;
    u8 marked;
    u8 udtype;
    u8 unused2;
    void *env;
    u32 len;
    void *metatable;
    u32 align1;
  } uv;
} wetgenes_pack_user_data;

#else

// lua hacks
typedef union { double u; void *s; long l; } wetgenes_pack_user_alignment;
typedef union wetgenes_pack_user_data {
  wetgenes_pack_user_alignment dummy;
  struct {
    void *next; char tt; char marked;
    void *metatable;
    void *env;
    size_t len;
  } uv;
} wetgenes_pack_user_data;

#endif






extern unsigned char * lua_toluserdata (lua_State *l, int idx, size_t *len)
{
	wetgenes_pack_user_data *g;

	unsigned char *p=lua_touserdata(l,idx);
	if(!p) { return 0; }
	if(len)
	{
		if(lua_islightuserdata(l,idx))
		{
			*len=0x7fffffff;
		}
		else
		{
			g=(wetgenes_pack_user_data*)(p-sizeof(wetgenes_pack_user_data));
			*len=g->uv.len;
		}	
	}
	
	return p;
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

