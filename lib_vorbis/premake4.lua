
project "lib_vorbis"

language "C"

files { "lib/**.c" , "include/**.h" }

-- these files have a main function, so need to be skipped
excludes { "lib/psytune.c" }
excludes { "lib/barkmel.c" }
excludes { "lib/tone.c" }


includedirs { "." , "lib" , "include" , "../lib_ogg/include" }


KIND{}

