

project "lua_midialsa"
language "C"
files { "midialsa.c" }

-- this lib is not thread safe so will need to be tweaked...

links { "lib_lua" }

KIND{kind="lua",name="midialsa_core"}
