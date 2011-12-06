
project "lua_main"
language "C"

includedirs { "../lib_lua/src" }

if NACL then -- we just link with prebuilt

--	files { "src/*.h", "src/lua.c" }

	links { "lua", "m" , "pthread" }

	kind("StaticLib")

elseif ANDROID then 

--	buildoptions()
--	buildoptions{ "-fno-stack-protector -fno-stack-protector-all" }
--	linkoptions { " -static -nodefaultlibs " }
--print( "OK",os.getenv("GCCLIB"))	
--	links { "../"..os.getenv("GCCLIB") } -- , "pthread"
--	flags { "StaticRuntime" }

--	links { "m" }

	linkoptions { " -static " }
	files { "../lib_lua/src/*.h", --[["src/lua.c"]]  }
	kind("SharedLib")

else

	kind "ConsoleApp"
	files { "../lib_lua/src/*.h", "../lib_lua/src/lua.c" }

	if os.get() == "windows" then


	else -- nix

		links { "dl" , "m" , "pthread" }
		
	end

end


--if #lua_lib_names>0 then

	links(lua_lib_names)

final_links={}

	if NACL then

	elseif ANDROID then
		
--"m", "GLESv1_CM", "dl","log"
--		linkoptions { "-lGLESv1_CM", "-llog", "-lstdc++", "-ldl", "-lm", "-lc" } --, "m", "log" }
		linkoptions { "-v" }
		links { "lib_lua" } --, "m", "log" }
		final_links={"c", "m","gcc"} 

--		linkoptions { "-L../../sdks/android-ndk/platforms/android-8/arch-arm/usr/lib" }
		
	linkoptions{ "-Bsymbolic"}

--	linkoptions{ "-Bsymbolic-functions"}
--	linkoptions{ "-Bdynamic"}

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
		
		links { "lib_lua" }

	else -- nix

-- we need to include libs again here for linking, cant prelink with statics?
-- it should probably auto handle stuff
-- anyway it gets complicated, so this is all hax

		links { "lib_lua" }
		links { "lib_z" }
--		links { "lib_sx" }
		links { "lua_grd_libpng" }
		links { "lua_zip_zziplib"}
		links { "GL" , "GLU" }
		links { "crypt" }
		
		links { "pthread" }
--		links { "sx" }
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



--end


	if NACL then

		SET_TARGET("","lua")

	elseif ANDROID then

		SET_TARGET("","liblua",true)

	else

		SET_TARGET("","lua",true)
		
	end

--linking lua with pthread makes gdb much happier when we use it later...

links(final_links) -- linker is dumb dumb dumb
