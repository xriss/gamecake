
project "lib_gif"

language "C"
files { "lib/**.c" , "lib/**.h" }

excludes { "lib/getarg.c" } 
excludes { "lib/qprintf.c" } 

links { "lib_z" }

defines { "HAVE_CONFIG_H" }

includedirs { "." , "../lib_z" }


KIND{}
