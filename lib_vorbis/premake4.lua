
project "lib_vorbis"

language "C"

files { "vorbis/lib/**.c" , "vorbis/include/**.h" }

-- these files have a main function, so need to be skipped
excludes { "vorbis/lib/psytune.c" }
excludes { "vorbis/lib/barkmel.c" }
excludes { "vorbis/lib/tone.c" }


includedirs { "vorbis" , "vorbis/lib" , "vorbis/include" , "../lib_ogg/ogg/include" }


KIND{}

