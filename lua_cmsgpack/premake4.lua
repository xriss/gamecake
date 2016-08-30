
project "lua_cmsgpack"
language "C"
files { "lua_cmsgpack.c" }

links { "lib_lua" }

includedirs { "." }

KIND{lua="cmsgpack"}

