
project "lua_box2d"
language "C"
files { "code/lua_box2d.c" }
links { "lib_lua" }

includedirs { "." , "./code" }
includedirs { "../lib_box2d/git/include" }

includedirs { "../lua_wire/c11threads/git" }

KIND{lua="wetgenes.box2d.core"}


project "lua_box3d"
language "C"
files { "code/lua_box3d.c" }
links { "lib_lua" }

includedirs { "." , "./code" }
includedirs { "../lib_box3d/git/include" }

includedirs { "../lua_wire/c11threads/git" }

KIND{lua="wetgenes.box3d.core"}
