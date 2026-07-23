
project "lua_wire"
language "C"
files { "code/lua_wire.c" }
links { "lib_lua" }
links { "lib_c11threads" }

includedirs { "." , "./code" }
includedirs { "../lib_c11threads/git" }

KIND{lua="wire.core"}

