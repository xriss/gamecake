
project "lua_zip_zziplib"
kind "StaticLib"
language "C++"
files { "zzip/**.cpp" , "zzip/**.c" , "zzip/**.h" }

if os.get() == "windows" then

else -- nix

end


includedirs { "." }


configuration {"Debug"}
targetdir(DBG_OBJ_DIR)

configuration {"Release"}
targetdir(EXE_OBJ_DIR)

