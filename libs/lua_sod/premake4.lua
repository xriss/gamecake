
project "lua_sod"
language "C++"
files { "code/**.cpp" , "code/**.c" , "code/**.h" , "all.h" }

links { "lib_lua" }

includedirs { "." }


KIND{lua="wetgenes.sod.core"}

