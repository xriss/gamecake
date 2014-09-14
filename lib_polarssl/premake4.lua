project "lib_polarssl"
language "C"
files { "./library/**.c" , "./library/**.h" , "./include/**.h" }

defines { "POLARSSL_HAVEGE_C" }

includedirs { "." , "include" }

KIND{}
