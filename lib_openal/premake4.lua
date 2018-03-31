
project "lib_openal"
kind "StaticLib"
language "C"

files {
	"mojoal/mojoal.c"
}

includedirs { ".","mojoal" }

KIND{}
