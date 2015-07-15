project "lua_polarssl"
language "C"
files { "./src/luapolarssl.c" }

defines { "POLARSSL_HAVEGE_C" }

includedirs { "./src" , "../lib_polarssl/include" }

KIND{lua="polarssl"}
