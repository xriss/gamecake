
project "lua_v4l2"
language "C"
files { "code/**.cpp" , "code/**.c" , "code/**.h" , "all.h" }

links { "lib_lua" }

includedirs { "." }

KIND{lua="wetgenes.v4l2.core"}

