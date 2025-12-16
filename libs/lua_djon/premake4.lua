
project "lua_djon"
language "C"
includedirs { "git/c" }
files { "git/lua/**.c" , "git/lua/**.h" }
links { "lib_lua" }

KIND{lua="djon.core"}
