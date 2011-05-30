

project "lua_posix"
language "C++"
files { "lposix.c" }

links { "lua51" , "crypt" }


SET_KIND("lua","posix","posix")
SET_TARGET("","posix")

