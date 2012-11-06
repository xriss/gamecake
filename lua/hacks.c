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

	unsigned char *p=(unsigned char*)lua_touserdata(L,idx);
	
	if(!p) { return 0; }
	
#if defined(LIB_LUAJIT)
	g=(GCudata*)(p-sizeof(GCudata));
#else
	g=(Udata*)(p-sizeof(Udata));
#endif
	
	if(len)
	{
#if defined(LIB_LUAJIT)
		*len=g->len;
#else
		*len=g->uv.len;
#endif
	}
	
	return p;
}

#if defined(NACL)

// we should never really call kill but may accidently link to it with ome libs

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
