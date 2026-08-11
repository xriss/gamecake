
project "lib_gif"

language "C"
files { "giflib/*.c" , "giflib/*.h" }

--excludes { "giflib/lib/getarg.c" } 
--excludes { "giflib/lib/qprintf.c" } 

links { "lib_z" }

--defines { "HAVE_CONFIG_H" }

includedirs { "." , "../lib_z/git" }


KIND{}
