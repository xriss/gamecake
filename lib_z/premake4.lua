
project "lib_z"
kind "StaticLib"
language "C"
files { "./**.cpp" , "./**.c" , "./**.h" }

if os.get() == "windows" then

else -- nix

end


includedirs { "." }


configuration {"Debug"}
targetdir(DBG_OBJ_DIR)

configuration {"Release"}
targetdir(EXE_OBJ_DIR)

