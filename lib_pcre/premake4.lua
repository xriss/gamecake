
project "lib_pcre"
kind "StaticLib"
language "C"
files { "**.c" , "./**.h" }
excludes { "dftables.c" }

--, "LINK_SIZE=2" ,
defines { "HAVE_CONFIG_H", "PCRE_STATIC" , "POSIX_MALLOC_THRESHOLD=10" }

includedirs { "." }

KIND{}

