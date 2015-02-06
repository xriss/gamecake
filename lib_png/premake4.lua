
project "lib_png"
kind "StaticLib"
language "C"

local D="./fixed"

files { D.."/*.cpp" , D.."/*.c" , D.."/*.h" }
excludes { D.."/pngtest.c" , D.."/example.c"}

links { "lib_z" }

defines { "PNG_STATIC" }




includedirs { "." , "../lib_z" }


KIND{}
