
project "lib_zip"

language "C"
files { "./lib/**.c" }


includedirs { "." , "./lib" , "../lib_z" }


KIND{}

