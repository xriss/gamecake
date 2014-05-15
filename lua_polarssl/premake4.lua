project "lua_polarssl"
language "C"
files { "./src/luapolarssl.c" }

includedirs { "./src" , "../lib_polarssl/include" }

KIND{lua="polarssl"}
