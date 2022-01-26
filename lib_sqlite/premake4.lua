
project "lib_sqlite"
kind "StaticLib"
language "C"
files { "sqlite-amalgamation/sqlite3.c" , "./sqlite-amalgamation/**.h" }


includedirs { "sqlite-amalgamation" }


KIND{}

