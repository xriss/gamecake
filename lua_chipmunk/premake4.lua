
project "lua_chipmunk"
language "C"
files { "code/**.c" , "code/**.h" , "all.h" }
files { "master/src/**.c" , "master/src/**.h" , "master/include/**.h" }

links { "lib_lua" }

includedirs { "." , "master/include" }

buildoptions{ "-std=c99" } -- newfangled flag

KIND{lua="wetgenes.chipmunk.core"}

