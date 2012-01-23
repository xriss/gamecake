
project "lib_zzip"
kind "StaticLib"
language "C"
files { "zzip/**.c" , "zzip/**.h" }

if WINDOWS then

	defines "ZZIP_HAVE_DIRECT_H"

else -- nix

	defines "ZZIP_HAVE_BYTESWAP_H"
	defines "ZZIP_HAVE_FNMATCH_H"
	defines "ZZIP_HAVE_STRNDUP"
	
end


includedirs { "." , "../lib_z" }


configuration {"Debug"}
targetdir(DBG_OBJ_DIR)

configuration {"Release"}
targetdir(EXE_OBJ_DIR)

