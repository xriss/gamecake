
project "lib_vorbis"

language "C"

files { "lib/**.c" , "include/**.h" }

excludes { "lib/psytune.c" }


includedirs { "." , "lib" , "include" }


KIND{}

