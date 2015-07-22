
project "lua_opus"
language "C"
files { "code/**.cpp" , "code/**.c" , "code/**.h" , "all.h" }

links { "lib_lua" }

includedirs { "." , "../lib_opus/include" }

KIND{lua="wetgenes.opus.core"}

