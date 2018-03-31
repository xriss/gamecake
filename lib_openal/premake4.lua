
project "lib_openal"
kind "StaticLib"
language "C"

files {
	"./mojoal/mojoal.c"
}

defines { "FLT_MAX=3.402823466e+38F"}

KIND{}
