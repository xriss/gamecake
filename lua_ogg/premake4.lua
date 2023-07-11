
project "lua_ogg"
language "C"
files { "code/**.cpp" , "code/**.c" , "code/**.h" , "all.h" }

links { "lib_lua" }

includedirs { "." , "../lib_vorbis/vorbis/include" , "../lib_ogg/ogg/include" }

KIND{lua="wetgenes.ogg.core"}

