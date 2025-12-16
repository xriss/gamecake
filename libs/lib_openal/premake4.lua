
project "lib_openal"
kind "StaticLib"
language "C"

files {
	"./mojoal/mojoal.c"
}

includedirs {
	"./mojoal/AL"
}

--defines { "FLT_MAX=3.402823466e+38F"}

KIND{}
