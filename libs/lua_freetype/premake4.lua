
project "lua_freetype"
language "C"

includedirs { "../lib_freetype/freetype/include/" , "." }

files { "code/**.cpp" , "code/**.c" , "code/**.h" , "all.h" }

links { "lib_lua" }

KIND{kind="lua",dir="wetgenes/freetype",name="core",luaname="wetgenes.freetype.core",luaopen="wetgenes_freetype_core"}

