
project "lua_tardis"
language "C"
files { "code/**.c" , "code/**.h" , "all.h" }

links { "lib_lua" }

includedirs { "." }

KIND{lua="wetgenes.tardis.core"}

