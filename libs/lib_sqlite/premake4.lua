
project "lib_sqlite"
kind "StaticLib"
language "C"
files { "sqlite-amalgamation/sqlite3.c" , "./sqlite-amalgamation/**.h" }


includedirs { "sqlite-amalgamation" }

defines{
	"SQLITE_ENABLE_JSON1",
	"SQLITE_ENABLE_FTS5",
	"SQLITE_ENABLE_RTREE",
	"SQLITE_ENABLE_DBSTAT_VTAB",
	"SQLITE_ENABLE_MATH_FUNCTIONS",
	"SQLITE_ENABLE_EXPLAIN_COMMENTS",
}


KIND{}

