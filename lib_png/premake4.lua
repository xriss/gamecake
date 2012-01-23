
project "lib_png"
kind "StaticLib"
language "C++"
files { "./**.cpp" , "./**.c" , "./**.h" }

links { "lib_z" }

defines { "PNG_STATIC" }




includedirs { "." , "../lib_z" }


configuration {"Debug"}
targetdir(DBG_OBJ_DIR)


configuration {"Release"}
targetdir(EXE_OBJ_DIR)


