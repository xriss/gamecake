
project "lib_jpeg"
kind "StaticLib"
language "C++"
files { "./**.cpp" , "./**.c" , "./**.h" }
excludes { "jmemdos.c" ,  "jmemmac.c" , "ansi2knr.c" , "example.c" }

defines { "JPEGSTATIC" }



includedirs { "." }

KIND{}


