project "lua_sqlite"
language "C"
files { "lsqlite3.c" }

links { "lib_sqlite" }

includedirs { "." , "../lib_sqlite/sqlite-amalgamation" }


KIND{kind="lua",name="lsqlite3"}


