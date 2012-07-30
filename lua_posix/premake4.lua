

project "lua_posix"
language "C++"
files { "lposix.c" , "lcurses.c" , "strlcpy.c" }
includedirs { "." }
links { "lib_lua" , "crypt" }


KIND{kind="lua",name="posix"}

