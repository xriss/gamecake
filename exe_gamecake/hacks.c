// hax to be honest, all this to create a lua_toluserdata function
// if lua or luajit change then this will break
// it is however still better than not having any bounds checking

#if defined(LIB_LUAJIT)
#include "../lib_luajit/src/lj_obj.h"
#else
#include "../lib_lua/src/lobject.h"
#endif

extern unsigned char * lua_toluserdata (lua_State *L, int idx, size_t *len)
{

#if defined(LIB_LUAJIT)
	GCudata *g;
#else
	Udata *g;
#endif

	unsigned char *p=0;
	
	if(lua_isuserdata(L,idx))
	{
		p=(unsigned char*)lua_touserdata(L,idx);
	}
	
	if(!p) { return 0; }
		
	if(len)
	{
#if defined(LIB_LUAJIT)
		g=(GCudata*)(p-sizeof(GCudata));
		*len=g->len;
#else
		g=(Udata*)(p-sizeof(Udata));
		*len=g->uv.len;
#endif
	}
	
	return p;
}

#if defined(NACL)

// we should never really call kill but may accidently link to it with some libs

int kill(int pid, int sig)
{
//	if(pid == __MYPID)
	{
		while(1)
		{
		}
	}
	return 0;
}

#endif

// fix openlib library names?
//void LUA_API luaopen_lanes( lua_State* L)
//{
//	luaopen_lanes_embedded(L, 0);
//}



#if defined(__LSB_VERSION__) // LSB hacks

#include <stdarg.h>
extern int __isoc99_sscanf(const char *a, const char *b, va_list args)
{
   int i;
   va_list ap;
   va_copy(ap,args);
   i=sscanf(a,b,ap);
   va_end(ap);
   return i;
}

extern char * secure_getenv (char const *name)
{
  return getenv (name);
}

#endif

