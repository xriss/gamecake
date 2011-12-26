
project "lib_zzip"
kind "StaticLib"
language "C"
files { "zzip/**.c" , "zzip/**.h" }

if os.get() == "windows" then

else -- nix

end


includedirs { "." , "../lib_z" }


configuration {"Debug"}
targetdir(DBG_OBJ_DIR)

configuration {"Release"}
targetdir(EXE_OBJ_DIR)

