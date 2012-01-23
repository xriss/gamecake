
project "lib_z"
kind "StaticLib"
language "C"
files { "./**.cpp" , "./**.c" , "./**.h" }




includedirs { "." }


configuration {"Debug"}
targetdir(DBG_OBJ_DIR)

configuration {"Release"}
targetdir(EXE_OBJ_DIR)

