
project "lib_z"
kind "StaticLib"
language "C"
files { "git/*.cpp" , "git/*.c" , "git/*.h" }


includedirs { "git" }

defines { "HAVE_UNISTD_H" }

KIND{}

