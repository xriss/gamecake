
project "lua_limp"
language "C"
includedirs { "code/" }
files { "code/**.c" , "code/**.h" }
links { "lib_lua" }

KIND{lua="limp.core"}
