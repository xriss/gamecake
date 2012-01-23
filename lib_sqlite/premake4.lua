
project "lib_sqlite"
kind "StaticLib"
language "C"
files { "sqlite3.c" , "./**.h" }




includedirs { "." }


configuration {"Debug"}
targetdir(DBG_OBJ_DIR)

configuration {"Release"}
targetdir(EXE_OBJ_DIR)

