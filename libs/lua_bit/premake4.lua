

project "lua_bit"
language "C"
files { "bit.c" }

links { "lib_lua" }

KIND{kind="lua",name="bit"}

