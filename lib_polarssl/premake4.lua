
project "lib_polarssl"
kind "StaticLib"
language "C"
files { "./library/**.c" , "./library/**.h" }
defines { "POLARSSL_HAVEGE_C" }
includedirs { "." , "include" }

KIND{}
