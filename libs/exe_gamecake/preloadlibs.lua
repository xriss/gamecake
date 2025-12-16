
	files { "preloadlibs.c" }
	
	local fp=io.open("preloadlibs.c","w")
	local t={}
	local function put(s)
		t[#t+1]=s
	end
	
	put([[
extern void wetgenes_cache_preloader(lua_State *L);
]])

	for i,v in ipairs(lua_lib_loads) do
	local n=v[1]
	local fn=v[2]
	put([[
extern int luaopen_]]..fn..[[(lua_State *L);
]])
	end
	local s1=table.concat(t)
	t={}
	
	put([[
    lua_getglobal(L, "package");
    lua_getfield(L, -1, "preload");
]])
	for i,v in ipairs(lua_lib_loads) do
	local n=v[1]
	local fn=v[2]
	put([[
    lua_pushliteral(L, "]]..n..[[");
    lua_pushcfunction(L, luaopen_]]..fn..[[);
    lua_settable(L, -3);
]])
	end

	put([[
    lua_pop(L, 2);
]])

	local s2=table.concat(t)
fp:write([[

#include "lua.h"
]]..s1..[[

extern void lua_preloadlibs(lua_State *L)
{
]]..s2..[[
	wetgenes_cache_preloader(L); // include embeded strings loader
}

]])
	fp:close()
