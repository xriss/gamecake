

project "lua_bit"
language "C++"
files { "bit.c" }

links { "lib_lua" }

SET_KIND("lua","bit","bit")
SET_TARGET("","bit")

