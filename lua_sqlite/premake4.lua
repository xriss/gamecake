project "lua_sqlite"
language "C"
files { "lsqlite3.c" }

links { "lib_sqlite" }

includedirs { "." , "../lib_sqlite" }


SET_KIND("lua","sqlite","lsqlite3")
SET_TARGET("","sqlite")

