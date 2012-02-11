
project "lua_al"
language "C"

files { "test.c" }

links { "lib_lua" }

includedirs { "../lib_openal/soft/include"}


KIND{kind="lua",name="al"}


