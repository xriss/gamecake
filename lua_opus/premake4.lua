
project "lua_opus"
language "C"
files { "code/**.cpp" , "code/**.c" , "code/**.h" , "all.h" }

links { "lib_lua" }

defines { "HAVE_STDINT_H" }  -- the speexdsp include needs this

includedirs { "." , "../lib_opus/opus/include" , "../lib_speexdsp/speexdsp/include" }

KIND{lua="wetgenes.opus.core"}

