

project "lua_posix"
language "C++"
files { "lposix.c" }

links { "lib_lua" , "crypt" }


SET_KIND("lua","posix","posix")
SET_TARGET("","posix")

