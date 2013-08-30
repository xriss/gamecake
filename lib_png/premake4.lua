
project "lib_png"
kind "StaticLib"
language "C"
files { "lpng163/*.cpp" , "lpng163/*.c" , "lpng163/*.h" }
excludes { "lpng163/pngtest.c" , "lpng163/example.c"}

links { "lib_z" }

defines { "PNG_STATIC" }




includedirs { "lpng163" , "../lib_z" }


KIND{}
