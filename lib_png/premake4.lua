
project "lib_png"
kind "StaticLib"
language "C"
files { "./*.cpp" , "./*.c" , "./*.h" }
excludes { "./pngtest.c" , "./example.c"}

links { "lib_z" }

defines { "PNG_STATIC" }




includedirs { "." , "../lib_z" }


KIND{}
