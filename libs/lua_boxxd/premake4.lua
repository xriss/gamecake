
project "lua_box2d"
language "C"
files { "code/lua_box2d.c" }
links { "lib_lua" }
links { "lib_c11threads" }

includedirs { "." , "./code" }
includedirs { "../lib_box2d/git/include" }
includedirs { "../lib_c11threads/git" }

KIND{lua="box2d.core"}


project "lua_box3d"
language "C"
files { "code/lua_box3d.c" }
links { "lib_lua" }
links { "lib_c11threads" }

includedirs { "." , "./code" }
includedirs { "../lib_box3d/git/include" }
includedirs { "../lib_c11threads/git" }

KIND{lua="box3d.core"}
