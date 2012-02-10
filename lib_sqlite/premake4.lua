
project "lib_sqlite"
kind "StaticLib"
language "C"
files { "sqlite3.c" , "./**.h" }




includedirs { "." }


KIND{}

