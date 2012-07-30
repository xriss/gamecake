

project "lua_posix"
language "C"
files { "lposix.c" , "strlcpy.c" }
includedirs { "." }
links { "lib_lua" , "crypt" }


KIND{kind="lua",name="posix_c"}

