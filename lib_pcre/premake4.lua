
project "lib_pcre"
kind "StaticLib"
language "C"
files { "**.c" , "./**.h" }
excludes { "dftables.c" }

if os.get() == "windows" then

else -- nix

end

--, "LINK_SIZE=2" ,
defines { "HAVE_CONFIG_H", "PCRE_STATIC" , "POSIX_MALLOC_THRESHOLD=10" }

includedirs { "." }


configuration {"Debug"}
targetdir(DBG_OBJ_DIR)

configuration {"Release"}
targetdir(EXE_OBJ_DIR)

