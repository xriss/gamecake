project "lua_pgsql"
language "C"
files { "./luapgsql.c" }

includedirs { "." , "../lib_pq/src/include/" , "../lib_pq/src/interfaces/libpq/" }

KIND{lua="pgsql"}
