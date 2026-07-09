
project "lua_box2d"
language "C"
files { "code/lua_box2d.c" , "code/lua_box2d.h" }
links { "lib_lua" }

includedirs { "." , "../lib_box2d/git/include" }

KIND{lua="box2d.core"}


project "lua_box3d"
language "C"
files { "code/lua_box3d.c" , "code/lua_box3d.h" }
links { "lib_lua" }

includedirs { "." , "../lib_box3d/git/include" }

KIND{lua="box3d.core"}
