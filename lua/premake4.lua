
if not NACL then -- we just link with prebuilt?

project "lua51"
language "C++"
files { "src/*.h", "src/*.cpp", "src/*.c" }
excludes { "src/lua.c" }
includedirs { "src" }

SET_KIND("luamain")

if NIX then
printf("MKSTEMP")
	defines("LUA_USE_MKSTEMP") -- remove warning
end

if #lua_lib_names>0 then
	defines("LUA_PRELOADLIBS=lua_preloadlibs")
end


configuration {"Debug"}
targetdir(DBG_OBJ_DIR)

configuration {"Release"}
targetdir(EXE_OBJ_DIR)

end




project "lua_main"
language "C++"

includedirs { "src" }

if NACL then -- we just link with prebuilt

--	files { "src/*.h", "src/lua.c" }

	links { "lua", "m" , "pthread" }

	kind("StaticLib")

elseif ANDROID then 

	files { "src/*.h", "src/lua.c" }

	links { "m" , "pthread" }

	kind("SharedLib")

else

	kind "ConsoleApp"
	files { "src/*.h", "src/lua.c" }

	if os.get() == "windows" then


	else -- nix

		links { "dl" , "m" , "pthread" }
		
	end

end


if #lua_lib_names>0 then

	links(lua_lib_names)


	if NACL then

	elseif ANDROID then

		links { "lua51" }

	elseif os.get() == "windows" then

--[[
		links { "opengl32" , "glu32" }
		links "gdi32"
		links "winmm"
		links "libmySQL"
		links { "WS2_32" , "libcrypto" , "libssl" }
		
		libdirs "../lua_sec/openssl/lib"
		libdirs { "../windows/mysql51/lib" }
]]
		
		links { "lua51" }

	else -- nix

		links { "lua51" }
		links {  "lua_grd_libpng" , "lua_grd_zlib" }
	
		links { "GL" , "GLU" }
		links { "crypt" }
		
		links { "pthread" }
		links { "sx" }
--		links { "ssl" }
--		links { "mysqlclient" }

--[[
		local fp=assert(io.popen("pkg-config --libs libsx"))
		local s=assert(fp:read("*l"))
		linkoptions { s }
		fp:close()
]]		
	end

	files { "preloadlibs.c" }

	local fp=io.open("preloadlibs.c","w")
	local t={}
	local function put(s)
		t[#t+1]=s
	end

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
}

]])
	fp:close()



end


	if NACL then

		SET_TARGET("","lua")

	elseif ANDROID then

		SET_TARGET("","luandroid",true)

	else

		SET_TARGET("","lua",true)
		
	end

--linking lua with pthread makes gdb much happier when we use it later...

