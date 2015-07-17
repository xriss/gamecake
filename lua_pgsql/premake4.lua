project "lua_pgsql"
language "C"
files { "./luapgsql.c" }

includedirs { "." , "../lib_pq/src/include/" , "../lib_pq/src/interfaces/libpq/" }

if NIX then
	defines( "_GNU_SOURCE" )
end

KIND{lua="pgsql"}


