
project "lua_al"
language "C"

files { "code/*.c" }

links { "lib_lua" , "lua_pack" }


KIND{lua="al.core"}


project "lua_alc"
language "C"

files { "code/lua_alc.c" }

links { "lib_lua" , "lua_pack" }


KIND{lua="alc.core"}

