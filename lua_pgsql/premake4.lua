project "lua_pgsql"
language "C"
files { "./luapgsql.c" }

includedirs { "." , "../lib_pq/src/include/" }

KIND{lua="pgsql"}
