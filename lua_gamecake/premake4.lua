
project "lua_gamecake"
language "C"
files { "code/**.cpp" , "code/**.c" , "code/**.h" , "all.h" }

links { "lib_lua" }

includedirs { "." }

KIND{lua="wetgenes.gamecake.core"}

