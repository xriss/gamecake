
project "lua_al"
language "C"

files { "code/*.c" }

links { "lib_lua" , "lua_pack" }

includedirs { "../lib_hacks/code" }

KIND{lua="wetgenes.al.core"}


project "lua_alc"
language "C"

files { "code/lua_alc.c" }

links { "lib_lua" , "lua_pack" }

includedirs { "../lib_hacks/code" }

KIND{lua="wetgenes.alc.core"}
