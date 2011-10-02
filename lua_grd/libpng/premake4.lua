
project "lua_grd_libpng"
kind "StaticLib"
language "C++"
files { "./**.cpp" , "./**.c" , "./**.h" }

links { "lua_grd_zlib" }

defines { "PNG_STATIC" }

if os.get() == "windows" then

else -- nix

end


includedirs { "." , "../zlib" }


configuration {"Debug"}
targetdir(DBG_OBJ_DIR)


configuration {"Release"}
targetdir(EXE_OBJ_DIR)


