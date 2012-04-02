
project "lib_jpeg"
kind "StaticLib"
language "C++"
files { "./**.cpp" , "./**.c" , "./**.h" }
excludes { "jmemdos.c" ,  "jmemmac.c" , "ansi2knr.c" , "example.c" }
excludes { "cjpeg.c", "djpeg.c" , "rdjpgcom.c" , "wrjpgcom.c" , "ckconfig.c" , "jpegtran.c" }

defines { "JPEGSTATIC" }



includedirs { "." }

KIND{}


