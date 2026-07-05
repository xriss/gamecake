
project "lua_box2d"
language "C"
files { "code/**.c" , "code/**.h" , "all.h" }
links { "lib_lua" }

includedirs { "." , "../lib_box2d/git/include" }

--buildoptions{ "-std=c99" } -- newfangled flag

KIND{lua="box2d.core"}


project "lua_box3d"
language "C"
files { "code/**.c" , "code/**.h" , "all.h" }
links { "lib_lua" }

includedirs { "." , "../lib_box3d/git/include" }

--buildoptions{ "-std=c99" } -- newfangled flag

KIND{lua="box3d.core"}
