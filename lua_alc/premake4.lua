
project "lua_alc"
language "C"

files { "code/lua_alc.c" }

links { "lib_lua" }

includedirs { "../lib_openal/soft/include"}


KIND{lua="alc.core"}

