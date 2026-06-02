
project "lua_dumbft"
language "C"
includedirs { "code" }
files { "code/**.c" , "code/**.h" }
links { "lib_lua" }

KIND{lua="dumbft.core"}

