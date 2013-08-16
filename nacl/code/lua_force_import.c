#include "lua.h"

extern int luaopen_wetgenes_win_nacl(lua_State *l);

extern int fakeimportfunction()
{
	luaopen_wetgenes_win_nacl_core(0);
	return 0;
}

