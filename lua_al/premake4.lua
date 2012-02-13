
project "lua_al"
language "C"

files { "code/*.c" }

links { "lib_lua" }

includedirs { "../lib_openal/soft/include"}


KIND{lua="al.core"}


project "lua_alc"
language "C"

files { "code/lua_alc.c" }

links { "lib_lua" }

includedirs { "../lib_openal/soft/include" }


KIND{lua="alc.core"}

