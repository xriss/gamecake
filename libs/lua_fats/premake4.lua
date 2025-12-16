
project "lua_fats"
language "C"
files { "code/**.c" , "code/**.h" , "all.h" }

links { "lib_lua" }

includedirs { "." }

KIND{lua="wetgenes.fats.core"}

