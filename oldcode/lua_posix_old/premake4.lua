

project "lua_posix"
language "C++"
files { "lposix.c" }

links { "lib_lua" , "crypt" }


KIND{kind="lua",name="posix"}

