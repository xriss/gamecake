
project "lua_fenestra"
language "C++"
files { "code/**.cpp" , "code/**.c" , "code/**.h" , "all.h" }

includedirs { "../lua_freetype/freetype/include/" }
includedirs { "../lib_sx/src/" }

links { "lib_lua" }

defines { "LUA_LIB" }

if WINDOWS then
	links { "opengl32" , "glu32" }
else -- nix
	links { "GL" , "GLU"  }
--[[
	local fp=assert(io.popen("pkg-config --cflags libsx"))
	local s=assert(fp:read("*l"))
	buildoptions { s }
	fp:close()

	local fp=assert(io.popen("pkg-config --libs libsx"))
	local s=assert(fp:read("*l"))
	linkoptions { s }
	fp:close()
]]

end


includedirs { "." }

KIND{kind="lua",dir="fenestra",name="core",luaname="fenestra.core",luaopen="fenestra_core"}

