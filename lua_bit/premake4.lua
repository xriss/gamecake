

project "lua_bit"
language "C++"
files { "bit.c" }

links { "lua51" }

SET_KIND("lua","bit","bit")
SET_TARGET("","bit")

